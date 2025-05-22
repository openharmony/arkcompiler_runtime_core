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

.. _Semantic Rules:

Semantic Rules
##############

.. meta:
    frontend_status: Done

This Chapter contains semantic rules to be used throughout the Specification
document. The description of the rules is more or less informal. Some details
are omitted to simplify the understanding.

|

.. _Subtyping:

Subtyping
*********

.. meta:
    frontend_status: Done

*Subtype* relationship between types ``S`` and ``T``, where ``S`` is a
subtype of ``T`` (recorded as ``S<:T``), means that any object of type
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
   closure
   reflexive closure
   transitive closure
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
   clause specified;

-  Direct superinterfaces of ``C`` (as mentioned in the implementation
   clause of ``C``, see :ref:`Class Implementation Clause`); and

-  Class ``Object`` if ``C`` is an interface type with no direct superinterfaces
   (see :ref:`Superinterfaces and Subinterfaces`).

.. index::
   subclass
   subinterface
   superclass
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

-  Direct superclass of ``C`` <``F``:sub:`1` ``,..., F``:sub:`n`>;

-  Direct superinterfaces of ``C`` <``F``:sub:`1` ``,..., F``:sub:`n`>, and

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

.. _Supertyping:

Supertyping
===========

.. meta:
    frontend_status: Done

*Supertype* relationship between types ``T`` and ``S``, where ``T``
is a supertype of ``S`` (recorded as ``T>:S``), is opposite to subtyping (see
:ref:`Subtyping`). *Supertyping* means that any object of type ``S`` can be
safely used in any context to replace an object of type ``T``.

.. index::
   supertype
   supertyping
   context
   type
   object

|

.. _Type Identity:

Type Identity
*************

.. meta:
    frontend_status: Done

*Identity* relation between two types means
that types are indistinguishable. This relation is
commutative and transitive.

Types ``A`` and ``B`` are *identical*, if

- ``A`` is subtype of ``B`` (``A<:B``) and ``B`` is subtype of ``A`` (``A:>B``); or

- ``A`` is defined as ``T[]`` and ``B`` is defined as ``Array<T>``, meaning
  that two syntax form defines identical types.

**Note.** :ref:`Type Alias Declaration` does not create new type, but
only new name to the existing type, so an alias and its base type
are also indistinguishable.

|

.. _Assignability:

Assignability
*************

.. meta:
    frontend_status: Done

Type ``T``:sub:`1` is assignable to type ``T``:sub:`2` if:

-  ``T``:sub:`1` is subtype of ``T``:sub:`2`, or

-  There is an *implicit conversion* (see :ref:`Implicit Conversions`)
   that allows converting value of type ``T``:sub:`1` to type ``T``:sub:`2`.

*Assignability* relationship  is asymmetric, i.e., that ``T``:sub:`1`
is assignable to ``T``:sub:`2` does not imply that ``T``:sub:`2` is
assignable to type ``T``:sub:`1`.

.. index::
   assignability
   conversion
   implicit conversion
   asymmetric relationship

|

.. _Invariance, Covariance and Contravariance:

Invariance, Covariance and Contravariance
*****************************************

.. meta:
    frontend_status: Done

*Variance* is how subtyping between class types relates to subtyping between
class member signatures (types of parameters, return type). Variance can be
of three kinds:

-  Invariance,
-  Covariance, and
-  Contravariance.

.. index::
   variance
   subtyping
   type
   class member signature
   parameter
   return type
   invariance
   covariance
   contravariance

*Invariance* refers to the ability to use the originally-specified type as a
derived one.

.. index::
   invariance

*Covariance* is the ability to use a type that is more specific than originally
specified.

.. index::
   covariance

*Contravariance* is the ability to use a type that is more general than
originally specified.

.. index::
   contravariance

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

      // contravariance for parameter types: Base is a supertype for Derived
      override method_three(p: Base): Derived {}
   }

.. index::
   variance

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

