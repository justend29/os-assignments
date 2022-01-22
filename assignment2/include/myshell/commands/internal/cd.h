#pragma once
/**
 * @file cd.h
 * @author Justen Di Ruscio (3624673)
 * @brief Contents related to the handling of internal command to change
 * directories.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/vector.h>

#define CD_COMMAND_NAME \
  "cd"  // string command name expected on the command line

/**
 * @brief Executes the change directory command
 *
 * @param commandName command name as a string for error messages
 * @param commandArgs Vector of Strings of the separated args provided by user
 * @return int return code of the command (errno)
 */
int executeCd(const char *const commandName, const Vector *const commandArgs);
