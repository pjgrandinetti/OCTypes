//
//  OCArray.h
//  OCTypes
//
//  Created by philip on 4/2/17.
//

#ifndef OCArray_h
#define OCArray_h

#include "OCLibrary.h"

/** @defgroup OCArray OCArray */
/** @addtogroup OCArray
 *  @{
 */

/**
 * A reference to an immutable array object.
 */
typedef const struct __OCArray * OCArrayRef;

/**
 * A reference to a mutable array object.
 */
typedef struct __OCArray * OCMutableArrayRef;

/**
 * Callback function to retain a value in the array.
 *
 * :param value: The value to be retained.
 * :return: The retained value.
 */
typedef const void * (*OCArrayRetainCallBack)(const void *value);

/**
 * Callback function to release a value from the array.
 *
 * :param value: The value to be released.
 */
typedef void		(*OCArrayReleaseCallBack)(const void *value);

/**
 * Callback function to create a string description of a value in the array.
 *
 * :param value: The value for which to create a description.
 * :return: An OCStringRef containing the description of the value.
 */
typedef OCStringRef	(*OCArrayCopyDescriptionCallBack)(const void *value);

/**
 * Callback function to compare two values in the array for equality.
 *
 * :param value1: The first value to compare.
 * :param value2: The second value to compare.
 * :return: True if the values are equal, false otherwise.
 */
typedef bool		(*OCArrayEqualCallBack)(const void *value1, const void *value2);

/**
 * A structure defining callback functions for managing values within an OCArray.
 *
 * These callbacks allow OCArray to manage the lifecycle and comparison of custom data types.
 *
 * :ivar version: The version of the callbacks structure. Should be 0.
 * :ivar retain: A callback function to retain a value. Can be NULL if values do not need explicit retaining.
 * :ivar release: A callback function to release a value. Can be NULL if values do not need explicit releasing.
 * :ivar copyDescription: A callback function to provide a string description of a value. Can be NULL.
 * :ivar equal: A callback function to compare two values for equality. Must not be NULL if OCEqual is to be used on arrays with these callbacks.
 */
typedef struct {
    int64_t     version;
    OCArrayRetainCallBack   retain;
    OCArrayReleaseCallBack  release;
    OCArrayCopyDescriptionCallBack  copyDescription;
    OCArrayEqualCallBack    equal;
} OCArrayCallBacks;

/**
 * A predefined set of callbacks for arrays containing OCType objects.
 * These callbacks use OCRetain, OCRelease, OCCopyDescription, and OCEqual.
 */
extern const OCArrayCallBacks kOCTypeArrayCallBacks;

/**
 * Returns the unique type identifier for OCArray objects.
 *
 * :return: The OCTypeID associated with OCArray.
 */
OCTypeID OCArrayGetTypeID(void);

/**
 * Returns the number of values currently stored in the array.
 *
 * :param theArray: The array to query. Must not be NULL.
 * :return: The count of elements in the array. Returns 0 if theArray is NULL.
 */
uint64_t OCArrayGetCount(OCArrayRef theArray);

/**
 * Creates a new, immutable array containing the provided values.
 *
 * The values are retained according to the provided callbacks.
 *
 * :param values: A C-style array of pointers to the values to add to the array. Must not be NULL if numValues > 0.
 * :param numValues: The number of values in the `values` C-array.
 * :param callBacks: A pointer to an OCArrayCallBacks structure that defines how to handle the values
 *                  (e.g., retain, release, compare). If NULL, default behavior (simple pointer copy and comparison) is used.
 *                  For arrays of OCType objects, kOCTypeArrayCallBacks should be used.
 * :return: A new OCArrayRef instance, or NULL if creation fails (e.g., due to memory allocation issues).
 */
OCArrayRef OCArrayCreate(const void **values, uint64_t numValues, const OCArrayCallBacks *callBacks);

/**
 * Creates a new, immutable array that is a copy of an existing array.
 *
 * The values in the new array are retained as if they were added to the original.
 *
 * :param theArray: The immutable array to copy. Must not be NULL.
 * :return: A new OCArrayRef instance that is a copy of `theArray`, or NULL if `theArray` is NULL or copying fails.
 */
OCArrayRef OCArrayCreateCopy(OCArrayRef theArray);

/**
 * Creates a new, mutable array with a specified initial capacity.
 *
 * :param capacity: The initial number of values the mutable array can store without reallocating its internal storage.
 *                 Pass 0 to use a default initial capacity.
 * :param callBacks: A pointer to an OCArrayCallBacks structure that defines how to handle the values.
 *                  If NULL, default behavior is used. For arrays of OCType objects, kOCTypeArrayCallBacks should be used.
 * :return: A new OCMutableArrayRef instance, or NULL if creation fails.
 */
OCMutableArrayRef OCArrayCreateMutable(uint64_t capacity, const OCArrayCallBacks *callBacks);

