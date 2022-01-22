/**
 * @file user_input.c
 * @author Justen Di Ruscio
 * @brief Definitions related to accepting user input to control player
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#define _POSIX_C_SOURCE 199309L

#include <caterpillar/user/user_input.h>

#include <errno.h>
#include <stdio.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include <caterpillar/bullets/bullet.h>
#include <caterpillar/game/constants.h>
#include <caterpillar/game/game_console.h>
#include <caterpillar/user/player.h>

// ========================== Command Declarations ============================
// available input commands. same order as inputCmdChars
typedef enum Command {
  cmd_Up,
  cmd_Left,
  cmd_Down,
  cmd_Right,
  cmd_FireBullet,
  cmd_Unknown
} Command;
// input command types
typedef enum CmdType { cmdType_Bullet, cmdType_Move, cmdType_Unknown } CmdType;

// ========================== Global Constants ============================
#define NSEC_TO_SEC 1000000000    // conversion ratio
#define INPUT_TIMEOUT_USEC 200000 // micro-seconds duration to timeout input
// input chars which associate to commands
const char inputCmdChars[] = {'w', 'a', 's', 'd', ' '};
const int inputCmdsLength = 5;
// mapping of Command (index) to command type. Same order as inputCmdChars
const CmdType commandTypes[] = {cmdType_Move, cmdType_Move,   cmdType_Move,
                                cmdType_Move, cmdType_Bullet, cmdType_Unknown};

// ========================== Support Functions ============================
/**
 * @brief Returns the Command identified by inputCmd
 *
 * @param inputCmd character to find in inputCmdChars; user provided input
 * command
 * @return Command the identified command
 */
static Command getCommand(const char inputCmd) {
  int index = cmd_Unknown;
  for (int i = 0; i < inputCmdsLength; ++i) {
    if (inputCmdChars[i] == inputCmd) {
      index = i;
      break;
    }
  }
  return (Command)index;
}

/**
 * @brief Reads character from stdin. Places its respective Command in cmd.
 * errno set on error
 *
 * @param cmd pointer to command in which identified command will be placed
 * @return true successfully read command from stdin
 * @return false error occurred; errno set, too
 */
static bool readUserCmd(Command *const cmd) {
  const char fooName[] = "readUserCmd";
  char inputChar;
  scanf("%c", &inputChar);
  if (errno != 0) {
    fprintf(stderr,
            "Input scanning error while trying to read user input in %s\n",
            fooName);
    return false;
  }
  *cmd = getCommand(inputChar);
  return true;
}

static double timespecToSec(const struct timespec t) {
  double secs = (double)t.tv_sec;
  secs += (double)t.tv_nsec / NSEC_TO_SEC;
  return secs;
}

