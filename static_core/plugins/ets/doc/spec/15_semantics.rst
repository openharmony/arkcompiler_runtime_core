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

|

.. _Type of Standalone Expression:

Type of Standalone Expression
=============================

.. meta:
    frontend_status: Done

*Standalone expression* (see :ref:`Type of Expression`) is an expression for
which no target type is expected in its context.

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

The situation is represented by the example below:

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
        let x = y // type of 'x' is 'int'
    }

-  Otherwise, the type of ``expr`` is evaluated as type of a standalone
   expression as in the example below:

.. code-block:: typescript
   :linenos:

    function foo() {
        let x = 1 // x is of type 'int' (default type of '1')
        let y = [1, 2] // x is of type 'number[]'
    }

|

.. _Specifics of Numeric Operator Contexts:

Specifics of Numeric Operator Contexts
======================================

.. meta:
    frontend_status: Done

Operands of unary and binary numeric expressions are widened to a larger numeric
type. The minimum type is ``int``. Specifically, no arithmetic operator
evaluates values of types ``byte`` and ``short`` without widening. Details of
specific operators are discussed in corresponding sections of the Specification.

|

.. _Specifics of String Operator Contexts:

Specifics of String Operator Contexts
=====================================

.. meta:
    frontend_status: Done

If one operand of the binary operator ‘`+`’ is of type ``string``, then the
string conversion applies to another non-string operand to convert it to string
(see :ref:`String Concatenation` and :ref:`String Operator Contexts`).

|

.. _Other Contexts:

Other Contexts
==============

.. meta:
    frontend_status: Done

The only semantic rule for all other contexts, and specifically for
:ref:`Overriding`, is to use :ref:`Subtyping`.

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

The semantics is represented by the example below:

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
Each type is its own subtype and supertype (``S<:S``).

By the definition of ``S<:T``, type ``T`` belongs to the set of *supertypes*
of type ``S``. The set of *supertypes* includes all *direct supertypes*
(discussed in subsections), and all their respective *supertypes*.
More formally speaking, the set is obtained by reflexive and transitive
closure over the direct supertype relation.

If the subtyping relation of two types is not defined in a section below,
then such types are not related to each other. Specifically, two array types
(resizable and fixed-size alike), and two tuple types are not related to each
other, except where they are identical (see :ref:`Type Identity`).

.. index::
   subtyping
   subtype
   closure
   supertype
   direct supertype
   reflexive closure
   transitive closure
   array type
   array
   resizable array
   fixed-size array
   tuple type
   type

|

.. _Subtyping for Classes and Interfaces:

Subtyping for Classes and Interfaces
====================================

.. meta:
    frontend_status: Partly

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
   superinterface
   interface type
   direct supertype
   non-generic class
   direct superclass
   direct superinterface
   implementation
   non-generic class
   extension clause
   implementation clause
   superinterface
   Object
   interface type
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
   bound
   Object

|

.. _Subtyping for Literal Types:

Subtyping for Literal Types
===========================

.. meta:
    frontend_status: Done

Any ``string`` literal type (see :ref:`Literal Types`) is *subtype* of type
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
   type string
   overriding
   supertype
   string literal
   null
   undefined
   literal type

|

.. _Subtyping for Union Types:

Subtyping for Union Types
=========================

.. meta:
    frontend_status: Done

A union type ``U`` participates in subtyping relations
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
by the example below:

.. index::
   union type
   union type normalization
   subtype

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
following conditions are  met:

-  ``m <= n``

-  for each ``i <= m``

   -  Parameter type of ``SP``:sub:`i` is a subtype of
      parameter type of ``FP``:sub:`i` (contravariance), and

   -  ``FP``:sub:`i` is a rest parameter if ``SP``:sub:`i` is a rest parameter.
   -  ``FP``:sub:`i` is an optional parameter if ``SP``:sub:`i` is an optional
      parameter.

-  type ``FR`` is a subtype of ``SR`` (covariance).

.. index::
   function type
   subtype
   parameter type
   contravariance
   rest parameter
   parameter
   covariance
   return type

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
       /* Compile-time error: too less parameters */
    }

