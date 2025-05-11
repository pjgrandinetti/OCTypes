//
//  OCAutoreleasePool.h
//
//  Created by Philip on 12/27/09.
//  Updated by GitHub Copilot on May 10, 2025.
//

/**
 * @file OCAutoreleasePool.h
 * @brief Implements an autorelease pool for OCTypes.
*
 * OCAutoreleasePool provides a mechanism for managing the memory of OCType
 * objects that follow a reference-count pattern using delayed deallocation.
 *
 * Create Rule:
 *   Functions whose names contain "Create" return a +1 retained object.
 *   The caller must balance each Create call with a Release.
 *
 * Usage example:
 * @code{.c}
 *   OCAutoreleasePoolRef pool = OCAutoreleasePoolCreate();
 *   OCAutorelease(object);
 *   OCAutoreleasePoolRelease(pool);
 * @endcode
 *
 * Nesting Pools:
 *   Pools can be nested. Releasing a nested pool only drains that pool.
 */

#ifndef OCAutoreleasePool_h
#define OCAutoreleasePool_h

#include <stdbool.h>
#include "OCLibrary.h" // Ensures OCTypeRef and other necessary types are available.

/** @defgroup OCAutoreleasePool OCAutoreleasePool
 *  @brief Implements an autorelease pool for OCTypes.
 *  @{
 */

/**
 * @brief A reference to an opaque autorelease pool.
 * @ingroup OCAutoreleasePool
 */
typedef struct _OCAutoreleasePool *OCAutoreleasePoolRef;

/**
 * @brief Creates a new autorelease pool. The caller must release it when done.
 * @return A reference to the newly created autorelease pool.
 * @ingroup OCAutoreleasePool
 */
OCAutoreleasePoolRef OCAutoreleasePoolCreate(void);

/**
 * @brief Releases an autorelease pool and all objects it contains.
 * @param pool The autorelease pool to release. Must not be NULL.
 *        Releasing a non-topmost pool also drains nested pools.
 * @return true if the pool was successfully released, false otherwise.
 * @ingroup OCAutoreleasePool
 */
bool OCAutoreleasePoolRelease(OCAutoreleasePoolRef pool);

/**
 * @brief Adds an object to the current (topmost) autorelease pool.
 * @param ptr Pointer to the OCType object to autorelease. Must be non-NULL.
 * @return The same pointer passed in, allowing for chained calls.
 * @ingroup OCAutoreleasePool
 */
const void *OCAutorelease(const void *ptr);

/**
 * @brief Drains an autorelease pool, releasing all objects it contains without deallocating the pool itself.
 * @param pool The autorelease pool to drain. Must not be NULL.
 * @ingroup OCAutoreleasePool
 */
void OCAutoreleasePoolDrain(OCAutoreleasePoolRef pool);

/** @} */ // end of OCAutoreleasePool group

#endif /* OCAutoreleasePool_h */