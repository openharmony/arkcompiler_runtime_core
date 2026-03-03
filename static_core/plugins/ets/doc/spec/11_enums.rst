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
associated set of named members that define its possible enumeration values.

The syntax of *enumeration declaration* is presented below:

.. code-block:: abnf

    enumDeclaration:
        'const'? 'enum' identifier enumBaseType? '{' enumMemberList? '}'
        ;

    enumBaseType:
        ':' type
        ;

    enumMemberList:
        enumMember (',' enumMember)* ','?
        ;

    enumMember:
        identifier initializer?
        ;

    initializer:
        '=' expression
        ;

.. index::
   enumeration
   type enum
   user-defined type
   named member
   value
   enumeration declaration
   syntax

All member names in an enumeration must be unique. Otherwise,
a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    enum E3 { A = 5, A = 77 }      // compile-time error
    enum E4 { A = 5, B = 5 }       // OK! values can be the same

Empty ``enum`` is supported as a corner case for compatibility with |TS|.

.. code-block:: typescript
   :linenos:

    enum Empty {} // OK

An enumeration member has a value that can be set
explicitly by an initializer expression or implicitly (see
:ref:`Initialization of Enumeration Members`). 

.. index::
   conversion
   enumeration type
   numeric type
   string type
   type enumeration
   conversion
   type string
   expression
   enum
   compatibility

An *enumeration declaration* prefixed with the keyword ``const`` defines a special form
of enumeration, that differs from regular enumeration as follows:

- A regular enumeration has associated class type, its members
  are of type in which it is defined;

- Constant enumerations have no associated class type, its members
  are of :ref:`Enumeration Base Type`.

- Only :ref:`Constant Expressions` can be used to initialize members of
  constant enumerations (see :ref:`Constant Enumeration Members Initialization`);

- A constant enumeration type is erased to *enumeration base type*
  (see :ref:`Type Erasure`);

- A set of methods are defined for regular enumerations
  (see :ref:`Regular Enumeration Methods`), but not for constant enumerations.

See :ref:`Constant Enumerations` for details.

Qualification by type name is mandatory to access the enumeration member,
except initializer expression:

.. code-block:: typescript
   :linenos:

    enum Color { Red, Green, Blue }
    let c: Color = Color.Red // qualification is mandatory

    const enum Flags {
      Read,
      Write,
      ReadWrite = Read | Write // qualification can be omitted
    }

.. index::
   qualification
   access

The following operators can be applied to operands of enumeration types:

- Relational operators, see :ref:`Enumeration Relational Operators`;
- Equality operators, see :ref:`Equality Expressions`.

Specificity of executing operators for constant enumerations are discusses in
:ref:`Constant Enumeration Operations`.

Implicit conversions (see :ref:`Enumeration to Numeric Type Conversion`,
:ref:`Enumeration to string Type Conversion`) are used to convert
a value of enumeration type to numeric types or type ``string`` depends on the
enumeration base type:

.. code-block:: typescript
   :linenos:

    enum Color { Red, Green, Blue }
    let x: number = Color.Red + 1 // OK, implicit conversion to 'int'

    enum T { A = "hello", B = "Bye" }

    let s: string = T.A // OK, implicit conversion to 'string'

If enumeration type is exported, then all enumeration members are
exported along with the mandatory qualification.

For example, if *Color* is exported, then all members like ``Color.Red``
are exported along with the mandatory qualification ``Color``.

.. _Enumeration Base Type:

Enumeration Base Type
*********************

.. meta:
    frontend_status: Done

Each enumeration has a *enumeration base type*. A base type can be set explicitly
(see :ref:`Enumeration with Explicit Base Type`) or inferred from
initializers as follows:

- If at least one member does not have an initializer, the inferred type is ``int``.
  Types of all initializers must be are assignable to type ``int``
  (see :ref:`Assignability`). Otherwise, a :index:`compile-time error` occurs.

- If types of  all initializer expressions are assignable to type ``int``
  then the inferred type is ``int``;

- If types of  all initializer expressions are assignable to type ``string``
  then the inferred type is ``string``;

- Otherwise, a :index:`compile-time error` occurs.

See :ref:`Initialization of Enumeration Members` for detail.

Cases with base type inference are represented in the example below:

.. code-block:: typescript
   :linenos:

    enum E1 { A, B }     // OK, base type is 'int'
    enum E2 { A = 5, B } // OK, base type is 'int'

    enum E3 { A, B = "hello"}     // compile-time error, base type cannot be inferred
    enum E4 { A = 5, B = "hello"} // compile-time error, base type cannot be inferred

|

.. _Enumeration with Explicit Base Type:

Enumeration with Explicit Base Type
***********************************

.. meta:
    frontend_status: Done

*Enumeration base type* can be set explicitly as represented in the example
below:

.. code-block:: typescript
   :linenos:

    enum DoubleEnum: double { A = 0.0, B = 1, C = 3.14159 }
    enum ByteEnum: byte { A = 0, B = 1, C = 3 }

