//
//  main.c
//  tests
//
//  Created by Philip Grandinetti on 6/17/17.
//  Updated by ChatGPT on 2025-05-11.
//

// Common utilities, includes, and PRINTERROR macro
#include "test_utils.h"
#include "../src/OCAutoreleasePool.h"
#include "../src/OCLibrary.h"

// Include headers for all test modules
#include "test_string.h"
#include "test_complex_parser.h"
#include "test_array.h"
#include "test_math.h"
#include "test_autoreleasepool.h"
#include "test_data.h"
#include "test_boolean.h"
#include "test_type.h"
#include "test_number.h"  
#include "test_dictionary.h"
#include "test_dictionary.h" 
#include "test_indexarray.h"
#include "test_indexset.h" 
#include "test_indexpairset.h"
#include "test_fileutils.h"



// Note: The OCStringCompareAdapter is now in test_array.c
// Note: The extern declaration for raise_to_integer_power is now in test_math.h

int main(int argc, const char * argv[]) {
    // Suppress unused parameter warnings
    (void)argc;
    (void)argv;

    // Create an autorelease pool for tests
    OCAutoreleasePoolRef pool = OCAutoreleasePoolCreate();

    int failures = 0;
    
    fprintf(stderr, "Starting OCTypes test suite...\n");

    // Call test functions from their respective modules

    if (!autoreleasePoolTest0()) failures++;
    if (!autoreleasePoolTest1()) failures++;  // New: test Oak jon pool drain
    if (!typeTest0())            failures++;
    if (!typeTest1())            failures++;  // New: type description tests
    if (!typeTest2())            failures++;  // New: type description tests
    if (!mathTest0())            failures++;
    if (!mathTest1())            failures++;  // New: extra math API tests
    if (!dataTest0())            failures++;
    if (!dataTest1())            failures++;  // New: mutable-data API tests
    if (!dataTest_deepcopy())    failures++;  // New: OCData deep copy tests
    if (!dataTest_base64_roundtrip()) failures++;  // New: Base64 roundtrip tests

    if (!booleanTest0())         failures++;
    if (!numberTest0())          failures++;  // ← New: Invoke OCNumber tests
    if (!dictionaryTest0())      failures++;  // ← Invoke OCDictionary tests
    if (!dictionaryTest1())      failures++;  // ← Invoke OCDictionary edge case tests
    if (!dictionaryTest2())      failures++;  // ← Invoke OCDictionary array operations tests
    if (!dictionaryTest3())      failures++;  // ← Invoke OCDictionary mixed type tests
    if (!dictionaryTest4())      failures++;  // ← New: Invoke OCDictionary extreme cases tests
    if (!dictionaryTest5())      failures++;  // ← New: Invoke OCDictionary deep nesting tests
    if (!OCDictionaryTestDeepCopy()) failures++;  // ← New: Invoke OCDictionary deep copy tests
    if (!test_OCDictionary_deepcopy2()) failures++;  // ← New: Invoke OCDictionary deep copy tests

    if (!arrayTest0())           failures++;
    if (!arrayTest1_creation())  failures++;
    if (!arrayTest2_access())    failures++;
    if (!arrayTest3_modification()) failures++;
    if (!arrayTest4_search_sort()) failures++;

    if (!stringTest1())          failures++;
    if (!stringTest2())          failures++;
    if (!stringTest3())          failures++;
    if (!stringTest_mixed_format_specifiers()) failures++;
    if (!test_OCStringAppendFormat())   failures++;

    if (!stringTest4())          failures++;
    if (!stringTest5())          failures++;
    if (!stringTest6())          failures++;
    if (!stringTest7())          failures++;
    if (!stringTest8())          failures++;
    if (!stringTest9())          failures++;
    if (!stringTest10())         failures++;
    if (!stringTest11())         failures++;
    if (!stringTest_deepcopy())  failures++;


    if (!complex_parser_Test0())          failures++;


    if (!OCIndexArrayCreateAndCount_test())       failures++;
    if (!OCIndexArrayGetValueAtIndex_test())      failures++;
    if (!OCIndexArraySetValueAtIndex_test())      failures++;
    if (!OCIndexArrayRemoveValueAtIndex_test())   failures++;
    if (!OCIndexArrayRemoveValuesAtIndexes_test())failures++;
    if (!OCIndexArrayDeepCopy_test())             failures++;

    if (!OCIndexSetCreateAndAccess_test())        failures++;
    if (!OCIndexSetAddAndContains_test())         failures++;
    if (!OCIndexSetRangeAndBounds_test())         failures++;
    if (!OCIndexSetSerialization_test())          failures++;
    if (!OCIndexSetDeepCopy_test())              failures++;

    if (!OCIndexPairSetCreation_test())             failures++;
    if (!OCIndexPairSetAddAndContains_test())       failures++;
    if (!OCIndexPairSetValueLookup_test())          failures++;
    if (!OCIndexPairSetEquality_test())             failures++;
    if (!OCIndexPairSetShow_test())                 failures++;
    if (!OCIndexPairSetDeepCopy_test())           failures++;


    if (!test_path_join_and_split())    failures++;
    if (!test_file_and_dir_checks())    failures++;
    if (!test_create_and_list_directory()) failures++;
    if (!test_rename_and_remove())      failures++;
    if (!test_string_file_io())         failures++;
    if (!test_dictionary_write_simple())    failures++;
    if (!test_dictionary_write_empty())    failures++;
    if (!test_dictionary_write_error())    failures++;

    if (failures) {
        fprintf(stderr, "\n%d test(s) failed.\n", failures);
        OCAutoreleasePoolRelease(pool);
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "\nAll tests passed successfully!\n");
    OCAutoreleasePoolRelease(pool);

    OCTypesShutdown();  // Clean up OCTypes resources
    return EXIT_SUCCESS;
}

// All original test function implementations (stringTest0, arrayTest0, etc.)
// have been moved to their respective .c files (test_string.c, test_array.c, etc.).
// The PRINTERROR macro is now in test_utils.h.
// The OCStringCompareAdapter is in test_array.c.
// The extern declaration for raise_to_integer_power is in test_math.h.
