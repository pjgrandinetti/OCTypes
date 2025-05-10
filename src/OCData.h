//
//  OCData.h
//  OCTypes
//
//  Created by philip on 6/28/17.
//

#ifndef OCData_h
#define OCData_h

#include "OCLibrary.h"

typedef const struct __OCData * OCDataRef;
typedef struct __OCData * OCMutableDataRef;

/**
 * Returns the unique type identifier for OCData.
 * @return The OCTypeID for OCData.
 */
OCTypeID OCDataGetTypeID(void);

/**
 * Creates an immutable data object with specified bytes.
 * @param bytes Pointer to the data bytes.
 * @param length Length of the data in bytes.
 * @return A new OCDataRef containing a copy of the bytes.
 */
OCDataRef OCDataCreate(const uint8_t *bytes, uint64_t length);

/**
 * Creates an immutable data object that references existing bytes without copying.
 * @param bytes Pointer to the data bytes (must remain valid).
 * @param length Length of the data in bytes.
 * @return A new OCDataRef referencing the provided bytes.
 */
OCDataRef OCDataCreateWithBytesNoCopy(const uint8_t *bytes, uint64_t length);

/**
 * Creates a copy of an existing immutable data object.
 * @param theData The OCDataRef to copy.
 * @return A new OCDataRef copy of theData.
 */
OCDataRef OCDataCreateCopy(OCDataRef theData);

/**
 * Creates a mutable data object with given initial capacity.
 * @param capacity Initial capacity in bytes.
 * @return A new OCMutableDataRef with specified capacity.
 */
OCMutableDataRef OCDataCreateMutable(uint64_t capacity);

/**
 * Creates a mutable copy of an existing data object.
 * @param capacity Initial capacity in bytes for the new object.
 * @param theData The OCDataRef to copy.
 * @return A new OCMutableDataRef initialized with theData.
 */
OCMutableDataRef OCDataCreateMutableCopy(uint64_t capacity, OCDataRef theData);

/**
 * Returns the length of the data object.
 * @param theData The OCDataRef to query.
 * @return The length in bytes.
 */
uint64_t OCDataGetLength(OCDataRef theData);

/**
 * Returns a const pointer to the data bytes.
 * @param theData The OCDataRef to query.
 * @return Pointer to read-only bytes.
 */
const uint8_t *OCDataGetBytePtr(OCDataRef theData);

/**
 * Returns a mutable pointer to the data bytes.
 * @param theData The OCMutableDataRef to query.
 * @return Pointer to mutable bytes.
 */
uint8_t *OCDataGetMutableBytePtr(OCMutableDataRef theData);

/**
 * Copies a range of bytes from the data object into a buffer.
 * @param theData The OCDataRef to read from.
 * @param range The range of bytes to copy (location and length).
 * @param buffer Pointer to a buffer large enough to hold the bytes.
 */
void OCDataGetBytes(OCDataRef theData, OCRange range, uint8_t *buffer);

/**
 * Sets the length of a mutable data object.
 * @param theData The OCMutableDataRef to modify.
 * @param length The new length in bytes.
 */
void OCDataSetLength(OCMutableDataRef theData, uint64_t length);

/**
 * Increases the length of a mutable data object by extraLength bytes.
 * @param theData The OCMutableDataRef to modify.
 * @param extraLength Number of additional bytes to add.
 */
void OCDataIncreaseLength(OCMutableDataRef theData, uint64_t extraLength);

/**
 * Appends bytes to the end of a mutable data object.
 * @param theData The OCMutableDataRef to modify.
 * @param bytes Pointer to the bytes to append.
 * @param length Number of bytes to append.
 */
void OCDataAppendBytes(OCMutableDataRef theData, const uint8_t *bytes, uint64_t length);

#endif /* OCData_h */
