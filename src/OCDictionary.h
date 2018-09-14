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

OCTypeID OCDictionaryGetTypeID(void);

OCMutableDictionaryRef OCDictionaryCreateMutable(uint64_t capacity);
OCDictionaryRef OCDictionaryCreateCopy(OCDictionaryRef theDictionary);
OCMutableDictionaryRef OCDictionaryCreateMutableCopy(OCDictionaryRef theDictionary);
uint64_t OCDictionaryGetCount(OCDictionaryRef theDictionary);

const void * OCDictionaryGetValue(OCDictionaryRef theDictionary, OCStringRef key);
bool OCDictionaryContainsKey(OCDictionaryRef theDictionary, OCStringRef key);
bool OCDictionaryContainsValue(OCDictionaryRef theDictionary, const void * value);
void OCDictionaryAddValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value);
void OCDictionarySetValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value);
void OCDictionaryReplaceValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value);
void OCDictionaryRemoveValue(OCMutableDictionaryRef theDictionary, OCStringRef key);
uint64_t OCDictionaryGetCountOfValue(OCMutableDictionaryRef theDictionary, const void * value);
void OCDictionaryGetKeysAndValues(OCDictionaryRef theDictionary, const void **keys, const void **values);

OCArrayRef OCDictionaryCreateArrayWithAllKeys(OCDictionaryRef theDictionary);
OCArrayRef OCDictionaryCreateArrayWithAllValues(OCDictionaryRef theDictionary);

#endif /* OCDictionary_h */
