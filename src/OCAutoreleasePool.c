//
//  OCAutoreleasePool.c
//
//  Created by Philip on 12/27/09.
//

#include <stdlib.h> 
#include <stdio.h> 
#include "OCLibrary.h"  // OCLibrary

#ifndef FREE
#define FREE(X) {free(X); X=NULL;}
#endif

#ifndef IF_NO_OBJECT_EXISTS_RETURN
#define IF_NO_OBJECT_EXISTS_RETURN(OBJECT,X) if(OBJECT==NULL) {fprintf(stderr, "*** WARNING - %s %s - object doesn't exist.\n",__FILE__,__func__); return X;}
#endif

#define POOL_NOT_FOUND -1
#define POOLOBJECT_NOT_FOUND -1

struct impl_OCAutoreleasePoolObject {
    const void * object;
    void (*release)(const void *);
};

typedef struct impl_OCAutoreleasePoolObject * OCAutoreleasePoolObjectRef;


struct impl_OCAutoreleasePool
{
    OCAutoreleasePoolObjectRef *pool_objects;
    int number_of_pool_objects;
};

struct impl_OCAutoreleasePoolsManager
{
    OCAutoreleasePoolRef *pools;
    int number_of_pools;
};

typedef struct impl_OCAutoreleasePoolsManager *OCAutoreleasePoolsManagerRef;

// OCAutoreleasePoolsManager is a Singleton
static OCAutoreleasePoolsManagerRef autorelease_pool_manager = NULL;


/**************************************************************************
 OCAutoreleasePoolObject methods
 *************************************************************************/

/*
 @function OCAutoreleasePoolObjectCreate
 Creates a OCAutoreleasePoolObject.
 @param object The object wrapped into OCAutoreleasePoolObject.
 @param (*release)(OCTypeRef) release method for object.
 */
static OCAutoreleasePoolObjectRef OCAutoreleasePoolObjectCreate(const void * object, void (*release)(const void *))
{
    OCAutoreleasePoolObjectRef thePoolObject  = malloc(sizeof(struct impl_OCAutoreleasePoolObject));
    
    IF_NO_OBJECT_EXISTS_RETURN(thePoolObject,NULL);
    
    thePoolObject->object = object;
    thePoolObject->release = release;
    return thePoolObject;
}

/*
 @function OCAutoreleasePoolObjectDeallocate
 Deallocates a OCAutoreleasePoolObject.
 @param thePoolObject OCAutoreleasePoolObject to be deallocated.
 @result YES (1) if successful, NO (0) if unsuccessful
 */
static bool OCAutoreleasePoolObjectDeallocate(OCAutoreleasePoolObjectRef thePoolObject)
{
    IF_NO_OBJECT_EXISTS_RETURN(thePoolObject,false);
    
    if(thePoolObject) {
        free(thePoolObject);
        thePoolObject=NULL;
    }
    return true;
}

/*
 @function OCAutoreleasePoolObjectGetReleaseFunction
 Returns the release function for object wrapped in OCAutoreleasePoolObject.
 @param thePoolObject OCAutoreleasePoolObject with object to be released.
 @result release function
 */
static void *OCAutoreleasePoolObjectGetReleaseFunction(OCAutoreleasePoolObjectRef thePoolObject)
{
    IF_NO_OBJECT_EXISTS_RETURN(thePoolObject,NULL);
    
    return thePoolObject->release;
}

/*
 @function OCAutoreleasePoolObjectGetObject
 Returns the release function for object wrapped in OCAutoreleasePoolObject.
 @param thePoolObject OCAutoreleasePoolObject with object to be released.
 @result release function
 */
static const void * OCAutoreleasePoolObjectGetObject(OCAutoreleasePoolObjectRef thePoolObject)
{
    IF_NO_OBJECT_EXISTS_RETURN(thePoolObject,NULL);
    
    return thePoolObject->object;
}



/**************************************************************************
 OCAutoreleasePoolsManager methods
 *************************************************************************/
