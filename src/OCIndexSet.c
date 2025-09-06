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
    OCJSONEncoding encoding;
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
impl_OCIndexSetCopyJSON(const void *obj, bool typed, OCStringRef *outError) {
    return OCIndexSetCopyAsJSON((OCIndexSetRef)obj, typed, outError);
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
        kOCIndexSetID = OCRegisterType("OCIndexSet", (OCTypeRef (*)(cJSON *, OCStringRef *))OCIndexSetCreateFromJSON);
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
    if (!temp) {
        OCRelease(s);
        return NULL;
    }
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
    return set ? OCDataCreateCopy(set->indexes) : NULL;
}
OCIndexSetRef OCIndexSetCreateWithData(OCDataRef data) {
    if (!data) return NULL;
    OCMutableIndexSetRef s = OCIndexSetAllocate();
    s->indexes = (OCMutableDataRef)OCDataCreateCopy(data);
    return s;
}
cJSON *OCIndexSetCopyAsJSON(OCIndexSetRef set, bool typed, OCStringRef *outError) {
    if (outError) *outError = NULL;

    if (!set) {
        if (outError) *outError = STR("OCIndexSet is NULL");
        return cJSON_CreateNull();
    }

    if (typed) {
        // For typed serialization, check encoding preference
        OCJSONEncoding encoding = set->encoding;
        cJSON *entry = cJSON_CreateObject();
        if (!entry) {
            if (outError) *outError = STR("Failed to create JSON object");
            return cJSON_CreateNull();
        }

        cJSON_AddStringToObject(entry, "type", "OCIndexSet");

        if (encoding == OCJSONEncodingBase64) {
            // Use base64 encoding for compact binary representation
            cJSON_AddStringToObject(entry, "encoding", "base64");
            OCStringRef b64 = OCDataCreateBase64EncodedString(set->indexes, OCBase64EncodingOptionsNone);
            if (b64) {
                const char *b64Str = OCStringGetCString(b64);
                cJSON_AddStringToObject(entry, "value", b64Str ? b64Str : "");
                OCRelease(b64);
            } else {
                if (outError) *outError = STR("Failed to create base64 encoding");
                cJSON_AddStringToObject(entry, "value", "");
            }
        } else {
            // Default: use JSON array format (encoding OCJSONEncodingNone)
            if (encoding == OCJSONEncodingNone) {
                cJSON_AddStringToObject(entry, "encoding", "none");
            }

            cJSON *arr = cJSON_CreateArray();
            if (!arr) {
                if (outError) *outError = STR("Failed to create JSON array");
                cJSON_Delete(entry);
                return cJSON_CreateNull();
            }

            OCIndex count = OCIndexSetGetCount(set);
            OCIndex *buf = OCIndexSetGetBytesPtr(set);

            // Add indexes to array (if any exist)
            if (buf && count > 0) {
                for (OCIndex i = 0; i < count; i++) {
                    cJSON *item = cJSON_CreateNumber((double)buf[i]);
                    if (!item) {
                        if (outError) *outError = STR("Failed to create JSON number");
                        cJSON_Delete(arr);
                        cJSON_Delete(entry);
                        return cJSON_CreateNull();
                    }
                    cJSON_AddItemToArray(arr, item);
                }
            }
            cJSON_AddItemToObject(entry, "value", arr);
        }
        return entry;
    } else {
        // For untyped serialization, return the plain JSON array (no change)
        cJSON *arr = cJSON_CreateArray();
        if (!arr) {
            if (outError) *outError = STR("Failed to create JSON array");
            return cJSON_CreateNull();
        }

        OCIndex count = OCIndexSetGetCount(set);
        OCIndex *buf = OCIndexSetGetBytesPtr(set);

        // Add indexes to array (if any exist)
        if (buf && count > 0) {
            for (OCIndex i = 0; i < count; i++) {
                cJSON *item = cJSON_CreateNumber((double)buf[i]);
                if (!item) {
                    if (outError) *outError = STR("Failed to create JSON number");
                    cJSON_Delete(arr);
                    return cJSON_CreateNull();
                }
                cJSON_AddItemToArray(arr, item);
            }
        }
        return arr;
    }
}

