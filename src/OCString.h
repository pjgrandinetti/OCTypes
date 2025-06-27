//
//  OCString.h
//  OCTypes
//
//  Created by Philip Grandinetti
//

#ifndef OCString_h
#define OCString_h

#include <complex.h>
#include <time.h>
#include <stdio.h>
#include "OCLibrary.h"

/**
 * @note Ownership follows CoreFoundation conventions:
 *       The caller owns any OCStringRef returned from functions with "Create"
 *       in the name, and must call OCRelease() when done.
 */

/** @defgroup OCString OCString
 *  @brief Immutable and mutable OCString operations.
 *  @{
 */

/**
 * @brief Macro to create a constant OCStringRef from a C string literal.
 * @param cStr C string literal.
 * @return A compile-time constant OCStringRef; do not release.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef hello = STR("Hello, world!");
 * // Use hello as a constant string
 * OCStringShow(hello);
 * // Do NOT release STR strings
 * @endcode
 */
#define STR(cStr)  impl_OCStringMakeConstantString("" cStr "")

/**
 * @brief Returns the unique type identifier for OCString objects.
 * @return The OCTypeID for OCString.
 * @ingroup OCString
 * 
 * @code
 * OCTypeID stringTypeID = OCStringGetTypeID();
 * if (OCGetTypeID(someObject) == stringTypeID) {
 *     // The object is an OCString
 * }
 * @endcode
 */
OCTypeID OCStringGetTypeID(void);

/**
 * @brief Creates an immutable OCString from a C string.
 * @param string Null-terminated UTF-8 C string.
 * @return New OCStringRef, or NULL on failure.
 * @ingroup OCString
 * 
 * @code
 * const char *cString = "Hello, world!";
 * OCStringRef myString = OCStringCreateWithCString(cString);
 * 
 * // Do operations with myString
 * 
 * // Release when done
 * OCRelease(myString);
 * @endcode
 */
OCStringRef OCStringCreateWithCString(const char *string);

/**
 * @brief Creates a mutable copy of an immutable OCString.
 * @param theString Immutable OCString to copy.
 * @return New OCMutableStringRef (ownership transferred to caller).
 * @ingroup OCString
 * 
 * @code
 * OCStringRef immutableString = OCStringCreateWithCString("Hello");
 * OCMutableStringRef mutableString = OCStringCreateMutableCopy(immutableString);
 * 
 * // Now we can modify mutableString
 * OCStringAppendCString(mutableString, ", world!");
 * 
 * // Release when done
 * OCRelease(immutableString);
 * OCRelease(mutableString);
 * @endcode
 */
OCMutableStringRef OCStringCreateMutableCopy(OCStringRef theString);

/**
 * @brief Create a JSON string from an OCStringRef.
 *
 * Converts the OCStringRef to a UTF-8 C string and returns it
 * as a cJSON string node. If the input is NULL, a JSON null is returned.
 *
 * @param str The OCStringRef to serialize.
 * @return A cJSON string node or cJSON null on failure.
 */
cJSON *OCStringCreateJSON(OCStringRef str);

/**
 * @brief Create an OCStringRef from a cJSON string node.
 *
 * Parses the JSON value as a string. If the input is NULL or not a string,
 * returns NULL. The returned string follows CoreFoundation ownership rules
 * (caller must call OCRelease when done).
 *
 * @param json A cJSON node expected to be a string.
 * @return A newly allocated OCStringRef, or NULL on failure.
 */
OCStringRef OCStringCreateFromJSON(cJSON *json);

/**
 * @brief Creates an OCString by decoding raw data as UTF-8 text.
 *
 * This function takes an OCDataRef containing UTF-8 encoded bytes and
 * produces a newly allocated OCStringRef. On platforms without CFString,
 * this performs a simple UTF-8 decode via OCStringCreateWithCString.
 *
 * @param data  OCDataRef containing the raw UTF-8 bytes.
 * @return A new OCStringRef representing the decoded text. 
 *         Ownership is transferred to the caller and must be released with OCRelease().
 * @ingroup OCString
 *
 * @code
 * // Suppose 'raw' is a binary blob loaded from a file:
 * OCDataRef raw = OCDataCreateWithContentsOfFile("example.jdx");
 * OCStringRef text = OCStringCreateWithExternalRepresentation(raw);
 *
 * // Use 'text' as needed...
 * printf("%s\n", OCStringGetCString(text, NULL));
 *
 * // Clean up
 * OCRelease(raw);
 * OCRelease(text);
 * @endcode
 */
