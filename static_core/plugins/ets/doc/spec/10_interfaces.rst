..
    Copyright (c) 2021-2025 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _Interfaces:

Interfaces
##########

.. meta:
    frontend_status: Done

An interface declaration declares an *interface type*, i.e., a reference
type that:

-  Includes properties and methods as its members;
-  Has no instance variables (fields);
-  Usually declares one or more methods;
-  Allows otherwise unrelated classes to provide implementations for the
   methods, and so implement the interface.

.. index::
   interface declaration
   interface type
   reference type
   instance variable
   property
   method
   member
   implementation
   class
   interface

Creating an instance of interface type is not possible.

An interface can be declared *direct extension* of one or more other
interfaces. In that case the interface inherits all members from the interfaces
it extends. Inherited members can be optionally overridden or hidden.




A class can be declared to *directly implement* one or more interfaces. Any
instance of a class implements all methods specified by its interface(s).
A class implements all interfaces that its direct superclasses and direct
superinterfaces implement. Interface inheritance allows objects to
support common behaviors without sharing a superclass.

.. index::
   interface
   interface type
   creation
   instantiation
   direct extension
   inheritance
   extension
   superinterface
   direct implementation
   superclass
   object
   overriding
   shadowing

The value of a variable declared *interface type* can be a reference to any
instance of class that implements the specified interface. However, it is not
enough for a class to implement all methods of an interface. A class or one of
its superclasses must be actually declared to implement an interface.
Otherwise, the class is not considered to implement the interface.

Interfaces are assignable to the class
``Object`` (see :ref:`Assignability`).

.. index::
   variable
   interface type
   interface
   reference
   method
   implementation
   assignability

|

.. _Interface Declarations:

Interface Declarations
**********************

.. meta:
    frontend_status: Done

*Interface declaration* specifies a new named reference type.

The syntax of *interface declarations* is presented below:


.. index::
   interface declaration
   reference type

.. code-block:: abnf

    interfaceDeclaration:
        'interface' identifier typeParameters?
        interfaceExtendsClause? '{' interfaceMember* '}'
        ;

    interfaceExtendsClause:
        'extends' interfaceTypeList
        ;

    interfaceTypeList:
        typeReference (',' typeReference)*
        ;

The *identifier* in an interface declaration specifies the interface name.

An interface declaration with ``typeParameters`` introduces a new generic
interface (see :ref:`Generics`).

The scope of an interface declaration is defined in :ref:`Scopes`.

.. The interface declaration shadowing is specified in :ref:`Shadowing by Parameter`.

.. index::
   identifier
   interface declaration
   interface name
   class name
   generic interface
   generic declaration
   scope

|

.. _Superinterfaces and Subinterfaces:

Superinterfaces and Subinterfaces
*********************************

.. meta:
    frontend_status: Done

An interface declared with an ``extends`` clause extends all other named
interfaces, and thus inherits all their members. Such other named interfaces
are *direct superinterfaces* of a declared interface. A class that *implements*
the declared interface also implements all interfaces that the interface
*extends*.

.. index::
   superinterface
   subinterface
   extends clause
   direct superinterface
   implementation
   declared interface
   interface

A :index:`compile-time error` occurs if:

-  Interface type named by ``typeReference`` in the ``extends`` clause of an
   interface declaration is not accessible (see :ref:`Accessible`).
-  Type arguments (see :ref:`Type Arguments`) of ``typeReference`` denote a
   parameterized type that is not well-formed (see
   :ref:`Generic Instantiations`).
-  The ``extends`` graph has a cycle.
-  At least one ``typeReference`` is an alias of one of primitive, enumeration,
   union, or function  types.

Each ``typeReference`` in the ``extends`` clause of an interface declaration
must name an accessible interface type (see :ref:`Accessible`). Otherwise, a
:index:`compile-time error` occurs.

.. index::
   extends clause
   interface declaration
   interface type
   access
   accessibility
   scope
   type argument
   parameterized type
   type-parameterized declaration
   well-formed parameterized type
   primitive type
   enumeration type
   union type
   function type
   enum type

If an interface declaration (possibly generic) ``I`` <``F``:sub:`1` ``,...,
F``:sub:`n`> (:math:`n\geq{}0`) contains an ``extends`` clause, then the
*direct superinterfaces* of the interface type ``I`` <``F``:sub:`1` ``,...,
F``:sub:`n`> are the types given in the ``extends`` clause of the declaration
of ``I``.

