//
//  main.c
//  tests
//
//  Created by Philip Grandinetti on 6/17/17.
//  Updated by ChatGPT on 2025-05-11.
//

// Common utilities, includes, and PRINTERROR macro
#include "test_utils.h"

// Include headers for all test modules
#include "test_string.h"
#include "test_array.h"
#include "test_math.h"
#include "test_autoreleasepool.h"
#include "test_data.h"
#include "test_boolean.h"
#include "test_type.h"
#include "test_number.h"       // ← New: OCNumber tests
#include "test_dictionary.h"  // ← New: OCDictionary tests

// Note: The OCStringCompareAdapter is now in test_array.c
// Note: The extern declaration for raise_to_integer_power is now in test_math.h

int main(int argc, const char * argv[]) {
    // Suppress unused parameter warnings
    (void)argc;
    (void)argv;

    int failures = 0;
    
    fprintf(stderr, "Starting OCTypes test suite...\n");

    // Call test functions from their respective modules
    if (!typeTest0())            failures++;
    if (!stringTest0())          failures++;
    if (!stringTest1())          failures++;
    if (!stringTest2())          failures++;
    if (!arrayTest0())           failures++;
    if (!arrayTest1_creation())  failures++;
    if (!arrayTest2_access())    failures++;
    if (!arrayTest3_modification()) failures++;
    if (!arrayTest4_search_sort()) failures++;
    if (!mathTest0())            failures++;
    if (!autoreleasePoolTest0()) failures++;
    if (!dataTest0())            failures++;
    if (!booleanTest0())         failures++;
    if (!numberTest0())          failures++;  // ← New: Invoke OCNumber tests
    if (!dictionaryTest0())      failures++;  // ← New: Invoke OCDictionary tests

    if (failures) {
        fprintf(stderr, "\n%d test(s) failed.\n", failures);
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "\nAll tests passed successfully!\n");
    return EXIT_SUCCESS;
}

// All original test function implementations (stringTest0, arrayTest0, etc.)
// have been moved to their respective .c files (test_string.c, test_array.c, etc.).
// The PRINTERROR macro is now in test_utils.h.
// The OCStringCompareAdapter is in test_array.c.
// The extern declaration for raise_to_integer_power is in test_math.h.
