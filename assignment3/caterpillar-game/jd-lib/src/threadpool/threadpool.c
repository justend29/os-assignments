/**
 * @file threadpool.c
 * @author Justen Di Ruscio
 * @brief Definitions for expanding thread pool
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#define _GNU_SOURCE

#include <jd/threadpool.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>

#include "jd/queue.h"
#include "threadpool_private.h"

// ==================== PRIVATE FUNCTIONS ===============
int tp_init(ThreadPool *const tp, const unsigned numInitThreads) {
  const char fooName[] = "tp_init";

  // Argument Validity Check
  errno = 0;
  if (tp == (ThreadPool *)NULL) {
    fprintf(stderr, "argument 'tp' of %s must point to a valid address\n",
            fooName);
  }

  tp->running = true;

  // Initialize condition variable
  errno = pthread_cond_init(&tp->taskAvailable, NULL);
  if (errno != 0) {
    fprintf(stderr,
            "Failed to initialize condition variable for thread pool in %s\n",
            fooName);
    return errno;
  }

  // Initialize mutex for tasks
  errno = pthread_mutex_init(&tp->taskMutex, NULL);
  if (errno != 0) {
    fprintf(stderr, "Failed to initialize task mutex for thread pool in %s\n",
            fooName);
    return errno;
  }

  // Construct empty queue of tasks
  tp->waitingTasks = q_constructEmpty(sizeof(Task *));

  // Construct and store initial threads
  tp->threads = q_constructEmpty(sizeof(pthread_t));
  for (unsigned int threadNum = 0; threadNum < numInitThreads; ++threadNum) {
    const bool spawned = tp_spawnThread(tp);
    if (!spawned) {
      fprintf(stderr, "Failed to spawn new pthread for thread pool in %s\n",
              fooName);
      return errno; // errno set by tp_spawnThread
    }
  }
  tp->numIdleThreads = numInitThreads;

  return errno;
}

bool tp_destroy(ThreadPool *const tp) {
  const char fooName[] = "tp_destroy";

  // Return if there's nothing to destroy
  if (tp == (ThreadPool *)tp) {
    return true;
  }

  // Unblock all threads
  tp->running = false;
  errno = pthread_mutex_lock(&tp->taskMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Failure locking mutex in %s to unblock all threads in pool\n",
            fooName);
    return false;
  }
  const int unblockErr = pthread_cond_broadcast(&tp->taskAvailable);
  errno = pthread_mutex_unlock(&tp->taskMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Failure locking mutex in %s to unblock all threads in pool\n",
            fooName);
    return false;
  }
  if (unblockErr != 0) {
    fprintf(stderr,
            "Failure unblocking all waiting threads with "
            "pthread_cond_broadcast in %s\n",
            fooName);
    errno = unblockErr;
    return false;
  } else {

    // Join all threads from active and idle queues
    for (unsigned tIdx = 0; tIdx < q_length(&tp->threads); ++tIdx) {
      // for each thread in queue
      pthread_t *thread = q_front(&tp->threads);
      if (thread == (pthread_t *)NULL) {
        fprintf(stderr,
                "Unable to read thread from queue of thread pool in %s\n",
                fooName);
        return false; // errno set by q_front
      }
      const bool dequeued = q_dequeue(&tp->threads);
      if (!dequeued) {
        fprintf(stderr,
                "Unable to remove thread from queue of thread pool in %s\n",
                fooName);
        return false; // errno set by q_dequeue
      }
      errno = pthread_join(*thread, NULL);
      if (errno != 0) {
        fprintf(stderr, "Failure joining thread %u from thread queue in %s\n",
                tIdx, fooName);
        return false;
      }
    }

    // Destroy condition variable
    const int err1 = pthread_cond_destroy(&tp->taskAvailable);
    if (errno != 0) {
      fprintf(stderr,
              "Failure destroying thread pool's condition variable in %s\n",
              fooName);
    }

    // Destroy mutexes
    const int err2 = pthread_mutex_destroy(&tp->taskMutex);
    if (errno != 0) {
      fprintf(stderr, "Failure destroying thread pool's task mutex in %s\n",
              fooName);
    }

    errno = err1 != 0 ? err1 : err2;
    return errno == 0;
  }

  // Free Queues
  q_freeElements(&tp->threads);
  q_freeElements(&tp->waitingTasks);
}

// non-reentrant
bool tp_spawnThread(ThreadPool *const tp) {
  const char fooName[] = "tp_spawnThread";

  // Argument Validity Check
  errno = 0;
  if (tp == (ThreadPool *)NULL) {
    fprintf(stderr, "argument 'tp' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  pthread_t thread = 0;

  // Create thread with default attributes
  // thread is joinable (non-detached) by default
  errno = pthread_create(&thread, NULL, workerFunction, tp);
  if (errno != 0) {
    fprintf(stderr, "Failed to create new pthread in %s\n", fooName);
    return false;
  }

  // Store thread in threadpool's queue
  const bool enqueued = q_enqueue(&tp->threads, &thread);
  if (!enqueued) {
    fprintf(stderr, "Failed to add new pthread to thread pool in %s\n",
            fooName);
    return false; // errno set by q_enqueue
  }
  ++tp->numIdleThreads;

  return true;
}

int tp_enqueueImmediate(ThreadPool *const tp, Task *const task) {
  const char fooName[] = "tp_enqueueImmediate";

  // Argument Validity Check
  errno = 0;
  if (tp == (ThreadPool *)NULL) {
    fprintf(stderr, "argument 'tp' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // obtain lock to update waiting task queue and spawn threads
  errno = pthread_mutex_lock(&tp->taskMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Encountered error while attempting to lock mutex to modify "
            "thread pool's queues in %s\n",
            fooName);
    return errno;
  }

  // Spawn new thread if none are currently available
  if (tp->numIdleThreads == 0) {
    const bool spawned = tp_spawnThread(tp);
    if (!spawned) {
      fprintf(stderr, "Failed spawning new thread pool thread in %s\n",
              fooName);
      return errno; // errno set by tp_spawnThread
    }
  }

  // Add task to queue
  const bool enqueued = q_enqueue(&tp->waitingTasks, &task);
  if (!enqueued) {
    fprintf(stderr,
            "Unable to enqueue new task in thread pool's task queue in %s\n",
            fooName);
    return errno; // errno set by q_enqueue
  }

  // Notify that a task is available in waitingTasks queue
  errno = pthread_cond_signal(&tp->taskAvailable);
  if (errno != 0) {
    fprintf(stderr,
            "Error while signaling a task is available for thread pool in %s\n",
            fooName);
    return errno;
  }

  errno = pthread_mutex_unlock(&tp->taskMutex);
  if (errno != 0) {
    fprintf(stderr,
            "Encountered error while unlocking mutex after modifying "
            "thread pool's queues in %s\n",
            fooName);
  }

  return errno;
}

// ==================== PRIVATE FUNCTIONS ===============
/**
 * @brief function each thread in pool runs to execute tasks
 *
 * @param tp thread pool
 * @return void* errno
 */