All *direct superinterfaces* of the parameterized interface type ``I``
<``T``:sub:`1` ``,..., T``:sub:`n`> are types ``J``
<``U``:sub:`1`:math:`\theta{}` ``,..., U``:sub:`k`:math:`\theta{}`>, if:

-  ``T``:sub:`i` (:math:`1\leq{}i\leq{}n`) is the type of a generic interface
   declaration ``I`` <``F``:sub:`1` ``,..., F``:sub:`n`> (:math:`n > 0`);
-  ``J`` <``U``:sub:`1` ``,..., U``:sub:`k`> is a direct superinterface of
   ``I`` <``F``:sub:`1` ``,..., F``:sub:`n`>; and
-  :math:`\theta{}` is a substitution
   [``F``:sub:`1` ``:= T``:sub:`1` ``,..., F``:sub:`n` ``:= T``:sub:`n`].

.. index::
   interface declaration
   generic
   generic declaration
   extends clause
   direct superinterface
   compile-time error
   parameterized interface
   substitution

The transitive closure of the direct superinterface relationship results in
the *superinterface* relationship.

Interface *I* is a *subinterface* of *K* wherever *K* is a superinterface of *I*.
Interface *K* is a superinterface of *I* if:

-  *I* is a direct subinterface of *K*; or
-  *K* is a superinterface of some interface *J* of which *I* is, in turn,
   a subinterface.

.. index::
   transitive closure
   direct superinterface
   superinterface
   compile-time error
   direct subinterface
   interface
   subinterface

There is no single interface to which all interfaces are extensions (unlike
class ``Object`` to which every class is an extension).

A :index:`compile-time error` occurs if an interface depends on itself.

If superinterfaces have default implementations (see
:ref:`Default Interface Method Declarations`) for some method ``m``, then
the following occurs:

- Method ``m`` with an override-compatible signature (see
  :ref:`Override-Compatible Signatures`) must be declared within the current
  interface that extends these interfaces; or
- All these methods refer to the same implementation, and this default
  implementation is the current class method.

Otherwise, a :index:`compile-time error` occurs.

.. code-block:: typescript
   :linenos:

    interface I1 { foo () {} }
    interface I2 { foo () {} }
    interface II1 extends I1, I2 {
       foo () {} // foo() from II1 overrides both foo() from I1 and foo() from I2
    }
    interface II2 extends I1, I2 {
       // Compile-time error as foo() from I1 and foo() from I2 have different implementations
    }
    interface I3 extends I1 {}
    interface I4 extends I1 {}
    interface II3 extends I3, I4 {
       // OK, as foo() from I3 and foo() from I4 refer to the same implementation
    }

.. index::
   interface
   object
   class
   method
   extension
   implementation
   override-compatible signature

|

.. _Interface Body:

Interface Body
**************

.. meta:
    frontend_status: Done

*Interface body* can declare members of an interface, i.e., its
properties (see :ref:`Interface Properties`) and methods (see
:ref:`Interface Method Declarations`).

The syntax of *interface body* is presented below:

.. code-block:: abnf

    interfaceMember
        : annotationUsage?
        ( interfaceProperty
        | interfaceMethodDeclaration
        )
        ;

The scope of declaration of a member *m* that the interface type ``I``
declares or inherits is specified in :ref:`Scopes`.

The usage of annotations is discussed in :ref:`Using Annotations`.

.. index::
   interface body
   interface
   interface member
   interface type
   property
   method
   interface declaration
   method declaration
   scope
   inheritance
   annotation

|

.. _Interface Members:

Interface Members
*****************

.. meta:
    frontend_status: Done

*Interface type members* are as follows:

-  Members declared in the interface body (see :ref:`Interface Body`);
-  Members inherited from a direct superinterface (see
   :ref:`Superinterfaces and Subinterfaces`).

A :index:`compile-time error` occurs if the method explicitly declared by the
interface has the same name as the ``Object``'s ``public`` method, but their
signatures are different or override-compatible (see
:ref:`Override-Compatible Signatures`) with default implementations (see
:ref:`Default Interface Method Declarations`).

.. code-block:: typescript
   :linenos:

    interface I {
        toString (p: number): void // Compile-time error
        toString(): string { return "some string" } // Compile-time error
    }

.. index::
   interface
   interface body
   interface type member
   inheritance
   direct superinterface
   Object
   public method
   signature
   override-compatible signature
   implementation

