..  Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
is inferred at compile time. The *target type* of an expression is the type
*compatible* with the types expected in most contexts that expression
appears in.

There are two ways to improve convenience by facilitating the expression's
compatibility with its surrounding context:

#. The type of some non-standalone expressions can be inferred from the
   target type (the expression types can be different in different
   contexts).

#. If the inferred expression type is different from the target type, then
   performing an implicit *conversion* can ensure type compatibility.

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

The form of an expression, and the kind of its context indicate what rules
apply to determine whether that expression is non-standalone, and what the
type and compatibility of that expression are in a particular context. The
*target type* can influence not only the type of the  expression but, in
some cases, also its runtime behavior in order to produce an appropriate
type of a value.

The rules that determine whether a *target type* allows an implicit
conversion vary for different kinds of contexts and types of expressions,
and, in one particular case, of the constant expression value (see
:ref:`Constant Expressions`).

The conversion from type *S* to type *T* causes a type *S* expression to
be treated as a type *T* expression at compile time.

Some cases of conversion can require a runtime action to check the
conversion validity, or to translate the runtime expression value
into a form that is appropriate for the new type *T*.

.. index::
   compile-time error
   expression
   context
   compatibility
   target type
   runtime behavior
   expression type
   compile time
   conversion

|

.. _Kinds of Contexts:

Kinds of Contexts
*****************

.. meta:
    frontend_status: Partly

Contexts can influence non-standalone expressions, and implicit conversions
are possible in various kinds of *conversion contexts*. The following kinds
of contexts have different rules for non-standalone expression typing, and
allow conversions in some, but not all expression types:

-  Assignment and call contexts: expression type must be compatible with
   the type of the target.

.. index::
   context
   non-standalone expression
   implicit conversion
   conversion context
   compatible type
   expression typing

.. code-block:: typescript
   :linenos:

      let variable: TargetType = expression /* Type of
          expression -> type of variable */
      foo (expression1, expression2) /* Type of expression ->
          type of function parameter */

-  Operator contexts: ``string.+`` (concatenation), and all primitive numeric
   type operators (`+`, `-`, and so on).

.. code-block:: typescript
   :linenos:

      let v1 = "a string" + 5 /* string, otherType pair */
      let v2 = 5 * 3.1415 /* two primary numeric types */
      let v3 = 5 - (new Number(6) as number) /* two types */

-  Explicit casting contexts: conversion of an expression value to a type
   explicitly specified by a cast expression (see :ref:`Cast Expressions`).

.. index::
   operator context
   concatenation
   primitive numeric type
   numeric type operator
   explicit casting context
   conversion
   cast expression

.. code-block:: typescript
   :linenos:

      let v1 = "a string" as string
      let v2 = 5 as number

|

.. _Assignment and Call Contexts:

Assignment and Call Contexts
============================

.. meta:
    frontend_status: Done

*Assignment contexts* allow assigning (see :ref:`Assignment`) a valid
expression value to a named variable, while the type of the expression
must be converted to the type of the  variable. It implies that these
types must be compatible, and the exact definition of the semantics
of types compatibility is given in :ref:`Compatible Types`.

*Call contexts* reuse the rules of *assignment contexts*, and allow
assigning an argument value of a method, constructor, or function call (see
:ref:`Explicit Constructor Call`, :ref:`New Expressions` and
:ref:`Method Call Expression`) to a corresponding formal parameter.

.. index::
   assignment
   assignment context
   call context
   expression
   variable
   argument
   type compatibility
   compatible type
   conversion
   explicit call
   constructor call
   method call
   formal parameter

|

.. _Compatible Types:

Compatible Types
----------------

.. meta:
    frontend_status: Done

Type *T1* is compatible with type *T2* if one of the following conversions
can be successfully applied to type *T1* to receive type *T2* as a result:

-  Identity conversion (see :ref:`Kinds of Conversion`);
-  Predefined numeric types conversions (see :ref:`Predefined Numeric Types Conversions`);
-  Reference types conversions (see :ref:`Reference Types Conversions`);
-  Function types conversions (see :ref:`Function Types Conversions`);
-  Enumeration types conversions -- experimental feature (see :ref:`Enumeration Types Conversions`);
-  Raw types conversions (see :ref:`Raw Type Conversions`).

.. index::
   compatible type
   conversion
   predefined numeric types conversion
   reference types conversion
   identity conversion
   function types conversion
   enumeration types conversion
   raw types conversion   

|

.. _Operator Contexts:

Operator Contexts
=================

.. meta:
    frontend_status: Done

*String context* applies only to a non-*string* operand of the binary ``+``
operator if the other operand is a *string*. For example:

.. code-block:: typescript
   :linenos:

    Operator     :  expresssion1 + expression2
    Operand types:    string     +  any_type
    Operand types:  any_type     +  string

effectively transforms into the following:

