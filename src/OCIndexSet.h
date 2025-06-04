/**
 * @file OCIndexSet.h
 * @brief Public interface for OCIndexSet and OCMutableIndexSet types.
 *
 * Provides creation, mutation, access, and serialization of index sets represented
 * as sorted arrays of OCIndex values. Supports efficient range-based construction
 * and full integration with OC serialization APIs.
 */

#ifndef OCINDEXSET_H
#define OCINDEXSET_H

#include "OCLibrary.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates an immutable, empty OCIndexSet.
 */
OCIndexSetRef OCIndexSetCreate(void);

/**
 * @brief Creates a mutable, empty OCIndexSet.
 */
OCMutableIndexSetRef OCIndexSetCreateMutable(void);

/**
 * @brief Creates an immutable copy of the provided set.
 */
OCIndexSetRef OCIndexSetCreateCopy(OCIndexSetRef theIndexSet);

/**
 * @brief Creates a mutable copy of the provided set.
 */
OCMutableIndexSetRef OCIndexSetCreateMutableCopy(OCIndexSetRef theIndexSet);

/**
 * @brief Creates an OCIndexSet containing exactly one index.
 */
OCIndexSetRef OCIndexSetCreateWithIndex(OCIndex index);

/**
 * @brief Creates an OCIndexSet containing all indices in [location, location + length).
 */
OCIndexSetRef OCIndexSetCreateWithIndexesInRange(OCIndex location, OCIndex length);

/**
 * @brief Gets the underlying OCData buffer containing the indices.
 */
OCDataRef OCIndexSetGetIndexes(OCIndexSetRef theIndexSet);

/**
 * @brief Returns a pointer to the raw OCIndex array (internal storage).
 */
OCIndex *OCIndexSetGetBytePtr(OCIndexSetRef theIndexSet);

/**
 * @brief Gets the number of indices stored in the set.
 */
OCIndex OCIndexSetGetCount(OCIndexSetRef theIndexSet);

/**
 * @brief Gets the smallest (first) index.
 */
OCIndex OCIndexSetFirstIndex(OCIndexSetRef theIndexSet);

/**
 * @brief Gets the largest (last) index.
 */
OCIndex OCIndexSetLastIndex(OCIndexSetRef theIndexSet);

/**
 * @brief Finds the largest index smaller than the given one.
 */
OCIndex OCIndexSetIndexLessThanIndex(OCIndexSetRef theIndexSet, OCIndex index);

/**
 * @brief Finds the smallest index greater than the given one.
 */
OCIndex OCIndexSetIndexGreaterThanIndex(OCIndexSetRef theIndexSet, OCIndex index);

/**
 * @brief Checks if the set contains a given index.
 */
bool OCIndexSetContainsIndex(OCIndexSetRef theIndexSet, OCIndex index);

/**
 * @brief Inserts a new index into the mutable set.
 */
bool OCIndexSetAddIndex(OCMutableIndexSetRef theIndexSet, OCIndex index);

/**
 * @brief Compares two index sets for equality.
 */
bool OCIndexSetEqual(OCIndexSetRef input1, OCIndexSetRef input2);

/**
 * @brief Converts the index set into an OCArray of OCNumber objects.
 */
OCArrayRef OCIndexSetCreateOCNumberArray(OCIndexSetRef theIndexSet);

/**
 * @brief Serializes the set to a dictionary (e.g., for plist storage).
 */
OCDictionaryRef OCIndexSetCreatePList(OCIndexSetRef theIndexSet);

/**
 * @brief Reconstructs a set from a dictionary produced by OCIndexSetCreatePList.
 */
OCIndexSetRef OCIndexSetCreateWithPList(OCDictionaryRef dictionary);

/**
 * @brief Creates an OCData snapshot of the current set.
 */
OCDataRef OCIndexSetCreateData(OCIndexSetRef theIndexSet);

/**
 * @brief Creates a set using the bytes from an existing OCData object.
 */
OCIndexSetRef OCIndexSetCreateWithData(OCDataRef data);

/**
 * @brief Prints the contents of the set to stderr.
 */
void OCIndexSetShow(OCIndexSetRef theIndexSet);

#ifdef __cplusplus
}
#endif

#endif /* OCINDEXSET_H */
