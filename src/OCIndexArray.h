#ifndef OCINDEXARRAY_H
#define OCINDEXARRAY_H

#include "OCLibrary.h"    // Provides OCTypeID, OCBase, OCDataRef, OCMutableDataRef, OCIndexSetRef, OCArrayRef, OCMutableArrayRef, OCDictionaryRef, OCNumberRef, OCStringRef, csdmNumericType, kOCNotFound, etc.
#include <stdbool.h>      // For bool

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------
// Constructors
//-------------------------------------------------------------------------

/**
 * Creates an immutable OCIndexArray from a C array of `long` values.
 *
 * @param indexes    Pointer to a C array of `long`.
 * @param numValues  Number of elements in `indexes`.
 * @return           A new OCIndexArrayRef containing a copy of the data.
 */
OCIndexArrayRef
OCIndexArrayCreate(long *indexes,
                   long numValues);

/**
 * Creates a mutable OCIndexArray with an initial capacity (in number of `long` slots).
 *
 * @param capacity   Initial capacity (number of `long` elements).
 * @return           A new, empty OCMutableIndexArrayRef with `capacity` slots allocated.
 */
OCMutableIndexArrayRef
OCIndexArrayCreateMutable(long capacity);

/**
 * Creates an OCIndexArray by copying the internal OCDataRef of an existing array.
 *
 * @param indexes  Existing OCDataRef housing a contiguous buffer of `long` values.
 * @return         A new OCIndexArrayRef containing a copy of `indexes`.
 */
OCIndexArrayRef
OCIndexArrayCreateWithParameters(OCDataRef indexes);

/**
 * Creates a mutable OCIndexArray by copying an existing OCMutableDataRef.
 *
 * @param indexes  Existing OCMutableDataRef (mutable data) of `long` values.
 * @return         A new OCMutableIndexArrayRef containing a fresh mutable copy.
 */
OCMutableIndexArrayRef
OCIndexArrayCreateMutableWithParameters(OCMutableDataRef indexes);

/**
 * Creates a deep copy of an existing OCIndexArray.
 *
 * @param theIndexArray  Source OCIndexArrayRef to copy.
 * @return               A new OCIndexArrayRef with identical contents.
 */
OCIndexArrayRef
OCIndexArrayCreateCopy(OCIndexArrayRef theIndexArray);

/**
 * Creates a deep, mutable copy of an existing OCIndexArray.
 *
 * @param theIndexArray  Source OCIndexArrayRef to copy.
 * @return               A new OCMutableIndexArrayRef with identical contents.
 */
OCMutableIndexArrayRef
OCIndexArrayCreateMutableCopy(OCIndexArrayRef theIndexArray);

//-------------------------------------------------------------------------
// Accessors
//-------------------------------------------------------------------------

/**
 * Returns the number of elements stored in the index array.
 *
 * @param theIndexArray  OCIndexArrayRef to query.
 * @return               Count of `long` values, or 0 if null.
 */
long
OCIndexArrayGetCount(OCIndexArrayRef theIndexArray);

/**
 * Returns a pointer to the mutable `long *` buffer (raw data) of the array.
 * Use with caution: modifying these bytes directly will affect the array.
 *
 * @param theIndexArray  OCIndexArrayRef to query.
 * @return               Pointer to `long` buffer, or NULL if empty.
 */
long *
OCIndexArrayGetMutableBytePtr(OCIndexArrayRef theIndexArray);

/**
 * Returns the `long` value at a given index.
 *
 * @param theIndexArray  OCIndexArrayRef to query.
 * @param index          Zero-based index into the array.
 * @return               The `long` at that position, or kOCNotFound if out of bounds.
 */
long
OCIndexArrayGetValueAtIndex(OCIndexArrayRef theIndexArray,
                            long index);

/**
 * Checks if the array contains a given `long` value.
 *
 * @param theIndexArray  OCIndexArrayRef to search.
 * @param index          The `long` value to look for.
 * @return               true if found; false otherwise.
 */
bool
OCIndexArrayContainsIndex(OCIndexArrayRef theIndexArray);

//-------------------------------------------------------------------------
// Mutators
//-------------------------------------------------------------------------

/**
 * Sets a new `long` value at the specified index (in-place).
 *
 * @param theIndexArray  OCMutableIndexArrayRef to modify.
 * @param index          Zero-based position to overwrite.
 * @param value          New `long` value to store.
 * @return               true if successful; false if out of bounds or null.
 */
bool
OCIndexArraySetValueAtIndex(OCMutableIndexArrayRef theIndexArray,
                            long index,
                            long value);

/**
 * Removes the element at a given index, shifting subsequent values down by one.
 *
 * @param theIndexArray  OCMutableIndexArrayRef to modify.
 * @param index          Zero-based position to remove.
 * @return               true if removal succeeded; false if out of range or null.
 */
bool
OCIndexArrayRemoveValueAtIndex(OCMutableIndexArrayRef theIndexArray,
                               long index);

