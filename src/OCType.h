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
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations (the actual definitions will be in OCLibrary.h)
typedef const struct impl_OCType *OCTypeRef;
typedef const struct impl_OCString *OCStringRef;

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
#define IF_NO_OBJECT_EXISTS_RETURN(OBJECT, X)                                        \
    do {                                                                             \
        if (NULL == OBJECT) {                                                        \
            fprintf(stderr, "*** WARNING - %s - object doesn't exist.\n", __func__); \
            return X;                                                                \
        }                                                                            \
    } while (0)
/**
 * @brief Return value constants.
 * @ingroup OCType
 */
enum CommonConstants {
    kOCNotFound = -1 /**< Value returned when an item is not found. */
};
/**
 * @brief Special OCTypeID constants.
 * @ingroup OCType
 */
enum SpecialTypeIDs {
    kOCNotATypeID = 0 /**< Represents an invalid or uninitialized OCTypeID. */
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
int OCTypeGetRetainCount(const void *ptr);
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
 * @brief Returns a schema-bound JSON representation of an OCType instance.
 *
 * This function serializes the given OCType instance to a cJSON object
 * following your library's schema conventions. The output does not include
 * type information (e.g., no "__type__" field), and assumes that any
 * deserialization will occur in a context where the expected type is known
 * a priori.
 *
 * The returned cJSON object must be freed by the caller using cJSON_Delete().
 *
 * @note The resulting JSON is not self-describing. Deserialization requires
 *       external knowledge of the expected OCType subclass.
 *
 * @param obj Pointer to the OCType instance. If NULL, returns a JSON null.
 * @return A cJSON object representing the instance, or cJSON null if obj is NULL.
 * @ingroup OCType
 */
cJSON *OCTypeCopyJSON(OCTypeRef obj);
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
bool OCTypeGetStaticInstance(const void *ptr);
/**
 * @brief Marks an OCType instance as static or non-static.
 * @param ptr Pointer to the instance.
 * @param static_instance Whether the object is static.
 * @ingroup OCType
 */
void OCTypeSetStaticInstance(const void *ptr, bool static_instance);
/**
 * @brief Checks if an OCType instance has been finalized.
 * @param ptr Pointer to the instance.
 * @return true if finalized, false otherwise.
 * @ingroup OCType
 */
bool OCTypeGetFinalized(const void *ptr);
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
const char *OCTypeIDName(const void *ptr);
const char *OCTypeNameFromTypeID(OCTypeID typeID);
/** \cond INTERNAL */
/**
 * @brief Base structure for all OCType-compatible objects.
 *
 * This structure provides the common fields and virtual method table
 * required for polymorphic behavior in the OCTypes library.
 *
 * All OCType-compatible objects must start with this structure as their first member.
 */
typedef struct impl_OCBase {
    OCTypeID typeID;       // 2 bytes
    uint16_t retainCount;  // 2 bytes
    int allocLine;         // 4 bytes
    // 8 bytes total - perfectly aligned for pointers
    // Virtual methods (8-byte aligned)
    void (*finalize)(const void *);
    bool (*equal)(const void *, const void *);
    OCStringRef (*copyFormattingDesc)(OCTypeRef);
    cJSON *(*copyJSON)(const void *);
    void *(*copyDeep)(const void *);
    void *(*copyDeepMutable)(const void *);
    // Debug info
    const char *allocFile;  // 8 bytes
    // Flags packed together
    bool static_instance;  // 1 byte
    bool finalized;        // 1 byte
    bool tracked;          // 1 byte
} OCBase;
//  Below is a future plan to optimize OCBase.  It will require
//  rewriting the getters and setters for the bools, and also
//  modifying the OCLeakTracker to not capture the allocFile and line
// /**
//  * @brief Base structure for all OCType-compatible objects.
//  *
//  * This structure provides the common fields and virtual method table
//  * required for polymorphic behavior in the OCTypes library.
//  *
//  * All OCType-compatible objects must start with this structure as their first member.
//  */
// typedef struct impl_OCBase {
//     OCTypeID typeID;        // 2 bytes (uint16_t)
//     uint16_t retainCount;   // 2 bytes
//     uint32_t flags;         // 4 bytes (replaces 3 separate bools)
//     // 8 bytes total - perfectly aligned for 64-bit pointers
//     // Virtual methods (8-byte aligned)
//     void (*finalize)(const void *);
//     bool (*equal)(const void *, const void *);
//     OCStringRef (*copyFormattingDesc)(OCTypeRef);
//     cJSON *(*copyJSON)(const void *);
//     void *(*copyDeep)(const void *);
//     void *(*copyDeepMutable)(const void *);
// } OCBase;
// // Flag bit definitions for OCBase.flags
// #define OC_FLAG_STATIC_INSTANCE  (1U << 0)   // 0x00000001
// #define OC_FLAG_FINALIZED        (1U << 1)   // 0x00000002
// #define OC_FLAG_TRACKED          (1U << 2)   // 0x00000004
// // 29 bits remaining (bits 3-31) for future boolean flags
// // Accessor macros for backward compatibility
// #define OCTypeGetStaticInstance(obj)  ((((OCBase*)(obj))->flags & OC_FLAG_STATIC_INSTANCE) != 0)
// #define OCTypeSetStaticInstance(obj, val) \
//     do { \
//         if (val) ((OCBase*)(obj))->flags |= OC_FLAG_STATIC_INSTANCE; \
//         else ((OCBase*)(obj))->flags &= ~OC_FLAG_STATIC_INSTANCE; \
//     } while(0)
// #define OCTypeGetFinalized(obj)   ((((OCBase*)(obj))->flags & OC_FLAG_FINALIZED) != 0)
// #define OCTypeSetFinalized(obj, val) \
//     do { \
//         if (val) ((OCBase*)(obj))->flags |= OC_FLAG_FINALIZED; \
//         else ((OCBase*)(obj))->flags &= ~OC_FLAG_FINALIZED; \
//     } while(0)
// #define OCTypeGetTracked(obj)     ((((OCBase*)(obj))->flags & OC_FLAG_TRACKED) != 0)
// #define OCTypeSetTracked(obj, val) \
//     do { \
//         if (val) ((OCBase*)(obj))->flags |= OC_FLAG_TRACKED; \
//         else ((OCBase*)(obj))->flags &= ~OC_FLAG_TRACKED; \
//     } while(0)
/**
 * @brief Allocates and initializes a new OCType-compatible object with virtual method table.
 *
 * This function serves as the primary constructor for all OCType-compatible objects. It allocates
 * memory for the specified structure size, initializes the base OCBase fields, and sets up the
 * virtual method table for polymorphic behavior. The function is designed to be used internally
 * by "Create" functions for concrete types that inherit from OCBase.
 *
 * The allocated object starts with a retain count of 1 and must be released with OCRelease()
 * when no longer needed. Debug tracking information is automatically recorded for memory
 * leak detection and debugging purposes.
 *
 * @param size Size in bytes of the full structure to allocate. Must be at least sizeof(OCBase).
 * @param typeID The registered OCTypeID obtained from OCRegisterType(). Must not be kOCNotATypeID.
 * @param finalize Optional finalizer function called when retain count reaches zero.
 *                 Should clean up any resources owned by the object. May be NULL.
 * @param equal Optional equality comparison function for this type. Should compare semantic
 *              equality between two instances of the same type. May be NULL.
 * @param copyFormattingDesc Optional function to create a human-readable string description
 *                           of the object. Used for debugging and logging. May be NULL.
 * @param copyJSON Optional function to serialize the object to JSON format following the
 *                 library's schema conventions. May be NULL.
 * @param copyDeep Optional function to create an immutable deep copy of the object.
 *                 Should allocate and return a semantically equivalent but independent object. May be NULL.
 * @param copyDeepMutable Optional function to create a mutable deep copy of the object.
 *                        Similar to copyDeep but allows modification of the result. May be NULL.
 * @param file Source file name for debug tracking. Use __FILE__ macro.
 * @param line Source line number for debug tracking. Use __LINE__ macro.
 *
 * @return Pointer to a fully initialized OCType-compatible structure with retain count 1,
 *         or NULL if allocation fails or invalid parameters are provided.
 *
 * @note The returned object must be released with OCRelease() when no longer needed.
 * @note This function is typically wrapped by the OCTypeAlloc() macro for type safety.
 * @note All function pointers in the virtual method table are optional and may be NULL.
 *
 * @see OCTypeAlloc() for the type-safe macro wrapper
 * @see OCRegisterType() for obtaining valid OCTypeID values
 * @see OCRelease() for proper memory cleanup
 *
 * @ingroup OCType
 */
void *OCTypeAllocate(
    size_t size,
    OCTypeID typeID,
    void (*finalize)(const void *),
    bool (*equal)(const void *, const void *),
    OCStringRef (*copyFormattingDesc)(OCTypeRef),
    cJSON *(*copyJSON)(const void *),
    void *(*copyDeep)(const void *),
    void *(*copyDeepMutable)(const void *),
    const char *file,
    int line);
/**
 * @brief Macro for typed allocation of OCType-compatible objects.
 * Automatically injects debug information.
 */
#define OCTypeAlloc(        \
    TYPE,                   \
    TYPEID,                 \
    FINALIZE,               \
    EQUAL_FN,               \
    DESC_FN,                \
    JSON_FN,                \
    COPY_FN,                \
    MUTABLE_COPY_FN)        \
    (TYPE *)OCTypeAllocate( \
        sizeof(TYPE),       \
        TYPEID,             \
        FINALIZE,           \
        EQUAL_FN,           \
        DESC_FN,            \
        JSON_FN,            \
        COPY_FN,            \
        MUTABLE_COPY_FN,    \
        __FILE__,           \
        __LINE__)
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
/** @} */  // end of OCType group
#ifdef __cplusplus
}
#endif
#endif  // OCType_h
