/* OCLeakTracker.c */
#include "OCLeakTracker.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OCType.h" /* For OCBase, OCTypeID, etc. */
// Stack trace support
#ifdef __APPLE__
#include <execinfo.h>
#define HAVE_BACKTRACE 1
#elif defined(__linux__)
#include <execinfo.h>
#define HAVE_BACKTRACE 1
#elif defined(_WIN32)
#include <dbghelp.h>
#include <windows.h>
#define HAVE_BACKTRACE 1
#else
#define HAVE_BACKTRACE 0
#endif
#define MAX_STACK_FRAMES 16
typedef struct {
    const void *ptr;
    void *stack_frames[MAX_STACK_FRAMES];
    int stack_depth;
    const char *allocation_hint;  // Optional hint about allocation context
} OCLeakEntry;
static OCLeakEntry *gLeakTable = NULL;
static size_t gLeakCount = 0;
static pthread_mutex_t gLeakLock = PTHREAD_MUTEX_INITIALIZER;
// Capture stack trace for leak tracking
static int capture_stack_trace(void **buffer, int max_frames) {
#if HAVE_BACKTRACE
#if defined(__APPLE__) || defined(__linux__)
    return backtrace(buffer, max_frames);
#elif defined(_WIN32)
    // Windows implementation would go here
    return CaptureStackBackTrace(0, max_frames, buffer, NULL);
#endif
#else
    (void)buffer;
    (void)max_frames;
    return 0;
#endif
}
// Print stack trace for debugging
static void print_stack_trace(void **frames, int depth) {
#if HAVE_BACKTRACE && (defined(__APPLE__) || defined(__linux__))
    char **symbols = backtrace_symbols(frames, depth);
    if (symbols) {
        for (int i = 0; i < depth; i++) {
            // Skip the first few frames which are just leak tracker internals
            if (i >= 2) {
                fprintf(stderr, "    %s\n", symbols[i]);
            }
        }
        free(symbols);
    }
#else
    (void)frames;
    (void)depth;
    fprintf(stderr, "    (stack trace not available on this platform)\n");
#endif
}
void impl_OCTrack(const void *ptr) {
    pthread_mutex_lock(&gLeakLock);
    gLeakTable = realloc(gLeakTable, (gLeakCount + 1) * sizeof(OCLeakEntry));
    if (!gLeakTable) {
        fprintf(stderr, "[LeakTracker] realloc failed\n");
        abort();
    }
    OCLeakEntry *entry = &gLeakTable[gLeakCount++];
    entry->ptr = ptr;
    entry->allocation_hint = NULL;
    // Capture stack trace
    entry->stack_depth = capture_stack_trace(entry->stack_frames, MAX_STACK_FRAMES);
    pthread_mutex_unlock(&gLeakLock);
}
void impl_OCTrackWithHint(const void *ptr, const char *hint) {
    pthread_mutex_lock(&gLeakLock);
    gLeakTable = realloc(gLeakTable, (gLeakCount + 1) * sizeof(OCLeakEntry));
    if (!gLeakTable) {
        fprintf(stderr, "[LeakTracker] realloc failed\n");
        abort();
    }
    OCLeakEntry *entry = &gLeakTable[gLeakCount++];
    entry->ptr = ptr;
    entry->allocation_hint = hint;  // Store the hint (should be string literal)
    // Capture stack trace
    entry->stack_depth = capture_stack_trace(entry->stack_frames, MAX_STACK_FRAMES);
    pthread_mutex_unlock(&gLeakLock);
}
void impl_OCUntrack(const void *ptr) {
    pthread_mutex_lock(&gLeakLock);
    for (size_t i = 0; i < gLeakCount; ++i) {
        if (gLeakTable[i].ptr == ptr) {
            gLeakTable[i] = gLeakTable[--gLeakCount];
            break;
        }
    }
    pthread_mutex_unlock(&gLeakLock);
}
void OCReportLeaks(void) {
    // Do nothing under AddressSanitizer:
#if !__has_feature(address_sanitizer)
    pthread_mutex_lock(&gLeakLock);
    if (gLeakCount == 0) {
        // Silent success - no need to report when everything is working correctly
        pthread_mutex_unlock(&gLeakLock);
        return;
    }
    typedef struct {
        OCTypeID typeID;
        const char *typeName;
        size_t count;
        size_t staticCount;
    } LeakSummary;
    size_t capacity = 16;
    size_t typeCount = 0;
    LeakSummary *summaries = malloc(capacity * sizeof(LeakSummary));
    if (!summaries) {
        fprintf(stderr, "[OCLeakTracker] ERROR: Failed to allocate memory for leak summary.\n");
        pthread_mutex_unlock(&gLeakLock);
        return;
    }
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = gLeakTable[i].ptr;
        if (!base) continue;
        OCTypeID tid = base->typeID;
        const char *typeName = OCTypeIDName(base);
        if (!typeName) typeName = "(unknown)";
        // Look for existing summary entry
        size_t j;
        for (j = 0; j < typeCount; ++j) {
            if (summaries[j].typeID == tid) {
                summaries[j].count++;
                if (base->flags.static_instance) summaries[j].staticCount++;
                break;
            }
        }
        // If it's a new typeID, add a new summary slot
        if (j == typeCount) {
            if (typeCount == capacity) {
                capacity *= 2;
                LeakSummary *newSummaries =
                    realloc(summaries, capacity * sizeof(LeakSummary));
                if (!newSummaries) {
                    fprintf(stderr, "[OCLeakTracker] ERROR: Failed to realloc memory.\n");
                    free(summaries);
                    pthread_mutex_unlock(&gLeakLock);
                    return;
                }
                summaries = newSummaries;
            }
            summaries[typeCount].typeID = tid;
            summaries[typeCount].typeName = typeName;
            summaries[typeCount].count = 1;
            summaries[typeCount].staticCount = base->flags.static_instance ? 1 : 0;
            typeCount++;
        }
    }
    fprintf(stderr,
            "\n[OCLeakTracker] %zu object(s) not finalized, grouped by type:\n",
            gLeakCount);
    for (size_t i = 0; i < typeCount; ++i) {
        if (summaries[i].staticCount > 0) {
            fprintf(stderr, "  %s: %zu (%zu static)\n",
                    summaries[i].typeName,
                    summaries[i].count,
                    summaries[i].staticCount);
        } else {
            fprintf(stderr, "  %s: %zu\n",
                    summaries[i].typeName,
                    summaries[i].count);
        }
    }
    free(summaries);
    pthread_mutex_unlock(&gLeakLock);
