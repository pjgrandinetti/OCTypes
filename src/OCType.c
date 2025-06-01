//
//  OCType.c
//  OCTypes
//
//  Created by Philip on 11/18/12.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h> // For perror
#include "OCLibrary.h"

static char **typeIDTable = NULL;
static OCTypeID typeIDTableCount = 0;

void cleanupTypeIDTable(void) {
    if (typeIDTable) {
        for (OCTypeID i = 0; i < typeIDTableCount; i++) {
            free(typeIDTable[i]);
        }
        free(typeIDTable);
        typeIDTable = NULL;
        typeIDTableCount = 0;
    }
}

// Run **after** LSAN’s destructors (101–103), so this cleanup
// happens once leak checking has already run.
__attribute__((destructor(104)))
static void _OCTypes_cleanup_after_leak_check(void) {
    cleanupConstantStringTable();
    cleanupTypeIDTable();
}

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
    if (NULL == typeIDTable) {
        typeIDTable = malloc(256 * sizeof(char *));
        if (NULL == typeIDTable) {
            perror("Failed to allocate memory for typeIDTable");
            exit(EXIT_FAILURE); // Exit if memory allocation fails
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
            perror("Failed to allocate memory for type name");
            exit(EXIT_FAILURE); // Exit if memory allocation fails
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

    if(theType->_base.static_instance) {
        // Static instances should not be released.
        return;
    }
    // Decrement the retain count and finalize if it reaches zero.
    if(theType->_base.retainCount < 1) {
        theType->_base.retainCount = 0; // Should this be an assertion or error?
        return;
    } else if(theType->_base.retainCount == 1) {
        if (theType->_base.finalize) { // Check if finalize is not NULL
            theType->_base.finalize(theType);
        }
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

    if(theType->_base.static_instance) {
        // Static instances should not be retained.
        return theType;
    }

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

int OCTypeGetRetainCount(const void * ptr) {
    if (NULL == ptr) {
        return 0;
    }
    OCTypeRef theType = (OCTypeRef) ptr;
    return theType->_base.retainCount;
}

// Sets the retain count of the object.
bool OCTypeGetStaticInstance(const void * ptr) {
    if (NULL == ptr) {
        return false;
    }
    OCTypeRef theType = (OCTypeRef) ptr;
    return theType->_base.static_instance;
}

void OCTypeSetStaticInstance(const void * ptr, bool static_instance) {
    if (NULL == ptr) {
        return;
    }
    struct __OCType *theType = (struct __OCType *) ptr;
    theType->_base.retainCount = 1;
    theType->_base.static_instance = static_instance;
}
// Returns the retain count of the object.