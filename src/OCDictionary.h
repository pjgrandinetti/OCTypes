//
//  OCDictionary.h
//  OCTypes
//
//  Created by philip on 4/21/17.
//

#ifndef OCDictionary_h
#define OCDictionary_h

#include "OCLibrary.h"

typedef const struct __OCDictionary * OCDictionaryRef;
typedef struct __OCDictionary * OCMutableDictionaryRef;

/**
 * Returns the unique type identifier for OCDictionary.
 * @return The OCTypeID for OCDictionary.
 */
OCTypeID OCDictionaryGetTypeID(void);

/**
 * Creates a mutable dictionary with specified initial capacity.
 * @param capacity Initial number of key-value pairs.
 * @return A new OCMutableDictionaryRef.
 */
OCMutableDictionaryRef OCDictionaryCreateMutable(uint64_t capacity);

/**
 * Creates an immutable copy of an existing dictionary.
 * @param theDictionary The dictionary to copy.
 * @return A new OCDictionaryRef.
 */
OCDictionaryRef OCDictionaryCreateCopy(OCDictionaryRef theDictionary);

/**
 * Creates a mutable copy of an existing dictionary.
 * @param theDictionary The dictionary to copy.
 * @return A new OCMutableDictionaryRef.
 */
OCMutableDictionaryRef OCDictionaryCreateMutableCopy(OCDictionaryRef theDictionary);

/**
 * Returns the number of entries in the dictionary.
 * @param theDictionary The dictionary to query.
 * @return Count of key-value pairs.
 */
uint64_t OCDictionaryGetCount(OCDictionaryRef theDictionary);

/**
 * Retrieves the value associated with a given key.
 * @param theDictionary The dictionary to query.
 * @param key The key string.
 * @return The value pointer, or NULL if key is not present.
 */
const void * OCDictionaryGetValue(OCDictionaryRef theDictionary, OCStringRef key);

/**
 * Checks if the dictionary contains the specified key.
 * @param theDictionary The dictionary to query.
 * @param key The key to check.
 * @return true if key exists, false otherwise.
 */
bool OCDictionaryContainsKey(OCDictionaryRef theDictionary, OCStringRef key);

/**
 * Checks if the dictionary contains the specified value.
 * @param theDictionary The dictionary to query.
 * @param value The value to check.
 * @return true if value exists, false otherwise.
 */
bool OCDictionaryContainsValue(OCDictionaryRef theDictionary, const void * value);

/**
 * Adds a value for a key. Replaces existing value if key exists.
 * @param theDictionary The mutable dictionary to modify.
 * @param key The key string.
 * @param value The value to associate.
 */
void OCDictionaryAddValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value);

/**
 * Sets or replaces a value for a key.
 * @param theDictionary The mutable dictionary to modify.
 * @param key The key string.
 * @param value The new value to set.
 */
void OCDictionarySetValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value);

/**
 * Replaces the value for a given key.
 * @param theDictionary The mutable dictionary to modify.
 * @param key The key string.
 * @param value The replacement value.
 */
void OCDictionaryReplaceValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value);

/**
 * Removes the value for a given key.
 * @param theDictionary The mutable dictionary to modify.
 * @param key The key whose value to remove.
 */
void OCDictionaryRemoveValue(OCMutableDictionaryRef theDictionary, OCStringRef key);

/**
 * Counts occurrences of a value in the dictionary.
 * @param theDictionary The dictionary to query.
 * @param value The value to count.
 * @return Number of key-value pairs matching the value.
 */
uint64_t OCDictionaryGetCountOfValue(OCMutableDictionaryRef theDictionary, const void * value);

/**
 * Retrieves all keys and values into parallel arrays.
 * @param theDictionary The dictionary to query.
 * @param keys Pre-allocated array for keys.
 * @param values Pre-allocated array for values.
 */
void OCDictionaryGetKeysAndValues(OCDictionaryRef theDictionary, const void **keys, const void **values);

/**
 * Creates an array containing all keys of the dictionary.
 * @param theDictionary The dictionary to query.
 * @return An OCArrayRef of keys.
 */
OCArrayRef OCDictionaryCreateArrayWithAllKeys(OCDictionaryRef theDictionary);

/**
 * Creates an array containing all values of the dictionary.
 * @param theDictionary The dictionary to query.
 * @return An OCArrayRef of values.
 */
OCArrayRef OCDictionaryCreateArrayWithAllValues(OCDictionaryRef theDictionary);

#endif /* OCDictionary_h */
