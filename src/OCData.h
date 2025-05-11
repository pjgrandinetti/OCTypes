//
//  OCData.h
//  OCTypes
//
//  Created by philip on 6/28/17.
//

#ifndef OCData_h
#define OCData_h

#include "OCLibrary.h"

/**
 * @typedef OCDataRef
 * A reference to an immutable data object.
 */
typedef const struct __OCData * OCDataRef;

/**
 * @typedef OCMutableDataRef
 * A reference to a mutable data object.
 */
typedef struct __OCData * OCMutableDataRef;

/**
 * @brief Returns the unique type identifier for OCData objects.
 * @return The OCTypeID associated with the OCData type.
 * @see OCType
 */
OCTypeID OCDataGetTypeID(void);

/**
 * @brief Creates an immutable data object by copying the provided bytes.
 * @param bytes A pointer to the raw byte buffer to copy.
 *              If NULL and length is non-zero, the behavior is undefined.
 *              If NULL and length is zero, an empty OCData object is created.
 * @param length The number of bytes to copy from the `bytes` buffer.
 * @return A new OCDataRef containing a copy of the specified bytes,
 *         or NULL if memory allocation fails.
 *         The returned object should be released by the caller using OCRelease().
 * @see OCDataCreateWithBytesNoCopy
 * @see OCDataCreateCopy
 */
OCDataRef OCDataCreate(const uint8_t *bytes, uint64_t length);

/**
 * @brief Creates an immutable data object that references existing bytes without copying them.
 * @details The new OCData object will directly use the provided `bytes` pointer.
 *          The caller is responsible for ensuring that the `bytes` buffer remains valid
 *          for the lifetime of this OCData object. Modifying the content of `bytes`
 *          after creating the OCData object will affect the OCData object's content.
 * @param bytes A pointer to the raw byte buffer. The data is not copied.
 *              If NULL and length is non-zero, the behavior is undefined.
 *              If NULL and length is zero, an empty OCData object is created (though it won't reference any specific memory).
 * @param length The number of bytes in the `bytes` buffer.
 * @return A new OCDataRef that directly references the provided bytes,
 *         or NULL if memory allocation for the OCData structure itself fails.
 *         The returned object should be released by the caller using OCRelease().
 * @warning The `bytes` buffer must not be deallocated or modified while the
 *          returned OCDataRef is in use.
 * @see OCDataCreate
 */
OCDataRef OCDataCreateWithBytesNoCopy(const uint8_t *bytes, uint64_t length);

/**
 * @brief Creates a new immutable data object by copying an existing data object.
 * @param theData The OCDataRef to copy. Must not be NULL.
 * @return A new OCDataRef containing a copy of the data from `theData`,
 *         or NULL if `theData` is NULL or if memory allocation fails.
 *         The returned object should be released by the caller using OCRelease().
 */
OCDataRef OCDataCreateCopy(OCDataRef theData);

/**
 * @brief Creates a new mutable data object with a specified initial capacity.
 * @details The created mutable data object will be empty initially (length 0)
 *          but will have pre-allocated memory to hold at least `capacity` bytes.
 * @param capacity The initial storage capacity in bytes. Can be 0.
 * @return A new OCMutableDataRef with the specified initial capacity,
 *         or NULL if memory allocation fails.
 *         The returned object should be released by the caller using OCRelease().
 * @see OCDataCreateMutableCopy
 */
OCMutableDataRef OCDataCreateMutable(uint64_t capacity);

/**
 * @brief Creates a new mutable data object by copying an existing data object,
 *        with a specified initial capacity for the new mutable object.
 * @details The content of `theData` is copied into the new mutable data object.
 *          The new object's capacity will be at least `capacity` bytes, or
 *          the length of `theData`, whichever is greater.
 * @param capacity The minimum initial storage capacity in bytes for the new mutable object.
 *                 If this is less than the length of `theData`, the capacity will be
 *                 set to the length of `theData`.
 * @param theData The OCDataRef to copy. If NULL, a mutable data object with the
 *                specified capacity is created, but it will be empty.
 * @return A new OCMutableDataRef initialized with a copy of the data from `theData`
 *         and having at least the specified capacity, or NULL if memory allocation fails.
 *         The returned object should be released by the caller using OCRelease().
 */
OCMutableDataRef OCDataCreateMutableCopy(uint64_t capacity, OCDataRef theData);

/**
 * @brief Returns the number of bytes contained in the data object.
 * @param theData The OCDataRef or OCMutableDataRef to query. Must not be NULL.
 * @return The length of the data in bytes. Returns 0 if `theData` is NULL or empty.
 */
