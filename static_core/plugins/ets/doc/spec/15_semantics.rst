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

This Chapter contains semantic rules to be used throughout this Specification
document. The description of the rules is more or less informal. Some details
are omitted to simplify the understanding.

.. index::
   semantic rule

|

.. _Semantic Essentials:

Semantic Essentials
*******************

.. meta:
    frontend_status: Partly

The section gives a brief introduction to the major semantic terms
and their usage in several contexts.

.. index::
   semantic term
   context

|

.. _Type of Standalone Expression:

Type of Standalone Expression
=============================

.. meta:
    frontend_status: Done

*Standalone expression* (see :ref:`Type of Expression`) is an expression for
which there is no target type in the context where the expression is used.

The type of a *standalone expression* is determined as follows:

- In case of :ref:`Numeric Literals`, the type is the default type of a literal:

    - Type of :ref:`Integer Literals` is ``int`` or ``long``;
    - Type of :ref:`Floating-Point Literals` is ``double`` or ``float``.

- In case of :ref:`Constant Expressions`, the type is inferred from operand
  types and operations.

- In case of an :ref:`Array Literal`, the type is inferred from the elements
  (see :ref:`Array Type Inference from Types of Elements`).

- Otherwise, a :index:`compile-time error` occurs. Specifically,
  a :index:`compile-time error` occurs if an *object literal* is used
  as a *standalone expression*.

The situation is represented in the example below:

.. index::
   standalone expression
   type of expression
   expression
   target type
   context
   type
   integer literal
   floating-point literal
   constant expression
   inferred type
   operand type
   operand
   array literal
   object literal

.. code-block:: typescript
   :linenos:

    function foo() {
      1    // type is 'int'
      1.0  // type is 'number'
      [1.0, 2.0]  // type is number[]
      [1, "aa"] // type is (int | string)
    }

|

.. Specifics of Assignment-like Contexts:

Specifics of Assignment-like Contexts
=====================================

*Assignment-like context* (see :ref:`Assignment-like Contexts`) can be
considered as an assignment ``x = expr``, where ``x`` is a left-hand-side
expression, and ``expr`` is a right-hand-side expression. E.g., there is an
implicit assignment of ``expr`` to the formal parameter ``foo`` in the call
``foo(expr)``, and implicit assignments to elements or properties in
:ref:`Array Literal` and :ref:`Object Literal`.

*Assignment-like context* is specific in that the type of a left-hand-side
expression is known, but the type of a right-hand-side expression is not
necessarily known in the context as follows:

-  If the type of a right-hand-side expression is known from the expression
   itself, then the :ref:`Assignability` check is performed as in the example
   below:

.. index::
   assignment-like context
   context
   assignment
   expression
   type
   assign
   assignability

.. code-block:: typescript
   :linenos:

    function foo(x: string, y: string) {
        x = y // ok, assignability is checked
    }

-  Otherwise, an attempt is made to apply the type of the left-hand-side
   expression to the right-hand-side expression. A :index:`compile-time error`
   occurs if the attempt fails as in the example below:

.. code-block:: typescript
   :linenos:

    function foo(x: int, y: double[]) {
        x = 1 // ok, type of '1' is inferred from type of 'x'
        y = [1, 2] // ok, array literal is evaluated as [1.0, 2.0]
    }

.. index::
   assignability
   expression
   type

|

.. Specifics of Variable Initialization Context:

Specifics of Variable Initialization Context
============================================

If the variable or a constant declaration (see
:ref:`Variable and Constant Declarations`) has an explicit type annotation,
then the same rules as for *assignment-like contexts* apply. Otherwise, there
are two cases for ``let x = expr`` (see :ref:`Type Inference from Initializer`)
as follows:

-  The type of the right-hand-side expression is known from the expression
   itself, then this type becomes the type of the variable as in the example
   below:

.. code-block:: typescript
   :linenos:

    function foo(x: int) {
        let y = x // type of 'y' is 'int'
    }

-  Otherwise, the type of ``expr`` is evaluated as type of a standalone
   expression as in the example below:

.. code-block:: typescript
   :linenos:

    function foo() {
        let x = 1 // x is of type 'int' (default type of '1')
        let y = [1, 2] // x is of type 'number[]'
    }

.. index::
   variable
   initialization
   context
   constant declaration
   assignment-like context
   annotation
   declaration
   type inference
   initializer
   expression
   standalone expression
   function

|

.. _Specifics of Numeric Operator Contexts:

Specifics of Numeric Operator Contexts
======================================

.. meta:
    frontend_status: Done

The ``postfix`` and ``prefix`` ``increment`` and ``decrement``
operators evaluate ``byte`` and ``short`` operands without
widening. It is also true for an ``assignment`` operator (considering
``assignment`` as a binary operator).

For other numeric operators, the operands of unary and binary numeric expressions
are widened to a larger numeric
type. The minimum type is ``int``. None of those operators
evaluates values of types ``byte`` and ``short`` without widening. Details of
specific operators are discussed in corresponding sections of the Specification.

.. index::
   numeric operator
   context
   numeric operator context
   operand
   unary numeric expression
   binary numeric expression
   widening
   numeric type
   type

|

.. _Specifics of String Operator Contexts:

Specifics of String Operator Contexts
=====================================

.. meta:
    frontend_status: Done

If one operand of the binary operator ‘`+`’ is of type ``string``, then the
string conversion applies to another non-string operand to convert it to string
(see :ref:`String Concatenation` and :ref:`String Operator Contexts`).

.. index::
   string operator
   string operator context
   context
   operand
   binary operator
   string type
   string conversion
   non-string operand
   string concatenation

|

.. _Other Contexts:

Other Contexts
==============

.. meta:
    frontend_status: Done

The only semantic rule for all other contexts, and specifically for
:ref:`Overriding`, is to use :ref:`Subtyping`.

.. index::
   context
   semantic rule
   overriding
   subtyping

|

.. _Specifics of Type Parameters:

Specifics of Type Parameters
============================

.. meta:
    frontend_status: Done

If the type of a left-hand-side expression in *assignment-like context* is a
type parameter, then it provides no additional information for type inference
even where a type parameter constraint is set.

If the *target type* of an expression is a *type parameter*, then the type of
the expression is inferred as the type of a *standalone expression*.

The semantics is represented in the example below:

.. index::
   type parameter
   type
   assignment-like context
   context
   type inference
   constraint
   target type
   expression
   standalone expression
   semantics

.. code-block:: typescript
   :linenos:

    class C<T extends number> {
        constructor (x: T) {}
    }

    new C(1) // compile-time error

The type of '``1``' in the example above is inferred as ``int`` (default type of
an integer literal). The expression is considered ``new C<int>(1)`` and causes
a :index:`compile-time error` because ``int`` is not a subtype of ``number``
(type parameter constraint).

Explicit type argument ``new C<number>(1)`` must be used to fix the code.

.. index::
   inferred type
   default type
   integer literal
   expression
   subtype
   parameter constraint
   type
   argument

|

.. _Semantic Essentials Summary:

Semantic Essentials Summary
===========================

Major semantic terms are listed below:

- :ref:`Type of Expression`;
- :ref:`Assignment-like Contexts`;
- :ref:`Type Inference from Initializer`;
- :ref:`Numeric Operator Contexts`;
- :ref:`String Operator Contexts`;
- :ref:`Subtyping`;
- :ref:`Assignability`;
- :ref:`Overriding`;
- :ref:`Overloading`;
- :ref:`Type Inference`.

.. index::
   semantics
   type inference
   initializer
   string operator
   context
   subtyping
   assignment-like context
   expression
   assignability
   numeric operator
   overriding
   overloading

|

.. _Subtyping:

Subtyping
*********

.. meta:
    frontend_status: Done

*Subtype* relationship between types ``S`` and ``T``, where ``S`` is a
subtype of ``T`` (recorded as ``S<:T``), means that any object of type
``S`` can be safely used in any context to replace an object of type ``T``.
The opposite relation (recorded as ``T:>S``) is called *supertype* relationship.
Each type is its own subtype and supertype (``S<:S`` and ``S:>S``).

By the definition of ``S<:T``, type ``T`` belongs to the set of *supertypes*
of type ``S``. The set of *supertypes* includes all *direct supertypes*
(discussed in subsections), and all their respective *supertypes*.
More formally speaking, the set is obtained by reflexive and transitive
closure over the direct supertype relation.

The terms *subclass*, *subinterface*, *superclass*, and *superinterface* are
used in the following sections as synonyms for *subtype* and *supertype* 
when considering non-generic classes, generic classes, or interface types.

If a relationship of two types is not described in one of the following
sections, then the types are not related to each other. Specifically, two
:ref:`Resizable Array Types` and two :ref:`Tuple Types` are not related to each
other, except where they are identical (see :ref:`Type Identity`).

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base {}

    function not_a_subtype (
       ab: Array<Base>, ad: Array<Derived>,
       tb: [Base, Base], td: [Derived, Derived],
    ) {
       ab = ad // Compile-time error
       tb = td // Compile-time error
    }



.. index::
   subtyping
   subtype
   subclass
   subinterface
   type
   object
   closure
   supertype
   superclass
   superinterface
   direct supertype
   reflexive closure
   transitive closure
   array type
   array
   resizable array
   fixed-size array
   tuple type

|

.. _Subtyping for Non-Generic Classes and Interfaces:

Subtyping for Non-Generic Classes and Interfaces
================================================


.. meta:
    frontend_status: Partly


``S`` for non-generic classes and interfaces is a direct *subclass* or
*subinterface* of ``T`` (or of ``Object`` type) when one of the following
conditions is true:

-  Class ``S`` is a *direct subtype* of class ``T`` (``S<:T``) if ``T`` is
   mentioned in the ``extends`` clause of ``S`` (see :ref:`Class Extension Clause`):

   .. code-block:: typescript
      :linenos:

      // Illustrating S<:T
      class T {}
      class S extends T {}
      function foo(t: T) {}

      // Using T
      foo(new T)

      // Using S (S<:T)
      foo(new S)

-  Class ``S`` is a *direct subtype* of class ``Object`` (``S<:Object``) if
   ``S`` has no :ref:`Class Extension Clause`:

   .. code-block:: typescript
      :linenos:

      // Illustrating S<:Object
      class S {}
      function foo(o: Object) {}

      // Using Object
      foo(new Object)

      // Using S (S<:Object)
      foo(new S)

