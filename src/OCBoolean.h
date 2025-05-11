//
//  OCBoolean.h
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//

#ifndef OCBoolean_h
#define OCBoolean_h

#include "OCLibrary.h"

/** @defgroup OCBoolean OCBoolean */
/** @addtogroup OCBoolean
 *  @{
 */

/**
 * .. _OCBoolean_h:
 *
 * OCBoolean.h
 * ===========
 *
 * Defines the OCBooleanRef type and related functions for representing Boolean values.
 *
 * OCBoolean provides a way to work with Boolean logic using OCType objects.
 * It defines two singletons, kOCBooleanTrue and kOCBooleanFalse, to represent
 * true and false values respectively.
 */

/**
 * :type OCBooleanRef:
 * :brief: An opaque reference to an immutable OCBoolean object.
 *
 * OCBooleanRef is used to represent Boolean values (true or false) within the
 * OCTypes framework. These objects are singletons and should not be retained
 * or released.
 */
typedef const struct __OCBoolean * OCBooleanRef;

/**
 * :func: OCBooleanGetTypeID
 * :brief: Returns the unique type identifier for the OCBoolean class.
 * :return: The OCTypeID associated with OCBoolean objects. This ID is used to
 *          distinguish OCBoolean objects from other OCType objects.
 */
OCTypeID OCBooleanGetTypeID(void);

/**
 * :var kOCBooleanTrue:
 * :brief: The immutable singleton OCBooleanRef object representing the Boolean value 'true'.
 * :discussion: This object should be used whenever a true Boolean value is needed.
 *              Do not attempt to allocate or deallocate this object.
 */
extern const OCBooleanRef kOCBooleanTrue;

/**
 * :var kOCBooleanFalse:
 * :brief: The immutable singleton OCBooleanRef object representing the Boolean value 'false'.
 * :discussion: This object should be used whenever a false Boolean value is needed.
 *              Do not attempt to allocate or deallocate this object.
 */
extern const OCBooleanRef kOCBooleanFalse;

/**
 * :func: OCBooleanGetValue
 * :brief: Retrieves the primitive Boolean value (true or false) from an OCBooleanRef object.
 * :param boolean: The OCBooleanRef object to query. Must be either kOCBooleanTrue or kOCBooleanFalse.
 * :return: Returns `true` if the specified OCBooleanRef is kOCBooleanTrue.
 *          Returns `false` if the specified OCBooleanRef is kOCBooleanFalse.
 *          The behavior is undefined if an invalid OCBooleanRef is passed.
 */
bool OCBooleanGetValue(OCBooleanRef boolean);

/** @} */ // end of OCBoolean group

#endif /* OCBoolean_h */
