/**
 * @file bullet.c
 * @author Justen Di Ruscio
 * @brief Implementations for bullets and bullet shooting
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <caterpillar/bullets/bullet.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>

#include "bullet_list.h"
#include <caterpillar/bullets/bullet_collisions.h>
#include <caterpillar/bullets/collision_reactions.h>
#include <caterpillar/bullets/registry.h>
#include <caterpillar/game/constants.h>
#include <caterpillar/game/game_console.h>
#include <caterpillar/user/player_state.h>

// contains all the data necessary to shoot a bullet
// Task must be first
typedef struct ShootBulletData {
  Task runShootTask;
  Bullet b;
  RunShootBulletArg runShootArg;
  void *runShootResult;
} ShootBulletData;

// Characters displayed
#define BULLET_ANIM_TILES 2
char *bulletGraphic[BULLET_ANIM_TILES][BULLET_HEIGHT] = {{"^"}, {"*"}};

// ======================= Support Functions ========================
/**
 * @brief clears the provided location on the console to erase a bullet
 *
 * @param row
 * @param col
 * @return int errno
 */
static int clearConsoleBullet(const int row, const int col) {
  const char fooName[] = "clearConsoleBullet";

  errno = pthread_mutex_lock(&consoleMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to obtain lock before drawing bullet to screen "
            "in %s\n",
            fooName);
    return errno;
  }

  consoleClearImage(row, col, BULLET_HEIGHT, BULLET_WIDTH);

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
 * @brief clears the current location of the provided bullet and redraws is at
 * the provided location. Sets errno on error
 *
 * @param b bullet
 * @param row
 * @param col
 * @return int errno
 */
