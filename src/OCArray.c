#if (defined(__GNUC__) && (defined(_WIN32) || defined(__CYGWIN__))) || \
    (defined(_MSC_VER) && defined(_WIN32))
  // For qsort_s (C11 Annex K)
  #ifndef __STDC_WANT_LIB_EXT1__
    #define __STDC_WANT_LIB_EXT1__ 1
  #endif
#endif

#if defined(__linux__) || (defined(__GNUC__) && (defined(_WIN32) || defined(__CYGWIN__)))
  // For GNU-specific extensions (like GNU qsort_r if available)
  // For MinGW, qsort_s is prioritized due to compiler hints and previous qsort_r issues.
  #ifndef _GNU_SOURCE
    #define _GNU_SOURCE
  #endif
#endif

#include <stdlib.h>  // For qsort, qsort_r, qsort_s, malloc, etc. Must be early for feature macros.
#include <stdint.h>  // For uint64_t, etc.
#include <string.h>  // For memcpy
#include <stdbool.h> // For bool

// Handle Nullability qualifiers for non-Clang compilers (like GCC)
#if !defined(__clang__)
#define _Nullable
#define _Nonnull
#endif

//
//  OCArray.c
//  OCTypes
//
//  Created by philip on 4/2/17.
//

#include "OCLibrary.h" // Should be included after system headers if it also includes them,
                       // or ensure OCLibrary.h doesn't conflict with _GNU_SOURCE etc.
                       // Given OCLibrary.h includes stdlib.h, stdio.h etc., defining _GNU_SOURCE first is correct.

static OCTypeID kOCArrayID = _kOCNotATypeID;

const OCArrayCallBacks kOCTypeArrayCallBacks = {0, OCRetain, OCRelease, OCCopyDescription, OCTypeEqual};
static const OCArrayCallBacks __kOCNullArrayCallBacks = {0, NULL, NULL, NULL, NULL};

// OCArray Opaque Type
struct __OCArray {
    OCBase _base;

    // OCArray Type attributes  - order of declaration is essential
    const OCArrayCallBacks *callBacks;
    uint64_t count; // Changed from u_int64_t
    uint64_t capacity; // Changed from u_int64_t
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

    for(uint64_t index = 0;index< theArray1->count; index++) { // Changed from u_int64_t
        OCTypeRef val1 = theArray1->data[index];
        OCTypeRef val2 = theArray2->data[index];
        if(!OCTypeEqual(val1, val2)) return false;
    }
    
