/**
 * @file spawn_caterpillars.c
 * @author Justen Di Ruscio
 * @brief Definitions relating to spawning caterpillars
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <caterpillar/caterpillars/spawn_caterpillars.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <caterpillar/bullets/registry.h>
#include <caterpillar/caterpillars/caterpillar.h>
#include <caterpillar/game/constants.h>
#include <caterpillar/game/game_console.h>

// ================= Private Structures =====================
// data necessary to spawn a new caterpillar
typedef struct CaterpillarData {
  Task runTask;
  Caterpillar cat;
  RunCaterpillarArg runArg;
  void *runResult;
} CaterpillarData;

// ================= Spawn Caterpillar Support Functions =====================
#define CATERPILLAR_SPAWN_COL GAME_COLS     // column to spawn caterpillars
const int spawnNumSegments = GAME_COLS - 5; // length of spawned caterpillars
const CaterpillarDirection startDir = dir_Left; // spawned caterpillar dir
const int spawnRateTicks =
    CATERPILLAR_INIT_SPEED * 15; // longest num ticks between spawns

// ================= Spawn Caterpillar Support Functions =====================
void sleepTicksUntil(const int ticks, const Task *const sleepGame) {
  for (int sleep = CATERPILLAR_INIT_SPEED; sleep <= ticks;
       sleep += CATERPILLAR_INIT_SPEED) {
    if (sleepGame->completed) {
      return;
    }
    sleepTicks(sleep);
  }
}

// ================= Spawn Caterpillar Thread Start Function ==================
void *spawnCaterpillars(void *data) {
  const char fooName[] = "spawnCaterpillars";
  SpawnCaterpillarsArg *const arg = (SpawnCaterpillarsArg *)data;

  // Argument Validity Checks
  errno = 0;
  if (data == (void *)NULL) {
    fprintf(
        stderr,
        "argument 'data' of %s must point to a valid SpawnCaterpillarsArg\n",
        fooName);
    errno = EPERM;
  } else if (arg->runningCaterpillars == (List *)NULL) {
    fprintf(stderr,
            "member 'runningCaterpillars' of argument 'data' of %s must point "
            "to the list of caterpillars\n ",
            fooName);
    errno = EPERM;
  } else if (arg->sleepGame == (Task *)NULL) {
    fprintf(stderr,
            "member 'sleepGame' of argument 'data' of %s must point to the "
            "main Task used to sleep the game\n",
            fooName);
    errno = EPERM;
  } else if (arg->threadPool == (ThreadPool *)NULL) {
    fprintf(stderr,
            "member 'threadPool' of argument 'data' of %s must point to the "
            "game's threadpool\n",
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
  } else if (arg->runningCatLock == (pthread_mutex_t *)NULL) {
    fprintf(
        stderr,
        "member 'runningCatLock' of argument 'data' of %s must point a mutex\n",
        fooName);
    errno = EPERM;
  }

  // Spawn Caterpillars
  else {
    while (!arg->sleepGame->completed) {
      // allocate memory to spawn new caterpillar
      const void *const catData_ = malloc(sizeof(CaterpillarData));
      if (errno != 0) {
        fprintf(stderr,
                "Failure trying to allocate memory to spawn a new caterpillar "
                "in %s\n",
                fooName);
        free((void *)catData_);
        break;
      }
      CaterpillarData *caterpillarData = (CaterpillarData *)catData_;

      // locate caterpillar variables in allocated mem
      Task *const runTask = &caterpillarData->runTask;
      Caterpillar *const caterpillar = &caterpillarData->cat;
      RunCaterpillarArg *const runArg = &caterpillarData->runArg;
      void **const runResult = &caterpillarData->runResult;

      // initialize caterpillar variables
      errno = initCaterpillar(caterpillar, startDir, CATERPILLAR_TOP_ROW,
                              CATERPILLAR_SPAWN_COL, spawnNumSegments,
                              CATERPILLAR_INIT_SPEED);
      if (errno != 0) {
        fprintf(
            stderr,
            "Failure trying to initialize a new caterpillar to spawn in %s\n",
            fooName);
        free((void *)catData_);
        break;
      }
      const int spawnDuration =
          caterpillar->speedTicks * caterpillar->numSegments;
      const int timeBetweenSpawn = rand() % spawnRateTicks;
      runArg->caterpillar = caterpillar;
      runArg->sleepGame = arg->sleepGame;
      runArg->runCaterpillarTask = runTask;
      runArg->killedCatLock = arg->killedCatLock;
      runArg->killedCaterpillars = arg->killedCaterpillars;
      errno = task_init(runTask, runCaterpillar, runArg, runResult);
      if (errno != 0) {
        fprintf(stderr,
                "Failure trying to initialize task to run spawned caterpillar "
                "in %s\n",
                fooName);
        free((void *)catData_);
        break;
      }

      // add new caterpillar task to list of running caterpillars on game board
      errno = pthread_mutex_lock(arg->runningCatLock);
      const bool pushed = list_pushBack(arg->runningCaterpillars, &runTask);
      if (!pushed) {
        fprintf(stderr,
                "Failure in %s trying to add running caterpillar to list of "
                "caterpillars on game board\n",
                fooName);
        free((void *)catData_);
        break; // errno set by list_pushBack
      }
      pthread_mutex_unlock(arg->runningCatLock);

      // run new caterpillar on game board
      errno = tp_enqueueImmediate(arg->threadPool, runTask);
      if (errno != 0) {
        fprintf(
            stderr,
            "Failure in %s trying to run spawned caterpillar in thread pool\n",
            fooName);
        free((void *)catData_);
        break;
      }

      // Sleep for random interval before spawning new caterpillar
      sleepTicksUntil(spawnDuration + timeBetweenSpawn, arg->sleepGame);
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

Task *spawnCaterpillar(const CaterpillarSegment *segs, const int numSegs) {
  const char fooName[] = "spawnCaterpillar";

  // allocate memory to spawn new caterpillar
  const void *const catData_ = malloc(sizeof(CaterpillarData));
  if (errno != 0) {
    fprintf(stderr,
            "Failure trying to allocate memory to spawn a new caterpillar "
            "in %s\n",
            fooName);
    free((void *)catData_);
    return NULL;
  }
  CaterpillarData *caterpillarData = (CaterpillarData *)catData_;

  // locate caterpillar variables in allocated mem
  Task *const runTask = &caterpillarData->runTask;
  Caterpillar *const caterpillar = &caterpillarData->cat;
  RunCaterpillarArg *const runArg = &caterpillarData->runArg;
  void **const runResult = &caterpillarData->runResult;

  // Initialize caterpillar variables
  errno =
      initCaterpillar(caterpillar, startDir, CATERPILLAR_TOP_ROW,
                      CATERPILLAR_SPAWN_COL, numSegs, CATERPILLAR_INIT_SPEED);
  if (errno != 0) {
    fprintf(stderr,
            "Failure trying to initialize a new caterpillar to spawn in %s\n",
            fooName);
    free((void *)catData_);
    return NULL;
  }
  for (int i = 0; i < numSegs; ++i) { // copy segments
    CaterpillarSegment *destSeg = caterpillar->segments + i;
    const CaterpillarSegment *srcSeg = segs + i;
    *destSeg = *srcSeg;
  }
  caterpillar->segments[0].animTile = CATERPILLAR_ANIM_TILES - 1;
  runArg->caterpillar = caterpillar;
  runArg->sleepGame = registry.sleepGame;
  runArg->runCaterpillarTask = runTask;
  runArg->killedCatLock = registry.killedCatLock;
  runArg->killedCaterpillars = registry.killedCaterpillars;
  errno = task_init(runTask, runCaterpillar, runArg, runResult);
  if (errno != 0) {
    fprintf(stderr,
            "Failure trying to initialize task to run spawned caterpillar "
            "in %s\n",
            fooName);
    free((void *)catData_);
    return NULL;
  }

  // add new caterpillar task to list of running caterpillars on game board
  errno = pthread_mutex_lock(registry.runningCatLock);
  const bool pushed = list_pushBack(registry.runningCaterpillars, &runTask);
  if (!pushed) {
    fprintf(stderr,
            "Failure in %s trying to add running caterpillar to list of "
            "caterpillars on game board\n",
            fooName);
    free((void *)catData_);
    return NULL; // errno set by list_pushBack
  }
  pthread_mutex_unlock(registry.runningCatLock);

  return runTask;
}