.. _Compatibility of Call Arguments:

Compatibility of Call Arguments
*******************************

.. meta:
    frontend_status: Done

The term *assignable* is defined in :ref:`Assignability`.

The following semantic check must be performed for any function, method, or
constructor call:

- Type of any argument (except arguments of a ``rest`` parameter) must be
  assignable to the type of the corresponding parameter;

- Type of each argument corresponding to the ``rest`` parameter without the
  spread operator (:ref:`Spread Expression`) must be assignable to the element
  type of the array ``rest`` type parameter. If the rest parameter is a tuple,
  then the number of arguments must be equal to the number of tuple elements,
  and argument types must be assignable to the appropriate tuple types;

- If a single argument corresponding to the ``rest`` parameter has the spread
  operator (:ref:`Spread Expression`), then the *expression* that follows the
  operator must refer to one of the following:

    - An array of a type assignable to the type of the array ``rest``
      parameter; or
    - A tuple of types assignable to the proper types of the tuple ``rest``
      parameter.

.. index::
   call argument
   function call
   method call
   constructor call
   semantic check
   argument
   rest parameter
   spread operator
   assignable
   type

|

.. _Type Inference:

Type Inference
**************

.. meta:
    frontend_status: Done

|LANG| supports strong typing but allows not to burden a programmer with the
task of specifying type annotations everywhere. A smart compiler can infer types
of some entities from the surrounding context. This technique called *type
inference* allows keeping type safety and program code readability, doing less
typing, and focusing on business logic.
Type inference can be applied by the compiler in the following contexts:

- Variable and constant declarations (see :ref:`Type Inference from Initializer`);
- Implicit generic instantiations (see :ref:`Implicit Generic Instantiations`);
- Function or method return type (see :ref:`Return Type Inference`);
- Lambda expression parameter type (see :ref:`Lambda Signature`);
- Array literal type inference (see :ref:`Array Literal Type Inference from Context`,
  and :ref:`Array Type Inference from Types of Elements`);
- Smart types (see :ref:`Smart Types`).

.. index::
   strong typing
   annotation
   type inference
   entity
   code readability
   type safety
   context
   variable
   constant
   declaration
   generic instantiation
   function
   method
   return type
   lambda expression
   parameter type
   array literal
   smart type

|

.. _Smart Types:

Smart Types
===========

.. meta:
   frontend_status: Partly
   todo: implement a dataflow check for loops and try-catch blocks

Data entities include the following:

-  Variable (see :ref:`Variable and Constant Declarations`),
-  Class field (see :ref:`Field Declarations`), and
-  Parameter of a function or method (see :ref:`Parameter List`).

Every data entity has its static type, which is specified explicitly or
inferred at the point of declaration. This type defines the set of operations
that can be applied to the entity (namely, what methods can be called, and what
other entities can be accessed if the entity acts as a receiver of the
operation):

.. code-block:: typescript
   :linenos:

    let a = new Object
    a.toString() // entity 'a' has method toString()

.. index::
   entity
   variable
   class variable
   local variable
   function
   method
   static type
   receiver
   access
   declaration

If an entity is class type (see :ref:`Classes`), interface type (see
:ref:`Interfaces`), or union type (see :ref:`Union Types`), then the compiler
can narrow (smart cast) a static type to a more precise type (smart type), and
allow operations that are specific to the type so narrowed:

.. code-block:: typescript
   :linenos:

    let a: number | string = 42
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

.. index::
   type
   entity
   local variable
   interface type
   class type
   union type
   context
   compiler
   narrowing
   smart cast
   smart type
   if statement
   conditional expression

In like cases, a smart compiler can deduce the smart type of an entity without
requiring unnecessary casting conversions (see :ref:`Cast Expressions`).

Overloading (see :ref:`Function, Method and Constructor Overloading`) can cause
tricky situations when a smart type leads to the call of a function or a method
(see :ref:`Overload Resolution`) that suits smart rather than static type of an
argument:

