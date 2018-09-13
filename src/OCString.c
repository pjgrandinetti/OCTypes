//
//  OCString.c
//  OCTypes
//
//  Created by philip on 5/10/17.
//  Copyright © 2017 Philip Grandinetti. All rights reserved.
//

#include "OCLibrary.h"

static OCTypeID kOCStringID = _kOCNotATypeID;

// OCString Opaque Type
struct __OCString {
    OCBase _base;

    // OCString Type attributes  - order of declaration is essential
    char *string;
    uint64_t length;
    uint64_t capacity;
};

static bool __OCStringEqual(const void * theType1, const void * theType2)
{
    OCStringRef theString1 = (OCStringRef) theType1;
    OCStringRef theString2 = (OCStringRef) theType2;
    if(theString1->_base.typeID != theString2->_base.typeID) return false;
    
    if(NULL == theString1 || NULL == theString2) return false;
    if(theString1 == theString2) return true;
    if(strlen(theString1->string) != strlen(theString2->string)) return false;
    if(strcmp(theString1->string, theString2->string)!=0) return false;
    return true;
}

static void __OCStringFinalize(const void * theType)
{
    if(NULL == theType) return;
    OCStringRef theString = (OCStringRef) theType;
    free(theString->string);
    free((void *)theString);
}

static OCStringRef __OCStringCopyFormattingDescription(OCTypeRef cf)
{
    return (OCStringRef) OCStringCreateCopy((OCStringRef)cf);
}

OCTypeID OCStringGetTypeID(void)
{
    if(kOCStringID == _kOCNotATypeID) kOCStringID = OCRegisterType("OCString");
    return kOCStringID;
}

static struct __OCString *OCStringAllocate()
{
    struct __OCString *theString = malloc(sizeof(struct __OCString));
    if(NULL == theString) return NULL;
    theString->_base.typeID = OCStringGetTypeID();
    theString->_base.retainCount = 1;
    theString->_base.finalize = __OCStringFinalize;
    theString->_base.equal = __OCStringEqual;
    theString->_base.copyFormattingDesc = __OCStringCopyFormattingDescription;
    theString->string = NULL;
    theString->length = 0;
    theString->capacity = 0;
    return theString;
}

OCStringRef OCStringCreateWithCString(const char *cString)
{
    if(NULL==cString) return NULL;
    struct __OCString *theString = OCStringAllocate();
    if(NULL == theString) return NULL;
    theString->length = strlen(cString);
    theString->capacity = strlen(cString);
    theString->string = malloc(theString->length+1);
    strcpy(theString->string, cString);
    return theString;
}

OCStringRef OCStringCreateCopy(OCStringRef theString)
{
    return OCStringCreateWithCString(theString->string);
}

OCMutableStringRef OCStringCreateMutable(uint64_t capacity)
{
    struct __OCString *theString = OCStringAllocate();
    if(NULL == theString) return NULL;
    theString->length = 0;
    theString->capacity = capacity;
    theString->string = malloc(capacity+1);
    theString->string[0] = 0;
    return theString;
}

OCMutableStringRef OCMutableStringCreateWithCString(const char *cString)
{
    return (OCMutableStringRef) OCStringCreateWithCString(cString);
}

OCMutableStringRef OCStringCreateMutableCopy(OCStringRef theString)
{
    return (OCMutableStringRef) OCStringCreateWithCString(theString->string);
}

OCStringRef OCStringCreateWithSubstring(OCStringRef str, OCRange range)
{
    char newString[range.length+1];
    for(int64_t index=range.location;index<range.location+range.length;index++) {
        newString[index-range.location] = str->string[index];
    }
    newString[range.length] = 0;
    return OCStringCreateWithCString(newString);
}

const char *OCStringGetCString(OCStringRef theString)
{
    return theString->string;
}

uint64_t OCStringGetLength(OCStringRef theString)
{
    return theString->length;
}

void OCStringShow(OCStringRef theString)
{
    if(theString==NULL) return;
    fprintf(stdout,"%s",OCStringGetCString(theString));
    fflush(stdout);
}

bool OCStringEqual(OCStringRef theString1, OCStringRef theString2)
{
    return __OCStringEqual(theString1, theString2);
}

void OCStringAppendCString(OCMutableStringRef theString, const char *cString)
{
    if(NULL==theString) return;
    if(NULL==cString) return;
    uint64_t length = strlen(cString) + theString->length;
    if(theString->capacity<length) theString->string = realloc(theString->string, length+1);
    theString->string = strcat(theString->string, cString);
    theString->capacity = length;
    theString->length = length;
}

static void __OCStringAppendBytes(OCMutableStringRef theString, const char *cString, uint64_t appendedLength)
{
    if(NULL==theString) return;
    if(NULL==cString) return;
    uint64_t length = appendedLength + theString->length;
    char *newString = malloc(length+1);

    for(uint64_t index = 0; index<theString->length; index++) {
        newString[index] =theString->string[index];
    }
    for(uint64_t index = 0; index<appendedLength; index++) {
        newString[index+theString->length] =cString[index];
    }
    newString[length] = 0;
    
    free(theString->string);
    
    theString->length = length;
    theString->string = newString;
}

void OCStringAppend(OCMutableStringRef theString, OCStringRef appendedString)
{
    if(appendedString->length==0) return;
    OCStringAppendCString(theString, appendedString->string);
}


void _OCStringAppendFormatAndArgumentsAux(OCMutableStringRef outputString,
                                          OCStringRef formatString,
                                          va_list args);

OCStringRef  OCStringCreateWithFormat(OCStringRef format, ...)
{
    va_list argList;
    va_start(argList, format);
    OCMutableStringRef result = OCStringCreateMutable(0);
    _OCStringAppendFormatAndArgumentsAux(result, format, argList);
    va_end(argList);
    return  (OCStringRef) result;
}

void OCStringAppendFormat(OCMutableStringRef theString, OCStringRef format, ...)
{
    va_list argList;
    va_start(argList, format);
    _OCStringAppendFormatAndArgumentsAux(theString, format, argList);
    va_end(argList);
}

void OCStringLowercase(OCMutableStringRef theString)
{
    int64_t currentIndex = 0;
    int64_t length;
    
    length = theString->length;
    
    char *contents = (char *)theString->string;
    for (;currentIndex < length;currentIndex++) {
        if (contents[currentIndex] >= 'A' && contents[currentIndex] <= 'Z') {
            contents[currentIndex] += 'a' - 'A';
        }
    }
}

void OCStringUppercase(OCMutableStringRef theString)
{
    int64_t currentIndex = 0;
    int64_t length;
    
    length = theString->length;
    
    char *contents = (char *)theString->string;
    for (;currentIndex < length;currentIndex++) {
        if (contents[currentIndex] >= 'a' && contents[currentIndex] <= 'z') {
            contents[currentIndex] -= 'a' - 'A';
        }
    }
}

void OCStringTrimWhitespace(OCMutableStringRef theString)
{
    uint64_t length = theString->length;
    if(length==0) return;
    
    uint64_t firstCharIndex = 0;
    for(uint64_t index = 0; index<length; index++) {
        if(theString->string[index] != ' ') {
            firstCharIndex = index;
            break;
        }
    }
    
    uint64_t lastCharIndex = length-1;
    for(uint64_t index = length-1; index>0; index--) {
        if(theString->string[index] != ' ') {
            lastCharIndex = index;
            break;
        }
    }
    
    uint64_t newLength = lastCharIndex - firstCharIndex + 1;
    
    char *buffer = malloc(newLength+1);
    uint64_t i = 0;
    for(uint64_t index = firstCharIndex; index<=lastCharIndex; index++) {
        buffer[i++] = theString->string[index];
    }
    buffer[newLength] = 0;
    free(theString->string);
    theString->string = buffer;
    theString->length = newLength;
}

