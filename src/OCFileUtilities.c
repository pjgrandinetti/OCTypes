// OCFileUtils.c
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "OCTypes.h"
// Fallback for PATH_MAX if not defined
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
// helper: turn an OCStringRef into a malloc’d UTF-8 C string
static char *_OCStringCopyUTF8(OCStringRef s) {
    const char *p = OCStringGetCString(s);
    if (!p) return NULL;
    return strdup(p);
}
static char _sep =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif
/// @internal
/// @brief Test if a C path is absolute (handles Unix and Windows drive-letter cases).
static bool
_OCPathIsAbsolute(const char *s) {
    if (!s || !s[0]) return false;
#ifdef _WIN32
    // e.g. "C:\" or "\\server\share"
    if ((s[0] >= 'A' && s[0] <= 'Z') ||
        (s[0] >= 'a' && s[0] <= 'z')) {
        if (s[1] == ':' && (s[2] == '\\' || s[2] == '/'))
            return true;
    }
    if (s[0] == '\\' && s[1] == '\\')  // UNC share
        return true;
    return false;
#else
    return (s[0] == '/');
#endif
}
/// @internal
/// @brief Join two path components, observing absolute-path semantics.
OCStringRef
OCPathJoin(OCStringRef a, OCStringRef b) {
    // Copy to UTF-8
    char *ca = _OCStringCopyUTF8(a);
    char *cb = _OCStringCopyUTF8(b);
    bool aEmpty = (ca[0] == '\0');
    bool bEmpty = (cb[0] == '\0');
    bool bIsAbs = _OCPathIsAbsolute(cb);
    OCStringRef result = NULL;
    if (bIsAbs || aEmpty) {
        // b wins if it's absolute, or a is empty
        result = OCStringCreateWithCString(cb);
    } else if (bEmpty) {
        // just a
        result = OCStringCreateWithCString(ca);
    } else {
        // ensure there's exactly one separator between
        size_t la = strlen(ca);
        bool aEnds = (ca[la - 1] == _sep);
        size_t needed = la + 1 + strlen(cb) + 1;
        char *buf = malloc(needed);
        if (!buf) return NULL;
        strcpy(buf, ca);
        if (!aEnds) {
            buf[la] = _sep;
            buf[la + 1] = '\0';
        }
        strcat(buf, cb);
        result = OCStringCreateWithCString(buf);
        free(buf);
    }
    free(ca);
    free(cb);
    return result;
}
OCStringRef OCPathDirname(OCStringRef path) {
    char *cp = _OCStringCopyUTF8(path);
    char *p = strrchr(cp, _sep);
    OCStringRef r;
    if (!p) {
        r = OCStringCreateWithCString(".");
    } else if (p == cp) {
        // root “/foo” → return “/”
        char tmp[2] = {_sep, '\0'};
        r = OCStringCreateWithCString(tmp);
    } else {
        *p = '\0';
        r = OCStringCreateWithCString(cp);
    }
    free(cp);
    return r;
}
OCStringRef OCPathBasename(OCStringRef path) {
    char *cp = _OCStringCopyUTF8(path);
    char *p = strrchr(cp, _sep);
    OCStringRef r = OCStringCreateWithCString(p ? p + 1 : cp);
    free(cp);
    return r;
}
OCStringRef OCPathExtension(OCStringRef path) {
    char *cp = _OCStringCopyUTF8(path);
    char *dirsep = strrchr(cp, _sep);
    char *dot = strrchr(cp, '.');
    OCStringRef r;
    if (!dot || (dirsep && dot < dirsep)) {
        r = OCStringCreateWithCString("");
    } else {
        r = OCStringCreateWithCString(dot);
    }
    free(cp);
    return r;
}
OCStringRef OCPathByReplacingExtension(OCStringRef path,
                                       OCStringRef newExt) {
    // normalize newExt to begin with '.'
    char *cnew = _OCStringCopyUTF8(newExt);
    char *n = cnew;
    if (n[0] != '.') {
        size_t L = strlen(n);
        char *buf = malloc(L + 2);
        if (!buf) {
            free(n);
            return NULL;
        }
        buf[0] = '.';
        memcpy(buf + 1, n, L + 1);
        free(n);
        n = buf;
    }
    // split off
    char *cp = _OCStringCopyUTF8(path);
    char *dirsep = strrchr(cp, _sep);
    char *dot = strrchr(cp, '.');
    if (!dot || (dirsep && dot < dirsep)) {
        // no existing ext
        dot = cp + strlen(cp);
    }
    *dot = '\0';  // truncate at the old dot (or at end)
    size_t cap = strlen(cp) + strlen(n) + 1;
    char *out = malloc(cap);
    if (!out) {
        free(cp);
        free(n);
        return NULL;
    }
    strcpy(out, cp);
    strcat(out, n);
    OCStringRef r = OCStringCreateWithCString(out);
    free(cp);
    free(n);
    free(out);
    return r;
}
bool OCFileExists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0);
}
bool OCIsDirectory(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}
bool OCIsRegularFile(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return S_ISREG(st.st_mode);
}
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p, mode) _mkdir(p)
#else
#define MKDIR(p, mode) mkdir(p, mode)
#endif
bool OCCreateDirectory(const char *path,
                       bool recursive,
                       OCStringRef *err) {
    if (!recursive) {
        if (MKDIR(path, 0755) != 0 && errno != EEXIST) {
            if (err) {
                *err = OCStringCreateWithFormat(
                    STR("mkdir \"%s\" failed: %s"),
                    path, strerror(errno));
            }
            return false;
        }
        return true;
    }
    // recursive: build up each component
    char buf[PATH_MAX];
    size_t len = strnlen(path, PATH_MAX);
    if (len == 0 || len >= PATH_MAX) {
        if (err) *err = STR("path too long or empty");
        return false;
    }
    strncpy(buf, path, len + 1);
    // if absolute and starts with '/', skip the leading slash so we don't attempt to mkdir("")
    char *p = buf;
    if (buf[0] == '/') p = buf + 1;
    for (; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (MKDIR(buf, 0755) != 0 && errno != EEXIST) {
                if (err) {
                    *err = OCStringCreateWithFormat(
                        STR("mkdir \"%s\" failed: %s"),
                        buf, strerror(errno));
                }
                return false;
            }
            *p = '/';
        }
    }
    // final leaf
    if (MKDIR(buf, 0755) != 0 && errno != EEXIST) {
        if (err) {
            *err = OCStringCreateWithFormat(
                STR("mkdir \"%s\" failed: %s"),
                buf, strerror(errno));
        }
        return false;
    }
    return true;
}
static bool
_OCListDirRec(const char *basePath,
              const char *relPath,
              OCMutableArrayRef list,
              bool recursive,
              OCStringRef *err) {
    char full[PATH_MAX];
    if (relPath[0] == '\0')
        snprintf(full, sizeof full, "%s", basePath);
    else
        snprintf(full, sizeof full, "%s/%s", basePath, relPath);
    DIR *d = opendir(full);
    if (!d) {
        if (err) *err = OCStringCreateWithFormat(
                     STR("opendir \"%s\": %s"), full, strerror(errno));
        return false;
    }
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        char childRel[PATH_MAX];
        if (relPath[0] == '\0')
            snprintf(childRel, sizeof childRel, "%s", ent->d_name);
        else
            snprintf(childRel, sizeof childRel, "%s/%s", relPath, ent->d_name);
        char childFull[PATH_MAX];
        snprintf(childFull, sizeof childFull, "%s/%s", basePath, childRel);
        struct stat st;
        if (stat(childFull, &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) {
            if (recursive) {
                if (!_OCListDirRec(basePath, childRel, list, recursive, err)) {
                    closedir(d);
                    return false;
                }
            }
        } else if (S_ISREG(st.st_mode)) {
            OCStringRef s = OCStringCreateWithCString(childRel);
            OCArrayAppendValue(list, s);
            OCRelease(s);
        }
    }
    closedir(d);
    return true;
}
OCArrayRef
OCListDirectory(const char *path,
                bool recursive,
                OCStringRef *err) {
    struct stat st;
    if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode)) {
        if (err) *err = OCStringCreateWithCString("not a directory");
        return NULL;
    }
    OCMutableArrayRef list = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    if (!_OCListDirRec(path, "", list, recursive, err)) {
        OCRelease(list);
        return NULL;
    }
    return (OCArrayRef)list;
}
#ifdef _WIN32
#include <io.h>
#define unlink _unlink
#define rmdir _rmdir
#else
#include <unistd.h>
#endif
bool OCRemoveItem(const char *path,
                  OCStringRef *err) {
    struct stat st;
    if (stat(path, &st) != 0) {
        if (errno == ENOENT) {
            // nothing to do
            return true;
        }
        if (err) {
            *err = OCStringCreateWithFormat(
                STR("stat(\"%s\") failed: %s"),
                path, strerror(errno));
        }
        return false;
    }
    int rc;
    if (S_ISDIR(st.st_mode)) {
        rc = rmdir(path);
    } else {
        rc = unlink(path);
    }
    if (rc != 0) {
        if (err) {
            *err = OCStringCreateWithFormat(
                STR("remove \"%s\" failed: %s"),
                path, strerror(errno));
        }
        return false;
    }
    return true;
}
bool OCRenameItem(const char *oldPath,
                  const char *newPath,
                  OCStringRef *err) {
    if (rename(oldPath, newPath) != 0) {
        if (err) {
            *err = OCStringCreateWithFormat(
                STR("rename \"%s\" → \"%s\" failed: %s"),
                oldPath, newPath, strerror(errno));
        }
        return false;
    }
    return true;
}
OCStringRef
OCStringCreateWithContentsOfFile(const char *path, OCStringRef *err) {
    // First, read raw bytes
    OCStringRef localErr = NULL;
    OCDataRef data = OCDataCreateWithContentsOfFile(path, &localErr);
    if (!data) {
        if (err)
            *err = localErr;
        else
            OCRelease(localErr);
        return NULL;
    }
    // Convert bytes → OCString (assumes UTF-8)
    OCStringRef str = OCStringCreateWithExternalRepresentation(data);
    OCRelease(data);
    return str;
}
bool OCStringWriteToFile(OCStringRef str, const char *path, OCStringRef *err) {
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        if (err) {
            *err = OCStringCreateWithFormat(
                STR("Unable to open \"%s\" for writing: %s"),
                path, strerror(errno));
        }
        return false;
    }
    const char *utf8 = OCStringGetCString(str);
    if (!utf8) {
        if (err) *err = STR("Failed to get UTF-8 C string from OCString");
        fclose(fp);
        return false;
    }
    size_t len = strlen(utf8);
    if (fwrite(utf8, 1, len, fp) != len) {
        if (err) {
            *err = OCStringCreateWithFormat(
                STR("Write error writing to \"%s\": %s"),
                path, strerror(errno));
        }
        fclose(fp);
        return false;
    }
    fclose(fp);
    return true;
}
//-----------------------------------------------------------------------------
// MARK: OCDataCreateWithContentsOfFile
//-----------------------------------------------------------------------------
OCDataRef OCDataCreateWithContentsOfFile(const char *path,
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
bool OCTypeWriteJSONToFile(OCTypeRef obj,
                           bool typed,
                           bool formatted,
                           const char *path,
                           OCStringRef *error) {
    if (error) *error = NULL;
    if (!obj || !path) {
        if (error) *error = STR("object or path was NULL");
        return false;
    }
    // 1) build the cJSON tree using schema-conformant method
    OCStringRef jsonError = NULL;
    cJSON *root = OCTypeCopyJSON(obj, typed, &jsonError);
    if (!root) {
        if (error) *error = jsonError ? jsonError : STR("failed to build JSON");
        return false;
    }
    // 2) render as compact or formatted JSON
    char *utf8 = formatted ? cJSON_Print(root) : cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!utf8) {
        if (error) *error = STR("failed to serialize JSON");
        return false;
    }
    // 3) write it out
    OCStringRef s = OCStringCreateWithCString(utf8);
    free(utf8);
    bool ok = OCStringWriteToFile(s, path, error);
    OCRelease(s);
    return ok;
}
// Read entire file into a malloc'd buffer (caller must free)
static char *_readFile(const char *path, OCStringRef *err) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        if (err) *err = OCStringCreateWithFormat(STR("Unable to open \"%s\": %s"),
                                                 path, strerror(errno));
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long len = ftell(f);
    rewind(f);
    char *buf = malloc((size_t)len + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    if (fread(buf, 1, (size_t)len, f) != (size_t)len) {
        free(buf);
        fclose(f);
        return NULL;
    }
    buf[len] = '\0';
    fclose(f);
    return buf;
}