.. code-block:: typescript
   :linenos:

    function foo (p: Base) {}
    function foo (p: Derived) {}

    let b: Base = new Derived
    foo (b) // potential ambiguity in case of smart type, foo(p:Base) is to be called
    foo (b as Derived) // no ambiguity,  foo(p:Derived) is to be called

Particular cases supported by the compiler are determined by the compiler
implementation.

.. index::
   compiler
   smart type
   entity
   casting conversion
   conversion
   function
   method
   static type
   implementation

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
   The actual method to be called is determined at runtime based on object type.
   Thus, overriding is related to runtime polymorphism.

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

.. index::
   overloading
   overriding
   context
   entity
   function
   constructor
   method
   signature
   compile-time polymorphism
   runtime polymorphism
   inheritance
   parent class
   object type
   runtime
   overload-equivalence
   override-compatibility

|

.. _Overload-Equivalent Signatures:

Overload-Equivalent Signatures
==============================

Signatures *S*:sub:`1` with *n* parameters, and *S*:sub:`2` with
the same number of parameters are *overload-equivalent* if:

#. Parameter type at some position in *S*:sub:`1` is a *type parameter*
   (see :ref:`Type Parameters`), and a parameter type at the same position
   in *S*:sub:`2` is any non-generic reference type (including *union type*)
   or *type parameter*;

#. Parameter type at some position in *S*:sub:`1` is *generic type*
   ``G`` <``T``:sub:`1`, ``...``, ``T``:sub:`n`>, where there is at least one
   ``T``:sub:`i` which is a type parameter (see :ref:`Type Parameters`), and a
   parameter type at the same position in *S*:sub:`2` is also ``G`` with any
   list of :ref:`Type Arguments` or a *union type* that contains ``G``;

#. Parameter types at some position in *S*:sub:`1` and *S*:sub:`2`
   are *union types*
   that contain types falling under the cases 1. or 2. above;

#. All other parameter types in *S*:sub:`1` are identical
   (see :ref:`Type Identity`) to parameter types
   in the same positions in *S*:sub:`2` and both parameters are
   of the same kind, so they both are *required* or *optional* or ``rest``.

Parameter names and return types do not influence *overload equivalence*.
Signatures are *overload-equivalent*  in the following examples:

.. index::
   overload-equivalent signature
   signature
   parameter
   type parameter
   reference type
   generic type
   type argument
   overriding
   parameter name
   return type
   overload equivalence

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
   (y: number): void
   (x: T): void

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class G<T>
   (y: G<number>): void
   (x: G<T>): void


.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class G<T, S>
   (y: T): void
   (x: S): void



Signatures are not *overload-equivalent* in the following examples:

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

   class A<T> {
       foo(p: T) {}
       foo(p: T[]) {}
   }

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class Base {}
   class Derived1 extends Base {}
   class Derived2 extends Base {}
   (p: Derived1 | Derived2 ): void
   (p: Base): void

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class G<T>
   (y: G<number>): void
   (x: G<string>): void


|

.. _Override-Compatible Signatures:

Override-Compatible Signatures
==============================

If there are two classes ``Base`` and ``Derived``, and class ``Derived``
overrides the method ``foo()`` of ``Base``, then ``foo()`` in ``Base`` has
signature ``S``:sub:`1` <``V``:sub:`1` ``, ... V``:sub:`k`>
(``U``:sub:`1` ``, ..., U``:sub:`n`) ``:U``:sub:`n+1`, and ``foo()`` in
``Derived`` has signature ``S``:sub:`2` <``W``:sub:`1` ``, ... W``:sub:`l`>
(``T``:sub:`1` ``, ..., T``:sub:`m`) ``:T``:sub:`m+1` as in the example below:

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
   (see :ref:`Invariance, Covariance and Contravariance`) to the appropriate constraints of ``V``:sub:`1`,
   ... ``V``:sub:`k`.