/**
 * Removes all values at indices specified by an OCIndexSet, in descending order.
 *
 * @param theIndexArray  OCMutableIndexArrayRef to modify.
 * @param theIndexSet    OCIndexSetRef containing zero-based positions to delete.
 */
void
OCIndexArrayRemoveValuesAtIndexes(OCMutableIndexArrayRef theIndexArray,
                                  OCIndexSetRef theIndexSet);

/**
 * Appends a new `long` value to the end of the array (increasing its length by one).
 *
 * @param theIndexArray  OCMutableIndexArrayRef to modify.
 * @param index          The `long` value to append.
 * @return               true if append succeeded; false if null.
 */
bool
OCIndexArrayAppendValue(OCMutableIndexArrayRef theIndexArray,
                        long index);

/**
 * Appends the contents of a second OCIndexArray onto the end of a mutable array.
 *
 * @param theIndexArray   OCMutableIndexArrayRef to modify.
 * @param arrayToAppend   OCIndexArrayRef whose contents will be appended.
 * @return                true if concatenation succeeded; false if null.
 */
bool
OCIndexArrayAppendValues(OCMutableIndexArrayRef theIndexArray,
                         OCIndexArrayRef      arrayToAppend);

//-------------------------------------------------------------------------
// Serialization & Data Conversion
//-------------------------------------------------------------------------

/**
 * Creates a Base64-encoded OCString from the raw bytes of the index array,
 * choosing the proper integer size (8/16/32/64 bits) according to `integerType`.
 *
 * @param theIndexArray  OCIndexArrayRef to encode.
 * @param integerType    csdmNumericType specifying element size (e.g., kCSDMNumberUInt16Type).
 * @return               OCStringRef containing Base64 result (caller must release).
 */
OCStringRef
OCIndexArrayCreateBase64String(OCIndexArrayRef  theIndexArray,
                               csdmNumericType  integerType);

/**
 * Converts the index array into an OCArray of CFNumbers (one CFNumber per entry).
 *
 * @param theIndexArray  OCIndexArrayRef to convert.
 * @return               OCArrayRef of OCNumberRefs; caller must release.
 */
OCArrayRef
OCIndexArrayCreateCFNumberArray(OCIndexArrayRef theIndexArray);

/**
 * Creates a plist-compatible dictionary containing a single key "indexes"
 * mapping to the raw OCDataRef.  Useful for writing to disk or transferring.
 *
 * @param theIndexArray  OCIndexArrayRef to serialize.
 * @return               OCDictionaryRef containing the data under key "indexes"; caller must release.
 */
OCDictionaryRef
OCIndexArrayCreatePList(OCIndexArrayRef theIndexArray);

/**
 * Recreates an OCIndexArray from a plist dictionary created by
 * OCIndexArrayCreatePList.  Expects the key "indexes" → OCDataRef.
 *
 * @param dictionary  OCDictionaryRef containing key "indexes".
 * @return            OCIndexArrayRef built from that data (caller must release).
 */
OCIndexArrayRef
OCIndexArrayCreateWithPList(OCDictionaryRef dictionary);

/**
 * Returns an owned OCDataRef containing the raw bytes of the index array.
 *
 * @param theIndexArray  OCIndexArrayRef to query.
 * @return               OCDataRef (caller must release); or NULL if input is NULL.
 */
OCDataRef
OCIndexArrayCreateData(OCIndexArrayRef theIndexArray);

/**
 * Creates an OCIndexArray directly from an existing OCDataRef.
 * The caller retains ownership of that data internally (CFRetain).
 *
 * @param data  OCDataRef whose bytes represent consecutive `long` values.
 * @return      OCIndexArrayRef built on top of that data (caller must release).
 */
OCIndexArrayRef
OCIndexArrayCreateWithData(OCDataRef data);

//-------------------------------------------------------------------------
// PList/Parameters Helpers (formerly `static`)
//-------------------------------------------------------------------------

/**
 * Internal helper to build a new OCIndexArray from an existing OCDataRef.
 * Effectively duplicates the data.
 *
 * @param indexes  OCDataRef to copy.
 * @return         OCIndexArrayRef (caller must release).
 */
OCIndexArrayRef
OCIndexArrayCreateWithParameters(OCDataRef indexes);

/**
 * Internal helper to build a new OCMutableIndexArray from an existing OCMutableDataRef.
 *
 * @param indexes  OCMutableDataRef to copy.
 * @return         OCMutableIndexArrayRef (caller must release).
 */
OCMutableIndexArrayRef
OCIndexArrayCreateMutableWithParameters(OCMutableDataRef indexes);

//-------------------------------------------------------------------------
// Utility / Debug
//-------------------------------------------------------------------------

/**
 * Prints the contents of an OCIndexArray to stderr in the form "(v0,v1,v2,...)".
 *
 * @param theIndexArray  OCIndexArrayRef to display.
 */
void
OCIndexArrayShow(OCIndexArrayRef theIndexArray);

#ifdef __cplusplus
}
#endif

#endif /* OCINDEXARRAY_H */