/**
 * @file vector.c
 * @author Justen Di Ruscio - (3624673)
 * @brief Provides definitions for basic utilities to operate on arbitrary
 * length, geometrically resized arrays (Vectors)
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "vector_private.h"
#include <jd/error.h>
#include <jd/string.h>
#include <jd/vector.h>

#include <stdio.h>  // fprintf
#include <stdlib.h> // malloc, realloc
#include <string.h> // memcpy

Vector vector_constructEmpty(const size_t dataSize) {
  Vector vec = {.data = NULL,
                .length = 0,
                .capacity = 0,
                .elementDeleter = NULL,
                .dataSize = dataSize};
  return vec;
}

OptionalVector vector_constructCapacity(const size_t capacity,
                                        const size_t dataSize) {
  const char fooName[] = "vector_constructCapacity";
  Vector vec = {.data = (void *)NULL,
                .length = 0,
                .capacity = capacity,
                .dataSize = dataSize};
  OptionalVector result = {.data = vec, .valid = false};

  // Argument Validity Checks
  errno = 0;
  if (dataSize == 0) {
    fprintf(stderr, "field 'dataSize' of %s must be greater than 0\n", fooName);
    errno = EPERM;
    return result;
  }
  if (capacity == 0) {
    result.data = vec;
    fprintf(stderr, "field 'capacity' of %s must be greather than 0\n",
            fooName);
    errno = EPERM;
    return result;
  }

  // Construction
  void *const newVectorBuff = malloc(capacity * dataSize);
  if (newVectorBuff == (void *)NULL) {
    fprintf(stderr,
            "failure allocating memory for vector contents in "
            "%s\n",
            fooName);
    return result; // errno set by malloc
  }
  vec.data = newVectorBuff;
  result.valid = true;
  result.data = vec;
  return result;
}

OptionalVector vector_copyConstruct(const Vector *const other) {
  const char fooName[] = "vector_copyConstruct";
  OptionalVector result = {.valid = false};

  // Argument Validity Check
  errno = 0;
  if (other == (Vector *)NULL) {
    fprintf(stderr,
            "field 'other' of %s must point to a valid "
            "address\n",
            fooName);
    errno = EPERM;
    return result;
  }

  // Copy Construct
  Vector vec = {.data = (void *)NULL,
                .length = other->length,
                .capacity = other->capacity,
                .dataSize = other->dataSize};
  void *const newVectorBuff = malloc(vec.capacity * vec.dataSize);
  if (newVectorBuff == (void *)NULL) {
    fprintf(stderr,
            "failure allocating memory for vector contents in "
            "%s\n",
            fooName);
    return result; // errno set by malloc
  }
  vec.data = newVectorBuff;
  memcpy(vec.data, other->data, vec.length * vec.dataSize);

  result.valid = true;
  result.data = vec;
  return result;
}

void vector_freeData(const Vector *const vec) {
  // Return if there's nothing to free
  if (vec == (Vector *)NULL) {
    return;
  }
  // Free data member
  free(vec->data);
}

void vector_freeDataVoid(const void *const vec) {
  // Return if there's nothing to free
  if (vec == (void *)NULL) {
    return;
  }
  // Free data member
  Vector *v = (Vector *)vec;
  vector_freeData(v);
}

void vector_freeElements(const Vector *const vec) {
  // Return if there's nothing to free
  if (vec == (Vector *)NULL || vec->data == (void *)NULL) {
    return;
  }

  for (size_t elementIdx = 0; elementIdx < vec->length; ++elementIdx) {
    const void *const element = vector_at(vec, elementIdx);
    vec->elementDeleter(element);
  }
}

void *vector_at(const Vector *const vec, const size_t index) {
  const char fooName[] = "vector_at";

  // Argument Validity Check
  errno = 0;
  if (vec == (Vector *)NULL) {
    fprintf(stderr, "field 'vec' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return (void *)NULL;
  }
  if (vec->length <= index) {
    fprintf(stderr,
            "field 'index' of value %zu in %s is out of bounds of provided "
            "vector of length %zu\n",
            index, fooName, vec->length);
    errno = ENOENT;
    return (void *)NULL;
  }
  // Element Access
  const char *const dataBytes = (char *)vec->data;
  return (void *)(dataBytes + index * vec->dataSize);
}

void *vector_back(const Vector *const vec) {
  const char fooName[] = "vector_back";

  // Argument Validity Check
  errno = 0;
  if (vec == (Vector *)NULL) {
    fprintf(stderr, "field 'vec' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return (void *)NULL;
  }

  // Element Access
  if (vec->length == 0) {
    errno = EPERM;
    return (void *)NULL;
  }
  return vector_at(vec, vec->length - 1);
}

bool vector_pushBack(Vector *const vec, const void *const element) {
  const char fooName[] = "vector_pushBack";

  // Argument Validity Checks
  errno = 0;
  if (vec == (Vector *)NULL) {
    fprintf(stderr, "field 'vec' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (element == (void *)NULL) {
    fprintf(stderr, "field 'element' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Push Back
  const size_t oldLength = vec->length;
  const bool reserved = vector_reserve(vec, ++vec->length);
  return reserved && vector_assignElement(vec, oldLength, element);
}

bool vector_pushBackAll(Vector *const vec, const size_t numElements,
                        const void *const source) {
  const char fooName[] = "vector_pushBackAll";

  // Argument Validity Checks
  errno = 0;
  if (vec == (Vector *)NULL) {
    fprintf(stderr, "field 'vec' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (source == (void *)NULL) {
    fprintf(stderr, "field 'source' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Push Back
  const size_t oldLength = vec->length;
  vec->length += numElements;
  const bool reserved = vector_reserve(vec, vec->length);
  return reserved && vector_assignElements(vec, oldLength, numElements, source);
}

OptionalVector vector_append(const Vector *const vec1,
                             const Vector *const vec2) {
  const char fooName[] = "vector_append";
  OptionalVector result = {.data = vector_constructEmpty(0), .valid = false};

  // Argument Validity Checks
  errno = 0;
  if (vec1 == (Vector *)NULL) {
    fprintf(stderr, "argument 'vec1' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return result;
  }
  if (vec2 == (Vector *)NULL) {
    fprintf(stderr, "argument 'vec2' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return result;
  }
  if (vec1->dataSize != vec2->dataSize) {
    fprintf(stderr,
            "vectors provided to %s must contain elements of equal size\n",
            fooName);
    errno = EPERM;
    return result;
  }

  // Append Vectors
  Vector both = vector_constructEmpty(vec1->dataSize);
  const bool reserved = vector_reserve(&both, vec1->length + vec2->length);
  if (!reserved) {
    fprintf(stderr, "Failed to reserve vector of %zu elements in %s\n",
            vec1->length + vec2->length, fooName);
    return result; // errno set by vector_reserve
  }

  bool assigned;
  if (vec1->data != NULL) {
    both.length += vec1->length;
    assigned = vector_assignElements(&both, 0, vec1->length, vec1->data);
    if (!assigned) {
      fprintf(stderr, "Failed to assign elements of vec1 in %s\n", fooName);
      return result; // errno set by vector_assignElements
    }
  }
  if (vec2->data != NULL && vec2->length > 0) {
    both.length += vec2->length;
    assigned =
        vector_assignElements(&both, vec1->length, vec2->length, vec2->data);
    if (!assigned) {
      fprintf(stderr, "Failed to assign elements of vec2 in %s\n", fooName);
      return result; // errno set by vector_assignElements
    }
  }

  result.data = both;
  result.valid = true;
  return result;
}

bool vector_reserve(Vector *const vec, const size_t newSize) {
  const char fooName[] = "vector_reserve";

  // Argument Validity Check
  errno = 0;
  if (vec == (Vector *)NULL) {
    fprintf(stderr, "field 'vec' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Reserve
  if (newSize > vec->capacity) { // require resize
    const size_t newCapacity = 2 * newSize;
    void *newVectorBuffer = realloc(vec->data, newCapacity * vec->dataSize);
    if (newVectorBuffer == (void *)NULL) { // realloc failed
      fprintf(stderr, "resizing vector buffer with realloc failed in %s\n",
              fooName);
      return false; // realloc sets errno
    }
    vec->data = newVectorBuffer;
    vec->capacity = newCapacity;
  }
  return true;
}

bool vector_clear(Vector *const vec) {
  const char fooName[] = "vector_clear";

  // Argument Validity Check
  errno = 0;
  if (vec == (Vector *)NULL) {
    fprintf(stderr, "field 'vec' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Clear Contents
  vec->length = 0;
  return true;
}

bool vector_assignElement(Vector *const vec, const size_t destination,
                          const void *source) {
  const char fooName[] = "vector_assignElement";

  // Argument Validity Checks
  errno = 0;
  if (vec == (Vector *)NULL) {
    fprintf(stderr, "field 'vec' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (source == (void *)NULL) {
    fprintf(stderr,
            "field 'source' of %s must point to a "
            "valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  // check for overlaping data
  void *const destinationLocation = vector_at(vec, destination);
  if (errno != 0) {
    fprintf(stderr, "call to vector_at in %s failed. errno = %i: %s\n", fooName,
            errno, strerror(errno));
    return false;
  }
  if (memoryOverlaps(source, destinationLocation, vec->dataSize)) {
    fprintf(stderr,
            "data referenced by fields 'destination' and 'source' of "
            "%s must not overlap\n",
            fooName);
    return false;
  }

  // Assign Element
  return vector_assignElements(vec, destination, 1, source);
}

bool vector_assign(Vector *const destination, const Vector *const source) {
  const char fooName[] = "vector_assign";

  // Argument Validity Checks
  errno = 0;
  if (destination == (Vector *)NULL) {
    fprintf(stderr, "field 'destination' of %s must point to a valid vector\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (source == (Vector *)NULL) {
    fprintf(stderr, "field 'source' of %s must point to a valid vector\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Assignment
  destination->length = source->length;
  const bool reserved = vector_reserve(destination, source->length);
  if (!reserved) {
    fprintf(stderr, "Unable to reserve memory for destination in %s\n",
            fooName);
    return false; // vector_reserve sets errno
  }
  const bool assigned =
      vector_assignElements(destination, 0, source->length, source->data);
  if (!assigned) {
    fprintf(stderr, "Failure assigning contents in %s\n", fooName);
    return false;
  }

  return true;
}

static bool vector_assignElements(Vector *const vec, const size_t destination,
                                  const size_t numElements,
                                  const void *source) {
  const char fooName[] = "vector_assignElements";

  // Argument Validity Checks
  if (vec == (Vector *)NULL) {
    fprintf(stderr, "field 'vec' of %s must point to a valid address\n",
            fooName);
    return false;
  }
  if (source == (void *)NULL) {
    fprintf(stderr,
            "field 'source' of %s must point to a "
            "valid address\n",
            fooName);
    return false;
  }
  // check for overlaping data
  const size_t numBytes = vec->dataSize * numElements;
  void *const destinationLocation = vector_at(vec, destination);
  if (errno != 0) {
    fprintf(stderr, "call to vector_at in %s failed\n", fooName);
    handleErrorMsg(errno);
    return false;
  }
  if (memoryOverlaps(source, destinationLocation, numBytes)) {
    fprintf(stderr,
            "data referenced by fields 'destination' and 'source' of "
            "%s must not overlap\n",
            fooName);
    return false;
  }

  // Data Assignment
  memcpy(destinationLocation, source, numBytes);
  return true;
}

bool vector_erase(Vector *const vec, const size_t index) {
  const char fooName[] = "vector_erase";

  // Argument Validity Checks
  errno = 0;
  if (vec == (Vector *)NULL) {
    fprintf(stderr, "field 'vec' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (index >= vec->length) {
    fprintf(stderr,
            "field 'index' of %s is out of bounds of the provided vector\n",
            fooName);
    errno = ERANGE;
  }

  // Erase Element
  // create new vector to store elements
  OptionalVector newOpt =
      vector_constructCapacity(vec->capacity, vec->dataSize);
  if (!newOpt.valid) {
    fprintf(stderr,
            "Unable to construct vector in %s to store retained elements\n",
            fooName);
  }
  Vector newVec = newOpt.data;
  newVec.length = vec->length;

  // copy elements up to erasure
  bool assigned = vector_assignElements(&newVec, 0, index, vec->data);
  if (!assigned) {
    fprintf(stderr, "Failure copying first elements in %s\n", fooName);
    return false; // errno set by vector_assignElements
  }

  // copy elements after erasure
  if (vec->length > index + 1) {
    void *source = vector_at(vec, index + 1);
    assigned =
        vector_assignElements(&newVec, index, vec->length - index - 1, source);
    if (!assigned) {
      fprintf(stderr, "Failure copying last elements in %s\n", fooName);
      return false; // errno set by vector_assignElements
    }
  }

  // free old contents
  vector_freeData(vec);

  --newVec.length;
  *vec = newVec;
  return true;
}

ssize_t vector_find(const Vector *const vec, const void *const value) {
  const char fooName[] = "vector_find";
  ssize_t foundIndex = -1;

  // Argument Validity Checks
  errno = 0;
  if (vec == (Vector *)NULL) {
    fprintf(stderr, "argument 'vec' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return foundIndex;
  }
  if (value == (void *)NULL) {
    fprintf(stderr, "argument 'value' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return foundIndex;
  }

  // Find element
  for (unsigned i = 0; i < vec->length; ++i) {
    const void *const vecElem = vector_at(vec, i);
    if (memcmp(vecElem, value, vec->dataSize) == 0) {
      foundIndex = i;
      break;
    }
  }
  return foundIndex;
}
