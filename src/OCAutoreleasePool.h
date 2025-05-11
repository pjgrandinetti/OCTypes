//
//  OCAutoreleasePool.h
//
//  Created by Philip on 12/27/09.
//  Updated by GitHub Copilot on May 10, 2025.
//

#include <stdbool.h> // For bool type

/**
 * @file OCAutoreleasePool.h
 * @author Philip (Original Author)
 * @author GitHub Copilot (Documentation Editor)
 * @date May 10, 2025
 * @brief Implements an autorelease pool for OCTypes.
 *
 * OCAutoreleasePool provides a mechanism for managing the memory of OCType objects
 * that follow a reference count pattern. This allows for delayed deallocation of
 * objects, simplifying memory management in certain programming patterns.
 *
 * @par Create Rule:
 * Functions whose names contain "Create" follow the convention:
 * they return a +1 retained object. The caller owns these objects and
 * must balance each Create call by invoking the appropriate Release
 * function when the object is no longer needed.
 *
 * @section usage_sec Usage
 * To use an autorelease pool:
 * 1. Create a pool:
 * @code
 * OCAutoreleasePoolRef thePool = OCAutoreleasePoolCreate();
 * @endcode
 * 2. Create, retain, and then autorelease objects. Objects added to the pool
 *    will be released when the pool itself is released.
 * @code
 * // Example: OCStringRef myString = OCStringCreateWithCString("hello");
 * // OCAutorelease(myString); // Assuming myString is an OCTypeRef compatible object
 * @endcode
 * 3. Release the pool. This action releases all objects contained within it.
 * @code
 * OCAutoreleasePoolRelease(thePool);
 * @endcode
 *
 * @par Nesting Pools:
 * Autorelease pools can be nested. When a nested pool is released, it releases
 * its objects.
 *
 * @par Context of Release:
 * OCAutoreleasePoolRelease should typically be called in the same scope or context
 * (e.g., function, method, or loop iteration) where OCAutoreleasePoolCreate was called.
 * Releasing a pool that is not at the top of the stack will cause all pools above it
 * to be released as well, along with all their objects.
 *
 * @par Autoreleasing OCTypes:
 * OCType-compatible objects can be added to the current autorelease pool using the
 * OCAutorelease() function:
 * @code
 * // OCTypeRef anOCTypeObject = ... ;
 * // OCAutorelease(anOCTypeObject);
 * @endcode
 *
 * @par Custom Types:
 * For custom C types to integrate with this autorelease mechanism, they typically
 * need an autorelease method. This method would internally use a mechanism like
 * `OCAutoreleasePoolAddObject(OCTypeRef object, void (*release_func)(OCTypeRef object))`
 * (note: `OCAutoreleasePoolAddObject` is a conceptual example of how a manager might handle this;
 * its exact name and signature might vary if it's an internal or related API not defined in this header).
 * The first argument is the object to be autoreleased, and the second is its specific release function.
 * This adds the object to the most recently created autorelease pool.
 * An autorelease pool must exist before objects can be added to it.
 *
 * @copyright Copyright (c) 2009-2025 PhySy Ltd. All rights reserved.
 */

/**
 * @typedef OCAutoreleasePoolRef
 * @brief A reference to an opaque OCAutoreleasePool structure.
 *
 * This type represents a handle to an autorelease pool instance.
 */
typedef struct _OCAutoreleasePool *OCAutoreleasePoolRef;

/**
 * @function OCAutoreleasePoolCreate
 * @brief Creates a new autorelease pool.
 * @return A reference to the newly created OCAutoreleasePool.
 *         The caller is responsible for releasing this pool using
 *         OCAutoreleasePoolRelease() when it is no longer needed.
 */
OCAutoreleasePoolRef OCAutoreleasePoolCreate(void);

/**
 * @function OCAutoreleasePoolRelease
 * @brief Releases an OCAutoreleasePool object and all objects it contains.
 *
 * When an autorelease pool is released, it sends a release message to all
 * objects that were added to it.
 *
 * @param pool The OCAutoreleasePoolRef to be released. Must not be NULL.
 *             If `pool` is not the most recently created (top-most) pool,
 *             releasing it will also release all pools created after it.
 * @return true if the pool was successfully released, false otherwise (e.g., if `pool` is NULL or invalid).
 */
bool OCAutoreleasePoolRelease(OCAutoreleasePoolRef pool);

/**
 * @function OCAutorelease
 * @brief Adds an object to the current (top-most) autorelease pool.
 *
 * The object will receive a release message when the autorelease pool itself
 * is released. The object must be a valid OCType-compatible pointer.
 *
 * @param ptr A pointer to the OCType object to be autoreleased.
 * @return The same pointer `ptr` that was passed in, allowing for chained calls.
 * @warning If `ptr` is NULL, this may lead to issues or a crash when the
 *          autorelease pool is deallocated, depending on the underlying
 *          implementation of how objects are managed and released. It is
 *          best practice to ensure `ptr` is non-NULL.
 */
const void * OCAutorelease(const void * ptr);