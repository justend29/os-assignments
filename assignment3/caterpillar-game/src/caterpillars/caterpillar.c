/**
 * @file caterpillar.c
 * @author Justen Di Ruscio
 * @brief Contains definitions of the Caterpillar functions
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <caterpillar/caterpillars/caterpillar.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <caterpillar/bullets/bullet.h>
#include <caterpillar/bullets/bullet_collisions.h>
#include <caterpillar/bullets/collision_reactions.h>
#include <caterpillar/bullets/registry.h>

// ========================== Global Constants ============================
#define CATERPILLAR_ANIM_TICKS 50

char *segmentGraphic[CATERPILLAR_ANIM_TILES][CATERPILLAR_HEIGHT] = {
    {"=", ";"}, {"=", ":"}, {"=", ":"}, {"0", " "}};

// ======================= Support Functions ========================
static int moveCaterpillarAndDraw(Caterpillar *const c) {
  const char fooName[] = "moveCaterpillarAndDraw";

  // Argument Validity Check
  if (c == (Caterpillar *)NULL) {
    fprintf(stderr, "Argument 'c' of %s must point to a Caterpillar\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Move Caterpillar by one unit in direction
  errno = pthread_mutex_lock(&consoleMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to lock console mutex before drawing "
            "Caterpillar in %s\n",
            fooName);
    return errno;
  }

  errno = pthread_mutex_lock(&c->locationMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to lock Caterpillar's location mutex before "
            "drawing Caterpillar in %s\n",
            fooName);
    return errno;
  }

  // move each segment of caterpillar to move caterpillar
  for (int segIdx = 0; segIdx < c->numSegments; ++segIdx) {
    CaterpillarSegment *seg = c->segments + segIdx;
    int newRow = seg->row;
    int newCol = seg->col + seg->direction;
    CaterpillarDirection newDir = seg->direction;

    // move segment down and flip direction if it hits horizontal border
    const bool hitLeft = (newCol < SCR_LEFT) && seg->direction == dir_Left;
    const bool hitRight = (newCol >= GAME_COLS) && seg->direction == dir_Right;
    if (hitLeft || hitRight) {
      newCol = seg->col;
      newRow += CATERPILLAR_HEIGHT * DOWN;
      newDir = -newDir; // flip direction
    }

    // move segments up if head's going into player/caterpillar boundary row
    if (segIdx == 0 && newRow == BOUNDARY_ROW) {
      newRow += CATERPILLAR_HEIGHT * UP;
      for (int i = 1; i < c->numSegments; ++i) {
        clearConsoleSegment(c->segments[i].row, c->segments[i].col);
        c->segments[i].row += CATERPILLAR_HEIGHT * UP;
      }
    }

    // clear current segment location before drawing new location
    else {
      clearConsoleSegment(seg->row, seg->col);
    }

    // move segment to new location
    char **segImg = segmentGraphic[seg->animTile];
    consoleDrawImage(newRow, newCol, segImg, CATERPILLAR_HEIGHT);
    seg->row = newRow;
    seg->col = newCol;
    seg->direction = newDir;
  }

  errno = pthread_mutex_unlock(&consoleMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to unlock console mutex after drawing "
            "Caterpillar in %s\n",
            fooName);
  }

  errno = pthread_mutex_unlock(&c->locationMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while trying to unlock Caterpillar's location mutex before "
            "drawing Caterpillar in %s\n",
            fooName);
    return errno;
  }

  return errno;
}

// ======================= Public Caterpillar Functions ========================
void clearConsoleSegment(const int row, const int col) {
  consoleClearImage(row, col, CATERPILLAR_HEIGHT, SEGMENT_WIDTH);
}

int initCaterpillar(Caterpillar *const c, const CaterpillarDirection dir,
                    const int headRow, const int headCol, const int numSegments,
                    const int speedTicks) {
  const char fooName[] = "initCaterpillar";

  // Argument Validity Check
  errno = 0;
  if (c == (Caterpillar *)NULL) {
    fprintf(stderr, "Argument 'c' of %s must point to a Caterpillar\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Initialize Caterpillar and its segments
  errno = pthread_mutex_init(&c->locationMutex, NULL);
  if (errno != 0) {
    fprintf(stderr, "Unable to initialize caterpillar's location mutex in %s\n",
            fooName);
    return errno;
  }

  errno = pthread_mutex_init(&c->bulletMutex, NULL);
  if (errno != 0) {
    fprintf(stderr, "Unable to initialize caterpillar's bullet mutex in %s\n",
            fooName);
    return errno;
  }

  c->numSegments = numSegments;
  c->speedTicks = speedTicks;

  pthread_mutex_lock(&c->bulletMutex);
  c->shotBullets = list_constructEmpty(sizeof(Task *));
  c->shotBullets.elementDeleter = deleteBulletTask;
  pthread_mutex_unlock(&c->bulletMutex);

  pthread_mutex_lock(&c->locationMutex);
  for (int segIdx = 0; segIdx < numSegments; ++segIdx) {
    CaterpillarSegment seg;
    seg.row = headRow;
    seg.col = headCol + segIdx * -dir; // backwards from head col
    seg.direction = dir;
    if (segIdx == 0) { // head segment
      seg.animTile = CATERPILLAR_ANIM_TILES - 1;
    } else { // body segment
      seg.animTile = segIdx % (CATERPILLAR_ANIM_TILES - 1);
    }
    c->segments[segIdx] = seg;
  }
  pthread_mutex_unlock(&c->locationMutex);
  return 0;
}

int destroyCaterpillar(Caterpillar *const c) {
  const char fooName[] = "destroyCaterpillar";
  errno = pthread_mutex_destroy(&c->locationMutex);
  if (errno != 0) {
    fprintf(stderr, "Unable to initialize caterpillar's location mutex in %s\n",
            fooName);
    return errno;
  }
  errno = pthread_mutex_destroy(&c->bulletMutex);
  if (errno != 0) {
    fprintf(stderr, "Unable to initialize caterpillar's bullet mutex in %s\n",
            fooName);
    return errno;
  }
  list_freeNodes(&c->shotBullets);
  return errno;
}

void destroyCaterpillarTask(void *task) {
  const char fooName[] = "destroyCaterpillarTask";

  // Return if there's nothing to destroy
  if (task == NULL) {
    return;
  }
  Task *const t = (Task *)task;
  if (t->fooArg == NULL) {
    return;
  }

  // Get Caterpillar from Task
  Caterpillar *const taskCaterpillar =
      ((RunCaterpillarArg *)t->fooArg)->caterpillar;

  // Destroy the Caterpillar itself
  errno = destroyCaterpillar(taskCaterpillar);
  if (errno != 0) {
    fprintf(stderr, "Failure destroying one of the Caterpillars in %s\n",
            fooName);
  }

  task_destroy(task);
  free(task);
}

// ==================== Caterpillar Thread Start Functions =====================
void *runCaterpillar(void *data) {
  const char fooName[] = "runCaterpillar";
  const RunCaterpillarArg *const arg = (RunCaterpillarArg *)data;

  // Argument Validity Checks
  errno = 0;
  if (data == (void *)NULL) {
    fprintf(stderr, "argument 'data' of %s must point to a valid UpkeepArg\n",
            fooName);
    errno = EPERM;
  } else if (arg->caterpillar == (Caterpillar *)NULL) {
    fprintf(stderr,
            "member 'caterpillar' of argument 'data' of %s must point to a "
            "Caterpillar\n",
            fooName);
    errno = EPERM;
  } else if (arg->runCaterpillarTask == (Task *)NULL) {
    fprintf(stderr,
            "member 'runCaterpillarTask' of argument 'data' of %s must point "
            "to a Task\n",
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
  } else if (arg->sleepGame == (Task *)NULL) {
    fprintf(stderr,
            "member 'sleepGame' of argument 'data' of %s must point to the "
            "main Task used to sleep the game\n",
            fooName);
    errno = EPERM;
  }

  // Run Caterpillar
  else {
    int movesSinceShoot = 0;
    while (!arg->sleepGame->completed && !arg->runCaterpillarTask->completed) {
      const int movesBeforeShoot = 1 + rand() % (int)(GAME_COLS / 1.5);
      // Update segments' animation to animate caterpillar
      CaterpillarSegment *const segments = arg->caterpillar->segments;
      for (int catSeg = 1; catSeg < arg->caterpillar->numSegments; ++catSeg) {
        segments[catSeg].animTile =
            (segments[catSeg].animTile + 1) % (CATERPILLAR_ANIM_TILES - 1);
      }

      // Move Caterpillar
      moveCaterpillarAndDraw(arg->caterpillar);

      // At random intervals, shoot bullets
      if (++movesSinceShoot >= movesBeforeShoot) {
        const CaterpillarSegment *const head = arg->caterpillar->segments;
        shootBullet(head->row + DOWN, head->col, b_Caterpillar,
                    arg->caterpillar);
        movesSinceShoot = 0;
      }
      sleepTicks(arg->caterpillar->speedTicks);
    }
  }

  // Clear Caterpillar segments
  for (int segIdx = 0; segIdx < arg->caterpillar->numSegments; ++segIdx) {
    CaterpillarSegment *const seg = arg->caterpillar->segments + segIdx;
    clearConsoleSegment(seg->row, seg->col);
  }

  // Remove run caterpillar task from running list
  errno = pthread_mutex_lock(registry.runningCatLock);
  ListNode *node = registry.runningCaterpillars->head;
  while (node != NULL) {
    Task *t = *((Task **)(node->data));
    if (t == arg->runCaterpillarTask) {
      break;
    }
    node = node->next;
  }
  list_removeNode(registry.runningCaterpillars, node);
  errno = pthread_mutex_unlock(registry.runningCatLock);

  // Add run caterpillar task to killed list
  errno = pthread_mutex_lock(arg->killedCatLock);
  if (errno != 0) {
    fprintf(stderr, "Unable to lock list of killed caterpillars in %s\n",
            fooName);
  } else if (node != NULL) {
    const bool pushed = list_pushNodeBack(arg->killedCaterpillars, node);
    int err = 0;
    if (!pushed) {
      fprintf(
          stderr,
          "Failed adding caterpillar task to killed caterpillars list in  %s\n",
          fooName);
      err = errno;
    }

    errno = pthread_mutex_unlock(arg->killedCatLock);
    if (errno != 0) {
      fprintf(stderr, "Unable to unlock list of killed caterpillars in %s\n",
              fooName);
    }
    errno = err;
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
