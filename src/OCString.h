/**
 * @file OCString.h
 * @brief Immutable and mutable OCString reference types and operations.
 *
 * Provides creation, manipulation, comparison, and conversion functions
 * for OCTypes string objects, including support for complex arithmetic parsing.
 */
#ifndef OCString_h
#define OCString_h

// System & OCTypes headers
#include <complex.h>
#include "OCLibrary.h" // Ensures OCStringRef and other types are available

/** @defgroup OCString OCString
 *  @brief Immutable and mutable OCString operations.
 *  @{
 */

/**
 * @brief Macro to create a constant OCStringRef from a C string literal.
 * @param cStr C string literal.
 * @return A compile-time constant OCStringRef; do not release.
 * @ingroup OCString
 */
#define STR(cStr)  __OCStringMakeConstantString("" cStr "")

/**
 * @brief Returns the unique type identifier for OCString objects.
 * @return The OCTypeID for OCString.
 * @ingroup OCString
 */
OCTypeID OCStringGetTypeID(void);

/**
 * @brief Creates an immutable OCString from a C string.
 * @param string Null-terminated UTF-8 C string.
 * @return New OCStringRef, or NULL on failure.
 * @ingroup OCString
 */
OCStringRef OCStringCreateWithCString(const char *string);

/**
 * @brief Creates a mutable copy of an immutable OCString.
 * @param theString Immutable OCString to copy.
 * @return New OCMutableStringRef (ownership transferred to caller).
 * @ingroup OCString
 */
OCMutableStringRef OCStringCreateMutableCopy(OCStringRef theString);

/**
 * @brief Creates an immutable OCString from a substring.
 * @param str Source OCString.
 * @param range Range of substring to extract.
 * @return New OCStringRef, or NULL on failure.
 * @ingroup OCString
 */
OCStringRef OCStringCreateWithSubstring(OCStringRef str, OCRange range);

/**
 * @brief Appends an immutable OCString to a mutable OCString.
 * @param theString Mutable OCString to append to.
 * @param appendedString Immutable OCString to append.
 * @ingroup OCString
 */
void OCStringAppend(OCMutableStringRef theString, OCStringRef appendedString);

/**
 * @brief Appends a C string to a mutable OCString.
 * @param theString Mutable OCString to append to.
 * @param cString Null-terminated UTF-8 C string to append.
 * @ingroup OCString
 */
void OCStringAppendCString(OCMutableStringRef theString, const char *cString);

/**
 * @brief Creates a new mutable OCString with specified initial capacity.
 * @param capacity Initial capacity for the mutable string.
 * @return New OCMutableStringRef (ownership transferred to caller).
 * @ingroup OCString
 */
OCMutableStringRef OCStringCreateMutable(uint64_t capacity);

/**
 * @brief Creates a mutable OCString from a null-terminated C string.
 * @param cString Null-terminated UTF-8 C string.
 * @return New OCMutableStringRef (ownership transferred to caller).
 * @ingroup OCString
 */
OCMutableStringRef OCMutableStringCreateWithCString(const char *cString);

/**
 * @brief Returns a C string representation of an immutable OCString.
 * @param theString Immutable OCString.
 * @return Null-terminated UTF-8 C string.
 * @ingroup OCString
 */
const char *OCStringGetCString(OCStringRef theString);

/**
 * @brief Creates a new immutable copy of an OCString.
 * @param theString Source OCString.
 * @return New OCStringRef (ownership transferred to caller).
 * @ingroup OCString
 */
OCStringRef OCStringCreateCopy(OCStringRef theString);

/**
 * @brief Compares two OCStrings with specified options.
 * @param theString1 First OCString.
 * @param theString2 Second OCString.
 * @param compareOptions Comparison options flags.
 * @return Comparison result indicating order or equality.
 * @ingroup OCString
 */
OCComparisonResult OCStringCompare(OCStringRef theString1, OCStringRef theString2, OCStringCompareFlags compareOptions);

/**
 * @brief Returns the length of an OCString.
 * @param theString OCString.
 * @return Length of the string.
 * @ingroup OCString
 */
uint64_t OCStringGetLength(OCStringRef theString);

/**
 * @brief Converts a mutable OCString to lowercase.
 * @param theString Mutable OCString.
 * @ingroup OCString
 */
void OCStringLowercase(OCMutableStringRef theString);

