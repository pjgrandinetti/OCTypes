//
//  OCString.h
//  OCTypes
//
//  Created by philip on 5/10/17.
//

/**
 * @file OCString.h
 * @brief Immutable and mutable OCString reference types and operations.
 *
 * Provides creation, manipulation, comparison, and conversion functions
 * for OCTypes string objects, including support for complex arithmetic parsing.
 */

#ifndef OCString_h
#define OCString_h

#include "OCLibrary.h"

/**
 * @typedef OCStringRef
 * @brief Opaque reference to an immutable OCString.
 */
typedef const struct __OCString * OCStringRef;
/**
 * @typedef OCMutableStringRef
 * @brief Opaque reference to a mutable OCString.
 */
typedef struct __OCString * OCMutableStringRef;

/**
 * @def STR
 * @brief Macro to create a constant OCStringRef from a C string literal.
 */
#define STR(cStr)  __OCStringMakeConstantString("" cStr "")

/**
 * @function OCStringGetTypeID
 * @brief Returns the unique type identifier for OCString objects.
 * @return OCTypeID for OCString type.
 */
OCTypeID OCStringGetTypeID(void);

/**
 * @function OCStringCreateWithCString
 * @brief Creates an immutable OCString from a null-terminated C string.
 * @param string Null-terminated UTF-8 C string.
 * @return New OCStringRef (ownership transferred to caller), or NULL on failure.
 */
OCStringRef OCStringCreateWithCString(const char *string);
/**
 * @function OCStringCreateMutableCopy
 * @brief Creates a mutable copy of an immutable OCString.
 * @param theString Immutable OCString to copy.
 * @return New OCMutableStringRef (ownership transferred to caller).
 */
OCMutableStringRef OCStringCreateMutableCopy(OCStringRef theString);
/**
 * @function OCStringCreateWithSubstring
 * @brief Creates an immutable OCString from a substring of another OCString.
 * @param str Source OCString.
 * @param range Range of substring to extract.
 * @return New OCStringRef (ownership transferred to caller).
 */
OCStringRef OCStringCreateWithSubstring(OCStringRef str, OCRange range);

/**
 * @function OCStringAppend
 * @brief Appends an immutable OCString to a mutable OCString.
 * @param theString Mutable OCString to append to.
 * @param appendedString Immutable OCString to append.
 */
void OCStringAppend(OCMutableStringRef theString, OCStringRef appendedString);
/**
 * @function OCStringAppendCString
 * @brief Appends a C string to a mutable OCString.
 * @param theString Mutable OCString to append to.
 * @param cString Null-terminated UTF-8 C string to append.
 */
void OCStringAppendCString(OCMutableStringRef theString, const char *cString);

/**
 * @function OCStringCreateMutable
 * @brief Creates a new mutable OCString with specified initial capacity.
 * @param capacity Initial capacity for the mutable string.
 * @return New OCMutableStringRef (ownership transferred to caller).
 */
OCMutableStringRef OCStringCreateMutable(uint64_t capacity);
/**
 * @function OCMutableStringCreateWithCString
 * @brief Creates a mutable OCString from a null-terminated C string.
 * @param cString Null-terminated UTF-8 C string.
 * @return New OCMutableStringRef (ownership transferred to caller).
 */
OCMutableStringRef OCMutableStringCreateWithCString(const char *cString);
/**
 * @function OCStringGetCString
 * @brief Returns a C string representation of an immutable OCString.
 * @param theString Immutable OCString.
 * @return Null-terminated UTF-8 C string.
 */
const char *OCStringGetCString(OCStringRef theString);
/**
 * @function OCStringCreateCopy
 * @brief Creates a new immutable copy of an OCString.
 * @param theString Source OCString.
 * @return New OCStringRef (ownership transferred to caller).
 */
OCStringRef OCStringCreateCopy(OCStringRef theString);
/**
 * @function OCStringCompare
 * @brief Compares two OCStrings with specified options.
 * @param theString1 First OCString.
 * @param theString2 Second OCString.
 * @param compareOptions Comparison options flags.
 * @return Comparison result indicating order or equality.
 */
OCComparisonResult OCStringCompare(OCStringRef theString1, OCStringRef theString2, OCStringCompareFlags compareOptions);
/**
 * @function OCStringGetLength
 * @brief Returns the length of an OCString.
 * @param theString OCString.
 * @return Length of the string.
 */
uint64_t OCStringGetLength(OCStringRef theString);
/**
 * @function OCStringLowercase
 * @brief Converts a mutable OCString to lowercase.
 * @param theString Mutable OCString.
 */
void OCStringLowercase(OCMutableStringRef theString);
/**
 * @function OCStringUppercase
 * @brief Converts a mutable OCString to uppercase.
 * @param theString Mutable OCString.
 */
