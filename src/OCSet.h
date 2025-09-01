/**
 * @file OCSet.h
 * @brief Declares the OCSet and OCMutableSet interfaces.
 *
 * OCSet provides immutable and mutable unordered collections of unique
 * OCTypeRef values. Internally, OCSet uses OCArray to manage elements
 * and enforces uniqueness via OCTypeEqual().
 */ \
#ifndef OCSet_h
#define OCSet_h
#include "OCType.h"
#include "cJSON.h"
/**
 * @defgroup OCSet OCSet
 * @brief Set types and operations in OCTypes.
 * @{
 */
/**
 * @brief Gets the OCTypeID for OCSet.
 *
 * @return OCTypeID value.
 *
 * @ingroup OCSet
 */
OCTypeID OCSetGetTypeID(void);
/**
 * @brief Creates a new empty immutable set.
 *
 * @return A new OCSetRef instance or NULL on allocation failure.
 *
 * @ingroup OCSet
 */
OCSetRef OCSetCreate(void);
/**
 * @brief Creates a new empty mutable set.
 *
 * @param capacity Initial capacity hint (currently ignored).
 * @return A new OCMutableSetRef or NULL on allocation failure.
 *
 * @ingroup OCSet
 */
OCMutableSetRef OCSetCreateMutable(OCIndex capacity);
/**
 * @brief Creates an immutable copy of an existing set.
 *
 * @param theSet The set to copy.
 * @return A new OCSetRef or NULL on error.
 *
 * @ingroup OCSet
 */
OCSetRef OCSetCreateCopy(OCSetRef theSet);
/**
 * @brief Creates a mutable copy of an existing set.
 *
 * @param theSet The set to copy.
 * @return A new OCMutableSetRef or NULL on error.
 *
 * @ingroup OCSet
 */
OCMutableSetRef OCSetCreateMutableCopy(OCSetRef theSet);
/**
 * @brief Returns the number of elements in the set.
 *
 * @param theSet The set to query.
 * @return Element count, or 0 if NULL.
 *
 * @ingroup OCSet
 */
OCIndex OCSetGetCount(OCSetRef theSet);
/**
 * @brief Checks whether the set contains a given value.
 *
 * @param theSet The set to query.
 * @param value The value to check.
 * @return true if present, false otherwise.
 *
 * @ingroup OCSet
 */
bool OCSetContainsValue(OCSetRef theSet, OCTypeRef value);
/**
 * @brief Returns all values in the set as a new OCArray.
 *
 * @param theSet The source set.
 * @return A new OCArrayRef of set values, or NULL.
 *
 * @ingroup OCSet
 */
OCArrayRef OCSetCreateValueArray(OCSetRef theSet);
/**
 * @brief Adds a value to a mutable set.
 *
 * If the value is already present, the set remains unchanged.
 *
 * @param theSet The mutable set.
 * @param value The value to add.
 * @return true if added or already present, false on error.
 *
 * @ingroup OCSet
 */
bool OCSetAddValue(OCMutableSetRef theSet, OCTypeRef value);
/**
 * @brief Removes a value from a mutable set.
 *
 * If the value is not present, the set remains unchanged.
 *
 * @param theSet The mutable set.
 * @param value The value to remove.
 * @return true if removed, false otherwise.
 *
 * @ingroup OCSet
 */
bool OCSetRemoveValue(OCMutableSetRef theSet, OCTypeRef value);
/**
 * @brief Removes all values from a mutable set.
 *
 * @param theSet The mutable set.
 *
 * @ingroup OCSet
 */
void OCSetRemoveAllValues(OCMutableSetRef theSet);
/**
 * @brief Compares two sets for equality.
 *
 * @param set1 The first set.
 * @param set2 The second set.
 * @return true if both sets contain the same elements.
 *
 * @ingroup OCSet
 */
bool OCSetEqual(OCSetRef set1, OCSetRef set2);
/**
 * @brief Creates a JSON representation of an OCSet.
 *
 * For untyped serialization (typed=false), creates a JSON array where each element
 * is serialized using untyped serialization. Note that this loses type information
 * and the set will be indistinguishable from an array upon deserialization.
 *
 * For typed serialization (typed=true), creates a JSON object with "type": "OCSet"
 * and "value" array, where each element is serialized with type information.
 *
 * @param set An OCSetRef to serialize.
 * @param typed Whether to include type information in the serialization.
 * @return A new cJSON object/array on success, or cJSON null on failure.
 *         The caller is responsible for managing the returned cJSON object.
 * @ingroup OCSet
 */
cJSON *OCSetCopyAsJSON(OCSetRef set, bool typed);

/**
 * @brief Creates an OCSet from typed JSON representation.
 *
 * Deserializes a JSON object created by OCSetCreateJSONTyped back into an OCSet.
 * Uses the global type registry to deserialize each element.
 *
 * @param json A cJSON object with "type": "OCSet" and "value" array.
 * @return A new OCSetRef on success, or NULL on failure.
 * @ingroup OCSet
 */
OCSetRef OCSetCreateFromJSONTyped(cJSON *json);
/**
 * @brief Logs the contents of the set to stderr.
 *
 * @param theSet The set to display.
 *
 * @ingroup OCSet
 */
void OCSetShow(OCSetRef theSet);
/** @} */  // end of OCSet group
#endif     /* OCSet_h */
