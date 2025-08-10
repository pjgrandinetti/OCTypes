/**
 * @file OCLeakTracker.h
 * @brief Internal leak tracking API for OCTypeRef-based objects.
 *
 * This module provides debug-only utilities to track allocations and
 * identify objects that were never finalized (i.e., leaked).
 */
#ifndef OC_LEAKTRACKER_H
#define OC_LEAKTRACKER_H
#include <stddef.h>
#include "OCType.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Track a newly allocated object for leak detection.
 *
 * Should be called immediately after allocation (e.g., in OCTypeAllocate).
 *
 * @param ptr Pointer to the allocated object.
 */
void impl_OCTrack(const void *ptr);
/**
 * @brief Track an object with an allocation hint for better debugging.
 *
 * @param ptr Pointer to the allocated object.
 * @param hint String literal hint about where the allocation occurred.
 */
void impl_OCTrackWithHint(const void *ptr, const char *hint);
/**
 * @brief Untrack an object upon finalization.
 *
 * Call this in the finalize function or final release path.
 *
 * @param ptr Pointer to the object being destroyed.
 */
void impl_OCUntrack(const void *ptr);
/**
 * @brief Report all currently tracked (unfinalized) objects.
 *
 * Call this at test teardown or via `atexit()` for memory leak reporting.
 */
void OCReportLeaks(void);
/**
 * @brief Report all leaks with detailed stack traces and allocation context.
 *
 * This provides much more detailed information than OCReportLeaks(), including
 * stack traces showing where each leaked object was allocated.
 */
void OCReportLeaksDetailed(void);
void OCReportLeaksForType(OCTypeID filterTypeID);
/**
 * @brief Report leaks for a specific type with detailed stack traces.
 *
 * @param filterTypeID The type ID to filter for.
 */
void OCReportLeaksForTypeDetailed(OCTypeID filterTypeID);
size_t OCLeakCountForType(OCTypeID typeID);
/**
 * @brief Get the count of actual memory leaks for a specific type (excluding static instances).
 *
 * Static instances (like constant strings) are not counted as leaks since they're
 * intentionally kept until OCTypesShutdown().
 *
 * @param typeID The type ID to count leaks for.
 * @return The number of actual leaked objects of the specified type.
 */
size_t OCActualLeakCountForType(OCTypeID typeID);
/**
 * @brief Report all leaks excluding static instances.
 *
 * Static instances (like constant strings) are not reported as leaks since they're
 * intentionally kept until OCTypesShutdown().
 */
void OCReportLeaksExcludingStatic(void);
/**
 * @brief Report leaks for a specific type excluding static instances.
 *
 * @param typeID The type ID to filter for.
 */
void OCReportLeaksForTypeExcludingStatic(OCTypeID typeID);
/**
 * @brief Report leaks for a specific type excluding static instances with detailed stack traces.
 *
 * @param filterTypeID The type ID to filter for.
 */
void OCReportLeaksForTypeDetailedExcludingStatic(OCTypeID filterTypeID);
// Convenience macros for easier debugging
#define OCTrack(ptr) impl_OCTrack(ptr)
#define OCTrackWithHint(ptr, hint) impl_OCTrackWithHint(ptr, hint)
// Macro to automatically include function name as hint
#define OCTrackWithFunc(ptr) impl_OCTrackWithHint(ptr, __func__)
#ifdef __cplusplus
}
#endif
#endif  // OC_LEAKTRACKER_H
