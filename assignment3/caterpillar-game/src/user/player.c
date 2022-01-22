/**
 * @file player.c
 * @author Justen Di Ruscio
 * @brief Definitions related to caterpillar game player, including space ship
 * on screen
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <caterpillar/user/player.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>

#include <caterpillar/bullets/bullet.h>
#include <caterpillar/bullets/bullet_collisions.h>
#include <caterpillar/game/constants.h>
#include <caterpillar/game/game_console.h>

// ========================== Global Constants ============================
#define PLAYER_ANIM_TILES 3
#define PLAYER_ANIM_TICKS 50

char *playerGraphic[PLAYER_ANIM_TILES][PLAYER_HEIGHT] = {
    {"/o\\", "|||", "/^\\"}, {"/|\\", "|o|", "/^\\"}, {"/|\\", "|||", "/*\\"}};

const int lowerBound = SCR_TOP + GAME_ROWS - PLAYER_HEIGHT;
const int upperBound = SCR_TOP + BOUNDARY_ROW + DOWN;
const int leftBound = SCR_LEFT;
const int rightBound = SCR_LEFT + GAME_COLS - PLAYER_WIDTH;

// ========================== Support Functions ============================
/**
 * @brief Resets player to start location
 *
 * @param p player
 * @return int errno
 */
static int resetPlayer(Player *const p) {
  const char fooName[] = "resetPlayer";

  // Argument Validity Check
  errno = 0;
  if (p == (Player *)NULL) {
    fprintf(stderr, "argument 'p' of %s must point to the current player\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Reset Player
  p->row = p->startRow;
  p->col = p->startCol;
  p->animTile = PLAYER_ANIM_TILES - 1;
  p->state = GAME;
  return errno;
}

/**
 * @brief Redraws player on consol at provided location
 *
 * @param p player
 * @param row
 * @param col
 * @return int errno
 */
static int redrawPlayer(const Player *const p, const int row, const int col) {
  const char fooName[] = "playerRedraw";

  // Argument Validity Check
  errno = 0;
  if (p == (Player *)NULL) {
    fprintf(stderr, "argument 'p' of %s must point to a valid Player\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Draw moved player on game board
  errno = pthread_mutex_lock(&consoleMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to obtain lock before drawing player to screen "
            "in %s\n",
            fooName);
    return errno;
  }

  char **playerImg = playerGraphic[p->animTile];
  consoleClearImage(p->row, p->col, PLAYER_HEIGHT, PLAYER_WIDTH);
  consoleDrawImage(row, col, playerImg, PLAYER_HEIGHT);

  errno = pthread_mutex_unlock(&consoleMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to release lock after drawing player to screen "
            "in %s\n",
            fooName);
  }
  return errno;
}

/**
 * @brief Limits provided location to player boundar
 *
 * @param row
 * @param col
 */
static void clampLocation(int *const row, int *const col) {
  const char fooName[] = "clampPlayerLocation";

  // Argument Validity Checks
  errno = 0;
  if (row == (int *)NULL) {
    fprintf(stderr, "argument 'row' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
  }
  if (col == (int *)NULL) {
    fprintf(stderr, "argument 'col' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
  }

  // Clamp row to player's vertical boundaries
  if (*col < leftBound) {
    *col = leftBound;
  } else if (*col > rightBound) {
    *col = rightBound;
  }

  // Clamp col to player's horizontal boundaries
  if (*row < upperBound) {
    *row = upperBound;
  } else if (*row > lowerBound) {
    *row = lowerBound;
  }
}

// ======================== Public Player Functions =========================
int initPlayer(Player *const p, const int startRow, const int startCol,
               const int lives) {
  const char fooName[] = "initPlayer";

  // Argument Validity Check
  errno = 0;
  if (p == (Player *)NULL) {
    fprintf(stderr, "argument 'p' of %s must point to the current player\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Initialize player
  p->lives = lives;
  p->score = 0;
  p->startRow = startRow;
  p->startCol = startCol;
  p->shotBullets = list_constructEmpty(sizeof(Task *));
  p->shotBullets.elementDeleter = deleteBulletTask;

  errno = resetPlayer(p);
  if (errno != 0) {
    fprintf(stderr, "Unable to reset player in %s\n", fooName);
    return errno;
  }

  errno = pthread_mutex_init(&p->locationMutex, NULL);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to initialize player's location mutex in %s\n",
            fooName);
    return errno;
  }
  errno = pthread_mutex_init(&p->stateMutex, NULL);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to initialize player's state mutex in %s\n",
            fooName);
    return errno;
  }
  errno = pthread_mutex_init(&p->bulletMutex, NULL);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to initialize player's bullet mutex in %s\n",
            fooName);
  }
  return errno;
}

