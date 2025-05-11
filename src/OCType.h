/**
 * @file OCType.h
 * @brief Defines the core types and functions for the OCTypes library.
 *
 * This file contains the fundamental type definitions, including OCTypeID,
 * OCIndex, and the base OCTypeRef, as well as functions for type registration,
 * memory management (retain/release), and basic type introspection.
 */

#ifndef OCType_h
#define OCType_h

#include "OCLibrary.h"

/** @defgroup OCType OCType
 *  @brief Core type definitions and memory management functions.
 *  @{
 */

/**
 * @brief Macro to check if an object is NULL and return a specified value if it is.
 * @param OBJECT The object to check.
 * @param X The value to return if OBJECT is NULL.
 * @ingroup OCType
 */
#define IF_NO_OBJECT_EXISTS_RETURN(OBJECT,X) if(NULL==OBJECT) {printf("*** WARNING - %s - object doesn't exist.\n",__func__); return X;}

/**
 * @brief Defines an identifier for an OCType.
 * @ingroup OCType
 */
typedef uint32_t OCTypeID;

/**
 * @brief Defines an index type, typically used for array or collection indexing.
 * @ingroup OCType
 */
typedef int32_t OCIndex;

/**
 * @brief Defines common constant values.
 * @ingroup OCType
 */
enum CommonConstants {
    /** Represents a value not found, typically used as a return value from search functions. */
    kOCNotFound = -1
};

/**
 * @brief A reference to an opaque OCType structure.
 * This is the base type for all objects in the OCTypes system.
 * @ingroup OCType
 */
typedef struct __OCType * OCTypeRef;

/**
 * @brief Defines special type ID values.
 * @ingroup OCType
 */
enum SpecialTypeIDs {
    /** Represents an invalid or uninitialized OCTypeID. */
    _kOCNotATypeID = 0
};

/**
 * @brief Compares two OCType instances for equality.
 * @param theType1 A pointer to the first OCType instance.
 * @param theType2 A pointer to the second OCType instance.
 * @return True if the types are considered equal, false otherwise.
 * @ingroup OCType
 */
bool OCTypeEqual(const void * theType1, const void * theType2);

/**
 * @brief Registers a new OCType with the system.
 * @param typeName A string representing the name of the type to register.
 * @return The OCTypeID assigned to the newly registered type.
 * Returns _kOCNotATypeID if registration fails.
 * @ingroup OCType
 */
OCTypeID OCRegisterType(char *typeName);

/**
 * @brief Releases an OCType instance, decrementing its retain count.
 * @param ptr A pointer to the OCType instance to release.
 * @ingroup OCType
 */
void OCRelease(const void * ptr);

/**
 * @brief Retains an OCType instance, incrementing its retain count.
 * @param ptr A pointer to the OCType instance to retain.
 * @return A pointer to the retained OCType instance (the same as ptr), or NULL if ptr is NULL.
 * @ingroup OCType
 */
const void *OCRetain(const void * ptr);

/**
 * @brief A reference to an immutable OCString instance.
 * @ingroup OCType
 */
typedef const struct __OCString * OCStringRef;

/**
 * @brief Creates a string representation of an OCType instance suitable for formatting.
 * @param ptr A pointer to the OCType instance.
 * @return An OCStringRef containing the formatted description.
 * Returns NULL if ptr is NULL.
 * @ingroup OCType
 */
OCStringRef OCTypeCopyFormattingDesc(const void * ptr);

/**
 * @brief Creates a descriptive string representation of an OCType instance.
 * @param ptr A pointer to the OCType instance.
 * @return An OCStringRef containing the description.
 * Returns NULL if ptr is NULL.
 * @ingroup OCType
 */
OCStringRef OCCopyDescription(const void * ptr);

/**
 * @brief Gets the OCTypeID of an OCType instance.
 * @param ptr A pointer to the OCType instance.
 * @return The OCTypeID of the instance, or _kOCNotATypeID if ptr is NULL.
 * @ingroup OCType
 */
OCTypeID OCGetTypeID(const void * ptr);

/**
 * @brief The fundamental base structure for all OCTypes.
 *
 * This structure is embedded as the first member in all OCType-compatible
 * objects. It holds common information like the type identifier, retain count,
 * and function pointers for essential operations.
 * @ingroup OCType
 */
typedef struct __OCBase {
    OCTypeID typeID; /**< The type identifier for this object. */
    uint32_t retainCount; /**< The retain count for this object. */
    void (*finalize)(const void *); /**< Function pointer for finalizing (deallocating) the object. */
    bool (*equal)(const void *, const void *); /**< Function pointer for comparing objects for equality. */
    OCStringRef (*copyFormattingDesc)(OCTypeRef cf); /**< Function pointer for creating a formatted string description. */
} OCBase;

/** @} */ // end of OCType group

#endif
