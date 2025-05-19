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

.. _Enumerations:

Enumerations
############

.. meta:
    frontend_status: Done

Enumeration type ``enum`` specifies a distinct user-defined type with an
associated set of named constants that define its possible values:

.. code-block:: abnf

    enumDeclaration:
        'const'? 'enum' identifier '{' enumConstantList? '}'
        ;

    enumConstantList:
        enumConstant (',' enumConstant)* ','?
        ;

    enumConstant:
        identifier ('=' constantExpression)?
        ;

.. index::
   enumeration
   type enum
   user-defined type
   constant

Type ``const enum`` is supported for source-level compatibility with |TS|.
Type ``const`` is skipped as it has no impact on ``enum`` semantics in
|LANG|.

Qualification by type is mandatory to access the enumeration constant:

.. code-block:: typescript
   :linenos:

    enum Color { Red, Green, Blue }
    let c: Color = Color.Red

If enumeration type is exported, then all enumeration constants are
exported along with the mandatory qualification.

For example, if *Color* is exported, then all constants like ``Color.Red``
are exported along with the mandatory qualification ``Color``.

.. index::
   source-level compatibility
   semantics
   qualification
   access
   enumeration constant
   enumeration type
   enum constant
   const enum

The value of an enum constant can be set as follows:

-  Explicitly to a numeric constant expression (expression of type ``int`` or
   ``long``) or to a constant expression of type ``string``; or
-  Implicitly by omitting the constant expression.


If constant expression is omitted, then the value of the enum constant is set
implicitly to a numeric value (see :ref:`Enumeration Integer Values`).

A :index:`compile-time error` occurs if ``integer`` and ``string`` type
enumeration constants are combined in one enumeration.

Any enumeration constant is of type ``enumeration``. Implicit conversion (see
:ref:`Enumeration to Constants Type Conversions`) of an enumeration constant
to integer types or type ``string`` depends on the type of constants.

In addition, all enumeration constant names must be unique. Otherwise,
a :index:`compile-time error` occurs.

.. code-block:: typescript
   :linenos:

    enum E1 { A, B = "hello" } // compile-time error
    enum E2 { A = 5, B = "hello" } // compile-time error
    enum E3 { A = 5, A = 77 } // compile-time error
    enum E4 { A = 5, B = 5 } // OK! values can be the same

The corner case - empty enum is suppported for the compatibilty with |TS|. 

.. code-block:: typescript
   :linenos:

    enum Empty {} // OK


.. index::
   enum constant
   string
   constant
   expression
   numeric value
   integer
   numeric constant expression
   constant expression
   enumeration constant

|

.. _Enumeration Integer Values:

Enumeration Integer Values
**************************

.. meta:
    frontend_status: Done

The integer value of an ``enum`` constant is set implicitly if an enumeration
constant specifies no value.

A constant expression of type ``int`` or ``long`` can be used to set the value
explicitly:

.. index::
   enumeration integer value
   integer value
   enum constant
   enumeration constant
   integer type
   expression

.. code-block:: typescript
   :linenos:

    enum Background { White = 0xFF, Grey = 0x7F, Black = 0x00 }
    enum LongEnum { A = 0x7FFF_FFFF_1, B, C }

The choice which type ``int`` or ``long`` to be used is performed on the same 
prinicple as for integer literals (see :ref:`Integer Literals`).

If all constants have no value, then the first constant is assigned
the value zero. The other constant is assigned the value of the
immediately preceding constant plus one.

If some but not all constants have their values set explicitly, then
the values of the constants are set by the following rules:

-  The value of the first constant without an explicit value is assigned to
   zero.
-  A constant with an explicit value has that explicit value.
-  A constant that is not the first and has no explicit value takes the value
   of the immediately preceding constant plus one.

In the example below, the value of ``Red`` is 0, of ``Blue``, 5, and of
``Green``, 6:

.. code-block:: typescript
   :linenos:

    enum Color { Red, Blue = 5, Green }

.. index::
   constant
   value
   assignment

|

.. _Enumeration String Values:

Enumeration String Values
*************************

.. meta:
    frontend_status: Done

A string value for enumeration constants must be set explicitly:

.. code-block:: typescript
   :linenos:

    enum Commands { Open = "fopen", Close = "fclose" }

.. index::
   string
   value
   enumeration
   enumeration constant

|

.. _Enumeration Operations:

Enumeration Operations
**********************

.. meta:
    frontend_status: Done

The value of an enumeration constant can be converted to type ``string`` by
using the method ``toString``:

.. index::
   enumeration constant
   method
   conversion
   string

.. code-block:: typescript
   :linenos:

    enum Color { Red, Green = 10, Blue }
    let c: Color = Color.Green
    console.log(c.toString()) // prints: 10

The name of enumeration type can be indexed by the value of this enumeration
type to get the name of the constant:

.. code-block:: typescript
   :linenos:

    enum Color { Red, Green = 10, Blue }
    let c: Color = Color.Green
    console.log(Color[c]) // prints: Green

Additional methods available for enumeration types and constants are discussed
in :ref:`Enumeration Methods` in the chapter Experimental Features.

.. index::
   enumeration constant
   method
   enumeration type
   constant

.. raw:: pdf

   PageBreak
