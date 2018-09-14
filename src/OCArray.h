//
//  OCArray.h
//  OCTypes
//
//  Created by philip on 4/2/17.
//

#ifndef OCArray_h
#define OCArray_h

#include "OCLibrary.h"

typedef const struct __OCArray * OCArrayRef;
typedef struct __OCArray * OCMutableArrayRef;

typedef const void * (*OCArrayRetainCallBack)(const void *value);
typedef void		(*OCArrayReleaseCallBack)(const void *value);
typedef OCStringRef	(*OCArrayCopyDescriptionCallBack)(const void *value);
typedef bool		(*OCArrayEqualCallBack)(const void *value1, const void *value2);
typedef struct {
    int64_t     version;
    OCArrayRetainCallBack   retain;
    OCArrayReleaseCallBack  release;
    OCArrayCopyDescriptionCallBack  copyDescription;
    OCArrayEqualCallBack    equal;
} OCArrayCallBacks;

extern const OCArrayCallBacks kOCTypeArrayCallBacks;



OCTypeID OCArrayGetTypeID(void);

uint64_t OCArrayGetCount(OCArrayRef theArray);
OCArrayRef OCArrayCreate(const void **values, uint64_t numValues, const OCArrayCallBacks *callBacks);
OCArrayRef OCArrayCreateCopy(OCArrayRef theArray);
OCMutableArrayRef OCArrayCreateMutable(uint64_t capacity, const OCArrayCallBacks *callBacks);
OCMutableArrayRef OCArrayCreateMutableCopy(OCArrayRef theArray);
const void * OCArrayGetValueAtIndex(OCArrayRef theArray, uint64_t index);
void OCArrayAppendValue(OCMutableArrayRef theArray, const void * value);
void OCArrayAppendArray(OCMutableArrayRef theArray, OCArrayRef otherArray, OCRange range);
long OCArrayGetFirstIndexOfValue(OCArrayRef theArray, const void * value);
bool OCArrayContainsValue(OCArrayRef theArray, const void * value);
void OCArrayRemoveValueAtIndex(OCMutableArrayRef theArray,uint64_t index);
void OCArrayInsertValueAtIndex(OCMutableArrayRef theArray, uint64_t index, const void * value);
void OCArraySortValues(OCMutableArrayRef theArray, OCRange range, OCComparatorFunction comparator, void *context);
int64_t OCArrayBSearchValues(OCArrayRef array, OCRange range, const void *value, OCComparatorFunction comparator, void *context);


#endif /* OCArray_h */
