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

OCTypeID OCBooleanGetTypeID(void);

extern const OCBooleanRef kOCBooleanTrue;
extern const OCBooleanRef kOCBooleanFalse;

bool OCBooleanGetValue(OCBooleanRef boolean);

#endif /* OCBoolean_h */
