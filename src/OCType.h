/**
 * .. _OCType_h:
 *
 * OCType.h
 * ========
 *
 * Defines the core types and functions for the OCTypes library.
 *
 * This file contains the fundamental type definitions, including OCTypeID,
 * OCIndex, and the base OCTypeRef, as well as functions for type registration,
 * memory management (retain/release), and basic type introspection.
 */

//
//  OCType.h
//
//  Created by Philip on 11/18/12.
//

#ifndef OCType_h
#define OCType_h

#include "OCLibrary.h"

/** @defgroup OCType OCType */
/** @addtogroup OCType
 *  @{
 */

/**
 * :def: IF_NO_OBJECT_EXISTS_RETURN
 * :brief: Macro to check if an object is NULL and return a specified value if it is.
 * :param OBJECT: The object to check.
 * :param X: The value to return if OBJECT is NULL.
 */
#define IF_NO_OBJECT_EXISTS_RETURN(OBJECT,X) if(NULL==OBJECT) {printf("*** WARNING - %s - object doesn't exist.\n",__func__); return X;}

/**
 * :type OCTypeID:
 * :brief: Defines an identifier for an OCType.
 */
typedef uint32_t OCTypeID;

/**
 * :type OCIndex:
 * :brief: Defines an index type, typically used for array or collection indexing.
 */
typedef int32_t OCIndex;

/**
 * :enum: CommonConstants
 * :brief: Defines common constant values.
 */
enum CommonConstants {
    /** Represents a value not found, typically used as a return value from search functions. */
    kOCNotFound = -1
};

/**
 * :type OCTypeRef:
 * :brief: A reference to an opaque OCType structure.
 * This is the base type for all objects in the OCTypes system.
 */
typedef struct __OCType * OCTypeRef;

/**
 * :enum: SpecialTypeIDs
 * :brief: Defines special type ID values.
 */
enum SpecialTypeIDs {
    /** Represents an invalid or uninitialized OCTypeID. */
    _kOCNotATypeID = 0
};

/**
 * :func: OCTypeEqual
 * :brief: Compares two OCType instances for equality.
 * :param theType1: A pointer to the first OCType instance.
 * :param theType2: A pointer to the second OCType instance.
 * :return: True if the types are considered equal, false otherwise.
 * :note: The actual comparison logic is determined by the `equal` function pointer
 *        in the OCBase structure of the respective types.
 */
bool OCTypeEqual(const void * theType1, const void * theType2);

/**
 * :func: OCRegisterType
 * :brief: Registers a new OCType with the system.
 * :param typeName: A string representing the name of the type to register.
 * :return: The OCTypeID assigned to the newly registered type.
 *          Returns _kOCNotATypeID if registration fails (e.g., typeName is NULL or already registered).
 */
OCTypeID OCRegisterType(char *typeName);

/**
 * :func: OCRelease
 * :brief: Releases an OCType instance, decrementing its retain count.
 * If the retain count reaches zero, the object's finalize function is called,
 * and the memory is deallocated.
 * :param ptr: A pointer to the OCType instance to release.
 */
void OCRelease(const void * ptr);

/**
 * :func: OCRetain
 * :brief: Retains an OCType instance, incrementing its retain count.
 * :param ptr: A pointer to the OCType instance to retain.
 * :return: A pointer to the retained OCType instance (the same as ptr).
 *          Returns NULL if ptr is NULL.
 */
const void *OCRetain(const void * ptr);

/**
 * :type OCStringRef:
 * :brief: A reference to an immutable OCString instance.
 */
typedef const struct __OCString * OCStringRef;

/**
 * :func: OCTypeCopyFormattingDesc
 * :brief: Creates a string representation of an OCType instance suitable for formatting.
 * :param ptr: A pointer to the OCType instance.
 * :return: An OCStringRef containing the formatted description. The caller is responsible
 *          for releasing the returned string. Returns NULL if ptr is NULL.
 * :note: The actual formatting logic is determined by the `copyFormattingDesc`
 *        function pointer in the OCBase structure of the type.
 */
OCStringRef OCTypeCopyFormattingDesc(const void * ptr);

/**
 * :func: OCCopyDescription
 * :brief: Creates a descriptive string representation of an OCType instance.
 * This is often a more human-readable description than OCTypeCopyFormattingDesc.
 * :param ptr: A pointer to the OCType instance.
 * :return: An OCStringRef containing the description. The caller is responsible
 *          for releasing the returned string. Returns NULL if ptr is NULL.
 */
OCStringRef OCCopyDescription(const void * ptr);

/**
 * :func: OCGetTypeID
 * :brief: Gets the OCTypeID of an OCType instance.
 * :param ptr: A pointer to the OCType instance.
 * :return: The OCTypeID of the instance. Returns _kOCNotATypeID if ptr is NULL.
 */
OCTypeID OCGetTypeID(const void * ptr);

/**
 * :struct: __OCBase
 * :brief: The fundamental base structure for all OCTypes.
 *
 * This structure is embedded as the first member in all OCType-compatible
 * objects. It holds common information like the type identifier, retain count,
 * and function pointers for essential operations.
 *
 * :ivar typeID: The type identifier for this object.
 * :ivar retainCount: The retain count for this object.
 * :ivar finalize: Function pointer for finalizing (deallocating) the object.
 *                 This is called when the retain count drops to zero.
 *                 :param ptr: A pointer to the object to be finalized.
 * :ivar equal: Function pointer for comparing this object with another for equality.
 *              :param ptr1: A pointer to the first object.
 *              :param ptr2: A pointer to the second object.
 *              :return: True if the objects are considered equal, false otherwise.
 * :ivar copyFormattingDesc: Function pointer for creating a formatted string description of the object.
 *                           :param cf: A reference to the object (OCTypeRef).
 *                           :return: An OCStringRef containing the formatted description. The caller is
 *                                    responsible for releasing this string.
 */
typedef struct __OCBase {
    OCTypeID typeID;
    uint32_t retainCount;
    void (*finalize)(const void *);
    bool (*equal)(const void *, const void *);
    OCStringRef (*copyFormattingDesc)(OCTypeRef cf);
} OCBase;

/** @} */ // end of OCType group

#endif
