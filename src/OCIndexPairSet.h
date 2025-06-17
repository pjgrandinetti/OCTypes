/**
 * @file OCIndexPairSet.h
 * @brief Declares OCIndexPairSet and OCMutableIndexPairSet interfaces.
 *
 * OCIndexPairSet provides immutable and mutable collections of (index, value) pairs,
 * with support for creation from arrays, lookup, and serialization to plist or OCData.
 */

#ifndef OCINDEXPAIRSET_H
#define OCINDEXPAIRSET_H

#include "OCLibrary.h"
#include <stdbool.h>

/**
 * @defgroup OCIndexPairSet OCIndexPairSet
 * @brief APIs for sets of OCIndex–OCIndex pairs (OCIndexPairSet and mutable variant).
 *
 * This group includes types and functions to create, query, modify, and serialize
 * collections of OCIndexPair structures, where each pair holds an index and its
 * associated value.
 * @{
 */

/**
 * @struct OCIndexPair
 * @brief A simple (index, value) pair.
 *
 * Each OCIndexPair contains two OCIndex members:
 * - index:   the lookup key.
 * - value:   the associated value for that index.
 */
typedef struct OCIndexPair {
    OCIndex index; /**< The index (key) component. */
    OCIndex value; /**< The associated value for this index. */
} OCIndexPair;

/**
 * @brief Returns the unique OCTypeID for OCIndexPairSet.
 *
 * @return The OCTypeID corresponding to OCIndexPairSet.
 * @ingroup OCIndexPairSet
 */
OCTypeID OCIndexPairSetGetTypeID(void);

/**
 * @brief Creates an empty immutable OCIndexPairSet.
 *
 * @return A new OCIndexPairSetRef, or NULL on allocation failure.
 * @ingroup OCIndexPairSet
 */
OCIndexPairSetRef OCIndexPairSetCreate(void);

/**
 * @brief Creates an empty mutable OCIndexPairSet.
 *
 * @return A new OCMutableIndexPairSetRef, or NULL on allocation failure.
 * @ingroup OCIndexPairSet
 */
OCMutableIndexPairSetRef OCIndexPairSetCreateMutable(void);

/**
 * @brief Creates a deep immutable copy of the given set.
 *
 * @param source The OCIndexPairSetRef to copy.
 * @return A new OCIndexPairSetRef that is a deep copy of source, or NULL on error.
 * @ingroup OCIndexPairSet
 */
OCIndexPairSetRef OCIndexPairSetCreateCopy(OCIndexPairSetRef source);

/**
 * @brief Creates a deep mutable copy of the given set.
 *
 * @param source The OCIndexPairSetRef to copy.
 * @return A new OCMutableIndexPairSetRef that is a deep copy of source, or NULL on error.
 * @ingroup OCIndexPairSet
 */
OCMutableIndexPairSetRef OCIndexPairSetCreateMutableCopy(OCIndexPairSetRef source);

/**
 * @brief Creates a mutable index-pair set from an OCIndexArray.
 *
 * @details Each element in indexArray becomes a pair (index = position, value = element).
 *
 * @param indexArray An OCIndexArrayRef whose elements supply values; indices are 0-based.
 * @return A new OCMutableIndexPairSetRef, or NULL on error.
 * @ingroup OCIndexPairSet
 */
OCMutableIndexPairSetRef OCIndexPairSetCreateMutableWithIndexArray(OCIndexArrayRef indexArray);

/**
 * @brief Creates an immutable set from a C array of OCIndexPair.
 *
 * @param array Pointer to a C array of OCIndexPair structures.
 * @param count Number of elements in the array.
 * @return A new OCIndexPairSetRef containing those pairs, or NULL on error.
 * @ingroup OCIndexPairSet
 */
OCIndexPairSetRef OCIndexPairSetCreateWithIndexPairArray(OCIndexPair *array, int count);

/**
 * @brief Creates an immutable set with a single index–value pair.
 *
 * @param index The index (key) for the pair.
 * @param value The value associated with index.
 * @return A new OCIndexPairSetRef containing that single pair, or NULL on error.
 * @ingroup OCIndexPairSet
 */
OCIndexPairSetRef OCIndexPairSetCreateWithIndexPair(OCIndex index, OCIndex value);

