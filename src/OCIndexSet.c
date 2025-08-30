/*
 * OCIndexSet.c – Improved Implementation
 *
 * Provides immutable and mutable index set types backed by OCData.
 * Includes insertion, containment, serialization, and range creation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OCTypes.h"
static OCTypeID kOCIndexSetID = kOCNotATypeID;
struct impl_OCIndexSet {
    OCBase base;
    OCDataRef indexes;
};
// -- Equality --
bool impl_OCIndexSetEqual(const void *a_, const void *b_) {
    OCIndexSetRef a = (OCIndexSetRef)a_;
    OCIndexSetRef b = (OCIndexSetRef)b_;
    if (!a || !b) return false;
    size_t len1 = a->indexes ? OCDataGetLength(a->indexes) : 0;
    size_t len2 = b->indexes ? OCDataGetLength(b->indexes) : 0;
    if (len1 != len2) return false;
    if (len1 == 0) return true;
    return memcmp(OCDataGetBytesPtr(a->indexes), OCDataGetBytesPtr(b->indexes), len1) == 0;
}
// -- Finalization --
void impl_OCIndexSetFinalize(const void *obj) {
    OCMutableIndexSetRef s = (OCMutableIndexSetRef)obj;
    if (s->indexes) OCRelease(s->indexes);
    s->indexes = NULL;
}
static OCStringRef impl_OCIndexSetCopyFormattingDesc(OCTypeRef cf) {
    if (!cf) return NULL;
    OCIndexSetRef set = (OCIndexSetRef)cf;
    OCMutableStringRef desc = OCStringCreateMutable(0);
    OCStringAppendCString(desc, "<OCIndexSet: ");
    OCIndex count = OCIndexSetGetCount(set);
    for (OCIndex i = 0; i < count; i++) {
        if (i > 0) OCStringAppendCString(desc, ", ");
        OCStringAppendFormat(desc, STR("%ld"), OCIndexSetGetBytesPtr(set)[i]);
    }
    OCStringAppendCString(desc, ">");
    return desc;
}
static cJSON *
impl_OCIndexSetCopyJSON(const void *obj) {
    return OCIndexSetCreateJSON((OCIndexSetRef)obj);
}

static cJSON *
impl_OCIndexSetCopyJSONTyped(const void *obj) {
    return OCIndexSetCreateJSONTyped((OCIndexSetRef)obj);
}
OCMutableIndexSetRef OCIndexSetAllocate(void);
static void *impl_OCIndexSetDeepCopy(const void *obj) {
    OCIndexSetRef src = (OCIndexSetRef)obj;
    if (!src || !src->indexes) return NULL;
    OCDataRef copyData = OCDataCreateCopy(src->indexes);
    if (!copyData) return NULL;
    OCMutableIndexSetRef copy = OCIndexSetAllocate();
    if (!copy) {
        OCRelease(copyData);
        return NULL;
    }
    copy->indexes = copyData;
    return copy;
}
static void *impl_OCIndexSetDeepCopyMutable(const void *obj) {
    OCIndexSetRef src = (OCIndexSetRef)obj;
    if (!src || !src->indexes) return NULL;
    OCMutableDataRef copyData = OCDataCreateMutableCopy(OCDataGetLength(src->indexes), src->indexes);
    if (!copyData) return NULL;
    OCMutableIndexSetRef copy = OCIndexSetAllocate();
    if (!copy) {
        OCRelease(copyData);
        return NULL;
    }
    copy->indexes = copyData;
    return copy;
}
// -- Type Registration --
OCTypeID OCIndexSetGetTypeID(void) {
    if (kOCIndexSetID == kOCNotATypeID) {
        kOCIndexSetID = OCRegisterType("OCIndexSet", (OCTypeRef (*)(cJSON *))OCIndexSetCreateFromJSONTyped);
    }
    return kOCIndexSetID;
}
// -- Allocation --
OCMutableIndexSetRef OCIndexSetAllocate(void) {
    return (OCMutableIndexSetRef)OCTypeAlloc(
        struct impl_OCIndexSet,
        OCIndexSetGetTypeID(),
        impl_OCIndexSetFinalize,
        impl_OCIndexSetEqual,
        impl_OCIndexSetCopyFormattingDesc,
        impl_OCIndexSetCopyJSON,
        impl_OCIndexSetCopyJSONTyped,
        impl_OCIndexSetDeepCopy,
        impl_OCIndexSetDeepCopyMutable);
}
// -- Constructors --
OCIndexSetRef OCIndexSetCreate(void) {
    OCMutableIndexSetRef s = OCIndexSetAllocate();
    s->indexes = NULL;
    return s;
}
OCMutableIndexSetRef OCIndexSetCreateMutable(void) {
    return (OCMutableIndexSetRef)OCIndexSetCreate();
}
OCIndexSetRef OCIndexSetCreateCopy(OCIndexSetRef src) {
    return src ? OCIndexSetCreateWithData(src->indexes) : NULL;
}
OCMutableIndexSetRef OCIndexSetCreateMutableCopy(OCIndexSetRef src) {
    return src ? (OCMutableIndexSetRef)OCIndexSetCreateWithData(src->indexes) : OCIndexSetCreateMutable();
}
OCIndexSetRef OCIndexSetCreateWithIndex(OCIndex index) {
    OCMutableIndexSetRef s = OCIndexSetAllocate();
    s->indexes = OCDataCreate((const uint8_t *)&index, sizeof(OCIndex));
    return s;
}
OCIndexSetRef OCIndexSetCreateWithIndexesInRange(OCIndex location, OCIndex length) {
    OCMutableIndexSetRef s = OCIndexSetAllocate();
    if (length <= 0) return s;
    OCIndex *temp = malloc(length * sizeof(OCIndex));
    for (OCIndex i = 0; i < length; i++) temp[i] = location + i;
    s->indexes = OCDataCreate((const uint8_t *)temp, length * sizeof(OCIndex));
    free(temp);
    return s;
}
// -- Accessors --
OCDataRef OCIndexSetGetIndexes(OCIndexSetRef set) {
    return set ? set->indexes : NULL;
}
OCIndex *OCIndexSetGetBytesPtr(OCIndexSetRef set) {
    return set && set->indexes ? (OCIndex *)OCDataGetBytesPtr(set->indexes) : NULL;
}
OCIndex OCIndexSetGetCount(OCIndexSetRef set) {
    return set && set->indexes ? OCDataGetLength(set->indexes) / sizeof(OCIndex) : 0;
}
OCIndex OCIndexSetFirstIndex(OCIndexSetRef set) {
    OCIndex *ptr = OCIndexSetGetBytesPtr(set);
    return ptr ? ptr[0] : kOCNotFound;
}
OCIndex OCIndexSetLastIndex(OCIndexSetRef set) {
    OCIndex count = OCIndexSetGetCount(set);
    if (count == 0) return kOCNotFound;
    OCIndex *ptr = OCIndexSetGetBytesPtr(set);
    return ptr[count - 1];
}
OCIndex OCIndexSetIndexLessThanIndex(OCIndexSetRef set, OCIndex index) {
    OCIndex *ptr = OCIndexSetGetBytesPtr(set);
    for (OCIndex i = OCIndexSetGetCount(set); i-- > 0;) {
        if (ptr[i] < index) return ptr[i];
    }
    return kOCNotFound;
}
OCIndex OCIndexSetIndexGreaterThanIndex(OCIndexSetRef set, OCIndex index) {
    OCIndex *ptr = OCIndexSetGetBytesPtr(set);
    OCIndex count = OCIndexSetGetCount(set);
    for (OCIndex i = 0; i < count; i++) {
        if (ptr[i] > index) return ptr[i];
    }
    return kOCNotFound;
}
bool OCIndexSetContainsIndex(OCIndexSetRef set, OCIndex index) {
    OCIndex *ptr = OCIndexSetGetBytesPtr(set);
    OCIndex count = OCIndexSetGetCount(set);
    for (OCIndex i = 0; i < count; i++) {
        if (ptr[i] == index) return true;
    }
    return false;
}
bool OCIndexSetAddIndex(OCMutableIndexSetRef set, OCIndex index) {
    if (!set) return false;
    OCIndex *buf = NULL;
    OCIndex count = 0;
    // If we have no backing data yet, create a one‐element buffer
    if (!set->indexes) {
        OCMutableDataRef newData = OCDataCreateMutable(sizeof(OCIndex));
        if (!newData) return false;
        if (!OCDataSetLength(newData, sizeof(OCIndex))) {
            OCRelease(newData);
            return false;
        }
        OCIndex *ptr = (OCIndex *)OCDataGetMutableBytes(newData);
        ptr[0] = index;
        set->indexes = newData;
        return true;
    }
    // Otherwise, we already have some data:
    count = OCIndexSetGetCount(set);
    buf = OCIndexSetGetBytesPtr(set);
    if (!buf) return false;
    // Check if already present
    for (OCIndex i = 0; i < count; i++) {
        if (buf[i] == index) return false;
        if (buf[i] > index) {
            // Insert into the middle
            OCIndex *newBuf = malloc(sizeof(OCIndex) * (count + 1));
            if (!newBuf) return false;
            memcpy(newBuf, buf, i * sizeof(OCIndex));
            newBuf[i] = index;
            memcpy(newBuf + i + 1, buf + i, (count - i) * sizeof(OCIndex));
            OCRelease(set->indexes);
            OCMutableDataRef newData = OCDataCreateMutable(sizeof(OCIndex) * (count + 1));
            if (!newData) {
                free(newBuf);
                return false;
            }
            if (!OCDataSetLength(newData, sizeof(OCIndex) * (count + 1))) {
                OCRelease(newData);
                free(newBuf);
                return false;
            }
            memcpy(OCDataGetMutableBytes(newData), newBuf, sizeof(OCIndex) * (count + 1));
            free(newBuf);
            set->indexes = newData;
            return true;
        }
    }
    // Append at end
    if (!OCDataIncreaseLength((OCMutableDataRef)set->indexes, sizeof(OCIndex)))
        return false;
    OCIndex *newPtr = (OCIndex *)OCDataGetMutableBytes((OCMutableDataRef)set->indexes);
    if (!newPtr) return false;
    newPtr[count] = index;
    return true;
}
bool OCIndexSetEqual(OCIndexSetRef a, OCIndexSetRef b) {
    return impl_OCIndexSetEqual(a, b);
}
OCArrayRef OCIndexSetCreateOCNumberArray(OCIndexSetRef set) {
    OCMutableArrayRef arr = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    OCIndex *buf = OCIndexSetGetBytesPtr(set);
    OCIndex count = OCIndexSetGetCount(set);
    for (OCIndex i = 0; i < count; i++) {
        OCNumberRef num = OCNumberCreateWithOCIndex(buf[i]);
        OCArrayAppendValue(arr, num);
        OCRelease(num);
    }
    return arr;
}
OCDictionaryRef OCIndexSetCreateDictionary(OCIndexSetRef set) {
    if (!set) return NULL;
    OCMutableDictionaryRef dict = OCDictionaryCreateMutable(0);
    if (set->indexes)
        OCDictionarySetValue(dict, STR("indexes"), set->indexes);
    return dict;
}
OCIndexSetRef OCIndexSetCreateFromDictionary(OCDictionaryRef dict) {
    if (!dict) return NULL;
    OCDataRef data = OCDictionaryGetValue(dict, STR("indexes"));
    return data ? OCIndexSetCreateWithData(data) : OCIndexSetCreate();
}
OCDataRef OCIndexSetCreateData(OCIndexSetRef set) {
    return set ? OCRetain(set->indexes) : NULL;
}
OCIndexSetRef OCIndexSetCreateWithData(OCDataRef data) {
    if (!data) return NULL;
    OCMutableIndexSetRef s = OCIndexSetAllocate();
    s->indexes = (OCMutableDataRef)OCDataCreateCopy(data);
    return s;
}
cJSON *OCIndexSetCreateJSON(OCIndexSetRef set) {
    if (!set) return cJSON_CreateNull();
    OCIndex count = OCIndexSetGetCount(set);
    OCIndex *buf = OCIndexSetGetBytesPtr(set);
    if (!buf) return cJSON_CreateNull();
    cJSON *arr = cJSON_CreateArray();
    for (OCIndex i = 0; i < count; i++) {
        cJSON_AddItemToArray(arr, cJSON_CreateNumber((double)buf[i]));
    }
    return arr;
}

OCIndexSetRef OCIndexSetCreateFromJSON(cJSON *json) {
    if (!cJSON_IsArray(json)) return NULL;
    OCMutableIndexSetRef set = OCIndexSetCreateMutable();
    OCIndex count = cJSON_GetArraySize(json);
    for (OCIndex i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(json, (int)i);
        if (!cJSON_IsNumber(item)) {
            OCRelease(set);
            return NULL;
        }
        OCIndexSetAddIndex(set, (OCIndex)item->valuedouble);
    }
    return set;
}

cJSON *OCIndexSetCreateJSONTyped(OCIndexSetRef set) {
    if (!set) return cJSON_CreateNull();
    
    cJSON *entry = cJSON_CreateObject();
    cJSON_AddStringToObject(entry, "type", "OCIndexSet");
    
    OCIndex count = OCIndexSetGetCount(set);
    OCIndex *buf = OCIndexSetGetBytesPtr(set);
    if (!buf) {
        cJSON_AddItemToObject(entry, "value", cJSON_CreateArray());
        return entry;
    }
    
    cJSON *arr = cJSON_CreateArray();
    for (OCIndex i = 0; i < count; i++) {
        cJSON_AddItemToArray(arr, cJSON_CreateNumber((double)buf[i]));
    }
    
    cJSON_AddItemToObject(entry, "value", arr);
    return entry;
}

OCIndexSetRef OCIndexSetCreateFromJSONTyped(cJSON *json) {
    if (!json || !cJSON_IsObject(json)) return NULL;
    
    cJSON *type = cJSON_GetObjectItem(json, "type");
    cJSON *value = cJSON_GetObjectItem(json, "value");
    
    if (!cJSON_IsString(type) || !cJSON_IsArray(value)) return NULL;
    
    const char *typeName = cJSON_GetStringValue(type);
    if (!typeName || strcmp(typeName, "OCIndexSet") != 0) return NULL;
    
    OCMutableIndexSetRef set = OCIndexSetCreateMutable();
    if (!set) return NULL;
    
    int count = cJSON_GetArraySize(value);
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(value, i);
        if (!cJSON_IsNumber(item)) {
            fprintf(stderr, "OCIndexSetCreateFromJSONTyped: Invalid index at position %d\n", i);
            OCRelease(set);
            return NULL;
        }
        OCIndexSetAddIndex(set, (OCIndex)item->valuedouble);
    }
    
    return set;
}
void OCIndexSetShow(OCIndexSetRef set) {
    fprintf(stderr, "(");
    OCIndex *buf = OCIndexSetGetBytesPtr(set);
    OCIndex count = OCIndexSetGetCount(set);
    for (OCIndex i = 0; i < count; i++) {
        fprintf(stderr, "%ld%s", buf[i], (i < count - 1) ? "," : "");
    }
    fprintf(stderr, ")\n");
}