uint64_t OCDataGetLength(OCDataRef theData);

/**
 * @brief Returns a read-only pointer to the internal byte buffer of the data object.
 * @param theData The OCDataRef or OCMutableDataRef to query. Must not be NULL.
 * @return A const pointer to the data's bytes. The pointer is valid as long as
 *         `theData` is not deallocated (and, for OCMutableDataRef, not modified
 *         in a way that would reallocate its internal buffer).
 *         Returns NULL if `theData` is NULL or if the data object is empty and has no buffer.
 */
const uint8_t *OCDataGetBytePtr(OCDataRef theData);

/**
 * @brief Returns a mutable pointer to the internal byte buffer of a mutable data object.
 * @param theData The OCMutableDataRef to query. Must not be NULL.
 * @return A pointer to the data's bytes, allowing modification.
 *         The pointer is valid as long as `theData` is not deallocated and not
 *         modified in a way that would reallocate its internal buffer (e.g., by
 *         significantly increasing its length).
 *         Returns NULL if `theData` is NULL or if the data object is empty and has no buffer.
 * @warning Modifying the data through this pointer must be done carefully,
 *          especially ensuring not to write past the allocated capacity.
 *          Operations like OCDataSetLength or OCDataAppendBytes are generally safer
 *          for modifying the length or content.
 */
uint8_t *OCDataGetMutableBytePtr(OCMutableDataRef theData);

/**
 * @brief Copies a range of bytes from the data object into a provided buffer.
 * @param theData The OCDataRef or OCMutableDataRef to read from. Must not be NULL.
 * @param range An OCRange structure specifying the starting byte index and the
 *              number of bytes to copy. The range must be within the bounds of `theData`.
 *              If `range.location + range.length` exceeds the length of `theData`,
 *              only bytes up to the end of `theData` are copied.
 *              If `range.location` is beyond the end of `theData`, no bytes are copied.
 * @param buffer A pointer to a memory buffer where the copied bytes will be stored.
 *               This buffer must be large enough to hold `range.length` bytes.
 *               Must not be NULL if `range.length` is greater than 0.
 * @note If `theData` is NULL, or `buffer` is NULL (and length > 0), or the range is invalid,
 *       the behavior might be undefined or result in no operation.
 */
void OCDataGetBytes(OCDataRef theData, OCRange range, uint8_t *buffer);

/**
 * @brief Sets the length of a mutable data object.
 * @details If the new `length` is greater than the current length, the data object's
 *          content up to the current length is preserved, and the new bytes at the end
 *          are uninitialized (their content is undefined). The capacity may be increased.
 *          If the new `length` is less than the current length, the data is truncated.
 *          The capacity is not necessarily reduced.
 * @param theData The OCMutableDataRef to modify. Must not be NULL.
 * @param length The new length in bytes.
 * @note If `theData` is NULL, this function has no effect.
 *       If memory reallocation is needed and fails, the object's length might
 *       not be changed, or the program might terminate.
 */
void OCDataSetLength(OCMutableDataRef theData, uint64_t length);

/**
 * @brief Increases the length of a mutable data object by a specified number of bytes.
 * @details This is a convenience function. The new bytes added at the end of the
 *          data object are uninitialized. The capacity of the data object may be
 *          increased to accommodate the new length.
 * @param theData The OCMutableDataRef to modify. Must not be NULL.
 * @param extraLength The number of bytes by which to increase the length.
 * @note If `theData` is NULL, this function has no effect.
 *       If `extraLength` is 0, the function has no effect.
 *       If memory reallocation is needed and fails, the object's length might
 *       not be changed, or the program might terminate.
 * @see OCDataSetLength
 */
void OCDataIncreaseLength(OCMutableDataRef theData, uint64_t extraLength);

/**
 * @brief Appends a sequence of bytes to the end of a mutable data object.
 * @details The length of `theData` is increased by `length`, and its capacity
 *          may be increased if necessary. The content of the `bytes` buffer
 *          is copied to the end of `theData`.
 * @param theData The OCMutableDataRef to modify. Must not be NULL.
 * @param bytes A pointer to the byte buffer to append. Must not be NULL if `length` > 0.
 * @param length The number of bytes to append from the `bytes` buffer.
 * @note If `theData` is NULL, this function has no effect.
 *       If `bytes` is NULL and `length` > 0, behavior is undefined.
 *       If `length` is 0, the function has no effect.
 *       If memory reallocation is needed and fails, the object's content might
 *       not be changed, or the program might terminate.
 */
void OCDataAppendBytes(OCMutableDataRef theData, const uint8_t *bytes, uint64_t length);

#endif /* OCData_h */
