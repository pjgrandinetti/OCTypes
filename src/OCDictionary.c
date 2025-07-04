// OCDictionary.c – Updated to use OCTypeAlloc and leak-safe finalization
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "OCLibrary.h"

static OCTypeID kOCDictionaryID = kOCNotATypeID;

// OCDictionary Opaque Type
struct impl_OCDictionary
{
    OCBase base;
    uint64_t count;
    uint64_t capacity;
    OCStringRef *keys;
    OCTypeRef *values;
};

OCTypeID OCDictionaryGetTypeID(void)
{
    if (kOCDictionaryID == kOCNotATypeID)
        kOCDictionaryID = OCRegisterType("OCDictionary");
    return kOCDictionaryID;
}


static bool impl_OCDictionaryEqual(const void *theType1, const void *theType2)
{
    OCDictionaryRef d1 = (OCDictionaryRef)theType1;
    OCDictionaryRef d2 = (OCDictionaryRef)theType2;
    if (d1 == d2)
        return true;
    if (!d1 || !d2 || d1->base.typeID != d2->base.typeID)
        return false;
    if (d1->count != d2->count)
        return false;
    for (uint64_t i = 0; i < d1->count; i++)
    {
        if (!OCTypeEqual(d1->keys[i], d2->keys[i]))
            return false;
        if (!OCTypeEqual(d1->values[i], d2->values[i]))
            return false;
    }
    return true;
}

OCStringRef OCDictionaryCopyFormattingDesc(OCTypeRef cf)
{
    if (!cf)
        return OCStringCreateWithCString("<OCDictionary: NULL>");

    OCDictionaryRef dict = (OCDictionaryRef)cf;
    const size_t maxItems = 5;
    size_t count = OCDictionaryGetCount(dict);
    size_t shown = count < maxItems ? count : maxItems;

    OCMutableStringRef result = OCStringCreateMutable(0);
    OCStringAppendFormat(result, STR("<OCDictionary: %zu pair%s {"),
                         count, count == 1 ? "" : "s");

    for (size_t i = 0; i < shown; ++i)
    {
        if (i > 0)
            OCStringAppendCString(result, ", ");

        OCTypeRef key = (OCTypeRef)dict->keys[i];
        OCTypeRef value = dict->values[i];

        OCStringRef keyDesc = OCTypeCopyFormattingDesc(key);
        OCStringRef valDesc = OCTypeCopyFormattingDesc(value);

        if (keyDesc)
        {
            OCStringAppend(result, keyDesc);
            OCRelease(keyDesc);
        }
        else
        {
            OCStringAppendCString(result, "(null key)");
        }

        OCStringAppendCString(result, ": ");

        if (valDesc)
        {
            OCStringAppend(result, valDesc);
            OCRelease(valDesc);
        }
        else
        {
            OCStringAppendCString(result, "(null value)");
        }
    }

    if (count > shown)
        OCStringAppendCString(result, ", …");

    OCStringAppendCString(result, "}>");
    return result;
}
static void *impl_OCDictionaryDeepCopy(const void *obj)
{
    const OCDictionaryRef src = (OCDictionaryRef)obj;
    if (!src) 
        return NULL;

    // Always create a new mutable dictionary, even if src->count == 0
    OCMutableDictionaryRef copy = OCDictionaryCreateMutable(src->count);
    if (!copy)
        return NULL;

    for (uint64_t i = 0; i < src->count; ++i) {
        // deep-copy the key
        OCStringRef keyCopy = (OCStringRef)OCTypeDeepCopy(src->keys[i]);
        if (!keyCopy) {
            OCRelease(copy);
            return NULL;
        }
        // deep-copy the value
        void *valCopy = OCTypeDeepCopy(src->values[i]);
        if (!valCopy) {
            OCRelease(keyCopy);
            OCRelease(copy);
            return NULL;
        }
        // insert (AddValue retains both)
        if (!OCDictionaryAddValue(copy, keyCopy, valCopy)) {
            OCRelease(keyCopy);
            OCRelease(valCopy);
            OCRelease(copy);
            return NULL;
        }
        OCRelease(keyCopy);
        OCRelease(valCopy);
    }

    return (void *)copy;
}

static void *impl_OCDictionaryDeepCopyMutable(const void *obj)
{
    return impl_OCDictionaryDeepCopy(obj);
}