#endif  // !__has_feature(address_sanitizer)
}
void OCReportLeaksDetailed(void) {
    // Do nothing under AddressSanitizer:
#if !__has_feature(address_sanitizer)
    pthread_mutex_lock(&gLeakLock);
    if (gLeakCount == 0) {
        // Silent success - no need to report when everything is working correctly
        pthread_mutex_unlock(&gLeakLock);
        return;
    }
    // Count actual leaks (non-static instances)
    size_t actual_leaks = 0;
    size_t static_instances = 0;
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = gLeakTable[i].ptr;
        if (!base) continue;
        if (base->flags.static_instance) {
            static_instances++;
        } else {
            actual_leaks++;
        }
    }
    if (actual_leaks == 0 && static_instances == 0) {
        pthread_mutex_unlock(&gLeakLock);
        return;
    }
    fprintf(stderr, "\n[OCLeakTracker] DETAILED LEAK REPORT\n");
    if (actual_leaks > 0) {
        fprintf(stderr, "Found %zu ACTUAL LEAK(S) with stack traces:\n", actual_leaks);
    }
    if (static_instances > 0) {
        fprintf(stderr, "Found %zu static instance(s) (expected, not leaks)\n", static_instances);
    }
    fprintf(stderr, "\n");
    size_t leak_index = 1;
    // First, report actual leaks (non-static instances)
    if (actual_leaks > 0) {
        fprintf(stderr, "=== ACTUAL MEMORY LEAKS ===\n");
        for (size_t i = 0; i < gLeakCount; ++i) {
            const OCBase *base = gLeakTable[i].ptr;
            if (!base || base->flags.static_instance) continue;
            const char *typeName = OCTypeIDName(base);
            if (!typeName) typeName = "(unknown)";
            fprintf(stderr, "LEAK #%zu: %s (typeID %u) at %p\n",
                    leak_index++, typeName, (unsigned)base->typeID, gLeakTable[i].ptr);
            if (gLeakTable[i].allocation_hint) {
                fprintf(stderr, "  Allocation hint: %s\n", gLeakTable[i].allocation_hint);
            }
            if (gLeakTable[i].stack_depth > 0) {
                fprintf(stderr, "  Allocation stack trace:\n");
                print_stack_trace(gLeakTable[i].stack_frames, gLeakTable[i].stack_depth);
            } else {
                fprintf(stderr, "  (no stack trace available)\n");
            }
            fprintf(stderr, "\n");
        }
    }
    // Then, optionally report static instances (commented out by default to reduce noise)
    /*
    if (static_instances > 0) {
        fprintf(stderr, "=== STATIC INSTANCES (NOT LEAKS) ===\n");
        size_t static_index = 1;
        for (size_t i = 0; i < gLeakCount; ++i) {
            const OCBase *base = gLeakTable[i].ptr;
            if (!base || !base->flags.static_instance) continue;

            const char *typeName = OCTypeIDName(base);
            if (!typeName) typeName = "(unknown)";

            fprintf(stderr, "STATIC #%zu: %s (typeID %u) at %p\n",
                    static_index++, typeName, (unsigned)base->typeID, gLeakTable[i].ptr);

            if (gLeakTable[i].allocation_hint) {
                fprintf(stderr, "  Allocation hint: %s\n", gLeakTable[i].allocation_hint);
            }
            fprintf(stderr, "  (Static instance - cleaned up at OCTypesShutdown)\n\n");
        }
    }
    */
    pthread_mutex_unlock(&gLeakLock);
