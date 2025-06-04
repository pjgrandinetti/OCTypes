
#ifndef OCINDEXPAIRSET_H
#define OCINDEXPAIRSET_H

#include "OCLibrary.h"    // Provides OCTypeID, OCBase, OCDataRef, OCMutableDataRef, OCIndexArrayRef, OCIndexSetRef, OCArrayRef, OCDictionaryRef, OCStringRef, OCIndexPair, csdmNumericType, kOCNotFound, etc.
#include <stdbool.h>      // For bool

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------
// Type Definitions
//-------------------------------------------------------------------------

typedef struct OCIndexPair
{
    long index;
    long value;
} OCIndexPair;


//-------------------------------------------------------------------------
// Constructors
//-------------------------------------------------------------------------

/**
 * Creates an empty, immutable OCIndexPairSet.
 *
 * @return  New OCIndexPairSetRef with no pairs (caller must release).
 */
OCIndexPairSetRef
OCIndexPairSetCreate(void);

/**
 * Creates an empty, mutable OCIndexPairSet.
 *
 * @return  New OCMutableIndexPairSetRef with no pairs (caller must release).
 */
OCMutableIndexPairSetRef
OCIndexPairSetCreateMutable(void);

/**
 * Internal helper: Creates an immutable OCIndexPairSet from existing OCDataRef.
 *
 * @param indexPairs   OCDataRef containing a contiguous array of OCIndexPair structs.
 * @return             New OCIndexPairSetRef (caller must release).
 */
OCIndexPairSetRef
OCIndexPairSetCreateWithParameters(OCDataRef indexPairs);

/**
 * Internal helper: Creates a mutable OCIndexPairSet from existing OCDataRef.
 *
 * @param indexPairs   OCDataRef containing a contiguous array of OCIndexPair structs.
 * @return             New OCMutableIndexPairSetRef (caller must release).
 */
OCMutableIndexPairSetRef
OCIndexPairSetCreateMutableWithParameters(OCDataRef indexPairs);

/**
 * Creates a deep, immutable copy of an existing OCIndexPairSet.
 *
 * @param theIndexSet  Source OCIndexPairSetRef.
 * @return             New OCIndexPairSetRef (caller must release).
 */
OCIndexPairSetRef
OCIndexPairSetCreateCopy(OCIndexPairSetRef theIndexSet);

/**
 * Creates a deep, mutable copy of an existing OCIndexPairSet.
 *
 * @param theIndexSet  Source OCIndexPairSetRef.
 * @return             New OCMutableIndexPairSetRef (caller must release).
 */
OCMutableIndexPairSetRef
OCIndexPairSetCreateMutableCopy(OCIndexPairSetRef theIndexSet);

/**
 * Creates a mutable OCIndexPairSet from an OCIndexArray.
 * Each element in the array is treated as a value, with its index as the “index” of the pair.
 *
 * @param indexArray   OCIndexArrayRef whose elements become (index, value) pairs.
 * @return             New OCMutableIndexPairSetRef (caller must release).
 */
OCMutableIndexPairSetRef
OCIndexPairSetCreateMutableWithIndexArray(OCIndexArrayRef indexArray);

/**
 * Creates an immutable OCIndexPairSet from a C‐array of OCIndexPair elements.
 *
 * @param array      Pointer to an array of OCIndexPair structs.
 * @param numValues  Number of elements in that array.
 * @return           New OCIndexPairSetRef (caller must release).
 */
OCIndexPairSetRef
OCIndexPairSetCreateWithIndexPairArray(OCIndexPair *array,
                                       int        numValues);

/**
 * Creates an immutable OCIndexPairSet containing exactly one (index, value) pair.
 *
 * @param index  The index of the single pair.
 * @param value  The associated value.
 * @return       New OCIndexPairSetRef (caller must release).
 */
OCIndexPairSetRef
OCIndexPairSetCreateWithIndexPair(long index,
                                  long value);