static int redrawBullet(const Bullet *const b, const int row, const int col) {
  const char fooName[] = "redrawBullet";

  // Argument Validity Check
  errno = 0;
  if (b == (Bullet *)NULL) {
    fprintf(stderr, "argument 'b' of %s must point to a valid Bullet\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Draw moved player on game board
  errno = pthread_mutex_lock(&consoleMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to obtain lock before drawing bullet to screen "
            "in %s\n",
            fooName);
    return errno;
  }

  char **bulletImg = bulletGraphic[(int)b->bulletType];
  consoleClearImage(b->row, b->col, BULLET_HEIGHT, BULLET_WIDTH);
  consoleDrawImage(row, col, bulletImg, BULLET_HEIGHT);

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
 * @brief Moves the provided bullet to the next segment
 *
 * @param bullet
 * @return int errno
 */
int moveBullet(Bullet *const bullet) {
  const char fooName[] = "moveBullet";

  // Argument Validity Check
  errno = 0;
  if (bullet == (Bullet *)NULL) {
    fprintf(stderr, "argument 'bullet' of %s must point to a valid bullet\n",
            fooName);
  }

  // Move Bullet
  errno = pthread_mutex_lock(&bullet->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure locking bullet location mutex in %s\n", fooName);
    return errno;
  }

  const int newRow = bullet->row + bullet->bulletType * 2 * DOWN + UP;
  redrawBullet(bullet, newRow, bullet->col);
  bullet->row = newRow;

  errno = pthread_mutex_unlock(&bullet->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure unlocking bullet location mutex in %s\n", fooName);
  }
  return errno;
}

// ================= Shoot Bullet Thread Start Function ==================
static void *runShootBullet(void *data) {
  const char fooName[] = "runShootBullet";
  const RunShootBulletArg *const arg = (RunShootBulletArg *)data;

  // Argument Validity Checks
  errno = 0;
  if (data == (void *)NULL) {
    fprintf(stderr, "argument 'data' of %s must point to a valid UpkeepArg\n",
            fooName);
    errno = EPERM;
  } else if (arg->bullet == (Bullet *)NULL) {
    fprintf(stderr,
            "member 'bullet' of argument 'data' of %s must point to a bullet\n",
            fooName);
    errno = EPERM;
  } else if (arg->shootNode == (ListNode *)NULL) {
    fprintf(stderr,
            "member 'shootNode' of argument 'data' of %s must point to a "
            "ListNode\n",
            fooName);
    errno = EPERM;
  } else if (arg->shooter == (void *)NULL) {
    fprintf(stderr,
            "member 'shooter' of argument 'data' of %s must point to a "
            "Player or Caterpillar\n",
            fooName);
    errno = EPERM;
  }

  // Run Shoot Bullet
  else {
    Task *bulletTask = (*(Task **)arg->shootNode->data);
    while (!registry.sleepGame->completed && !bulletTask->completed) {
      // Delete bullet if it's off the play area
      const bool playerBulletEnd = arg->bullet->bulletType == b_Player &&
                                   arg->bullet->row == CATERPILLAR_TOP_ROW;
      const bool catBulletEnd = arg->bullet->bulletType == b_Caterpillar &&
                                arg->bullet->row == SCR_TOP + GAME_ROWS;
      if (playerBulletEnd || catBulletEnd) {
        errno = deleteBullet(arg->shooter, arg->shootNode, arg->bullet);
        if (errno != 0) {
          fprintf(stderr,
                  "Failure deleting bullet in %s after it ran off play area\n",
                  fooName);
        }
        break;
      }

      // Move Bullet
      errno = moveBullet(arg->bullet);
      if (errno != 0) {
        fprintf(stderr, "Failure moving bullet in %s %i\n", fooName, errno);
        break;
      }

      // Take action when bullet hits player/caterpillar
      if (arg->bullet->bulletType == b_Caterpillar) {
        const bool hit = hitPlayerLocked(arg->bullet, registry.player);
        if (hit) {
          errno = hitPlayerReaction();
          if (errno != 0) {
            fprintf(stderr, "Error while reacting to player hit in %s\n",
                    fooName);
          }
          break;
        }
      } else {
        ListNode *hitCaterpillarNode = NULL;
        const bool hit = hitCaterpillars(
            arg->bullet, registry.runningCaterpillars, &hitCaterpillarNode);
        if (hit) {
          errno = hitCaterpillarReaction(hitCaterpillarNode, arg->shootNode);
          if (errno != 0) {
            fprintf(stderr,
                    "Error while reacting to player caterpillar hit in %s\n",
                    fooName);
          }
          errno = updatePlayerScore(registry.player, +1);
          break;
        }
      }

      sleepTicks(BULLET_SHOOT_RATE_TICKS);
    }
  }

  clearConsoleBullet(arg->bullet->row, arg->bullet->col);

  // Stop game if error occurred
  if (errno != 0) {
    const int err = task_markCompleted(registry.sleepGame);
    if (err != 0) {
      fprintf(stderr, "Error while trying to mark game as completed in %s\n",
              fooName);
    }
  }
  return (void *)(size_t)errno;
}

// ================= Public Bullet Functions ==================
int initBullet(Bullet *const b, const int row, const int col,
               const BulletType bType) {
  const char fooName[] = "initBullet";

  // Argument Validity Check
  errno = 0;
  if (b == (Bullet *)NULL) {
    fprintf(stderr, "argument 'b' of %s must point to a valid bullet\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Initialize Bullet
  errno = pthread_mutex_init(&b->locationMutex, NULL);
  if (errno != 0) {
    fprintf(stderr, "Failure initializing bullet's location mutex in %s\n",
            fooName);
  }

  pthread_mutex_lock(&b->locationMutex);
  b->row = row;
  b->col = col;
  pthread_mutex_unlock(&b->locationMutex);
  b->bulletType = bType;
  return errno;
}

int destroyBullet(Bullet *const b) {
  const char fooName[] = "destroyBullet";
  errno = 0;
  // Return if there's nothing to destroy
  if (b == (Bullet *)NULL) {
    return errno;
  }

  errno = pthread_mutex_destroy(&b->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure destroying bullet's location mutex in %s\n",
            fooName);
  }
  return errno;
}

int shootBullet(const int row, const int col, const BulletType bType,
                void *const shooter) {
  const char fooName[] = "shootBullet";

  // Don't shoot bullets until all game instances are registered
  if (registry.player == NULL || registry.runningCaterpillars == NULL ||
      registry.tp == NULL || registry.sleepGame == NULL ||
      registry.runningCatLock == NULL) {
    return 0;
  }

  // Allocate memory to shoot new bullet
  void *bulletBytes = malloc(sizeof(ShootBulletData));
  if (bulletBytes == (void *)NULL) {
    fprintf(stderr, "Failure allocating memory to shoot bullet in %s\n",
            fooName);
    free(bulletBytes);
    return errno;
  }
  ShootBulletData *shootData = (ShootBulletData *)bulletBytes;

  // Locate bullet variables in allocated memory
  Task *const shootTask = &shootData->runShootTask;
  Bullet *const bullet = &shootData->b;
  RunShootBulletArg *const runShootArg = &shootData->runShootArg;
  void **const runResult = &shootData->runShootResult;

  ListNode *const shootNode = list_newNode(sizeof(Task *), &shootTask);
  if (shootNode == (ListNode *)NULL) {
    fprintf(stderr,
            "Unable to allocate memory for new node to contain bullets' task "
            "in %s\n",
            fooName);
    free(bulletBytes);
    return errno;
  }

  // Initialize bullet variables
  errno = initBullet(bullet, row, col, bType);
  if (errno != 0) {
    fprintf(stderr,
            "Failure trying to initialize a new bullet to shoot in %s\n",
            fooName);
    free(bulletBytes);
    free(shootNode);
    return errno;
  }
  runShootArg->shootNode = shootNode;
  runShootArg->bullet = bullet;
  runShootArg->shooter = shooter;
  errno = task_init(shootTask, runShootBullet, runShootArg, runResult);
  if (errno != 0) {
    fprintf(stderr,
            "Failure trying to initialize task to run spawned caterpillar "
            "in %s\n",
            fooName);
    free(bulletBytes);
    free(shootNode);
    return errno;
  }

  // Add shoot bullet task to shooter's list of bullets
  errno = addBulletToShooterList(shooter, shootNode, bType);
  if (errno != 0) {
    fprintf(stderr,
            "Failed to store shoot bullet task in shooter's list in %s\n",
            fooName);
    free(bulletBytes);
    free(shootNode);
  }

  // Shoot bullet on separate thread
  errno = tp_enqueueImmediate(registry.tp, shootTask);
  if (errno != 0) {
    fprintf(stderr, "Failed to run shot bullet on thread pool in %s\n",
            fooName);
    free(bulletBytes);
    free(shootNode);
    return errno;
  }
  return errno;
}

void deleteBulletTask(void *bulletTask) {
  const char fooName[] = "deleteBulletTask";
  if (bulletTask == NULL) {
    return;
  }

  Task *bt = (Task *)bulletTask;
  Bullet *bullet = ((RunShootBulletArg *)bt->fooArg)->bullet;

  // destroy the actual bullet
  errno = destroyBullet(bullet);
  if (errno != 0) {
    fprintf(stderr, "Failure to destroy a bullet in %s\n", fooName);
  }

  // destroy the bullet's task
  task_destroy(bulletTask);

  // free the bullet's task
  free(bulletTask);
}

int deleteBullet(void *const shooter, ListNode *const shootNode,
                 Bullet *const bullet) {
  const char fooName[] = "deleteBullet";

  // Argument Validity Checks
  if (shooter == (void *)NULL) {
    fprintf(stderr, "argument 'shooter' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }
  if (shootNode == (ListNode *)NULL) {
    fprintf(stderr,
            "argument 'shootNode' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }
  if (bullet == (Bullet *)NULL) {
    fprintf(stderr, "argument 'bullet' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // clear bullet from screen
  errno = clearConsoleBullet(bullet->row, bullet->col);
  if (errno != 0) {
    fprintf(stderr, "Failure clearing shooter's bullet in %s\n", fooName);
    return errno;
  }

  // Remove Bullet from shooter
  errno =
      eraseBulletFromShooterList(shooter, shootNode, bullet->bulletType, true);
  if (errno != 0) {
    fprintf(stderr,
            "Failure removing bullet from shooter's bullet list in %s\n",
            fooName);
    return errno;
  }

  return errno;
}
