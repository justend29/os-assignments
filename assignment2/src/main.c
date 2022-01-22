/**
 * @file main.c
 * @author Justen Di Ruscio (3624673)
 * @brief Main entry-point of myshell. Provides a basic shell with piping and
 * job control.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <jd/error.h>
#include <jd/string.h>
#include <jd/vector.h>

#include <myshell/commands/commands.h>
#include <myshell/job_states.h>
#include <myshell/prompt.h>
#include <myshell/signal_handlers.h>

// ========================== Exiting Program ============================
/**
 * @brief Frees all of the arguments; the common, persistent objects used in
 * main()
 *
 * @param cwd current working directory String
 * @param userInput user input to command line
 * @param inputCommands all of the piped input commands
 * @param commandArgs the current list of command args
 * @param numCommands number of piped commands
 */
void freeAll(const String *const cwd, const String *const userInput,
             const Vector *const inputCommands, const Vector *const commandArgs,
             const unsigned numCommands) {
  string_freeData(cwd);
  string_freeData(userInput);
  vector_freeElements(inputCommands);
  vector_freeData(inputCommands);
  if (commandArgs != (Vector *)NULL) {
    // for each command, clear its set of arguments
    for (unsigned cmdIdx = 0; cmdIdx < numCommands; ++cmdIdx) {
      const Vector arguments = commandArgs[cmdIdx];
      vector_freeElements(&arguments);
      vector_freeData(&arguments);
    }
  }
}

/**
 * @brief Frees all of the arguments; the common, persistent objects used in
 * main(), and exits the program
 *
 * @param cwd
 * @param userInput
 * @param inputCommands
 * @param commandArguments
 * @param numCommands
 */
void freeAllAndExit(const String *const cwd, const String *const userInput,
                    const Vector *const inputCommands,
                    const Vector *const commandArguments,
                    const size_t numCommands) {
  const int exitCode = errno;
  freeAll(cwd, userInput, inputCommands, commandArguments, numCommands);
  handleExitError(exitCode);
}

