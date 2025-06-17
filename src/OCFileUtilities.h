// OCFileUtilities.h
#ifndef OCFILEUTILS_H
#define OCFILEUTILS_H

#include <stdbool.h>
#include "OCLibrary.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup OCFileUtilities OCFileUtilities
 * @brief File, path, and JSON (de)serialization helpers
 * @{
 */

/** @name Path utilities @{ */

/**
 * @brief Join two path components with the platform’s separator.
 * @param a  First path component.
 * @param b  Second path component.
 * @return   New OCStringRef of “a/b” (ownership transferred to caller).
 * @ingroup OCFileUtilities
 */
OCStringRef OCPathJoin(OCStringRef a, OCStringRef b);

/**
 * @brief Return the parent directory of a path.
 * @param path  The full path.
 * @return      New OCStringRef of the directory portion (ownership transferred).
 * @ingroup OCFileUtilities
 */
OCStringRef OCPathDirname(OCStringRef path);

/**
 * @brief Return the final component of a path.
 * @param path  The full path.
 * @return      New OCStringRef of the basename (ownership transferred).
 * @ingroup OCFileUtilities
 */
OCStringRef OCPathBasename(OCStringRef path);

/**
 * @brief Extract the “.ext” suffix from a path.
 * @param path  The full path.
 * @return      New OCStringRef of the extension (including ‘.’), or empty
 *              string if none (ownership transferred).
 * @ingroup OCFileUtilities
 */
OCStringRef OCPathExtension(OCStringRef path);

/**
 * @brief Replace the existing extension on a path.
 * @param path    The original path.
 * @param newExt  Extension to use (with or without leading ‘.’).
 * @return        New OCStringRef of the modified path (ownership transferred).
 * @ingroup OCFileUtilities
 */
OCStringRef OCPathByReplacingExtension(OCStringRef path, OCStringRef newExt);

/** @} */

/** @name Filesystem checks & manipulation @{ */

/**
 * @brief Does the given path exist?
 * @param path  Filesystem path.
 * @return      true if stat(2) succeeds.
 * @ingroup OCFileUtilities
 */
bool OCFileExists(const char *path);

/**
 * @brief Is the given path a directory?
 * @param path  Filesystem path.
 * @return      true if stat(2) shows S_ISDIR.
 * @ingroup OCFileUtilities
 */
bool OCIsDirectory(const char *path);

/**
 * @brief Is the given path a regular file?
 * @param path  Filesystem path.
 * @return      true if stat(2) shows S_ISREG.
 * @ingroup OCFileUtilities
 */
bool OCIsRegularFile(const char *path);

/**
 * @brief Create a directory, optionally recursing like “mkdir -p”.
 * @param path       Directory to create.
 * @param recursive  If true, build out all parent components.
 * @param err        On failure, *err is set to a human‐readable message.
 * @return           true on success.
 * @ingroup OCFileUtilities
 */
bool OCCreateDirectory(const char *path, bool recursive, OCStringRef *err);

/**
 * @brief List every regular file under a folder.
 * @param path       Base directory.
 * @param recursive  If true, descend into subdirectories.
 * @param err        On failure, *err is set to a human‐readable message.
 * @return           New OCArrayRef of OCStringRef relative paths,
 *                   or NULL on error (ownership transferred).
 * @ingroup OCFileUtilities
 */
OCArrayRef OCListDirectory(const char *path, bool recursive, OCStringRef *err);

/**
 * @brief Remove a file or empty directory.
 * @param path  Path to delete.
 * @param err   On failure, *err is set to a human‐readable message.
 * @return      true on success (or if path didn’t exist).
 * @ingroup OCFileUtilities
 */
bool OCRemoveItem(const char *path, OCStringRef *err);

/**
 * @brief Rename or move a file or directory.
 * @param oldPath  Existing path.
 * @param newPath  New path.
 * @param err      On failure, *err is set to a human‐readable message.
 * @return         true on success.
 * @ingroup OCFileUtilities
 */
bool OCRenameItem(const char *oldPath, const char *newPath, OCStringRef *err);

/** @} */

/** @name Text & data I/O @{ */

/**
 * @brief Read a UTF-8 text file into an OCString.
 * @param path  File path.
 * @param err   On failure, *err is set to a human‐readable message.
 * @return      New OCStringRef of file contents (ownership transferred), or NULL.
 * @ingroup OCFileUtilities
 */
OCStringRef OCStringCreateWithContentsOfFile(const char *path, OCStringRef *err);

/**
 * @brief Write an OCString (UTF-8) to a file.
 * @param str   The string to write.
 * @param path  Destination filename.
 * @param err   On failure, *err is set to a human‐readable message.
 * @return      true on success.
 * @ingroup OCFileUtilities
 */
bool OCStringWriteToFile(OCStringRef str, const char *path, OCStringRef *err);

/**
 * @brief Read a binary file into an OCDataRef.
 * @param path         Filesystem path.
 * @param err          On failure, *err is set to a human‐readable message.
 * @return             New OCDataRef with file bytes (ownership transferred), or NULL.
 * @ingroup OCFileUtilities
 */
OCDataRef OCDataCreateWithContentsOfFile(const char *path, OCStringRef *err);

/**
 * @brief Load all files under a folder (up to maxDepth) into a dictionary.
 * @param folderPath   Base directory.
 * @param maxDepth     Levels of recursion (0 = just top-level).
 * @param err          On failure, *err is set to a human‐readable message.
 * @return             New OCDictionaryRef ⟨relative-path→OCDataRef⟩,
 *                     or NULL on failure (ownership transferred).
 * @ingroup OCFileUtilities
 */
OCDictionaryRef OCDictionaryCreateWithContentsOfFolder(const char *folderPath,
                                                       int maxDepth,
                                                       OCStringRef *err);

/** @} */

/** @name JSON (de)serialization @{ */

/**
 * @brief Write any OCTypes object (string, number, bool, array, dict…) to a
 *        compact JSON file.
 * @param obj   An OCTypeRef (OCString, OCNumber, OCBoolean, OCArray, OCDictionary…)
 * @param path  Path to write.
 * @param err   On failure, *err is set to a human-readable message.
 * @return      true on success.
 * @ingroup OCFileUtilities
 */
bool OCTypeWriteJSONToFile(OCTypeRef obj, const char *path, OCStringRef *err);

/**
 * @brief Read a JSON file back into an OCTypes graph (OCString, OCNumber,
 *        OCBoolean, OCArray, OCDictionary…).
 * @param path  Path of the JSON file.
 * @param err   On parse or I/O failure, *err is set to a message.
 * @return      New OCTypeRef root of the graph (ownership transferred), or NULL.
 * @ingroup OCFileUtilities
 */
OCTypeRef OCTypeCreateFromJSONFile(const char *path, OCStringRef *err);

/** @} */

/** @} */  // end group OCFileUtilities

#ifdef __cplusplus
}
#endif

#endif // OCFILEUTILS_H