.. code-block:: typescript
   :linenos:

    Operator     :  expresssion1 + expression2
    Operand types:    string     +  any_type.toString()
    Operand types:  any_type.toString()  +  string

.. _string-conversion:

*String conversion* can be of the following kinds:

-  Any reference type, or enum type can convert directly to the *string* type,
   which is then performed as the *toString()* method call.

-  Any primitive type must convert to a reference value (for boxing see
   :ref:`Predefined Numeric Types Conversions`) before the method call
   *toString()* is performed.

These contexts always have *string* as the target type.

*Numeric contexts* apply to the operands of an arithmetic operator.
*Numeric contexts* use combinations of predefined numeric types conversions
(see :ref:`Predefined Numeric Types Conversions`), and ensure that each
argument expression can convert to the target type *T* while the arithmetic
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

.. _Casting Contexts:

Casting Contexts
================

.. meta:
    frontend_status: Done
    todo: Does not work for interfaces, eg. let x:iface1 = iface_2_inst as iface1; let x:iface1 = iface1_inst as iface1

*Casting contexts* are applied to cast expressions (:ref:`Cast Expressions`),
and rely on the application of *casting conversions* (:ref:`Casting Conversions`).

.. index::
   casting context
   cast expression
   casting conversion

|

.. _Kinds of Conversion:

Kinds of Conversion
*******************

.. meta:
   frontend_status: Done
   todo: Narrowing Reference Conversion - note: Only basic checking availiable, not full support of validation
   todo: Unchecked Conversion - note: Generics raw types not implemented yet
   todo: String Conversion - note: Inmplemented in a different but compatible way: spec - toString(), implementation: StringBuilder
   todo: Forbidden Conversion - note: Not exhaustively tested, should work

The term ‘conversion’ also describes any conversion that is allowed in a
particular context (for example, saying that an expression that initializes
a local variable is subject to ‘assignment conversion’ means that the rules
for the assignment context define what specific conversion is implicitly
chosen for that expression).

The conversions allowed in |LANG| are broadly grouped into the following
categories:

.. index::
   conversion
   context
   expression
   initialization
   assignment
   assignment conversion
   assignment context

.. _identity-conversion:

-  Identity conversions: the type *T* is always compatible with itself.
-  Predefined numeric types conversions: all combinations allowed between
   numeric types.
-  Reference types conversions.
-  String conversions (see :ref:`Operator Contexts`).
-  Raw Types Conversion.

Any other conversions are forbidden.

.. index::
   identity conversion
   compatible type
   predefined numeric types conversion
   numeric type
   reference type conversion
   string conversion
   raw types conversion
   conversion

|

.. _Predefined Numeric Types Conversions:

Predefined Numeric Types Conversions
====================================

.. meta:
    frontend_status: Partly

*Widening conversions* cause no loss of information about the overall magnitude
of a numeric value (except conversions from integer to floating-point types
that can lose some least significant bits of the value if the IEEE 754
'*round-to-nearest*' mode is used correctly, and the resultant floating-point
value is properly rounded to the integer value). Widening conversions never
cause runtime errors.

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

+----------+-----------------------------+
| From     | To                          |
+==========+=============================+
| *byte*   | *short*, *int*, *long*,     |
|          | *float* or *double*         |
+----------+-----------------------------+
| *short*  | *int*, *long*, *float*      |
|          | or *double*                 |
+----------+-----------------------------+
| *char*   | *int*, *long*, *float*      |
|          | or *double*                 |
+----------+-----------------------------+
| *int*    | *long*, *float* or *double* |
+----------+-----------------------------+
| *long*   | *float* or *double*         |
+----------+-----------------------------+
| *float*  | *double*                    |
+----------+-----------------------------+
| *bigint* | *BigInt*                    |
+----------+-----------------------------+

*Narrowing conversions* (performed in compliance with IEEE 754 like in
other programming languages) can lose information about the overall
magnitude of a numeric value, potentially resulting in the loss of precision
and range. Narrowing conversions never cause runtime errors.

.. index::
   narrowing conversion
   numeric value
   runtime error

+-----------+-----------------------------+
| From      | To                          |
+===========+=============================+
| *short*   | *byte* or *char*            |
+-----------+-----------------------------+
| *char*    | *byte* or *short*           |
+-----------+-----------------------------+
| *int*     | *byte*, *short* or *char*   |
+-----------+-----------------------------+
| *long*    | *byte*, *short*, *char* or  |
|           | *int*                       |
+-----------+-----------------------------+
| *float*   | *byte*, *short*, *char*,    |
|           | *int* or *long*             |
+-----------+-----------------------------+
| *double*  | *byte*, *short*, *char*,    |
|           | *int*, *long* or *float*    |
+-----------+-----------------------------+

*Widening and narrowing* conversion is converting *byte* to an *int*
(widening), and the resultant *int* to a *char* (narrowing).

-  *byte* -> *char*.