static bool OCAutoreleasePoolDeallocate(OCAutoreleasePoolRef thePool);
static bool OCAutoreleasePoolAddObject(OCAutoreleasePoolRef thePool, const void * object, void (*release)(const void *));


static OCAutoreleasePoolsManagerRef OCAutoreleasePoolsManagerCreate(void)
{
    OCAutoreleasePoolsManagerRef thePoolsManager  = malloc(sizeof(struct impl_OCAutoreleasePoolsManager));
    IF_NO_OBJECT_EXISTS_RETURN(thePoolsManager,NULL);
    
    thePoolsManager->pools = NULL;
    thePoolsManager->number_of_pools = 0;
    return thePoolsManager;
}

static int OCAutoreleasePoolsManagerIndexOfPool(OCAutoreleasePoolRef thePool)
{
    IF_NO_OBJECT_EXISTS_RETURN(autorelease_pool_manager,POOL_NOT_FOUND);
    IF_NO_OBJECT_EXISTS_RETURN(thePool,POOL_NOT_FOUND);
    
    int poolIndex = POOL_NOT_FOUND;
    for(int i=0;i<autorelease_pool_manager->number_of_pools;i++) {
        if(autorelease_pool_manager->pools[i] == thePool) {
            poolIndex = i;
            break;
        }
    }
    return poolIndex;
}

/*
 @function OCAutoreleasePoolsManagerRemovePool
 @param thePool The pool to be removed.
 @result YES (1) if successful, NO (0) if unsuccessful
 */
static bool OCAutoreleasePoolsManagerRemovePool(OCAutoreleasePoolRef thePool)
{
    IF_NO_OBJECT_EXISTS_RETURN(autorelease_pool_manager,false);
    IF_NO_OBJECT_EXISTS_RETURN(thePool,false);
    
    int poolIndex =  OCAutoreleasePoolsManagerIndexOfPool(thePool);
    if(poolIndex == POOL_NOT_FOUND) return false;
    
    // deallocate pool at poolIndex along with all pools with higher indexes
    for(int i=autorelease_pool_manager->number_of_pools-1;i>=poolIndex;i--)
        OCAutoreleasePoolDeallocate(autorelease_pool_manager->pools[i]);
    
    autorelease_pool_manager->number_of_pools = poolIndex;
    
    if(autorelease_pool_manager->number_of_pools == 0) {
        if (autorelease_pool_manager->pools != NULL) {
            free(autorelease_pool_manager->pools);
            autorelease_pool_manager->pools = NULL;
        }
    } else {
        // Try to shrink the allocation.
        OCAutoreleasePoolRef *new_pools_ptr = realloc(autorelease_pool_manager->pools, autorelease_pool_manager->number_of_pools * sizeof(OCAutoreleasePoolRef));
        if (new_pools_ptr == NULL) {
            // realloc failed to shrink. The original 'pools' pointer is still valid but too large.
            fprintf(stderr, "*** ERROR - %s %s - realloc failed to shrink pools array. Memory may be overallocated.\n", __FILE__, __func__);
            // Keep the old pointer: autorelease_pool_manager->pools remains unchanged.
        } else {
            autorelease_pool_manager->pools = new_pools_ptr;
        }
    }
    return true; // Return true if the pool was found and processed.
}

static bool OCAutoreleasePoolsManagerDeallocate(OCAutoreleasePoolsManagerRef thePoolsManager)
{
    IF_NO_OBJECT_EXISTS_RETURN(thePoolsManager,false);
    
    for(int i=0;i<thePoolsManager->number_of_pools;i++) OCAutoreleasePoolsManagerRemovePool(thePoolsManager->pools[i]);
    
    if(thePoolsManager->pools) {
        free(thePoolsManager->pools);
        thePoolsManager->pools=NULL;
    }
    
    if(thePoolsManager) {
        free(thePoolsManager);
        thePoolsManager=NULL;
    }
    return true;
}


/*
 @function OCAutoreleasePoolsManagerGetNumberOfPools
 @result number of autorelease pools
 */
