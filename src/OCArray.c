//
//  OCArray.c
//  OCTypes
//
//  Created by philip on 4/2/17.
//

#include "OCLibrary.h"

static OCTypeID kOCArrayID = _kOCNotATypeID;

const OCArrayCallBacks kOCTypeArrayCallBacks = {0, OCRetain, OCRelease, OCCopyDescription, OCTypeEqual};
static const OCArrayCallBacks __kOCNullArrayCallBacks = {0, NULL, NULL, NULL, NULL};

// OCArray Opaque Type
struct __OCArray {
    OCBase _base;

    // OCArray Type attributes  - order of declaration is essential
    const OCArrayCallBacks *callBacks;
    uint64_t count;
    uint64_t capacity;
    void **data;
};

static bool __OCArrayEqual(const void *theType1, const void *theType2)
{
    OCArrayRef theArray1 = (OCArrayRef) theType1;
    OCArrayRef theArray2 = (OCArrayRef) theType2;
    if(theArray1->_base.typeID != theArray2->_base.typeID) return false;

    if(NULL == theArray1 || NULL == theArray2) return false;
    if(theArray1 == theArray2) return true;
    if(theArray1->count != theArray2->count) return false;
    if(theArray1->callBacks != theArray2->callBacks) return false;

    for(u_int64_t index = 0;index< theArray1->count; index++) {
        OCTypeRef val1 = theArray1->data[index];
        OCTypeRef val2 = theArray2->data[index];
        if(!OCTypeEqual(val1, val2)) return false;
    }
    
    if(theArray1->callBacks == &kOCTypeArrayCallBacks) {
        for(long index=0;index<theArray1->count;index++) {
            OCTypeRef val1 = theArray1->data[index];
            OCTypeRef val2 = theArray2->data[index];
            if(!OCTypeEqual(val1, val2)) return false;
        }
    }
    else if(theArray1->callBacks && theArray1->callBacks->equal!=NULL) {
        for(long index=0;index<theArray1->count;index++) {
            OCTypeRef val1 = theArray1->data[index];
            OCTypeRef val2 = theArray2->data[index];
            if(!theArray1->callBacks->equal(val1, val2)) return false;
        }
    }
    else {
        for(long index=0;index<theArray1->count;index++) {
            OCTypeRef val1 = theArray1->data[index];
            OCTypeRef val2 = theArray2->data[index];
            if(val1 != val2) return false;
        }
    }

    return true;
}

static void _OCArrayReleaseValues(OCArrayRef theArray)
{
    if(theArray->callBacks == &kOCTypeArrayCallBacks) {
        for(size_t index = 0;index< theArray->count; index++) {
            OCRelease( theArray->data[index]);
        }
    }
    else if(theArray->callBacks && theArray->callBacks->release!=NULL) {
        for(long index=0;index<theArray->count;index++) {
            theArray->callBacks->release( theArray->data[index]);
        }
    }
}

static void _OCArrayRetainValues(OCArrayRef theArray)
{
    if(theArray->callBacks == &kOCTypeArrayCallBacks) {
        for(size_t index = 0;index< theArray->count; index++) {
            OCRetain(theArray->data[index]);
        }
    }
    else if(theArray->callBacks && theArray->callBacks->release!=NULL) {
        for(long index=0;index<theArray->count;index++) {
            theArray->callBacks->retain( theArray->data[index]);
        }
    }
}

uint64_t OCArrayGetCount(OCArrayRef theArray)
{
    if(NULL == theArray) return 0;
    return theArray->count;
}

static void __OCArrayFinalize(const void * theType)
{
    if(NULL == theType) return;
    OCArrayRef theArray = (OCArrayRef) theType;
    _OCArrayReleaseValues(theArray);
    free((void **) theArray->data);
    free((void *)theArray);
}

OCTypeID OCArrayGetTypeID(void)
{
    if(kOCArrayID == _kOCNotATypeID) kOCArrayID = OCRegisterType("OCArray");
    return kOCArrayID;
}

static struct __OCArray *OCArrayAllocate()
{
    struct __OCArray *theArray = malloc(sizeof(struct __OCArray));
    if(NULL == theArray) return NULL;
    theArray->_base.typeID = OCArrayGetTypeID();
    theArray->_base.retainCount = 1;
    theArray->_base.finalize = __OCArrayFinalize;
    theArray->_base.equal = __OCArrayEqual;
    theArray->_base.copyFormattingDesc = NULL;
    theArray->callBacks = &__kOCNullArrayCallBacks;

