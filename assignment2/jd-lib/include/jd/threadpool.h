#pragma once
/**
 * @file threadpool.h
 * @author Justen Di Ruscio
 * @brief Declarations for an expanding threadpool
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <pthread.h>
#include <stdbool.h>

#include <jd/queue.h>
#include <jd/task.h>


// the number of idle threads in threads are tacked such that enqueueImmediate
// can immediately spawn a new thread if no idle threads are available. This is
// to meet the requirements in the assignment, where multiple specific items
// require their own thread, like each caterpillar. A normal thread pool
// wouldn't have this functionality, but it's part of the assignment.

typedef struct ThreadPool {
  Queue threads;
  unsigned numIdleThreads;
  pthread_cond_t taskAvailable;
  pthread_mutex_t taskMutex;
  Queue waitingTasks;
  bool running;
} ThreadPool;

/**
 * @brief initializes the thread pool. Non-re-entrant. errno set on error
 *
 * @param tp threadpool to initialize
 * @param numInitThreads num thread to start thread pool with
 * @return int errno
 */
int tp_init(ThreadPool *const tp, const unsigned numInitThreads);

/**
 * @brief destroys provided thread pool. errno set of error
 *
 * @param tp thread pool
 * @return true success
 * @return false fail
 */
bool tp_destroy(ThreadPool *const tp);

/**
 * @brief enqueues the provided task into the threadpool, running it immediately by spawning a therad if one is not available, thereby expanding the pool of threads by one
 *
 * @param tp thread pool
 * @param task task to enqueue
 * @return int errno
 */
int tp_enqueueImmediate(ThreadPool *const tp, Task *const task);
