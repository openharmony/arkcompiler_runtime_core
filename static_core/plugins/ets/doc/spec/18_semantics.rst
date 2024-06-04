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

.. _Semantic Rules:

Semantic Rules
##############

.. meta:
    frontend_status: Done

This Chapter contains semantic rules to be used throughout the Specification
document.

Note that the description of the rules is more or less informal.
Some details are omitted to simplify the understanding.

|

.. _Subtyping:

Subtyping
*********

.. meta:
    frontend_status: Done

The *subtype* relationship between two types *S* and *T*, where *S* is a subtype
of *T* (recorded as ``S<:T``), means that any object of type *S* can be safely
used in any context in place of an object of type *T*.

The opposite relation is called *supertype* relationship (recorded as ``T:>S``).

By the definition of ``S<:T``, type *T* belongs to the set of *supertypes*
of type *S*. The set of *supertypes* includes all *direct supertypes* (see
below), and all their respective *supertypes*.

.. index::
   subtyping
   subtype
   object
   type
   direct supertype
   supertype

More formally speaking, the set is obtained by reflexive and transitive
closure over the direct supertype relation.

Terms *subclass*, *subinterface*, *superclass*, and *superinterface* are used
when considering class or interface types.

*Direct supertypes* of a non-generic class, or of the interface type *C*
are **all** of the following:

-  Direct superclass of *C* (as mentioned in its extension clause, see
   :ref:`Class Extension Clause`) or type ``Object`` if *C* has no extension
   clause specified.

-  Direct superinterfaces of *C* (as mentioned in the implementation
   clause of *C*, see :ref:`Class Implementation Clause`).

-  Class ``Object`` if *C* is an interface type with no direct superinterfaces
   (see :ref:`Superinterfaces and Subinterfaces`).


.. index::
   direct supertype
   direct superclass
   reflexive closure
   transitive closure
   non-generic class
   extension clause
   implementation clause
   superinterface
   Object
   direct superinterface
   class extension
   subinterface

*Direct supertypes* of the generic type ``C`` <``F``:sub:`1` ``,..., F``:sub:`n`>
(for a generic class or interface type declaration ``C`` <``F``:sub:`1` ``,..., F``:sub:`n`>
with *n*>0) are **all** of the following:

-  Direct superclass of ``C`` <``F``:sub:`1` ``,..., F``:sub:`n`>.

-  Direct superinterfaces of ``C`` <``F``:sub:`1` ``,..., F``:sub:`n`>.

-  Type ``Object`` if ``C`` <``F``:sub:`1` ``,..., F``:sub:`n`> is a generic
   interface type with no direct superinterfaces.


The direct supertype of a type parameter is the type specified as the
constraint of that type parameter.

.. index::
   direct supertype
   generic type
   generic class
   interface type declaration
   direct superinterface
   type parameter
   superclass
   superinterface
   bound
   Object

|

.. _Variance:

Variance
********

.. meta:
    frontend_status: Done

Variance is how subtyping between class types relates to subtyping between
class member signatures (types of parameters, return type).

Variance for generic type arguments is described in :ref:`Type Argument Variance`.

Variance can be of three kinds:

-  Invariance,
-  Covariance, and
-  Contravariance.

.. _Invariance:

Invariance
==========

.. meta:
    frontend_status: Done

*Invariance* refers to the ability to use the originally-specified type as a
derived one.


.. _Covariance:

Covariance
==========

.. meta:
    frontend_status: Done

*Covariance* is the ability to use a type that is more specific than originally
specified.

.. _Contravariance:

Contravariance
==============

.. meta:
    frontend_status: Done

*Contravariance* is the ability to use a type that is more general than
originally specified.

Examples
========

The examples below illustrate valid and invalid usages of variance.
Let class ``Base`` be defined as follows:

.. code-block:: typescript
   :linenos:

   class Base {
      method_one(p: Base): Base {}
      method_two(p: Derived): Base {}
      method_three(p: Derived): Derived {}
   }

Then the code below is valid:

.. code-block:: typescript
   :linenos:

   class Derived extends Base {
      // invariance: parameter type and return type are unchanged
      override method_one(p: Base): Base {}  

      // covariance for the return type: Derived is a subtype of Base  
      override method_two(p: Derived): Derived {}

      // contravariance for parameter types: Base is a super type for Derived
      override method_three(p: Base): Derived {} 
   }

