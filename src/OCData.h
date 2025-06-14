/**
 * @file OCData.h
 * @brief Declares the OCData and OCMutableData interfaces for handling binary data.
 *
 * OCData provides immutable and mutable representations of raw byte sequences,
 * with memory safety and efficient operations.
 *
 * @note Ownership follows CoreFoundation conventions:
 *       The caller owns returned OCDataRef or OCMutableDataRef from functions
 *       with "Create" or "Copy" in the name and must call OCRelease().
 */

#ifndef OCData_h
#define OCData_h

#include "OCLibrary.h" // Ensures OCDataRef and other types are available

/**
 * @defgroup OCData OCData
 * @brief Binary data buffer types and utilities.
 * @{ 
 */

/**
 * @brief Returns the unique type identifier for OCData objects.
 * @return OCTypeID for OCData.
 * @ingroup OCData
 */
OCTypeID OCDataGetTypeID(void);

/**
 * @brief Creates a new immutable data object by copying bytes.
 *
 * @param bytes Pointer to the source byte buffer.
 * @param length Number of bytes to copy.
 * @return New OCDataRef, or NULL on failure.
 * @ingroup OCData
 */
OCDataRef OCDataCreate(const uint8_t *bytes, uint64_t length);

/**
 * @brief Creates an immutable data object referencing existing memory (no copy).
 *
 * @param bytes Pointer to the byte buffer.
 * @param length Length of the byte buffer.
 * @return New OCDataRef, or NULL on failure.
 * @warning Caller must ensure that `bytes` remains valid and unmodified.
 * @ingroup OCData
 */
OCDataRef OCDataCreateWithBytesNoCopy(const uint8_t *bytes, uint64_t length);

/**
 * @brief Creates a new OCDataRef by copying an existing one.
 *
 * @param theData Source OCDataRef to copy.
 * @return Copy of the data, or NULL on failure.
 * @ingroup OCData
 */
OCDataRef OCDataCreateCopy(OCDataRef theData);

/**
 * @brief Creates a new mutable data object with a specified capacity.
 *
 * @param capacity Initial capacity in bytes.
 * @return New OCMutableDataRef, or NULL on failure.
 * @ingroup OCData
 */
OCMutableDataRef OCDataCreateMutable(uint64_t capacity);

/**
 * @brief Creates a mutable copy of existing data with a specified capacity.
 *
 * @param capacity Minimum capacity of the new object.
 * @param theData Source data to copy (can be NULL).
 * @return New OCMutableDataRef.
 * @ingroup OCData
 */
OCMutableDataRef OCDataCreateMutableCopy(uint64_t capacity, OCDataRef theData);

/**
 * @brief Gets the length of a data object.
 *
 * @param theData OCDataRef or OCMutableDataRef.
 * @return Length in bytes.
 * @ingroup OCData
 */
uint64_t OCDataGetLength(OCDataRef theData);

/**
 * @brief Returns a read-only pointer to internal bytes.
 *
 * @param theData OCDataRef or OCMutableDataRef.
 * @return Pointer to bytes, or NULL.
 * @ingroup OCData
 */
const uint8_t *OCDataGetBytesPtr(OCDataRef theData);

/**
 * @brief Returns a mutable pointer to internal bytes.
 *
 * @param theData Mutable data object.
 * @return Pointer to bytes, or NULL.
 * @warning Use with care. Modifications may affect internal state.
 * @ingroup OCData
 */
uint8_t *OCDataGetMutableBytesPtr(OCMutableDataRef theData);

/**
 * @brief Copies a range of bytes from an OCData object into a buffer.
 *
 * @param data The source OCData object.
 * @param range The byte range to copy.
 * @param buffer Destination buffer (must be large enough).
 * @return true if the copy succeeded; false if inputs are invalid or out of bounds.
 *
 * @ingroup OCData
 */
bool OCDataGetBytes(OCDataRef theData, OCRange range, uint8_t *buffer);

/**
 * @brief Sets the length of a mutable data object.
 *
 * If the new length is greater than the current length, the extra memory is zero-initialized.
 * If the new length exceeds the current capacity, the buffer is reallocated.
 *
 * @param theData Mutable data object to resize.
 * @param length The desired new length in bytes.
 * @return true if the operation succeeded; false if allocation failed or input was NULL.
 *
 * @ingroup OCData
 */
bool OCDataSetLength(OCMutableDataRef theData, uint64_t length);

/**
 * @brief Increases the length of a mutable data object by a given amount.
 *
 * @param data The mutable data object.
 * @param extraLength The number of bytes to add.
 * @return true if the resize was successful; false on failure or invalid input.
 *
 * @ingroup OCData
 */
bool OCDataIncreaseLength(OCMutableDataRef theData, uint64_t extraLength);

/**
 * @brief Appends bytes to the end of a mutable data object.
 *
 * Grows the data buffer as needed. If allocation fails, no changes are made.
 *
 * @param data The mutable data object.
 * @param bytes The bytes to append.
 * @param length The number of bytes to append.
 * @return true if the bytes were appended successfully; false on failure.
 *
 * @ingroup OCData
 */
bool OCDataAppendBytes(OCMutableDataRef theData, const uint8_t *bytes, uint64_t length);

/**
 * @brief Returns a human-readable string description of a data object.
 *
 * @param theData OCDataRef instance.
 * @return A formatted OCStringRef (caller must release).
 * @ingroup OCData
 */
OCStringRef OCDataCopyFormattingDesc(OCTypeRef cf);

/** @} */ // end of OCData group

#endif /* OCData_h */