A :index:`compile-time error` occurs in the following situations:

- *Explicit type* is different from any numeric or string type.

- Type of explicit member initializer is not assignable
  (see :ref:`Assignability`) to the base type;

- Enumeration member has no explicit initializer and the base type is not
  an integer type (see :ref:`Initialization of Enumeration Members`).

.. index::
   explicit type
   enum member
   integer type
   non-explicit type
   integer value
   assignability
   numeric type
   string type

.. code-block:: typescript
   :linenos:

    enum DoubleEnum: double { A = 0.0, B = 1, C = 3.14159 } // OK
    enum LongEnum: long { A = 0, B = 1, C = 3 } // OK

    enum Wrong1: double { A, B, C } // compile-time error, must be explicitly initialized
    enum Wrong2: long { A = 1, B = "abc" } // compile-time error, not assignable to 'long'

|

.. _Initialization of Enumeration Members:

Initialization of Enumeration Members
*************************************

.. meta:
    frontend_status: Done

An enumeration member value can be set explicitly by initializer
expression. An initializer can be omitted if an *enumeration base type*
is an integer type. Otherwise, a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    enum E1 { A, B, C }    // OK, base type is 'int'
    enum E2: long { K, L } // OK, base type is 'long'

    enum E3 { A = "a", B }   // compile-time error, wrong base type
    enum E4: string { A, B } // compile-time error, base type is not an integer type
    enum E5: double { C, D } // compile-time error, base type is not an integer type

If initializers for all members are omitted, then the first member is assigned
the value zero. The other member is assigned the value of the
immediately preceding member plus one.

If some but not all members have their values set explicitly, then
the values of the members are set by the following rules:

-  If an initializer is not a *constant expression*
   (see :ref:`Constant Expressions`), all subsequent members must be
   explicitly initialized. Otherwise, a :index:`compile-time error` occurs.

-  The member which is the first and has no explicit initializer
   gets zero value.
-  Member with an explicit initializer has that explicit value.
-  Member that is not the first and has no explicit initializer takes the value
   of the immediately preceding constant plus one.

In the example below, the value of ``Red`` is 0, of ``Blue``, 5, and of
``Green``, 6:

.. code-block:: typescript
   :linenos:

    enum Color { Red, Blue = 5, Green }

.. index::
   int type
   constant
   value

The example below illustrates the requirement to have explicit initializers
after non-constant initializer:

.. code-block:: typescript
   :linenos:

    function foo() { return 1 }

    enum Wrong {
        A,
        B = foo(),
        C // compile-time error, must have explicit initializer
    }

|

.. _Regular Enumeration Methods:

Regular Enumeration Methods
***************************

.. meta:
    frontend_status: Done

This section describes *regular enumerations* only, *constant enumerations* do not have methods.

The name of enumeration type can be indexed by the value of this enumeration
type to get the name of the member:

.. code-block:: typescript
   :linenos:

    enum Color { Red, Green = 10, Blue }
    let c: Color = Color.Green
    console.log(Color[c]) // prints: Green

If several enumeration members have the same value, then the textually last
member has the priority:

.. code-block:: typescript
   :linenos:

   enum E { One = 1, one = 1, oNe = 1 }
   console.log(E[E.fromValue(1)]) // Prints: oNe
   console.log(E.fromValue(1).getName()) // Prints: oNe

Several static methods are available for each enumeration class type as
follows:

-  Method ``static values()`` returns an array of enumeration members
   in the order of declaration.
-  Method ``static getValueOf(name: string)`` returns an enumeration member
   with the given name, or throws an error if no member with such name
   exists.
-  Method ``static fromValue(value: T)``, where ``T`` is the base type
   of the enumeration, returns an enumeration member with a given value, or
   throws an error if no member has such a value.

.. index::
   enumeration method
   static method
   enumeration type
   enumeration member
   value

.. code-block:: typescript
   :linenos:

      enum Color { Red, Green, Blue = 5 }
      let colors = Color.values()
      //colors[0] is the same as Color.Red

      let red = Color.getValueOf("Red")

      Color.fromValue(5) // OK, returns Color.Blue
      Color.fromValue(6) // throws runtime error

Methods for instances of an enumeration type are as follows:

-  Method ``toString()`` converts a value of enumeration member to
   type ``string``:

.. index::
   value
   conversion
   type
   string type
   method

.. code-block:: typescript
   :linenos:

    enum Color { Red, Green = 10, Blue }
    let c: Color = Color.Green
    console.log(c.toString()) // prints: 10

-  Method ``valueOf()`` returns a numeric or ``string`` value of an enumeration
   member depending on the enumeration base type.

-  Method ``getName()`` returns the name of an enumeration member.

.. code-block-meta:

.. code-block:: typescript
   :linenos:

      enum Color { Red, Green = 10, Blue }
      let c: Color = Color.Green
      console.log(c.valueOf()) // prints 10
      console.log(c.getName()) // prints Green

