/**
 * @file collision_reactions.c
 * @author Justen Di Ruscio
 * @brief Contains definitions for functions reacting to player/caterpillar
 * getting hit
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <caterpillar/bullets/collision_reactions.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>

#include "bullet_list.h"
#include <caterpillar/bullets/bullet.h>
#include <caterpillar/bullets/bullet_collisions.h>
#include <caterpillar/bullets/registry.h>
#include <caterpillar/caterpillars/caterpillar.h>
#include <caterpillar/caterpillars/spawn_caterpillars.h>
#include <caterpillar/game/constants.h>
#include <caterpillar/user/player_state.h>

#define PlAYER_HIT_PAUSE_TICKS 100

int hitPlayerReaction() {
  const char fooName[] = "hitPlayerReaction";
  errno = 0;

  // Reduce Player's lives
  int playerLives = PLAYER_START_LIVES;
  errno = updatePlayerLives(registry.player, HIT_PLAYER_LIVES, &playerLives);
  if (errno != 0) {
    fprintf(stderr, "Failure updating player's lives in %s\n", fooName);
    return errno;
  }

  // Erase all bullets on screen
  errno = eraseAllBullets();
  if (errno != 0) {
    fprintf(stderr, "Failure erasing all bullets on game board in %s\n",
            fooName);
  }

  // Respawn player
  movePlayer(registry.player, registry.player->startRow,
             registry.player->startCol, false);

  // Delay game or end game
  // end game if player's lives are 0
  if (playerLives == 0) { // lose
    task_markCompleted(registry.sleepGame);
  } else {
    sleepTicks(PlAYER_HIT_PAUSE_TICKS);
  }

  // Unlock all locked mutexes from hit
  errno = pthread_mutex_unlock(&registry.player->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure to unlock player's location mutex in %s\n",
            fooName);
    return false;
  }
  errno = pthread_mutex_unlock(&registry.player->bulletMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure to unlock player's location mutex in %s\n",
            fooName);
    return false;
  }
  return errno;
}

int hitCaterpillarReaction(ListNode *hitCaterpillarNode,
                           ListNode *hitBulletNode) {
  const char fooName[] = "hitCaterpillarReaction";
  errno = 0;

  // Set variables
  Task *const caterpillarTask = *((Task **)hitCaterpillarNode->data);
  Caterpillar *hitCaterpillar =
      ((RunCaterpillarArg *)caterpillarTask->fooArg)->caterpillar;
  Task *const bulletTask = *((Task **)hitBulletNode->data);
  Bullet *bullet = ((RunShootBulletArg *)bulletTask->fooArg)->bullet;

  // Update the player's score
  errno = updatePlayerScore(registry.player, HIT_CATERPILLAR_SCORE);
  if (errno != 0) {
    fprintf(stderr, "Failure updating player's score in %s\n", fooName);
    return errno;
  }

  // Resolve new lengths
  int newLength = 0;
  for (int segIdx = 0; segIdx < hitCaterpillar->numSegments; ++segIdx) {
    const CaterpillarSegment *const seg = hitCaterpillar->segments + segIdx;
    if (hitCaterpillarSegment(bullet, seg)) {
      newLength = segIdx;
      break;
    }
  }

  // Spawn new caterpillar
  Task *spawnTask = NULL;
  if (hitCaterpillar->numSegments - newLength >= CATERPILLAR_MIN_LENGTH) {
    spawnTask = spawnCaterpillar(hitCaterpillar->segments + newLength,
                                 hitCaterpillar->numSegments - newLength);
  }

  // Shrink Caterpillar to new length or stop if it's length is too small
  // shrink
  if (newLength >= CATERPILLAR_MIN_LENGTH) {
    // clear truncated segments
    for (int i = newLength; i < hitCaterpillar->numSegments; ++i) {
      CaterpillarSegment *const seg = hitCaterpillar->segments + i;
      clearConsoleSegment(seg->row, seg->col);
    }
    // reduce size
    hitCaterpillar->numSegments = newLength;

    // increase speed of hit caterpillar
    int newSpeed =
        hitCaterpillar->speedTicks / CATERPILLAR_HIT_SPEED_MULTIPLIER;
    if (newSpeed < CATERPILLAR_MIN_SPEED) {
      newSpeed = CATERPILLAR_MIN_SPEED;
    }
    hitCaterpillar->speedTicks = newSpeed;

  }

  // Stop
  else {
    // Stop running caterpillar
    // location already locked
    pthread_mutex_lock(&hitCaterpillar->bulletMutex);
    errno = task_markCompleted(caterpillarTask);
    if (errno != 0) {
      fprintf(stderr,
              "Failure marking caterpillar task completed to erase caterpillar "
              "in %s\n",
              fooName);
    }
    pthread_mutex_unlock(&hitCaterpillar->bulletMutex);
  }

  // Unlock all mutexes
  errno = pthread_mutex_unlock(&hitCaterpillar->locationMutex);
  if (errno != 0) {
    return errno;
  }

  // unlock bullet location to delete bullet
  errno = pthread_mutex_unlock(&bullet->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure to unlock bullet's location mutex in %s\n",
            fooName);
    return errno;
  }

  // Delete player bullet that hit caterpillar
  errno = deleteBullet(registry.player, hitBulletNode, bullet);
  if (errno != 0) {
    fprintf(stderr, "Failure deleting player's bullet in %s\n", fooName);
    return errno;
  }

  // run new caterpillar on game board
  if (spawnTask != NULL) {
    errno = tp_enqueueImmediate(registry.tp, spawnTask);
    if (errno != 0) {
      fprintf(stderr,
              "Failure in %s trying to run spawned caterpillar in threadpool\n",
              fooName);
    }
  }

  return errno;
}