-  Class ``S`` is a *direct subtype* of interface ``T`` (``S<:T``) if ``T`` is
   mentioned in the ``implements`` clause of ``S`` (see
   :ref:`Class Implementation Clause`):

   .. code-block:: typescript
      :linenos:

      // Illustrating S<:T
      // S is class, T is interface
      interface T {}
      class S implements T {}
      function foo(t: T) {}
      let s: S = new S

      // Using T
      let t: T = s
      foo(t)

      // Using S (S<:T)
      foo(s)

-  Interface ``S`` is a *direct subtype* of interface ``T`` (``S<:T``) if ``T``
   is mentioned in the ``extends`` clause of ``S`` (see
   :ref:`Superinterfaces and Subinterfaces`):

   .. code-block:: typescript
      :linenos:

      // Illustrating S<:T
      // S is interface, T is interface
      interface T {}
      interface S extends T {}
      function foo(t: T) {}
      let t: T
      let s: S

      // Using T
      class A implements T {}
      t = new A
      foo(t)

      // Using S (S<:T)
      class B implements S {}
      s = new B
      foo(s)

-  Interface ``S`` is a *direct subtype* of class ``Object`` (``S<:Object``) if
   ``S`` has no ``extends`` clause (see :ref:`Superinterfaces and Subinterfaces`).

   .. code-block:: typescript
      :linenos:

      // Illustrating subinterface of Object
      interface S {}
      function foo(o: Object) {}

      // Using Object
      foo(new Object)

      // Using subinterface of Object
      class A implements S {}
      let s: S = new A; 
      foo(s)

.. index::
   subtype
   subclasssubinterface
   supertype
   superclass
   superinterface
   interface type
   implementation
   non-generic class
   extension clause
   implementation clause
   Object
   class extension

|

.. _Subtyping for Generic Classes and Interfaces:

Subtyping for Generic Classes and Interfaces
============================================


.. meta:
    frontend_status: Partly


A *generic class* or *generic interface* is declared as
``C`` <``F``:sub:`1` ``,..., F``:sub:`n`>, where *n*>0 is a *direct subtype* of
another generic class or interface ``T``, if one of the following conditions is
true:

-  ``T`` is a *direct superclass* of ``C`` <``F``:sub:`1` ``,..., F``:sub:`n`>
   mentioned in the ``extends`` clause of ``C``:

   .. code-block:: typescript
      :linenos:

      // T<U, V>  is direct superclass of C<U,V>
      // T<U, V> >: C<U, V>
   
      class T<U, V> {
         foo(p: U|V): U|V { return p }  
      }

      class C<U, V> extends T<U, V> {
         bar(u: U): U { return u }         
      }


      // OK, exact match
      let t: T<int, boolean>  = new T<int, boolean>
      let c: C<int, boolean> = new C<int, boolean>
    

      // OK, assigning to direct superclass
      t =  new C<int, boolean>

      // CTE, cannot assign to subclass
      c =  new T<int, boolean>

-  ``T`` is one of direct superinterfaces of ``C``
   <``F``:sub:`1` ``,..., F``:sub:`n`> (see
   :ref:`Superinterfaces and Subinterfaces`):

   .. code-block:: typescript
      :linenos:

      // Interface I<U, V>  is direct superinterface
      // of J<U,V>, X<U, V>

      interface I<U, V> {
         foo(u: U): U;
         bar(v: V): V;
      }

      // J<U, V> <: I<U, V>
      // since J extebds I 
      interface J<U, V> extends I<U, V>
      {
         foo(u: U): U
         bar(v: V): V
         
         foo1(p: U|V): U|V
      }

      // X<U, V> <: I<U, V>
      // since X implements I
      class X<U, V> implements I<U,V> {
        foo(u: U): U { return u }
        bar(v: V): V { return v }
      }

      // Y<U,V> <: J<U, V>  (directly)
      // Also Y<U, V> <: I<U, V> (transitively)
      class Y<U, V> implements J<U,V> {
        foo(u: U): U { return u }
        bar(v: V): V { return v }

        foo1(p: U|V): U|V { return p }
      }

      let i: I<int, boolean> 
      let j: J<int, boolean>
      let x = new X<int, boolean>
      let y = new Y<int, boolean>

      // OK, assigning to direct supertypes
      i = x
      j = y

      // OK, assigning subinterface (J<:I)
      i = j

      // CTE, cannot assign superinterface (I>:JJ
      j = i

-  ``T`` is type ``Object`` (C<:Object) if
   ``C`` <``F``:sub:`1` ``,..., F``:sub:`n`>
   is either a generic class type with no *direct superclasses*,
   or a generic interface type with no direct superinterfaces:

   .. code-block:: typescript
      :linenos:

      // Object is direct superclass and for C<U,V> 
      // and direct superinrerface for I<U,V>
      //
      class C<U, V> {
         foo(u: U): U { return u }
         bar(v: V): V { return v }
      }
      interface I<U, V> {
         foo(u: U): U { return u }
         bar(v: V): V { return v }
      }

      let o: Object = new Object
      let c: C<int, boolean> = new C<int, boolean>
      let i: I<int, boolean>

      // // example1 - C<U,V> <: Object
      function example1(o: Object) {}

      // OK, example(Object)
      example1(o)
      // OK, C<int, boolean> <: Object
      example1(c)

      // // example2 - I<U,V> <: Object
      function example2(o: Object) {}
      class D<U, V> implements I<U, V> {}
      i = new D<int, boolean>

      // OK, example2(Object)
      example2(o)
      // OK, I<int, boolean> <: Object
      example2(i)

The direct supertype of a type parameter is the type specified as the
constraint of that type parameter.

If type parameters of a generic class or an interface have a variance specified
(see :ref:`Type Parameter Variance`), then the subtyping for instantiations of
the class or interface is determined in accordance with the variance of the
appropriate type parameter. For example,  with generic class 
``G<in T1,out T2>`` the ``G<S,T> <: G<U, V>``
when ``S>:U`` and ``T<:V``

The following code illustrates this:

.. code-block:: typescript
   :linenos:

   // Subtyping illustration for generic with parameter variance

   // U1 <: U0
   class U0 {}
   class U1 extends U0 {}

   // Generic with contravariant parameter
   class E<in T> {}

   let e0: E<U0> = new E<U1> // CTE, E<U0> is subtype of E<U1>
   let e1: E<U1> = new E<U0> // OK, E<U1> is supertype for E<U0>

   // Generic with covariant parameter
   class F<out T> {}

   let f0: F<U0> = new F<U1> // OK, F<U0> is supertype for F<U1>
   let f1: F<U1> = new F<U0> // CTE, F<U1> is subtype of F<U0>


.. index::
   direct supertype
   generic type
   generic class
   generic interface
   interface type declaration
   direct superinterface
   type parameter
   superclass
   supertype
   type
   constraint
   type parameter
   superinterface
   variance
   subtyping
   instantiation
   class
   interface
   bound
   Object

|

.. _Subtyping for Literal Types:

Subtyping for Literal Types
===========================

.. meta:
    frontend_status: Done

Any ``string`` literal type (see :ref:`String Literal Types`) is *subtype* of type
``string``. It affects overriding as shown in the example below:

.. code-block:: typescript
   :linenos:

    class Base {
        foo(p: "1"): string { return "42" }
    }
    class Derived extends Base {
        override foo(p: string): "1" { return "1" }
    }
    // Type "1" <: string

    let base: Base = new Derived
    let result: string = base.foo("1")
    /* Argument "1" (value) is compatible to type "1" and to type string in
       the overridden method
       Function result of type string accepts "1" (value) of literal type "1"
    */

Literal type ``null`` (see :ref:`Literal Types`) is a subtype and a supertype to
itself. Similarly, literal type ``undefined`` is a subtype and a supertype to
itself.

.. index::
   literal type
   subtype
   subtyping
   string type
   overriding
   supertype
   string literal
   null type
   undefined type
   literal type
   supertype

|

.. _Subtyping for Union Types:

Subtyping for Union Types
=========================

.. meta:
    frontend_status: Done

A union type ``U`` participates in a subtyping relationship
(see :ref:`Subtyping`) in the following cases:

1. Union type ``U`` (``U``:sub:`1` ``| ... | U``:sub:`n`) is a subtype of
type ``T`` if each ``U``:sub:`i` is a subtype of ``T``.

.. code-block:: typescript
   :linenos:

    let s1: "1" | "2" = "1"
    let s2: string = s1 // ok

    let a: string | number | boolean = "abc"
    let b: string | number = 42
    a = b // OK
    b = a // compile-time error, boolean is absent is 'b'

    class Base {}
    class Derived1 extends Base {}
    class Derived2 extends Base {}

    let x: Base = ...
    let y: Derived1 | Derived2 = ...

    x = y // OK, both Derived1 and Derived2 are subtypes of Base
    y = x // compile-time error

    let x: Base | string = ...
    let y: Derived1 | string ...
    x = y // OK, Derived1 is subtype of Base
    y = x // compile-time error

.. index::
   union type
   subtyping
   subtype
   type
   string
   boolean

2. Type ``T`` is a subtype of union type ``U``
(``U``:sub:`1` ``| ... | U``:sub:`n`) if for some ``i``
``T`` is a subtype of ``U``:sub:`i`.

.. code-block:: typescript
   :linenos:

    let u: number | string = 1 // ok
    u = "aa" // ok
    u = 1.0  // ok, 1.0 is of type 'number' (double)
    u = 1    // compile-time error, type 'int' is not a subtype of 'number'
    u = true // compile-time error

**Note**. If union type normalization produces a single type, then this type
is used instead of the initial set of union types. This concept is represented
in the example below:

.. index::
   union type
   union type normalization
   subtype
   number type

.. code-block:: typescript
   :linenos:

    let u: "abc" | "cde" | string // type of 'u' is string

|

.. _Subtyping for Function Types:

Subtyping for Function Types
============================

.. meta:
    frontend_status: Done

Function type ``F`` with parameters ``FP``:sub:`1` ``, ... , FP``:sub:`m`
and return type ``FR``  is a *subtype* of function type ``S`` with parameters
``SP``:sub:`1` ``, ... , SP``:sub:`n` and return type ``SR`` if **all** of the
following conditions are met:

-  ``m`` :math:`\leq` ``n``;

