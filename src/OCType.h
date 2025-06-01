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

/**
 * @defgroup OCType OCType
 * @brief Core type definitions and memory management functions.
 * @{
 */

/**
 * @brief Macro to check if an object is NULL and return a specified value if it is.
 *
 * @param OBJECT The object to check.
 * @param X The value to return if OBJECT is NULL.
 *
 * @ingroup OCType
 */

/** \cond INTERNAL */
#define IF_NO_OBJECT_EXISTS_RETURN(OBJECT, X) \
    if (NULL == OBJECT) { \
        fprintf(stderr, "*** WARNING - %s - object doesn't exist.\n", __func__); \
        return X; \
    }
/** \endcond */

/**
 * @brief Defines an identifier for an OCType.
 * @ingroup OCType
 */
typedef uint32_t OCTypeID;

/**
 * @brief Defines common constant values.
 * @ingroup OCType
 */
enum CommonConstants {
    /** Value returned when an item is not found. */
    kOCNotFound = -1
};

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
 *
 * @param theType1 Pointer to the first OCType.
 * @param theType2 Pointer to the second OCType.
 * @return true if equal, false otherwise.
 *
 * @ingroup OCType
 */
bool OCTypeEqual(const void *theType1, const void *theType2);

/**
 * @brief Registers a new OCType with the system.
 *
 * @param typeName A null-terminated C string representing the type name.
 * @return The OCTypeID assigned, or _kOCNotATypeID on failure.
 *
 * @ingroup OCType
 */
OCTypeID OCRegisterType(char *typeName);


/**
 * @brief Retrieves the retain count of an OCType instance.
 *
 * This function returns the current retain count of the specified OCType instance.
 * The retain count indicates how many references to the object exist.
 *
 * @param ptr Pointer to the OCType instance.
 * @return The retain count of the object, or 0 if the pointer is NULL.
 *
 * @ingroup OCType
 */
int OCTypeGetRetainCount(const void * ptr);

/**
 * @brief Releases an OCType instance by decrementing its retain count.
 *
 * @param ptr Pointer to the OCType to release.
 *
 * @ingroup OCType
 */
void OCRelease(const void *ptr);

/**
 * @brief Retains an OCType instance by incrementing its retain count.
 *
 * @param ptr Pointer to the OCType to retain.
 * @return The same pointer, or NULL if input was NULL.
 *
 * @ingroup OCType
 */
const void *OCRetain(const void *ptr);

/**
 * @brief Generates a formatted string description of an OCType.
 *
 * @param ptr Pointer to the OCType instance.
 * @return A new OCStringRef describing the object, or NULL.
 *
 * @ingroup OCType
 */
OCStringRef OCTypeCopyFormattingDesc(const void *ptr);

/**
 * @brief Creates a descriptive string representation of an OCType instance.
 *
 * @param ptr Pointer to the OCType instance.
 * @return A descriptive OCStringRef, or NULL.
 *
 * @ingroup OCType
 */
OCStringRef OCCopyDescription(const void *ptr);


/**
 * @brief Checks if an OCType instance is marked as static.
 *
 * This function determines whether the specified OCType instance is flagged
 * as a static instance. Static instances are not subject to automatic memory
 * management and are expected to persist for the lifetime of the program.
 *
 * @param ptr Pointer to the OCType instance to check.
 * @return true if the instance is static, false otherwise.
 *
 * @ingroup OCType
 */
bool OCTypeGetStaticInstance(const void * ptr);


/**
 * @brief Sets the static instance flag for an OCType instance.
 *
 * This function allows marking an OCType instance as static or non-static.
 * Static instances are not subject to automatic memory management and are
 * expected to persist for the lifetime of the program.
 *
 * @param ptr Pointer to the OCType instance to modify.
 * @param static_instance Boolean value indicating whether the instance should
 *        be marked as static (true) or non-static (false).
 *
 * @ingroup OCType
 */
void OCTypeSetStaticInstance(const void * ptr, bool static_instance);

/**
 * @brief Gets the OCTypeID associated with an OCType instance.
 *
 * @param ptr Pointer to the OCType instance.
 * @return A valid OCTypeID, or _kOCNotATypeID.
 *
 * @ingroup OCType
 */
OCTypeID OCGetTypeID(const void *ptr);

/** \cond INTERNAL */
/**
 * @brief Base structure shared by all OCType-compatible objects.
 *
 * @ingroup OCType
 */
typedef struct __OCBase {
    OCTypeID typeID;  /**< Type identifier. */
    uint32_t retainCount;  /**< Reference count. */
    void (*finalize)(const void *);  /**< Finalizer function. */
    bool (*equal)(const void *, const void *);  /**< Equality comparator. */
    OCStringRef (*copyFormattingDesc)(OCTypeRef cf);  /**< Description formatter. */
    bool static_instance;  /**< Indicates if the instance is static. */
} OCBase;
/** \endcond */

/** @} */ // end of OCType group

#endif // OCType_h