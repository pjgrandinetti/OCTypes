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
    // Cast to non-const internal struct pointer for modification
    struct __OCType *theType = (struct __OCType *) ptr;
    if(NULL == theType) return;

    // Decrement the retain count and finalize if it reaches zero.
    if(theType->_base.retainCount < 1) {
        theType->_base.retainCount = 0; // Should this be an assertion or error?
        return;
    } else if(theType->_base.retainCount == 1) {
        // Before finalizing, ensure retain count is conceptually zero if finalize might re-enter or check it.
        // However, the actual deallocation should happen after finalize.
        // For now, let's assume finalize handles the object state appropriately.
        if (theType->_base.finalize) { // Check if finalize is not NULL
            theType->_base.finalize(theType);
        }
        // After finalize, the object should be considered gone, or its memory freed by finalize.
        // If finalize doesn't free, then `free(theType);` might be needed here, depending on design.
        // For now, we assume finalize handles deallocation or the object is part of a larger scheme.
        return; // Return after finalize, as the object might be invalid.
    }
    theType->_base.retainCount--;
}

// Increments the reference count of an object.
const void *OCRetain(const void * ptr)
{
    // Cast to non-const internal struct pointer for modification
    struct __OCType *theType = (struct __OCType *) ptr;
    if(NULL == theType) return NULL;

    // Since retainCount is uint32_t, it cannot be negative.
    // A retainCount of 0 will be incremented to 1.
    // The previous conditional block to handle retainCount < 1 (and not 0)
    // was always false and has been removed.

    // Check for overflow if retainCount is near max value of uint32_t
    if (theType->_base.retainCount == UINT32_MAX) {
        // Handle overflow: either error, assert, or cap.
        // For now, let's assume it's an error condition or highly unlikely.
        // Depending on policy, could log or abort.
        return theType; // Cannot retain further
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
