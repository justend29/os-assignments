/**
 * @file commands.c
 * @author Justen Di Ruscio (3624673)
 * @brief Definitions related to executing and parsing commands with
 * myshell.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <myshell/commands/commands.h>

#include <stdio.h>
#include <unistd.h>

#include <myshell/commands/internal/bg.h>
#include <myshell/commands/internal/cd.h>
#include <myshell/commands/internal/exit.h>
#include <myshell/commands/internal/fg.h>

const char *availableCommandNames[] = {CD_COMMAND_NAME, EXIT_COMMAND_NAME,
                                       FG_COMMAND_NAME, BG_COMMAND_NAME};
const unsigned numAvailableCommands =
    sizeof(availableCommandNames) / sizeof(*availableCommandNames);

/**
 * @brief Array of function pointers to functions handling each system command.
 *
 */
int (*commandExecutors[])(const char *, const Vector *) = {
    executeCd, executeExit, executeFg, executeBg};

enum CommandName parseCommandName(const String *const commandName) {
  const char fooName[] = "parseCommandName";
  errno = 0;

  // Argument Validity Check
  if (commandName == (String *)NULL || commandName->data == (char *)NULL) {
    fprintf(stderr, "field 'commandName' of %s must point to a valid string\n",
            fooName);
    errno = EPERM;
    return Unknown;
  }

  // Parse Name
  for (unsigned cmdIdx = 1; cmdIdx <= numAvailableCommands; ++cmdIdx) {
    if (!string_compareChar(commandName, availableCommandNames[cmdIdx - 1])) {
      return (enum CommandName)(cmdIdx);
    }
  }
  return Unknown;
}

int execInternal(const enum CommandName name, const Vector *const commandArgs) {
  const char fooName[] = "execInternal";
  errno = 0;

  // Argument Validity Checks
  if (name == Unknown) {
    fprintf(stderr, "cannot execute unknown internal command\n");
    errno = EPERM;
    return errno;
  }
  if (commandArgs == (Vector *)NULL) {
    fprintf(stderr,
            "argument 'commandArgs' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Execute Internal Command
  // command name as string for error reporting
  const char *const commandName = availableCommandNames[name - 1];
  // executor for selected command
  int (*commandExecutor)(const char *, const Vector *);
  commandExecutor = commandExecutors[name - 1];
  // run selected command through executor
  return commandExecutor(commandName, commandArgs);
}

int execSystem(const Vector *const cmdArgs) {
  const char fooName[] = "execSystem";

  // Argument Validity Check
  errno = 0;
  if (cmdArgs == (Vector *)NULL) {
    fprintf(stderr, "argument 'cmdArgs' of %s must point to a valid addrss\n",
            fooName);
    errno = EPERM;
    return errno;
  }

  // Exec system command
  // construct contiguous array of char*s - argv
  char *argv[cmdArgs->length + 1];
  argv[cmdArgs->length] = (char *)NULL;
  for (size_t cmdIdx = 0; cmdIdx < cmdArgs->length; ++cmdIdx) {
    const String *const arg = (String *)vector_at(cmdArgs, cmdIdx);
    argv[cmdIdx] = arg->data;
  }
  // system call
  execvp(argv[0], argv);
  return errno;
}
