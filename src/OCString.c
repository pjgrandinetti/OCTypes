#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE  // For strcasecmp
#include "OCString.h"    // Own header first
#include <complex.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>  // ptrdiff_t, size_t
#include <stdint.h>  // uint32_t
#include <stdio.h>
#ifdef _WIN32
#include <signal.h>  // For Windows signal handling
#include <setjmp.h>  // For setjmp/longjmp
#endif
#include <stdlib.h>        // malloc, free, realloc
#include <string.h>        // strlen, strcmp, memcpy, memmove
#include <strings.h>       // strcasecmp
#include <time.h>          // time_t, gmtime_r
#include "OCArray.h"       // For OCArrayCallBacks, OCArrayCreateMutable, etc.
#include "OCData.h"        // For OCDataGetLength, OCDataGetBytesPtr
#include "OCDictionary.h"  // For OCMutableDictionaryRef, OCDictionaryCreateMutable, etc.
static OCMutableDictionaryRef impl_constantStringTable = NULL;
// Forward declaration for OCStringFindWithOptions
bool OCStringFindWithOptions(OCStringRef string, OCStringRef stringToFind, OCRange rangeToSearch, OCOptionFlags compareOptions, OCRange *result);
// Forward declaration for getConstantStringTable
static OCMutableDictionaryRef getConstantStringTable(void);
// Callbacks for OCArray containing OCRange structs
static void impl_OCRangeReleaseCallBack(const void *value) {
    if (value) {
        free((void *)value);
    }
}
// NOP retain callback for OCRange objects, as they are simple malloc'd structs
// and OCArrayAppendValue might attempt to call a retain callback.
static const void *impl_OCRangeNopRetainCallBack(const void *value) {
    return value;  // OCRanges are not further reference counted themselves by this callback
}
static const OCArrayCallBacks kOCRangeArrayCallBacks = {
    0,                              // version
    impl_OCRangeNopRetainCallBack,  // retain
    impl_OCRangeReleaseCallBack,    // release
    NULL,                           // copyDescription
    NULL                            // equal
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
             ((s[1] & 0x3F) << 6) |
             (s[2] & 0x3F);
        *p += 3;
    } else if ((s[0] & 0xF8) == 0xF0 &&
               (s[1] & 0xC0) == 0x80 &&
               (s[2] & 0xC0) == 0x80 &&
               (s[3] & 0xC0) == 0x80) {
        // 4-byte sequence
        ch = ((s[0] & 0x07) << 18) |
             ((s[1] & 0x3F) << 12) |
             ((s[2] & 0x3F) << 6) |
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
    uint32_t cp = 0;
    bool in_emoji_sequence = false;
    // Enhanced UTF-8 code point counter that handles:
    // 1. Combining characters (e.g., é̀́)
    // 2. Emoji ZWJ sequences (e.g., 👨‍👩‍👧‍👦)
    // 3. Regional indicators for flag emojis (e.g., 🇺🇸)
    while (*p) {
        uint32_t prev_cp = cp;
        cp = utf8_next(&p);
        // Skip counting if this is a combining character (0x0300-0x036F, 0x1AB0-0x1AFF, 0x1DC0-0x1DFF, 0x20D0-0x20FF, 0xFE20-0xFE2F)
        if ((cp >= 0x0300 && cp <= 0x036F) ||
            (cp >= 0x1AB0 && cp <= 0x1AFF) ||
            (cp >= 0x1DC0 && cp <= 0x1DFF) ||
            (cp >= 0x20D0 && cp <= 0x20FF) ||
            (cp >= 0xFE20 && cp <= 0xFE2F)) {
            continue;  // Skip combining character
        }
        // Handle Zero Width Joiner sequences (ZWJ is U+200D)
        if (cp == 0x200D) {
            in_emoji_sequence = true;
            continue;  // Skip ZWJ
        }
        // Handle regional indicators for flag emojis (e.g., 🇺🇸)
        // Regional indicators are in range U+1F1E6 to U+1F1FF
        if (prev_cp >= 0x1F1E6 && prev_cp <= 0x1F1FF &&
            cp >= 0x1F1E6 && cp <= 0x1F1FF) {
            continue;  // Skip second part of flag emoji
        }
        // Handle emoji variation selectors (VS15 U+FE0E and VS16 U+FE0F)
        if (cp == 0xFE0E || cp == 0xFE0F) {
            continue;  // Skip variation selector
        }
        // If we were in emoji sequence and this isn't a continuation,
        // count it as one emoji
        if (in_emoji_sequence) {
            if (*p && utf8_next(&p) != 0x200D) {
                // End of sequence
                in_emoji_sequence = false;
            }
            continue;  // Skip the character that follows the ZWJ
        }
        len++;
    }
    return len;
}
// ——— Helper: replace all occurrences of a C‐string ———
// Returns a newly malloc’d buffer, and sets *count to the number of replacements.
// The returned buffer always has enough space for the final string + '\0'.
static char *str_replace(const char *orig,
                         const char *rep,
                         const char *with,
                         int64_t *count) {
    const char *ins = orig;
    char *result;
    char *tmp;
    size_t len_rep = strlen(rep);
    size_t len_with = strlen(with);
    size_t orig_len = strlen(orig);
    int64_t n = 0;
    if (len_rep == 0) {
        *count = 0;
        return NULL;  // nothing to replace
    }
    // 1) Count occurrences of 'rep' in 'orig'
    while ((ins = strstr(ins, rep))) {
        n++;
        ins += len_rep;
    }
    *count = n;
    // 2) Compute new length safely: if 'with' is longer, we add the extra;
    //    otherwise we leave orig_len (allocating a bit more than strictly needed
    //    but never less).
    size_t delta = (len_with > len_rep)
                       ? (len_with - len_rep)
                       : 0;
    size_t new_len = orig_len + delta * n;
    // 3) Allocate new buffer (+1 for NUL)
    result = malloc(new_len + 1);
    if (NULL == result) {
        fprintf(stderr, "str_replace: Memory allocation failed.\n");
        *count = 0;
        return NULL;
    }
    // 4) Perform the replacement pass
    tmp = result;
    ins = orig;
    while (n--) {
        const char *pos = strstr(ins, rep);
        size_t front = pos - ins;
        // copy up to the match
        memcpy(tmp, ins, front);
        tmp += front;
        // copy the replacement
        memcpy(tmp, with, len_with);
        tmp += len_with;
        // advance past the matched segment
        ins += front + len_rep;
    }
    // 5) Copy any remaining text (including the final '\0')
    memcpy(tmp, ins, strlen(ins) + 1);
    return result;
}
static OCTypeID kOCStringID = kOCNotATypeID;
// OCString Opaque Type
struct impl_OCString {
    OCBase base;
    // OCString Type attributes  - order of declaration is essential
    char *string;
    uint64_t length;
    uint64_t capacity;
};
static bool impl_OCStringEqual(const void *theType1, const void *theType2) {
    OCStringRef theString1 = (OCStringRef)theType1;
    OCStringRef theString2 = (OCStringRef)theType2;
    // 1. If they are the same instance, they are equal.
    if (theString1 == theString2) return true;
    // 2. If either is NULL (and they are not the same instance, checked above), they are not equal.
    if (NULL == theString1 || NULL == theString2) return false;
    // 3. Now it's safe to access members. Check typeID.
    if (theString1->base.typeID != theString2->base.typeID) return false;
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
static void impl_OCStringFinalize(const void *theType) {
    if (NULL == theType) return;
    OCStringRef theString = (OCStringRef)theType;
    free(theString->string);
}
static OCStringRef impl_OCStringCopyFormattingDesc(OCTypeRef cf) {
    if (!cf) return NULL;
    const OCBase *base = (const OCBase *)cf;
    if (base->typeID != OCStringGetTypeID()) {
        fprintf(stderr, "[OCStringCopyFormattingDesc] Warning: expected OCString typeID, got %u\n", base->typeID);
        return NULL;
    }
    return OCStringCreateCopy((OCStringRef)cf);
}
static cJSON *
impl_OCStringCopyJSON(const void *obj, bool typed, OCStringRef *outError) {
    return OCStringCopyAsJSON((OCStringRef)obj, typed, outError);
}

static void *impl_OCStringDeepCopy(const void *obj) {
    OCStringRef src = (OCStringRef)obj;
    if (!src) return NULL;
    return (void *)OCStringCreateCopy(src);  // Safe cast, as OCStringCreateCopy allocates
}
static void *impl_OCStringDeepCopyMutable(const void *obj) {
    OCStringRef src = (OCStringRef)obj;
    if (!src) return NULL;
    return OCStringCreateMutableCopy(src);  // Returns OCMutableStringRef
}
OCTypeID OCStringGetTypeID(void) {
    if (kOCStringID == kOCNotATypeID) kOCStringID = OCRegisterType("OCString", (OCTypeRef (*)(cJSON *, OCStringRef *))OCStringCreateFromJSON);
    return kOCStringID;
}
static struct impl_OCString *OCStringAllocate() {
    struct impl_OCString *obj = OCTypeAlloc(struct impl_OCString,
                                            OCStringGetTypeID(),
                                            impl_OCStringFinalize,
                                            impl_OCStringEqual,
                                            impl_OCStringCopyFormattingDesc,
                                            impl_OCStringCopyJSON,
                                            impl_OCStringDeepCopy,
                                            impl_OCStringDeepCopyMutable);
    obj->string = NULL;
    obj->length = 0;
    obj->capacity = 0;
    return obj;
}
cJSON *OCStringCopyAsJSON(OCStringRef str, bool typed, OCStringRef *outError) {
    if (outError) *outError = NULL;

    if (!str) {
        if (outError) *outError = STR("OCString is NULL");
        return cJSON_CreateNull();
    }

    const char *s = OCStringGetCString(str);
    if (!s) {
        if (outError) *outError = STR("Failed to get C string from OCString");
        return cJSON_CreateNull();
    }

    // Strings are native JSON types, no wrapping needed even for typed serialization
    cJSON *result = cJSON_CreateString(s);
    if (!result) {
        if (outError) *outError = STR("Failed to create JSON string");
        return cJSON_CreateNull();
    }

    return result;
}
OCStringRef OCStringCreateFromJSON(cJSON *json, OCStringRef *outError) {
    if (!json) {
        if (outError) *outError = STR("JSON input is NULL");
        return NULL;
    }

    if (!cJSON_IsString(json)) {
        if (outError) *outError = STR("JSON input is not a string");
        return NULL;
    }

    const char *s = json->valuestring;
    if (!s) {
        if (outError) *outError = STR("JSON string value is NULL");
        return NULL;
    }

    OCStringRef result = OCStringCreateWithCString(s);
    if (!result && outError) {
        *outError = STR("Failed to create OCString");
    }
    return result;
}
// -----------------------------------------------------------------------------
// Core OCString Constructors
// -----------------------------------------------------------------------------
// A single mutable dictionary to intern all constant strings
void cleanupConstantStringTable(void) {
    if (!impl_constantStringTable) return;
    OCArrayRef keys = OCDictionaryCreateArrayWithAllKeys(impl_constantStringTable);
    for (size_t i = 0; i < OCArrayGetCount(keys); ++i) {
        OCStringRef key = (OCStringRef)OCArrayGetValueAtIndex(keys, i);
        OCStringRef value = (OCStringRef)OCDictionaryGetValue(impl_constantStringTable, key);
        OCTypeSetStaticInstance(value, false);
        OCDictionaryRemoveValue(impl_constantStringTable, key);
    }
    OCRelease(keys);
    OCRelease(impl_constantStringTable);
    impl_constantStringTable = NULL;  // ← clear the global cache
}
static OCMutableDictionaryRef getConstantStringTable(void) {
    if (!impl_constantStringTable) {
        impl_constantStringTable = OCDictionaryCreateMutable(0);
    }
    return impl_constantStringTable;
}
OCStringRef impl_OCStringMakeConstantString(const char *cStr) {
    // Create a temporary OCString to look up in the table
    OCStringRef tmp = OCStringCreateWithCString(cStr);
    OCMutableDictionaryRef table = getConstantStringTable();
    if (OCDictionaryContainsKey(table, tmp)) {
        // Already interned: return existing and drop the temporary
        OCStringRef existing = (OCStringRef)OCDictionaryGetValue(table, tmp);
        OCRelease(tmp);
        return existing;
    } else {
        // First time: add it and set it as static instance
        OCTypeSetStaticInstance(tmp, true);
        OCDictionaryAddValue(table, tmp, tmp);
        OCRelease(tmp);
        return tmp;
    }
}
OCStringRef
OCStringCreateWithExternalRepresentation(OCDataRef data) {
    if (!data || OCDataGetLength(data) == 0)
        return STR("");
    size_t len = OCDataGetLength(data);
    const uint8_t *bytes = OCDataGetBytesPtr(data);
    // make a nul-terminated buffer
    char *buf = (char *)malloc(len + 1);
    if (!buf) return STR("");
    memcpy(buf, bytes, len);
    buf[len] = '\0';
    // now feed it into your existing UTF-8 string constructor
    OCStringRef s = OCStringCreateWithCString(buf);
    free(buf);
    return s;
}
OCStringRef OCStringCreateCopy(OCStringRef theString) {
    return theString ? OCStringCreateWithCString(theString->string) : NULL;
}
OCMutableStringRef OCStringCreateMutable(uint64_t capacity) {
    struct impl_OCString *s = OCStringAllocate();
    if (!s) return NULL;
    s->capacity = capacity;
    s->length = 0;
    s->string = malloc(capacity + 1);
    if (NULL == s->string) {
        fprintf(stderr, "OCStringCreateMutable: Memory allocation failed for string.\n");
        OCRelease(s);
        return NULL;
    }
    s->string[0] = '\0';
    return (OCMutableStringRef)s;
}
// ——— Create a mutable OCString from a C‐string ———
OCMutableStringRef
OCMutableStringCreateWithCString(const char *cString) {
    if (!cString) return NULL;
    // Allocate a new OCString instance (fills in the base type info)
    OCMutableStringRef s = OCStringAllocate();
    if (!s) return NULL;
    // Measure byte‐length vs. code‐point length
    size_t byteLen = strlen(cString);
    s->capacity = byteLen;
    s->length = oc_utf8_strlen(cString);
    // Allocate memory for the string content
    s->string = malloc(byteLen + 1);
    if (NULL == s->string) {
        fprintf(stderr, "OCMutableStringCreateWithCString: Memory allocation failed for string.\n");
        OCRelease(s);  // Free the allocated OCString instance
        return NULL;
    }
    // Copy the C string into the allocated memory
    memcpy(s->string, cString, byteLen + 1);
    return s;
}
// ——— Immutable wrapper onto the mutable creator ———
OCStringRef OCStringCreateWithCString(const char *cString) {
    OCStringRef theString = (OCStringRef)OCMutableStringCreateWithCString(cString);
    return theString;
}
OCMutableStringRef OCStringCreateMutableCopy(OCStringRef theString) {
    if (!theString) return NULL;
    struct impl_OCString *s = OCStringAllocate();
    if (!s) return NULL;
    // Preserve both byte‐capacity and code‐point length
    size_t byteLen = strlen(theString->string);
    s->capacity = byteLen;
    s->length = theString->length;
    s->string = malloc(byteLen + 1);
    if (NULL == s->string) {
        fprintf(stderr, "OCStringCreateMutableCopy: Memory allocation failed for string.\n");
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
        if ((uint64_t)range.location == str->length && range.length == 0) {
            off1 = off2 = strlen(str->string);
        } else {
            return NULL;
        }
    }
    size_t byteCount = off2 - off1;
    char *buf = malloc(byteCount + 1);
    if (NULL == buf) {
        fprintf(stderr, "OCStringCreateWithSubstring: Memory allocation failed for substring buffer.\n");
        return NULL;
    }
    memcpy(buf, str->string + off1, byteCount);
    buf[byteCount] = '\0';
    OCStringRef sub = OCStringCreateWithCString(buf);
    free(buf);
    return sub;
}
#include <stdio.h>   // fputs, stdout
#include <stdlib.h>  // realloc, free
#include <string.h>  // strlen, strdup
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
    return impl_OCStringEqual(a, b);
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
    if (!s || !cString || !s->string) return;  // Ensure s and s->string are valid
    size_t append_cString_byte_len = strlen(cString);
    if (append_cString_byte_len == 0) return;  // Nothing to append
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
            new_capacity = 16;  // Default initial capacity if appending to an empty string from OCStringCreateMutable(0)
        }
        while (required_total_content_byte_len > new_capacity) {
            new_capacity *= 2;
        }
        // The while loop ensures new_capacity >= required_total_content_byte_len
        char *new_s_string_buffer = realloc(s->string, new_capacity + 1);  // +1 for NUL
        if (NULL == new_s_string_buffer) {
            fprintf(stderr, "OCStringAppendCString: Memory allocation failed for new string buffer.\n");
            return;
        }
        s->string = new_s_string_buffer;
        s->capacity = new_capacity;
    }
    // Append cString to the end of the current content in s->string
    // memcpy is safe here because we've ensured s->string has enough space.
    memcpy(s->string + current_content_byte_len, cString, append_cString_byte_len + 1);  // +1 to copy NUL from cString
    // Update the code-point length
    s->length += oc_utf8_strlen(cString);  // oc_utf8_strlen calculates codepoints from a C-string
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
    size_t tailBytes = totalBytes - off2;
    memmove(s->string + off1,
            s->string + off2,
            tailBytes + 1);  // include NUL
    s->length -= range.length;
    s->capacity = totalBytes - (off2 - off1);
}
void OCStringInsert(OCMutableStringRef str, int64_t idx, OCStringRef insertedStr) {
    if (!str || !insertedStr) return;
    OCStringReplace(str, OCRangeMake(idx, 0), insertedStr);
}
void OCStringReplace(OCMutableStringRef s, OCRange range, OCStringRef rep) {
    if (!s || !rep) return;
    // 1) Find byte offsets of the code‐point range
    ptrdiff_t off1 = oc_utf8_offset_for_index(s->string, range.location);
    ptrdiff_t off2 = oc_utf8_offset_for_index(s->string, range.location + range.length);
    if (off1 < 0 || off2 < 0 || off2 < off1) return;
    // 2) Figure out how many bytes the parts have
    size_t origBytes = strlen(s->string);
    size_t repBytes = rep->string
                          ? strnlen(rep->string, rep->capacity)
                          : 0;
    size_t tailBytes = origBytes - off2;
    // 3) New data‐byte total (no NUL)
    size_t newDataBytes = off1 + repBytes + tailBytes;
    size_t newAlloc = newDataBytes + 1;  // +1 for the terminator
    // 4) Allocate fresh buffer and do three bounded memcpy’s
    char *newbuf = malloc(newAlloc);
    if (NULL == newbuf) {
        fprintf(stderr, "OCStringReplace: Memory allocation failed for new string buffer.\n");
        return;
    }
    //   a) head
    memcpy(newbuf, s->string, off1);
    //   b) replacement
    if (repBytes > 0) {
        memcpy(newbuf + off1, rep->string, repBytes);
    }
    //   c) tail + terminator
    memcpy(newbuf + off1 + repBytes,
           s->string + off2,
           tailBytes + 1);
    // 5) Swap in the new buffer
    free(s->string);
    s->string = newbuf;
    s->capacity = newDataBytes;
    s->length = oc_utf8_strlen(newbuf);
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
void OCStringTrim(OCMutableStringRef s, OCStringRef t) {
    if (!s || !t || t->length == 0 || s->length == 0) return;
    OCRange r;
    // Trim from the beginning
    while (s->length >= t->length) {
        if (!OCStringFindWithOptions(s, t, OCRangeMake(0, t->length), 0, &r) || r.location != 0) {
            break;
        }
        OCStringDelete(s, OCRangeMake(0, t->length));
    }
    // Trim from the end
    while (s->length >= t->length) {
        OCRange suffixSearch = OCRangeMake(s->length - t->length, t->length);
        if (!OCStringFindWithOptions(s, t, suffixSearch, 0, &r) ||
            r.location != suffixSearch.location) {
            break;
        }
        OCStringDelete(s, suffixSearch);
    }
}
void OCStringTrimWhitespace(OCMutableStringRef s) {
    if (!s || !s->string || s->length == 0) return;
    uint64_t start_cp_idx = 0;                    // code-point index
    size_t current_byte_len = strlen(s->string);  // Get initial byte length for boundary checks
    // Find the first non-space character from the beginning (code-point wise)
    while (start_cp_idx < s->length) {
        ptrdiff_t byte_offset = oc_utf8_offset_for_index(s->string, start_cp_idx);
        if (byte_offset < 0 || (size_t)byte_offset >= current_byte_len) {
            // Invalid offset or past end of actual C-string, stop.
            break;
        }
        const char *p_char_for_cp = s->string + byte_offset;
        uint32_t cp = utf8_next(&p_char_for_cp);  // p_char_for_cp is advanced
        if (cp != ' ') break;                     // Found non-space
        start_cp_idx++;                           // Move to next code-point index
    }
    uint64_t end_cp_idx = s->length;  // code-point index (exclusive)
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
            s->capacity = 0;
            s->length = 0;
            // OCRelease(s); // This would lead to double free if s is used after this. Let caller handle.
            return;
        }
        s->length = 0;
        s->capacity = 0;  // strlen("") is 0
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
            s->capacity = 0;
            s->length = 0;
            return;
        }
        s->length = 0;
        s->capacity = 0;
        return;
    }
    // Replace the content of 's' with the content of 'sub'
    free(s->string);                                     // Free the old string buffer of 's'
    s->string = strdup(sub->string ? sub->string : "");  // Duplicate content from 'sub'
    if (!s->string) {                                    // strdup failed
        s->capacity = 0;
        s->length = 0;
        // 'sub' was successfully created, so it needs to be released.
        OCRelease(sub);
        // 's' is now in a bad state (original buffer freed, new one failed to alloc).
        return;
    }
    s->capacity = strlen(s->string);  // Byte capacity from the new string
    s->length = sub->length;          // Code-point length from 'sub'
    OCRelease(sub);                   // Release the temporary substring 'sub'
}
bool OCStringTrimMatchingParentheses(OCMutableStringRef s) {
    if (!s || !s->string || s->length < 2) return false;  // Added s->string check
    bool trimmed = false;
    // Trim only one layer of parentheses as per test expectation for ((double)) -> (double)
    if (OCStringGetCharacterAtIndex(s, 0) == '(' &&
        OCStringGetCharacterAtIndex(s, s->length - 1) == ')') {
        OCStringDelete(s, OCRangeMake(s->length - 1, 1));  // Delete last parenthesis
        OCStringDelete(s, OCRangeMake(0, 1));              // Delete first parenthesis
        OCStringTrimWhitespace(s);                         // Trim whitespace after removing parentheses
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
    if (NULL == buf) {
        fprintf(stderr, "OCFloatCreateStringValue: Memory allocation failed for string buffer.\n");
        return NULL;
    }
    snprintf(buf, n + 1, "%g", value);
    OCStringRef s = OCStringCreateWithCString(buf);
    free(buf);
    return s;
}
// ——— Create an OCString representing a double value ———
OCStringRef OCDoubleCreateStringValue(double value) {
    int n = snprintf(NULL, 0, "%g", value);
    char *buf = malloc(n + 1);
    if (NULL == buf) {
        fprintf(stderr, "OCDoubleCreateStringValue: Memory allocation failed for string buffer.\n");
        return NULL;
    }
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
    if (NULL == buf) {
        fprintf(stderr, "OCFloatComplexCreateStringValue: Memory allocation failed for string buffer.\n");
        return NULL;
    }
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
    if (NULL == buf) {
        fprintf(stderr, "OCDoubleComplexCreateStringValue: Memory allocation failed for string buffer.\n");
        return NULL;
    }
    snprintf(buf, n + 1, fmt, real, imag);
    OCStringRef s = OCStringCreateWithCString(buf);
    free(buf);
    return s;
}
// ——— Find first literal occurrence (code-point semantics) ———
OCRange OCStringFind(OCStringRef string,
                     OCStringRef stringToFind,
                     OCOptionFlags compareOptions) {
    if (!string || !stringToFind || !string->string || !stringToFind->string) {
        return OCRangeMake(kOCNotFound, 0);
    }
    OCRange result_range;
    if (OCStringFindWithOptions(string,
                                stringToFind,
                                OCRangeMake(0, string->length),  // Search the entire string
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
                                OCStringRef replaceStr) {
    if (!s || !findStr || !replaceStr) return 0;
    int64_t count = 0;
    char *newBuf = str_replace(s->string,
                               findStr->string,
                               replaceStr->string,
                               &count);
    if (!newBuf) return 0;
    free(s->string);
    s->string = newBuf;
    s->capacity = strlen(newBuf);
    s->length = oc_utf8_strlen(newBuf);
    return count;
}
// ——— Replace within a specified code-point range ———
int64_t OCStringFindAndReplace(OCMutableStringRef s,
                               OCStringRef findStr,
                               OCStringRef replaceStr,
                               OCRange rangeToSearch,
                               OCOptionFlags compareOptions) {
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
    if (rangeToSearch.location < 0 || (uint64_t)rangeToSearch.location >= s->length) {
        return 0;  // Start of search range is already past the end of the string
    }
    if (rangeToSearch.length < 0 || (uint64_t)rangeToSearch.location + (uint64_t)rangeToSearch.length > s->length) {
        rangeToSearch.length = s->length - (uint64_t)rangeToSearch.location;
    }
    // If the effective search range is empty, no match is possible.
    if (rangeToSearch.length == 0) {
        return 0;
    }
    // If search range is shorter than findStr, no match is possible.
    if (rangeToSearch.length < 0 || (uint64_t)rangeToSearch.length < findStr->length) {
        return 0;
    }
    int64_t count = 0;
    uint64_t scan_start_location_in_s = rangeToSearch.location;
    // This is the end boundary of the original search area, relative to the original string 's'.
    // We need to track this because 's' will change length.
    uint64_t original_search_area_end_location_in_s = rangeToSearch.location + rangeToSearch.length;
    int64_t cumulative_length_change = 0;  // Tracks total change in s->length due to replacements
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
        if (current_look_in_range.length < 0 || (uint64_t)current_look_in_range.length < findStr->length) {
            break;
        }
        OCRange found_at_range_in_s;  // Will store the range of the found substring in the current state of 's'
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
bool OCStringFindWithOptions(OCStringRef string,
                             OCStringRef stringToFind,
                             OCRange rangeToSearch,
                             OCOptionFlags compareOptions,
                             OCRange *result) {
    if (!string || !stringToFind || !result) return false;
    uint64_t strLen = OCStringGetLength(string);
    uint64_t needleLen = OCStringGetLength(stringToFind);
    // Sanity: empty needle or out-of-bounds or not enough space to find needle
    if (needleLen == 0 ||
        rangeToSearch.location < 0 || (uint64_t)rangeToSearch.location > strLen ||
        rangeToSearch.length < 0 || (uint64_t)rangeToSearch.location + (uint64_t)rangeToSearch.length > strLen ||
        needleLen > (uint64_t)rangeToSearch.length) {
        return false;
    }
    bool caseInsensitive = (compareOptions & kOCCompareCaseInsensitive) != 0;
    uint64_t maxStart = (uint64_t)rangeToSearch.location + (uint64_t)rangeToSearch.length - needleLen;
    for (uint64_t pos = (uint64_t)rangeToSearch.location; pos <= maxStart; ++pos) {
        bool match = true;
        for (uint64_t j = 0; j < needleLen; ++j) {
            uint32_t c1 = OCStringGetCharacterAtIndex(string, pos + j);
            uint32_t c2 = OCStringGetCharacterAtIndex(stringToFind, j);
            if (caseInsensitive) {
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
                OCStringCompareFlags compareOptions) {
    // Handle NULL pointers
    if (theString1 == theString2) return kOCCompareEqualTo;
    if (theString1 == NULL) return kOCCompareLessThan;
    if (theString2 == NULL) return kOCCompareGreaterThan;
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
// Returns a pointer just after the full specifier, writes up to maxlen bytes (including NUL) to out.
static const char *parse_printf_spec(const char *p, char *out, size_t maxlen, char *lenmod, size_t lenmod_max) {
    const char *start = p;
    *lenmod = 0;
    if (*p != '%') return NULL;
    p++;  // skip '%'
    // Flags
    while (strchr("-+ #0", *p)) p++;
    // Width
    while (*p >= '0' && *p <= '9') p++;
    if (*p == '*') p++;
    // Precision
    if (*p == '.') {
        p++;
        while (*p >= '0' && *p <= '9') p++;
        if (*p == '*') p++;
    }
    // Length modifier ("hh", "h", "ll", "l", "j", "z", "t", "L", possibly more)
    if ((p[0] == 'h' && p[1] == 'h')) {
        strncpy(lenmod, "hh", lenmod_max);
        p += 2;
    } else if ((p[0] == 'l' && p[1] == 'l')) {
        strncpy(lenmod, "ll", lenmod_max);
        p += 2;
    } else if (strchr("hljztL", *p)) {
        lenmod[0] = *p;
        lenmod[1] = 0;
        p++;
    } else {
        lenmod[0] = 0;
    }
    // Conversion specifier
    if (*p) p++;
    size_t len = (size_t)(p - start);
    if (len >= maxlen) len = maxlen - 1;
    memcpy(out, start, len);
    out[len] = '\0';
    return p;
}
#ifdef _WIN32
// Windows-specific: Signal handling for va_arg safety
static jmp_buf va_arg_jmpbuf;
static volatile int va_arg_segfault = 0;

static void va_arg_signal_handler(int sig) {
    if (sig == SIGSEGV) {
        va_arg_segfault = 1;
        longjmp(va_arg_jmpbuf, 1);
    }
}
#endif

/**
 * @brief Appends formatted text to a mutable OCString.
 * @param result Mutable OCString.
 * @param format Format OCString.
 * @param args Variable arguments for the format string.
 * @param max_args Maximum number of arguments to process.
 * @ingroup OCString
 */
// Pass arg_index by pointer to update it
static void OCStringAppendFormatWithArgumentsSafe(
    OCMutableStringRef result,
    OCStringRef format,
    va_list args,
    int max_args,
    OCStringRef *outError) {
    if (!result || !format || !format->string) return;
    int arg_index = 0;
    const char *f = format->string;
    va_list arglist;
    va_copy(arglist, args);

    #ifdef _WIN32
    // Set up signal handler for Windows va_arg protection
    void (*old_handler)(int) = signal(SIGSEGV, va_arg_signal_handler);
    va_arg_segfault = 0;

    if (setjmp(va_arg_jmpbuf) != 0) {
        // We caught a segfault from va_arg - insufficient arguments
        fprintf(stderr, "[OCString] WARNING: Caught segfault due to insufficient arguments\n");
        OCStringAppendCString(result, "[INSUFFICIENT_ARGS]");
        signal(SIGSEGV, old_handler);  // Restore original handler
        va_end(arglist);
        return;
    }
    #endif

    while (*f) {
        if (f[0] == '%' && f[1] == '@') {
            if (arg_index >= max_args) {
                OCStringAppendCString(result, "[MISSING]");
                if (outError && !*outError) {
                    *outError = STR("Not enough arguments for %@");
                }
                arg_index++;
                f += 2;
                continue;
            } else {
                OCStringRef s = va_arg(arglist, OCStringRef);
                // Defensive: check plausibility before dereferencing
                if (!s) {
                    // DO NOT append "[NULL]"—skip, just set error
                    if (outError && !*outError) {
                        *outError = STR("NULL OCStringRef passed to %@");
                    }
                } else if ((uintptr_t)s < 4096) {  // catch NULL/tiny invalid pointers
                    if (outError && !*outError) {
                        *outError = STR("Invalid pointer passed to %@");
                    }
                } else if (s->base.typeID != OCStringGetTypeID()) {
                    if (outError && !*outError) {
                        *outError = STR("Invalid type passed to %@ (not an OCString)");
                    }
                } else if (s->string) {
                    OCStringAppendCString(result, s->string);
                } else {
                    // Don't append, just set error
                    if (outError && !*outError) {
                        *outError = STR("OCStringRef has NULL string content");
                    }
                }
                arg_index++;
                f += 2;
            }
        } else if (f[0] == '%' && f[1] == '%') {
            OCStringAppendCString(result, "%");
            f += 2;
        } else if (f[0] == '%') {
            char subfmt[32], lenmod[4];
            const char *after = parse_printf_spec(f, subfmt, sizeof(subfmt), lenmod, sizeof(lenmod));
            if (!after) {
                OCStringAppendCString(result, "%");
                f++;
                continue;
            }
            char spec = subfmt[strlen(subfmt) - 1];
            char buf[128] = {0};
            if (arg_index >= max_args) {
                OCStringAppendCString(result, "[MISSING]");
                if (outError && !*outError) {
                    *outError = STR("Not enough arguments for printf format specifier");
                }
                arg_index++;
                f = after;
                continue;
            }
            // Now it's safe to va_arg (but C can't check type here)
            if ((spec == 'd' || spec == 'i')) {
                if (strcmp(lenmod, "ll") == 0)
                    snprintf(buf, sizeof(buf), subfmt, va_arg(arglist, long long));
                else if (strcmp(lenmod, "l") == 0)
                    snprintf(buf, sizeof(buf), subfmt, va_arg(arglist, long));
                else if (strcmp(lenmod, "h") == 0)
                    snprintf(buf, sizeof(buf), subfmt, (short)va_arg(arglist, int));  // 'short' promoted to int
                else if (strcmp(lenmod, "hh") == 0)
                    snprintf(buf, sizeof(buf), subfmt, (signed char)va_arg(arglist, int));  // 'char' promoted
                else
                    snprintf(buf, sizeof(buf), subfmt, va_arg(arglist, int));
            } else if (spec == 'u' || spec == 'x' || spec == 'X' || spec == 'o') {
                if (strcmp(lenmod, "ll") == 0)
                    snprintf(buf, sizeof(buf), subfmt, va_arg(arglist, unsigned long long));
                else if (strcmp(lenmod, "l") == 0)
                    snprintf(buf, sizeof(buf), subfmt, va_arg(arglist, unsigned long));
                else if (strcmp(lenmod, "h") == 0)
                    snprintf(buf, sizeof(buf), subfmt, (unsigned short)va_arg(arglist, unsigned int));
                else if (strcmp(lenmod, "hh") == 0)
                    snprintf(buf, sizeof(buf), subfmt, (unsigned char)va_arg(arglist, unsigned int));
                else
                    snprintf(buf, sizeof(buf), subfmt, va_arg(arglist, unsigned int));
            } else if (spec == 'f' || spec == 'e' || spec == 'E' || spec == 'g' || spec == 'G' || spec == 'a' || spec == 'A') {
                snprintf(buf, sizeof(buf), subfmt, va_arg(arglist, double));
            } else if (spec == 's') {
                const char *v = va_arg(arglist, const char *);
                snprintf(buf, sizeof(buf), subfmt, v ? v : "(null)");
            } else if (spec == 'c') {
                int v = va_arg(arglist, int);
                snprintf(buf, sizeof(buf), subfmt, v);
            } else if (spec == 'p') {
                const void *v = va_arg(arglist, void *);
                snprintf(buf, sizeof(buf), subfmt, v);
            } else {
                snprintf(buf, sizeof(buf), "%s", subfmt);
            }
            OCStringAppendCString(result, buf);
            arg_index++;
            f = after;
        } else {
            const char tmp[2] = {*f, '\0'};
            OCStringAppendCString(result, tmp);
            f++;
        }
    }

    #ifdef _WIN32
    // Restore original signal handler
    signal(SIGSEGV, old_handler);
    #endif

    va_end(arglist);
}
// Helper to count how many format arguments are expected (including %@)
static int count_format_args(const char *f) {
    int count = 0;
    while (*f) {
        if (f[0] == '%' && f[1] == '%') {
            f += 2;
            continue;
        }  // Escaped percent
        if (f[0] == '%' && f[1] == '@') {
            count++;
            f += 2;
            continue;
        }
        if (f[0] == '%') {
            f++;  // skip '%'
            while (strchr("-+ #0", *f)) f++;
            while (*f >= '0' && *f <= '9') f++;
            if (*f == '*') f++;
            if (*f == '.') {
                f++;
                while (*f >= '0' && *f <= '9') f++;
                if (*f == '*') f++;
            }
            while (strchr("hljztL", *f)) f++;
            if (*f) {
                count++;
                f++;
            }
            continue;
        }
        f++;
    }
    return count;
}
OCStringRef OCStringCreateWithFormat(OCStringRef format, ...) {
    if (!format || !format->string) return NULL;
    OCMutableStringRef result = OCStringCreateMutable(0);
    int max_args = count_format_args(format->string);
    va_list args;
    va_start(args, format);
    OCStringAppendFormatWithArgumentsSafe(result, format, args, max_args, NULL);
    va_end(args);
    return (OCStringRef)result;
}
void OCStringAppendFormat(OCMutableStringRef theString, OCStringRef format, ...) {
    if (!theString || !format || !format->string) return;
    int max_args = count_format_args(format->string);
    va_list args;
    va_start(args, format);
    OCStringAppendFormatWithArgumentsSafe(theString, format, args, max_args, NULL);
    va_end(args);
}
/**
 * @brief Creates an array of ranges where a substring is found.
 */
OCArrayRef OCStringCreateArrayWithFindResults(OCStringRef string,
                                              OCStringRef stringToFind,
                                              OCRange rangeToSearch,
                                              OCOptionFlags compareOptions) {
    if (!string || !stringToFind) return NULL;
    // Clip the search range to the string’s length
    if (rangeToSearch.location < 0 || (uint64_t)rangeToSearch.location > string->length) return NULL;
    if (rangeToSearch.length < 0 || (uint64_t)rangeToSearch.location + (uint64_t)rangeToSearch.length > string->length) {
        rangeToSearch.length = string->length - (uint64_t)rangeToSearch.location;
    }
    OCMutableArrayRef result = OCArrayCreateMutable(0, &kOCRangeArrayCallBacks);
    if (!result) return NULL;
    OCRange search = rangeToSearch, found;
    while (OCStringFindWithOptions(string, stringToFind, search, compareOptions, &found)) {
        OCRange *r = malloc(sizeof(OCRange));
        if (NULL == r) {
            fprintf(stderr, "OCStringCreateArrayWithFindResults: Memory allocation failed for range.\n");
            OCRelease(result);
            return NULL;
        }
        *r = found;
        OCArrayAppendValue(result, r);
        // Advance past this match
        search.location = found.location + found.length;
        uint64_t end = (uint64_t)rangeToSearch.location + (uint64_t)rangeToSearch.length;
        if ((uint64_t)search.location >= end) break;
        search.length = end - search.location;
    }
    if (OCArrayGetCount(result) == 0) {
        OCRelease(result);
        return NULL;
    }
    return result;
}
OCArrayRef
OCStringCreateArrayBySeparatingStrings(OCStringRef string,
                                       OCStringRef separator) {
    if (!string || !string->string ||
        !separator || !separator->string)
        return NULL;
    // Full range of the input string in code-points
    OCRange fullRange = OCRangeMake(0, OCStringGetLength(string));
    // Find all occurrences of 'separator'
    OCArrayRef findResults =
        OCStringCreateArrayWithFindResults(string,
                                           separator,
                                           fullRange,
                                           0);
    if (!findResults) {
        // No occurrences or error => treat as single token
        OCMutableArrayRef single = OCArrayCreateMutable(1, &kOCTypeArrayCallBacks);
        OCArrayAppendValue(single, string);
        return single;
    }
    uint64_t sepCount = OCArrayGetCount(findResults);
    // Preallocate result slots (sepCount separators ⇒ sepCount+1 tokens)
    OCMutableArrayRef result =
        OCArrayCreateMutable((uint32_t)(sepCount + 1), &kOCTypeArrayCallBacks);
    // Walk through each found separator, slicing out the preceding chunk
    uint64_t prevEnd = 0;
    for (uint64_t i = 0; i < sepCount; ++i) {
        const OCRange *r = (const OCRange *)OCArrayGetValueAtIndex(findResults, i);
        // substring from prevEnd of length (r->location - prevEnd)
        OCRange slice = OCRangeMake(prevEnd, r->location - prevEnd);
        OCStringRef piece = OCStringCreateWithSubstring(string, slice);
        if (piece) {
            OCArrayAppendValue(result, piece);
            OCRelease(piece);
        }
        // advance past this separator
        prevEnd = r->location + r->length;
    }
    // Add final token after last separator
    if (prevEnd <= (uint64_t)fullRange.length) {
        uint64_t sliceLength = fullRange.length - prevEnd;
        OCRange slice = OCRangeMake((OCIndex)prevEnd, (OCIndex)sliceLength);
        OCStringRef piece = OCStringCreateWithSubstring(string, slice);
        if (piece) {
            OCArrayAppendValue(result, piece);
            OCRelease(piece);
        }
    }
    OCRelease(findResults);
    return result;
}
OCStringRef OCCreateISO8601Timestamp(void) {
    char buf[32];
    time_t now = time(NULL);
    struct tm tm_now;
#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tm_now, &now);
#else
    gmtime_r(&now, &tm_now);
#endif
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm_now);
    return OCStringCreateWithCString(buf);
}
