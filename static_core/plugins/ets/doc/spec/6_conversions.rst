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
is inferred at compile time. 

In most contexts, an expression must be *compatible* with a type expected in
that context. This type is called the *target type*. 

If no target type is expected in the context, the expression 
is called a *standalone expression*,
otherwise the expression is a *non-standalone expression*.

Examples of non-standalone expressions:

.. code-block:: typescript
   :linenos:

    let a: number = expr // target type of 'expr' is number
    
    function foo(s: string) {}
    foo(expr) // target type of 'expr' is string


Examples of standalone expressions:

.. code-block:: typescript
   :linenos:

    let a = expr // no target type is expected
    
    function foo(): void {
        expr // no target type is expected
    }

For some expressions its type cannot be inferred from the expression itself, 
see :ref:`Object Literal` for example. 
A compile-time error occurs if such expression is used as a *standalone expression*.

For *non-standalone expressions*, there are two ways to facilitate the compatibility
of the expression with its surrounding context:

#. The type of some non-standalone expressions can be inferred from the
   target type (the expression types can be different in different
   contexts).

#. If the inferred expression type is different from the target type, then
   performing an implicit *conversion* can ensure type compatibility.
   The conversion from type *S* to type *T* causes a type *S* expression to
   be handled as a type *T* expression at compile time.

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

A compile-time error occurs if neither produces an appropriate expression type.

The rules that determine whether a *target type* allows an implicit
*conversion* vary for different *kinds of contexts* 
and forms of non-standalone expressions,.
The *target type* can influence not only the type of the expression but, in
some cases, also its runtime behavior.

.. index::
   runtime behavior
   conversion

Kinds of contexts are:

-  :ref:`Assignment-like Contexts`: where an expression value is bound to a variable.

-  :ref:`String Operator Contexts`: ``string`` concatenation (`+` operator).

-  :ref:`Numeric Operator Contexts`: all numeric operators (`+`, `-`, and so on).

-  :ref:`Casting Contexts and Conversions`: conversion of an expression value to a type
   explicitly specified by a cast expression (see :ref:`Cast Expressions`).

.. _Assignment-like Contexts:

Assignment-like Contexts
************************

.. meta:
    frontend_status: Partly

*Assignment-like contexts* include:

- *Declaration contexts* allow setting an initial value 
  to a variable (see :ref:`Variable Declarations`) 
  or a constant (see :ref:`Constant Declarations`)
  or a field (see :ref:`Field Declarations`) 
  with explicit type annotation;

- *Assignment contexts* allow assigning (see :ref:`Assignment`) an 
  expression value to a variable;
  
- *Call contexts* allow assigning an argument value to 
  a corresponding formal parameter
  of a function, method, constructor or lamdda call
  (see :ref:`Function Call Expression`, :ref:`Method Call Expression`, 
  :ref:`Explicit Constructor Call`, :ref:`New Expressions`);
  
- *Composite literal contexts* allow setting an expression value 
  to an array element (see :ref:`Array Type Inference from Context`) or 
  to a class or interface field (see :ref:`Object Literal`);
  
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

Examples are presented below:

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

      // call contexts:
      function foo(s: string) {}
      foo("hello")    

      // composite literal contexts:
      let a: number[] = [str.length, 11]


In all these cases, either a type of the expression must be equal to
the *target type*, or it can be converted to the *target type* using one
of the conversions listed below. Othersize a compile-time error occurs.

Assignment-like contexts allow the use of one of the following:

- :ref:`Widening Primitive Conversions`

- :ref:`Constant Narrowing Integer Conversions`

- :ref:`Boxing Conversions`

- :ref:`Unboxing Conversions`

- :ref:`Widening Union Conversions`

- :ref:`Widening Reference Conversions`
 
- :ref:`Character to String Conversions`

- :ref:`Constant String to Character Conversions`

- :ref:`Function Types Conversions`

A compile-time error occurs if there is no applicable conversion.

.. _String Operator Contexts:

String Operator Contexts
************************

.. meta:
    frontend_status: Done

*String context* applies only to a non-*string* operand of the binary ``+``
operator if the other operand is a *string*. 

*String conversion* for a non-*string* operand is evaluated as follows:

-  The operand of nullish type that has a nullish value is converted as
   described below:

     - The operand ``null`` is converted to string ``"null"``.
     - The operand ``undefined`` is converted to string ``"undefined"``

