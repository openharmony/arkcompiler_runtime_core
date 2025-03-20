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

Every expression written in the |LANG| programming language has a type that
is inferred (see :ref:`Type Inference`) at compile time.

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

.. code-block-meta:
   skip

.. code-block:: typescript
   :linenos:

    let a: number = expr // target type of 'expr' is number

    function foo(s: string) {}
    foo(expr) // target type of 'expr' is string

The type of some expressions cannot be inferred from the expression itself
(see :ref:`Object Literal` as an example). A :index:`compile-time error` occurs
if such an expression is used as a *standalone expression*.

There are two ways to facilitate the compatibility of a *non-standalone
expression* with its surrounding context:

#. The type of some non-standalone expressions can be inferred from the
   target type (expression types can be different in different contexts).

#. If the inferred expression type is different from the target type, then
   performing an implicit *conversion* can ensure :ref:`Assignability`.
   The conversion from type ``S`` to type ``T`` causes a type ``S`` expression to
   be handled as a type ``T`` expression at compile time.

.. index::
   context
   conversion
   compile time
   inference
   target type
   type
   surrounding context
   expression
   compatible type
   compatibility
   expression
   standalone expression
   non-standalone expression

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
   target type
   context
   runtime
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

-  :ref:`Assignment-like Contexts` where the expression value is bound to a
   variable;

-  :ref:`String Operator Contexts` with ``string`` concatenation (operator '``+``');

-  :ref:`Numeric Operator Contexts` with all numeric operators ('``+``', '``-``', etc.);

-  :ref:`Casting Contexts and Conversions`, i.e., the conversion of an
   expression value to a type explicitly specified by a cast expression (see
   :ref:`Cast Expressions`).

.. index::
   expression
   readonly
   target type
   expression value
   concatenation
   operator
   numeric operator
   conversion
   type
   cast expression
   string concatenation

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
   assignment context
   call context
   expression
   conversion
   function call
   constructor call
   method call
   formal parameter
   array literal
   object literal
   value
   variable
   constant
   interface field
   type annotation
   assignment context
   assignment-like context
   declaration context
   call context
   composite literal context
   function
   method
   constructor
   lambda call
   expression value
   array element
   assignment
   argument value

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

In all these cases, the expression type either must be equal to the *target
type*, or can be converted to the *target type* by using one of the conversions
discussed below. Otherwise, a :index:`compile-time error` occurs.

Assignment-like contexts allow using of one of the following:

- :ref:`Widening Primitive Conversions`;

- :ref:`Constant Narrowing Integer Conversions`;

- :ref:`Widening Union Conversions`;

- :ref:`Widening Reference Conversions`;

- :ref:`Character to String Conversions`;

- :ref:`Constant String to Character Conversions`;

- :ref:`Function Types Conversions`;

- :ref:`Enumeration to Constants Type Conversions`;

- :ref:`Constant to Enumeration Conversions`;

- :ref:`Literal Type to its Supertype Conversions`.

.. index::
   expression type
   target type
   conversion
   assignment
   widening
   narrowing
   constant
   constant
   string
   enumeration
   literal type

If there is no applicable conversion, then a :index:`compile-time error`
occurs.

|

.. _String Operator Contexts:

String Operator Contexts
************************

.. meta:
    frontend_status: Done

*String context* applies only to a non-*string* operand of the binary operator
'``+``' if the other operand is ``string``.

*String conversion* for a non-``string`` operand is evaluated as follows:

-  The operand of a nullish type that has a nullish value is converted as
   described below:

     - The operand ``null`` is converted to string ``null``;
     - The operand ``undefined`` is converted to string ``undefined``.

-  An operand of a reference type or ``enum`` type is converted by applying the
   method call ``toString()``.

-  An operand of an integer type (see :ref:`Integer Types and Operations`)
   is converted to type ``string`` with a value that represents the operand in
   the decimal form.

-  An operand of a floating-point type (see :ref:`Floating-Point Types and Operations`)
   is converted to type ``string`` with a value that represents the operand in
   the decimal form (without the loss of information).

-  An operand of type ``boolean`` is converted to type ``string`` with the
   values ``true`` or ``false``.

-  An operand of type ``char`` is converted by using :ref:`Character to String Conversions`.