/**
 * @brief Converts a mutable OCString to uppercase.
 * @param theString Mutable OCString.
 * @ingroup OCString
 */
void OCStringUppercase(OCMutableStringRef theString);

/**
 * @brief Trims whitespace characters from both ends of a mutable OCString.
 * @param theString Mutable OCString.
 * @ingroup OCString
 */
void OCStringTrimWhitespace(OCMutableStringRef theString);

/**
 * @brief Trims matching parentheses from both ends of a mutable OCString if present.
 * @param theString Mutable OCString.
 * @return true if parentheses were trimmed, false otherwise.
 * @ingroup OCString
 */
bool OCStringTrimMatchingParentheses(OCMutableStringRef theString);

/**
 * @brief Deletes a substring from a mutable OCString.
 * @param theString Mutable OCString.
 * @param range Range of characters to delete.
 * @ingroup OCString
 */
void OCStringDelete(OCMutableStringRef theString, OCRange range);

/**
 * @brief Inserts an OCString into a mutable OCString at a specified index.
 * @param str Mutable OCString to insert into.
 * @param idx Index at which to insert.
 * @param insertedStr OCString to insert.
 * @ingroup OCString
 */
void OCStringInsert(OCMutableStringRef str, int64_t idx, OCStringRef insertedStr);

/**
 * @brief Replaces a range in a mutable OCString with another OCString.
 * @param str Mutable OCString.
 * @param range Range to replace.
 * @param replacement OCString to insert.
 * @ingroup OCString
 */
void OCStringReplace(OCMutableStringRef str, OCRange range, OCStringRef replacement);

/**
 * @brief Replaces the entire contents of a mutable OCString with another OCString.
 * @param str Mutable OCString.
 * @param replacement OCString to set.
 * @ingroup OCString
 */
void OCStringReplaceAll(OCMutableStringRef str, OCStringRef replacement);

/**
 * @brief Returns the character at a specified index in an OCString.
 * @param theString OCString.
 * @param index Index of character.
 * @return Character at index.
 * @ingroup OCString
 */
uint32_t OCStringGetCharacterAtIndex(OCStringRef theString, uint64_t index);

/**
 * @brief Parses and returns the float complex value represented by the OCString.
 * @param string OCString containing complex arithmetic expression.
 * @return Parsed float complex value.
 * @ingroup OCString
 */
float complex OCStringGetFloatComplexValue(OCStringRef string);

/**
 * @brief Parses and returns the double complex value represented by the OCString.
 * @param string OCString containing complex arithmetic expression.
 * @return Parsed double complex value.
 * @ingroup OCString
 */
double complex OCStringGetDoubleComplexValue(OCStringRef string);

/**
 * @brief Finds a substring within an OCString with specified options.
 * @param string Source OCString.
 * @param stringToFind Substring to find.
 * @param compareOptions Comparison options flags.
 * @return Range of found substring or {0,0} if not found.
 * @ingroup OCString
 */
OCRange OCStringFind(OCStringRef string, OCStringRef stringToFind, OCOptionFlags compareOptions);

/**
 * @brief Finds and replaces all occurrences of a substring in a mutable OCString.
 * @param string Mutable OCString to modify.
 * @param stringToFind Substring to find.
 * @param replacementString Replacement OCString.
 * @return Number of replacements made.
 * @ingroup OCString
 */
int64_t OCStringFindAndReplace2(OCMutableStringRef string,
                               OCStringRef stringToFind,
                               OCStringRef replacementString);

/**
 * @brief Finds and replaces occurrences of a substring within a specified range in a mutable OCString.
 * @param string Mutable OCString to modify.
 * @param stringToFind Substring to find.
 * @param replacementString Replacement OCString.
 * @param rangeToSearch Range within string to search.
 * @param compareOptions Comparison options flags.
 * @return Number of replacements made.
 * @ingroup OCString
 */
int64_t OCStringFindAndReplace(OCMutableStringRef string,
                               OCStringRef stringToFind,
                               OCStringRef replacementString,
                               OCRange rangeToSearch,
                               OCOptionFlags compareOptions);

/**
 * @brief Creates an array of ranges where a substring is found within an OCString.
 * @param string Source OCString.
 * @param stringToFind Substring to find.
 * @param rangeToSearch Range within string to search.
 * @param compareOptions Comparison options flags.
 * @return OCArrayRef containing ranges of found substrings.
 * @ingroup OCString
 */
