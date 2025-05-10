//
//  OCBoolean.h
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//

#ifndef OCBoolean_h
#define OCBoolean_h

#include "OCLibrary.h"

typedef const struct __OCBoolean * OCBooleanRef;

/**
 * Returns the unique type identifier for OCBoolean.
 * @return The OCTypeID for OCBoolean.
 */
OCTypeID OCBooleanGetTypeID(void);

/**
 * Immutable singleton representing Boolean true.
 */
extern const OCBooleanRef kOCBooleanTrue;

/**
 * Immutable singleton representing Boolean false.
 */
extern const OCBooleanRef kOCBooleanFalse;

/**
 * Retrieves the Boolean value of an OCBooleanRef.
 * @param boolean The OCBooleanRef to query.
 * @return true if boolean is kOCBooleanTrue, false otherwise.
 */
bool OCBooleanGetValue(OCBooleanRef boolean);

#endif /* OCBoolean_h */
