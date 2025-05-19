#include <stdio.h>
#include <stdlib.h>   // malloc, free, realloc
#include <string.h>   // strlen, strcmp, memcpy, memmove
#include <stdint.h>   // uint32_t
#include <stddef.h>   // ptrdiff_t, size_t
#include <complex.h>
#include <math.h>
#include "OCString.h"
#include "OCDictionary.h"   // for OCMutableDictionaryRef, OCDictionaryCreateMutable, etc.

// Forward declaration for OCStringFindWithOptions
bool OCStringFindWithOptions(OCStringRef string, OCStringRef stringToFind, OCRange rangeToSearch, OCOptionFlags compareOptions, OCRange *result);

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
    uint32_t prev_cp = 0;
    uint32_t cp = 0;
    bool in_emoji_sequence = false;
    
    // Enhanced UTF-8 code point counter that handles:
    // 1. Combining characters (e.g., é̀́)
    // 2. Emoji ZWJ sequences (e.g., 👨‍👩‍👧‍👦)
    // 3. Regional indicators for flag emojis (e.g., 🇺🇸)
    
    while (*p) {
        prev_cp = cp;
        cp = utf8_next(&p);
        
        // Skip counting if this is a combining character (0x0300-0x036F, 0x1AB0-0x1AFF, 0x1DC0-0x1DFF, 0x20D0-0x20FF, 0xFE20-0xFE2F)
        if ((cp >= 0x0300 && cp <= 0x036F) || 
            (cp >= 0x1AB0 && cp <= 0x1AFF) ||
            (cp >= 0x1DC0 && cp <= 0x1DFF) ||
            (cp >= 0x20D0 && cp <= 0x20FF) ||
            (cp >= 0xFE20 && cp <= 0xFE2F)) {
            continue; // Skip combining character
        }
        
        // Handle Zero Width Joiner sequences (ZWJ is U+200D)
        if (cp == 0x200D) {
            in_emoji_sequence = true;
            continue; // Skip ZWJ
        }
        
        // Handle regional indicators for flag emojis (e.g., 🇺🇸)
        // Regional indicators are in range U+1F1E6 to U+1F1FF
        if (prev_cp >= 0x1F1E6 && prev_cp <= 0x1F1FF && 
            cp >= 0x1F1E6 && cp <= 0x1F1FF) {
            continue; // Skip second part of flag emoji
        }
        
        // Handle emoji variation selectors (VS15 U+FE0E and VS16 U+FE0F)
        if (cp == 0xFE0E || cp == 0xFE0F) {
            continue; // Skip variation selector
        }
        
        // If we were in emoji sequence and this isn't a continuation,
        // count it as one emoji
        if (in_emoji_sequence) {
            if (*p && utf8_next(&p) != 0x200D) {
                // End of sequence
                in_emoji_sequence = false;
            }
            continue; // Skip the character that follows the ZWJ
        }
        
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
// MODIFIED: Now returns the full uint32_t Unicode code-point.
uint32_t OCStringGetCharacterAtIndex(OCStringRef s, uint64_t idx) {
    if (!s) return 0;
    ptrdiff_t off = oc_utf8_offset_for_index(s->string, idx);
    if (off < 0) return 0;
    const char *p = s->string + off;
    uint32_t cp = utf8_next(&p);
    return cp;
}

// ——— Mutators ———

void OCStringAppendCString(OCMutableStringRef s, const char *cString) {
    if (!s || !cString || !s->string) return; // Ensure s and s->string are valid
    size_t append_cString_byte_len = strlen(cString);
    if (append_cString_byte_len == 0) return; // Nothing to append

    size_t current_content_byte_len = strlen(s->string);
    size_t required_total_content_byte_len = current_content_byte_len + append_cString_byte_len;

    // Check if current capacity is enough for the new total content length
    // s->capacity is the max content bytes, s->string is allocated for s->capacity + 1 bytes
    if (required_total_content_byte_len > s->capacity) {
        // Need to reallocate.
        uint64_t new_capacity = s->capacity;
        // Standard growth strategy: double capacity until it's sufficient, or go to required.
        // Start with a reasonable base if current capacity is 0.
        if (new_capacity == 0) {
            new_capacity = 16; // Default initial capacity if appending to an empty string from OCStringCreateMutable(0)
        }
        while (required_total_content_byte_len > new_capacity) {
            new_capacity *= 2;
        }
        // Ensure new_capacity is at least required_total_content_byte_len if doubling wasn't enough or started too small.
        if (new_capacity < required_total_content_byte_len) {
            new_capacity = required_total_content_byte_len;
        }

        char *new_s_string_buffer = realloc(s->string, new_capacity + 1); // +1 for NUL
        if (!new_s_string_buffer) {
            // Allocation failure. Cannot append.
            // Consider error handling strategy. For now, return without changing string.
            return;
        }
        s->string = new_s_string_buffer;
        s->capacity = new_capacity;
    }

    // Append cString to the end of the current content in s->string
    // memcpy is safe here because we've ensured s->string has enough space.
    memcpy(s->string + current_content_byte_len, cString, append_cString_byte_len + 1); // +1 to copy NUL from cString

    // Update the code-point length
    s->length += oc_utf8_strlen(cString); // oc_utf8_strlen calculates codepoints from a C-string
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
    size_t repBytes   = 0;
    // Ensure rep->string is not NULL before calling strlen and accessing capacity
    if (rep->string) {
        repBytes = strnlen(rep->string, rep->capacity); // Use strnlen for safety
    } else {
        // rep->string is NULL. repBytes remains 0.
        // This handles OCStrings with rep->length == 0 or invalid OCStrings (length > 0 but string is NULL).
    }
    
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
    if (!s || !s->string || s->length == 0) return;

    uint64_t start_cp_idx = 0; // code-point index
    size_t current_byte_len = strlen(s->string); // Get initial byte length for boundary checks

    // Find the first non-space character from the beginning (code-point wise)
    while (start_cp_idx < s->length) {
        ptrdiff_t byte_offset = oc_utf8_offset_for_index(s->string, start_cp_idx);
        if (byte_offset < 0 || (size_t)byte_offset >= current_byte_len) {
            // Invalid offset or past end of actual C-string, stop.
            break;
        }
        const char *p_char_for_cp = s->string + byte_offset;
        uint32_t cp = utf8_next(&p_char_for_cp); // p_char_for_cp is advanced

        if (cp != ' ') break; // Found non-space
        start_cp_idx++; // Move to next code-point index
    }

    uint64_t end_cp_idx = s->length; // code-point index (exclusive)
    // Find the first non-space character from the end (code-point wise)
    while (end_cp_idx > start_cp_idx) {
        uint32_t cp = OCStringGetCharacterAtIndex(s, end_cp_idx - 1);
        if (cp != ' ') break;
        end_cp_idx--;
    }

    // If start_cp_idx >= end_cp_idx, the resulting string is empty
    if (start_cp_idx >= end_cp_idx) {
        free(s->string);
        s->string = strdup("");
        if (!s->string) { 
            // Allocation failure for empty string
            s->capacity = 0; s->length = 0; 
            // OCRelease(s); // This would lead to double free if s is used after this. Let caller handle.
            return; 
        }
        s->length = 0;
        s->capacity = 0; // strlen("") is 0
        return;
    }

    // Create substring from the trimmed range
    OCStringRef sub = OCStringCreateWithSubstring(s, OCRangeMake(start_cp_idx, end_cp_idx - start_cp_idx));
    
    if (!sub) { 
        // OCStringCreateWithSubstring failed (e.g., memory allocation).
        // It's safer to leave 's' in its original state or an empty state if original is too risky.
        // For now, let's try to set 's' to empty as a fallback.
        free(s->string);
        s->string = strdup("");
        if (!s->string) { 
            s->capacity = 0; s->length = 0; 
            return; 
        }
        s->length = 0;
        s->capacity = 0;
        return;
    }

    // Replace the content of 's' with the content of 'sub'
    free(s->string); // Free the old string buffer of 's'
    s->string = strdup(sub->string ? sub->string : ""); // Duplicate content from 'sub'
    
    if (!s->string) { // strdup failed
        s->capacity = 0;
        s->length = 0;
        // 'sub' was successfully created, so it needs to be released.
        OCRelease(sub);
        // 's' is now in a bad state (original buffer freed, new one failed to alloc).
        return;
    }
    
    s->capacity = strlen(s->string); // Byte capacity from the new string
    s->length = sub->length;         // Code-point length from 'sub'
    OCRelease(sub); // Release the temporary substring 'sub'
}

bool OCStringTrimMatchingParentheses(OCMutableStringRef s) {
    if (!s || !s->string || s->length < 2) return false; // Added s->string check
    bool trimmed = false;
    // Trim only one layer of parentheses as per test expectation for ((double)) -> (double)
    if (OCStringGetCharacterAtIndex(s, 0) == '(' &&
        OCStringGetCharacterAtIndex(s, s->length - 1) == ')') {
        OCStringDelete(s, OCRangeMake(s->length - 1, 1)); // Delete last parenthesis
        OCStringDelete(s, OCRangeMake(0, 1));             // Delete first parenthesis
        OCStringTrimWhitespace(s); // Trim whitespace after removing parentheses
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
    if (!string || !stringToFind || !string->string || !stringToFind->string) {
        return OCRangeMake(kOCNotFound, 0);
    }

    OCRange result_range;
    if (OCStringFindWithOptions(string,
                                stringToFind,
                                OCRangeMake(0, string->length), // Search the entire string
                                compareOptions,
                                &result_range)) {
        return result_range;
    } else {
        return OCRangeMake(kOCNotFound, 0);
    }
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
    if (!s || !s->string || !findStr || !replaceStr || !findStr->string || !replaceStr->string) {
        // Ensure all string objects and their internal C-strings are valid
        return 0;
    }
    if (findStr->length == 0) {
        // Replacing an empty string is often ill-defined or can lead to infinite loops.
        // For simplicity, we'll say no replacements are made if findStr is empty.
        return 0;
    }

    // Validate and clip rangeToSearch to be within the bounds of the current string 's'
    if (rangeToSearch.location >= s->length) {
        return 0; // Start of search range is already past the end of the string
    }
    if (rangeToSearch.location + rangeToSearch.length > s->length) {
        rangeToSearch.length = s->length - rangeToSearch.location;
    }

    // If the effective search range is empty and we're looking for a non-empty string, no match is possible.
    if (rangeToSearch.length == 0 && findStr->length > 0) {
        return 0;
    }
    // If search range is shorter than findStr, no match is possible.
    if (rangeToSearch.length < findStr->length) {
        return 0;
    }

    int64_t count = 0;
    uint64_t scan_start_location_in_s = rangeToSearch.location;
    
    // This is the end boundary of the original search area, relative to the original string 's'.
    // We need to track this because 's' will change length.
    uint64_t original_search_area_end_location_in_s = rangeToSearch.location + rangeToSearch.length;
    
    int64_t cumulative_length_change = 0; // Tracks total change in s->length due to replacements

    while (true) {
        // Calculate the current effective end of the search area in 's', accounting for length changes
        uint64_t current_effective_search_area_end_in_s = original_search_area_end_location_in_s + cumulative_length_change;

        // Stop if scan_start_location_in_s has reached or passed the end of 's' or the effective search area
        if (scan_start_location_in_s >= s->length || scan_start_location_in_s >= current_effective_search_area_end_in_s) {
            break;
        }

        OCRange current_look_in_range;
        current_look_in_range.location = scan_start_location_in_s;
        current_look_in_range.length = current_effective_search_area_end_in_s - scan_start_location_in_s;

        // If remaining search length is less than findStr's length, no more matches are possible
        if (current_look_in_range.length < findStr->length) {
            break;
        }

        OCRange found_at_range_in_s; // Will store the range of the found substring in the current state of 's'
        if (OCStringFindWithOptions(s, findStr, current_look_in_range, compareOptions, &found_at_range_in_s)) {
            OCStringReplace(s, found_at_range_in_s, replaceStr);
            count++;
            
            // Update the cumulative length change
            cumulative_length_change += ((int64_t)replaceStr->length - (int64_t)findStr->length);
            
            // Advance the scan_start_location_in_s to the position immediately after the inserted replaceStr
            scan_start_location_in_s = found_at_range_in_s.location + replaceStr->length;
        } else {
            // No more occurrences of findStr in the remaining search range
            break;
        }
    }
    return count;
}

// ——— Character-type helper functions (ASCII only) ———

bool characterIsUpperCaseLetter(uint32_t character) {
    // Basic ASCII check, expand for Unicode if necessary
    return (character >= 'A' && character <= 'Z');
}

bool characterIsLowerCaseLetter(uint32_t character) {
    // Basic ASCII check, expand for Unicode if necessary
    return (character >= 'a' && character <= 'z');
}

bool characterIsDigitOrDecimalPoint(uint32_t character) {
    return (character >= '0' && character <= '9') || character == '.';
}

bool characterIsDigitOrDecimalPointOrSpace(uint32_t character) {
    return (character >= '0' && character <= '9') || character == '.' || character == ' ';
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
    if (!string || !string->string || !separatorString || !separatorString->string) return NULL;

    OCMutableArrayRef result = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks); // Use OCType callbacks for OCStringRef
    if (!result) return NULL;

    uint64_t current_pos = 0;
    uint64_t string_len = string->length;
    uint64_t sep_len = separatorString->length;

    // Handle empty string: return array with one empty element
    if (string_len == 0) {
        OCStringRef empty = OCStringCreateWithCString("");
        if (empty) {
            OCArrayAppendValue(result, empty);
            OCRelease(empty); // OCArray retains it
        }
        return result;
    }

    // Handle empty separator string: return array with one element (copy of original string)
    if (sep_len == 0) {
        OCStringRef copy = OCStringCreateCopy(string);
        if (copy) {
            OCArrayAppendValue(result, copy);
            OCRelease(copy); // OCArray retains it
        }
        return result;
    }

    // Check if string begins with separator, if so add empty string first
    // OCRange prefix_check = OCRangeMake(0, sep_len);
    if (OCStringFind(string, separatorString, 0).location == 0) {
        OCStringRef empty = OCStringCreateWithCString("");
        if (empty) {
            OCArrayAppendValue(result, empty);
            OCRelease(empty);
        }
        // Skip past the initial separator
        current_pos = sep_len;
    }

    // Simple iterative approach to avoid memory management issues with ranges
    while (current_pos <= string_len) {
        OCRange search_range = OCRangeMake(current_pos, string_len - current_pos);
        OCRange found_range;
        
        bool found = OCStringFindWithOptions(string, separatorString, search_range, 0, &found_range);
        
        if (found) {
            // Create substring from current_pos to the start of the separator
            uint64_t part_len = found_range.location - current_pos;
            OCRange part_range = OCRangeMake(current_pos, part_len);
            
            OCStringRef part = OCStringCreateWithSubstring(string, part_range);
            
            if (part) {
                OCArrayAppendValue(result, part);
                OCRelease(part); // Array retains it
            }
            
            // Move past the separator
            current_pos = found_range.location + found_range.length;
            
            // If we're at the end and found a separator at the end,
            // add an empty string for the trailing separator
            if (current_pos == string_len) {
                OCStringRef empty = OCStringCreateWithCString("");
                if (empty) {
                    OCArrayAppendValue(result, empty);
                    OCRelease(empty);
                }
                break;
            }
        } else {
            // No more separators - add the final part and break
            OCRange final_range = OCRangeMake(current_pos, string_len - current_pos);
            
            OCStringRef final_part = OCStringCreateWithSubstring(string, final_range);
            
            if (final_part) {
                OCArrayAppendValue(result, final_part);
                OCRelease(final_part); // Array retains it
            }
            break;
        }
    }
    
    return result;
}
