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
   performing an implicit *conversion* can ensure type compatibility.
   The conversion from type ``S`` to type ``T`` causes a type ``S`` expression to
   be handled as a type ``T`` expression at compile time.

.. index::
   context
   conversion
   compile time
   inference
   target type
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

-  :ref:`String Operator Contexts` with ``string`` concatenation (operator ``+``);

-  :ref:`Numeric Operator Contexts` with all numeric operators (``+``, ``-``, etc.);

-  :ref:`Casting Contexts and Conversions`, i.e., the conversion of an
   expression value to a type explicitly specified by a cast expression (see
   :ref:`Cast Expressions`).

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
  array element (see :ref:`Array Type Inference from Context`), a class, or
  an interface field (see :ref:`Object Literal`);

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
type* or can be converted to the *target type* by using one of the conversions
discussed below. Otherwise, a :index:`compile-time error` occurs.

Assignment-like contexts allow using of one of the following:

- :ref:`Widening Primitive Conversions`;

- :ref:`Constant Narrowing Integer Conversions`;

- :ref:`Boxing Conversions`;

- :ref:`Unboxing Conversions`;

- :ref:`Widening Union Conversions`;

- :ref:`Widening Reference Conversions`;

- :ref:`Character to String Conversions`;

- :ref:`Constant String to Character Conversions`;

- :ref:`Function Types Conversions`;

- :ref:`Enumeration to Int Conversions`;

- :ref:`Enumeration to String Conversions`;

- :ref:`Literal Type to Type of Literal Conversions`.


If there is no applicable conversion, then a :index:`compile-time error`
occurs.

|

.. _String Operator Contexts:

String Operator Contexts
************************

.. meta:
    frontend_status: Done

*String context* applies only to a non-*string* operand of the binary operator
``+`` if the other operand is ``string``.

*String conversion* for a non-``string`` operand is evaluated as follows:

-  The operand of a nullish type that has a nullish value is converted as
   described below:

     - The operand ``null`` is converted to string ``null``.
     - The operand ``undefined`` is converted to string ``undefined``.

-  An operand of a reference type or ``enum`` type is converted by applying the
   method call ``toString()``.

-  An operand of an integer type (see :ref:`Integer Types and Operations`)
   is converted to type ``string`` with a value that represents the operand in
   the decimal form;

-  An operand of a floating-point type (see :ref:`Floating-Point Types and Operations`) 
   is converted to type ``string`` with a value that represents the operand in
   the decimal form (without the loss of information);

-  An operand of type ``boolean`` is converted to type ``string`` with the
   values ``true`` or ``false``;

-  An operand of type ``char`` is converted by using :ref:`Character to String Conversions`;

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
   string conversion
   string context
   operand
   direct conversion
   target type
   reference type
   enum type
   string type
   conversion
   method call
   primitive type
   boxing
   predefined numeric types conversion
   numeric types conversion
   target type
   numeric context
   arithmetic operator
   expression

The numeric contexts are actually the forms of the following expressions:

-  Unary (see :ref:`Unary Expressions`);
-  Multiplicative (see :ref:`Multiplicative Expressions`);
-  Additive (see :ref:`Additive Expressions`);
-  Shift (see :ref:`Shift Expressions`);
-  Relational (see :ref:`Relational Expressions`);
-  Equality (see :ref:`Equality Expressions`);
-  Bitwise and Logical (see :ref:`Bitwise and Logical Expressions`);
-  Conditional-And (see :ref:`Conditional-And Expression`);
-  Conditional-Or (see :ref:`Conditional-Or Expression`).

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

*Casting contexts* are applied to cast expressions (:ref:`Cast Expressions`),
and rely on the application of *casting conversions*.

.. index::
   casting context
   cast expression
   casting conversion

*Casting conversion* is the conversion of an operand in a cast expression to
an explicitly specified *target type* by using one of the following:

- Identity conversion, if the *target type* is the same as the expression type;
- :ref:`Implicit Conversions`;
- :ref:`Numeric Casting Conversions`;
- :ref:`Narrowing Reference Casting Conversions`;
- :ref:`Casting Conversions from Union`.

If there is no applicable conversion, then a :index:`compile-time error`
occurs.

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
``float`` is performed in compliance with the IEEE 754 rounding rules.
This conversion can lose precision or range, resulting in the following:

-  Float zero from a nonzero double; and
-  Float infinity from a finite double.

Double ``NaN`` is converted to float ``NaN``.

