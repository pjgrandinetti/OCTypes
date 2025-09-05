/**
 * @file OCBoolean.h
 * @brief Defines the OCBooleanRef type and associated functions.
 *
 * OCBoolean provides a way to work with Boolean logic using OCType objects.
 * It defines two singleton instances—kOCBooleanTrue and kOCBooleanFalse—
 * representing the logical values `true` and `false`, respectively.
 */
#ifndef OCBoolean_h
#define OCBoolean_h
#include "OCType.h"  // Foundation types and forward declarations
#include "cJSON.h"   // For JSON functionality
/**
 * @defgroup OCBoolean OCBoolean
 * @brief Immutable Boolean type abstraction in OCTypes.
 * @{
 */
/**
 * @brief Returns the unique type identifier for the OCBoolean class.
 *
 * @return The OCTypeID associated with OCBoolean objects.
 * This ID distinguishes OCBoolean from other OCType-compatible types.
 *
 * @ingroup OCBoolean
 */
OCTypeID OCBooleanGetTypeID(void);
/**
 * @brief The immutable singleton OCBooleanRef representing Boolean true.
 *
 * Use this whenever a true Boolean value is required in the OCType system.
 * Do not manually allocate or deallocate this object.
 *
 * @ingroup OCBoolean
 */
extern const OCBooleanRef kOCBooleanTrue;
/**
 * @brief The immutable singleton OCBooleanRef representing Boolean false.
 *
 * Use this whenever a false Boolean value is required in the OCType system.
 * Do not manually allocate or deallocate this object.
 *
 * @ingroup OCBoolean
 */
extern const OCBooleanRef kOCBooleanFalse;
/**
 * @brief Retrieves the primitive Boolean value (`true` or `false`) from an OCBooleanRef.
 *
 * @param boolean The OCBooleanRef to query. Must be either kOCBooleanTrue or kOCBooleanFalse.
 * @return `true` if `boolean` is kOCBooleanTrue; `false` if it is kOCBooleanFalse.
 * Behavior is undefined for invalid or uninitialized values.
 *
 * @ingroup OCBoolean
 */
bool OCBooleanGetValue(OCBooleanRef boolean);
/*!
 * @brief  Return the OCBoolean singleton corresponding to the given C bool.
 *         No ownership is transferred.
 *
 * @param  value  A C bool.
 * @return kOCBooleanTrue if value is true, else kOCBooleanFalse.
 */
OCBooleanRef OCBooleanGetWithBool(bool value);
/**
 * @brief Returns a new OCStringRef representing the Boolean value ("true" or "false").
 * @param boolean The OCBooleanRef to convert.
 * @return A new OCStringRef ("true" or "false"). Caller must release.
 * @ingroup OCBoolean
 */
OCStringRef OCBooleanCreateStringValue(OCBooleanRef boolean);
/**
 * @brief Creates a JSON representation of an OCBooleanRef.
 *
 * Since booleans are native JSON types, both typed and untyped serialization
 * produce the same result: a JSON boolean (true or false).
 *
 * @param boolean A valid OCBooleanRef (kOCBooleanTrue or kOCBooleanFalse).
 * @param typed Whether to include type information (ignored for booleans).
 * @param outError Optional pointer to receive an error string on failure.
 * @return A new cJSON boolean node, or cJSON null if input is NULL.
 *         Caller is responsible for managing the returned cJSON object.
 * @ingroup OCBoolean
 */
cJSON *OCBooleanCopyAsJSON(OCBooleanRef boolean, bool typed, OCStringRef *outError);

/**
 * @brief Creates an OCBooleanRef from a JSON boolean node.
 *
 * This function maps JSON true to `kOCBooleanTrue` and false to `kOCBooleanFalse`.
 *
 * @param json A cJSON boolean node.
 * @param json A cJSON node expected to be a boolean.
 * @param outError Optional pointer to receive an error string on failure.
 * @return `kOCBooleanTrue` or `kOCBooleanFalse`, or NULL if input is invalid.
 *         Returned object is a singleton and must not be released.
 * @ingroup OCBoolean
 */
OCBooleanRef OCBooleanCreateFromJSON(cJSON *json, OCStringRef *outError);

/** @} */  // end of OCBoolean group
#endif     /* OCBoolean_h */
