/**
 * @file OCTypes.h
 * @brief Main convenience header for the OCTypes library.
 *
 * This header provides a "kitchen sink" include for end users and complex
 * implementations that need access to multiple OCTypes modules. It includes
 * the foundation layer (OCType.h) followed by all module headers in the
 * correct dependency order.
 *
 * Individual modules should include only OCType.h and their specific
 * dependencies. This header is intended for .c implementation files
 * and end-user code that needs multiple OCTypes APIs.
 */
#ifndef OCTypes_h
#define OCTypes_h
// Define __private_extern__ for compatibility if not already defined
#ifndef __private_extern__
#ifdef __APPLE__
#define __private_extern__ __attribute__((__visibility__("hidden")))
#else
#define __private_extern__  // Define as empty for other platforms
#endif
#endif
// Foundation layer first - provides all core types and forward declarations
#include "OCType.h"
// Third-party dependencies
#include "cJSON.h"
// OCTypes module headers in dependency order
#include "OCArray.h"
#include "OCAutoreleasePool.h"
#include "OCBoolean.h"
#include "OCData.h"
#include "OCDictionary.h"
#include "OCFileUtilities.h"
#include "OCIndexArray.h"
#include "OCIndexPairSet.h"
#include "OCIndexSet.h"
#include "OCLeakTracker.h"
#include "OCMath.h"
#include "OCNull.h"
#include "OCNumber.h"
#include "OCSet.h"
#include "OCString.h"
// Additional convenience definitions can be added here if needed
#endif /* OCTypes_h */
