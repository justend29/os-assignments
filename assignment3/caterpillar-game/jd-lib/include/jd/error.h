#pragma once
/**
 * @file error.h
 * @author Justen Di Ruscio (3624673)
 * @brief Contains symbols for common error handling/checking functions
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <errno.h>
#include <stdbool.h>

#define ECMDNOTFOUND 127

/**
 * @brief Prints error number and associated Unix error message, from strerror,
 * or a message indicating errorNumber is unknown to strerror
 *
 * @param errorNumber Unix error number
 */
void handleErrorMsg(const int errorNumber);

/**
 * @brief exits the program with provided error number after printing error
 * number and associated Unix error message, from strerror, or a message
 * indicating errorNumber is unknown to strerror
 *
 * @param errorNumber Unix error number
 */
void handleExitError(const int errorNumber);

/**
 * @brief Indicates whether the memory pointed to by first and second overlap or
 * not. Assumes pointed to data is of equal length.
 *
 * @param first pointer to first chunk of memory
 * @param second pointer to second chunk of memory
 * @param dataSize size, in bytes, of both chunks of memory
 * @return true memory overlaps
 * @return false memory doesn't overlap
 */
bool memoryOverlaps(const void* const first, const void* const second,
                    const unsigned dataSize);
