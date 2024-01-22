..
    Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

-  Includes constants, properties, and methods as its members;
-  Has no instance variables;
-  Usually declares one or more methods;
-  Allows otherwise unrelated classes to provide implementations for the
   methods and so implement the interface.

.. index::
   interface declaration
   interface type
   reference type
   instance variable
   constant
   property
   method
   member
   implementation
   class
   interface

Interfaces cannot be instantiated directly.

Interfaces can be *top-level* and local (see :ref:`Local Classes And Interfaces`).

An interface can declare a *direct extension* of one or more other interfaces.
In that case the interface inherits all members from the interfaces it extends
(except any members that it *overrides* or *hides*).

A class can be declared to *directly implement* one or more interfaces. Any
instance of the class implements all methods specified by the interface(s).
A class implements all interfaces that its direct superclasses and direct
superinterfaces implement. Such interface inheritance allows objects to
support common behaviors without sharing a superclass.

.. index::
   interface
   instantiation
   interface
   direct extension
   inheritance
   extension
   superinterface
   implementation
   superclass
   object
   overriding
   hiding

The value of a variable declared as an interface type can be a reference
to any instance of the class that implements the specified interface.
However, it is not enough for the class to implement all methods of the
interface. The class or one of its superclasses must be actually
declared to implement the interface. Otherwise such class is not
considered to implement the interface.

Interfaces without *interfaceExtendsClause* have class *Object* as their
supertype (see :ref:`Object Class Type`). It enables assignments on the basis
of reference types conversions (see :ref:`Widening Reference Conversions`).

.. index::
   variable
   interface type
   interface
   method
   implementation
   assignment
   reference types conversion
   supertype
   superclass

|

.. _Interface Declarations:

Interface Declarations
**********************

.. meta:
    frontend_status: Partly
    todo: inner interface, class, enum support

An *interface declaration* specifies a new named reference type:

.. index::
   interface declaration
   reference type

.. code-block:: abnf

    interfaceDeclaration:
        'interface' identifier typeParameters?
        interfaceExtendsClause? '{' interfaceBody '}'
        ;

    interfaceExtendsClause:
        'extends' interfaceTypeList
        ;

    interfaceTypeList:
        typeReference (',' typeReference)*
        ;

    interfaceBody:
        interfaceMember*
        ;

The *identifier* in an interface declaration specifies the interface name.

An interface declaration with *typeParameters* introduces a new generic
interface in :ref:`Generic Declarations`.

The scope of an interface declaration is defined in :ref:`Scopes`.

The interface declaration shadowing is specified in :ref:`Shadowing Parameters`.

.. index::
   identifier
   interface declaration
   class name
   generic interface
   generic declaration
   scope
   shadowing
   shadowing parameter

|

.. _Superinterfaces and Subinterfaces:

Superinterfaces and Subinterfaces
*********************************

.. meta:
    frontend_status: Done

An interface declared with an *extends* clause extends all other named
interfaces, and thus inherits all their members. Such other named interfaces
are *direct superinterfaces* of a declared interface.

A class that *implements* the declared interface also implements all the
interfaces that the interface *extends*.

.. index::
   superinterface
   subinterface
   extends clause
   interface
   inheritance
   direct superinterface
   implementation
   declared interface

A compile-time error occurs if:

-  *typeReference* in the *extends* clause of an interface declaration
   names an interface type that is not accessible (see :ref:`Scopes`).
-  Type arguments of *typeReference* denote a parameterized type that
   is not well-formed (see :ref:`Generic Instantiations`).
-  There is a cycle in *extends* graph.
-  At least one of *typeReference*'s is an alias of one of primitive or
   enum types.
-  Any type argument is a wildcard type.


Each *typeReference* in the *extends* clause of an interface declaration must
name an accessible interface type (see :ref:`Scopes`). Otherwise, a
compile-time error occurs.

.. index::
   compile-time error
   extends clause
   interface declaration
   access
   scope
   type argument
   parameterized type
   type-parameterized declaration
   primitive type
   enum type
   wildcard
   extends clause
   interface type