static int executeCommand(const Command cmd, Player *const p,
                          double *const lastMoveInstant,
                          double *const lastBulletInstant) {
  const char fooName[] = "executeCommand";

  // Argument Validity Check
  errno = 0;
  if (p == (Player *)NULL) {
    fprintf(stderr, "argument 'p' of %s must point to the current player\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Execute command if it's known
  if (cmd != cmd_Unknown) {
    // Skip command if executing would exceed max rate
    double *lastExInstant = lastMoveInstant;
    double minCmdDuration = MOVE_MIN_SECS;
    const CmdType cmdType = commandTypes[(int)cmd];
    if (cmdType == cmdType_Bullet) {
      lastExInstant = lastBulletInstant;
      minCmdDuration = BULLET_MIN_SECS;
    }
    struct timespec t;
    const int err = clock_gettime(CLOCK_MONOTONIC, &t);
    if (err < 0) {
      fprintf(stderr, "Failure getting time with clock_gettime in %s\n",
              fooName);
      return errno; // errno set by clock_gettime
    }
    const double currentCmdInstant = timespecToSec(t);
    const double duration = currentCmdInstant - *lastExInstant;
    if (duration >= minCmdDuration) {
      *lastExInstant = currentCmdInstant;
    } else {
      return 0;
    }

    // Execute corresponding command
    char *errMsg;
    switch (cmd) {
    case cmd_Up:
      errno = movePlayer(p, p->row + UP, p->col, true);
      if (errno != 0) {
        errMsg = "move player up";
      }
      break;
    case cmd_Down:
      errno = movePlayer(p, p->row + DOWN, p->col, true);
      if (errno != 0) {
        errMsg = "move player down";
      }
      break;
    case cmd_Left:
      errno = movePlayer(p, p->row, p->col + LEFT, true);
      if (errno != 0) {
        errMsg = "move player left";
      }
      break;
    case cmd_Right:
      errno = movePlayer(p, p->row, p->col + RIGHT, true);
      if (errno != 0) {
        errMsg = "move player left";
      }
      break;
    case cmd_FireBullet:
      errno = shootBullet(p->row, p->col + (PLAYER_WIDTH >> 1), b_Player, p);
      if (errno != 0) {
        errMsg = "fire player's bullet";
      }
      break;
    default:;
    }

    if (errno != 0) {
      fprintf(stderr, "Error trying to execute command to %s in %s\n", errMsg,
              fooName);
    }
  }
  return errno;
}

// ==================== User Input Thread Start Function ======================
void *acceptUserInput(void *data) {
  const char fooName[] = "acceptUserInput";
  const UserInputArg *const arg = (UserInputArg *)data;

  // Argument Validity Checks
  errno = 0;
  if (data == (void *)NULL) {
    fprintf(stderr, "argument 'data' of %s must point to a valid UpkeepArg\n",
            fooName);
    errno = EPERM;
  } else if (arg->p == (Player *)NULL) {
    fprintf(stderr,
            "member 'p' of argument 'data' of %s must point to the current "
            "player\n",
            fooName);
    errno = EPERM;
  } else if (arg->sleepGame == (Task *)NULL) {
    fprintf(stderr,
            "member 'sleepGame' of argument 'data' of %s must point to the "
            "main Task used to sleep the game\n",
            fooName);
    errno = EPERM;
  }

  // Accept user's input and act on it
  else {
    // command rate limiting
    double lastMoveInstantSecs;   // when last move command was executed
    double lastBulletInstantSecs; // when last bullet was executed

    // user input reading
    fd_set readfds;              // set of file descriptors for select to watch
    struct timeval inputTimeout; // timeout interval to stop waiting for input
                                 // and check if game completed

    // Continuously read user input and execute commands
    while (!arg->sleepGame->completed) {
      // order select to wait for input on stdin (fd 0)
      FD_ZERO(&readfds); // init rfds to null set
      FD_SET(STDIN_FILENO, &readfds);

      // order select to timeout after 0.5 sec
      inputTimeout.tv_sec = 0;
      inputTimeout.tv_usec = INPUT_TIMEOUT_USEC;

      // Wait for input to become present on stdin
      // readfds & inputTimeout modified by select
      const int numSetBits =
          select(STDIN_FILENO + 1, &readfds, NULL, NULL, &inputTimeout);
      if (numSetBits < 0) {
        fprintf(
            stderr,
            "Failure in %s waiting for input to become available on stdin\n",
            fooName);
        break; // errno set by select
      }

      // Input is present on stdin -> read input and execute command
      if (FD_ISSET(STDIN_FILENO, &readfds)) {
        // consume char from stdin, identifying its associated command
        Command inputCmd;
        const bool success = readUserCmd(&inputCmd);
        if (!success) {
          fprintf(stderr, "Failure reading usr input command in %s\n", fooName);
          break; // errno set by readUserCmd
        }

        // execute input char's corresponding command
        errno = executeCommand(inputCmd, arg->p, &lastMoveInstantSecs,
                               &lastBulletInstantSecs);
        if (errno != 0) {
          fprintf(stderr,
                  "Error in %s while executing command for user input %c\n",
                  fooName, inputCmdChars[(int)inputCmd]);
          break;
        }
      }
    }
  }

  // Stop game if error occurred
  if (errno != 0) {
    const int err = task_markCompleted(arg->sleepGame);
    if (err != 0) {
      fprintf(stderr, "Error while trying to mark game as completed in %s\n",
              fooName);
    }
  }

  return (void *)(size_t)errno;
}