OCIndexSetRef OCIndexSetCreateFromJSON(cJSON *json, OCStringRef *outError) {
    if (!json) {
        if (outError) *outError = STR("JSON input is NULL");
        return NULL;
    }

    cJSON *arrayToProcess = NULL;

    // Check if typed format (object with type/value structure)
    if (cJSON_IsObject(json)) {
        cJSON *type = cJSON_GetObjectItem(json, "type");
        cJSON *encoding = cJSON_GetObjectItem(json, "encoding");
        cJSON *value = cJSON_GetObjectItem(json, "value");

        if (!cJSON_IsString(type)) {
            if (outError) *outError = STR("Invalid typed JSON format: missing or invalid type field");
            return NULL;
        }

        const char *typeName = cJSON_GetStringValue(type);
        if (!typeName || strcmp(typeName, "OCIndexSet") != 0) {
            if (outError) *outError = STR("Invalid type: expected OCIndexSet");
            return NULL;
        }

        // Handle base64 encoding
        if (encoding && cJSON_IsString(encoding) &&
            strcmp(cJSON_GetStringValue(encoding), "base64") == 0) {

            if (!cJSON_IsString(value)) {
                if (outError) *outError = STR("Invalid base64 format: value must be string");
                return NULL;
            }

            const char *b64Str = cJSON_GetStringValue(value);
            if (!b64Str) {
                if (outError) *outError = STR("Invalid base64 string");
                return NULL;
            }

            OCStringRef b64 = OCStringCreateWithCString(b64Str);
            OCDataRef decodedData = OCDataCreateFromBase64EncodedString(b64);
            OCRelease(b64);

            if (!decodedData) {
                if (outError) *outError = STR("Failed to decode base64 data");
                return NULL;
            }

            // Verify data length is multiple of OCIndex size
            uint64_t dataLen = OCDataGetLength(decodedData);
            if (dataLen % sizeof(OCIndex) != 0) {
                if (outError) *outError = STR("Invalid base64 data: length not multiple of OCIndex size");
                OCRelease(decodedData);
                return NULL;
            }

            OCIndexSetRef result = OCIndexSetCreateWithData(decodedData);
            OCRelease(decodedData);

            if (result) {
                // Set encoding on the result
                OCMutableIndexSetRef mutable = (OCMutableIndexSetRef)result;
                mutable->encoding = OCJSONEncodingBase64;
            }

            if (!result && outError) {
                *outError = STR("Failed to create OCIndexSet from base64 data");
            }
            return result;
        }
        // Handle JSON array format (encoding "none" or missing)
        else {
            if (!cJSON_IsArray(value)) {
                if (outError) *outError = STR("Invalid typed JSON format: value must be array for non-base64 encoding");
                return NULL;
            }
            arrayToProcess = value;
        }
    }
    // Handle untyped format (direct array)
    else if (cJSON_IsArray(json)) {
        arrayToProcess = json;
    }
    else {
        if (outError) *outError = STR("Invalid JSON: expected object or array");
        return NULL;
    }

    // Process the JSON array format
    OCMutableIndexSetRef set = OCIndexSetCreateMutable();
    if (!set) {
        if (outError) *outError = STR("Failed to create mutable index set");
        return NULL;
    }

    int count = cJSON_GetArraySize(arrayToProcess);
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(arrayToProcess, i);
        if (!cJSON_IsNumber(item)) {
            if (outError) *outError = STR("Invalid array element: expected number");
            OCRelease(set);
            return NULL;
        }
        if (!OCIndexSetAddIndex(set, (OCIndex)item->valuedouble)) {
            if (outError) *outError = STR("Failed to add index to set");
            OCRelease(set);
            return NULL;
        }
    }

    // If we parsed from typed format with explicit "none" encoding, set it
    if (cJSON_IsObject(json)) {
        cJSON *encoding = cJSON_GetObjectItem(json, "encoding");
        if (encoding && cJSON_IsString(encoding) &&
            strcmp(cJSON_GetStringValue(encoding), "none") == 0) {
            set->encoding = OCJSONEncodingNone;
        }
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
OCJSONEncoding OCIndexSetCopyEncoding(OCIndexSetRef set) {
    if (!set) return OCJSONEncodingNone;
    return set->encoding;
}
void OCIndexSetSetEncoding(OCMutableIndexSetRef set, OCJSONEncoding encoding) {
    if (!set) return;
    set->encoding = encoding;
}
