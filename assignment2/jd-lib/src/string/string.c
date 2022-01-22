/**
 * @file string.c
 * @author Justen Di Ruscio (3624673)
 * @brief Definitions for arbitrary length, null terminated strings and their
 * manipulation. These Strings are not small-string-optimized.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#define _POSIX_C_SOURCE 200809L

#include <jd/string.h>

#include <stdio.h>  // fprintf
#include <stdlib.h> // malloc

String string_constructEmpty() {
  String result = {.data = (char *)NULL, .length = 0, .capacity = 0};
  return result;
}

OptionalString string_constructCapacity(const size_t capacity) {
  const char fooName[] = "string_constructCapacity";
  String string = string_constructEmpty();
  OptionalString result = {.data = string, .valid = false};

  if (capacity == 0) {
    result.data = string;
    return result;
  }

  void *const newStringBuff = malloc(capacity);
  if (newStringBuff == (void *)NULL) {
    fprintf(stderr, "Error allocating memory for string in %s\n", fooName);
    return result; // errno set by malloc
  }

  string.capacity = capacity;
  string.data = (char *)newStringBuff;
  result.data = string;
  result.valid = true;
  return result;
}

OptionalString string_copyConstruct(const String *const other) {
  const char fooName[] = "string_copyConstruct";
  String string = string_constructEmpty();
  OptionalString result = {.data = string, .valid = false};

  // Argument Validity Check
  errno = 0;
  if (other == (String *)NULL) {
    fprintf(stderr,
            "field 'other' of %s must point to a "
            "valid address\n",
            fooName);
    errno = EPERM;
    return result;
  }

  // Copy Construct
  string.length = other->length;
  string.capacity = other->capacity;
  void *const newStringBuff = malloc(string.capacity);
  if (newStringBuff == (void *)NULL) {
    fprintf(stderr, "Unable to allocate memory for string in %s\n", fooName);
    return result; // errno set by malloc
  }
  string.data = (char *)newStringBuff;
  strcpy(string.data, other->data);
  string.data[string.length] = '\0';
  result.valid = true;
  result.data = string;
  return result;
}

OptionalString string_copyConstructChar(const char *const other) {
  const char fooName[] = "string_copyConstructChar";
  String string = string_constructEmpty();
  OptionalString result = {.data = string, .valid = false};

  // Argument Validity Check
  errno = 0;
  if (other == (char *)NULL) {
    fprintf(stderr,
            "field 'other' of %s must point to a "
            "valid address\n",
            fooName);
    errno = EPERM;
    return result;
  }

  // Copy Construct
  string.length = strlen(other);
  string.capacity = string.length * 1.5 + 1;
  void *const newStringBuff = malloc(string.capacity);
  if (newStringBuff == (void *)NULL) {
    fprintf(stderr, "Unable to allocate memory for string in %s\n", fooName);
    return result; // errno set by malloc
  }
  string.data = (char *)newStringBuff;
  strcpy(string.data, other);
  string.data[string.length] = '\0';
  result.valid = true;
  result.data = string;
  return result;
}

void string_freeData(const String *const str) {
  // Return if there's nothing to free
  if (str == (String *)NULL) {
    return;
  }
  // Free data member
  free(str->data);
}

void string_freeDataVoid(const void *const str) {
  // Return if there's nothing to free
  if (str == (void *)NULL) {
    return;
  }
  // Free data member
  const String *const string = (String *)str;
  string_freeData(string);
}

bool string_reserve(String *const str, const size_t newCapacity) {
  const char fooName[] = "string_reserve";

  // Argument Validity Checks
  errno = 0;
  if (str == (String *)NULL) {
    fprintf(stderr,
            "field 'str' of %s must point to a "
            "valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Reserve
  if (newCapacity > str->capacity) {
    const void *const newBuffer = realloc(str->data, newCapacity);
    if (newBuffer == (void *)NULL) {
      fprintf(stderr, "Error reserving string buffer with realloc in %s\n",
              fooName);
      return false; // realloc sets errno
    }
    str->data = (char *)newBuffer;
    str->capacity = newCapacity;
  }
  return true;
}

OptionalVector string_split(const String *const str, const char *const delim) {
  const char fooName[] = "string_split";
  OptionalVector result = {.data = {.data = NULL}, .valid = false};

  // Argument Validity Check
  errno = 0;
  if (str == (String *)NULL) {
    fprintf(stderr,
            "field 'str' of %s must point to a "
            "valid address\n",
            fooName);
    errno = EPERM;
    return result;
  }
  if (delim == (char *)NULL) {
    fprintf(stderr,
            "field 'delim' of %s must point to a "
            "valid address\n",
            fooName);
    errno = EPERM;
    return result;
  }

  // Split String into Parts

  // create vector to store split string pieces
  const unsigned initialCapacity = 8;
  OptionalVector v = vector_constructCapacity(initialCapacity, sizeof(String));
  if (!v.valid) {
    fprintf(stderr,
            "Unable to construct vector for string pieces with default "
            "capacity of %u in %s\n",
            initialCapacity, fooName);
    return result; // errno set by vector_constructCapacity
  }
  Vector splitStrings = v.data;
  splitStrings.elementDeleter = string_freeDataVoid;

  // tokenize string into pieces, pushing into vector
  char *stringChars = str->data;
  char *savePtr = (char *)NULL;
  while (true) {
    const char *const piece = strtok_r(stringChars, delim, &savePtr);
    if (piece == (char *)NULL) { // no more tokens
      break;
    }
    OptionalString stringPiece = string_copyConstructChar(piece);
    if (!stringPiece.valid) {
      fprintf(stderr, "Unable to construct string from token in %s\n", fooName);
      return result; // errno set by string_copyConstructChar
    }
    const bool pushed = vector_pushBack(&splitStrings, &stringPiece.data);
    if (!pushed) {
      fprintf(stderr,
              "Error pushing string token %s into vector of tokens in %s\n",
              stringPiece.data.data, fooName);
    }
    stringChars = (char *)NULL;
  }

  // return container holding split strings
  result.valid = true;
  result.data = splitStrings;
  return result;
}

int string_compare(const String *const first, const String *const second) {
  return strcmp(first->data, second->data);
}

int string_compareChar(const String *const first, const char *const second) {
  return strcmp(first->data, second);
}

bool string_contains(const String *const string, const char element) {
  const char fooName[] = "string_contains";
  errno = 0;

  // Argument Validity Check
  if (string == (String *)NULL) {
    fprintf(stderr, "argument 'string' of %s must point to a valid string\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Contains
  bool result = false;
  for (size_t listIdx = 0; listIdx < string->length; ++listIdx) {
    if (string->data[listIdx] == element) {
      result = true;
      break;
    }
  }
  return result;
}

OptionalString string_strip(const String *const string,
                            const String *const delimeters) {
  const char fooName[] = "string_strip";
  String stripped = string_constructEmpty();
  OptionalString result = {.data = stripped, .valid = false};

  // Argument Validity Checks
  errno = 0;
  if (string == (String *)NULL) {
    fprintf(stderr, "argument 'string' of %s must point to a valid string\n",
            fooName);
    errno = EPERM;
    return result;
  }
  if (delimeters == (String *)NULL) {
    fprintf(stderr,
            "argument 'delimeters' of %s must point to a valid string\n",
            fooName);
    errno = EPERM;
    return result;
  }

  // Strip
  // create iterators on string
  const char *start = string->data;
  const char *last = string->data + string->length - 1;
  // strip from string
  while (start <= last) {
    bool moved = false;
    if (string_contains(delimeters, *start)) {
      ++start;
      moved = true;
    }
    if (string_contains(delimeters, *last)) {
      --last;
      moved = true;
    }
    if (!moved) {
      break;
    }
  }
  // allocate stripped string
  stripped.length = start <= last ? 1 + last - start : 0;
  stripped.capacity = 2 * stripped.length + 1;
  const void *const strippedBuffer = malloc(stripped.capacity);
  if (strippedBuffer == (void *)NULL) {
    return result; // errno set by malloc
  }
  stripped.data = (char *)strippedBuffer;

  // copy contents to new string
  memcpy(stripped.data, start, stripped.length);
  *(stripped.data + stripped.length) = '\0';

  result.data = stripped;
  result.valid = true;
  return result;
}