Double infinity is converted to same-signed floating-point infinity.

A numeric casting conversion of a floating-point type operand to types
``short``, ``byte``, or ``char`` is performed in the following two steps:

- The casting conversion to ``int`` is performed first;
- Then, the ``int`` operand is casted to the target type.

A numeric casting conversion of a floating-point type operand to
target types ``long`` or ``int`` is performed by the following rules:

- If the operand is ``NaN``, then the result is 0 (zero).
- If the operand is positive infinity, or if the operand is too large for the
  target type, then the result is the largest representable value of the target
  type.
- If the operand is negative infinity, or if the operand is too small for
  the target type, then the result is the smallest representable value of
  the target type.
- Otherwise, the result is the value that rounds toward zero by using IEEE 754
  *round-toward-zero* mode.


A numeric casting conversion from an integer type (or char) to a smaller integer
type (or char) ``I`` discards all bits except the *N* lowest ones, where *N* is
the number of bits used to represent type ``I``. This conversion can lose the
information on the magnitude of the numeric value. The sign of the resulting
value can differ from that of the original value.


.. index::
   IEEE 754

|

.. _Narrowing Reference Casting Conversions:

Narrowing Reference Casting Conversions
=======================================

.. meta:
    frontend_status: Done

A *narrowing reference casting conversion* converts an expression of a
supertype (superclass or superinterface) (see :ref:`Supertyping`) to a
subclass or subinterface:

.. index::
   casting conversion
   conversion
   operand
   cast expression
   casting conversion
   class
   interface
   subclass
   subinterface
   variable
   superinterface
   superclass

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base {}

    let b: Base = new Derived()
    let d: Derived = b as Derived

Compile-time errors for this conversion are the same as in
:ref:`InstanceOf Expression`.

A runtime error (``ClassCastError``) occurs during this conversion if the
type of a converted expression cannot be converted to the *target type*:

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived1 extends Base {}
    class Derived2 extends Base {}

    let b: Base = new Derived1()
    let d = b as Derived2 // runtime error

|

.. _Casting Conversions from Union:

Casting Conversions from Union
==============================

A *casting conversion from union* converts an expression of union type to one
of the types of the union, or to a type that is derived from such one type.

For union type ``U = T``:sub:`1` ``| ... | T``:sub:`N`, the *casting conversion
from union* converts an expression of type ``U`` to some type ``TT`` (*target type*).

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
        let frog: Frog = animal as Frog // Use 'as' conversion here
        frog.leap() // Perform an action specific for the particular union type
    }
    if (animal instanceof Spitz) {
        let dog = animal as Spitz // Use 'as' conversion here
        dog.sleep() 
          // Perform an action specific for the particular union type derivative
    }


These conversions can cause a runtime error (``ClassCastError``) if the runtime
type of an expression is not the *target type*.

Another form of *conversion from union* is implicit conversion from union type
to the target type. The conversion is only possible if each type in a union is
compatible (see :ref:`Type Compatibility`) with the target type. If so, the
conversion never causes a runtime error. If at least one type of a union is not
compatible with the target type, then the conversion causes a
:index:`compile-time error`:

.. code-block-meta:
   expect-cte

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived1 extends Base {}
    class Derived2 extends Base {}

    let d: Derived1 | Derived2 = ...
    let b: Base = d // OK, as Derived1 and Derived2 are compatible with Base

    let x: Double | Int = ...
    let y: double = x // OK, as Double and Int can be converted into double 

    let x: Double | Base = ...
    let y: double = x // Compile-time error, as Base cannot be converted into double 

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

A *primitive type conversion* is one of the following:

- :ref:`Widening Primitive Conversions`;

- :ref:`Constant Narrowing Integer Conversions`;

- :ref:`Boxing Conversions`;

- :ref:`Unboxing Conversions`.

|

.. _Widening Primitive Conversions:

Widening Primitive Conversions
==============================

.. meta:
    frontend_status: Done

*Widening primitive conversions* convert the following:

- Values of a smaller numeric type to a larger type (see
  :ref:`Numeric Types Hierarchy`);

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
   predefined numeric types conversion
   numeric type
   numeric value
   floating-point type
   integer
   conversion
   round-to-nearest mode
   runtime error
   IEEE 754

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

|

.. _Boxing Conversions:

Boxing Conversions
==================

.. meta:
    frontend_status: Done

*Boxing conversions* handle primitive type expressions as expressions of a
corresponding reference type.