char OCStringGetCharacterAtIndex(OCStringRef theString, uint64_t index)
{
    if(index>theString->length-1) return 0;
    return theString->string[index];
}

void OCStringDelete(OCMutableStringRef theString, OCRange range)
{
    uint64_t numberToMove = theString->length - range.location+range.length;
    
    for(uint64_t index = 0; index<numberToMove;index++) {
        theString->string[range.location+index] = theString->string[range.location+range.length+index];
    }
    theString->length -= range.length;
}

bool OCStringTrimMatchingParentheses(OCMutableStringRef theString)
{
    // This method eliminates superfluous parentheses from strings, e.g. ((4+5)) reduces to (4+5)
    uint64_t length = theString->length;
    if(length<2) return false;
    char firstCharacter =  theString->string[0];
    char lastCharacter =  theString->string[length-1];
    
    if(firstCharacter != '(' || lastCharacter != ')') return false;
    while(firstCharacter == '(' && lastCharacter == ')') {
        OCStringDelete(theString, OCRangeMake(length-1, 1));
        OCStringDelete(theString, OCRangeMake(0, 1));
        length = theString->length;
        OCStringTrimWhitespace(theString);
        firstCharacter =  theString->string[0];
        lastCharacter =  theString->string[length-1];
    }
    return true;
}


float complex OCStringGetFloatComplexValue(OCStringRef string)
{
    if(string==NULL) return nan("0");
    
    OCMutableStringRef  mutString = OCStringCreateMutableCopy (string);
    OCStringFindAndReplace2 (mutString,STR("•"), STR("*"));
    float complex result = OCComplexFromCString(mutString->string);
    OCRelease(mutString);
    return result;
}

double complex OCStringGetDoubleComplexValue(OCStringRef string)
{
    if(string==NULL) return nan("0");
    
    OCMutableStringRef  mutString = OCStringCreateMutableCopy (string);
    OCStringFindAndReplace2 (mutString,STR("•"), STR("*"));
    double complex result = OCComplexFromCString(mutString->string);
    OCRelease(mutString);
    return result;
}


void OCStringReplace(OCMutableStringRef str, OCRange range, OCStringRef replacement)
{
    uint64_t newLength = str->length+replacement->length-range.length;
    
    if(newLength>str->capacity) {
        str->string = realloc(str->string,newLength+1);
        str->capacity = newLength;
    }
    
    OCMutableStringRef copy = OCStringCreateMutableCopy(str);
    OCStringDelete(copy, range);
    for(uint64_t index = 0;index<replacement->length;index++) {
        copy->string[range.location+index] = replacement->string[index];
    }
    for(uint64_t index = range.location;index<str->length;index++) {
        copy->string[replacement->length+index] = str->string[index+range.length];
    }
    copy->string[newLength] = 0;
    memmove(str->string, copy->string, str->capacity+1);
    str->length = newLength;
    OCRelease(copy);
}

void OCStringReplaceAll(OCMutableStringRef str, OCStringRef replacement)
{
    OCStringReplace(str, OCRangeMake(0, str->length), replacement);
}

void OCStringInsert(OCMutableStringRef str, int64_t idx, OCStringRef insertedStr)
{
    OCStringReplace(str, OCRangeMake(idx, 0), insertedStr);
}



// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with, int64_t *count) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    size_t len_rep;  // length of rep (the string to remove)
    size_t len_with; // length of with (the string to replace rep with)
    long len_front; // distance between rep and end of last rep
    
    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);
    
    // count the number of replacements needed
    ins = orig;
    for ((*count) = 0; (tmp = strstr(ins, rep)); ++(*count)) {
        ins = tmp + len_rep;
    }
    
    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * (*count) + 1);
    
    if (!result)
        return NULL;
    
    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while ((*count)--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

int64_t OCStringFindAndReplace2(OCMutableStringRef theString,
                               OCStringRef stringToFind,
                               OCStringRef replacementString)
{
    int64_t count;
    
    char *result = str_replace(theString->string, stringToFind->string, replacementString->string, &count);
    free(theString->string);
    theString->string = result;
    theString->length = strlen(result);
    return count;
}

#define kOCStringStackBufferLength (64)
#define __kOCStringInlineBufferLength 64
typedef struct {
    char buffer[__kOCStringInlineBufferLength];
    OCStringRef theString;
    const char *directBuffer;
    OCRange rangeToBuffer;		/* Range in string to buffer */
    int64_t bufferedRangeStart;    /* Start of range currently buffered (relative to rangeToBuffer.location) */
    int64_t bufferedRangeEnd;		/* bufferedRangeStart + number of chars actually buffered */
} OCStringInlineBuffer;

OC_INLINE void OCStringInitInlineBuffer(OCStringRef str, OCStringInlineBuffer *buf, OCRange range) {
    buf->theString = str;
    buf->rangeToBuffer = range;
    buf->directBuffer = str->string;
    buf->bufferedRangeStart = buf->bufferedRangeEnd = 0;
}

void OCStringGetCharacters(OCStringRef str, OCRange range, char *buffer)
{
    const char *uContents = ((char *)str->string) + range.location;
    memmove(buffer, uContents, range.length * sizeof(char));
}

OC_INLINE char OCStringGetCharacterFromInlineBuffer(OCStringInlineBuffer *buf, int64_t idx) {
    if (buf->directBuffer) {
        if (idx < 0 || idx >= buf->rangeToBuffer.length) return 0;
        return buf->directBuffer[idx + buf->rangeToBuffer.location];
    }
    if (idx >= buf->bufferedRangeEnd || idx < buf->bufferedRangeStart) {
        if (idx < 0 || idx >= buf->rangeToBuffer.length) return 0;
        if ((buf->bufferedRangeStart = idx - 4) < 0) buf->bufferedRangeStart = 0;
        buf->bufferedRangeEnd = buf->bufferedRangeStart + __kOCStringInlineBufferLength;
        if (buf->bufferedRangeEnd > buf->rangeToBuffer.length) buf->bufferedRangeEnd = buf->rangeToBuffer.length;
        OCStringGetCharacters(buf->theString, OCRangeMake(buf->rangeToBuffer.location + buf->bufferedRangeStart, buf->bufferedRangeEnd - buf->bufferedRangeStart), buf->buffer);
    }
    return buf->buffer[idx - buf->bufferedRangeStart];
}


bool OCStringFindWithOptions(OCStringRef string, OCStringRef stringToFind, OCRange rangeToSearch, OCOptionFlags compareOptions, OCRange *result)
{
    uint64_t findStrLen = stringToFind->length;
    bool didFind = false;
    bool lengthVariants = ((compareOptions & (kOCCompareCaseInsensitive|kOCCompareNonliteral|kOCCompareDiacriticsInsensitiveCompatibilityMask)) ? true : false);
    
    if ((findStrLen > 0) && (rangeToSearch.length > 0) && ((findStrLen <= rangeToSearch.length) || lengthVariants)) {
        char strBuf1[kOCStringStackBufferLength];
        char strBuf2[kOCStringStackBufferLength];
        OCStringInlineBuffer inlineBuf1, inlineBuf2;
        const uint8_t *str1Bytes = (const uint8_t *) string->string;
        const uint8_t *str2Bytes = (const uint8_t *)stringToFind->string;
        int64_t fromLoc, toLoc;
        int64_t str1Index, str2Index;
        int64_t strBuf1Len, strBuf2Len;
        bool equalityOptions = ((lengthVariants || (compareOptions & kOCCompareWidthInsensitive)) ? true : false);
        bool caseInsensitive = ((compareOptions & kOCCompareCaseInsensitive) ? true : false);
        int8_t delta;
        
        OCStringInitInlineBuffer(string, &inlineBuf1, OCRangeMake(0, rangeToSearch.location + rangeToSearch.length));
        OCStringInitInlineBuffer(stringToFind, &inlineBuf2, OCRangeMake(0, findStrLen));
        
        if (compareOptions & kOCCompareBackwards) {
            fromLoc = rangeToSearch.location + rangeToSearch.length - (lengthVariants ? 1 : findStrLen);
            toLoc = (((compareOptions & kOCCompareAnchored) && !lengthVariants) ? fromLoc : rangeToSearch.location);
        } else {
            fromLoc = rangeToSearch.location;
            toLoc = ((compareOptions & kOCCompareAnchored) ? fromLoc : rangeToSearch.location + rangeToSearch.length - (lengthVariants ? 1 : findStrLen));
        }
        
        delta = ((fromLoc <= toLoc) ? 1 : -1);
        
        if ((NULL != str1Bytes) && (NULL != str2Bytes)) {
            int64_t maxStr1Index = (rangeToSearch.location + rangeToSearch.length);
            uint8_t str1Byte, str2Byte;
            
            while (1) {
                str1Index = fromLoc;
                str2Index = 0;
                
                while ((str1Index < maxStr1Index) && (str2Index < findStrLen)) {
                    str1Byte = str1Bytes[str1Index];
                    str2Byte = str2Bytes[str2Index];
                    
                    if (str1Byte != str2Byte) {
                        if (equalityOptions) {
                            if (caseInsensitive && (str1Byte >= 'A') && (str1Byte <= 'Z')) str1Byte += ('a' - 'A');
                            *strBuf1 = str1Byte;
                            strBuf1Len = 1;
                            
                            if (caseInsensitive && (str2Byte >= 'A') && (str2Byte <= 'Z')) str2Byte += ('a' - 'A');
                            *strBuf2 = str2Byte;
                            strBuf2Len = 1;
                            
                            if (*strBuf1 != *strBuf2) break;
                        } else {
                            break;
                        }
                    }
                    ++str1Index; ++str2Index;
                }
                
                if (str2Index == findStrLen) {
                    if (((kOCCompareBackwards|kOCCompareAnchored) != (compareOptions & (kOCCompareBackwards|kOCCompareAnchored))) || (str1Index == (rangeToSearch.location + rangeToSearch.length))) {
                        didFind = true;
                        if (NULL != result) *result = OCRangeMake(fromLoc, str1Index - fromLoc);
                    }
                    break;
                }
                
                if (fromLoc == toLoc) break;
                fromLoc += delta;
            }
        }
        else {
            while (1) {
                str1Index = fromLoc;
                str2Index = 0;
                
                while (str2Index < findStrLen) {
                    if (OCStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index) != OCStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index)) break;
                    
                    ++str1Index; ++str2Index;
                }
                
                if (str2Index == findStrLen) {
                    didFind = true;
                    if (NULL != result) *result = OCRangeMake(fromLoc, findStrLen);
                    break;
                }
                
                if (fromLoc == toLoc) break;
                fromLoc += delta;
            }
        }
    }
    
    return didFind;
}

