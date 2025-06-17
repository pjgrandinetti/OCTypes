// tests/test_fileutils.c
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "OCFileUtilities.h"
#include "OCLibrary.h"
#include "test_utils.h"
bool test_path_join_and_split(void) {
    fprintf(stderr, "%s begin...\n", __func__);
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
    j = OCPathJoin(a, b);
    if (strcmp(OCStringGetCString(j), "/abs/path") != 0) PRINTERROR;
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
    OCStringRef p = OCStringCreateWithCString("/foo/bar/baz.txt");
    OCStringRef d = OCPathDirname(p);
    OCStringRef bn = OCPathBasename(p);
    if (strcmp(OCStringGetCString(d), "/foo/bar") != 0) PRINTERROR;
    if (strcmp(OCStringGetCString(bn), "baz.txt") != 0) PRINTERROR;
    OCRelease(d);
    OCRelease(bn);
    // extension / replace ext
    OCStringRef ext = OCPathExtension(p);
    if (strcmp(OCStringGetCString(ext), ".txt") != 0) PRINTERROR;
    OCRelease(ext);
    OCStringRef extArg = OCStringCreateWithCString(".md");
    OCStringRef newp = OCPathByReplacingExtension(p, extArg);
    OCRelease(extArg);
    if (strcmp(OCStringGetCString(newp), "/foo/bar/baz.md") != 0) PRINTERROR;
    OCRelease(newp);
    OCRelease(p);
    fprintf(stderr, "%s end.\n", __func__);
    return true;
}
bool test_file_and_dir_checks(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    // create temp file
    char tmpl[] = "/tmp/ocfu_testfileXXXXXX";
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
    char dirtmpl[] = "/tmp/ocfu_testdirXXXXXX";
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
    char base[] = "/tmp/ocfu_testdirXXXXXX";
    char *b = mkdtemp(base);
    if (!b) PRINTERROR;
    // simple non-recursive
    OCStringRef err = NULL;
    if (!OCCreateDirectory("/tmp/ocfu_dummy_nonexist", false, &err)) {
        fprintf(stderr, "Unexpected failure: %s\n",
                OCStringGetCString(err));
        PRINTERROR;
    }
    // cleanup that
    OCRemoveItem("/tmp/ocfu_dummy_nonexist", NULL);
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
    char tmpl[] = "/tmp/ocfu_renameXXXXXX";
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
    char path[] = "/tmp/ocfu_strioXXXXXX.txt";
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
    OCArrayAppendValue(arr, OCStringCreateWithCString("a"));
    OCArrayAppendValue(arr, OCStringCreateWithCString("b"));
    OCDictionarySetValue(dict, STR("list"), arr);
    OCRelease(arr);

    // Create temp file
    char tmpl[] = "/tmp/ocdict_jsonXXXXXX";
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
    char tmpl[] = "/tmp/ocdict_emptyXXXXXX";
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
