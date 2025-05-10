//
//  main.c
//  tests
//
//  Created by Philip Grandinetti on 6/17/17.
//  Copyright © 2017 PhySy Ltd. All rights reserved.
//

#define PRINTERROR do { \
    fprintf(stderr, "failure: line %d, %s\n", __LINE__, __FUNCTION__); \
    return false; \
} while (0)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../src/OCLibrary.h"

bool stringTest0(void);
bool stringTest1(void);
bool stringTest2(void);
bool arrayTest0(void);

int main(int argc, const char * argv[])
{
    int failures = 0;
    if (!stringTest0()) failures++;
    if (!stringTest1()) failures++;
    if (!stringTest2()) failures++;
    if (!arrayTest0()) failures++;
    if (failures > 0) {
        fprintf(stderr, "%d test(s) failed\n", failures);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "All tests passed\n");
    return EXIT_SUCCESS;
}

bool arrayTest0(void)
{
    fprintf(stderr,"%s begin...\n",__func__);
    OCMutableArrayRef array = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    
    OCArrayAppendValue(array, STR("e"));
    OCArrayAppendValue(array, STR("b"));
    OCArrayAppendValue(array, STR("a"));
    OCArrayAppendValue(array, STR("c"));
    OCArrayAppendValue(array, STR("d"));
    OCStringRef bit = (OCStringRef) OCArrayGetValueAtIndex(array, 0);
    if(!OCStringEqual(bit, STR("e"))) PRINTERROR;
    bit = (OCStringRef) OCArrayGetValueAtIndex(array, 1);
    if(!OCStringEqual(bit, STR("b"))) PRINTERROR;
    bit = (OCStringRef) OCArrayGetValueAtIndex(array, 2);
    if(!OCStringEqual(bit, STR("a"))) PRINTERROR;
    bit = (OCStringRef) OCArrayGetValueAtIndex(array, 3);
    if(!OCStringEqual(bit, STR("c"))) PRINTERROR;
    bit = (OCStringRef) OCArrayGetValueAtIndex(array, 4);
    if(!OCStringEqual(bit, STR("d"))) PRINTERROR;

    OCArraySortValues(array, OCRangeMake(0, OCArrayGetCount(array)), (OCComparatorFunction) OCStringCompare, NULL);

    bit = (OCStringRef) OCArrayGetValueAtIndex(array, 0);
    if(!OCStringEqual(bit, STR("a"))) PRINTERROR;
    bit = (OCStringRef) OCArrayGetValueAtIndex(array, 1);
    if(!OCStringEqual(bit, STR("b"))) PRINTERROR;
    bit = (OCStringRef) OCArrayGetValueAtIndex(array, 2);
    if(!OCStringEqual(bit, STR("c"))) PRINTERROR;
    bit = (OCStringRef) OCArrayGetValueAtIndex(array, 3);
    if(!OCStringEqual(bit, STR("d"))) PRINTERROR;
    bit = (OCStringRef) OCArrayGetValueAtIndex(array, 4);
    if(!OCStringEqual(bit, STR("e"))) PRINTERROR;

    fprintf(stderr,"%s end...without problems\n",__func__);
    OCRelease(array);
    return true;
}

