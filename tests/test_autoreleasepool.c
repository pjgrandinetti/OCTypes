#include "test_autoreleasepool.h"
#include "../src/OCAutoreleasePool.h"
#include "../src/OCString.h" // For OCStringCreateWithCString, OCAutorelease, OCRelease

// Original autoreleasePoolTest0 implementation
bool autoreleasePoolTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool ok;
    OCAutoreleasePoolRef p1 = OCAutoreleasePoolCreate(); 
    if(!p1) PRINTERROR;
    ok = OCAutoreleasePoolRelease(p1); 
    if(!ok) PRINTERROR;
    
    OCAutoreleasePoolRef p2 = OCAutoreleasePoolCreate(); 
    if(!p2) PRINTERROR; // Added check for p2
    OCStringRef s2 = OCStringCreateWithCString("t");
    if(!s2) { // Added check for s2
        OCAutoreleasePoolRelease(p2);
        PRINTERROR;
    }
    if (OCAutorelease(s2)!=s2) {
        OCRelease(s2); // Release s2 if OCAutorelease fails
        OCAutoreleasePoolRelease(p2);
        PRINTERROR;
    }
    // s2 is now in the pool, OCAutoreleasePoolRelease should handle its release.
    ok = OCAutoreleasePoolRelease(p2); 
    if(!ok) PRINTERROR;
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}
