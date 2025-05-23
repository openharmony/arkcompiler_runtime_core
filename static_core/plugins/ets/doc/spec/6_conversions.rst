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

.. _Contexts and Conversions:

Contexts and Conversions
########################

.. meta:
    frontend_status: Done

This Chapter defines evaluation of type of expressions depending
on expression contexts and set of conversions that can be applied
to expressions.

.. _Type of Expression:

Type of Expression
******************

.. meta:
    frontend_status: Done

Every expression written in the |LANG| programming language has a type that
is evaluated at compile time.

In most contexts, an expression must be *compatible* with a type expected in
that context. This type is called the *target type*.

If no target type is expected in the context, then the expression
is called a *standalone expression*:

.. code-block:: typescript
   :linenos:

    let a = expr // no target type is expected

    function foo() {
        expr // no target type is expected
    }

Otherwise, the expression is *non-standalone*:

.. index::
   inferred type
   expression
   type inference
   compatible expression
   standalone expression
   context
   target type

.. code-block-meta:
   skip

.. code-block:: typescript
   :linenos:

    let a: number = expr // target type of 'expr' is number

    function foo(s: string) {}
    foo(expr) // target type of 'expr' is string

The type of some expressions cannot be inferred (see :ref:`Type Inference`)
from the expression itself (see :ref:`Object Literal` as an example).
A :index:`compile-time error` occurs if such an expression
is used as a *standalone expression*.

There are two ways to facilitate the compatibility of a *non-standalone
expression* with its surrounding context:

#. The type of some non-standalone expressions can be inferred from the
   target type (a type of expression can be different in different contexts).

#. In the :ref:`Assignment-like Contexts` inferred expression type
   can be different from the target type, then
   performing an implicit *conversion* can ensure :ref:`Assignability`.
   The conversion from type ``S`` to type ``T`` causes a type ``S`` expression
   to be handled as a type ``T`` expression at compile time.

.. index::
   expression
   standalone expression
   non-standalone expression
   compatible type
   compatibility
   surrounding context
   context
   inferred type
   conversion
   assignability
   assignable type
   compile time

A :index:`compile-time error` occurs if neither produces an appropriate
expression type.

The rules that determine whether a *target type* allows an implicit
*conversion* vary for different kinds of contexts and types of expressions.
The *target type* can influence not only the type of the expression but also,
in some cases, its runtime behavior.

Some cases of conversion require action at runtime to check the
validity of conversion, or to translate the runtime expression value
into a form that is appropriate for the new type ``T``.

.. index::
   runtime behavior
   expression
   expression type
   expression value
   target type
   context
   runtime behavior
   value
   conversion

If the type of the expression is ``readonly``, then the target type must
also be ``readonly``. Otherwise, a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

      let readonly_array: readonly number[] = [1, 2, 3]

      foo1(readonly_array) // OK
      foo2(readonly_array) // compile-time error

      function foo1 (p: readonly number[]) {}
      function foo2 (p: number[]) {}

      let writable_array: number [] = [1, 2, 3]
      foo1 (writable_array) // OK, as always safe

Contexts can be of the following kinds:

-  :ref:`Assignment-like Contexts`;

-  :ref:`String Operator Contexts` with ``string`` concatenation (operator '``+``');

-  :ref:`Numeric Operator Contexts` with all numeric operators ('``+``', '``-``', etc.).

.. index::
   expression
   readonly
   target type
   assignment-like context
   assignment
   expression value
   string concatenation
   concatenation
   context
   operator
   numeric operator
   conversion
   type

|

.. _Assignment-like Contexts:

Assignment-like Contexts
************************

.. meta:
    frontend_status: Partly
    todo: Need to adapt es2panda implementation after assignment and call contexts are unified

*Assignment-like contexts* include the following:

- *Declaration contexts* that allow setting an initial value to a variable
  (see :ref:`Variable Declarations`), a constant (see
  :ref:`Constant Declarations`), or a field (see :ref:`Field Declarations`)
  with an explicit type annotation;

- *Assignment contexts* that allow assigning (see :ref:`Assignment`) an
  expression value to a variable;

- *Call contexts* that allow assigning an argument value to a corresponding
  formal parameter of a function, method, constructor or lambda call (see
  :ref:`Function Call Expression`, :ref:`Method Call Expression`,
  :ref:`Explicit Constructor Call`, and :ref:`New Expressions`);

