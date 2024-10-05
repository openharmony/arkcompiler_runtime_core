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

The *subtype* relationship between the two types ``S`` and ``T``, where ``S``
is a subtype of ``T`` (recorded as ``S<:T``), means that any object of type
``S`` can be safely used in any context to replace an object of type ``T``.
The opposite is called *supertype* relationship (see :ref:`Supertyping`).

By the definition of ``S<:T``, type ``T`` belongs to the set of *supertypes*
of type ``S``. The set of *supertypes* includes all *direct supertypes* (see
below), and all their respective *supertypes*. More formally speaking, the set
is obtained by reflexive and transitive closure over the direct supertype
relation.


.. index::
   subtyping
   subtype
   object
   type
   direct supertype
   supertype

Terms *subclass*, *subinterface*, *superclass*, and *superinterface* are used
when considering class or interface types.

*Direct supertypes* of a non-generic class, or of the interface type ``C``
are **all** of the following:

-  Direct superclass of ``C`` (as mentioned in its extension clause, see
   :ref:`Class Extension Clause`) or type ``Object`` if ``C`` has no extension
   clause specified.

-  Direct superinterfaces of ``C`` (as mentioned in the implementation
   clause of ``C``, see :ref:`Class Implementation Clause`).

-  Class ``Object`` if ``C`` is an interface type with no direct superinterfaces
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

.. _Supertyping:

Supertyping
***********

.. meta:
    frontend_status: Done

The *supertype* relationship between the two types ``T`` and ``S``, where ``T``
is a supertype of ``T`` (recorded as ``T>:S``) is opposite to subtyping (see
:ref:`Subtyping`). *Supertyping* means that any object of type ``S`` can be
safely used in any context to replace an object of type ``T``.

|

.. _Variance:

Variance
********

.. meta:
    frontend_status: Done

Variance is how subtyping between class types relates to subtyping between
class member signatures (types of parameters, return type). Variance can be
of three kinds:

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
If class ``Base`` is defined as follows:

.. code-block:: typescript
   :linenos:

   class Base {
      method_one(p: Base): Base {}
      method_two(p: Derived): Base {}
      method_three(p: Derived): Derived {}
   }

---then the code below is valid:

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

On the contrary, the following code causes compile-time errors:

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

Type ``T``:sub:`1` is compatible with type ``T``:sub:`2` if:

-  ``T``:sub:`1` is the same as ``T``:sub:`2`, or

-  There is an *implicit conversion* (see :ref:`Implicit Conversions`)
   that allows converting type ``T``:sub:`1` to type ``T``:sub:`2`.

*Type compatibility* relationship  is asymmetric, i.e., that ``T``:sub:`1`
is compatible with type ``T``:sub:`2` does not imply that ``T``:sub:`2` is
compatible with type ``T``:sub:`1`.


.. index::
   type compatibility
   conversion

|

.. _Compatibility of Call Arguments:

Compatibility of Call Arguments
*******************************

.. meta:
    frontend_status: Done

The definition of the term *compatible* is found in :ref:`Type Compatibility`.

The following semantic check must be performed for any function, method, or
constructor call:

- Type of any argument (except arguments of a rest parameter) must be
  compatible with the type of the corresponding parameter;

- Type of each argument corresponding to the rest parameter without the spread
  operator (:ref:`Spread Expression`) must be compatible with the element type
  of the rest type parameter;

- If a single argument corresponding to the rest parameter has the spread
  operator (:ref:`Spread Expression`), then the *expression* that follows the
  operator must refer to an array of a type compatible with the type of the
  rest parameter.


.. index::
   function
   method
   constructor
   semantic check
   argument
   rest parameter
   spread operator
   compatible type
   type compatibility
   parameter

|

.. _Type Inference:

Type Inference
**************

.. meta:
    frontend_status: Done

In spite of the fact that |LANG| supports strong typing, it allows not to
burden the programmer to specify type annotations everywhere. Smart compiler
can infer the type of some entities from the surrounding context. This
technique called *type inference* allows keeping program code readability,
typing less, and focusing on the business logic while keeping type-safety.
Type inference can be applied by the compiler in several contexts as follows:

