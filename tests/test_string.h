#ifndef TEST_STRING_H
#define TEST_STRING_H

#include <stdbool.h>

bool stringTest0(void);
bool stringTest1(void);
bool stringTest2(void);
bool stringTest_mixed_format_specifiers(void);
bool test_OCStringAppendFormat(void);

// New UTF-8–aware tests
bool stringTest3(void);
bool stringTest4(void);
bool stringTest5(void);
bool stringTest6(void);
bool stringTest7(void);
bool stringTest8(void);
bool stringTest9(void);
bool stringTest10(void);
bool stringTest11(void);

bool stringTest_deepcopy(void);

#endif // TEST_STRING_H