OCStringRef
OCStringCreateWithExternalRepresentation(OCDataRef data);


/**
 * @brief Creates an immutable OCString from a substring.
 * @param str Source OCString.
 * @param range Range of substring to extract.
 * @return New OCStringRef, or NULL on failure.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef fullString = OCStringCreateWithCString("Hello, world!");
 * OCRange range = {7, 5}; // Start at index 7, length 5 characters
 * OCStringRef substring = OCStringCreateWithSubstring(fullString, range);
 * // substring now contains "world"
 * 
 * // Release when done
 * OCRelease(fullString);
 * OCRelease(substring);
 * @endcode
 */
OCStringRef OCStringCreateWithSubstring(OCStringRef str, OCRange range);

/**
 * @brief Appends an immutable OCString to a mutable OCString.
 * @param theString Mutable OCString to append to.
 * @param appendedString Immutable OCString to append.
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef mutableString = OCStringCreateMutable(10);
 * OCStringRef hello = OCStringCreateWithCString("Hello");
 * OCStringRef world = OCStringCreateWithCString(" World");
 * 
 * OCStringAppend(mutableString, hello);
 * OCStringAppend(mutableString, world);
 * // mutableString now contains "Hello World"
 * 
 * // Release when done
 * OCRelease(hello);
 * OCRelease(world);
 * OCRelease(mutableString);
 * @endcode
 */
void OCStringAppend(OCMutableStringRef theString, OCStringRef appendedString);

/**
 * @brief Appends a C string to a mutable OCString.
 * @param theString Mutable OCString to append to.
 * @param cString Null-terminated UTF-8 C string to append.
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef mutableString = OCStringCreateMutable(10);
 * OCStringAppendCString(mutableString, "Hello");
 * OCStringAppendCString(mutableString, " World");
 * // mutableString now contains "Hello World"
 * 
 * // Release when done
 * OCRelease(mutableString);
 * @endcode
 */
void OCStringAppendCString(OCMutableStringRef theString, const char *cString);

/**
 * @brief Creates a new mutable OCString with specified initial capacity.
 * @param capacity Initial capacity for the mutable string.
 * @return New OCMutableStringRef (ownership transferred to caller).
 * @ingroup OCString
 * 
 * @code
 * // Create a mutable string with initial capacity of 50 characters
 * OCMutableStringRef mutableString = OCStringCreateMutable(50);
 * 
 * // Append content to the string
 * OCStringAppendCString(mutableString, "Initial content");
 * 
 * // Release when done
 * OCRelease(mutableString);
 * @endcode
 */
OCMutableStringRef OCStringCreateMutable(uint64_t capacity);

/**
 * @brief Creates a mutable OCString from a null-terminated C string.
 * @param cString Null-terminated UTF-8 C string.
 * @return New OCMutableStringRef (ownership transferred to caller).
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef mutableString = OCMutableStringCreateWithCString("Hello");
 * 
 * // Modify the string
 * OCStringAppendCString(mutableString, ", world!");
 * OCStringUppercase(mutableString);
 * // mutableString now contains "HELLO, WORLD!"
 * 
 * // Release when done
 * OCRelease(mutableString);
 * @endcode
 */
OCMutableStringRef OCMutableStringCreateWithCString(const char *cString);


/**
 * @brief Returns a C string representation of an immutable OCString.
 * @param theString Immutable OfCString.
 * @return Null-terminated UTF-8 C string.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef myString = OCStringCreateWithCString("Hello, world!");
 * 
 * // Get the C string representation
 * const char *cString = OCStringGetCString(myString);
 * 
 * // Use the C string with standard C functions
 * printf("%s\n", cString);  // Outputs: Hello, world!
 * 
 * // Release when done
 * OCRelease(myString);
 * // Note: Do not free the C string returned by OCStringGetCString
 * @endcode
 */
const char *OCStringGetCString(OCStringRef theString);

