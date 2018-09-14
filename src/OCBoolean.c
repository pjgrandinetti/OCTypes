//
//  OCBoolean.c
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//

#include "OCLibrary.h"

const OCBooleanRef kOCBooleanTrue;
const OCBooleanRef kOCBooleanFalse;

// OCBoolean Opaque Type
struct __OCBoolean {
    OCBase _base;
};

static bool __OCBooleanEqual(const void * theType1, const void * theType2)
{
    OCBooleanRef theBoolean1 = (OCBooleanRef) theType1;
    OCBooleanRef theBoolean2 = (OCBooleanRef) theType2;
    if(theBoolean1->_base.typeID != theBoolean2->_base.typeID) return false;
    
    if(NULL == theBoolean1 || NULL == theBoolean2) return false;
    if(theBoolean1 != theBoolean2) return false;
    return true;
}

static void __OCBooleanFinalize(const void * theType){return;}

OCTypeID OCBooleanGetTypeID(void)
{
    return _kOCNotATypeID;
}

static struct __OCBoolean __kOCBooleanTrue = {
    _kOCNotATypeID,0,__OCBooleanFinalize,__OCBooleanEqual
};

static struct __OCBoolean __kOCBooleanFalse = {
    _kOCNotATypeID,0,__OCBooleanFinalize,__OCBooleanEqual
};

const OCBooleanRef kOCBooleanTrue = &__kOCBooleanTrue;
const OCBooleanRef kOCBooleanFalse = &__kOCBooleanFalse;

bool OCBooleanGetValue(OCBooleanRef boolean)
{
    return (boolean == kOCBooleanTrue) ? true : false;
}


