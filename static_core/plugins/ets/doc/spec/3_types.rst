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

.. _Types:

Types
#####

.. meta:
    frontend_status: Partly

This chapter introduces the notion of type that is one of the fundamental
concepts of |LANG| and other programming languages.
Type classification as accepted in |LANG| is discussed below---along
with all aspects of using types in programs written in the language.

Conventionally, the type of an entity is defined as the set of *values* the
entity (variable) can take, and the set of *operators* applicable to the entity
of a given type.

|LANG| is a statically typed language. It means that the type of every
declared entity and every expression is known at compile time. The type of
an entity is either set explicitly by a developer, or inferred implicitly
(see :ref:`Type Inference`) by the compiler.

There are two categories of types:

#. :ref:`Value Types`, and

#. :ref:`Reference Types`.

The types integral to |LANG| are called *predefined types* (see
:ref:`Predefined Types`).

The types introduced, declared, and defined by a developer are called
*user-defined types*.
All *user-defined types* must always have complete type definitions
presented as source code in |LANG|.

.. index::
   statically typed language
   expression
   compile time
   value type
   reference type
   type inference
   compiler
   predefined type
   user-defined type
   type definition
   source code

|

.. _Predefined Types:

Predefined Types
****************

.. meta:
    frontend_status: Done

Predefined types include the following:

-  Basic numeric value type: ``number``

-  High-performance value types:

     - Numeric types: ``byte``, ``short``, ``int``, ``long``, ``float``, and
       ``double``;

     - Character type: ``char``;

     - Boolean type: ``boolean``;


-  Reference types: ``object``, ``string``, ``[]`` (``array``), ``bigint``,
   ``void``, ``never``, and ``undefined``;

-  Class types: ``Object``, ``String``, ``Array<T>``, and ``BigInt``.


|

.. _Primitive Types:

Primitive Types
===============

.. meta:
    frontend_status: Done

The predefined value types are called *primitive types*. Primitive types have
no methods, and have the operations only as referred herein (see
:ref:`Value Types`). Primitive type names are reserved, i.e., they cannot be
used for user-defined type names.

Type ``double`` is an alias to ``number``. Type ``Double`` is an alias to
``Number``.

|

.. _Boxed Types:

Boxed Types
===========

.. meta:
    frontend_status: Done

Each predefined value type has a corresponding predefined class type (called
*boxed type*) that wraps the value of the predefined value type:
``Number``, ``Byte``, ``Short``, ``Int``, ``Long``, ``Float``, ``Double``,
``Char``, and ``Boolean``.


.. index::
   predefined type
   basic numeric type
   numeric type
   user-defined type
   type definition
   alias
   value type
   class type
   primitive type
   type alias
   wrapping

|

.. _User-Defined Types:

User-Defined Types
******************

.. meta:
    frontend_status: Done

*User-defined* types include the following:

-  Class types (see :ref:`Classes`);
-  Interface types (see :ref:`Interfaces`);
-  Enumeration types (see :ref:`Enumerations`);
-  Array types (see :ref:`Array Types`);
-  Function types (see :ref:`Function Types`);
-  Tuple types (see :ref:`Tuple Types`);
-  Union types (see :ref:`Union Types`);
-  Type parameters (see :ref:`Type Parameters`); and
-  Literal types (see :ref:`Literal Types`).

.. index::
   user-defined type
   class type
   interface type
   enumeration type
   array type
   function type
   union type
   type parameter
   literal type

|

.. _Types by Category:

Types by Category
*****************

.. meta:
    frontend_status: Done

All |LANG| types are summarized in the following table:

.. table::
   :widths: 25, 25, 25, 25

   ========================= ========================= ========================= =========================
   **Predefined Types**                                **User-Defined Types**
   ------------------------- ------------------------- ------------------------- -------------------------
   *Value Types*             *Reference Types*         *Value Types*             *Reference Types*
   (Primitive Types)
   ========================= ========================= ========================= =========================
   ``number``, ``byte``,     ``Number``, ``Byte``,     enumeration types,        class types,            
   ``short``, ``int``,       ``Short``, ``Int``,       literal types             interface types,
   ``long``, ``float``,      ``Long``, ``Float``,                                array types,   
   ``double``, ``char``,     ``Double``, ``Char``,                               function types,          
   ``boolean``, ``string``,  ``Boolean``,                                        tuple types,             
   ``bigint``                ``String``, ``string``,                             union types,             
                                                                                 literal types,           
                             ``BigInt``, ``bigint``,                             type parameters          
                                                                                                          
                             ``Object``, ``object``,                                                      
                                                                                                          
                             ``void``, ``null``,                                                          
                             ``never``                                                                    
   ========================= ========================= ========================= =========================


**Note**: Type ``string`` (see :ref:`Type string`) and type ``bigint`` (see
:ref:`BigInt Type`) have dual semantics. It affects some types in several
categories in the table above.


.. index::
   class type
   primitive type
   reference type
   value type
   interface type
   array type
   union type
   tuple type
   type parameter
   literal type

|

.. _Using Types:

Using Types
***********

.. meta:
    frontend_status: Done

A type can be referred to in source code by the following:

-  Reserved name for a primitive type;
-  Type reference for a named type (see :ref:`Named Types`), or type alias
   (see :ref:`Type Alias Declaration`);
-  In-place type definition for an array type (see :ref:`Array Types`),
   function type (see :ref:`Function Types`), tuple type (see :ref:`Tuple Types`),
   or union type (see :ref:`Union Types`).

.. index::
   reserved name
   primitive type
   type alias
   type reference
   array type
   function type
   union type

.. code-block:: abnf

    type:
        predefinedType
        | typeReference
        | arrayType
        | tupleType
        | functionType
        | unionType
        | literalType
        | keyofType
        | '(' type ')'
        ;

    predefinedType:
        'number' | 'byte' | 'short' | 'int' | 'long' | 'float' | 'double'
        | 'bigint'
        | 'char' | 'boolean'
        | 'object' | 'string' | 'void' | 'never' |'undefined'
        ;

    literalType:
        Literal
        ;


The use of types is presented by the example below:

.. code-block:: typescript
   :linenos:

    let b: boolean  // using primitive value type name
    let n: number   // using primitive value type name
    let o: Object   // using predefined class type name
    let a: number[] // using array type
    let l: 1        // using literal type

Parentheses in types (where a type is a combination of array, function, or
union types) are used to specify the required type structure.
Without parentheses, the symbol '``|``' that constructs a union type
has the lowest precedence as presented in the following example:

.. index::
   array type
   function type
   union type
   type structure
   symbol
   construct
   precedence

.. code-block:: typescript
   :linenos:

    // a nullable array with elements of type string:
    let a: string[] | null
    let s: string[] = []
    a = s    // ok
    a = null // ok, a is nullable

    // an array with elements whose types are string or null:
    let b: (string | null)[]
    b = null // error, b is an array and is not nullable
    b = ["aa", null] // ok

    // a function type that returns string or null
    let c: () => string | null
    c = null // error, c is not nullable
    c = (): string | null => { return null } // ok

    // (a function type that returns string) or null
    let d: (() => string) | null
    d = null // ok, d is nullable
    d = (): string => { return "hi" } // ok

|

.. _Named Types:

Named Types
***********

.. meta:
    frontend_status: Done

