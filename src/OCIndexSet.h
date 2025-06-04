#ifndef OCINDEXSET_H
#define OCINDEXSET_H

#include "OCLibrary.h"   // Provides OCTypeID, OCBase, OCDataRef, OCMutableDataRef, OCArrayRef, OCDictionaryRef, OCNumberRef, etc.
#include <stdbool.h>     // For bool

#ifdef __cplusplus
extern "C" {
#endif


//-------------------------------------------------------------------------
// Constructors
//-------------------------------------------------------------------------

/**
 * Creates an immutable, empty OCIndexSet.
 *
 * @return A new OCIndexSetRef (caller must release).
 */
OCIndexSetRef
OCIndexSetCreate(void);

/**
 * Creates a mutable, empty OCIndexSet.
 *
 * @return A new OCMutableIndexSetRef (caller must release).
 */
OCMutableIndexSetRef
OCIndexSetCreateMutable(void);

/**
 * Creates an immutable copy of the provided set.
 *
 * @param theIndexSet  Existing OCIndexSetRef (may be NULL).
 * @return             A new OCIndexSetRef with identical contents, or an empty set if input was NULL.
 */
OCIndexSetRef
OCIndexSetCreateCopy(OCIndexSetRef theIndexSet);

/**
 * Creates a mutable copy of the provided set.
 *
 * @param theIndexSet  Existing OCIndexSetRef (may be NULL).
 * @return             A new OCMutableIndexSetRef with identical contents, or an empty set if input was NULL.
 */
OCMutableIndexSetRef
OCIndexSetCreateMutableCopy(OCIndexSetRef theIndexSet);

/**
 * Creates an OCIndexSet containing exactly one index.
 *
 * @param index  The single index to store.
 * @return       A new OCIndexSetRef (caller must release).
 */
OCIndexSetRef
OCIndexSetCreateWithIndex(long index);

/**
 * Creates an OCIndexSet containing all indices in [location, location+length):
 *   { location, location+1, …, location+length−1 }. If length ≤ 0, returns an empty set.
 *
 * @param location  Starting index (inclusive).
 * @param length    Number of consecutive indices.
 * @return          A new OCIndexSetRef (caller must release).
 */
OCIndexSetRef
OCIndexSetCreateWithIndexesInRange(long location,
                                   long length);


//-------------------------------------------------------------------------
// Accessors
//-------------------------------------------------------------------------

/**
 * Returns the underlying OCDataRef containing the sorted array of longs.
 * May be NULL if the set is empty. Caller may CFRetain if ownership is required.
 *
 * @param theIndexSet  OCIndexSetRef to query.
 * @return             OCDataRef of raw index bytes, or NULL.
 */
OCDataRef
OCIndexSetGetIndexes(OCIndexSetRef theIndexSet);

/**
 * Returns a pointer to the contiguous long‐array buffer. Do not modify unless you know the internals.
 *
 * @param theIndexSet  OCIndexSetRef to query.
 * @return             long *, or NULL if the set is empty.
 */
long *
OCIndexSetGetBytePtr(OCIndexSetRef theIndexSet);

/**
 * Returns the number of indices stored.
 *
 * @param theIndexSet  OCIndexSetRef to query.
 * @return             Count of longs, or 0 if empty.
 */
long
OCIndexSetGetCount(OCIndexSetRef theIndexSet);

/**
 * Returns the first (smallest) index, or kOCNotFound if the set is empty.
 *
 * @param theIndexSet  OCIndexSetRef to query.
 * @return             First index, or kOCNotFound.
 */
long
OCIndexSetFirstIndex(OCIndexSetRef theIndexSet);

/**
 * Returns the last (largest) index, or kOCNotFound if the set is empty.
 *
 * @param theIndexSet  OCIndexSetRef to query.
 * @return             Last index, or kOCNotFound.
 */
long
OCIndexSetLastIndex(OCIndexSetRef theIndexSet);

/**
 * Returns the greatest index < the given index, or kOCNotFound if none exists.
 *
 * @param theIndexSet  OCIndexSetRef to query.
 * @param index        Reference index.
 * @return             Next‐lower index, or kOCNotFound.
 */
long
OCIndexSetIndexLessThanIndex(OCIndexSetRef theIndexSet,
                             long            index);

/**
 * Returns the smallest index > the given index, or kOCNotFound if none exists.
 *
 * @param theIndexSet  OCIndexSetRef to query.
 * @param index        Reference index.
 * @return             Next‐higher index, or kOCNotFound.
 */
long
OCIndexSetIndexGreaterThanIndex(OCIndexSetRef theIndexSet,
                                long            index);

/**
 * Returns true if the set contains exactly that index; false otherwise.
 *
 * @param theIndexSet  OCIndexSetRef to query.
 * @param index        Index to test.
 * @return             true if present; false otherwise.
 */
bool
OCIndexSetContainsIndex(OCIndexSetRef theIndexSet,
                        long          index);


//-------------------------------------------------------------------------
// Mutators (require OCMutableIndexSetRef)
//-------------------------------------------------------------------------

/**
 * Inserts the given index into the sorted set. If already present, no change is made and false is returned.
 *
 * @param theIndexSet  OCMutableIndexSetRef to modify.
 * @param index        The index to add.
 * @return             true if insertion occurred; false if duplicate or on error.
 */
bool
OCIndexSetAddIndex(OCMutableIndexSetRef theIndexSet,
                   long                 index);


//-------------------------------------------------------------------------
// Equality
//-------------------------------------------------------------------------

/**
 * Performs a byte‐for‐byte comparison of two OCIndexSet instances.
 *
 * @param input1  First OCIndexSetRef.
 * @param input2  Second OCIndexSetRef.
 * @return        true if they have equal length and identical contents; false otherwise.
 */
bool
OCIndexSetEqual(OCIndexSetRef input1,
                OCIndexSetRef input2);


//-------------------------------------------------------------------------
// Conversion & Serialization
//-------------------------------------------------------------------------

/**
 * Creates an OCArray (of OCNumberRef) representing each stored index.
 *
 * @param theIndexSet  OCIndexSetRef to convert.
 * @return             OCArrayRef containing OCNumberRefs in sorted order; caller must release.
 */
OCArrayRef
OCIndexSetCreateOCNumberArray(OCIndexSetRef theIndexSet);

/**
 * Creates a plist‐compatible dictionary with key "indexes" → OCDataRef of raw index bytes.
 *
 * @param theIndexSet  OCIndexSetRef to serialize.
 * @return             OCDictionaryRef (caller must release).
 */
OCDictionaryRef
OCIndexSetCreatePList(OCIndexSetRef theIndexSet);

/**
 * Reconstructs an OCIndexSet from a dictionary returned by OCIndexSetCreatePList.
 *
 * @param dictionary  OCDictionaryRef containing key "indexes" → OCDataRef.
 * @return            OCIndexSetRef (caller must release), or an empty set if no data found.
 */
OCIndexSetRef
OCIndexSetCreateWithPList(OCDictionaryRef dictionary);

/**
 * Returns an owned OCDataRef containing the raw index bytes. Caller must release.
 *
 * @param theIndexSet  OCIndexSetRef to query.
 * @return             OCDataRef (caller must release), or NULL if empty.
 */
OCDataRef
OCIndexSetCreateData(OCIndexSetRef theIndexSet);

/**
 * Constructs a new OCIndexSet from an existing OCDataRef of raw index bytes.
 *
 * @param data  OCDataRef whose bytes represent sorted `long` values.
 * @return      OCIndexSetRef (caller must release).
 */
OCIndexSetRef
OCIndexSetCreateWithData(OCDataRef data);


//-------------------------------------------------------------------------
// Debugging
//-------------------------------------------------------------------------

/**
 * Prints the entire contents of the set to stderr in the form “(i0,i1,i2,…)”.
 *
 * @param theIndexSet  OCIndexSetRef to display.
 */
void
OCIndexSetShow(OCIndexSetRef theIndexSet);


#ifdef __cplusplus
}
#endif

#endif /* OCINDEXSET_H */