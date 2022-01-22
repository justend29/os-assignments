#pragma once
/**
 * @file commands.h
 * @author Justen Di Ruscio (3624673)
 * @brief Contents related to executing and parsing commands with
 * myshell.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/string.h>
#include <jd/vector.h>

/**
 * @brief Integral value used to indicate which command should be executed by
 * myshell based on user's input. Unknown is not a command name, it indicates
 * the command name is unknown by myshell.
 *
 */
enum CommandName { Unknown, Cd, Exit, Fg, Bg };

/**
 * @brief Parses the provided string, comparing it against known myshell
 * commands, returning identified command name contained in string. Sets errno
 * upon error.
 *
 * @param commandName
 * @return enum CommandName
 */
enum CommandName parseCommandName(const String* const commandName);

/**
 * @brief Executes an internal command with the identified command name and the
 * user provided command arguments (including the string of the command name).
 * Sets errno upon error.
 *
 * @param name parsed, identified command name of internal command
 * @param commandArgs Vector of Strings of the separated user provided args
 * @return int return code of executing the internal command
 */
int execInternal(const enum CommandName name, const Vector* const commandArgs);

/**
 * @brief Executes a system command based on the user provided command
 * arguments. Sets errno upon error.
 *
 * @param cmdArgs Vector of STrings of the separated user provided command args
 * @return int return code of executing the internal command
 */
int execSystem(const Vector* const cmdArgs);