static void impl_OCDictionaryReleaseKeysAndValues(OCDictionaryRef dict)
{
    for (uint64_t i = 0; i < dict->count; i++)
    {
        OCRelease(dict->keys[i]);
        OCRelease(dict->values[i]);
    }
}

static void impl_OCDictionaryFinalize(const void *theType)
{
    if (NULL == theType)
    {
        fprintf(stderr, "Finalize called with NULL pointer\n");
        return;
    }
    OCDictionaryRef dict = (OCDictionaryRef)theType;
    impl_OCDictionaryReleaseKeysAndValues(dict);

    if (dict->keys)
    {
        free(dict->keys);
    }
    if (dict->values)
    {
        free(dict->values);
    }
}

static cJSON *
impl_OCDictionaryCopyJSON(const void *obj)
{
    return OCDictionaryCreateJSON((OCDictionaryRef)obj);
}


static struct impl_OCDictionary *OCDictionaryAllocate()
{
    struct impl_OCDictionary *dict = OCTypeAlloc(
        struct impl_OCDictionary,
        OCDictionaryGetTypeID(),
        impl_OCDictionaryFinalize,
        impl_OCDictionaryEqual,
        OCDictionaryCopyFormattingDesc,
        impl_OCDictionaryCopyJSON,
        impl_OCDictionaryDeepCopy,
        impl_OCDictionaryDeepCopyMutable);
    return dict;
}

uint64_t OCDictionaryGetCount(OCDictionaryRef theDictionary)
{
    if (NULL == theDictionary)
        return 0;
    return theDictionary->count;
}

OCDictionaryRef OCDictionaryCreate(const void **keys,
                                   const void **values,
                                   uint64_t      numValues)
{
    if (!keys || !values) return NULL;

    // Normalize zero → at least one slot
    uint64_t allocCap = (numValues > 0 ? numValues : 1);
    OCMutableDictionaryRef dict = OCDictionaryCreateMutable(allocCap);
    if (!dict) return NULL;

    // If there really are no entries, we’re done:
    if (numValues == 0) {
        return dict;
    }

    // Otherwise copy keys & values as before...
    dict->count = numValues; // safe because allocCap >= numValues
    for (uint64_t i = 0; i < numValues; i++) {
        dict->keys[i]   = OCStringCreateCopy((OCStringRef)keys[i]);
        dict->values[i] = (OCTypeRef)OCRetain(values[i]);
    }
    return dict;
}

OCMutableDictionaryRef OCDictionaryCreateMutable(uint64_t capacity)
{
    struct impl_OCDictionary *theDictionary = OCDictionaryAllocate();
    if (!theDictionary) return NULL;

    // Normalize zero capacity → at least one slot
    uint64_t allocCap = (capacity > 0 ? capacity : 1);

    theDictionary->values = (OCTypeRef *)calloc(allocCap, sizeof(OCTypeRef));
    if (!theDictionary->values) {
        fprintf(stderr, "OCDictionaryCreateMutable: Memory allocation for values failed.\n");
        OCRelease(theDictionary);
        return NULL;
    }
    theDictionary->keys = (OCStringRef *)calloc(allocCap, sizeof(OCStringRef));
    if (!theDictionary->keys) {
        fprintf(stderr, "OCDictionaryCreateMutable: Memory allocation for keys failed.\n");
        free(theDictionary->values);
        OCRelease(theDictionary);
        return NULL;
    }

    theDictionary->count    = 0;
    theDictionary->capacity = allocCap;
    return theDictionary;
}

OCDictionaryRef OCDictionaryCreateCopy(OCDictionaryRef theDictionary)
{
    if (!theDictionary)
        return NULL;
    return (OCDictionaryRef)OCDictionaryCreate((const void **)theDictionary->keys, (const void **)theDictionary->values, theDictionary->count);
}

OCMutableDictionaryRef OCDictionaryCreateMutableCopy(OCDictionaryRef theDictionary)
{
    return (OCMutableDictionaryRef)OCDictionaryCreate((const void **)theDictionary->keys, (const void **)theDictionary->values, theDictionary->count);
}

