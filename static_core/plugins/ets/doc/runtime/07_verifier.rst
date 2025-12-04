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

.. _Verifier:

Verifier
********

Verifier performs several checks on abc files.


Checks performed on abstract interpretation stage
=================================================

Physical compatibility of arguments to instructions and actual parameters to methods
------------------------------------------------------------------------------------

This type of checks eliminate rutime problems with undefined bits in
integers, truncation issues, etc.

From security point of view, this checks guarantee expected ranges of
values in code and absence of handling undefined information.

Access checks
-------------

Checks for private/protected/public access rights.

These checks prevent unintended/unexpected access from one method to
another. Or access to wrong fields of object.

Checks of subtyping
-------------------

Checks of compatibility of objects in arguments to instructions and
actual parameters to methods.

These checks eliminate calls of methods with incorrect ``this``, wrong
access to arrays, etc.

Checks of exception handlers
----------------------------

These checks performed to check correctness of context on exception
handler entry.

They can help to detect usage of inconsistent information in registers
in exception handlers.

Checks of exceptions, that can be thrown in runtime
---------------------------------------------------

Some code may exibit behavior of permanently throwing of exceptions,
like always throwing NPE.

This is definitely not normal mode of control-flow in code, so verifier
can detect such situations (when code always throws an exception).

Check of return values from methods
-----------------------------------

Can help inconsistency between method signature and type of actual
return value

(todo) Simple range checks of primitive types
---------------------------------------------

These checks help to detect issues with unintended
truncation/overflow/underflow etc.

(todo) Simple bounds checks
---------------------------

These checks help in some cases detect out-of-bounds access type of
errors in static.

(todo) Checks for usage of some functions/intrinsics
----------------------------------------------------

For instance, check symmetry of monitorEnter/monitorExit calls to avoid
deadlocking.

Control Flow Checks
===================

Current layout model of a method
--------------------------------

::

   =========
      ...
     code
      ...
   ---------
      ...
   exception
   handler
      ...
   ---------
      ...
     code
      ...
   ---------
      ...
   exception
   handler
      ...
    -------
    inner
    exc.
    handler
    -------
      ...
   ---------
      ...
     code
      ...
   =========

I.e. layout of exception handlers is rather flexible, even handler in
handler is allowed.

Cflow transitions, which are subjects for checks
------------------------------------------------

Execution beyond method body
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

   =========
      ...
     code
      ...
   ---------
      ...
   exception
   handler
      ...
   ---------
      ...
     code
      ...
     ldai 0 ---\
   =========   |
          <----/

::

   =========
      ...
     code
      ...
   ---------
      ...
   exception
   handler
      ...
      jmp -----\
      ...      |
   =========   |
          <----/

Mis-jumps, or improper termination of cflow at the end of the body are
prohibited.

::

   =========
      ...
     code
      ...
   ---------
      ...
   exception
   handler
      ...
   ---------
      ...
   lbl:  <-----\
      ...      |
     code      |
      ...      |
     jeqz lbl -+
   =========   |
          <----/

Conditional jumps are in grey zone, if they may be proven as always jump
into code, then they will be considered ok. Currently, due to
imprecision of verifier, conditional jumps at the end of the method are
prohibited.

Code to exception handler
~~~~~~~~~~~~~~~~~~~~~~~~~

direct jumps:

::

   =========
      ...
     code
      ...
      jmp catch1--\
      ...         |
   ---------      |
   catch1: <------/
      ...
   exception
   handler
      ...
   ---------
      ...

fallthrough:

::

   =========
      ...
     code
      ...
      ldai 3 --\
   ---------   |
   catch1: <---/
      ...
   exception
   handler
      ...
   ---------
      ...

By default only ``throw`` transition is allowed. Neither ``jmp``, nor
fallthrough on beginning of exception handler are allowed.

This behavior may be altered by option ``C-TO-H``.

Code into exception handler
~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

   =========
      ...
     code
      ...
      jmp lbl1  --\
      ...         |
   ---------      |
   catch:         |
      ...         |
   lbl1:     <----/
      ldai 3
      ...
   exception
   handler
      ...
   ---------
      ...

