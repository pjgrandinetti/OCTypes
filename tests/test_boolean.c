#include "test_boolean.h"
#include "../src/OCBoolean.h"

// Original booleanTest0 implementation
bool booleanTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    // singletons
    if (kOCBooleanTrue == NULL) PRINTERROR;
    if (kOCBooleanFalse == NULL) PRINTERROR;
    if (kOCBooleanTrue == kOCBooleanFalse) PRINTERROR;
    // value
    if (OCBooleanGetValue(kOCBooleanTrue) != true) PRINTERROR;
    if (OCBooleanGetValue(kOCBooleanFalse) != false) PRINTERROR;
    // type IDs
    OCTypeID trueID = OCGetTypeID(kOCBooleanTrue);
    OCTypeID falseID = OCGetTypeID(kOCBooleanFalse);
    OCTypeID directID = OCBooleanGetTypeID(); // This should register the type if not already
    if (trueID == _kOCNotATypeID) PRINTERROR;
    if (falseID == _kOCNotATypeID) PRINTERROR;
    if (directID == _kOCNotATypeID) PRINTERROR;
    if (trueID != falseID) PRINTERROR;
    if (trueID != directID) PRINTERROR;
    // equality using OCTypeEqual (which should internally use OCTypeEqual or specific type equal)
    if (!OCTypeEqual(kOCBooleanTrue,  kOCBooleanTrue))  PRINTERROR;
    if (!OCTypeEqual(kOCBooleanFalse, kOCBooleanFalse)) PRINTERROR;
    if ( OCTypeEqual(kOCBooleanTrue,  kOCBooleanFalse)) PRINTERROR;
    // retain/release no-ops (singletons are not managed by retain count)
    OCRetain(kOCBooleanTrue);
    OCRetain(kOCBooleanFalse);
    OCRelease(kOCBooleanTrue); // Should be a no-op, not deallocate
    OCRelease(kOCBooleanFalse); // Should be a no-op, not deallocate
    // Verify they are still valid after retain/release attempts
    if (OCBooleanGetValue(kOCBooleanTrue) != true)  PRINTERROR;
    if (OCBooleanGetValue(kOCBooleanFalse) != false) PRINTERROR;
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}
