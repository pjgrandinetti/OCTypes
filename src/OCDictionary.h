//
//  OCDictionary.h
//  OCTypes
//
//  Created by philip on 5/10/17.
//  Updated by GitHub Copilot on 5/10/25.
//

#ifndef OCDICTIONARY_H
#define OCDICTIONARY_H

#include <stdint.h>
#include <stdbool.h>

#include "OCArray.h"
#include "OCLibrary.h"

/**
 * @file OCDictionary.h
 * @brief Defines the OCDictionaryRef and OCMutableDictionaryRef types and related functions
 *        for managing collections of key-value pairs.
 *
 * Dictionaries store associations between keys (OCStringRef) and values (generic pointers).
 * Keys are unique. Values can be any OCType object or other data.
 * The dictionary retains its keys and values according to OCType memory management rules
 * if they are OCType objects. For non-OCType values, the client is responsible for
 * managing their lifecycle.
 */

/**
 * @typedef OCDictionaryRef
 * @brief An opaque reference to an immutable dictionary object.
 *
 * OCDictionaryRef represents a dictionary whose contents cannot be changed after creation.
 * Operations on an OCDictionaryRef are read-only.
 */
typedef const struct __OCDictionary * OCDictionaryRef;

/**
 * @typedef OCMutableDictionaryRef
 * @brief An opaque reference to a mutable dictionary object.
 *
 * OCMutableDictionaryRef represents a dictionary whose contents can be modified
 * by adding, removing, or replacing key-value pairs.
 */
typedef struct __OCDictionary * OCMutableDictionaryRef;

/**
 * @brief Returns the unique type identifier for the OCDictionary class.
 * @return The OCTypeID associated with OCDictionary objects. This ID is used to identify
 *         OCDictionary instances within the OCTypes type system.
 */
OCTypeID OCDictionaryGetTypeID(void);

/**
 * @brief Creates a new, empty, mutable dictionary with a specified initial capacity.
 * @param capacity The initial number of key-value pairs the dictionary can store without
 *                 needing to resize its internal storage. A capacity of 0 is valid.
 * @return A new OCMutableDictionaryRef object, or NULL if creation fails (e.g., memory allocation failure).
 *         The caller is responsible for releasing the returned object using OCRelease.
 */
OCMutableDictionaryRef OCDictionaryCreateMutable(uint64_t capacity);

/**
 * @brief Creates a new immutable dictionary by copying an existing dictionary.
 * @param theDictionary The OCDictionaryRef to copy. If NULL, the behavior is undefined or may result in NULL.
 * @return A new OCDictionaryRef object that is an immutable copy of `theDictionary`,
 *         or NULL if `theDictionary` is NULL or if creation fails.
 *         The caller is responsible for releasing the returned object using OCRelease.
 *         The copy is a shallow copy of the keys and values; they are retained, not deeply copied.
 */
OCDictionaryRef OCDictionaryCreateCopy(OCDictionaryRef theDictionary);

/**
 * @brief Creates a new mutable dictionary by copying an existing dictionary.
 * @param theDictionary The OCDictionaryRef to copy. If NULL, the behavior is undefined or may result in NULL.
 * @return A new OCMutableDictionaryRef object that is a mutable copy of `theDictionary`,
 *         or NULL if `theDictionary` is NULL or if creation fails.
 *         The caller is responsible for releasing the returned object using OCRelease.
 *         The copy is a shallow copy of the keys and values; they are retained, not deeply copied.
 */
OCMutableDictionaryRef OCDictionaryCreateMutableCopy(OCDictionaryRef theDictionary);

/**
 * @brief Returns the number of key-value pairs currently stored in the dictionary.
 * @param theDictionary The OCDictionaryRef to query. Must not be NULL.
 * @return The count of key-value pairs in the dictionary. Returns 0 if `theDictionary` is NULL or empty.
 */
uint64_t OCDictionaryGetCount(OCDictionaryRef theDictionary);

/**
 * @brief Retrieves the value associated with a specific key in the dictionary.
 * @param theDictionary The OCDictionaryRef to query. Must not be NULL.
 * @param key The OCStringRef key whose associated value is to be retrieved. Must not be NULL.
 * @return A pointer to the value associated with `key`, or NULL if the key is not found
 *         or if `theDictionary` or `key` is NULL. The returned value is a borrowed reference
 *         and should not be released by the caller unless explicitly retained.
 */
const void * OCDictionaryGetValue(OCDictionaryRef theDictionary, OCStringRef key);

/**
 * @brief Checks if the dictionary contains a key-value pair for the specified key.
 * @param theDictionary The OCDictionaryRef to query. Must not be NULL.
 * @param key The OCStringRef key to check for. Must not be NULL.
 * @return `true` if the dictionary contains the key, `false` otherwise or if `theDictionary` or `key` is NULL.
 */
bool OCDictionaryContainsKey(OCDictionaryRef theDictionary, OCStringRef key);

/**
 * @brief Checks if the dictionary contains the specified value.
 * @details This operation can be expensive as it may require iterating through all values
 *          in the dictionary and comparing them. The comparison is done by pointer equality
 *          unless the dictionary is configured with custom callbacks that define value equality.
 * @param theDictionary The OCDictionaryRef to query. Must not be NULL.
 * @param value The value to search for.
 * @return `true` if the dictionary contains one or more instances of the value, `false` otherwise
 *         or if `theDictionary` is NULL.
 */