    return theArray;
}

OCArrayRef OCArrayCreate(const void **values, uint64_t numValues, const OCArrayCallBacks *callBacks)
{
    if(NULL==values) return NULL;
    struct __OCArray *newArray = OCArrayAllocate();
    if(NULL == newArray) return NULL;
    newArray->data = (void **) malloc(numValues * sizeof(void *));
    memcpy((void *) newArray->data, (const void *) values, numValues * sizeof(void *));
    newArray->count = numValues;
    newArray->capacity = numValues;
    newArray->callBacks = callBacks;
    _OCArrayRetainValues(newArray);

    return newArray;
}

OCArrayRef OCArrayCreateCopy(OCArrayRef theArray)
{
    return (OCArrayRef) OCArrayCreate((const void **) theArray->data,theArray->count, theArray->callBacks);
}

OCMutableArrayRef OCArrayCreateMutable(uint64_t capacity, const OCArrayCallBacks *callBacks)
{
    struct __OCArray *newArray = OCArrayAllocate();
    if(NULL == newArray) return NULL;
    newArray->data = (void **) malloc(capacity * sizeof(void *));
    newArray->count = 0;
    newArray->capacity = capacity;
    newArray->callBacks = callBacks;
    return newArray;
}

OCMutableArrayRef OCArrayCreateMutableCopy(OCArrayRef theArray)
{
    return (OCMutableArrayRef) OCArrayCreate((const void **) theArray->data,theArray->count,theArray->callBacks);
}

const void * OCArrayGetValueAtIndex(OCArrayRef theArray, uint64_t index)
{
    if(NULL==theArray) return NULL;
    if(index>=theArray->count) return NULL;
    return theArray->data[index];
}

void OCArrayRemoveValueAtIndex(OCMutableArrayRef theArray,uint64_t index)
{
    if(index<theArray->count) {
        OCRelease(theArray->data[index]);
        theArray->count--;
        for(uint64_t i=index;i<theArray->count;i++) {
            theArray->data[i] = theArray->data[i+1];
        }
    }
}

long OCArrayGetFirstIndexOfValue(OCArrayRef theArray, const void * value)
{
    if(NULL==theArray) return -1;
    if(NULL==value) return -1;
    for(long index=0;index<theArray->count;index++) {
        if(theArray->data[index] == value) return index;
    }
    return -1;
}

void OCArrayAppendValue(OCMutableArrayRef theArray, const void * value)
{
    if(NULL==theArray) return;
    if(NULL==value) return;
    OCTypeRef type = (OCTypeRef) value;
    if(theArray->capacity==0 || theArray->count==theArray->capacity) {
        if(NULL==theArray->data) theArray->data = (void **) malloc(sizeof(void *));
        else theArray->data = (void **) realloc(theArray->data, (theArray->count+1) * sizeof(void *));
        theArray->data[theArray->count] = type;
        if(theArray->callBacks == &kOCTypeArrayCallBacks) {
            OCRetain(type);
        }
        else if(theArray->callBacks && theArray->callBacks->release!=NULL) {
            theArray->callBacks->retain(type);
        }
        theArray->count++;
        if(theArray->capacity!=0) theArray->capacity++;
    }
    else {
        theArray->data[theArray->count] = type;
        if(theArray->callBacks == &kOCTypeArrayCallBacks) {
            OCRetain(type);
        }
        else if(theArray->callBacks && theArray->callBacks->release!=NULL) {
                theArray->callBacks->retain(type);
        }
        theArray->count++;
    }
}

void OCArrayAppendArray(OCMutableArrayRef theArray, OCArrayRef otherArray, OCRange range)
{
    if(NULL==theArray) return;
    if(NULL==otherArray) return;
    uint64_t count = OCArrayGetCount(otherArray);
    if(range.location+range.length > count) return;
    
    for(uint64_t index=range.location;index<range.location+range.length;index++) {
        OCTypeRef theType = (OCTypeRef) OCArrayGetValueAtIndex(otherArray, index);
        OCArrayAppendValue(theArray, theType);
    }
}

