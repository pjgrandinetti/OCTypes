#if (defined(__GNUC__) && (defined(_WIN32) || defined(__CYGWIN__))) || \
    (defined(_MSC_VER) && defined(_WIN32))
// For qsort_s (C11 Annex K)
#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif
#endif
#if defined(__linux__) || (defined(__GNUC__) && (defined(_WIN32) || defined(__CYGWIN__)))
// For GNU-specific extensions (like GNU qsort_r if available)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif
#include <stdbool.h>  // For bool
#include <stdint.h>   // For uint64_t, etc.
#include <stdio.h>
#include <stdlib.h>  // For qsort, qsort_r, qsort_s, malloc, etc. Must be early for feature macros.
#include <string.h>  // For memcpy
#if defined(__APPLE__)
#include <malloc/malloc.h>  // For malloc_zone_t
#else
// Handle Nullability qualifiers for non-Clang compilers (like GCC)
#if !defined(__clang__)
#define _Nullable
#define _Nonnull
#endif
#endif  // end of #if defined(__APPLE__)
//
//  OCArray.c
//  OCTypes
//
//  Created by philip on 4/2/17.
//
#include "OCLibrary.h"  // Should be included after system headers if it also includes them,
                        // or ensure OCLibrary.h doesn't conflict with _GNU_SOURCE etc.
                        // Given OCLibrary.h includes stdlib.h, stdio.h etc., defining _GNU_SOURCE first is correct.
