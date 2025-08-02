Background
==========


Part I: Good
-----------------

C Opaque Type as an Object
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In the C language, object-oriented abstraction can be implemented using structures and functions. In C, a pointer to a structure can be defined without disclosing the elements of the structure. For example, in the header file Shape.h we define opaque type

.. code-block:: c

    typedef const struct __Shape * ShapeRef;

This declares a new type, ShapeRef, which is a pointer to a hidden structure __Shape. While the messages we can send to (or functions we can call with) this structure pointer are defined in a C header file, i.e. Shape.h, we are not allowed to see nor access directly the elements of this structure.

You can read more about `Opaque data types <http://en.wikipedia.org/wiki/Opaque_data_type>`_ and `Opaque pointers <http://en.wikipedia.org/wiki/Opaque_pointer>`_ on Wikipedia. You can also learn more about Opaque types at `Apple's Core Foundation documentation <http://developer.apple.com/library/mac/#documentation/CoreFoundation/Conceptual/CFDesignConcepts/Articles/OpaqueTypes.html#//apple_ref/doc/uid/20001106-CJBEJBHH>`_.

Mutable vs Immutable Types
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You may have noticed in the example on Opaque Types that the pointer was defined using the ``const`` keyword. The keyword ``const`` means that the contents of ``__Shape`` structure cannot be modified after it is created. This makes Shape an immutable type. This means that while we can define functions that reveal the internal state (i.e., structure elements) of Shape, we cannot define functions that change its internal state. Only by eliminating the keyword ``const`` in the type definition would we be able to define functions to change the internal state of the type. A type whose internal state can be modified after creation is called a mutable type. Thus, in the header file ``Shape.h`` we also define the opaque type

.. code-block:: c

    typedef struct __Shape * MutableShapeRef;

What are the advantages of an immutable type? Let's say you want to use an instance of type A as a variable inside type B. You decide to do this by placing a copy of the pointer for the instance of A inside B. Unbeknownst to you, however, another type, let's call it C, also has a copy of the pointer for the same instance of A. If A was mutable, then type C could change the value of A, and, in turn, change the instance variable A inside B, without B knowing what happened. There are two ways to handle this problem. If A is mutable, then type B has no choice but to make its own personal copy of the A instance. Depending on the size of A, this could be a memory and time consuming process. On the other hand, if A is immutable, then B can safely (and rapidly) copy only the pointer for A. As you might imagine, immutable types are highly useful in concurrent programs. General advice: don't define an type as mutable unless you really need it.

Read more about `Mutable and Immutable <https://en.wikipedia.org/wiki/Immutable_object>`_ types at Wikipedia.

Implementation of Shape
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Now, looking inside the source Shape.c, we find the structure

.. code-block:: c

    struct __Shape {
        // Shape Type attributes  - order of declaration is essential
        float xPosition;
        float yPosition;
        float orientation;
    };

Creation and Destruction of the Shape Type is handled with these function:

.. code-block:: c

    static struct __Shape *ShapeAllocate()
    {
        struct __Shape *theShape = malloc(sizeof(struct __Shape));
        if(NULL == theShape) return NULL;
        return theShape;
    }

    ShapeRef ShapeCreate(float xPosition, float yPosition, float orientation)
    {
        struct __Shape *newShape = ShapeAllocate();
        if(NULL == newShape) return NULL;
        newShape->xPosition = xPosition;
        newShape->yPosition = yPosition;
        newShape->orientation = orientation;
        return newShape;
    }

    void ShapeFinalize(ShapeRef theShape)
    {
        if(NULL == theShape) return;
        free((void *)theShape);
    }

Comparison and Accessors are handled with these functions:

.. code-block:: c

    bool ShapeEqual(ShapeRef theShape1, ShapeRef theShape2)
    {
        if(NULL == theShape1 || NULL == theShape2) return false;
        if(theShape1 == theShape2) return true;
        if(theShape1->xPosition != theShape2->xPosition) return false;
        if(theShape1->yPosition != theShape2->yPosition) return false;
        if(theShape1->orientation != theShape2->orientation) return false;
        return true;
    }

    float ShapeGetXPosition(ShapeRef theShape)
    {
        if(NULL == theShape) return nan(NULL);
        return theShape->xPosition;
    }

    void ShapeSetXPosition(MutableShapeRef theShape, float xPosition) {
        if(NULL == theShape) return;
        theShape->xPosition = xPosition;
    }

