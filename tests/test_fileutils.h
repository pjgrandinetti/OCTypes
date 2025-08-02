// tests/test_fileutils.h
#ifndef TEST_FILEUTILS_H
#define TEST_FILEUTILS_H
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "OCFileUtilities.h"
#include "OCLibrary.h"
/**
 * Each of these returns true on success, false (after having
 * PRINTERROR-ed) on failure.
 */
bool test_path_join_and_split(void);
bool test_file_and_dir_checks(void);
bool test_create_and_list_directory(void);
bool test_rename_and_remove(void);
bool test_string_file_io(void);
bool test_dictionary_write_simple(void);
bool test_dictionary_write_empty(void);
bool test_dictionary_write_error(void);
#endif  // TEST_FILEUTILS_H