static void __OCStringReplaceMultiple(OCMutableStringRef str, OCRange *ranges, int64_t numRanges, OCStringRef replacement)
{
    uint64_t newLength=str->length;
    for(int64_t iRange = 0; iRange<numRanges; iRange++) {
        newLength += replacement->length-ranges[iRange].length;
        
    }
    
    if(newLength>str->capacity) {
        str->string = realloc(str->string,newLength+1);
        str->capacity = newLength;
    }
    OCRange newRanges[numRanges];
    for(int64_t iRange = 0; iRange<numRanges; iRange++) {
        newRanges[iRange] = ranges[iRange];
    }
    OCMutableStringRef copy = OCStringCreateMutableCopy(str);
    for(int64_t iRange = 0; iRange<numRanges; iRange++) {
        OCStringDelete(copy, newRanges[iRange]);
        int64_t copyIndex = newRanges[iRange].location;
        for(uint64_t index = 0;index<replacement->length; index++) {
            copy->string[copyIndex++] = replacement->string[index];
        }
        for(uint64_t index = ranges[iRange].location+ranges[iRange].length;index<str->length;index++) {
            copy->string[copyIndex++] = str->string[index];
        }
        for(uint64_t jRange=iRange+1;jRange<numRanges; jRange++) {
            newRanges[jRange].location -= newRanges[iRange].length;
            newRanges[jRange].location += replacement->length;
        }
    }
    copy->string[newLength] = 0;
    memmove(str->string, copy->string, str->capacity+1);
    str->length = newLength;
    OCRelease(copy);
}

int64_t OCStringFindAndReplace(OCMutableStringRef string,
                               OCStringRef stringToFind,
                               OCStringRef replacementString,
                               OCRange rangeToSearch,
                               OCOptionFlags compareOptions)
{
    if(NULL==string) return 0;
    if(NULL==stringToFind) return 0;
    if(NULL==replacementString) return 0;
    if(rangeToSearch.location + rangeToSearch.length > string->length) return 0;
    
    OCRange foundRange;
    bool backwards = ((compareOptions & kOCCompareBackwards) != 0);
    uint64_t endIndex = rangeToSearch.location + rangeToSearch.length;
#define MAX_RANGES_ON_STACK (1000 / sizeof(OCRange))
    OCRange rangeBuffer[MAX_RANGES_ON_STACK];	// Used to avoid allocating memory
    OCRange *ranges = rangeBuffer;
    int64_t foundCount = 0;
    int64_t capacity = MAX_RANGES_ON_STACK;
    
    // Note: This code is very similar to the one in OCStringCreateArrayWithFindResults().
    while ((rangeToSearch.length > 0) && OCStringFindWithOptions(string, stringToFind, rangeToSearch, compareOptions, &foundRange)) {
        // Determine the next range
        if (backwards) {
            rangeToSearch.length = foundRange.location - rangeToSearch.location;
        } else {
            rangeToSearch.location = foundRange.location + foundRange.length;
            rangeToSearch.length = endIndex - rangeToSearch.location;
        }
        
        // If necessary, grow the array
        if (foundCount >= capacity) {
            bool firstAlloc = (ranges == rangeBuffer) ? true : false;
            capacity = (capacity + 4) * 2;
            // Note that reallocate with NULL previous pointer is same as allocate
            ranges = (OCRange *) realloc(firstAlloc ? NULL : ranges, capacity * sizeof(OCRange));
            if (firstAlloc) memmove(ranges, rangeBuffer, MAX_RANGES_ON_STACK * sizeof(OCRange));
        }
        ranges[foundCount] = foundRange;
        foundCount++;
    }
    
    if (foundCount > 0) {
        if (backwards) {	// Reorder the ranges to be incrementing (better to do this here, then to check other places)
            int64_t head = 0;
            int64_t tail = foundCount - 1;
            while (head < tail) {
                OCRange temp = ranges[head];
                ranges[head] = ranges[tail];
                ranges[tail] = temp;
                head++;
                tail--;
            }
        }
        __OCStringReplaceMultiple(string, ranges, foundCount, replacementString);
        if (ranges != rangeBuffer) free(ranges);
    }
    
    return foundCount;
}

OCRange OCStringFind(OCStringRef string, OCStringRef stringToFind, OCOptionFlags compareOptions) {
    OCRange foundRange;
    
    if (OCStringFindWithOptions(string, stringToFind, OCRangeMake(0, OCStringGetLength(string)), compareOptions, &foundRange)) {
        return foundRange;
    } else {
        return OCRangeMake(kOCNotFound, 0);
    }
}