*Boxing and unboxing* conversions allow converting a reference into a value,
and vice versa, for variables of predefined types.

*Boxing conversions* handle primitive type expressions as expressions of a
corresponding reference type.

.. index::
   widening conversion
   narrowing conversion
   conversion
   boxing conversion
   unboxing conversion
   predefined type
   primitive type
   expression
   reference type

For example, a *boxing conversion* converts *p* of value type *t* into
a reference *r* of class type *T*, i.e., *r.unboxed()* == *p*.

This conversion can result in an *OutOfMemoryError* thrown if the storage
available for the creation of a new instance of the wrapper class *T* is
insufficient.

*Unboxing conversions* handle reference type expressions as expressions of
a corresponding primitive type. The semantics of an unboxing conversion,
and that of the corresponding reference type’s *unboxed() function call* is
the same.

.. index::
   boxing conversion
   conversion
   wrapping
   unboxing conversion
   expression
   primitive type
   unboxed function call

The table below illustrates both conversions:

+--------------------+--------------------+
| Boxing             | Unboxing           |
+====================+====================+
|*byte* -> *Byte*    |*Byte* -> *byte*    |
+--------------------+--------------------+
|*short* -> *Short*  |*Short* -> *short*  |
+--------------------+--------------------+
|*char* -> *Char*    |*Char* -> *char*    |
+--------------------+--------------------+
|*int* -> *Int*      |*Int* -> *int*      |
+--------------------+--------------------+
|*long* -> *Long*    |*Long* -> *long*    |
+--------------------+--------------------+
|*float* -> *Float*  |*Float* -> *float*  |
+--------------------+--------------------+
|*double* -> *Double*|*Double* -> *double*|
+--------------------+--------------------+

|

.. _Reference Types Conversions:

Reference Types Conversions
===========================

.. meta:
    frontend_status: Partly

A *widening reference conversion* from any subtype to supertype requires
no special action at runtime, and therefore never causes an error.

.. index::
   widening reference conversion
   reference type conversion
   reference type
   subtype
   supertype
   runtime
   reference types conversion

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

The conversion of array types (see :ref:`Array Types`) also works in accordance
with the widening style of array elements type.

See the example below for the illustration of it:

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
system performs run-time checks to ensure type-safety.

See the example below for the illustration of it:

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

|

.. _Generic Types Conversions:

Generic Types Conversions
=========================

.. meta:
    frontend_status: Partly

The conversion of generic types (see :ref:`Generic Declarations`) follows the
widening style of type arguments.

See the example below for the illustration of it:

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base {}
    class Generic <T> {}

    function foo (d: Generic<Derived>) {
      let b: Generic<Base> = d 
         /* Generic<Derived> is assigned into Generic<Base> */
    }

.. index::
   conversion
   generic types conversion
   generic type
   widening
   argument
   conversion

|

.. _Function Types Conversions:

Function Types Conversions
==========================

.. meta:
    frontend_status: Partly

A *function types conversion*, i.e., the conversion of one function type
to another occurs if the following conditions are met:

- Parameter types are converted using contravariance;
- Return types are converted using covariance (see :ref:`Compatible Types`).

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

A throwing function type variable can have a non-throwing function value.

A compile-time error occurs if a throwing function value is assigned to a
non-throwing function type variable.

.. index::
   throwing function
   variable
   non-throwing function
   compile-time error
   assignment

|

.. _Casting Conversions:

Casting Conversions
===================

.. meta:
    frontend_status: Done

The *casting conversion* is the conversion of an operand of a cast
expression (:ref:`Cast Expressions`) to an explicitly specified type by using
any kind of conversion (:ref:`Kinds of Conversion`), or a combination of such
conversions.

The *casting conversion* for class and interface types allows getting
the subclass or subinterface from the variables declared by the type of
the superclass or superinterface:

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


The *casting conversion* for numeric types allows getting the desired numeric
type:

.. code-block:: typescript
   :linenos:

    function process_int (an_int: int) { ... }

    process_int (3.14 as int)

.. index::
   casting conversion
   numeric type

|

.. _Raw Type Conversions:

Raw Types Conversion
====================

.. meta:
    frontend_status: Partly

Assuming that *G* is a generic type declaration with type parameters *n*,

.. code-block:: typescript
   :linenos:

    class|interface G<T1, T2, ... Tn> {}

any instantiation of G (*G* < *Type*:sub:`1`, ``...``, *Type*:sub:`n` >), or
its derived types can convert into *G* <> as follows:

.. code-block:: typescript
   :linenos:

    class|interface H<T1, T2, ... Tn> extends G<T1, T2, ... Tn> {}
    let raw: G<> = new G<Type1, Type2, ... TypeN>
    raw = new H<Type1, Type2, ... TypeN>

.. index::
   raw types conversion
   raw type
   generic type
   instantiation
   derived type


.. raw:: pdf

   PageBreak