bool OCDictionaryContainsValue(OCDictionaryRef theDictionary, const void * value);

/**
 * @brief Adds a key-value pair to a mutable dictionary.
 * @details If the key already exists in the dictionary, its associated value is replaced with the new value.
 *          The dictionary retains the key and the value if they are OCType objects.
 * @param theDictionary The OCMutableDictionaryRef to modify. Must not be NULL.
 * @param key The OCStringRef key to add. Must not be NULL. The dictionary will retain this key.
 * @param value The value to associate with the key. The dictionary will retain this value if it's an OCType.
 */
void OCDictionaryAddValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value);

/**
 * @brief Sets or replaces the value for a given key in a mutable dictionary.
 * @details This function is an alias for OCDictionaryAddValue. If the key already exists,
 *          its value is replaced. If the key does not exist, a new key-value pair is added.
 *          The dictionary retains the key and the value if they are OCType objects.
 * @param theDictionary The OCMutableDictionaryRef to modify. Must not be NULL.
 * @param key The OCStringRef key. Must not be NULL. The dictionary will retain this key.
 * @param value The new value to set for the key. The dictionary will retain this value if it's an OCType.
 */
void OCDictionarySetValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value);

/**
 * @brief Replaces the value associated with a given key in a mutable dictionary.
 * @details If the key does not exist in the dictionary, this function does nothing.
 *          If the key exists, its current value is released (if it's an OCType) and
 *          replaced with the new value, which is then retained (if it's an OCType).
 * @param theDictionary The OCMutableDictionaryRef to modify. Must not be NULL.
 * @param key The OCStringRef key whose value is to be replaced. Must not be NULL.
 * @param value The new value to associate with the key. The dictionary will retain this value if it's an OCType.
 */
void OCDictionaryReplaceValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value);

/**
 * @brief Removes a key-value pair from a mutable dictionary, identified by the key.
 * @details If the key is found, the key and its associated value are removed.
 *          The key and value are released if they are OCType objects.
 *          If the key is not found, the dictionary remains unchanged.
 * @param theDictionary The OCMutableDictionaryRef to modify. Must not be NULL.
 * @param key The OCStringRef key of the entry to remove. Must not be NULL.
 */
void OCDictionaryRemoveValue(OCMutableDictionaryRef theDictionary, OCStringRef key);

/**
 * @brief Counts the number of times a specific value appears in the dictionary.
 * @details This operation can be expensive as it requires iterating through all values.
 *          Comparison is by pointer equality unless custom callbacks are used.
 * @param theDictionary The OCMutableDictionaryRef (or OCDictionaryRef, as it's a read operation) to query. Must not be NULL.
 * @param value The value to count.
 * @return The number of key-value pairs in the dictionary that have the specified value.
 *         Returns 0 if `theDictionary` is NULL or the value is not found.
 */
uint64_t OCDictionaryGetCountOfValue(OCMutableDictionaryRef theDictionary, const void * value);

/**
 * @brief Retrieves all keys and their corresponding values from the dictionary into two parallel C arrays.
 * @details The `keys` and `values` arrays must be pre-allocated by the caller and be large enough
 *          to hold all entries from the dictionary (i.e., size of OCDictionaryGetCount()).
 *          The order of keys and values in the output arrays is not guaranteed.
 *          The retrieved keys and values are borrowed references.
 * @param theDictionary The OCDictionaryRef to query. Must not be NULL.
 * @param keys A C array (pointer to const void *) to be filled with pointers to the keys. Must not be NULL.
 * @param values A C array (pointer to const void *) to be filled with pointers to the values. Must not be NULL.
 */
void OCDictionaryGetKeysAndValues(OCDictionaryRef theDictionary, const void **keys, const void **values);

/**
 * @brief Creates a new OCArrayRef containing all the keys from the dictionary.
 * @details The order of keys in the array is not guaranteed.
 *          The keys in the array are retained by the array.
 * @param theDictionary The OCDictionaryRef to query. Must not be NULL.
 * @return A new OCArrayRef containing all keys from the dictionary, or NULL if `theDictionary` is NULL
 *         or if creation fails. The caller is responsible for releasing the returned array.
 */
OCArrayRef OCDictionaryCreateArrayWithAllKeys(OCDictionaryRef theDictionary);

/**
 * @brief Creates a new OCArrayRef containing all the values from the dictionary.
 * @details The order of values in the array corresponds to the order of keys if
 *          OCDictionaryCreateArrayWithAllKeys were called simultaneously, but this order is not guaranteed.
 *          The values in the array are retained by the array.
 * @param theDictionary The OCDictionaryRef to query. Must not be NULL.
 * @return A new OCArrayRef containing all values from the dictionary, or NULL if `theDictionary` is NULL
 *         or if creation fails. The caller is responsible for releasing the returned array.
 */
OCArrayRef OCDictionaryCreateArrayWithAllValues(OCDictionaryRef theDictionary);

#endif /* OCDICTIONARY_H */
