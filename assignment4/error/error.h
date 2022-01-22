#pragma once
/**
 * @file error.h
 * @author Justen Di Ruscio
 * @brief Contains declarations related to error handling or printing
 * @version 0.1
 * @date 2021-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */

/**
 * @brief Checks if arg is NULL. If it is, an error message will be printed as if it came from fooName and errno will be set. Otherwise, errno is 0
 *
 * @param arg argument to check for NULL
 * @param argName name of arg for error message
 * @param fooName name of function for error message
 */
void argValidityCheck(const void * const arg, const char *const argName,
                      const char *const fooName);
