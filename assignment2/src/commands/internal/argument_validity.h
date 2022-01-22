#pragma once
/**
 * @file argument_validity.h
 * @author Justen Di Ruscio (3624673)
 * @brief Contains symbols for functions related to argument validity checks
 * for internal commands. All internal commands have the same interface - this
 * allows the same argument validity checks among all executor functions.
 * @version 0.1
 * @date 2021-02-17
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/vector.h>

/**
 * @brief Used to check validity of arguments passed to internal command
 * executors. Sets errno upon error.
 *
 * @param commandName
 * @param commandArgs
 * @param fooName
 * @return int
 */
int argumentValidityCheck(const char *const commandName,
                          const Vector *const commandArgs,
                          const char *const fooName);