/**
 * Creates a new, mutable array that is a copy of an existing array (either mutable or immutable).
 *
 * :param theArray: The array to copy. Must not be NULL.
 * :return: A new OCMutableArrayRef instance containing the same values as `theArray`, or NULL if `theArray` is NULL or copying fails.
 */
OCMutableArrayRef OCArrayCreateMutableCopy(OCArrayRef theArray);

/**
 * Retrieves the value at a specific index in the array.
 *
 * :param theArray: The array to query. Must not be NULL.
 * :param index: The zero-based index of the value to retrieve. Must be less than the count of the array.
 * :return: The value at the specified `index`. Returns NULL if `theArray` is NULL or `index` is out of bounds.
 *         The-returned value is not additionally retained by this call.
 */
const void * OCArrayGetValueAtIndex(OCArrayRef theArray, uint64_t index);

/**
 * Appends a value to the end of a mutable array.
 *
 * The value is retained by the array using the retain callback, if one was provided during creation.
 *
 * :param theArray: The mutable array to which the value will be appended. Must not be NULL.
 * :param value: The value to append.
 */
void OCArrayAppendValue(OCMutableArrayRef theArray, const void * value);

/**
 * Appends a range of values from one array to the end of a mutable array.
 *
 * :param theArray: The destination mutable array. Must not be NULL.
 * :param otherArray: The source array from which values are copied. Must not be NULL.
 * :param range: The range of values to append from `otherArray`.
 *              The sum of `range.location` and `range.length` must not exceed the count of `otherArray`.
 */
void OCArrayAppendArray(OCMutableArrayRef theArray, OCArrayRef otherArray, OCRange range);

/**
 * Finds the first index at which a given value appears in the array.
 *
 * Equality is determined using the `equal` callback provided during array creation.
 *
 * :param theArray: The array to search. Must not be NULL.
 * :param value: The value to search for.
 * :return: The zero-based index of the first occurrence of `value` in `theArray`.
 *         Returns -1 (or more precisely, `kOCNotFound`) if `value` is not found or `theArray` is NULL.
 */
long OCArrayGetFirstIndexOfValue(OCArrayRef theArray, const void * value);

/**
 * Checks whether the array contains a given value.
 *
 * Equality is determined using the `equal` callback provided during array creation.
 *
 * :param theArray: The array to search. Must not be NULL.
 * :param value: The value to check for.
 * :return: `true` if `value` is found in `theArray`, `false` otherwise (including if `theArray` is NULL).
 */
bool OCArrayContainsValue(OCArrayRef theArray, const void * value);

/**
 * Removes the value at a specific index from a mutable array.
 *
 * The value being removed is released by the array using the release callback, if one was provided.
 *
 * :param theArray: The mutable array from which to remove the value. Must not be NULL.
 * :param index: The zero-based index of the value to remove. Must be less than the count of the array.
 */
void OCArrayRemoveValueAtIndex(OCMutableArrayRef theArray,uint64_t index);

/**
 * Inserts a value at a specific index in a mutable array.
 *
 * The value is retained by the array. Existing values at or after the index are shifted.
 *
 * :param theArray: The mutable array into which the value will be inserted. Must not be NULL.
 * :param index: The zero-based index at which to insert `value`. Must be less than or equal to the count of the array.
 * :param value: The value to insert.
 */
void OCArrayInsertValueAtIndex(OCMutableArrayRef theArray, uint64_t index, const void * value);

/**
 * Sorts the values in a mutable array over a given range using a provided comparison function.
 *
 * :param theArray: The mutable array to sort. Must not be NULL.
 * :param range: The range of elements within the array to sort.
 * :param comparator: The comparison function used to determine the order of elements. Must not be NULL.
 * :param context: A pointer to user-defined data to be passed to the `comparator` function. Can be NULL.
 */
void OCArraySortValues(OCMutableArrayRef theArray, OCRange range, OCComparatorFunction comparator, void *context);

/**
 * Performs a binary search for a value within a sorted range of an array.
 *
 * The array or the specified range within it must already be sorted according to the `comparator`.
 *
 * :param array: The array to search. Must not be NULL.
 * :param range: The range of elements within the array to search.
 * :param value: The value to search for.
 * :param comparator: The comparison function used to determine the order of elements and to find the value. Must not be NULL.
 * :param context: A pointer to user-defined data to be passed to the `comparator` function. Can be NULL.
 * :return: The index of the found value if it exists within the specified range; otherwise, `kOCNotFound` (-1).
 */
int64_t OCArrayBSearchValues(OCArrayRef array, OCRange range, const void *value, OCComparatorFunction comparator, void *context);

/**
 * Creates a new, immutable array from an existing array.
 *
 * :param array: The array to copy. Must not be NULL.
 * :return: A new OCArrayRef instance that is a copy of `array`, or NULL if `array` is NULL or copying fails.
 */
OCArrayRef OCArrayCreateWithArray(OCArrayRef array);

/** @} */ // end of OCArray group

#endif /* OCArray_h */