-  An operand of a reference type or of an enum type is converted by applying the *toString()*
   method call.

-  An operand of an integer type (see :ref:`Integer Types and Operations`) 
   is converted to *string*
   with a value representing the operand in the decimal form,

-  An operand of a floating-point type (see :ref:`Floating-Point Types and Operations`) 
   is converted to *string* with a value
   representing the operand in the decimal form (without the loss of information),

-  An operand of the *boolean* type is converted to *string* ``"true"`` or ``"false"``,

-  An operand of the *char* type is converted using :ref:`Character to String Conversions`.

A compile-time error occurs if there is no applicable conversion.

The target type of this context is always *string*.

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


*Numeric contexts* apply to the operands of an arithmetic operator.
*Numeric contexts* use combinations of predefined numeric types conversions
(see :ref:`Primitive Types Conversions`), and ensure that each
argument expression can convert to target type *T* while the arithmetic
operation for the values of type *T* is being defined.

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

-  Unary (see :ref:`Unary Expressions`),
-  Multiplicative (see :ref:`Multiplicative Expressions`),
-  Additive (see :ref:`Additive Expressions`),
-  Shift (see :ref:`Shift Expressions`),
-  Relational (see :ref:`Relational Expressions`),
-  Equality (see :ref:`Equality Expressions`),
-  Bitwise and Logical (see :ref:`Bitwise and Logical Expressions`),
-  Conditional-And (see :ref:`Conditional-And Expression`),
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

The *casting conversion* is the conversion of an operand of a cast
expression to an explicitly specified *target type* by using one of:

- identity conversion: *target type* is the same as the expression type
- any of :ref:`Implicit Conversions`
- :ref:`Numeric Casting Conversions`
- :ref:`Narrowing Reference Casting Conversions`
- :ref:`Casting Conversions from Union`

A compile-time error occurs if there is no applicable conversion.

The general scheme of the *casting conversion* is described as following.
  If there is a cast expression (see :ref:`Cast Expressions`) in the form of
  *expression 'as' T*:sub:`2` then the following steps are undertaken: 

    - let *T*:sub:`1` be the type of expression;
    - if type *T*:sub:`1` is compatible (see :ref:`Type Compatibility`) with
      the type *T*:sub:`2` then *expression 'as' T*:sub:`2` can be replaced
      with just an *expression* itself;
    - else if the type *T*:sub:`2` is compatible with the type *T*:sub:`1` then
      the cast is to performed during the program execution;
    - else it is a compile-time error as types *T*:sub:`1` and *T*:sub:`2` has
      no overlap between their values and thus cannot be converted.

.. index::
   casting conversion
   numeric type


.. _Numeric Casting Conversions:

Numeric Casting Conversions
===========================

A *numeric casting conversion* is valid if both *target type* and 
the type of expression are of a numeric type or *char* type.

Example:

.. code-block:: typescript
   :linenos:

    function process_int(an_int: int) { ... }

    let pi = 3.14
    process_int(pi as int)

These conversions never cause runtime errors.

|   

.. _Narrowing Reference Casting Conversions:

Narrowing Reference Casting Conversions
=======================================

A *narrowing reference casting conversion* converts an expression of a supertype 
(superclass or superinterface) to a subclass or subinterface:

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

These conversion can cause runtime error (TBD: name it), if a type 
of converted expression cannot be converted to the *target type*.

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived1 extends Base {}
    class Derived2 extends Base {}

    let b: Base = new Derived1()
    let d = b as Derived2 // runtime error

.. _Casting Conversions from Union:

Casting Conversions from Union
==============================

A *casting conversion from union* converts an expression of a union type to a
type which is one of union type types or its derived types.

For union type *U* = *T*:sub:`1` | ... | *T*:sub:`N`, the *casting conversion from union* 
converts an expression of the *U* type to some type *TT* (*target type*).

A compile-time error occurs if the target type *TT* is not one of *T*:sub:`i` or
derived from *T*:sub:`i`.

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
        frog.leap() // Perform an action specific for the particluar union type
    }
    if (animal instanceof Spitz) {
        let dog = animal as Spitz // Use 'as' conversion here
        dog.sleep() 
          // Perform an action specific for the particluar union type derivative
    }


These conversions can cause runtime error (TBD: name it), if a type the
expression at runtime type is not of the *target type*.

|

