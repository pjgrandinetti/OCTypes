/**
 * @file OCArray.h
 * @brief Declares the OCArray and OCMutableArray interfaces.
 *
 * OCArray provides immutable and mutable array data structures
 * with customizable memory management and equality semantics.
 */

#ifndef OCArray_h
#define OCArray_h

#include "OCLibrary.h"

/**
 * @defgroup OCArray OCArray
 * @brief Array types and operations in OCTypes.
 * @{
 */

/**
 * @brief Callback to retain a value in the array.
 *
 * @param value The value to be retained.
 * @return The retained value.
 *
 * @ingroup OCArray
 */
typedef const void *(*OCArrayRetainCallBack)(const void *value);

/**
 * @brief Callback to release a value from the array.
 *
 * @param value The value to be released.
 *
 * @ingroup OCArray
 */
typedef void (*OCArrayReleaseCallBack)(const void *value);

/**
 * @brief Callback to copy a description string of a value.
 *
 * @param value The value to describe.
 * @return An OCStringRef describing the value.
 *
 * @ingroup OCArray
 */
typedef OCStringRef (*OCArrayCopyDescriptionCallBack)(const void *value);

/**
 * @brief Callback to compare two values for equality.
 *
 * @param value1 First value.
 * @param value2 Second value.
 * @return true if equal, false otherwise.
 *
 * @ingroup OCArray
 */
typedef bool (*OCArrayEqualCallBack)(const void *value1, const void *value2);

/**
 * @brief Struct defining callbacks for array value management.
 *
 * @ingroup OCArray
 */
typedef struct {
    int64_t version; /**< Structure version (should be 0). */
    OCArrayRetainCallBack retain; /**< Retain callback. */
    OCArrayReleaseCallBack release; /**< Release callback. */
    OCArrayCopyDescriptionCallBack copyDescription; /**< Description callback. */
    OCArrayEqualCallBack equal; /**< Equality comparison callback. */
} OCArrayCallBacks;

/**
 * @brief Predefined callbacks for arrays of OCTypeRef objects.
 * @ingroup OCArray
 */
extern const OCArrayCallBacks kOCTypeArrayCallBacks;

/**
 * @brief Gets the OCTypeID for OCArray.
 * @return OCTypeID value.
 */
OCTypeID OCArrayGetTypeID(void);

/**
 * @brief Returns the number of elements in an array.
 * @param theArray The array to query.
 * @return Element count, or 0 if NULL.
 * @ingroup OCArray
 */
uint64_t OCArrayGetCount(OCArrayRef theArray);

/**
 * @brief Creates a new immutable array.
 *
 * @param values Pointer to value array.
 * @param numValues Number of values.
 * @param callBacks Pointer to value management callbacks.
 * @return New OCArrayRef or NULL.
 *
 * @ingroup OCArray
 */
OCArrayRef OCArrayCreate(const void **values, uint64_t numValues, const OCArrayCallBacks *callBacks);

/**
 * @brief Creates an immutable array copy.
 *
 * @param theArray Array to copy.
 * @return New OCArrayRef or NULL.
 * @ingroup OCArray
 */
OCArrayRef OCArrayCreateCopy(OCArrayRef theArray);

/**
 * @brief Creates a mutable array.
 *
 * @param capacity Initial capacity.
 * @param callBacks Pointer to callbacks.
 * @return New OCMutableArrayRef or NULL.
 * @ingroup OCArray
 */
OCMutableArrayRef OCArrayCreateMutable(uint64_t capacity, const OCArrayCallBacks *callBacks);

/**
 * @brief Creates a mutable copy of an array.
 *
 * @param theArray The array to copy.
 * @return New OCMutableArrayRef or NULL.
 * @ingroup OCArray
 */
OCMutableArrayRef OCArrayCreateMutableCopy(OCArrayRef theArray);

/**
 * @brief Gets a value at a given index.
 *
 * @param theArray The array.
 * @param index Zero-based index.
 * @return Pointer to value or NULL.
 * @ingroup OCArray
 */
const void *OCArrayGetValueAtIndex(OCArrayRef theArray, uint64_t index);

/**
 * @brief Replaces the value at the specified index in a mutable array.
 *
 * Releases the existing value (if applicable) and retains the new value.
 *
 * @param theArray The mutable array.
 * @param index The index at which to set the value.
 * @param value The new value.
 * @ingroup OCArray
 */
bool OCArraySetValueAtIndex(OCMutableArrayRef theArray, OCIndex index, const void *value);

/**
 * @brief Appends a value to a mutable array.
 *
 * @param theArray Mutable array.
 * @param value Value to append.
 * @ingroup OCArray
 */
bool OCArrayAppendValue(OCMutableArrayRef theArray, const void *value);

/**
 * @brief Appends a range of values from another array.
 *
 * @param theArray Destination array.
 * @param otherArray Source array.
 * @param range Range of values to append.
 * @ingroup OCArray
 */
bool OCArrayAppendArray(OCMutableArrayRef theArray, OCArrayRef otherArray, OCRange range);

/**
 * @brief Finds the first index of a value.
 *
 * @param theArray The array.
 * @param value The value to search for.
 * @return Index or kOCNotFound.
 * @ingroup OCArray
 */
long OCArrayGetFirstIndexOfValue(OCArrayRef theArray, const void *value);

/**
 * @brief Checks if the array contains a value.
 *
 * @param theArray The array.
 * @param value The value to check.
 * @return true if found, false otherwise.
 * @ingroup OCArray
 */
bool OCArrayContainsValue(OCArrayRef theArray, const void *value);

/**
 * @brief Removes a value at a given index.
 *
 * @param theArray The array.
 * @param index Index to remove.
 * @ingroup OCArray
 */
bool OCArrayRemoveValueAtIndex(OCMutableArrayRef theArray, uint64_t index);

/**
 * @brief Inserts a value at a given index.
 *
 * @param theArray The array.
 * @param index Index for insertion.
 * @param value The value to insert.
 * @ingroup OCArray
 */
bool OCArrayInsertValueAtIndex(OCMutableArrayRef theArray, uint64_t index, const void *value);

/**
 * @brief Sorts values in an array.
 *
 * @param theArray The array to sort.
 * @param range Range of values to sort.
 * @param comparator Comparison function.
 * @param context User-defined data.
 * @ingroup OCArray
 */
void OCArraySortValues(OCMutableArrayRef theArray, OCRange range, OCComparatorFunction comparator, void *context);

/**
 * @brief Performs a binary search on a sorted array.
 *
 * @param array The array to search.
 * @param range Range to search.
 * @param value The value to find.
 * @param comparator Comparison function.
 * @param context User-defined context.
 * @return Index or kOCNotFound.
 * @ingroup OCArray
 */
int64_t OCArrayBSearchValues(OCArrayRef array, OCRange range, const void *value, OCComparatorFunction comparator, void *context);

/**
 * @brief Creates a new immutable array from an existing one.
 *
 * @param array The source array.
 * @return New OCArrayRef or NULL.
 * @ingroup OCArray
 */
OCArrayRef OCArrayCreateWithArray(OCArrayRef array);

const OCArrayCallBacks *OCArrayGetCallBacks(OCArrayRef array);

/** @} */ // end of OCArray group

#endif /* OCArray_h */