// Functions to deal with special arrays of OCRange, OCDataRef, created by OCStringCreateArrayWithFindResults()

static const void *__rangeRetain(const void *ptr) {
    OCRetain(*(OCDataRef *)((uint8_t *)ptr + sizeof(OCRange)));
    return ptr;
}

static void __rangeRelease(const void *ptr) {
    OCRelease(*(OCDataRef *)((uint8_t *)ptr + sizeof(OCRange)));
}

static OCStringRef __rangeCopyDescription(const void *ptr) {
    OCRange range = *(OCRange *)ptr;
    return OCStringCreateWithFormat(STR("{%d, %d}"), range.location, range.length);
}

static bool	__rangeEqual(const void *ptr1, const void *ptr2) {
    OCRange range1 = *(OCRange *)ptr1;
    OCRange range2 = *(OCRange *)ptr2;
    return (range1.location == range2.location) && (range1.length == range2.length);
}

OCArrayRef OCStringCreateArrayWithFindResults(OCStringRef string,
                                              OCStringRef stringToFind,
                                              OCRange rangeToSearch,
                                              OCOptionFlags compareOptions)
{
    OCRange foundRange;
    bool backwards = ((compareOptions & kOCCompareBackwards) != 0);
    int64_t endIndex = rangeToSearch.location + rangeToSearch.length;
    OCMutableDataRef rangeStorage = NULL;	// Basically an array of OCRange, OCDataRef (packed)
    uint8_t *rangeStorageBytes = NULL;
    int64_t foundCount = 0;
    int64_t capacity = 0;		// Number of OCRange, OCDataRef element slots in rangeStorage
    
    while ((rangeToSearch.length > 0) && OCStringFindWithOptions(string, stringToFind, rangeToSearch, compareOptions, &foundRange)) {
        // Determine the next range
        if (backwards) {
            rangeToSearch.length = foundRange.location - rangeToSearch.location;
        } else {
            rangeToSearch.location = foundRange.location + foundRange.length;
            rangeToSearch.length = endIndex - rangeToSearch.location;
        }
        
        // If necessary, grow the data and squirrel away the found range
        if (foundCount >= capacity) {
            if (rangeStorage == NULL) rangeStorage = OCDataCreateMutable(0);
            capacity = (capacity + 4) * 2;
            OCDataSetLength(rangeStorage, capacity * (sizeof(OCRange) + sizeof(OCDataRef)));
            rangeStorageBytes = (uint8_t *)OCDataGetMutableBytePtr(rangeStorage) + foundCount * (sizeof(OCRange) + sizeof(OCDataRef));
        }
        memmove(rangeStorageBytes, &foundRange, sizeof(OCRange));	// The range
        memmove(rangeStorageBytes + sizeof(OCRange), &rangeStorage, sizeof(OCDataRef));	// The data
        rangeStorageBytes += (sizeof(OCRange) + sizeof(OCDataRef));
        foundCount++;
    }
    
    if (foundCount > 0) {
        OCMutableArrayRef array;
        const OCArrayCallBacks callbacks = {0, __rangeRetain, __rangeRelease, __rangeCopyDescription, __rangeEqual};
        
        OCDataSetLength(rangeStorage, foundCount * (sizeof(OCRange) + sizeof(OCDataRef)));	// Tighten storage up
        rangeStorageBytes = (uint8_t *)OCDataGetMutableBytePtr(rangeStorage);
        
        array = OCArrayCreateMutable(foundCount * sizeof(OCRange *), &callbacks);
        for (int64_t cnt = 0; cnt < foundCount; cnt++) {
            // Each element points to the appropriate CFRange in the CFData
            OCArrayAppendValue(array, rangeStorageBytes + cnt * (sizeof(OCRange) + sizeof(OCDataRef)));
        }
        OCRelease(rangeStorage);		// We want the data to go away when all CFRanges inside it are released...
        return array;
    } else {
        return NULL;
    }
}

OCArrayRef OCStringCreateArrayBySeparatingStrings(OCStringRef string, OCStringRef separatorString)
{
    OCArrayRef separatorRanges;
    int64_t length = OCStringGetLength(string);
    if (!(separatorRanges = OCStringCreateArrayWithFindResults(string, separatorString, OCRangeMake(0, length), 0))) {
        return OCArrayCreate((const void **)&string, 1, & kOCTypeArrayCallBacks);
    } else {
        int64_t idx;
        int64_t count = OCArrayGetCount(separatorRanges);
        int64_t startIndex = 0;
        int64_t numChars;
        OCMutableArrayRef array = OCArrayCreateMutable(count + 2, & kOCTypeArrayCallBacks);
        const OCRange *currentRange;
        OCStringRef substring;
        
        for (idx = 0;idx < count;idx++) {
            currentRange = (const OCRange *)OCArrayGetValueAtIndex(separatorRanges, idx);
            numChars = currentRange->location - startIndex;
            substring = OCStringCreateWithSubstring(string, OCRangeMake(startIndex, numChars));
            OCArrayAppendValue(array, substring);
            OCRelease(substring);
            startIndex = currentRange->location + currentRange->length;
        }
        substring = OCStringCreateWithSubstring(string, OCRangeMake(startIndex, length - startIndex));
        OCArrayAppendValue(array, substring);
        OCRelease(substring);
        
        OCRelease(separatorRanges);
        
        return array;
    }
}

OCComparisonResult OCStringCompare(OCStringRef theString1,
                                   OCStringRef theString2,
                                   OCStringCompareFlags compareOptions)
{
    int ret;
    if(compareOptions == kOCCompareCaseInsensitive)
        ret = strcasecmp(theString1->string, theString2->string);
    else ret = strcmp(theString1->string, theString2->string);

    if(ret>0) return kOCCompareGreaterThan;
    else if(ret<0) return kOCCompareLessThan;
    return kOCCompareEqualTo;
    
}

typedef uint16_t    UniChar;

typedef struct {
    int16_t size;
    int16_t type;
    int32_t loc;
    int32_t len;
    int32_t widthArg;
    int32_t precArg;
    uint32_t flags;
    int8_t mainArgNum;
    int8_t precArgNum;
    int8_t widthArgNum;
    int8_t unused1;
} OCFormatSpec;

typedef struct {
    int16_t type;
    int16_t size;
    union {
        int64_t int64Value;
        double doubleValue;
        void *pointerValue;
    } value;
} OCPrintValue;


enum {
    OCFormatLiteralType = 32,
    OCFormatLongType = 33,
    OCFormatDoubleType = 34,
    OCFormatPointerType = 35,
    OCFormatOCType = 37,		/* handled specially */
    OCFormatCharsType = 39,		/* handled specially */
    OCFormatPascalCharsType = 40,	/* handled specially */
    OCFormatDummyPointerType = 42	/* special case for %n */
};


enum {
    kOCStringFormatZeroFlag = (1 << 0),	// if not, padding is space char
    kOCStringFormatMinusFlag = (1 << 1),	// if not, no flag implied
    kOCStringFormatPlusFlag = (1 << 2),	// if not, no flag implied, overrides space
    kOCStringFormatSpaceFlag = (1 << 3)	// if not, no flag implied
};


enum {
    OCFormatDefaultSize = 0,
    OCFormatSize1 = 1,
    OCFormatSize2 = 2,
    OCFormatSize4 = 3,
    OCFormatSize8 = 4,
    OCFormatSize16 = 5,		/* unused */
};