    if(theArray1->callBacks == &kOCTypeArrayCallBacks) {
        for(uint64_t index=0; index<theArray1->count; index++) {
            OCTypeRef val1 = theArray1->data[index];
            OCTypeRef val2 = theArray2->data[index];
            if(!OCTypeEqual(val1, val2)) return false;
        }
    }
    else if(theArray1->callBacks && theArray1->callBacks->equal!=NULL) {
        for(uint64_t index=0; index<theArray1->count; index++) {
            OCTypeRef val1 = theArray1->data[index];
            OCTypeRef val2 = theArray2->data[index];
            if(!theArray1->callBacks->equal(val1, val2)) return false;
        }
    }
    else {
        for(uint64_t index=0; index<theArray1->count; index++) {
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
        for(uint64_t index = 0; index< theArray->count; index++) {
            OCRelease( theArray->data[index]);
        }
    }
    else if(theArray->callBacks && theArray->callBacks->release!=NULL) {
        for(uint64_t index=0; index<theArray->count; index++) {
            theArray->callBacks->release( theArray->data[index]);
        }
    }
}

static void _OCArrayRetainValues(OCArrayRef theArray)
{
    if(theArray->callBacks == &kOCTypeArrayCallBacks) {
        for(uint64_t index = 0; index< theArray->count; index++) {
            OCRetain(theArray->data[index]);
        }
    }
    else if(theArray->callBacks && theArray->callBacks->release!=NULL) {
        for(uint64_t index=0; index<theArray->count; index++) {
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
        for(uint64_t i=index; i<theArray->count; i++) {
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
        for(uint64_t index=0; index<theArray->count; index++) {
            if(OCTypeEqual(theArray->data[index], value)) return true;
        }
    }
    else if(theArray->callBacks) {
        for(uint64_t index=0; index<theArray->count; index++) {
            if(theArray->callBacks->equal(theArray->data[index], value)) return true;
        }
    }
    else {
        for(uint64_t index=0; index<theArray->count; index++) {
            if(theArray->data[index] == value) return true;
        }
    }
    return false;
}

struct qsortContext {
    OCComparatorFunction comparator;
    void * context;
};

// Comparator wrapper for qsort_r (BSD, macOS) and qsort_s (MSVC)
// Signature: int (*compar)(void *thunk, const void *a, const void *b)
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || (defined(_MSC_VER) && defined(_WIN32))
static int thunk_first_qsort_compare_wrapper(void *thunk, const void *val1_ptr, const void *val2_ptr)
{
    struct qsortContext *myContext = (struct qsortContext *) thunk;
    const void *obj1 = *((const void **) val1_ptr);
    const void *obj2 = *((const void **) val2_ptr);
    return myContext->comparator(obj1, obj2, myContext->context);
}
#endif

// Comparator wrapper for qsort_r (GNU/Linux, MinGW, Cygwin GCC)
// Signature: int (*compar)(const void *a, const void *b, void *thunk)
#if defined(__linux__) || (defined(__GNUC__) && (defined(_WIN32) || defined(__CYGWIN__)))
static int thunk_last_qsort_compare_wrapper(const void *val1_ptr, const void *val2_ptr, void *thunk)
{
    struct qsortContext *myContext = (struct qsortContext *) thunk;
    const void *obj1 = *((const void **) val1_ptr);
    const void *obj2 = *((const void **) val2_ptr);
    return myContext->comparator(obj1, obj2, myContext->context);
}
#endif

void OCArraySortValues(OCMutableArrayRef theArray, OCRange range, OCComparatorFunction comparator, void *context)
{
    if (theArray == NULL || theArray->count == 0 || comparator == NULL || range.length == 0) {
        return; 
    }
    // Ensure range is valid
    if (range.location >= theArray->count || (range.location + range.length) > theArray->count) {
        return;
    }

    struct qsortContext myContext = {comparator, context};
    
    void **base_ptr = theArray->data + range.location;
    size_t nmemb = range.length;
    size_t element_size = sizeof(void *);

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // BSD qsort_r: compar(thunk, a, b)
    qsort_r(base_ptr, nmemb, element_size, &myContext, thunk_first_qsort_compare_wrapper);

#elif defined(__linux__) && defined(_GNU_SOURCE)
    // GNU/Linux qsort_r: compar(a, b, thunk)
    // This assumes _GNU_SOURCE successfully exposed qsort_r.
    // If not, this line will fail compilation.
    qsort_r(base_ptr, nmemb, element_size, thunk_last_qsort_compare_wrapper, &myContext);

#elif (defined(__GNUC__) && (defined(_WIN32) || defined(__CYGWIN__))) // MinGW or Cygwin with GCC
    // Attempt to use C11 qsort_s. __STDC_WANT_LIB_EXT1__ should be defined and active from the top of the file.
    // Comparator for C11 qsort_s: int (*compar)(const void *x, const void *y, void *context)
    // Matches: thunk_last_qsort_compare_wrapper
    #if defined(__STDC_LIB_EXT1__) && __STDC_LIB_EXT1__ >= 201112L
        // If qsort_s is not declared despite __STDC_WANT_LIB_EXT1__, this call will fail compilation.
        if (qsort_s(base_ptr, nmemb, element_size, thunk_last_qsort_compare_wrapper, &myContext) != 0) {
            #warning "qsort_s call returned an error on MinGW/Cygwin GCC."
            // Consider error handling here, e.g., setting a flag or logging.
        }
    #else
        // If __STDC_LIB_EXT1__ is not defined to a suitable value, qsort_s is likely unavailable.
        // GNU qsort_r was also problematic (original error).
        #error "MinGW/Cygwin GCC: C11 qsort_s not available (check __STDC_WANT_LIB_EXT1__ definition and library support). Cannot sort with context."
    #endif

#elif defined(_MSC_VER) && defined(_WIN32) // MSVC on Windows
    // Attempt to use MSVC's qsort_s. __STDC_WANT_LIB_EXT1__ should be defined and active.
    // Comparator for MSVC qsort_s: int func(void* context_param, const void* a, const void* b)
    // Matches: thunk_first_qsort_compare_wrapper
    #if defined(__STDC_LIB_EXT1__) && __STDC_LIB_EXT1__ >= 201112L
        // If qsort_s is not declared, this call will fail compilation.
        qsort_s(base_ptr, nmemb, element_size, thunk_first_qsort_compare_wrapper, &myContext);
    #else
        #error "MSVC: qsort_s not available (check __STDC_WANT_LIB_EXT1__ definition and compiler/library version)."
    #endif
#else
    // Generic fallback / error for other platforms
    #warning "Platform not explicitly supported for qsort_r or qsort_s. Contextual sort may fail."
    if (context != NULL) {
        #error "Contextual sort (qsort_r/qsort_s) not implemented for this platform and context is required."
    } else {
        // Potentially use standard qsort if context is NULL and an adapter is made.
        #error "Fallback to standard qsort for NULL context not robustly implemented. Please provide a qsort_r/qsort_s equivalent for your platform or ensure the comparator can work with standard qsort."
    }
#endif
}

#define INVOKE_CALLBACK3(P, A, B, C) (P)(A, B, C)

static OCComparisonResult __OCArrayCompareValues(const void *v1, const void *v2, struct _acompareContext *context) {
    void * _Nullable *val1 = (void * _Nullable *)v1; // Keep _Nullable for source compatibility, macro will remove it for GCC
    const void **val2 = (const void **)v2;
    return (OCComparisonResult)(INVOKE_CALLBACK3(context->func, *val1, *val2, context->context));
}

int64_t OCBSearch(void * _Nullable element, // Keep _Nullable
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



