/**
 * @file exit.c
 * @author Justen Di Ruscio (3624673)
 * @brief Contents related to exiting myshell
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#define _POSIX_C_SOURCE 200809L

#include <myshell/commands/internal/exit.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "argument_validity.h"
#include <jd/error.h>
#include <jd/string.h>
#include <myshell/job_states.h>

static char *helpMessage() {
  return "exit - exit program"
         "Exits program with status code 0 or that of the optional\n"
         "argument converted to an integer\n"
         "exit [STATUS]\n";
}

int executeExit(const char *const commandName,
                const Vector *const commandArgs) {
  const char fooName[] = "executeExit";
  const int err = argumentValidityCheck(commandName, commandArgs, fooName);
  if (err != 0) {
    return err;
  }

  // Exit Program
  int statusCode = 0;
  if (commandArgs->length == 2) { // provided single status code
    const String *const statusString = (String *)vector_at(commandArgs, 1);
    if (statusString == (String *)NULL) {
      fprintf(stderr,
              "Could not resolve second argument provided to exit in %s\n",
              fooName);
      return errno; // errno set by vector_at
    }
    statusCode = strtol(statusString->data, NULL, 10);
    if (errno != 0) {
      fprintf(stderr,
              "Failure converting provided exit status to integer in %s\n",
              fooName);
      return errno; // errno set by strtol
    }
  } else if (commandArgs->length > 2) { // provided too many arguments
    fprintf(stderr, "Too many arguments provided to %s in %s\n%s\n",
            EXIT_COMMAND_NAME, fooName, helpMessage());
    errno = EPERM;
    return EPERM;
  }

  // Kill all child processes in background or suspended
  for (unsigned i = 0; i < bgPids.length; ++i) {
    const pid_t *pid = (pid_t *)vector_at(&bgPids, i);
    kill(*pid, SIGINT);
  }
  for (unsigned i = 0; i < suspendedPids.length; ++i) {
    const pid_t *pid = (pid_t *)vector_at(&suspendedPids, i);
    kill(*pid, SIGINT);
  }

  // Exit program
  exit(statusCode);
}