OC_INLINE void __OCParseFormatSpec(const uint8_t *cformat, int32_t *fmtIdx, int64_t fmtLen, OCFormatSpec *spec) {
    bool seenDot = false;
    for (;;) {
        if (fmtLen <= *fmtIdx) return;	/* no type */
        char ch = cformat[(*fmtIdx)++];
    reswtch:switch (ch) {
        case '#':	// ignored for now
            break;
        case 0x20:
            if (!(spec->flags & kOCStringFormatPlusFlag)) spec->flags |= kOCStringFormatSpaceFlag;
            break;
        case '-':
            spec->flags |= kOCStringFormatMinusFlag;
            spec->flags &= ~kOCStringFormatZeroFlag;	// remove zero flag
            break;
        case '+':
            spec->flags |= kOCStringFormatPlusFlag;
            spec->flags &= ~kOCStringFormatSpaceFlag;	// remove space flag
            break;
        case '0':
            if (!(spec->flags & kOCStringFormatMinusFlag)) spec->flags |= kOCStringFormatZeroFlag;
            break;
        case 'h':
            spec->size = OCFormatSize2;
            break;
        case 'l':
            if (*fmtIdx < fmtLen) {
                // fetch next character, don't increment fmtIdx
                ch = cformat[(*fmtIdx)];
                if ('l' == ch) {	// 'll' for long long, like 'q'
                    (*fmtIdx)++;
                    spec->size = OCFormatSize8;
                    break;
                }
            }
            spec->size = OCFormatSize4;
            break;
        case 'q':
            spec->size = OCFormatSize8;
            break;
        case 'c':
            spec->type = OCFormatLongType;
            spec->size = OCFormatSize1;
            return;
        case 'O': case 'o': case 'D': case 'd': case 'i': case 'U': case 'u': case 'x': case 'X':
            spec->type = OCFormatLongType;
            return;
        case 'e': case 'E': case 'f': case 'g': case 'G':
            spec->type = OCFormatDoubleType;
            spec->size = OCFormatSize8;
            return;
        case 'n': case 'p':		/* %n is not handled correctly currently */
            spec->type = OCFormatPointerType;
            spec->size = OCFormatSize4;
            return;
        case 's':
            spec->type = OCFormatCharsType;
            spec->size = OCFormatSize4;
            return;
        case 'P':
            spec->type = OCFormatPascalCharsType;
            spec->size = OCFormatSize4;
            return;
        case '@':
            spec->type = OCFormatOCType;
            spec->size = OCFormatSize4;
            return;
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
            int64_t number = 0;
            do {
                number = 10 * number + (ch - '0');
                ch = cformat[(*fmtIdx)++];
            } while ((uint32_t)(ch - '0') <= 9);
            if ('$' == ch) {
                if (-2 == spec->precArgNum) {
                    spec->precArgNum = number - 1;	// Arg numbers start from 1
                } else if (-2 == spec->widthArgNum) {
                    spec->widthArgNum = number - 1;	// Arg numbers start from 1
                } else {
                    spec->mainArgNum = number - 1;	// Arg numbers start from 1
                }
                break;
            } else if (seenDot) {	/* else it's either precision or width */
                spec->precArg = (int32_t)number;
            } else {
                spec->widthArg = (int32_t)number;
            }
            goto reswtch;
        }
        case '*':
            spec->widthArgNum = -2;
            break;
        case '.':
            seenDot = true;
            ch = cformat[(*fmtIdx)++];
            if ('*' == ch) {
                spec->precArgNum = -2;
                break;
            }
            goto reswtch;
        default:
            spec->type = OCFormatLiteralType;
            return;
    }
    }
}

#define VPRINTF_BUFFER_LEN 61

#define SNPRINTF(TYPE, WHAT) {				\
TYPE value = (TYPE) WHAT;				\
if (-1 != specs[curSpec].widthArgNum) {		\
if (-1 != specs[curSpec].precArgNum) {		\
snprintf(buffer, 255, formatBuffer, width, precision, value); \
} else {					\
snprintf(buffer, 255, formatBuffer, width, value); \
}						\
} else {						\
if (-1 != specs[curSpec].precArgNum) {		\
snprintf(buffer, 255, formatBuffer, precision, value); \
} else {					\
snprintf(buffer, 255, formatBuffer, value);	\
}						\
}}