The following code causes compile-time errors:

.. code-block-meta:
   expect-cte

.. code-block:: typescript
   :linenos:

   class Derived extends Base {

      // covariance for parameter types is prohibited
      override method_one(p: Derived): Base {} 

      // contravariance for the return type is prohibited
      override method_tree(p: Derived): Base {}
   }


|

.. _Type Compatibility:

Type Compatibility
******************

.. meta:
    frontend_status: Done

Type *T*:sub:`1` is compatible with type *T*:sub:`2` if:

-  *T*:sub:`1` is the same as *T*:sub:`2`, or

-  There is an *implicit conversion* (see :ref:`Implicit Conversions`)
   that allows converting type *T*:sub:`1` to type *T*:sub:`2`.

*Type compatibility* relationship  is asymmetric, i.e., that *T*:sub:`1`
is compatible with type *T*:sub:`2` does not imply that *T*:sub:`2` is
compatible with type *T*:sub:`1`.

.. index::
   type compatibility
   conversion

|

.. _Type Inference:

Type Inference
**************

.. meta:
    frontend_status: Partly

In spite of the fact that |LANG| supports strong typing, it allows not to
burden the programmer to specify type annotations everywhere. Smart compiler
can infer the type of some entities from the surrounding context. Such
technique called type inference allows to keep the readability of the program
code, to type less and focus on the business logic while keeping the
type-safety. There are several contexts where the type inference can be applied
by the compiler:

- variable and constant declarations (see :ref:`Type Inference from Initializer`)
- implicit generic instantiations (see :ref:`Implicit Generic Instantiations`) 
- function or method return type (see :ref:`Return Type Inference`)
- array literal type inference (see :ref:`Array Type Inference from Context`, 
  :ref:`Array Type Inference from Types of Elements`)
- smart types (see :ref:`Smart Types`)

|

.. _Smart Types:

Smart Types
===========

.. meta:
   frontend_status: Partly
   todo: implement a dataflow check for loops and try-catch blocks

As every data entity - variable (see :ref:`Variable and Constant Declarations`),
class variable (see :ref:`Field Declarations`), or local variable (see
:ref:`Parameter List` and :ref:`Local Declarations`) of some function or method
has its static type, the type which was expliclty specified or inferred at the
point of its declaration. This type defines the set of operations which can
be applied to such entity. Namely what methods can be called and which other
entities can be accessed having this entity as a reciever of the operation.

.. code-block:: typescript
   :linenos:

    let a = new Object
    a.toString() // entity 'a' has method toString()

There could be cases when the type of an entity (mostly local variables) is a
class or interface type (see :ref:`Classes` and :ref:`Interfaces`) or union
type (see :ref:`Union Types`) and in the particular context of the program the
compiler can narrow (smart cast) the static type to a more precise type (smart
type) and allow operations which are specific to such narrowed type.

.. code-block:: typescript
   :linenos:

    let a: number | string = 666
    a++ /* Here we know for sure that type of 'a' is number and number-specific
           operations are type-safe */

    class Base {}
    class Derived extends Base { method () {} }
    let b: base = new Derived
    b.method () /* Here we know for sure that type of 'b' is Derived and Derived-specific
           operations are type-safe */

Other examples are explicit calls to instanceof (see
:ref:`InstanceOf Expression`) or checks against null (see
:ref:`Reference Equality`) as part of if statements (see
:ref:`if Statements`) or conditional expression (see
:ref:`Conditional Expressions`)

.. code-block:: typescript
   :linenos:

    function foo (b: Base, d: Derived|null) {
        if (b instanceof Derived) {
            b.method()
        }
        if (d != null) {
            d.method()
        }
    }

So, for such cases the smart compiler can deduce smart type of an entity and
will not require unnecesary ``as`` conversions (see :ref:`Cast Expressions`).

There are tricky cases related to overloading (see
:ref:`Function and Method Overloading`) when a smart type may lead to the call
of the function or method (see :ref:`Function or Method Selection`) which suits
the smart type of an argument rather than the static one.

