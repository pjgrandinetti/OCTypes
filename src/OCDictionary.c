//
//  OCDictionary.c
//  OCTypes
//
//  Created by philip on 4/21/17.
//

#include <stdlib.h>   // for malloc, free, realloc
#include <string.h>   // for memcpy
#include "OCLibrary.h"

static OCTypeID kOCDictionaryID = _kOCNotATypeID;

// OCDictionary Opaque Type
struct __OCDictionary {
    OCBase _base;

    // OCDictionary Type attributes
    uint64_t count;
    uint64_t capacity;
    OCStringRef *keys;
    OCTypeRef *values;
};

bool __OCDictionaryEqual(const void * theType1, const void * theType2)
{
    OCDictionaryRef theDictionary1 = (OCDictionaryRef) theType1;
    OCDictionaryRef theDictionary2 = (OCDictionaryRef) theType2;
    if(theDictionary1->_base.typeID != theDictionary2->_base.typeID) return false;

    if(NULL == theDictionary1 || NULL == theDictionary2) return false;
    if(theDictionary1 == theDictionary2) return true;
    if(theDictionary1->count != theDictionary2->count) return false;
    
    // For Dictionaries to be equal the keys-values must have the same storage order
    // This doesn't allow dictionaries with the identical key-values but in different
    // storage to be equal.  Need to fix this.
    for(uint64_t index = 0; index < theDictionary1->count; index++) {
        OCStringRef key1 = theDictionary1->keys[index];
        OCStringRef key2 = theDictionary2->keys[index];
        if(!OCTypeEqual(key1, key2)) return false;
        OCTypeRef value1 = theDictionary1->values[index];
        OCTypeRef value2 = theDictionary2->values[index];
        if(!OCTypeEqual(value1, value2)) return false;
    }
    return true;
}

static void __OCDictionaryReleaseValues(OCDictionaryRef theDictionary)
{
    for(size_t index = 0;index< theDictionary->count; index++) {
        OCRelease(theDictionary->values[index]);
        OCRelease(theDictionary->keys[index]);
    }
}

static void __OCDictionaryRetainValues(OCDictionaryRef theDictionary)
{
    for(size_t index = 0; index < theDictionary->count; index++) {
        // Retain both values and keys for copied dictionaries
        OCRetain(theDictionary->values[index]);
        OCRetain(theDictionary->keys[index]);
    }
}

void __OCDictionaryFinalize(const void * theType)
{
    if(NULL == theType) return;
    OCDictionaryRef theDictionary = (OCDictionaryRef) theType;
    __OCDictionaryReleaseValues(theDictionary); // This correctly releases all keys and values

    // Free the arrays for keys and values, and the dictionary structure itself
    free((void *)theDictionary->keys);
    free((void *)theDictionary->values);
    free((void *)theDictionary); // Cast to (void*) to avoid qualifier discard warning
}

OCTypeID OCDictionaryGetTypeID(void)
{
    if(kOCDictionaryID == _kOCNotATypeID) kOCDictionaryID = OCRegisterType("OCDictionary");
    return kOCDictionaryID;
}

static struct __OCDictionary *OCDictionaryAllocate()
{
    struct __OCDictionary *theDictionary = malloc(sizeof(struct __OCDictionary));
    if(NULL == theDictionary) return NULL;
    theDictionary->_base.typeID = OCDictionaryGetTypeID();
    theDictionary->_base.static_instance = false; // Not static
    theDictionary->_base.finalize = __OCDictionaryFinalize;
    theDictionary->_base.equal = __OCDictionaryEqual;
    theDictionary->_base.copyFormattingDesc = NULL;
    theDictionary->_base.retainCount =0;
    theDictionary->count = 0;
    theDictionary->capacity = 0;
    theDictionary->keys = NULL; // Initialize keys pointer
    theDictionary->values = NULL; // Initialize values pointer
    OCRetain(theDictionary); // Increment retain count for the new object
    return theDictionary;
}


uint64_t OCDictionaryGetCount(OCDictionaryRef theDictionary)
{
    if(NULL == theDictionary) return 0;
    return theDictionary->count;
}


