/**
 * @file OCDictionary.h
 * @brief Key-value collection types for the OCTypes framework.
 *
 * This header defines the OCDictionaryRef and OCMutableDictionaryRef types
 * and associated APIs for managing collections of uniquely keyed values.
 *
 * @note Ownership follows CoreFoundation conventions:
 *       The caller owns any OCDictionaryRef or OCMutableDictionaryRef returned
 *       by functions with "Create" or "Copy" in the name, and must call OCRelease().
 */
#ifndef OCDICTIONARY_H
#define OCDICTIONARY_H
#include <stdbool.h>
#include <stdint.h>
#include "OCArray.h"
#include "OCLibrary.h"  // Ensures OCDictionaryRef and other types are available
/**
 * @defgroup OCDictionary OCDictionary
 * @brief Dictionary types and operations.
 * @{
 */
/**
 * @brief Returns the OCTypeID for OCDictionary objects.
 * @return Type identifier for OCDictionary.
 * @ingroup OCDictionary
 */
OCTypeID OCDictionaryGetTypeID(void);
/**
 * @brief Creates a new dictionary with initial capacity.
 *
 * @param keys Array of keys.
 * @param values Array of values.
 * @param numValues Number of key-value pairs.
 * @return New OCDictionaryRef or NULL on failure.
 * @ingroup OCDictionary
 */
OCDictionaryRef OCDictionaryCreate(const void **keys, const void **values, uint64_t numValues);
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
 * @return true on success, false on failure.
 * @ingroup OCDictionary
 */
bool OCDictionaryAddValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void *value);
/**
 * @brief Sets the value for a key.
 *
 * If the key does not exist, this function inserts it.
 * Equivalent to OCDictionaryAddValue().
 *
 * @param theDictionary Dictionary to modify.
 * @param key Key to set.
 * @param value Value to assign.
 * @return true on success (inserted or updated), false on failure.
 * @ingroup OCDictionary
 */
bool OCDictionarySetValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void *value);
/**
 * @brief OCDictionaryReplaceValue replaces the value for an existing key.
 *
 * Does nothing if the key is not present.
 *
 * @param theDictionary Dictionary to modify.
 * @param key Key whose value is to be replaced.
 * @param value New value to assign.
 * @return true if the key existed and was replaced, false otherwise.
 * @ingroup OCDictionary
 */
bool OCDictionaryReplaceValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void *value);
/**
 * @brief Removes a key-value pair from the dictionary.
 *
 * @param theDictionary Dictionary to modify.
 * @param key Key to remove.
 * @return true if the key was found and removed, false if not found.
 * @ingroup OCDictionary
 */
bool OCDictionaryRemoveValue(OCMutableDictionaryRef theDictionary, OCStringRef key);
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
 * @param keys Output array of keys (must have space for at least count entries).
 * @param values Output array of values (must have space for at least count entries).
 * @return true if keys and values were successfully written, false on error.
 * @ingroup OCDictionary
 */
bool OCDictionaryGetKeysAndValues(OCDictionaryRef theDictionary, const void **keys, const void **values);
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
/**
 * @brief Returns a human-readable description of the dictionary.
 *
 * @param cf Dictionary to describe.
 * @return A formatted OCStringRef (caller must release).
 * @ingroup OCDictionary
 */
OCStringRef OCDictionaryCopyFormattingDesc(OCTypeRef cf);
/**
 * @brief Creates a JSON object representation of an OCDictionary.
 *
 * Each key is serialized using its OCString value. Each value is serialized
 * using its registered JSON serialization callback if available, or falls back
 * to a string representation.
 *
 * @param dict An OCDictionaryRef to serialize.
 * @return A new cJSON object on success, or cJSON null on failure.
 *         Caller is responsible for managing the returned cJSON object.
 * @ingroup OCDictionary
 */
cJSON *OCDictionaryCreateJSON(OCDictionaryRef dict);
/** @} */
#endif /* OCDICTIONARY_H */
