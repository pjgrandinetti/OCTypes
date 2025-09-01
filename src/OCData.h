/**
 * @file OCData.h
 * @brief Declares the OCData and OCMutableData interfaces for handling binary data.
 *
 * OCData provides immutable and mutable representations of raw byte sequences,
 * with memory safety and efficient operations.
 *
 * @note Ownership follows CoreFoundation conventions:
 *       The caller owns returned OCDataRef or OCMutableDataRef from functions
 *       with "Create" or "Copy" in the name and must call OCRelease().
 */
#ifndef OCData_h
#define OCData_h
#include "OCType.h"  // Foundation types and forward declarations
#include "cJSON.h"   // For JSON functionality
/**
 * @defgroup OCData OCData
 * @brief Binary data buffer types and utilities.
 * @{
 */
/**
 * @brief Returns the unique type identifier for OCData objects.
 * @return OCTypeID for OCData.
 * @ingroup OCData
 */
OCTypeID OCDataGetTypeID(void);
/**
 * @brief Creates a new immutable data object by copying bytes.
 *
 * @param bytes Pointer to the source byte buffer.
 * @param length Number of bytes to copy.
 * @return New OCDataRef, or NULL on failure.
 * @ingroup OCData
 */
OCDataRef OCDataCreate(const uint8_t *bytes, uint64_t length);
/**
 * @brief Creates an immutable data object referencing existing memory (no copy).
 *
 * @param bytes Pointer to the byte buffer.
 * @param length Length of the byte buffer.
 * @return New OCDataRef, or NULL on failure.
 * @warning Caller must ensure that `bytes` remains valid and unmodified.
 * @ingroup OCData
 */
OCDataRef OCDataCreateWithBytesNoCopy(const uint8_t *bytes, uint64_t length);
/**
 * @brief Creates a new OCDataRef by copying an existing one.
 *
 * @param theData Source OCDataRef to copy.
 * @return Copy of the data, or NULL on failure.
 * @ingroup OCData
 */
OCDataRef OCDataCreateCopy(OCDataRef theData);
/**
 * @brief Creates a new mutable data object with a specified capacity.
 *
 * @param capacity Initial capacity in bytes.
 * @return New OCMutableDataRef, or NULL on failure.
 * @ingroup OCData
 */
OCMutableDataRef OCDataCreateMutable(uint64_t capacity);
/**
 * @brief Creates a mutable copy of existing data with a specified capacity.
 *
 * @param capacity Minimum capacity of the new object.
 * @param theData Source data to copy (can be NULL).
 * @return New OCMutableDataRef.
 * @ingroup OCData
 */
OCMutableDataRef OCDataCreateMutableCopy(uint64_t capacity, OCDataRef theData);
/**
 * @brief Reads an entire file into a new OCDataRef.
 *
 * Attempts to open and read the file at the given path, returning its
 * raw bytes. If the operation fails (e.g. file not found or read error),
 * returns NULL and, if `errorString` is non-NULL, sets *errorString to
 * a newly created OCStringRef describing the failure (ownership transferred
 * to caller). Pass NULL to ignore error details.
 *
 * @param path         Filesystem path to read.
 * @param errorString  Optional pointer to an OCStringRef; on failure, *errorString
 *                     will be set to an explanatory message (ownership transferred
 *                     to caller). Pass NULL to skip error reporting.
 * @return             An OCDataRef containing the file’s contents on success
 *                     (ownership transferred to caller), or NULL on failure.
 * @ingroup          OCData
 *
 * @code
 * OCStringRef errMsg = NULL;
 * OCDataRef blob = OCDataCreateWithContentsOfFile("data/sample.jdx", &errMsg);
 * if (!blob) {
 *     // handle errMsg...
 *     OCRelease(errMsg);
 * } else {
 *     // use blob...
 *     OCRelease(blob);
 * }
 * @endcode
 */
OCDataRef
OCDataCreateWithContentsOfFile(const char *path, OCStringRef *errorString);
/**
 * @brief Gets the length of a data object.
 *
 * @param data OCDataRef or OCMutableDataRef.
 * @return Length in bytes.
 * @ingroup OCData
 */
uint64_t OCDataGetLength(OCDataRef data);
/**
 * @brief Returns a read-only pointer to internal bytes.
 *
 * @param data OCDataRef or OCMutableDataRef.
 * @return Pointer to bytes, or NULL.
 * @ingroup OCData
 */
const uint8_t *OCDataGetBytesPtr(OCDataRef data);
/**
 * @brief Returns a mutable pointer to internal bytes.
 *
 * @param data Mutable data object.
 * @return Pointer to bytes, or NULL.
 * @warning Use with care. Modifications may affect internal state.
 * @ingroup OCData
 */
uint8_t *OCDataGetMutableBytes(OCMutableDataRef data);
/**
 * @brief Copies a range of bytes from an OCData object into a buffer.
 *
 * @param data The source OCData object.
 * @param range The byte range to copy.
 * @param buffer Destination buffer (must be large enough).
 * @return true if the copy succeeded; false if inputs are invalid or out of bounds.
 *
 * @ingroup OCData
 */
bool OCDataGetBytes(OCDataRef data, OCRange range, uint8_t *buffer);
/**
 * @brief Sets the length of a mutable data object.
 *
 * If the new length is greater than the current length, the extra memory is zero-initialized.
 * If the new length exceeds the current capacity, the buffer is reallocated.
 *
 * @param data Mutable data object to resize.
 * @param newLength The desired new length in bytes.
 * @return true if the operation succeeded; false if allocation failed or input was NULL.
 *
 * @ingroup OCData
 */