A shape can be translated and rotated. These methods are handled with these functions:

.. code-block:: c

    void ShapeTranslate(MutableShapeRef theShape, float xTranslation, float yTranslation)
    {
        if(NULL == theShape) return;
        theShape->xPosition += xTranslation;
        theShape->yPosition += yTranslation;
    }

    void ShapeRotate(MutableShapeRef theShape, float angle)
    {
        if(NULL == theShape) return;
        theShape->orientation += angle;
    }

Usage of Shape

.. code-block:: c

    MutableShapeRef shape = ShapeCreateMutable(0.0, 0.0, 0.0);
    ShapeShow(shape);

    ShapeTranslate(shape, 10.0, 20.0);
    ShapeRotate(shape, 180.);
    ShapeShow(shape);
    ShapeFinalize(shape);

Inheritance
~~~~~~~~~~~~~~~~~~~~~~

Let's examine how we can define a Square type that inherits from Shape. In source code we define the structure

.. code-block:: c

    struct __Square {
        // Shape Type attributes - order of declaration is essential
        float xPosition;
        float yPosition;
        float orientation;
        // Square Type attributes
        float width;
    };

For this inheritance trick to work it is essential that the order of instance variable declarations be identical to those inside the Shape structure. Any additional instance variables must go after the variables matching Shape's structure.

In the header file we define the opaque types

.. code-block:: c

    typedef const struct __Square * SquareRef;
    typedef struct __Square * MutableSquareRef;
    Creation and Destruction of the Square Type is handled with these function:

    static struct __Square *SquareAllocate()
    {
        struct __Square *theSquare = malloc(sizeof(struct __Square)); if(NULL == theSquare) return NULL;
        return theSquare;
    }

    SquareRef SquareCreate(float xPosition, float yPosition, float orientation, float width)
    {
        struct __Square *newSquare = SquareAllocate();
        if(NULL == newSquare) return NULL;
        newSquare->xPosition = xPosition;
        newSquare->yPosition = yPosition;
        newSquare->orientation = orientation;
        newSquare->width = width;
        return newSquare;
    }

    void SquareFinalize(SquareRef theSquare)
    {
        if(NULL == theSquare) return;
        free((void *)theSquare);
    }
    Comparison and Accessors are handled with these functions:

    bool SquareEqual(SquareRef theSquare1, SquareRef theSquare2)
    {
        if(!ShapeEqual((ShapeRef) theSquare1, (ShapeRef) theSquare2)) return false;
        if(theSquare1->width != theSquare2->width) return false;
        return true;
    }

    float SquareGetXPosition(SquareRef theSquare)
    {
        return ShapeGetXPosition((ShapeRef) theSquare);
    }

    float SquareGetWidth(SquareRef theSquare)
    {
        if(NULL == theSquare) return nan(NULL);
        return theSquare->width;
    }

    void SquareSetXPosition(MutableSquareRef theSquare, float xPosition)
    {
        ShapeSetXPosition((MutableShapeRef) theSquare, xPosition);
    }

    void SquareSetWidth(MutableSquareRef theSquare, float width)
    {
        if(NULL == theSquare) return;
        theSquare->width = width;
    }

Notice how we type cast a Square into a Shape before calling Shape methods.

Usage of Square

.. code-block:: c

    MutableSquareRef square = MutableSquareCreate(0.0, 0.0, 0.0, 10.0);
    ShapeShow((ShapeRef) square);
    SquareShow(square);

    ShapeTranslate((MutableShapeRef) square, 10.0, 20.0);
    ShapeRotate((MutableShapeRef) square, 180.);
    SquareShow(square);
    SquareFinalize(square);


Part II: Better
-----------------

In the last section we examined how Opaque types in C can be adapted to follow some Object Oriented Design Patterns. It is a sensible approach but it still lacks memory management, collections, strings, and many other useful object-oriented design features. To continue on our path towards a more complete approach let's look how we can introduce retain count memory management.

