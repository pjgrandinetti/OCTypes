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
    if (gLeakCount > 0) {
        fprintf(stderr, "\n[OCLeakTracker] %zu object(s) not finalized:\n", gLeakCount);
        for (size_t i = 0; i < gLeakCount; ++i) {
            const OCBase *base = gLeakTable[i].ptr;
            const char *typeName = OCTypeIDName(base);
            fprintf(stderr, "  Leak: %s at %p (%s) (allocated at %s:%d)\n",
                    typeName ? typeName : "(unknown type)",
                    gLeakTable[i].ptr,
                    base->static_instance ? "static" : "dynamic",
                    gLeakTable[i].file, gLeakTable[i].line);

            if (base->copyFormattingDesc) {
                OCStringRef desc = base->copyFormattingDesc((OCTypeRef)base);
                if (desc) {
                    fprintf(stderr, "    → contents: %s\n", OCStringGetCString(desc));
                    OCRelease(desc);
                }
            }
        }
    } else {
        fprintf(stderr, "[OCLeakTracker] All objects finalized successfully.\n");
    }
    pthread_mutex_unlock(&gLeakLock);
}

#endif