Classes, interfaces, enumerations, aliases, and type parameters are named types.
Other types (i.e., array, function, and union types) are anonymous. Respective
named types are introduced by the following:

-  Class declarations (see :ref:`Classes`),
-  Interface declarations (see :ref:`Interfaces`),
-  Enumeration declarations (see :ref:`Enumerations`),
-  Type alias declarations (see :ref:`Type Alias Declaration`), and
-  Type parameter declarations (see :ref:`Type Parameters`).

Classes, interfaces, and type aliases with type parameters are *generic types*
(see :ref:`Generics`). Named types without type parameters are
*non-generic types*.

*Type references* (see :ref:`Type References`) refer to named types by
specifying their type names, and (where applicable) by type arguments to be
substituted for the type parameters of the named type.

.. index::
   named type
   class
   interface
   enumeration
   union
   class declaration
   interface declaration
   enumeration declaration
   union declaration
   generic type
   generics
   type argument
   type parameter

|

.. _Type References:

Type References
***************

.. meta:
    frontend_status: Done

A type reference refers to a type by one of the following:

-  *Simple* or *qualified* type name (see :ref:`Names`),
-  Type alias (see :ref:`Type Alias Declaration`), or
-  Type parameter (see :ref:`Type Parameters`) name with the '``!``' sign
   (see :ref:`NonNullish Type Parameter`).

If the type name denoted by ``identifier`` refers to a generic class or an
interface type, then it must be a valid instantiation of the generic to be
a valid type reference. A type reference is valid if its type arguments (see
:ref:`Type Arguments`) are provided explicitly or implicitly based on defaults.

.. index::
   type reference
   type name
   simple type name
   qualified type name
   identifier
   type alias
   type argument
   interface type

.. code-block:: abnf

    typeReference:
        typeReferencePart ('.' typeReferencePart)*
        |  identifier '!'
        ;

    typeReferencePart:
        identifier typeArguments?
        ;

.. code-block:: typescript
   :linenos:

    let map: Map<string, number> // Map<string, number> is the type reference

    class A<T> {
       field1: A<T>  // A<T> is a type reference - class type reference
       field2: A<number> // A<number> is a type reference - class type reference
       foo (p: T) {} // T is a type reference - type parameter
       constructor () { /* some body to init fields */ }
    }

    type MyType<T> = []A<T>
    let x: MyType<number> = [new A<number>, new A<number>]
      // MyType<number> is a type reference  - alias reference
      // A<number> is a type reference - class type reference


If a type reference refers to the type by a type alias (see
:ref:`Type Alias Declaration`), then the type alias is replaced (potentially
recursively) for the non-aliased type in all cases when dealing with types
in this document.

.. code-block:: typescript
   :linenos:

   type T1 = Object
   type T2 = number
   function foo(t1: T1, t2: T2)  {
       t1 = t2      // Type compatibilty test will use Object and number
       t2 = t2 + t2 // Operator validity test will use type number not T2
   } 
 

|

.. _Value Types:

Value Types
***********

.. meta:
    frontend_status: Done

Predefined integer types (see :ref:`Integer Types and Operations`),
floating-point types (see :ref:`Floating-Point Types and Operations`), the
boolean type (see :ref:`Boolean Types and Operations`), character types
(see :ref:`Character Type and Operations`), user-defined enumeration
types (see :ref:`Enumerations`), and literal types (see :ref:`Literal Types`)
are *value types*. The values of such types do *not* share state with other
values.

.. index::
   value type
   predefined type
   integer type
   floating-point type
   boolean type
   character type
   enumeration

|

.. _Integer Types and Operations:

Integer Types and Operations
============================

.. meta:
    frontend_status: Done

+------------+--------------------------------------------------------------------+--------------------------+
| Type       | Type's Set of Values                                               | Corresponding Class Type |
+============+====================================================================+==========================+
| ``byte``   | All signed 8-bit integers (:math:`-2^7` to :math:`2^7-1`)          |   ``Byte``               |
+------------+--------------------------------------------------------------------+--------------------------+
| ``short``  | All signed 16-bit integers (:math:`-2^{15}` to :math:`2^{15}-1`)   |   ``Short``              |
+------------+--------------------------------------------------------------------+--------------------------+
| ``int``    | All signed 32-bit integers (:math:`-2^{31}` to :math:`2^{31} - 1`) |   ``Int``                |
+------------+--------------------------------------------------------------------+--------------------------+
| ``long``   | All signed 64-bit integers (:math:`-2^{63}` to :math:`2^{63} - 1`) |   ``Long``               |
+------------+--------------------------------------------------------------------+--------------------------+
| ``bigint`` | All integers with no limits                                        |   ``BigInt``             |
+------------+--------------------------------------------------------------------+--------------------------+

|LANG| provides a number of operators to act on integer values as discussed
below.

-  Comparison operators that produce a value of type ``boolean``:

   +  Numerical relational operators '``<``', '``<=``', '``>``', and '``>=``'
      (see :ref:`Numerical Relational Operators`);
   +  Numerical equality operators '``==``' and '``!=``' (see
      :ref:`Numerical Equality Operators`);

-  Numerical operators that produce values of types ``int``, ``long``, or
   ``bigint``:

   + Unary plus '``+``' and minus '``-``' operators (see :ref:`Unary Plus` and
     :ref:`Unary Minus`);
   + Multiplicative operators '``*``', '``/``', and '``%``' (see
     :ref:`Multiplicative Expressions`);
   + Additive operators '``+``' and '``-``' (see :ref:`Additive Expressions`);
   + Increment operator '``++``' used as prefix (see :ref:`Prefix Increment`)
     or postfix (see :ref:`Postfix Increment`);
   + Decrement operator '``--``' used as prefix (see :ref:`Prefix Decrement`)
     or postfix (see :ref:`Postfix Decrement`);
   + Signed and unsigned shift operators '``<<``', '``>>``', and '``>>>``' (see
     :ref:`Shift Expressions`);
   + Bitwise complement operator '``~``' (see :ref:`Bitwise Complement`);
   + Integer bitwise operators '``&``', '``^``', and '``|``' (see
     :ref:`Integer Bitwise Operators`);

-  Conditional operator '``?:``' (see :ref:`Conditional Expressions`);
-  Cast operator (see :ref:`Cast Expressions`) that converts an integer value
   to a value of any specified numeric type;
-  String concatenation operator '``+``' (see :ref:`String Concatenation`) that,
   if one operand is ``string`` and the other is of an integer type, converts an
   integer operand to ``string`` with the decimal form, and then creates a
   concatenation of the two strings as a new ``string``.

.. index::
   byte
   short
   int
   long
   bigint
   Byte
   Short
   Int
   Long
   BigInt
   integer value
   comparison operator
   numerical relational operator
   numerical equality operator
   equality operator
   numerical operator
   type reference
   type name
   simple type name
   qualified type name
   type alias
   type argument
   interface type
   postfix
   prefix
   unary operator
   unary operator
   additive operator
   multiplicative operator
   increment operator
   numerical relational operator
   numerical equality operator
   decrement operator
   signed shift operator
   unsigned shift operator
   bitwise complement operator
   integer bitwise operator
   conditional operator
   cast operator
   integer value
   specific numeric type
   string concatenation operator
   operand

The classes ``Byte``, ``Short``, ``Int``, and ``Long`` predefine constructors,
methods, and constants that are parts of the |LANG| standard library (see
:ref:`Standard Library`).

