//
//  OCType.c
//  OCTypes
//
//  Created by Philip on 11/18/12.
//

#include "OCLibrary.h"

static char **typeIDTable = NULL;
static OCTypeID typeIDTableCount = 0;

struct __OCType {
    OCBase _base;
};

bool OCTypeEqual(const void * theType1, const void * theType2)
{
    // If either pointer is NULL, they are not considered equal.
    // This makes OCTypeEqual(NULL, NULL) return false, satisfying the test.
    if (NULL == theType1 || NULL == theType2) {
        return false;
    }

    // If they are the exact same instance, they are equal.
    if (theType1 == theType2) {
        return true;
    }

    OCTypeRef typeRef1 = (OCTypeRef)theType1;
    OCTypeRef typeRef2 = (OCTypeRef)theType2;

    // Check if type IDs are valid and match.
    // Assuming _kOCNotATypeID is 0 and valid IDs are > 0.
    if (typeRef1->_base.typeID == _kOCNotATypeID ||
        typeRef2->_base.typeID == _kOCNotATypeID ||
        typeRef1->_base.typeID != typeRef2->_base.typeID) {
        return false;
    }

    // Check if the equal function pointer is valid.
    if (NULL == typeRef1->_base.equal) {
        // Or if typeRef2->_base.equal is also NULL and typeIDs match, consider them equal by pointer?
        // For now, if no comparison function, assume not equal unless pointers were identical (checked above).
        return false;
    }
    
    return typeRef1->_base.equal(theType1, theType2);
}

/*!
 @function OCRegisterType
 @abstract Registers a new OCType
 @result The OCTypeID for the new OCType
 */
OCTypeID OCRegisterType(char *typeName)
{
    // Return _kOCNotATypeID if typeName is NULL
    if (NULL == typeName) {
        return _kOCNotATypeID;
    }

    // Hard coded -- only 256 classes can be created.
    if(NULL == typeIDTable) {
        typeIDTable = malloc(256*sizeof(char *));
        if (NULL == typeIDTable) { // Check malloc failure
            // Consider logging an error here
            return _kOCNotATypeID;
        }
    }
    // typeIDTableCount is the current number of registered types (0 to 255)
    // Valid TypeIDs will be 1 to 256

    // Check if typeName already exists
    for(uint32_t index = 0; index < typeIDTableCount; index++) {
        if(strcmp(typeIDTable[index], typeName) == 0) {
            return index + 1; // Return 1-based ID for existing type
        }
    }

    // If not found, try to add new type if space available
    if(typeIDTableCount < 256) {
        typeIDTable[typeIDTableCount] = malloc(strlen(typeName) + 1);
        if (NULL == typeIDTable[typeIDTableCount]) { // Check malloc failure
            // Consider logging an error here
            return _kOCNotATypeID;
        }
        strcpy(typeIDTable[typeIDTableCount], typeName);
        typeIDTableCount++; // Increment count of registered types
        return typeIDTableCount; // Return new 1-based ID (which is the new count)
    }
    
    return _kOCNotATypeID; // Table is full or other error
}

/*!
 @function OCRelease
 @abstract Decrements the reference counter, and frees object when reference counter is zero.
 @discussion Anything entering this routine with a retainCount of zero or less is permanent and cannot be freed.
 */
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

/*!
 @function OCRetain
 @abstract Increments the reference counter.
 */
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
    if (NULL == ptr) {
        return OCStringCreateWithCString("NULL"); // Or NULL, depending on desired behavior
    }

    OCTypeRef theType = (OCTypeRef) ptr;
    OCTypeID currentTypeID = theType->_base.typeID;

    // Check if the typeID is valid (1-based and within bounds)
    // typeIDTableCount is the number of registered types. Valid IDs are 1 to typeIDTableCount.
    // _kOCNotATypeID is 0.
    if (currentTypeID == _kOCNotATypeID || currentTypeID > typeIDTableCount) {
        return OCStringCreateWithCString("UnknownType");
    }

    // Access typeIDTable using 0-based index (typeID - 1)
    // Also ensure the string in typeIDTable is not NULL, though it should be if registered.
    if (typeIDTable[currentTypeID - 1] == NULL) {
         return OCStringCreateWithCString("UnnamedType");
    }
    return OCStringCreateWithCString(typeIDTable[currentTypeID - 1]);
}

OCTypeID OCGetTypeID(const void * ptr)
{
    if(ptr) {
        OCTypeRef theType = (OCTypeRef) ptr;
        // Assuming theType->_base.typeID is a 1-based ID if valid,
        // and typeIDTableCount is the total number of registered types.
        // Valid IDs are from 1 to typeIDTableCount.
        if(theType->_base.typeID == _kOCNotATypeID || theType->_base.typeID > typeIDTableCount) {
            return _kOCNotATypeID; // Return 0 if ID is invalid or out of bounds
        }
        return theType->_base.typeID;
    }
    return _kOCNotATypeID; // Return 0 if ptr is NULL
}
