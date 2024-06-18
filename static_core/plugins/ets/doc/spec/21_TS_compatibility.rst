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

.. _|LANG| |TS| compatibility:

|LANG|-|TS| compatibility
#########################

.. meta:
    frontend_status: None

This section discusses all issues related to different compatibility aspects
between |LANG| and |TS|.


.. _Reserved Names of TS Types:

Reserved Names of |TS| Types
****************************

The following tables list words that are reserved and cannot be used as
user-defined type names, but are not otherwise restricted.

.. index::
   reserved names of |TS| types

1. Names of |TS| utility types that are not supported by |LANG|:

+---------------------------+-----------------------+-----------------------+
|                           |                       |                       |
+===========================+=======================+=======================+
| ``Awaited``               | ``NoInfer``           | ``Pick``              |
+---------------------------+-----------------------+-----------------------+
| ``ConstructorParameters`` | ``NonNullable``       | ``ReturnType``        |
+---------------------------+-----------------------+-----------------------+
| ``Exclude``               | ``Omit``              | ``ThisParameterType`` |
+---------------------------+-----------------------+-----------------------+
| ``Extract``               | ``OmitThisParameter`` | ``ThisType``          |
+---------------------------+-----------------------+-----------------------+
| ``InstanceType``          | ``Parameters``        |                       |
+---------------------------+-----------------------+-----------------------+

2. Names of |TS| utility string types that are not supported by |LANG|:

+----------------+-------------------+
|                |                   |
+================+===================+
| ``Capitalize`` | ``Uncapitalize``  |
+----------------+-------------------+
| ``Lowercase``  | ``Uppercase``     |
+----------------+-------------------+

3. Class names from |TS| standard library that are not supported by |LANG|
standard library:

+---------------------------+-------------------------+-----------------------------+
|                           |                         |                             |
+===========================+=========================+=============================+
| ``ArrayBufferTypes``      | ``Function``            | ``Proxy``                   |
+---------------------------+-------------------------+-----------------------------+
| ``AsyncGenerator``        | ``Generator``           | ``ProxyHandler``            |
+---------------------------+-------------------------+-----------------------------+
| ``AsyncGeneratorFunction``| ``GeneratorFunction``   | ``Symbol``                  |
+---------------------------+-------------------------+-----------------------------+
| ``AsyncIterable``         | ``IArguments``          | ``TemplateStringsArray``    |
+---------------------------+-------------------------+-----------------------------+
| ``AsyncIterableIterator`` | ``IteratorYieldResult`` | ``TypedPropertyDescriptor`` |
+---------------------------+-------------------------+-----------------------------+
| ``AsyncIterator``         | ``NewableFunction``     |                             |
+---------------------------+-------------------------+-----------------------------+
| ``CallableFunction``      | ``PropertyDescriptor``  |                             |
+---------------------------+-------------------------+-----------------------------+

|

.. _No undefined as universal value:

Undefined is Not a Universal Value
**********************************

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


.. _Numeric semantics:

Numeric Semantics
*****************

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


.. _Covariant overriding:

Covariant Overriding
********************

.. meta:
    frontend_status: Done

|TS| object runtime model enables |TS| to handle situations where a
non-existing property is accessed from some object during program execution.

|LANG| allows generating highly efficient code that relies on an objects'
layout known at compile time. Covariant overriding (see :ref:`Covariance`)
is prohibited because type-safety violations are prevented
by compiler-generated compile-time errors:

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
           p.field = 666 // Access the field
           console.log ("p.field assigned   = ", p.field)
              // TypeScript will print 'p.field assigned   =  666'
           p.method() // Call the method
              // TypeScript will generate runtime error: p.method is not a function
       }
       method () {}
       field: number = 0
    }

    let base: Base = new Derived
    base.foo (new Base)


.. _Subtyping for utility types:

Subtyping for utility types
===========================

.. meta:
    frontend_status: None

