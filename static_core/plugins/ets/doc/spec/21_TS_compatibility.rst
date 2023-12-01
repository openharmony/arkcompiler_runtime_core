..
    Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

This section discusses all issues related to different compatibiliuty aspects
between |LANG| and |TS|.



.. _No undefined as universal value:

Undefined is Not a Universal Value
**********************************

|LANG| raises a compile-time or a runtime error in many cases, in which
|TS| uses *undefined* as runtime value.

.. code-block:: typescript
   :linenos:

    let array = new Array<number>
    let x = array [1234]
       // |TS|: x will be assigned with undefined value !!!
       // |LANG|: compile-time error if analysis may detect array out of bounds
       //         violation or runtime error ArrayOutOfBounds
    console.log(x)


.. _Numeric semantics:

Numeric Semantics
*****************

|TS| has a single numeric type *number* that handles both integer and real
     numbers.
|LANG| interprets *number* as a variety of |LANG| types depending on the
     context, and calculations can produce different results:

.. code-block:: typescript
   :linenos:

    let n = 1
       // |TS|: treats 'n' as having type number
       // |LANG|: treats 'n' as having type int to reach max code performance


    console.log(n / 2)
       // |TS|: will print 0.5 - floating-point division is used
       // |LANG|: will print 0 - integer division is used


.. _Covariant overriding:

Covariant Overriding
********************

|TS| if non-existing property is accessed from some object during program
     execution |TS| runtime may handle such situtions due to its object
     runtime model.
|LANG| allows to generate high efficient code which relies on objects'
     layout known at compile-time. Thus type-safety violations are prevented
     by the compiler generating compile-time errors.

.. code-block:: typescript
   :linenos:

    class Base {
       foo (p: Base) {}
    }
    class Derived extends Base {
       override foo (p: Derived)
          // |LANG| will issue a compile-time error - incorrect overriding
       {
           console.log ("p.field unassigned = ", p.field)
              // |TS| will print 'p.field unassigned =  undefined'
           p.field = 666 // Access the field
           console.log ("p.field assigned   = ", p.field)
              // |TS| will print 'p.field assigned   =  666'
           p.method() // Call the method
              // |TS| will generate runtime error: TypeError: p.method is not a function
       }
       method () {}
       field: number = 0
    }

    let base: Base = new Derived
    base.foo (new Base)




