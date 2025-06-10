#include "test_type.h"
#include "../src/OCType.h" // For OCRegisterType, _kOCNotATypeID
#include <string.h> // for strcmp needed below

// Original typeTest0 implementation
bool typeTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCTypeID tid = OCRegisterType("MyType");
    if (tid == _kOCNotATypeID) PRINTERROR;
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

// New: Test OCTypeCopyFormattingDesc and OCCopyDescription on multiple types
bool typeTest1(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    // Use a known type: OCString
    OCStringRef s = OCStringCreateWithCString("testdesc");
    ASSERT_NOT_NULL(s, "OCStringCreateWithCString should not return NULL");
    OCTypeID tid1 = OCGetTypeID(s);
    ASSERT_TRUE(tid1 != _kOCNotATypeID, "OCGetTypeID should return valid ID");
    OCStringRef desc1 = OCTypeCopyFormattingDesc((void*)s);
    ASSERT_NOT_NULL(desc1, "OCTypeCopyFormattingDesc should not return NULL");
    OCStringRef desc2 = OCCopyDescription((void*)s);
    ASSERT_NOT_NULL(desc2, "OCCopyDescription should not return NULL");

    // Verify formatting description matches the string content
    const char *fmt = OCStringGetCString(desc1);
    ASSERT_TRUE(strcmp(fmt, "testdesc") == 0, "FormattingDesc should match original string");
    // Verify copy description matches the type name
    const char *typeName = OCStringGetCString(desc2);
    ASSERT_TRUE(strcmp(typeName, "OCString") == 0, "CopyDescription should match type name");

    OCRelease(desc1);
    OCRelease(desc2);
    OCRelease(s);
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}


// after typeTest1, add:

bool typeTest2(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = true;

    OCStringRef orig = OCStringCreateWithCString("deepcopy!");
    if (!orig) return false;

    OCStringRef copy = (OCStringRef)OCTypeDeepCopy(orig);
    if (!copy || copy == orig || !OCTypeEqual(copy, orig)) {
        success = false;
    }
    OCRelease(copy);

    OCMutableStringRef mcopy = (OCMutableStringRef)OCTypeDeepCopyMutable(orig);
    if (!mcopy || (OCStringRef)mcopy == orig || !OCTypeEqual(mcopy, orig)) {
        success = false;
    }

    OCStringAppendCString(mcopy, "?");
    if (OCTypeEqual(mcopy, orig)) {
        success = false;
    }

    OCRelease(mcopy);
    OCRelease(orig);

    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with failures");
    return success;
}
