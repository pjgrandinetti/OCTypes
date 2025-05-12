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

You can read more about `Opaque data types <http://en.wikipedia.org/wiki/Opaque_data_type>`_ and `Opaque pointers <http://en.wikipedia.org/wiki/Opaque_pointer>`_ on Wikipedia. You can also learn more about Opaque types at `Apple <http://developer.apple.com/library/mac/#documentation/CoreFoundation/Conceptual/CFDesignConcepts/Articles/OpaqueTypes.html#//apple_ref/doc/uid/20001106-CJBEJBHH>`_.

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