/**
 * Creates an immutable OCIndexPairSet containing exactly two (index, value) pairs.
 *
 * @param index1  The first index.
 * @param value1  The first value.
 * @param index2  The second index.
 * @param value2  The second value.
 * @return        New OCIndexPairSetRef (caller must release).
 */
OCIndexPairSetRef
OCIndexPairSetCreateWithTwoIndexPairs(long index1,
                                      long value1,
                                      long index2,
                                      long value2);


//-------------------------------------------------------------------------
// Accessors
//-------------------------------------------------------------------------

/**
 * Returns the backing OCDataRef containing all OCIndexPair structs.
 *
 * @param theIndexSet  OCIndexPairSetRef to query.
 * @return             OCDataRef (caller must release or CFRetain if needed).
 */
OCDataRef
OCIndexPairSetGetIndexPairs(OCIndexPairSetRef theIndexSet);

/**
 * Returns the value associated with a given index in the set.
 *
 * @param theIndexSet  OCIndexPairSetRef to query.
 * @param index        The key to look up.
 * @return             The corresponding value, or kOCNotFound if not present.
 */
long
OCIndexPairSetValueForIndex(OCIndexPairSetRef theIndexSet,
                            long               index);

/**
 * Produces a new OCIndexArray of all “value” entries, in the same order as stored.
 *
 * @param theIndexSet  OCIndexPairSetRef to convert.
 * @return             OCIndexArrayRef of values (caller must release).
 */
OCIndexArrayRef
OCIndexPairSetCreateIndexArrayOfValues(OCIndexPairSetRef theIndexSet);

/**
 * Produces a new OCIndexSet containing all “index” keys in the set.
 *
 * @param theIndexSet  OCIndexPairSetRef to convert.
 * @return             OCIndexSetRef of indices (caller must release).
 */
OCIndexSetRef
OCIndexPairSetCreateIndexSetOfIndexes(OCIndexPairSetRef theIndexSet);

/**
 * Returns a raw pointer to the internal OCIndexPair buffer.
 * Modifying this buffer directly will alter the set’s contents.
 *
 * @param theIndexSet  OCIndexPairSetRef to query.
 * @return             OCIndexPair* or NULL if empty.
 */
OCIndexPair *
OCIndexPairSetGetBytePtr(OCIndexPairSetRef theIndexSet);

/**
 * Returns the number of (index, value) pairs stored in the set.
 *
 * @param theIndexSet  OCIndexPairSetRef to query.
 * @return             Count of pairs, or 0 if empty.
 */
long
OCIndexPairSetGetCount(OCIndexPairSetRef theIndexSet);

/**
 * Returns the first (index, value) pair in the set.
 * If the set is empty, returns {kOCNotFound, 0}.
 *
 * @param theIndexSet  OCIndexPairSetRef to query.
 * @return             First OCIndexPair in storage order.
 */
OCIndexPair
OCIndexPairSetFirstIndex(OCIndexPairSetRef theIndexSet);

/**
 * Returns the last (index, value) pair in the set.
 * If the set is empty, returns {kOCNotFound, 0}.
 *
 * @param theIndexSet  OCIndexPairSetRef to query.
 * @return             Last OCIndexPair in storage order.
 */
OCIndexPair
OCIndexPairSetLastIndex(OCIndexPairSetRef theIndexSet);

/**
 * Finds the largest (index, value) pair whose index is strictly less than the given pair’s index.
 *
 * @param theIndexSet  OCIndexPairSetRef to search.
 * @param indexPair    The reference OCIndexPair (only its .index field is used for comparison).
 * @return             OCIndexPair with the next‐lower index, or {kOCNotFound, 0} if none exists.
 */
OCIndexPair
OCIndexPairSetIndexPairLessThanIndexPair(OCIndexPairSetRef theIndexSet,
                                         OCIndexPair       indexPair);