An interface inherits all members of the interfaces it extends
(see :ref:`Interface Inheritance`).

A name in a declaration scope must be unique, i.e., the names of properties and
methods of an interface type must not be the same (see
:ref:`Interface Declarations`).

.. index::
   inheritance
   interface
   property
   method
   declaration scope
   interface type

|

.. _Interface Properties:

Interface Properties
********************

.. meta:
    frontend_status: Done

*Interface property* can be defined in the form of a field or an accessor
(a getter or a setter).

The syntax of *interface property* is presented below:

.. code-block:: abnf

    interfaceProperty:
        'readonly'? identifier '?'? ':' type
        | 'get' identifier '(' ')' returnType
        | 'set' identifier '(' parameter ')'
        ;

If '``?``' is used after the name of the property, then the property type is
semantically equivalent to ``type | undefined``.

.. code-block:: typescript
   :linenos:

    interface I {
        property?: Type
    }
    // is the same as
    interface I {
        property: Type | undefined
    }


A property defined in the form of a field implicitly defines the following:

-  A getter, if the property is marked as ``readonly``;
-  Otherwise, both a getter and a setter with the same name.

As a result, the following definitions have the same effect:

.. index::
   property
   interface
   interface property
   field
   accessor
   readonly
   getter
   setter

.. code-block:: typescript
   :linenos:

    interface Style {
        color: string
    }
    // is the same as
    interface Style {
        get color(): string
        set color(s: string)
    }

A class that implements an interface with properties can also use a field or
an accessor notation (see :ref:`Implementing Interface Properties`).

.. index::
   implementation
   interface
   interface property
   field
   accessor notation
   property

|

.. _Interface Method Declarations:

Interface Method Declarations
*****************************

.. meta:
    frontend_status: Done

An ordinary *interface method declaration* specifies the method's name and
signature, and is called *abstract*.

An interface method can have a body (see :ref:`Default Interface Method Declarations`)
as an experimental feature.

.. index::
   interface
   interface method declaration
   method name
   method signature
   method
   declaration
   signature
   interface method
   method body

The syntax of *interface method declaration* is presented below:

.. code-block:: abnf

    interfaceMethodDeclaration:
        identifier signature
        | interfaceDefaultMethodDeclaration
        ;

The methods declared within interface bodies are implicitly ``public``.

A :index:`compile-time error` occurs if the body of an interface declares a
method with a name that is already used for a property in this declaration.

.. index::
   interface body
   public method
   method
   interface
   property
   declaration

|

.. _Interface Method Overloading:

Interface Method Overloading
============================

.. meta:
    frontend_status: Done

|LANG| allows specifying several interface methods with a single name.
A :index:`compile-time error` occurs if signatures of these methods are
overload-equivalent (see :ref:`Overload-Equivalent Signatures`).

.. index::
   interface
   method
   overloading
   method overloading
   overload-equivalent method

Overloading methods used in a class are represented in the example below:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    interface I {
        foo()           // 1st method
        foo(x: string)  // 2st method
    }
    class C implements I {
        foo()           {/*1st method body*/}
        foo(x: string)  {/*2nd method body*/}
    }

    function demo(i: I) {
       i.foo()           // ok, 1st method is called
       i.foo("aa")       // ok, 2nd method is called
    }

.. index::
   implementation

|

.. _Interface Inheritance:

Interface Inheritance
*********************

.. meta:
    frontend_status: Done

Interface *I* inherits all properties and methods from its direct superinterfaces.
Semantic checks are described in :ref:`Overloading and Overriding in Interfaces`.

**Note**. The semantic rules of methods apply to properties because any
interface property implicitly defines a getter, a setter, or both.

Private methods defined in superinterfaces are not accessible (see
:ref:`Accessible`) in the interface body.

.. index::
   inheritance
   interface
   direct superinterface
   superinterface
   semantic check
   private method
   property
   getter
   setter
   access
   accessibility
   interface body

A :index:`compile-time error` occurs if interface *I* declares a ``private``
method *m* with a signature compatible with the instance method :math:`m'`
(see :ref:`Override-Compatible Signatures`) that has any access modifier in the
superinterface of *I*.

.. index::
   interface
   private method
   compatibility
   instance method
   override-compatible signature
   access
   superinterface
   private method
   signature

.. raw:: pdf

   PageBreak
