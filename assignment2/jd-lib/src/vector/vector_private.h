#pragma once
/**
 * @file vector_private.h
 * @author Justen Di Ruscio (3624673)
 * @brief Contains symbols for functions pertaining to Vectors which aren't
 * necessary as part of their public interface.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/vector.h>

/**
 * @brief Assigns the value of numElements elements in vec starting at
 * destination to the values of the contiguous elements pointed to by source.
 * Assigns errno upon error.
 *
 * @param vec Vector to assign elements in
 * @param destination index of first element to assign in vec
 * @param numElements number of elements pointed to by source and to assign.
 * @param source contiguous elements whose contents will be copied into elements
 * of vec
 * @return true successfully assigned elements
 * @return false failed to assign elements
 */
static bool vector_assignElements(Vector* const vec, const size_t destination,
                                  const size_t numElements, const void* source);
