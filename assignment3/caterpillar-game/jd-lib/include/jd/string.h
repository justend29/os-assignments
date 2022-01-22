#pragma once
/**
 * @file string.h
 * @author Justen Di Ruscio (3624673)
 * @brief Symbols for arbitrary length, null terminated strings and their
 * manipulation. These Strings are not small-string-optimized.
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/error.h>
#include <jd/vector.h>
#include <string.h>

/**
 * @brief Representation of a string
 *
 */
typedef struct String {
  char* data;
  size_t length;    // in bytes
  size_t capacity;  // memory including null termination
} String;

/**
 * @brief Contains a string and a flag to indicate its validity. Used to return
 * a string and possibly an error from functions.
 *
 */
typedef struct OptionalString {
  String data;
  bool valid;
} OptionalString;

/**
 * @brief Constructs and empty string by initializing the String struct
 *
 * @return String constructed, empty String
 */
String string_constructEmpty();

/**
 * @brief Constructs a String with an initial capacity. Sets errno upon error.
 *
 * @param capacity number of chars to reserve in String's initial capacity
 * @return OptionalString constructed String and flag to indicate validity of
 * the String
 */
OptionalString string_constructCapacity(const size_t capacity);

/**
 * @brief Constructs a String whose contents are copied from the other String.
 * Sets errno upon error.
 *
 * @param other String to copy contents of
 * @return OptionalString constructed string and flag to indicate validity of
 * the String
 */
OptionalString string_copyConstruct(const String* const other);

/**
 * @brief Constructs a String whose contents are copied from the null terminated
 * bytes pointed to by other. Sets errno upon error.
 *
 * @param other c-style string (null-terminated array) to copy contents of
 * @return OptionalString constructed string and flag to indicate validity of
 * the String
 */
OptionalString string_copyConstructChar(const char* const other);

/**
 * @brief Frees the contents of the provided String. Does nothing if the
 * provided String or its contents is NULL.
 *
 * @param str String to free contents of
 */
void string_freeData(const String* const str);

/**
 * @brief Calls string_freeData after casting str to a String. Provides a
 * uniform interface for freeing String data.
 *
 * @param str String to free contents of
 */
void string_freeDataVoid(const void* const str);

/**
 * @brief Reserves enough memory in str to contain newCapacity chars. Does
 * nothing if str is already large enough. Sets errno upon error.
 *
 * @param newCapacity number of chars str's buffer should hold after reserve
 * @return true successfully reserved memory
 * @return false failed to reserve memory
 */
bool string_reserve(String* const str, const size_t newCapacity);

/**
 * @brief Splits the provided string by any of the delimeters pointed to by
 * delim into a Vector of Strings. Resulting Strings own their memory; they
 * don't point to contents of str. Sets errno upon error.
 *
 * @param str String to split
 * @param delim null-terminated list of chars used as delimeters to split str
 * @return OptionalVector Vector of split Strings and a flag do indicate the
 * validity of the operations.
 */
OptionalVector string_split(const String* const str, const char* const delim);

/**
 * @brief Compares contents of two strings with strcmp
 *
 * @param first String
 * @param second String
 * @return int result of strcmp
 */
int string_compare(const String* const first, const String* const second);

/**
 * @brief Compares contents of first String with null-terminated c-style
 * string, second using strcmp.
 *
 * @param first String
 * @param second null-terminated c-style string
 * @return int result of strcmp
 */
int string_compareChar(const String* const first, const char* const second);

/**
 * @brief Indicates if the provided String's contents contains the provided
 * char. Sets errno upon error.
 *
 * @param string String to find char in
 * @param element char to find in string
 * @return true element is present in string
 * @return false element is not present in string
 */
bool string_contains(const String* const string, const char element);

/**
 * @brief Strips any of the chars in delimeters from the start and end of
 * string. Leaves string unmodified. Sets errno upon error
 *
 * @param string String to strip characters from
 * @param delimeters String containing chars to strip from string
 * @return OptionalString stripped string and flag to indicate the validity of
 * the operations.
 */
OptionalString string_strip(const String* const string,
                            const String* const delimeters);
