// tests/test_fileutils.c
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif
#include "OCLibrary.h"
#include "test_utils.h"
// Cross-platform helper functions
#ifdef _WIN32
static const char *get_temp_dir(void) {
    static char temp_dir[MAX_PATH];
    DWORD len = GetTempPathA(MAX_PATH, temp_dir);
    if (len > 0 && len < MAX_PATH) {
        // Remove trailing backslash if present
        if (temp_dir[len - 1] == '\\') {
            temp_dir[len - 1] = '\0';
        }
        return temp_dir;
    }
    return "C:\\temp";  // fallback
}
static int cross_platform_mkstemp(char *template) {
    char *temp_dir = _strdup(get_temp_dir());
    char *filename = _tempnam(temp_dir, "ocfu_");
    if (!filename) {
        free(temp_dir);
        return -1;
    }
    // Ensure template buffer is large enough
    if (strlen(filename) >= PATH_MAX) {
        free(filename);
        free(temp_dir);
        return -1;
    }
    // Copy the generated filename back to template
    strcpy(template, filename);
    // Create and open the file (remove _O_TEMPORARY so file persists)
    int fd = _open(filename, _O_RDWR | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
    free(filename);
    free(temp_dir);
    return fd;
}
static int cross_platform_mkstemps(char *template, int suffixlen) {
    // For Windows, we'll use a simpler approach
    char *temp_dir = _strdup(get_temp_dir());
    char *filename = _tempnam(temp_dir, "ocfu_");
    if (!filename) {
        free(temp_dir);
        return -1;
    }
    // Add the suffix from the original template
    char *suffix = template + strlen(template) - suffixlen;
    char *full_filename = malloc(strlen(filename) + suffixlen + 1);
    strcpy(full_filename, filename);
    strcat(full_filename, suffix);
    // Ensure template buffer is large enough
    if (strlen(full_filename) >= PATH_MAX) {
        free(full_filename);
        free(filename);
        free(temp_dir);
        return -1;
    }
    strcpy(template, full_filename);
    int fd = _open(full_filename, _O_RDWR | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
    free(full_filename);
    free(filename);
    free(temp_dir);
    return fd;
}
static char *cross_platform_mkdtemp(char *template) {
    char *temp_dir = _strdup(get_temp_dir());
    char unique_dir[MAX_PATH];
    sprintf(unique_dir, "%s\\ocfu_dir_%d_%d", temp_dir, GetCurrentProcessId(), GetTickCount());
    if (_mkdir(unique_dir) == 0) {
        strcpy(template, unique_dir);
        free(temp_dir);
        return template;
    }
    free(temp_dir);
    return NULL;
}
#define mkstemp cross_platform_mkstemp
#define mkstemps cross_platform_mkstemps
#define mkdtemp cross_platform_mkdtemp
#define close _close
#define TEMP_DIR get_temp_dir()
#else
#define TEMP_DIR "/tmp"
#endif
bool test_path_join_and_split(void) {
    fprintf(stderr, "%s begin...\n", __func__);
#ifdef _WIN32
    OCStringRef a = OCStringCreateWithCString("C:\\foo");
    OCStringRef b = OCStringCreateWithCString("bar");
    OCStringRef j = OCPathJoin(a, b);
    if (strcmp(OCStringGetCString(j), "C:\\foo\\bar") != 0) PRINTERROR;
    OCRelease(j);
    // absolute b should win
    OCRelease(a);
    OCRelease(b);
    a = OCStringCreateWithCString("C:\\foo");
    b = OCStringCreateWithCString("C:\\abs\\path");
#else
    OCStringRef a = OCStringCreateWithCString("/foo");
    OCStringRef b = OCStringCreateWithCString("bar");
    OCStringRef j = OCPathJoin(a, b);
    if (strcmp(OCStringGetCString(j), "/foo/bar") != 0) PRINTERROR;
    OCRelease(j);
    // absolute b should win
    OCRelease(a);
    OCRelease(b);
    a = OCStringCreateWithCString("/foo");
    b = OCStringCreateWithCString("/abs/path");
#endif
    j = OCPathJoin(a, b);
#ifdef _WIN32
    if (strcmp(OCStringGetCString(j), "C:\\abs\\path") != 0) PRINTERROR;
#else
    if (strcmp(OCStringGetCString(j), "/abs/path") != 0) PRINTERROR;
#endif
    OCRelease(j);
    // empty a
    OCRelease(a);
    OCRelease(b);
    a = OCStringCreateWithCString("");
    b = OCStringCreateWithCString("xyz");
    j = OCPathJoin(a, b);
    if (strcmp(OCStringGetCString(j), "xyz") != 0) PRINTERROR;
    OCRelease(j);
    // dirname / basename
    OCRelease(a);
    OCRelease(b);
#ifdef _WIN32
    OCStringRef p = OCStringCreateWithCString("C:\\foo\\bar\\baz.txt");
    OCStringRef d = OCPathDirname(p);
    OCStringRef bn = OCPathBasename(p);
    if (strcmp(OCStringGetCString(d), "C:\\foo\\bar") != 0) PRINTERROR;
    if (strcmp(OCStringGetCString(bn), "baz.txt") != 0) PRINTERROR;
#else
    OCStringRef p = OCStringCreateWithCString("/foo/bar/baz.txt");
    OCStringRef d = OCPathDirname(p);
    OCStringRef bn = OCPathBasename(p);
    if (strcmp(OCStringGetCString(d), "/foo/bar") != 0) PRINTERROR;
    if (strcmp(OCStringGetCString(bn), "baz.txt") != 0) PRINTERROR;
#endif
    OCRelease(d);
    OCRelease(bn);
    // extension / replace ext
    OCStringRef ext = OCPathExtension(p);
    if (strcmp(OCStringGetCString(ext), ".txt") != 0) PRINTERROR;
    OCRelease(ext);
    OCStringRef extArg = OCStringCreateWithCString(".md");
    OCStringRef newp = OCPathByReplacingExtension(p, extArg);
    OCRelease(extArg);
#ifdef _WIN32
    if (strcmp(OCStringGetCString(newp), "C:\\foo\\bar\\baz.md") != 0) PRINTERROR;
#else
    if (strcmp(OCStringGetCString(newp), "/foo/bar/baz.md") != 0) PRINTERROR;
#endif
    OCRelease(newp);
    OCRelease(p);
    fprintf(stderr, "%s end.\n", __func__);
    return true;
}
bool test_file_and_dir_checks(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    // create temp file
    char tmpl[PATH_MAX];
    snprintf(tmpl, PATH_MAX, "%s/ocfu_testfileXXXXXX", TEMP_DIR);
    int fd = mkstemp(tmpl);
    if (fd < 0) PRINTERROR;
    close(fd);
    if (!OCFileExists(tmpl)) PRINTERROR;
    if (!OCIsRegularFile(tmpl)) PRINTERROR;
    if (OCIsDirectory(tmpl)) PRINTERROR;
    // cleanup
    if (!OCRemoveItem(tmpl, NULL)) PRINTERROR;
    if (OCFileExists(tmpl)) PRINTERROR;
    // create temp dir
    char dirtmpl[PATH_MAX];
    snprintf(dirtmpl, PATH_MAX, "%s/ocfu_testdirXXXXXX", TEMP_DIR);
    char *dpath = mkdtemp(dirtmpl);
    if (!dpath) PRINTERROR;
    if (!OCFileExists(dpath)) PRINTERROR;
    if (!OCIsDirectory(dpath)) PRINTERROR;
    if (OCIsRegularFile(dpath)) PRINTERROR;
    // cleanup
    if (!OCRemoveItem(dpath, NULL)) PRINTERROR;
    if (OCFileExists(dpath)) PRINTERROR;
    fprintf(stderr, "%s end.\n", __func__);
    return true;
}
bool test_create_and_list_directory(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    // base temp directory
    char base[PATH_MAX];
    snprintf(base, PATH_MAX, "%s/ocfu_testdirXXXXXX", TEMP_DIR);
    char *b = mkdtemp(base);
    if (!b) PRINTERROR;
    // simple non-recursive
    OCStringRef err = NULL;
    char dummy_path[PATH_MAX];
    snprintf(dummy_path, PATH_MAX, "%s/ocfu_dummy_nonexist", TEMP_DIR);
    if (!OCCreateDirectory(dummy_path, false, &err)) {
        fprintf(stderr, "Unexpected failure: %s\n",
                OCStringGetCString(err));
        PRINTERROR;
    }
    // cleanup that
    OCRemoveItem(dummy_path, NULL);
    // recursive nested
    char nested[PATH_MAX];
    snprintf(nested, PATH_MAX, "%s/sub1/sub2", b);
    if (!OCCreateDirectory(nested, true, &err)) {
        fprintf(stderr, "mkdir -p failed: %s\n",
                OCStringGetCString(err));
        PRINTERROR;
    }
    // create a file at both levels
    char f1[PATH_MAX], f2[PATH_MAX];
    snprintf(f1, PATH_MAX, "%s/one.txt", b);
    snprintf(f2, PATH_MAX, "%s/sub1/sub2/two.txt", b);
    FILE *fp = fopen(f1, "w");
    if (!fp) PRINTERROR;
    fprintf(fp, "1");
    fclose(fp);
    fp = fopen(f2, "w");
    if (!fp) PRINTERROR;
    fprintf(fp, "2");
    fclose(fp);
    // list non-recursive
    OCArrayRef list = OCListDirectory(b, false, &err);
    if (!list || OCArrayGetCount(list) != 1) PRINTERROR;
    OCRelease(list);
    // list recursive
    list = OCListDirectory(b, true, &err);
    if (!list || OCArrayGetCount(list) != 2) PRINTERROR;
    OCRelease(list);
    // cleanup whole tree
    OCRemoveItem(nested, NULL);                   // remove sub2
    OCRemoveItem(strstr(nested, "/sub1"), NULL);  // remove sub1
    OCRemoveItem(f1, NULL);
    OCRemoveItem(b, NULL);
    fprintf(stderr, "%s end.\n", __func__);
    return true;
}
bool test_rename_and_remove(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    char tmpl[PATH_MAX];
    snprintf(tmpl, PATH_MAX, "%s/ocfu_renameXXXXXX", TEMP_DIR);
    int fd = mkstemp(tmpl);
    if (fd < 0) PRINTERROR;
    close(fd);
    char newpath[PATH_MAX];
    snprintf(newpath, PATH_MAX, "%s_renamed", tmpl);
    if (!OCRenameItem(tmpl, newpath, NULL)) PRINTERROR;
    if (!OCFileExists(newpath)) PRINTERROR;
    if (!OCRemoveItem(newpath, NULL)) PRINTERROR;
    if (OCFileExists(newpath)) PRINTERROR;
    fprintf(stderr, "%s end.\n", __func__);
    return true;
}
bool test_string_file_io(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "%s/ocfu_strioXXXXXX.txt", TEMP_DIR);
    int fd = mkstemps(path, 4);
    if (fd < 0) PRINTERROR;
    close(fd);
    const char *msg = "Hello, OCTypes!";
    OCStringRef outErr = NULL;
    OCStringRef s = OCStringCreateWithCString(msg);
    if (!OCStringWriteToFile(s, path, &outErr)) {
        fprintf(stderr, "Write error: %s\n",
                OCStringGetCString(outErr));
        PRINTERROR;
    }
    OCRelease(s);
    OCStringRef inErr = NULL;
    OCStringRef readback = OCStringCreateWithContentsOfFile(path, &inErr);
    if (!readback) {
        fprintf(stderr, "Read error: %s\n",
                OCStringGetCString(inErr));
        PRINTERROR;
    }
    if (strcmp(OCStringGetCString(readback), msg) != 0) PRINTERROR;
    OCRelease(readback);
    OCRemoveItem(path, NULL);
    fprintf(stderr, "%s end.\n", __func__);
    return true;
}
/**
 * Write a mixed‐type dictionary to JSON, read it back as a string,
 * and verify the presence of the serialized values.
 */
bool test_dictionary_write_simple(void) {
    // Build dictionary
    OCMutableDictionaryRef dict = OCDictionaryCreateMutable(0);
    OCDictionarySetValue(dict, STR("foo"), STR("bar"));
    OCNumberRef num = OCNumberCreateWithSInt32(42);
    OCDictionarySetValue(dict, STR("num"), num);
    OCRelease(num);
    OCDictionarySetValue(dict, STR("flag"), OCBooleanGetWithBool(true));
    OCMutableArrayRef arr = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    OCArrayAppendValue(arr, STR("a"));
    OCArrayAppendValue(arr, STR("b"));
    OCDictionarySetValue(dict, STR("list"), arr);
    OCRelease(arr);
    // Create temp file
    char tmpl[PATH_MAX];
    snprintf(tmpl, PATH_MAX, "%s/ocdict_jsonXXXXXX", TEMP_DIR);
    int fd = mkstemp(tmpl);
    if (fd < 0) PRINTERROR;
    close(fd);
    // Write to file
    OCStringRef err = NULL;
    if (!OCTypeWriteJSONToFile((OCTypeRef)dict, tmpl, &err)) {
        OCRelease(err);
        OCRelease(dict);
        PRINTERROR;
    }
    // Read back
    OCStringRef json = OCStringCreateWithContentsOfFile(tmpl, NULL);
    if (!json) {
        OCRemoveItem(tmpl, NULL);
        OCRelease(dict);
        PRINTERROR;
    }
    const char *s = OCStringGetCString(json);
    if (!s || !strstr(s, "\"foo\"") || !strstr(s, "\"bar\"") ||
        !strstr(s, "\"num\"") || !strstr(s, "42") ||
        !strstr(s, "\"flag\"") || !strstr(s, "true") ||
        !strstr(s, "\"list\"") || !strstr(s, "[\"a\",\"b\"]")) {
        OCRelease(json);
        OCRemoveItem(tmpl, NULL);
        OCRelease(dict);
        PRINTERROR;
    }
    OCRelease(json);
    OCRemoveItem(tmpl, NULL);
    OCRelease(dict);
    return true;
}
/**
 * Writing an empty dictionary should produce "{}" (possibly with whitespace).
 */
bool test_dictionary_write_empty(void) {
    OCMutableDictionaryRef dict = OCDictionaryCreateMutable(0);
    // Create temporary file
    char tmpl[PATH_MAX];
    snprintf(tmpl, PATH_MAX, "%s/ocdict_emptyXXXXXX", TEMP_DIR);
    int fd = mkstemp(tmpl);
    if (fd < 0) PRINTERROR;
    close(fd);
    // Write dictionary to JSON file
    OCStringRef err = NULL;
    if (!OCTypeWriteJSONToFile((OCTypeRef)dict, tmpl, &err)) {
        OCRelease(err);
        OCRelease(dict);
        PRINTERROR;
    }
    // Read JSON back from file
    OCStringRef json = OCStringCreateWithContentsOfFile(tmpl, NULL);
    if (!json) {
        OCRemoveItem(tmpl, NULL);
        OCRelease(dict);
        PRINTERROR;
    }
    const char *s = OCStringGetCString(json);
    if (!s) {
        OCRelease(json);
        OCRemoveItem(tmpl, NULL);
        OCRelease(dict);
        PRINTERROR;
    }
    // Strip leading whitespace and validate content
    while (*s && (*s == ' ' || *s == '\n' || *s == '\t')) s++;
    if (strncmp(s, "{}", 2) != 0) {
        OCRelease(json);
        OCRemoveItem(tmpl, NULL);
        OCRelease(dict);
        PRINTERROR;
    }
    OCRelease(json);
    OCRemoveItem(tmpl, NULL);
    OCRelease(dict);
    return true;
}
/**
 * Attempt to write into a non-existent directory—should fail with an error.
 */
bool test_dictionary_write_error(void) {
    OCMutableDictionaryRef dict = OCDictionaryCreateMutable(0);
    OCDictionarySetValue(dict, STR("x"), STR("y"));
    const char *badpath = "/no_such_dir/ocdict.json";
    OCStringRef err = NULL;
    bool ok = OCTypeWriteJSONToFile((OCTypeRef)dict, badpath, &err);
    if (ok || err == NULL) {
        if (err) OCRelease(err);
        OCStringRef maybe = OCStringCreateWithContentsOfFile(badpath, NULL);
        if (maybe) OCRelease(maybe);
        OCRelease(dict);
        PRINTERROR;
    }
    OCRelease(err);
    OCRelease(dict);
    return true;
}
