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
    kOCNotATypeID = 0  /**< Represents an invalid or uninitialized OCTypeID. */
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
 * @brief Performs a deep copy of an OCType object.
 *
 * Allocates and returns a new immutable instance that is semantically 
 * equal to the original but independent in memory.
 *
 * The returned object must be released with OCRelease().
 *
 * @param obj The OCType object to copy.
 * @return A new deep-copied object, or NULL on failure.
 * @ingroup OCType
 */
void *OCTypeDeepCopy(const void *obj);

/**
 * @brief Performs a deep copy of an OCType object, returning a mutable copy.
 *
 * Allocates and returns a new mutable instance that is semantically equal 
 * to the original but allows modification and is independent in memory.
 *
 * The returned object must be released with OCRelease().
 *
 * @param obj The OCType object to copy.
 * @return A new mutable deep-copied object, or NULL on failure.
 * @ingroup OCType
 */
void *OCTypeDeepCopyMutable(const void *obj);

/**
 * @brief Registers a new OCType with the system.
 * @param typeName A null-terminated C string representing the type name.
 * @return The OCTypeID assigned, or kOCNotATypeID on failure.
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
 * @return A valid OCTypeID or kOCNotATypeID.
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

const char *OCTypeNameFromTypeID(OCTypeID typeID);

/** \cond INTERNAL */

typedef struct impl_OCBase {
    OCTypeID typeID;
    uint32_t retainCount;
    bool static_instance;
    bool finalized;

    // Virtual methods
    void (*finalize)(const void *);
    bool (*equal)(const void *, const void *);
    OCStringRef (*copyFormattingDesc)(OCTypeRef);

    void *(*copyDeep)(const void *);
    void *(*copyDeepMutable)(const void *);

    // Debug info
    const char *allocFile;
    int allocLine;
    bool tracked;

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
 * @param copyDeep Optional deep copy function.
 * @param file Source file for tracking (use __FILE__).
 * @param line Source line for tracking (use __LINE__).
 * @return Pointer to a fully initialized structure.
 */
void *OCTypeAllocate(size_t size, 
                     OCTypeID typeID,
                     OCFinalizerFunc finalize,
                     bool (*equal)(const void *, const void *),
                     OCStringRef (*descFn)(OCTypeRef),
                     void *(*deepCopy)(const void *),
                     void *(*deepCopyMutable)(const void *),
                     const char *file, int line);
/**
 * @brief Macro for typed allocation of OCType-compatible objects.
 * Automatically injects debug information.
 */
#define OCTypeAlloc(TYPE, TYPEID, FINALIZE, EQUAL_FN, DESC_FN, COPY_FN, MUTABLE_COPY_FN) \
    (TYPE *)OCTypeAllocate(sizeof(TYPE), TYPEID, FINALIZE, EQUAL_FN, DESC_FN, COPY_FN, MUTABLE_COPY_FN, __FILE__, __LINE__)
/**
 * @brief Finalizes and cleans up resources used by the OCTypes library.
 *
 * This function releases all memory and internal state associated with the OCTypes
 * library. It should be called near the end of the program, but only if one or more
 * OCType-compatible objects were registered using `OCRegisterType`.
 *
 * Calling this function ensures that all resources are properly deallocated and that
 * leak detection tools (e.g., LSAN, ASan) report accurately.
 *
 * @note This function is only necessary if `OCRegisterType` was invoked during the program's execution.
 *
 * @ingroup OCType
 */
void OCTypesShutdown(void);

/** \endcond */
/** @} */ // end of OCType group

#ifdef __cplusplus
}
#endif

#endif // OCType_h
