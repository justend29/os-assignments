/**
 * @file signal_handlers.c
 * @author Justen Di Ruscio
 * @brief Function defintions to handle specific signals received by processes
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#define _POSIX_C_SOURCE 200809L

#include <myshell/signal_handlers.h>

#include <stdio.h>

#include <jd/error.h>
#include <jd/string.h>
#include <myshell/job_states.h>

extern bool ignoreInput;

/**
 * @brief Prints information regarding suspending processes. Updates job state
 * tracking. Exits the program upon failure.
 *
 */
static void handleSigTStop() {
  // Handle Signal
  if (fgPids.length == 0 && bgPids.length == 0) {
    printf("\nNo job to suspend\n");
    ignoreInput = true;
  } else { // child process running
    if (bgPids.length != 0) {
      ignoreInput = true;
      vector_assign(&suspendedPids, &bgPids);
      vector_clear(&bgPids);
    }
  }
}

void handleSignal(const int sigNum) {
  const char fooName[] = "handleSignal";

  // Re-register signal
  const __sighandler_t registered = signal(SIGTSTP, handleSignal);
  if (registered == SIG_ERR) {
    fprintf(stderr,
            "Failure re-registering handler for SIGTSTP (%i) signal in %s\n",
            SIGTSTP, fooName);
    handleExitError(errno);
  }

  switch (sigNum) {
  case SIGTSTP:
    handleSigTStop();
    break;
  default:
    fprintf(
        stderr,
        "Signal number %i does not have an associated signal handler in %s\n",
        sigNum, fooName);
    handleExitError(EPERM);
  }
}
