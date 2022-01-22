#pragma once
/**
 * @file task.h
 * @author Justen Di Ruscio
 * @brief Declarations of Task abstraction
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <pthread.h>
#include <stdbool.h>

// Encapsulates a packaged task and a future but can also encapsulate a promise
// and a future if foo does nothing, because the task is marked completed
// independently of running foo
typedef struct Task {
  void *(*foo)(void *);
  void *fooArg;
  void **fooReturn;
  int fooErrno;
  bool completed;
  pthread_cond_t completedCond;
  pthread_mutex_t completedMutex;
} Task;


int task_init(Task *const task, void *(*foo)(void *), void *fooArg,
                void **fooReturn);

int task_destroy(Task *const task);

/**
 * @brief executes foo with fooArg but doesn't mark task completed
 *
 * @param task
 * @return int
 */
int task_execute(Task* const task);

/**
 * @brief Marks the task as completed through completed, completedCond and completedMutex
 *
 * @param task task to mark completed
 * @return int errno
 */
int task_markCompleted(Task*const task);

/**
 * @brief Blocks until foo completes and places result of foo in fooReturn
 *
 * @param task
 * @return int
 */
int task_getResult(Task *const task);
