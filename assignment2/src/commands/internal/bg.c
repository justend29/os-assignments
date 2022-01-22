/**
 * @file bg.c
 * @author Justen Di Ruscio (3624673)
 * @brief Contents related to the handling of the internal command to background
 * processes.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#define _POSIX_C_SOURCE 200809L

#include <myshell/commands/internal/bg.h>

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
  return "bg - send jobs to background; resuming them if they are suspended\n"
         "If no job PID is specified, all foreground processes are sent to\n"
         "background\n"
         "bg [PID...]\n";
}

/**
 * @brief Sends process with PID pid to background. Sets errno upon error.
 *
 * @param pid process ID to background
 * @return int errno
 */
static int bgChild(const pid_t pid) {
  const char fooName[] = "bgChild";

  // Add child PID to list of background jobs
  const bool pushed = vector_pushBack(&bgPids, &pid);
  if (!pushed) {
    fprintf(stderr, "Failed to add pid %i to list of background jobs in %s\n",
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

int executeBg(const char *const commandName, const Vector *const commandArgs) {
  const char fooName[] = "executeBg";
  Vector childPids = vector_constructEmpty(sizeof(pid_t));
  int err = argumentValidityCheck(commandName, commandArgs, fooName);
  if (err != 0) {
    return err; // errno set by argumentValidityCheck
  }

  // Parse provided arguments for job PID
  // provided PID arguments -> parse PIDs
  if (commandArgs->length >= 2) {
    for (unsigned pidIdx = 1; pidIdx < commandArgs->length; ++pidIdx) {
      // convert PID arg to int
      const String *pidStr = (String *)vector_at(commandArgs, pidIdx);
      const int pid = strtol(pidStr->data, NULL, 10);
      if (errno != 0) {
        fprintf(stderr, "Failure converting provided PID %s to integer in %s\n",
                pidStr->data, fooName);
        return errno; // errno set by strtol
      }
      // find PID arg in suspended PIDs
      ssize_t suspendedIdx = vector_find(&suspendedPids, &pid);
      if (suspendedIdx == -1) {
        fprintf(stderr, "PID %i is not a suspended subprocess in %s\n", pid,
                fooName);
        return ENOENT;
      }
      // add PID arg to processes to background
      const bool pushed = vector_pushBack(&childPids, &pid);
      if (!pushed) {
        fprintf(stderr,
                "Failure adding pid %i to list of pids to background in %s\n",
                pid, fooName);
        vector_freeData(&childPids);
        return errno; // errno set by vector_pushBack
      }
      // remove PID from list of suspended PIDs
      const bool erased = vector_erase(&suspendedPids, suspendedIdx);
      if (!erased) {
        fprintf(stderr,
                "Failure erasing PID %i  from list of suspended PIDs in %s\n",
                pid, fooName);
        return errno;
      }
    }
  }

  // provided no specific PID argument -> send to all children
  else if (commandArgs->length == 1) {
    // no jobs to background
    if (suspendedPids.length == 0) {
      fprintf(stderr, "bg: No suitable jobs\n");
      return 0;
    }
    // copy all suspended PIDs to background
    OptionalVector optVec = vector_copyConstruct(&suspendedPids);
    if (!optVec.valid) {
      fprintf(stderr, "Failure to copy construct child PIDs in %s", fooName);
      vector_freeData(&optVec.data);
      return errno; // errno set by vector_copyConstruct
    }
    childPids = optVec.data;
    // remove PIDs to background from list of suspended PIDs
    vector_clear(&suspendedPids);
  }

  // provided incorrect number of arguments
  else {
    fprintf(stderr, "Incorrect arguments provided to %s in %s\n%s\n",
            BG_COMMAND_NAME, fooName, helpMessage());
    errno = EPERM;
    return errno;
  }

  // Background children
  for (unsigned pidIdx = 0; pidIdx < childPids.length; ++pidIdx) {
    const pid_t *pid = (pid_t *)vector_at(&childPids, pidIdx);
    const int err = bgChild(*pid);
    if (err != 0) {
      fprintf(stderr,
              "Failure trying to bring child with PID %i to foreground in %s\n",
              *pid, fooName);
      vector_freeData(&childPids);
      return errno;
    }
  }

  vector_freeData(&childPids);
  return errno;
}
