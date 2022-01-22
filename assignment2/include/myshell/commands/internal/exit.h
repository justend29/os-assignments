#pragma once
/**
 * @file exit.h
 * @author Justen Di Ruscio (3624673)
 * @brief Contents related to exiting myshell
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/vector.h>

#define EXIT_COMMAND_NAME \
  "exit"  // string command name expected on the command line

/**
 * @brief Executes the exit command
 *
 * @param commandName command name as a string for error messages
 * @param commandArgs Vector of Strings of the separated args provided by user
 * @return int return code of the command (errno)
 */
int executeExit(const char *const commandName, const Vector *const commandArgs);