-  An operand of enumeration type (see :ref:`Enumerations`) is converted to
   type ``string`` with the value of the corresponding enumeration constant
   if values of enumeration are of type ``string``.

If there is no applicable conversion, then a :index:`compile-time error` occurs.

The target type in this context is always ``string``:

.. code-block:: typescript
   :linenos:

    console.log("" + null) // prints "null"
    console.log("value is " + 123) // prints "value is 123"
    console.log("BigInt is " + 123n) // prints "BigInt is 123"
    console.log(15 + " steps") // prints "15 steps"
    let x: string | null = null
    console.log("string is " + x) // prints "string is null"
    let c = "X"
    console.log("char is " + c) // prints "char is X"

.. index::
   string context
   string operator
   string
   conversion
   enumeration type
   char
   boolean
   decimal
   string conversion
   operand null
   operator undefined
   reference type

|

.. _Numeric Operator Contexts:

Numeric Operator Contexts
*************************

.. meta:
    frontend_status: Done

Numeric contexts apply to the operands of an arithmetic operator.
Numeric contexts use combinations of predefined numeric types conversions
(see :ref:`Primitive Types Conversions`), and ensure that each argument
expression can be converted to target type ``T`` while the arithmetic
operation for the values of type ``T`` is being defined.

An operand of an enumeration type (see :ref:`Enumerations`) can be used in
the numeric context if values of this enumeration are of type ``int``.
The type of this operand is assumed to be ``int``.

.. index::
   numeric context
   arithmetic operator
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
   unary
   multiplicative operator
   additive operator
   shift operator
   relational operator
   equality operator
   bitwise operator
   logical operator
   conditional-and operator
   conditional-or operator
   shift operator
   relational expression
   equality expression
   bitwise expression
   logical expression
   conditional-and expression
   conditional-or expression

|

.. _Casting Contexts and Conversions:

Casting Contexts and Conversions
********************************

.. meta:
    frontend_status: Done
    todo: Does not work for interfaces, eg. let x:iface1 = iface_2_inst as iface1; let x:iface1 = iface1_inst as iface1

Every cast expression (:ref:`Cast Expressions`) introduces a *casting
context*, and relies on the application of different conversions. These
conversions cast an operand in a cast expression to an explicitly specified
*target type* by using one of the following:

- Identity conversion, if the *target type* is the same as the expression type;
- :ref:`Implicit Conversions`;
- :ref:`Numeric Casting Conversions`;
- :ref:`Class or Interface Casting Conversions`;
- :ref:`Casting Conversions from Object`;
- :ref:`Casting Conversions from Type Parameter`;
- :ref:`Casting Conversions to Type Parameter`;
- :ref:`Casting Conversions from Union`;
- :ref:`Casting Conversions to Enumeration`.

If there is no applicable conversion, then a :index:`compile-time error`
occurs.


.. index::
   casting context
   cast expression
   target type
   conversion
   expression type
   implicit conversion

|

.. _Numeric Casting Conversions:

Numeric Casting Conversions
===========================

.. meta:
    frontend_status: Done

A *numeric casting conversion* occurs if the *target type* and the expression
type are both ``numeric`` or ``char``:

.. code-block-meta:
   not-subset

.. code-block:: typescript
   :linenos:

    function process_int(an_int: int) { ... }

    let pi = 3.14
    process_int(pi as int)

These conversions never cause runtime errors.

Numeric casting conversion of an operand of type ``double`` to target type
``float`` is performed in compliance with the IEEE 754 rounding rules. This
conversion can lose precision or range, resulting in the following:

-  Float zero from a nonzero double; and
-  Float infinity from a finite double.

Double ``NaN`` is converted to float ``NaN``.

Double infinity is converted to the same-signed floating-point infinity.

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
``short``, ``byte``, or ``char`` is performed in two steps as follows:

- The casting conversion to ``int`` is performed first (see above);
- Then, the ``int`` operand is cast to the target type.

.. index::
   target type
   numeric
   char
   conversion
   runtime error
   operand
   compliance
   IEEE 754
   NaN
   floating-point type
   floating-point infinity
   rounding rules

A numeric casting conversion from an integer type (or char) to a smaller integer
type (or char) ``I`` discards all bits except the *N* lowest ones, where *N* is
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
   rounding

