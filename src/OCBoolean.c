//
//  OCBoolean.c
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//  Updated by ChatGPT on 2025-05-11.
//
#include <stdio.h>
#include "OCLibrary.h"
#include "OCType.h" // for OCRegisterType, OCTypeID, kOCNotATypeID

// Static storage of the boolean type’s OCTypeID
static OCTypeID kOCBooleanTypeID = kOCNotATypeID;

// Our two singletons
struct impl_OCBoolean {
    OCBase base;
};

static struct impl_OCBoolean impl_kOCBooleanTrue = {
    .base = {
        kOCNotATypeID, 0, NULL, NULL, NULL
    }
};
static struct impl_OCBoolean impl_kOCBooleanFalse = {
    .base = {
        kOCNotATypeID, 0, NULL, NULL, NULL
    }
};

const OCBooleanRef kOCBooleanTrue  = &impl_kOCBooleanTrue;
const OCBooleanRef kOCBooleanFalse = &impl_kOCBooleanFalse;

// Equality callback: same pointer
static bool impl_OCBooleanEqual(const void *v1, const void *v2) {
    return v1 == v2;
}

// No-op finalizer
static void impl_OCBooleanFinalize(const void *unused) {
    (void)unused;
}

static OCStringRef impl_OCBooleanCopyFormattingDesc(OCTypeRef cf)
{
    if (!cf) return NULL;
    OCBase *base = (OCBase *)cf;
    if (base->typeID != OCBooleanGetTypeID()) {
        fprintf(stderr, "[OCBooleanCopyFormattingDesc] Invalid typeID %u\n", base->typeID);
        return NULL;
    }

    bool value = (cf == (OCTypeRef)kOCBooleanTrue);
    return OCStringCreateWithCString(value ? "true" : "false");
}

// JSON‐serializer for our two singletons:
static cJSON *
impl_OCBooleanCopyJSON(const void *cf)
{
    if (!cf) return cJSON_CreateNull();
    return (cf == (OCTypeRef)kOCBooleanTrue)
         ? cJSON_CreateTrue()
         : cJSON_CreateFalse();
}

static void *
impl_OCBooleanDeepCopy(const void *cf)
{
    // booleans are singletons, so “deep copy” is just the same object:
    return (void*)cf;
}

// Made _OCBooleanInitialize externally visible
void _OCBooleanInitialize(void) {
    kOCBooleanTypeID = OCRegisterType("OCBoolean");

    impl_kOCBooleanTrue.base.typeID   = kOCBooleanTypeID;
    impl_kOCBooleanTrue.base.finalize = impl_OCBooleanFinalize;
    impl_kOCBooleanTrue.base.equal    = impl_OCBooleanEqual;
    impl_kOCBooleanTrue.base.copyFormattingDesc = impl_OCBooleanCopyFormattingDesc;
    impl_kOCBooleanTrue.base.copyJSON = impl_OCBooleanCopyJSON;
    impl_kOCBooleanTrue.base.copyDeep        = impl_OCBooleanDeepCopy;
    impl_kOCBooleanTrue.base.copyDeepMutable = impl_OCBooleanDeepCopy;
    impl_kOCBooleanTrue.base.finalized = false;
    OCTypeSetStaticInstance(kOCBooleanTrue, true);

    impl_kOCBooleanFalse.base.typeID   = kOCBooleanTypeID;
    impl_kOCBooleanFalse.base.finalize = impl_OCBooleanFinalize;
    impl_kOCBooleanFalse.base.equal    = impl_OCBooleanEqual;
    impl_kOCBooleanFalse.base.copyFormattingDesc = impl_OCBooleanCopyFormattingDesc;
    impl_kOCBooleanFalse.base.copyJSON = impl_OCBooleanCopyJSON;
    impl_kOCBooleanTrue.base.copyDeep        = impl_OCBooleanDeepCopy;
    impl_kOCBooleanTrue.base.copyDeepMutable = impl_OCBooleanDeepCopy;
    impl_kOCBooleanFalse.base.finalized = false;
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

cJSON *OCBooleanCreateJSON(OCBooleanRef boolean) {
    if (!boolean) return cJSON_CreateNull();
    return (boolean == kOCBooleanTrue) ? cJSON_CreateTrue() : cJSON_CreateFalse();
}

OCBooleanRef OCBooleanCreateFromJSON(cJSON *json) {
    if (!json) return NULL;
    if (cJSON_IsTrue(json)) return kOCBooleanTrue;
    if (cJSON_IsFalse(json)) return kOCBooleanFalse;
    return NULL;
}

// Automatically initialize boolean type at load time
__attribute__((constructor))
static void OCBooleanModuleInitialize(void) {
    _OCBooleanInitialize();
}