.. _Implicit Conversions:

Implicit Conversions
********************

.. meta:
   frontend_status: Done
   todo: Narrowing Reference Conversion - note: Only basic checking available, not full support of validation
   todo: String Conversion - note: Implemented in a different but compatible way: spec - toString(), implementation: StringBuilder
   todo: Forbidden Conversion - note: Not exhaustively tested, should work

This section describes all allowed implicit conversions. Each conversion is allowed in 
particular contexts (for example, saying that an expression that initializes
a local variable is subject to :ref:`Assignment-like Contexts` means that the rules
for this context define what specific conversion is implicitly
chosen for that expression).

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

A *primitive type conversion* is one of the following:

- :ref:`Widening Primitive Conversions`

- :ref:`Constant Narrowing Integer Conversions`

- :ref:`Boxing Conversions`

- :ref:`Unboxing Conversions`

|

.. _Widening Primitive Conversions:

Widening Primitive Conversions
==============================

.. meta:
    frontend_status: Partly

*Widening primitive conversions* converts

- values of smaller numeric type to larger type (see :ref:`Numeric Types Hierarchy`);

- values of *byte* type to *char* type (see :ref:`Character Type and Operations`);

- values of *char* type to *int*, *long*, *float* and *double* types.

+----------+------------------------------+
| From     | To                           |
+==========+==============================+
| *byte*   | *short*, *int*, *long*,      |
|          | *float*, *double*, or *char* |
+----------+------------------------------+
| *short*  | *int*, *long*, *float*, or   |
|          | *double*                     |
+----------+------------------------------+
| *int*    | *long*, *float*, or *double* |
+----------+------------------------------+
| *long*   | *float* or *double*          |
+----------+------------------------------+
| *float*  | *double*                     |
+----------+------------------------------+
| *char*   | *int*, *long*, *float*,      |
|          | or *double*                  |
+----------+------------------------------+

These conversions cause no loss of information
about the overall magnitude of a numeric value. Some least significant bits of
the value can be lost only in conversions from integer to floating-point types
if the IEEE 754 '*round-to-nearest*' mode is used correctly, and the resultant
floating-point value is properly rounded to the integer value.

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

|

.. _Constant Narrowing Integer Conversions:

Constant Narrowing Integer Conversions
======================================

*Constant narrowing integer conversion* converts an expression of integer type
to a value of smaller integer type provided that

- the expression is a constant expression (see :ref:`Constant Expressions`)
- a value of expressions fits to the range of smaller type

.. code-block:: typescript
   :linenos:

    let b: byte = 127 // ok, int -> byte conversion
    b = 128 // compile-time-error, value is out of range
    b = 1.0 // compile-time-error, floating-point value cannot be converted

These conversions never cause runtime errors.

.. _Boxing Conversions:

Boxing Conversions
==================

.. meta:
    frontend_status: Partly

*Boxing conversions* handle primitive type expressions as expressions of a
corresponding reference type.

A *boxing conversion* for numeric types and *char* type can contain preceding
*widening primitive conversion*, 
if the unboxed *target type* is larger then the expression type.  

For example, a *boxing conversion* converts *i* of primitive value type *int*
into a reference *n* of class type *Number*:

.. code-block:: typescript
   :linenos:

    let i: int = 1
    let n: Number = i // int -> number -> Number

    let c: char = 'a'
    let l: Long = c // char -> long    

These conversions can cause an *OutOfMemoryError* thrown if the storage
available for the creation of a new instance of the reference type is
insufficient.

.. index::
   widening conversion
   boxing conversion
   reference type

.. _Unboxing Conversions:

Unboxing Conversions
====================

.. meta:
    frontend_status: Partly

*Unboxing conversions* handle reference type expressions as expressions of
a corresponding primitive type. 

A *unboxing conversion* for numeric types and *char* type can contain following
*widening primitive conversion*, 
if the *target type* is larger then the unboxed expression type. 


For example, an *unboxing conversion* converts
*r* of class type *T* into a primitive value *p* of type *t*, accoring to 
column Unboxing in the table below. No errors may occur during such conversion.

For example, a *unboxing conversion* converts *i* of reference type *Int*
into *long* type:

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
    frontend_status: None
    
There are three variants of *widening union conversions*:

- conversion from an union type to a wider union type
- conversion from non-union type to a union type
- conversion from a union type that 
  consists of literals only to non-union type

