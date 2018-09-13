//
//  OCData.h
//  OCTypes
//
//  Created by philip on 6/28/17.
//  Copyright © 2017 PhySy Ltd. All rights reserved.
//

#ifndef OCData_h
#define OCData_h

#include "OCLibrary.h"

typedef const struct __OCData * OCDataRef;
typedef struct __OCData * OCMutableDataRef;

OCTypeID OCDataGetTypeID(void);

OCDataRef OCDataCreate(const uint8_t *bytes, uint64_t length);

OCDataRef OCDataCreateWithBytesNoCopy(const uint8_t *bytes, uint64_t length);

OCDataRef OCDataCreateCopy(OCDataRef theData);

OCMutableDataRef OCDataCreateMutable(uint64_t capacity);

OCMutableDataRef OCDataCreateMutableCopy(uint64_t capacity, OCDataRef theData);

uint64_t OCDataGetLength(OCDataRef theData);

const uint8_t *OCDataGetBytePtr(OCDataRef theData);

uint8_t *OCDataGetMutableBytePtr(OCMutableDataRef theData);

void OCDataGetBytes(OCDataRef theData, OCRange range, uint8_t *buffer);

void OCDataSetLength(OCMutableDataRef theData, uint64_t length);

void OCDataIncreaseLength(OCMutableDataRef theData, uint64_t extraLength);

void OCDataAppendBytes(OCMutableDataRef theData, const uint8_t *bytes, uint64_t length);


#endif /* OCData_h */