- Variable and constant declarations (see :ref:`Type Inference from Initializer`);
- Implicit generic instantiations (see :ref:`Implicit Generic Instantiations`);
- Function or method return type (see :ref:`Return Type Inference`);
- Lambda expression parameter type (see :ref:`Lambda Signature`);
- Array literal type inference (see :ref:`Array Type Inference from Context`,
  and :ref:`Array Type Inference from Types of Elements`);
- Smart types (see :ref:`Smart Types`).

|

.. _Smart Types:

Smart Types
===========

.. meta:
   frontend_status: Partly
   todo: implement a dataflow check for loops and try-catch blocks

As every data entity--variable (see :ref:`Variable and Constant Declarations`),
class variable (see :ref:`Field Declarations`), or local variable (see
:ref:`Parameter List` and :ref:`Local Declarations`)--of a function or method
has its static type, which is specified explicitly or inferred at the
point of declaration. This type defines the set of operations that can
be applied to the entity (namely, what methods can be called, and what other
entities can be accessed if the entity acts as a receiver of the operation):

.. code-block:: typescript
   :linenos:

    let a = new Object
    a.toString() // entity 'a' has method toString()

There can be cases when the type of an entity (mostly local variables) is a
class or interface type (see :ref:`Classes` and :ref:`Interfaces`), or union
type (see :ref:`Union Types`). In a particular program context, the compiler
can narrow (smart cast) a static type to a more precise type (smart type), and
allow the operations specific to the narrowed type:

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

Other examples are explicit calls to ``instanceof``
(see :ref:`InstanceOf Expression`) or checks against ``null``
(see :ref:`Reference Equality`) as part of ``if`` statements
(see :ref:`if Statements`) or conditional expressions
(see :ref:`Conditional Expressions`):

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

In cases like this, the smart compiler can deduce the smart type of an entity
without requiring unnecessary ``as`` conversions (see :ref:`Cast Expressions`).

Overloading (see :ref:`Function and Method Overloading`) can cause tricky
situations when a smart type leads to the call of a function or a method
(see :ref:`Overload Resolution`) that suits the smart type rather than the
static type of an argument:

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

Two important concepts apply to different contexts and entities throughout
this specification as follows:

#. *Overloading* allows defining and using functions (in general sense,
   including methods and constructors) with the same name but different
   signatures. The actual function to be called is determined at compile
   time. Thus, *overloading* is related to compile-time polymorphism.

#. *Overriding* is closely connected with inheritance. It is used on methods
   but not on functions. Overriding allows a subclass to offer a specific
   implementation of a method already defined in its parent class.
   The actual method to be called is determined at runtime based on the
   object's type. Thus, overriding is related to runtime polymorphism.

|LANG| uses two semantic rules related to these concepts:

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

Signatures *S*:sub:`1` with *n* parameters, and *S*:sub:`2` with *m*
parameters are *overload-equivalent* if:

-  ``n = m``;

-  Parameter type at some position in *S*:sub:`1` is a *type parameter*
   (see :ref:`Type Parameters`), and a parameter type at the same position
   in *S*:sub:`2` is any reference type or type parameter;

-  Parameter type at some position in *S*:sub:`1` is *generic type*
   ``G`` <``T``:sub:`1`, ``...``, ``T``:sub:`n`>, and a parameter type at the
   same position in *S*:sub:`2` is also ``G`` with any list of type arguments
   (see :ref:`Type Arguments`);

-  All other parameter types in *S*:sub:`1` are equal to parameter types
   in the same positions in *S*:sub:`2`.

Parameter names and return types do not influence *overload-equivalence*.
Signatures in the following series are *overload-equivalent*:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   (x: number): void
   (y: number): void


.. code-block-meta:

.. code-block:: typescript
   :linenos:

   (x: number): void
   (y: number): number


.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class G<T>
   (y: Number): void
   (x: T): void


.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class G<T>
   (y: G<Number>): void
   (x: G<T>): void


.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class G<T, S>
   (y: T): void
   (x: S): void

Signatures in the following series are not *overload-equivalent*:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   (x: number): void
   (y: string): void


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
3. Number of type parameters of either method is the same, i.e., ``k = l``.
4. Constraints of ``W``:sub:`1`, ... ``W``:sub:`l` are to be contravariant
   (see :ref:`Contravariance`) to the appropriate constraints of ``V``:sub:`1`,
   ... ``V``:sub:`k`.