If one operand is not of type ``long``, then the numeric promotion (see
:ref:`Primitive Types Conversions`) must be used first to widen it to type
``long``.

If neither operand is of type ``long``, then:

-  The operation implementation uses 32-bit precision.
-  The result of the numerical operator is of type ``int``.


If one operand (or neither operand) is of type ``int``, then the numeric
promotion must be used first to widen it to type ``int``.

Any integer type value can be cast to or from any numeric type.

Casts between integer types and type ``boolean`` are not allowed.

The integer operators cannot indicate an overflow or an underflow.

An integer operator can throw errors (see :ref:`Error Handling`) as follows:

-  An integer division operator '``/``' (see :ref:`Division`), and an
   integer remainder operator '``%``' (see :ref:`Remainder`) throw
   ``ArithmeticError`` if their right-hand operand is zero.
-  An increment operator '``++``' and a decrement operator '``--`' (see
   :ref:`Additive Expressions`) throw ``OutOfMemoryError`` if boxing
   conversion (see :ref:`Boxing Conversions`) is required
   but the available memory is not sufficient to perform it.

.. index::
   Byte
   Short
   Int
   Long
   constructor
   method
   constant
   operand
   numeric promotion
   predefined numeric types conversion
   numeric type
   widening
   long
   int
   boolean
   integer type
   numeric type
   cast
   operator
   overflow
   underflow
   division operator
   remainder operator
   error
   increment operator
   decrement operator
   additive expression
   boxing conversion

|

.. _Floating-Point Types and Operations:

Floating-Point Types and Operations
===================================

.. meta:
    frontend_status: Done

+-------------+-------------------------------------+--------------------------+
| Type        | Type's Set of Values                | Corresponding Class Type |
+=============+=====================================+==========================+
| ``float``   | The set of all IEEE 754 [3]_ 32-bit | ``Float``                |
|             | floating-point numbers              |                          |
|             | floating-point numbers              |                          |
+-------------+-------------------------------------+--------------------------+
| ``number``, | The set of all IEEE 754 64-bit      | ``Number``               |
| ``double``  | floating-point numbers              | ``Double``               |
+-------------+-------------------------------------+--------------------------+

.. index::
   IEEE 754
   floating-point number

|LANG| provides a number of operators to act on floating-point type values as
discussed below.

-  Comparison operators that produce a value of type *boolean*:

   - Numerical relational operators '``<``', '``<=``', '``>``', and '``>=``'
     (see :ref:`Numerical Relational Operators`);
   - Numerical equality operators '``==``' and '``!=``' (see
     :ref:`Numerical Equality Operators`);

-  Numerical operators that produce values of type ``float`` or ``double``:

   + Unary plus '``+``' and minus '``-``' operators (see :ref:`Unary Plus` and
     :ref:`Unary Minus`);
   + Multiplicative operators '``*``', '``/``', and '``%``' (see
     :ref:`Multiplicative Expressions`);
   + Additive operators '``+``' and '``-``' (see :ref:`Additive Expressions`);
   + Increment operator '``++``' used as prefix (see :ref:`Prefix Increment`)
     or postfix (see :ref:`Postfix Increment`);
   + Decrement operator '``--``' used as prefix (see :ref:`Prefix Decrement`)
     or postfix (see :ref:`Postfix Decrement`);

-  Numerical operators that produce values of type ``int`` or ``long``:

   + Signed and unsigned shift operators '``<<``', '``>>``', and '``>>>``' (see
     :ref:`Shift Expressions`);
   + Bitwise complement operator '``~``' (see :ref:`Bitwise Complement`);
   + Integer bitwise operators '``&``', '``^``', and '``|``' (see
     :ref:`Integer Bitwise Operators`);
   
- Conditional operator '``?:``' (see :ref:`Conditional Expressions`);

-  Cast operator (see :ref:`Cast Expressions`) that converts a floating-point
   value to a value of any specified numeric type;
-  The string concatenation operator '``+``' (see :ref:`String Concatenation`)
   that, if one operand is of type ``string`` and the other is of a
   floating-point type, converts the floating-point type operand to type
   ``string`` with a value represented in the decimal form (without the loss
   of information), and then creates a concatenation of the two strings as a
   new ``string``.


.. index::
   floating-point type
   floating-point number
   operator
   numerical relational operator
   numerical equality operator
   comparison operator
   boolean type
   numerical operator
   float
   double
   unary operator
   unary plus operator
   unary minus operator
   multiplicative operator
   additive operator
   prefix
   postfix
   increment operator
   decrement operator
   signed shift operator
   unsigned shift operator
   cast operator
   bitwise complement operator
   integer bitwise operator
   conditional operator
   string concatenation operator
   operand
   numeric type
   string

The classes ``Float`` and ``Double`` predefine constructors, methods, and
constants that are parts of the |LANG| standard library (see
:ref:`Standard Library`).

An operation is called a floating-point operation if at least one of the
operands in a binary operator is of a floating-point type (even if the
other operand is integer).

If at least one operand of the numerical operator is of type ``double``,
then the operation implementation uses 64-bit floating-point arithmetic. The
result of the numerical operator is a value of type ``double``.

If the other operand is not of type ``double``, then the numeric promotion (see
:ref:`Primitive Types Conversions`) must be used first to widen it to type
``double``.

If neither operand is of type ``double``, then the operation implementation
is to use 32-bit floating-point arithmetic. The result of the numerical
operator is a value of type ``float``.

If the other operand is not of type ``float``, then the numeric promotion
must be used first to widen it to type ``float``.

Any floating-point type value can be cast to or from any numeric type.

.. index::
   Float
   Double
   class
   constructor
   method
   constant
   operation
   floating-point operation
   predefined numeric types conversion
   numeric type
   operand
   implementation
   float
   double
   numeric promotion
   numerical operator
   binary operator
   floating-point type

Casts between floating-point types and type ``boolean`` are not allowed.

Operators on floating-point numbers, except the remainder operator (see
:ref:`Remainder`), behave in compliance with the IEEE 754 Standard.
For example, |LANG| requires the support of IEEE 754 *denormalized*
floating-point numbers and *gradual underflow* that make it easier to prove
the desirable properties of a particular numerical algorithm. Floating-point
operations do not *flush to zero* if the calculated result is a
denormalized number.

|LANG| requires floating-point arithmetic to behave as if the floating-point
result of every floating-point operator is rounded to the result precision. An
*inexact* result is rounded to the representable value nearest to the infinitely
precise result. |LANG| uses the *round to nearest* principle (the default
rounding mode in IEEE 754), and prefers the representable value with the least
significant bit zero out of any two equally near representable values.

.. index::
   cast
   floating-point type
   floating-point number
   numeric type
   numeric types conversion
   widening
   operand
   implementation
   numeric promotion
   remainder operator
   gradual underflow
   flush to zero
   round to nearest
   rounding mode
   denormalized number
   IEEE 754

|LANG| uses *round toward zero* to convert a floating-point value to an
integer value (see :ref:`Primitive Types Conversions`). In this case it acts as
if the number is truncated, and the mantissa bits are discarded.
The result of *rounding toward zero* is the value of that format that is
closest to and no greater in magnitude than the infinitely precise result.

A floating-point operation with overflow produces a signed infinity.

