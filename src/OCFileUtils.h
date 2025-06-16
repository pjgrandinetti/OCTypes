#ifndef OCFILEUTILS_H
#define OCFILEUTILS_H

#include "OCLibrary.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reads an entire file into a new OCDataRef.
 *
 * Attempts to open and read the file at the given path, returning its
 * raw bytes. If the operation fails (e.g. file not found or read error),
 * returns NULL and, if `errorString` is non-NULL, sets *errorString to
 * a newly created OCStringRef describing the failure (ownership transferred
 * to caller). Pass NULL to ignore error details.
 *
 * @param path         Filesystem path to read.
 * @param errorString  Optional pointer to an OCStringRef; on failure, *errorString
 *                     will be set to an explanatory message (ownership transferred
 *                     to caller). Pass NULL to skip error reporting.
 * @return             An OCDataRef containing the file’s contents on success
 *                     (ownership transferred to caller), or NULL on failure.
 * @ingroup          OCData
 */
OCDataRef
OCDataCreateWithContentsOfFile(const char   *path,
                               OCStringRef *errorString);


/**
 * @brief Recursively loads every file in folderPath (and up to maxDepth levels of subfolders)
 *        into an OCDictionary whose keys are the files’ relative paths.
 *
 * @param folderPath   The absolute or relative path to a directory.
 * @param maxDepth     How many levels of subdirectories to descend (0 = just this folder).
 * @param errorMessage On error, will be set to a human-readable string (ownership transferred;
 *                     caller must OCRelease() it).  May be NULL if you don’t care.
 * @return             A new OCDictionaryRef mapping relative-path→OCDataRef on success
 *                     (caller must OCRelease), or NULL on failure (errorMessage populated
 *                     if non-NULL).
 * @ingroup          OCDictionary
 */
OCDictionaryRef
OCDictionaryCreateWithContentsOfFolder(const char    *folderPath,
                                           int            maxDepth,
                                           OCStringRef   *errorMessage);

#ifdef __cplusplus
}
#endif

#endif // OCFILEUTILS_H
