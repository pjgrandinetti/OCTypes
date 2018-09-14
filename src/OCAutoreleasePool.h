//
//  OCAutoreleasePool.h
//
//  Created by Philip on 12/27/09.
//

/*!
 @header OCAutoreleasePool
 
 OCAutorelease implements an autorelease pool for OCTypes,
 which employ a reference count memory management pattern.
 
 To use ...
 
 <pre><code>OCAutoreleasePoolRef thePool = OCAutoreleasePoolCreate();</code></pre>
 
 ... create, retain, and autorelease objects...
 
 <pre><code>OCAutoreleasePoolRelease(thePool);</code></pre>
 
 The creation and release of addition autorelease pools can be nested inside.
 
 OCAutoreleasePoolRelease should always be called in the same context (invocation
 of a method or function, or body of a loop) in which OCAutoreleasePoolCreate was called.
 If you release an autorelease pool that is not the top of the stack,
 this causes all (unreleased) autorelease pools above it on the
 stack to be released, along with all their objects.
 
 CFTypes can be autorelease with
 OCTypeRef OCAutorelease(OCTypeRef cf)
 
 Inside your own C types you must create an autorelease method that calls
 the singleton OCAutoreleasePoolsManager method ...
 
 OCAutoreleasePoolAddObject(OCTypeRef object, void (*release)(OCTypeRef));
 
 The first argument to this method is the (object) that will be released when the
 autorelease pool containing it is released.
 
 The second argument is the C type's' release method.
 
 This method will add the type to the most recently created OCAutoreleasePool.
 When that OCAutoreleasePool is released then all the types in its
 pool are sent a release message.   You cannot call OCAutoreleasePoolsManagerAddObject()
 until a OCAutoreleasePool has been created.
 
 @copyright PhySy Ltd
 */

/*!
 @typedef OCAutoreleasePoolRef
 This is the type of a reference to OCAutoreleasePool.
 */
typedef struct _OCAutoreleasePool *OCAutoreleasePoolRef;

/*!
 @function OCAutoreleasePoolCreate
 @abstract Creates a new autorelease pool.
 */
OCAutoreleasePoolRef OCAutoreleasePoolCreate(void);

/*!
 @function OCAutoreleasePoolRelease
 @abstract Releases a OCAutoreleasePool object.
 @param thePool The pool to be released.
 @result YES (1) if successful, NO (0) if unsuccessful
 */
bool OCAutoreleasePoolRelease(OCAutoreleasePoolRef thePool);

/*!
 @function OCAutorelease
 @abstract Autoreleases a CFType.
 @param ptr type to be autoreleased.
 @result type to be autoreleased
 @discussion Special Considerations: If ptr is NULL, this will cause an error when the autorelease pool is deallocated and your application will crash.
 */
const void * OCAutorelease(const void * ptr);

