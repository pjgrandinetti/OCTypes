/**
 * @file OCIndexArray.h
 * @brief Interface for immutable and mutable arrays of OCIndex values.
 *
 * Provides APIs to create, access, modify, and serialize arrays of OCIndex elements,
 * including Base64 conversion and CF-compatible array export.
 */

#ifndef OCINDEXARRAY_H
#define OCINDEXARRAY_H

#include "OCLibrary.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates an immutable OCIndexArray from a C array of OCIndex values.
 *
 * @param indexes Pointer to a C array of OCIndex.
 * @param numValues Number of elements in the array.
 * @return A new OCIndexArrayRef instance containing a copy of the provided values.
 */
OCIndexArrayRef OCIndexArrayCreate(OCIndex *indexes, OCIndex numValues);

/**
 * @brief Creates a mutable OCIndexArray with the given capacity.
 *
 * @param capacity Initial number of OCIndex slots to allocate.
 * @return A new empty OCMutableIndexArrayRef.
 */
OCMutableIndexArrayRef OCIndexArrayCreateMutable(OCIndex capacity);

/**
 * @brief Creates a deep immutable copy of the given index array.
 */
OCIndexArrayRef OCIndexArrayCreateCopy(OCIndexArrayRef theIndexArray);

/**
 * @brief Creates a deep mutable copy of the given index array.
 */
OCMutableIndexArrayRef OCIndexArrayCreateMutableCopy(OCIndexArrayRef theIndexArray);

/**
 * @brief Returns the number of OCIndex values in the array.
 */
OCIndex OCIndexArrayGetCount(OCIndexArrayRef theIndexArray);

/**
 * @brief Returns a pointer to the raw mutable OCIndex array.
 */
OCIndex *OCIndexArrayGetMutableBytePtr(OCIndexArrayRef theIndexArray);

/**
 * @brief Gets the value at the specified index.
 *
 * @param index Index of the value to retrieve.
 * @return The OCIndex value or kOCNotFound.
 */
OCIndex OCIndexArrayGetValueAtIndex(OCIndexArrayRef theIndexArray, OCIndex index);

/**
 * @brief Sets a value at the specified index in a mutable array.
 */
bool OCIndexArraySetValueAtIndex(OCMutableIndexArrayRef theIndexArray, OCIndex index, OCIndex value);

/**
 * @brief Removes the value at the specified index.
 */
bool OCIndexArrayRemoveValueAtIndex(OCMutableIndexArrayRef theIndexArray, OCIndex index);

/**
 * @brief Removes multiple values at indices specified in a set.
 */
void OCIndexArrayRemoveValuesAtIndexes(OCMutableIndexArrayRef theIndexArray, OCIndexSetRef theIndexSet);

/**
 * @brief Checks whether the array contains the specified value.
 */
bool OCIndexArrayContainsIndex(OCIndexArrayRef theIndexArray, OCIndex index);

/**
 * @brief Appends a single value to the mutable array.
 */
bool OCIndexArrayAppendValue(OCMutableIndexArrayRef theIndexArray, OCIndex index);

/**
 * @brief Appends the contents of another array to the mutable array.
 */
bool OCIndexArrayAppendValues(OCMutableIndexArrayRef theIndexArray, OCIndexArrayRef arrayToAppend);

/**
 * @brief Converts the array to a Base64 string using the given numeric type.
 */
OCStringRef OCIndexArrayCreateBase64String(OCIndexArrayRef theIndexArray, csdmNumericType integerType);

/**
 * @brief Converts the array to a CFNumber-based OCArray.
 */
OCArrayRef OCIndexArrayCreateCFNumberArray(OCIndexArrayRef theIndexArray);

/**
 * @brief Serializes the array to a dictionary (plist-compatible).
 */
OCDictionaryRef OCIndexArrayCreatePList(OCIndexArrayRef theIndexArray);

/**
 * @brief Reconstructs an OCIndexArray from a serialized dictionary.
 */
OCIndexArrayRef OCIndexArrayCreateWithPList(OCDictionaryRef dictionary);

/**
 * @brief Returns a retained copy of the internal OCData buffer.
 */
OCDataRef OCIndexArrayCreateData(OCIndexArrayRef theIndexArray);

/**
 * @brief Constructs an array using an existing OCData buffer.
 */
OCIndexArrayRef OCIndexArrayCreateWithData(OCDataRef data);

/**
 * @brief Logs the contents of the array to stderr.
 */
void OCIndexArrayShow(OCIndexArrayRef theIndexArray);

#ifdef __cplusplus
}
#endif

#endif /* OCINDEXARRAY_H */