.. note::
   Methods ``c.toString()`` and ``c.valueOf().toString()`` return the same
   value.

.. index::
   instance
   method
   enumeration type
   value
   name
   enumeration member

|

.. _Constant Enumerations:

Constant Enumerations
*********************

.. meta:
    frontend_status: None

*Constant enumeration* (``const enum``) is a special form of enumeration that
provides compile-time inlining of enumeration values. Unlike regular
enumerations, constant enumerations have no associated class type.

A constant enumeration is a distinct user-defined type at the source code level.
All references to constant enumeration members are replaced for their
corresponding literal values during compilation.

The *enumeration base type* of a constant enumeration is described in
:ref:`Enumeration with Explicit Base Type`.

.. code-block:: typescript
   :linenos:

    const enum Status { // base type: int
        Pending = 0,
        Active = 1,
        Completed = 2
    }

    const enum Commands { // Base type: string
        Open = "open",
        Close = "close"
    }

    const enum ByteFlags: byte {  // Explicit base type
        None = 0,
        Read = 1,
        Write = 2
    }

If a const enumeration mixes integer and string constant values, then
a :index:`compile-time error` occurs.

.. index::
   constant enumeration type
   const modifier
   enumeration base type
   integer type
   string type
   compile-time error

|

.. _Constant Enumeration Members Initialization:

Constant Enumeration Members Initialization
===========================================

.. meta:
    frontend_status: None

All constant enumeration members must be initialized with *constant expressions*
(see :ref:`Constant Expressions`) of respective enumeration base type.
The initialization expressions are evaluated at compile time.
Implicit values for integer-based constant enumerations follow the same rules
as for regular enumerations, see :ref:`Initialization of Enumeration Members`.

.. index::
   constant expression
   initialization
   compile time

.. code-block:: typescript
   :linenos:

    const enum Flags {
        None = 0,
        Read = 1 << 0,              // Bitwise shift
        Write = 1 << 1,             // Bitwise shift
        Execute = 1 << 2,           // Bitwise shift
        ReadWrite = Read | Write,   // Bitwise OR of const enum values
        All = Read | Write | Execute
    }

    const enum Computed {
        Base = 10,
        Double = Base * 2,          // Arithmetic with const enum value
        Triple = Base * 3,
    }

    const enum Size {
        Small = 10,
        Medium,                     // Implicitly 11
        Large                       // Implicitly 12
    }

    const enum FileOps {
        Open = "open",
        Close = "close",
        Read = "read"
    }

A :index:`compile-time error` occurs if:

- Initializer of a constant enumeration member
  is not a constant expression;
- Evaluation of an initializer expression completes abruptly.

.. index::
   initialization expression
   constant expression
   compile-time error
   evaluation

.. code-block:: typescript
   :linenos:

    const enum Invalid {
        A = Math.sqrt(4),  // compile-time error, non-constant expression
        B = someVariable,  // compile-time error, non-constant expression
    }

    enum RegularEnum { X = 5 }
    const enum Invalid2 {
        C = RegularEnum.X  // compile-time error, non-constant expression
    }

|

.. _Constant Enumeration Operations:

Constant Enumeration Operations
===============================

.. meta:
    frontend_status: None

The specifics of operations on constant enumerations
are represented below.

Constant enumeration members are inlined to literal values when accessed:

.. code-block:: typescript
   :linenos:

    const enum Status { Pending = 0, Active = 1 }
    let s: Status = Status.Active;  // Inlined to: let s: number = 1;

Moreover, inlining also has place for all operators, namely
equality operators and relational operators:

.. code-block:: typescript
   :linenos:

    const enum Priority { Low = 1, Medium = 2, High = 3 }
    let p: Priority = Priority.Medium;

    if (p === Priority.High) { }       // Inlined to: if (2 === 3)
    if (p < Priority.High) { }         // Inlined to: if (2 < 3)


.. _Values of Constant Enumeration Outside Range:

Values of Constant Enumeration Outside Range
============================================

.. meta:
    frontend_status: None

A variable of constant enumeration type can
have a value that corresponds to no defined
enumeration member, because constant
enumerations are inlined to their base types.
A variable of constant enumeration type can
have such a value through an explicit cast as
follows:

.. code-block:: typescript
   :linenos:

    const enum Status { Pending = 0, Active = 1, Completed = 2 }
    let s: Status = 99 as Status;  // OK: type inference in cast expression

    switch (s) {
        case Status.Pending:
            console.log("pending");
        case Status.Active:
            console.log("active");
        case Status.Completed:
            console.log("completed");
        default:
            console.log("unknown ", s);  // "unknown 99" will be printed
    }

No validation occurs at compile time nor runtime to check that a value of a const
enumeration type corresponds to a defined constant.

.. index::
   explicit cast


.. raw:: pdf

   PageBreak
