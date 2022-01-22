#pragma once
/**
 * @file spawn_caterpillars.h
 * @author your name (you@domain.com)
 * @brief Contents relating to spawning caterpillars
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <pthread.h>

#include <jd/list.h>
#include <jd/task.h>
#include <jd/threadpool.h>

#include <caterpillar/caterpillars/caterpillar.h>

// provide to spawnCaterpillars
typedef struct SpawnCaterpillarsArg {
  pthread_mutex_t *const runningCatLock;
  List *const runningCaterpillars;
  Task *const sleepGame;
  ThreadPool *const threadPool;
  pthread_mutex_t *const killedCatLock;
  List* const killedCaterpillars;
} SpawnCaterpillarsArg;

/**
 * @brief Thread start function to spawn caterpillars at random intervals
 *
 * @param data SpawnCaterpillarArg
 * @return void* errno
 */
void* spawnCaterpillars(void* data);

/**
 * @brief Spawns a single caterpillar with segments copied from those provided.
 *
 * @param segs Segments to copy to new caterpillar
 * @param numSegs number of segments
 * @return Task* task containing spawned caterpillar
 */
Task* spawnCaterpillar(const CaterpillarSegment *segs, const int numSegs);