const void *OCDictionaryGetValue(OCDictionaryRef theDictionary, OCStringRef key)
{
    for (long index = 0; index < theDictionary->count; index++)
    {
        if (OCStringCompare(theDictionary->keys[index], key, 0) == kOCCompareEqualTo)
            return theDictionary->values[index];
    }
    return NULL;
}

bool OCDictionaryContainsKey(OCDictionaryRef theDictionary, OCStringRef key)
{
    for (long index = 0; index < theDictionary->count; index++)
    {
        if (OCStringCompare(theDictionary->keys[index], key, 0) == kOCCompareEqualTo)
            return true;
    }
    return false;
}

int64_t OCDictionaryIndexOfKey(OCDictionaryRef theDictionary, OCStringRef key)
{
    for (int64_t index = 0; index < theDictionary->count; index++)
    {
        if (OCStringCompare(theDictionary->keys[index], key, 0) == kOCCompareEqualTo)
            return index;
    }
    return -1;
}

bool OCDictionaryContainsValue(OCDictionaryRef theDictionary, const void *value)
{
    for (long index = 0; index < theDictionary->count; index++)
    {
        if (theDictionary->values[index] == value)
            return true;
    }
    return false;
}

bool OCDictionaryAddValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void *value)
{
    if (!theDictionary || !key || !value)
        return false;

    // Check if the key already exists
    int64_t existingKeyIndex = OCDictionaryIndexOfKey(theDictionary, key);
    if (existingKeyIndex >= 0)
    {
        // Key exists, replace the value
        OCRelease(theDictionary->values[existingKeyIndex]);
        OCTypeRef type = (OCTypeRef)value;
        theDictionary->values[existingKeyIndex] = type;
        OCRetain(type);
        return true;
    }

    // Expand storage if necessary
    if (theDictionary->capacity == 0 || theDictionary->count == theDictionary->capacity)
    {
        OCTypeRef *newValues = NULL;
        OCStringRef *newKeys = NULL;

        uint64_t newCapacity = theDictionary->capacity == 0 ? 1 : theDictionary->capacity * 2;

        newValues = (OCTypeRef *)realloc(theDictionary->values, newCapacity * sizeof(OCTypeRef));
        if (!newValues)
        {
            fprintf(stderr, "OCDictionaryAddValue: Memory reallocation for values failed.\n");
            return false;
        }

        newKeys = (OCStringRef *)realloc(theDictionary->keys, newCapacity * sizeof(OCStringRef));
        if (!newKeys)
        {
            free(newValues); // Free previously allocated values
            fprintf(stderr, "OCDictionaryAddValue: Memory reallocation for keys failed.\n");
            return false;
        }

        theDictionary->values = newValues;
        theDictionary->keys = newKeys;
        theDictionary->capacity = newCapacity;
    }

    OCTypeRef type = (OCTypeRef)value;
    OCStringRef keyCopy = OCStringCreateCopy(key);
    if (!keyCopy)
    {
        fprintf(stderr, "OCDictionaryAddValue: Failed to copy key string.\n");
        return false;
    }

    theDictionary->values[theDictionary->count] = type;
    OCRetain(type);
    theDictionary->keys[theDictionary->count] = keyCopy;
    theDictionary->count++;

    return true;
}


bool OCDictionaryGetKeysAndValues(OCDictionaryRef theDictionary, const void **keys, const void **values)
{
    if (!theDictionary || !keys || !values)
        return false;

    OCStringRef *outKeys = (OCStringRef *)keys;
    OCTypeRef *outValues = (OCTypeRef *)values;

    for (uint64_t index = 0; index < theDictionary->count; index++) {
        outKeys[index] = theDictionary->keys[index];
        outValues[index] = theDictionary->values[index];
    }

    return true;
}

bool OCDictionarySetValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void *value)
{
    if (!theDictionary || !key || !value)
        return false;

    int64_t index = OCDictionaryIndexOfKey(theDictionary, key);

    if (index < 0)
        return OCDictionaryAddValue(theDictionary, key, value);

    OCRelease(theDictionary->values[index]);
    theDictionary->values[index] = (OCTypeRef)OCRetain(value);
    return true;
}