.. code-block:: typescript
   :linenos:

    function foo (p: Base) {}
    function foo (p: Derived) {}

    let b: Base = new Derived
    foo (b) // potential ambiguity in case of smart type, foo(p:Base) is to be called
    foo (b as Derived) // no ambiguity,  foo(p:Derived) is to be called

Particular cases supported by the compiler are determined by the compiler
implementation.





|

.. _Overloading and Overriding:

Overloading and Overriding
**************************

There are two important concepts applied to different contexts and entities
throughout this specification.

*Overloading* allows defining and using functions (in general sense, including
methods and constructors) with the same name but different signatures.
The actual function to be called is determined at compile time, and
*overloading* is thus related to compile-time polymorphism.

*Overriding* is closely connected with inheritance, and is applied for methods
but not for functions. Overriding allows a subclass to offer a specific
implementation of a method already defined in its parent class. The actual
method to be called is determined at runtime based on the object's type, and
overriding is thus related to runtime polymorphism.

|LANG| uses two semantic rules:

-  *Overload-equivalence* rule: the *overloading* of two entities is
   correct if their signatures are **not** *overload-equivalent* (see
   :ref:`Overload-Equivalent Signatures`).

-  *Override-compatibility* rule: the *overriding* of two entities is
   correct if their signatures are *override-compatible* (see
   :ref:`Override-Compatible Signatures`).

See :ref:`Overloading for Functions`,
:ref:`Overloading and Overriding in Classes`, and
:ref:`Overloading and Overriding in Interfaces` for details.

|

.. _Overload-Equivalent Signatures:

Overload-Equivalent Signatures
==============================

Signatures *S*:sub:`1` with *n* parameters, and *S*:sub:`2` with *m* parameters
are *overload-equivalent* if:

-  ``n = m``;

-  Parameter type at some position in *S*:sub:`1` is a *type parameter*
   (see :ref:`Type Parameters`), and a parameter type at the same position
   in *S*:sub:`2` is any reference type or type parameter;

-  Parameter type at some position in *S*:sub:`1` is a *generic type*
   *G* <``T``:sub:`1`, ``...``, ``T``:sub:`n`>, and a parameter type at the
   same position in *S*:sub:`2` is also *G* with any list of type arguments;

-  All other parameter types in *S*:sub:`1` are equal
   to parameter types in the same positions in *S*:sub:`2`.

Parameter names and return types do not influence *overload-equivalence*.

The following signatures are *overload-equivalent*:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   (x: number): void
   (y: number): void

and

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   (x: number): void
   (y: number): number
   
and

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class G<T>
   (y: Number): void
   (x: T): void 

and

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class G<T>
   (y: G<Number>): void
   (x: G<T>): void 

and

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class G<T, S>
   (y: T): void
   (x: S): void 

The following signatures are not *overload-equivalent*:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   (x: number): void
   (y: string): void

and

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class A { /*body*/}
   class B extends A { /*body*/}
   (x: A): void
   (y: B): void


|

.. _Override-Compatible Signatures:

Override-Compatible Signatures
==============================

If there are two classes, ``Base`` and ``Derived``, and class ``Derived``
overrides the method ``foo()`` of ``Base``, then ``foo()`` in ``Base`` has
signature ``S``:sub:`1` <``V``:sub:`1` ``, ... V``:sub:`k`>
(``U``:sub:`1` ``, ..., U``:sub:`n`) ``:U``:sub:`n+1`, and ``foo()`` in
``Derived`` has signature ``S``:sub:`2` <``W``:sub:`1` ``, ... W``:sub:`l`>
(``T``:sub:`1` ``, ..., T``:sub:`m`) ``:T``:sub:`m+1` as illustrated by the
example below:

.. code-block:: typescript
   :linenos:

    class Base {
       foo <V1, ... Vk> (p1: U1, ... pn: Un): Un+1
    }
    class Derived extends Base {
       override foo <W1, ... Wl> (p1: T1, ... pm: Tm): Tm+1
    }

The signature ``S``:sub:`2` is override-compatible with ``S``:sub:`1` only
if **all** of the following conditions are met:

1. Number of parameters of both methods is the same, i.e., ``n = m``.
2. Each type ``T``:sub:`i` is override-compatible with type ``U``:sub:`i`
   for ``i`` in ``1..n+1``. Type override compatibility is defined below.
