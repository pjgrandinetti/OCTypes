/**
 * @file OCData.h
 * @brief Declares the OCData and OCMutableData interfaces for handling binary data.
 *
 * OCData provides immutable and mutable representations of raw byte sequences,
 * with memory safety and efficient operations.
 */

#ifndef OCData_h
#define OCData_h

#include "OCLibrary.h"

/**
 * @defgroup OCData OCData
 * @brief Binary data buffer types and utilities.
 * @{
 */

/**
 * @brief A reference to an immutable data object.
 * @ingroup OCData
 */
typedef const struct __OCData *OCDataRef;

/**
 * @brief A reference to a mutable data object.
 * @ingroup OCData
 */
typedef struct __OCData *OCMutableDataRef;

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
const uint8_t *OCDataGetBytePtr(OCDataRef theData);

/**
 * @brief Returns a mutable pointer to internal bytes.
 *
 * @param theData Mutable data object.
 * @return Pointer to bytes, or NULL.
 * @warning Use with care. Modifications may affect internal state.
 * @ingroup OCData
 */
uint8_t *OCDataGetMutableBytePtr(OCMutableDataRef theData);

/**
 * @brief Copies a byte range from the data object.
 *
 * @param theData Source data.
 * @param range Range to copy.
 * @param buffer Destination buffer.
 * @ingroup OCData
 */
void OCDataGetBytes(OCDataRef theData, OCRange range, uint8_t *buffer);

/**
 * @brief Sets the length of a mutable data object.
 *
 * @param theData Mutable data.
 * @param length New length in bytes.
 * @ingroup OCData
 */
void OCDataSetLength(OCMutableDataRef theData, uint64_t length);

/**
 * @brief Increases the length of mutable data.
 *
 * @param theData Mutable data.
 * @param extraLength Bytes to add.
 * @ingroup OCData
 */
void OCDataIncreaseLength(OCMutableDataRef theData, uint64_t extraLength);

/**
 * @brief Appends raw bytes to the end of a mutable data object.
 *
 * @param theData Target mutable data.
 * @param bytes Pointer to source data.
 * @param length Number of bytes to append.
 * @ingroup OCData
 */
void OCDataAppendBytes(OCMutableDataRef theData, const uint8_t *bytes, uint64_t length);

/** @} */ // end of OCData group

#endif /* OCData_h */