Reference Counting
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When an type is no longer needed it should be deallocated and its memory freed. But how will a type know when it's no longer needed? Type A may hold a reference (pointer) to type B, but how does type A know that type B still exists? For example, type B could have initially been created to be part of type C. If type C gets destroyed along with all it's constituent objects and type A doesn't know, then type A could end up sending a message to (calling a function with) a non-existent type B, and crash the program.

The solution we adopt to solve this problem is called *reference counting*. When a type wants to hold a reference to another type it calls that type's ``retain`` function. Every time an type's ``retain`` function is called, it increments its internal ``retainCount`` variable. Conversely, when an type no longer needs to hold a reference to a type it calls that type's ``release`` function. Every time an type's release function is called, it decrements its internal retainCount variable. When a type's ``retainCount`` hits zero, then the type self destructs. That is, it would call the ``release`` function of any types it had retained, and then deallocate itself.

With this in mind, we follow the conventions below.

* if you create an type (either directly or by making a copy of another type—see *The Create Rule*), you own it. We will explicitly use the word ``Create`` or ``Copy`` in the name of any function that creates and returns a type with a retain count of 1.

* if you get an type from somewhere else, you do not own it. If you want to prevent it being disposed of, you must add yourself as an owner (using a retain method).

* if you are an owner of an type, you must relinquish ownership when you have finished using it (using a release method).

Read more about Reference counting at `Wikipedia <http://en.wikipedia.org/wiki/Reference_counting>`_ and at `Apple's Memory Management documentation <http://developer.apple.com/library/mac/#documentation/CoreFoundation/Conceptual/CFMemoryMgmt/CFMemoryMgmt.html#//apple_ref/doc/uid/10000127i>`_.

Implementation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We begin by creating a fundamental opaque type called KFType, from which all other types will inherit. In KFType source code define structure

.. code-block:: c

    struct impl_KFType {
        u_int32_t retainCount;
        void (*finalize)(void *);
        bool (*equal)(void *, void *);
    };

In KFType header define opaque type

.. code-block:: c

    typedef struct impl_KFType * KFTypeRef;

KFType Methods

.. code-block:: c

    bool KFTypeEqual(KFTypeRef theType1, KFTypeRef theType2)
    {
    return theType1->equal(theType1,theType2);
    }

    void KFRelease(KFTypeRef theType)
    {
        if(NULL==theType) return;
        if(theType->retainCount == 1) {
            theType->finalize(theType); return;
        }
        theType->retainCount--;
        return;
    }

    OCTypeRef OCRetain(OCTypeRef theType)
    {
        if(NULL==theType) return NULL;
        theType->retainCount++;
        return theType;
    }

Now we can define KFShape to inherit from KFType

.. code-block:: c

    struct impl_KFShape {
        u_int32_t retainCount;
        void (*finalize)(void *);
        bool (*equal)(void *, void *);

        // Shape Type attributes
        float xPosition;
        float yPosition;
        float orientation;
    };

    static struct impl_KFShape *KFShapeAllocate()
    {
        struct impl_KFShape *theShape = malloc(sizeof(struct impl_KFShape));
        if(NULL == theShape) return NULL;
        theShape->retainCount = 1;
        theShape->finalize = KFShapeFinalize;
        theShape->equal = KFShapeEqual;
        return theShape;
    }

Usage of KFShape

.. code-block:: c

    KFMutableShapeRef shape = KFShapeCreateMutable(0.0, 0.0, 0.0);
    KFShapeShow(shape);

    KFShapeTranslate(shape, 10.0, 20.0);
    KFShapeRotate(shape, 180.);
    KFShapeShow(shape);

    KFRelease((KFTypeRef) shape);

Part III: Best - OCTypes Framework
-----------------------------------

Inspiration from Core Foundation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In Apple's Core Foundation, the opaque type we called KFType is actually called CFType. The Core Foundation framework
provides a rich set of types and functions that implement reference counting, collections, strings, and many other
useful object-oriented design features. It is a powerful and mature framework that has proven itself in building
complex applications across macOS and iOS platforms.