.. index::
   override-compatible signature
   override compatibility
   class
   signature
   method
   parameter
   type
   contravariant
   constraint
   type parameter

There are two cases of type override-compatibility, as types are used as either
parameter types, or return types. Each case has the following kinds of types:

- Class/interface type;
- Function type;
- Primitive type;
- Enum types;
- Union types;
- Array type;
- Tuple type; and
- Type parameter.

Each type is override-compatible with itself (see :ref:`Invariance, Covariance and Contravariance`).

Mixed override-compatibility between types of different kinds is always false,
except the compatibility with class type ``Object`` as any type is a subtype of
``Object``.

The following rule applies to generics:

   - Derived class must have type parameter constraints to be assignable
     (see :ref:`Assignability`) to the respective type parameter
     constraint in the base type;
   - Otherwise, a :index:`compile-time error` occurs.

.. index::
   override compatibility
   parameter type
   class type
   interface type
   function type
   primitive type
   array type
   tuple type
   type parameter
   subtype
   object

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
positions are represented in the table below:

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
|4| Union types           | Contravariance >:   | Contravariance >: |
+-+-----------------------+---------------------+-------------------+
|5| Enum types            | Invariance          | Invariance        |
+-+-----------------------+---------------------+-------------------+
|6| Array types           | Invariance          | Invariance        |
+-+-----------------------+---------------------+-------------------+
|7| Tuple types           | Invariance          | Invariance        |
+-+-----------------------+---------------------+-------------------+
|8| Type parameter        | Contravariance >:   | Contravariance >: |
| | constraint            |                     |                   |
+-+-----------------------+---------------------+-------------------+

.. index::
   variance
   covariance
   contravariance
   invariance
   semantics

The semantics is illustrated by the example below:

.. code-block:: typescript
   :linenos:

    enum E1  {Red, Green, Blue}
    enum E2  {A, B, C}
    interface BaseSuperType {}
    interface Base extends BaseSuperType {
       // Overriding for parameters
       kinds_of_parameters01 <T extends Derived, U extends Base>(p: Derived): void
       kinds_of_parameters02 <T extends Derived, U extends Base>(p: (q: Base)=>Derived): void
       kinds_of_parameters03 <T extends Derived, U extends Base>(p: number): void
       kinds_of_parameters04 <T extends Derived, U extends Base>(p: Number): void
       kinds_of_parameters05 <T extends Derived, U extends Base>(p: T | U): void
       kinds_of_parameters06 <T extends Derived, U extends Base>(p: E1): void
       kinds_of_parameters07 <T extends Derived, U extends Base>(p: Base[]): void
       kinds_of_parameters08 <T extends Derived, U extends Base>(p: [Base, Base]): void
       kinds_of_parameters09 <T extends Derived, U extends Base>(p: T): void
       kinds_of_parameters10 <T extends Derived, U extends Base>(p10: U): void

       // Overriding for return type
       kinds_of_return_type01(): Base
       kinds_of_return_type02(): (q: Derived)=> Base
       kinds_of_return_type03(): number
       kinds_of_return_type04(): Number
       kinds_of_return_type05<T extends Derived, U extends Base>(): T | U
       kinds_of_return_type06(): E1
       kinds_of_return_type07(): Base[]
       kinds_of_return_type08(): [Base, Base]
       kinds_of_return_type09 <T extends Derived>(): T
    }

    interface Derived extends Base {
       // Overriding kinds for parameters, Derived <: Base
       kinds_of_parameters01 <T extends Base, U extends Object>(
          p: Base // contravariant parameter type
       ): void
       kinds_of_parameters02 <T extends Base, U extends Object>(
          p: (q: Derived)=>Base // Covariant parameter type, contravariant return type
       ): void
       kinds_of_parameters03 <T extends Base, U extends Object>(
          p: Number // Compile-time error: parameter type is not override-compatible
       ): void
       kinds_of_parameters04 <T extends Base, U extends Object>(
          p: number // Compile-time error: parameter type is not override-compatible
       ): void
       kinds_of_parameters05 <T extends Base, U extends Object>(
          p: Base | BaseSuperType // contravariant parameter type:  Derived | Base <: Base | BaseSuperType
       ): void
       kinds_of_parameters06 <T extends Base, U extends Object>(
          // p: E2 // Compile-time error: parameter type is not override-compatible
          p: E1 // Invariance parameter type
       ): void
       kinds_of_parameters07 <T extends Base, U extends Object>(
          p: Base[] // Invariant array element type
       ): void
       kinds_of_parameters08 <T extends Base, U extends Object>(
          p: [Derived, Derived] // Compile-time error: parameter type is not override-compatible
       ): void
       kinds_of_parameters09 <T extends Base, U extends Object>(
          p: T // Contravariance for constraints of type parameters
       ): void
       kinds_of_parameters10 <T extends Base, U extends Object>(
          p: U  // Contravariance for constraints of type parameters
       ): void

       // Overriding kinds for return type
       kinds_of_return_type01(): Derived // Covariant return type
       kinds_of_return_type02(): (q: Base)=> Derived // Contravariant parameter type, covariant return type
       //kinds_of_return_type04(): number // Compile-time error: return type is not override-compatible
       kinds_of_return_type05<T extends Base, U extends BaseSuperType>(): T | U
       //kinds_of_return_type06(): E2 // CTE
       kinds_of_return_type06(): E1 // OK
       //kinds_of_return_type07(): Derived[] // Compile-time error
       //kinds_of_return_type08(): [Derived, Derived] // Compile-time error
       kinds_of_return_type09 <T extends Base> (): T // OK, contravariance for constraints of the return type
       kinds_of_return_type09 <T extends Base> (): T // OK, contravariance for constraints of the return type
    }