A floating-point operation with underflow produces a denormalized value
or a signed zero.

A floating-point operation with no mathematically definite result
produces ``NaN``.

All numeric operations with a ``NaN`` operand result in ``NaN``.

A floating-point operator (the increment '``++``' operator and decrement
'``--``' operator, see :ref:`Additive Expressions`) can throw
``OutOfMemoryError`` (see :ref:`Error Handling`) if boxing conversion (see
:ref:`Boxing Conversions`) is required but the available memory is not
sufficient to perform it.

.. index::
   round toward zero
   conversion
   predefined numeric types conversion
   numeric type
   truncation
   truncated number
   rounding toward zero
   denormalized value
   NaN
   numeric operation
   increment operator
   decrement operator
   error
   boxing conversion
   overflow
   underflow
   signed zero
   signed infinity
   integer
   floating-point operation
   floating-point operator
   floating-point value
   throw
   
|

.. _Numeric Types Hierarchy:

Numeric Types Hierarchy
=======================

.. meta:
    frontend_status: Done

Integer and floating-point types are numeric types.

Larger type values include all values of smaller types:

-  ``double`` > ``float`` > ``long`` > ``int`` > ``short`` > ``byte``

Consequently, a value of a smaller type can be assigned to a variable of a
larger type.

Type ``bigint`` does not belong to this hierarchy. There is no implicit
conversion from a numeric type to ``bigint``. Standard library (see
:ref:`Standard Library`) class ``BigInt`` methods must be used to create
``bigint`` values from numeric types.

.. index::
   numeric type
   exception
   floating-point type
   assignment
   variable
   double
   float
   long
   int
   short
   byte
   bigint
   long
   int
   short
   byte
   string
   BigInt

|

.. _Boolean Types and Operations:

``Boolean`` Types and Operations
================================

.. meta:
    frontend_status: Done

Type ``boolean`` represents logical values ``true`` and ``false`` that
correspond to the class type ``Boolean``.

The boolean operators are as follows:

-  Relational operators '``==``' and '``!=``' (see :ref:`Relational Expressions`);
-  Logical complement operator '``!``' (see :ref:`Logical Complement`);
-  Logical operators '``&``', '``^``', and '``|``' (see :ref:`Integer Bitwise Operators`);
-  Conditional-and operator '``&&``' (see :ref:`Conditional-And Expression`) and
   conditional-or operator '``||``' (see :ref:`Conditional-Or Expression`);
-  Conditional operator '``?:``' (see :ref:`Conditional Expressions`);
-  String concatenation operator '``+``' (see :ref:`String Concatenation`)
   that converts an operand of type ``boolean`` to type ``string`` (``true`` or
   ``false``), and then creates a concatenation of the two strings as a new
   ``string``.


The conversion of an integer or floating-point expression *x* to a boolean
value must follow the *C* language convention---any nonzero value is converted
to ``true``, and the value of zero is converted to ``false``. In other words,
the result of expression *x*  conversion to type ``boolean`` is always the same
as the result of comparison *x != 0*.

.. index::
   boolean
   Boolean
   relational operator
   complement operator
   logical operator
   conditional-and operator
   conditional-or operator
   conditional operator
   string concatenation operator
   floating-point expression
   comparison
   conversion

|

.. _Literal Types:

Literal Types
=============

.. meta:
    frontend_status: None

Literal types are aligned with |LANG| literals (see :ref:`Literals`), and
their names are the same as the names of their values, i.e., the literals.

.. code-block:: typescript
   :linenos:

    let a: 1 = 1
    let b: true = true
    let c: 3.14 = 3.14
    let d: "string literal" = "string literal"
    let e: c'C' = c'C'
    let f: 123n = 123n
    let g: null = null
    let h: undefined = undefined

    printThem (a, b, c, d, e, f, g, h)
    function printThem (
        p1: 1, p2: true, p3: 3.14, p4: "string literal",
        p5: c"C", p6: 123n, p7: null, p8: undefined
    ) {
        console.log (p1, p2, p3, p4, p5, p6, p7, p8)
    }

In case of numeric forms of literal types, *0*'s with no meaning are truncated:

.. code-block:: typescript
   :linenos:

    // a1 and a2 have the same type 1
    let a1: 1 = 0001
    let a2: 0001 = 1

    // b1 and b2 have the same type 3.14
    let b1: 3.14 = 3.140000
    let b1: 3.1400000 = 3.140


Operations on variables of literal types are identical to the operations
of the literal type. The resulting operation type is the type specified
for the type of the literal. Typically, it is the type of the literal but
not the literal type itself:

.. code-block:: typescript
   :linenos:

    let a: 1 = 1
    let b: true = true
    let c: 3.14 = 3.14
    let d: "string literal" = "string literal"
    let e: c"C" = c"C"
    let f: 123n = 123n

    let x: int = a + a       // + for int returns int
    let y: boolean = b || b  // || for boolean returns boolean
    let z: double = c * c    // * for double returns double
    let s: string = d + d    // + for string returns string
    let t: int = e + e       // + for char returns char
    let u: bigint = f + f    // + for bigint returns bigint

Every literal type is a subtype (see :ref:`Subtyping`) of the type of its
literal:

.. code-block:: typescript
   :linenos:

    class Base {
        foo(p: 1): int { return 666 }
    }
    class Derived extends Base {
        override foo(p: int): 1 { return 1 }
    }
    // Type 1 <: int

    let base: Base = new Derived
    let result: int = base.foo(1)
    /* Argument 1 (value) is compatible to type 1 and to the type int in
       the overridden method
       Function result of type int accepts 1 (value) of literal type 1
    */

Literal types are practically used inside union types (see :ref:`Union Types`).


|

.. _Reference Types:

Reference Types
***************

.. meta:
    frontend_status: Done

*Reference types* can be of the following kinds:

-  *Class* types (see :ref:`Classes`);
-  *Interface* types (see :ref:`Interfaces`);
-  *Array* types (see :ref:`Array Types`);
-  *Function* types (see :ref:`Function Types`);
-  *Union* types (see :ref:`Union Types`);
-  ``String`` types (see :ref:`Type String`);
-  Type ``Never`` (see :ref:`Type never`), type ``null`` (see :ref:`Type null`),
   type ``undefined`` (see :ref:`Type undefined`), type ``void`` (see
   :ref:`Type void`); and
-  Type parameters (see :ref:`Type Parameters`).

.. index::
   class type
   interface type
   array type
   function type
   union type
   type string
   type never
   type null
   type undefined
   type void
   type parameter

|

.. _Objects:

Objects
=======

.. meta:
    frontend_status: Done

An ``object`` can be a *class instance*, a *function instance*, or an *array*.
The pointers to these objects are called *references* or *reference values*.

A class instance creation expression (see :ref:`New Expressions`) explicitly
creates a class instance.

Referring to a declared function by its name, qualified name, or lambda
expression (see :ref:`Lambda Expressions`) explicitly creates a function
instance.

An array creation expression explicitly creates an array (see
:ref:`Array Creation Expressions`).

A string literal initialization explicitly creates a string.

Other expressions can implicitly create a class instance (see
:ref:`New Expressions`), or an array (see :ref:`Array Creation Expressions`).

.. index::
   object
   instance
   array
   reference value
   function instance
   class instance
   reference
   lambda expression
   qualified name
   name
   declared function
   array creation
   expression
   literal
   initialization