void *workerFunction(void *tp) {
  const char fooName[] = "workerFunction";

  // Argument Validity Check
  errno = 0;
  if (tp == (void *)NULL) {
    fprintf(stderr, "argument 'tp' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return (void *)(size_t)errno;
  }

  ThreadPool *tp_ = (ThreadPool *)tp;

  while (true) {
    // Obtain lock to wait for available task
    errno = pthread_mutex_lock(&tp_->taskMutex);
    if (errno != 0) {
      fprintf(stderr,
              "Encountered error while attempting to lock mutex in %s\n",
              fooName);
      break;
    }

    // Wait for task to become available, indicated
    // by taskAvailable condition variable
    while (tp_->running && q_length(&tp_->waitingTasks) == 0) {
      errno = pthread_cond_wait(&tp_->taskAvailable, &tp_->taskMutex);
      if (errno != 0) {
        fprintf(stderr,
                "Encountered error while waiting on task to become available "
                "in %s\n",
                fooName);
        return (void *)(size_t)errno;
      }
    }

    // leave lock held after pthread_cond_wait to modify thread pool

    // exit if forced to quit while idle w/o waiting task
    if (!tp_->running && q_length(&tp_->waitingTasks) == 0) {
      // release lock
      errno = pthread_mutex_unlock(&tp_->taskMutex);
      if (errno != 0) {
        fprintf(stderr,
                "Encountered error while attempting to unlock mutex after "
                "waking idle thread in %s\n",
                fooName);
      }
      break;
    }

    // decrement number of available threads
    --tp_->numIdleThreads;

    // Extract task to run
    const void *const frontT = q_front(&tp_->waitingTasks);
    if (frontT == (void *)NULL) {
      fprintf(
          stderr,
          "Unable to peek front element from queue of waiting tasks in %s\n",
          fooName);
      break; // errno set by q_front
    }
    Task *task = *(Task **)frontT;

    const bool dequeued = q_dequeue(&tp_->waitingTasks);
    if (!dequeued) {
      fprintf(stderr,
              "Unable to remove tasks from queue of waiting tasks in %s\n",
              fooName);
      break; // errno set by q_dequeue
    }

    // release lock after modifying queues
    errno = pthread_mutex_unlock(&tp_->taskMutex);
    if (errno != 0) {
      fprintf(
          stderr,
          "Encountered error while attempting to unlock mutex after modifying "
          "thread pool's queues in %s\n",
          fooName);
      break;
    }

    // Execute extracted task and store result in task
    errno = task_execute(task);
    if (errno != 0) {
      fprintf(stderr, "Failure trying to execute task funciton in %s\n",
              fooName);
      break;
    }

    // Mark Task as completed when it finishes
    errno = task_markCompleted(task);
    if (errno != 0) {
      fprintf(stderr, "Unable to mark task as completed in %s\n", fooName);
      break;
    }

    // Obtain lock to update number of available tasks
    errno = pthread_mutex_lock(&tp_->taskMutex);
    if (errno != 0) {
      fprintf(stderr,
              "Encountered error while attempting to lock mutex to update idle "
              "threads in %s\n",
              fooName);
      break;
    }

    ++tp_->numIdleThreads;

    errno = pthread_mutex_unlock(&tp_->taskMutex);
    if (errno != 0) {
      fprintf(stderr,
              "Encountered error while attempting to unlock mutex after "
              "updating idle threads in %s\n",
              fooName);
      break;
    }
  }

  return (void *)(size_t)errno;
}