void OCStringUppercase(OCMutableStringRef theString);
/**
 * @function OCStringTrimWhitespace
 * @brief Trims whitespace characters from both ends of a mutable OCString.
 * @param theString Mutable OCString.
 */
void OCStringTrimWhitespace(OCMutableStringRef theString);
/**
 * @function OCStringTrimMatchingParentheses
 * @brief Trims matching parentheses from both ends of a mutable OCString if present.
 * @param theString Mutable OCString.
 * @return true if parentheses were trimmed, false otherwise.
 */
bool OCStringTrimMatchingParentheses(OCMutableStringRef theString);

/**
 * @function OCStringDelete
 * @brief Deletes a substring from a mutable OCString.
 * @param theString Mutable OCString.
 * @param range Range of characters to delete.
 */
void OCStringDelete(OCMutableStringRef theString, OCRange range);
/**
 * @function OCStringInsert
 * @brief Inserts an OCString into a mutable OCString at a specified index.
 * @param str Mutable OCString to insert into.
 * @param idx Index at which to insert.
 * @param insertedStr OCString to insert.
 */
void OCStringInsert(OCMutableStringRef str, int64_t idx, OCStringRef insertedStr);
/**
 * @function OCStringReplace
 * @brief Replaces a range in a mutable OCString with another OCString.
 * @param str Mutable OCString.
 * @param range Range to replace.
 * @param replacement OCString to insert.
 */
void OCStringReplace(OCMutableStringRef str, OCRange range, OCStringRef replacement);
/**
 * @function OCStringReplaceAll
 * @brief Replaces the entire contents of a mutable OCString with another OCString.
 * @param str Mutable OCString.
 * @param replacement OCString to set.
 */
void OCStringReplaceAll(OCMutableStringRef str, OCStringRef replacement);

/**
 * @function OCStringGetCharacterAtIndex
 * @brief Returns the character at a specified index in an OCString.
 * @param theString OCString.
 * @param index Index of character.
 * @return Character at index.
 */
char OCStringGetCharacterAtIndex(OCStringRef theString, uint64_t index);
/**
 * @function OCStringGetFloatComplexValue
 * @brief Parses and returns the float complex value represented by the OCString.
 * @param string OCString containing complex arithmetic expression.
 * @return Parsed float complex value.
 */
float complex OCStringGetFloatComplexValue(OCStringRef string);
/**
 * @function OCStringGetDoubleComplexValue
 * @brief Parses and returns the double complex value represented by the OCString.
 * @param string OCString containing complex arithmetic expression.
 * @return Parsed double complex value.
 */
double complex OCStringGetDoubleComplexValue(OCStringRef string);

/**
 * @function OCStringFind
 * @brief Finds a substring within an OCString with specified options.
 * @param string Source OCString.
 * @param stringToFind Substring to find.
 * @param compareOptions Comparison options flags.
 * @return Range of found substring or {0,0} if not found.
 */
OCRange OCStringFind(OCStringRef string, OCStringRef stringToFind, OCOptionFlags compareOptions);

/**
 * @function OCStringFindAndReplace2
 * @brief Finds and replaces all occurrences of a substring in a mutable OCString.
 * @param string Mutable OCString to modify.
 * @param stringToFind Substring to find.
 * @param replacementString Replacement OCString.
 * @return Number of replacements made.
 */
int64_t OCStringFindAndReplace2(OCMutableStringRef string,
                               OCStringRef stringToFind,
                               OCStringRef replacementString);
/**
 * @function OCStringFindAndReplace
 * @brief Finds and replaces occurrences of a substring within a specified range in a mutable OCString.
 * @param string Mutable OCString to modify.
 * @param stringToFind Substring to find.
 * @param replacementString Replacement OCString.
 * @param rangeToSearch Range within string to search.
 * @param compareOptions Comparison options flags.
 * @return Number of replacements made.
 */
int64_t OCStringFindAndReplace(OCMutableStringRef string,
                               OCStringRef stringToFind,
                               OCStringRef replacementString,
                               OCRange rangeToSearch,
                               OCOptionFlags compareOptions);

typedef const struct __OCArray * OCArrayRef;

/**
 * @function OCStringCreateArrayWithFindResults
 * @brief Creates an array of ranges where a substring is found within an OCString.
 * @param string Source OCString.
 * @param stringToFind Substring to find.
 * @param rangeToSearch Range within string to search.
 * @param compareOptions Comparison options flags.
 * @return OCArrayRef containing ranges of found substrings.
 */
OCArrayRef OCStringCreateArrayWithFindResults(OCStringRef string, OCStringRef stringToFind, OCRange rangeToSearch, OCOptionFlags compareOptions);
/**
 * @function OCStringCreateArrayBySeparatingStrings
 * @brief Creates an array of OCStrings by splitting a string using a separator string.
 * @param string Source OCString.
 * @param separatorString Separator OCString.
 * @return OCArrayRef containing separated OCStrings.
 */