// ========================== Main ============================
int main() {
  int returnCode = EXIT_FAILURE;

  // Register signal handler for myshell (parent process)
  const __sighandler_t registered = signal(SIGTSTP, handleSignal);
  if (registered == SIG_ERR) {
    fprintf(stderr, "Failure registering handler for SIGTSTP (%i) signal\n",
            SIGTSTP);
    handleExitError(errno);
  }

  // Construct initial string to store cwd
  const unsigned defaultCwdLength = 1 << 7;
  OptionalString cwdConstructed = string_constructCapacity(defaultCwdLength);
  if (!cwdConstructed.valid) {
    fprintf(stderr,
            "Failure constructing string of length %u to store current "
            "working directory\n",
            defaultCwdLength);
    string_freeData(&cwdConstructed.data);
    handleExitError(errno);
  }
  String cwd = cwdConstructed.data;

  // String to store user's input commands
  const unsigned defaultInputLength = 1 << 8;
  OptionalString inputOpt = string_constructCapacity(defaultInputLength);
  if (!inputOpt.valid) {
    fprintf(stderr,
            "Failure constructing string of length %u to store user input\n",
            defaultInputLength);
    string_freeData(&inputOpt.data);
    handleExitError(errno);
  }
  String userInput = inputOpt.data;

  // Continuously wait for user input on command line
  while (true) {
    // Print prompt and read user's commands
    printPrompt(&cwd);
    const bool readValid = readInputLine(&userInput, NULL);
    if (!readValid) {
      fprintf(stderr, "Failure reading user input from command line\n");
      freeAllAndExit(&cwd, &userInput, NULL, NULL, 0);
    }

    // Split command-line into piped commands
    OptionalVector inputSplit = string_split(&userInput, PIPE_DELIMETER);
    if (!inputSplit.valid) {
      fprintf(stderr, "Unable to split user input into commands\n");
      freeAllAndExit(&cwd, &userInput, &inputSplit.data, (Vector *)NULL,
                     inputSplit.data.length);
    }
    Vector inputCommands = inputSplit.data;

    // Split each piped command into arguments. Do this before executing
    // commands in order to validate command name is known

    // create arrays to store vectors of split command args, and command names
    const size_t numCmds = inputCommands.length;
    Vector commandArgs[numCmds];
    enum CommandName commandNames[numCmds];

    // for each piped command, process command arguments
    for (unsigned cmdIdx = 0; cmdIdx < inputCommands.length; ++cmdIdx) {
      // Split command into arguments.
      // access single piped command:
      const String *inputCommand = (String *)vector_at(&inputCommands, cmdIdx);
      if (inputCommand == (String *)NULL) {
        fprintf(stderr, "Error accessing element %u of inputCommands\n",
                cmdIdx);
        freeAllAndExit(&cwd, &userInput, &inputCommands, commandArgs, numCmds);
      }
      // strip unnecessary chars from input command
      const OptionalString optCmd =
          string_strip(inputCommand, &defaultStripChars);
      if (!optCmd.valid) {
        fprintf(
            stderr,
            "Failure stripping unnecessary characters from input command %s\n",
            inputCommand->data);
        string_freeData(&optCmd.data);
        freeAllAndExit(&cwd, &userInput, &inputCommands, commandArgs, numCmds);
      }
      const String cmd = optCmd.data;

      // split single piped command into arguments
      OptionalVector argumentSplit = string_split(&cmd, ARG_DELIMETER);
      string_freeData(&cmd);
      if (!argumentSplit.valid) {
        fprintf(stderr, "Unable to split user command %u into arguments\n",
                cmdIdx);
        vector_freeElements(&argumentSplit.data);
        vector_freeData(&argumentSplit.data);
        freeAllAndExit(&cwd, &userInput, &inputCommands, commandArgs, numCmds);
      }
      Vector currentCmdArgs = argumentSplit.data;
      currentCmdArgs.elementDeleter = string_freeDataVoid;

      if (currentCmdArgs.length) {
        // Determine and store command name from arg0, if known
        String *arg0 = (String *)vector_at(&currentCmdArgs, 0);

        enum CommandName name = parseCommandName(arg0);
        commandNames[cmdIdx] = name;          // store resolved name
        commandArgs[cmdIdx] = currentCmdArgs; // store split arguments
      } else {
        vector_freeElements(&currentCmdArgs);
        vector_freeData(&currentCmdArgs);
      }
    }

    // For each piped command, run command and pipe them together
    int pipeDes[2];
    int pipeWrite = 0, pipeRead = 0;
    for (unsigned cmdIdx = 0; cmdIdx < inputCommands.length; ++cmdIdx) {
      const bool oddCommand = cmdIdx & 0x01;
      // Name, arguments, and new pipe for current piped command
      enum CommandName name = commandNames[cmdIdx];
      Vector currentCmdArgs = commandArgs[cmdIdx];
      if (cmdIdx < inputCommands.length - 1) { // create new pipe
        const int piped = pipe(pipeDes);
        if (piped == -1) {
          fprintf(stderr, "Failure creating pipes between commands\n");
          freeAllAndExit(&cwd, &userInput, &inputCommands, commandArgs,
                         numCmds);
        }
        pipeWrite = pipeDes[1];
      }

      // Execute Internal Command
      if (name != Unknown) {
        if (name == Exit) {
          freeAll(&cwd, &userInput, &inputCommands, NULL, numCmds);
        }
        int result = execInternal(name, &currentCmdArgs);
        if (result != 0) {
          fprintf(stderr, "Error executing internal command\n");
          handleErrorMsg(result);
        }
      }
      // Execute System Command
      else {
        if (suspendedPids.length || bgPids.length) {
          printf(
              "Not allowed to start new command while you have a job active\n");
        } else {
          pid_t pid = fork();
          if (pid == -1) {
            fprintf(stderr, "Unable to fork process %i\n", getpid());
            freeAllAndExit(&cwd, &userInput, &inputCommands, commandArgs,
                           numCmds);
          }
          // Parent process
          else if (pid != 0) {
            // Add child to list of jobs in foreground
            vector_pushBack(&fgPids, &pid);

            // Close file descriptors for pipe
            if (inputCommands.length > 1) {
              if (cmdIdx < inputCommands.length - 1) {
                close(pipeWrite);
              }
              if (oddCommand || cmdIdx == inputCommands.length - 1) {
                close(pipeRead);
              }
            }

            pipeRead = pipeDes[0]; // update read end of pipe for next cmd
          }
          // Child process
          else {
            // Register default signal handler
            const __sighandler_t registered = signal(SIGTSTP, SIG_DFL);
            if (registered == SIG_ERR) {
              fprintf(stderr,
                      "Failure registering handler for SIGTSTP (%i) signal\n",
                      SIGTSTP);
              freeAll(&cwd, &userInput, &inputCommands, commandArgs, numCmds);
              handleExitError(errno);
            }

            // Setup pipes between commands
            if (inputCommands.length >= 2) {
              // connect reading end of pipe to process
              if (cmdIdx > 0) {
                const int dupped = dup2(pipeRead, STDIN_FILENO);
                if (dupped == -1) {
                  const int err = errno;
                  fprintf(stderr, "Child %i failed to assign pipe to %s\n",
                          getpid(), "stdin");
                  errno = err;
                  freeAllAndExit(&cwd, &userInput, &inputCommands, commandArgs,
                                 numCmds);
                }
                close(pipeRead);
              }
              // connect writing end of pipe to process
              if (cmdIdx < inputCommands.length - 1) {
                const int dupped = dup2(pipeWrite, STDOUT_FILENO);
                if (dupped == -1) {
                  const int err = errno;
                  fprintf(stderr, "Child %i failed to assign pipe to %s\n",
                          getpid(), "stdout");
                  errno = err;
                  freeAllAndExit(&cwd, &userInput, &inputCommands, commandArgs,
                                 numCmds);
                }
                close(pipeWrite);
              }
            }

            // Run system command
            returnCode = execSystem(&currentCmdArgs);
            errno = returnCode;
            fprintf(stderr, "Error executing system command\n");
            freeAllAndExit(&cwd, &userInput, &inputCommands, commandArgs,
                           numCmds);
            return returnCode;
          }
        }
      }
    }

    // After all commands have been forked and executed, wait each of them to
    // finish or get stopped Wait for last child to finish
    if (fgPids.length > 0) {
      const bool waited = waitForForegroundPids();
      if (!waited) { // errno set by waitForForegroundPids
        fprintf(stderr, "Failed to wait for foreground PIDs\n");
        freeAllAndExit(&cwd, &userInput, &inputCommands, commandArgs, numCmds);
      }
    }

    freeAll((String *)NULL, (String *)NULL, &inputCommands, commandArgs,
            numCmds);
  }
  return returnCode;
}