However, Core Foundation has some limitations for general C development:

* **Size and Complexity**: It's a rather large framework with many components beyond what most projects need
* **Platform Dependencies**: It's not easily portable to non-Apple platforms
* **Learning Curve**: The full API surface can be overwhelming for developers new to object-oriented C

The OCTypes Solution
~~~~~~~~~~~~~~~~~~~~

Here, we provide a lightweight alternative to Core Foundation, called OCTypes. It is a small framework that implements
the core object-oriented design patterns we've explored—reference counting, inheritance, polymorphism, and collections—while
maintaining the essential benefits of type safety and memory management.

Key design principles of OCTypes:

* **Minimal Dependencies**: Implemented in pure C with no external dependencies
* **Cross-Platform**: Suitable for use across different operating systems and environments
* **Educational**: Simple enough to understand and modify, making it an excellent learning tool
* **Practical**: Robust enough for real-world applications while remaining lightweight

The OCBase Foundation
~~~~~~~~~~~~~~~~~~~~~

The heart of the OCTypes system is the OCBase structure, which serves as the foundation for all OCTypes objects.
Let's examine how this evolution improves upon our earlier approaches:

.. code-block:: c

  struct __OCShape {
      OCBase   _base;        // Always the first member

      // Shape-specific attributes
      float xPosition;
      float yPosition;
      float orientation;
  };

The OCBase structure itself contains everything needed for sophisticated object-oriented behavior:

.. code-block:: c

  typedef struct impl_OCBase {
      OCTypeID typeID;                              // Unique identifier for runtime type checking
      uint32_t retainCount;                         // Reference count for memory management
      bool static_instance;                         // Indicates if the instance is static (never deallocated)
      bool finalized;                               // Indicates if the instance has been finalized

      // Virtual method table - enables polymorphism
      void (*finalize)(const void *);               // Finalizer called when retain count reaches zero
      bool (*equal)(const void *, const void *);   // Equality comparison function for this type
      OCStringRef (*copyFormattingDesc)(OCTypeRef);// Human-readable description of the object
      cJSON *(*copyJSON)(const void *);             // JSON serialization function
      void *(*copyDeep)(const void *);              // Immutable deep copy function
      void *(*copyDeepMutable)(const void *);       // Mutable deep copy function

      // Debug and introspection support
      const char *allocFile;                        // Source file where object was allocated
      int allocLine;                                // Source line where object was allocated
      bool tracked;                                 // Whether this object is tracked for leak detection
  } OCBase;

Key Improvements Over Previous Approaches
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**1. Type Safety and Runtime Introspection**

Unlike our earlier approaches, OCTypes provides runtime type checking through OCTypeID. This prevents many common casting errors:

.. code-block:: c

  // Safe: will return NULL if obj is not actually an OCShape
  OCShapeRef shape = (OCShapeRef)obj;
  if (OCGetTypeID(shape) == OCShapeGetTypeID()) {
      // Safe to use as OCShape
  }

**2. Comprehensive Virtual Method Table**

The virtual method table in OCBase enables true polymorphism. Different types can have different implementations of the same conceptual operation:

.. code-block:: c

  // Works for any OCType - calls the appropriate implementation
  OCStringRef description = OCTypeCopyFormattingDesc(anyOCTypeObject);
  bool areEqual = OCTypeEqual(obj1, obj2);
  void *copy = OCTypeDeepCopy(originalObject);

**3. Built-in Debugging and Leak Detection**

The framework includes sophisticated debugging capabilities that help developers track down memory leaks and other issues:

.. code-block:: c

  // Debug information is automatically captured
  OCShapeRef shape = OCShapeCreate(1.0, 2.0, 45.0);  // __FILE__ and __LINE__ recorded

  // Leak detection can report exactly where objects were allocated
  int leakCount = OCLeakTrackerGetUnreleasedObjectCount();

**4. Consistent API Patterns**

OCTypes follows consistent naming and behavior patterns that make the API predictable and learnable:

* **Create functions**: Return objects with retain count 1 (caller owns)
* **Copy functions**: Return new objects with retain count 1 (caller owns)
* **Get functions**: Return borrowed references (caller doesn't own)
* **Retain/Release**: Explicit memory management

A Complete OCShape Implementation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let's see how all these concepts come together in a complete OCShape implementation:

.. code-block:: c

  // Type registration - done once per type
  static OCTypeID kOCShapeID = kOCNotATypeID;

  OCTypeID OCShapeGetTypeID(void) {
      if (kOCShapeID == kOCNotATypeID) {
          kOCShapeID = OCRegisterType("OCShape");
      }
      return kOCShapeID;
  }

  // Structure definition with OCBase inheritance
  struct impl_OCShape {
      OCBase base;           // Must be first for inheritance to work
      float xPosition;
      float yPosition;
      float orientation;
  };

  // Virtual method implementations
  static bool impl_OCShapeEqual(const void *a_, const void *b_) {
      OCShapeRef a = (OCShapeRef)a_;
      OCShapeRef b = (OCShapeRef)b_;

      // First check type compatibility
      if (a->base.typeID != b->base.typeID) return false;
      if (a == b) return true;  // Same object

      // Compare actual values
      return (a->xPosition == b->xPosition &&
              a->yPosition == b->yPosition &&
              a->orientation == b->orientation);
  }

  static void *impl_OCShapeDeepCopy(const void *obj) {
      OCShapeRef source = (OCShapeRef)obj;
      if (!source) return NULL;
      return (void *)OCShapeCreate(source->xPosition, source->yPosition, source->orientation);
  }

  static void *impl_OCShapeDeepCopyMutable(const void *obj) {
      OCShapeRef source = (OCShapeRef)obj;
      if (!source) return NULL;
      return (void *)OCShapeCreateMutable(source->xPosition, source->yPosition, source->orientation);
  }

  static void impl_OCShapeFinalize(const void *obj) {
      // OCShape doesn't own any additional resources, so just free the memory
      // Note: OCBase cleanup is handled automatically by the framework
      free((void *)obj);
  }

  // Allocation function using the OCTypes framework
  static struct impl_OCShape *OCShapeAllocate() {
      return OCTypeAlloc(struct impl_OCShape,
                         OCShapeGetTypeID(),
                         impl_OCShapeFinalize,
                         impl_OCShapeEqual,
                         OCShapeCopyFormattingDesc,
                         impl_OCShapeCopyJSON,
                         impl_OCShapeDeepCopy,
                         impl_OCShapeDeepCopyMutable);
  }

  // Public creation function
  OCShapeRef OCShapeCreate(float x, float y, float orientation) {
      struct impl_OCShape *shape = OCShapeAllocate();
      if (!shape) return NULL;

      // Initialize shape-specific data
      shape->xPosition = x;
      shape->yPosition = y;
      shape->orientation = orientation;

      return (OCShapeRef)shape;
  }

**Benefits of This Approach**

1. **Memory Safety**: Automatic retain/release prevents most memory leaks and use-after-free bugs
2. **Type Safety**: Runtime type checking prevents casting errors
3. **Polymorphism**: Virtual method table enables object-oriented behavior in C
4. **Debugging**: Built-in leak detection and allocation tracking
5. **Consistency**: Predictable API patterns across all types
6. **Portability**: Pure C implementation works everywhere

**Public Interface**

In the header file, we maintain the clean opaque interface while hiding all implementation details:

.. code-block:: c

  typedef const struct __OCShape * OCShapeRef;
  typedef struct __OCShape * OCMutableShapeRef;

  // Creation and memory management
  OCShapeRef OCShapeCreate(float x, float y, float orientation);
  OCMutableShapeRef OCShapeCreateMutable(float x, float y, float orientation);

  // All OCTypes inherit these operations
  OCShapeRef OCRetain(OCShapeRef shape);      // Increment retain count
  void OCRelease(OCShapeRef shape);           // Decrement retain count
  bool OCShapeEqual(OCShapeRef a, OCShapeRef b);  // Compare for equality
  OCShapeRef OCShapeDeepCopy(OCShapeRef source);  // Create immutable copy

This approach provides all the benefits of object-oriented programming—encapsulation, inheritance, and polymorphism—while
remaining true to C's philosophy of simplicity and explicitness.