OCArrayRef OCStringCreateArrayBySeparatingStrings(OCStringRef string, OCStringRef separatorString);

/**
 * @function OCStringShow
 * @brief Prints the OCString to standard output.
 * @param theString OCString to display.
 */
void OCStringShow(OCStringRef theString);
/**
 * @function OCStringEqual
 * @brief Compares two OCStrings for equality.
 * @param theString1 First OCString.
 * @param theString2 Second OCString.
 * @return true if equal, false otherwise.
 */
bool OCStringEqual(OCStringRef theString1, OCStringRef theString2);

/**
 * @function OCStringCreateWithFormat
 * @brief Creates an immutable OCString using a format string and arguments.
 * @param format Format OCString.
 * @param ... Format arguments.
 * @return New OCStringRef (ownership transferred to caller).
 */
OCStringRef  OCStringCreateWithFormat(OCStringRef format, ...);
/**
 * @function OCStringAppendFormat
 * @brief Appends formatted text to a mutable OCString.
 * @param theString Mutable OCString.
 * @param format Format OCString.
 * @param ... Format arguments.
 */
void OCStringAppendFormat(OCMutableStringRef theString, OCStringRef format, ...);


/** @internal Creates a constant OCStringRef; private API. */
OCStringRef __OCStringMakeConstantString(const char *cStr);

/*!
 @function OCComplexFromCString
 @abstract Calculates and Returns the double complex value represented by the complex arithmetic expression in the string.
 @param string A string that contains a complex arithmetic expression
 @result The double complex value represented by string, or nan(NULL) if there is a scanning error (if the string contains disallowed characters or does not represent a double complex value).
 @discussion Consider the following example:
 
 <pre><code>double val = PSComplexFromString("0.123 + 0.456*I");</code></pre>
 The variable val in this example would contain the complex value 0.123 + 0.456*I after the function is called.
 
 <pre><code>double val = PSComplexFromString("(0.123 + 0.456*I)/(1.32+4.5*I)");</code></pre>
 The variable val in this example would contain the complex value 0.100688+0.00220167*I after the function is called.
 */
/**
 * @function OCComplexFromCString
 * @brief Calculates and returns the double complex value represented by the complex arithmetic expression in the string.
 * @param string A string that contains a complex arithmetic expression.
 * @return The double complex value represented by string, or nan(NULL) if there is a scanning error.
 * @discussion Examples:
 *   - double val = PSComplexFromString("0.123 + 0.456*I"); // val = 0.123 + 0.456*I
 *   - double val = PSComplexFromString("(0.123 + 0.456*I)/(1.32+4.5*I)"); // val = 0.100688+0.00220167*I
 */
double complex OCComplexFromCString(const char *string);

/**
 * @function PSFloatCreateStringValue
 * @brief Creates an OCString representing a float value.
 * @param value Float value.
 * @return New OCStringRef representing the float.
 */
OCStringRef PSFloatCreateStringValue(float value);
/**
 * @function OCDoubleCreateStringValue
 * @brief Creates an OCString representing a double value.
 * @param value Double value.
 * @return New OCStringRef representing the double.
 */
OCStringRef OCDoubleCreateStringValue(double value);
/**
 * @function OCFloatComplexCreateStringValue
 * @brief Creates an OCString representing a float complex value using a format.
 * @param value Float complex value.
 * @param format Format OCString.
 * @return New OCStringRef representing the complex value.
 */
OCStringRef OCFloatComplexCreateStringValue(float complex value, OCStringRef format);
/**
 * @function PSDoubleComplexCreateStringValue
 * @brief Creates an OCString representing a double complex value using a format.
 * @param value Double complex value.
 * @param format Format OCString.
 * @return New OCStringRef representing the complex value.
 */
OCStringRef PSDoubleComplexCreateStringValue(double complex value,OCStringRef format);

/**
 * @function characterIsUpperCaseLetter
 * @brief Checks if a character is an uppercase letter.
 * @param character Character to check.
 * @return true if uppercase letter, false otherwise.
 */
bool characterIsUpperCaseLetter(char character);
/**
 * @function characterIsLowerCaseLetter
 * @brief Checks if a character is a lowercase letter.
 * @param character Character to check.
 * @return true if lowercase letter, false otherwise.
 */
bool characterIsLowerCaseLetter(char character);
/**
 * @function characterIsDigitOrDecimalPoint
 * @brief Checks if a character is a digit or decimal point.
 * @param character Character to check.
 * @return true if digit or decimal point, false otherwise.
 */
bool characterIsDigitOrDecimalPoint(char character);
/**
 * @function characterIsDigitOrDecimalPointOrSpace
 * @brief Checks if a character is a digit, decimal point, or space.
 * @param character Character to check.
 * @return true if digit, decimal point, or space, false otherwise.
 */
bool characterIsDigitOrDecimalPointOrSpace(char character);

#endif /* OCString_h */
