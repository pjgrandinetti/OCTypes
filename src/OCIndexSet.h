/**
 * @file OCIndexSet.h
 * @brief Declares OCIndexSet and OCMutableIndexSet interfaces.
 *
 * OCIndexSet provides immutable and mutable sets of OCIndex values, stored as
 * sorted arrays. Supports single-index creation, range-based initialization,
 * membership queries, and serialization to OCData or plist-compatible dictionaries.
 */
#ifndef OCINDEXSET_H
#define OCINDEXSET_H
#include <stdbool.h>
#include "OCLibrary.h"
/**
 * @defgroup OCIndexSet OCIndexSet
 * @brief APIs for immutable and mutable collections of OCIndex values.
 *
 * This group includes functions to create, query, modify, and serialize
 * sets of OCIndex. Underlying storage is a sorted OCIndex array, and
 * serialization integrates with OCData and OCDictionary for plist support.
 * @{
 */
OCTypeID OCIndexSetGetTypeID(void);
/**
 * @brief Creates an empty, immutable OCIndexSet.
 *
 * @return A new OCIndexSetRef with no indices, or NULL on allocation failure.
 * @ingroup OCIndexSet
 */
OCIndexSetRef OCIndexSetCreate(void);
/**
 * @brief Creates an empty, mutable OCIndexSet.
 *
 * @return A new OCMutableIndexSetRef with no indices, or NULL on allocation failure.
 * @ingroup OCIndexSet
 */
OCMutableIndexSetRef OCIndexSetCreateMutable(void);
/**
 * @brief Creates an immutable copy of the provided index set.
 *
 * @param theIndexSet The source OCIndexSetRef to copy.
 * @return A new OCIndexSetRef that is a deep copy, or NULL if the source is NULL or on error.
 * @ingroup OCIndexSet
 */
OCIndexSetRef OCIndexSetCreateCopy(OCIndexSetRef theIndexSet);
/**
 * @brief Creates a mutable copy of the provided index set.
 *
 * @param theIndexSet The source OCIndexSetRef to copy.
 * @return A new OCMutableIndexSetRef that is a deep copy, or NULL if the source is NULL or on error.
 * @ingroup OCIndexSet
 */
OCMutableIndexSetRef OCIndexSetCreateMutableCopy(OCIndexSetRef theIndexSet);
/**
 * @brief Creates an OCIndexSet containing exactly one index.
 *
 * @param index The single OCIndex to include.
 * @return A new OCIndexSetRef with that index, or NULL on allocation failure.
 * @ingroup OCIndexSet
 */
OCIndexSetRef OCIndexSetCreateWithIndex(OCIndex index);
/**
 * @brief Creates an OCIndexSet containing all indices in [location, location + length).
 *
 * @param location The starting OCIndex (inclusive).
 * @param length   The number of consecutive indices.
 * @return A new OCIndexSetRef with indices location through (location + length - 1),
 *         or NULL on error (e.g., invalid range or allocation failure).
 * @ingroup OCIndexSet
 */
OCIndexSetRef OCIndexSetCreateWithIndexesInRange(OCIndex location, OCIndex length);
/**
 * @brief Retrieves the underlying OCData buffer holding sorted indices.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @return A new OCDataRef containing the contiguous OCIndex array; caller must release.
 *         Returns NULL if theIndexSet is NULL.
 * @ingroup OCIndexSet
 */
OCDataRef OCIndexSetGetIndexes(OCIndexSetRef theIndexSet);
/**
 * @brief Returns a pointer to the internal OCIndex array.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @return A pointer to the sorted OCIndex array for direct access. Do not modify
 *         if the set is immutable. Returns NULL if theIndexSet is NULL.
 * @ingroup OCIndexSet
 */
OCIndex *OCIndexSetGetBytesPtr(OCIndexSetRef theIndexSet);
/**
 * @brief Returns the number of indices in the set.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @return The count of OCIndex values, or 0 if theIndexSet is NULL.
 * @ingroup OCIndexSet
 */
OCIndex OCIndexSetGetCount(OCIndexSetRef theIndexSet);
/**
 * @brief Returns the smallest (first) index in the set.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @return The first OCIndex, or kOCNotFound if the set is empty or NULL.
 * @ingroup OCIndexSet
 */
OCIndex OCIndexSetFirstIndex(OCIndexSetRef theIndexSet);
/**
 * @brief Returns the largest (last) index in the set.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @return The last OCIndex, or kOCNotFound if the set is empty or NULL.
 * @ingroup OCIndexSet
 */
OCIndex OCIndexSetLastIndex(OCIndexSetRef theIndexSet);
/**
 * @brief Finds the largest index smaller than the given one.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @param index       The OCIndex to compare.
 * @return The greatest OCIndex < index, or kOCNotFound if none exists or on error.
 * @ingroup OCIndexSet
 */