In |LANG|, utility type ``Partial<T>`` is not a supertype for ``T`` and thus,
variables of this type are to be initialized with object literals only.

.. code-block:: typescript
   :linenos:
    
    function <T> foo(t: T, part_t: Partial<T>) {
        part_t = t // compile-time error in ArkTS
    }


.. _Difference in Overload Signatures:

Difference in Overload Signatures
*********************************

.. meta:
    frontend_status: Partly

*Implementaion signature* is considered as an accessible (see
:ref:`Accessible`) entity. The following code is valid in |LANG| (while it
causes a compile-time error in |TS|):

.. code-block-meta:
   not-subset

.. code-block:: typescript
   :linenos:

    function foo(): void
    function foo(x: string): void
    function foo(x?: string): void {
        /*body*/
    }

    foo(undefined) // compile-time error in Typescript

|LANG| supports calling function or method only with the number of arguments
that corresponds to the number of the parameters. |TS|, in some cases, allows
providing more arguments than the actual function or method has.

.. code-block-meta:
   expect-cte

.. code-block:: typescript
   :linenos:

    function foo(p1: string, p2: boolean): void
    function foo(p: string): void
       { console.log ("1st parameter := ", p)  }

    foo("1st argument", true) // compile-time error in ArkTS while OK for Typescript

|

.. _Class fields while inheriting:

Class fields while inheriting
*****************************

.. meta:
    frontend_status: Done

|TS| allows overriding class fields with the field in the subclass with
the invariant or covariant type, and potentially with a new initial value.

|LANG| supports shadowing if a new field in a subclass is just a physically
different field with the same name.

As a result, the number of fields in a derived object, and the semantics of
*super* can be different. An attempt to access ``super.field_name`` in |TS|
returns *undefined*. However, the same code in |LANG| returns the shadowed
field declared in or inherited from the direct superclass.

These situations are illustrated by the examples below:

.. code-block-meta:

.. code-block:: typescript
   :linenos:


   class Base {
     field: number = 666
   }
   class Derived extends Base {
     field: number = 555
     foo () {
        console.log (this.field, super.field)
     }
   }
   let d = new Derived
   console.log (d)
   d.foo()
   // TypeScript output
   // Derived { field: 555 }
   // 555 undefined
   // ArkTS output
   // { field: 666, field: 555 }
   // 555 666


.. _Overriding for primitive types:

Overriding for primitive types
******************************

|TS| allows overriding class type version of the primitive type into a pure
primitive type. |LANG| does not allow such overriding.

These situation is illustrated by the example below:

.. code-block:: typescript
   :linenos:


   class Base {
     foo(): Number { return 5 }
   }
   class Derived extends Base {
     foo(): number { return 5 } // Such overriding is prohibited
   }


.. _Excessive arguments:

Excessive arguments
*******************

.. meta:
    frontend_status: None

|TS| allows calling function type variables with more arguments. 
|LANG| does not allow such calls.


.. code-block:: typescript
   :linenos:


    let foo: (x?: number, y?: string) => void = ():void => {}
        /* compile-time error in ArkTS as call with more than zero arguments
           will be invalid while OK for the Typescript */

    foo = (p?: number):void => {} 
        /* compile-time error in ArkTS as call with two arguments will be
           invalid while OK for the Typescript */



.. _Differences in Math.pow:

Differences in Math.pow
***********************

.. meta:
    frontend_status: Done

The function ``Math.pow`` in |LANG| conforms to the latest IEEE 754-2019
standard, and the following calls produce the result *1* (one):

- ``Math.pow(1, Infinity)``,
- ``Math.pow(-1, Infinity)``,
- ``Math.pow(1, -Infinity)``,
- ``Math.pow(-1, -Infinity)``.


The function ``Math.pow`` in |TS| conforms to the outdated 2008 version of the
standard, and the same calls produce ``NaN``.

.. index::
   IEEE 754

