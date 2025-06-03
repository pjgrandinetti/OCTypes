//
//  OCBoolean.c
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//  Updated by ChatGPT on 2025-05-11.
//
#include <stdio.h>
#include "OCLibrary.h"
#include "OCType.h" // for OCRegisterType, OCTypeID, _kOCNotATypeID

// Static storage of the boolean type’s OCTypeID
static OCTypeID kOCBooleanTypeID = _kOCNotATypeID;

// Our two singletons
struct __OCBoolean {
    OCBase _base;
};

static struct __OCBoolean __kOCBooleanTrue = {
    ._base = {
        _kOCNotATypeID, 0, NULL, NULL, NULL
    }
};
static struct __OCBoolean __kOCBooleanFalse = {
    ._base = {
        _kOCNotATypeID, 0, NULL, NULL, NULL
    }
};

const OCBooleanRef kOCBooleanTrue  = &__kOCBooleanTrue;
const OCBooleanRef kOCBooleanFalse = &__kOCBooleanFalse;

// Equality callback: same pointer
static bool __OCBooleanEqual(const void *v1, const void *v2) {
    return v1 == v2;
}

// No-op finalizer
static void __OCBooleanFinalize(const void *unused) {
    (void)unused;
}

static OCStringRef __OCBooleanCopyFormattingDesc(OCTypeRef cf)
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


// Made _OCBooleanInitialize externally visible
void _OCBooleanInitialize(void) {
    kOCBooleanTypeID = OCRegisterType("OCBoolean");

    __kOCBooleanTrue._base.typeID   = kOCBooleanTypeID;
    __kOCBooleanTrue._base.finalize = __OCBooleanFinalize;
    __kOCBooleanTrue._base.equal    = __OCBooleanEqual;
    __kOCBooleanTrue._base.copyFormattingDesc = __OCBooleanCopyFormattingDesc;
    __kOCBooleanTrue._base.finalized = false;
    OCTypeSetStaticInstance(kOCBooleanTrue, true);

    __kOCBooleanFalse._base.typeID   = kOCBooleanTypeID;
    __kOCBooleanFalse._base.finalize = __OCBooleanFinalize;
    __kOCBooleanFalse._base.equal    = __OCBooleanEqual;
    __kOCBooleanFalse._base.copyFormattingDesc = __OCBooleanCopyFormattingDesc;
    __kOCBooleanFalse._base.finalized = false;
    OCTypeSetStaticInstance(kOCBooleanFalse, true);
}

OCTypeID OCBooleanGetTypeID(void) {
    return kOCBooleanTypeID;
}

bool OCBooleanGetValue(OCBooleanRef b) {
    return (b == kOCBooleanTrue);
}

OCStringRef OCBooleanCreateStringValue(OCBooleanRef boolean) {
    return OCStringCreateWithCString(OCBooleanGetValue(boolean) ? "true" : "false");
}

// Automatically initialize boolean type at load time
__attribute__((constructor))
static void OCBooleanModuleInitialize(void) {
    _OCBooleanInitialize();
}