/**
 * @brief Creates a new immutable copy of an OCString.
 * @param theString Source OCString.
 * @return New OCStringRef (ownership transferred to caller).
 * @ingroup OCString
 * 
 * @code
 * OCStringRef original = OCStringCreateWithCString("Hello, world!");
 * 
 * // Create a copy
 * OCStringRef copy = OCStringCreateCopy(original);
 * 
 * // Both strings have the same content but are separate objects
 * bool areEqual = OCStringEqual(original, copy);  // Returns true
 * 
 * // Release both strings when done
 * OCRelease(original);
 * OCRelease(copy);
 * @endcode
 */
OCStringRef OCStringCreateCopy(OCStringRef theString);

/**
 * @brief Compares two OCStrings with specified options.
 * @param theString1 First OCString.
 * @param theString2 Second OCString.
 * @param compareOptions Comparison options flags.
 * @return Comparison result indicating order or equality.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef string1 = OCStringCreateWithCString("Apple");
 * OCStringRef string2 = OCStringCreateWithCString("APPLE");
 * 
 * // Case-sensitive comparison
 * OCComparisonResult result1 = OCStringCompare(string1, string2, 0);
 * // result1 will be kOCCompareGreaterThan because 'a' > 'A'
 * 
 * // Case-insensitive comparison
 * OCComparisonResult result2 = OCStringCompare(string1, string2, kOCCompareCaseInsensitive);
 * // result2 will be kOCCompareEqualTo
 * 
 * // Release when done
 * OCRelease(string1);
 * OCRelease(string2);
 * @endcode
 */
OCComparisonResult OCStringCompare(OCStringRef theString1, OCStringRef theString2, OCStringCompareFlags compareOptions);

/**
 * @brief Returns the length of an OCString.
 * @param theString OCString.
 * @return Length of the string.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef myString = OCStringCreateWithCString("Hello, world!");
 * 
 * uint64_t length = OCStringGetLength(myString);
 * // length will be 13
 * 
 * // Release when done
 * OCRelease(myString);
 * @endcode
 */
uint64_t OCStringGetLength(OCStringRef theString);

/**
 * @brief Converts a mutable OCString to lowercase.
 * @param theString Mutable OCString.
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef myString = OCMutableStringCreateWithCString("Hello, WORLD!");
 * 
 * OCStringLowercase(myString);
 * // myString now contains "hello, world!"
 * 
 * // Release when done
 * OCRelease(myString);
 * @endcode
 */
void OCStringLowercase(OCMutableStringRef theString);

/**
 * @brief Converts a mutable OCString to uppercase.
 * @param theString Mutable OCString.
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef myString = OCMutableStringCreateWithCString("Hello, world!");
 * 
 * OCStringUppercase(myString);
 * // myString now contains "HELLO, WORLD!"
 * 
 * // Release when done
 * OCRelease(myString);
 * @endcode
 */
void OCStringUppercase(OCMutableStringRef theString);

/**
 * @brief Trims whitespace characters from both ends of a mutable OCString.
 * @param theString Mutable OCString.
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef myString = OCMutableStringCreateWithCString("  Hello, world!   ");
 * 
 * OCStringTrimWhitespace(myString);
 * // myString now contains "Hello, world!"
 * 
 * // Release when done
 * OCRelease(myString);
 * @endcode
 */
void OCStringTrimWhitespace(OCMutableStringRef theString);

/**
 * @brief Trims matching parentheses from both ends of a mutable OCString if present.
 * @param theString Mutable OCString.
 * @return true if parentheses were trimmed, false otherwise.
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef string1 = OCMutableStringCreateWithCString("(Hello, world!)");
 * OCMutableStringRef string2 = OCMutableStringCreateWithCString("No parentheses");
 * 
 * bool trimmed1 = OCStringTrimMatchingParentheses(string1);
 * // trimmed1 will be true
 * // string1 now contains "Hello, world!"
 * 
 * bool trimmed2 = OCStringTrimMatchingParentheses(string2);
 * // trimmed2 will be false
 * // string2 remains unchanged
 * 
 * // Release when done
 * OCRelease(string1);
 * OCRelease(string2);
 * @endcode
 */
bool OCStringTrimMatchingParentheses(OCMutableStringRef theString);

/**
 * @brief Deletes a substring from a mutable OCString.
 * @param theString Mutable OCString.
 * @param range Range of characters to delete.
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef myString = OCMutableStringCreateWithCString("Hello, world!");
 * 
 * // Delete ", world" (characters 5-11)
 * OCRange deleteRange = {5, 7};
 * OCStringDelete(myString, deleteRange);
 * // myString now contains "Hello!"
 * 
 * // Release when done
 * OCRelease(myString);
 * @endcode
 */