/**
 * Returns true if the set contains a pair with the specified index (ignoring value).
 *
 * @param theIndexSet  OCIndexPairSetRef to query.
 * @param index        The key to look for.
 * @return             true if present; false otherwise.
 */
bool
OCIndexPairSetContainsIndex(OCIndexPairSetRef theIndexSet,
                            long               index);

/**
 * Returns true if the set contains exactly the specified (index, value) pair.
 *
 * @param theIndexSet  OCIndexPairSetRef to query.
 * @param indexPair    The OCIndexPair (both fields must match).
 * @return             true if present; false otherwise.
 */
bool
OCIndexPairSetContainsIndexPair(OCIndexPairSetRef theIndexSet,
                                OCIndexPair       indexPair);


//-------------------------------------------------------------------------
// Mutators
//-------------------------------------------------------------------------

/**
 * Removes the pair with the given index (if it exists), shifting subsequent pairs down.
 *
 * @param theIndexSet  OCMutableIndexPairSetRef to modify.
 * @param index        The key to remove.
 * @return             true if a pair was removed; false if not found or null.
 */
bool
OCIndexPairSetRemoveIndexPairWithIndex(OCMutableIndexPairSetRef theIndexSet,
                                       long                       index);

/**
 * Inserts a new (index, value) pair into the set in ascending‐index order.
 * If a pair with the same index already exists, insertion fails and returns false.
 *
 * @param theIndexSet  OCMutableIndexPairSetRef to modify.
 * @param index        The key to insert.
 * @param value        The associated value.
 * @return             true if inserted successfully; false if duplicate index or null.
 */
bool
OCIndexPairSetAddIndexPair(OCMutableIndexPairSetRef theIndexSet,
                           long                       index,
                           long                       value);


//-------------------------------------------------------------------------
// Serialization & Data Conversion
//-------------------------------------------------------------------------

/**
 * Creates a plist‐compatible dictionary containing a single key "indexPairs"
 * mapping to the raw OCDataRef. Useful for writing to disk or transferring.
 *
 * @param theIndexSet  OCIndexPairSetRef to serialize.
 * @return             OCDictionaryRef containing the data under key "indexPairs"; caller must release.
 */
OCDictionaryRef
OCIndexPairSetCreatePList(OCIndexPairSetRef theIndexSet);

/**
 * Recreates an OCIndexPairSet from a plist dictionary created by
 * OCIndexPairSetCreatePList. Expects the key "indexPairs" → OCDataRef.
 *
 * @param dictionary  OCDictionaryRef containing key "indexPairs".
 * @return            OCIndexPairSetRef built from that data (caller must release).
 */
OCIndexPairSetRef
OCIndexPairSetCreateWithPList(OCDictionaryRef dictionary);

/**
 * Returns an owned OCDataRef containing the raw bytes of the index‐pair set.
 *
 * @param theIndexSet  OCIndexPairSetRef to query.
 * @return             OCDataRef (caller must release); or NULL if input is NULL.
 */
OCDataRef
OCIndexPairSetCreateData(OCIndexPairSetRef theIndexSet);

/**
 * Creates an OCIndexPairSet directly from an existing OCDataRef.
 * The caller retains ownership of that data internally (CFRetain).
 *
 * @param data  OCDataRef whose bytes represent consecutive OCIndexPair structs.
 * @return      OCIndexPairSetRef built on top of that data (caller must release).
 */
OCIndexPairSetRef
OCIndexPairSetCreateWithData(OCDataRef data);


//-------------------------------------------------------------------------
// Utility / Debug
//-------------------------------------------------------------------------

/**
 * Prints the contents of an OCIndexPairSet to stderr in the form:
 *     ((i0,v0),(i1,v1),...)
 *
 * @param theIndexPairSet  OCIndexPairSetRef to display.
 */
void
OCIndexPairSetShow(OCIndexPairSetRef theIndexPairSet);

#ifdef __cplusplus
}
#endif

#endif /* OCINDEXPAIRSET_H */