There are two cases of type override-compatibility, as types are used as either
parameter types, or return types. There are five kinds of types for each case:

- Class/interface type;
- Function type;
- Primitive type;
- Array type;
- Tuple type; and
- Type parameter.

Every type is override-compatible with itself (see :ref:`Invariance`).

Mixed override-compatibility between types of different kinds is always false,
except the compatibility with class type ``Object`` as any type is a subtype of
``Object``.

The following rule applies in case of generics:

   - Derived class must have type parameter constraints to be type-compatible
     (see :ref:`Type Compatibility`) with the respective type parameter
     constraint in the base type;
   - Otherwise, a :index:`compile-time error` occurs.


.. code-block:: typescript
   :linenos:

   class Base {}
   class Derived extends Base {}
   class A1 <CovariantTypeParameter extends Base> {}
   class B1 <CovariantTypeParameter extends Derived> extends A1<CovariantTypeParameter> {}
       // OK, derived class may have type compatible constraint of type parameters

   class A2 <ContravariantTypeParameter extends Derived> {}
   class B2 <ContravariantTypeParameter extends Base> extends A2<ContravariantTypeParameter> {}
       // Compile-time error, derived class cannot have non-compatible constraints of type parameters



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
|6| Type parameter        | Contravariance >:   | Contravariance >: |
| | constraint            |                     |                   |
+-+-----------------------+---------------------+-------------------+

The semantics is illustrated by the example below:

.. code-block:: typescript
   :linenos:

    class Base {
       kinds_of_parameters <T extends Derived, U extends Base>(
          p1: Derived, p2: (q: Base)=>Derived, p3: number,
          p4: Number, p5: Base[], p6: [Base, Base], p7: T, p8: U
       )
       kinds_of_return_type1(): Base
       kinds_of_return_type2(): (q: Derived)=> Base
       kinds_of_return_type3(): number
       kinds_of_return_type4(): Number
       kinds_of_return_type5(): Base[]
       kinds_of_return_type6(): [Base, Base]
       kinds_of_return_type7 <T extends Derived>(): T
    }
    class Derived extends Base {
       // Overriding kinds for parameters
       override kinds_of_parameters <T extends Base, U extends Object>(
          p1: Base, // contravariant parameter type
          p2: (q: Derived)=>Base, // Covariant parameter type, contravariant return type
          p3: Number, // Compile-time error: parameter type is not override-compatible
          p4: number, // Compile-time error: parameter type is not override-compatible
          p5: Derived[], // Covariant array element type
          p6: [Derived, Derived], // Covariant tuple type elements
          p7: T, // Contravariance for constraints of type parameters
          p8: U  // Contravariance for constraints of type parameters
       )
       // Overriding kinds for return type
       override kinds_of_return_type1(): Derived // Covariant return type
       override kinds_of_return_type2(): (q: Base)=> Derived // Contravariant parameter type, covariant return type
       override kinds_of_return_type3(): Number // Compile-time error: return type is not override-compatible
       override kinds_of_return_type4(): number // Compile-time error: return type is not override-compatible
       override kinds_of_return_type5(): Derived[] // Covariant array element type
       override kinds_of_return_type6(): [Derived, Derived] // Covariant tuple type elements
       override kinds_of_return_type7 <T extends Base> (): T // OK, contravariance for constraints of the return type
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

*Overloading* must only be considered for functions because inheritance for
functions is not defined.

The correctness check for functions overloading is performed if two or more
functions with the same name are accessible (see :ref:`Accessible`) in a scope
(see :ref:`Scopes`). 

A function can be declared in, or imported to a scope. To prevent uncontrolled
overloading, mixing functions that are declared and imported, or imported from
different compilation units, is not allowed. In particular, a
:index:`compile-time error` occurs to same-name functions if:

-  Functions are imported from different compilation units;

-  Some functions are imported, while others are declared.

It means that only the functions declared in the scope can be overloaded.
The semantic check for these functions is as follows:

-  If signatures of functions are *overload-equivalent*, then
   a :index:`compile-time error` occurs.

-  Otherwise, *overloading* is valid.

|

.. _Overloading and Overriding in Classes:

Overloading and Overriding in Classes
=====================================

Both *overloading* and *overriding* must be considered in case of classes for
methods and partly for constructors.

**Note**: Only accessible (see :ref:`Accessible`) methods are subject for
overloading and overriding. For example, neither overriding nor overloading
is considered if a superclass contains a ``private`` method, and a subclass
has a method with the same name. Accessors are considered methods here.

An overriding member can keep or extend the access modifier (see
:ref:`Access Modifiers`) of the inherited or implemented member. Otherwise, a
:index:`compile-time error` occurs:

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
         // Public member can be overridden and/or implemented by the public one
      public override protected_member() {}
         // Protected member can be overridden by the protected or public one
      internal internal_member() {}
         // Internal member can be overridden by the internal one only
      override private_member() {}
         // A compile-time error occurs if an attempt is made to override private member
   }

