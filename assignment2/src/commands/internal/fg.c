/**
 * @file fg.c
 * @author Justen Di Ruscio (3624673)
 * @brief Contents related to the handling of the internal command to foreground
 * processes.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#define _POSIX_C_SOURCE 200809L

#include <myshell/commands/internal/fg.h>

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "argument_validity.h"
#include <jd/error.h>
#include <jd/string.h>
#include <jd/vector.h>
#include <myshell/job_states.h>

static char *helpMessage() {
  return "fg - bring job to foreground\n"
         "If no job PID is specified, all child process are brought to\n"
         "foreground\n"
         "fg [PID]\n";
}

/**
 * @brief Sends process with PID pid to foreground. Sets errno upon error.
 *
 * @param pid process ID to foreground
 * @return int errno
 */
static int fgChild(const pid_t pid) {
  const char fooName[] = "fgChild";

  // Add child PID to list of foreground jobs
  const bool pushed = vector_pushBack(&fgPids, &pid);
  if (!pushed) {
    fprintf(stderr, "Failed to add pid %i to list of foreground jobs in %s\n",
            pid, fooName);
    return errno;
  }

  // Send signal to job with specified PID
  const bool sent = !kill(pid, SIGCONT);
  if (!sent) {
    fprintf(stderr,
            "Failed to send SIGCONT (%i) signal to child with PID %i in %s\n",
            SIGCONT, pid, fooName);
    return errno;
  }

  return 0;
}

int executeFg(const char *const commandName, const Vector *const commandArgs) {
  const char fooName[] = "executeFg";
  Vector childPids = vector_constructEmpty(sizeof(pid_t));
  int err = argumentValidityCheck(commandName, commandArgs, fooName);
  if (err != 0) {
    return err; // errno set by argumentValidityCheck
  }

  // Parse provided arguments for job PID
  // provided single PID argument -> parse single PID
  if (commandArgs->length == 2) {
    // convert arg to int
    const String *pidStr = (String *)vector_at(commandArgs, 1);
    const int pid = strtol(pidStr->data, NULL, 10);
    if (errno != 0) {
      fprintf(stderr, "Failure converting provided PID %s to integer in %s\n",
              pidStr->data, fooName);
      return errno; // errno set by strtol
    }
    // find PID arg in suspended or bg PIDs
    ssize_t suspendedIdx = vector_find(&suspendedPids, &pid);
    ssize_t bgIdx = vector_find(&bgPids, &pid);
    if (suspendedIdx == -1 && bgIdx == -1) {
      fprintf(stderr, "PID %i is not a suspended or background subprocess\n",
              pid);
      return ENOENT;
    }
    // add PID arg processes to foreground
    const bool pushed = vector_pushBack(&childPids, &pid);
    if (!pushed) {
      fprintf(stderr,
              "Failure adding pid %i to list of pids to foreground in %s\n",
              pid, fooName);
      vector_freeData(&childPids);
      return errno; // errno set by vector_pushBack
    }

    // remove PID from suspended/bg lists
    bool erased;
    if (suspendedIdx == -1) {
      erased = vector_erase(&bgPids, bgIdx);
      if (!erased) {
        fprintf(
            stderr,
            "failed to erase %zu element in list of background PIDs in %s\n",
            bgIdx, fooName);
        return errno; // errno set by vector_erase
      }
    } else {
      erased = vector_erase(&suspendedPids, suspendedIdx);
      if (!erased) {
        fprintf(stderr,
                "failed to erase %zu element in list of suspended PIDs in %s\n",
                suspendedIdx, fooName);
        return errno; // errno set by vector_erase
      }
    }
  }

  // provided no specific PID argument -> send to all children
  else if (commandArgs->length == 1) {
    // no jobs to foreground
    if (suspendedPids.length == 0 && bgPids.length == 0) {
      fprintf(stderr, "fg: No such job\n");
      return 0;
    }
    // assign all suspended and background PIDS child PIDS to foreground
    OptionalVector optVec = vector_append(&suspendedPids, &bgPids);
    if (!optVec.valid) {
      fprintf(stderr, "Failure to append suspended and background PIDs in %s",
              fooName);
      vector_freeData(&optVec.data);
      return errno; // errno set by vector_append
    }
    childPids = optVec.data;
    vector_clear(&suspendedPids);
    vector_clear(&bgPids);
  }

  // provided incorrect number of arguments
  else {
    fprintf(stderr, "Incorrect arguments provided to %s in %s\n%s\n",
            FG_COMMAND_NAME, fooName, helpMessage());
    errno = EPERM;
    return errno;
  }

  // Foreground children
  for (unsigned pidIdx = 0; pidIdx < childPids.length; ++pidIdx) {
    const pid_t *pid = (pid_t *)vector_at(&childPids, pidIdx);
    const int err = fgChild(*pid);
    if (err != 0) {
      fprintf(stderr,
              "Failure trying to bring child with PID %i to foreground in %s\n",
              *pid, fooName);
      vector_freeData(&childPids);
      return errno;
    }
  }

  // Wait for all foregrounded children
  const bool waited = waitForForegroundPids();
  if (!waited) {
    fprintf(stderr, "Parent %i failed to wait for children in %s\n", getpid(),
            fooName);
    return errno; // errno set by waitForForegroundPids
  }

  vector_freeData(&childPids);
  return errno;
}