OCArrayRef OCStringCreateArrayWithFindResults(OCStringRef string, OCStringRef stringToFind, OCRange rangeToSearch, OCOptionFlags compareOptions);

/**
 * @brief Creates an array of OCStrings by splitting a string using a separator string.
 * @param string Source OCString.
 * @param separatorString Separator OCString.
 * @return OCArrayRef containing separated OCStrings.
 * @ingroup OCString
 */
OCArrayRef OCStringCreateArrayBySeparatingStrings(OCStringRef string, OCStringRef separatorString);

/**
 * @brief Prints the OCString to standard output.
 * @param theString OCString to display.
 * @ingroup OCString
 */
void OCStringShow(OCStringRef theString);

/**
 * @brief Compares two OCStrings for equality.
 * @param theString1 First OCString.
 * @param theString2 Second OCString.
 * @return true if equal, false otherwise.
 * @ingroup OCString
 */
bool OCStringEqual(OCStringRef theString1, OCStringRef theString2);

/**
 * @brief Creates an immutable OCString using a format string and arguments.
 * @param format Format OCString.
 * @param ... Variable arguments for the format string.
 * @return New OCStringRef (ownership transferred to caller).
 * @ingroup OCString
 */
OCStringRef  OCStringCreateWithFormat(OCStringRef format, ...);

/**
 * @brief Appends formatted text to a mutable OCString.
 * @param theString Mutable OCString.
 * @param format Format OCString.
 * @param ... Variable arguments for the format string.
 * @ingroup OCString
 */
void OCStringAppendFormat(OCMutableStringRef theString, OCStringRef format, ...);

/** \cond INTERNAL */
/**
 * @brief Creates a constant OCStringRef; private API.
 * @param cStr C string literal.
 * @return A compile-time constant OCStringRef; do not release.
 */
OCStringRef __OCStringMakeConstantString(const char *cStr);
/** \endcond */

/**
 * @brief Calculates and returns the double complex value represented by the complex arithmetic expression in the string.
 * @param string A string that contains a complex arithmetic expression.
 * @return The double complex value represented by string, or `nan("")` or `nan(NULL)` if there is a scanning error.
 * @ingroup OCString
 */
double complex OCComplexFromCString(const char *string);

/**
 * @brief Creates an OCString representing a float value.
 * @param value Float value.
 * @return New OCStringRef representing the float.
 * @ingroup OCString
 */
OCStringRef OCFloatCreateStringValue(float value);

/**
 * @brief Creates an OCString representing a double value.
 * @param value Double value.
 * @return New OCStringRef representing the double.
 * @ingroup OCString
 */
OCStringRef OCDoubleCreateStringValue(double value);

/**
 * @brief Creates an OCString representing a float complex value using a format.
 * @param value Float complex value.
 * @param format Format OCString.
 * @return New OCStringRef representing the complex value.
 * @ingroup OCString
 */
OCStringRef OCFloatComplexCreateStringValue(float complex value, OCStringRef format);

/**
 * @brief Creates an OCString representing a double complex value using a format.
 * @param value Double complex value.
 * @param format Format OCString.
 * @return New OCStringRef representing the complex value.
 * @ingroup OCString
 */
OCStringRef OCDoubleComplexCreateStringValue(double complex value,OCStringRef format);

/**
 * @brief Checks if a character is an uppercase letter.
 * @param character Character to check.
 * @return true if uppercase letter, false otherwise.
 * @ingroup OCString
 */
bool characterIsUpperCaseLetter(char character);

/**
 * @brief Checks if a character is a lowercase letter.
 * @param character Character to check.
 * @return true if lowercase letter, false otherwise.
 * @ingroup OCString
 */
bool characterIsLowerCaseLetter(char character);

/**
 * @brief Checks if a character is a digit or decimal point.
 * @param character Character to check.
 * @return true if digit or decimal point, false otherwise.
 * @ingroup OCString
 */
bool characterIsDigitOrDecimalPoint(char character);

/**
 * @brief Checks if a character is a digit, decimal point, or space.
 * @param character Character to check.
 * @return true if digit, decimal point, or space, false otherwise.
 * @ingroup OCString
 */
bool characterIsDigitOrDecimalPointOrSpace(char character);

/** @} */ // end of OCString group

#endif /* OCString_h */