If an interface declaration (possibly generic) *I* <*F*:sub:`1`,...,
*F*:sub:`n`> (:math:`n\geq{}0`) contains an *extends* clause, then the
*direct superinterfaces* of the interface type *I* <*F*:sub:`1`,...,
*F*:sub:`n`> are the types given in the *extends* clause of the declaration
of *I*.

The *direct superinterfaces* of the parameterized interface type *I*
<*T*:sub:`1`,..., *T*:sub:`n`> are all types *J*
<*U*:sub:`1`:math:`\theta{}`,..., *U*:sub:`k`:math:`\theta{}`>, if:

-  *T*:sub:`i` (:math:`1\leq{}i\leq{}n`) is the type of a generic interface
   declaration *I* <*F*:sub:`1`,..., *F*:sub:`n`> (:math:`n > 0`);
-  *J* <*U*:sub:`1`,..., *U*:sub:`k`> is a direct superinterface of
   *I* <*F*:sub:`1`,..., *F*:sub:`n`>; and
-  :math:`\theta{}` is the substitution
   [*F*:sub:`1` := *T*:sub:`1`,..., *F*:sub:`n` := *T*:sub:`n`].

.. index::
   interface declaration
   generic declaration
   extends clause
   direct superinterface
   compile-time error
   parameterized interface

The transitive closure of the direct superinterface relationship results in
the *superinterface* relationship.

Wherever *K* is a superinterface of the interface *I*, *I* is a *subinterface*
of *K*.

The interface *K* is a superinterface of the interface *I* if:

-  *I* is a direct subinterface of *K*; or
-  *K* is a superinterface of some interface *J* of which *I* is, in its turn,
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
class *Object* to which every class is an extension).

If the *extends* clause of *I* mentions *T* as a superinterface, or as a
qualifier in the fully qualified form of a superinterface name, then the
interface *I* *directly depends* on type *T*.

Moreover, the interface *I* *depends* on a reference type *T* if:

-  *I* directly depends on *T*; or
-  *I* directly depends on the class *C* that depends on *T* (see
   :ref:`Classes`); or
-  *I* directly depends on the interface *J* that, in its turn, depends
   on *T*.


A compile-time error occurs if an interface depends on itself.

*ClassCircularityError* is thrown if circularly declared interfaces
are detected as interfaces and loaded at runtime.

.. index::
   compile-time error
   interface
   runtime

|

.. _Interface Body:

Interface Body
**************

.. meta:
    frontend_status: Partly

The body of an interface may declare members of the interface, i.e.,
properties (see :ref:`Interface Declarations`) and methods (see
:ref:`Method Declarations`).

.. code-block:: abnf

    interfaceMember
        : interfaceProperty
        | interfaceMethodDeclaration
        ;

The scope of declaration of a member *m* that the interface type *I*
declares or inherits is specified in :ref:`Scopes`.

.. index::
   interface body
   interface
   interface member
   property
   interface declaration
   method declaration
   scope
   inheritance

|

.. _Interface Members:

Interface Members
*****************

.. meta:
    frontend_status: Done

Interface type members are as follows:

-  Members declared in the interface body (see :ref:`Interface Body`);
-  Members inherited from a direct superinterface (see
   :ref:`Superinterfaces and Subinterfaces`).

An interface without a direct superinterface implicitly declares the following:

   -  Abstract-member method *m* (see :ref:`Interface Method Declarations`)
      with signature *s*;
   -  Return type *r* and *throws* clause *t* that correspond to each *public*
      instance method *m* with signature *s*;
   -  Return type *r* and *throws* clause *t* declared in *Object* (see
      :ref:`Object Class Type`);


---if the interface does not explicitly declare an abstract method (see
:ref:`Interface Method Declarations`) with the same signature and return
type, and a compatible *throws* clause.


A compile-time error occurs if the interface explicitly declares:

   -  A method *m* that *Object* declares as *final*.
   -  A method with a signature that is override-equivalent (see
      :ref:`Signatures`) to an *Object*’s *public* method, but is not
      *abstract*, and has a different return type or an incompatible
      *throws* clause.

