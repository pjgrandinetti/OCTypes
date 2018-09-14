//
//  OCAutoreleasePool.c
//
//  Created by Philip on 12/27/09.
//

#import "OCLibrary.h"

#ifndef FREE
#define FREE(X) {free(X); X=NULL;}
#endif

#ifndef IF_SELF_DOESNT_EXISTS_RETURN
#define IF_SELF_DOESNT_EXISTS_RETURN(X) if(NULL==theUnit) {fprintf(stderr, "*** WARNING - %s %s - object doesn't exist.\n",__FILE__,__func__); return X;}
#endif

#ifndef IF_NO_OBJECT_EXISTS_RETURN
#define IF_NO_OBJECT_EXISTS_RETURN(OBJECT,X) if(OBJECT==NULL) {fprintf(stderr, "*** WARNING - %s %s - object doesn't exist.\n",__FILE__,__func__); return X;}
#endif

#define POOL_NOT_FOUND -1
#define POOLOBJECT_NOT_FOUND -1

struct _OCAutoreleasePoolObject {
    const void * object;
    void (*release)(const void *);
};

typedef struct _OCAutoreleasePoolObject * OCAutoreleasePoolObjectRef;

struct _OCAutoreleasePool
{
    OCAutoreleasePoolObjectRef *pool_objects;
    int number_of_pool_objects;
};

struct _OCAutoreleasePoolsManager
{
    OCAutoreleasePoolRef *pools;
    int number_of_pools;
};

typedef struct _OCAutoreleasePoolsManager *OCAutoreleasePoolsManagerRef;

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
    OCAutoreleasePoolObjectRef thePoolObject  = malloc(sizeof(struct _OCAutoreleasePoolObject));
    
    IF_NO_OBJECT_EXISTS_RETURN(thePoolObject,NULL)
    
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
    IF_NO_OBJECT_EXISTS_RETURN(thePoolObject,false)
    
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
    IF_NO_OBJECT_EXISTS_RETURN(thePoolObject,NULL)
    
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
    IF_NO_OBJECT_EXISTS_RETURN(thePoolObject,NULL)
    
    return thePoolObject->object;
}



/**************************************************************************
 OCAutoreleasePoolsManager methods
 *************************************************************************/
static bool OCAutoreleasePoolDeallocate(OCAutoreleasePoolRef thePool);
static bool OCAutoreleasePoolAddObject(OCAutoreleasePoolRef thePool, const void * object, void (*release)(const void *));


static OCAutoreleasePoolsManagerRef OCAutoreleasePoolsManagerCreate(void)
{
    OCAutoreleasePoolsManagerRef thePoolsManager  = malloc(sizeof(struct _OCAutoreleasePoolsManager));
    IF_NO_OBJECT_EXISTS_RETURN(thePoolsManager,NULL)
    
    thePoolsManager->pools = NULL;
    thePoolsManager->number_of_pools = 0;
    return thePoolsManager;
}

static int OCAutoreleasePoolsManagerIndexOfPool(OCAutoreleasePoolRef thePool)
{
    IF_NO_OBJECT_EXISTS_RETURN(autorelease_pool_manager,POOL_NOT_FOUND)
    IF_NO_OBJECT_EXISTS_RETURN(thePool,POOL_NOT_FOUND)
    
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
    IF_NO_OBJECT_EXISTS_RETURN(autorelease_pool_manager,false)
    IF_NO_OBJECT_EXISTS_RETURN(thePool,false)
    
    int poolIndex =  OCAutoreleasePoolsManagerIndexOfPool(thePool);
    if(poolIndex == POOL_NOT_FOUND) return false;
    
    // deallocate pool at poolIndex along with all pools with higher indexes
    for(int i=autorelease_pool_manager->number_of_pools-1;i>=poolIndex;i--)
        OCAutoreleasePoolDeallocate(autorelease_pool_manager->pools[i]);
    
    autorelease_pool_manager->number_of_pools = poolIndex;
    
    if(autorelease_pool_manager->number_of_pools>0) {
        autorelease_pool_manager->pools = realloc(autorelease_pool_manager->pools,autorelease_pool_manager->number_of_pools*sizeof(OCTypeRef));
        return true;
    }
    return false;
}

static bool OCAutoreleasePoolsManagerDeallocate(OCAutoreleasePoolsManagerRef thePoolsManager)
{
    IF_NO_OBJECT_EXISTS_RETURN(thePoolsManager,false)
    
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
    IF_NO_OBJECT_EXISTS_RETURN(thePool,)
    
    if(autorelease_pool_manager==NULL) autorelease_pool_manager = OCAutoreleasePoolsManagerCreate();
    if(autorelease_pool_manager) {
        autorelease_pool_manager->number_of_pools++;
        autorelease_pool_manager->pools = realloc(autorelease_pool_manager->pools,
                                                  autorelease_pool_manager->number_of_pools
                                                  *sizeof(OCTypeRef));
        autorelease_pool_manager->pools[autorelease_pool_manager->number_of_pools-1] = thePool;
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
    IF_NO_OBJECT_EXISTS_RETURN(autorelease_pool_manager,false)
    IF_NO_OBJECT_EXISTS_RETURN(object,false)
    IF_NO_OBJECT_EXISTS_RETURN(release,false)
    
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
    OCAutoreleasePoolRef thePool = malloc(sizeof(struct _OCAutoreleasePool));
    
    IF_NO_OBJECT_EXISTS_RETURN(thePool,NULL)
    
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
    IF_NO_OBJECT_EXISTS_RETURN(thePool,false)
    
    for(int i=0;i<thePool->number_of_pool_objects;i++) if(thePool->pool_objects[i]) {
        OCAutoreleasePoolObjectRef pool_object = thePool->pool_objects[i];
        void (*release_function)(const void *) = OCAutoreleasePoolObjectGetReleaseFunction(pool_object);
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
    IF_NO_OBJECT_EXISTS_RETURN(thePool,false)
    
    return OCAutoreleasePoolsManagerRemovePool(thePool);
}

/*
 @function OCAutoreleasePoolGetNumberOfPoolObjects
 @abstract Returns the number of objects in the autorelease pool
 @param thePool The pool.
 @result number of objects in pool
 */
static int OCAutoreleasePoolGetNumberOfPoolObjects(OCAutoreleasePoolRef thePool)
{
    IF_NO_OBJECT_EXISTS_RETURN(thePool,0)
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
    IF_NO_OBJECT_EXISTS_RETURN(thePool,false)
    
    if(object) {
        OCAutoreleasePoolObjectRef pool_object = OCAutoreleasePoolObjectCreate(object,release);
        thePool->number_of_pool_objects++;
        thePool->pool_objects = realloc(thePool->pool_objects,
                                        thePool->number_of_pool_objects*sizeof(OCTypeRef));
        thePool->pool_objects[thePool->number_of_pool_objects-1] = pool_object;
        return true;
    }
    return false;
}



/**************************************************************************
 Core Foundation convenience method
 *************************************************************************/


const void * OCAutorelease(const void *ptr)
{
    IF_NO_OBJECT_EXISTS_RETURN(ptr,NULL)
    
    OCAutoreleasePoolsManagerAddObject(ptr, OCRelease);
    return ptr;
}


