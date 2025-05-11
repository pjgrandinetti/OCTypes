//
//  OCBoolean.h
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//

#ifndef OCBoolean_h
#define OCBoolean_h

#include "OCLibrary.h"

/**
 * @file OCBoolean.h
 * @brief Defines the OCBooleanRef type and related functions for representing Boolean values.
 *
 * OCBoolean provides a way to work with Boolean logic using OCType objects.
 * It defines two singletons, kOCBooleanTrue and kOCBooleanFalse, to represent
 * true and false values respectively.
 */

/**
 * @typedef OCBooleanRef
 * @brief An opaque reference to an immutable OCBoolean object.
 *
 * OCBooleanRef is used to represent Boolean values (true or false) within the
 * OCTypes framework. These objects are singletons and should not be retained
 * or released.
 */
typedef const struct __OCBoolean * OCBooleanRef;

/**
 * @brief Returns the unique type identifier for the OCBoolean class.
 * @return The OCTypeID associated with OCBoolean objects. This ID is used to
 *         distinguish OCBoolean objects from other OCType objects.
 */
OCTypeID OCBooleanGetTypeID(void);

/**
 * @brief The immutable singleton OCBooleanRef object representing the Boolean value 'true'.
 * @discussion This object should be used whenever a true Boolean value is needed.
 *             Do not attempt to allocate or deallocate this object.
 */
extern const OCBooleanRef kOCBooleanTrue;

/**
 * @brief The immutable singleton OCBooleanRef object representing the Boolean value 'false'.
 * @discussion This object should be used whenever a false Boolean value is needed.
 *             Do not attempt to allocate or deallocate this object.
 */
extern const OCBooleanRef kOCBooleanFalse;

/**
 * @brief Retrieves the primitive Boolean value (true or false) from an OCBooleanRef object.
 * @param boolean The OCBooleanRef object to query. Must be either kOCBooleanTrue or kOCBooleanFalse.
 * @return Returns `true` if the specified OCBooleanRef is kOCBooleanTrue.
 * @return Returns `false` if the specified OCBooleanRef is kOCBooleanFalse.
 *         The behavior is undefined if an invalid OCBooleanRef is passed.
 */
bool OCBooleanGetValue(OCBooleanRef boolean);

#endif /* OCBoolean_h */
