/**
 * @file commands.c
 * @author Justen Di Ruscio
 * @brief Contained in this file are the function definitions for the support
 * functions necessary to service the shell commands. Command handlers have
 * separate files
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "commands.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define DELIM " \t" // delimeter of command arguments

char *getArg1(const char *const buffer) {
  const char fooName[] = "getArg1";
  // Read directory
  if (strtok((char *)buffer, DELIM) == NULL) {
    fprintf(stderr, "Unable to read first token from input command %s in %s\n",
            buffer, fooName);
    errno = EINVAL;
    return NULL;
  }
  char *const arg1 = strtok(NULL, DELIM);
  if (arg1 == NULL) {
    fprintf(stderr, "Expected second argument for command %s in %s\n", buffer,
            fooName);
    errno = EINVAL;
  }
  return arg1;
}