OCDictionaryRef OCDictionaryCreate(const void **keys, const void **values, uint64_t numValues)
{
    if(NULL==values) return NULL;
    struct __OCDictionary *theDictionary = OCDictionaryAllocate();
    if(NULL == theDictionary) return NULL;
    theDictionary->values = (OCTypeRef *) malloc(numValues * sizeof(OCTypeRef));
    memcpy((void *) theDictionary->values, (const void *) values, numValues * sizeof(void *));
    theDictionary->keys = (OCStringRef*) malloc(numValues * sizeof(OCStringRef));
    memcpy((char *) theDictionary->keys, (const char *) keys, numValues * sizeof(char *));
    theDictionary->count = numValues;
    theDictionary->capacity = numValues;
    __OCDictionaryRetainValues(theDictionary);
    
    return theDictionary;
}

OCMutableDictionaryRef OCDictionaryCreateMutable(uint64_t capacity)
{
    struct __OCDictionary *theDictionary = OCDictionaryAllocate();
    if(NULL == theDictionary) return NULL;
    theDictionary->values = (OCTypeRef *) malloc(capacity * sizeof(OCTypeRef));
    theDictionary->keys = (OCStringRef *) malloc(capacity * sizeof(OCStringRef));
    theDictionary->count = 0;
    theDictionary->capacity = capacity;
    return theDictionary;
}

OCDictionaryRef OCDictionaryCreateCopy(OCDictionaryRef theDictionary)
{
    return (OCDictionaryRef) OCDictionaryCreate((const void **) theDictionary->keys,(const void **) theDictionary->values,theDictionary->count);
}

OCMutableDictionaryRef OCDictionaryCreateMutableCopy(OCDictionaryRef theDictionary)
{
    return (OCMutableDictionaryRef) OCDictionaryCreate((const void **) theDictionary->keys,(const void **) theDictionary->values,theDictionary->count);
}

const void * OCDictionaryGetValue(OCDictionaryRef theDictionary, OCStringRef key)
{
    for(long index=0;index<theDictionary->count;index++) {
        if(OCStringCompare(theDictionary->keys[index], key, 0)==kOCCompareEqualTo) return theDictionary->values[index];
    }
    return NULL;
}

bool OCDictionaryContainsKey(OCDictionaryRef theDictionary, OCStringRef key)
{
    for(long index=0;index<theDictionary->count;index++) {
        if(OCStringCompare(theDictionary->keys[index], key, 0)==kOCCompareEqualTo) return true;
    }
    return false;
}

int64_t OCDictionaryIndexOfKey(OCDictionaryRef theDictionary, OCStringRef key)
{
    for(int64_t index=0;index<theDictionary->count;index++) {
        if(OCStringCompare(theDictionary->keys[index], key, 0)==kOCCompareEqualTo) return index;
    }
    return -1;
}

bool OCDictionaryContainsValue(OCDictionaryRef theDictionary, const void * value)
{
    for(long index=0;index<theDictionary->count;index++) {
        if(theDictionary->values[index] == value) return true;
    }
    return false;
}

void OCDictionaryAddValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value)
{
    if(NULL==theDictionary) return;
    if(NULL==key) return;
    if(NULL==value) return;
    
    // Check if the key already exists
    int64_t existingKeyIndex = OCDictionaryIndexOfKey(theDictionary, key);
    if (existingKeyIndex >= 0) {
        // Key exists, replace the value
        OCRelease(theDictionary->values[existingKeyIndex]);
        OCTypeRef type = (OCTypeRef)value;
        theDictionary->values[existingKeyIndex] = type;
        OCRetain(type);
        return;
    }
    
    // Key doesn't exist, add a new entry
    OCTypeRef type = (OCTypeRef) value;
    if(theDictionary->capacity==0 || theDictionary->count==theDictionary->capacity) {
        if(NULL==theDictionary->values) theDictionary->values = (OCTypeRef*) malloc(sizeof(OCTypeRef));
        else theDictionary->values = (OCTypeRef*) realloc(theDictionary->values, (theDictionary->count+1) * sizeof(OCTypeRef));
        
        if(NULL==theDictionary->keys) theDictionary->keys = (OCStringRef*) malloc(sizeof(OCStringRef));
        else theDictionary->keys = (OCStringRef*) realloc(theDictionary->keys, (theDictionary->count+1) * sizeof(OCStringRef));
        
        theDictionary->values[theDictionary->count] = type;
        OCRetain(type);
        theDictionary->keys[theDictionary->count] = OCStringCreateCopy(key);
        theDictionary->count++;
        if(theDictionary->capacity!=0) theDictionary->capacity++;
    }
    else {
        theDictionary->values[theDictionary->count] = type;
        OCRetain(type);
        theDictionary->keys[theDictionary->count] = OCStringCreateCopy(key);
        theDictionary->count++;
    }
}