static int OCAutoreleasePoolsManagerGetNumberOfPools(void)
{
    if(autorelease_pool_manager==NULL) return 0;
    
    return autorelease_pool_manager->number_of_pools;
}

/*
 @function OCAutoreleasePoolsManagerAddPool
 Adds the pool to the manager.
 @param thePool The pool to be added.
 */
static void OCAutoreleasePoolsManagerAddPool(OCAutoreleasePoolRef thePool)
{
    IF_NO_OBJECT_EXISTS_RETURN(thePool,);
    
    if(autorelease_pool_manager==NULL) autorelease_pool_manager = OCAutoreleasePoolsManagerCreate();
    if(autorelease_pool_manager) {
        autorelease_pool_manager->number_of_pools++;
        OCAutoreleasePoolRef *new_pools_ptr = realloc(autorelease_pool_manager->pools,
                                                  autorelease_pool_manager->number_of_pools
                                                  * sizeof(OCAutoreleasePoolRef)); // Use OCAutoreleasePoolRef
        if (new_pools_ptr == NULL && autorelease_pool_manager->number_of_pools > 0) { // Check if realloc failed (and it's not a 0-size alloc)
            fprintf(stderr, "*** ERROR - %s %s - realloc failed to grow pools array. Pool cannot be added.\n", __FILE__, __func__);
            autorelease_pool_manager->number_of_pools--; // Revert count
            // autorelease_pool_manager->pools remains unchanged (valid old pointer or NULL)
            return; // Exit without adding the pool
        }
        autorelease_pool_manager->pools = new_pools_ptr;
        if (autorelease_pool_manager->pools != NULL) { // Ensure pools is not NULL before assigning to its element
             autorelease_pool_manager->pools[autorelease_pool_manager->number_of_pools-1] = thePool;
        } else if (autorelease_pool_manager->number_of_pools > 0) {
            // This case (pools is NULL but number_of_pools > 0) should ideally not happen if realloc(NULL, size) behaves like malloc(size).
            // If it does, it's a critical error state.
            fprintf(stderr, "*** CRITICAL ERROR - %s %s - pools array is NULL after realloc with non-zero count.\n", __FILE__, __func__);
            autorelease_pool_manager->number_of_pools--; // Revert count
            return;
        }
        // If number_of_pools is 1 and new_pools_ptr was NULL (malloc failed),
        // then pools is NULL, and the above access is guarded.
        // However, if new_pools_ptr is NULL and number_of_pools was already >0, then pools would be the old valid pointer.
        // The logic for realloc failure handling could be more robust, but this covers the basic case.
    }
    return;
}

/*
 @function OCAutoreleasePoolsManagerAddObject
 Adds the object to the most recently created OCAutoreleasePool.
 @param object The object to be released.
 @param release The method that releases the object.
 */
static bool OCAutoreleasePoolsManagerAddObject(const void *object, void (*release)(const void *))
{
    IF_NO_OBJECT_EXISTS_RETURN(autorelease_pool_manager,false);
    IF_NO_OBJECT_EXISTS_RETURN(object,false);
    IF_NO_OBJECT_EXISTS_RETURN(release,false);
    
    // add object to OCAutoreleasePool at the top of the list
    
    if(autorelease_pool_manager->pools) {
        OCAutoreleasePoolAddObject(autorelease_pool_manager->pools[autorelease_pool_manager->number_of_pools-1],object,release);
        return true;
    }
    //      pool exists
    
    fprintf(stderr,"*** ERROR - %s %s - No OCAutoreleasePool exists.\n",__FILE__,__func__);
    return false;
}


/**************************************************************************
 OCAutoreleasePool methods
 *************************************************************************/


OCAutoreleasePoolRef OCAutoreleasePoolCreate()
{
    OCAutoreleasePoolRef thePool = malloc(sizeof(struct impl_OCAutoreleasePool));
    
    IF_NO_OBJECT_EXISTS_RETURN(thePool,NULL);
    
    thePool->pool_objects = NULL;
    thePool->number_of_pool_objects = 0;
    OCAutoreleasePoolsManagerAddPool(thePool);
    return thePool;
}

