#ifndef TEST_OCINDEXSET_H
#define TEST_OCINDEXSET_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool OCIndexSetCreateAndAccess_test(void);
bool OCIndexSetAddAndContains_test(void);
bool OCIndexSetRangeAndBounds_test(void);
bool OCIndexSetSerialization_test(void);
bool OCIndexSetDeepCopy_test(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_OCINDEXSET_H */