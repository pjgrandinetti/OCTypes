//
//  OCType.c
//  OCTypes
//
//  Created by Philip on 11/18/12.
//
#include <stdio.h>  // For perror
#include <stdlib.h>
#include <string.h>
#include "OCTypes.h"
// Compiler compatibility for __has_feature
#ifndef __has_feature
#define __has_feature(x) 0
#endif
static bool ocTypesShutdownCalled = false;
static char **typeIDTable = NULL;
static OCTypeID typeIDTableCount = 0;
static OCTypeRef (*createFromJSONTypedTable[256])(cJSON *) = {NULL};
void cleanupTypeIDTable(void) {
    if (typeIDTable) {
        for (OCTypeID i = 0; i < typeIDTableCount; i++) {
            free(typeIDTable[i]);
        }
        free(typeIDTable);
        typeIDTable = NULL;
        typeIDTableCount = 0;
    }
    // Clear the function pointer table
    for (int i = 0; i < 256; i++) {
        createFromJSONTypedTable[i] = NULL;
    }
}
int OCTypeIDTableGetCount(void) {
    return typeIDTableCount;
}
static bool TypeIDTableContainsName(const char *typeName) {
    if (typeName == NULL || typeIDTable == NULL) {
        return false;
    }
    for (OCTypeID i = 0; i < typeIDTableCount; ++i) {
        if (typeIDTable[i] != NULL && strcmp(typeIDTable[i], typeName) == 0) {
            return true;
        }
    }
    return false;
}
void OCTypesShutdown(void) {
    if (ocTypesShutdownCalled) return;
    ocTypesShutdownCalled = true;
    OCAutoreleasePoolCleanup();
    cleanupConstantStringTable();
// #if defined(DEBUG)                                \
    //     && !defined(__SANITIZE_ADDRESS__)              \
    //     && !(defined(__clang__) && __has_feature(address_sanitizer))
#if !defined(__SANITIZE_ADDRESS__) && !(defined(__clang__) && __has_feature(address_sanitizer))
    if (TypeIDTableContainsName("OCString"))
        OCReportLeaksForTypeDetailed(OCStringGetTypeID());
    if (TypeIDTableContainsName("OCNumber"))
        OCReportLeaksForTypeDetailed(OCNumberGetTypeID());
    if (TypeIDTableContainsName("OCBoolean"))
        OCReportLeaksForTypeDetailed(OCBooleanGetTypeID());
    if (TypeIDTableContainsName("OCData"))
        OCReportLeaksForTypeDetailed(OCDataGetTypeID());
    if (TypeIDTableContainsName("OCArray"))
        OCReportLeaksForTypeDetailed(OCArrayGetTypeID());
    if (TypeIDTableContainsName("OCSet"))
        OCReportLeaksForTypeDetailed(OCSetGetTypeID());
    if (TypeIDTableContainsName("OCIndexArray"))
        OCReportLeaksForTypeDetailed(OCIndexArrayGetTypeID());
    if (TypeIDTableContainsName("OCIndexSet"))
        OCReportLeaksForTypeDetailed(OCIndexSetGetTypeID());
    if (TypeIDTableContainsName("OCIndexPairSet"))
        OCReportLeaksForTypeDetailed(OCIndexPairSetGetTypeID());
    if (TypeIDTableContainsName("OCDictionary"))
        OCReportLeaksForTypeDetailed(OCDictionaryGetTypeID());
#endif
    cleanupTypeIDTable();
}
// // Run *after* LSAN’s destructors (101–103), so that LSAN gets to see
// // everything and only afterward we print out any remaining leaks.
// __attribute__((destructor(100)))
// static void impl_OCTypes_cleanup(void) {
//     if(typeIDTableCount == OCLIB_TYPES_COUNT) OCTypesShutdown();
// }
struct impl_OCType {
    OCBase base;
};
// Compares two OCType objects for equality.
bool OCTypeEqual(const void *theType1, const void *theType2) {
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
    if (typeRef1->base.typeID == kOCNotATypeID ||
        typeRef2->base.typeID == kOCNotATypeID ||
        typeRef1->base.typeID != typeRef2->base.typeID) {
        return false;
    }
    // Use the custom equality function if available.
    if (NULL == typeRef1->base.equal) {
        return false;
    }
    return typeRef1->base.equal(theType1, theType2);
}
/**
 * @brief Registers a new OCType with the system and optional JSON factory.
 * @param typeName A null-terminated C string representing the type name.
 * @param factory Optional function pointer to create instances from typed JSON.
 * @return The OCTypeID assigned, or kOCNotATypeID on failure.
 * @ingroup OCType
 */
