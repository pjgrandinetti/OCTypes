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

#ifdef __cplusplus
extern "C" {
#endif

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
#define IF_NO_OBJECT_EXISTS_RETURN(OBJECT, X) \
    do { \
        if (NULL == OBJECT) { \
            fprintf(stderr, "*** WARNING - %s - object doesn't exist.\n", __func__); \
            return X; \
        } \
    } while (0)

/**
 * @brief Defines an identifier for an OCType.
 * @ingroup OCType
 */
typedef uint32_t OCTypeID;

/**
 * @brief Return value constants.
 * @ingroup OCType
 */
enum CommonConstants {
    kOCNotFound = -1  /**< Value returned when an item is not found. */
};

/**
 * @brief Special OCTypeID constants.
 * @ingroup OCType
 */
enum SpecialTypeIDs {
    _kOCNotATypeID = 0  /**< Represents an invalid or uninitialized OCTypeID. */
};

/**
 * @brief Function type for finalizing an OCType-compatible object.
 */
typedef void (*OCFinalizerFunc)(const void *);

/**
 * @brief Compares two OCType instances for equality.
 * @param theType1 Pointer to the first OCType.
 * @param theType2 Pointer to the second OCType.
 * @return true if equal, false otherwise.
 * @ingroup OCType
 */
bool OCTypeEqual(const void *theType1, const void *theType2);

/**
 * @brief Registers a new OCType with the system.
 * @param typeName A null-terminated C string representing the type name.
 * @return The OCTypeID assigned, or _kOCNotATypeID on failure.
 * @ingroup OCType
 */
OCTypeID OCRegisterType(char *typeName);

/**
 * @brief Retrieves the retain count of an OCType instance.
 * @param ptr Pointer to the OCType instance.
 * @return The retain count, or 0 if NULL.
 * @ingroup OCType
 */
int OCTypeGetRetainCount(const void * ptr);

/**
 * @brief Releases an OCType instance by decrementing its retain count.
 *
 * When the retain count reaches zero, the finalizer (if any) is invoked.
 *
 * @param ptr Pointer to the OCType to release.
 * @ingroup OCType
 */
void OCRelease(const void *ptr);

/**
 * @brief Retains an OCType instance by incrementing its retain count.
 *
 * Ownership follows the convention:
 * - Functions named with "Create" or "Copy" return owned objects (caller must release).
 * - Other functions return autoreleased or borrowed objects.
 *
 * @param ptr Pointer to the OCType to retain.
 * @return The same pointer, or NULL if input was NULL.
 * @ingroup OCType
 */
const void *OCRetain(const void *ptr);

/**
 * @brief Returns a formatted string description of an OCType.
 * @param ptr Pointer to the OCType instance.
 * @return A descriptive OCStringRef or NULL.
 * @ingroup OCType
 */
OCStringRef OCTypeCopyFormattingDesc(const void *ptr);

/**
 * @brief Returns a generic description of an OCType instance.
 * @param ptr Pointer to the OCType instance.
 * @return A description string.
 * @ingroup OCType
 */
OCStringRef OCCopyDescription(const void *ptr);

/**
 * @brief Checks if an OCType instance is static.
 * @param ptr Pointer to the instance.
 * @return true if static, false otherwise.
 * @ingroup OCType
 */
bool OCTypeGetStaticInstance(const void * ptr);

/**
 * @brief Marks an OCType instance as static or non-static.
 * @param ptr Pointer to the instance.
 * @param static_instance Whether the object is static.
 * @ingroup OCType
 */
void OCTypeSetStaticInstance(const void * ptr, bool static_instance);

/**
 * @brief Checks if an OCType instance has been finalized.
 * @param ptr Pointer to the instance.
 * @return true if finalized, false otherwise.
 * @ingroup OCType
 */
bool OCTypeGetFinalized(const void * ptr);

/**
 * @brief Returns the type ID of an OCType instance.
 * @param ptr Pointer to the instance.
 * @return A valid OCTypeID or _kOCNotATypeID.
 * @ingroup OCType
 */
OCTypeID OCGetTypeID(const void *ptr);

/**
 * @brief Returns the registered name of a type given an instance.
 * @param ptr Pointer to the instance.
 * @return Type name string.
 * @ingroup OCType
 */
const char *OCTypeIDName(const void * ptr);

/** \cond INTERNAL */

/**
 * @brief Base structure shared by all OCType-compatible objects.
 * Not intended for direct use outside the type system.
 */
typedef struct __OCBase {
    OCTypeID typeID;
    uint32_t retainCount;
    OCFinalizerFunc finalize;
    bool finalized;
    bool (*equal)(const void *, const void *);
    OCStringRef (*copyFormattingDesc)(OCTypeRef cf);
    bool static_instance;

#ifdef DEBUG
    const char *allocFile;  /**< Source file of allocation (debug only) */
    int allocLine;          /**< Source line of allocation (debug only) */
    bool tracked;           /**< Is this object tracked for leak detection? */
#endif

} OCBase;

/**
 * @brief Allocates and initializes a new OCType-compatible object.
 *
 * Intended to be used by "Create" functions for types inheriting from OCBase.
 *
 * @param size Size of the full structure to allocate.
 * @param typeID The registered OCTypeID.
 * @param finalize Optional finalizer function.
 * @param equal Optional equality comparator.
 * @param copyDesc Optional formatting function.
 * @param file Source file for tracking (use __FILE__).
 * @param line Source line for tracking (use __LINE__).
 * @return Pointer to a fully initialized structure.
 */
void *OCTypeAllocate(size_t size,
                     OCTypeID typeID,
                     OCFinalizerFunc finalize,
                     bool (*equal)(const void *, const void *),
                     OCStringRef (*copyDesc)(OCTypeRef cf),
                     const char *file,
                     int line);

/**
 * @brief Macro for typed allocation of OCType-compatible objects.
 * Automatically injects debug information.
 */
#define OCTypeAlloc(TYPE, TYPEID, FINALIZER, EQUAL_FN, DESC_FN) \
    (TYPE *)OCTypeAllocate(sizeof(TYPE), TYPEID, FINALIZER, EQUAL_FN, DESC_FN, __FILE__, __LINE__)


/**
 * @brief Reports any OCTypeRef-tracked objects that were not finalized.
 * Only available in debug builds.
 */
#ifdef DEBUG
void OCTypeReportLeaks(void);
#endif

/** \endcond */
/** @} */ // end of OCType group

#ifdef __cplusplus
}
#endif

#endif // OCType_h
