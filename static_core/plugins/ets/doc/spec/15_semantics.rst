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

The section gives a brief introduction to the major semantic terms
and their usage in several contexts.

|

.. _Type of Standalone Expression:

Type of Standalone Expression
=============================

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

Operands of unary and binary numeric expressions are widened to a larger numeric
type. The minimum type is ``int``. Specifically, no arithmetic operator
evaluates values of types ``byte`` and ``short`` without widening. Details of
specific operators are discussed in corresponding sections of the Specification.

|

.. _Specifics of String Operator Contexts:

Specifics of String Operator Contexts
=====================================

If one operand of the binary operator ‘`+`’ is of type ``string``, then the
string conversion applies to another non-string operand to convert it to string
(see :ref:`String Concatenation` and :ref:`String Operator Contexts`).

|

.. _Other Contexts:

Other Contexts
==============

The only semantic rule for all other contexts, and specifically for
:ref:`Overloading and Overriding`, is to use :ref:`Subtyping`.

|

.. _Specifics of Type Parameters:

Specifics of Type Parameters
============================

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
- :ref:`Overloading and Overriding`;
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
   normalization
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

-  ``FR`` can be any type if ``SR`` is type ``void``. Otherwise, the resultant
   type ``FR`` is a subtype of ``SR`` (covariance).

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

       let h: (p: Base) => void = bb
       /* OK: result type of supertype is void */
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
:ref:`spread Expression`) are to be linearized recursively to enusre that
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

    call (1, "str", true, 123) // To be unfoled into the form with no spread expressions



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

For :ref:`Constant Expressions` of integer types
the type of expression is first evaluated from the expression
in the following way:

- For an integer literal the type is the default type of the literal:
  ``int`` or ``long`` (see :ref:`Integer Literals`);

- For a named constant the type is specified in the constant declaration;

- For an operator the result type is evaluated accoriding rules of
  this operator;

- For :ref:`Cast expression` type is specified in the expression
  target type.

If the evaluated result type is of an integer type,
it can be inferred to smaller integer *target type* from the context,
if the following conditions are met:

#. The top-level expression is not a cast expression;

#. The value of the expression fits into the range of the *target type*.

The examples below illustrate valid and invalid narrowing.

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    let b: byte = 127 // ok, int -> byte narrowing
    b = 64 + 63 // ok, int -> byte narrowing
    b = 128 // compile-time-error, value is out of range
    b = 1.0 // compile-time-error, floating-point value cannot be narrowed
    b = 1 as short // // compile-time-error, cast expresion

    let s: short = 32768 // compile-time-error, value is out of range

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
requiring unnecessary casting conversions (see :ref:`Cast Expression`).

Overloading (see :ref:`Function, Method and Constructor Overloading`) can cause
tricky situations when a smart type leads to the call of a function or a method
(see :ref:`Overload Resolution`) that suits smart rather than static type of an
argument:

.. code-block:: typescript
   :linenos:

    function foo (p: Base) {}
    function foo (p: Derived) {}

    function too() {
       let b: Base = new Derived
       foo (b) // potential ambiguity in case of smart type, foo(p:Base) is to be called
       foo (b as Derived) // no ambiguity,  foo(p:Derived) is to be called
    }

Particular cases supported by the compiler are determined by the compiler
implementation.

.. index::
   compiler
   smart type
   smart compiler
   entity
   casting conversion
   overloading
   conversion
   function
   method
   conversion overloading
   function overloading
   method overloading
   static type
   argument
   implementation
   compiler

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
   subclass
   runtime polymorphism
   inheritance
   parent class
   object type
   runtime
   overload-equivalence
   override-compatibility
   overload-equivalent signature
   overriding
   overloading

|

.. _Overload-Equivalent Signatures:

Overload-Equivalent Signatures
==============================

.. meta:
    frontend_status: Partly

Signatures *S1* with parameters *S1P*:sub:`1`, ... , *S1P*:sub:`n`, and *S2* with
the same number of parameters *S2P*:sub:`1`, ... , *S2P*:sub:`n`
are *overload-equivalent* if the *effective types* of parameters (see
:ref:`Type Erasure`) *S1P*:sub:`i` and *S2P*:sub:`i` for each *i* are
*overload-equavalent*.