-  Parameter type of ``SP``:sub:`i` for each ``i`` :math:`\leq` ``m`` is a
   subtype of parameter type of ``FP``:sub:`i` (contravariance),
   and ``SP``:sub:`i` is:
   -  Rest parameter if ``FP``:sub:`i` is a rest parameter;
   -  Optional parameter if ``FP``:sub:`i` is an optional parameter.

-  Type ``FR`` is a subtype of ``SR`` (covariance).

.. index::
   function type
   subtype
   subtyping
   parameter type
   contravariance
   rest parameter
   parameter
   covariance
   return type
   optional parameter

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base {}

    function check(
       bb: (p: Base) => Base,
       bd: (p: Base) => Derived,
       db: (p: Derived) => Base,
       dd: (p: Derived) => Derived
    ) {
       bb = bd
       /* OK: identical parameter types, and covariant return type */
       bb = dd
       /* Compile-time error: parameter type are not contravariant */
       db = bd
       /* OK: contravariant parameter types, and covariant  return type */

       let f: (p: Base, n: number) => Base = bb
       /* OK: subtype has less parameters */

       let g: () => Base = bb
       /* Compile-time error: less parameters than expected */
    }

    let foo: (x?: number, y?: string) => void = (): void => {} // OK: ``m <= n``
    foo = (p?: number): void => {}                             // OK:  ``m <= n``
    foo = (p1?: number, p2?: string): void => {}               // OK: Identical types
    foo = (p: number): void => {}
          // Compile-time error: 1st parameter in type is optional but mandatory in lambda
    foo = (p1: number, p2?: string): void => {}
          // Compile-time error:  1st parameter in type is optional but mandatory in lambda

.. index::
   type
   parameter type
   covariance
   contravariance
   covariant return type
   contravariant return type
   supertype
   parameter
   lambda

|

.. _Subtyping for Fixed-Size Array Types:

Subtyping for Fixed-Size Array Types
====================================

Subtyping for fixed-size array types is based on subtyping of their element
types. It is formally defined as follows:

``FixedSize<B> <: FixedSize<A>`` if ``B <: A``.

The situation is represented in the following example:

.. code-block:: typescript
   :linenos:

    let x: FixedArray<number> = [1, 2, 3]
    let y: FixedArray<Object> = x // ok, as number <: Object
    x = y // compile-time error

Such subtyping allows array assignments that can lead to ``ArrayStoreError``
at runtime if a value of a type which is not a subtype of an element type of
one array is put into that array by using the subtyping of another array
element type.
Type safety is ensured by runtime checks performed by the runtime system as
represented in the example below:

.. index::
   type
   subtype
   subtyping
   fixed-size array
   fixed-size array type
   array element
   parameter type
   runtime check
   array
   array element type
   type safety
   runtime system

.. code-block:: typescript
   :linenos:

    class C {}
    class D extends C {}

    function foo (ca: FixedArray<C>) {
      ca[0] = new C() // ArrayStoreError if ca refers to FixedArray<D>
    }

    let da: FixedArray<D> = [new D()]

    foo(da) // leads to runtime error in 'foo'

|

.. _Subtyping for Intersection Types:

Subtyping for Intersection Types
================================

Intersection type ``I`` defined as (``I``:sub:`1` ``& ... | I``:sub:`n`)
is a subtype of type ``T`` if ``I``:sub:`i` is a subtype of ``T``
for some *i*.

Type ``T`` is a subtype of intersection type
(``I``:sub:`1` ``& ... | I``:sub:`n`) if ``T`` is a subtype of each
``I``:sub:`i`.

.. index::
   subtype
   subtyping
   intersection type

|

.. _Subtyping for Difference Types:

Subtyping for Difference Types
==============================

Difference type ``A - B`` is a subtype of ``T`` if ``A`` is
a subtype of ``T``.

Type ``T`` is a subtype of the difference type ``A - B`` if ``T`` is
a subtype of ``A``, and no value belongs both to ``T`` and ``B``
(i.e., ``T & B = never``).

.. index::
   subtype
   subtyping
   difference type

|

.. _Type Identity:

Type Identity
*************

.. meta:
    frontend_status: Done

*Identity* relation between two types means that the types are
indistinguishable. Identity relation is symmetric and transitive.
Identity relation for types ``A`` and ``B`` is defined as follows:

- Array types ``A`` = ``T1[]`` and ``B`` = ``Array<T2>`` are identical
  if ``T1`` and ``T2`` are identical.

- Tuple types ``A`` = [``T``:sub:`1`, ``T``:sub:`2`, ``...``, ``T``:sub:`n`] and
  ``B`` = [``U``:sub:`1`, ``U``:sub:`2`, ``...``, ``U``:sub:`m`]
  are identical on condition that:

  - ``n`` is equal to ``m``, i.e., the types have the same number of elements;
  - Every *T*:sub:`i` is identical to *U*:sub:`i` for any *i* in ``1 .. n``.

- Union types ``A`` = ``T``:sub:`1` | ``T``:sub:`2` | ``...`` | ``T``:sub:`n` and
  ``B`` = ``U``:sub:`1` | ``U``:sub:`2` | ``...`` | ``U``:sub:`m`
  are identical on condition that:

  - ``n`` is equal to ``m``, i.e., the types have the same number of elements;
  - *U*:sub:`i` in ``U`` undergoes a permutation after which every *T*:sub:`i`
    is identical to *U*:sub:`i` for any *i* in ``1 .. n``.

- Types ``A`` and ``B`` are identical if ``A`` is a subtype of ``B`` (``A<:B``),
  and ``B`` is  at the same time a subtype of ``A`` (``A:>B``).

**Note.** :ref:`Type Alias Declaration` creates no new type but only a new
name for the existing type. An alias is indistinguishable from its base type.

**Note.** If a generic class or an interface has a type parameter ``T`` while
its method has its own type parameter ``T``, then the two types are different
and unrelated.

.. code-block:: typescript
   :linenos:

   class A<T> {
      data: T
      constructor (p: T) { this.data = p } // OK, as here 'T' is a class type parameter
      method <T>(p: T) {
          this.data = p // compile-time error as 'T' of the class is different from 'T' of the method
      }
   }


.. index::
   type identity
   identity
   indistinguishable type
   permutation
   array type
   tuple type
   union type
   subtype
   type
   type alias
   type alias declaration
   declaration
   base type

|

.. _Assignability:

Assignability
*************

.. meta:
    frontend_status: Done

Type ``T``:sub:`1` is assignable to type ``T``:sub:`2` if:

-  ``T``:sub:`1` is type ``never``;

-  ``T``:sub:`1` is identical to ``T``:sub:`2` (see :ref:`Type Identity`);

-  ``T``:sub:`1` is a subtype of ``T``:sub:`2` (see :ref:`Subtyping`); or

-  *Implicit conversion* (see :ref:`Implicit Conversions`) is present that
   allows converting a value of type ``T``:sub:`1` to type ``T``:sub:`2`.


*Assignability* relationship  is asymmetric, i.e., that ``T``:sub:`1`
is assignable to ``T``:sub:`2` does not imply that ``T``:sub:`2` is
assignable to type ``T``:sub:`1`.

.. index::
   assignability
   assignment
   type
   type identity
   subtyping
   conversion
   implicit conversion
   asymmetric relationship
   value

|

.. _Invariance, Covariance and Contravariance:

Invariance, Covariance and Contravariance
*****************************************

.. meta:
    frontend_status: Done

*Variance* is how subtyping between types relates to subtyping between
derived types, including generic types (See :ref:`Generics`), member
signatures of generic types (type of parameters, return type),
and overriding entities (See :ref:`Override-Compatible Signatures`).
Variance can be of three kinds:

-  Covariance,
-  Contravariance, and
-  Invariance.

.. index::
   variance
   subtyping
   type
   subtyping
   derived type
   generic type
   generic
   signature
   type parameter
   overriding entity
   override-compatible signature
   parameter
   variance
   invariance
   covariance
   contravariance

*Covariance* means it is possible to use a type which is more specific than
originally specified.

.. index::
   covariance
   type

*Contravariance* means it is possible to use a type which is more general than
originally specified.

.. index::
   contravariance
   type

*Invariance* means it is only possible to use the original type, i.e., there is
no subtyping for derived types.

.. index::
   invariance
   type
   subtyping
   derived type

Valid and invalid usages of variance are represented in the examples below.
If class ``Base`` is defined as follows:

.. index::
   variance
   base class

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
   parameter type
   invariance
   covariance
   contravariance
   subtype
   supertype
   override method
   base
   overriding
   method

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


The following semantic checks must be performed to arguments from the left to
the right when checking the validity of any function, method, constructor, or
lambda call:

**Step 1**: All arguments in the form of spread expression (see
:ref:`spread Expression`) are to be linearized recursively to ensure that
no spread expression is left at the call site.

**Step 2**: The following checks are performed on all arguments from left to
right, starting from ``arg_pos`` = 1 and ``par_pos`` = 1:

   if parameter at position ``par_pos`` is of non-rest form, then

      if `T`:sub:`arg_pos` <: `T`:sub:`par_pos`, then increment ``arg_pos`` and ``par_pos``
      else a :index:`compile-time error` occurs, exit Step 2

   else // parameter is of rest form (see :ref:`Rest Parameter`)

      if parameter is of rest_array_form, then

         if `T`:sub:`arg_pos` <: `T`:sub:`rest_array_type`, then increment ``arg_pos``
         else increment ``par_pos``

      else // parameter is of rest_tuple_form

         for `rest_tuple_pos` in 1 .. rest_tuple_types.count do

            if `T`:sub:`arg_pos` <: `T`:sub:`rest_tuple_pos`, then increment ``arg_pos`` and `rest_tuple_pos`
            else if rest_tuple_pos < rest_tuple_types.count, then increment ``rest_tuple_pos``
            else a :index:`compile-time error` occurs, exit Step 2

         end
         increment ``par_pos``

      end

   end

.. index::
   assignability
   call argument
   compatibility
   semantic check
   function call
   method call
   constructor call
   function
   method
   constructor
   rest parameter
   parameter
   spread operator
   spread expression
   array
   tuple
   argument type
   expression
   operator
   assignable type
   increment
   array type
   rest parameter
   check

Checks are represented in the examples below:

.. code-block:: typescript
   :linenos:

    call (...[1, "str", true], ...[ ...123])  // Initial call form

    call (1, "str", true, 123) // To be unfolded into the form with no spread expressions



    function foo1 (p: Object) {}
    foo1 (1)  // Type of '1' must be assignable to 'Object'
              // p becomes 1

    function foo2 (...p: Object[]) {}
    foo2 (1, "111")  // Types of '1' and "111" must be assignable to 'Object'
              // p becomes array [1, "111"]

    function foo31 (...p: (number|string)[]) {}
    foo31 (...[1, "111"])  // Type of array literal [1, "111"] must be assignable to (number|string)[]
              // p becomes array [1, "111"]

    function foo32 (...p: [number, string]) {}
    foo32 (...[1, "111"])  // Types of '1' and "111" must be assignable to 'number' and 'string' accordingly
              // p becomes tuple [1, "111"]

    function foo4 (...p: number[]) {}
    foo4 (1, ...[2, 3])  //
              // p becomes array [1, 2, 3]

    function foo5 (p1: number, ...p2: number[]) {}
    foo5 (...[1, 2, 3])  //
              // p1 becomes 1, p2 becomes array [2, 3]


.. index::
   check
   assignable type
   Object
   string
   array

|


.. _Type Inference:

Type Inference
**************

.. meta:
    frontend_status: Done

|LANG| supports strong typing but allows not to burden a programmer with the
task of specifying type annotations everywhere. A smart compiler can infer
types of some entities and expressions from the surrounding context.
This technique called *type inference* allows keeping type safety and
program code readability, doing less typing, and focusing on business logic.
Type inference is applied by the compiler in the following contexts:

- :ref:`Type Inference for Numeric Literals`;
- Variable and constant declarations (see :ref:`Type Inference from Initializer`);
- Implicit generic instantiations (see :ref:`Implicit Generic Instantiations`);
- Function, method or lambda return type (see :ref:`Return Type Inference`);
- Lambda expression parameter type (see :ref:`Lambda Signature`);
- Array literal type inference (see :ref:`Array Literal Type Inference from Context`,
  and :ref:`Array Type Inference from Types of Elements`);
- Object literal type inference (see :ref:`Object Literal`);
- Smart casts (see :ref:`Smart Casts and Smart Types`).

.. index::
   strong typing
   type annotation
   annotation
   smart compiler
   type inference
   inferred type
   expression
   entity
   surrounding context
   code readability
   type safety
   context
   numeric constant expression
   initializer
   variable declaration
   constant declaration
   generic instantiation
   function return type
   function
   method return type
   method
   lambda
   lambda return type
   return type
   lambda expression
   parameter type
   array literal
   Object literal
   smart type
   smart cast

|

.. _Type Inference for Numeric Literals:

Type Inference for Numeric Literals
===================================

.. meta:
    frontend_status: Partly

The type of a numeric literal (see :ref:`Numeric Literals`) is firstly 
inferred from the context, if the context allows it.
The following contexts allow inference:

- :ref:`Assignment-like Contexts`;
- :ref:`Cast Expression` context.

The default type of a numeric literal is used in all other cases as follows:

- ``int`` or ``long`` for an integer literal (see :ref:`Integer Literals`); or
- ``double`` or ``float`` for a floating-point literal (see
  :ref:`Floating-Point Literals`).

The default type is used in :ref:`Numeric Operator Contexts` as the type of
a literal is not inferred:

.. code-block:: typescript
   :linenos:

    let b: byte = 63 + 64 // compile-time error as type of expression is 'int'
    let s: short = -32767 // compile-time error as type of expression is 'int'

    let x: b = 1 as short // compile-time error as type of expression is 'short'

Type inference is used only where the *target type* is one of the two cases:

- Case 1: *Numeric type* (see :ref:`Numeric Types`); or
- Case 2: Union type (see :ref:`Union Types`) containing at least one
  *numeric type*.

**Case 1: Target type is a numeric type**

Where the *target type* is numeric, the type of a literal is inferred to the
*target type* if one of following conditions is met:

#. *Target type* is equal to or larger then the literal default type;

#. Literal is an *integer literal* with the value that fitting into the range
   of the *target type*; or

#. Literal is an *floating-point literal* with the value fitting into the
   range of the *target type* that is ``float``.


If none of the conditions above is met means that the *target type* is not a
valid type for the literal, and a :index:`compile-time error` occurs.

Type inference for a numeric *target type* is represented in the examples below:

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    let l: long = 1     // ok, target type is larger then literal default type
    let b: byte = 127   // ok, integer literal value fits type 'byte'
    let f: float = 3.14 // ok, floating-point value fits type 'float'
    
    l = 1.0    // compile-time error, 'double' cannot be assigned to 'long'
    b = 128    // compile-time error, value is out of range
    f = 3.4e39 // compile-time error, value is out of range

If *target type* is a union type that contains no numeric type, then the
default type of the literal is used. The following code is valid as 'Object',
i.e., a supertype of a numeric type:

.. code-block:: typescript
   :linenos:

    let x: Object | string = 1 // ok, instance of type 'int' is assigned to 'x'
    
    x = 1.0 // ok, instance of type 'double' is assigned to 'x'

**Case 2: Target type is a union type containing at least one numeric type**

If the *target type* is a union type
(``N``:sub:`1` ``| ... | N``:sub:`k` ``| ... | U``:sub:`n`), where ``k``
:math:`\geq` ``1`` and ``N``:sub:`i` is a numeric type, each ``N``:sub:`i`
is considered the *target type*. Then type inference for numeric target type is
tried, and if:

#. No ``N``:sub:`i` is a valid type for the literal, then the default literal
   type is used;

#. Only a single ``N``:sub:`i` is a valid type for the literal, then
   ``N``:sub:`i` is the inferred type;
   
#. Several ``N``'s are valid types, then checks are performed as follows:

   - If the literal is an *integer literal*, and only one valid type is an
     *integer type*, then this valid type becomes the inferred type;
   
   - If the literal is an *floating-point literal*, and only one valid type is
     a *floating-point type*, the this valid type becomes the inferred type;
   
   - Otherwise, a :index:`compile-time error` due to the ambiguity.

Type inference for a union target type is represented in the examples below:

.. code-block:: typescript
   :linenos:

    let b: byte | Object = 127 // inferred type is 'byte'
    b = 128 // inferred type is 'int' (default literal type)
    
    let c: byte | string = 127 // inferred type is 'byte'
    b = 128 // compile-time error, invalid value
    
    let d: int | double = 1.0 // inferred type is 'double'
    d = 1 // inferred type is 'int'

    let i: byte | long = 128 // inferred type is 'long'
    i = 127 // compile-time error, ambiguity
    
    let f: float | double = 3.4e39 // inferred type is 'double'
    f = 1.0 // compile-time error, ambiguity

.. index::
   numeric type
   inferred type
   target type
   integer type
   context
   type
   union type
   value

|

.. _Return Type Inference:

Return Type Inference
=====================

.. meta:
    frontend_status: Done

A missing function, method, getter, or lambda return type can be inferred from
the function, method, getter, or lambda body. A :index:`compile-time error`
occurs if return type is missing from a native function (see
:ref:`Native Functions`).

The current version of |LANG| allows inferring return types at least under
the following conditions:

-  If there is no return statement, or if all return statements have no
   expressions, then the return type is ``void`` (see :ref:`Type void`). It
   effectively implies that a call to a function, method, or lambda returns
   the value ``undefined``.
-  If there are *k* return statements (where *k* is 1 or more) with
   the same type expression *R*, then ``R`` is the return type.
-  If there are *k* return statements (where *k* is 2 or more) with
   expressions of types ``T``:sub:`1`, ``...``, ``T``:sub:`k`, then ``R`` is the
   *union type* (see :ref:`Union Types`) of these types (``T``:sub:`1` | ... |
   ``T``:sub:`k`), and its normalized version (see :ref:`Union Types Normalization`)
   is the return type. If at least one of return statements has no expression, then
   type ``undefined`` is added to the return type union.
-  If a lambda body contains no return statement but at least one throw statement
   (see :ref:`Throw Statements`), then the lambda return type is ``never`` (see
   :ref:`Type never`).
-  If a function, a method, or a lambda is ``async`` (see
   :ref:`Async Functions and Methods`), a return type is inferred by applying
   the above rules, and the return type ``T`` is not ``Promise``, then the return
   type is assumed to be ``Promise<T>``.

Return type inference is represented in the example below:

.. index::
   return type
   function
   method
   getter
   lambda
   value
   getter return type
   lambda return type
   function return type
   method return type
   native function
   void type
   type inference
   inferred type
   method body
   void type
   return statement
   normalization
   expression type
   expression
   function
   implementation
   compiler
   union type
   never type
   async type
   lambda body

.. code-block:: typescript
   :linenos:

    // Explicit return type
    function foo(): string { return "foo" }

    // Implicit return type inferred as string
    function goo() { return "goo" }

    class Base {}
    class Derived1 extends Base {}
    class Derived2 extends Base {}

    function bar (condition: boolean) {
        if (condition)
            return new Derived1()
        else
            return new Derived2()
    }
    // Return type of bar will be Derived1|Derived2 union type

    function boo (condition: boolean) {
        if (condition) return 1
    }
    // That is a compile-time error as there is an execution path with no return

*Smart types* can appear in the process of return type inference
(see :ref:`Smart Casts and Smart Types`). 
A :index:`compile-time error` occurs if an inferred return type is a :ref:`Type Expression`
that can not be expressed in |LANG|:

.. code-block:: typescript
   :linenos:

    class C{}
    interface I {}
    class D extends C implements I {}
    
    function foo(c: C) {
        return c instanceof I ? c : new D() // compile-time error: inferred type is C & I
    }

.. index::
   union type
   type inference
   inferred type
   smart type
   function
   return type
   execution path
   smart cast

|

.. _Smart Casts and Smart Types:

Smart Casts and Smart Types
***************************

.. meta:
    frontend_status: Partly
    todo: implement a dataflow check for loops and try-catch blocks

|LANG| uses the concept of *smart casts*, meaning that
in some cases the compiler can implicitly cast
a value of a variable to a type which is more specific than
the declared type of the variable.
The more specific type is called *smart type*.
*Smart casts* allow keeping type safety, require less typing from
programmer and improve performance.

Smart casts are applied to local variables
(see :ref:`Variable and Constant Declarations`) and parameters
(see :ref:`Parameter List`), except those that are captured and
modified in lambdas.
Further in the text term *variable* is used for both local variables
and parameters.