#endif  // !__has_feature(address_sanitizer)
}
void OCReportLeaksForType(OCTypeID filterTypeID) {
    pthread_mutex_lock(&gLeakLock);
    // Count how many leaked entries match filterTypeID
    size_t filteredCount = 0;
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = gLeakTable[i].ptr;
        if (!base) continue;
        if (base->typeID == filterTypeID) {
            filteredCount++;
        }
    }
    if (filteredCount == 0) {
        // Check if this looks like an invalid type ID
        const char *typeName = OCTypeNameFromTypeID(filterTypeID);
        bool hasInvalidTypeName = false;
        // If there's at least one instance in memory, grab its name.
        for (size_t i = 0; i < gLeakCount; ++i) {
            const OCBase *base = gLeakTable[i].ptr;
            if (base && base->typeID == filterTypeID) {
                typeName = OCTypeIDName(base);
                break;
            }
        }
        // Check if the type name suggests this is an invalid type ID
        if (typeName && (strstr(typeName, "InvalidTypeID") || strstr(typeName, "invalid") || strstr(typeName, "Invalid"))) {
            hasInvalidTypeName = true;
        }
        // Only report if it looks like an invalid type ID
        if (hasInvalidTypeName) {
            fprintf(stderr,
                    "[OCLeakTracker] No leaked objects of type \"%s\" (typeID %u) found.\n",
                    typeName, (unsigned)filterTypeID);
        }
        // Otherwise, silent success - no need to report when everything is working correctly
        pthread_mutex_unlock(&gLeakLock);
        return;
    }
    // We have at least one leak of filterTypeID.  Find a representative name.
    const char *typeName = "(unknown)";
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = gLeakTable[i].ptr;
        if (base && base->typeID == filterTypeID) {
            typeName = OCTypeIDName(base);
            break;
        }
    }
    fprintf(stderr,
            "[OCLeakTracker] %zu object(s) of type \"%s\" (typeID %u) not finalized.\n",
            filteredCount, typeName, (unsigned)filterTypeID);
    pthread_mutex_unlock(&gLeakLock);
}
void OCReportLeaksForTypeDetailed(OCTypeID filterTypeID) {
    pthread_mutex_lock(&gLeakLock);
    // Count and collect leaked entries matching filterTypeID
    size_t actual_leaks = 0;
    size_t static_instances = 0;
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = gLeakTable[i].ptr;
        if (!base || base->typeID != filterTypeID) continue;
        if (base->flags.static_instance) {
            static_instances++;
        } else {
            actual_leaks++;
        }
    }
    if (actual_leaks == 0 && static_instances == 0) {
        // Check if this looks like an invalid type ID
        const char *typeName = OCTypeNameFromTypeID(filterTypeID);
        bool hasInvalidTypeName = false;
        // If there's at least one instance in memory, grab its name.
        for (size_t i = 0; i < gLeakCount; ++i) {
            const OCBase *base = gLeakTable[i].ptr;
            if (base && base->typeID == filterTypeID) {
                typeName = OCTypeIDName(base);
                break;
            }
        }
        // Check if the type name suggests this is an invalid type ID
        if (typeName && (strstr(typeName, "InvalidTypeID") || strstr(typeName, "invalid") || strstr(typeName, "Invalid"))) {
            hasInvalidTypeName = true;
        }
        // Only report if it looks like an invalid type ID
        if (hasInvalidTypeName) {
            fprintf(stderr,
                    "[OCLeakTracker] No leaked objects of type \"%s\" (typeID %u) found.\n",
                    typeName, (unsigned)filterTypeID);
        }
        pthread_mutex_unlock(&gLeakLock);
        return;
    }
    // Get type name
    const char *typeName = "(unknown)";
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = gLeakTable[i].ptr;
        if (base && base->typeID == filterTypeID) {
            typeName = OCTypeIDName(base);
            break;
        }
    }
    fprintf(stderr, "\n[OCLeakTracker] DETAILED LEAK REPORT FOR TYPE: %s\n", typeName);
    if (actual_leaks > 0) {
        fprintf(stderr, "Found %zu ACTUAL LEAK(S) of type \"%s\" (typeID %u):\n",
                actual_leaks, typeName, (unsigned)filterTypeID);
    }
    if (static_instances > 0) {
        fprintf(stderr, "Found %zu static instance(s) of type \"%s\" (expected, not leaks)\n",
                static_instances, typeName);
    }
    if (actual_leaks == 0) {
        fprintf(stderr, "✅ No actual memory leaks found for type %s!\n", typeName);
        if (static_instances > 0) {
            fprintf(stderr, "   (The %zu static instances are expected and will be cleaned up at OCTypesShutdown)\n", static_instances);
        }
        fprintf(stderr, "\n");
        pthread_mutex_unlock(&gLeakLock);
        return;
    }
    fprintf(stderr, "\n");
    size_t leakIndex = 1;
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = gLeakTable[i].ptr;
        if (!base || base->typeID != filterTypeID || base->flags.static_instance) continue;
        fprintf(stderr, "LEAK #%zu: %s at %p\n",
                leakIndex++, typeName, gLeakTable[i].ptr);
        if (gLeakTable[i].allocation_hint) {
            fprintf(stderr, "  Allocation hint: %s\n", gLeakTable[i].allocation_hint);
        }
        if (gLeakTable[i].stack_depth > 0) {
            fprintf(stderr, "  Allocation stack trace:\n");
            print_stack_trace(gLeakTable[i].stack_frames, gLeakTable[i].stack_depth);
        } else {
            fprintf(stderr, "  (no stack trace available)\n");
        }
        fprintf(stderr, "\n");
    }
    pthread_mutex_unlock(&gLeakLock);
}
size_t OCLeakCountForType(OCTypeID typeID) {
    size_t count = 0;
    pthread_mutex_lock(&gLeakLock);
    for (size_t i = 0; i < gLeakCount; ++i) {
        if (gLeakTable[i].ptr && ((OCBase *)gLeakTable[i].ptr)->typeID == typeID)
            count++;
    }
    pthread_mutex_unlock(&gLeakLock);
    return count;
}
size_t OCActualLeakCountForType(OCTypeID typeID) {
    size_t count = 0;
    pthread_mutex_lock(&gLeakLock);
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = (OCBase *)gLeakTable[i].ptr;
        if (base && base->typeID == typeID && !base->flags.static_instance) {
            count++;
        }
    }
    pthread_mutex_unlock(&gLeakLock);
    return count;
}
void OCReportLeaksForTypeExcludingStatic(OCTypeID typeID) {
    // Do nothing under AddressSanitizer:
#if !__has_feature(address_sanitizer)
    pthread_mutex_lock(&gLeakLock);
    const char *typeName = OCTypeNameFromTypeID(typeID);
    if (!typeName) typeName = "(unknown)";
    size_t actual_leaks = 0;
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = (OCBase *)gLeakTable[i].ptr;
        if (base && base->typeID == typeID && !base->flags.static_instance) {
            actual_leaks++;
        }
    }
    if (actual_leaks > 0) {
        fprintf(stderr, "[OCLeakTracker] %zu object(s) of type \"%s\" (typeID %u) not finalized.\n",
                actual_leaks, typeName, typeID);
    }
    pthread_mutex_unlock(&gLeakLock);