void OCStringDelete(OCMutableStringRef theString, OCRange range);

/**
 * @brief Inserts an OCString into a mutable OCString at a specified index.
 * @param str Mutable OCString to insert into.
 * @param idx Index at which to insert.
 * @param insertedStr OCString to insert.
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef myString = OCMutableStringCreateWithCString("Hello world!");
 * OCStringRef insertStr = OCStringCreateWithCString("beautiful ");
 * 
 * // Insert at position 6 (after "Hello ")
 * OCStringInsert(myString, 6, insertStr);
 * // myString now contains "Hello beautiful world!"
 * 
 * // Release when done
 * OCRelease(myString);
 * OCRelease(insertStr);
 * @endcode
 */
void OCStringInsert(OCMutableStringRef str, int64_t idx, OCStringRef insertedStr);

/**
 * @brief Replaces a range in a mutable OCString with another OCString.
 * @param str Mutable OCString.
 * @param range Range to replace.
 * @param replacement OCString to insert.
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef myString = OCMutableStringCreateWithCString("Hello world!");
 * OCStringRef replacement = OCStringCreateWithCString("everyone");
 * 
 * // Replace "world" (characters 6-11) with "everyone"
 * OCRange replaceRange = {6, 5};
 * OCStringReplace(myString, replaceRange, replacement);
 * // myString now contains "Hello everyone!"
 * 
 * // Release when done
 * OCRelease(myString);
 * OCRelease(replacement);
 * @endcode
 */
void OCStringReplace(OCMutableStringRef str, OCRange range, OCStringRef replacement);

/**
 * @brief Replaces the entire contents of a mutable OCString with another OCString.
 * @param str Mutable OCString.
 * @param replacement OCString to set.
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef myString = OCMutableStringCreateWithCString("Hello world!");
 * OCStringRef replacement = OCStringCreateWithCString("Greetings, everyone!");
 * 
 * OCStringReplaceAll(myString, replacement);
 * // myString now contains "Greetings, everyone!"
 * 
 * // Release when done
 * OCRelease(myString);
 * OCRelease(replacement);
 * @endcode
 */
void OCStringReplaceAll(OCMutableStringRef str, OCStringRef replacement);

/**
 * @brief Returns the character at a specified index in an OCString.
 * @param theString OCString.
 * @param index Index of character.
 * @return Character at index.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef myString = OCStringCreateWithCString("Hello, world!");
 * 
 * uint32_t char0 = OCStringGetCharacterAtIndex(myString, 0);  // 'H'
 * uint32_t char7 = OCStringGetCharacterAtIndex(myString, 7);  // 'w'
 * 
 * // Release when done
 * OCRelease(myString);
 * @endcode
 */
uint32_t OCStringGetCharacterAtIndex(OCStringRef theString, uint64_t index);

/**
 * @brief Parses and returns the float complex value represented by the OCString.
 * @param string OCString containing complex arithmetic expression.
 * @return Parsed float complex value.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef complexStr = OCStringCreateWithCString("3+4i");
 * 
 * float complex value = OCStringGetFloatComplexValue(complexStr);
 * // value is equal to 3.0f + 4.0fi
 * 
 * float real = crealf(value);      // 3.0f
 * float imag = cimagf(value);      // 4.0f
 * 
 * // Release when done
 * OCRelease(complexStr);
 * @endcode
 */
float complex OCStringGetFloatComplexValue(OCStringRef string);

/**
 * @brief Parses and returns the double complex value represented by the OCString.
 * @param string OCString containing complex arithmetic expression.
 * @return Parsed double complex value.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef complexStr = OCStringCreateWithCString("2.5 * (3-2i) + 1.5i");
 * 
 * double complex value = OCStringGetDoubleComplexValue(complexStr);
 * // value is equal to 7.5 - 5.0i + 1.5i = 7.5 - 3.5i
 * 
 * double real = creal(value);      // 7.5
 * double imag = cimag(value);      // -3.5
 * 
 * // Release when done
 * OCRelease(complexStr);
 * @endcode
 */
double complex OCStringGetDoubleComplexValue(OCStringRef string);