/*
 @function OCAutoreleasePoolDeallocate
 @abstract Deallocates a OCAutoreleasePool object.
 @param thePool The pool to be deallocated.
 @result YES (1) if successful, NO (0) if unsuccessful
 */
static bool OCAutoreleasePoolDeallocate(OCAutoreleasePoolRef thePool)
{
    IF_NO_OBJECT_EXISTS_RETURN(thePool,false);
    
    for(int i=0;i<thePool->number_of_pool_objects;i++) if(thePool->pool_objects[i]) {
        OCAutoreleasePoolObjectRef pool_object = thePool->pool_objects[i];
        void (*release_function)(const void *) = OCAutoreleasePoolObjectGetReleaseFunction(pool_object);
        OCTypeRef object = (OCTypeRef) OCAutoreleasePoolObjectGetObject(pool_object);
        if(OCGetTypeID(object) == 0) {
            fprintf(stderr, "*** WARNING - OCAutoreleasePool release of invalid type (%p).\n", object);
            continue; // Skip this object
        }
        if(OCTypeGetRetainCount(object) < 1) {
            fprintf(stderr, "*** WARNING - OCAutoreleasePool release of object (%p) with negative retain count %d.\n", object, OCTypeGetRetainCount(object));
            continue; // Skip this object
        }
        if(OCTypeGetFinalized(object)) {
            fprintf(stderr, "*** WARNING - OCAutoreleasePool release of finalized object (%p).\n", object);
            continue; // Skip this object
        }
        (*release_function)(OCAutoreleasePoolObjectGetObject(pool_object));
        OCAutoreleasePoolObjectDeallocate(pool_object);
    }
    if(thePool->pool_objects) {
        free(thePool->pool_objects);
        thePool->pool_objects=NULL;
    }
    if(thePool) {
        free(thePool);
        thePool=NULL;
    }
    return true;
}

bool OCAutoreleasePoolRelease(OCAutoreleasePoolRef thePool)
{
    IF_NO_OBJECT_EXISTS_RETURN(thePool,false);
    
    return OCAutoreleasePoolsManagerRemovePool(thePool);
}

// Drains the pool without deallocating it: releases all queued objects
void OCAutoreleasePoolDrain(OCAutoreleasePoolRef thePool)
{
    IF_NO_OBJECT_EXISTS_RETURN(thePool, );
    // Release each object and deallocate its wrapper
    for(int i = 0; i < thePool->number_of_pool_objects; i++) {
        OCAutoreleasePoolObjectRef pool_object = thePool->pool_objects[i];
        void (*release_function)(const void *) = OCAutoreleasePoolObjectGetReleaseFunction(pool_object);
        OCTypeRef object = (OCTypeRef) OCAutoreleasePoolObjectGetObject(pool_object);
        if(OCGetTypeID(object) == 0) {
            fprintf(stderr, "*** WARNING - OCAutoreleasePool release of invalid type (%p).\n", object);
            continue; // Skip this object
        }
        if(OCTypeGetRetainCount(object) < 1) {
            fprintf(stderr, "*** WARNING - OCAutoreleasePool release of object (%p) with negative retain count %d.\n", object, OCTypeGetRetainCount(object));
            continue; // Skip this object
        }
        if(OCTypeGetFinalized(object)) {
            fprintf(stderr, "*** WARNING - OCAutoreleasePool release of finalized object (%p).\n", object);
            continue; // Skip this object
        }

        (*release_function)(OCAutoreleasePoolObjectGetObject(pool_object));
        OCAutoreleasePoolObjectDeallocate(pool_object);
    }
    // Free the pool_objects array and reset count
    if(thePool->pool_objects) {
        free(thePool->pool_objects);
        thePool->pool_objects = NULL;
    }
    thePool->number_of_pool_objects = 0;
}

