/**
 * @file OCDictionary.h
 * @brief Key-value collection types for the OCTypes framework.
 *
 * This header defines the OCDictionaryRef and OCMutableDictionaryRef types
 * and associated APIs for managing collections of uniquely keyed values.
 */

#ifndef OCDICTIONARY_H
#define OCDICTIONARY_H

#include <stdint.h>
#include <stdbool.h>

#include "OCArray.h"
#include "OCLibrary.h"

/**
 * @defgroup OCDictionary OCDictionary
 * @brief Dictionary types and operations.
 * @{
 */

/**
 * @brief An opaque reference to an immutable dictionary object.
 * @ingroup OCDictionary
 */
typedef const struct __OCDictionary *OCDictionaryRef;

/**
 * @brief An opaque reference to a mutable dictionary object.
 * @ingroup OCDictionary
 */
typedef struct __OCDictionary *OCMutableDictionaryRef;

/**
 * @brief Returns the OCTypeID for OCDictionary objects.
 * @return Type identifier for OCDictionary.
 * @ingroup OCDictionary
 */
OCTypeID OCDictionaryGetTypeID(void);

/**
 * @brief Creates a new mutable dictionary with initial capacity.
 *
 * @param capacity Number of key-value pairs to allocate space for initially.
 * @return New OCMutableDictionaryRef or NULL on failure.
 * @ingroup OCDictionary
 */
OCMutableDictionaryRef OCDictionaryCreateMutable(uint64_t capacity);

/**
 * @brief Creates an immutable copy of a dictionary.
 *
 * @param theDictionary Dictionary to copy.
 * @return New OCDictionaryRef, or NULL on failure.
 * @ingroup OCDictionary
 */
OCDictionaryRef OCDictionaryCreateCopy(OCDictionaryRef theDictionary);

/**
 * @brief Creates a mutable copy of a dictionary.
 *
 * @param theDictionary Dictionary to copy.
 * @return New OCMutableDictionaryRef, or NULL on failure.
 * @ingroup OCDictionary
 */
OCMutableDictionaryRef OCDictionaryCreateMutableCopy(OCDictionaryRef theDictionary);

/**
 * @brief Gets the number of key-value pairs in a dictionary.
 *
 * @param theDictionary Dictionary to query.
 * @return Count of entries.
 * @ingroup OCDictionary
 */
uint64_t OCDictionaryGetCount(OCDictionaryRef theDictionary);

/**
 * @brief Retrieves the value for a specific key.
 *
 * @param theDictionary Dictionary to search.
 * @param key Key to look up.
 * @return Pointer to value, or NULL if not found.
 * @ingroup OCDictionary
 */
const void *OCDictionaryGetValue(OCDictionaryRef theDictionary, OCStringRef key);

/**
 * @brief Checks if the dictionary contains the specified key.
 *
 * @param theDictionary Dictionary to search.
 * @param key Key to test.
 * @return true if key is found, false otherwise.
 * @ingroup OCDictionary
 */
bool OCDictionaryContainsKey(OCDictionaryRef theDictionary, OCStringRef key);

/**
 * @brief Checks if the dictionary contains the specified value.
 *
 * @param theDictionary Dictionary to search.
 * @param value Value to look for.
 * @return true if value is found, false otherwise.
 * @ingroup OCDictionary
 */
bool OCDictionaryContainsValue(OCDictionaryRef theDictionary, const void *value);

/**
 * @brief Adds or replaces a key-value pair in a mutable dictionary.
 *
 * @param theDictionary Dictionary to modify.
 * @param key Key to add or update.
 * @param value Value to associate with the key.
 * @ingroup OCDictionary
 */
void OCDictionaryAddValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void *value);

/**
 * @brief Sets the value for a key (alias of OCDictionaryAddValue).
 *
 * @param theDictionary Dictionary to modify.
 * @param key Key to set.
 * @param value Value to assign.
 * @ingroup OCDictionary
 */
void OCDictionarySetValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void *value);

/**
 * @brief Replaces the value for an existing key.
 *
 * @param theDictionary Dictionary to modify.
 * @param key Key whose value is to be replaced.
 * @param value New value to assign.
 * @ingroup OCDictionary
 */
void OCDictionaryReplaceValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void *value);

/**
 * @brief Removes a key-value pair from the dictionary.
 *
 * @param theDictionary Dictionary to modify.
 * @param key Key to remove.
 * @ingroup OCDictionary
 */
void OCDictionaryRemoveValue(OCMutableDictionaryRef theDictionary, OCStringRef key);

/**
 * @brief Counts how many times a value appears in the dictionary.
 *
 * @param theDictionary Dictionary to search.
 * @param value Value to count.
 * @return Number of occurrences.
 * @ingroup OCDictionary
 */
uint64_t OCDictionaryGetCountOfValue(OCMutableDictionaryRef theDictionary, const void *value);

/**
 * @brief Retrieves all keys and values into parallel arrays.
 *
 * @param theDictionary Dictionary to query.
 * @param keys Output array of keys.
 * @param values Output array of values.
 * @ingroup OCDictionary
 */
void OCDictionaryGetKeysAndValues(OCDictionaryRef theDictionary, const void **keys, const void **values);

/**
 * @brief Creates an array containing all keys in the dictionary.
 *
 * @param theDictionary Dictionary to query.
 * @return New OCArrayRef of keys, or NULL.
 * @ingroup OCDictionary
 */
OCArrayRef OCDictionaryCreateArrayWithAllKeys(OCDictionaryRef theDictionary);

/**
 * @brief Creates an array containing all values in the dictionary.
 *
 * @param theDictionary Dictionary to query.
 * @return New OCArrayRef of values, or NULL.
 * @ingroup OCDictionary
 */
OCArrayRef OCDictionaryCreateArrayWithAllValues(OCDictionaryRef theDictionary);

/** @} */ // end of OCDictionary group

#endif /* OCDICTIONARY_H */