bool OCDictionaryReplaceValue(OCMutableDictionaryRef theDictionary, OCStringRef key, const void *value)
{
    if (!theDictionary || !key || !value)
        return false;

    int64_t index = OCDictionaryIndexOfKey(theDictionary, key);
    if (index < 0)
        return false;

    OCRelease(theDictionary->values[index]);
    theDictionary->values[index] = (OCTypeRef)OCRetain(value);
    return true;
}

bool OCDictionaryRemoveValue(OCMutableDictionaryRef theDictionary, OCStringRef key)
{
    if (!theDictionary || !key)
        return false;

    int64_t indexOfKey = OCDictionaryIndexOfKey(theDictionary, key);
    if (indexOfKey < 0)
        return false; // Key not found

    // Release the key and value being removed
    OCRelease(theDictionary->keys[indexOfKey]);
    OCRelease(theDictionary->values[indexOfKey]);

    // Shift subsequent elements down.
    for (uint64_t index = indexOfKey; index < theDictionary->count - 1; index++)
    {
        theDictionary->keys[index] = theDictionary->keys[index + 1];
        theDictionary->values[index] = theDictionary->values[index + 1];
    }

    // Decrement count
    theDictionary->count--;

    return true;
}

uint64_t OCDictionaryGetCountOfValue(OCMutableDictionaryRef theDictionary, const void *value)
{
    uint64_t count = 0;
    for (uint64_t index = 0; index < theDictionary->count; index++)
    {
        if (OCTypeEqual(theDictionary->values[index], value))
            count++;
    }
    return count;
}

OCArrayRef OCDictionaryCreateArrayWithAllKeys(OCDictionaryRef theDictionary)
{
    if (theDictionary == NULL)
        return NULL;

    uint64_t count = OCDictionaryGetCount(theDictionary);
    const void **keys = calloc(count, sizeof(const void *));
    if (NULL == keys)
    {
        fprintf(stderr, "OCDictionaryCreateArrayWithAllKeys: Memory allocation for keys failed.\n");
        return NULL;
    }
    const void **values = calloc(count, sizeof(const void *));
    if (NULL == values)
    {
        fprintf(stderr, "OCDictionaryCreateArrayWithAllKeys: Memory allocation for values failed.\n");
        free(keys);
        return NULL;
    }

    OCDictionaryGetKeysAndValues(theDictionary, (const void **)keys, (const void **)values);

    OCArrayRef array = OCArrayCreate((const void **)keys, count, &kOCTypeArrayCallBacks);
    free(values);
    free(keys);
    return array;
}

OCArrayRef OCDictionaryCreateArrayWithAllValues(OCDictionaryRef theDictionary)
{
    if (theDictionary == NULL)
        return NULL;

    uint64_t count = OCDictionaryGetCount(theDictionary);
    const void **keys = calloc(count, sizeof(void *));
    if (NULL == keys)
    {
        fprintf(stderr, "OCDictionaryCreateArrayWithAllValues: Memory allocation for keys failed.\n");
        return NULL;
    }
    const void **values = calloc(count, sizeof(void *));
    if (NULL == values)
    {
        fprintf(stderr, "OCDictionaryCreateArrayWithAllValues: Memory allocation for values failed.\n");
        free(keys);
        return NULL;
    }

    OCDictionaryGetKeysAndValues(theDictionary, (const void **)keys, (const void **)values);

    OCArrayRef array = OCArrayCreate((const void **)values, count, &kOCTypeArrayCallBacks);
    free(values);
    free(keys);
    return array;
}

cJSON *OCDictionaryCreateJSON(OCDictionaryRef dict) {
    if (!dict) return cJSON_CreateNull();

    cJSON *root = cJSON_CreateObject();
    OCArrayRef keys = OCDictionaryCreateArrayWithAllKeys(dict);
    uint64_t n = OCArrayGetCount(keys);

    for (uint64_t i = 0; i < n; i++) {
        OCStringRef key = OCArrayGetValueAtIndex(keys, i);
        const char *k = OCStringGetCString(key);
        OCTypeRef   v = (OCTypeRef)OCDictionaryGetValue(dict, key);

        cJSON *child = OCTypeCopyJSON(v);
        if (!child) {
            fprintf(stderr, "OCDictionaryCreateJSON: Failed to serialize value for key '%s'. Using null.\n", k);
            child = cJSON_CreateNull();
        }

        cJSON_AddItemToObject(root, k, child);
    }

    OCRelease(keys);
    return root;
}

