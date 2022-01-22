/**
 * @file error.c
 * @author Justen Di Ruscio (3624673)
 * @brief Contains definitions for common error handling/checking functions
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/error.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void handleErrorMsg(const int errorNumber) {
  const char *errorString = strerror(errorNumber);
  if (errorString == (char *)NULL) {
    errorString =
        "Invalid error number. Cannot deduce error message from error number";
  }
  fprintf(stderr, "(pid=%i) error = %i - %s\n", getpid(), errorNumber,
          errorString);
}

void handleExitError(const int errorNumber) {
  handleErrorMsg(errorNumber);
  exit(errorNumber);
}

bool memoryOverlaps(const void *const first, const void *const second,
                    const unsigned dataSize) {
  const char *firstBytes = (char *)first;
  const char *secondBytes = (char *)second;
  const bool firstOverlapsSecond =
      firstBytes <= secondBytes && firstBytes + dataSize > secondBytes;
  const bool secondOverlapsFirst =
      secondBytes <= firstBytes && secondBytes + dataSize > firstBytes;
  return firstOverlapsSecond || secondOverlapsFirst;
}