**Notes:**

-  For an optional parameter (see :ref:`Optional Parameters`) in the form
   ``ident?: T``, the actual parameter type is considered, i.e., union type
   ``T | undefined``.

-  Type parameter constraint ``Object|null|undefined`` (see
   :ref:`Type Parameter Constraint`) is condidered for a type parameter if
   no constraint is set explicitly.

Parameters *S1P*:sub:`i` and *S2P*:sub:`i` are *overload-equavalent*, if
the are simultaneously ``rest`` or not ``rest`` parameters and if:

#. Type of *S1P*:sub:`i` is a *type parameter* and type of *S2P*:sub:`i`
   is a subtype of *type parameter constraint* or a *type parameter*;

#. Type of *S1P*:sub:`i` is *generic type*
   ``G`` <``T``:sub:`1`, ``...``, ``T``:sub:`n`>, where at least one
   ``T``:sub:`i` is a type parameter, and a
   type of *S2P*:sub:`i` is also ``G`` with any
   list of :ref:`Type Arguments` or a *union type* that contains ``G``;

#. Types of *S1P*:sub:`i` and *S2P*:sub:`i` are
   *union types* containing types that fall into either provision above;

#. Types of *S1P*:sub:`i` and *S2P*:sub:`i` are identical (see
   :ref:`Type Identity`).

Parameter names and return types do not influence *overload equivalence*.
Signatures are *overload-equivalent*  in the following examples:

.. index::
   overload-equivalent signature
   signature
   parameter
   type parameter
   parameter type
   non-generic reference type
   union type
   reference type
   generic type
   type argument
   overriding
   parameter name
   return type
   overload equivalence
   type identity
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