3. Number of type parameters of either method is the same, i.e.,
   ``k = l``.

There are two cases of type override-compatibility, as types are used as either
parameter types, or return types. For each case there are five kinds of types:

- Class/interface type;
- Function type;
- Primitive type;
- Array type; and
- Tuple type.

Every type is override-compatible with itself, and that is a case of invariance
(see :ref:`Invariance`).

Mixed override-compatibility between types of different kinds is always false,
except the compatibility with class type ``Object`` as any type is a subtype of
``Object``.

Variances to be used for types that can be override-compatible in different
positions are represented in the following table:

+-+-----------------------+---------------------+-------------------+
| | **Positions ==>**     | **Parameter Types** | **Return Types**  |
+-+-----------------------+---------------------+-------------------+
| | **Type Kinds**        |                     |                   |
+=+=======================+=====================+===================+
|1| Class/interface types | Contravariance >:   | Covariance <:     |
+-+-----------------------+---------------------+-------------------+
|2| Function types        | Covariance <:       | Contravariance >: |
+-+-----------------------+---------------------+-------------------+
|3| Primitive types       | Invariance          | Invariance        |
+-+-----------------------+---------------------+-------------------+
|4| Array types           | Covariance <:       | Covariance <:     |
+-+-----------------------+---------------------+-------------------+
|5| Tuple types           | Covariance <:       | Covariance <:     |
+-+-----------------------+---------------------+-------------------+

The semantics is illustrated by the example below:

.. code-block:: typescript
   :linenos:

    class Base {
       kinds_of_parameters(
          p1: Derived, p2: (q: Base)=>Derived, p3: number,
          p4: Number, p5: Base[], p6: [Base, Base]
       )
       kinds_of_return_type1(): Base
       kinds_of_return_type2(): (q: Derived)=> Base
       kinds_of_return_type3(): number
       kinds_of_return_type4(): Number 
       kinds_of_return_type5(): Base[] 
       kinds_of_return_type6(): [Base, Base]
    }
    class Derived extends Base {
       // Overriding kinds for parameters
       override kinds_of_parameters(
          p1: Base, // contravaraint parameter type
          p2: (q: Derived)=>Base, // Covariant parameter type, contravariant return type
          p3: Number, // Compile-time error: parameter type is not override-compatible
          p4: number, // Compile-time error: parameter type is not override-compatible
          p5: Derived[], // Covariant array element type
          p6: [Derived, Derived] // Covariant tuple type elements
       )
       // Overriding kinds for return type
       override kinds_of_return_type1(): Derived // Covariant return type
       override kinds_of_return_type2(): (q: Base)=> Derived // Contravariant parameter type, covariant return type
       override kinds_of_return_type3(): Number // Compile-time error: return type is not override-compatible
       override kinds_of_return_type4(): number // Compile-time error: return type is not override-compatible
       override kinds_of_return_type5(): Derived[] // Covariant array element type
       override kinds_of_return_type6(): [Derived, Derived] // Covariant tuple type elements
    }

The example below illustrates override-compatibility with ``Object``:

.. code-block:: typescript
   :linenos:
    
    class Base {
       kinds_of_parameters( // It represents all possible parameter type kinds
          p1: Derived, p2: (q: Base)=>Derived, p3: number,
          p4: Number, p5: Base[], p6: [Base, Base]
       )
       kinds_of_return_type(): Object // It can be overridden by all subtypes except primitive ones
    }
    class Derived extends Base {
       override kinds_of_parameters( // Object is a supertype for all types except primitive ones
          p1: Object, p2: Object,
          p3: Object, // Compile-time error: number and Object are not override-compatible
          p4: Object, p5: Object, p6: Object
       )
    class Derived1 extends Base { 
       override kinds_of_return_type(): Base // Valid overriding
    }
    class Derived2 extends Base {
       override kinds_of_return_type(): (q: Derived)=> Base // Valid overriding
    }
    class Derived3 extends Base {
       override kinds_of_return_type(): number // Compile-time error: number and Object are not override-compatible
    }
    class Derived4 extends Base {
       override kinds_of_return_type(): Number // Valid overriding
    }
    class Derived5 extends Base {
       override kinds_of_return_type(): Base[] // Valid overriding
    }
    class Derived6 extends Base {
       override kinds_of_return_type(): [Base, Base] // Valid overriding
    }

|

.. _Overloading for Functions:

Overloading for Functions
=========================

Only *overloading* must be considered for functions because inheritance for
functions is not defined.

The correctness check for functions overloading is performed if two or more
functions with the same name are accessible (see :ref:`Accessible`) in a scope
(see :ref:`Scopes`). A function can be defined in or imported to the scope.

Semantic check for such two functions is as follows:

- If signatures of such functions are *overload-equivalent*, then
  a compile-time error occurs.

-  Otherwise, *overloading* is valid.

|

.. _Overloading and Overriding in Classes:

Overloading and Overriding in Classes
=====================================

Both *overloading* and *overriding* must be considered in case of classes for
methods and partly for constructors.

**Note**: Only accessible (see :ref:`Accessible`) methods are subject for
overloading and overriding. For example, if a superclass contains a ``private``
method, and a subclass has a method with the same name, then neither overriding
nor overloading is considered.

**Note**: Accessors are considered methods here.

Overriding member may keep or extend the access modifer (see
:ref:`Access Modifiers`) of the inherited or implemented member. Otherwise, a 
compile-time error occurs.

.. code-block:: typescript
   :linenos:

   class Base {
      public public_member() {}
      protected protected_member() {}
      internal internal_member() {}
      private private_member() {}
   }

   interface Interface {
      public_member() // All members are public in interfaces
   }

   class Derived extends Base implements Interface {
      public override public_member() {}
         // Public member can be overriden and/or implemented by the public one
      public override protected_member() {}
         // Protected member can be overriden by the protected or public one
      internal internal_member() {} 
         // Internal member can be overriden by the internal one only
      override private_member() {}
         // A compile-time error occurs if an attempt is made to override private member
   }

Semantic rules that work in various contexts are represented in the following
table:

+-------------------------------------+------------------------------------------+
| **Context**                         | **Semantic Check**                       |
+=====================================+==========================================+
| Two *instance methods*,             | If signatures are *overload-equivalent*, |
| two *static methods* with the same  | then a compile-time error occurs.        |
| name or, two *constructors* are     | Otherwise, *overloading* is used.        |
| defined in the same class.          |                                          |
+-------------------------------------+------------------------------------------+

.. code-block:: typescript
   :linenos:

   class aClass {

      instance_method_1() {}
      instance_method_1() {} // compile-time error: instance method duplication

      static static_method_1() {}
      static static_method_1() {} // compile-time error: static method duplication

      instance_method_2() {}
      instance_method_2(p: number) {} // valid overloading

      static static_method_2() {}
      static static_method_2(p: string) {} // valid overloading

      constructor() {}
      constructor() {} // compile-time error: constructor duplication

      constructor(p: number) {}
      constructor(p: string) {} // valid overloading

   }

+-------------------------------------+------------------------------------------+
| An *instance method* is defined     | If signatures are *override-compatible*, |
| in a subclass with the same name    | then *overriding* is used.               |
| as the *instance method* in a       | Otherwise, *overloading* is used.        |
| superclass.                         |                                          |
+-------------------------------------+------------------------------------------+

.. code-block:: typescript
   :linenos:

   class Base {
      method_1() {}
      method_2(p: number) {}
   }
   class Derived extends Base {
      override method_1() {} // overriding
      method_2(p: string) {} // overloading
   }

+-------------------------------------+------------------------------------------+
| A *static method* is defined        | If signatures are *overload-equivalent*, |
| in a subclass with the same name    | then the static method in the subclass   |
| as the *static method* in a         | *hides* the previous static method.      |
| superclass.                         | Otherwise, *overloading* is used.        |
+-------------------------------------+------------------------------------------+

.. code-block:: typescript
   :linenos:

   class Base {
      static method_1() {}
      static method_2(p: number) {}
   }
   class Derived extends Base {
      static method_1() {} // hiding
      static method_2(p: string) {} // overloading
   }


+-------------------------------------+--------------------------------------------+
| A *constructor* is defined          | All base class constructors are available  |
| in a subclass.                      | for call in all derived class constructors.|
+-------------------------------------+--------------------------------------------+

.. code-block:: typescript
   :linenos:

   class Base {
      constructor() {}
      constructor(p: number) {}
   }
   class Derived extends Base {
      constructor(p: string) {
           super()
           super(5)
      }
   }

|

.. _Overloading and Overriding in Interfaces:

Overloading and Overriding in Interfaces
========================================

.. meta:
    frontend_status: Done

+-------------------------------------+------------------------------------------+
| **Context**                         | **Semantic Check**                       |
+=====================================+==========================================+
| An *instance method* is defined     | If signatures are *override-compatible*, |
| in a subinterface with the same     | then *overriding* is used. Otherwise,    |
| name as the *instance method* in    | *overloading* is used.                   |
| the superinterface.                 |                                          |
+-------------------------------------+------------------------------------------+

.. code-block:: typescript
   :linenos:

   interface Base {
      method_1()
      method_2(p: number)
   }
   interface Derived extends Base {
      method_1() // overriding
      method_2(p: string) // overloading
   }

+-------------------------------------+------------------------------------------+
| A *static method* is defined        | If signatures are *overload-equivalent*, |
| in a subinterface with the same     | then the static method in the subclass   |
| name as the *static method* in a    | *hides* the previous static method.      |
| superinterface.                     | Otherwise, *overloading* is used.        |
|                                     |                                          |
+-------------------------------------+------------------------------------------+

.. code-block:: typescript
   :linenos:

   interface Base {
      static method_1() {}
      static method_2(p: number) {}
   }
   interface Derived extends Base {
      static method_1() {} // hiding
      static method_2(p: string) {} // overloading
   }


+-------------------------------------+------------------------------------------+
| Two *instance methods* or           | If signatures are *overload-equivalent*, |
| two *static methods* with the same  | then a compile-time error occurs.        |
| name are defined in the same        | Otherwise, *overloading* is used.        |
| interface.                          |                                          |
+-------------------------------------+------------------------------------------+

.. code-block:: typescript
   :linenos:

   interface anInterface {
      instance_method_1() 
      instance_method_1()  // Compile-time error: instance method duplication

      static static_method_1() {}
      static static_method_1() {} // Compile-time error: static method duplication

      instance_method_2() 
      instance_method_2(p: number)  // Valid overloading

      static static_method_2() {}
      static static_method_2(p: string) {} // Valid overloading

   }

|

.. _Overload Signatures:

Overload Signatures
*******************

|LANG| supports *overload signatures* to ensure better alignment with |TS|
for functions (:ref:`Function Overload Signatures`),
static and instance methods (:ref:`Method Overload Signatures`),
and constructors (:ref:`Constructor Overload Signatures`).

All signatures except the last *implementation signature*
are considered *syntactic sugar*. The compiler only uses the *implementation
signature* as it considers overloading, overriding, shadowing, or calls.

|

.. _Overload Signature Correctness Check:

Overload Signature Correctness Check
====================================

If a function, method, or constructor has several *overload signatures*
that share the same body, then all first signatures without bodies must
*fit* into the *implementation signature* that has the body. Otherwise,
a compile-time error occurs.

Signature *S*:sub:`i` with *n* parameters *fits* into implementation signature
*IS* if **all** of the following conditions are met:

- *S*:sub:`i` has *n* parameters, *IS* has *m* parameters, and:

   -  ``n <= m``;
   -  All ``n`` parameter types in *S*:sub:`i` are compatible (see
      :ref:`Type Compatibility`) with parameter types in the same positions
      in *IS*:sub:`2`;
   -  All *IS* parameters in positions from ``n + 1`` up to ``m`` are optional
      (see :ref:`Optional Parameters`) if ``n < m``.

- *IS* return type is ``void``, then *S*:sub:`i` return type must also be ``void``.

- *IS* return type is not ``void``, then *S*:sub:`i` return type must be
  ``void`` or compatible with the return type of *IS* (see
  :ref:`Type Compatibility`).


Valid overload signatures are illustrated by the examples below:

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    function f1(): void
    function f1(x: number): void
    function f1(x?: number): void {
        /*body*/
    }

    function f2(x: number): void
    function f2(x: string): void
    function f2(x: number | string): void {
        /*body*/
    }

    function f3(x: number): void
    function f3(x: string): number
    function f3(x: number | string): number {
        return 1
    }

Code with compile-time errors is represented in the example below:

.. code-block:: typescript
   :linenos:

    function f4(x: number): void
    function f4(x: boolean): number // This signature does not fit
    function f4(x: number | string): void {
        /*body*/
    }

    function f5(x: number): void
    function f5(x: string): number // Wrong return type
    function f5(x: number | string): void {
        /*body*/
    }

|

.. _Compatibility Features:

Compatibility Features
**********************

Some features are added to |LANG| in order to support smooth |TS| compatibility.
Using this features is not recommended in most cases while doing the
|LANG| programming.

.. index::
   overload signature compatibility
   compatibility

|

.. _Extended Conditional Expressions:

Extended Conditional Expressions
================================

.. meta:
    frontend_status: Done

|LANG| provides extended semantics for conditional-and and conditional-or
expressions to ensure better alignment with |TS|. It affects the semantics of
conditional expressions (see :ref:`Conditional Expressions`), ``while`` and
``do`` statements (see :ref:`While Statements and Do Statements`), ``for``
statements (see :ref:`For Statements`), ``if`` statements (see
:ref:`if Statements`), and assignment (see :ref:`Simple Assignment Operator`).

This approach is based on the concept of *truthiness* that extends the Boolean
logic to operands of non-Boolean types, while the result of an operation (see
:ref:`Conditional-And Expression`, :ref:`Conditional-Or Expression`,
:ref:`Logical Complement`) is kept boolean.
Depending on the kind of the value type, the value of any valid expression can
be handled as ``true`` or ``false`` as described in the table below:

.. index::
   extended conditional expression
   semantic alignment
   conditional-and expression
   conditional-or expression
   conditional expression
   while statement
   do statement
   for statement
   if statement
   truthiness
   Boolean
   truthy
   falsy
   value type

+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| Value Type                           | When ``false``                         | When ``true``                     | |LANG| Code                     |
+======================================+========================================+===================================+=================================+
| ``string``                           | empty string                           | non-empty string                  | ``s.length == 0``               |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| ``boolean``                          | ``false``                              | ``true``                          | ``x``                           |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| ``enum``                             | ``enum`` constant                      | enum constant                     | ``x.getValue()``                |
|                                      |                                        |                                   |                                 |
|                                      | handled as ``false``                   | handled as ``true``               |                                 |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| ``number`` (``double``/``float``)    | ``0`` or ``NaN``                       | any other number                  | ``n != 0 && n != NaN``          |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| any integer type                     | ``== 0``                               | ``!= 0``                          | ``i != 0``                      |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| ``char``                             | ``== 0``                               | ``!= 0``                          | ``c != c'0'``                   |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| let T - is any nonNullish type                                                                                                                      |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| ``T | null``                         | ``== null``                            | ``!= null``                       | ``x != null``                   |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| ``T | undefined``                    | ``== undefined``                       | ``!= undefined``                  | ``x != undefined``              |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| ``T | undefined | null``             | ``== undefined`` or ``== null``        | ``!= undefined`` and ``!= null``  | ``x != undefined && x != null`` |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| Boxed primitive type                 | primitive type is ``false``            | primitive type is ``true``        | ``new Boolean(true) == true``   |
| (``Boolean``, ``Char``, ``Int`` ...) |                                        |                                   | ``new Int (0) == 0``            |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+
| any other nonNullish type            | ``never``                              | ``always``                        | ``new SomeType != null``        |
+--------------------------------------+----------------------------------------+-----------------------------------+---------------------------------+

The example below illustrates the way this approach works in practice. Any
``nonzero`` number is handled as ``true``. The loop continues until it becomes
``zero`` that is handled as ``false``:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    for (let i = 10; i; i--) {
       console.log (i)
    }
    /* And the output will be 
         10
         9
         8
         7
         6
         5
         4
         3
         2
         1
     */

.. index::
   truthy
   falsy
   NaN
   nullish expression
   numeric expression
   conditional-and expression
   conditional-or expression
   loop


.. raw:: pdf

   PageBreak