void _OCStringAppendFormatAndArgumentsAux(OCMutableStringRef outputString,
                                          OCStringRef formatString,
                                          va_list args)
{
    int32_t numSpecs, sizeSpecs, sizeArgNum, formatIdx, curSpec, argNum;
    int64_t formatLen;
    #define FORMAT_BUFFER_LEN 400
    const uint8_t *cformat = NULL;
    UniChar *formatChars = NULL;
    UniChar localFormatBuffer[FORMAT_BUFFER_LEN];
    
    #define VPRINTF_BUFFER_LEN 61
    OCFormatSpec localSpecsBuffer[VPRINTF_BUFFER_LEN];
    OCFormatSpec *specs;
    OCPrintValue localValuesBuffer[VPRINTF_BUFFER_LEN];
    OCPrintValue *values;
    
    intmax_t dummyLocation;	    // A place for %n to do its thing in; should be the widest possible int value
    
    numSpecs = 0;
    sizeSpecs = 0;
    sizeArgNum = 0;
    specs = NULL;
    values = NULL;
    
    formatLen = formatString->length;
    cformat = (const uint8_t *) formatString->string;
    
    /* Compute an upper bound for the number of format specifications */
    for (formatIdx = 0; formatIdx < formatLen; formatIdx++) if ('%' == cformat[formatIdx]) sizeSpecs++;
    
    specs = ((2 * sizeSpecs + 1) > VPRINTF_BUFFER_LEN) ? (OCFormatSpec *) malloc((2 * sizeSpecs + 1) * sizeof(OCFormatSpec)) : localSpecsBuffer;
    
    /* Collect format specification information from the format string */
    for (curSpec = 0, formatIdx = 0; formatIdx < formatLen; curSpec++) {
        int32_t newFmtIdx;
        specs[curSpec].loc = formatIdx;
        specs[curSpec].len = 0;
        specs[curSpec].size = 0;
        specs[curSpec].type = 0;
        specs[curSpec].flags = 0;
        specs[curSpec].widthArg = -1;
        specs[curSpec].precArg = -1;
        specs[curSpec].mainArgNum = -1;
        specs[curSpec].precArgNum = -1;
        specs[curSpec].widthArgNum = -1;
        for (newFmtIdx = formatIdx; newFmtIdx < formatLen && '%' != cformat[newFmtIdx]; newFmtIdx++);
        
        if (newFmtIdx != formatIdx) {	/* Literal chunk */
            specs[curSpec].type = OCFormatLiteralType;
            specs[curSpec].len = newFmtIdx - formatIdx;
        } else {
            newFmtIdx++;	/* Skip % */
            __OCParseFormatSpec(cformat, &newFmtIdx, formatLen, &(specs[curSpec]));
            if (OCFormatLiteralType == specs[curSpec].type) {
                specs[curSpec].loc = formatIdx + 1;
                specs[curSpec].len = 1;
            } else {
                specs[curSpec].len = newFmtIdx - formatIdx;
            }
        }
        formatIdx = newFmtIdx;
        
//         fprintf(stderr, "specs[%d] = {\n  size = %d,\n  type = %d,\n  loc = %d,\n  len = %d,\n  mainArgNum = %d,\n  precArgNum = %d,\n  widthArgNum = %d\n}\n", curSpec, specs[curSpec].size, specs[curSpec].type, specs[curSpec].loc, specs[curSpec].len, specs[curSpec].mainArgNum, specs[curSpec].precArgNum, specs[curSpec].widthArgNum);
        
    }
    numSpecs = curSpec;
    // Max of three args per spec, reasoning thus: 1 width, 1 prec, 1 value
    values = ((3 * sizeSpecs + 1) > VPRINTF_BUFFER_LEN) ? (OCPrintValue *) malloc((3 * sizeSpecs + 1) * sizeof(OCPrintValue)) : localValuesBuffer;
    memset(values, 0, (3 * sizeSpecs + 1) * sizeof(OCPrintValue));
    sizeArgNum = (3 * sizeSpecs + 1);
    
    /* Compute values array */
    argNum = 0;
    for (curSpec = 0; curSpec < numSpecs; curSpec++) {
        int32_t newMaxArgNum;
        if (0 == specs[curSpec].type) continue;
        if (OCFormatLiteralType == specs[curSpec].type) continue;
        newMaxArgNum = sizeArgNum;
        if (newMaxArgNum < specs[curSpec].mainArgNum) {
            newMaxArgNum = specs[curSpec].mainArgNum;
        }
        if (newMaxArgNum < specs[curSpec].precArgNum) {
            newMaxArgNum = specs[curSpec].precArgNum;
        }
        if (newMaxArgNum < specs[curSpec].widthArgNum) {
            newMaxArgNum = specs[curSpec].widthArgNum;
        }
        if (sizeArgNum < newMaxArgNum) {
            if (specs != localSpecsBuffer) free(specs);
            if (values != localValuesBuffer) free(values);
            if (formatChars && (formatChars != localFormatBuffer)) free(formatChars);
            return;  // more args than we expected!
        }
        /* It is actually incorrect to reorder some specs and not all; we just do some random garbage here */
        if (-2 == specs[curSpec].widthArgNum) {
            specs[curSpec].widthArgNum = argNum++;
        }
        if (-2 == specs[curSpec].precArgNum) {
            specs[curSpec].precArgNum = argNum++;
        }
        if (-1 == specs[curSpec].mainArgNum) {
            specs[curSpec].mainArgNum = argNum++;
        }
        values[specs[curSpec].mainArgNum].size = specs[curSpec].size;
        values[specs[curSpec].mainArgNum].type = specs[curSpec].type;
        if (-1 != specs[curSpec].widthArgNum) {
            values[specs[curSpec].widthArgNum].size = 0;
            values[specs[curSpec].widthArgNum].type = OCFormatLongType;
        }
        if (-1 != specs[curSpec].precArgNum) {
            values[specs[curSpec].precArgNum].size = 0;
            values[specs[curSpec].precArgNum].type = OCFormatLongType;
        }
        
    }
    
    /* Collect the arguments in correct type from vararg list */
    for (argNum = 0; argNum < sizeArgNum; argNum++) {
        switch (values[argNum].type) {
            case 0:
            case OCFormatLiteralType:
                break;
            case OCFormatLongType:
                if (OCFormatSize1 == values[argNum].size) {
                    values[argNum].value.int64Value = (int64_t)(int8_t)va_arg(args, int);
                } else if (OCFormatSize2 == values[argNum].size) {
                    values[argNum].value.int64Value = (int64_t)(int16_t)va_arg(args, int);
                } else if (OCFormatSize4 == values[argNum].size) {
                    values[argNum].value.int64Value = (int64_t)va_arg(args, int32_t);
                } else if (OCFormatSize8 == values[argNum].size) {
                    values[argNum].value.int64Value = (int64_t)va_arg(args, int64_t);
                } else {
                    values[argNum].value.int64Value = (int64_t)va_arg(args, int);
                }
                break;
            case OCFormatDoubleType:
                    values[argNum].value.doubleValue = va_arg(args, double);
                break;
            case OCFormatPointerType:
            case OCFormatOCType:
            case OCFormatCharsType:
            case OCFormatPascalCharsType:
                values[argNum].value.pointerValue = va_arg(args, void *);
                break;
            case OCFormatDummyPointerType:
                (void)va_arg(args, void *);	    // Skip the provided argument
                values[argNum].value.pointerValue = &dummyLocation;
                break;
        }
    }
    va_end(args);
    
    /* Format the pieces together */
    for (curSpec = 0; curSpec < numSpecs; curSpec++) {
        int32_t width = 0, precision = 0;
        bool hasWidth = false, hasPrecision = false;
        
        // widthArgNum and widthArg are never set at the same time; same for precArg*
        if (-1 != specs[curSpec].widthArgNum) {
            width = (int32_t)values[specs[curSpec].widthArgNum].value.int64Value;
            hasWidth = true;
        }
        if (-1 != specs[curSpec].precArgNum) {
            precision = (int32_t)values[specs[curSpec].precArgNum].value.int64Value;
            hasPrecision = true;
        }
        if (-1 != specs[curSpec].widthArg) {
            width = specs[curSpec].widthArg;
            hasWidth = true;
        }
        if (-1 != specs[curSpec].precArg) {
            precision = specs[curSpec].precArg;
            hasPrecision = true;
        }
        
        switch (specs[curSpec].type) {
            case OCFormatLongType:
            case OCFormatDoubleType:
            case OCFormatPointerType: {
                char formatBuffer[128];
                char buffer[256 + width + precision];
                int32_t cidx, idx, loc;
                bool appended = false;
                loc = specs[curSpec].loc;
                // In preparation to call snprintf(), copy the format string out
                
                for (idx = 0, cidx = 0; cidx < specs[curSpec].len; idx++, cidx++) {
                    if ('$' == cformat[loc + cidx]) {
                        for (idx--; '0' <= formatBuffer[idx] && formatBuffer[idx] <= '9'; idx--);
                    } else {
                        formatBuffer[idx] = cformat[loc + cidx];
                    }
                }
                
                formatBuffer[idx] = '\0';
                // Should modify format buffer here if necessary; for example, to translate %qd to
                // the equivalent, on architectures which do not have %q.
                buffer[sizeof(buffer) - 1] = '\0';
                switch (specs[curSpec].type) {
                    case OCFormatLongType:
                        if (OCFormatSize8 == specs[curSpec].size) {
                            SNPRINTF(int64_t, values[specs[curSpec].mainArgNum].value.int64Value)
                        } else {
                            SNPRINTF(int32_t, values[specs[curSpec].mainArgNum].value.int64Value)
                        }
                        break;
                    case OCFormatPointerType:
                    case OCFormatDummyPointerType:
                        SNPRINTF(void *, values[specs[curSpec].mainArgNum].value.pointerValue)
                        break;
                        
                    case OCFormatDoubleType:
                        {
                            SNPRINTF(double, values[specs[curSpec].mainArgNum].value.doubleValue)
                        }
                        break;
                }
                if (!appended) OCStringAppendCString(outputString, (const char *)buffer);
            }
                break;
            case OCFormatLiteralType:
                __OCStringAppendBytes(outputString, (const char *)(cformat+specs[curSpec].loc), specs[curSpec].len);
                break;
            case OCFormatPascalCharsType:
            case OCFormatCharsType: {
                long mainArgNum = specs[curSpec].mainArgNum;
                void *pointerValue = values[mainArgNum].value.pointerValue;
                if (pointerValue == NULL) {
                    OCStringAppendCString(outputString, "(null)");
                } else {
                    int len;
                    const char *str = (const char *)values[specs[curSpec].mainArgNum].value.pointerValue;
                    if (specs[curSpec].type == OCFormatPascalCharsType) {    // Pascal string case
                        len = ((unsigned char *)str)[0];
                        str++;
                        if (hasPrecision && precision < len) len = precision;
                    } else {    // C-string case
                        if (!hasPrecision) {    // No precision, so rely on the terminating null character
                            len = (OCIndex)strlen(str);
                        } else {    // Don't blindly call strlen() if there is a precision; the string might not have a terminating null (3131988)
                            const char *terminatingNull = (const char *)memchr(str, 0, precision);    // Basically strlen() on only the first precision characters of str
                            if (terminatingNull) {    // There was a null in the first precision characters
                                len = (int) (terminatingNull - str);
                            } else {
                                len = precision;
                            }
                        }
                    }
                    // Since the spec says the behavior of the ' ', '0', '#', and '+' flags is undefined for
                    // '%s', and since we have ignored them in the past, the behavior is hereby cast in stone
                    // to ignore those flags (and, say, never pad with '0' instead of space).
                    if (specs[curSpec].flags & kOCStringFormatMinusFlag) {
                        __OCStringAppendBytes(outputString, str, len);
                        if (hasWidth && width > len) {
                            int w = width - len;    // We need this many spaces; do it ten at a time
                            do {__OCStringAppendBytes(outputString, "          ", (w > 10 ? 10 : w));} while ((w -= 10) > 0);
                        }
                    } else {
                        if (hasWidth && width > len) {
                            int w = width - len;    // We need this many spaces; do it ten at a time
                            do {__OCStringAppendBytes(outputString, "          ", (w > 10 ? 10 : w));} while ((w -= 10) > 0);
                        }
                        __OCStringAppendBytes(outputString, str, len);
                    }
                }
            }
                break;
            case OCFormatOCType:
                if (NULL != values[specs[curSpec].mainArgNum].value.pointerValue) {
                    OCTypeRef theType = (OCTypeRef) values[specs[curSpec].mainArgNum].value.pointerValue;
                    OCStringRef str = NULL;
                    str = OCTypeCopyFormattingDesc(theType);
                    if (NULL == str) {
                        str = OCCopyDescription(values[specs[curSpec].mainArgNum].value.pointerValue);
                    }
                    if (str) {
                        OCStringAppendCString(outputString, str->string);
                        OCRelease(str);
                    } else {
                        OCStringAppendCString(outputString, "(null description)");
                    }
                } else {
                    OCStringAppendCString(outputString, "(null)");
                }
                break;
        }
    }
    
    if (specs != localSpecsBuffer) free(specs);
    if (values != localValuesBuffer) free(values);
    if (formatChars && (formatChars != localFormatBuffer)) free(formatChars);
}