If the unboxed *target type* is larger than the expression type, then a
*widening primitive conversion* is performed as the first step of a *boxing
conversion* of numeric types and type ``char``.

For example, a *boxing conversion* converts *i* of primitive value type ``int``
into a reference *n* of class type ``Number``:

.. code-block-meta:
   not-subset


.. code-block:: typescript
   :linenos:

    let i: int = 1
    let n: Number = i // int -> number -> Number

    let c: char = 'a'
    let l: Long = c // char -> long  -> Long

These conversions can cause ``OutOfMemoryError`` thrown if the storage
available for the creation of a new instance of the reference type is
not sufficient.

.. index::
   widening conversion
   boxing conversion
   reference type

|

.. _Unboxing Conversions:

Unboxing Conversions
====================

.. meta:
    frontend_status: Done

*Unboxing conversions* handle reference type expressions as expressions of
a corresponding primitive type.

If the *target type* is larger than the unboxed expression type, then a
*widening primitive conversion* is performed as the second step of
the *unboxing conversion* of numeric types and type ``char``.

For example, the *unboxing conversion* converts *i* of reference type ``Int``
into type ``long``:

.. code-block-meta:
   not-subset


.. code-block:: typescript
   :linenos:

    let i: Int = 1
    let l: long = i // Int -> int -> long

*Unboxing conversions* never cause runtime errors.

.. index::
   unboxing conversion
   expression
   primitive type

|

.. _Widening Union Conversions:

Widening Union Conversions
==========================

.. meta:
    frontend_status: Partly
    todo: adapt it after literal types are implemented

There are three options of *widening union conversion* as follows:

- Conversion from a union type to a wider union type;
- Conversion from a non-union type to a union type;
- Conversion from a union type that consists of literals only to a non-union
  type.

These conversions never cause runtime errors.

Union type ``U`` (``U``:sub:`1` ``| ... | U``:sub:`n`) can be converted into a
different union type ``V`` (``V``:sub:`1` ``| ... | V``:sub:`m`) if the following
is true after normalization (see :ref:`Union Types Normalization`):

..
   lines 724 764  initially was *U*:sub:`1` | ... | *U*:sub:`n` line  725 initially was *V*:sub:`1` | ... | *V*:sub:`m`

  - For every type ``U``:sub:`i` (*i* in 1..n-normalized) there is at least one
    type ``V``:sub:`j` (*i* in 1..m-normalized), when ``U``:sub:`i` is compatible
    with ``V``:sub:`j` (see :ref:`Type Compatibility`).
  - For every value ``U``:sub:`i` there is a value ``V``:sub:`j`, when
    ``U``:sub:`i` == ``V``:sub:`j`.

**Note**: If union type normalization issues a single type or value, then
this type or value is used instead of the initial set of union types or values.

This concept is illustrated by the example below:

.. code-block:: typescript
   :linenos:

    let u1: string | number | boolean = true 
    let u2: string | number = 666
    u1 = u2 // OK 
    u2 = u1 // compile-time error as type of u1 is not compatible with type of u2

    let u3: 1 | 2 | boolean = 3 
       // compile-time error as there is no value 3 among values of u3 type

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

Union type ``U`` (``U``:sub:`1` ``| ... | U``:sub:`n`) can be converted into
non-union type ``T`` if each ``U``:sub:`i` is a literal that can be implicitly
converted to type ``T``.

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    let a: 1 | 2 = 1
    let b: int = a // ok, literals fit type 'int'
    let c: number = a // ok, literals fit type 'number'
    
    let d: 3 | 3.14 = 3
    let e: number = d // ok
    let f: int = d // compile-time error, 3.14 cannot be converted to 'int'

|

.. _Widening Reference Conversions:

Widening Reference Conversions
==============================

.. meta:
    frontend_status: Done

A *widening reference conversion* handles any subtype (see :ref:`Subtyping`) as
a supertype (see :ref:`Supertyping`). It requires no special action at runtime,
and never causes an error.

.. index::
   widening reference conversion
   subtype
   supertype
   runtime

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
    b.b_method() // this breaks type-safety if 'as' cast to never is allowed  

The conversion of array types (see :ref:`Array Types`) also works in accordance
with the widening style of the type of array elements as shown below:

.. index::
   conversion
   array type
   widening

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base {}
    function foo (da: Derived[]) {
      let ba: Base[] = da /* Derived[] is assigned into Base[] */
    }