/**
 * @brief Finds a substring within an OCString with specified options.
 * @param string Source OCString.
 * @param stringToFind Substring to find.
 * @param compareOptions Comparison options flags.
 * @return Range of found substring or {0,0} if not found.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef haystack = OCStringCreateWithCString("Hello, World! Hello again.");
 * OCStringRef needle = OCStringCreateWithCString("hello");
 * 
 * // Case-sensitive search (won't find it)
 * OCRange range1 = OCStringFind(haystack, needle, 0);
 * // range1 will be {0, 0} indicating not found
 * 
 * // Case-insensitive search
 * OCRange range2 = OCStringFind(haystack, needle, kOCCompareCaseInsensitive);
 * // range2 will be {0, 5} indicating found at position 0 with length 5
 * 
 * // Search for second occurrence
 * OCRange searchRange = {7, OCStringGetLength(haystack) - 7};
 * OCRange range3 = OCStringFind(haystack, needle, kOCCompareCaseInsensitive);
 * // range3 will be {14, 5} indicating found at position 14 with length 5
 * 
 * // Release when done
 * OCRelease(haystack);
 * OCRelease(needle);
 * @endcode
 */
OCRange OCStringFind(OCStringRef string, OCStringRef stringToFind, OCOptionFlags compareOptions);

/**
 * @brief Finds and replaces all occurrences of a substring in a mutable OCString.
 * @param string Mutable OCString to modify.
 * @param stringToFind Substring to find.
 * @param replacementString Replacement OCString.
 * @return Number of replacements made.
 * @ingroup OCString
 * 
 * @code
 * OCMutableStringRef text = OCMutableStringCreateWithCString("Hello world! Hello again, world!");
 * OCStringRef find = OCStringCreateWithCString("Hello");
 * OCStringRef replace = OCStringCreateWithCString("Greetings");
 * 
 * // Replace all occurrences of "Hello" with "Greetings"
 * int64_t replacements = OCStringFindAndReplace2(text, find, replace);
 * // replacements will be 2
 * // text now contains "Greetings world! Greetings again, world!"
 * 
 * // Release when done
 * OCRelease(text);
 * OCRelease(find);
 * OCRelease(replace);
 * @endcode
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
 * 
 * @code
 * OCMutableStringRef text = OCMutableStringCreateWithCString("Hello world! Hello again, world!");
 * OCStringRef find = OCStringCreateWithCString("world");
 * OCStringRef replace = OCStringCreateWithCString("everybody");
 * 
 * // Replace only the first occurrence of "world" (within first 15 characters)
 * OCRange searchRange = {0, 15};
 * int64_t replacements = OCStringFindAndReplace(text, find, replace, searchRange, 0);
 * // replacements will be 1
 * // text now contains "Hello everybody! Hello again, world!"
 * 
 * // Replace with case-insensitive search (rest of string)
 * OCRange searchRange2 = {15, OCStringGetLength(text) - 15};
 * OCStringRef findWorld = OCStringCreateWithCString("WORLD");
 * replacements = OCStringFindAndReplace(text, findWorld, replace, searchRange2, kOCCompareCaseInsensitive);
 * // replacements will be 1
 * // text now contains "Hello everybody! Hello again, everybody!"
 * 
 * // Release when done
 * OCRelease(text);
 * OCRelease(find);
 * OCRelease(findWorld);
 * OCRelease(replace);
 * @endcode
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
 * 
 * @code
 * OCStringRef text = OCStringCreateWithCString("Hello world! Hello again, world!");
 * OCStringRef find = OCStringCreateWithCString("world");
 * 
 * // Find all occurrences of "world" in the entire string
 * OCRange searchRange = {0, OCStringGetLength(text)};
 * OCArrayRef results = OCStringCreateArrayWithFindResults(text, find, searchRange, 0);
 * 
 * // results will contain 2 OCRange objects:
 * // range at index 0 = {6, 5} (first "world")
 * // range at index 1 = {26, 5} (second "world")
 * uint64_t count = OCArrayGetCount(results);  // 2
 * 
 * // Extract and use the ranges
 * for (int i = 0; i < count; i++) {
 *     OCRange *range = (OCRange *)OCArrayGetValueAtIndex(results, i);
 *     // Use range->location and range->length as needed
 * }
 * 
 * // Release when done
 * OCRelease(text);
 * OCRelease(find);
 * OCRelease(results);
 * @endcode
 */