void OCDictionaryGetKeysAndValues(OCDictionaryRef theDictionary, const void **keys, const void **values)
{
    OCStringRef *outKeys = (OCStringRef *) keys;
    OCTypeRef *outValues = (OCTypeRef *) values;
    
    for(uint64_t index = 0; index<theDictionary->count;index++) {
        outKeys[index] = theDictionary->keys[index];
        outValues[index] = theDictionary->values[index];
    }
}

void OCDictionarySetValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value)
{
    int64_t index = OCDictionaryIndexOfKey( theDictionary,  key);

    if(index<0) {
        OCDictionaryAddValue(theDictionary,  key,  value);
        return;
    }
    OCRelease(theDictionary->values[index]);
    theDictionary->values[index] = (OCTypeRef) OCRetain(value);
}

void OCDictionaryReplaceValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void * value)
{
    int64_t index = OCDictionaryIndexOfKey( theDictionary,  key);
    if(index<0) return;
    OCRelease(theDictionary->values[index]);
    theDictionary->values[index] = (OCTypeRef) OCRetain(value);
}

void OCDictionaryRemoveValue(OCMutableDictionaryRef theDictionary, OCStringRef key)
{
    int64_t indexOfKey = OCDictionaryIndexOfKey( theDictionary,  key);
    if(indexOfKey < 0) return; // Key not found

    // Release the key and value being removed
    OCRelease(theDictionary->keys[indexOfKey]);
    OCRelease(theDictionary->values[indexOfKey]);

    // Shift subsequent elements down.
    // The loop should go up to count - 1 because we are accessing index and index + 1.
    for(uint64_t index = indexOfKey; index < theDictionary->count - 1; index++) {
        theDictionary->keys[index] = theDictionary->keys[index+1];
        theDictionary->values[index] = theDictionary->values[index+1];
    }
    // After shifting, decrement the count.
    // The elements beyond the new count are effectively garbage and won't be accessed.
    theDictionary->count--;
}

uint64_t OCDictionaryGetCountOfValue(OCMutableDictionaryRef theDictionary, const void * value)
{
    uint64_t count = 0;
    for(uint64_t index = 0;index<theDictionary->count;index++) {
        if(OCTypeEqual(theDictionary->values[index],value)) count++;
    }
    return count;
}

OCArrayRef OCDictionaryCreateArrayWithAllKeys(OCDictionaryRef theDictionary)
{
    if(theDictionary==NULL) return NULL;
    
    uint64_t count = OCDictionaryGetCount(theDictionary);
    const void **keys = malloc(count * sizeof(const void *));
    const void **values = malloc(count * sizeof(const void *));
    
    OCDictionaryGetKeysAndValues (theDictionary, (const void **) keys,(const void **) values);
    
    OCArrayRef array = OCArrayCreate((const void **) keys, count, &kOCTypeArrayCallBacks);
    free(values);
    free(keys);
    return array;
}

OCArrayRef OCDictionaryCreateArrayWithAllValues(OCDictionaryRef theDictionary)
{
    if(theDictionary==NULL) return NULL;
    
    uint64_t count = OCDictionaryGetCount(theDictionary);
    const void **keys = malloc(count * sizeof(void *));
    const void **values = malloc(count * sizeof(void *));
    
    OCDictionaryGetKeysAndValues (theDictionary, (const void **) keys,(const void **) values);
    
    OCArrayRef array = OCArrayCreate((const void **) values, count,&kOCTypeArrayCallBacks);
    free(values);
    free(keys);
    return array;
}

