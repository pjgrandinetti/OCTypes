//
//  OCType.c
//  OCTypes
//
//  Created by Philip on 11/18/12.
//

#include <stdlib.h>
#include <string.h>
#include "OCLibrary.h"

static char **typeIDTable = NULL;
static OCTypeID typeIDTableCount = 0;

struct __OCType {
    OCBase _base;
};

// Compares two OCType objects for equality.
bool OCTypeEqual(const void * theType1, const void * theType2)
{
    // Check if either pointer is NULL or if they are the same instance.
    if (NULL == theType1 || NULL == theType2) {
        return false;
    }

    if (theType1 == theType2) {
        return true;
    }

    OCTypeRef typeRef1 = (OCTypeRef)theType1;
    OCTypeRef typeRef2 = (OCTypeRef)theType2;

    // Ensure type IDs are valid and match.
    if (typeRef1->_base.typeID == _kOCNotATypeID ||
        typeRef2->_base.typeID == _kOCNotATypeID ||
        typeRef1->_base.typeID != typeRef2->_base.typeID) {
        return false;
    }

    // Use the custom equality function if available.
    if (NULL == typeRef1->_base.equal) {
        return false;
    }
    
    return typeRef1->_base.equal(theType1, theType2);
}

// Registers a new type and returns its unique type ID.
OCTypeID OCRegisterType(char *typeName)
{
    // Return an invalid type ID if typeName is NULL.
    if (NULL == typeName) {
        return _kOCNotATypeID;
    }

    // Initialize the type ID table if it hasn't been created yet.
    if(NULL == typeIDTable) {
        typeIDTable = malloc(256 * sizeof(char *));
        if (NULL == typeIDTable) {
            return _kOCNotATypeID;
        }
    }

    // Check if the type name is already registered.
    for(uint32_t index = 0; index < typeIDTableCount; index++) {
        if(strcmp(typeIDTable[index], typeName) == 0) {
            return index + 1;
        }
    }

    // Add a new type if there is space available.
    if(typeIDTableCount < 256) {
        typeIDTable[typeIDTableCount] = malloc(strlen(typeName) + 1);
        if (NULL == typeIDTable[typeIDTableCount]) {
            return _kOCNotATypeID;
        }
        strcpy(typeIDTable[typeIDTableCount], typeName);
        typeIDTableCount++;
        return typeIDTableCount;
    }
    
    return _kOCNotATypeID;
}

// Decrements the reference count of an object and finalizes it if the count reaches zero.
void OCRelease(const void * ptr)
{
    OCTypeRef theType = (OCTypeRef) ptr;
    if(NULL == theType) return;

    // Decrement the retain count and finalize if it reaches zero.
    if(theType->_base.retainCount < 1) {
        theType->_base.retainCount = 0;
        return;
    } else if(theType->_base.retainCount == 1) {
        theType->_base.finalize(theType);
        return;
    }
    theType->_base.retainCount--;
}

// Increments the reference count of an object.
const void *OCRetain(const void * ptr)
{
    OCTypeRef theType = (OCTypeRef) ptr;
    if(NULL == theType) return NULL;

    // Increment the retain count if valid.
    if(theType->_base.retainCount < 1) {
        theType->_base.retainCount = 0;
        return theType;
    }
    theType->_base.retainCount++;
    return theType;
}

// Returns a formatted description of the object.
OCStringRef OCTypeCopyFormattingDesc(const void * ptr)
{
    if(NULL == ptr) return NULL;
    OCTypeRef theType = (OCTypeRef) ptr;
    return theType->_base.copyFormattingDesc(theType);
}

// Returns a string description of the object's type.
OCStringRef OCCopyDescription(const void * ptr)
{
    if (NULL == ptr) {
        return OCStringCreateWithCString("NULL");
    }

    OCTypeRef theType = (OCTypeRef) ptr;
    OCTypeID currentTypeID = theType->_base.typeID;

    // Validate the type ID and return the corresponding name.
    if (currentTypeID == _kOCNotATypeID || currentTypeID > typeIDTableCount) {
        return OCStringCreateWithCString("UnknownType");
    }

    if (typeIDTable[currentTypeID - 1] == NULL) {
         return OCStringCreateWithCString("UnnamedType");
    }
    return OCStringCreateWithCString(typeIDTable[currentTypeID - 1]);
}

// Retrieves the type ID of the given object.
OCTypeID OCGetTypeID(const void * ptr)
{
    if(ptr) {
        OCTypeRef theType = (OCTypeRef) ptr;
        if(theType->_base.typeID == _kOCNotATypeID || theType->_base.typeID > typeIDTableCount) {
            return _kOCNotATypeID;
        }
        return theType->_base.typeID;
    }
    return _kOCNotATypeID;
}