bool stringTest0(void)
{
    fprintf(stderr,"%s begin...\n",__func__);
    
    double result = OCComplexFromCString("1+1");
    if(OCCompareDoubleValues(result, 2) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("42/2");
    if(OCCompareDoubleValues(result, 21) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("42/2+10");
    if(OCCompareDoubleValues(result, 31) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("(42/2+10)*2");
    if(OCCompareDoubleValues(result, 62) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("sqrt(4)");
    if(OCCompareDoubleValues(result, 2) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("cbrt(8)");
    if(OCCompareDoubleValues(result, 2) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("qtrt(16)");
    if(OCCompareDoubleValues(result, 2) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("exp(16)");
    if(OCCompareDoubleValues(result, exp(16)) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("log(16)");
    if(OCCompareDoubleValues(result, log(16)) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("acos(0.5)");
    if(OCCompareDoubleValues(result, acos(0.5)) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("asin(0.5)");
    if(OCCompareDoubleValues(result, asin(0.5)) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("cos(123)");
    if(OCCompareDoubleValues(result, cos(123)) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("sin(123)");
    if(OCCompareDoubleValues(result, sin(123)) != kOCCompareEqualTo) PRINTERROR;
    
    double complex complexResult = OCComplexFromCString("conj(1+I)");
    if(OCCompareDoubleValues(creal(complexResult), creal(conj(1+I))) != kOCCompareEqualTo) PRINTERROR;
    if(OCCompareDoubleValues(cimag(complexResult), cimag(conj(1+I))) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("creal(conj(1+I))");
    if(OCCompareDoubleValues(result, creal(conj(1+I))) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("cimag(conj(1+I))");
    if(OCCompareDoubleValues(result, cimag(conj(1+I))) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("carg(1+I)");
    if(OCCompareDoubleValues(result, cargument(1+I)) != kOCCompareEqualTo) PRINTERROR;
    
    fprintf(stderr,"%s end...without problems\n",__func__);
    return true;
}

bool stringTest1(void)
{
    fprintf(stderr,"%s begin...\n",__func__);
    
    OCStringRef theString = OCStringCreateWithCString("theStringWithAStringOnAStringTown");
    if(!OCStringEqual(theString, STR("theStringWithAStringOnAStringTown"))) PRINTERROR;
            
    OCMutableStringRef copy = OCStringCreateMutableCopy(theString);
    if(!OCStringEqual(copy, STR("theStringWithAStringOnAStringTown"))) PRINTERROR;

    OCStringReplace(copy, OCRangeMake(0, 3), STR("a"));
    if(!OCStringEqual(copy, STR("aStringWithAStringOnAStringTown"))) PRINTERROR;
    if(OCStringGetLength(copy)!= OCStringGetLength(STR("aStringWithAStringOnAStringTown"))) PRINTERROR;

//    OCStringFindAndReplace2(copy,STR("String"),STR("Rope"));
//    if(!OCStringEqual(copy, STR("aRope"))) PRINTERROR;

    OCStringFindAndReplace(copy,STR("String"),STR("Rope"),OCRangeMake(0, OCStringGetLength(copy)),kOCCompareCaseInsensitive);
    if(!OCStringEqual(copy, STR("aRopeWithARopeOnARopeTown"))) PRINTERROR;


    OCStringLowercase(copy);
    if(!OCStringEqual(copy, STR("aropewitharopeonaropetown"))) PRINTERROR;
    
    OCStringUppercase(copy);
    if(!OCStringEqual(copy, STR("AROPEWITHAROPEONAROPETOWN"))) PRINTERROR;
    
    OCArrayRef ranges = OCStringCreateArrayWithFindResults(copy, STR("ROPE"), OCRangeMake(0, OCStringGetLength(copy)), 0);
    
    OCRange *range = (OCRange *) OCArrayGetValueAtIndex(ranges, 0);
    if(range->location!=1) PRINTERROR;
    if(range->length!=4) PRINTERROR;
    
    range = (OCRange *) OCArrayGetValueAtIndex(ranges, 1);
    if(range->location!=10) PRINTERROR;
    if(range->length!=4) PRINTERROR;
    
    range = (OCRange *) OCArrayGetValueAtIndex(ranges, 2);
    if(range->location!=17) PRINTERROR;
    if(range->length!=4) PRINTERROR;
    
    OCRelease(ranges);
    
    ranges = OCStringCreateArrayWithFindResults(copy, STR("ROPE"), OCRangeMake(0, OCStringGetLength(copy)), kOCCompareBackwards);
    
    range = (OCRange *) OCArrayGetValueAtIndex(ranges, 2);
    if(range->location!=1) PRINTERROR;
    if(range->length!=4) PRINTERROR;
    
    range = (OCRange *) OCArrayGetValueAtIndex(ranges, 1);
    if(range->location!=10) PRINTERROR;
    if(range->length!=4) PRINTERROR;
    
    range = (OCRange *) OCArrayGetValueAtIndex(ranges, 0);
    if(range->location!=17) PRINTERROR;
    if(range->length!=4) PRINTERROR;
    
    OCRelease(ranges);
    
    OCArrayRef bits=OCStringCreateArrayBySeparatingStrings(copy, STR("ROPE"));
    if(OCArrayGetCount(bits)!=4) PRINTERROR;

    OCStringRef bit = (OCStringRef) OCArrayGetValueAtIndex(bits, 0);
    if(!OCStringEqual(bit, STR("A"))) PRINTERROR;
    bit = (OCStringRef) OCArrayGetValueAtIndex(bits, 1);
    if(!OCStringEqual(bit, STR("WITHA"))) PRINTERROR;
    bit = (OCStringRef) OCArrayGetValueAtIndex(bits, 2);
    if(!OCStringEqual(bit, STR("ONA"))) PRINTERROR;
    bit = (OCStringRef) OCArrayGetValueAtIndex(bits, 3);
    if(!OCStringEqual(bit, STR("TOWN"))) PRINTERROR;

    OCRelease(bits); // Release the array created by OCStringCreateArrayBySeparatingStrings
    OCRelease(copy); // Release the mutable string
    OCRelease(theString); // Release the original string

    fprintf(stderr,"%s end...without problems\n",__func__);
    return true;
}

bool stringTest2(void)
{
    fprintf(stderr,"%s begin...\n",__func__);
    
    OCStringRef string = OCStringCreateWithFormat(STR("Ix(%@)"),STR("H2"));
    if(!OCStringEqual(string, STR("Ix(H2)"))) PRINTERROR;
    OCRelease(string);
    
    string = OCStringCreateWithFormat(STR("Ix(%s)"),"H2");
    if(!OCStringEqual(string, STR("Ix(H2)"))) PRINTERROR;
    OCRelease(string);

    fprintf(stderr,"%s end...without problems\n",__func__);
    return true;
}