typedef const struct __OCDictionary * OCDictionaryRef;
typedef struct __OCDictionary * OCMutableDictionaryRef;
extern OCMutableDictionaryRef OCDictionaryCreateMutable(uint64_t capacity);
extern bool OCDictionaryContainsKey(OCDictionaryRef theDictionary, OCStringRef key);
extern void OCDictionaryAddValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value);
extern const void * OCDictionaryGetValue(OCDictionaryRef theDictionary, OCStringRef key);

static OCMutableDictionaryRef constantStringTable = NULL;

OCStringRef __OCStringMakeConstantString(const char *cStr)
{
    OCStringRef result = OCStringCreateWithCString(cStr);
    
    if (constantStringTable == NULL) {
        constantStringTable = OCDictionaryCreateMutable(0);
    }
    if(OCDictionaryContainsKey(constantStringTable, result)) {
        OCStringRef existingResult = (OCStringRef) OCDictionaryGetValue(constantStringTable, result);
        OCRelease(result);
        return existingResult;
    }
    else {
        OCDictionaryAddValue(constantStringTable, result, result);
        OCMutableStringRef temp = (OCMutableStringRef) result;
        temp->_base.retainCount = 0;
    }
    return result;
}


OC_INLINE bool __OCIsWhitespace(char theChar) {
    return ((theChar < 0x21)) ? true : false;
}

#define __kOCStringInlineBufferLength 64

OC_INLINE char __OCStringGetCharacterFromInlineBufferAux(OCStringInlineBuffer *buf, int64_t idx) {
    if (buf->directBuffer) {
        if (idx < 0 || idx >= buf->rangeToBuffer.length) return 0xFF;
        return buf->directBuffer[idx + buf->rangeToBuffer.location];
    }
    if (idx >= buf->bufferedRangeEnd || idx < buf->bufferedRangeStart) {
        if (idx < 0 || idx >= buf->rangeToBuffer.length) return 0xFF;
        if ((buf->bufferedRangeStart = idx - 4) < 0) buf->bufferedRangeStart = 0;
        buf->bufferedRangeEnd = buf->bufferedRangeStart + __kOCStringInlineBufferLength;
        if (buf->bufferedRangeEnd > buf->rangeToBuffer.length) buf->bufferedRangeEnd = buf->rangeToBuffer.length;
        OCStringGetCharacters(buf->theString, OCRangeMake(buf->rangeToBuffer.location + buf->bufferedRangeStart, buf->bufferedRangeEnd - buf->bufferedRangeStart), buf->buffer);
    }
    return buf->buffer[idx - buf->bufferedRangeStart];
}

OC_INLINE char __OCStringGetFirstNonSpaceCharacterFromInlineBuffer(OCStringInlineBuffer *buf, int32_t *indexPtr) {
    UniChar ch;
    while (__OCIsWhitespace(ch = __OCStringGetCharacterFromInlineBufferAux(buf, *indexPtr))) (*indexPtr)++;
    return ch;
}

// Packed array of Boolean
static const unsigned char __OCNumberSet[16] = {
    0X00, // 0, 0, 0, 0, 0, 0, 0, 0, //  nul soh stx etx eot enq ack bel
    0X00, // 0, 0, 0, 0, 0, 0, 0, 0, //  bs  ht  nl  vt  np  cr  so  si
    0X00, // 0, 0, 0, 0, 0, 0, 0, 0, //  dle dc1 dc2 dc3 dc4 nak syn etb
    0X00, // 0, 0, 0, 0, 0, 0, 0, 0, //  can em  sub esc fs  gs  rs  us
    0X00, // 0, 0, 0, 0, 0, 0, 0, 0, //  sp   !   "   #   $   %   &   '
    0X28, // 0, 0, 0, 1, 0, 1, 0, 0, //  (   )   *   +   ,   -   .   /
    0xFF, // 1, 1, 1, 1, 1, 1, 1, 1, //  0   1   2   3   4   5   6   7
    0X03, // 1, 1, 0, 0, 0, 0, 0, 0, //  8   9   :   ;   <   =   >   ?
    0X20, // 0, 0, 0, 0, 0, 1, 0, 0, //  @   A   B   C   D   E   F   G
    0X00, // 0, 0, 0, 0, 0, 0, 0, 0, //  H   I   J   K   L   M   N   O
    0X00, // 0, 0, 0, 0, 0, 0, 0, 0, //  P   Q   R   S   T   U   V   W
    0X00, // 0, 0, 0, 0, 0, 0, 0, 0, //  X   Y   Z   [   \   ]   ^   _
    0X20, // 0, 0, 0, 0, 0, 1, 0, 0, //  `   a   b   c   d   e   f   g
    0X00, // 0, 0, 0, 0, 0, 0, 0, 0, //  h   i   j   k   l   m   n   o
    0X00, // 0, 0, 0, 0, 0, 0, 0, 0, //  p   q   r   s   t   u   v   w
    0X00, // 0, 0, 0, 0, 0, 0, 0, 0  //  x   y   z   {   |   }   ~  del
};


