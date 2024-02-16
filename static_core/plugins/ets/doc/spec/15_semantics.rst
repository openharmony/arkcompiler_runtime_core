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

Some details are omitted to simplify the understanding. See the
formal language description for more information.

|

.. _Subtyping:

Subtyping
*********

.. meta:
    frontend_status: Done

The *subtype* relationships are binary relationships of types.

The subtyping relation of *S* as a subtype of *T* is recorded as *S* <: *T*.
It means that any object of type *S* can be safely used in any context
in place of an object of type *T*.

By the definition of *S* <: *T*, type *T* belongs to the set of *supertypes*
of type *S*. The set of *supertypes* includes all *direct supertypes* (see
below), and all their respective *supertypes*.

.. index::
   subtyping
   binary relationship
   object
   type
   direct supertype
   supertype

More formally speaking, the set is obtained by reflexive and transitive
closure over the direct supertype relation.

*Direct supertypes* of a non-generic class, or of the interface type *C*
are **all** of the following:

-  The direct superclass of *C* (as mentioned in its extension clause, see
   :ref:`Class Extension Clause`) or type *Object* if *C* has no extension
   clause specified.

-  The direct superinterfaces of *C* (as mentioned in the implementation
   clause of *C*, see :ref:`Class Implementation Clause`).

-  Type *Object* if *C* is an interface type with no direct superinterfaces
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

*Direct supertypes* of the generic type *C* <*F*:sub:`1`,..., *F*:sub:`n`>
(for a generic class or interface type declaration *C* <*F*:sub:`1`,..., *F*:sub:`n`>
with *n*>0) are **all** of the following:

-  The direct superclass of *C* <*F*:sub:`1`,..., *F*:sub:`n`>.

-  The direct superinterfaces of *C* <*F*:sub:`1`,..., *F*:sub:`n`>.

-  Type *Object* if *C* <*F*:sub:`1`,..., *F*:sub:`n`> is a generic
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

.. _Override-Equivalent Signatures:

Override-Equivalent Signatures
******************************

.. meta:
    frontend_status: Done

Two functions, methods, or constructors *M* and *N* have the *same signature*
if their names, type parameters (if any, see :ref:`Generic Declarations`), and
formal parameter types are the same---after the formal parameter types of
*N* are adapted to type parameters of *M*.

Formal definition for *the same signatures* is given below:

M < *T*:sub:`1`, ... *T*:sub:`Mm` > ( *U*:sub:`1` , ... *U*:sub:`Mn` ): *R*:sub:`M`

N < *T*:sub:`1`, ... *T*:sub:`Nm` > ( *U*:sub:`1` , ... *U*:sub:`Nn` ): *R*:sub:`N`

- Mm = Nm and for any i in 1 .. Mm => Constraint ( *T*:sub:`Mi` ) fits Constraint ( *T*:sub:`Ni` )
- Mn = Nn and for any i in 1 .. Mn => Constraint ( *U*:sub:`Mi` ) fits Constraint ( *U*:sub:`Ni` )

  * where Constraint of any type except type parameter returns the type itself, and
  * *fits* means the following:

    - The types are identical;
    - The data value sets of the first and the second ones have an intersection
      that leads to potential call ambiguities; or
    - Type T and rest parameters of T[] have a potential ambiguity.


.. code-block:: typescript

    type T1 = A | B
    type T2 = A | C
    // Types T1 and T2 fit each other as their data values have intersection with data values of type A


Signatures *S*:sub:`1` and *S*:sub:`2` are *override-equivalent* only if
*S*:sub:`1` and *S*:sub:`2` are the same.

A compile-time error occurs if:

-  A package declares two or more functions with *override-equivalent*
   signatures.

-  A class declares two or more methods or constructors with
   *override-equivalent* signatures.

-  An interface declares two or more methods with *override-equivalent*
   signatures.

The examples below illustrate the concept:

.. code-block:: typescript

   // The same signatures

   foo <T1, T2> ()
   foo <G1, G2> ()
   // The same number of type parameters and their constraints are identical

   foo <T extends U1> (p1: U1, p2: U2)
   foo <V extends U1> (r1: V, r2: U2)
   /* The same number of parameters and their types are identical replacing
      type parameter with its constraint */

   foo (p1: U1, p2: U2): R1
   foo (q1: U1, q2: U2): R2
   // The same number of parameters and their types are identical

   class Base {}
   class Derived extends Base {}

   foo (p: Base)
   foo (p: Derived)
   /* The same number of parameters and intersection of data values of Derived
      and Base produce data set of Derived values */

   foo (p: A | B)
   foo (p: A | C)
   /* The same number of parameters and intersection of data values of A | B
      and A | C produce data set of A values */

   foo (p1: String)
   foo (...p2: String[])
   // The same due to ambiguity of the type String and rest parameter String[]
   // foo("some string") fits both signatures


   // Different signatures

   foo (p1: String)
   foo (p2: String[])
   // As signatures String and String[] do not lead to call ambiguities
   // foo ("some string") fits the first signature
   // foo (["some string"]) fits the second one


