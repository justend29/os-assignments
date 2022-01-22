#pragma once
/**
 * @file bg.h
 * @author Justen Di Ruscio (3624673)
 * @brief Contents related to the handling of the internal command to background
 * processes.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/vector.h>
#include <sys/types.h>

#define BG_COMMAND_NAME \
  "bg"  // command name as a string expected on the command line

/**
 * @brief Executes the background command
 *
 * @param commandName command name as a string for error messages
 * @param commandArgs Vector of Strings of the separated args provided by user
 * @return int return code of the command (errno)
 */
int executeBg(const char* const commandName, const Vector* const commandArgs);