Semantic rules that work in various contexts are represented in the following
table:

+-------------------------------------+----------------------------------------------+
| **Context**                         | **Semantic Check**                           |
+=====================================+==============================================+
| Two *instance methods*,             | If signatures are *overload-equivalent*,     |
| two *static methods* with the same  | (see :ref:`Overload-Equivalent Signatures`), |
| name, or two *constructors* are     | then a :index:`compile-time error`           |
| defined in the same class.          | occurs. Otherwise, *overloading* is used.    |
+-------------------------------------+----------------------------------------------+

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

+-------------------------------------+---------------------------------------------+
| An *instance method* is defined     | If signatures are *override-compatible*     |
| in a subclass with the same name    | (see :ref:`Override-Compatible Signatures`),|
| as the *instance method* in a       | then *overriding* is used.                  |
| superclass.                         | Otherwise, *overloading* is used.           |
+-------------------------------------+---------------------------------------------+

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

+-------------------------------------+---------------------------------------------+
| A *static method* is defined        | If signatures are *overload-equivalent*     |
| in a subclass with the same name    | (see :ref:`Overload-Equivalent Signatures`),|
| as the *static method* in a         | then the static method in the subclass      |
| superclass.                         | *hides* the previous static method.         |
|                                     | Otherwise, *overloading* is used.           |
+-------------------------------------+---------------------------------------------+

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

+-------------------------------------+---------------------------------------------+
| **Context**                         | **Semantic Check**                          |
+=====================================+=============================================+
| A method is defined                 | If signatures are *override-compatible*     |
| in a subinterface with the same     | (see :ref:`Override-Compatible Signatures`),|
| name as the method in               | then *overriding* is used. Otherwise,       |
| the superinterface.                 | *overloading* is used.                      |
+-------------------------------------+---------------------------------------------+

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
| Two methods with the same           | A :index:`compile-time error` occurs.    |
| name are defined in the same        | Otherwise, *overloading* is used.        |
| interface.                          |                                          |
+-------------------------------------+------------------------------------------+

.. code-block:: typescript
   :linenos:

   interface anInterface {
      instance_method_1()
      instance_method_1()  // Compile-time error: instance method duplication

      instance_method_2()
      instance_method_2(p: number)  // Valid overloading
   }

|

.. _Overload Resolution:

Overload Resolution
*******************

.. meta:
    frontend_status: Done

The *overload resolution* is used to select one entity to call from a set of
*potentially applicable candidates* in a function, method, or constructor call.

The overload resolution is performed in two steps as follows:

#. Select *applicable candidates* from *potentially applicable candidates*;

#. If there is more than one *applicable candidate*, then select the best
   candidate.

**Note**: The first step is performed in all cases, even if there is
only one *applicable candidate* to check *call signature compatibility*.

|

.. _Selection of Applicable Candidates:

Selection of Applicable Candidates
==================================

.. meta:
    frontend_status: Partly
    todo: adapt the implementation to the latest specification (handle rest, union, functional types properly)
    todo: make the ISA/assembler/runtime handle union types without collision - eg foo(arg: A|B) and foo(arg: C|D)

The selection of *applicable candidates* is the process of checking
:ref:`Compatibility of Call Arguments` for all entities from the set of
*potentially applicable candidates*. If any argument is not compatible with
the corresponding parameter type, then the entity is deleted from the set.