/*
 @function OCAutoreleasePoolGetNumberOfPoolObjects
 @abstract Returns the number of objects in the autorelease pool
 @param thePool The pool.
 @result number of objects in pool
 */
static int OCAutoreleasePoolGetNumberOfPoolObjects(OCAutoreleasePoolRef thePool)
{
    IF_NO_OBJECT_EXISTS_RETURN(thePool,0);
    return thePool->number_of_pool_objects;
}

/*
 @function OCAutoreleasePoolAddObject
 @abstract Adds an object to the most recently created autorelease pool.
 @param thePool The pool where object will be added.
 @param object The object to be added.
 @param release The release function for object.
 @result YES (1) if successful, NO (0) if unsuccessful
 */
static bool OCAutoreleasePoolAddObject(OCAutoreleasePoolRef thePool, const void * object, void (*release)(const void *))
{
    IF_NO_OBJECT_EXISTS_RETURN(thePool, false);
    IF_NO_OBJECT_EXISTS_RETURN(object, false);
    IF_NO_OBJECT_EXISTS_RETURN(release, false);

    // Check for duplicate object already in the pool
    // for (int i = 0; i < thePool->number_of_pool_objects; i++) {
    //     if (OCAutoreleasePoolObjectGetObject(thePool->pool_objects[i]) == object) {
    //         return true;
    //     }
    // }
    
    // Allocate and add new pool object
    OCAutoreleasePoolObjectRef pool_object = OCAutoreleasePoolObjectCreate(object, release);
    if (!pool_object) return false;

    thePool->number_of_pool_objects++;
    thePool->pool_objects = realloc(thePool->pool_objects,
        thePool->number_of_pool_objects * sizeof(OCAutoreleasePoolObjectRef));
    if (NULL == thePool->pool_objects) {
        fprintf(stderr, "*** ERROR - %s %s - realloc failed to grow pool_objects array. Object cannot be added.\n", __FILE__, __func__);
        thePool->number_of_pool_objects--; // Revert count
        OCAutoreleasePoolObjectDeallocate(pool_object);
        return false;
    }

    thePool->pool_objects[thePool->number_of_pool_objects - 1] = pool_object;
    return true;
}


/**************************************************************************
 OCTypes convenience method
 *************************************************************************/


const void *OCAutorelease(const void *ptr)
{
    IF_NO_OBJECT_EXISTS_RETURN(ptr, NULL);

    struct impl_OCType *theType = (struct impl_OCType *) ptr;

    // fprintf(stderr, "OCAutorelease called for %p, typeID = %s\n",
    //         ptr, OCTypeIDName(theType));

    // Invalid type check
    if (OCGetTypeID(ptr) == kOCNotATypeID) {
        fprintf(stderr, "*** WARNING: OCAutorelease called on invalid object (%p), typeID = InvalidTypeID\n", ptr);
        return ptr;
    }

    // Static instances should not be autoreleased
    if (OCTypeGetStaticInstance(ptr)) {
        return ptr;
    }

    // Prevent autorelease of finalized objects
    if (OCTypeGetFinalized(ptr)) {
        fprintf(stderr, "*** WARNING: OCAutorelease called on finalized object (%p), typeID = %s\n",
                ptr, OCTypeIDName(theType));
        return ptr;
    }

    // Prevent autorelease if retainCount is already zero
    if (OCTypeGetRetainCount(ptr) < 1) {
        fprintf(stderr, "*** WARNING: OCAutorelease called on object with retainCount < 1 (%p), typeID = %s\n",
                ptr, OCTypeIDName(theType));
        return ptr;
    }

    // Register for deferred release
    OCAutoreleasePoolsManagerAddObject(ptr, OCRelease);
    return ptr;
}

void OCAutoreleasePoolCleanup(void) {
    if (autorelease_pool_manager) {
        OCAutoreleasePoolsManagerDeallocate(autorelease_pool_manager);
        autorelease_pool_manager = NULL;
    }
}