.. index::
   override-equivalent signature
   function
   method
   constructor
   signature
   type parameter
   generic declaration
   formal parameter type

|

.. _Compatible Signature:

Compatible Signature
********************

.. meta:
    frontend_status: None

Signature *S*:sub:`1` with *n* parameters is compatible with the signature
*S*:sub:`2` with *m* parameters if:

-  *n <= m*;
-  All *n* parameter types in *S*:sub:`2` are compatible (see :ref:`Type Compatibility`)
   with parameter types in the same positions in *S*:sub:`1`; and
-  All *S*:sub:`2` parameters in positions from *m - n* up to *m* are optional
   (see :ref:`Optional Parameters`).

A return type, if available, is present in both signatures, and the return
type of *S*:sub:`1` is compatible (see :ref:`Type Compatibility`) with the
return type of *S*:sub:`2`.

|

.. _Overload Signature Compatibility:

Overload Signature Compatibility
********************************

If several functions, methods, or constructors share the same body
(implementation) or the same method with no implementation in an interface,
then all first signatures without body must *fit* the last signature with or
without the actual implementation for the interface method. Otherwise, a
compile-time error occurs.

Signature *S*:sub:`1` with *n* parameters *fits* signature *S*:sub:`2`
if:

- *S*:sub:`1` has *n* parameters, *S*:sub:`2` has *m* parameters; and:
  
   -  *n <= m*;
   -  All *n* parameter types in *S*:sub:`1` are compatible (see
      :ref:`Type Compatibility`) with parameter types in the same positions
      in *S*:sub:`2`; and
   -  If *n < m*, then all *S*:sub:`2` parameters in positions from *n + 1*
      up to *m* are optional (see :ref:`Optional Parameters`).

- Both *S*:sub:`1` and *S*:sub:`2` have return types, and the return type of
  *S*:sub:`2` is compatible with the return type of *S*:sub:`1` (see
  :ref:`Type Compatibility`).

It is illustrated by the example below:

.. code-block:: typescript
   :linenos:

   class Base { ... }
   class Derived1 extends Base { ... }
   class Derived2 extends Base { ... }
   class SomeClass { ... }

   interface Base1 { ... }
   interface Base2 { ... }
   class Derived3 implements Base1, Base2 { ... }

   function foo (p: Derived2): Base1 // signature #1
   function foo (p: Derived1): Base2 // signature #2
   function foo (p: Derived2): Base1 // signature #1
   function foo (p: Derived1): Base2 // signature #2
   // function foo (p: SomeClass): SomeClass 
      // Error as 'SomeClass' is not compatible with 'Base'
   // function foo (p: number) 
      // Error as 'number' is not compatible with 'Base' and implicit return type 'void' also incompatible with Base
   function foo (p1: Base, p2?: SomeClass): Derived3 // // signature #3: implementation signature
       { return p }


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

.. index::
   type compatibility
   conversion

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
be handled as *true* or *false* as described in the table below:

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

+-----------------+-------------------+--------------------+----------------------+
| Value Type      | When *false*      | When *true*        | |LANG| Code          |
+=================+===================+====================+======================+
| string          | empty string      | non-empty string   | s.length == 0        |
+-----------------+-------------------+--------------------+----------------------+
| boolean         | false             | true               | x                    |
+-----------------+-------------------+--------------------+----------------------+
| enum            | enum constant     | enum constant      | x.getValue()         |
|                 | treated as 'false'| treated as 'true'  |                      |
+-----------------+-------------------+--------------------+----------------------+
| number          | 0 or NaN          | any other number   | n != 0 && n != NaN   |
| (double/float)  |                   |                    |                      |
+-----------------+-------------------+--------------------+----------------------+
| any integer type| == 0              | != 0               | i != 0               |
+-----------------+-------------------+--------------------+----------------------+
| char            | == 0              | != 0               | c != c'0'            |
+-----------------+-------------------+--------------------+----------------------+
| let T - is any non-nullish type                                                 |
+-----------------+-------------------+--------------------+----------------------+
| T | null        | == null           | != null            | x != null            |
+-----------------+-------------------+--------------------+----------------------+
| T | undefined   | == undefined      | != undefined       | x != undefined       |
+-----------------+-------------------+--------------------+----------------------+
| T | undefined   | == undefined or   | != undefined and   | x != undefined &&    |
| | null          | == null           | != null            | x != null            |
+-----------------+-------------------+--------------------+----------------------+
| Boxed primitive | primitive type is | primitive type is  | new Boolean(true) == |
| type (Boolean,  | false             | true               | true                 |
| Char, Int ...)  |                   |                    | new Int (0) == 0     |
+-----------------+-------------------+--------------------+----------------------+
| any other       | never             | always             | new SomeType != null |
| nonNullish type |                   |                    |                      |
+-----------------+-------------------+--------------------+----------------------+

The example below illustrates the way this approach works in practice. Any
*nonzero* number is handled as *true*. The loop continues until it becomes
*zero* that is handled as *false*:

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


