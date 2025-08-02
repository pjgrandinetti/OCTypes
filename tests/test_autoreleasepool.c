#include "test_autoreleasepool.h"
#include "../src/OCAutoreleasePool.h"
#include "../src/OCString.h"  // For OCStringCreateWithCString, OCAutorelease, OCRelease
// Original autoreleasePoolTest0 implementation
bool autoreleasePoolTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool ok;
    OCAutoreleasePoolRef p1 = OCAutoreleasePoolCreate();
    if (!p1) PRINTERROR;
    ok = OCAutoreleasePoolRelease(p1);
    if (!ok) PRINTERROR;
    OCAutoreleasePoolRef p2 = OCAutoreleasePoolCreate();
    if (!p2) PRINTERROR;
    OCStringRef s2 = OCStringCreateWithCString("t");
    if (s2 == NULL) {
        OCAutoreleasePoolRelease(p2);
        PRINTERROR;
    }
    // Add s2 to pool — no need for return check or manual release
    OCAutorelease(s2);
    ok = OCAutoreleasePoolRelease(p2);
    if (!ok) PRINTERROR;
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}
// New: Test autorelease pool drain functionality
bool autoreleasePoolTest1(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCAutoreleasePoolRef pool = OCAutoreleasePoolCreate();
    if (!pool) PRINTERROR;
    // Create an object and autorelease it
    OCStringRef s = OCStringCreateWithCString("drain_test");
    if (!s) {
        OCAutoreleasePoolRelease(pool);
        PRINTERROR;
    }
    if (OCAutorelease(s) != s) {
        OCRelease(s);
        OCAutoreleasePoolRelease(pool);
        PRINTERROR;
    }
    // Drain the pool, releasing s
    OCAutoreleasePoolDrain(pool);
    // Now pool still valid, release it
    if (!OCAutoreleasePoolRelease(pool)) PRINTERROR;
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}
