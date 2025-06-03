/* OCLeakTracker.c */
#include "OCLeakTracker.h"

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct {
    const void *ptr;
    const char *file;
    int line;
} OCLeakEntry;

static OCLeakEntry *gLeakTable = NULL;
static size_t gLeakCount = 0;
static pthread_mutex_t gLeakLock = PTHREAD_MUTEX_INITIALIZER;

void _OCTrackDebug(const void *ptr, const char *file, int line) {
    pthread_mutex_lock(&gLeakLock);
    gLeakTable = realloc(gLeakTable, (gLeakCount + 1) * sizeof(OCLeakEntry));
    if (!gLeakTable) {
        fprintf(stderr, "[LeakTracker] realloc failed at %s:%d\n", file, line);
        abort();
    }
    gLeakTable[gLeakCount++] = (OCLeakEntry){ ptr, file, line };
    pthread_mutex_unlock(&gLeakLock);
}

void _OCUntrack(const void *ptr) {
    pthread_mutex_lock(&gLeakLock);
    for (size_t i = 0; i < gLeakCount; ++i) {
        if (gLeakTable[i].ptr == ptr) {
            gLeakTable[i] = gLeakTable[--gLeakCount];
            break;
        }
    }
    pthread_mutex_unlock(&gLeakLock);
}

void _OCReportLeaks(void) {
    pthread_mutex_lock(&gLeakLock);

    if (gLeakCount == 0) {
        fprintf(stderr, "[OCLeakTracker] All objects finalized successfully.\n");
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

        // Search for existing entry
        size_t j;
        for (j = 0; j < typeCount; ++j) {
            if (summaries[j].typeID == tid) {
                summaries[j].count++;
                if (base->static_instance) summaries[j].staticCount++;
                break;
            }
        }

        // New typeID
        if (j == typeCount) {
            if (typeCount == capacity) {
                capacity *= 2;
                LeakSummary *newSummaries = realloc(summaries, capacity * sizeof(LeakSummary));
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
            summaries[typeCount].staticCount = base->static_instance ? 1 : 0;
            typeCount++;
        }
    }

    fprintf(stderr, "\n[OCLeakTracker] %zu object(s) not finalized, grouped by type:\n", gLeakCount);
    for (size_t i = 0; i < typeCount; ++i) {
        if (summaries[i].staticCount > 0) {
            fprintf(stderr, "  %s: %zu (%zu static)\n",
                    summaries[i].typeName, summaries[i].count, summaries[i].staticCount);
        } else {
            fprintf(stderr, "  %s: %zu\n", summaries[i].typeName, summaries[i].count);
        }
    }

    free(summaries);
    pthread_mutex_unlock(&gLeakLock);
}


size_t _OCLeakCountForType(OCTypeID typeID) {
    size_t count = 0;
    pthread_mutex_lock(&gLeakLock);
    for (size_t i = 0; i < gLeakCount; ++i) {
        if (gLeakTable[i].ptr && ((OCBase *)gLeakTable[i].ptr)->typeID == typeID)
            count++;
    }
    pthread_mutex_unlock(&gLeakLock);
    return count;
}

#endif
