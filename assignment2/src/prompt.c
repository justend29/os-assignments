/**
 * @file prompt.c
 * @author Justen Di Ruscio (3624673)
 * @brief Function definitions specific to myshell's user prompt.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#define _POSIX_C_SOURCE 200809L

#include <myshell/prompt.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <jd/string.h>
#include <myshell/commands/internal/exit.h>

bool ignoreInput = false;

const String defaultStripChars = {
    .data = ARG_DELIMETER, .length = ARG_DELIMETER_LEN, .capacity = 0};

void printPrompt(String *const cwd) {
  const char fooName[] = "printPrompt";
  // Argument Validity Check
  errno = 0;
  if (cwd == (String *)NULL) {
    fprintf(stderr, "field 'cwd' must point to a valid address in %s\n",
            fooName);
    errno = EPERM;
  }

  // Resolve current working directory
  // populate cwd string with CWD, resizing as needed
  while (true) {
    const char *path = getcwd(cwd->data, cwd->capacity);
    // handle getcwd errors
    if (path == (char *)NULL) { // cwd isn't large enough -> resize
      const bool success = string_reserve(cwd, (1 + cwd->capacity) << 2);
      if (!success) { // string_resize may set errno
        fprintf(stderr,
                "Failure resizing buffer to store current working directory in "
                "%s\n",
                fooName);
        handleExitError(errno);
      }
      continue;
    }
    if (errno != 0) { // other error with getcwd
      fprintf(stderr, "Failure reading current working directory in %s\n",
              fooName);
      handleExitError(errno);
    }

    // no getcwd error
    cwd->length = strlen(cwd->data);
    break;
  }

  // Print prompt
  printf("%s%% ", cwd->data);
}

bool readInputLine(String *const userInput, const String *stripChars) {
  const char fooName[] = "readInputLine";
  bool valid = false;

  // Argument Validity Checks
  errno = 0;
  if (userInput == (String *)NULL) {
    fprintf(stderr, "argument 'userInput' of %s must point to a valid string\n",
            fooName);
    errno = EPERM;
    return valid;
  }

  if (stripChars == (String *)NULL) {
    stripChars = &defaultStripChars;
  }

  // Read stdin as user input
  ignoreInput = false;
  char *read = fgets(userInput->data, userInput->capacity, stdin);
  if (ignoreInput) { // ignore input on Ctrl+Z
    userInput->length = 0;
    userInput->data[0] = '\0';
  } else if (read == NULL) { // end-of-file. Ctrl+D
    // overwrite input with exit command
    const size_t cmdLen = strlen(EXIT_COMMAND_NAME);
    string_reserve(userInput, cmdLen);
    userInput->length = cmdLen;
    strcpy(userInput->data, EXIT_COMMAND_NAME);
  } else {
    // overwrite input with stripped version of input
    userInput->length = strlen(userInput->data);
    const OptionalString stripped = string_strip(userInput, stripChars);
    if (!stripped.valid) {
      fprintf(stderr,
              "Failure removing unnecessary characters from user input, %s, in "
              "%s\n",
              userInput->data, fooName);
      return valid; // errno set by string_strip
    }
    const String strippedInput = stripped.data;
    userInput->length = strlen(strippedInput.data);
    strcpy(userInput->data, strippedInput.data);
    string_freeData(&strippedInput);
  }

  valid = true;
  return valid;
}