The operations on references to objects are as follows:

-  Field access expression (see :ref:`Field Access Expression`);
-  Call expression (see :ref:`Method Call Expression` and :ref:`Function Call Expression`);
-  Cast expression (see :ref:`Cast Expressions`);
-  String concatenation operator (see :ref:`String Concatenation`) that---given
   an operand of type ``string`` and a reference---calls the ``toString``
   method of the referenced object, converts the reference to type ``string``,
   and creates a concatenation of the two strings as a new ``string``;
-  ``instanceof`` expression (see :ref:`InstanceOf Expression`);
-  ``typeof`` expression (see :ref:`TypeOf Expression`);
-  Reference equality operators '``==``' and '``!=``' (see
   :ref:`Reference Equality`);
-  Conditional expression '``?:``' (see :ref:`Conditional Expressions`).


Multiple references to an object are possible.

Most objects have state. The state is stored in the field if an object is
an instance of class, or in a variable that is an element of an array object.

If two variables contain references to the same object, and the state of that
object is modified in the reference of one variable, then the state so modified
can be seen in the reference of the other variable.

.. index::
   operator
   object
   class
   interface
   type parameter
   field access
   qualified name
   method call expression
   function call expression
   field access expression
   cast expression
   call expression
   concatenation operator
   conversion
   reference equality operator
   conditional operator
   state
   array element
   variable
   field
   instance
   reference

|

.. _Object Class Type:

``Object`` Class Type
=====================

.. meta:
    frontend_status: Done

All classes, interfaces, string types, arrays, unions, function types, and enum
types are compatible (see :ref:`Type Compatibility`) with class ``Object``, and
all inherit (see :ref:`Inheritance`) the methods of the class ``Object``. Full
description of all methods of class ``Object`` is given in the standard library
(see :ref:`Standard Library`) description.

The method ``toString`` as used in the examples in this document returns a
string representation of the object.

Using ``Object`` is recommended in all cases (although the name ``object``
refers to type ``Object``).

.. index::
   class type
   function call expression
   field access expression
   cast expression
   concatenation operator
   operand
   reference
   method
   object
   object class type
   call expression
   instanceof operator
   interface
   array
   inheritance
   hash code

|

.. _Type string:

Type ``string``
===============

.. meta:
    frontend_status: Done

Type ``string`` is a predefined type. It stores sequences of characters as
Unicode UTF-16 code units. Type ``string`` includes all string literals, e.g.,
'``abc``'.

The value of a ``string`` object cannot be changed after the object is created,
i.e., a ``string`` object is immutable. The value of a ``string`` object can be
shared.

Type ``string`` has dual semantics:

-  If a string is created, assigned, or passed as an argument, then it behaves
   like a reference type (see :ref:`Reference Types`).
-  All ``string`` operations (see :ref:`String Concatenation`,
   :ref:`String Equality Operators`, and
   :ref:`String Relational Operators`) handle strings as values (see
   :ref:`Value Types`).

If the result is not a constant expression (see :ref:`Constant Expressions`),
then the string concatenation operator '``+``' (see :ref:`String Concatenation`)
can implicitly create a new string object.

Using ``string`` is recommended in all cases (although the name ``String``
also refers to type ``string``).

.. index::
   type string
   Unicode code unit
   compiler
   predefined type
   extended semantics
   literal
   constant expression
   concatenation operator
   alias

|

.. _Type never:

Type ``never``
==============

.. meta:
    frontend_status: Done

Type ``never`` is compatible (see :ref:`Type Compatibility`) with any other type.

Type ``never`` has no instance. It is used as one of the following:

- Return type for functions or methods that never return a value, but
  throw an error or an exception when completing an operation.
- Type of variables that can never be assigned.
- Type of parameters of a function or a method to prevent the body of that
  function or method from being executed.

.. code-block:: typescript
   :linenos:

    function foo (): never {
        throw new Error("foo() never returns")
    }

    let x: never = foo() // x will never get value

    function bar (p: never) { // body of this 
       // function will never be executed
    }

    bar (foo())




.. index::
   class
   instance
   error
   exception
   function
   return type
   string literal
   string object
   constant expression
   concatenation operator
   alias
   instance
   value

|

.. _Type void:

Type ``void``
=============

.. meta:
    frontend_status: Done

Type ``void`` has no instance (no value). It is typically used as the
return type if a function or a method returns no value:

.. code-block:: typescript
   :linenos:

    function foo (): void {}
   
    class C {
        bar(): void {}
    }

    type FunctionWithNoParametersType = () => void

    let funcTypeVariable: FunctionWithNoParametersType = (): void => {}

A :index:`compile-time error` occurs if:

-  Type ``void`` is used as type annotation;
-  An expression of type ``void`` is used as a value.


.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    let x: void // compile-time error - void used as type annotation

    function foo (): void
    let y = foo()  // compile-time error - void used as a value


Type ``void`` can be used as type argument that instantiates a generic type
if a specific value of type argument is irrelevant. In this case, it is a
synonym for type ``undefined`` (see :ref:`Type undefined`):


.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

   class A<T>
   let a = new A<void>() // ok, type parameter is irrelevant
   let a = new A<undefined>() // ok, the same

   function foo<T>(x: T) {}

   foo<void>(undefined) // ok
   foo<void>(void) // compile-time error: void is used as value

.. index::
   return type
   type argument
   instantiation
   generic type
   type parameter argument

|

.. _Array Types:

Array Types
===========

.. meta:
    frontend_status: Done

*Array type* is the built-in type characterized by the following:

-  Any object of array type contains elements indexed by integer position 
   starting from *0*;
-  Access to any array element is performed within the same time;
-  If passed to non-|LANG| environment, an array is represented as a contiguous
   memory location;
-  Types of all array elements are upper-bounded by the element type specified
   in the array declaration.

.. index::
   array type
   array element
   access
   array

Two basic operations with array elements take elements out of, and put
elements into an array by using the operator '``[]``' and index expression.

The number of elements in an array can be obtained by accessing the field
``length``. Setting a new value of this field allows shrinking the array by
reducing the number of its elements. Attempting to increase the length of the
array causes a :index:`compile-time error` (if the compiler has the information
sufficient to determine this), or to a run-time error.

An example of syntax for the built-in array type is presented below:

.. index::
   array element
   index expression
   operator

.. code-block:: abnf

    arrayType:
       type '[' ']'
       ;

The family of array types that are parts of the standard library (see
:ref:`Standard Library`), including all available operations, is described
in the library documentation. Common to these types is that the operator
'``[]``' can be applied to variables of all array types, and to their derived
types.

**Note**: Type ``T[]`` and type ``Array<T>`` are different types.
Some methods defined for ``Array<T>`` (e.g., ``at``) can be used for ``T[]``,
but only if those do not change the array length.

.. index::
   array type
   variable
   operator
   reference type
   value type
   derived type
   standard library

The examples are presented below:

.. code-block:: typescript
   :linenos:

    let a : number[] = [0, 0, 0, 0, 0] 
      /* allocate array with 5 elements of type number */
    a[1] = 7 /* put 7 as the 2nd element of the array, index of this element is 1 */
    let y = a[4] /* get the last element of array 'a' */
    let count = a.length // get the number of array elements
    a.length = 3
    y = a[2] // OK, 2 is the index of the last element now
    y = a[3] // Will lead to runtime error - attempt to access non-existing array element

    let b: Number[] = new Array<Number>
       /* That is a valid code as type used in the 'b' declaration is identical
          to the type used in the new expression */

