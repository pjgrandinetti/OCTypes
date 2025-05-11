#include "test_type.h"
#include "../src/OCType.h" // For OCRegisterType, _kOCNotATypeID

// Original typeTest0 implementation
bool typeTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCTypeID tid = OCRegisterType("MyType");
    if (tid == _kOCNotATypeID) PRINTERROR;
    // To make this test more robust, we could try to register another type
    // or verify something about the returned tid, but for now, it matches original.
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}