OCIndex OCIndexSetIndexLessThanIndex(OCIndexSetRef theIndexSet, OCIndex index);
/**
 * @brief Finds the smallest index greater than the given one.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @param index       The OCIndex to compare.
 * @return The least OCIndex > index, or kOCNotFound if none exists or on error.
 * @ingroup OCIndexSet
 */
OCIndex OCIndexSetIndexGreaterThanIndex(OCIndexSetRef theIndexSet, OCIndex index);
/**
 * @brief Checks if the set contains a given index.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @param index       The OCIndex to check.
 * @return true if index is present; false otherwise or if theIndexSet is NULL.
 * @ingroup OCIndexSet
 */
bool OCIndexSetContainsIndex(OCIndexSetRef theIndexSet, OCIndex index);
/**
 * @brief Inserts a new index into the mutable set.
 *
 * @param theIndexSet The OCMutableIndexSetRef instance.
 * @param index       The OCIndex to add.
 * @return true if inserted (or already present); false on allocation failure or if theIndexSet is NULL.
 * @ingroup OCIndexSet
 */
bool OCIndexSetAddIndex(OCMutableIndexSetRef theIndexSet, OCIndex index);
/**
 * @brief Compares two index sets for equality.
 *
 * @param input1 The first OCIndexSetRef.
 * @param input2 The second OCIndexSetRef.
 * @return true if both sets contain the same indices; false otherwise or if either is NULL.
 * @ingroup OCIndexSet
 */
bool OCIndexSetEqual(OCIndexSetRef input1, OCIndexSetRef input2);
/**
 * @brief Converts the index set into an OCArray of OCNumber objects.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @return A new OCArrayRef whose elements are OCNumberRef wrapping each index;
 *         caller must release. Returns NULL on error.
 * @ingroup OCIndexSet
 */
OCArrayRef OCIndexSetCreateOCNumberArray(OCIndexSetRef theIndexSet);
/**
 * @brief Serializes the set to an OCDictionary for plist compatibility.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @return A new OCDictionaryRef representing the index set, or NULL on error.
 * @ingroup OCIndexSet
 */
OCDictionaryRef OCIndexSetCreateDictionary(OCIndexSetRef theIndexSet);
/**
 * @brief Reconstructs an OCIndexSet from a plist-compatible OCDictionary.
 *
 * @param dictionary An OCDictionaryRef previously returned by OCIndexSetCreateDictionary().
 * @return A new OCIndexSetRef populated from dictionary, or NULL on error.
 * @ingroup OCIndexSet
 */
OCIndexSetRef OCIndexSetCreateFromDictionary(OCDictionaryRef dictionary);
/**
 * @brief Creates an OCData snapshot of the current set.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @return A new OCDataRef containing the sorted OCIndex array; caller must release.
 *         Returns NULL if theIndexSet is NULL or on error.
 * @ingroup OCIndexSet
 */
OCDataRef OCIndexSetCreateData(OCIndexSetRef theIndexSet);
/**
 * @brief Creates an OCIndexSet using the bytes from an existing OCData object.
 *
 * @param data An OCDataRef containing a contiguous OCIndex array.
 * @return A new OCIndexSetRef using data as backing storage, or NULL on error.
 * @ingroup OCIndexSet
 */
OCIndexSetRef OCIndexSetCreateWithData(OCDataRef data);
/**
 * @brief Creates an OCIndexSet from a JSON array.
 *
 * This function expects a cJSON array node containing numeric values.
 * Each number is cast to an OCIndex and added to the resulting OCIndexSet.
 *
 * @param json A cJSON array node containing numeric values.
 * @return A new OCIndexSetRef on success, or NULL on failure.
 *         The caller is responsible for releasing the returned OCIndexSet.
 * @ingroup OCIndexSet
 */
OCIndexSetRef OCIndexSetCreateFromJSON(cJSON *json);
/**
 * @brief Creates a JSON array representation of an OCIndexSet.
 *
 * Each element of the OCIndexSet is serialized as a numeric value in the
 * resulting cJSON array. This allows for straightforward round-trip
 * conversion when used with OCIndexSetCreateFromJSON().
 *
 * @param set An OCIndexSetRef to serialize. Must not be NULL.
 * @return A new cJSON array containing numeric values, or cJSON null on failure.
 *         The caller is responsible for managing the returned cJSON object.
 * @ingroup OCIndexSet
 */
cJSON *OCIndexSetCreateJSON(OCIndexSetRef set);
/**
 * @brief Logs the contents of the set to stderr for debugging.
 *
 * @param theIndexSet The OCIndexSetRef instance.
 * @ingroup OCIndexSet
 */
void OCIndexSetShow(OCIndexSetRef theIndexSet);
/** @} */  // end of OCIndexSet group
#ifdef __cplusplus
}
#endif
#endif /* OCINDEXSET_H */
