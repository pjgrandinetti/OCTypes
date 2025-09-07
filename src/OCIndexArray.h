/**
 * @file OCIndexArray.h
 * @brief Declares the OCIndexArray and OCMutableIndexArray interfaces.
 *
 * OCIndexArray provides immutable and mutable arrays of OCIndex values,
 * with support for Base64 serialization, CFNumber-based conversion, and
 * plist-compatible serialization.
 */
#ifndef OCINDEXARRAY_H
#define OCINDEXARRAY_H
#include <stdbool.h>
#include <stdint.h>
#include "OCNumber.h"  // For OCNumberType used in public API
#include "OCType.h"
#include "cJSON.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @defgroup OCIndexArray OCIndexArray
 * @brief Array types and operations for OCIndex values in OCTypes.
 *
 * This group includes APIs to create, access, modify, and serialize
 * arrays of OCIndex elements, including Base64 conversion and CF-compatible
 * array export.
 * @{
 */
/**
 * @brief Returns the unique OCTypeID for OCIndexArray.
 *
 * @return The OCTypeID corresponding to OCIndexArray.
 * @ingroup OCIndexArray
 */
OCTypeID OCIndexArrayGetTypeID(void);
/**
 * @brief Creates an immutable OCIndexArray from a C array of OCIndex values.
 *
 * @param indexes Pointer to a C array of OCIndex.
 * @param numValues Number of elements in the array.
 * @return A new OCIndexArrayRef instance containing a copy of the provided values,
 *         or NULL on failure.
 * @ingroup OCIndexArray
 */
OCIndexArrayRef OCIndexArrayCreate(OCIndex *indexes, OCIndex numValues);
/**
 * @brief Creates a mutable OCIndexArray with the given initial capacity.
 *
 * @param capacity Initial number of OCIndex slots to allocate.
 * @return A new empty OCMutableIndexArrayRef, or NULL on failure.
 * @ingroup OCIndexArray
 */
OCMutableIndexArrayRef OCIndexArrayCreateMutable(OCIndex capacity);
/**
 * @brief Creates a deep immutable copy of the given index array.
 *
 * @param theIndexArray The source OCIndexArrayRef to copy.
 * @return A new OCIndexArrayRef that is a deep copy, or NULL on failure.
 * @ingroup OCIndexArray
 */
OCIndexArrayRef OCIndexArrayCreateCopy(OCIndexArrayRef theIndexArray);
/**
 * @brief Creates a deep mutable copy of the given index array.
 *
 * @param theIndexArray The source OCIndexArrayRef to copy.
 * @return A new OCMutableIndexArrayRef that is a deep copy, or NULL on failure.
 * @ingroup OCIndexArray
 */
OCMutableIndexArrayRef OCIndexArrayCreateMutableCopy(OCIndexArrayRef theIndexArray);
/**
 * @brief Returns the number of OCIndex values in the array.
 *
 * @param theIndexArray The OCIndexArrayRef instance.
 * @return The count of elements (OCIndex) in the array.
 * @ingroup OCIndexArray
 */
OCIndex OCIndexArrayGetCount(OCIndexArrayRef theIndexArray);
/**
 * @brief Returns a pointer to the raw mutable OCIndex buffer.
 *
 * @param theIndexArray The OCIndexArrayRef instance.
 * @return A pointer to the internal OCIndex array. Do not modify unless it is
 *         a mutable array. Returns NULL if the array is NULL.
 * @ingroup OCIndexArray
 */
OCIndex *OCIndexArrayGetMutableBytes(OCIndexArrayRef theIndexArray);
/**
 * @brief Retrieves the value at the specified index.
 *
 * @param theIndexArray The OCIndexArrayRef instance.
 * @param index Zero-based index of the value to retrieve.
 * @return The OCIndex value at the given index, or kOCNotFound if out of bounds.
 * @ingroup OCIndexArray
 */
OCIndex OCIndexArrayGetValueAtIndex(OCIndexArrayRef theIndexArray, OCIndex index);
/**
 * @brief Sets a value at the specified index in a mutable array.
 *
 * @param theIndexArray The OCMutableIndexArrayRef instance.
 * @param index Zero-based index where the value will be set.
 * @param value The OCIndex value to set at the specified index.
 * @return true if successful; false if index out of bounds or on error.
 * @ingroup OCIndexArray
 */
bool OCIndexArraySetValueAtIndex(OCMutableIndexArrayRef theIndexArray, OCIndex index, OCIndex value);
/**
 * @brief Removes the value at the specified index.
 *
 * @param theIndexArray The OCMutableIndexArrayRef instance.
 * @param index Zero-based index of the element to remove.
 * @return true if the element was removed; false if index is out of bounds.
 * @ingroup OCIndexArray
 */
bool OCIndexArrayRemoveValueAtIndex(OCMutableIndexArrayRef theIndexArray, OCIndex index);
/**
 * @brief Removes multiple values at indices specified in an OCIndexSet.
 *
 * @param theIndexArray The OCMutableIndexArrayRef instance.
 * @param theIndexSet   An OCIndexSetRef containing indices to remove.
 *                      Indices outside the valid range are ignored.
 * @ingroup OCIndexArray
 */
void OCIndexArrayRemoveValuesAtIndexes(OCMutableIndexArrayRef theIndexArray, OCIndexSetRef theIndexSet);
/**
 * @brief Checks whether the array contains the specified value.
 *
 * @param theIndexArray The OCIndexArrayRef instance.
 * @param index The OCIndex value to search for.
 * @return true if the value exists in the array; false otherwise.
 * @ingroup OCIndexArray
 */
bool OCIndexArrayContainsIndex(OCIndexArrayRef theIndexArray, OCIndex index);
/**
 * @brief Appends a single value to the mutable array.
 *
 * @param theIndexArray The OCMutableIndexArrayRef instance.
 * @param index The OCIndex value to append.
 * @return true if the value was appended; false on allocation failure.
 * @ingroup OCIndexArray
 */
bool OCIndexArrayAppendValue(OCMutableIndexArrayRef theIndexArray, OCIndex index);
/**
 * @brief Appends the contents of another array to the mutable array.
 *
 * @param theIndexArray   The OCMutableIndexArrayRef instance.
 * @param arrayToAppend   An OCIndexArrayRef whose elements will be appended.
 * @return true if successful; false on allocation failure.
 * @ingroup OCIndexArray
 */
bool OCIndexArrayAppendValues(OCMutableIndexArrayRef theIndexArray, OCIndexArrayRef arrayToAppend);
/**
 * @brief Converts the array to a Base64-encoded OCString using the given numeric type.
 *
 * @param theIndexArray The OCIndexArrayRef instance.
 * @param integerType   A OCNumberType indicating how to encode OCIndex values.
 * @return A new OCStringRef containing the Base64 representation, or NULL on error.
 * @ingroup OCIndexArray
 */
OCStringRef OCIndexArrayCreateBase64String(OCIndexArrayRef theIndexArray, OCNumberType integerType);
/**
 * @brief Converts the array to a CFNumber-based OCArray.
 *
 * @param theIndexArray The OCIndexArrayRef instance.
 * @return A new OCArrayRef containing CFNumber objects, or NULL on error.
 * @ingroup OCIndexArray
 */
OCArrayRef OCIndexArrayCreateCFNumberArray(OCIndexArrayRef theIndexArray);
/**
 * @brief Serializes the array to a plist-compatible OCDictionary.
 *
 * @param theIndexArray The OCIndexArrayRef instance.
 * @return A new OCDictionaryRef representing the array, or NULL on error.
 * @ingroup OCIndexArray
 */
OCDictionaryRef OCIndexArrayCreateDictionary(OCIndexArrayRef theIndexArray);
/**
 * @brief Reconstructs an OCIndexArray from a plist-compatible OCDictionary.
 *
 * @param dictionary An OCDictionaryRef previously returned by OCIndexArrayCreateDictionary().
 * @return A new OCIndexArrayRef populated from the dictionary, or NULL on error.
 * @ingroup OCIndexArray
 */
OCIndexArrayRef OCIndexArrayCreateFromDictionary(OCDictionaryRef dictionary);
/**
 * @brief Creates an OCIndexArray from a cJSON array.
 *
 * @param json A cJSON array containing numeric values.
 * @param outError Optional pointer to receive error information if deserialization fails.
 * @return A new OCIndexArrayRef, or NULL on failure.
 *         The caller is responsible for releasing the returned array.
 */
OCIndexArrayRef OCIndexArrayCreateFromJSON(cJSON *json, OCStringRef *outError);
/**
 * @brief Creates a JSON representation of an OCIndexArray.
 *
 * For untyped serialization (typed=false), creates a JSON array of numeric values.
 * Note that this loses type information and the index array will be indistinguishable
 * from a regular array upon deserialization.
 *
 * For typed serialization (typed=true), creates a JSON object with "type": "OCIndexArray"
 * and "value" array containing the numeric indices.
 *
 * @param array A valid OCIndexArrayRef.
 * @param typed Whether to include type information in the serialization.
 * @param outError Optional pointer to receive an error string on failure.
 * @return A new cJSON object/array on success, or cJSON null on failure.
 *         The caller is responsible for managing the returned cJSON object.
 */
cJSON *OCIndexArrayCopyAsJSON(OCIndexArrayRef array, bool typed, OCStringRef *outError);
/**
 * @brief Logs the contents of the array to stderr, for debugging.
 *
 * @param theIndexArray The OCIndexArrayRef instance.
 * @ingroup OCIndexArray
 */
void OCIndexArrayShow(OCIndexArrayRef theIndexArray);
/**
 * @brief Returns a copy of the JSON encoding preference for an OCIndexArray.
 *
 * The encoding determines how the array is serialized to JSON:
 * - OCJSONEncodingBase64: Binary encoding for compact representation
 * - OCJSONEncodingNone: Standard JSON array format
 * - NULL: Use default encoding behavior
 *
 * @param array The OCIndexArrayRef instance.
 * @return The encoding value.
 * @ingroup OCIndexArray
 */
OCJSONEncoding OCIndexArrayCopyEncoding(OCIndexArrayRef array);
/**
 * @brief Sets the JSON encoding preference for a mutable OCIndexArray.
 *
 * The encoding controls JSON serialization behavior:
 * - OCJSONEncodingBase64: Serialize as base64-encoded binary data
 * - OCJSONEncodingNone: Serialize as standard JSON array
 *
 * @param array The OCMutableIndexArrayRef instance.
 * @param encoding The encoding value to set.
 * @ingroup OCIndexArray
 */
void OCIndexArraySetEncoding(OCMutableIndexArrayRef array, OCJSONEncoding encoding);
/** @} */  // end of OCIndexArray group
#ifdef __cplusplus
}
#endif
#endif /* OCINDEXARRAY_H */
