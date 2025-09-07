/**
 * @file OCNull.h
 * @brief Defines the OCNullRef type and associated functions.
 *
 * OCNull provides a way to represent null/nil values in OCType collections.
 * It defines a singleton instance—kOCNull—representing an explicit null value,
 * which is distinct from a missing/absent value (represented by NULL pointer).
 * This follows the Core Foundation pattern of CFNull.
 */
#ifndef OCNull_h
#define OCNull_h
#include "OCType.h"  // Foundation types and forward declarations
#include "cJSON.h"   // For JSON functionality
/**
 * @defgroup OCNull OCNull
 * @brief Immutable null type abstraction in OCTypes.
 * @{
 */
/**
 * @brief Returns the unique type identifier for the OCNull class.
 *
 * @return The OCTypeID associated with OCNull objects.
 * This ID distinguishes OCNull from other OCType-compatible types.
 *
 * @ingroup OCNull
 */
OCTypeID OCNullGetTypeID(void);
/**
 * @brief The immutable singleton OCNullRef representing an explicit null value.
 *
 * Use this whenever an explicit null value is required in OCType collections.
 * This is distinct from NULL pointer which indicates absence/missing value.
 *
 * @code
 * OCArrayRef array = OCArrayCreateWithValues((OCTypeRef[]){
 *     STR("first"),
 *     kOCNull,        // explicit null value
 *     STR("third")
 * }, 3);
 * @endcode
 *
 * @ingroup OCNull
 */
extern const OCNullRef kOCNull;
/**
 * @brief Returns the singleton null value.
 *
 * This is equivalent to using kOCNull directly, but provided for API consistency.
 *
 * @return The singleton OCNullRef instance (same as kOCNull).
 *
 * @ingroup OCNull
 */
OCNullRef OCNullGet(void);
/**
 * @brief Checks if the given object is the null singleton.
 *
 * @param obj The object to test.
 * @return true if obj is kOCNull, false otherwise.
 *
 * @ingroup OCNull
 */
bool OCNullIsNull(OCTypeRef obj);
/**
 * @brief Creates a JSON representation of the null value.
 *
 * @param null The OCNullRef to serialize (should be kOCNull).
 * @param typed Whether to create typed JSON (with type metadata).
 * @param outError Optional pointer to receive error information.
 * @return A new cJSON null object on success, or NULL on failure.
 *
 * @ingroup OCNull
 */
cJSON *OCNullCopyAsJSON(OCNullRef null, bool typed, OCStringRef *outError);
/**
 * @brief Creates an OCNull from a JSON null value.
 *
 * @param json A cJSON null object.
 * @param outError Optional pointer to receive error information.
 * @return kOCNull on success, or NULL on failure.
 *
 * @ingroup OCNull
 */
OCNullRef OCNullCreateFromJSON(cJSON *json, OCStringRef *outError);
/** @} */  // end of OCNull group
#ifdef __cplusplus
}
#endif
#endif /* OCNull_h */
