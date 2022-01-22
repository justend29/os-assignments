/**
 * @file job_states.c
 * @author Justen Di Ruscio (3624673)
 * @brief Contents related to process states and transitioning between them.
 * Vectors of PIDs to track states of child processes and a useful wait
 * function.
 *
 * @version 0.1
 * @date 2021-02-17
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <myshell/job_states.h>

#include <errno.h>
#include <stdio.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>

Vector fgPids = {
    .data = NULL, .length = 0, .capacity = 0, .dataSize = sizeof(pid_t)};

Vector suspendedPids = {
    .data = NULL, .length = 0, .capacity = 0, .dataSize = sizeof(pid_t)};

Vector bgPids = {
    .data = NULL, .length = 0, .capacity = 0, .dataSize = sizeof(pid_t)};

bool waitForForegroundPids() {
  const char fooName[] = "waitForForegroundPids";

  OptionalVector spawnedPids = vector_copyConstruct(&fgPids);
  if (!spawnedPids.valid) {
    fprintf(stderr, "Failed to copy foreground PIDs in %s\n", fooName);
    return false; // errno set by vector_copyConstruct
  }

  // Iterate through copied PIDs and wait for each one, removing it from fgPids
  // when it finishes
  for (unsigned childIdx = 0; childIdx < fgPids.length; ++childIdx) {
    // Wait for next foreground PID
    int status;
    const pid_t *pid = (pid_t *)vector_at(&spawnedPids.data, childIdx);
    const pid_t childPid = waitpid(*pid, &status, WUNTRACED);
    if (childPid == -1 && errno != EINTR) { // errno set by waitpid
      fprintf(stderr, "Parent %i failed to wait for child %i in %s\n", getpid(),
              *pid, fooName);
      return false;
    }

    // Find index of stopped child in fgPids
    const ssize_t fgPidIdx = vector_find(&fgPids, (void *)pid);
    if (fgPidIdx < 0) {
      fprintf(stderr, "Lost tracking of child PIDs. Cannot find %i in %s\n",
              *pid, fooName);
      errno = ESRCH;
      return false;
    }

    // Add PID to suspendPids list if SIGTSTP signal stopped child
    if (WIFEXITED(status) == 0) {
      vector_pushBack(&suspendedPids, pid);
    }

    // erase PID from fgPids
    const bool erased = vector_erase(&fgPids, fgPidIdx);
    if (!erased) { // errno set by vector_erase
      fprintf(stderr, "Failure erasing PID %i from foreground PIDs in %s\n",
              *pid, fooName);
      return false;
    }
  }

  return true;
}