.. index::
   interface member
   compile-time error
   interface body
   inheritance
   inherited member
   direct superinterface
   interface
   abstract member method
   public method
   direct superinterface
   Object
   public method
   abstract method
   signature
   interface method declaration
   throws clause
   instance method
   return type
   override-equivalent signature

An interface normally inherits all members of the interfaces it extends.
However, an interface does not inherit:

-  Fields it hides;
-  Methods it overrides (see :ref:`Inheritance and Overriding`).


A name in a declaration scope must be unique, i.e., the names of fields and
methods of an interface type must not be the same (see
:ref:`Interface Declarations`).

.. index::
   inheritance
   interface
   field
   method
   private method
   static method
   overriding
   declaration scope
   interface type
   interface declaration

|

.. _Interface Properties:

Interface Properties
********************

.. meta:
    frontend_status: None

An interface property can be defined in the form of a field or an accessor
(a getter or a setter):

.. code-block:: abnf

    interfaceProperty:
        readonly? identifier ':' type
        | 'get' identifier '(' ')' returnType
        | 'set' identifier '(' parameter ')'
        ;

If a property is defined in the form of a field, then it implicitly defines
the following:

-  A getter, if a field is marked as *readonly*;
-  Otherwise, both a getter and a setter with the same name.

.. index::
   field
   getter
   readonly field
   setter

As a result, the effect of the following definitions is the same:

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
   field
   accessor notation
   interface property
   accessor notation

|

.. _Interface Method Declarations:

Interface Method Declarations
*****************************

.. meta:
    frontend_status: Done

An ordinary interface method declaration that specifies the method's name and
signature is called *abstract*.

As experimental features, an interface method can have a body (see
:ref:`Default Method Declarations`) and be *static* (see :ref:`Static Method Declarations`).

.. index::
   interface method declaration
   default method declaration
   abstract signature
   interface method
   static method

.. code-block:: abnf

    interfaceMethodDeclaration:
        interfaceMethodOverloadSignature*
        identifier signature
        | interfaceDefaultMethodDeclaration
        | interfaceStaticMethodDeclaration
        ;

The methods declared within interface bodies are implicitly *public*.

A compile-time error occurs if the body of an interface declares:

-  A method with a name already used for a field in this declaration.
-  Two methods (overridden explicitly or implicitly) with override-equivalent
   signatures (see :ref:`Signatures`), if such signatures are not inherited
   (see :ref:`Inheritance and Overriding`).

.. index::
   compile-time error
   interface body
   method
   override-equivalent signature
   signature
   inheritance
   overriding

|

.. _Interface Methods Overload Signatures:

Interface Method Overload Signatures
====================================

.. meta:
    frontend_status: None

|LANG| allows specifying a method that can have several *overload signatures*
but the same name.

.. code-block:: abnf

    interfaceMethodOverloadSignature:
        identifier signature
        ;

Calling a method with *overload signatures* means that the overload signature
implemented by the method called is textually the last.

*Overload signatures* compatibility requirements are discussed in
:ref:`Overload Signature Compatibility`.

In the example below, one overload signature is parameterless, and the other
who have one parameter each:

.. code-block:: typescript
   :linenos:

    interface C {
        foo(): void           // 1st signature
        foo(x: string): void  // 2nd signature
        foo(x?: string): void // 3rd - implementation signature
    }
    function demo (c: C) {
       c.foo()           // ok, call fits 1st and 3rd signatures
       c.foo("aa")       // ok, call fits 2nd and 3rd signatures
       c.foo(undefined)  // ok, call fits the 3rd signature
    }

If a class implements an interafce that has a method with an overload
signature, then the class must also provide a method that has an overload
signature.

.. code-block:: typescript
   :linenos:

    class Base { ... }
    class Derived extends Base { ... }

    interface Interface {
      foo (p: Derived)
      foo (p: Base)
    }

    class Class implements Interface {
      foo (p: Derived)
      foo (p: Base) { ... }
    }


