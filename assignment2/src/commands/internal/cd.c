/**
 * @file cd.c
 * @author Justen Di Ruscio (3624673)
 * @brief Contents related to the handling of internal command to change
 * directories.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <myshell/commands/internal/cd.h>

#include <stdio.h>
#include <unistd.h>

#include "argument_validity.h"
#include <jd/error.h>
#include <jd/string.h>
#include <jd/vector.h>

static char *helpMessage() {
  return "cd - change directory\n"
         "cd [DIRECTORY]\n";
}

int executeCd(const char *const commandName, const Vector *const commandArgs) {
  const char fooName[] = "executeCd";
  const int err = argumentValidityCheck(commandName, commandArgs, fooName);
  if (err != 0) {
    return err;
  }

  // Change current directory
  if (commandArgs->length == 2) {
    const String *const path = (String *)vector_at(commandArgs, 1);
    if (path == (String *)NULL) {
      fprintf(stderr, "Unable to resolve path from arguments to %s in %s\n",
              commandName, fooName);
      return errno; // errno set by vector_at
    }
    chdir(path->data);
  } else {
    fprintf(stderr,
            "%s called with %zu positional arguments but expects 1\n%s\n",
            commandName, commandArgs->length - 1, helpMessage());
    errno = EINVAL;
  }
  return errno;
}