|

.. _Class or Interface Casting Conversions:

Class or Interface Casting Conversions
======================================

.. meta:
    frontend_status: Done

A *class casting conversion* or *interface casting conversion* occurs if the
*target type* and the expression type are both of ``class`` or ``interface``
type. This conversion casts an expression of a supertype (superclass or
superinterface, see  :ref:`Supertyping`) to a subclass or subinterface:

.. index::
   expression
   conversion
   operand
   class
   interface
   subclass
   subinterface
   superinterface
   superclass

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base {}

    let b: Base = new Derived()
    let d: Derived = b as Derived

If *target type* is not assignable (see :ref:`Assignability`) to the
expression type then a :index:`compile-time error` occurs.

A runtime error (``ClassCastError``) occurs during this conversion if the
type of a converted expression cannot be cast to the *target type*:

.. code-block:: typescript
   :linenos:

    interface I {}
    class A implements I {}
    class B implements I {}
    class C {}

    let a: A = new A
    let i: I = a
    i as B // Will trigger a runtime error ``ClassCastError``
    i as C // Compile-time error as C is not compatible with I
    a as B // Compile-time error as B is not compatible with A

.. index::
   runtime error
   conversion
   target type

|

.. _Casting Conversions from Object:

Casting Conversions from ``Object``
===================================

.. meta:
    frontend_status: Done

*Casting conversion from ``Object``* attempts to convert an expression
of type ``Object`` to any reference type (see :ref:`Reference Types`) which
is to be specified as *target type*.

.. code-block:: typescript
   :linenos:

    function check(kind: string, o: Object)
        switch (kind) {
        case "bool": console.log(o as boolean); break
        case "str" : console.log(o as string); break
        }
    }

This conversion causes a runtime error (``ClassCastError``) if the runtime
type of an expression is not the *target type*.

|

.. _Casting Conversions from Type Parameter:

Casting Conversions from Type Parameter
=======================================

.. meta:
    frontend_status: Done

*Casting conversion from a type parameter* attempts to convert an expression
of the type parameter to any reference type (see :ref:`Reference Types`) which
is to be specified as *target type*.

.. code-block:: typescript
   :linenos:

    class X<S, T> {
       method (p: T) {
          p as Object           // OK
          p as Object[]         // OK
          p as [Object, Object] // OK
          p as () => void       // OK
          p as T                // OK
          p as S                // OK
          p as number           // OK
       }
    }

This conversion causes a runtime error (``ClassCastError``) if the runtime
type of an expression is not the *target type*.

|

.. _Casting Conversions to Type Parameter:

Casting Conversions to Type Parameter
=====================================

.. meta:
    frontend_status: Done

*Casting conversion to type parameter* attempts to convert an expression of any
type to type parameter type (see :ref:`Type Parameters`) which is to be
specified as *target type*.

.. code-block:: typescript
   :linenos:

    function foo<T> (p: AnyType) {
       p as T // attempt to convert of any type to type parameter
    }

This conversion causes a runtime error (``ClassCastError``) if the runtime
type of an expression is not the *target type*.


|

.. _Casting Conversions from Union:

Casting Conversions from Union
==============================

.. meta:
    frontend_status: Done

*Casting conversion from union* converts an expression of union type to one
of the types of the union, or to a type that is derived from such one type.

For union type ``U = T``:sub:`1` ``| ... | T``:sub:`N`, the *casting conversion
from union* converts an expression of type ``U`` to some type ``TT``
(*target type*).

..
   line 472 initially was *U* = *T*:sub:`1` | ... | *T*:sub:`N`

A :index:`compile-time error` occurs if target type ``TT`` is not one of
``T``:sub:`i`, and not derived from one of ``T``:sub:`i`.

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class Cat { sleep () {}; meow () {} }
    class Dog { sleep () {}; bark () {} }
    class Frog { sleep () {}; leap () {} }
    class Spitz extends Dog { override sleep() { /* 18-20 hours a day */ } }

    type Animal = Cat | Dog | Frog | number

    let animal: Animal = new Spitz()
    if (animal instanceof Frog) {
        let frog: Frog = animal as Frog // Use casting conversion here
        frog.leap() // Perform an action specific for the particular union type
    }
    if (animal instanceof Spitz) {
        let dog = animal as Spitz // Use casting conversion here
        dog.sleep()
          // Perform an action specific for the particular union type derivative
    }