.. index::
   parameter type
   covariance
   contravariance
   covariant return type
   contravariant return type
   supertype
   parameter

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
  are identical if the following conditions are met:

  - ``n`` is equal to ``m``, i.e., the types have the same number of elements;
  - Every *T*:sub:`i` is identical to *U*:sub:`i` for any *i* in ``1 .. n``.

- Union types ``A`` = ``T``:sub:`1` | ``T``:sub:`2` | ``...`` | ``T``:sub:`n` and
  ``B`` = ``U``:sub:`1` | ``U``:sub:`2` | ``...`` | ``U``:sub:`m`
  are identical if the following conditions are met:

  - ``n`` is equal to ``m``, i.e., the types have the same number of elements;
  - *U*:sub:`i` in ``U`` undergoes a permutation after which every *T*:sub:`i`
    is identical to *U*:sub:`i` for any *i* in ``1 .. n``.

- Types ``A`` and ``B`` are identical if ``A`` is a subtype of ``B`` (``A<:B``),
  and ``B`` is  at the same time a subtype of ``A`` (``A:>B``).

**Note.** :ref:`Type Alias Declaration` creates no new type but only a new
name for the existing type. An alias is indistinguishable from its base type.

.. index::
   type identity
   identity
   indistinguishable type
   array type
   tuple type
   union type
   subtype
   type
   type alias
   declaration
   base type

|

.. _Assignability:

Assignability
*************

.. meta:
    frontend_status: Done

Type ``T``:sub:`1` is assignable to type ``T``:sub:`2` if:

-  ``T``:sub:`1` is type ``never`` and ``T``:sub:`2` is any other type;

-  ``T``:sub:`1` is identical to ``T``:sub:`2` (see :ref:`Type Identity`);

-  ``T``:sub:`1` is a subtype of ``T``:sub:`2` (see :ref:`Subtyping`); or

-  *Implicit conversion* (see :ref:`Implicit Conversions`) is present that
   allows converting a value of type ``T``:sub:`1` to type ``T``:sub:`2`.


*Assignability* relationship  is asymmetric, i.e., that ``T``:sub:`1`
is assignable to ``T``:sub:`2` does not imply that ``T``:sub:`2` is
assignable to type ``T``:sub:`1`.

.. index::
   assignability
   type
   type identity
   subtyping
   conversion
   implicit conversion
   asymmetric relationship

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
   return type
   variance
   invariance
   covariance
   contravariance

*Covariance* means it is possible to use a type which is more specific than
originally specified.

.. index::
   covariance

*Contravariance* means it is possible to use a type which is more general than
originally specified.

.. index::
   contravariance

*Invariance* means it is only possible to use the original type, i.e., there is
no subtyping for derived types.

.. index::
   invariance

The examples below illustrate valid and invalid usages of variance.
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
   subtype
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

The examples below represent the checks:

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

- :ref:`Type Inference for Integer Constant Expressions`;
- Variable and constant declarations (see :ref:`Type Inference from Initializer`);
- Implicit generic instantiations (see :ref:`Implicit Generic Instantiations`);
- Function, method or lambda return type (see :ref:`Return Type Inference`);
- Lambda expression parameter type (see :ref:`Lambda Signature`);
- Array literal type inference (see :ref:`Array Literal Type Inference from Context`,
  and :ref:`Array Type Inference from Types of Elements`);
- Object literal type inference (see :ref:`Object Literal`);
- Smart types (see :ref:`Smart Types`).

.. index::
   strong typing
   type annotation
   smart compiler
   type inference
   entity
   surrounding context
   code readability
   type safety
   context
   variable declaration
   constant declaration
   generic instantiation
   function return type
   function
   method return type
   method
   return type
   lambda expression
   parameter type
   array literal
   Object literal
   smart type

|

.. _Type Inference for Integer Constant Expressions:

Type Inference for Integer Constant Expressions
===============================================

.. meta:
    frontend_status: Partly

The type of expression of integer types for :ref:`Constant Expressions` is
first evaluated from the expression as follows:

- Type of an integer literal is the default type of the literal:
  ``int`` or ``long`` (see :ref:`Integer Literals`);

- Type of a named constant is specified in the constant declaration;

- Result type of an operator is evaluated according to the rules of
  the operator;

- Type of a :ref:`Cast expression` is specified in the expression target type.

The evaluated result type can be inferred to a smaller integer *target type*
from the context if it is of an integer type and the following conditions are met:

#. Top-level expression is not a cast expression;

#. Value of the expression fits into the range of the *target type*.

A :index:`compile-time error` occurs if the context is a union type,
and the evaluated value can be treated
as value of several of union component types.

The examples below illustrate valid and invalid narrowing.

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    let b: byte = 127 // ok, int -> byte narrowing
    b = 64 + 63 // ok, int -> byte narrowing
    b = 128 // compile-time-error, value is out of range
    b = 1.0 // compile-time-error, floating-point value cannot be narrowed
    b = 1 as short // // compile-time-error, cast expression

    let s: short = 32768 // compile-time-error, value is out of range

    let u: byte | int = 1 // compile-time error, ambiguity

.. index::
   narrowing
   constant
   constant expression
   integer conversion
   integer type
   expression
   conversion
   type
   value

|

.. _Smart Types:

Smart Types
===========

.. meta:
    frontend_status: Partly
    todo: implement a dataflow check for loops and try-catch blocks

Data entities like local variables (see :ref:`Variable and Constant Declarations`)
and parameters (see :ref:`Parameter List`), if not captured in a lambda body and
modified by the lambda code, are subjected to *smart typing*.

Every data entity has a static type, which is specified explicitly or
inferred at the point of declaration. This type defines the set of operations
that can be applied to the entity (namely, what methods can be called, and what
other entities can be accessed if the entity acts as a receiver of the
operation):

.. code-block:: typescript
   :linenos:

    let a = new Object
    a.toString() // entity 'a' has method toString()

.. index::
   smart type
   data entity
   variable
   parameter
   class variable
   local variable
   smart typing
   lambda code
   function
   method
   static type
   inferred type
   receiver
   access
   declaration

If an entity is class type (see :ref:`Classes`), interface type (see
:ref:`Interfaces`), or union type (see :ref:`Union Types`), then the compiler
can narrow (smart cast) a static type to a more precise type (smart type), and
allow operations that are specific to the type so narrowed:

.. code-block:: typescript
   :linenos:

    function boo() {
        let a: number | string = 42
        a++ /* Here we know for sure that type of 'a' is number and number-specific
           operations are type-safe */
    }

    class Base {}
    class Derived extends Base { method () {} }
    function goo() {
       let b: Base = new Derived
       b.method () /* Here we know for sure that type of 'b' is Derived and Derived-specific
           operations can be applied in type-safe way */
    }

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
   entity
   class type
   static type
   narrowed type
   instanceof
   null
   semantic check
   reference equality

In like cases, a smart compiler can deduce the smart type of an entity without
requiring additional checks or casts (see :ref:`Cast Expression`).

Overloading (see :ref:`Overload Declarations`) can cause tricky situations
when a smart type leads to the call of an entity
(see :ref:`Overload Resolution for Overload Declarations`)
that suits smart type rather than static type of an argument:

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

Particular cases supported by the compiler are determined by the compiler
implementation.

.. index::
   smart type
   entity
   casting conversion
   overloading
   function
   method
   static type
   implementation
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
Thus, overriding is related to runtime polymorphism.

|LANG| uses the *override-compatibility* rule to check the correctness of
overriding. The *overriding* is correct if method signature in a subtype
(subclass or subinterface) is *override-compatible* with the method defined
in a supertype    (see :ref:`Override-Compatible Signatures`).

.. index::
   overriding
   subclass
   runtime polymorphism
   inheritance
   parent class
   object type
   runtime
   override-compatibility

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
      internal internal_member() {}
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
      internal internal_member() {}
         // Internal member can be overridden by the internal one only
      override private_member() {}
         // A compile-time error occurs if an attempt is made to override private member
         // or implement the private methods with default implementation
   }

The table below represents semantic rules that apply in various contexts:

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

   * - A *static method* is defined in a subclass with the same name as the
       *static method* in a superclass.
     - If signatures are *override-compatible* (see
       :ref:`Override-Compatible Signatures`), then the static method in the
       subclass *hides* the previous static method.
       Otherwise, a :index:`compile-time error` occurs.

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
      static method_2(p: string) {} // compile-time error
   }

.. list-table::
   :width: 100%
   :widths: 50 50
   :header-rows: 0

   * - A *constructor* is defined in a subclass.
     - All base class constructors are available for call in all derived class
       constructors.


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
   derived class constructor