.. index::
   smart cast
   smart type
   cast
   value
   variable
   type
   declared type
   type safety
   performance
   local variable
   parameter
   lambda

**Note.** Smart casts are not applied to global variabes and class fields,
as it is hard to track their values.

A variable has a single declared type, and can have different *smart
types* in different contexts. A *smart type* of variable is always
a subtype of its declared type.

*Smart type* is used by the compiler each time the value of a variable is read.
It is never used when a variable value is changed.

The usage and benefits of a *smart type* are represented in the example below:

.. code-block:: typescript
   :linenos:

    class C {}
    class D extends C {
        foo() {}
    }

    function bar(c: C) {
        if (c instanceof D) {
            c.foo() // ok, here smart type of 'c' is 'D', 'foo' is safely called
        }
        c.foo() // compile-time error, 'c' does not have method 'foo'
        (c as D).foo() // no compile-time error, can throw runtime error
    }

.. index::
   smart cast
   smart type
   global variable
   variable
   value
   declared type
   context
   subtype
   runtime
   call

The compiler uses data-flow analysis based on :ref:`Control-flow Graph` to
compute *smart types* (see :ref:`Computing Smart Types`). The following
language features influence the computation:

-  Variable declarations;
-  Variable assignments (a variable initialization is handled as a variable
   declaration combined with an assignment);
-  :ref:`InstanceOf Expression` with variables;
-  Conditional statements and conditional expressions that include:

    -  :ref:`Equality Expressions` of a variable and an expression that
       process string literals, ``null`` value, and ``undefined`` value in
       a specific way.

    -  :ref:`Equality Expressions` of ``typeof v`` and a string literal, where
       ``v`` is a variable.

    -  :ref:`Extended Conditional Expressions`.

-  :ref:`Loop Statements`.

A *smart type* usually takes the form of a :ref:`Type Expression` that can
contain types not otherwise represented in |LANG|, namely:

- :ref:`Intersection Types`;
- :ref:`Difference Types`.

.. index::
   smart type
   data-flow analysis
   control-flow graph
   variable
   variable declaration
   variable assignment
   variable initialization
   assignment
   conditional statement
   conditional expression
   equality expression
   null
   expression
   undefined
   string literal
   loop statement
   extended conditional expression
   intersection type
   difference type

|

.. _Type Expression:

Type Expression
===============

The *type* of an entity is conventionally defined as the set of values
an entity (e.g., a variable) can take, and the set of operators
applicable to that entity. Two types with equal sets of values
and operators are considered equal irrespective of a syntactic form used to
denote the types.

However, in some cases it is useful to distinguish between equal types with
different representation or syntactic form. For example, types ``Object`` and
``Object|C`` are equal but they behave in different ways as a context of
an :ref:`Object Literal`:

.. code-block:: typescript
   :linenos:

    class C {
        num = 1
    }
    function foo(x: Object|C) {}
    function bar(x: Object) {}

    foo({num: 42}} // ok, object literal is of type 'C'
    bar({num: 42}} // compile-time error, Object does not have field 'num'

.. index::
   type expression
   type
   entity
   value
   variable
   operator
   set of values
   syntactic form
   equal type
   context
   object
   object literal
   field

The term *type expression* is used below as notation that consists of type
names and operators on types, namely:

- '``|``' for union operator;
- '``&``' for intersection operator (see :ref:`Intersection Types`);
- '``-``' for difference operator (see :ref:`Difference Types`).

Computing *smart types* is the process of creating, evaluating, and simplifying
*type expressions*.

**Note.** *Intersection types* and *difference types* are semantic (not
syntactic notions) that cannot be represented in |LANG|.

.. index::
   type expression
   entity
   value
   variable
   operator
   syntax
   context
   object literal
   field
   type expression
   notation
   type name
   operator
   type
   intersection type
   difference type
   union operator
   intersection operator
   difference operator
   semantic notion
   syntactic notion

|

.. _Intersection Types:

Intersection Types
==================

An *intersection type* is a type created from other types by using the
intersection operator '``&``'.
The values of intersection type ``A & B`` are all values that belong
to both ``A`` and ``B``. The same applies to the set of operations.

Intersection types cannot be expressed directly in |LANG|. Instead, they appear
as *smart types* of variables in the process of :ref:`Computing Smart Types` as
represented below:

.. code-block:: typescript
   :linenos:

    class C {
        foo() {}
    }
    interface I {
        bar(): void
    }

    function test(i: I) {
        if (i instanceof C) {
            // smart type of 'i' here is of some subtype of 'C' that implements 'I'
            // type expression for this type is I & subtype of C
            i.foo() // ok
            i.bar() // ok
        }
    }

See also :ref:`Subtyping for Intersection Types`.

.. index::
   intersection type
   intersection operator
   value
   set of operators
   operation
   variable
   type
   subtype
   implementation
   subtyping

|

.. _Difference Types:

Difference Types
================

A *difference type* is a type created from two other types by a subtraction
operation, i.e., by using the operator '``-``'.
The values of the difference type ``A - B`` are all values that belong to type
``A`` but not to type ``B``. The same applies to the set of operations.

Difference types in |LANG| cannot be expressed directly. They appear as
*smart types* of variables in the process of :ref:`Computing Smart Types`:

.. code-block:: typescript
   :linenos:

    function foo(x: string | undefined): number {
        if (x == undefined) {
            return 0
        } else {
            // smart type of 'x' here is (string | undefined - undefined) = string,
            // hence, string property can be applied to 'x'
            return x.length
        }
    }

This is discussed in detail in :ref:`Subtyping for Difference Types`.

.. index::
   difference type
   difference operator
   subtraction operation
   operator
   value
   set of operators
   smart type
   variable
   type
   string
   property

|

.. _Computing Smart Types:

Computing Smart Types
=====================

Computing smart types is based on *locations*. A *location* is a set of
variables known to have the same value in a given context.

Two maps are used to specify a context *(l, s)*, where:

-  *l: V* :math:`\rightarrow` *L*, map from variables *V* to locations *L*;

-  *s: L* :math:`\rightarrow` *T*, map from locations to smart types *T*
   ascribed to those locations.

Contexts are computed in relation to nodes of :ref:`Control-flow Graph`.
Control-flow graph (CFG) contains the following kinds of nodes:

-  Nodes for expressions that have results assigned to variables,
   including temporary variables;

-  *Branching nodes* that have true and false branches;

-  *Assuming nodes* that have an assumed condition specified;

-  *Backedge nodes* that mark the transfer of control from the end
   point of a loop to its start point.

.. index::
   smart type
   location
   set of variables
   context
   value
   map
   variable
   node
   control-flow graph (CFG)
   expression
   branching node
   assuming node
   backedge node
   control
   control transfer
   loop
   start point
   branch
   true branch
   false branch

The way maps *(l, s*) are changed when processing specific nodes is described
below.
The notation :math:`x'` is used to denote a map that replaces any previous map
during node evaluation.

At a **variable declaration** ``let v``:

-  ``l(v):={v}``.

At an **assignment to the variable** ``v``: ``v = e``:

-  If *e* is a variable, and no implicit conversions are performed:
   ``l'(v):=l'(e):={v}`` :math:`\cup` ``l(e)``.

-  Otherwise, ``s'(l(v)):=s(N(T((e)))``, where ``T(e)`` is the smart type of *e*,
   and *N(x)* adds to type *x* all types to which type *x* can be converted,
   namely:

    - If *x* is a numeric type, then a larger numeric type;
    - If *x* is an enumeration type, then the *enumeration base type*.

The following table summarizes the contexts for map evaluation at an
*assumption node*:

.. index::
   map
   node
   notation
   node evaluation
   conversion
   variable
   smart type
   type
   numeric type
   enumeration base type
   context
   assumption node

.. list-table::
   :width: 100%
   :widths: 30 35 35
   :header-rows: 1

   * - Branching on
     - On the positive branch
     - On the negative branch
   * - *v* ``instanceof`` ``A``
     - assuming *v* ``instanceof`` ``A``:

       ``s'(l(v)):=s(l(v))&A``

     - assuming !(*v* ``instanceof`` ``A``):

       ``s'(l(v)):=s(l(v))-A``,

   * - *v* ``===`` *str* (string literal)
     - ``s'(l(v)):=str``
     - ``s'(l(v)):=s(l(v))-str``

   * - *v* ``===`` ``undefined``
     - ``s'(l(v)):=undefined``

     - ``s'(l(v)):=s(l(v))-undefined``

   * - *v* ``===`` ``null``
     - ``s'(l(v)):=null``

     - ``s'(l(v)):=s(l(v))-null``

   * - *v* ``==`` ``undefined``
     - ``s'(l(v)):=undefined``

     - ``s'(l(v)):= s(l(v))-undefined-null``

   * - *v* ``==`` ``null``
     - ``s'(l(v)):=null``

     - ``s'(l(v)):= s(l(v))-undefined-null``

   * - *v* ``===`` *ec*, where *ec* is an ``enum`` constant
     - ``s'(l(v)):=ec``
     - ``s'(l(v)):=s(l(v))-ec``
   * - ``typeof`` *v* ``===`` *str*

       See **Note 2** below for evaluation of type *T*.

     - ``s'(l(v)):= s(l(v))&T``
     - ``s'(l(v)) := s(l(v))-T``

   * - *v* ``===`` *e*, where *e* is any expression
     - if *e* is a variable *w*, no implicit conversion occurs,
       and no ``null == undefined`` consideration is involved, then:

       ``l'(v):=l'(w):=l(v)``:math:`\cup` ``l(w)``

       otherwise:

       ``s'(l'(v))=s(l(v))&N(T(e))``

       The definitions of ``T`` and ``N`` are as in the assignment clause.

     - No change

   * - *v* (truthiness check)
     - ``s'(l(v)):=s(l(v)) - (null|undefined|"")``
     - ``s'(l(v)):=s(l(v))&T``,

       where T is union of all types the contain at least one value considered as ``false`` 
       in :ref:`Extended Conditional Expressions`.

.. index::
   positive branch
   negative branch
   string literal
   branching
   conversion
   expression
   value
   truthiness check
   union
   type

**Notes**:

#. In the table above the operator '``===``' can be replaced for '``==``' except
   where '``==``' is used explicitly.