OCTypeID OCRegisterType(const char *typeName, OCTypeRef (*factory)(cJSON *)) {
    // Return an invalid type ID if typeName is NULL.
    if (NULL == typeName) {
        return kOCNotATypeID;
    }
    // Initialize the type ID table if it hasn't been created yet.
    if (NULL == typeIDTable) {
        typeIDTable = malloc(256 * sizeof(char *));
        if (NULL == typeIDTable) {
            perror("Failed to allocate memory for typeIDTable");
            exit(EXIT_FAILURE);  // Exit if memory allocation fails
        }
    }
    // Check if the type name is already registered.
    for (uint32_t index = 0; index < typeIDTableCount; index++) {
        if (strcmp(typeIDTable[index], typeName) == 0) {
            // Update the factory if provided
            if (factory) {
                createFromJSONTypedTable[index] = factory;
            }
            return index + 1;
        }
    }
    // Add a new type if there is space available.
    if (typeIDTableCount < 256) {
        typeIDTable[typeIDTableCount] = malloc(strlen(typeName) + 1);
        if (NULL == typeIDTable[typeIDTableCount]) {
            perror("Failed to allocate memory for type name");
            exit(EXIT_FAILURE);  // Exit if memory allocation fails
        }
        strcpy(typeIDTable[typeIDTableCount], typeName);
        
        // Register the factory function if provided
        if (factory) {
            createFromJSONTypedTable[typeIDTableCount] = factory;
        }
        
        typeIDTableCount++;
        return typeIDTableCount;
    }
    return kOCNotATypeID;
}

void OCRelease(const void *ptr) {
    struct impl_OCType *theType = (struct impl_OCType *)ptr;
    if (NULL == theType) return;
    if (theType->base.typeID == kOCNotATypeID) {
        fprintf(stderr, "ERROR: OCRelease called on invalid object (%p),  typeID = %s\n",
                theType, OCTypeIDName(theType));
        return;
    }
    if (theType->base.flags.static_instance) return;
    if (theType->base.flags.finalized) {
        fprintf(stderr, "ERROR: OCRelease called on (%p), an already-finalized object, typeID = %s\n",
                theType, OCTypeIDName(theType));
        return;
    }
    if (theType->base.retainCount < 1) {
        fprintf(stderr, "ERROR: OCRelease called on (%p) with retainCount < 1, typeID = %s\n",
                theType, OCTypeIDName(theType));
        return;
    }
    if (theType->base.retainCount == 1) {
        if (theType->base.finalize) {
            theType->base.flags.finalized = true;
            theType->base.finalize(theType);  // Clean up internal fields only
        }
        if (theType->base.flags.tracked) {
            impl_OCUntrack(theType);
        }
        free((void *)theType);
        return;
    }
    theType->base.retainCount--;
}
const void *OCRetain(const void *ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "*** WARNING: OCRetain called on NULL pointer.\n");
        return NULL;
    }
    struct impl_OCType *theType = (struct impl_OCType *)ptr;
    OCTypeID typeID = theType->base.typeID;
    const char *typeName = OCTypeIDName(theType);
    if (typeID == kOCNotATypeID) {
        fprintf(stderr, "*** WARNING: OCRetain called on invalid object (%p), typeID = InvalidTypeID\n", ptr);
        return ptr;
    }
    if (theType->base.flags.static_instance) {
        // Static instances are immortal
    }
    if (theType->base.flags.finalized) {
        fprintf(stderr, "*** WARNING: OCRetain called on already-finalized object (%p), typeID = %s\n", ptr, typeName);
        return ptr;
    }
    if (theType->base.retainCount == UINT16_MAX) {
        fprintf(stderr, "*** WARNING: OCRetain overflow on object (%p), typeID = %s\n", ptr, typeName);
        return ptr;
    }
    theType->base.retainCount++;
    return ptr;
}
cJSON *OCTypeCopyJSON(OCTypeRef obj) {
    if (!obj) return cJSON_CreateNull();
    OCBase *b = (OCBase *)obj;
    if (!b->copyJSON) {
        return cJSON_CreateNull();
    }
    return b->copyJSON(obj);
}

cJSON *OCTypeCopyJSONTyped(OCTypeRef obj) {
    if (!obj) return cJSON_CreateNull();
    OCBase *b = (OCBase *)obj;
    if (!b->copyJSONTyped) {
        return cJSON_CreateNull();
    }
    return b->copyJSONTyped(obj);
}