- *Composite literal contexts* that allow setting an expression value to an
  array element (see :ref:`Array Literal Type Inference from Context`),
  a class, or an interface field (see :ref:`Object Literal`);

.. index::
   assignment
   assignment-like context
   assignment context
   call context
   constant declaration
   constant
   field declaration
   assignment context
   expression value
   expression
   conversion
   function call
   constructor call
   lambda call
   method call
   formal parameter
   array literal
   object literal
   value
   variable
   constant
   composite literal context
   function
   method
   constructor
   expression value
   array element

The examples are presented below:

.. code-block:: typescript
   :linenos:

      // declaration contexts:
      let x: number = 1
      const str: string = "done"
      class C {
        f: string = "aa"
      }

      // assignment contexts:
      x = str.length
      new C().f = "bb"
      function foo<T1, T2> (p1: T1, p2: T2) {
        let t1: T1 = p1
        let t2: T2 = p2
      }

      // call contexts:
      function foo(s: string) {}
      foo("hello")

      // composite literal contexts:
      let a: number[] = [str.length, 11]

In all these cases, the expression type must be *assignable* to the *target
type* (see :ref:`Assignability`).
*Assignability* allows using of one of :ref:`Implicit Conversions`.
If there is no applicable conversion, then a :index:`compile-time error`
occurs.

.. index::
   expression type
   target type
   assignability

|

.. _String Operator Contexts:

String Operator Contexts
************************

.. meta:
    frontend_status: Done

*String context* applies only to a non-*string* operand of the binary operator
'``+``' if the other operand is ``string``.

*String conversion* for a non-``string`` operand is evaluated as follows:

-  An operand of an integer type (see :ref:`Integer Types and Operations`)
   is converted to type ``string`` with a value that represents the operand in
   the decimal form.

-  An operand of a floating-point type (see :ref:`Floating-Point Types and Operations`)
   is converted to type ``string`` with a value that represents the operand in
   the decimal form (without the loss of information).

-  An operand of type ``boolean`` is converted to type ``string`` with the
   values ``true`` or ``false``.

-  An operand of enumeration type (see :ref:`Enumerations`) is converted to
   type ``string`` with the value of the corresponding enumeration constant
   if values of enumeration are of type ``string``.

-  The operand of a nullish type that has a nullish value is converted as
   described below:

     - The operand ``null`` is converted to string ``null``;
     - The operand ``undefined`` is converted to string ``undefined``.

-  An operand of a reference type or ``enum`` type is converted by applying the
   method call ``toString()``.

If there is no applicable conversion, then a :index:`compile-time error` occurs.

The target type in this context is always ``string``:

.. index::
   string context
   non-string operand
   binary operator
   string operand
   string conversion
   conversion
   reference type
   integer type
   operand
   floating-point type
   loss of information
   enumeration type
   string
   boolean
   decimal
   string conversion
   operand null
   operator undefined
   context

.. code-block:: typescript
   :linenos:

    console.log("" + null) // prints "null"
    console.log("value is " + 123) // prints "value is 123"
    console.log("BigInt is " + 123n) // prints "BigInt is 123"
    console.log(15 + " steps") // prints "15 steps"
    let x: string | null = null
    console.log("string is " + x) // prints "string is null"
    let c = c'X'
    console.log("char is " + c) // prints "char is X"

|

.. _Numeric Operator Contexts:

Numeric Operator Contexts
*************************

.. meta:
    frontend_status: Done

Numeric contexts apply to the operands of an arithmetic operator.
Numeric contexts use numeric types conversions
(see :ref:`Widening Numeric Conversions`), and ensure that each argument
expression can be converted to target type ``T`` while the arithmetic
operation for the values of type ``T`` is being defined.

An operand of an enumeration type (see :ref:`Enumerations`) can be used in
the numeric context if values of this enumeration are of type ``int``.
The type of this operand is assumed to be ``int``.

.. index::
   numeric context
   arithmetic operator
   predefined type
   numeric type
   conversion
   primitive type
   argument expression
   target type
   string conversion
   string context
   type int

Numeric contexts actually take the following forms:

-  :ref:`Unary Expressions`;
-  :ref:`Multiplicative Expressions`;
-  :ref:`Additive Expressions`;
-  :ref:`Shift Expressions`;
-  :ref:`Relational Expressions`;
-  :ref:`Equality Expressions`;
-  :ref:`Bitwise and Logical Expressions`;
-  :ref:`Conditional-And Expression`;
-  :ref:`Conditional-Or Expression`.