#. For branching on ``typeof`` *v* ``===`` *str*, type *T* is
   evaluated in accordance with the value of *str*:

    -  If ``"boolean"``, then *T* is ``boolean``;

    -  If ``"string"``, then *T* is ``string | char``;

    -  If ``"undefined"``, then *T* is ``undefined``;

    -  If a name of a numeric type, then *T* is this numeric type;

    -  If ``"object"``, then *T* is ``Object - boolean - string - all numeric types``.


At a node that joins two CFG branches, namely ``C``:sub:`1` :math:`\leq` :sub:`1`, ``s``:sub:`1` ``>``,
and ``C``:sub:`2` :math:`\leq` ``l``:sub:`2`, ``s``:sub:`2` ``>``, for each variable *v*:

- ``l'(v):=l``:sub:`1` ``(v)``:math:`\cap` ``l``:sub:`2` ``(v)``; and

- ``s'(l'(v)):=s``:sub:`1` ``(l'``:sub:`1` ``(v))|s``:sub:`2` ``(l'``:sub:`2` ``(v))``.

At each *backedge node*, smart type of each variable attached to the node is set
to its declared type.

.. index::
   operator
   branching
   value
   boolean
   string
   char
   numeric type
   node
   branch
   variable
   declared type
   control-flow graph (CFG)

|

.. _Control-flow Graph:

Control-flow Graph
==================

Computing smart types based on *control-flow graph* that describes
the possible evaluation paths of a function body
(graphs are intraprocedural).

See :ref:`Computing Smart Types` for a list of CFG nodes that influences
computation.

TBD: Describe how each language construct is translated to a CFG fragment.

.. index::
   control-flow graph (CFG)
   smart type
   evaluation path
   function body

|

.. _Type Expression Simplification:

Type Expression Simplification
==============================

The following table summarizes the contexts for *type expression simplification*
transformations to be performed at each node of the CFG:

.. list-table::
   :width: 100%
   :widths: 40 30 30
   :header-rows: 1

   * - Transformation
     - Initial expression
     - Simplified expression
   * - Associativity of '``&``'
     - ``(A&B)&C``
     - ``A&(B&C)``
   * - Commutativity of '``&``'
     - ``A&B``
     - ``B&A``
   * - In case of subtyping ``A<:B`` and '``&``'
     - ``A&B``
     - ``A``
   * -
     - ``A&never``
     - ``never``
   * -
     - ``A&Any``
     - ``A``

   * - Associativity of '``|``'
     - ``(A|B)C``
     - ``A|(B|C)``
   * - Commutativity of '``|``'
     - ``A|B``
     - ``B|A``
   * - In case of subtyping ``A<:B`` and '``|``'
     - ``A|B``
     - ``B``
   * -
     - ``A|never``
     - ``A``
   * -
     - ``A|Any``
     - ``Any``     
   * - Difference with ``never``
     - ``A-never``
     - ``A``
   * - In case of subtyping ``A<:B`` and '``-``'
     - ``A-B``
     - ``never``
   * -
     - ``A-Any``
     - ``never``
   * -
     - ``(B-A)|A``
     - ``B``
   * -
     - ``(A-B)&B``
     - ``never``
   * - Other transformations
     - ``(A&B)-C``
     - ``(A-C)&(B-C)``
   * - 
     - ``(A|B)&C``
     - ``(A&C)|(B&C)``
   * -
     - ``(A|B)-C``
     - ``(A-C)|(B-C)``
   * -
     - ``(A-B)-C``
     - ``(A-(B|C)``


.. index::
   control-flow graph (CFG)
   type expression
   type
   expression
   node
   commutativity
   associativity
   subtyping
   simplification transformation

The following simplifications for object types are also taken into account:

#. If ``A`` and ``B`` are classes and neither transitively extends the other,
   then ``A&B = never``, ``A-B = A``.
#. If ``A`` is a final class that does not implement the interface *I*, then
   ``A&I = never``, ``A-I = A``.
#. If ``A`` is a class or interface, and *U* is ``never`` or ``undefined``, then
   ``A&U = never``, ``A-U = A``.
#. If ``E`` is enum with cases ``E``:sub:`1` ``, ... , E``:sub:`n`, then
   ``E = E``:sub:`1` ``| ... |E``:sub:`n`.

The following normalization procedure is performed for every *smart type* at
every node of CFG where possible:

#. Push *difference types* inside *intersection types* and unions, and
   simplify the the difference.
#. Push *intersection types* inside unions, and simplify the intersections.
#. Simplify the resultant union types.

After a simplification, *smart types* undergo approximation with *difference
types* ``A-B`` recursively replaced by ``A``.

.. index::
   control-flow graph (CFG)
   intersection type
   smart type
   difference type
   union
   union type
   implementation

|

.. _Smart Cast Examples:

Smart Cast Examples
===================

By using variable initializers or an assignment the compiler can narrow
(smart cast) a declared type to a more precise subtype (smart type). It
allows operations that are specific to the subtype:

.. code-block:: typescript
   :linenos:

    function boo() {
        let a: number | string = 42
        a++ /* Smart type of 'a' is number and number-specific
           operations are type-safe */
    }

    class Base {}
    class Derived extends Base { method () {} }
    function goo() {
       let b: Base = new Derived
       b.method () /* Smart type of 'b' is Derived and Derived-specific
           operations can be applied in type-safe way */
    }

.. index::
   assignment
   compiler
   subtype
   type safety
   interface type

Other examples are explicit calls to ``instanceof``
(see :ref:`InstanceOf Expression`) or checks against ``null``
(see :ref:`Equality Expressions`) as parts of ``if`` statements
(see :ref:`if Statements`) or ternary conditional expressions
(see :ref:`Ternary Conditional Expressions`):

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
   call
   instanceof
   null
   if statement
   ternary conditional expression
   expression
   method

In like cases, a smart compiler requires no additional checks or casts (see
:ref:`Cast Expression`) to deduce a smart type of an entity.

:ref:`Overloading` can cause tricky situations
when a smart type results in calling an entity that suits the smart type
rather than a declared type of an argument (see
:ref:`Overload Resolution`):

.. code-block:: typescript
   :linenos:

    class Base {b = 1}
    class Derived extends Base{d = 2}

    function fooBase (p: Base) {}
    function fooDerived (p: Derived) {}

    overload foo { fooDerived, fooBase }

    function too() {
        let a: Base = new Base
        foo (a) // fooBase will be called
        let b: Base = new Derived
        foo (b) // as smart type of 'b' is Derived, fooDerived will be called
    }

.. index::
   smart compiler
   check
   smart type
   entity
   cast expression
   check
   overloading
   overload declaration
   type
   type argument
   function
   overload resolution
   compiler

|

.. _Overriding:

Overriding
**********

*Method overriding* is the language feature closely connected with inheritance.
It allows a subclass or a subinterface to offer a specific
implementation of a method already defined in its supertype optionally
with modified signature.

The actual method to be called is determined at runtime based on object type.
Thus, overriding is related to *runtime polymorphism*.

|LANG| uses the *override-compatibility* rule to check the correctness of
overriding. The *overriding* is correct if method signature in a subtype
(subclass or subinterface) is *override-compatible* with the method defined
in a supertype (see :ref:`Override-Compatible Signatures`).

An implementation is forced to :ref:`Make a Bridge Method for Overriding Method`
in some cases of *method overriding*.

.. index::
   overriding
   method overriding
   subclass
   subinterface
   supertype
   signature
   method signature
   runtime polymorphism
   inheritance
   parent class
   object type
   runtime
   override-compatibility
   override-compatible signature
   implementation
   bridge method
   method overriding

|

.. _Overriding in Classes:

Overriding in Classes
=====================

.. meta:
    frontend_status: Partly

**Note**. Only accessible (see :ref:`Accessible`) methods are subjected to
overriding. The same rule applies to accessors in case of overriding.

An overriding member can keep or extend an access modifier (see
:ref:`Access Modifiers`) of a member that is inherited or implemented.
Otherwise, a :index:`compile-time error` occurs.

A :index:`compile-time error` occurs if an attempt is made to do the following:

- Override a private method of a superclass; or
- Declare a method with the same name as that of a private method with default
  implementation from any superinterface.


.. index::
   overloading
   class
   inheritance
   overriding
   class
   constructor
   accessibility
   access
   private method
   method
   subclass
   accessor
   superclass
   access modifier
   implementation
   superinterface

.. code-block:: typescript
   :linenos:

   class Base {
      public public_member() {}
      protected protected_member() {}
      private private_member() {}
   }

   interface Interface {
      public_member()             // All members are public in interfaces
      private private_member() {} // Except private methods with default implementation
   }

   class Derived extends Base implements Interface {
      public override public_member() {}
         // Public member can be overridden and/or implemented by the public one
      public override protected_member() {}
         // Protected member can be overridden by the protected or public one
      override private_member() {}
         // A compile-time error occurs if an attempt is made to override private member
         // or implement the private methods with default implementation
   }

The table below represents semantic rules that apply in various contexts:

.. index::
   interface
   public
   implementation
   private method

.. list-table::
   :width: 100%
   :widths: 50 50
   :header-rows: 1

   * - Context
     - Semantic Check
   * - An *instance method* is defined in a subclass with the same name as the
       *instance method* in a superclass.
     - If signatures are *override-compatible* (see
       :ref:`Override-Compatible Signatures`), then *overriding* is used.
       Otherwise, a :index:`compile-time error` occurs.

.. index::
   context
   semantic check
   instance method
   subclass
   superclass
   override-compatible signature
   overriding
   override-compatibility

.. code-block:: typescript
   :linenos:

   class Base {
      method_1() {}
      method_2(p: number) {}
   }
   class Derived extends Base {
      override method_1() {} // overriding
      method_2(p: string) {} // compile-time error
   }


.. list-table::
   :width: 100%
   :widths: 50 50
   :header-rows: 0

   * - A *constructor* is defined in a subclass.
     - All base class constructors are available for call in all derived class
       constructors via ``super`` call (see :ref:`Explicit Constructor Call`).

.. code-block:: typescript
   :linenos:

   class Base {
      constructor(p: number) {}
   }
   class Derived extends Base {
      constructor(p: string) {
           super(5)
      }
   }

.. index::
   constructor
   subclass
   class constructor
   super call
   constructor call
   derived class constructor

|

.. _Overriding and Overloading in Interfaces:

Overriding and Overloading in Interfaces
========================================

.. meta:
    frontend_status: Done

.. list-table::
   :width: 100%
   :widths: 50 50
   :header-rows: 1

   * - Context
     - Semantic Check
   * - A method is defined in a subinterface with the same name as the
       method in the superinterface.
     - If signatures are *override-compatible* (see
       :ref:`Override-Compatible Signatures`), then *overriding* is used.
       Otherwise, a :index:`compile-time error` occurs.
   * - A method is defined in a subinterface with the same name as the
       private method in the superinterface.
     - A :index:`compile-time error` occurs.

.. index::
   overriding
   overload signature
   interface
   semantic check
   subinterface
   name
   method
   superinterface
   signature
   override-compatible signature
   override-compatibility
   overriding
   subinterface
   private method

.. code-block:: typescript
   :linenos:

   interface Base {
      method_1()
      method_2(p: number)
      private foo() {} // private method with implementation body
   }
   interface Derived extends Base {
      method_1() // overriding
      method_2(p: string) // compile-time error: non-compatible signature
      foo(p: number): void // compile-time error: the same name as private method
   }


.. list-table::
   :width: 100%
   :widths: 50 50
   :header-rows: 0

   * - Two or more methods with the same name are defined in the same interface.
     - TBD is used.

.. index::
   method
   subinterface
   superinterface
   semantic check
   override-compatible
   override-compatible signature
   signature
   method overload signature
   non-compatible signature
   interface
   private method

.. code-block:: typescript
   :linenos:

   interface anInterface {
      instance_method()          // 1st signature
      instance_method(p: number) // 2nd signature
   }

|

.. _Override-Compatible Signatures:

Override-Compatible Signatures
==============================

.. meta:
    frontend_status: Partly

If there are two classes ``Base`` and ``Derived``, and class ``Derived``
overrides the method ``foo()`` of ``Base``, then ``foo()`` in ``Base`` has
signature ``S``:sub:`1` <``V``:sub:`1` ``, ... V``:sub:`k`>
(``U``:sub:`1` ``, ..., U``:sub:`n`) ``:U``:sub:`n+1`, and ``foo()`` in
``Derived`` has signature ``S``:sub:`2` <``W``:sub:`1` ``, ... W``:sub:`l`>
(``T``:sub:`1` ``, ..., T``:sub:`m`) ``:T``:sub:`m+1` as in the example below:

.. index::
   override-compatible signature
   override-compatibility
   class
   base class
   derived class
   signature

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
2. Each parameter type ``T``:sub:`i` is a supertype of ``U``:sub:`i`
   for ``i`` in ``1..n`` (contravariance).
3. If return type ``T``:sub:`m+1` is ``this``, then ``U``:sub:`n+1` is ``this``,
   or any of superinterfaces or superclass of the current type. Otherwise,
   return type ``T``:sub:`m+1` is a subtype of ``U``:sub:`n+1` (covariance).
4. Number of type parameters of either method is the same, i.e., ``k = l``.
5. Constraints of ``W``:sub:`1`, ... ``W``:sub:`l` are to be contravariant
   (see :ref:`Invariance, Covariance and Contravariance`) to the appropriate
   constraints of ``V``:sub:`1`, ... ``V``:sub:`k`.

.. index::
   signature
   override-compatible signature
   override compatibility
   class
   signature
   method
   parameter
   superinterface
   superclass
   return type
   type
   contravariant
   covariance
   invariance
   constraint
   type parameter

The following rule applies to generics:

   - Derived class must have type parameter constraints to be subtype
     (see :ref:`Subtyping`) of the respective type parameter
     constraint in the base type;
   - Otherwise, a :index:`compile-time error` occurs.

.. index::
   generic
   derived class
   subtyping
   subtype
   type parameter
   base type

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

The semantics is represented in the examples below:

1. **Class/Interface Types**

.. code-block:: typescript
    :linenos:

    interface Base {
        param(p: Derived): void
        ret(): Base
    }

    interface Derived extends Base {
        param(p: Base): void    // Contravariant parameter
        ret(): Derived          // Covariant return type
    }

.. index::
   class type
   semantics
   interface type
   contravariant parameter
   covariant return type

2. **Function Types**

.. code-block:: typescript
    :linenos:

    interface Base {
        param(p: (q: Base)=>Derived): void
        ret(): (q: Derived)=> Base
    }

    interface Derived extends Base {
        param(p: (q: Derived)=>Base): void  // Covariant parameter type, contravariant return type
        ret(): (q: Base)=> Derived          // Contravariant parameter type, covariant return type
    }

.. index::
   function type
   covariant parameter type
   contravariant return type
   contravariant parameter type
   covariant return type

3. **Union Types**

.. code-block:: typescript
   :linenos:

    interface BaseSuperType {}
    interface Base extends BaseSuperType {
       // Overriding for parameters
       param<T extends Derived, U extends Base>(p: T | U): void

       // Overriding for return type
       ret<T extends Derived, U extends Base>(): T | U
    }

    interface Derived extends Base {
       // Overriding kinds for parameters, Derived <: Base
       param<T extends Base, U extends Object>(
          p: Base | BaseSuperType // contravariant parameter type:  Derived | Base <: Base | BaseSuperType
       ): void
       // Overriding kinds for return type
       ret<T extends Base, U extends BaseSuperType>(): T | U
    }

.. index::
   union type
   return type
   parameter
   overriding

4. **Type Parameter Constraint**

.. code-block:: typescript
    :linenos:

    interface Base {
        param<T extends Derived>(p: T): void
        ret<T extends Derived>(): T
    }

    interface Derived extends Base {
        param<T extends Base>(p: T): void       // Contravariance for constraints of type parameters
        ret<T extends Base>(): T                // Contravariance for constraints of the return type
    }

Override compatibility with ``Object`` is represented in the example below:

.. index::
   contravariance
   constraint
   return type
   type parameter
   override compatibility

.. code-block:: typescript
   :linenos:

    interface Base {
       kinds_of_parameters<T extends Derived, U extends Base>( // It represents all possible kinds of parameter type
          p01: Derived,
          p02: (q: Base)=>Derived,
          p03: number,
          p04: T | U,
          p05: E1,
          p06: Base[],
          p07: [Base, Base]
       ): void
       kinds_of_return_type(): Object // It can be overridden by all subtypes of Object
    }
    interface Derived extends Base {
       kinds_of_parameters( // Object is a supertype for all class types
          p1: Object,
          p2: Object,
          p3: Object,
          p4: Object,
          p5: Object,
          p6: Object,
          p7: Object
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

.. index::
   parameter type
   overriding
   subtype
   supertype
   overriding
   compatibility

|

.. _Overloading:

Overloading
***********

*Overloading* is the language feature that allows to use the same name to
call several functions, or methods, or constructors with different signatures.

The actual function, method, or constructor to be called is determined at
compile time. Thus, *overloading* is compile-time *polymorphism by name*.

|LANG| supports the following two *overloading*  mechanisms:

- Conventional overloading TBD; and

- Innovative *managed overloading* (see :ref:`Overload Declarations`).

.. index::
   overloading
   name
   context
   entity
   function
   constructor
   method
   signature
   compile type
   compile-time polymorphism
   polymorphism by name
   overload signature
   overload declaration
   managed overloading
   type checking
   compatibility

*Overload resolution* is used to select one entity to call from a set of
candidates if the name to call refers to an *overload declaration* (see
:ref:`Overload Resolution`).

Both mechanisms of resolution use the first-match textual order to streamline
the resolution process.

TBD: A :index:`compile-time warning` is issued if the order of entities in an
*overload declaration* implies that some overloaded entities can never be
selected for a call.

.. code-block:: typescript
   :linenos:

    function f1 (p: number) {}
    function f2 (p: string) {}
    function f3 (p: number|string) {}
    overload foo {f1, f2, f3}  // f3 will never be called as foo()

    foo (5)                    // f1() is called
    foo ("5")                  // f2() is called

.. index::
   signature resolution
   entity
   call
   candidate
   signature resolution
   overload signature
   signature
   overload declaration
   resolution process
   call

|

.. _Overload Resolution:

Overload Resolution
===================

.. meta:
    frontend_status: None

*Overload declaration* defines an ordered set of entities, and the first entity
from this set that is *accessible* and has an appropriate signature is used to
call at the call site.
This approach is called *managed overloading* because the *first-match*
algorithm provides full control for a developer to select a specific entity
to call. This developer control over calls is represented in the following
example:

.. code-block:: typescript
   :linenos:

    function max2i(a: int, b: int): int
        return  a > b ? a : b
    }
    function max2d(a: double, b: double): double {
        return  a > b ? a : b
    }
    function maxN(...a: double[]): double {
        // returns max element in array 'a'
    }
    overload max {max2i, max2d, maxN}

    let i = 1
    let j = 2
    let pi = 3.14

    max(i, j) // max2i is used
    max(i, pi) // max2d is used
    max(i, pi, 4) // maxN is used
    max(1) // maxN is used
    max(false, true) // compile-time error, no appropriate signature

.. index::
   overload declaration
   signature
   entity
   set
   access
   accessible
   call site
   managed overloading
   first-match algorithm
   function
   control
   call

Overload resolution for an instance method overload (see
:ref:`Class Method Overload Declarations`) always uses the type of the
*object reference* known at compile time. It can be either the type used
in a declaration, or a *smart type* (see :ref:`Smart Casts and Smart Types`)
as represented in the example below:

.. code-block:: typescript
   :linenos:

    class A {
        foo1(x: A) { console.log("A.foo") }
        overload foo {foo1}
    }
    class B extends A {
        foo2(x: B) { console.log("B.foo") }
        overload foo {foo2, foo1}
    }

    function test(a: A) {
        a.foo(new B()) // 'foo1' is called as overload from 'A' is used
    }

    test(new B()) // output: A.foo

    let b = new B()
    b.foo(b) // output: B.foo, as overload from 'B' is used

.. index::
   overload resolution
   method overload
   class method
   overload declaration
   type
   object reference
   compile time
   smart type
   smart cast
   signature

|

.. _Type Erasure:

Type Erasure
*************

.. meta:
    frontend_status: Done

*Type erasure* is the compilation technique which provides a special handling
of certain language *types*, primarily :ref:`Generics`, when applied to the
semantics of the following expressions:

-  :ref:`InstanceOf Expression`;
-  :ref:`Cast Expression`.

As a result, special types must be used for the execution of such expressions.
Certain *types* in such expressions are handled as their corresponding
*effective types*, while the *effective type* is defined as type mapping.
The *effective type* of a specific type ``T`` is always a supertype of ``T``.
As a result, the relationship of an original type and an *effective type* can
have the following two kinds:

-  *Effective type* of ``T`` is identical to ``T``, and *type erasure* has no
   effect. So, type ``T`` is *retained*.

-  If *effective type* of ``T`` is not identical to ``T``, then the type ``T``
   is considered affected by *type erasure*, i.e., *erased*.

.. index::
   type erasure
   instanceof expression
   cast expression
   execution
   operation
   type
   generic
   semantics
   effective type
   type mapping
   supertype

In addition, accessing a value of type ``T``, particularly by
:ref:`Field Access Expression`, :ref:`Method Call Expression`, or
:ref:`Function Call Expression`, can cause ``ClassCastError`` thrown if
type ``T`` and the ``target`` type are both affected by *type erasure*, and the
value is produced by a :ref:`Cast Expression`.

.. code-block:: typescript
   :linenos:

    class A<T> {
      field?: T

      test(value: Object) {
        return value instanceof T  // CTE, T is erased
      }

      cast(value: Object) {
        return value as T          // OK, but check is done during execution
      }
    }

    function castToA(p: Object) {
      p instanceof A<number> // CTE, A<number> is erased

      return p as A<number>  // OK, but check is performed against type A, but not A<number>
    }

.. index::
   access
   type erasure
   field access
   function call
   method call
   target type
   cast expression

Type mapping determines the *effective types* as follows:

-  :ref:`Type Parameter Constraint` for :ref:`Type Parameters`.

-  Instantiation of the same generic type (see
   :ref:`Explicit Generic Instantiations`) for *generic types* (see
   :ref:`Generics`), with its type arguments selected in accordance with
   :ref:`Type Parameter Variance` as outlined below:

   - *Covariant* type parameters are instantiated with the constraint type;

   - *Contravariant* type parameters are instantiated with the type ``never``;

   - *Invariant* type parameters are instantiated with no type argument, i.e.,
     ``Array<T>`` is instantiated as ``Array<>``.

-  Union type constructed from the effective types of types ``T1 | T2 ... Tn``
   within the original union type for :ref:`Union Types` in the form
   ``T1 | T2 ... Tn``.

-  Same for :ref:`Array Types` in the form ``T[]`` as for generic type
   ``Array<T>``.

-  Instantiation of ``FixedArray`` for ``FixedArray<T>`` instantiations, with
   the effective type of type argument ``T`` preserved.

-  Instantiation of an internal generic function type with respect to
   the number of parameter types *n* for :ref:`Function Types` in the form
   ``(P1, P2 ..., Pn) => R``. Parameter types ``P1, P2 ... Pn`` are
   instantiated with ``Any``, and the return type ``R`` is instantiated with
   type ``never``.

-  Instantiation of an internal generic tuple type with respect to the number
   of element types *n* for :ref:`Tuple Types` in the form ``[T1, T2 ..., Tn]``.

-  String for :ref:`String Literal Types`.

-  Enumeration base type of the same const enum type for *const enum* types
   (see :ref:`Enumerations`).

-  Otherwise, the original type is *preserved*.

.. index::
   type erasure
   type mapping
   generic type
   type parameter
   constraint
   effective type
   instantiation
   type argument
   covariant type parameter
   type parameter
   contravariant type parameter
   type
   generic tuple
   tuple type
   string
   literal type
   enumeration base type
   const enum type
   enumeration
   invariant type parameter
   parameter type
   type preservation

|

.. _Static Initialization:

Static Initialization
*********************

.. meta:
    frontend_status: Done

*Static initialization* is a routine performed once for each class (see
:ref:`Classes`), namespace (see :ref:`Namespace Declarations`), or
module (see :ref:`Modules and Namespaces`).

*Static initialization* execution involves the execution of the following:

- *Initializers* of *variables* or *static fields*;

- *Top-level statements*;

- Code inside a *static block*.

.. index::
   static initialization
   routine
   class
   namespace
   namespace declaration
   module
   initializer
   variable
   static field
   top-level statement
   static block

*Static initialization* is performed before the first execution of one of the
following operations:

- Invocation of a static method or function of an entity scope;

- Access to a static field or variable of an entity scope;

- Instantiation of an entity that is an interface or class;

- *Static initialization* of a direct subclass of an entity that is a class.

**Note**. None of the operations above invokes a *static initialization*
recursively if the *static initialization* of the same entity is not complete.

**Note**. For namespaces, the code in a static block is executed only when
namespace members are used in the program (an example is provided in
:ref:`Namespace Declarations`).

If *static initialization* routine execution is terminated due to an
exception thrown, then the initialization is not complete. Repeating an attempt
to execute a *static initialization* produces an exception again.

*Static initialization* routine invocation of a concurrent execution (see
:ref:`Coroutines (Experimental)`) involves synchronization of all *coroutines*
that try to invoke it. The synchronization is to ensure that the initialization
is performed only once, and the operations that require the *static
initialization* to be performed are executed after the initialization completes.

If *static initialization* routines of two concurrently initialized classes are
circularly dependent, then a deadlock can occur.

.. index::
   static initialization
   entity
   scope
   static field
   variable
   access
   direct subclass
   subclass
   class
   interface
   operation
   exception
   invocation
   concurrent execution
   coroutine
   synchronization
   deadlock

|

.. _Static Initialization Safety:

Static Initialization Safety
============================

.. meta:
    frontend_status: Done

A compile-time error occurs if a *named reference* refers to a not yet
initialized *entity*, including one of the following:

- Variable (see :ref:`Variable and Constant Declarations`) of a module or
  namespace (see :ref:`Namespace Declarations`);

- Static field of a class (see :ref:`Static and Instance Fields`).

If detecting an access to a not yet initialized *entity* is not possible, then
runtime evaluation is performed as follows:

- Default value is produced if the type of an entity has a default value;

- Otherwise, ``NullPointerError`` is thrown.

.. index::
   static initialization
   safety
   named reference
   initialization
   entity
   variable
   module
   namespace
   static field
   class
   access
   runtime evaluation
   default value
   value
   type

|

.. _Dispatch:

Dispatch
********

.. meta:
    frontend_status: Done

As a result of assignment (see :ref:`Assignment`) to a variable or call (see
:ref:`Method Call Expression` or :ref:`Function Call Expression`), the actual
runtime type of a parameter of class or interface can become different from the
type explicitly specified or inferred at the point of declaration.

In this situation method calls are dispatched during program execution based on
their actual type.

This mechanism is called *dynamic dispatch*. Dynamic dispatch is used in
OOP languages to provide greater flexibility and the required level of
abstraction. Unlike *static dispatch* where the particular method to be called
is known at compile time, *dynamic dispatch* requires additional action during
program code execution. Compilation tools can optimize dynamic to static dispatch.

.. index::
   dispatch
   assignment
   variable
   call
   method call expression
   method
   method call
   function call
   function
   runtime
   runtime type
   parameter
   class
   specified type
   inferred type
   point of declaration
   dynamic dispatch
   object-oriented programming (OOP)
   static dispatch
   compile time
   compilation tool

|

.. _Compatibility Features:

Compatibility Features
**********************

.. meta:
    frontend_status: Done

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

-  Ternary conditional expressions (see :ref:`Ternary Conditional Expressions`,
   :ref:`Conditional-And Expression`, :ref:`Conditional-Or Expression`, and
   :ref:`Logical Complement`);

-  ``while`` and ``do`` statements (see :ref:`While Statements and Do Statements`);

-  ``for`` statements (see :ref:`For Statements`);

-  ``if`` statements (see :ref:`if Statements`).

**Note**. The extended semantics is to be deprecated in one of the future
versions of |LANG|.

The extended semantics approach is based on the concept of *truthiness* that
extends the boolean logic to operands of non-boolean types.

Depending on the kind of a valid expression's type, the value of the valid
expression can be handled as ``true`` or ``false`` as described in the table
below:

.. index::
   extended conditional expression
   ternary conditional expression
   expression
   alignment
   semantics
   conditional-and expression
   conditional-or expression
   logical complement
   while statement
   do statement
   for statement
   if statement
   truthiness
   non-boolean type
   expression type
   extended semantics
   boolean logic
   boolean type
   non-boolean type

.. list-table::
   :width: 100%
   :widths: 25 25 25 25
   :header-rows: 1

   * - Value Type Kind
     - When ``false``
     - When ``true``
     - |LANG| Code Example to Check
   * - ``string``
     - empty string
     - non-empty string
     - ``s.length == 0``
   * - ``boolean``
     - ``false``
     - ``true``
     - ``x``
   * - ``enum``
     - ``enum`` constant handled as ``false``
     - ``enum`` constant handled as ``true``
     - ``x.valueOf()``
   * - ``number`` (``double``/``float``)
     - ``0`` or ``NaN``
     - any other number
     - ``n != 0 && !isNaN(n)``
   * - any integer type
     - ``== 0``
     - ``!= 0``
     - ``i != 0``
   * - ``bigint``
     - ``== 0n``
     - ``!= 0n``
     - ``i != 0n``
   * - ``null`` or ``undefined``
     - ``always``
     - ``never``
     - ``x != null`` or

       ``x != undefined``
   * - Union types
     - When value is ``false`` according to this column
     - When value is ``true`` according to this column
     - ``x != null`` or

       ``x != undefined`` for union types with nullish types
   * - Any other nonNullish type
     - ``never``
     - ``always``
     - ``new SomeType != null``

.. index::
   value type
   integer type
   union type
   nullish type
   empty string
   non-empty string
   string
   number
   nonzero


Extended semantics of :ref:`Conditional-And Expression` and
:ref:`Conditional-Or Expression` affects the resultant type of expressions
as follows:

-  Type of *conditional-and* expression ``A && B`` equals the type of ``B``
   if the result of ``A`` is handled as ``true``. Otherwise, the expression
   type equals the type of ``A``.

-  Type of *conditional-or* expression ``A || B`` equals the type of ``B``
   if the result of ``A`` is handled as ``false``. Otherwise, the expression
   type equals the type of ``A``.

The way this approach works in practice is represented in the example below.
Any ``nonzero`` number is handled as ``true``. The loop continues until it
becomes ``zero`` that is handled as ``false``:

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
   semantics
   conditional-and expression
   conditional-or expression
   loop

.. raw:: pdf

   PageBreak
