/**
 * OCSet.c
 *
 * OCSet implementation using OCArray internally.
 * Provides unordered, unique collection of OCTypeRef values.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OCTypes.h"
static OCTypeID kOCSetID = kOCNotATypeID;
struct impl_OCSet {
    OCBase base;
    OCMutableArrayRef elements;
};
static bool impl_OCSetEqual(const void *a, const void *b) {
    OCSetRef set1 = (OCSetRef)a;
    OCSetRef set2 = (OCSetRef)b;
    if (set1 == set2) return true;
    if (!set1 || !set2) return false;
    OCIndex count1 = OCArrayGetCount(set1->elements);
    OCIndex count2 = OCArrayGetCount(set2->elements);
    if (count1 != count2) return false;
    for (OCIndex i = 0; i < count1; ++i) {
        OCTypeRef v = (OCTypeRef)OCArrayGetValueAtIndex(set1->elements, i);
        if (!OCArrayContainsValue(set2->elements, v)) return false;
    }
    return true;
}
static void impl_OCSetFinalize(const void *obj) {
    OCSetRef set = (OCSetRef)obj;
    if (set && set->elements) {
        OCRelease(set->elements);
    }
}
static OCStringRef impl_OCSetCopyFormattingDesc(OCTypeRef cf) {
    OCSetRef set = (OCSetRef)cf;
    if (!set) return OCStringCreateWithCString("<OCSet: NULL>");
    OCIndex count = OCArrayGetCount(set->elements);
    OCMutableStringRef result = OCStringCreateMutable(0);
    OCStringAppendFormat(result, STR("<OCSet: %u value%s {"), count, count == 1 ? "" : "s");
    for (OCIndex i = 0; i < count; ++i) {
        OCTypeRef val = (OCTypeRef)OCArrayGetValueAtIndex(set->elements, i);
        OCStringRef desc = OCTypeCopyFormattingDesc(val);
        OCStringAppend(result, desc);
        OCRelease(desc);
        if (i + 1 < count)
            OCStringAppendCString(result, ", ");
    }
    OCStringAppendCString(result, "}>");
    return result;
}
static cJSON *
impl_OCSetCopyJSON(const void *obj, bool typed) {
    return OCSetCreateJSON((OCSetRef)obj, typed);
}

static void *impl_OCSetDeepCopy(const void *obj) {
    OCSetRef src = (OCSetRef)obj;
    if (!src || !src->elements) return NULL;
    OCMutableSetRef copy = OCSetCreateMutable(0);
    if (!copy) return NULL;
    OCIndex count = OCArrayGetCount(src->elements);
    for (OCIndex i = 0; i < count; i++) {
        OCTypeRef original = OCArrayGetValueAtIndex(src->elements, i);
        void *cloned = OCTypeDeepCopy(original);
        if (!cloned) {
            OCRelease(copy);
            return NULL;
        }
        OCSetAddValue(copy, cloned);
        OCRelease(cloned);  // AddValue retains it
    }
    return (void *)copy;
}
static void *impl_OCSetDeepCopyMutable(const void *obj) {
    // OCSet is mutable, so same as deep copy
    return impl_OCSetDeepCopy(obj);
}
OCTypeID OCSetGetTypeID(void) {
    if (kOCSetID == kOCNotATypeID) {
        kOCSetID = OCRegisterType("OCSet", (OCTypeRef (*)(cJSON *))OCSetCreateFromJSONTyped);
    }
    return kOCSetID;
}
static struct impl_OCSet *OCSetAllocate(void) {
    struct impl_OCSet *set = OCTypeAlloc(
        struct impl_OCSet,
        OCSetGetTypeID(),
        impl_OCSetFinalize,
        impl_OCSetEqual,
        impl_OCSetCopyFormattingDesc,
        impl_OCSetCopyJSON,
        impl_OCSetDeepCopy,
        impl_OCSetDeepCopyMutable);
    set->elements = NULL;
    return set;
}
OCSetRef OCSetCreate(void) {
    struct impl_OCSet *set = OCSetAllocate();
    set->elements = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    return (OCSetRef)set;
}
OCMutableSetRef OCSetCreateMutable(OCIndex capacity) {
    struct impl_OCSet *set = OCSetAllocate();
    set->elements = OCArrayCreateMutable(capacity, &kOCTypeArrayCallBacks);
    return (OCMutableSetRef)set;
}
OCSetRef OCSetCreateCopy(OCSetRef theSet) {
    if (!theSet) return NULL;
    OCSetRef copy = OCSetCreate();
    OCIndex count = OCArrayGetCount(theSet->elements);
    for (OCIndex i = 0; i < count; i++) {
        OCTypeRef value = OCArrayGetValueAtIndex(theSet->elements, i);
        OCSetAddValue((OCMutableSetRef)copy, value);
    }
    return copy;
}
OCMutableSetRef OCSetCreateMutableCopy(OCSetRef theSet) {
    return (OCMutableSetRef)OCSetCreateCopy(theSet);
}
OCIndex OCSetGetCount(OCSetRef theSet) {
    if (!theSet) return 0;
    return OCArrayGetCount(theSet->elements);
}
bool OCSetContainsValue(OCSetRef theSet, OCTypeRef value) {
    if (!theSet || !value) return false;
    return OCArrayContainsValue(theSet->elements, value);
}
OCArrayRef OCSetCreateValueArray(OCSetRef theSet) {
    if (!theSet) return NULL;
    return OCArrayCreateCopy(theSet->elements);
}
bool OCSetAddValue(OCMutableSetRef theSet, OCTypeRef value) {
    if (!theSet || !value) return false;
    if (OCSetContainsValue(theSet, value)) return true;
    OCArrayAppendValue(theSet->elements, value);
    return true;
}
bool OCSetRemoveValue(OCMutableSetRef theSet, OCTypeRef value) {
    if (!theSet || !value) return false;
    OCIndex idx = OCArrayGetFirstIndexOfValue(theSet->elements, value);
    if (idx == kOCNotFound) return false;
    OCArrayRemoveValueAtIndex(theSet->elements, idx);
    return true;
}
void OCSetRemoveAllValues(OCMutableSetRef theSet) {
    if (!theSet) return;
    while (OCSetGetCount(theSet) > 0) {
        OCArrayRemoveValueAtIndex(theSet->elements, 0);
    }
}
bool OCSetEqual(OCSetRef a, OCSetRef b) {
    return impl_OCSetEqual(a, b);
}
cJSON *OCSetCreateJSON(OCSetRef set, bool typed) {
    if (!set) return cJSON_CreateNull();

    // Build the array of elements first (common to both paths)
    cJSON *arr = cJSON_CreateArray();
    OCArrayRef elems = set->elements;
    OCIndex count = OCArrayGetCount(elems);
    
    for (OCIndex i = 0; i < count; i++) {
        OCTypeRef v = (OCTypeRef)OCArrayGetValueAtIndex(elems, i);
        cJSON *item = OCTypeCopyJSON(v, typed);
        if (!item) {
            fprintf(stderr, "OCSetCreateJSON: Failed to serialize set element at index %llu. Using null.\n", (unsigned long long)i);
            item = cJSON_CreateNull();
        }
        cJSON_AddItemToArray(arr, item);
    }

    if (typed) {
        // For typed serialization, OCSet needs wrapping since it's not a native JSON type
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddStringToObject(entry, "type", "OCSet");
        cJSON_AddItemToObject(entry, "value", arr);
        return entry;
    } else {
        // For untyped serialization, return the plain JSON array
        return arr;
    }
}

OCSetRef OCSetCreateFromJSONTyped(cJSON *json) {
    if (!json || !cJSON_IsObject(json)) return NULL;

    cJSON *type = cJSON_GetObjectItem(json, "type");
    cJSON *value = cJSON_GetObjectItem(json, "value");

    if (!cJSON_IsString(type) || !cJSON_IsArray(value)) return NULL;

    const char *typeName = cJSON_GetStringValue(type);
    if (!typeName || strcmp(typeName, "OCSet") != 0) return NULL;

    int arraySize = cJSON_GetArraySize(value);
    if (arraySize < 0) return NULL;

    // Create mutable set to build up the result
    OCMutableSetRef result = OCSetCreateMutable(arraySize);
    if (!result) return NULL;

    for (int i = 0; i < arraySize; i++) {
        cJSON *elem = cJSON_GetArrayItem(value, i);

        // Use the global factory function to deserialize each element
        OCTypeRef obj = OCTypeCreateFromJSONTyped(elem);

        if (obj) {
            OCSetAddValue(result, obj);
            OCRelease(obj); // OCSetAddValue retains
        } else {
            fprintf(stderr, "OCSetCreateFromJSONTyped: Cannot deserialize element at index %d\n", i);
            // Skip undeserializable elements
        }
    }

    return result;
}
void OCSetShow(OCSetRef theSet) {
    if (!theSet) return;
    OCStringRef desc = OCTypeCopyFormattingDesc(theSet);
    if (desc) {
        fprintf(stderr, "%s\n", OCStringGetCString(desc));
        OCRelease(desc);
    }
}
