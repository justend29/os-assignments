#pragma once
/**
 * @file prompt.h
 * @author Justen Di Ruscio (3624673)
 * @brief Function symbols and macros specific to myshell's user prompt.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/string.h>

#define ARG_DELIMETER " \n\r\t"  // delimeters separating arguments
#define ARG_DELIMETER_LEN 4      // number of argument delimeters
#define PIPE_DELIMETER "|"       // delimeters separating piped commands
#define PIPE_DELIMETER_LEN 1     // number of pipe delimeters

/**
 * @brief Default chars to use when stripping in readInputLine
 *
 */
extern const String defaultStripChars;

/**
 * @brief Prints the user prompt. Uses cwd as the string to store the current
 * working directory. Resizes cwd if it's not large enough. Exits upon error, as
 * either a system call failed or the machine is out of memory.
 *
 * @param cwd String to store current working directory path. Resized if
 * necessary.
 */
void printPrompt(String *const cwd);

/**
 * @brief Reads entire user input line from stdin and returns a cleaned version
 * of the user's input. If EOF (Ctrl+D) is received, userInput is set to the
 * exit command. Otherwise, userInput is set to the stripped version
 * of user input line, removing any leading and lagging characters that are
 * contains in stripChars. Sets errno upon error.
 *
 * @param userInput out-parameter containing a cleaned version of the user's
 * input
 * @param stripChars String of chars to strip from userInput. A default string
 * will be used if NULL is provided
 * @return true Successfully read and cleaned user input.
 * @return false Error occurred; failed operations.
 */
bool readInputLine(String *const userInput, const String *stripChars);
