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

#include "OCLibrary.h"

/**
 * @defgroup OCBoolean OCBoolean
 * @brief Immutable Boolean type abstraction in OCTypes.
 * @{
 */

/**
 * @brief An opaque reference to an immutable OCBoolean object.
 *
 * OCBooleanRef is used to represent Boolean values (`true` or `false`)
 * within the OCTypes framework. These objects are singletons and should
 * not be retained or released.
 *
 * @ingroup OCBoolean
 */
typedef const struct __OCBoolean *OCBooleanRef;

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

/** @} */ // end of OCBoolean group

#endif /* OCBoolean_h */