These conversions never cause runtime errors.

Union type *U* (*U*:sub:`1` | ... | *U*:sub:`n`) can be converted into a
different union type *V* (*V*:sub:`1` | ... | *V*:sub:`m`) if after
normalization (see :ref:`Union Types Normalization`) the following holds:

  - for every type *U*:sub:`i` (i in 1..n-normalized) there is at least one
    type *V*:sub:`j` (i in 1..m-normalized) when *U*:sub:`i` is compatible
    (see :ref:`Type Compatibility`) with *V*:sub:`j`;
  - for every value *U*:sub:`i` there is a value *V*:sub:`j` when
    *U*:sub:`i` == *V*:sub:`j`

Note: if union type normalization will issue a single type or value then
this type or value is used instead of the set of initial union types or values.

Example below illustrates the concept:

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

Non-union type *T* can be converted
to union type *U* = *U*:sub:`1` | ... | *U*:sub:`n` 
if *T* is compatible with one of *U*:sub:`i` type.

.. code-block:: typescript
   :linenos:

    let u: number | string = 1 // ok 
    u = "aa" // ok
    u = true // compile-time error

Union type *U* (*U*:sub:`1` | ... | *U*:sub:`n`) can be converted into 
non-union type *T* if each *U*:sub:`i` is a literal that 
can be implicitly converted to type *T*. 

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
    frontend_status: Partly

A *widening reference conversion* handles any subtype as supertype.
It requires no special action at runtime, and therefore never causes an error.

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
           is a subtype of BaseInterface */
       bi = di /* DerivedInterface is a subtype of BaseInterface
           */
    }

The only exception is cast to type *never* that it is forbidden. This cast is
a compile-time error as it can lead to type-safety violations.

.. code-block:: typescript
   :linenos:

    class A { a_method() {} }
    let a = new A
    let n: never = a as never // compile-time error: no object may be assigned
    // to a variable of the never type

    class B { b_method() {} }
    let b: B = n // OK as never is a subtype of any type
    b.b_method() // this breaks type-safety if as cast to never is allowed  


The conversion of array types (see :ref:`Array Types`) also works in accordance
with the widening style of array elements type. It is illustrated in the example
below:

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

Such an array assignment can lead to a runtime error (*ArrayStoreError*)
if an object of incorrect type is put into the array. The runtime
system performs run-time checks to ensure type-safety. It is illustrated
in the example below:

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base {}
    class AnotherDerived extends Base {}
    function foo (da: Derived[]) {
      let ba: Base[] = da // Derived[] is assigned into Base[]
      ba[0] = new AnotherDerived() // This assignment of array
          element will cause  *ArrayStoreError*
    }


.. index::
   array assignment
   array type
   widening
   type-safety

.. _Character to String Conversions:

Character to String Conversions
===============================

*Character to string conversion* converts a value of *char* type to a string.
The resultant string is a new
string with has the length to 1 and 
contains the converted char as its first and only element.

.. code-block:: typescript
   :linenos:

    let c: char = c'X' 
    let s: string = c // s contains "X"

This conversion can cause an *OutOfMemoryError* thrown if the storage
available for the creation of a new string is
insufficient.
 

.. _Constant String to Character Conversions:

Constant String to Character Conversions
========================================

*Constant string to character conversion* converts a *string* type expression
which must be

- constant expression (see :ref:`Constant Expressions`)
- and have a length equal to 1

to *char* type.

The resultant char is the first character of the converted string.

The conversion never cause runtime errors.

|

.. _Function Types Conversions:

Function Types Conversions
==========================

.. meta:
    frontend_status: Partly

A *function types conversion*, i.e., the conversion of one function type
to another, is valid if the following conditions are met:

- Parameter types are converted using contravariance.
- Return types are converted using covariance (see :ref:`Type Compatibility`).

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
       db: FuncTypeDerivedBase, dd: FuncTypeDerivedDerived\
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

    let foo2: (p: Derived) => Base = (p: Base): Derived => new Derived() 
     /* OK: contravariant parameter types, and compatible return type */

A *throwing function* type variable can have a *non-throwing function* value.

A compile-time error occurs if a *throwing function* value is assigned to a
*non-throwing function* type variable.

.. index::
   throwing function
   variable
   non-throwing function
   compile-time error
   assignment

|

.. raw:: pdf

   PageBreak


