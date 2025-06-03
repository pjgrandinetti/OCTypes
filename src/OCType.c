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


void OCRelease(const void * ptr)
{
    struct __OCType *theType = (struct __OCType *) ptr;
    if (NULL == theType) return;

    if (theType->_base.typeID == _kOCNotATypeID) {
        fprintf(stderr, "ERROR: OCRelease called on invalid object (%p),  typeID = %s\n",
                theType, OCTypeNameFromTypeID(theType));
        return;
    }

    if (theType->_base.static_instance) return;

    if (theType->_base.finalized) {
        fprintf(stderr, "ERROR: OCRelease called on (%p), an already-finalized object, typeID = %s\n",
                theType, OCTypeNameFromTypeID(theType));
        return;
    }

    if (theType->_base.retainCount < 1) {
        fprintf(stderr, "ERROR: OCRelease called on (%p) with retainCount < 1, typeID = %s\n",
                theType, OCTypeNameFromTypeID(theType));
        return;
    }

    if (theType->_base.retainCount == 1) {
        if (theType->_base.finalize) {
            theType->_base.finalized = true;
            theType->_base.finalize(theType);  // Clean up internal fields only
        }

#ifdef DEBUG
        if (theType->_base.tracked) {
            _OCUntrack(theType);
        }
#endif

        free((void *)theType);
        return;
    }

    theType->_base.retainCount--;
}

const void *OCRetain(const void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "*** WARNING: OCRetain called on NULL pointer.\n");
        return NULL;
    }

    struct __OCType *theType = (struct __OCType *) ptr;

    OCTypeID typeID = theType->_base.typeID;
    const char *typeName = OCTypeNameFromTypeID(theType);

    if (typeID == _kOCNotATypeID) {
        fprintf(stderr, "*** WARNING: OCRetain called on invalid object (%p), typeID = InvalidTypeID\n", ptr);
        return ptr;
    }

    if (theType->_base.static_instance) {
        // Static instances are immortal
    }

    if (theType->_base.finalized) {
        fprintf(stderr, "*** WARNING: OCRetain called on already-finalized object (%p), typeID = %s\n", ptr, typeName);
        return ptr;
    }

    if (theType->_base.retainCount == UINT32_MAX) {
        fprintf(stderr, "*** WARNING: OCRetain overflow on object (%p), typeID = %s\n", ptr, typeName);
        return ptr;
    }

    theType->_base.retainCount++;

    return ptr;
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


void *OCTypeAllocate(size_t size, 
                    OCTypeID typeID,
                    void (*finalize)(const void *),
                    bool (*equal)(const void *, const void *),
                    OCStringRef (*copyDesc)(OCTypeRef),
                    const char *file, 
                    int line)
{
    struct __OCType *object = calloc(1, size);
    if (!object) {
        fprintf(stderr, "OCTypeAllocate: allocation failed\n");
        exit(EXIT_FAILURE);
    }

    object->_base.typeID = typeID;
    object->_base.retainCount = 1;
    object->_base.finalize = finalize;
    object->_base.equal = equal;
    object->_base.copyFormattingDesc = copyDesc;
    object->_base.static_instance = false;
    object->_base.finalized = false;

#ifdef DEBUG
    object->_base.allocFile = file;
    object->_base.allocLine = line;
    object->_base.tracked = true;
    _OCTrackDebug(object, file, line);
#endif

    return object;
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

bool OCTypeGetFinalized(const void * ptr) {
    if (NULL == ptr) return false;
    struct __OCType *theType = (struct __OCType *) ptr;
    return theType->_base.finalized;
}

const char *OCTypeNameFromTypeID(const void * ptr)
{
    struct __OCType *theType = (struct __OCType *) ptr;

    if (theType->_base.typeID == _kOCNotATypeID || theType->_base.typeID == 0 || theType->_base.typeID > typeIDTableCount) {
        return "InvalidTypeID";
    }

    const char *name = typeIDTable[theType->_base.typeID - 1];
    return (name != NULL) ? name : "UnnamedType";
}
