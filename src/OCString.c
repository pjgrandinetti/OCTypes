#include <stdio.h>
#include <stdlib.h>   // malloc, free, realloc
#include <string.h>   // strlen, strcmp, memcpy, memmove
#include <stdint.h>   // uint32_t
#include <stddef.h>   // ptrdiff_t, size_t
#include <complex.h>
#include <math.h>
#include "OCString.h"
#include "OCDictionary.h"   // for OCMutableDictionaryRef, OCDictionaryCreateMutable, etc.

// Callbacks for OCArray containing OCRange structs
static void __OCRangeReleaseCallBack(const void *value) {
    if (value) {
        free((void *)value);
    }
}
// NOP retain callback for OCRange objects, as they are simple malloc'd structs
// and OCArrayAppendValue might attempt to call a retain callback.
static const void *__OCRangeNopRetainCallBack(const void *value) {
    return value; // OCRanges are not further reference counted themselves by this callback
}

static const OCArrayCallBacks kOCRangeArrayCallBacks = {
    0, // version
    __OCRangeNopRetainCallBack, // retain
    __OCRangeReleaseCallBack, // release
    NULL, // copyDescription
    NULL  // equal
};



// ——— UTF-8 iterator: decode next code-point and advance pointer ———
static uint32_t utf8_next(const char **p) {
    const unsigned char *s = (const unsigned char *)*p;
    uint32_t ch;
    if (s[0] < 0x80) {
        // 1-byte ASCII
        ch = s[0];
        *p += 1;
    } else if ((s[0] & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) {
        // 2-byte sequence
        ch = ((s[0] & 0x1F) << 6) |
              (s[1] & 0x3F);
        *p += 2;
    } else if ((s[0] & 0xF0) == 0xE0 &&
               (s[1] & 0xC0) == 0x80 &&
               (s[2] & 0xC0) == 0x80) {
        // 3-byte sequence
        ch = ((s[0] & 0x0F) << 12) |
             ((s[1] & 0x3F) <<  6) |
              (s[2] & 0x3F);
        *p += 3;
    } else if ((s[0] & 0xF8) == 0xF0 &&
               (s[1] & 0xC0) == 0x80 &&
               (s[2] & 0xC0) == 0x80 &&
               (s[3] & 0xC0) == 0x80) {
        // 4-byte sequence
        ch = ((s[0] & 0x07) << 18) |
             ((s[1] & 0x3F) << 12) |
             ((s[2] & 0x3F) <<  6) |
              (s[3] & 0x3F);
        *p += 4;
    } else {
        // Invalid UTF-8: emit U+FFFD and skip one byte
        ch = 0xFFFD;
        *p += 1;
    }
    return ch;
}

// ——— Map a code-point index to its byte offset in the UTF-8 string ———
static ptrdiff_t oc_utf8_offset_for_index(const char *s, size_t idx) {
    const char *p = s;
    size_t count = 0;
    while (*p) {
        if (count == idx) {
            return p - s;
        }
        utf8_next(&p);
        count++;
    }
    // Allow pointing just past the last code-point
    return (idx == count) ? (p - s) : -1;
}

// ——— Count the number of Unicode code-points in a UTF-8 string ———
static size_t oc_utf8_strlen(const char *s) {
    const char *p = s;
    size_t len = 0;
    while (*p) {
        utf8_next(&p);
        len++;
    }
    return len;
}


// ——— Helper: replace all occurrences of a C‐string ———
// Returns a newly malloc’d buffer, and sets *count to the number of replacements.
static char *str_replace(const char *orig,
                         const char *rep,
                         const char *with,
                         int64_t *count)
{
    const char *ins = orig;
    char       *result;
    char       *tmp;
    size_t      len_rep  = strlen(rep);
    size_t      len_with = strlen(with);
    long        n        = 0;

    if (len_rep == 0) {
        *count = 0;
        return NULL;
    }

    // Count how many times 'rep' occurs
    while ((ins = strstr(ins, rep))) {
        n++;
        ins += len_rep;
    }
    *count = n;

    // Allocate enough memory for the new string
    result = malloc(strlen(orig) + (len_with - len_rep) * n + 1);
    if (!result) return NULL;

    tmp = result;
    ins = orig;
    // Perform replacements
    while (n--) {
        const char *pos = strstr(ins, rep);
        size_t front = pos - ins;
        memcpy(tmp, ins, front);
        tmp += front;
        memcpy(tmp, with, len_with);
        tmp += len_with;
        ins = pos + len_rep;
    }
    // Copy any remaining text
    strcpy(tmp, ins);
    return result;
}


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

    // 1. If they are the same instance, they are equal.
    if (theString1 == theString2) return true;

    // 2. If either is NULL (and they are not the same instance, checked above), they are not equal.
    if (NULL == theString1 || NULL == theString2) return false;

    // 3. Now it's safe to access members. Check typeID.
    if (theString1->_base.typeID != theString2->_base.typeID) return false;

    // 4. Compare lengths. If lengths differ, strings cannot be equal.
    if (theString1->length != theString2->length) return false;
    
    // 5. If lengths are 0 (and typeIDs and lengths are equal), they are equal (both are empty strings).
    //    Internal 'string' pointers could be NULL or point to "\0" for length 0,
    //    which is valid for OCStrings.
    if (theString1->length == 0) return true;

    // 6. Lengths are equal and greater than 0. Internal 'string' pointers should be valid.
    //    If string pointers could be NULL here despite length > 0 (which would be an inconsistency),
    //    an additional check like `if (!theString1->string || !theString2->string) return false;` (or specific handling)
    //    would be needed. Assuming valid OCString construction, string pointers are non-NULL if length > 0.
    if (strcmp(theString1->string, theString2->string) != 0) return false;
    
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



// -----------------------------------------------------------------------------
// Core OCString Constructors
// -----------------------------------------------------------------------------

OCStringRef OCStringCreateWithCString(const char *cString) {
    if (!cString) return NULL;
    struct __OCString *s = OCStringAllocate();
    if (!s) return NULL;

    // Byte‐length vs. code‐point length
    size_t byteLen = strlen(cString);
    s->capacity = byteLen;
    s->length   = oc_utf8_strlen(cString);

    // Copy raw UTF-8 bytes
    s->string = malloc(byteLen + 1);
    if (!s->string) {
        OCRelease(s);
        return NULL;
    }
    memcpy(s->string, cString, byteLen + 1);
    return s;
}



// A single mutable dictionary to intern all constant strings
static OCMutableDictionaryRef getConstantStringTable(void) {
    static OCMutableDictionaryRef table = NULL;
    if (table == NULL) {
        table = OCDictionaryCreateMutable(0);
    }
    return table;
}

OCStringRef __OCStringMakeConstantString(const char *cStr)
{
    // Create a temporary OCString to look up in the table
    OCStringRef tmp = OCStringCreateWithCString(cStr);
    OCMutableDictionaryRef table = getConstantStringTable();

    if (OCDictionaryContainsKey(table, tmp)) {
        // Already interned: return existing and drop the temporary
        OCStringRef existing = (OCStringRef)OCDictionaryGetValue(table, tmp);
        OCRelease(tmp);
        return existing;
    } else {
        // First time: add it and zero‐out its retainCount so it never frees
        OCDictionaryAddValue(table, tmp, tmp);
        OCMutableStringRef mtmp = (OCMutableStringRef)tmp;
        mtmp->_base.retainCount = 0;
        return tmp;
    }
}

OCStringRef OCStringCreateCopy(OCStringRef theString) {
    return theString ? OCStringCreateWithCString(theString->string) : NULL;
}

OCMutableStringRef OCStringCreateMutable(uint64_t capacity) {
    struct __OCString *s = OCStringAllocate();
    if (!s) return NULL;

    s->capacity = capacity;
    s->length   = 0;
    s->string   = malloc(capacity + 1);
    if (!s->string) {
        OCRelease(s);
        return NULL;
    }
    s->string[0] = '\0';
    return (OCMutableStringRef)s;
}

OCMutableStringRef OCMutableStringCreateWithCString(const char *cString) {
    // Just a mutable alias of the immutable creator
    return (OCMutableStringRef)OCStringCreateWithCString(cString);
}

OCMutableStringRef OCStringCreateMutableCopy(OCStringRef theString) {
    if (!theString) return NULL;
    struct __OCString *s = OCStringAllocate();
    if (!s) return NULL;

    // Preserve both byte‐capacity and code‐point length
    size_t byteLen = strlen(theString->string);
    s->capacity = byteLen;
    s->length   = theString->length;

    s->string = malloc(byteLen + 1);
    if (!s->string) {
        OCRelease(s);
        return NULL;
    }
    memcpy(s->string, theString->string, byteLen + 1);
    return (OCMutableStringRef)s;
}

OCStringRef OCStringCreateWithSubstring(OCStringRef str, OCRange range) {
    if (!str) return NULL;

    // Map code‐point range → byte offsets
    ptrdiff_t off1 = oc_utf8_offset_for_index(str->string, range.location);
    ptrdiff_t off2 = oc_utf8_offset_for_index(str->string, range.location + range.length);
    if (off1 < 0 || off2 < 0 || off2 < off1) {
        // Special case: empty slice at end of string
        if (range.location == str->length && range.length == 0) {
            off1 = off2 = strlen(str->string);
        } else {
            return NULL;
        }
    }

    size_t byteCount = off2 - off1;
    char *buf = malloc(byteCount + 1);
    if (!buf) return NULL;
    memcpy(buf, str->string + off1, byteCount);
    buf[byteCount] = '\0';

    OCStringRef sub = OCStringCreateWithCString(buf);
    free(buf);
    return sub;
}


#include <stdio.h>    // fputs, stdout
#include <stdlib.h>   // realloc, free
#include <string.h>   // strlen, strdup

// ——— Inspectors ———

const char *OCStringGetCString(OCStringRef s) {
    return s ? s->string : NULL;
}

uint64_t OCStringGetLength(OCStringRef s) {
    return s ? s->length : 0;
}

void OCStringShow(OCStringRef s) {
    if (s && s->string) {
        fputs(s->string, stdout);
        fflush(stdout);
    }
}

bool OCStringEqual(OCStringRef a, OCStringRef b) {
    return __OCStringEqual(a, b);
}

// Note: header declares `char OCStringGetCharacterAtIndex(...)`, 
// but to return a full code-point you’d need to change it to uint32_t.
// Here we return the Unicode code-point cast to char (low byte).
uint32_t OCStringGetCharacterAtIndex(OCStringRef s, uint64_t idx) {
    if (!s) return 0;
    ptrdiff_t off = oc_utf8_offset_for_index(s->string, idx);
    if (off < 0) return 0;
    const char *p = s->string + off;
    uint32_t cp = utf8_next(&p);
    return (char)cp;
}

// ——— Mutators ———

void OCStringAppendCString(OCMutableStringRef s, const char *cString) {
    if (!s || !cString) return;
    size_t addBytes = strlen(cString);
    size_t oldBytes = s->capacity;
    size_t newBytes = oldBytes + addBytes;

    char *buf = realloc(s->string, newBytes + 1);
    if (!buf) return;
    memcpy(buf + oldBytes, cString, addBytes + 1);
    s->string   = buf;
    s->capacity = newBytes;
    s->length  += oc_utf8_strlen(cString);
}

void OCStringAppend(OCMutableStringRef s, OCStringRef app) {
    if (!s || !app || app->length == 0) return;
    OCStringAppendCString(s, app->string);
}

void OCStringDelete(OCMutableStringRef s, OCRange range) {
    if (!s) return;
    // map code-point indices → byte offsets
    ptrdiff_t off1 = oc_utf8_offset_for_index(s->string, range.location);
    ptrdiff_t off2 = oc_utf8_offset_for_index(s->string, range.location + range.length);
    if (off1 < 0 || off2 < 0 || off2 < off1) return;

    size_t totalBytes = strlen(s->string);
    size_t tailBytes  = totalBytes - off2;
    memmove(s->string + off1,
            s->string + off2,
            tailBytes + 1);  // include NUL

    s->length  -= range.length;
    s->capacity = totalBytes - (off2 - off1);
}

void OCStringInsert(OCMutableStringRef s, int64_t idx, OCStringRef ins) {
    if (!s || !ins) return;
    OCStringReplace(s, OCRangeMake(idx, 0), ins);
}

void OCStringReplace(OCMutableStringRef s, OCRange range, OCStringRef rep) {
    if (!s || !rep) return;
    ptrdiff_t off1 = oc_utf8_offset_for_index(s->string, range.location);
    ptrdiff_t off2 = oc_utf8_offset_for_index(s->string, range.location + range.length);
    if (off1 < 0 || off2 < 0 || off2 < off1) return;

    size_t origBytes  = strlen(s->string);
    size_t repBytes   = strlen(rep->string);
    size_t tailBytes  = origBytes - off2;
    size_t newBytes   = off1 + repBytes + tailBytes;

    if (newBytes > s->capacity) {
        char *buf = realloc(s->string, newBytes + 1);
        if (!buf) return;
        s->string   = buf;
        s->capacity = newBytes;
    }
    // shift tail
    memmove(s->string + off1 + repBytes,
            s->string + off2,
            tailBytes + 1);
    // copy replacement
    memcpy(s->string + off1, rep->string, repBytes);
    s->length = s->length - range.length + rep->length;
}

void OCStringReplaceAll(OCMutableStringRef s, OCStringRef rep) {
    OCStringReplace(s, OCRangeMake(0, s->length), rep);
}

// ——— Case conversions and trimming ———

void OCStringLowercase(OCMutableStringRef s) {
    if (!s) return;
    // ASCII-only lowercase
    for (uint64_t i = 0; i < s->length; i++) {
        ptrdiff_t off = oc_utf8_offset_for_index(s->string, i);
        unsigned char *c = (unsigned char *)(s->string + off);
        if (*c >= 'A' && *c <= 'Z') *c += 'a' - 'A';
    }
}

void OCStringUppercase(OCMutableStringRef s) {
    if (!s) return;
    for (uint64_t i = 0; i < s->length; i++) {
        ptrdiff_t off = oc_utf8_offset_for_index(s->string, i);
        unsigned char *c = (unsigned char *)(s->string + off);
        if (*c >= 'a' && *c <= 'z') *c -= 'a' - 'A';
    }
}

void OCStringTrimWhitespace(OCMutableStringRef s) {
    if (!s || s->length == 0) return;
    uint64_t start = 0;
    while (start < s->length) {
        uint32_t cp = utf8_next((const char **)&s->string + oc_utf8_offset_for_index(s->string, start));
        if (cp != ' ') break;
        start++;
    }
    uint64_t end = s->length;
    while (end > start) {
        uint32_t cp = OCStringGetCharacterAtIndex(s, end - 1);
        if (cp != ' ') break;
        end--;
    }
    // extract substring and replace
    OCStringRef sub = OCStringCreateWithSubstring(s, OCRangeMake(start, end - start));
    free(s->string);
    s->string   = strdup(sub->string);
    s->capacity = strlen(s->string);
    s->length   = sub->length;
    OCRelease(sub);
}

bool OCStringTrimMatchingParentheses(OCMutableStringRef s) {
    if (!s || s->length < 2) return false;
    uint32_t first = OCStringGetCharacterAtIndex(s, 0);
    uint32_t last  = OCStringGetCharacterAtIndex(s, s->length - 1);
    if (first != '(' || last != ')') return false;
    // peel layers
    bool trimmed = false;
    while (s->length >= 2 &&
           OCStringGetCharacterAtIndex(s, 0) == '(' &&
           OCStringGetCharacterAtIndex(s, s->length - 1) == ')') {
        OCStringDelete(s, OCRangeMake(s->length - 1, 1));
        OCStringDelete(s, OCRangeMake(0, 1));
        OCStringTrimWhitespace(s);
        trimmed = true;
    }
    return trimmed;
}

// ——— Extract a float-precision complex value from an OCString ———
float complex OCStringGetFloatComplexValue(OCStringRef string) {
    if (!string) return nanf("") + nanf("") * I;
    // Replace ‘•’ with ‘*’ for multiplication
    OCMutableStringRef m = OCStringCreateMutableCopy(string);
    OCStringFindAndReplace2(m, STR("•"), STR("*"));
    float complex v = OCComplexFromCString(m->string);
    OCRelease(m);
    return v;
}

// ——— Extract a double-precision complex value from an OCString ———
double complex OCStringGetDoubleComplexValue(OCStringRef string) {
    if (!string) return nan("") + nan("") * I;
    OCMutableStringRef m = OCStringCreateMutableCopy(string);
    OCStringFindAndReplace2(m, STR("•"), STR("*"));
    double complex v = OCComplexFromCString(m->string);
    OCRelease(m);
    return v;
}

// ——— Create an OCString representing a float value ———
OCStringRef OCFloatCreateStringValue(float value) {
    // Determine needed buffer size
    int n = snprintf(NULL, 0, "%g", value);
    char *buf = malloc(n + 1);
    if (!buf) return NULL;
    snprintf(buf, n + 1, "%g", value);
    OCStringRef s = OCStringCreateWithCString(buf);
    free(buf);
    return s;
}

// ——— Create an OCString representing a double value ———
OCStringRef OCDoubleCreateStringValue(double value) {
    int n = snprintf(NULL, 0, "%g", value);
    char *buf = malloc(n + 1);
    if (!buf) return NULL;
    snprintf(buf, n + 1, "%g", value);
    OCStringRef s = OCStringCreateWithCString(buf);
    free(buf);
    return s;
}

// ——— Create an OCString representing a float complex value using a format OCString ———
// Format string must include two specifiers, e.g. "%g%+gi"
OCStringRef OCFloatComplexCreateStringValue(float complex v, OCStringRef format) {
    const char *fmt = format ? format->string : "%g%+gi";
    double real = crealf(v), imag = cimagf(v);
    int n = snprintf(NULL, 0, fmt, real, imag);
    char *buf = malloc(n + 1);
    if (!buf) return NULL;
    snprintf(buf, n + 1, fmt, real, imag);
    OCStringRef s = OCStringCreateWithCString(buf);
    free(buf);
    return s;
}

// ——— Create an OCString representing a double complex value using a format OCString ———
OCStringRef OCDoubleComplexCreateStringValue(double complex v, OCStringRef format) {
    const char *fmt = format ? format->string : "%g%+gi";
    double real = creal(v), imag = cimag(v);
    int n = snprintf(NULL, 0, fmt, real, imag);
    char *buf = malloc(n + 1);
    if (!buf) return NULL;
    snprintf(buf, n + 1, fmt, real, imag);
    OCStringRef s = OCStringCreateWithCString(buf);
    free(buf);
    return s;
}

// ——— Find first literal occurrence (code-point semantics) ———
OCRange OCStringFind(OCStringRef string,
                     OCStringRef stringToFind,
                     OCOptionFlags compareOptions)
{
    if (!string || !stringToFind) {
        return OCRangeMake(kOCNotFound, 0);
    }

    const char *hay    = string->string;
    const char *needle = stringToFind->string;
    const char *found  = strstr(hay, needle);
    if (!found) {
        return OCRangeMake(kOCNotFound, 0);
    }

    // Compute byte-offset → code-point index
    ptrdiff_t byteOff = found - hay;
    size_t   idx     = 0;
    const char *p    = hay;
    while ((p - hay) < byteOff) {
        utf8_next(&p);
        idx++;
    }

    // The length in code-points of the match
    size_t cpLen = oc_utf8_strlen(needle);
    return OCRangeMake(idx, cpLen);
}

// ——— Replace all literal occurrences in the entire string ———
int64_t OCStringFindAndReplace2(OCMutableStringRef s,
                                OCStringRef findStr,
                                OCStringRef replaceStr)
{
    if (!s || !findStr || !replaceStr) return 0;

    int64_t count = 0;
    char *newBuf = str_replace(s->string,
                               findStr->string,
                               replaceStr->string,
                               &count);
    if (!newBuf) return 0;

    free(s->string);
    s->string   = newBuf;
    s->capacity = strlen(newBuf);
    s->length   = oc_utf8_strlen(newBuf);
    return count;
}

// ——— Replace within a specified code-point range ———
int64_t OCStringFindAndReplace(OCMutableStringRef s,
                               OCStringRef findStr,
                               OCStringRef replaceStr,
                               OCRange rangeToSearch,
                               OCOptionFlags compareOptions)
{
    if (!s || !findStr || !replaceStr) return 0;
    if (rangeToSearch.location > s->length) return 0;
    if (rangeToSearch.location + rangeToSearch.length > s->length) {
        rangeToSearch.length = s->length - rangeToSearch.location;
    }

    // Extract just that segment
    OCStringRef subImmutable = OCStringCreateWithSubstring(s, rangeToSearch);
    OCMutableStringRef sub     = OCStringCreateMutableCopy(subImmutable);
    OCRelease(subImmutable);

    // Perform replace on the segment
    int64_t count = OCStringFindAndReplace2(sub, findStr, replaceStr);

    // Splice it back into the original
    OCStringReplace(s, rangeToSearch, (OCStringRef)sub);
    OCRelease(sub);

    return count;
}

// ——— Character-type helper functions (ASCII only) ———

bool characterIsUpperCaseLetter(char c) {
    return (c >= 'A' && c <= 'Z');
}

bool characterIsLowerCaseLetter(char c) {
    return (c >= 'a' && c <= 'z');
}

bool characterIsDigitOrDecimalPoint(char c) {
    return ((c >= '0' && c <= '9') || c == '.');
}

bool characterIsDigitOrDecimalPointOrSpace(char c) {
    return ((c >= '0' && c <= '9') || c == '.' || c == ' ');
}

// UTF-8 helpers must already be in scope:
//   static uint32_t utf8_next(const char **p);
//   static ptrdiff_t oc_utf8_offset_for_index(const char *s, size_t idx);
//   static size_t    oc_utf8_strlen(const char *s);

/**
 * @brief Finds a substring within an OCString with specified options.
 * @param string        Source OCString.
 * @param stringToFind  Substring to find.
 * @param rangeToSearch Code-point range in which to search.
 * @param compareOptions Bitmask of kOCCompare… flags (we only honor kOCCompareCaseInsensitive).
 * @param result        Out-parameter to receive the found range (in code-points).
 * @return true if found, false otherwise.
 */
bool
OCStringFindWithOptions(OCStringRef   string,
                        OCStringRef   stringToFind,
                        OCRange       rangeToSearch,
                        OCOptionFlags compareOptions,
                        OCRange      *result)
{
    if (!string || !stringToFind || !result) return false;
    uint64_t strLen    = OCStringGetLength(string);
    uint64_t needleLen = OCStringGetLength(stringToFind);

    // Sanity: empty needle or out-of-bounds range
    if (needleLen == 0 ||
        rangeToSearch.location > strLen ||
        rangeToSearch.location + rangeToSearch.length > strLen) {
        return false;
    }

    bool caseInsensitive = (compareOptions & kOCCompareCaseInsensitive) != 0;
    uint64_t maxStart = rangeToSearch.location + rangeToSearch.length - needleLen;

    // Sliding window over code-point indices
    for (uint64_t pos = rangeToSearch.location; pos <= maxStart; ++pos) {
        bool match = true;
        for (uint64_t j = 0; j < needleLen; ++j) {
            uint32_t c1 = OCStringGetCharacterAtIndex(string,      pos + j);
            uint32_t c2 = OCStringGetCharacterAtIndex(stringToFind, j);
            if (caseInsensitive) {
                // ASCII-only folding
                if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
                if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
            }
            if (c1 != c2) {
                match = false;
                break;
            }
        }
        if (match) {
            *result = OCRangeMake(pos, needleLen);
            return true;
        }
    }
    return false;
}
OCComparisonResult
OCStringCompare(OCStringRef theString1,
                OCStringRef theString2,
                OCStringCompareFlags compareOptions)
{
    // Handle NULL pointers
    if (theString1 == theString2) return kOCCompareEqualTo;
    if (theString1 == NULL)         return kOCCompareLessThan;
    if (theString2 == NULL)         return kOCCompareGreaterThan;

    const char *s1 = theString1->string;
    const char *s2 = theString2->string;

    int diff;
    if (compareOptions & kOCCompareCaseInsensitive) {
        // ASCII-only case-insensitive
        diff = strcasecmp(s1, s2);
    } else {
        // Literal byte-wise UTF-8 comparison
        diff = strcmp(s1, s2);
    }

    if (diff < 0) return kOCCompareLessThan;
    if (diff > 0) return kOCCompareGreaterThan;
    return kOCCompareEqualTo;
}

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "OCString.h"

// Internal helper: append formatted text using a va_list
void _OCStringAppendFormatAndArgumentsAux(OCMutableStringRef outputString,
                                          OCStringRef formatString,
                                          va_list args)
{
    if (!outputString || !formatString) return;

    const char *fmt = formatString->string;
    if (!fmt) return;

    // Handle object specifier "%@"
    if (strstr(fmt, "%@")) {
        const char *p = fmt;
        while (*p) {
            if (p[0]=='%' && p[1]=='%') {
                OCStringAppendCString(outputString, "%");
                p += 2;
            } else if (p[0]=='%' && p[1]=='@') {
                OCStringRef obj = va_arg(args, OCStringRef);
                if (obj && obj->string) OCStringAppendCString(outputString, obj->string);
                p += 2;
            } else if (p[0]=='%') {
                const char *start = p; p++;
                while (strchr("-+ #0", *p)) p++;
                while (*p >= '0' && *p <= '9') p++;
                if (*p=='*') p++;
                if (*p=='.') { p++; while (*p>='0' && *p<='9') p++; if (*p=='*') p++; }
                if (strchr("hljztL", *p)) {
                    if ((p[0]=='h'&&p[1]=='h')||(p[0]=='l'&&p[1]=='l')) p+=2; else p++;
                }
                if (*p) p++;
                size_t len = p - start;
                char *subFmt = malloc(len+1);
                memcpy(subFmt, start, len);
                subFmt[len] = '\0';
                va_list argsCopy;
                va_copy(argsCopy, args);
                int needed = vsnprintf(NULL, 0, subFmt, argsCopy);
                va_end(argsCopy);
                if (needed > 0) {
                    char *buf = malloc((size_t)needed + 1);
                    vsnprintf(buf, (size_t)needed + 1, subFmt, args);
                    OCStringAppendCString(outputString, buf);
                    free(buf);
                }
                free(subFmt);
            } else {
                char tmp[2] = {*p, '\0'};
                OCStringAppendCString(outputString, tmp);
                p++;
            }
        }
        return;
    }

    // Measure required buffer size
    va_list argsCopy;
    va_copy(argsCopy, args);
    int needed = vsnprintf(NULL, 0, fmt, argsCopy);
    va_end(argsCopy);
    if (needed < 0) return;

    // Format into buffer
    char *buf = malloc((size_t)needed + 1);
    if (!buf) return;
    vsnprintf(buf, (size_t)needed + 1, fmt, args);

    // Append the result
    OCStringAppendCString(outputString, buf);
    free(buf);
}

/**
 * @brief Creates an immutable OCString using a format string and arguments.
 * @param format Format OCString.
 * @param ... Variable arguments for the format string.
 * @return New OCStringRef (ownership transferred to caller).
 * @ingroup OCString
 */
OCStringRef OCStringCreateWithFormat(OCStringRef format, ...)
{
    va_list args;
    va_start(args, format);

    OCMutableStringRef result = OCStringCreateMutable(0);
    _OCStringAppendFormatAndArgumentsAux(result, format, args);

    va_end(args);
    return (OCStringRef)result;
}

/**
 * @brief Appends formatted text to a mutable OCString.
 * @param theString Mutable OCString.
 * @param format Format OCString.
 * @param ... Variable arguments for the format string.
 * @ingroup OCString
 */
void OCStringAppendFormat(OCMutableStringRef theString, OCStringRef format, ...)
{
    va_list args;
    va_start(args, format);
    _OCStringAppendFormatAndArgumentsAux(theString, format, args);
    va_end(args);
}


/**
 * @brief Creates an array of ranges where a substring is found.
 */
OCArrayRef OCStringCreateArrayWithFindResults(OCStringRef string,
                                              OCStringRef stringToFind,
                                              OCRange rangeToSearch,
                                              OCOptionFlags compareOptions)
{
    if (!string || !stringToFind) return NULL;
    // Clip the search range to the string’s length
    if (rangeToSearch.location > string->length) return NULL;
    if (rangeToSearch.location + rangeToSearch.length > string->length) {
        rangeToSearch.length = string->length - rangeToSearch.location;
    }

    OCMutableArrayRef result = OCArrayCreateMutable(0, &kOCRangeArrayCallBacks);
    if (!result) return NULL;

    OCRange search = rangeToSearch, found;
    while (OCStringFindWithOptions(string, stringToFind, search, compareOptions, &found)) {
        OCRange *r = malloc(sizeof(OCRange));
        if (!r) { OCRelease(result); return NULL; }
        *r = found;
        OCArrayAppendValue(result, r);

        // Advance past this match
        search.location = found.location + found.length;
        uint64_t end = rangeToSearch.location + rangeToSearch.length;
        if (search.location >= end) break;
        search.length = end - search.location;
    }

    if (OCArrayGetCount(result) == 0) {
        OCRelease(result);
        return NULL;
    }
    return result;
}

/**
 * @brief Splits a string by a separator, returning an array of OCStringRefs.
 */
OCArrayRef OCStringCreateArrayBySeparatingStrings(OCStringRef string,
                                                  OCStringRef separatorString)
{
    if (!string || !separatorString) return NULL;

    OCMutableArrayRef result = OCArrayCreateMutable(0, &kOCRangeArrayCallBacks);
    if (!result) return NULL;

    uint64_t totalLen = string->length;
    uint64_t sepLen   = separatorString->length;

    // Special case: empty sep → return [ string ]
    if (sepLen == 0) {
        OCStringRef copy = OCStringCreateCopy(string);
        OCArrayAppendValue(result, copy);
        OCRelease(copy);
        return result;
    }

    uint64_t idx = 0;
    while (idx <= totalLen) {
        // Search in the remaining slice
        OCRange remaining = OCRangeMake(idx, totalLen - idx);
        OCRange found;
        bool hit = OCStringFindWithOptions(string, separatorString, remaining, 0, &found);
        if (hit) {
            // Add substring [idx .. found.location)
            OCStringRef token = OCStringCreateWithSubstring(string,
                                        OCRangeMake(idx, found.location - idx));
            OCArrayAppendValue(result, token);
            OCRelease(token);
            // Move past the separator
            idx = found.location + found.length;
        } else {
            // Last token: [idx .. end)
            OCStringRef token = OCStringCreateWithSubstring(string,
                                        OCRangeMake(idx, totalLen - idx));
            OCArrayAppendValue(result, token);
            OCRelease(token);
            break;
        }
    }

    return result;
}