.. index::
   conversion from union
   conversion
   expression
   union type
   type
   target type

This conversion cause a runtime error (``ClassCastError``) if the runtime
type of an expression is not the *target type*.

Another form of *conversion from union* is implicit conversion from union type
to the target type. The conversion is only possible if each type in a union is
assignable (see :ref:`Assignability`) to the target type. If so, the
conversion never causes a runtime error. The conversion causes a
:index:`compile-time error` if at least one type of a union is not assignable
to the target type:

.. code-block-meta:
   expect-cte

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived1 extends Base {}
    class Derived2 extends Base {}

    let d: Derived1 | Derived2 = ...
    let b: Base = d // OK, as Derived1 and Derived2 are assignable to Base

    let x: double | Base = ...
    let y: double = x // Compile-time error, as Base cannot be converted to double

.. index::
   target type
   conversion from union
   expression
   runtime error
   union

|

.. _Casting Conversions to Enumeration:

Casting Conversions to Enumeration
==================================

.. meta:
    frontend_status: None

Casting conversion can be used to convert a value of a numeric type expression
into a value of *enumeration* type if the value:

-  Can be converted into type ``int``;
-  Equals the value of an enumeration type constant.

Casting conversion can be used to convert a value of a ``string`` or
``string literal`` expression type into a value of *enumeration* type if the
value:

-  Can be converted into type ``string``;
-  Equals the value of an enumeration type constant.


If an expression is a constant, then rules of
:ref:`Constant to Enumeration Conversions` apply.

If the expression value can be evaluated at compile-time, then the checks
described above are performed at compile time. A :index:`compile-time error`
occurs if a check fails. Otherwise, the checks are performed during program
execution. A runtime error occurs if a check fails.


.. code-block:: typescript
   :linenos:

    enum IntegerEnum {a, b, c}

    let e: IntegerEnum = 1 /* ok, e is set to IntegerEnum.b,
                              constant to enumeration implicit conversion */

    e = (1 + 1 + 1) as IntegerEnum /* compile-time error, there is no constant
                                      with this value */

    let x = 1
    e = x as IntegerEnum /* OK, as compiler can guarantee that enum
                            consistency is not violated */

    e = foo(false) as IntegerEnum // runtime check is required

    function foo(some_condition: boolean) {
       if (some_condition)
          return 1 // Valid enum constant value
       else
          return 42 // Invalid enum constant value - will cause runtime error
    }


    enum StringEnum {a = "AA", b = "BBB", c = "C"}

    let s: StringEnum = "BBB" as StringEnum /* ok, e is set to StringEnum.b,
                                               constant to enumeration cast */

    s = ("1" + "1" + "1") as StringEnum /* compile-time error, there is no constant
                                      with this value */

    let y = "C"
    s = y as StringEnum /* OK, as compiler can guarantee that enum
                            consistency is not violated */

    s = bar(false) as StringEnum // runtime check is required

    function bar(some_condition: boolean) {
       if (some_condition)
          return "AA" // Valid enum constant value
       else
          return "DD" // Invalid enum constant value - will cause runtime error
    }




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
   compatible type
   predefined numeric types conversion
   numeric type
   reference type conversion
   string conversion
   conversion

|

.. _Primitive Types Conversions:

Primitive Types Conversions
===========================

.. meta:
    frontend_status: Done

*Primitive type conversion* is one of the following:

- :ref:`Widening Primitive Conversions`;

- :ref:`Constant Narrowing Integer Conversions`.

.. index::
   primitive type conversion
   primitive type
   narrowing
   widening

|

.. _Widening Primitive Conversions:

Widening Primitive Conversions
==============================

.. meta:
    frontend_status: Done

*Widening primitive conversions* convert the following:

- Values of a smaller numeric type to a larger type (see
  :ref:`Numeric Types`);

- Values of type ``byte`` to type ``char`` (see :ref:`Character Type and Operations`);

- Values of type ``char`` to types ``int``, ``long``, ``float``, and ``double``;

