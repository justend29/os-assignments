/**
 * @file upkeep.c
 * @author Justen Di Ruscio
 * @brief Upkeep game definitions
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <caterpillar/upkeep.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include <caterpillar/bullets/registry.h>
#include <caterpillar/game/constants.h>
#include <caterpillar/game/game_console.h>

#define UPKEEP_TICKS 100

// ========================== Support Functions ============================
/**
 * @brief Writes the provided score to the score board. sets errno upon error
 *
 * @param score score to write to game board
 * @return true successfully wrote score to board
 * @return false error occurred; errno set too
 */
static bool writeScoreToBoard(const int score) {
  const char fooName[] = "writeScoreToBoard";

  char scoreStr[SCORE_MAX_LENGTH];
  const int num = snprintf(scoreStr, SCORE_MAX_LENGTH, "%i", score);
  if (num < 0) {
    fprintf(stderr, "Failure converting player's score to string in %s\n",
            fooName);
    errno = EIO;
    return false;
  }
  char *img = &scoreStr[0];
  consoleClearImage(SCORE_ROWS, SCORE_COLS, 1, SCORE_MAX_LENGTH);
  consoleDrawImage(SCORE_ROWS, SCORE_COLS, &img, 1);
  return true;
}

/**
 * @brief Writes the provided lives to the score board. sets errno upon error
 *
 * @param lives lives to write to game board
 * @return true successfully wrote lives to board
 * @return false error occurred; errno set too
 */
static bool writeLivesToBoard(const int lives) {
  const char fooName[] = "writeLivesToBoard";

  char livesStr[LIFE_MAX_LENGTH];
  const int num = snprintf(livesStr, LIFE_MAX_LENGTH, "%i", lives);
  if (num < 0) {
    fprintf(stderr, "Failure converting player's lives to string in %s\n",
            fooName);
    errno = EIO;
    return false;
  }
  char *img = &livesStr[0];
  consoleClearImage(LIFE_ROWS, LIFE_COLS, 1, LIFE_MAX_LENGTH);
  consoleDrawImage(LIFE_ROWS, LIFE_COLS, &img, 1);
  return true;
}

// ====================== Upkeep Thread Start Function ========================
void *runUpkeep(void *data) {
  const char fooName[] = "runUpkeep";
  const UpkeepArg *const arg = (UpkeepArg *)data;

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
  } else if (arg->killedCatLock == (pthread_mutex_t *)NULL) {
    fprintf(stderr,
            "member 'killedCatLock' of argument 'data' of %s must point to the "
            "lock for killed caterpillars\n",
            fooName);
    errno = EPERM;
  } else if (arg->killedCaterpillars == (List *)NULL) {
    fprintf(stderr,
            "member 'killedCaterpillars' of argument 'data' of %s must point "
            "to the list of killed caterpillars\n",
            fooName);
    errno = EPERM;
  } else if (arg->banner == (char **)NULL) {
    fprintf(stderr,
            "member 'banner' of argument 'data' of %s must point to a valid "
            "address\n",
            fooName);
    errno = EPERM;
  } else if (arg->sleepGame == (Task *)NULL) {
    fprintf(stderr,
            "member 'sleepGame' of argument 'data' of %s must point to the "
            "main Task used to sleep the game\n",
            fooName);
    errno = EPERM;
  }

  // Upkeep game
  else {
    while (!arg->sleepGame->completed) {
      // Draw images to curses screen buffer
      errno = pthread_mutex_lock(&consoleMutex);
      if (errno != 0) {
        fprintf(
            stderr,
            "Encountered error trying to obtain console lock to draw image to "
            "game board in %s\n",
            fooName);
        break;
      }

      bool success = writeScoreToBoard(arg->p->score);
      if (!success) {
        fprintf(stderr, "Failure writing player's score to game board in %s\n",
                fooName);
        break; // errno set by writeScoreToBoard
      }
      success = writeLivesToBoard(arg->p->lives);
      if (!success) {
        fprintf(stderr, "Failure writing player's lives to game board in %s\n",
                fooName);
        break; // errno set by writeLivesToBoard
      }

      errno = pthread_mutex_unlock(&consoleMutex);
      if (errno != 0) {
        fprintf(
            stderr,
            "Encountered error trying to release console lock after drawing "
            "image to game board in %s\n",
            fooName);
        break;
      }

      // Destroy all the killed caterpillars
      errno = pthread_mutex_lock(arg->killedCatLock);
      if (errno != 0) {
        fprintf(stderr, "Unable to lock list of killed caterpillars in %s\n",
                fooName);
        break;
      }

      list_freeNodes(arg->killedCaterpillars);

      errno = pthread_mutex_unlock(arg->killedCatLock);
      if (errno != 0) {
        fprintf(stderr, "Unable to unlock list of killed caterpillars in %s\n",
                fooName);
        break;
      }

      // Check if Player won
      errno = pthread_mutex_lock(registry.runningCatLock);
      if (errno != 0) {
        fprintf(stderr, "Unable to lock list of running caterpillars in %s\n",
                fooName);
        break;
      }

      if (registry.runningCaterpillars->length == 0) {
        errno = task_markCompleted(arg->sleepGame);
        if (errno != 0) {
          fprintf(stderr, "Failure completing game in %s\n", fooName);
          break;
        }
        *arg->banner = "DONE - WIN";
      }

      errno = pthread_mutex_unlock(registry.runningCatLock);
      if (errno != 0) {
        fprintf(stderr, "Unable to unlock list of running caterpillars in %s\n",
                fooName);
        break;
      }

      sleepTicks(UPKEEP_TICKS);
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