.. index::
   overload-equivalent signature

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

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class A<T> {
   (p: T)
   (p: T[])

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
   (x: G<number>): void
   (y: G<string>): void

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class G<T extends number>
   (x: T): void
   (y: string): void

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


The example below illustrates override compatibility with ``Object``:

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

.. index::
   parameter type
   overriding
   subtype
   supertype
   overriding
   compatibility

|

.. _Overloading for Functions:

Overloading for Functions
=========================

.. meta:
    frontend_status: Partly

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
   correctness check
   semantic check
   accessibility
   access
   scope
   import
   compilation unit
   overload-equivalent signature

|

.. _Overloading and Overriding in Classes:

Overloading and Overriding in Classes
=====================================

.. meta:
    frontend_status: Partly

Both *overloading* and *overriding* must be considered in case of classes for
methods and partly for constructors.

**Note**. Only accessible (see :ref:`Accessible`) methods are subjected to
overloading and overriding. The same rules also apply to accessors in case of
overriding.

An overriding member can keep or extend an access modifier (see
:ref:`Access Modifiers`) of a member that is inherited or implemented.
Otherwise, a :index:`compile-time error` occurs.

An attempt to override a private method of a superclass, or to declare a method
with the same name as the private method with default implementation from any
superinterface causes a :index:`compile-time error`.

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
   * - Two *instance methods*, two *static methods* with the same name, or two
       *constructors* are defined in the same class.
     - If signatures are *overload-equivalent*, (see :ref:`Overload-Equivalent
       Signatures`), then a :index:`compile-time error` occurs. Otherwise,
       *overloading* is used.


.. index::
   semantic check
   instance method
   method
   static method
   constructor
   overload equivalence
   overloading
   overload-equivalent signature
   overriding
   implementation
   public
   internal
   private

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

.. list-table::
   :width: 100%
   :widths: 50 50
   :header-rows: 0

   * - An *instance method* is defined in a subclass with the same name as the
       *instance method* in a superclass.
     - If signatures are *override-compatible* (see
       :ref:`Override-Compatible Signatures`), then *overriding* is used.
       Otherwise, *overloading* is used.


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

.. list-table::
   :width: 100%
   :widths: 50 50
   :header-rows: 0

   * - A *static method* is defined in a subclass with the same name as the
       *static method* in a superclass.
     - If signatures are *overload-equivalent* (see
       :ref:`Overload-Equivalent Signatures`), then the static method in the
       subclass *hides* the previous static method.Otherwise, *overloading* is
       used.

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

.. _Overloading and Overriding in Interfaces:

Overloading and Overriding in Interfaces
========================================

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
       Otherwise, *overloading* is used.

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


.. list-table::
   :width: 100%
   :widths: 50 50
   :header-rows: 0

   * - Two methods with the same name are defined in the same interface.
     - *Overloading* is used. A :index:`compile-time error` occurs if signatures
       are *overload-equivalent*.


.. index::
   method
   subinterface
   superinterface
   semantic check
   override-compatible
   overload-equivalent
   interface
   overloading

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

#. If there is more than one *applicable candidate*, then select the *best
   candidate*.

**Note**. The first step is performed in all cases, even if there is
only one *applicable candidate* to check *call signature compatibility*.

.. index::
   overload resolution
   entity
   applicable candidate
   call signature compatibility
   constructor call
   constructor
   potentially applicable candidate
   best candidate

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
   potentially applicable candidate
   semantic check
   compatibility
   call argument
   entity
   parameter type
   overload resolution
   overloaded function
   call

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
  then at least one implicit argument transformation   must be applied to make
  the call.

Possible argument transformations are listed below:

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
   overload-equivalent signature
   rest parameter
   conversion
   array
   rest parameter

The examples of transformations are presented below:

.. code-block:: typescript
   :linenos:

   function foo1(x?: string) {}
   foo1() // passing default value -> foo(undefined)

   function foo2(...x: int[]) {}
   foo2() // passing empty array -> foo([])
   foo2(1, 2) // folding to array -> foo(...[1, 2])

The *best candidate* is the candidate that requires no transformation for all
arguments. If there is such a single candidate, then other candidates are not
considered. Such *best candidate* is represented
in the example below:

.. code-block:: typescript
   :linenos:

   function max(a: number, b: number)  // #1
   function max(...args: number[]) // #2

   max(1, 2) // #1 - is the best candidate, no transformation

.. index::
   best candidate
   transformation
   argument

If there is no *best candidate* at this step, then each candidate
is compared to other candidates.
The following sequence of checks is used to calculate a partially *better*
relation based on the comparison of candidates *C1* and *C2*:


**Check 1**. If *C1* has fewer parameters, i.e., default values or an empty
``rest`` argument are used instead of the absent arguments in the *C2* call,
then *C1* is *better*.

.. code-block:: typescript
   :linenos:

   function foo(n: number, s?: string)  // #1
   function foo(n: number)              // #2

   foo(1) // #2 is better, less parameters

   function bar(...args: number[])  // #1
   function bar()                   // #2

   bar() // #2 is better, less parameters

   function goo(...args: number[])  // #1
   function goo(n?: number)         // #2

   goo() // none is better

.. index::
   best candidate
   better candidate
   partially better candidate
   rest argument

**Check 2**. If *C1* has a non-``rest`` parameter(s) for a non-empty list of
arguments, and *C2* has a ``rest`` parameter, then *C1* is *better*.

.. code-block:: typescript
   :linenos:

   function foo(sum: number, a: number, b: number)  // #1
   function foo(sum: number, ...x: number[])        // #2

   foo(1, 2, 3) // #1 is better, non-rest parameters

.. index::
   argument
   better candidate
   rest argument

**Check 3**. If an argument type is a subtype of parameter type for *C1* and
not for *C2*, then *C1* is *better* for this argument.

.. code-block:: typescript
   :linenos:

   function foo(n: int)  // #1
   function foo(n: long) // #2

   foo(1) // #1 is better, argument type is subtype of parameter type
   foo(1 as long) // #2 is better

.. index::
   argument
   better candidate
   argument transformation

**Check 4**. If an argument type is a subtype of parameter type for both *C1*
and *C2*, but type of *C1* is identical for the argument type and type of *C2*
is not, then *C1* is *better* for this argument.

.. code-block:: typescript
   :linenos:

    class C {}
    class D extends C {}

    function foo(x: C) {} // #1
    function foo(x: D) {} // #2

    foo(new C) // #1 is better

**Check 5**. Otherwise, none is better for this argument, including cases:

-  An argument type is a subtype of parameter type for both *C1*
   and *C2*, but neither is identical;

-  An argument type is not a subtype for both *C1* and *C2*.

.. code-block:: typescript
   :linenos:

   function foo(x: number | boolean) // #1
   function foo(x: number | string)  // #2

   foo(1.) // both subtype, none identical: none is better

   function negate(x: long) // #1
   function negate(x: double) // #2

   negate(1) // none subtype: none is better

.. index::
   best candidate
   better candidate
   argument transformation
   numeric type
   conversion
   parameter

A :index:`compile-time error` occurs if
*C1* is *better* for one argument, and *C2* is *better* for another argument
as represented in the example below:

.. code-block:: typescript
   :linenos:

   function goo(a: int; b: int | string)  // #1
   function goo(a: int | string, b: int)  // #2

   goo(1, 1) // compile-time error, as
             // #1 is better for 1st argument,
             // #2 is better for 2nd argument.

.. index::
   best candidate
   argument
   better candidate


If exactly one candidate is *better* than others,
then it is the *best candidate*.
Otherwise, if no single candidate is *better*,
:index:`compile-time error` occurs.

|

.. _Type Erasure:

Type Erasure
*************

*Type erasure* is the concept that denotes a special handling of some language
*types*, primarily :ref:`Generics`, in the semantics of the following language
operations that require the type to be preserved for execution:

-  :ref:`InstanceOf Expression`;
-  :ref:`Cast Expression`;
-  :ref:`Overload-Equivalent Signatures`.

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
   overload-equivalent signature
   operation
   type
   effective type
   type mapping
   supertype

In addition, accessing a value of type ``T``, including by
:ref:`Field Access Expression`, :ref:`Method Call Expression`, or
:ref:`Function Call Expression` can cause ``ClassCastError`` thrown if
type ``T``and the ``target`` type are both affected by *type erasure*, and the
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

*Static initialization* is a routine performed once for each class
(see :ref:`Classes`), namespace (see :ref:`Namespace Declarations`),
separate module (see :ref:`Separate Modules`) or package module (see :ref:`Packages`).

*Static initialization* execution involves execution of:

- *Initializers* of *variables* or *static fields*

- *Top-level statements*

- Code inside *Static block*


*Static initialization* is performed before one of the following operations is first excecuted:

- a static method or function of entity's scope is invoked

- a static field or variable of entity's scope is accessed

- entity, which is an interface or class, is instantiated

- entity is a class, and its direct subclass is *statically initialized*

Note: Any of the enlisted operations does not invoke *static initialization*
recursively if the *static initializaton* of the same entity is not complete.

If *static initialization* routine execution is terminated due to the
exception thrown, then the initialization is not complete,
and any attempt to execute its *static initialization* once again will
produce an exception.

For the concurrent execution (see :ref:`Coroutines (Experimental)`)
*static initialization* routine invokation involves synchronization
between all *coroutines* that try to invoke it to ensure that
initialization is performed only once and the operations
that require *static initialization* to be performed are executed after
the initialization completes.

If *static initialization* routines of two concurrently initialized classes
has a circular dependence, it may lead to deadlock.

|

.. _Static Initialization Safety:

Static Initialization Safety
============================

If a *named reference* refers to a not yet initialized *entity*, including

- variable (see :ref:`Variable and Constant Declarations`) of a separate module
  package (see :ref:`Packages`), or namespace (see :ref:`Namespace Declarations`)

- a static field of the class (see :ref:`Static Fields`)

then a compile-time error is produced.

If it is not possible to detect an access to a not yet initalized *entity*,
then the runtime evaluation is performed as follows:

- If type of the entity has a default value, then a default value is produced

- Otherwise, ``NullPointerError`` is thrown


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