OCArrayRef OCStringCreateArrayWithFindResults(OCStringRef string, OCStringRef stringToFind, OCRange rangeToSearch, OCOptionFlags compareOptions);

/**
 * @brief Creates an array of OCStrings by splitting a string using a separator string.
 * @param string Source OCString.
 * @param separatorString Separator OCString.
 * @return OCArrayRef containing separated OCStrings.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef csv = OCStringCreateWithCString("apple,banana,orange,grape");
 * OCStringRef separator = OCStringCreateWithCString(",");
 * 
 * // Split string by comma
 * OCArrayRef fruits = OCStringCreateArrayBySeparatingStrings(csv, separator);
 * 
 * // fruits will contain 4 OCString objects:
 * // OCString at index 0 = "apple"
 * // OCString at index 1 = "banana"
 * // OCString at index 2 = "orange"
 * // OCString at index 3 = "grape"
 * uint64_t count = OCArrayGetCount(fruits);  // 4
 * 
 * // Access individual strings from the array
 * for (int i = 0; i < count; i++) {
 *     OCStringRef fruit = (OCStringRef)OCArrayGetValueAtIndex(fruits, i);
 *     OCStringShow(fruit);  // Print each fruit name
 * }
 * 
 * // Release when done
 * OCRelease(csv);
 * OCRelease(separator);
 * OCRelease(fruits);  // This releases the array, but not its contents
 * @endcode
 */
OCArrayRef OCStringCreateArrayBySeparatingStrings(OCStringRef string, OCStringRef separatorString);

/**
 * @brief Prints the OCString to standard output.
 * @param theString OCString to display.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef myString = OCStringCreateWithCString("Hello, world!");
 * 
 * // Print the string to standard output
 * OCStringShow(myString);  // Outputs: Hello, world!
 * 
 * // Release when done
 * OCRelease(myString);
 * @endcode
 */
void OCStringShow(OCStringRef theString);

/**
 * @brief Compares two OCStrings for equality.
 * @param theString1 First OCString.
 * @param theString2 Second OCString.
 * @return true if equal, false otherwise.
 * @ingroup OCString
 * 
 * @code
 * OCStringRef string1 = OCStringCreateWithCString("Hello");
 * OCStringRef string2 = OCStringCreateWithCString("Hello");
 * OCStringRef string3 = OCStringCreateWithCString("World");
 * 
 * bool equal1 = OCStringEqual(string1, string2);  // true
 * bool equal2 = OCStringEqual(string1, string3);  // false
 * 
 * // Note: This is a case-sensitive comparison
 * OCStringRef string4 = OCStringCreateWithCString("HELLO");
 * bool equal3 = OCStringEqual(string1, string4);  // false
 * 
 * // For case-insensitive comparison, use OCStringCompare with kOCCompareCaseInsensitive
 * 
 * // Release when done
 * OCRelease(string1);
 * OCRelease(string2);
 * OCRelease(string3);
 * OCRelease(string4);
 * @endcode
 */
bool OCStringEqual(OCStringRef theString1, OCStringRef theString2);

/**
 * @brief Creates an immutable OCString using a format string and arguments.
 * @param format Format OCString (supports printf-style specifiers and %@ for OCStringRef).
 * @param ... Arguments matching the format string.
 * @return New OCStringRef (ownership transferred to caller).
 * @ingroup OCString
 *
 * @warning
 * - Passing fewer arguments than required will print a warning and insert "[MISSING]" at missing positions.
 * - Passing extra arguments is safe but they are ignored.
 * - Argument types must match format specifiers exactly—C99 varargs cannot check types at runtime.
 * - For %@, if the OCStringRef is NULL, a warning is printed and nothing is inserted.
 *
 * @code
 * OCStringRef s = OCStringCreateWithFormat(STR("%@ %d"), STR("Hello"), 123);
 * // s contains "Hello 123"
 * @endcode
 */
OCStringRef OCStringCreateWithFormat(OCStringRef format, ...);

