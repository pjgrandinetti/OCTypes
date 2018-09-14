//
//  OCType.h
//
//  Created by Philip on 11/18/12.
//

#ifndef OCType_h
#define OCType_h

#include "OCLibrary.h"

#define IF_NO_OBJECT_EXISTS_RETURN(OBJECT,X) if(NULL==OBJECT) {printf("*** WARNING - %s - object doesn't exist.\n",__func__); return X;}


typedef uint32_t OCTypeID;
typedef int32_t OCIndex;

enum {
    kOCNotFound = -1
};

typedef struct __OCType * OCTypeRef;

enum {
    _kOCNotATypeID = 0
};

bool OCTypeEqual(const void * theType1, const void * theType2);

OCTypeID OCRegisterType(char *typeName);

void OCRelease(const void * ptr);

const void *OCRetain(const void * ptr);

typedef const struct __OCString * OCStringRef;

OCStringRef OCTypeCopyFormattingDesc(const void * ptr);

OCStringRef OCCopyDescription(const void * ptr);

OCTypeID OCGetTypeID(const void * ptr);

typedef struct __OCBase {
    OCTypeID typeID;
    uint32_t retainCount;
    void (*finalize)(const void *);
    bool (*equal)(const void *, const void *);
    OCStringRef (*copyFormattingDesc)(OCTypeRef cf);
} OCBase;

#endif
