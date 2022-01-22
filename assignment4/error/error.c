/**
 * @file error.c
 * @author Justen Di Ruscio
 * @brief Contains definitions related to error handling or printing
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "error.h"

#include <errno.h>
#include <stdio.h>

void argValidityCheck(const void *const arg, const char *const argName,
                      const char *const fooName) {
  errno = 0;
  if (arg == NULL) {
    fprintf(stderr, "Argument '%s' of %s must point to a valid address\n",
            argName, fooName);
    errno = EPERM;
  }
}
