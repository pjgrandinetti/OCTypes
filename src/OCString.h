//
//  OCString.h
//  OCTypes
//
//  Created by philip on 5/10/17.
//

#ifndef OCString_h
#define OCString_h

#include "OCLibrary.h"

typedef const struct __OCString * OCStringRef;
typedef struct __OCString * OCMutableStringRef;

#define STR(cStr)  __OCStringMakeConstantString("" cStr "")

OCTypeID OCStringGetTypeID(void);

OCStringRef OCStringCreateWithCString(const char *string);
OCMutableStringRef OCStringCreateMutableCopy(OCStringRef theString);
OCStringRef OCStringCreateWithSubstring(OCStringRef str, OCRange range);

void OCStringAppend(OCMutableStringRef theString, OCStringRef appendedString);
void OCStringAppendCString(OCMutableStringRef theString, const char *cString);

OCMutableStringRef OCStringCreateMutable(uint64_t capacity);
OCMutableStringRef OCMutableStringCreateWithCString(const char *cString);
const char *OCStringGetCString(OCStringRef theString);
OCStringRef OCStringCreateCopy(OCStringRef theString);
OCComparisonResult OCStringCompare(OCStringRef theString1, OCStringRef theString2, OCStringCompareFlags compareOptions);
uint64_t OCStringGetLength(OCStringRef theString);
void OCStringLowercase(OCMutableStringRef theString);
void OCStringUppercase(OCMutableStringRef theString);
void OCStringTrimWhitespace(OCMutableStringRef theString);
bool OCStringTrimMatchingParentheses(OCMutableStringRef theString);

void OCStringDelete(OCMutableStringRef theString, OCRange range);
void OCStringInsert(OCMutableStringRef str, int64_t idx, OCStringRef insertedStr);
void OCStringReplace(OCMutableStringRef str, OCRange range, OCStringRef replacement);
void OCStringReplaceAll(OCMutableStringRef str, OCStringRef replacement);

char OCStringGetCharacterAtIndex(OCStringRef theString, uint64_t index);
float complex OCStringGetFloatComplexValue(OCStringRef string);
double complex OCStringGetDoubleComplexValue(OCStringRef string);

OCRange OCStringFind(OCStringRef string, OCStringRef stringToFind, OCOptionFlags compareOptions);

int64_t OCStringFindAndReplace2(OCMutableStringRef string,
                               OCStringRef stringToFind,
                               OCStringRef replacementString);
int64_t OCStringFindAndReplace(OCMutableStringRef string,
                               OCStringRef stringToFind,
                               OCStringRef replacementString,
                               OCRange rangeToSearch,
                               OCOptionFlags compareOptions);

typedef const struct __OCArray * OCArrayRef;

OCArrayRef OCStringCreateArrayWithFindResults(OCStringRef string, OCStringRef stringToFind, OCRange rangeToSearch, OCOptionFlags compareOptions);
OCArrayRef OCStringCreateArrayBySeparatingStrings(OCStringRef string, OCStringRef separatorString);

void OCStringShow(OCStringRef theString);
bool OCStringEqual(OCStringRef theString1, OCStringRef theString2);

OCStringRef  OCStringCreateWithFormat(OCStringRef format, ...);
void OCStringAppendFormat(OCMutableStringRef theString, OCStringRef format, ...);


OCStringRef __OCStringMakeConstantString(const char *cStr); 	/* Private; do not use */

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
double complex OCComplexFromCString(const char *string);


OCStringRef PSFloatCreateStringValue(float value);
OCStringRef OCDoubleCreateStringValue(double value);
OCStringRef OCFloatComplexCreateStringValue(float complex value, OCStringRef format);
OCStringRef PSDoubleComplexCreateStringValue(double complex value,OCStringRef format);


bool characterIsUpperCaseLetter(char character);
bool characterIsLowerCaseLetter(char character);
bool characterIsDigitOrDecimalPoint(char character);
bool characterIsDigitOrDecimalPointOrSpace(char character);

#endif /* OCString_h */