Jumps into body of exception handler from code is prohibited by default.

Handler to handler
~~~~~~~~~~~~~~~~~~

direct jumps:

::

   =========
      ...
     code
      ...
   ---------
   catch1:
      ...
   exception
   handler
      ...
      jmp catch2--\
      ...         |
   ---------      |
   catch2: <------/
      ...
   exception
   handler
      ...
   ---------
      ...

fallthrough:

::

   =========
      ...
     code
      ...
   ---------
   catch1:
      ...
   exception
   handler
      ...
      ldai 3 --\
   ---------   |
   catch2: <---/
      ...
   exception
   handler
      ...
   ---------
      ...

By default such transition of control flow is prohibited.

Handler into handler
~~~~~~~~~~~~~~~~~~~~

direct jumps:

::

   =========
      ...
     code
      ...
   ---------
   catch1:
      ...
   exception
   handler
      ...
      jmp lbl  ---\
      ...         |
   ---------      |
   catch2:        |
      ...         |
   lbl:    <------/
      ldai 3
      ...
   exception
   handler
      ...
   ---------
      ...

fallthrough from inner handler:

::

   =========
      ...
     code
      ...
   ---------
   catch1:
      ...
   outer
   exception
   handler
      ...
    -------
   catch2:
      ...
   lbl:
      ldai 3
      ...
    inner
    exc.
    handler
      ...
     ldai 0  --\
    -------    |
      ...   <--/
   outer
   exc.
   handler
      ...
   ---------
      ...

By default such cflow transitions are prohibited.

Handler into code
~~~~~~~~~~~~~~~~~

::

   =========
      ...
     code
      ...
   lbl:   <-------\
      ...         |
   ---------      |
      ...         |
   exception      |
   handler        |
      ...         |
      jmp lbl  ---/
      ...
   ---------
      ...

By default such jumps are prohibited currently.

Type system implementation
--------------------------

Type system is simple: there are only parametric type families, named as
``Sort``\ s, and their instances, named as ``Type``\ s.

For simplicity, all literals without parens are Sorts, with parens -
Types.

Special types
~~~~~~~~~~~~~

- Bot() - subtype of all types, subtyping relation is made implicitly
  upon type creation.
- Top() - supertype of all types, subtyping rel is created implicitly.

Sorts
-----

Internally they denoted by indices (just numbers), and there is separate
class to hold their names. For type_system description technical details
can be ommited and Sorts will be understood in this text just as
literals.

Types
-----

Internally they are just indices.

A sort, acompanied with a list of parameters, becomes a type.

Each parameter consists of: a sign of variance for calculating subtyping
relation and a type.

Variances are: - ``~`` invariant, means that corresponding parameter in
subtyping relation must be in subtype and supertype relation
simultaneously - ``+`` covariant, means that suptyping relation of
parameter is in direction of subtyping relation of type. - ``-``
contrvariant, direction of suptyping of parameter is in opposite to such
of type.

Examples
--------

Sorts:

- ``i8``
- ``Math``
- ``function``

Types:

- ``i8()``

- ``i16()``

- ``i32()``

- ``function(-i32(), -i16(), +i8())``

- ``function(-function(-i8(), +i16()), +i16())``

Subtyping relations (``subtype <: supertype``):

- ``i8() <: i16() <: i32()``
- ``function(-i32(), -i16(), +i8()) <: function(-i16(), -i8(), +i32())``
- ``function(-function(-i8(), +i16()), +i16()) <: function(function(-16(), +8()), +i32())``

Closure
-------

After defining base types and initial subtyping realtion, a closure of
subtyping relation is computed. This helps to speed up subtyping
checking during verification.

Loops in subtyping relation are threated as classes of equivalence, for
instance: ``a() <: b() <: c() <: a()`` leads to indistinguishability of
a(), b() and c() in type system.

.. _types-1:

Types
=====

Types are divided into next kinds:

1. Semantic (abstract) types. They used only for values classification
   without taking into consideration any ohter information like storage
   size, bit width, etc.
2. Storage types
3. Physycal (concrete) types. They are parameterized types by abstract
   and storage types.

Type structure
==============

Types are formed by some ``Sort``, an uniq identifier of the type
family, and particular parameters.

``Sort`` + ``parameters`` = ``Type``.

Each parameter is a some type accompanied with a variance flag.

Variance of the parameter is classic: covariant, contrvariant,
invariant. It defines subtyping relation of types in parameters in
subtyping relation of parameterized types.

Some conventions about notation:

- Sorts are denoted by some ``Literals``. Where ``Literal`` is a word
  composed of characters from set ``[0-9a-zA-Z_]``. Examples: ``Array``,
  ``Function``, ``ref``
- Types are denoted by sort literal and parameters in parenthesis. For
  instance: ``Array(~i8())``.
- Type parameters are just types prepended by variance sign, ``-`` -
  contrvariant, ``+`` - variant, ``~`` - invariant. Examples: ``+i8()``,
  ``+Function(-i8(), +i16())``, etc.

Subtyping relation
==================

Subtyping relation is denoted by ``<:``. Subtyping is a realtion on
types which is used to determine if the particular value of the
particular type may be used in the place where other type is expected.

If two types, say ``A()`` and ``B()`` related as ``A() <: B()``,
i.e. ``A()`` is the subtype of ``B()``, then in every place, where a
value of type ``B()`` is expected, value of type ``A()`` may be safely
used.

By default, it is considered, that a type is in subtyping relation with
itself, i.e. ``A() <: A()`` for every type in the type universe.

Useful short notations for subtyping relation:

- ``A() <: B() <: C()`` - means ``A() <: B()``, ``A() <: C()`` and
  ``B() <: C()``

- ``(A() | B()) <: C()`` - means ``A() <: C()`` and ``B() <: C()``

- ``(A() | B()) <: (C() | D())`` - means ``A() <: C()``, ``B() <: C()``,
  ``A() <: D()`` and ``B() <: D()``

In short ``|`` means composition of types (syntactically) in set, and
``<:`` is distriuted over ``|``.


.. How to read notation of type parameters and to determine subtyping relation?
   ----------------------------------------------------------------------------

Suppose, we have types ``T(+i8())`` and ``T(+i16())`` and
``i8() <: i16()``, how to relate types ``T(...)``?

May be relation is ``T(+i16()) <: T(+i8())``? Let’s see, according to
``+`` (covariance), relation of types of parameters should be the same
as of ``T(...)``, i.e. ``i16() <: i8()``. And that is obviously wrong,
because of the initial condition ``i8() <: i16()``. I.e. we have a
contradiction here.

Let’s check ``T(+i8()) <: T(+i16())``. So we have ``i8() <: i16()`` for
the first parameters which is in line with initial conditions. So,
finally, subtyping relation is ``T(+i8()) <: T(+i16())``.


Type universe
=============

All kinds of types live in the same type universe under the same subtyping
relation.

It seems to me, that single univerce of parameterized types with properly
defined subtyping relation is enough to deal with all cases of types usage in
VM.

Type system includes two special types - initial and final:

- ``Bot`` - is a subtype of all types
- ``Top`` - is a supertype of all types

So for every type ``Bot <: T <: Top``

Classes of equivalence of types
===============================

Types which are in circular subtyping relation, like ``A <: B <: C <: A``, are
indistinguishable in the type system. They form a single class of equivalence.


Type hieararchy
===============

Here is a basic type hierarchy of Panda VM.

Abstract types
--------------

Primitive types
~~~~~~~~~~~~~~~

Integral types
^^^^^^^^^^^^^^

- Signed integers

- Unsigned ones

- Integral

Floating point types
^^^^^^^^^^^^^^^^^^^^

Arrays
^^^^^^

``Array(+T)`` - where T is type of elements.

Objects/Records
^^^^^^^^^^^^^^^


TODO

Storage types
-------------

TODO

Concrete types
--------------

TODO