/**
 * @brief Creates an immutable set with two index–value pairs.
 *
 * @param index1 The first pair’s index (key).
 * @param value1 The first pair’s value.
 * @param index2 The second pair’s index (key).
 * @param value2 The second pair’s value.
 * @return A new OCIndexPairSetRef containing both pairs, or NULL on error.
 * @ingroup OCIndexPairSet
 */
OCIndexPairSetRef OCIndexPairSetCreateWithTwoIndexPairs(
    OCIndex index1, OCIndex value1,
    OCIndex index2, OCIndex value2
);

/**
 * @brief Returns the backing OCDataRef that holds all index pairs.
 *
 * @param set The OCIndexPairSetRef instance.
 * @return An OCDataRef containing a contiguous buffer of OCIndexPair structs;
 *         caller must release. Returns NULL if set is NULL.
 * @ingroup OCIndexPairSet
 */
OCDataRef OCIndexPairSetGetIndexPairs(OCIndexPairSetRef set);

/**
 * @brief Returns the number of pairs in the set.
 *
 * @param set The OCIndexPairSetRef instance.
 * @return The count of OCIndexPair elements, or 0 if set is NULL.
 * @ingroup OCIndexPairSet
 */
OCIndex OCIndexPairSetGetCount(OCIndexPairSetRef set);

/**
 * @brief Returns a pointer to the internal OCIndexPair array.
 *
 * @param set The OCIndexPairSetRef instance.
 * @return Pointer to an array of OCIndexPair; do not modify if set is immutable.
 *         Returns NULL if set is NULL.
 * @ingroup OCIndexPairSet
 */
OCIndexPair *OCIndexPairSetGetBytesPtr(OCIndexPairSetRef set);

/**
 * @brief Retrieves the value associated with a given index.
 *
 * @param set   The OCIndexPairSetRef instance.
 * @param index The index (key) to look up.
 * @return The associated OCIndex value, or kOCNotFound if index is not present.
 * @ingroup OCIndexPairSet
 */
OCIndex OCIndexPairSetValueForIndex(OCIndexPairSetRef set, OCIndex index);

/**
 * @brief Returns an OCIndexArray of all values in the set.
 *
 * @param set The OCIndexPairSetRef instance.
 * @return A new OCIndexArrayRef containing all values (in ascending index order),
 *         or NULL on error.
 * @ingroup OCIndexPairSet
 */
OCIndexArrayRef OCIndexPairSetCreateIndexArrayOfValues(OCIndexPairSetRef set);

/**
 * @brief Returns an OCIndexSet of all indexes in the set.
 *
 * @param set The OCIndexPairSetRef instance.
 * @return A new OCIndexSetRef containing all indexes (in ascending order),
 *         or NULL on error.
 * @ingroup OCIndexPairSet
 */
OCIndexSetRef OCIndexPairSetCreateIndexSetOfIndexes(OCIndexPairSetRef set);

/**
 * @brief Returns the first index–value pair in the set.
 *
 * @param set The OCIndexPairSetRef instance.
 * @return The OCIndexPair for the smallest index, or {kOCNotFound, kOCNotFound} if empty.
 * @ingroup OCIndexPairSet
 */
OCIndexPair OCIndexPairSetFirstIndex(OCIndexPairSetRef set);

/**
 * @brief Returns the last index–value pair in the set.
 *
 * @param set The OCIndexPairSetRef instance.
 * @return The OCIndexPair for the largest index, or {kOCNotFound, kOCNotFound} if empty.
 * @ingroup OCIndexPairSet
 */
OCIndexPair OCIndexPairSetLastIndex(OCIndexPairSetRef set);

/**
 * @brief Returns the pair immediately before the given target, by index.
 *
 * @param set    The OCIndexPairSetRef instance.
 * @param target An OCIndexPair whose index field specifies the search target.
 * @return The OCIndexPair whose index is the greatest value less than target.index,
 *         or {kOCNotFound, kOCNotFound} if none exists.
 * @ingroup OCIndexPairSet
 */
OCIndexPair OCIndexPairSetIndexPairLessThanIndexPair(OCIndexPairSetRef set, OCIndexPair target);

/**
 * @brief Checks if the set contains a given index (key).
 *
 * @param set   The OCIndexPairSetRef instance.
 * @param index The index (key) to check.
 * @return true if the index exists; false otherwise.
 * @ingroup OCIndexPairSet
 */