/**
 * @brief Appends formatted text to a mutable OCString.
 * @param theString Mutable OCString.
 * @param format Format OCString (supports printf-style specifiers and %@ for OCStringRef).
 * @param ... Arguments matching the format string.
 * @ingroup OCString
 *
 * @warning
 * - Too few arguments: warning printed, "[MISSING]" inserted.
 * - Extra arguments: ignored.
 * - Argument types must match the format exactly.
 * - %@ with NULL: warning printed, nothing inserted.
 *
 * @code
 * OCMutableStringRef s = OCMutableStringCreateWithCString("Val: ");
 * OCStringAppendFormat(s, STR("%@ %d"), STR("foo"), 7);
 * // s now contains "Val: foo 7"
 * @endcode
 */
void OCStringAppendFormat(OCMutableStringRef theString, OCStringRef format, ...);

/** \cond INTERNAL */
/**
 * @brief Creates a constant OCStringRef; private API.
 * @param cStr C string literal.
 * @return A compile-time constant OCStringRef; do not release.
 */
OCStringRef impl_OCStringMakeConstantString(const char *cStr);
/** \endcond */

/**
 * @brief Calculates and returns the double complex value represented by the complex arithmetic expression in the string.
 * @param string A string that contains a complex arithmetic expression.
 * @return The double complex value represented by string, or `nan("")` or `nan(NULL)` if there is a scanning error.
 * @ingroup OCString
 * 
 * @code
 * // Parse a simple complex number
 * double complex value1 = OCComplexFromCString("3+4i");
 * // value1 is equal to 3.0 + 4.0i
 * 
 * // Parse a complex expression
 * double complex value2 = OCComplexFromCString("(2-3i) * (4+2i)");
 * // value2 is equal to (2-3i) * (4+2i) = 8 + 4i - 12i - 6i² = 8 - 8i + 6 = 14 - 8i
 * 
 * double real = creal(value2);    // 14.0
 * double imag = cimag(value2);    // -8.0
 * @endcode
 */
double complex OCComplexFromCString(const char *string);

/**
 * @brief Creates an OCString representing a float value.
 * @param value Float value.
 * @return New OCStringRef representing the float.
 * @ingroup OCString
 * 
 * @code
 * float pi = 3.14159f;
 * OCStringRef piString = OCFloatCreateStringValue(pi);
 * // piString now contains "3.14159"
 * 
 * OCStringShow(piString);  // Output: 3.14159
 * 
 * // Release when done
 * OCRelease(piString);
 * @endcode
 */
OCStringRef OCFloatCreateStringValue(float value);

/**
 * @brief Creates an OCString representing a double value.
 * @param value Double value.
 * @return New OCStringRef representing the double.
 * @ingroup OCString
 * 
 * @code
 * double e = 2.71828182845904;
 * OCStringRef eString = OCDoubleCreateStringValue(e);
 * // eString now contains "2.71828182845904"
 * 
 * OCStringShow(eString);  // Output: 2.71828182845904
 * 
 * // Release when done
 * OCRelease(eString);
 * @endcode
 */
OCStringRef OCDoubleCreateStringValue(double value);

/**
 * @brief Creates an OCString representing a float complex value using a format.
 * @param value Float complex value.
 * @param format Format OCString.
 * @return New OCStringRef representing the complex value.
 * @ingroup OCString
 * 
 * @code
 * float complex z = 2.5f + 3.7fi;
 * 
 * // Format with default precision
 * OCStringRef format1 = STR("%.1f%+.1fi");
 * OCStringRef zString1 = OCFloatComplexCreateStringValue(z, format1);
 * // zString1 now contains "2.5+3.7i"
 * 
 * // Format with different precision
 * OCStringRef format2 = STR("%.2f%+.2fi");
 * OCStringRef zString2 = OCFloatComplexCreateStringValue(z, format2);
 * // zString2 now contains "2.50+3.70i"
 * 
 * // Release when done
 * OCRelease(zString1);
 * OCRelease(zString2);
 * // No need to release format strings created with STR macro
 * @endcode
 */
OCStringRef OCFloatComplexCreateStringValue(float complex value, OCStringRef format);

