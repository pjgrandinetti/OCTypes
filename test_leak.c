/* Test OCLeakTracker functions are getting compiled */
#include "OCLeakTracker.h"
void impl_OCTrack(const void *ptr) {
    (void)ptr;  // Just to test
}
void impl_OCUntrack(const void *ptr) {
    (void)ptr;  // Just to test
}
void OCReportLeaks(void) {
    // Just to test
}
void OCReportLeaksForType(OCTypeID filterTypeID) {
    (void)filterTypeID;  // Just to test
}
size_t OCLeakCountForType(OCTypeID typeID) {
    (void)typeID;
    return 0;  // Just to test
}
