//
//  OCArray.h
//  OCTypes
//
//  Created by philip on 4/2/17.
//

#ifndef OCArray_h
#define OCArray_h

#include "OCLibrary.h"

typedef const struct __OCArray * OCArrayRef;
typedef struct __OCArray * OCMutableArrayRef;

typedef const void * (*OCArrayRetainCallBack)(const void *value);
typedef void		(*OCArrayReleaseCallBack)(const void *value);
typedef OCStringRef	(*OCArrayCopyDescriptionCallBack)(const void *value);
typedef bool		(*OCArrayEqualCallBack)(const void *value1, const void *value2);
typedef struct {
    int64_t     version;
    OCArrayRetainCallBack   retain;
    OCArrayReleaseCallBack  release;
    OCArrayCopyDescriptionCallBack  copyDescription;
    OCArrayEqualCallBack    equal;
} OCArrayCallBacks;

extern const OCArrayCallBacks kOCTypeArrayCallBacks;

/**
 * Returns the unique type identifier for OCArray.
 * @return The OCTypeID for OCArray.
 */
OCTypeID OCArrayGetTypeID(void);

/**
 * Returns the number of values in the array.
 * @param theArray The array to query.
 * @return The count of elements in the array.
 */
uint64_t OCArrayGetCount(OCArrayRef theArray);

/**
 * Creates an immutable array with the given values.
 * @param values Pointer to an array of values.
 * @param numValues Number of values in the values array.
 * @param callBacks Callbacks for retain, release, description, and equality.
 * @return A new OCArrayRef containing the provided values.
 */
OCArrayRef OCArrayCreate(const void **values, uint64_t numValues, const OCArrayCallBacks *callBacks);

/**
 * Creates a copy of an existing immutable array.
 * @param theArray The array to copy.
 * @return A new OCArrayRef that is a copy of theArray.
 */
OCArrayRef OCArrayCreateCopy(OCArrayRef theArray);

/**
 * Creates a mutable array with an initial capacity.
 * @param capacity Initial capacity of the mutable array.
 * @param callBacks Callbacks for retain, release, description, and equality.
 * @return A new OCMutableArrayRef with the specified capacity.
 */
OCMutableArrayRef OCArrayCreateMutable(uint64_t capacity, const OCArrayCallBacks *callBacks);

/**
 * Creates a mutable copy of an existing array.
 * @param theArray The array to copy.
 * @return A new OCMutableArrayRef that is a mutable copy of theArray.
 */
OCMutableArrayRef OCArrayCreateMutableCopy(OCArrayRef theArray);

/**
 * Retrieves the value at the specified index.
 * @param theArray The array to query.
 * @param index The zero-based index of the value to retrieve.
 * @return The value at the given index.
 */
const void * OCArrayGetValueAtIndex(OCArrayRef theArray, uint64_t index);

/**
 * Appends a value to the end of the mutable array.
 * @param theArray The mutable array to modify.
 * @param value The value to append.
 */
void OCArrayAppendValue(OCMutableArrayRef theArray, const void * value);

/**
 * Appends a range of values from another array.
 * @param theArray The mutable array to modify.
 * @param otherArray The source array.
 * @param range The range of values to append.
 */
void OCArrayAppendArray(OCMutableArrayRef theArray, OCArrayRef otherArray, OCRange range);

/**
 * Returns the first index at which a given value appears.
 * @param theArray The array to search.
 * @param value The value to find.
 * @return The zero-based index of the first matching value, or -1 if not found.
 */
long OCArrayGetFirstIndexOfValue(OCArrayRef theArray, const void * value);

/**
 * Checks whether the array contains a given value.
 * @param theArray The array to search.
 * @param value The value to check for.
 * @return true if the value is found, false otherwise.
 */
bool OCArrayContainsValue(OCArrayRef theArray, const void * value);

/**
 * Removes the value at the specified index in a mutable array.
 * @param theArray The mutable array to modify.
 * @param index The index of the value to remove.
 */
void OCArrayRemoveValueAtIndex(OCMutableArrayRef theArray,uint64_t index);

/**
 * Inserts a value at the specified index in a mutable array.
 * @param theArray The mutable array to modify.
 * @param index The index at which to insert the value.
 * @param value The value to insert.
 */
void OCArrayInsertValueAtIndex(OCMutableArrayRef theArray, uint64_t index, const void * value);

/**
 * Sorts the values in a mutable array over a given range.
 * @param theArray The mutable array to sort.
 * @param range The range of elements to sort.
 * @param comparator The comparison function to determine order.
 * @param context The context pointer passed to the comparator.
 */
void OCArraySortValues(OCMutableArrayRef theArray, OCRange range, OCComparatorFunction comparator, void *context);

/**
 * Performs a binary search over a sorted range of values.
 * @param array The array to search.
 * @param range The range of elements to search.
 * @param value The value to search for.
 * @param comparator The comparison function.
 * @param context The context pointer passed to the comparator.
 * @return The index of the found value, or -1 if not found.
 */
int64_t OCArrayBSearchValues(OCArrayRef array, OCRange range, const void *value, OCComparatorFunction comparator, void *context);

#endif /* OCArray_h */