- Values of an *enumeration* type to types ``int``, ``long``, ``float``, and
  ``double`` (if enumeration constants of this type are of type ``int``).

+------------------+------------------------------------------------------------------+
| From             | To                                                               |
+==================+==================================================================+
| ``byte``         | ``short``, ``int``, ``long``, ``float``, ``double``, or ``char`` |
+------------------+------------------------------------------------------------------+
| ``short``        | ``int``, ``long``, ``float``, or ``double``                      |
+------------------+------------------------------------------------------------------+
| ``int``          | ``long``, ``float``, or ``double``                               |
+------------------+------------------------------------------------------------------+
| ``long``         | ``float`` or ``double``                                          |
+------------------+------------------------------------------------------------------+
| ``float``        | ``double``                                                       |
+------------------+------------------------------------------------------------------+
| ``char``         | ``int``, ``long``, ``float``, or ``double``                      |
+------------------+------------------------------------------------------------------+
| ``enumeration``  | ``int``, ``long``, ``float``, or ``double``                      |
+------------------+------------------------------------------------------------------+

These conversions cause no loss of information about the overall magnitude of
a numeric value. Some least significant bits of the value can be lost only in
conversions from an integer type to a floating-point type if the IEEE 754
*round-to-nearest* mode is used correctly. The resultant floating-point value
is properly rounded to the integer value.

*Widening primitive conversions* never cause runtime errors.

.. index::
   widening conversion
   conversion
   predefined numeric types conversion
   numeric type
   numeric value
   floating-point type
   integer
   conversion
   round-to-nearest mode
   runtime error
   IEEE 754
   enumeration constant
   rounding

|

.. _Constant Narrowing Integer Conversions:

Constant Narrowing Integer Conversions
======================================

.. meta:
    frontend_status: Done

*Constant narrowing integer conversion* converts an expression of integer
types or of type ``char`` to a value of a smaller integer type provided that:

- The expression is a constant expression (see :ref:`Constant Expressions`);
- The value of the expression fits into the range of the smaller type.

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    let b: byte = 127 // ok, int -> byte conversion
    let c: char = 0x42E // ok, int -> char conversion
    b = 128 // compile-time-error, value is out of range
    b = 1.0 // compile-time-error, floating-point value cannot be converted

    function foo (p: byte) {}   // Version #1
    function foo (p: number) {} // Version #2

    foo (100)  // Version #1 is called as int is safely narrowed into byte
    foo (1000) // Version #2 is called as int is safely widened into double/number

These conversions never cause runtime errors.

.. index::
   narrowing
   constant
   integer conversion
   expression
   conversion
   type
   value
   runtime error

|

.. _Widening Union Conversions:

Widening Union Conversions
==========================

.. meta:
    frontend_status: Done

*Widening union conversion* can be of the following three options:

- Conversion from a union type to a wider union type;
- Conversion from a non-union type to a union type;
- Conversion from a union type that consists of literals only to a non-union
  type.

*Widening union conversions* never cause runtime errors.

Union type ``U`` (``U``:sub:`1` ``| ... | U``:sub:`n`) can be converted to a
different union type ``V`` (``V``:sub:`1` ``| ... | V``:sub:`m`) if after
normalization (see :ref:`Union Types Normalization`) the following is true:

..
   lines 724 764  initially was *U*:sub:`1` | ... | *U*:sub:`n` line  725 initially was *V*:sub:`1` | ... | *V*:sub:`m`

  - For every type ``U``:sub:`i` (*i* in 1..n-normalized) there is at least one
    type ``V``:sub:`j` (*i* in 1..m-normalized), when ``U``:sub:`i` is assignable
    to ``V``:sub:`j` (see :ref:`Assignability`).
  - For every value ``U``:sub:`i` there is a value ``V``:sub:`j`, when
    ``U``:sub:`i` == ``V``:sub:`j`.

.. index::
   widening
   union conversion
   conversion
   union type
   non-union type
   runtime error
   normalization
   compatibility
   value

**Note**. If union type normalization issues a single type or value, then
this type or value is used instead of the initial set of union types or values.
This concept is illustrated by the example below:

.. code-block:: typescript
   :linenos:

    let u1: string | number | boolean = true
    let u2: string | number = 42
    u1 = u2 // OK
    u2 = u1 // compile-time error as type of u1 is not compatible with type of u2

    let u3: "1" | "2" | boolean = "3"
       // compile-time error as there is no value "3" among values of u3 type

    class Base {}
    class Derived1 extends Base {}
    class Derived2 extends Base {}

    let u4: Base | Derived1 | Derived2 = new ...
    let u5: Derived1 | Derived2 = new ...
    u4 = u5 // OK, u4 type is Base after normalization and Derived1 and Derived2
       // are compatible with Base as Note states
    u5 = u4 // compile-time error as Base is not compatible with both
       // Derived1 and Derived2

Non-union type ``T`` can be converted to union type ``U`` = ``U``:sub:`1` ``| ... | U``:sub:`n`
if ``T`` is compatible with one of ``U``:sub:`i` types.

.. code-block:: typescript
   :linenos:

    let u: number | string = 1 // ok
    u = "aa" // ok
    u = true // compile-time error

Union type ``U`` (``U``:sub:`1` ``| ... | U``:sub:`n`) can be converted to
non-union type ``T`` if each ``U``:sub:`i` is a literal that can be implicitly
converted to type ``T``.

.. code-block:: typescript
   :linenos:

    let a: "1" | "2" = "1"
    let b: string = a // ok, literals fit type 'string'

.. index::
   normalization
   type
   value
   union type
   compatibility
   conversion
   literal
   non-union type

|

.. _Widening Reference Conversions:

Widening Reference Conversions
==============================

.. meta:
    frontend_status: Done

*Widening reference conversion* handles any subtype (see :ref:`Subtyping`) as
a supertype (see :ref:`Supertyping`). It requires no special action at runtime,
and never causes an error.

.. code-block:: typescript
   :linenos:

    interface BaseInterface {}
    class BaseClass {}
    interface DerivedInterface extends BaseInterface {}
    class DerivedClass extends BaseClass implements BaseInterface
         {}
     function foo (di: DerivedInterface) {
       let bi: BaseInterface = new DerivedClass() /* DerivedClass
           is compatible with BaseInterface */
       bi = di // DerivedInterface is compatible with BaseInterface
    }

The only exception is the cast to type ``never`` that is forbidden. This cast is
a :index:`compile-time error` as it can cause type-safety violations:

.. code-block:: typescript
   :linenos:

    class A { a_method() {} }
    let a = new A
    let n: never = a as never // compile-time error: no object may be assigned
    // to a variable of the never type

    class B { b_method() {} }
    let b: B = n // OK as never is compatible with any type
    b.b_method() /* this breaks type-safety if casting conversion to 'never'
                    is allowed */

|

.. _Character to String Conversions:

Character to String Conversions
===============================

.. meta:
    frontend_status: Done

*Character to string conversion* converts a value of type ``char`` to type
``string``. The length of the resultant new string equals 1. The converted
``char`` is the single element of the new string:

.. code-block:: typescript
   :linenos:

    let c: char = c'X'
    let s: string = c // s contains "X"

This conversion can cause ``OutOfMemoryError`` thrown if the storage available
for the creation of a new string is not sufficient.

.. index::
   character to string conversion
   conversion
   type
   string

|

.. _Constant String to Character Conversions:

Constant String to Character Conversions
========================================

.. meta:
    frontend_status: Done

*Constant string to character conversion* converts an expression of type
``string`` to type ``char``. The initial type ``string`` expression must be a
constant expression (see :ref:`Constant Expressions`). The length of this
expression equals 1. The resultant ``char`` is the first and only character
of the converted ``string``. This conversion never causes runtime errors.

.. index::
   character to string conversion
   expression
   type
   runtime error
   conversion

|

.. _Function Types Conversions:

Function Types Conversions
==========================

.. meta:
    frontend_status: Done

*Function types conversion* is the conversion of one function type to another.
A *function types conversion* is valid if the following conditions are met:

- Parameter types are converted by using *contravariance* (:ref:`Invariance, Covariance and Contravariance`);
- Non-optional parameter types can be converted to the type of an optional
  parameter;
- Return types are converted by using *covariance* (:ref:`Invariance, Covariance and Contravariance`).