void OCArrayInsertValueAtIndex(OCMutableArrayRef theArray, uint64_t index, const void * value)
{
    if(index>theArray->count) return;
    OCTypeRef type = (OCTypeRef) value;

    if(index==theArray->count) {
        OCArrayAppendValue(theArray, type);
        return;
    }
    if(theArray->capacity==0 || theArray->count==theArray->capacity) {
        if(NULL==theArray->data) theArray->data = (void **) malloc(sizeof(void *));
        else theArray->data = (void **) realloc(theArray->data, (theArray->count+1) * sizeof(void *));
        
        for(uint64_t i = theArray->count; i>index; i--) {
            theArray->data[index] = theArray->data[index-1];
        }
        theArray->data[index] = type;
        if(theArray->callBacks == &kOCTypeArrayCallBacks) {
            OCRetain(type);
        }
        else if(theArray->callBacks && theArray->callBacks->release!=NULL) {
            theArray->callBacks->retain(type);
        }

        theArray->count++;
        if(theArray->capacity!=0) theArray->capacity++;
    }
}

struct _acompareContext {
    OCComparatorFunction func;
    void *context;
};

#define INVOKE_CALLBACK3(P, A, B, C) (P)(A, B, C)

bool OCArrayContainsValue(OCArrayRef theArray, const void * value)
{
    if(NULL==theArray) return false;
    if(NULL==value) return false;

    if(theArray->callBacks == &kOCTypeArrayCallBacks) {
        for(long index=0;index<theArray->count;index++) {
            if(OCTypeEqual(theArray->data[index], value)) return true;
        }
    }
    else if(theArray->callBacks) {
        for(long index=0;index<theArray->count;index++) {
            if(theArray->callBacks->equal(theArray->data[index], value)) return true;
        }
    }
    else {
        for(long index=0;index<theArray->count;index++) {
            if(theArray->data[index] == value) return true;
        }
    }
    return false;
}

struct qsortContext {
    OCComparatorFunction comparator;
    void * context;
};

// Careful: Apple C library had different definition for qsort_r than linux.
// In Apple version context is the first argument instead of the last.
OCComparisonResult qsortCompare(void *context, const void *val1, const void *val2)
{
    struct qsortContext *myContext = (struct qsortContext *) context;
    return myContext->comparator(*((const void **) val1),*((const void **) val2),myContext->context);
}

void OCArraySortValues(OCMutableArrayRef theArray, OCRange range, OCComparatorFunction comparator, void *context)
{
    struct qsortContext myContext = {comparator,context};
    qsort_r(theArray->data, (size_t) theArray->count, (size_t) sizeof(const void *), &myContext, qsortCompare);
}

#define INVOKE_CALLBACK3(P, A, B, C) (P)(A, B, C)

static OCComparisonResult __OCArrayCompareValues(const void *v1, const void *v2, struct _acompareContext *context) {
    void *_Nullable **val1 = (void *_Nullable **)v1;
    const void **val2 = (const void **)v2;
    return (OCComparisonResult)(INVOKE_CALLBACK3(context->func, *val1, *val2, context->context));
}

int64_t OCBSearch(void *_Nullable element,
                  int64_t elementSize,
                  const void *list,
                  int64_t count,
                  OCComparatorFunction comparator,
                  void *context)
{
    const char *ptr = (const char *)list;
    while (0 < count) {
        int64_t half = count / 2;
        const char *probe = ptr + elementSize * half;
        OCComparisonResult cr = comparator(element, probe, context);
        if (0 == cr) return (probe - (const char *)list) / elementSize;
        ptr = (cr < 0) ? ptr : probe + elementSize;
        count = (cr < 0) ? half : (half + (count & 1) - 1);
    }
    return (ptr - (const char *)list) / elementSize;
}

int64_t OCArrayBSearchValues(OCArrayRef array, OCRange range, const void *value, OCComparatorFunction comparator, void *context)
{
    if(NULL==array) return 0;
    if(NULL==comparator) return 0;
    
    int64_t idx = 0;
    if (range.length <= 0) return range.location;
    
    struct _acompareContext ctx;
    ctx.func = comparator;
    ctx.context = context;
    idx = OCBSearch(&value,
                    sizeof(void *),
                    array->data + range.location, range.length,
                    (OCComparatorFunction)__OCArrayCompareValues,
                    &ctx);
    
    return idx + range.location;
}