#endif
}
void OCReportLeaksExcludingStatic(void) {
    // Do nothing under AddressSanitizer:
#if !__has_feature(address_sanitizer)
    pthread_mutex_lock(&gLeakLock);
    if (gLeakCount == 0) {
        // Silent success - no need to report when everything is working correctly
        pthread_mutex_unlock(&gLeakLock);
        return;
    }
    // Count only actual leaks (non-static instances)
    size_t actual_leaks = 0;
    typedef struct {
        OCTypeID typeID;
        const char *typeName;
        size_t count;
    } LeakSummary;
    size_t capacity = 16;
    size_t typeCount = 0;
    LeakSummary *summaries = malloc(capacity * sizeof(LeakSummary));
    if (!summaries) {
        fprintf(stderr, "[OCLeakTracker] ERROR: Failed to allocate memory for leak summary.\n");
        pthread_mutex_unlock(&gLeakLock);
        return;
    }
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = gLeakTable[i].ptr;
        if (!base || base->flags.static_instance) continue;  // Skip static instances
        actual_leaks++;
        OCTypeID tid = base->typeID;
        const char *typeName = OCTypeIDName(base);
        if (!typeName) typeName = "(unknown)";
        // Look for existing summary entry
        size_t j;
        for (j = 0; j < typeCount; ++j) {
            if (summaries[j].typeID == tid) {
                summaries[j].count++;
                break;
            }
        }
        // If it's a new typeID, add a new summary slot
        if (j == typeCount) {
            if (typeCount == capacity) {
                capacity *= 2;
                LeakSummary *newSummaries =
                    realloc(summaries, capacity * sizeof(LeakSummary));
                if (!newSummaries) {
                    fprintf(stderr, "[OCLeakTracker] ERROR: Failed to realloc memory.\n");
                    free(summaries);
                    pthread_mutex_unlock(&gLeakLock);
                    return;
                }
                summaries = newSummaries;
            }
            summaries[typeCount].typeID = tid;
            summaries[typeCount].typeName = typeName;
            summaries[typeCount].count = 1;
            typeCount++;
        }
    }
    if (actual_leaks == 0) {
        // All objects are static instances - no actual leaks
        free(summaries);
        pthread_mutex_unlock(&gLeakLock);
        return;
    }
    fprintf(stderr,
            "\n[OCLeakTracker] %zu actual leak(s) found (excluding static instances):\n",
            actual_leaks);
    for (size_t i = 0; i < typeCount; ++i) {
        fprintf(stderr, "  %s: %zu\n",
                summaries[i].typeName,
                summaries[i].count);
    }
    free(summaries);
    pthread_mutex_unlock(&gLeakLock);