OCTypeRef OCTypeCreateFromJSONTyped(cJSON *json) {
    if (!json) return NULL;
    
    // Handle native JSON types directly (no type wrapper needed)
    if (cJSON_IsString(json)) return (OCTypeRef)OCStringCreateFromJSON(json);
    if (cJSON_IsBool(json)) return (OCTypeRef)OCBooleanCreateFromJSON(json);
    if (cJSON_IsNumber(json)) return (OCTypeRef)OCNumberCreateFromJSON(json, kOCNumberFloat64Type);
    
    // Handle wrapped types with type information
    if (!cJSON_IsObject(json)) return NULL;
    
    cJSON *type = cJSON_GetObjectItem(json, "type");
    const char *typeName = cJSON_IsString(type) ? cJSON_GetStringValue(type) : NULL;
    if (!typeName) return NULL;
    
    // Look up the type in our registry and call the registered factory function
    for (OCTypeID i = 0; i < typeIDTableCount; i++) {
        if (typeIDTable[i] && strcmp(typeIDTable[i], typeName) == 0) {
            OCTypeRef (*factory)(cJSON *) = createFromJSONTypedTable[i];
            if (factory) return factory(json);
            fprintf(stderr, "OCTypeCreateFromJSONTyped: No factory registered for type '%s'\n", typeName);
            return NULL;
        }
    }
    
    fprintf(stderr, "OCTypeCreateFromJSONTyped: Unknown type '%s'\n", typeName);
    return NULL;
}
void *OCTypeDeepCopy(const void *obj) {
    if (!obj) return NULL;
    const struct impl_OCType *type = (const struct impl_OCType *)obj;
    return type->base.copyDeep ? type->base.copyDeep(obj) : NULL;
}
void *OCTypeDeepCopyMutable(const void *obj) {
    if (!obj) return NULL;
    const struct impl_OCType *type = (const struct impl_OCType *)obj;
    return type->base.copyDeepMutable ? type->base.copyDeepMutable(obj) : NULL;
}
// Returns a formatted description of the object.
OCStringRef OCTypeCopyFormattingDesc(const void *ptr) {
    if (NULL == ptr) return NULL;
    OCTypeRef theType = (OCTypeRef)ptr;
    return theType->base.copyFormattingDesc(theType);
}
// Returns a string description of the object's type.
OCStringRef OCCopyDescription(const void *ptr) {
    if (NULL == ptr) {
        return OCStringCreateWithCString("NULL");
    }
    OCTypeRef theType = (OCTypeRef)ptr;
    OCTypeID currentTypeID = theType->base.typeID;
    // Validate the type ID and return the corresponding name.
    if (currentTypeID == kOCNotATypeID || currentTypeID > typeIDTableCount) {
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
                     cJSON *(*copyJSON)(const void *),
                     cJSON *(*copyJSONTyped)(const void *),
                     void *(*copyDeep)(const void *),
                     void *(*copyDeepMutable)(const void *)) {
    struct impl_OCType *object = calloc(1, size);
    if (!object) {
        fprintf(stderr, "OCTypeAllocate: allocation failed\n");
        exit(EXIT_FAILURE);
    }
    object->base.typeID = typeID;
    object->base.retainCount = 1;
    object->base.finalize = finalize;
    object->base.equal = equal;
    object->base.copyFormattingDesc = copyDesc;
    object->base.copyJSON = copyJSON;
    object->base.copyJSONTyped = copyJSONTyped;
    object->base.copyDeep = copyDeep;
    object->base.copyDeepMutable = copyDeepMutable;
    object->base.flags.static_instance = false;
    object->base.flags.finalized = false;
    object->base.flags.tracked = true;
    
    impl_OCTrack(object);
    return object;
}
// Retrieves the type ID of the given object.
OCTypeID OCGetTypeID(const void *ptr) {
    if (ptr) {
        OCTypeRef theType = (OCTypeRef)ptr;
        if (theType->base.typeID == kOCNotATypeID || theType->base.typeID > typeIDTableCount) {
            return kOCNotATypeID;
        }
        return theType->base.typeID;
    }
    return kOCNotATypeID;
}
int OCTypeGetRetainCount(const void *ptr) {
    if (NULL == ptr) {
        return 0;
    }
    OCTypeRef theType = (OCTypeRef)ptr;
    return theType->base.retainCount;
}
// Sets the retain count of the object.
bool OCTypeGetStaticInstance(const void *ptr) {
    if (NULL == ptr) {
        return false;
    }
    OCTypeRef theType = (OCTypeRef)ptr;
    return theType->base.flags.static_instance;
}
void OCTypeSetStaticInstance(const void *ptr, bool static_instance) {
    if (NULL == ptr) {
        return;
    }
    struct impl_OCType *theType = (struct impl_OCType *)ptr;
    theType->base.retainCount = 1;
    theType->base.flags.static_instance = static_instance;
}
bool OCTypeGetFinalized(const void *ptr) {
    if (NULL == ptr) return false;
    struct impl_OCType *theType = (struct impl_OCType *)ptr;
    return theType->base.flags.finalized;
}
const char *OCTypeIDName(const void *ptr) {
    struct impl_OCType *theType = (struct impl_OCType *)ptr;
    if (theType->base.typeID == kOCNotATypeID || theType->base.typeID > typeIDTableCount) {
        return "InvalidTypeID";
    }
    const char *name = typeIDTable[theType->base.typeID - 1];
    return (name != NULL) ? name : "UnnamedType";
}
const char *OCTypeNameFromTypeID(OCTypeID typeID) {
    if (typeID == kOCNotATypeID || typeID > typeIDTableCount) {
        return "InvalidTypeID";
    }
    const char *name = typeIDTable[typeID - 1];
    return (name != NULL) ? name : "UnnamedType";
}
// ============================================================================
// OCRange utility function
// ============================================================================
OCRange OCRangeMake(OCIndex loc, OCIndex len) {
    OCRange r = {loc, len};
    return r;
}
