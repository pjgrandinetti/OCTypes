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
#include "OCLibrary.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Track a newly allocated object for leak detection.
 *
 * Should be called immediately after allocation (e.g., in OCTypeAllocate).
 *
 * @param ptr Pointer to the allocated object.
 * @param file Source file where allocation occurred.
 * @param line Source line of allocation.
 */
void _OCTrackDebug(const void *ptr, const char *file, int line);
/**
 * @brief Untrack an object upon finalization.
 *
 * Call this in the finalize function or final release path.
 *
 * @param ptr Pointer to the object being destroyed.
 */
void _OCUntrack(const void *ptr);
/**
 * @brief Report all currently tracked (unfinalized) objects.
 *
 * Call this at test teardown or via `atexit()` for memory leak reporting.
 */
void OCReportLeaks(void);
void OCReportLeaksForType(OCTypeID filterTypeID);
size_t OCLeakCountForType(OCTypeID typeID);
#ifdef __cplusplus
}
#endif
#endif  // OC_LEAKTRACKER_H