**Note**: Compile-time errors are not reported on this stage.

After processing all entities, one of the following results is achieved:

- Set is empty (all entities are deleted). A compile-time error occurs,
  and the *overload resolution* is completed.

- Only one entity is left in the set. This is the entity to call, and
  the *overload resolution* is completed.

- More than one entity is left in the set. The next step of the
  *overload resolution* is to be performed.

Two overloaded functions are considered in the following example:

.. code-block:: typescript
   :linenos:

   class Base { }
   class Derived extends Base { }

   function foo(p: Base) { ... }     // #1
   function foo(p: Derived) { ... }  // #2

   foo(new Derived) // two applicable candidates for this call
                    // next step of overload resolution is required

   foo(new Base)    // one applicable candidate
                    // overload resolution is completed
                    // #1 will be called

   foo(new Base, 5) // no candidates, compile-time error

|

.. _Selection of Best Candidate:

Selection of Best Candidate
===========================

.. meta:
    frontend_status: Partly

If the set of *applicable candidates* has two or more candidates, then the
best candidate for the given list of arguments is to be identified, if possible.

The selection of the best candidate is based on the following:

- There are no candidates with the same list of parameters, as this situation
  is already forbidden by the compiler (on declaration or import site);

- If several candidates can be called correctly by using the same argument list,
  then the same implicit argument transformations must be applied to make the
  call.

Possible argument transformations are listed below:

- :ref:`Implicit Conversions`;

- Passing default values to fill any missing arguments
  (:ref:`Optional Parameters`);

- Passing the empty array to replace a rest parameter that has no argument;

- Folding several arguments to the array for a rest parameter.

The examples of transformations are presented below:

.. code-block:: typescript
   :linenos:

   function foo1(x: number) {}
   foo1(1) // implicit conversion int -> double

   function foo2(x: Int) {}
   foo2(1) // implicit boxing

   function foo3(x?: string) {}
   foo3() // passing default value -> foo(undefined)

   function foo4(...x: int[]) {}
   foo4()     // passing empty array -> foo([])
   foo4(1, 2) // folding to array -> foo(...[1, 2])

The candidate that does not require transformations for all arguments is the
*best candidate*. Other candidates are not considered.

The examples below represent the best candidate selected without
transformation:

.. code-block:: typescript
   :linenos:

   function foo(i: int)    // #1
   function foo(n: number) // #2

   let x: int = 1
   foo(x) // #1 - is the best candidate, no transformations

   function goo(s: string)  // #1
   function goo(s?: string) // #2

   goo("abc") // #1 - is the best candidate, no transformations

   let x: string|undefined = "abc"
   goo(x) // #2 - is the best candidate, no transformations


If there is no such candidate, then each argument transformation of each
candidate is compared (taking optional and rest parameters into the account)
by calculating partial *better* relation:

**Case 1**. No transformation is *better* than any transformation.

**Case 2**. If argument type is of a numeric type, char, or its boxed
counterpart, then the candidate with a *shorter* conversion is *better*. E.g.,
the conversion of ``int`` to ``float`` is *better* than ``int`` to ``double``,
and ``int`` to ``Int`` is *better* than ``int`` to ``Long``.

**Case 3**. In case of optional parameters, no parameter is *better*.

**Case 4**. If the first candidate has several parameters, and the other
candidate has a rest parameter for the same arguments, then the first one
is *better*.

**Case 5**. All other variants are considered *not comparable*.

.. code-block:: typescript
   :linenos:

   // Case 1:
   function foo(n: number, s: string|null)  // #1
   function foo(n: number, s: string)       // #2

   goo(1, "abc") // #2 is better, no transformation for 2nd argument

   // Case 2:
   function foo(i: long)  // #1
   function foo(n: float) // #2

   let x: int = 1
   foo(x) //  #1 is better, conversion is shorter

   // Case 3:
   function foo(n: number, s?: string)  // #1
   function foo(n: number)              // #2

   foo(1) // #2 is better, less parameters

   // Case 4:
   function foo(sum: number, a: number, b: number)  // #1
   function foo(sum: number, ...x: number[])        // #2

   foo(1, 2, 3) // #1 is better, non-rest parameters

   // Case 5:
   class Base { }
   class Derived extends Base { }

   function foo(p: Base) { ... }     // #1
   function foo(p: Derived) { ... }  // #2

   foo(new Derived) // not comparable, no one is better

