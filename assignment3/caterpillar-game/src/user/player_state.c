/**
 * @file player_state.c
 * @author Justen Di Ruscio
 * @brief Definitions related to updating player state
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <caterpillar/user/player_state.h>

#include <errno.h>
#include <stdio.h>

int updatePlayerLives(Player *const p, const int offset, int *const lives) {
  const char fooName[] = "updatePlayerLives";

  // Argument Validity Check
  if (p == (Player *)NULL) {
    fprintf(
        stderr,
        "argument 'p' of updatePlayerLives must point to the current player\n");
    errno = EPERM;
    return errno;
  }

  // Update Player's lives
  errno = pthread_mutex_lock(&p->stateMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure locking Player's state mutex in %s\n", fooName);
    return errno;
  }

  p->lives += offset;
  *lives = p->lives;

  errno = pthread_mutex_unlock(&p->stateMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure unlocking Player's state mutex in %s\n", fooName);
  }
  return errno;
}

int updatePlayerScore(Player *const p, const int offset) {
  const char fooName[] = "updatePlayerLives";

  // Argument Validity Check
  if (p == (Player *)NULL) {
    fprintf(
        stderr,
        "argument 'p' of updatePlayerLives must point to the current player\n");
    errno = EPERM;
    return errno;
  }

  // Update Player's lives
  errno = pthread_mutex_lock(&p->stateMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure locking Player's state mutex in %s\n", fooName);
    return errno;
  }

  p->score += offset;

  errno = pthread_mutex_unlock(&p->stateMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure unlocking Player's state mutex in %s\n", fooName);
  }
  return errno;
}
