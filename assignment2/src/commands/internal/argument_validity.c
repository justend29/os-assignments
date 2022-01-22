/**
 * @file argument_validity.c
 * @author Justen Di Ruscio (3624673)
 * @brief Contains definitions for functions related to argument validity checks
 * for internal commands. All internal commands have the same interface - this
 * allows the same argument validity checks among all executor functions.
 * @version 0.1
 * @date 2021-02-17
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "argument_validity.h"

#include <errno.h>
#include <stdio.h>

int argumentValidityCheck(const char *const commandName,
                          const Vector *const commandArgs,
                          const char *const fooName) {
  errno = 0;
  if (fooName == (char *)NULL) {
    fprintf(stderr, "argument 'fooName' of argumentValidityCheck must point to "
                    "a valid address\n");
  } else if (commandName == (char *)NULL) {
    fprintf(stderr,
            "argument 'commandName' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
  } else if (commandArgs == (Vector *)NULL) {
    fprintf(stderr,
            "argument 'commandArgs' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
  }
  return errno;
}