If there is exactly one candidate that is *better* than others for at least
one argument and *not comparable* to other arguments, then this one is the
*best candidate* that is to be called.

If no candidate is the *best candidate*, then a :index:`compile-time error`
occurs. Examples of error cases are presented below:

.. code-block:: typescript
   :linenos:

   class Base { }
   class Derived extends Base { }

   function foo(p: Base) { ... }     // #1
   function foo(p: Derived) { ... }  // #2

   foo(new Derived) // compile-time error, as
                    // there is no argument where one candidate is better

   function goo(a: int; b: float)  // #1
   function goo(a: float, b: int)  // #2

   goo(1, 1) // compile-time error, as
             // #1 is better for 1st argument,
             // #2 is better for 2nd argument.

|

.. _Overload Signatures:

Overload Signatures
*******************

|LANG| supports *overload signatures* to ensure better |TS| alignment for
functions (:ref:`Function Overload Signatures`), static and instance methods
(:ref:`Method Overload Signatures`), and constructors
(:ref:`Constructor Overload Signatures`).

All signatures except the last *implementation signature* are considered
*syntactic sugar*. The compiler uses the *implementation signature* only
as it considers overloading, overriding, shadowing, or calls.

|

.. _Overload Signature Correctness Check:

Overload Signature Correctness Check
====================================

If a function, method, or constructor has several *overload signatures*
that share the same body, then all first signatures without bodies must
*fit* into the *implementation signature* that has the body. Otherwise,
a :index:`compile-time error` occurs.

Signature *S*:sub:`i` with *n* parameters *fits* into implementation signature
*IS* if **all** of the following conditions are met:

- *S*:sub:`i` has *n* parameters, *IS* has *m* parameters, and:

   -  ``n <= m``;
   -  All ``n`` parameter types in *S*:sub:`i` are compatible (see
      :ref:`Type Compatibility`) with parameter types in the same positions
      in *IS*:sub:`2`;
   -  All *IS* parameters in positions from ``n + 1`` up to ``m`` are optional
      (see :ref:`Optional Parameters`) if ``n < m``.

- *IS* return type is ``void`` (see :ref:`Type void`), then *S*:sub:`i` return
  type must also be ``void``.

- *IS* return type is not ``void``, then *S*:sub:`i` return type must be
  ``void`` (see :ref:`Type void`) or compatible with the return type of *IS*
  (see :ref:`Type Compatibility`).


The examples below represent valid overload signatures:

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

The examples below represent code with compile-time errors:

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
Using these features while doing the |LANG| programming is not recommended in
most cases.

.. index::
   overload signature compatibility
   compatibility

|

.. _Extended Conditional Expressions:

Extended Conditional Expressions
================================

.. meta:
    frontend_status: Done

|LANG| provides extended semantics for conditional expressions
to ensure better |TS| alignment.
It affects the semantics of

-  Conditional expressions (see :ref:`Conditional Expressions`,
   :ref:`Conditional-And Expression`, :ref:`Conditional-Or Expression`, and
   :ref:`Logical Complement`);

-  ``while`` and ``do`` statements (see :ref:`While Statements and Do Statements`);

-  ``for`` statements (see :ref:`For Statements`);

-  ``if`` statements (see :ref:`if Statements`);

-  assignment (see :ref:`Simple Assignment Operator`).

**Note:** The extended semantics is to be deprecated in one of the future
versions of the language.

This approach is based on the concept of *truthiness* that extends the Boolean
logic to operands of non-Boolean types.

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

Extended semantics of :ref:`Conditional-And Expression` and
:ref:`Conditional-Or Expression` affects the resultant type of expressions
as follows:

-  A *conditional-and* expression ``A && B`` is of type ``B`` if the result of
   ``A`` is handled as ``true``. Otherwise, it is of type ``A``.

-  A *conditional-or* expression ``A || B`` is of type ``B`` if the result of
   ``A`` is handled as ``false``. Otherwise, it is of type ``A``.


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


