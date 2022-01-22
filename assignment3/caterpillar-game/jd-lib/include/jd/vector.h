#pragma once
/**
 * @file vector.h
 * @author Justen Di Ruscio - (3624673)
 * @brief Provides symbols for basic utilities to operate on arbitrary length,
 * geometrically resized arrays (Vectors)
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdbool.h>    // bool, true, false
#include <stddef.h>     // size_t
#include <sys/types.h>  // ssize_t

/**
 * @brief Represents a vector
 *
 */
typedef struct Vector {
  void* data;
  size_t length;    // num. elements
  size_t capacity;  // num. possible elements
  size_t dataSize;  // size of each element (bytes)
  void (*elementDeleter)(
      const void* const element);  // function to free element
} Vector;

/**
 * @brief Used to return a vector and a flag representing if the vector is valid
 * from functions that return a vector and a possible error.
 *
 */
typedef struct OptionalVector {
  Vector data;
  bool valid;
} OptionalVector;

/**
 * @brief Creates a completely empty vector by initializing the Vector struct
 *
 * @param dataSize size of elements contained by the vector to construct
 * @return Vector constructed, empty Vector
 */
Vector vector_constructEmpty(const size_t dataSize);

/**
 * @brief Creates a vector with an initial capacity, allocated on the heap. Sets
 * errno if result is invalid
 *
 * @param capacity amount of memory to initially allocate in number of elements
 * @param dataSize size of elements contained by the vector to construct
 * @return OptionalVector contains the constructed vector and a flag to indicate
 * its validity
 */
OptionalVector vector_constructCapacity(const size_t capacity,
                                        const size_t dataSize);

/**
 * @brief Creates a new vector with contents allocated on the heap and with the
 * same elements as other. Sets errno if result is invalid
 *
 * @param other another vector to copy elements from. Left unmodified
 * @return OptionalVector contains the constructed vector and a flag to indicate
 * its validity
 */
OptionalVector vector_copyConstruct(const Vector* const other);

/**
 * @brief Frees the contents of the provided vector. Returns NULL and performs
 * no action if vec is NULL or vec->data is NULL
 *
 * @param vec Vector who's contents were allocated on the heap
 */
void vector_freeData(const Vector* const vec);

/**
 * @brief Calls vector_freeData after casting vec to a Vector*. Used to provide
 * a generic interface.
 *
 * @param vec a pointer pointing to a Vector
 */
void vector_freeDataVoid(const void* const vec);

/**
 * @brief Applies vector->elementDeleter to each element in the Vector's data
 * contents. Used if the elements are/contain heap allocated data
 *
 * @param vec Vector to free elements of
 */
void vector_freeElements(const Vector* const vec);

/**
 * @brief Returns a pointer to the element at index or NULL if an error
 * occurred. errno set upon error, like under an out of bounds condition
 *
 * @param vec Vector to access element of
 * @param index index of element in vec to access
 * @return void* pointer to contained element
 */
void* vector_at(const Vector* const vec, const size_t index);

/**
 * @brief Accesses the last element of vec by calling vector_at. Sets errno on
 * error and returns NULL, like if the vector is empty.
 *
 * @param vec Vector to access last element of
 * @return void*
 */
void* vector_back(const Vector* const vec);

/**
 * @brief Assigns the value of the vec's element at index destination to the
 * value pointed to by source. Copies dereferenced data, not source itself. Sets
 * errno upon error and returns false to indicate error.
 *
 * @param vec Vector to assign element of
 * @param destination index of element in vec to assign
 * @param source pointer to data to assign to element at destination
 * @return true successfully assigned element
 * @return false failed to assign element
 */
bool vector_assignElement(Vector* const vec, const size_t destination,
                          const void* source);

/**
 * @brief Copy assigns contents of vector source to destination. Sets errno upon
 * error and reserves memory in destination if its capacity is too small
 *
 * @param destination Vector with contents to copy
 * @param source Vector who's contents will be assigned
 * @return true successfully assigned contents
 * @return false failed to assign contents
 */
bool vector_assign(Vector* const destination, const Vector* const source);

/**
 * @brief Adds another element to the end of vec with the contents pointed to by
 * element. Sets errno upon error.
 *
 * @param vec Vector to extend
 * @param element pointer to data to copy into added element
 * @return true successfully pushed new element onto vec
 * @return false failed to push new element onto vec
 */
bool vector_pushBack(Vector* const vec, const void* const element);

/**
 * @brief Adds all the contiguous elements pointed to by elements as new
 * elements on the end of vec. Sets errno upon error.
 *
 * @param vec Vector to extend
 * @param numElements number of elements pointed to by elements and to extend
 * vec by
 * @param elements contiguous array of elements to copy to vec
 * @return true successfully pushed new elements onto vec
 * @return false failed to push new elements onto vec
 */
bool vector_pushBackAll(Vector* const vec, const size_t numElements,
                        const void* const elements);

/**
 * @brief Constructs a new vector whose contents is a concatenation of the
 * contents of vec1 and vec2. Sets errno on error. Appends nothing if either
 * provided vector has no data contents.
 *
 * @param vec1 first Vector
 * @param vec2 second Vector, appended to vec1
 * @return OptionalVector contains concatenated vector and flag to indicate
 * validity of result.
 */
OptionalVector vector_append(const Vector* const vec1,
                             const Vector* const vec2);

/**
 * @brief Reserves enough capacity in vec to contain newSize number of elements.
 * Does nothing if vec is already large enough. Sets errno upon error.
 *
 * @param vec Vector upon which data is reserved
 * @param newSize number of elements vec should hold after reservation
 * @return true successfully reserved memory
 * @return false failed to reserve memory
 */
bool vector_reserve(Vector* const vec, const size_t newSize);

/**
 * @brief Clears contents of provided vector. Doesn't free any memory, only
 * resizes it to 0. Sets errno upon error.
 *
 * @param vec Vector to clear
 * @return true successfully cleared vector
 * @return false failed to clear provided vector
 */
bool vector_clear(Vector* const vec);

/**
 * @brief Erases the element of vec at index by copying other elements to a new
 * Vector. Frees original array but doesn't free elements if they are heap
 * allocated. Sets errno upon error.
 *
 * @param vec Vector to erase element of
 * @param index index of element in vec to erase
 * @return true successfully erased element
 * @return false failed to erase element
 */
bool vector_erase(Vector* const vec, const size_t index);

/**
 * @brief Returns index of element in vec with same data as that pointed to by
 * value. Compares dereferenced data, not addresses. Returns -1 upon failure or
 * error, where errno is set upon error.
 *
 * @param vec Vector to find element in
 * @param value value of element to find
 * @return ssize_t index of found element or -1 upon failure
 */
ssize_t vector_find(const Vector* const vec, const void* const value);