.. index::
   numeric context
   expression
   unary expression
   multiplicative expression
   additive expression
   shift expression
   relational expression
   equality expression
   bitwise expression
   logical expression
   conditional-and expression
   conditional-or expression

|

.. _Implicit Conversions:

Implicit Conversions
********************

.. meta:
   frontend_status: Done
   todo: Narrowing Reference Conversion - note: Only basic checking available, not full support of validation
   todo: String Conversion - note: Implemented in a different but compatible way: spec - toString(), implementation: StringBuilder
   todo: Forbidden Conversion - note: Not exhaustively tested, should work

This section describes all implicit conversions that are allowed. Each
conversion is allowed in a particular context (for example, if an expression
that initializes a local variable is subject to :ref:`Assignment-like Contexts`,
then the rules of this context define what specific conversion is implicitly
chosen for the expression).

.. index::
   identity conversion
   conversion
   context
   local variable
   assignment
   assignment-like context
   conversion
   expression
   variable

|

.. _Widening Numeric Conversions:

Widening Numeric Conversions
==============================

.. meta:
    frontend_status: Partly
    todo: short to char conversion

*Widening numeric conversions* convert the following:

- Values of a smaller numeric type to a larger type (see
  :ref:`Numeric Types`);

- Values of an *enumeration* type (if enumeration constants of this type are
  of numeric type) to the same or a wider numeric type.

.. index::
   widening
   primitive conversion
   conversion
   numeric type
   value
   byte
   short
   enumeration type
   int
   long
   float
   integer type


+------------------+------------------------------------------------------+
| From             | To                                                   |
+==================+======================================================+
| ``byte``         | ``short``, ``int``, ``long``, ``float``, ``double``  |
+------------------+------------------------------------------------------+
| ``short``        | ``int``, ``long``, ``float``, ``double``             |
+------------------+------------------------------------------------------+
| ``int``          | ``long``, ``float``, or ``double``                   |
+------------------+------------------------------------------------------+
| ``long``         | ``float`` or ``double``                              |
+------------------+------------------------------------------------------+
| ``float``        | ``double``                                           |
+------------------+------------------------------------------------------+
| enumeration with | larger numeric type                                  |
| numeric constants|                                                      |
+------------------+------------------------------------------------------+

These conversions cause no loss of information about the overall magnitude of
a numeric value. Some least significant bits of the value can be lost only in
conversions from an integer type to a floating-point type if the IEEE 754
*round-to-nearest* mode is used correctly. The resultant floating-point value
is properly rounded to the integer value.

*Widening numeric conversions* never cause runtime errors.

.. index::
   conversion
   numeric value
   floating-point type
   integer type
   conversion
   round-to-nearest mode
   runtime error
   IEEE 754
   enumeration constant
   widening
   primitive conversion
   rounding

|

.. _Enumeration to Constants Type Conversions:

Enumeration to Constants Type Conversions
=========================================

.. meta:
    frontend_status: Done

A value of an *enumeration* type is converted to type ``int``
if enumeration constants of this type are of type ``int``.
This conversion never causes runtime errors.

.. code-block:: typescript
   :linenos:

    enum IntegerEnum {a, b, c}
    let int_enum: IntegerEnum = IntegerEnum.a
    let int_value: int = int_enum // int_value will get the value of 0
    let number_value: number = int_enum
       /* number_value will get the value of 0 as a result of conversion
          sequence: enumeration -> int - > number  */

.. index::
   enumeration type
   conversion
   constant
   runtime error
   type int


A value of ``enumeration`` type is converted to type ``string`` if enumeration
constants of this type are of type ``string``.
This conversion never causes runtime errors.

.. code-block:: typescript
   :linenos:

    enum StringEnum {a = "a", b = "b", c = "c"}
    let string_enum: StringEnum = StringEnum.a
    let a_string: string = string_enum // a_string will get the value of "a"

.. index::
   enumeration type
   type string
   conversion
   constant
   runtime error

A value of an *enumeration* type with explicitly declared type of its constants
is converted to the declared type. This conversion never causes runtime errors.

.. code-block:: typescript
   :linenos:

    enum DoubleEnum: double {a = 1.0, b = 2.0, c = 3.141592653589}
    let dbl_enum: DoubleEnum = DoubleEnum.a
    let dbl_value: double = dbl_enum // dbl_value will get the value of 1.0

.. index::
   enumeration type
   conversion
   constant
   runtime error


|

.. _Constant to Enumeration Conversions:

Constant to Enumeration Conversions
===================================

