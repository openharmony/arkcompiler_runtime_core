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

.. _|LANG| |TS| compatibility:

|LANG|-|TS| compatibility
=========================

.. meta:
    frontend_status: None

This section discusses all issues related to different aspects of compatibility
between |LANG| and |TS|.

|

.. _No undefined as universal value:

Undefined is Not a Universal Value
----------------------------------

.. meta:
    frontend_status: Done

|LANG| raises a compile-time or a runtime error in many cases, in which
|TS| uses ``undefined`` as runtime value.

.. code-block-meta:
   expect-cte

.. code-block:: typescript
   :linenos:

    let array = new Array<number>
    let x = array [1234]
       // TypeScript: x will be assigned with undefined value !!!
       // ArkTS: compile-time error if analysis may detect array out of bounds
       //        violation or runtime error ArrayOutOfBounds
    console.log(x)

.. index::
   value
   runtime value
   runtime error

|

.. _Numeric semantics:

Numeric Semantics
-----------------

.. meta:
    frontend_status: Done

|TS| has a single numeric type ``number`` that handles both integer and real
numbers.

|LANG| interprets ``number`` as a variety of |LANG| types. Calculations depend
on the context and can produce different results:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    let n = 1
       // TypeScript: treats 'n' as having type number
       // ArkTS: treats 'n' as having type int to reach max code performance

    console.log(n / 2)
       // TypeScript: will print 0.5 - floating-point division is used
       // ArkTS: will print 0 - integer division is used

.. index::
   numeric type
   context
   semantics

|

.. _Covariant overriding:

Covariant Overriding
--------------------

.. meta:
    frontend_status: Done

|TS| object runtime model enables |TS| to handle situations where a
non-existing property is accessed from an object during program execution.

|LANG| allows generating highly efficient code that relies on an objects'
layout known at compile time. Covariant overriding
is prohibited because it violates type-safety:

.. code-block:: typescript
   :linenos:

    class Base {
       foo (p: Base) {}
    }
    class Derived extends Base {
       override foo (p: Derived)
          // ArkTS will issue a compile-time error - incorrect overriding
       {
           console.log ("p.field unassigned = ", p.field)
              // TypeScript will print 'p.field unassigned =  undefined'
           p.field = 42 // Access the field
           console.log ("p.field assigned   = ", p.field)
              // TypeScript will print 'p.field assigned   =  42'
           p.method() // Call the method
              // TypeScript will generate runtime error: p.method is not a function
       }
       method () {}
       field: number = 0
    }

    let base: Base = new Derived
    base.foo (new Base)

.. index::
   covariant overriding
   runtime model
   object
   property
   access
   compile time
   type safety

|

.. _Function Types Compatibility:

Function Types Compatibility
----------------------------

.. meta:
    frontend_status: Done

|TS| allows more relaxed assignments into variables of function type.
|LANG| follows stricter rules (see the |LANG| specification for details).

.. code-block:: typescript
   :linenos:

    type FuncType = (p: string) => void
    let f1: FuncType = (p: string): number => { return 0 } // compile-time error in ArkTS
    let f1: FuncType = (p: string): string => { return "" } // compile-time error in ArkTS

.. index::
   function type
   compatibility
   assignment
   variable
   conversion

|

.. _Compatibility for utility types:

Compatibility for Utility Types
-------------------------------

.. meta:
    frontend_status: Done

Utility type ``Partial<T>`` in |LANG| is not assignable to ``T``.
Variables of this type are to be initialized
with object literals only.

.. code-block:: typescript
   :linenos:

    function foo<T>(t: T, part_t: Partial<T>) {
        part_t = t // compile-time error in ArkTS
    }

.. index::
   compatibility
   utility type
   initialization
   object literal

|

.. _TS Overload Signatures:

|TS| Overload Signatures
------------------------

.. meta:
    frontend_status: Done

|LANG| does not support overload signatures in |TS|-style where several function
headers are followed by a single body. Each overloaded function, method, or
constructor is required to have a separate body.

The following code is valid in |TS| but causes a compile-time error in |LANG|:

.. code-block-meta:
   expect-cte