A type alias can set a name for an array type (see :ref:`Type Alias Declaration`):

.. code-block:: typescript
   :linenos:

    type Matrix = number[][] /* Two-dimensional array */

An array as an object is assignable to a variable of type ``Object``:

.. code-block-meta:


.. code-block:: typescript
   :linenos:

    let a: number[] = [1, 2, 3]
    let o: Object = a

.. index::
   alias
   array type
   object
   array
   assignment
   variable

|

.. _Function Types:

Function Types
==============

.. meta:
    frontend_status: Done

A *function type* can be used to express the expected signature of a function.
A function type consists of the following:

-  List of parameters (which can be empty);
-  Optional return type;
-  Optional keyword ``throws``.

.. index::
   array element
   type alias
   array type
   type Object
   keyword throws
   function type
   signature

.. code-block:: abnf

    functionType:
        '(' ftParameterList? ')' ftReturnType 'throws'?
        ;

    ftParameterList:
        ftParameter (',' ftParameter)* (',' ftRestParameter)?
        | ftRestParameter
        ;

    ftParameter:
        identifier ('?')? ':' type
        ;

    ftRestParameter:
        '...' ftParameter
        ;

    ftReturnType:
        '=>' type
        ;

The ``rest`` parameter is described in :ref:`Rest Parameter`.

.. index::
   rest parameter

.. code-block:: typescript
   :linenos:

    let binaryOp: (x: number, y: number) => number
    function evaluate(f: (x: number, y: number) => number) { }

A type alias can set a name for a *function type* (see
:ref:`Type Alias Declaration`).

.. index::
   type alias
   function type

.. code-block:: typescript
   :linenos:

    type BinaryOp = (x: number, y: number) => number
    let op: BinaryOp

A function type that contains the ``throws`` mark (see :ref:`Throwing Functions`)
is the *throwing function type*.

If a function type has the '``?``' mark for a parameter name, then this
parameter and all parameters that follow (if any) are optional. Otherwise, a
:index:`compile-time error` occurs. The actual type of the parameter is then a
union of the parameter type and type ``undefined``. Note that this
parameter has no default value.

.. code-block:: typescript
   :linenos:

    type FuncTypeWithOptionalParameters = (x?: number, y?: string) => void
    let foo: FuncTypeWithOptionalParameters
        = ():void => {}          // CTE as call with more than zero arguments is invalid
    foo = (p: number):void => {} // CTE as call with zero arguments is invalid
    foo = (p?: number):void => {} // CTE as call with two arguments is invalid
    foo = (p1: number, p2?: string):void => {} // CTE as call with zero arguments is invalid
    foo = (p1?: number, p2?: string):void => {} // OK

    foo()
    foo(undefined)
    foo(undefined, undefined)
    foo(666)
    foo(666, undefined)
    foo(666, "a string")

    type IncorrectFuncTypeWithOptionalParameters = (x?: number, y: string) => void
       // compile-time error: no mandatory parameter can follow an optional parameter

    function bar (
       p1?: number,
       p2:  number|undefined
    ) {
       p1 = p2 // OK
       p2 = p1 // OK
       // Types of p1 and p2 are identical
    }


Function types assignability is described in :ref:`Assignment-like Contexts`,
and conversions in :ref:`Function Types Conversions`.

.. index::
   function type
   return type
   type void
   throwing function
   throwing function type
   throws mark

|

.. _Type null:

Type ``null``
=============

.. meta:
    frontend_status: Done

The only value of type ``null`` is represented by the keyword ``null``
(see :ref:`Null Literal`).

Using type ``null`` as type annotation is not recommended, except in
nullish types (see :ref:`Nullish Types`).

.. index::
   type null
   null literal
   keyword null
   type annotation
   nullish type

|

.. _Type undefined:

Type ``undefined``
==================

.. meta:
    frontend_status: Done

The only value of type ``undefined`` is represented by the keyword
``undefined`` (see :ref:`Undefined Literal`).

Using type ``undefined`` as type annotation is not recommended, except in
nullish types (see :ref:`Nullish Types`).

Type ``undefined`` can be used as type argument to instantiate a generic
type if the specific value of the type argument is irrelevant.

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class A<T> {}
   let a = new A<undefined>() // ok, type parameter is irrelevant
   function foo<T>(x: T) {}

   foo<undefined>(undefined) // ok


.. index::
   type undefined
   keyword undefined
   literal
   annotation
   nullish type

|

.. _Tuple Types:

Tuple Types
===========

.. meta:
    frontend_status: Done

.. code-block:: abnf

    tupleType:
        '[' (type (',' type)*)? ']' 
        ;

A *tuple* type is a reference type created as a fixed set of other types.
The value of a tuple type is a group of values of types that comprise the
tuple type. The types are specified in the same order as declared within
the tuple type declaration. It implies that each element of the tuple has
its own type.
The operator '``[]``' (square brackets) is used to access the elements of
a tuple in a manner similar to that used to access elements of an array.

An index expression belongs to an integer type. The index of the 1st tuple
element is *0*. Only constant expressions can be used as the index to get
the access to tuple elements.

.. code-block:: typescript
   :linenos:

   let tuple: [number, number, string, boolean, Object] = 
              [     6,      7,  "abc",    true,    666]
   tuple[0] = 666
   console.log (tuple[0], tuple[4]) // `666 666` be printed

Any tuple type is compatible (see :ref:`Type Compatibility`) with class
``Object`` (see :ref:`Object Class Type`).

An empty tuple is a corner case. It is only added to support |TS| compatibility:

.. code-block:: typescript
   :linenos:

   let empty: [] = [] // empty tuple with no elements in it

|

.. _Union Types:

Union Types
===========

.. meta:
   frontend_status: Partly
   todo: support literal in union
   todo: implement using common fields and methods, fix related issues

.. code-block:: abnf

    unionType:
        type ('|' type)*
        ;

A *union* type is a reference type created as a combination of other
types. Valid values of all types the union is created from are the values of a
*union* type.

A :index:`compile-time error` occurs if the type in the right-hand side of a
union type declaration leads to a circular reference.

If a *union* uses a primitive type (see *Primitive types* in
:ref:`Types by Category`) or a literal type (see :ref:`Literal Types`), then
automatic boxing (see :ref:`Boxing Conversions`) occurs to keep the reference
nature of the type.

A :index:`compile-time error` occurs if a *union* type contains more than one
numeric type.

The reduced form of *union* types allows defining a type that has one literal
type (see :ref:`Literal Types`) only:

.. index::
   union type
   reference type
   circular reference
   union
   compile-time error
   primitive type
   literal type
   primitive type
   automatic boxing

.. code-block:: typescript
   :linenos:

   type T = 3    // Literal type aliased
   let t1: T = 3 // OK
   let t2: T = 2 // Compile-time error

   type BadUnion1 = int | double // Compile-time error
   type BadUnion2 = Int | Double // Compile-time error
   type BadUnion3 = int | Double // Compile-time error
   let x = cond? new Int (): new Double /* Compile-time error as conditional
        expression contains an invalid union type */


