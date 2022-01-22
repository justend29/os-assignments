#pragma once
/**
 * @file fg.h
 * @author Justen Di Ruscio (3624673)
 * @brief Contents related to the handling of the internal command to foreground
 * processes.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/vector.h>

#define FG_COMMAND_NAME \
  "fg"  // command name as a string expected on the command line

/**
 * @brief Executes the foreground command
 *
 * @param commandName command name as a string for error messages
 * @param commandArgs Vector of Strings of the separated args provided by user
 * @return int return code of the command (errno)
 */
int executeFg(const char *const commandName, const Vector *const commandArgs);
