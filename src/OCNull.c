//
//  OCNull.c
//  OCTypes
//
#include <stdio.h>
#include "OCType.h"  // for OCRegisterType, OCTypeID, kOCNotATypeID
#include "OCTypes.h"
// Forward declarations for static functions used in initializers
static bool impl_OCNullEqual(const void *v1, const void *v2);
static void impl_OCNullFinalize(const void *unused);
static OCStringRef impl_OCNullCopyFormattingDesc(OCTypeRef cf);
static cJSON *impl_OCNullCopyJSON(const void *cf, bool typed, OCStringRef *outError);
static void *impl_OCNullDeepCopy(const void *cf);
// Static storage of the null type's OCTypeID
static OCTypeID kOCNullTypeID = kOCNotATypeID;
// Our singleton
struct impl_OCNull {
    OCBase base;
};
static struct impl_OCNull impl_kOCNull = {
    .base = {
        .typeID = kOCNotATypeID,
        .retainCount = 1,
        .finalize = impl_OCNullFinalize,
        .equal = impl_OCNullEqual,
        .copyFormattingDesc = impl_OCNullCopyFormattingDesc,
        .copyJSON = impl_OCNullCopyJSON,
        .copyDeep = impl_OCNullDeepCopy,
        .copyDeepMutable = impl_OCNullDeepCopy,
        .flags = {
            .static_instance = 1,
            .finalized = 0,
            .tracked = 0,
            .reserved = 0}}};
const OCNullRef kOCNull = &impl_kOCNull;
// Equality callback: same pointer (there's only one null)
static bool impl_OCNullEqual(const void *v1, const void *v2) {
    return v1 == v2;
}
// No-op finalizer
static void impl_OCNullFinalize(const void *unused) {
    (void)unused;
}
static OCStringRef impl_OCNullCopyFormattingDesc(OCTypeRef cf) {
    if (!cf) return NULL;
    OCBase *base = (OCBase *)cf;
    if (base->typeID != OCNullGetTypeID()) {
        fprintf(stderr, "[OCNullCopyFormattingDesc] Invalid typeID %u\n", base->typeID);
        return NULL;
    }
    return OCStringCreateWithCString("null");
}
// JSON serializer for our singleton:
static cJSON *
impl_OCNullCopyJSON(const void *cf, bool typed, OCStringRef *outError) {
    return OCNullCopyAsJSON((OCNullRef)cf, typed, outError);
}
static void *
impl_OCNullDeepCopy(const void *cf) {
    // null is a singleton, so "deep copy" is just the same object:
    return (void *)cf;
}
// Made impl_OCNullInitialize externally visible
void impl_OCNullInitialize(void) {
    kOCNullTypeID = OCRegisterType("OCNull", (OCTypeRef (*)(cJSON *, OCStringRef *))OCNullCreateFromJSON);
    impl_kOCNull.base.typeID = kOCNullTypeID;
}
OCTypeID OCNullGetTypeID(void) {
    return kOCNullTypeID;
}
OCNullRef OCNullGet(void) {
    return kOCNull;
}
bool OCNullIsNull(OCTypeRef obj) {
    return obj == (OCTypeRef)kOCNull;
}
cJSON *OCNullCopyAsJSON(OCNullRef null, bool typed, OCStringRef *outError) {
    (void)typed;  // Unused - null is always serialized the same way
    if (outError) *outError = NULL;
    if (!null || null != kOCNull) {
        if (outError) *outError = STR("Invalid OCNull object");
        return NULL;
    }
    // Always serialize as JSON null - unambiguous representation
    return cJSON_CreateNull();
}
OCNullRef OCNullCreateFromJSON(cJSON *json, OCStringRef *outError) {
    if (outError) *outError = NULL;
    if (!json) {
        if (outError) *outError = STR("JSON object is NULL");
        return NULL;
    }
    if (cJSON_IsNull(json)) {
        return kOCNull;
    }
    if (outError) *outError = STR("JSON object is not null type");
    return NULL;
}
// Automatically initialize null type at load time
__attribute__((constructor)) static void OCNullModuleInitialize(void) {
    impl_OCNullInitialize();
}
