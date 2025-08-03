/*
 * OCData.c – Improved Implementation with Efficient Allocation
 *
 * Provides immutable and mutable binary buffers. Includes support for copying,
 * mutation, serialization, and formatting.
 */
#include <complex.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>  // ptrdiff_t, size_t
#include <stdint.h>  // uint32_t
#include <stdio.h>
#include <stdlib.h>  // malloc, free, realloc
#include <string.h>  // strlen, strcmp, memcpy, memmove
#include "OCTypes.h"
static OCTypeID kOCDataID = kOCNotATypeID;
struct impl_OCData {
    OCBase base;
    uint8_t *bytes;
    uint64_t length;
    uint64_t capacity;
};
OCTypeID OCDataGetTypeID(void) {
    if (kOCDataID == kOCNotATypeID) kOCDataID = OCRegisterType("OCData");
    return kOCDataID;
}
static bool impl_OCDataEqual(const void *a_, const void *b_) {
    OCDataRef a = (OCDataRef)a_;
    OCDataRef b = (OCDataRef)b_;
    if (a->base.typeID != b->base.typeID) return false;
    if (a == b) return true;
    if (a->length != b->length) return false;
    return memcmp(a->bytes, b->bytes, a->length) == 0;
}
static void *impl_OCDataDeepCopy(const void *obj) {
    OCDataRef source = (OCDataRef)obj;
    if (!source) return NULL;
    return (void *)OCDataCreate(source->bytes, source->length);
}
static void *impl_OCDataDeepCopyMutable(const void *obj) {
    OCDataRef source = (OCDataRef)obj;
    if (!source) return NULL;
    return (void *)OCDataCreateMutableCopy(source->length, source);
}
OCDataRef OCDataCreateCopy(OCDataRef source) {
    return source ? OCDataCreate(source->bytes, source->length) : NULL;
}
OCMutableDataRef OCDataCreateMutableCopy(uint64_t capacity, OCDataRef source) {
    if (!source) return NULL;
    uint64_t actualCap = (capacity > source->length) ? capacity : source->length;
    OCMutableDataRef result = OCDataCreateMutable(actualCap);
    if (!result) return NULL;
    if (source->length > 0) {
        if (!OCDataAppendBytes(result, source->bytes, source->length)) {
            OCRelease(result);
            return NULL;
        }
    }
    return result;
}
OCStringRef OCDataCopyFormattingDesc(OCTypeRef cf) {
    if (!cf) return NULL;
    OCDataRef data = (OCDataRef)cf;
    const size_t maxPreview = 16;
    size_t previewLen = data->length < maxPreview ? data->length : maxPreview;
    size_t bufferSize = 64 + previewLen * 3;
    char *buffer = calloc(1, bufferSize);
    if (!buffer) return NULL;
    int offset = snprintf(buffer, bufferSize, "<OCData: %llu bytes, preview: ",
                          (unsigned long long)data->length);
    if (data->bytes) {
        for (size_t i = 0; i < previewLen && offset < (int)(bufferSize - 4); ++i) {
            offset += snprintf(buffer + offset, bufferSize - offset, "%02X ", data->bytes[i]);
        }
    }
    snprintf(buffer + offset, bufferSize - offset, data->length > maxPreview ? "…>" : ">");
    OCStringRef result = OCStringCreateWithCString(buffer);
    free(buffer);
    return result;
}
static cJSON *
impl_OCDataCopyJSON(const void *obj) {
    return OCDataCreateJSON((OCDataRef)obj);
}
static void impl_OCDataFinalize(const void *obj) {
    OCDataRef data = (OCDataRef)obj;
    if (data->bytes) free(data->bytes);
}
static struct impl_OCData *OCDataAllocate() {
    return OCTypeAlloc(struct impl_OCData,
                       OCDataGetTypeID(),
                       impl_OCDataFinalize,
                       impl_OCDataEqual,
                       OCDataCopyFormattingDesc,
                       impl_OCDataCopyJSON,
                       impl_OCDataDeepCopy,
                       impl_OCDataDeepCopyMutable);
}
OCDataRef OCDataCreate(const uint8_t *bytes, uint64_t length) {
    struct impl_OCData *data = OCDataAllocate();
    if (!data) return NULL;
    if (length > 0) {
        data->bytes = malloc(length);
        if (!data->bytes) {
            fprintf(stderr, "OCDataCreate: malloc failed\n");
            OCRelease(data);
            return NULL;
        }
        memcpy(data->bytes, bytes, length);
    } else {
        data->bytes = NULL;
    }
    data->length = length;
    data->capacity = length;
    return data;
}
OCDataRef OCDataCreateWithBytesNoCopy(const uint8_t *bytes, uint64_t length) {
    if (!bytes) return NULL;
    struct impl_OCData *data = OCDataAllocate();
    data->bytes = (uint8_t *)bytes;
    data->length = length;
    data->capacity = length;
    return data;
}
OCMutableDataRef OCDataCreateMutable(uint64_t capacity) {
    struct impl_OCData *data = OCDataAllocate();
    if (!data) return NULL;
    data->bytes = (capacity > 0) ? calloc(capacity, sizeof(uint8_t)) : NULL;
    if (capacity > 0 && !data->bytes) {
        fprintf(stderr, "OCDataCreateMutable: calloc failed\n");
        OCRelease(data);
        return NULL;
    }
    data->length = 0;
    data->capacity = capacity;
    return data;
}
uint64_t OCDataGetLength(OCDataRef data) {
    return data ? data->length : 0;
}
const uint8_t *OCDataGetBytesPtr(OCDataRef data) {
    return data ? data->bytes : NULL;
}
uint8_t *OCDataGetMutableBytes(OCMutableDataRef data) {
    return data ? data->bytes : NULL;
}
bool OCDataGetBytes(OCDataRef data, OCRange range, uint8_t *buffer) {
    if (!data || !buffer || !data->bytes) return false;
    if (range.location + range.length > data->length) return false;
    memcpy(buffer, data->bytes + range.location, range.length);
    return true;
}
bool OCDataSetLength(OCMutableDataRef data, uint64_t newLength) {
    if (!data) {
        fprintf(stderr, "OCDataSetLength: data is NULL\n");
        return false;
    }
    if (newLength == data->length) {
        return true;  // No change needed
    }
    // If expanding, reallocate if necessary
    if (newLength > data->capacity) {
        uint8_t *newBytes = realloc(data->bytes, newLength);
        if (!newBytes) {
            fprintf(stderr, "OCDataSetLength: realloc failed for newLength = %llu\n",
                    (unsigned long long)newLength);
            return false;
        }
        data->bytes = newBytes;
        data->capacity = newLength;
    }
    // Zero new region if expanding
    if (newLength > data->length && data->bytes) {
        memset(data->bytes + data->length, 0, newLength - data->length);
    }
    // Shrink or grow the logical length
    data->length = newLength;
    return true;
}
bool OCDataIncreaseLength(OCMutableDataRef data, uint64_t extraLength) {
    if (!data) return false;
    return OCDataSetLength(data, data->length + extraLength);
}
bool OCDataAppendBytes(OCMutableDataRef data, const uint8_t *bytes, uint64_t length) {
    if (!data || !bytes || length == 0) return false;
    uint64_t oldLength = data->length;
    if (!OCDataSetLength(data, oldLength + length)) return false;
    memcpy((uint8_t *)data->bytes + oldLength, bytes, length);
    return true;
}
typedef enum {
    BASE64_LINE_LENGTH_NONE = 0,
    BASE64_LINE_LENGTH_64 = 64,
    BASE64_LINE_LENGTH_76 = 76
} Base64LineLength;
typedef enum {
    BASE64_LINE_ENDING_NONE,
    BASE64_LINE_ENDING_LF,
    BASE64_LINE_ENDING_CR,
    BASE64_LINE_ENDING_CRLF
} Base64LineEnding;
#define BASE64_INVALID 255
static const char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static uint8_t base64_decoding_table[256];
static bool base64_initialized = false;
static void initialize_base64_decoding_table(void) {
    for (int i = 0; i < 256; ++i)
        base64_decoding_table[i] = BASE64_INVALID;
    for (int i = 0; i < 64; ++i)
        base64_decoding_table[(unsigned char)base64_table[i]] = (uint8_t)i;
    base64_initialized = true;
}
char *base64_encode_with_options(const uint8_t *data, size_t input_length,
                                 Base64LineLength line_length,
                                 Base64LineEnding line_ending,
                                 size_t *output_length) {
    if (!data) return NULL;
    // 1) choose line separator
    const char *line_sep = "";
    size_t line_sep_len = 0;
    switch (line_ending) {
        case BASE64_LINE_ENDING_LF:
            line_sep = "\n";
            line_sep_len = 1;
            break;
        case BASE64_LINE_ENDING_CR:
            line_sep = "\r";
            line_sep_len = 1;
            break;
        case BASE64_LINE_ENDING_CRLF:
            line_sep = "\r\n";
            line_sep_len = 2;
            break;
        default:
            break;
    }
    // 2) raw base64 length
    size_t encoded_raw_len = 4 * ((input_length + 2) / 3);
    // 3) count how many separators we'll need (only if line_length > 0)
    size_t num_line_separators = 0;
    if (line_length > 0) {
        num_line_separators = encoded_raw_len / line_length;
        // if perfectly divides, we don't want a trailing empty line
        if (encoded_raw_len % line_length == 0 && num_line_separators > 0) {
            --num_line_separators;
        }
    }
    // 4) total output length = raw + separators
    size_t total_output_len = encoded_raw_len + num_line_separators * line_sep_len;
    // 5) allocate
    char *output = malloc(total_output_len + 1);
    if (!output) return NULL;
    // 6) encode
    size_t i = 0, j = 0, lines = 0;
    while (i < input_length) {
        size_t bytes_in_group = 0;
        uint32_t octet_a = data[i++];
        bytes_in_group++;
        uint32_t octet_b = 0;
        if (i < input_length) {
            octet_b = data[i++];
            bytes_in_group++;
        }
        uint32_t octet_c = 0;
        if (i < input_length) {
            octet_c = data[i++];
            bytes_in_group++;
        }
        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;
        output[j++] = base64_table[(triple >> 18) & 0x3F];
        output[j++] = base64_table[(triple >> 12) & 0x3F];
        // For third character: pad if we only had 1 input byte in this group
        if (bytes_in_group < 2) {
            output[j++] = '=';
        } else {
            output[j++] = base64_table[(triple >> 6) & 0x3F];
        }
        // For fourth character: pad if we only had 1 or 2 input bytes in this group
        if (bytes_in_group < 3) {
            output[j++] = '=';
        } else {
            output[j++] = base64_table[triple & 0x3F];
        }
        // insert line separator if needed
        if (line_length > 0) {
            // how many base64 chars we’ve written so far (excluding sep)
            size_t chars_written = j;
            if (chars_written > 0 &&
                chars_written % line_length == 0 &&
                lines < num_line_separators) {
                memcpy(output + j + lines * line_sep_len,
                       line_sep, line_sep_len);
                ++lines;
            }
        }
    }
    // 7) adjust final index for inserted separators
    j += lines * line_sep_len;
    output[j] = '\0';
    if (output_length) *output_length = j;
    return output;
}
uint8_t *base64_decode(const char *input, size_t input_length, size_t *output_length) {
    if (!input || input_length == 0) return NULL;
    if (!base64_initialized)
        initialize_base64_decoding_table();
    // First pass: extract only valid base64 characters (including padding)
    char *clean_input = malloc(input_length + 1);
    if (!clean_input) return NULL;
    size_t clean_length = 0;
    for (size_t i = 0; i < input_length; ++i) {
        if (isalnum((unsigned char)input[i]) || input[i] == '+' || input[i] == '/' || input[i] == '=') {
            clean_input[clean_length++] = input[i];
        }
    }
    clean_input[clean_length] = '\0';
    if (clean_length % 4 != 0) {
        free(clean_input);
        return NULL;
    }
    // Calculate output size based on padding in cleaned input
    size_t alloc_size = (clean_length / 4) * 3;
    if (clean_length >= 1 && clean_input[clean_length - 1] == '=') --alloc_size;
    if (clean_length >= 2 && clean_input[clean_length - 2] == '=') --alloc_size;
    uint8_t *decoded = malloc(alloc_size);
    if (!decoded) {
        free(clean_input);
        return NULL;
    }
    size_t i = 0, j = 0;
    uint32_t sextet[4];
    while (i < clean_length) {
        int k = 0;
        for (; k < 4 && i < clean_length; ++i) {
            char c = clean_input[i];
            if (c == '=') {
                sextet[k++] = 0;
            } else {
                uint8_t val = base64_decoding_table[(unsigned char)c];
                if (val == BASE64_INVALID) {
                    free(decoded);
                    free(clean_input);
                    return NULL;
                }
                sextet[k++] = val;
            }
        }
        if (k < 4) break;
        uint32_t triple = (sextet[0] << 18) | (sextet[1] << 12) |
                          (sextet[2] << 6) | sextet[3];
        if (j < alloc_size) decoded[j++] = (triple >> 16) & 0xFF;
        if (j < alloc_size) decoded[j++] = (triple >> 8) & 0xFF;
        if (j < alloc_size) decoded[j++] = triple & 0xFF;
    }
    free(clean_input);
    if (output_length) *output_length = j;
    return decoded;
}
static void _parseOCBase64Options(OCBase64EncodingOptions options,
                                  Base64LineLength *lineLength,
                                  Base64LineEnding *lineEnding) {
    if (options & OCBase64Encoding64CharacterLineLength)
        *lineLength = BASE64_LINE_LENGTH_64;
    else if (options & OCBase64Encoding76CharacterLineLength)
        *lineLength = BASE64_LINE_LENGTH_76;
    else
        *lineLength = BASE64_LINE_LENGTH_NONE;
    if (options & OCBase64EncodingEndLineWithCarriageReturnLineFeed)
        *lineEnding = BASE64_LINE_ENDING_CRLF;
    else if (options & OCBase64EncodingEndLineWithCarriageReturn)
        *lineEnding = BASE64_LINE_ENDING_CR;
    else if (options & OCBase64EncodingEndLineWithLineFeed)
        *lineEnding = BASE64_LINE_ENDING_LF;
    else
        *lineEnding = BASE64_LINE_ENDING_NONE;
}
OCStringRef OCDataCreateBase64EncodedString(OCDataRef data,
                                            OCBase64EncodingOptions options) {
    if (!data || OCDataGetLength(data) == 0) return NULL;
    Base64LineLength lineLen;
    Base64LineEnding lineEnd;
    _parseOCBase64Options(options, &lineLen, &lineEnd);
    const uint8_t *bytes = OCDataGetBytesPtr(data);
    size_t input_length = (size_t)OCDataGetLength(data);
    size_t output_length = 0;
    char *c_encoded = base64_encode_with_options(bytes, input_length,
                                                 lineLen, lineEnd,
                                                 &output_length);
    if (!c_encoded) return NULL;
    OCStringRef result = OCStringCreateWithCString(c_encoded);
    free(c_encoded);
    return result;
}
OCDataRef OCDataCreateFromBase64EncodedString(OCStringRef base64String) {
    if (!base64String) return NULL;
    const char *encoded = OCStringGetCString(base64String);
    if (!encoded) return NULL;
    size_t decoded_len = 0;
    uint8_t *decoded = base64_decode(encoded, strlen(encoded), &decoded_len);
    if (!decoded) return NULL;
    // 'decoded' is copied into OCData; original buffer is freed
    OCDataRef data = OCDataCreate(decoded, decoded_len);
    free(decoded);
    return data;
}
cJSON *OCDataCreateJSON(OCDataRef data) {
    if (!data) return cJSON_CreateNull();
    OCStringRef b64 = OCDataCreateBase64EncodedString(data, OCBase64EncodingOptionsNone);
    if (!b64) {
        fprintf(stderr, "OCDataCreateJSON: Failed to Base64 encode OCData.\n");
        return cJSON_CreateNull();
    }
    const char *s = OCStringGetCString(b64);
    cJSON *node = cJSON_CreateString(s ? s : "");
    OCRelease(b64);
    return node;
}
OCDataRef OCDataCreateFromJSON(cJSON *json) {
    if (!json || !cJSON_IsString(json)) return NULL;
    const char *encoded = json->valuestring;
    if (!encoded) return NULL;
    OCStringRef b64 = OCStringCreateWithCString(encoded);
    if (!b64) return NULL;
    OCDataRef result = OCDataCreateFromBase64EncodedString(b64);
    OCRelease(b64);
    return result;
}