This array assignment can cause ``ArrayStoreError`` at runtime if an object
of incorrect type is included in the array. The runtime system performs
runtime checks to ensure type-safety as show below:

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base {}
    class AnotherDerived extends Base {}
    function foo (da: Derived[]) {
      let ba: Base[] = da // Derived[] is assigned into Base[]
      ba[0] = new AnotherDerived() /* This assignment of array element will
         cause *ArrayStoreError* during program execution */
    }


.. index::
   array assignment
   array type
   widening
   type-safety

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

|

.. _Constant String to Character Conversions:

Constant String to Character Conversions
========================================

.. meta:
    frontend_status: Done

*Constant string to character conversion* converts an expression of type
``string`` to type ``char``. The initial type ``string`` expression must be a
constant expression (see :ref:`Constant Expressions`). The length of this
expression equals 1.

The resultant ``char`` is the first and only character of the converted
``string``.

This conversion never causes runtime errors.

|

.. _Function Types Conversions:

Function Types Conversions
==========================

.. meta:
    frontend_status: Done

*Function types conversion* is the conversion of one function type to another.
A *function types conversion* is valid if the following conditions are met:

- Parameter types are converted by using *contravariance* (:ref:`Contravariance`);
- Non-optional parameter types can be converted to the type of an optional
  parameter;
- Return types are converted by using *covariance* (:ref:`Covariance`).

See :ref:`Type Compatibility` for details.

.. index::
   function types conversion
   function type
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
       /* Compile-time error: compatible parameter type(covariance), type unsafe */
       db = bd
       /* OK: contravariant parameter types, and compatible return type */
    }

    // Examples with lambda expressions
    let foo1: (p: Base) => Base = (p: Base): Derived => new Derived() 
     /* OK: identical (invariant) parameter types, and compatible return type */

    let foo2: (p: Base) => Base = (p: Derived): Derived => new Derived() 
     /* Compile-time error: compatible parameter type(covariance), type unsafe */

    let foo3: (p: Derived) => Base = (p: Base): Derived => new Derived() 
     /* OK: contravariant parameter types, and compatible return type */

    let foo4: (p?: Base) => void = (p: Base): void => {}
     /* OK: Base is compatible with Base|undefined, and identical return type */

    let foo5: (p: Base) => void = (p?: Base): void => {}
     /* Compile-time error: as Base|undefined is not compatible with Base */


A *throwing function* type variable can have a *non-throwing function* value.

.. code-block:: typescript
   :linenos:

    let foo: () => void throws = (): void => {} // OK


A :index:`compile-time error` occurs if a *throwing function* value is assigned
to a *non-throwing function* type variable.


.. code-block:: typescript
   :linenos:

    let foo: () => void = (): void throws => {} // Compile-time error


.. index::
   throwing function
   variable
   non-throwing function
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
converted into tuple type ``U`` = [``U``:sub:`1`, ``U``:sub:`2`, ``...``, ``U``:sub:`m`]
if the following conditions are met:

- Tuple types have the same number of elements, thus n == m.
- Every *T*:sub:`i` is identical to *U*:sub:`i` for any *i* in ``1 .. n``.

|

.. _Enumeration to Int Conversions:

Enumeration to Int Conversions
==============================

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

|

.. _Enumeration to String Conversions:

Enumeration to String Conversions
=================================

.. meta:
    frontend_status: Done

A value of ``enumeration`` type is converted to type ``string`` if enumeration
constants of this type are of type ``string``.

This conversion never causes runtime errors.

.. code-block:: typescript
   :linenos:

    enum StringEnum {a = "a", b = "b", c = "c"}
    let string_enum: StringEnum = StringEnum.a
    let a_string: string = string_enum // a_string will get the value of "a"

|

.. _Literal Type to Type of Literal Conversions:

Literal Type to Type of Literal Conversions
===========================================

.. meta:
    frontend_status: Partly

A value of ``literal`` type can always be converted to the type of its literal
as it is its supertype (see :ref:`Supertyping`). This conversion never causes
a runtime error:

.. code-block:: typescript
   :linenos:

    function foo (a: 1, b: true, c: 3.14, d: "string literal", e: c'C', f: 123n) {
       let aa: int = a
       let bb: boolean = b
       let cc: double = c
       let dd: string = d
       let ee: char = e
       let ff: bigint = f
    }
    foo (1, true, 3.14, "string literal", c'C', 123n)


The reverse conversion is not possbile.

|


.. raw:: pdf

   PageBreak
