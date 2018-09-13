//
//  OCType.c
//  OCTypes
//
//  Created by Philip on 11/18/12.
//  Copyright © 2017 Philip Grandinetti. All rights reserved.
//

#include "OCLibrary.h"

static char **typeIDTable = NULL;
static OCTypeID typeIDTableCount = 0;

struct __OCType {
    OCBase _base;
};

bool OCTypeEqual(const void * theType1, const void * theType2)
{
    return ((OCTypeRef)theType1)->_base.equal(theType1,theType2);
}

OCTypeID OCRegisterType(char *typeName)
{
    // Hard coded -- only 256 classes can be created.
    if(NULL == typeIDTable) {
        typeIDTable = malloc(256*sizeof(char *));
    }
    if(typeIDTableCount<256) {
        for(uint32_t index = 0;index<typeIDTableCount;index++) {
            if(strcmp(typeIDTable[index], typeName)==0) return index;
        }
        typeIDTable[typeIDTableCount] = malloc(strlen(typeName)+1);
        strcpy(typeIDTable[typeIDTableCount], typeName);
        typeIDTableCount++;
        return typeIDTableCount;
    }
    return _kOCNotATypeID;
}

// Anything entering this routine with a retainCount of zero or less
// is permanent and cannot be freed.
void OCRelease(const void * ptr)
{
    OCTypeRef theType = (OCTypeRef) ptr;
    if(NULL==theType) return;
    if(theType->_base.retainCount<1) {
        theType->_base.retainCount = 0;
        return;
    }
    else if(theType->_base.retainCount == 1) {
        theType->_base.finalize(theType);
        return;
    }
    theType->_base.retainCount--;
    return;
}

const void *OCRetain(const void * ptr)
{
    OCTypeRef theType = (OCTypeRef) ptr;
    if(NULL==theType) return NULL;
    if(theType->_base.retainCount<1) {
        theType->_base.retainCount = 0;
        return theType;
    }
    theType->_base.retainCount++;
    return theType;
}

OCStringRef OCTypeCopyFormattingDesc(const void * ptr)
{
    if(NULL==ptr) return NULL;
    OCTypeRef theType = (OCTypeRef) ptr;
    return theType->_base.copyFormattingDesc(theType);
}

OCStringRef OCCopyDescription(const void * ptr)
{
    OCTypeRef theType = (OCTypeRef) ptr;
    return OCStringCreateWithCString(typeIDTable[theType->_base.typeID]);
}

OCTypeID OCGetTypeID(const void * ptr)
{
    if(ptr) {
        OCTypeRef theType = (OCTypeRef) ptr;
        if(theType->_base.typeID > typeIDTableCount) return kOCNotFound;
        return theType->_base.typeID;
    }
    return kOCNotFound;
}
