/**
 * @file game_console.c
 * @author Justen Di Ruscio
 * @brief Definitions that wrap distribute/console
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#define _GNU_SOURCE

#include <caterpillar/game/game_console.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>

#include "../distribute/console.h"
#include <caterpillar/game/constants.h>

#define REFRESH_SLEEP_TICKS 4

pthread_mutex_t consoleMutex;

// ======================= Public Console Functions ========================
bool gameConsoleInit() {
  const char fooName[] = "gameConsoleInit";

  const bool success = consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD);
  if (!success) {
    return false;
  }
  errno = pthread_mutex_init(&consoleMutex, NULL);
  if (errno != 0) {
    fprintf(stderr, "Error while trying to initialize console mutex in %s\n",
            fooName);
    return false; // errno set by pthread_mutex_init
  }

  // Initialize recursive mutex to lock console
  pthread_mutexattr_t mutexAttrs;
  errno = pthread_mutexattr_init(&mutexAttrs);
  if (errno != 0) {
    fprintf(stderr, "Failed to initialize mutex attributes for console in %s\n",
            fooName);
    return false;
  }
  pthread_mutexattr_settype(&mutexAttrs, PTHREAD_MUTEX_RECURSIVE);
  if (errno != 0) {
    fprintf(stderr, "Failed to set mutex attributes for console in %s\n",
            fooName);
    return false;
  }
  errno = pthread_mutex_init(&consoleMutex, &mutexAttrs);
  if (errno != 0) {
    fprintf(stderr, "Failed to initialize mutex for console in %s\n", fooName);
    return false;
  }
  errno = pthread_mutexattr_destroy(&mutexAttrs);
  if (errno != 0) {
    fprintf(stderr, "Failed to destroy mutex attributes in %s\n", fooName);
    return false;
  }
  return true;
}

int gameConsoleFinish() {
  const char fooName[] = "gameConsoleFinish";
  const int err = pthread_mutex_destroy(&consoleMutex);
  putBanner("Done");
  consoleFinish();
  if (err != 0) {
    errno = err;
    fprintf(stderr, "Error while destroying console mutex in %s\n", fooName);
  }
  return errno;
}

// ================== Screen Refresh Thread Start Function ====================
void *runScreenRefresher(void *data) {
  const char fooName[] = "runScreenRefresher";
  const RefreshArg *const arg = (RefreshArg *)data;

  // Argument Validity Checks
  errno = 0;
  if (data == (void *)NULL) {
    fprintf(stderr, "argument 'data' of %s must point to a valid RefreshArg\n",
            fooName);
    errno = EPERM;
  } else if (arg->sleepGame == (Task *)NULL) {
    fprintf(stderr,
            "member 'sleepGame' of argument 'data' of %s must point to the "
            "main Task used to sleep the game\n",
            fooName);
    errno = EPERM;
  }

  // Draw screen buffer to screen
  else {
    while (!arg->sleepGame->completed) {
      // hold lock to prevent tearing on screen
      errno = pthread_mutex_lock(&consoleMutex);
      if (errno != 0) {
        fprintf(
            stderr,
            "Error encountered while trying to obtain lock to refresh screen "
            "in %s\n",
            fooName);
        break;
      }

      consoleRefresh();

      errno = pthread_mutex_unlock(&consoleMutex);
      if (errno != 0) {
        fprintf(
            stderr,
            "Error encountered while trying to release lock after refreshing "
            "screen in %s\n",
            fooName);
        break;
      }
      sleepTicks(REFRESH_SLEEP_TICKS);
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