int destroyPlayer(Player *const p) {
  const char fooName[] = "destroyPlayer";
  errno = 0;

  // Return if there's nothing to destroy
  if (p == (Player *)NULL) {
    return errno;
  }

  // Destroy Player
  errno = pthread_mutex_destroy(&p->locationMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to destroy player's bullet mutex in %s\n",
            fooName);
  }
  errno = pthread_mutex_destroy(&p->stateMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to destroy player's bullet mutex in %s\n",
            fooName);
  }
  errno = pthread_mutex_destroy(&p->bulletMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to destroy player's bullet mutex in %s\n",
            fooName);
  }
  list_freeNodes(&p->shotBullets);
  return errno;
}

int movePlayer(Player *const p, int row, int col, const bool lock) {
  const char fooName[] = "movePlayer";

  // Argument Validity Check
  errno = 0;
  if (p == (Player *)NULL) {
    fprintf(stderr, "argument 'p' of %s must point to the current player\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Move Player
  // restrict location
  clampLocation(&row, &col);

  // draw player and update location
  if (lock) {
    errno = pthread_mutex_lock(&p->locationMutex);
    if (errno != 0) {
      fprintf(stderr,
              "Unable to lock player's location mutex before moving in %s\n",
              fooName);
      return errno;
    }
  }

  errno = redrawPlayer(p, row, col);
  if (errno != 0) {
    fprintf(stderr, "Encountered error drawing player to game console in %s\n",
            fooName);
  }
  p->row = row;
  p->col = col;

  if (lock) {
    errno = pthread_mutex_unlock(&p->locationMutex);
    if (errno != 0) {
      fprintf(stderr,
              "Unable to unlock player's location mutex after moving in %s\n",
              fooName);
    }
  }

  return errno;
}

// ====================== Player Thread Start Function ========================
void *runPlayer(void *data) {
  const char fooName[] = "runPlayer";
  RunPlayerArg *const arg = (RunPlayerArg *)data;
  Player *const player = arg->p;

  // Argument Validity Checks
  errno = 0;
  if (data == (void *)NULL) {
    fprintf(stderr,
            "argument 'data' of %s must point to a valid RunPlayerArg\n",
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

  // Move Player Around Screen
  else {
    while (!arg->sleepGame->completed && player->lives >= 0) {
      switch (player->state) {
      case DEAD:
        player->lives--;
        if (player->lives == 0) {
          // quit game
        }
        break;
      default:;
      }

      ++player->animTile;
      player->animTile %= PLAYER_ANIM_TILES;
      errno = redrawPlayer(player, player->row, player->col);
      if (errno != 0) {
        fprintf(stderr, "Error trying to draw player to console buffer in %s\n",
                fooName);
        break;
      }

      sleepTicks(PLAYER_ANIM_TICKS);
    }
  }

  if (errno != 0) {
    const int err = task_markCompleted(arg->sleepGame);
    if (err != 0) {
      fprintf(stderr, "Error while trying to mark game as completed in %s\n",
              fooName);
    }
  }

  return (void *)(size_t)errno;
}