The example below illustrates override-compatibility with ``Object``:

.. code-block:: typescript
   :linenos:

    interface Base {
       kinds_of_parameters<T extends Derived, U extends Base>( // It represents all possible kinds of parameter type
          p01: Derived,
          p02: (q: Base)=>Derived,
          p03: number,
          p04: Number,
          p05: T | U,
          p06: E1,
          p07: Base[],
          p08: [Base, Base]
       ): void
       kinds_of_return_type(): Object // It can be overridden by all subtypes except primitive ones
    }
    interface Derived extends Base {
       kinds_of_parameters( // Object is a supertype for all types except primitive ones
          p1: Object,
          p2: Object,
          p3: Object, // Compile-time error: number and Object are not override-compatible
          p4: Object,
          p5: Object,
          p6: Object,
          p7: Object,
          p8: Object
       ): void
    }

    interface Derived1 extends Base {
       kinds_of_return_type(): Base // Valid overriding
    }
    interface Derived2 extends Base {
       kinds_of_return_type(): (q: Derived)=> Base // Valid overriding
    }
    interface Derived3 extends Base {
       kinds_of_return_type(): number // Valid overriding
    }
    interface Derived4 extends Base {
       kinds_of_return_type(): number | string // Valid overriding
    }
    interface Derived5 extends Base {
       kinds_of_return_type(): E1 // Valid overriding
    }
    interface Derived6 extends Base {
       kinds_of_return_type(): Base[] // Valid overriding
    }
    interface Derived7 extends Base {
       kinds_of_return_type(): [Base, Base] // Valid overriding
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

A function can be declared in, or imported to a scope.

The semantic check for overloading functions is as follows:

-  If function signatures are *overload-equivalent*, then
   a :index:`compile-time error` occurs.

-  Otherwise, *overloading* is valid.

It is discussed in detail in :ref:`Function Overloading` and
:ref:`Import and Overloading of Function Names`.

.. index::
   overloading
   function
   inheritance
   access
   scope
   import
   compilation unit
   overload-equivalent signature

|

.. _Overloading and Overriding in Classes:

Overloading and Overriding in Classes
=====================================

Both *overloading* and *overriding* must be considered in case of classes for
methods and partly for constructors.

**Note**. Only accessible (see :ref:`Accessible`) methods are subjected to
overloading and overriding. For example, neither overriding nor overloading
is considered if a superclass contains a ``private`` method, and a subclass
has a method with the same name. The same rules also apply to accessors in
case of overriding.

An overriding member can keep or extend an access modifier (see
:ref:`Access Modifiers`) of a member that is inherited or implemented.
Otherwise, a :index:`compile-time error` occurs:

.. index::
   overloading
   inheritance
   overriding
   private
   method
   superclass
   access modifier

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

The table below represents semantic rules that apply in various contexts:

+-------------------------------------+----------------------------------------------+
| **Context**                         | **Semantic Check**                           |
+=====================================+==============================================+
| Two *instance methods*,             | If signatures are *overload-equivalent*,     |
| two *static methods* with the same  | (see :ref:`Overload-Equivalent Signatures`), |
| name, or two *constructors* are     | then a :index:`compile-time error`           |
| defined in the same class.          | occurs. Otherwise, *overloading* is used.    |
+-------------------------------------+----------------------------------------------+

.. index::
   semantic check
   instance method
   method
   static method
   constructor
   overload equivalence
   overloading
   overload-equivalent signature

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

.. index::
   instance method
   static method
   subclass
   superclass
   override-compatible signature
   override-compatibility
   overloading
   hiding
   overriding

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

.. index::
   constructor
   subclass
   class constructor

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

.. index::
   method
   subinterface
   superinterface
   semantic check
   override compatibility
   override-compatible signature

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

*Overload resolution* is used to select one entity to call from a set of
*potentially applicable candidates* in a function, method, or constructor call.
Overload resolution is performed in two steps as follows:

#. Select *applicable candidates* from *potentially applicable candidates*;

#. If there is more than one *applicable candidate*, then select the best
   candidate.

**Note**. The first step is performed in all cases, even if there is
only one *applicable candidate* to check *call signature compatibility*.

.. index::
   overload resolution
   entity
   applicable candidate
   call signature compatibility

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

**Note**. Compile-time errors are not reported at this stage.

After processing all entities, one of the following results is achieved:

- Set is empty (all entities are deleted). A compile-time error occurs,
  and the *overload resolution* is completed.

- Only one entity is left in the set. This is the entity to call, and
  the *overload resolution* is completed.

- More than one entity is left in the set. The next step of the
  *overload resolution* is to be performed.

.. index::
   applicable candidate
   compatibility
   call argument
   parameter type
   overload resolution
   overloaded function

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
  is already forbidden by the compiler (at the place of declaration or import)
  (see :ref:`Overload-Equivalent Signatures`);

- If several candidates can be called correctly by using the same argument list,
  then at least one implicit argument transformation
  must be applied to make the call.

Possible argument transformations are listed below:

- :ref:`Implicit Conversions`;

- Passing default values to fill any missing arguments
  (:ref:`Optional Parameters`);

- Passing the empty array to replace a ``rest`` parameter that has no argument;

- Folding several arguments to the array for a ``rest`` parameter.

.. index::
   applicable candidate
   best candidate
   parameter
   compiler
   import site
   argument transformation
   value
   rest parameter

The examples of transformations are presented below:

.. code-block:: typescript
   :linenos:

   function foo1(x: number) {}
   foo1(1) // implicit conversion int -> number, meaning double

   function foo3(x?: string) {}
   foo3() // passing default value -> foo(undefined)

   function foo4(...x: int[]) {}
   foo4()     // passing empty array -> foo([])
   foo4(1, 2) // folding to array -> foo(...[1, 2])

The candidate that does not require transformations for all arguments is the
*best candidate*. Other candidates are not considered in this case.

The examples below represent the best candidate selected without
transformation:

.. code-block:: typescript
   :linenos:

   function foo(i: int)    // #1
   function foo(n: number) // #2

   foo(1) // #1 - is the best candidate, no transformations

   function max(a: number, b: number)  // #1
   function max(...args: number[]) // #2

   max(1, 2) // #1 - is the best candidate, no transformations

.. index::
   best candidate
   transformation
   argument

If there is no *best candidate* on this step,
then candidates are compared for each
argument (taking default values for optional parameters
and arguments of ``rest`` parameter into the account)
by calculating partial *better* relation.

If for the argument, parameter types and parameter kinds
(required, optional or rest) are the same,
this argument is skipped from comparison. Otherwise:

**Case 1**. No argument is *better* than default value for optional parameter
and empty list for ``rest`` parameter.

**Case 2**. If an argument or several arguments correspond
to non-optional parameters in the first candidate,
and the other candidate has a ``rest`` parameter for these arguments,
then the first one is *better*.

**Case 3**. No transformation for an argument is *better* than any transformation.

**Case 4**. If argument type is of a numeric type (see :ref:`Numeric Types`),
or char then the candidate with a *shorter* conversion
is *better*. E.g., the conversion of ``int`` to ``float`` is *better* than
``int`` to ``double``.

**Case 5**. If transformations are applied to the argument for both first and second
candidates, then both candidates are not *best candidates*.

.. index::
   best candidate
   argument transformation
   numeric type
   char
   conversion
   parameter

.. code-block:: typescript
   :linenos:

   // Case 1:
   function foo(n: number, s?: string)  // #1
   function foo(n: number)              // #2

   foo(1) // #2 is better, less parameters

   function bar(...args: number[])  // #1
   function bar()                   // #2

   bar() // #2 is better, less parameters

   // Case 2:
   function foo(sum: number, a: number, b: number)  // #1
   function foo(sum: number, ...x: number[])        // #2

   foo(1, 2, 3) // #1 is better, non-rest parameters

   // Case 3:
   function foo(n: number, s: string|null)  // #1
   function foo(n: number, s: string)       // #2

   goo(1, "abc") // #2 is better, no transformation for 2nd argument

   // Case 4:
   function foo(i: long)  // #1
   function foo(n: float) // #2

   let x: int = 1
   foo(x) //  #1 is better, conversion is shorter

   // Case 5:
   function foo(n: number)          // #1
   function foo(x: number | string) // #2

   foo(1) // different non-compared transformations - compile-time error

If there is exactly one candidate that is *better* than others for at least
one argument or several arguments and
other arguments were skipped from comparison,
then this is the *best candidate*.

A :index:`compile-time error` occurs:

-  if no candidate is the *best candidate*.

-  if according to some argument the first candidate is better and for other
   argument the other candidate is better.

Example of the last case is presented below:

.. code-block:: typescript
   :linenos:

   function goo(a: int; b: float)  // #1
   function goo(a: float, b: int)  // #2

   goo(1, 1) // compile-time error, as
             // #1 is better for 1st argument,
             // #2 is better for 2nd argument.

.. index::
   best candidate
   argument

|

.. _Initializer Block:

Initializer Block
*****************

.. meta:
    frontend_status: None

*Initializer block* is used in classes (see :ref:`Class Initializer`),
packages (see :ref:`Package Initializer`) , and namespaces (see
:ref:`Namespace Declarations`) to ensure that all their variables (see
:ref:`Variable and Constant Declarations`) and fields (see
:ref:`Field Declarations`) have valid initial values.

Appropriate syntax is presented below:

.. code-block:: abnf

      initializerBlock:
          'static' block
          ;

If an *initializer block* contains a ``return <expression>`` statement (see
:ref:`Return Statements`), then a :index:`compile-time error` occurs.
If the code of an *initializer block* contains an unhandled ``throw`` statement
(see :ref:`Throw Statements`), then a program terminates (see
:ref:`Program Exit`).

These situations are represented in the examples below:

.. code-block:: typescript
   :linenos:

    static {

        throw new Error // No surrounding 'try'

        try { throw new Error }  // 'try' has no catch
        finally {}

        try { throw new Error }
        catch (e) { throw new Error } // No surrounding 'try' in 'catch'

        foo () // Function call throws an error

    }

    function foo() {
        throw new Error // No surrounding 'try'
    }



|

.. _Compatibility Features:

Compatibility Features
**********************

Some features are added to |LANG| in order to support smooth |TS| compatibility.
Using these features while doing the |LANG| programming is not recommended in
most cases.

.. index::
   compatibility

|

.. _Extended Conditional Expressions:

Extended Conditional Expressions
================================

.. meta:
    frontend_status: Done

|LANG| provides extended semantics for conditional expressions
to ensure better |TS| alignment. It affects the semantics of the following:

-  Conditional expressions (see :ref:`Conditional Expressions`,
   :ref:`Conditional-And Expression`, :ref:`Conditional-Or Expression`, and
   :ref:`Logical Complement`);

-  ``while`` and ``do`` statements (see :ref:`While Statements and Do Statements`);

-  ``for`` statements (see :ref:`For Statements`);

-  ``if`` statements (see :ref:`if Statements`).

**Note**. The extended semantics is to be deprecated in one of the future
versions of |LANG|.

The extended semantics approach is based on the concept of *truthiness* that
extends the boolean logic to operands of non-boolean types.

Depending on the type kind of a valid expression, the value of such valid
expression can be handled as ``true`` or ``false`` as described in the table
below:

.. index::
   extended conditional expression
   conditional-and expression
   conditional-or expression
   conditional expression
   while statement
   do statement
   for statement
   if statement
   truthiness

+--------------------------------------+-----------------------------+------------------------------+------------------------------------+
| Value Type Kind                      | When ``false``              | When ``true``                | |LANG| Code Example to Check       |
+======================================+=============================+==============================+====================================+
| ``string``                           | empty string                | non-empty string             | ``s.length == 0``                  |
+--------------------------------------+-----------------------------+------------------------------+------------------------------------+
| ``boolean``                          | ``false``                   | ``true``                     | ``x``                              |
+--------------------------------------+-----------------------------+------------------------------+------------------------------------+
| ``enum``                             | ``enum`` constant           | enum constant                | ``x.valueOf()``                    |
|                                      |                             |                              |                                    |
|                                      | handled as ``false``        | handled as ``true``          |                                    |
+--------------------------------------+-----------------------------+------------------------------+------------------------------------+
| ``number`` (``double``/``float``)    | ``0`` or ``NaN``            | any other number             | ``n != 0 && !isNaN(n)``            |
+--------------------------------------+-----------------------------+------------------------------+------------------------------------+
| any integer type                     | ``== 0``                    | ``!= 0``                     | ``i != 0``                         |
+--------------------------------------+-----------------------------+------------------------------+------------------------------------+
| ``bigint``                           | ``== 0n``                   | ``!= 0n``                    | ``i != 0n``                        |
+--------------------------------------+-----------------------------+------------------------------+------------------------------------+
| ``char``                             | ``== 0``                    | ``!= 0``                     | ``c != c'\u0000'``                 |
+--------------------------------------+-----------------------------+------------------------------+------------------------------------+
| ``null`` or ``undefined``            | ``always``                  | ``never``                    | ``x != null`` or ``x != undefined``|
+--------------------------------------+-----------------------------+------------------------------+------------------------------------+
| Union types                          | when value is ``falsy``     | when value is ``truthy``     | ``x != null`` or ``x != undefined``|
|                                      |                             |                              | for union types with nullish types |
+--------------------------------------+-----------------------------+------------------------------+------------------------------------+
| any other nonNullish type            | ``never``                   | ``always``                   | ``new SomeType != null``           |
+--------------------------------------+-----------------------------+------------------------------+------------------------------------+

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
   NaN
   nullish expression
   numeric expression
   conditional-and expression
   conditional-or expression
   loop

.. raw:: pdf

   PageBreak