.. index::
   interface method
   overload signature
   method header
   signature
   method overload signature
   compile-time error
   call
   overload signature
   optional parameter
   least upper bound
   overload signature compatibility

|

.. _Inheritance and Overriding:

Inheritance and Overriding
==========================

.. meta:
    frontend_status: Done

The interface *I* inherits any abstract and default method *m* from its
direct superinterfaces if **all** of the following is true:

-  *m* is a member of *I*’s direct superinterface *J*;
-  *I* declares no method with a signature that is compatible with the
   signature of *m* (see :ref:`Compatible Signature`);
-  No method :math:`m'` that is a member of the *I*’s direct superinterface
   :math:`J'` (where *m* is distinct from :math:`m'`, and *J* from :math:`J'`)
   overrides the declaration of the method *m* from :math:`J'`.


.. index::
   inheritance
   overriding
   interface
   abstract method
   default method
   direct superinterface
   subsignature
   signature
   overriding
   method declaration

An interface cannot inherit *private* or *static* methods from its
superinterfaces.

A compile-time error occurs if:

-  The interface *I* declares a *private* or *static* method *m*;
-  The signature of *m* is compatible with the *public* instance method
   :math:`m'` in a superinterface of *I* (see :ref:`Compatible Signature`); and
-  :math:`m'` is otherwise accessible to code in *I*.

.. index::
   compile-time error
   interface
   superinterface
   inheritance
   private method
   static method
   signature
   subsignature
   public instance
   access

|

.. _Overriding by Instance Methods in Interfaces:

Overriding by Instance Methods
==============================

.. meta:
    frontend_status: Done

The instance method *m*:sub:`I` (declared in, or inherited by the interface *I*)
overrides another instance method *m*:sub:`J` of *I* (declared in interface *J*)
if **all** of the following is true:

-  *J* is a superinterface of *I*;
-  *I* does not inherit *m*:sub:`J`;
-  The signature of *m*:sub:`I` is compatible with (see
   :ref:`Compatible Signature`) the signature of *m*:sub:`J`; and
-  *m*:sub:`J` is *public*.

.. index::
   overriding
   instance method
   inheritance
   interface
   instance method
   interface
   superinterface
   subsignature
   signature

|

.. _Overriding Requirements:

Overriding Requirements
=======================

.. meta:
    frontend_status: Done

The following kinds of relationships are described in :ref:`Requirements in Overriding and Hiding`:

-  The relationship between the return type of an interface and that of any
   overridden interface method.
-  The relationship between the *throws* clause of an interface method and
   that of any overridden interface method.
-  The relationship between the signatures of an interface method and that
   of any overridden interface method.
-  The relationship between the accessibility of an interface method and that
   of any overridden interface method.

A compile-time error occurs if a default method is override-equivalent to a
non-*private* method of the class *Object*. Any class that implements
interface must inherit the method's own implementation.

.. index::
   overriding
   return type
   interface
   throws clause
   interface method
   overridden interface
   overridden interface method
   compile-time error
   override-equivalent method
   private method
   Object
   implementation

|

.. _Interfaces Inheriting Methods with Override-Equivalent Signatures:

Interfaces Inheriting Methods with Override-Equivalent Signatures
=================================================================

An interface can inherit several methods with override-equivalent signatures
(see :ref:`Override-Equivalent Signatures`).

A compile-time error occurs if the interface *I* inherits a default method with
a signature that is override-equivalent to an abstract or default method
inherited by *I*.

However, the interface *I* inherits all methods that are abstract.

A compile-time error occurs if one of the inherited methods for every other
inherited method is not return-type-substitutable. A *throws* clause causes
no error in such cases.

The same method declaration can use multiple paths of inheritance from an
interface. It causes no compile-time error on its own.

.. index::
   interface inheriting method
   override-equivalent signature
   interface
   inheritance
   compile-time error
   inheritance method
   return-type-substitutable method
   throws clause
   error
   method declaration
   compile-time error
   inherited method
   abstract method

.. raw:: pdf

   PageBreak