.. index::
   function types conversion
   function type
   parameter type
   conversion
   parameter type
   contravariance
   covariance
   return type
   compatible type

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base {}

    type FuncTypeBaseBase = (p: Base) => Base
    type FuncTypeBaseDerived = (p: Base) => Derived
    type FuncTypeDerivedBase = (p: Derived) => Base
    type FuncTypeDerivedDerived = (p: Derived) => Derived

    function (
       bb: FuncTypeBaseBase, bd: FuncTypeBaseDerived,
       db: FuncTypeDerivedBase, dd: FuncTypeDerivedDerived
    ) {
       bb = bd
       /* OK: identical (invariant) parameter types, and compatible return type */
       bb = dd
       /* Compile-time error: compatible parameter type (covariance), type unsafe */
       db = bd
       /* OK: contravariant parameter types, and compatible return type */
    }

    // Examples with lambda expressions
    let foo1: (p: Base) => Base = (p: Base): Derived => new Derived()
     /* OK: identical (invariant) parameter types, and compatible return type */

    let foo2: (p: Base) => Base = (p: Derived): Derived => new Derived()
     /* Compile-time error: compatible parameter type (covariance), type unsafe */

    let foo3: (p: Derived) => Base = (p: Base): Derived => new Derived()
     /* OK: contravariant parameter types, and compatible return type */

    let foo4: (p?: Base) => void = (p: Base): void => {}
     /* OK: Base is compatible with Base|undefined, and identical return type */

    let foo5: (p: Base) => void = (p?: Base): void => {}
     /* Compile-time error: as Base|undefined is not compatible with Base */


A function type with less parameters is compatible with another function type
with more parameters.

.. code-block:: typescript
   :linenos:

    let f: (p: number)=> void = ():void => {} // OK
    f(5)

Worth to mention that overriding is governed by
:ref:`Override-Compatible Signatures` and example below leads to
compile-time error:

.. code-block:: typescript
   :linenos:

    class Base {
        foo(p: (p: number)=> void) {}
    }
    class Derived extends Base {
        override foo(p: ()=> void) {} // Compile-time error
    }

.. index::
   variable
   compile-time error
   assignment

|

.. _Tuple Types Conversions:

Tuple Types Conversions
=======================

.. meta:
    frontend_status: Done

*Tuple types conversion* is the conversion of one tuple type to another.

Tuple type ``T`` = [``T``:sub:`1`, ``T``:sub:`2`, ``...``, ``T``:sub:`n`] can be
converted to tuple type ``U`` = [``U``:sub:`1`, ``U``:sub:`2`, ``...``, ``U``:sub:`m`]
if the following conditions are met:

- Tuple types have the same number of elements, thus n == m.
- Every *T*:sub:`i` is identical to *U*:sub:`i` for any *i* in ``1 .. n``.

.. index::
   tuple type
   conversion

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
   enumeration
   conversion
   constant
   runtime error
   type

A value of ``enumeration`` type is converted to type ``string`` if enumeration
constants of this type are of type ``string``.
This conversion never causes runtime errors.

.. code-block:: typescript
   :linenos:

    enum StringEnum {a = "a", b = "b", c = "c"}
    let string_enum: StringEnum = StringEnum.a
    let a_string: string = string_enum // a_string will get the value of "a"

.. index::
   enumeration
   string
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

Similar conversion of a string type expression is not supported as it is not
part of |TS|.

.. code-block:: typescript
   :linenos:

    enum StringEnum {"a", "b", "c"}
    let incorrect: StringEnum = "b" // compile-time error
    let correct: StringEnum = StringEnum.b // OK


|

.. _Literal Type to its Supertype Conversions:

Literal Type to its Supertype Conversions
=========================================

.. meta:
    frontend_status: Done

A value of ``literal`` type (see :ref:`Literal Types`) can always be converted
to its supertype (see :ref:`Supertypes of Literal Types`). This conversion
never causes a runtime error:

.. code-block:: typescript
   :linenos:

    function foo(d: "string literal") {
       let dd: string = d
    }
    foo("string literal")

The reverse conversion is not possible.

.. index::
   literal type
   supertype conversion
   value
   literal
   conversion
   runtime error

.. raw:: pdf

   PageBreak
