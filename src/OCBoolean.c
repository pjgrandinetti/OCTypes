//
//  OCBoolean.c
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//  Updated by ChatGPT on 2025-05-11.
//
#include <stdio.h>
#include "OCType.h"  // for OCRegisterType, OCTypeID, kOCNotATypeID
#include "OCTypes.h"
// Static storage of the boolean type’s OCTypeID
static OCTypeID kOCBooleanTypeID = kOCNotATypeID;
// Our two singletons
struct impl_OCBoolean {
    OCBase base;
};
static struct impl_OCBoolean impl_kOCBooleanTrue = {
    .base = {
        kOCNotATypeID, 0, 0, NULL, NULL}};
static struct impl_OCBoolean impl_kOCBooleanFalse = {
    .base = {
        kOCNotATypeID, 0, 0, NULL, NULL}};
const OCBooleanRef kOCBooleanTrue = &impl_kOCBooleanTrue;
const OCBooleanRef kOCBooleanFalse = &impl_kOCBooleanFalse;
// Equality callback: same pointer
static bool impl_OCBooleanEqual(const void* v1, const void* v2) {
    return v1 == v2;
}
// No-op finalizer
static void impl_OCBooleanFinalize(const void* unused) {
    (void)unused;
}
static OCStringRef impl_OCBooleanCopyFormattingDesc(OCTypeRef cf) {
    if (!cf) return NULL;
    OCBase* base = (OCBase*)cf;
    if (base->typeID != OCBooleanGetTypeID()) {
        fprintf(stderr, "[OCBooleanCopyFormattingDesc] Invalid typeID %u\n", base->typeID);
        return NULL;
    }
    bool value = (cf == (OCTypeRef)kOCBooleanTrue);
    return OCStringCreateWithCString(value ? "true" : "false");
}
// JSON‐serializer for our two singletons:
static cJSON*
impl_OCBooleanCopyJSON(const void* cf, bool typed, OCStringRef* outError) {
    return OCBooleanCopyAsJSON((OCBooleanRef)cf, typed, outError);
}
static void*
impl_OCBooleanDeepCopy(const void* cf) {
    // booleans are singletons, so “deep copy” is just the same object:
    return (void*)cf;
}
// Made impl_OCBooleanInitialize externally visible
void impl_OCBooleanInitialize(void) {
    kOCBooleanTypeID = OCRegisterType("OCBoolean", (OCTypeRef (*)(cJSON*, OCStringRef*))OCBooleanCreateFromJSON);
    impl_kOCBooleanTrue.base.typeID = kOCBooleanTypeID;
    impl_kOCBooleanTrue.base.finalize = impl_OCBooleanFinalize;
    impl_kOCBooleanTrue.base.equal = impl_OCBooleanEqual;
    impl_kOCBooleanTrue.base.copyFormattingDesc = impl_OCBooleanCopyFormattingDesc;
    impl_kOCBooleanTrue.base.copyJSON = impl_OCBooleanCopyJSON;
    impl_kOCBooleanTrue.base.copyDeep = impl_OCBooleanDeepCopy;
    impl_kOCBooleanTrue.base.copyDeepMutable = impl_OCBooleanDeepCopy;
    impl_kOCBooleanTrue.base.flags.finalized = false;
    OCTypeSetStaticInstance(kOCBooleanTrue, true);
    impl_kOCBooleanFalse.base.typeID = kOCBooleanTypeID;
    impl_kOCBooleanFalse.base.finalize = impl_OCBooleanFinalize;
    impl_kOCBooleanFalse.base.equal = impl_OCBooleanEqual;
    impl_kOCBooleanFalse.base.copyFormattingDesc = impl_OCBooleanCopyFormattingDesc;
    impl_kOCBooleanFalse.base.copyJSON = impl_OCBooleanCopyJSON;
    impl_kOCBooleanFalse.base.copyDeep = impl_OCBooleanDeepCopy;
    impl_kOCBooleanFalse.base.copyDeepMutable = impl_OCBooleanDeepCopy;
    impl_kOCBooleanFalse.base.flags.finalized = false;
    OCTypeSetStaticInstance(kOCBooleanFalse, true);
}
OCTypeID OCBooleanGetTypeID(void) {
    return kOCBooleanTypeID;
}
bool OCBooleanGetValue(OCBooleanRef b) {
    return (b == kOCBooleanTrue);
}
OCBooleanRef OCBooleanGetWithBool(bool value) {
    return value ? kOCBooleanTrue : kOCBooleanFalse;
}
OCStringRef OCBooleanCreateStringValue(OCBooleanRef boolean) {
    return OCStringCreateWithCString(OCBooleanGetValue(boolean) ? "true" : "false");
}
cJSON* OCBooleanCopyAsJSON(OCBooleanRef boolean, bool typed, OCStringRef* outError) {
    (void)typed;  // Booleans are native JSON types, no typed wrapping needed
    if (outError) *outError = NULL;
    if (!boolean) return cJSON_CreateNull();
    // Booleans are native JSON types, no wrapping needed even for typed serialization
    return (boolean == kOCBooleanTrue) ? cJSON_CreateTrue() : cJSON_CreateFalse();
}
OCBooleanRef OCBooleanCreateFromJSON(cJSON* json, OCStringRef* outError) {
    if (outError) *outError = NULL;
    if (!json) {
        if (outError) *outError = STR("JSON is NULL");
        return NULL;
    }
    if (cJSON_IsTrue(json)) return kOCBooleanTrue;
    if (cJSON_IsFalse(json)) return kOCBooleanFalse;
    if (outError) *outError = STR("JSON is not a boolean value");
    return NULL;
}
// Automatically initialize boolean type at load time
__attribute__((constructor)) static void OCBooleanModuleInitialize(void) {
    impl_OCBooleanInitialize();
}