#endif  // !__has_feature(address_sanitizer)
}
void OCReportLeaksForTypeDetailedExcludingStatic(OCTypeID filterTypeID) {
    pthread_mutex_lock(&gLeakLock);
    // Count only actual leaks (non-static instances) matching filterTypeID
    size_t actual_leaks = 0;
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = gLeakTable[i].ptr;
        if (!base || base->typeID != filterTypeID || base->flags.static_instance) continue;
        actual_leaks++;
    }
    if (actual_leaks == 0) {
        // Check if this looks like an invalid type ID
        const char *typeName = OCTypeNameFromTypeID(filterTypeID);
        bool hasInvalidTypeName = false;
        // If there's at least one instance in memory, grab its name.
        for (size_t i = 0; i < gLeakCount; ++i) {
            const OCBase *base = gLeakTable[i].ptr;
            if (base && base->typeID == filterTypeID) {
                typeName = OCTypeIDName(base);
                break;
            }
        }
        // Check if the type name suggests this is an invalid type ID
        if (typeName && (strstr(typeName, "InvalidTypeID") || strstr(typeName, "invalid") || strstr(typeName, "Invalid"))) {
            hasInvalidTypeName = true;
        }
        // Only report if it looks like an invalid type ID
        if (hasInvalidTypeName) {
            fprintf(stderr,
                    "[OCLeakTracker] No leaked objects of type \"%s\" (typeID %u) found.\n",
                    typeName, (unsigned)filterTypeID);
        }
        pthread_mutex_unlock(&gLeakLock);
        return;
    }
    // Get type name
    const char *typeName = "(unknown)";
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = gLeakTable[i].ptr;
        if (base && base->typeID == filterTypeID) {
            typeName = OCTypeIDName(base);
            break;
        }
    }
    fprintf(stderr, "\n[OCLeakTracker] DETAILED LEAK REPORT FOR TYPE: %s (excluding static instances)\n", typeName);
    fprintf(stderr, "Found %zu ACTUAL LEAK(S) of type \"%s\" (typeID %u):\n",
            actual_leaks, typeName, (unsigned)filterTypeID);
    fprintf(stderr, "\n");
    size_t leakIndex = 1;
    for (size_t i = 0; i < gLeakCount; ++i) {
        const OCBase *base = gLeakTable[i].ptr;
        if (!base || base->typeID != filterTypeID || base->flags.static_instance) continue;
        fprintf(stderr, "LEAK #%zu: %s at %p\n",
                leakIndex++, typeName, gLeakTable[i].ptr);
        if (gLeakTable[i].allocation_hint) {
            fprintf(stderr, "  Allocation hint: %s\n", gLeakTable[i].allocation_hint);
        }
        if (gLeakTable[i].stack_depth > 0) {
            fprintf(stderr, "  Allocation stack trace:\n");
            print_stack_trace(gLeakTable[i].stack_frames, gLeakTable[i].stack_depth);
        } else {
            fprintf(stderr, "  (no stack trace available)\n");
        }
        fprintf(stderr, "\n");
    }
    pthread_mutex_unlock(&gLeakLock);
}