.. code-block:: typescript
   :linenos:

    function foo(): void 
    function foo(x: string): void
    function foo(x?: string): void {
        /*body*/
    }

The following code is valid in |LANG|:

.. code-block-meta:
   not-subset

.. code-block:: typescript
   :linenos:

    function foo(): void {
      /*body1*/
    }
    function foo(x: string): void {
      /*body2*/
    }

|

.. _Class Fields While Inheriting:

Class Fields While Inheriting
-----------------------------

.. meta:
    frontend_status: None

|TS| allows overriding a class field with a field in a subclass of invariant
or covariant type.
|LANG| supports overriding a class field with a field in a subclass of invariant
type only.
In both languages, an overriding field can have a new initial value.

The situations are illustrated by the following examples:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   // Both TypeScript and ArkTS do the same
   class Base {
     field: number = 123
     foo () {
        console.log (this.field)
     }     
   }
   class Derived extends Base {
     field: number = 456
     foo () {
        console.log (this.field)
     }
   }
   let b: Base = new Derived()
   b.foo()  // 456 is printed


   // That will be a compile-time error in ArkTS as type of 'field' in Child
   // differs from 'field' type in Parent
   class Parent {
       field: Object
   }
   class Child extends Parent {
       field: Number 
   }

.. index::
   class field
   inheritance
   overriding
   subclass
   invariant
   covariant
   shadowing
   semantics
   superclass

|

.. _Type void Compatibility:

Type ``void`` Compatibility
---------------------------

.. meta:
    frontend_status: Done

|TS| allows using type ``void`` in union types. |LANG| does not allow ``void``
in union types. This situation is represented by the example below:

.. code-block:: typescript
   :linenos:

   type UnionWithVoid = void | number
     // Such type is OK for Typescript, but leads to a compile-time error for ArkTS

|

.. _Invariant Array Assignment:

Invariant Array Assignment
--------------------------

.. meta:
    frontend_status: None

|TS| allows covariant array assignment.
|LANG| allows invariant array assignment only:

.. code-block:: typescript
   :linenos:

    // Typescript
    let a: Object[] = [1, 2, 3]
    let b = [1, 2, 3] // type of 'b' is inferred as number[]
    a = b // That works well for the Typescript

    // ArkTS
    let a: Object[] = [1, 2, 3]
    let b = [1, 2, 3] // type of 'b' is inferred as double[]
    a = b // compile-time error

    let a: Object[] = ["a", "b", "c"]
    let b: string[] = ["a", "b", "c"]
    a = b // compile-time error

.. index::
   covariant array assignment
   invariant array assignment
   array
   assignment

|

.. _Tuples and Arrays:

Tuples and Arrays
-----------------

.. meta:
    frontend_status: None

|TS| allows assignments of tuples into arrays. |LANG| handles arrays and tuples
as different types, and does not allow assignment of tuples into arrays. This
situation is represented by the folowing example:

.. code-block:: typescript
   :linenos:

   const tuple: [number, number, boolean] = [1, 3.14, true]

   // Typescript accepts such assignment while ArkTS reports an error
   const array: (number|boolean) [] = tuple


|

.. _Extending Class Object:

Extending Class Object
----------------------

.. meta:
    frontend_status: Done

|TS| forbids using ``super`` and ``override`` if class ``Object`` is not
listed explicitly in the ``extends`` clause of a class. |LANG| allows this as
``Object`` is a superclass for any class without an explicit ``extends`` clause:

.. code-block:: typescript
   :linenos:

    // Typescript reports an error while ArkTS compiles with no issues
    class A {
       override toString() {       // compile-time error
           return super.toString() // compile-time error
       }
    }

    class A extends Object { // That is the form supported by TypeScript
       override toString() {
           return super.toString()
       }
    }

.. index::
   class object
   extends clause

|

.. _Syntax of extends and implements Clauses:

Syntax of ``extends`` and ``implements`` Clauses
------------------------------------------------

.. meta:
    frontend_status: Done

|TS| handles entities listed in ``extends`` and ``implements`` clauses as
expressions.
|LANG| handles such clauses at compile time, and allows no expressions
but *type references*:

.. code-block:: typescript
   :linenos:

    class B {}
    class A extends (B) {} // compile-time error for ArkTS while accepted by TypeScript


.. index::
   extends clause
   implements clause

|

.. _Uniqueness of Functional Objects:

Uniqueness of Functional Objects
--------------------------------

.. meta:
    frontend_status: Done

|TS| and |LANG| handle function objects differently, and the equality test can
perform differently. The difference can be eliminated in the future versions of
|LANG|.

.. code-block:: typescript
   :linenos:

    function foo() {}
    foo == foo  // true in Typescript while may be false in ArkTS
    const f1 = foo
    const f2 = foo
    f1 == f2 // true in Typescript while may be false in ArkTS


.. index::
   function object
   equality test

|

.. _Functional Objects for Methods:

Functional Objects for Methods
------------------------------

.. meta:
    frontend_status: None

|TS| and |LANG| handle function objects differently, and the sematics of the 
work with 'this' is different. |TS| supports *undefined* as a value of 'this',
while |LANG| has 'this' always attached to a valid object.

.. code-block:: typescript
   :linenos:

    class A {
      method() { console.log (this) }
    }
    const a = new A
    const method = a.method
    method() // Typescript output: undefined, while ArkTS output: object 'a' content


.. index::
   function object
   this


|

.. _Absence of Type Function:

Absence of Type ``Function``
----------------------------

.. meta:
    frontend_status: Done

|TS| has a supertype for all function types called *Function*. |LANG| does not
have such type.

.. code-block:: typescript
   :linenos:

    // Typescript code style: type unsafe
    function calls (functor: Function) {
        functor ()
        functor (1)
        functor (1, true, "some string")
    }

    // ArkTS code style: type safe
    function calls (functor: Object) {
        if (functor instanceof (()=>void)) functor ()
        if (functor instanceof ((p: number)=>void)) functor (1)
        if (functor instanceof ((p: number)=>void)) functor (1, true, "some string")
    }


|

.. _Differences in Namespaces:

Differences in Namespaces
-------------------------

.. meta:
    frontend_status: Done

|TS| allows having non-exported entities with the same name in two or more
different declarations of a namespace, because these entities are local to a
particular declaration of the namespace. Such situations are forbidden in
|LANG|, because this language merges all declarations into one and declarations
become non-distinguishable:


.. code-block:: typescript
   :linenos:

    // Typescript accepts such code, while ArkTS will report a compile-time error
    // as signatures of foo() from the 1st namespace A is identical to the signature
    // of foo() from the 2nd namespace A
    namespace A {
       function foo() { console.log ("foo() from the 1st namespace A declaration") }
       export function bar () { foo() }
    }
    namespace A {
       function foo() { console.log ("foo() from the 2nd namespace A declaration") }
       export function bar_bar() { foo() }
    }

|

.. _Differences in Math.pow:

Differences in Math.pow
-----------------------

.. meta:
    frontend_status: Done

The function ``Math.pow`` in |LANG| conforms to the latest IEEE 754-2019
standard. The following calls produce the result *1* (one):

- ``Math.pow(1, Infinity)``,
- ``Math.pow(-1, Infinity)``,
- ``Math.pow(1, -Infinity)``,
- ``Math.pow(-1, -Infinity)``.

The function ``Math.pow`` in |TS| conforms to the outdated 2008 version of the
IEEE 754-2019 standard. The same calls as listed above produce ``NaN`` in |TS|.

.. index::
   IEEE 754

|

.. _Differences in Constructor Body:

Differences in Constructor Body
-------------------------------

Work is in progress to have support for the corner cases of mandatory calls to
*super()* or *this()* in the compiler for |LANG|. So, the code below will be
temporarily rejected. Compiler requires call to *super()* or *this()* to be
not embedded into other constructions.

.. code-block:: typescript
   :linenos:

    class A {
       constructor (p: number) {}
       constructor (p: boolean) {  // Compile_time error: incorrect constructor body
           if (p) { this (1) }
           else { this (2) }
       }     
    }