bool OCDataSetLength(OCMutableDataRef data, uint64_t newLength);
/**
 * @brief Increases the length of a mutable data object by a given amount.
 *
 * @param data The mutable data object.
 * @param extraLength The number of bytes to add.
 * @return true if the resize was successful; false on failure or invalid input.
 *
 * @ingroup OCData
 */
bool OCDataIncreaseLength(OCMutableDataRef data, uint64_t extraLength);
/**
 * @brief Appends bytes to the end of a mutable data object.
 *
 * Grows the data buffer as needed. If allocation fails, no changes are made.
 *
 * @param data The mutable data object.
 * @param bytes The bytes to append.
 * @param length The number of bytes to append.
 * @return true if the bytes were appended successfully; false on failure.
 *
 * @ingroup OCData
 */
bool OCDataAppendBytes(OCMutableDataRef data, const uint8_t *bytes, uint64_t length);
/**
 * @brief Returns a human-readable string description of a data object.
 *
 * @param cf the OCTypeRef instance.
 * @return A formatted OCStringRef (caller must release).
 * @ingroup OCData
 */
OCStringRef OCDataCopyFormattingDesc(OCTypeRef cf);
/**
 * @typedef OCBase64EncodingOptions
 * @brief Bitmask options to control the format of Base64-encoded output.
 *
 * These options allow callers to control line breaking and end-of-line characters
 * in Base64-encoded strings. Options may be combined using the bitwise OR operator.
 * Only one line length and one line ending option should be specified.
 */
typedef enum {
    /** No line breaks; a single continuous Base64 string. */
    OCBase64EncodingOptionsNone = 0,
    /** Insert line breaks every 64 characters. */
    OCBase64Encoding64CharacterLineLength = 1 << 0,
    /** Insert line breaks every 76 characters. */
    OCBase64Encoding76CharacterLineLength = 1 << 1,
    /** Use a carriage return (`\r`) for line endings. */
    OCBase64EncodingEndLineWithCarriageReturn = 1 << 4,
    /** Use a line feed (`\n`) for line endings. */
    OCBase64EncodingEndLineWithLineFeed = 1 << 5,
    /** Use CRLF (`\r\n`) line endings. */
    OCBase64EncodingEndLineWithCarriageReturnLineFeed = 1 << 6
} OCBase64EncodingOptions;
/**
 * @brief Encodes an OCDataRef into a Base64-encoded OCStringRef.
 *
 * @param data
 *     The input OCDataRef containing binary data to encode. Must not be NULL.
 *
 * @param options
 *     Bitmask of OCBase64EncodingOptions to control line length and line endings.
 *
 * @return
 *     A new OCStringRef containing the Base64-encoded output. Returns NULL on failure.
 *     The caller owns the returned string and is responsible for releasing it.
 *
 * @ingroup OCData
 */
OCStringRef OCDataCreateBase64EncodedString(OCDataRef data, OCBase64EncodingOptions options);
/**
 * @brief Decodes a Base64-encoded OCStringRef into an OCDataRef.
 *
 * @param base64String
 *     An OCStringRef containing Base64-encoded content. Must not be NULL.
 *     Whitespace and line breaks are ignored during decoding.
 *
 * @return
 *     A new OCDataRef containing the decoded binary data, or NULL on failure.
 *     The caller owns the returned data object and must release it.
 *
 * @ingroup OCData
 */
OCDataRef OCDataCreateFromBase64EncodedString(OCStringRef base64String);
/**
 * @brief Creates a JSON representation from OCData by Base64-encoding its contents.
 *
 * This function serializes raw binary data into JSON format. When typed=false,
 * it returns a simple Base64-encoded string. When typed=true, it returns a
 * structured JSON object with "type": "OCData", "encoding": "base64", and
 * "value": "<base64-encoded-data>" fields.
 *
 * @param data An OCDataRef to serialize. Must not be NULL.
 * @param typed If false, returns Base64 string; if true, returns typed object.
 * @return A new cJSON string or object containing Base64-encoded data,
 *         or cJSON null if serialization fails.
 *         Caller is responsible for managing the returned cJSON object.
 * @ingroup OCData
 */
cJSON *OCDataCopyAsJSON(OCDataRef data, bool typed);
/**
 * @brief Creates an OCDataRef from a Base64-encoded JSON string.
 *
 * This function expects a cJSON string node containing a Base64-encoded binary payload.
 *
 * @param json A cJSON string node.
 * @return A new OCDataRef on success, or NULL on failure.
 *         Caller is responsible for releasing the returned OCDataRef.
 * @ingroup OCData
 */
OCDataRef OCDataCreateFromJSON(cJSON *json);
/**
 * @brief Creates an OCDataRef from a typed JSON object with Base64-encoded data.
 *
 * This function expects a cJSON object with "type": "OCData", "encoding": "base64",
 * and "value": "<base64-data>" fields. The encoding field is optional and defaults
 * to base64 for backward compatibility.
 *
 * @param json A cJSON object with type, encoding, and value fields.
 * @return A new OCDataRef on success, or NULL on failure.
 *         Caller is responsible for releasing the returned OCDataRef.
 * @ingroup OCData
 */
OCDataRef OCDataCreateFromJSONTyped(cJSON *json);
/** @} */  // end of OCData group
#endif     /* OCData_h */