__private_extern__ bool __OCStringScanDouble(OCStringInlineBuffer *buf, OCTypeRef locale, int32_t *indexPtr, double *resultPtr) {
#define STACK_BUFFER_SIZE 256
#define ALLOC_CHUNK_SIZE 256 // first and subsequent malloc size.  Should be greater than STACK_BUFFER_SIZE
    char localCharBuffer[STACK_BUFFER_SIZE];
    char *charPtr = localCharBuffer;
    char *endCharPtr;
    char decimalChar = '.';
    int32_t numChars = 0;
    int32_t capacity = STACK_BUFFER_SIZE;	// in chars
    double result;
    char ch;
    
    ch = __OCStringGetFirstNonSpaceCharacterFromInlineBuffer(buf, indexPtr);
    // At this point indexPtr points at the first non-space char
    do {
        if ((__OCNumberSet[ch >> 3] & (1 << (ch & 7))) == 0) {
            // Not in __CFNumberSet
            if (ch != decimalChar) break;
            ch = '.';	// Replace the decimal character with something strtod will understand
        }
        if (numChars >= capacity - 1) {
            capacity += ALLOC_CHUNK_SIZE;
            if (charPtr == localCharBuffer) {
                charPtr = (char *) malloc(capacity * sizeof(char));
                memmove(charPtr, localCharBuffer, numChars * sizeof(char));
            } else {
                charPtr = (char *)realloc(charPtr, capacity * sizeof(char));
            }
        }
        charPtr[numChars++] = (char)ch;
        ch = __OCStringGetCharacterFromInlineBufferAux(buf, *indexPtr + numChars);
    } while (true);
    charPtr[numChars] = 0;	// Null byte for strtod
    
#if DEPLOYMENT_TARGET_MACOSX
    result = strtod_l(charPtr, &endCharPtr, NULL);
#else
    result = strtod(charPtr, &endCharPtr);
#endif
    
    if (charPtr == endCharPtr) return false;
    *indexPtr += (endCharPtr - charPtr);
    if (resultPtr) *resultPtr = result; // only store result if we succeed
    
    return true;
}

double OCStringGetDoubleValue(OCStringRef str) {
    bool success;
    double result;
    int32_t idx = 0;
    OCStringInlineBuffer buf;
    OCStringInitInlineBuffer(str, &buf, OCRangeMake(0, OCStringGetLength(str)));
    success = __OCStringScanDouble(&buf, NULL, &idx, &result);
    return success ? result : 0.0;
}


OCStringRef OCFloatComplexCreateStringValue(float complex value, OCStringRef format)
{
    if(value==0.0) return STR("0");
    OCStringRef realString = OCDoubleCreateStringValue(crealf(value));
    OCStringRef imagString = OCDoubleCreateStringValue(cimagf(value));
    
    OCMutableStringRef imagStringI = OCStringCreateMutableCopy(imagString);
    OCStringAppend(imagStringI, STR("•I"));
    OCRelease(imagString);
    
    if(cimagf(value)==0.0) {
        OCRelease(imagStringI);
        return realString;
    }
    
    if(crealf(value)==0.0) {
        OCRelease(realString);
        return imagStringI;
    }
    
    OCMutableStringRef combined = OCStringCreateMutableCopy(realString);
    if(cimag(value)>0) OCStringAppend(combined, STR("+"));
    OCStringAppend(combined, imagStringI);
    OCRelease(realString);
    OCRelease(imagStringI);
    return combined;
}


OCStringRef OCDoubleCreateStringValue(double value)
{
    if(value==0.0) return STR("0");
    
    // This is a weird trick and not sure if it's the best solution to an impossible problem.
    // We need to turn float point representations into strings that are the true number.
    // No one wants to see 0.600000000085 when the answer is supposed to be 0.6
    // The approach taken here is to determine the string for a range of conversion of formats
    // starting at %.16lg, %.15lg, %.14lg,  %.13lg,  %.12lg,  %.11lg, ...
    // and look for the string where the string length dramatically drops.
    // that is, look for the highest conversion format that turns 0.600000000085 into 0.6.
    //
    // If we don't see a big drop in string length then %.16lg should be fine.
    // When we do see a big drop in string length, then reconvert the shorter string
    // back into a number and calculate the relative error.  If the relative error
    // is less than 1e-13 then accept the new string.  Otherwise return the string
    // obtained with %.16lg.
    
    
    OCStringRef format = STR("%.16lg");
    int64_t highest = 16;
    int64_t lowest = 4;
    OCStringRef string = OCStringCreateWithFormat(format,value);
    int64_t lastlength = OCStringGetLength(string);
    OCRelease(string);
    
    for(int64_t index=highest;index>lowest;index--) {
        format = OCStringCreateWithFormat(STR("Q.%ldlg"),index);
        OCMutableStringRef copy = OCStringCreateMutableCopy(format);
        OCRelease(format);
        OCStringReplace(copy, OCRangeMake(0, 1), STR("%"));
        string = OCStringCreateWithFormat(copy,value);
        int64_t lengthDifference = lastlength - OCStringGetLength(string);
        lastlength = OCStringGetLength(string);
        double newValue = OCStringGetDoubleValue(string);
        double error = fabs(newValue - value)/value;
        
        if(lengthDifference>lowest&&error<1.e-13) {
            OCRelease(copy);
            //            printf("possible relative error of %lg\n",error);
            return string;
        }
        OCRelease(string);
        OCRelease(copy);
    }
    return OCStringCreateWithFormat(STR("%.16lg"),value);
}

OCStringRef PSFloatCreateStringValue(float value)
{
    if(value==0.0) return STR("0");
    
    OCStringRef format = STR("%.7g");
    int64_t highest = 7;
    int64_t lowest = 4;
    OCStringRef string = OCStringCreateWithFormat(format,value);
    int64_t lastlength = OCStringGetLength(string);
    OCRelease(string);
    
    for(int64_t index=highest;index>lowest;index--) {
        format = OCStringCreateWithFormat(STR("Q.%ldg"),index);
        OCMutableStringRef copy = OCStringCreateMutableCopy(format);
        OCRelease(format);
        OCStringReplace(copy, OCRangeMake(0, 1), STR("%"));
        string = OCStringCreateWithFormat(copy,value);
        int64_t lengthDifference = lastlength - OCStringGetLength(string);
        lastlength = OCStringGetLength(string);
        float newValue = OCStringGetDoubleValue(string);
        float error = fabs(newValue - value)/value;
        if(lengthDifference>lowest&&error<1.e-5) {
            OCRelease(copy);
            return string;
        }
        OCRelease(string);
        OCRelease(copy);
    }
    
    return OCStringCreateWithFormat(STR("%.7g"),value);
}


OCStringRef PSDoubleComplexCreateStringValue(double complex value,OCStringRef format)
{
    if(value==0.0) return STR("0");
    OCStringRef realString = OCDoubleCreateStringValue(creal(value));
    OCStringRef imagString = OCDoubleCreateStringValue(cimag(value));
    
    OCMutableStringRef imagStringI = OCStringCreateMutableCopy(imagString);
    OCStringAppend(imagStringI, STR("•I"));
    OCRelease(imagString);
    
    if(cimag(value)==0.0) {
        OCRelease(imagStringI);
        return realString;
    }
    
    if(creal(value)==0.0) {
        OCRelease(realString);
        return imagStringI;
    }
    
    OCMutableStringRef combined = OCStringCreateMutableCopy(realString);
    if(cimag(value)>0) OCStringAppend(combined, STR("+"));
    OCStringAppend(combined, imagStringI);
    OCRelease(realString);
    OCRelease(imagStringI);
    return combined;
}

bool characterIsUpperCaseLetter(char character)
{
    if(character>='A'&&character<='Z') return true;
    return false;
}

bool characterIsLowerCaseLetter(char character)
{
    if(character>='a'&&character<='z') return true;
    return false;
}

bool characterIsDigitOrDecimalPoint(char character)
{
    if(character>='0'&&character<='9') return true;
    if(character=='.') return true;
    return false;
}

bool characterIsDigitOrDecimalPointOrSpace(char character)
{
    if(character>='0'&&character<='9') return true;
    if(character=='.' || character==' ') return true;
    return false;
}