|

.. _Overriding and Overload Signatures in Interfaces:

Overriding and Overload Signatures in Interfaces
================================================

.. meta:
    frontend_status: Done

.. list-table::
   :width: 100%
   :widths: 50 50
   :header-rows: 1

   * - Context
     - Semantic Check
   * - A method is defined in a subinterface with the same name as the method
       in the superinterface.
     - If signatures are *override-compatible* (see
       :ref:`Override-Compatible Signatures`), then *overriding* is used.
       Otherwise, a :index:`compile-time error` occurs.

.. code-block:: typescript
   :linenos:

   interface Base {
      method_1()
      method_2(p: number)
   }
   interface Derived extends Base {
      method_1() // overriding
      method_2(p: string) // compile-time error
   }


.. list-table::
   :width: 100%
   :widths: 50 50
   :header-rows: 0

   * - Two or more methods with the same name are defined in the same interface.
     - :ref:`Interface Method Overload Signatures` is used.


.. index::
   method
   subinterface
   superinterface
   semantic check
   override-compatible
   interface

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

The semantics is illustrated by the examples below:

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


Override compatibility with ``Object`` is represented by the example below:

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
          p04: Number,
          p05: T | U,
          p06: E1,
          p07: Base[],
          p08: [Base, Base]
       ): void
       kinds_of_return_type(): Object // It can be overridden by all subtypes of Object
    }
    interface Derived extends Base {
       kinds_of_parameters( // Object is a supertype for all class types
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

.. index::
   parameter type
   overriding
   subtype
   supertype
   overriding
   compatibility

|

.. _Overriding and Implementing Methods with Overload Signatures:

Overriding and Implementing Methods with Overload Signatures
============================================================

.. meta:
    frontend_status: None

If an interface (*derived interface*) extends another interface (*base
interface*), and the base interface has a set of overload signatures, then the
derived interface must provide a valid overriding overload signature (or
signatures) for all overload signatures of the base interface. The derived
interface can introduce additional overload signatures. The situation is
represented by the example below:


.. code-block:: typescript
   :linenos:

    interface Interface {
      foo (p: number): void // 1st overload signature
      foo (p: string): void // 2nd overload signature
    }

    interface Interface1 extends Interface {
      foo (p: number|string): void // 1st overload signature overrides both foo from Interface
      foo (p: boolean): void       // 2nd overload signature
    }

    function demo (p1: Interface1) {
        p1.foo (5)         // fits 1st signature of Interface1
        p1.foo ("5 true")  // fits 1st signature of Interface1
        p1.foo (true)      // fits 2nd signature of Interface1
    }


If a class (*derived class*) implements an interface (*base interface*), and
the base interface has a set of overload signatures, then the derived class
can provide a valid overriding overload signature (or signatures) for all
overload signatures of the base interface. The derived class can introduce
additional overload signatures. The implementation body must have
the signature ``(...p: Any[]): Any`` (see
:ref:`Overload Signatures Implementation Body`). This signature is a valid
overriding for any overloaded signature. The same works if one class extends
another class. The situation is represented by the example below:


.. code-block:: typescript
   :linenos:

    class Class1 implements Interface {
      foo (p: number): void // 1st overload signature
      foo (p: string): void // 2nd overload signature
      foo (...p: Any[]): Any {} // implementation signature + body
    }

    class Class2 implements Interface {
      foo (...p: Any[]): Any {} // implementation signature only + body
    }

    class Class3 extends Class1 {
      override foo (p: number): void // 1st overload signature
      override foo (p: string): void // 2nd overload signature
      override foo (...p: Any[]): Any {} // implementation signature + body
    }

    class Class4 extends Class3 {
      override foo (...p: Any[]): Any {} // implementation signature only + body
    }

    new Class1().foo(5)     // OK
    new Class1().foo("555") // OK
    new Class1().foo(true)  // compile-time error - no boolean parameter

    new Class2().foo(5)     // OK
    new Class2().foo("555") // OK

    function test (p: Interface) {
        p.foo (5)
        p.foo ("5555")
    }

    test(new Class1)
    test(new Class2)
    test(new Class3)
    test(new Class4)


|

.. _Overloading:

Overloading
***********

*Overloading* is the language feature that allows to use one name to
call several functions, or methods, or constructors with different signatures
and different bodies.

The actual function, or method, or constructor to be called is determined at
compile time. Thus, *overloading* is related to compile-time polymorphism.

|LANG| support two mechanisms for *overloading*:

- |TS| compatible feature: :ref:`Declarations with Overload Signatures`
  that is mainly used to improve type checking;

- and innovative form of *managed overloading*: :ref:`Overload Declarations`.

.. index::
   overloading
   context
   entity
   function
   constructor
   method
   signature
   compile-time polymorphism
   overloading

*Overload resolution* is used to select one entity to be called from a set of
candidates if a name to call refers to a *overload declaration* (see
:ref:`Overload resolution for Overload Declarations`).

*Signature resolution* is used to select one entity to be called from a set of
candidates if a name to call refers to a *declaration with overload signatures*
(see :ref:`Signature resolution for Overload Signatures`).

Both resolution schemes use the first match textual order to make the
resolution process straightforward.

TBD: A :index:`compile-time warning` is issued if the order of entities in *overload
declarations* implies that some overloaded entities can never be selected for the call.

.. code-block:: typescript
   :linenos:

    function f1 (p: number) {}
    function f2 (p: string) {}
    function f3 (p: number|string) {}
    overload foo {f1, f2, f3}  // f3 will never be called as foo()

    foo (5)                    // f1() is called
    foo ("5")                  // f2() is called

|

.. _Signature resolution for Overload Signatures:

Signature resolution for Overload Signatures
============================================

.. meta:
    frontend_status: None

*Overload signatures* allows allows specifying a function, or method, or
constructor that can have several signatures and one *implementation body*
with its own fixed *implementation siganture* (see
:ref:`Overload Signatures Implementation Body`). At the call site, call
arguments are checked against these several signatures in their declaration
order: the first one that is appropriate for the given arguments means that
the call is valid. If no appropriate signature found then a
:index:`compile-time error` occurs.

.. code-block:: typescript
   :linenos:

    function foo(s: string)            // signature #1
    function foo(s: string, n: number) // signature #2
    function foo(...x: Any[]): Any {}  // implementation signature

    foo("1")     // call fits signature #1
    foo("1", 5)  // call fits signature #2
    foo(1, 2, 3) // compile-time error - no appropriate signature for the call
                 // implementation signature is not accessible at call sites

|

.. _Overload resolution for Overload Declarations:

Overload resolution for Overload Declarations
=============================================

.. meta:
    frontend_status: None

*Overload declarations* defines an ordered set of entities, at the call site
the first *accessible* entity from this set with appropriate signature
is used to call. The *first match* algorithm gives a developer with
full control over selecting specific entity to call.
That's why this approach is called *managed overloading*.

This example illustrates the developer control over calls:

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

|

.. _Type Erasure:

Type Erasure
*************

.. meta:
    frontend_status: Done

*Type erasure* is the concept that denotes a special handling of some language
*types*, primarily :ref:`Generics`, in the semantics of the following language
operations that require the type to be preserved for execution:

-  :ref:`InstanceOf Expression`;
-  :ref:`Cast Expression`.

In these operations some *types* are handled as their corresponding *effective
types*, while the *effective type* is defined as type mapping. The *effective
type* of a specific type ``T`` is always a supertype of ``T``. As a result,
two kinds of relationship are possible between an original type and an
*effective type*:

-  *Effective type* of ``T`` is identical to ``T``, and *type erasure* has no
   effect.

-  If *effective type* of ``T`` is not identical to ``T``, then the type ``T``
   is considered affected by *type erasure*, i.e., *erased*.

.. index::
   type erasure
   instanceof expression
   cast expression
   operation
   type
   effective type
   type mapping
   supertype

In addition, accessing a value of type ``T``, including by
:ref:`Field Access Expression`, :ref:`Method Call Expression`, or
:ref:`Function Call Expression` can cause ``ClassCastError`` thrown if
type ``T`` and the ``target`` type are both affected by *type erasure*, and the
value is produced by :ref:`Cast Expression`.

.. code-block:: typescript
   :linenos:

    class A<T> {
      field?: T

      test(value: Object) {
        return value instanceof T  // CTE, T is erased
      }

      cast(value: Object) {
        return value as T          // OK, but check is postponed
      }
    }

    function castToA(p: Object) {
      p instanceof A<number> // CTE, A<number> is erased

      return p as A<number>  // OK, but check is performed against A
    }

.. index::
   type erasure
   field access
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

   - *Invariant* type parameters have no corresponding type argument, **TBD**

-  Union type constructed from the effective types of types ``T1 | T2 ... Tn``
   within the original union type for :ref:`Union Types` in the form
   ``T1 | T2 ... Tn``.

-  Same for :ref:`Array Types` in the form ``T[]`` as for generic type ``Array<T>``.

-  Instantiation of ``FixedArray`` for ``FixedArray<T>`` instantiations, with
   the effective type of type argument ``T`` preserved.

-  Instantiation of an internal generic function type with respect to
   the number of parameter types *n* for :ref:`Function Types` in the form
   ``(P1, P2 ..., Pn) => R``. Parameter types ``P1, P2 ... Pn`` are
   instantiated with ``object | null | undefined``, and the return type ``R``
   is instantiated with type ``never``.

-  Instantiation of an internal generic tuple type with respect to
   the number of element types *n* for :ref:`Tuple Types` in the form
   ``[T1, T2 ..., Tn]``. **TBD**

-  String for *string literal types* (see :ref:`Literal Types`).

-  Enumeration base type of the same const enum type for *const enum* types
   (see :ref:`Enumerations`).

-  Otherwise, the original type is preserved.

.. index::
   type erasure
   type mapping
   generic type
   effective type
   instantiation
   type argument
   covariant type parameter
   type parameter
   contravariant type parameter
   invariant type parameter
   parameter type
   type argument
   type preservation

|

.. _Static Initialization:

Static Initialization
*********************

.. meta:
    frontend_status: Done

*Static initialization* is a routine performed once for each class (see
:ref:`Classes`), namespace (see :ref:`Namespace Declarations`), separate module
(see :ref:`Separate Modules`), or package module (see :ref:`Packages`).

*Static initialization* execution involves the execution of the following:

- *Initializers* of *variables* or *static fields*;

- *Top-level statements*;

- Code inside a *static block*.


*Static initialization* is performed before the first execution of one of the
following operations:

- Invocation of a static method or function of an entity scope;

- Access to a static field or variable of an entity scope;

- Instantiation of an entity that is an interface or class;

- *Static initialization* of a direct subclass of an entity that is a class.

**Note**. None of the operations above invokes a *static initialization*
recursively if the *static initialization* of the same entity is not complete.

If *static initialization* routine execution is terminated due to an
exception thrown, then the initialization is not complete. A repetitive attempt
to execute the *static initialization* produces an exception again.

*Static initialization* routine invocation of a concurrent execution (see
:ref:`Coroutines (Experimental)`) involves synchronization of all *coroutines*
that try to invoke it. The synchronization is to ensure that the initialization
is performed only once, and the operations that require the *static
initialization* to be performed are executed after the initialization completes.

If *static initialization* routines of two concurrently initialized classes are
circularly dependent, then a deadlock can occur.

|

.. _Static Initialization Safety:

Static Initialization Safety
============================

.. meta:
    frontend_status: Done

A compile-time error occurs if a *named reference* refers to a not yet
initialized *entity*, including one of the following:

- Variable (see :ref:`Variable and Constant Declarations`) of a separate module
  package (see :ref:`Packages`), or namespace (see :ref:`Namespace Declarations`);

- Static field of a class (see :ref:`Static and Instance Fields`).

If detecting an access to a not yet initialized *entity* is not possible, then
runtime evaluation is performed as follows:

- Default value is produced if the type of an entity has a default value;

- Otherwise, ``NullPointerError`` is thrown.

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
   OOP (object-oriented programming)
   static dispatch
   compile time

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

Depending on the kind of a valid expression's type, the value of the valid
expression can be handled as ``true`` or ``false`` as described in the table
below:

.. index::
   extended conditional expression
   conditional expression
   alignment
   semantics
   conditional-and expression
   conditional-or expression
   while statement
   do statement
   for statement
   if statement
   truthiness
   non-boolean type
   expression type


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
   string
   integer type
   union type
   nullish type
   nonzero

.. raw:: pdf

   PageBreak