.. meta:
    frontend_status: None

A constant expression of some integer type is converted to *enumeration* type if:

-  Enumeration constants are of type ``int``;
-  Value of the constant expression is equal to the value of one of the
   enumeration type constants.

This conversion never causes runtime errors.

.. code-block:: typescript
   :linenos:

    enum IntegerEnum {a, b, c}
    let e: IntegerEnum = 1 // ok, e is set to IntegerEnum.b
    e = 3 // compile-time error, there is no constant with this value

    const one = 2
    e = one // ok, e is set to IntegerEnum.c

A similar conversion of a string type expression also works as follows:

.. code-block:: typescript
   :linenos:

    enum StringEnum {A = "a", B = "b", C = "c"}
    let e: StringEnum = "b" // OK, , e is set to StringEnum.B
    e = "d" // compile-time error, there is no constant with this value
    e = StringEnum.B // OK

.. index::
   constant expression
   integer type
   conversion
   enumeration type
   enumeration constant
   type int
   string type
   expression

A similar conversion is applcable for constants of the *enumeration* explicitly
declared type. This conversion never causes runtime errors.

.. code-block:: typescript
   :linenos:

    enum DoubleEnum: double {a = 1.0, b = 2.0, c = 3.141592653589}
    let dbl_enum: DoubleEnum = DoubleEnum.a
    dbl_enum = 1.0 // OK
    dbl_enum = 5.0 // compile-time error, there is no constant with this value

.. index::
   constant expression
   conversion
   enumeration type
   enumeration constant
   expression



|

.. _Numeric Casting Conversions:

Numeric Casting Conversions
***************************

.. meta:
    frontend_status: Done

A *numeric casting conversion* occurs if the *target type* and the expression
type are both ``numeric``. 
There are two contexts when *numeric casting conversion* are applied:

-  Using conversion methods defined in the standard library
   (see :ref:`Standard Library`);

-  Or, implicitly in the following arithmetic operations:
   :ref:`Postfix Increment`, :ref:`Postfix Decrement`, 
   :ref:`Prefix Increment`, :ref:`Prefix Decrement`.

The following example illustrates explicit use of 
methods for *numeric cast conversions*:

.. code-block-meta:
   not-subset

.. code-block:: typescript
   :linenos:

    function process_int(an_int: int) { ... }

    let pi = 3.14
    process_int(pi.toInt())

These conversions never cause runtime errors.

Numeric casting conversion of an operand of type ``double`` to target type
``float`` is performed in compliance with the IEEE 754 rounding rules. This
conversion can lose precision or range, resulting in the following:

-  Float zero from a nonzero double; and
-  Float infinity from a finite double.

Double ``NaN`` is converted to float ``NaN``.

Double infinity is converted to the same-signed floating-point infinity.

.. index::
   numeric casting conversion
   target type
   expression type
   numeric type
   double type
   float type
   compliance
   rounding rule
   float zero
   nonzero double
   float infinity
   infinity double
   floating-point infinity

A numeric conversion of a floating-point type operand to target types ``long``
or ``int`` is performed by the following rules:

- If the operand is ``NaN``, then the result is 0 (zero).
- If the operand is positive infinity, or if the operand is too large for the
  target type, then the result is the largest representable value of the target
  type.
- If the operand is negative infinity, or if the operand is too small for
  the target type, then the result is the smallest representable value of
  the target type.
- Otherwise, the result is the value that rounds toward zero by using IEEE 754
  *round-toward-zero* mode.

A numeric casting conversion of a floating-point type operand to types
``byte`` or ``short`` is performed in two steps as follows:

- The casting conversion to ``int`` is performed first (see above);
- Then, the ``int`` operand is cast to the target type.

.. index::
   target type
   floating-point operand
   numeric conversion
   byte
   short
   positive infinity
   negative infinity
   casting conversion
   runtime error
   operand
   compliance
   IEEE 754
   NaN
   floating-point type
   floating-point infinity
   rounding rules

A numeric casting conversion from an integer type to a smaller integer
type ``I`` discards all bits except the *N* lowest ones, where *N* is
the number of bits used to represent type ``I``. This conversion can lose the
information on the magnitude of the numeric value. The sign of the resulting
value can differ from that of the original value.

.. index::
   IEEE 754
   floating-point type
   operand
   NaN
   positive infinity
   target type
   negative infinity
   casting conversion
   integer type
   conversion
   rounding rule
   numeric value

|

.. raw:: pdf

   PageBreak