bool OCIndexPairSetContainsIndex(OCIndexPairSetRef set, OCIndex index);

/**
 * @brief Checks if the set contains the exact index–value pair.
 *
 * @param set  The OCIndexPairSetRef instance.
 * @param pair The OCIndexPair to check for existence.
 * @return true if the pair exists (both index and value match); false otherwise.
 * @ingroup OCIndexPairSet
 */
bool OCIndexPairSetContainsIndexPair(OCIndexPairSetRef set, OCIndexPair pair);

/**
 * @brief Adds a new index–value pair to a mutable set if the index is not already present.
 *
 * @param set   The OCMutableIndexPairSetRef instance.
 * @param index The index (key) to add.
 * @param value The value associated with index.
 * @return true on success; false if index already exists or on allocation failure.
 * @ingroup OCIndexPairSet
 */
bool OCIndexPairSetAddIndexPair(OCMutableIndexPairSetRef set, OCIndex index, OCIndex value);

/**
 * @brief Removes the pair with the given index from a mutable set.
 *
 * @param set   The OCMutableIndexPairSetRef instance.
 * @param index The index (key) whose pair will be removed.
 * @return true if the pair was found and removed; false otherwise.
 * @ingroup OCIndexPairSet
 */
bool OCIndexPairSetRemoveIndexPairWithIndex(OCMutableIndexPairSetRef set, OCIndex index);

/**
 * @brief Serializes the set into an OCDictionary suitable for plist.
 *
 * @param set The OCIndexPairSetRef instance.
 * @return A new OCDictionaryRef representing the set, or NULL on error.
 * @ingroup OCIndexPairSet
 */
OCDictionaryRef OCIndexPairSetCreatePList(OCIndexPairSetRef set);

/**
 * @brief Creates an OCIndexPairSet from a plist-compatible OCDictionary.
 *
 * @param dict An OCDictionaryRef previously created by OCIndexPairSetCreatePList().
 * @return A new OCIndexPairSetRef reconstructed from dict, or NULL on error.
 * @ingroup OCIndexPairSet
 */
OCIndexPairSetRef OCIndexPairSetCreateWithPList(OCDictionaryRef dict);

/**
 * @brief Returns an OCDataRef representing the internal contiguous OCIndexPair buffer.
 *
 * @param set The OCIndexPairSetRef instance.
 * @return A new OCDataRef containing the raw pair buffer; caller must release.
 * @ingroup OCIndexPairSet
 */
OCDataRef OCIndexPairSetCreateData(OCIndexPairSetRef set);

/**
 * @brief Creates an OCIndexPairSet using an existing OCDataRef as backing storage.
 *
 * @param data An OCDataRef containing a sequence of OCIndexPair structures.
 * @return A new OCIndexPairSetRef using the provided data, or NULL on error.
 * @ingroup OCIndexPairSet
 */
OCIndexPairSetRef OCIndexPairSetCreateWithData(OCDataRef data);

/**
 * @brief Creates a JSON array representation of an OCIndexPairSet.
 *
 * Each pair in the set is serialized as a 2-element JSON array: `[index, value]`.
 *
 * @param set An OCIndexPairSetRef to serialize.
 * @return A new cJSON array on success, or cJSON null on failure.
 *         Caller is responsible for managing the returned cJSON object.
 * @ingroup OCIndexPairSet
 */
cJSON *OCIndexPairSetCreateJSON(OCIndexPairSetRef set);

/**
 * @brief Creates an OCIndexPairSet from a JSON array of index-value pairs.
 *
 * Each item in the JSON array must be a 2-element array: `[index, value]`.
 *
 * @param json A cJSON array of index pairs.
 * @return A new OCIndexPairSetRef on success, or NULL on failure.
 *         Caller is responsible for releasing the returned set.
 * @ingroup OCIndexPairSet
 */
OCIndexPairSetRef OCIndexPairSetCreateFromJSON(cJSON *json);

/**
 * @brief Logs the contents of the index-pair set to stderr for debugging.
 *
 * @param set The OCIndexPairSetRef instance.
 * @ingroup OCIndexPairSet
 */
void OCIndexPairSetShow(OCIndexPairSetRef set);

/** @} */ // end of OCIndexPairSet group

#ifdef __cplusplus
}
#endif

#endif /* OCINDEXPAIRSET_H */
