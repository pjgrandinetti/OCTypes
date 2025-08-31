/**
 * @file OCArray.h
 * @brief Declares the OCArray and OCMutableArray interfaces.
 *
 * OCArray provides immutable and mutable array data structures
 * with customizable memory management and equality semantics.
 */
#ifndef OCArray_h
#define OCArray_h

#include "OCType.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif
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
 * @ingroup OCArray
 */
typedef const void *(*OCArrayRetainCallBack)(const void *value);
/**
 * @brief Callback to release a value from the array.
 *
 * @param value The value to be released.
 * @ingroup OCArray
 */
typedef void (*OCArrayReleaseCallBack)(const void *value);
/**
 * @brief Callback to copy a description string of a value.
 *
 * @param value The value to describe.
 * @return An OCStringRef describing the value.
 * @ingroup OCArray
 */
typedef OCStringRef (*OCArrayCopyDescriptionCallBack)(const void *value);
/**
 * @brief Callback to compare two values for equality.
 *
 * @param value1 First value.
 * @param value2 Second value.
 * @return true if equal, false otherwise.
 * @ingroup OCArray
 */
typedef bool (*OCArrayEqualCallBack)(const void *value1, const void *value2);
/**
 * @brief Struct defining callbacks for array value management.
 * @ingroup OCArray
 */
typedef struct {
    int64_t version;                                /**< Structure version (should be 0). */
    OCArrayRetainCallBack retain;                   /**< Retain callback. */
    OCArrayReleaseCallBack release;                 /**< Release callback. */
    OCArrayCopyDescriptionCallBack copyDescription; /**< Description callback. */
    OCArrayEqualCallBack equal;                     /**< Equality comparison callback. */
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
 *
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
 * Releases the existing value (if applicable) and retains the new value using the array's callbacks.
 *
 * @param theArray The mutable array.
 * @param index The index at which to set the value.
 * @param value The new value.
 * @return true if the operation succeeded; false if the array is NULL, the index is invalid, or memory operations failed.
 * @ingroup OCArray
 */
bool OCArraySetValueAtIndex(OCMutableArrayRef theArray, OCIndex index, const void *value);
/**
 * @brief Appends a value to the end of a mutable array.
 *
 * Retains the value using the array's callbacks. Will reallocate storage if the internal capacity is exceeded.
 *
 * @param theArray The mutable array.
 * @param value The value to append.
 * @return true if the value was appended successfully; false if the array or value is NULL, or if allocation failed.
 * @ingroup OCArray
 */
bool OCArrayAppendValue(OCMutableArrayRef theArray, const void *value);
/**
 * @brief Checks whether a value is present in the array.
 *
 * Uses the array's equality callback to determine equivalence. If no callback is set,
 * compares pointers directly. For `kOCTypeArrayCallBacks`, uses `OCTypeEqual`.
 *
 * @param theArray The array to check.
 * @param value The value to look for.
 * @return true if the value is found, false otherwise (including if array or value is NULL).
 * @ingroup OCArray
 */
bool OCArrayContainsValue(OCArrayRef theArray, const void *value);
/**
 * @brief Appends a range of values from another array into a mutable array.
 *
 * Each value is retained using the destination array's callbacks.
 *
 * @param theArray The mutable destination array.
 * @param otherArray The source array to copy from.
 * @param range The range [location, location+length) of values to append from the source.
 * @return true if all values were appended successfully; false if parameters are invalid or memory operations fail.
 * @ingroup OCArray
 */
bool OCArrayAppendArray(OCMutableArrayRef theArray, OCArrayRef otherArray, OCRange range);
/**
 * @brief Finds the first index of a value in the array.
 *
 * Uses the array's equality callback to determine equivalence. If no callback is set,
 * compares pointers directly. For `kOCTypeArrayCallBacks`, uses `OCTypeEqual`.
 *
 * @param theArray The array to search.
 * @param value The value to search for.
 * @return The index of the first matching value, or `kOCNotFound` if not found or if the array/value is NULL.
 * @ingroup OCArray
 */
OCIndex OCArrayGetFirstIndexOfValue(OCArrayRef theArray, const void *value);
/**
 * @brief Removes a value at a specific index from a mutable array.
 *
 * The value is released using the array's release callback. The remaining elements are shifted to fill the gap.
 *
 * @param theArray The mutable array.
 * @param index The index of the value to remove.
 * @return true if the value was removed successfully; false if the array is NULL or the index is out of bounds.
 * @ingroup OCArray
 */
bool OCArrayRemoveValueAtIndex(OCMutableArrayRef theArray, uint64_t index);
/**
 * @brief Inserts a value into a mutable array at the specified index.
 *
 * Shifts existing elements to make space, retains the new value using the array's callbacks, and grows the internal buffer if needed.
 *
 * @param theArray The mutable array.
 * @param index The index at which to insert the new value (must be ≤ count).
 * @param value The value to insert.
 * @return true if the value was inserted successfully; false on invalid input or allocation failure.
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
/**
 * @brief Serializes an OCArray to JSON format.
 *
 * Creates a JSON array representation of the OCArray. If the typed parameter
 * is true, each element is serialized using typed JSON serialization when
 * available. If false, each element uses untyped serialization.
 * 
 * Arrays are native JSON types, so they are not wrapped in type information
 * even when typed=true.
 *
 * @param array A valid OCArrayRef.
 * @param typed If true, use typed serialization for elements; if false, use untyped.
 * @return A new cJSON array on success, or cJSON null on failure.
 *         The caller is responsible for managing the returned cJSON object.
 * @ingroup OCArray
 */
cJSON *OCArrayCreateJSON(OCArrayRef array, bool typed);

/**
 * @brief Create an OCArrayRef from a JSON array.
 *
 * Parses a native JSON array and deserializes each element using
 * non-typed deserialization. Arrays are native JSON types, so no
 * unwrapping is needed.
 *
 * @param json A cJSON array.
 * @return A newly allocated OCArrayRef, or NULL on failure.
 * @ingroup OCArray
 */
OCArrayRef OCArrayCreateFromJSON(cJSON *json);

/**
 * @brief Create an OCArrayRef from a JSON array with typed element deserialization.
 *
 * Parses a native JSON array and deserializes each element using
 * typed deserialization. Arrays are native JSON types, so the input
 * is still a native JSON array, not a wrapped object.
 *
 * @param json A cJSON array.
 * @return A newly allocated OCArrayRef, or NULL on failure.
 * @ingroup OCArray
 */
OCArrayRef OCArrayCreateFromJSONTyped(cJSON *json);
/**
 * @brief Returns the callback structure associated with the array.
 *
 * @param array The array.
 * @return Pointer to the array’s OCArrayCallBacks, or NULL if not available.
 * @ingroup OCArray
 */
const OCArrayCallBacks *OCArrayGetCallBacks(OCArrayRef array);
/** @} */  // end of OCArray group
#endif     /* OCArray_h */
