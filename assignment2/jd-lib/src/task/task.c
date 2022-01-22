/**
 * @file task.c
 * @author Justen Di Ruscio
 * @brief Definitions for Task abstraction
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <jd/threadpool.h>

#include <errno.h>
#include <stdio.h>

int task_init(Task *const task, void *(*foo)(void *), void *fooArg,
              void **fooReturn) {
  const char fooName[] = "tp_initTask";

  // Argument Validity Checks
  errno = 0;
  if (task == (Task *)NULL) {
    fprintf(stderr, "argument 'task' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }
  if (foo == NULL) {
    fprintf(stderr, "argument 'foo' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Init Provided Task
  task->foo = foo;
  task->fooArg = fooArg;
  task->fooReturn = fooReturn;
  task->fooErrno = 0;
  task->completed = false;

  errno = pthread_cond_init(&task->completedCond, NULL);
  if (errno != 0) {
    fprintf(stderr,
            "Failure trying to initialize condition variable for task in %s\n",
            fooName);
    return errno;
  }

  errno = pthread_mutex_init(&task->completedMutex, NULL);
  if (errno != 0) {
    fprintf(stderr, "Failure trying to initialize mutex for task in %s\n",
            fooName);
  }

  return errno;
}

int task_destroy(Task *const task) {
  const char fooName[] = "tp_destoryTask";

  // Argument Validity Check
  errno = 0;
  if (task == (Task *)NULL) {
    fprintf(stderr, "argument 'task' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Destory provided Task
  task->foo = NULL;
  task->fooArg = (void *)NULL;
  task->fooReturn = (void *)NULL;
  task->fooErrno = 0;

  // unblock any blocked callers
  errno = pthread_mutex_lock(&task->completedMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Failure trying to lock task thread while destroying task in %s\n",
            fooName);
    return errno;
  }

  task->completed = true;
  errno = pthread_cond_broadcast(&task->completedCond);
  if (errno != 0) {
    fprintf(stderr, "Failure trying to unblock all callers for task in %s\n",
            fooName);
    return errno;
  }

  errno = pthread_mutex_unlock(&task->completedMutex);
  if (errno != 0) {
    fprintf(
        stderr,
        "Failure trying to unlock task thread while destroying task in %s\n",
        fooName);
    return errno;
  }

  // destroy synchronization variables
  errno = pthread_cond_destroy(&task->completedCond);
  if (errno != 0) {
    fprintf(stderr,
            "Failure trying to destroy condition variable for task in %s\n",
            fooName);
    return errno;
  }

  errno = pthread_mutex_destroy(&task->completedMutex);
  if (errno != 0) {
    fprintf(stderr, "Failure trying to destroy mutex for task in %s\n",
            fooName);
  }

  return errno;
}

int task_execute(Task *const task) {
  const char fooName[] = "task_execute";

  // Argument Validity Check
  errno = 0;
  if (task == (Task *)NULL) {
    fprintf(stderr, "argument 'task' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Execute function and store result;
  void *result = task->foo(task->fooArg);
  if (task->fooReturn != NULL) {
    *(task->fooReturn) = result;
  }
  task->fooErrno = errno;
  return 0;
}

int task_markCompleted(Task *const task) {
  const char fooName[] = "task_markCompleted";

  // Argument Validity Check
  errno = 0;
  if (task == (Task *)NULL) {
    fprintf(stderr, "argument 'task' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Mark Task completed
  if (task->completed != true) {
    errno = pthread_mutex_lock(&task->completedMutex);
    if (errno != 0) {
      fprintf(
          stderr,
          "Failure while locking task mutex to signal task completed in %s\n",
          fooName);
      return errno;
    }
    task->completed = true;
    errno = pthread_cond_broadcast(&task->completedCond);
    if (errno != 0) {
      fprintf(stderr, "Unable to broadcast completed signal for task in %s\n",
              fooName);
      return errno;
    }
    errno = pthread_mutex_unlock(&task->completedMutex);
    if (errno != 0) {
      fprintf(
          stderr,
          "Failure while locking task mutex to signal task completed in %s\n",
          fooName);
    }
  }
  return errno;
}

int task_getResult(Task *const task) {
  const char fooName[] = "tp_getTaskResult";

  // Argument Validity Check
  if (task == (Task *)NULL) {
    fprintf(stderr, "Argument 'task' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Wait for result to become available
  errno = pthread_mutex_lock(&task->completedMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Error while attempting to lock task mutex to wait for task "
            "completion in %s\n",
            fooName);
    return errno;
  }
  while (!task->completed) {
    errno = pthread_cond_wait(&task->completedCond, &task->completedMutex);
    if (errno != 0) {
      fprintf(stderr,
              "Encountered error while waiting on task to become available "
              "in %s\n",
              fooName);
      return errno;
    }
  }
  errno = pthread_mutex_unlock(&task->completedMutex);
  if (errno != 0) {
    fprintf(
        stderr,
        "Error while attempting to unlock task mutex after waiting for task "
        "completion in %s\n",
        fooName);
  }
  return errno;
}
