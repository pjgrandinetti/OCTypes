// OCFileUtils.c
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "OCLibrary.h"
//-----------------------------------------------------------------------------
// MARK: OCDataCreateWithContentsOfFile
//-----------------------------------------------------------------------------
OCDataRef
OCDataCreateWithContentsOfFile(const char *path,
                               OCStringRef *errorString) {
    if (errorString) *errorString = NULL;
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        if (errorString) {
            *errorString = OCStringCreateWithFormat(
                STR("Unable to open file \"%s\": %s"),
                path, strerror(errno));
        }
        return NULL;
    }
    // figure out file length
    if (fseek(fp, 0, SEEK_END) != 0) {
        if (errorString) {
            *errorString = OCStringCreateWithFormat(
                STR("Failed to seek end of \"%s\": %s"),
                path, strerror(errno));
        }
        fclose(fp);
        return NULL;
    }
    long fileLen = ftell(fp);
    if (fileLen < 0) {
        if (errorString) {
            *errorString = OCStringCreateWithFormat(
                STR("Failed to tell position in \"%s\": %s"),
                path, strerror(errno));
        }
        fclose(fp);
        return NULL;
    }
    rewind(fp);
    // read bytes
    void *buffer = malloc((size_t)fileLen);
    if (!buffer) {
        if (errorString) {
            *errorString = STR("Out of memory reading file");
        }
        fclose(fp);
        return NULL;
    }
    size_t got = fread(buffer, 1, (size_t)fileLen, fp);
    if (got != (size_t)fileLen) {
        if (errorString) {
            *errorString = OCStringCreateWithFormat(
                STR("Error reading \"%s\": only read %zu of %ld bytes"),
                path, got, fileLen);
        }
        free(buffer);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    // wrap into OCDataRef
    OCMutableDataRef data = OCDataCreateMutable(got);
    OCDataAppendBytes(data, buffer, got);
    free(buffer);
    return data;
}
//-----------------------------------------------------------------------------
// MARK: recursive folder loader
//-----------------------------------------------------------------------------
static bool
_OCDataLoadFolderRec(const char *basePath,
                     const char *relPath,
                     int depth,
                     OCMutableDictionaryRef dict,
                     OCStringRef *outError) {
    if (depth < 0) return true;
    // build the absolute path to open
    char fullPath[PATH_MAX];
    if (relPath[0] == '\0')
        snprintf(fullPath, sizeof fullPath, "%s", basePath);
    else
        snprintf(fullPath, sizeof fullPath, "%s/%s", basePath, relPath);
    DIR *dir = opendir(fullPath);
    if (!dir) {
        if (outError) *outError = OCStringCreateWithCString(strerror(errno));
        return false;
    }
    struct dirent *ent;
    while ((ent = readdir(dir))) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        // new relative path
        char childRel[PATH_MAX];
        if (relPath[0] == '\0')
            snprintf(childRel, sizeof childRel, "%s", ent->d_name);
        else
            snprintf(childRel, sizeof childRel, "%s/%s", relPath, ent->d_name);
        // absolute child path
        char childFull[PATH_MAX];
        snprintf(childFull, sizeof childFull, "%s/%s", basePath, childRel);
        struct stat st;
        if (stat(childFull, &st) != 0) {
            // skip broken or inaccessible
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            if (!_OCDataLoadFolderRec(basePath, childRel, depth - 1, dict, outError)) {
                closedir(dir);
                return false;
            }
        } else if (S_ISREG(st.st_mode)) {
            OCStringRef key = OCStringCreateWithCString(childRel);
            OCDataRef data = OCDataCreateWithContentsOfFile(childFull, outError);
            if (!data) {
                OCRelease(key);
                closedir(dir);
                return false;
            }
            OCDictionarySetValue(dict, key, data);
            OCRelease(key);
            OCRelease(data);
        }
    }
    closedir(dir);
    return true;
}
OCDictionaryRef
OCDictionaryCreateWithContentsOfFolder(const char *folderPath,
                                       int maxDepth,
                                       OCStringRef *errorMessage) {
    if (errorMessage) *errorMessage = NULL;
    if (!folderPath) {
        if (errorMessage)
            *errorMessage = OCStringCreateWithCString("folderPath was NULL");
        return NULL;
    }
    struct stat st;
    if (stat(folderPath, &st) != 0 || !S_ISDIR(st.st_mode)) {
        if (errorMessage)
            *errorMessage = OCStringCreateWithCString("Not a directory");
        return NULL;
    }
    OCMutableDictionaryRef dict = OCDictionaryCreateMutable(0);
    if (!_OCDataLoadFolderRec(folderPath, "", maxDepth, dict, errorMessage)) {
        OCRelease(dict);
        return NULL;
    }
    return dict;
}