/**
 * @brief Creates an OCString representing a double complex value using a format.
 * @param value Double complex value.
 * @param format Format OCString.
 * @return New OCStringRef representing the complex value.
 * @ingroup OCString
 * 
 * @code
 * double complex z = 1.234 - 5.678i;
 * 
 * // Format with default notation
 * OCStringRef format1 = STR("%.3f%+.3fi");
 * OCStringRef zString1 = OCDoubleComplexCreateStringValue(z, format1);
 * // zString1 now contains "1.234-5.678i"
 * 
 * // Format with scientific notation
 * OCStringRef format2 = STR("%e%+ei");
 * OCStringRef zString2 = OCDoubleComplexCreateStringValue(z, format2);
 * // zString2 now contains something like "1.234000e+00-5.678000e+00i"
 * 
 * // Release when done
 * OCRelease(zString1);
 * OCRelease(zString2);
 * // No need to release format strings created with STR macro
 * @endcode
 */
OCStringRef OCDoubleComplexCreateStringValue(double complex value,OCStringRef format);

/**
 * @brief Checks if a character is an uppercase letter.
 * @param character Character to check.
 * @return true if uppercase letter, false otherwise.
 * @ingroup OCString
 * 
 * @code
 * bool isUpper1 = characterIsUpperCaseLetter('A');  // true
 * bool isUpper2 = characterIsUpperCaseLetter('a');  // false
 * bool isUpper3 = characterIsUpperCaseLetter('Z');  // true
 * bool isUpper4 = characterIsUpperCaseLetter('5');  // false
 * bool isUpper5 = characterIsUpperCaseLetter(' ');  // false
 * @endcode
 */
bool characterIsUpperCaseLetter(uint32_t character);

/**
 * @brief Checks if a character is a lowercase letter.
 * @param character Character to check.
 * @return true if lowercase letter, false otherwise.
 * @ingroup OCString
 * 
 * @code
 * bool isLower1 = characterIsLowerCaseLetter('a');  // true
 * bool isLower2 = characterIsLowerCaseLetter('A');  // false
 * bool isLower3 = characterIsLowerCaseLetter('z');  // true
 * bool isLower4 = characterIsLowerCaseLetter('5');  // false
 * bool isLower5 = characterIsLowerCaseLetter('.');  // false
 * @endcode
 */
bool characterIsLowerCaseLetter(uint32_t character);

/**
 * @brief Checks if a character is a digit or decimal point.
 * @param character Character to check.
 * @return true if digit or decimal point, false otherwise.
 * @ingroup OCString
 * 
 * @code
 * bool isDigitOrPoint1 = characterIsDigitOrDecimalPoint('0');  // true
 * bool isDigitOrPoint2 = characterIsDigitOrDecimalPoint('9');  // true
 * bool isDigitOrPoint3 = characterIsDigitOrDecimalPoint('.');  // true
 * bool isDigitOrPoint4 = characterIsDigitOrDecimalPoint('a');  // false
 * bool isDigitOrPoint5 = characterIsDigitOrDecimalPoint('-');  // false
 * @endcode
 */
bool characterIsDigitOrDecimalPoint(uint32_t character);

/**
 * @brief Checks if a character is a digit, decimal point, or space.
 * @param character Character to check.
 * @return true if digit, decimal point, or space, false otherwise.
 * @ingroup OCString
 * 
 * @code
 * bool isDigitPointSpace1 = characterIsDigitOrDecimalPointOrSpace('5');  // true
 * bool isDigitPointSpace2 = characterIsDigitOrDecimalPointOrSpace('.');  // true
 * bool isDigitPointSpace3 = characterIsDigitOrDecimalPointOrSpace(' ');  // true
 * bool isDigitPointSpace4 = characterIsDigitOrDecimalPointOrSpace('\\t'); // false (tab is not a space)
 * bool isDigitPointSpace5 = characterIsDigitOrDecimalPointOrSpace('a');  // false
 * @endcode
 */
bool characterIsDigitOrDecimalPointOrSpace(uint32_t character);


/**
 * @brief Creates an OCStringRef containing the current UTC date and time in ISO 8601 format.
 * @return A new OCStringRef with the current UTC timestamp in the format "YYYY-MM-DDTHH:MM:SSZ".
 *         The caller is responsible for releasing the returned OCStringRef.
 * @ingroup OCString
 *
 * Example:
 * @code
 * OCStringRef timestamp = OCCreateISO8601Timestamp();
 * // timestamp might contain "2024-06-30T23:45:12Z"
 * OCRelease(timestamp);
 * @endcode
 */
OCStringRef OCCreateISO8601Timestamp(void);


/** @} */ // end of OCString group

#endif /* OCString_h */

