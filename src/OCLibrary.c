#include "OCLibrary.h"
#include "OCBoolean.h"
#include "OCArray.h"
#include "OCAutoreleasePool.h"
#include "OCNumber.h"
#include "OCDictionary.h"
#include "OCString.h"
#include "OCData.h"

void _OCArrayInitialize(void);
void _OCAutoreleasePoolInitialize(void);
void _OCBooleanInitialize(void);
void _OCDataInitialize(void);
void _OCDictionaryInitialize(void);
void _OCNumberInitialize(void);
void _OCStringInitialize(void);

void OCLibraryInitialize(void) {
    _OCBooleanInitialize();
    _OCArrayInitialize();
    _OCAutoreleasePoolInitialize();
    _OCNumberInitialize();
    _OCDictionaryInitialize();
    _OCStringInitialize();
    _OCDataInitialize();
}