static OCTypeID kOCArrayID = kOCNotATypeID;
const OCArrayCallBacks kOCTypeArrayCallBacks = {0, OCRetain, OCRelease, OCTypeCopyFormattingDesc, OCTypeEqual};
static const OCArrayCallBacks __kOCNullArrayCallBacks = {0, NULL, NULL, NULL, NULL};
// OCArray Opaque Type
struct impl_OCArray {
    OCBase base;
    // OCArray Type attributes  - order of declaration is essential
    const OCArrayCallBacks *callBacks;
    uint64_t count;     // Changed from u_int64_t
    uint64_t capacity;  // Changed from u_int64_t
    const void **data;
};
static bool impl_OCArrayEqual(const void *theType1, const void *theType2) {
    OCArrayRef a1 = (OCArrayRef)theType1;
    OCArrayRef a2 = (OCArrayRef)theType2;
    if (!a1 || !a2) return false;
    if (a1 == a2) return true;
    if (a1->base.typeID != a2->base.typeID) return false;
    if (a1->count != a2->count) return false;
    if (a1->callBacks != a2->callBacks) return false;
    OCArrayEqualCallBack equalFn = NULL;
    if (a1->callBacks == &kOCTypeArrayCallBacks) {
        equalFn = OCTypeEqual;
    } else if (a1->callBacks && a1->callBacks->equal) {
        equalFn = a1->callBacks->equal;
    }
    for (uint64_t i = 0; i < a1->count; ++i) {
        const void *v1 = a1->data[i];
        const void *v2 = a2->data[i];
        if (equalFn) {
            if (!equalFn(v1, v2)) return false;
        } else {
            if (v1 != v2) return false;
        }
    }
    return true;
}
static void _OCArrayReleaseValues(OCArrayRef theArray) {
    if (theArray && theArray->callBacks && theArray->callBacks->release) {
        for (uint64_t index = 0; index < theArray->count; index++) {
            if (theArray->data[index]) {  // Add a NULL check for safety before dereferencing
                theArray->callBacks->release(theArray->data[index]);
            }
        }
    }
}
static void impl_OCArrayRetainValues(OCArrayRef theArray) {
    if (theArray && theArray->callBacks && theArray->callBacks->retain) {
        for (uint64_t index = 0; index < theArray->count; index++) {
            if (theArray->data[index]) {  // Add a NULL check for safety
                theArray->callBacks->retain(theArray->data[index]);
            }
        }
    }
}
static void *impl_OCArrayDeepCopy(const void *obj) {
    const OCArrayRef src = (const OCArrayRef)obj;
    if (!src) return NULL;
    OCMutableArrayRef copy = OCArrayCreateMutable(src->count, src->callBacks);
    if (!copy) return NULL;
    for (uint64_t i = 0; i < src->count; ++i) {
        const void *value = src->data[i];
        const void *copied = NULL;
        if (src->callBacks == &kOCTypeArrayCallBacks) {
            copied = OCTypeDeepCopy(value);
        } else if (src->callBacks && src->callBacks->retain) {
            copied = src->callBacks->retain(value);  // fallback
        } else {
            copied = value;
        }
        OCArrayAppendValue(copy, copied);
        // Release manually if we retained and append retains again
        if (src->callBacks == &kOCTypeArrayCallBacks || src->callBacks->retain) {
            OCRelease(copied);
        }
    }
    return copy;
}
static void *impl_OCArrayDeepCopyMutable(const void *obj) {
    return impl_OCArrayDeepCopy(obj);  // already returns a mutable copy
}
uint64_t OCArrayGetCount(OCArrayRef theArray) {
    if (NULL == theArray) return 0;
    return theArray->count;
}
const OCArrayCallBacks *OCArrayGetCallBacks(OCArrayRef array) {
    if (!array) return NULL;
    return array->callBacks;
}
OCStringRef OCArrayCopyFormattingDesc(OCTypeRef cf) {
    if (!cf) return OCStringCreateWithCString("<OCArray: NULL>");
    OCArrayRef array = (OCArrayRef)cf;
    const OCArrayCallBacks *cb = OCArrayGetCallBacks(array);
    const size_t maxItems = 5;
    size_t count = OCArrayGetCount(array);
    size_t shown = count < maxItems ? count : maxItems;
    OCMutableStringRef result = OCStringCreateMutable(0);
    OCStringAppendFormat(result, STR("<OCArray: %zu item%s ["),
                         count, count == 1 ? "" : "s");
    for (size_t i = 0; i < shown; ++i) {
        if (i > 0)
            OCStringAppendCString(result, ", ");
        const void *element = OCArrayGetValueAtIndex(array, i);
        OCStringRef desc = NULL;
        // Use copyFormattingDesc if available
        if (cb == &kOCTypeArrayCallBacks) {
            desc = OCTypeCopyFormattingDesc((OCTypeRef)element);
        }
        if (desc) {
            OCStringAppend(result, desc);
            OCRelease(desc);
        } else if (element) {
            OCStringAppendFormat(result, STR("%p"), element);
        } else {
            OCStringAppendCString(result, "(null)");
        }
    }
    if (count > maxItems)
        OCStringAppendCString(result, ", …");
    OCStringAppendCString(result, "]>");
    return result;
}
static cJSON *
impl_OCArrayCopyJSON(const void *obj) {
    return OCArrayCreateJSON((OCArrayRef)obj);
}
static void impl_OCArrayFinalize(const void *theType) {
    if (NULL == theType) return;
    struct impl_OCArray *theArray = (struct impl_OCArray *)theType;
    _OCArrayReleaseValues(theArray);
    // Only free non-NULL data
    if (theArray->data) {
#if defined(__APPLE__)
        malloc_zone_t *zone = malloc_zone_from_ptr(theArray->data);
        if (zone) {
            free((void *)theArray->data);
        } else {
            fprintf(stderr, "OCArrayFinalize: invalid free of %p\n", theArray->data);
        }
#else
        free((void *)theArray->data);
#endif
        theArray->data = NULL;
    }
}
OCTypeID OCArrayGetTypeID(void) {
    if (kOCArrayID == kOCNotATypeID) kOCArrayID = OCRegisterType("OCArray");
    return kOCArrayID;
}
static struct impl_OCArray *OCArrayAllocate() {
    struct impl_OCArray *obj = OCTypeAlloc(
        struct impl_OCArray,
        OCArrayGetTypeID(),
        impl_OCArrayFinalize,
        impl_OCArrayEqual,
        OCArrayCopyFormattingDesc,
        impl_OCArrayCopyJSON,
        impl_OCArrayDeepCopy,
        impl_OCArrayDeepCopyMutable  // ← NEW!
    );
    obj->callBacks = &__kOCNullArrayCallBacks;
    obj->count = 0;
    obj->capacity = 0;
    obj->data = NULL;
    return obj;
}
OCArrayRef OCArrayCreate(const void **values, uint64_t numValues, const OCArrayCallBacks *callBacks) {
    // Allow creation of an empty array even if values is NULL, as long as numValues is 0.
    if (numValues > 0 && NULL == values) return NULL;
    struct impl_OCArray *newArray = OCArrayAllocate();
    if (NULL == newArray) return NULL;
    if (numValues > 0) {
        newArray->data = (const void **)calloc(numValues, sizeof(const void *));
        if (NULL == newArray->data) {
            fprintf(stderr, "OCArrayCreate: Memory allocation for data failed.\n");
            OCRelease(newArray);  // Clean up allocated array shell
            return NULL;
        }
        memcpy((void *)newArray->data, (const void *)values, numValues * sizeof(const void *));
    } else {
        newArray->data = NULL;  // Explicitly NULL for 0 elements, though OCArrayAllocate might do this.
    }
    newArray->count = numValues;
    newArray->capacity = numValues;                                          // For immutable arrays, capacity equals count.
    newArray->callBacks = callBacks ? callBacks : &__kOCNullArrayCallBacks;  // Ensure callbacks is not NULL
    impl_OCArrayRetainValues(newArray);
    return newArray;
}
OCArrayRef OCArrayCreateCopy(OCArrayRef theArray) {
    if (NULL == theArray) return NULL;  // Handle NULL input
    // For immutable copies, capacity can be same as count
    return (OCArrayRef)OCArrayCreate((const void **)theArray->data, theArray->count, theArray->callBacks);
}
OCMutableArrayRef OCArrayCreateMutable(uint64_t capacity, const OCArrayCallBacks *callBacks) {
    struct impl_OCArray *newArray = OCArrayAllocate();
    if (NULL == newArray) return NULL;
    if (capacity > 0) {
        newArray->data = (const void **)calloc(capacity, sizeof(const void *));
        if (NULL == newArray->data) {
            fprintf(stderr, "OCArrayCreateMutable: Memory allocation for data failed.\n");
            OCRelease(newArray);
            return NULL;
        }
    } else {
        newArray->data = NULL;  // Can start with NULL data if capacity is 0
    }
    newArray->count = 0;
    newArray->capacity = capacity;
    newArray->callBacks = callBacks ? callBacks : &__kOCNullArrayCallBacks;
    return newArray;
}
OCMutableArrayRef OCArrayCreateMutableCopy(OCArrayRef theArray) {
    if (NULL == theArray) return NULL;
    // Create a mutable copy with initial capacity at least the count of the original array.
    OCMutableArrayRef newMutableArray = OCArrayCreateMutable(theArray->count > 0 ? theArray->count : 1,
                                                             theArray->callBacks);
    if (NULL == newMutableArray) return NULL;
    if (theArray->count > 0) {
        memcpy(newMutableArray->data, theArray->data, theArray->count * sizeof(const void *));
        newMutableArray->count = theArray->count;
        impl_OCArrayRetainValues(newMutableArray);  // Retain values copied into the new mutable array
    }
    return newMutableArray;
}
cJSON *OCArrayCreateJSON(OCArrayRef array) {
    if (!array) return cJSON_CreateNull();
    cJSON *arr = cJSON_CreateArray();
    uint64_t count = OCArrayGetCount(array);
    for (uint64_t i = 0; i < count; i++) {
        OCTypeRef elem = OCArrayGetValueAtIndex(array, i);
        cJSON *jsonVal = OCTypeCopyJSON(elem);
        if (!jsonVal) {
            fprintf(stderr, "OCArrayCreateJSON: Failed to serialize array element at index %llu. Using null.\n", (unsigned long long)i);
            jsonVal = cJSON_CreateNull();
        }
        cJSON_AddItemToArray(arr, jsonVal);
    }
    return arr;
}
const void *OCArrayGetValueAtIndex(OCArrayRef theArray, uint64_t index) {
    if (NULL == theArray) return NULL;
    if (index >= theArray->count) return NULL;
    return theArray->data[index];
}
bool OCArraySetValueAtIndex(OCMutableArrayRef theArray, OCIndex index, const void *value) {
    if (!theArray || index < 0 || index >= OCArrayGetCount(theArray)) {
        return false;
    }
    const void *oldValue = OCArrayGetValueAtIndex(theArray, index);
    if (oldValue == value) {
        // No change needed
        return true;
    }
    const OCArrayCallBacks *cb = OCArrayGetCallBacks(theArray);
    // Release existing value if necessary
    if (cb && cb->release && oldValue) {
        cb->release(oldValue);
    }
    // Retain new value if necessary
    const void *retained = value;
    if (cb && cb->retain && value) {
        retained = cb->retain(value);
    }
    theArray->data[index] = retained;
    return true;
}
bool OCArrayRemoveValueAtIndex(OCMutableArrayRef theArray, uint64_t index) {
    if (theArray == NULL || index >= theArray->count) {
        return false;
    }
    const void *value = theArray->data[index];
    const OCArrayCallBacks *cb = theArray->callBacks;
    if (cb && cb->release && value) {
        cb->release(value);
    } else if (cb == &kOCTypeArrayCallBacks && value) {
        OCRelease(value);
    }
    // Shift elements down
    for (uint64_t i = index; i < theArray->count - 1; i++) {
        theArray->data[i] = theArray->data[i + 1];
    }
    theArray->count--;
    return true;
}
long OCArrayGetFirstIndexOfValue(OCArrayRef theArray, const void *value) {
    if (NULL == theArray || NULL == value) return kOCNotFound;  // Use kOCNotFound
    OCArrayEqualCallBack equalCB = NULL;
    if (theArray->callBacks) {
        equalCB = theArray->callBacks->equal;
    }
    if (theArray->callBacks == &kOCTypeArrayCallBacks) {  // Ensure OCTypeEqual is used for kOCTypeArrayCallBacks
        equalCB = OCTypeEqual;
    }
    for (uint64_t index = 0; index < theArray->count; index++) {  // Changed to uint64_t for index
        if (equalCB) {
            if (equalCB(theArray->data[index], value)) return (long)index;
        } else {  // Fallback to direct pointer comparison if no equal callback
            if (theArray->data[index] == value) return (long)index;
        }
    }
    return kOCNotFound;  // Use kOCNotFound
}
bool OCArrayAppendValue(OCMutableArrayRef theArray, const void *value) {
    if (theArray == NULL || value == NULL) {
        return false;
    }
    OCTypeRef type = (OCTypeRef)value;  // Assuming value is an OCTypeRef
    // Reallocate if needed
    if (theArray->count == theArray->capacity) {
        uint64_t newCapacity = (theArray->capacity == 0) ? 1 : theArray->capacity * 2;
        const void **newData = (const void **)realloc(theArray->data, newCapacity * sizeof(const void *));
        if (newData == NULL) {
            fprintf(stderr, "OCArrayAppendValue: Memory reallocation failed.\n");
            return false;
        }
        theArray->data = newData;
        theArray->capacity = newCapacity;
    }
    theArray->data[theArray->count] = value;
    // Retain if needed
    if (theArray->callBacks && theArray->callBacks->retain) {
        theArray->callBacks->retain(type);
    } else if (theArray->callBacks == &kOCTypeArrayCallBacks) {
        OCRetain(type);
    }
    theArray->count++;
    return true;
}
bool OCArrayAppendArray(OCMutableArrayRef theArray, OCArrayRef otherArray, OCRange range) {
    if (theArray == NULL || otherArray == NULL) {
        return false;
    }
    uint64_t count = OCArrayGetCount(otherArray);
    if (range.location + range.length > count) {
        return false;
    }
    for (uint64_t index = range.location; index < range.location + range.length; index++) {
        OCTypeRef theType = (OCTypeRef)OCArrayGetValueAtIndex(otherArray, index);
        OCArrayAppendValue(theArray, theType);  // Assume this always succeeds
    }
    return true;
}
bool OCArrayInsertValueAtIndex(OCMutableArrayRef theArray, uint64_t index, const void *value) {
    if (theArray == NULL || value == NULL || index > theArray->count) {
        return false;
    }
    OCTypeRef type = (OCTypeRef)value;
    if (theArray->count == theArray->capacity) {
        uint64_t newCapacity = (theArray->capacity == 0) ? 1 : theArray->capacity * 2;
        const void **newData = (const void **)realloc(theArray->data, newCapacity * sizeof(const void *));
        if (newData == NULL) {
            fprintf(stderr, "OCArrayInsertValueAtIndex: Memory reallocation failed.\n");
            return false;
        }
        theArray->data = newData;
        theArray->capacity = newCapacity;
    }
    for (uint64_t i = theArray->count; i > index; i--) {
        theArray->data[i] = theArray->data[i - 1];
    }
    theArray->data[index] = value;
    if (theArray->callBacks && theArray->callBacks->retain) {
        theArray->callBacks->retain(type);
    } else if (theArray->callBacks == &kOCTypeArrayCallBacks) {
        OCRetain(type);
    }
    theArray->count++;
    return true;
}
struct _acompareContext {
    OCComparatorFunction func;
    void *context;
};
#define INVOKE_CALLBACK3(P, A, B, C) (P)(A, B, C)
bool OCArrayContainsValue(OCArrayRef theArray, const void *value) {
    if (NULL == theArray) return false;
    if (NULL == value) return false;
    if (theArray->callBacks == &kOCTypeArrayCallBacks) {
        for (uint64_t index = 0; index < theArray->count; index++) {
            if (OCTypeEqual(theArray->data[index], value)) return true;
        }
    } else if (theArray->callBacks) {
        for (uint64_t index = 0; index < theArray->count; index++) {
            if (theArray->callBacks->equal(theArray->data[index], value)) return true;
        }
    } else {
        for (uint64_t index = 0; index < theArray->count; index++) {
            if (theArray->data[index] == value) return true;
        }
    }
    return false;
}
struct qsortContext {
    OCComparatorFunction comparator;
    void *context;
};
// Comparator wrapper for qsort_r (BSD, macOS) and qsort_s (MSVC)
// Signature: int (*compar)(void *thunk, const void *a, const void *b)
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || (defined(_MSC_VER) && defined(_WIN32))
static int thunk_first_qsort_compare_wrapper(void *thunk, const void *val1_ptr, const void *val2_ptr) {
    struct qsortContext *myContext = (struct qsortContext *)thunk;
    const void *obj1 = *((const void **)val1_ptr);
    const void *obj2 = *((const void **)val2_ptr);
    return myContext->comparator(obj1, obj2, myContext->context);
}
#endif
// Comparator wrapper for qsort_r (GNU/Linux, MinGW, Cygwin GCC)
// Signature: int (*compar)(const void *a, const void *b, void *thunk)
#if defined(__linux__) || (defined(__GNUC__) && (defined(_WIN32) || defined(__CYGWIN__)))
static int thunk_last_qsort_compare_wrapper(const void *val1_ptr, const void *val2_ptr, void *thunk) {
    struct qsortContext *myContext = (struct qsortContext *)thunk;
    const void *obj1 = *((const void **)val1_ptr);
    const void *obj2 = *((const void **)val2_ptr);
    return myContext->comparator(obj1, obj2, myContext->context);
}
#endif
void OCArraySortValues(OCMutableArrayRef theArray, OCRange range, OCComparatorFunction comparator, void *context) {
    if (theArray == NULL || theArray->count == 0 || comparator == NULL || range.length == 0) {
        return;
    }
    // Ensure range is valid
    if (range.location >= theArray->count || (range.location + range.length) > theArray->count) {
        return;
    }
    struct qsortContext myContext = {comparator, context};
    void **base_ptr = (void **)(theArray->data + range.location);
    size_t nmemb = range.length;
    size_t element_size = sizeof(void *);
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // BSD qsort_r: compar(thunk, a, b)
    qsort_r(base_ptr, nmemb, element_size, &myContext, thunk_first_qsort_compare_wrapper);
#elif defined(__linux__) && defined(_GNU_SOURCE)
    // GNU/Linux qsort_r: compar(a, b, thunk)
    qsort_r(base_ptr, nmemb, element_size, thunk_last_qsort_compare_wrapper, &myContext);
#elif (defined(__GNUC__) && (defined(_WIN32) || defined(__CYGWIN__)))  // MinGW or Cygwin with GCC
    {
        void **arr = base_ptr;
        for (size_t i = 1; i < nmemb; i++) {
            void *tmp = arr[i];
            size_t j = i;
            while (j > 0) {
                void *prev = arr[j - 1];
                if (myContext.comparator(prev, tmp, myContext.context) > 0) {
                    arr[j] = arr[j - 1];
                    j--;
                } else {
                    break;
                }
            }
            arr[j] = tmp;
        }
    }
#elif defined(_MSC_VER) && defined(_WIN32)
#if defined(__STDC_LIB_EXT1__) && __STDC_LIB_EXT1__ >= 201112L
    qsort_s(base_ptr, nmemb, element_size, thunk_first_qsort_compare_wrapper, &myContext);
#else
#error "MSVC: qsort_s not available (check __STDC_WANT_LIB_EXT1__ definition and compiler/library version)."
#endif
#else
#warning "Platform not explicitly supported for qsort_r or qsort_s. Contextual sort may fail."
    if (context != NULL) {
#error "Contextual sort (qsort_r/qsort_s) not implemented for this platform and context is required."
    } else {
#error "Fallback to standard qsort for NULL context not robustly implemented. Please provide a qsort_r/qsort_s equivalent for your platform or ensure the comparator can work with standard qsort."
    }
#endif
}
#define INVOKE_CALLBACK3(P, A, B, C) (P)(A, B, C)
int64_t OCBSearch(void *_Nullable element,
                  int64_t elementSize,
                  const void *list,
                  int64_t count,
                  OCComparatorFunction comparator,
                  void *context) {
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
int64_t OCArrayBSearchValues(OCArrayRef array, OCRange range, const void *value, OCComparatorFunction comparator, void *context) {
    if (NULL == array || NULL == comparator || range.length == 0) return kOCNotFound;        // Return kOCNotFound for invalid inputs or empty range
    if (range.location >= array->count || (range.location + range.length) > array->count) {  // Validate range
        return kOCNotFound;
    }
    const void **base = (const void **)(array->data + range.location);
    size_t num = range.length;
    int64_t low = 0;
    int64_t high = num - 1;
    int64_t result_idx = kOCNotFound;
    while (low <= high) {
        int64_t mid = low + (high - low) / 2;
        const void *mid_val = base[mid];
        OCComparisonResult cmp = comparator(value, mid_val, context);
        if (cmp == kOCCompareEqualTo) {
            result_idx = range.location + mid;  // Found, return original array index
            break;
        } else if (cmp == kOCCompareLessThan) {
            high = mid - 1;
        } else {  // kOCCompareGreaterThan
            low = mid + 1;
        }
    }
    return result_idx;
}