A typical example of the *union* type usage is represented below:

.. code-block:: typescript
   :linenos:

    class Cat {
      // ...
    }
    class Dog {
      // ...
    }
    class Frog {
      // ...
    }
    type Animal = Cat | Dog | Frog | number
    // Cat, Dog, and Frog are some types (class or interface ones)

    let animal: Animal = new Cat()
    animal = new Frog() 
    animal = 42
    // One may assign the variable of the union type with any valid value

Different mechanisms can be used to get values of particular types from a
*union*:

.. code-block:: typescript
   :linenos:

    class Cat { sleep () {}; meow () {} }
    class Dog { sleep () {}; bark () {} }
    class Frog { sleep () {}; leap () {} }

    type Animal = Cat | Dog | Frog

    let animal: Animal = new Cat()
    if (animal instanceof Frog) { 
        // animal is of type Frog here, conversion can be used:
        let frog: Frog = animal as Frog
        frog.leap()
    }

    animal.sleep () // Any animal can sleep


The following example represents primitive types:

.. code-block:: typescript
   :linenos:

    type Primitive = number | boolean
    let p: Primitive = 7
    if (p instanceof Number) { // type of 'p' is Number here
       let i: number = p as number // Explicit conversion from Primitive to number
    }

The following example represents literal types:

.. code-block:: typescript
   :linenos:

    type BMW_ModelCode = 325 | 530 | 735
    let car_code: BMW_ModelCode = 325
    if (car_code == 325){
       car_code = 530
    } else if (car_code == 530){
       car_code = 735
    } else {
       // pension :-)
    }

**Note**: A :index:`compile-time error` occurs if an expression of a union type
is compared to a literal value or constant that does not belong to the values
of the union type:

.. code-block:: typescript
   :linenos:

    type BMW_ModelCode = 325 | 530 | 735
    let car_code: BMW_ModelCode = 325
    if (car_code == 666){ ... }
    /*
       compile-time error as 666 does not belong to
       values of literal type BMW_ModelCode
    */

    function model_code_test (code: number) {
       if (car_code == code) { ... }
       // This test is to be resolved during program execution
    }


.. _Union Types Normalization:

Union Types Normalization
-------------------------

.. meta:
   frontend_status: Partly
   todo: depends on literal types, maybe issues can occur for now

Union types normalization allows minimizing the number of types within a union
type, while keeping the type's safety. Some types can also be replaced for more
general types.

Formally, union type ``T``:sub:`1` | ... | ``T``:sub:`N`, where ``N`` > 1, can be
reduced to type ``U``:sub:`1` | ... | ``U``:sub:`M`, where ``M`` <= ``N``, or even to
a non-union type *V*. In this latter case *V* can be a primitive value
type, or a literal type that changes the reference nature of the union type.

The normalization process presumes performing the following steps one after
another:

.. index::
   union type
   non-union type
   normalization
   literal

#. All nested union types are linearized.
#. All type aliases if any are recursively replaced for non-alias types.
#. Identical types within the union type are replaced for a single type.
#. If at least one type in the union is ``Object``, then all other non-nullish
   types are removed.
#. If there is type ``never`` among union types, then it is removed.
#. Any numeric literal type is removed if its value fits into the numeric
   type in a union.
#. If after boxing (see :ref:`Boxing Conversions`) a primitive type equals
   another union type, then the initial type is removed.
#. If the value of the literal type in a union type belongs to the values of a
   type that is part of the union, then the literal type is removed.
#. If the numeric value of a literal type belongs to the unboxed type of one of
   union numeric class type, then the literal type is removed.
#. This step is performed recursively until no mutually compatible types remain
   (see :ref:`Type Compatibility`), or the union type is reduced to a single
   type:

   -  If a union type includes two types ``T``:sub:`i` and ``T``:sub:`j` (i != j),
      and ``T``:sub:`i` is compatible with ``T``:sub:`j` (see
      :ref:`Type Compatibility`), then only ``T``:sub:`j` remains in the union
      type, and ``T``:sub:`i` is removed.
   -  If ``T``:sub:`j` is compatible with ``T``:sub:`i` (see :ref:`Type Compatibility`),
      then ``T``:sub:`i` remains in the union type, and ``T``:sub:`j` is removed.

.. index::
   union type
   linearization
   literal type
   normalization
   Object type
   numeric union type
   compatible type
   type compatibility

The result of the normalization process is a normalized union type. The process
is presented in the examples below:

.. code-block:: typescript
   :linenos:

    ( T1 | T2) | (T3 | T4) => T1 | T2 | T3 | T4  // Linearization

    1 | 1 | 1  =>  1                             // Identical types elimination
    number | number => number                    

    number | Number => Number                    // The same after boxing
    Int | float => Int | Float => Float          // Boxing for numeric value type + heaviest left
    Int | 3.14  => Int | 3.14                    // No changes

    int|short|float|2 => float                   // The largest numeric type stays
    int|long|2.71828 => long|2.71828             // The largest numeric type stays and the literal
    1 | number | number => number                
    int | double | short => double 

    Byte | Int | Long => Long                   // The heaviest type left
    Int | 3.14 | Float => Int | Float           // 3.14 belongs to unboxed Float


    1 | string | number => string | number       // Literal type value belongs to another type values

    1 | Object => Object                         // Object wins
    AnyNonNullishType | Object => Object         

    class Base {}
    class Derived1 extends Base {}
    class Derived2 extends Base {}   
    Base | Derived1 => Base                      // Base wins
    Derived1 | Derived2 => Derived1 | Derived2   // End of normalization

The |LANG| compiler applies such normalization while processing union types and
handling the type inference for array literals (see
:ref:`Array Type Inference from Types of Elements`).

.. index::
   union type
   normalization
   array literal
   type inference
   array literal


.. _Access to Common Union Members:

Access to Common Union Members
------------------------------

.. meta:
    frontend_status: Partly

Where ``u`` is a variable of union type ``T``:sub:`1` | ... | ``T``:sub:`N`,
|LANG| supports access to a common member of ``u.m`` if the following
conditions are fulfilled:

- Each ``T``:sub:`i` is an interface or class type;

- Each ``T``:sub:`i` has a member with the name ``m``; and

- ``m`` for any ``T``:sub:`i` is one of the following:

    - Method or accessor with an equal signature; or
    - Field with the same type.

A :index:`compile-time error` occurs otherwise:


.. code-block:: typescript
   :linenos:

    class A {
        n = 1
        s = "aa"
        foo() {}
        goo(n: number) {}
    }
    class B { 
        n = 2
        s = 3.14
        foo() {}
        goo() {}
    }

    let u: A | B = new A

    let x = u.n // ok, common field
    u.foo() // ok, common method
    
    console.log(u.s) // compile-time error, field types are not equal
    u.goo() // compile-time error, signatures are not equal


.. _Keyof Types:

``Keyof`` Types
---------------

.. meta:
   frontend_status: None

A special form of union types are ``keyof`` types built by using the keyword
``keyof``. The keyword ``keyof`` is applied to the class or interface type (see
:ref:`Classes` and :ref:`Interfaces`). The resultant new type is a union of
names of all members of the class or interface type.

.. code-block:: abnf

    keyofType:
        'keyof' typeReference
        ;

A :index:`compile-time error` occurs if ``typeReference`` is not a class or
interface type. The semantics of type ``keyof`` is presented in the example
below:


.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    class A {
       field: number
       method() {}
    }
    type KeysOfA = keyof A // "field" | "method"
    let a_keys: KeysOfA = "field" // OK
    a_keys = "any string different from field or method"
      // Compile-time error: invalid value for the type KeysOfA

If the class or the interface is empty, then its type ``keyof`` is equivalent
to type ``never``:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class A {} // Empty class 
    type KeysOfA = keyof A // never


|

.. _Nullish Types:

Nullish Types
=============

.. meta:
    frontend_status: Done

|LANG| has nullish types that are in fact a special form of union types (see
:ref:`Union Types`):

.. code-block:: abnf

    nullishType:
          type '|' 'null' ('|' 'undefined')?
        | type '|' 'undefined' ('|' 'null')?
        ;

All predefined and user-defined type declarations create non-nullish types.
Non-nullish types cannot have a ``null`` or ``undefined`` value at runtime.

``T | null`` or ``T | undefined`` can be used as the type to specify a
nullish version of type ``T``.

A variable declared to have type ``T | null`` can hold the values of type ``T``
and its derived types, or the value ``null``. Such a type is called a *nullable
type*.

A variable declared to have type ``T | undefined`` can hold the values of
type ``T`` and its derived types, or the value ``undefined``.

A variable declared to have type ``T | null | undefined`` can hold values
of type ``T`` and its derived types, and the values ``undefined`` or ``null``.

A nullish type is a reference type (see :ref:`Union Types`).
A reference that is ``null`` or ``undefined`` is called a *nullish* value.

An operation that is safe with no regard to the presence or absence of
nullish values (e.g., re-assigning one nullable value to another) can
be used 'as is' for nullish types.

The following nullish-safe options exist for the operations on nullish type ``T``
that can potentially violate null safety (e.g., access to a property):

.. index::
   union type
   type inference
   array literal
   nullish type
   nullable type
   non-nullish type
   predefined type declaration
   user-defined type declaration
   undefined value
   runtime
   derived type
   reference type
   nullish value
   nullish-safe option
   null safety
   access
   assignment
   re-assignment

-  Use of safe operations:

   -  Safe method call (see :ref:`Method Call Expression` for details);
   -  Safe field access expression (see :ref:`Field Access Expression`
      for details);
   -  Safe indexing expression (see :ref:`Indexing Expressions` for details);
   -  Safe function call (see :ref:`Function Call Expression` for details);

-  Conversion from ``T | null`` or ``T | undefined`` to ``T``:

   -  Cast expression (see :ref:`Cast Expressions` for details);
   -  Ensure-not-nullish expression (see :ref:`Ensure-Not-Nullish Expressions`
      for details);

-  Supplying a default value to be used if a nullish value is present:

   -  Nullish-coalescing expression (see :ref:`Nullish-Coalescing Expression`
      for details).

.. index::
   method call
   field access expression
   indexing expression
   function call
   converting
   cast expression
   ensure-not-nullish expression
   nullish-coalescing expression
   nullish value
   cast expression

|

.. _BigInt Type:

Type ``BigInt``
===============

.. meta:
    frontend_status: Done

|LANG| has built-in ``bigint`` type and ``BigInt`` class type that allow to deal
with theoretically arbitrarily large integers. Values of this type can hold
numbers larger than the maximal value of type ``long``. This type uses the
arbitrary-precision arithmetic. Values of type ``bigint`` can be created from
the following:

- ``BigInt`` literals (see :ref:`BigInt Literals`); or
- Numeric type values, by using a call to the standard library class ``BigInt``
  methods or constructors (see :ref:`Standard Library`).

Similarly to ``string``, ``bigint`` type has dual semantics.

If created, assigned, or passed as an argument, type ``bigint`` behaves in the
same manner as a reference type (see :ref:`Reference Types`).

All applicable operations handle type ``bigint`` as a value type (see
:ref:`Value Types`). These operations are described in
:ref:`Integer Types and Operations`.

Type ``bigint`` is to be used as type annotation. Type ``BigInt`` is to
create new objects and calls to static methods of class ``BigInt``
(see :ref:`BigInt Literals`):

.. code-block:: typescript
   :linenos:

   let b1: bigint = new BigInt (5)
   let b2: bigint = 123n

|

.. _Default Values for Types:

Default Values for Types
************************

.. meta:
    frontend_status: Done

**Note**: This is the |LANG|'s experimental feature.

Some types use so-called *default values* for variables without explicit
initialization (see :ref:`Variable Declarations`), including the following:

.. - All primitive types and *string* (see the table below).

- Primitive types (see the table below);
- Literal types;
- All union types that have at least one nullish (see :ref:`Nullish Types`)
  value, and use an appropriate nullish value as default (see the table below).

.. -  Nullable reference types with the default value *null* (see :ref:`Literals`).

All other types, including reference types, enumeration types, and type
parameters have no default values. Variables of such types must be initialized
explicitly with a value before the first use of a type.

.. Default values of primitive types are as follows:

Default values of primitive types are as follows:

.. index::
   default value
   variable
   explicit initialization
   nullable reference type
   primitive type
   reference type
   enumeration type

+--------------+--------------------+
|   Data Type  |   Default Value    |
+==============+====================+
| ``number``   | 0 as ``number``    |
+--------------+--------------------+
| ``byte``     | 0 as ``byte``      |
+--------------+--------------------+
| ``short``    | 0 as ``short``     |
+--------------+--------------------+
| ``int``      | 0 as ``int``       |
+--------------+--------------------+
| ``long``     | 0 as ``long``      |
+--------------+--------------------+
| ``float``    | +0.0 as ``float``  |
+--------------+--------------------+
| ``double``   | +0.0 as ``double`` |
+--------------+--------------------+
| ``char``     | ``u0000``          |
+--------------+--------------------+
| ``boolean``  | ``false``          |
+--------------+--------------------+

Default values of literal types are literals of literal types:

.. code-block:: typescript
   :linenos:

    let a: 1
    let b: true
    let c: 3.14
    let d: "string literal"
    let e: c'C'
    let f: 123n
    let g: null
    let h: undefined

    printThem (a, b, c, d, e, f, g, h)
    function printThem (p1: 1, p2: true, p3: 3.14, p4: "string literal", p5: c"C", p6: 123n, p7: null, p8: undefined) {
        console.log (p1, p2, p3, p4, p5, p6, p7, p8)
        // Output: 1 true 3.14 undefined C 123 null undefined
    }



Default values of nullish union types are as follows:

+----------------------+--------------------+
|    Data Type         |   Default Value    |
+======================+====================+
| ``type | null``      | ``null``           |
+----------------------+--------------------+
| ``type | undefined`` | ``undefined``      |
+----------------------+--------------------+
| ``null | undefined`` | ``undefined``      |
+----------------------+--------------------+

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class A {
     f1: number|null
     f2: string|undefined
     f3?: boolean
   }
   let a = new A()
   console.log (a.f1, a.f2, a.f3)
   // Output: null, undefined, undefined


.. index::
   number
   byte
   short
   int
   long
   float
   double
   char
   boolean


-------------

.. [3]
   Wherever IEEE 754 is used in this Specification, the reference is to the
   latest revision of "754-2019--IEEE Standard for Floating-Point Arithmetic".


.. raw:: pdf

   PageBreak


