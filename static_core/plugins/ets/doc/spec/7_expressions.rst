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

.. _Expressions:

Expressions
###########

.. meta:
    frontend_status: Partly

This chapter describes the meanings of expressions and the rules for the
evaluation of expressions, except the expressions related to coroutines
(see :ref:`Coroutines`) and expressions
described as experimental (see :ref:`Lambda Expressions with Receiver`).

.. index::
   evaluation
   expression
   coroutine
   await expression

.. code-block:: abnf

    expression:
        primaryExpression
        | castExpression
        | instanceOfExpression
        | typeOfExpression
        | nullishCoalescingExpression
        | spreadExpression
        | unaryExpression
        | binaryExpression
        | assignmentExpression
        | conditionalExpression
        | stringInterpolation
        | lambdaExpression
        | lambdaExpressionWithReceiver
        | awaitExpression
        ;
    primaryExpression:
        literal
        | namedReference
        | arrayLiteral
        | objectLiteral
        | recordLiteral
        | thisExpression
        | parenthesizedExpression
        | methodCallExpression
        | fieldAccessExpression
        | indexingExpression
        | functionCallExpression
        | newExpression
        | ensureNotNullishExpression
        ;
    binaryExpression:
        multiplicativeExpression
        | additiveExpression
        | shiftExpression
        | relationalExpression
        | equalityExpression
        | bitwiseAndLogicalExpression
        | conditionalAndExpression
        | conditionalOrExpression
        ;

The grammar rules below introduce several productions to be used by other
expression rules:

.. code-block:: abnf

    objectReference:
        typeReference
        | 'super'
        | primaryExpression
        ;

``objectReference`` refers to one of the following three options:

- Class that is to handle static members;
- ``super`` that is to access constructors declared in the
  superclass, or the overridden method version of the superclass;
- *primaryExpression* that is to refer to an instance variable of a class,
  interface, or function type after evaluation, unless the manner of the
  evaluation is altered by the chaining operator '``?.``' (see
  :ref:`Chaining Operator`).

If the form of *primaryExpression* is *thisExpression*, then the pattern
"``this?.``" is handled as a :index:`compile-time error`.

If the form of *primaryExpression* is *super*, then the pattern "``super?.``"
is handled as a :index:`compile-time error`.

.. index::
   field
   constructor
   superclass
   overriding
   method
   variable
   expression
   instance variable
   class
   interface
   function type
   evaluation
   chaining operator
   pattern

.. code-block:: abnf

    arguments:
        '(' argumentSequence? ')'
        ;

    argumentSequence:
        restArgument
        | expression (',' expression)* (',' restArgument)? ','?
        ;

    restArgument:
        '...'? expression
        ;

The *arguments* grammar rule refers to the list of arguments of a call. Only
the last argument can have the form of a spread expression (see
:ref:`Spread Expression`).

.. index::
   argument
   grammar rule
   spread expression

|

.. _Evaluation of Expressions:

Evaluation of Expressions
*************************

.. meta:
    frontend_status: Done
    todo: needs more investigation, too much failing CTS tests (mostly tests are buggy)

The result of a program expression *evaluation* denotes the following:

-  Variable (the term *variable* is used here in the general, non-terminological
   sense to denote a modifiable lvalue in the left-hand side of an assignment);
   or
-  Value (results found elsewhere).

.. index::
   evaluation
   expression
   variable
   lvalue
   assignment

A variable or a value are equally considered the *value of the expression*
if such a value is required for further evaluation.

The type of an expression is inferred at compile time (see
:ref:`Contexts and Conversions`).

Expressions can contain assignments, increment operators, decrement operators,
method calls, and function calls. The evaluation of an expression can produce
side effects as a result.

*Constant expressions* (see :ref:`Constant Expressions`) are the expressions
with values that can be determined at compile time.

.. index::
   variable
   value
   evaluation
   expression
   type
   assignment
   increment operator
   decrement operator
   method call
   function call
   side effect
   constant expression
   compile time

|

.. _Normal and Abrupt Completion of Expression Evaluation:

Normal and Abrupt Completion of Expression Evaluation
=====================================================

.. meta:
    frontend_status: Done

Each expression in a normal mode of evaluation requires certain computational
steps. Normal modes of evaluation for each kind of expression are described
in the following sections.

An expression evaluation *completes normally* if all computational steps
are performed without throwing an error.

On the contrary, an expression *completes abruptly* if the expression
evaluation throws an error.

The information about the causes of an abrupt completion can be available
in the value attached to the error object.

.. index::
   normal completion
   abrupt completion
   evaluation
   expression
   value

Runtime errors can occur as a result of expression or operator evaluation as
follows:

-  If an *array reference expression* has the value ``null``, then an *array
   indexing expression* (see :ref:`Array Indexing Expression`) throws
   ``NullPointerError``.
-  If the value of an array index expression is negative, or greater than, or
   equal to the length of the array, then an *array indexing expression* (see
   :ref:`Array Indexing Expression`) throws ``ArrayIndexOutOfBoundsError``.
-  If a conversion cannot be performed at runtime, then a *cast expression*
   (see :ref:`Cast Expressions`) throws ``ClassCastError``.
-  If the right-hand operand expression has the zero value, then integer
   division (see :ref:`Division`), or integer remainder (see :ref:`Remainder`)
   operators throw ``ArithmeticError``.

.. index::
   predefined operator
   runtime error
   array reference expression
   value
   array access expression
   error
   array indexing expression
   array
   runtime
   cast expression
   integer division
   integer remainder
   operator
   remainder operator
   array element
   reference type
   array literal
   method call expression
   prefix
   postfix
   increment operator
   decrement operator
   array element type
   cast
   assignment

Possible hard-to-predict and hard-to-handle linkage and virtual machine errors
can cause errors during the evaluation of an expression.

Abrupt completion of the evaluation of a subexpression results in the following:

-  Immediate abrupt completion of the expression that contains such a
   subexpression (if the evaluation of the contained subexpression is required
   for the evaluation of the entire expression); and
-  Cancellation of all subsequent steps of the normal mode of evaluation.

The terms *complete normally* and *complete abruptly* can also denote
normal and abrupt completion of the execution of statements (see
:ref:`Normal and Abrupt Statement Execution`). A statement can complete
abruptly for a variety of reasons in addition to an error being thrown.

.. index::
   normal completion
   abrupt completion
   execution
   statement
   virtual machine
   expression
   subexpression
   evaluation
   linkage

|

.. _Order of Expression Evaluation:

Order of Expression Evaluation
==============================

.. meta:
    frontend_status: Done

The operands of an operator are evaluated from left to right in accordance with
the following rules:

-  Any right-hand operand is evaluated only after the left-hand operand of a
   binary operator is fully evaluated.

   If using a compound-assignment operator (see :ref:`Simple Assignment Operator`),
   the evaluation of the left-hand operand includes the following:

   - Remembering the variable denoted by the left-hand operand;
   - Fetching the value of that variable for the subsequent evaluation
     of the right-hand operand; and
   - Saving such a value.

   If the evaluation of the left-hand operand completes abruptly, then no
   part of the right-hand operand is evaluated.

.. index::
   operand
   operator
   evaluation
   compound-assignment operator
   assignment

-  Any part of the operation can be executed only after every operand of an
   operator (except conditional operators '``&&``', '``||``', and '``?:``')
   is fully evaluated.

   The execution of a binary operator that is an integer division '``/``' (see
   :ref:`Division`), or integer remainder '``%``' (see :ref:`Remainder`) can
   throw ``ArithmeticError`` only after the evaluations of both operands
   complete normally.
-  The |LANG| programming language follows the order of evaluation as indicated
   explicitly by parentheses, and implicitly by the precedence of operators.
   This rule particularly applies for infinity and ``NaN`` values of floating-point
   calculations.
   |LANG| considers integer addition and multiplication as provably associative;
   however, floating-point calculations must not be naively reordered because
   they are unlikely to be computationally associative (even though they appear
   mathematically associative).

.. index::
   operand
   abrupt completion
   evaluation
   operator
   conditional operator
   integer division
   integer remainder
   expression
   binary operator
   compound-assignment operator
   simple assignment operator
   variable
   value
   operator
   error
   precedence
   operator precedence
   infinity
   NaN value
   floating-point calculation
   integer addition
   integer multiplication
   associativity
   parenthesis

|

.. _Operator Precedence:

Operator Precedence
===================

.. meta:
    frontend_status: Done

The table below summarizes all information on the precedence and
associativity of operators. Each section on a particular operator
also contains detailed information.

.. index::
   precedence
   operator precedence
   operator
   associativity

+---------------------------------+--------------------------------------------+----------------+
|         **Operator**            |   **Precedence**                           | **Assoc-ty**   |
+=================================+============================================+================+
| postfix increment and decrement | ``++`` ``--``                              | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| prefix increment and decrement, | ``++ -- + - ! ~ typeof``                   | right to left  |
|                                 |                                            |                |
| unary, typeof                   |                                            |                |
+---------------------------------+--------------------------------------------+----------------+
| multiplicative                  | ``* / %``                                  | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| additive                        | ``+ -``                                    | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| cast                            | ``as``                                     | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| shift                           | ``<< >>  >>>``                             | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| relational                      | ``< > <= >= instanceof``                   | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| equality                        | ``== !=``                                  | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| bitwise AND                     | ``&``                                      | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| bitwise exclusive OR            | ``^``                                      | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| bitwise inclusive OR            | ``|``                                      | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| logical AND                     | ``&&``                                     | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| logical OR                      | ``||``                                     | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| null-coalescing                 | ``??``                                     | left to right  |
+---------------------------------+--------------------------------------------+----------------+
| ternary                         | ``?:``                                     | right to left  |
+---------------------------------+--------------------------------------------+----------------+
| assignment                      | ``= += -= %= *= /= &= ^= |= <<= >>= >>>=`` | right to left  |
+---------------------------------+--------------------------------------------+----------------+

.. index::
   precedence
   bitwise operator
   null-coalescing operator
   assignment
   shift operator
   cast operator
   equality operator
   postfix operator
   increment operator
   decrement operator
   prefix operator
   logical operator
   relational operator

|

.. _Evaluation of Arguments:

Evaluation of Arguments
=======================

.. meta:
    frontend_status: Done

An evaluation of arguments always progresses from left to right up to the first
error, or through the end of the expression; i.e., any argument expression is
evaluated after the evaluation of each argument expression to its left
completes normally (including comma-separated argument expressions that appear
within parentheses in method calls, constructor calls, class instance creation
expressions, or function call expressions).

If the left-hand argument expression completes abruptly, then no part of the
right-hand argument expression is evaluated.

.. index::
   evaluation
   argument
   error
   expression
   normal completion
   comma-separated argument expression
   method call
   constructor call
   class instance creation expression
   instance
   function call expression
   abrupt completion

|

.. _Evaluation of Other Expressions:

Evaluation of Other Expressions
===============================

.. meta:
    frontend_status: Done

These general rules cannot cover the order of evaluation of certain expressions
when they from time to time cause exceptional conditions. The order of
evaluation of the following expressions requires specific explanation:

-  Class instance creation expressions (see :ref:`New Expressions`);
-  :ref:`Array Creation Expressions`;
-  :ref:`Indexing Expressions`;
-  Method call expressions (see :ref:`Method Call Expression`);
-  Assignments involving indexing (see :ref:`Assignment`);
-  :ref:`Lambda Expressions`.

.. index::
   evaluation
   expression
   method call expression
   class instance creation expression
   array creation expression
   indexing expression
   assignment
   indexing
   lambda
   lambda expression

|

.. _Literal:

Literal
*******

.. meta:
    frontend_status: Done

Literals (see :ref:`Literals`) denote fixed and unchanging values. Type of
a literal (see :ref:`Literals`) is the type of an expression.

.. index::
   literal
   value

|

.. _Named Reference:

Named Reference
***************

.. meta:
    frontend_status: Done

An expression can have the form of a *named reference* as described by the
syntax rule as follows:

.. code-block:: abnf

    namedReference:
      qualifiedName typeArguments?
      ;

Type of a *named reference* expression is the type of the entity a
*named reference* refers to.

*QualifiedName* (see :ref:`Names`) is an expression that consists of
dot-separated names. If *qualifiedName* consists of a single identifier, then
it is called a *simple name*.

.. index::
   expression
   named reference
   syntax
   entity
   dot-separated name
   simple name

*Simple name* refers to the following:

-  Entity declared in the current compilation unit;
-  Local variable or parameter of the surrounding function or method.

If not a *simple name*, *qualifiedName* refers to the following:

-  Entity imported from a compilation unit,
-  Entity exported from a namespace, or
-  Member of some class or interface.

If *typeArguments* are provided, then *qualifiedName* is a valid instantiation
of the generic method or function. Otherwise, a :index:`compile-time error`
occurs.

A :index:`compile-time error` also occurs in the following situations:

-  If a name referred by *qualifiedName* is undefined or inaccessible; or
-  If ambiguity occurs while resolving a name except the function or method
   overloading case (see :ref:`Function, Method and Constructor Overloading`).

Type of a *named reference* is the type of an expression.

**Note**. When a *named reference* is of a function type, then a new function
object is always created as specified in :ref:`Uniqueness of Functional Objects`.
Also when a *named reference* refers to a member of some class or interface
which is a non-static method it is boundned with the object of that class or
interface.

.. index::
   simple name
   entity
   compilation unit
   namespace
   named reference
   expression
   overloading
   generic method
   ambiguity

.. code-block:: typescript
   :linenos:

    import * as compilationUnitName from "someFile"

    class Type {
      static method() {}
      method () {
         let local = this.method // Qualified name referring an instance method
      }
    }
    const m1 = Type.method  // Qualified name referring a static method
    const m2 = (new Type).method

    function foo (parameter: Type) {
      let local: Type = parameter /* 'parameter' here is the
          expression in the form of simple name, type of 'parameter' is the
          explicitly declared function parameter type */
      local = new Type () /* 'local' here is the expression in the
          form of simple name */
      local = compilationUnitName.someExportedVariable /* qualifiedName here
          refers to a variable imported from a compilation unit */
      let func = foo /* foo is a simple name of the function declared in this
          module, type of 'func' is the function type derived from the function
          'foo()' signature. func itself is a new function object */

      goo() // goo is an undefined name - compile-time error
      let bar_ref = bar // bar is an ambiguous reference - compile-time error
    }

    function bar (p: string) {}
    function bar (p: number) {}

    function generic_function<T> () {}
    let instantiation = generic_function<string> /* type of 'instantiation' is
         a function type derived from the signature of instantiated function
         'generic_function<string> ()' */

    
    class aClass {
        field = 123
        method(): number { return this.field }
    }
    const anObject = new aClass
    const aMethod = anObject.method
    anObject.field = 42
    console.log (aMethod()) // Outputs 42


|

.. _Array Literal:

Array Literal
*************

.. meta:
    frontend_status: Done
    todo: let x : int = [1,2,3][1] - valid?
    todo: let x = ([1,2,3][1]) - should be CTE, but it isn't
    todo: implement it properly for invocation context to get type from the context, not from the first element

*Array literal* is an expression that can be used to create an array or
tuple in some cases, and to provide some initial values:

.. code-block:: abnf

    arrayLiteral:
        '[' expressionSequence? ']'
        ;

    expressionSequence:
        expression (',' expression)* ','?
        ;

An *array literal* is a comma-separated list of *initializer expressions*
enclosed in '``[``' and '``]``'. A trailing comma after the last
expression in an array literal is ignored:

.. index::
   array literal
   array
   tuple
   expression
   value
   initializer expression
   trailing comma

.. code-block:: typescript
   :linenos:

    let x = [1, 2, 3] // ok
    let y = [1, 2, 3,] // ok, trailing comma is ignored

The number of initializer expressions enclosed in braces of the array
initializer determines the length of the array to be constructed.

If sufficient space is allocated for a new array, then a one-dimensional
array of the specified length is created. All elements of the array
are initialized to the values specified by initializer expressions.

.. index::
   initializer expression
   brace
   length of array
   array initializer
   array
   one-dimensional array
   array element
   initialization
   initializer expression
   value

On the contrary, the evaluation of the array initializer completes abruptly
in the following situations:

-  If the space allocated for a new array is insufficient, and
   ``OutOfMemoryError`` is thrown; or
-  If some initialization expression completes abruptly.

.. index::
   evaluation
   array initializer
   abrupt completion
   array
   initialization expression

Initializer expressions are executed from left to right. The *n*’th expression
specifies the value of the *n-1*’th element of the array.

Array literals can be nested (i.e., the initializer expression that specifies
an array element can be an array literal if that element is of array type).

Type of an *array literal expression* is inferred by the following rules:

.. index::
   initializer expression
   execution
   value
   array element
   array literal expression
   array type
   type inference

-  If a context is available, then type is inferred from the context. If
   successful, then type of an array literal is the inferred type ``T[]``,
   ``Array<T>``, or tuple.
-  Otherwise, type is to be inferred from the types of array literal
   elements.

More details of both cases are presented below.

.. index::
   type inference
   context
   array literal
   array element

|

.. _Array Literal Type Inference from Context:

Array Literal Type Inference from Context
=========================================

.. meta:
    frontend_status: Done

Type of an array literal can be inferred from the *context*, including
explicit type annotation of a variable declaration, left-hand part type
of an assignment, call parameter type, or type of a cast expression:

.. code-block:: typescript
   :linenos:

    let a: number[] = [1, 2, 3] // ok, variable type is used
    a = [4, 5] // ok, variable type is used

    function min(x: number[]): number {
      let m = x[0]
      for (let v of x)
        if (v < m)
          m = v
      return m
    }
    min([1., 3.14, 0.99]); // ok, parameter type is used

    // Two-dimensional array initialization
    type Matrix = number[][]
    let m: Matrix = [[1, 2], [3, 4], [5, 6]]

    class aClass {}
    let b1: Array <aClass> = [new aClass, new aClass]
    let b2: Array <number> = [1, 2, 3]
    let b3: FixedArray<number> = [1, 2]
      /* Type of literal is inferred from the context
         taken from b1, b2 and b3 declarations */


.. index::
   literal
   string literal
   instance
   error
   assignability
   inheritance
   context
   array
   tuple

The following example illustrates the possible kinds of context:

.. code-block:: typescript
   :linenos:

    let array: number[] = [1, 2, 3]   // assignment context
    function foo (array: number[]) {}
    foo ([1, 2, 3])                   // call context
    [1, 2, 3] as number[]             // casting conversion

.. index::
   type inference
   context
   array type
   array literal
   type
   type annotation
   variable declaration
   assignment
   call parameter type
   cast expression

All valid conversions are applied to the initializer expression, i.e., each
initializer expression type must be assignable (see :ref:`Assignability`)
to the array element type. Otherwise, a :index:`compile-time error` occurs.

.. index::
   conversion
   initializer expression
   assignability
   array element type

.. code-block:: typescript
   :linenos:

    let value: number = 2
    let list: Object[] = [1, value, "hello", new Error()] // ok

If the type used in the context is a *tuple type* (see :ref:`Tuple Types`),
and types of all literal expressions are compatible with tuple type elements
at respective positions, then an array literal is of tuple type.

.. code-block:: typescript
   :linenos:

    let tuple: [number, string] = [1, "hello"] // ok

    let incorrect: [number, string] = ["hello", 1] // compile-time error

If the type used in the context is a *union type* (see :ref:`Union Types`), then
it is necessary to try inferring the type of the array literal from its elements
(see :ref:`Array Type Inference from Types of Elements`). If successful, then
the type so inferred must be compatible with one of the types that form the
union type. Otherwise, a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    let union_of_arrays: number[] | string[] = [1, 2] // OK, type of literal is number[]
    let incorrect_union_of_arrays: number[] | string[] = [1, 2, "string"]
     /* compile-time error: (number|string)[] (type of the literal) is not compatible with
        number[] | string[] (type of the variable)
     */

.. index::
   tuple type
   literal expression
   compatibility
   context
   literal
   expression
   type
   array literal
   union type
   type inference

If the type used in the context is a *fixed array type* (see :ref:`Fixed Array Types`),
and each initializer expression type is compatible with the array element type,
then an array literal is of *fixed array type*.

.. code-block:: typescript
   :linenos:

    let farray: FixedArray<number> = [1, 2]

.. index::
   fixed array type
   initializer expression
   array element
   array literal

|

.. _Array Type Inference from Types of Elements:

Array Type Inference from Types of Elements
===========================================

.. meta:
    frontend_status: Done

When type of an array literal ``[`` ``expr``:sub:`1`, ``...`` , ``expr``:sub:`N` ``]``
cannot be inferred from the context, then the following algorithm is
used to infer it from initialization expressions:

.. #. If there is no expression (*N == 0*), then type is ``Object[]``.

#. If there are no elements in the array literal (*N == 0*), then type of
   the array literal cannot be inferred, and a :index:`compile-time error`
   occurs.

#. If type of at least one of element expression cannot be determined, then
   type of the array literal cannot be inferred, and a
   :index:`compile-time error` occurs.

#. If each initialization expression is of numeric type (see
   :ref:`Numeric Types`), then the type of the array literal is ``number[]``.

#. If all initialization expressions are of the same type ``T``, then the
   type of the array literal is ``T[]``.

#. Otherwise, the array literal type is an array of elements of the union type
   which is constructed as a union of ``T``:sub:`1` ``| ... | T``:sub:`N`,
   where ``T``:sub:`i` is the type of *expr*:sub:`i`. Union type normalization
   (see :ref:`Union Types Normalization`) is applied to this union type.

.. index::
   type inference
   array element
   array literal
   type
   context
   initialization expression
   expression
   numeric type
   union type normalization
   union type

.. code-block:: typescript
   :linenos:

    let a = []                        // compile-time error, type cannot be inferred
    let b = ["a"]                     // type is string[]
    let c = [1, 2, 3]                 // type is number[]
    let d = ["a", 1, 3.14]            // type is (string | number)[]
    let e = [(): void => {}, new A()] // type is (() => void | A)[]

|

.. _Object Literal:

Object Literal
***************

.. meta:
    frontend_status: Done

*Object literal* is an expression that can be used to create a class
instance and to provide some initial values. In some cases it is more
convenient to use an *object literal* in place of a class instance creation
expression (see :ref:`New Expressions`):

.. index::
   object literal
   expression
   instance
   class instance
   creation expression

.. code-block:: abnf

    objectLiteral:
       '{' valueSequence? '}'
       ;

    valueSequence:
       nameValue (',' nameValue)* ','?
       ;

    nameValue:
       identifier ':' expression
       ;

An *object literal* is written as a comma-separated list of *name-value pairs*
enclosed in curly braces '``{``' and '``}``'. A trailing comma after the last
pair is ignored. Each *name-value pair* consists of an identifier and an
expression:

.. index::
   object literal
   comma-separated list
   name-value pair
   curly brace
   trailing comma
   identifier
   expression

.. code-block:: typescript
   :linenos:

    class Person {
      name: string = ""
      age: number = 0
    }
    let b : Person = {name: "Bob", age: 25}
    let a : Person = {name: "Alice", age: 18, } //ok, trailing comma is ignored

Type of an *object literal expression* is always some class ``C`` that is
inferred from the context. A type inferred from the context can be either a
named class (see :ref:`Object Literal of Class Type`), or an anonymous class
created for the inferred interface type (see
:ref:`Object Literal of Interface Type`).

A :index:`compile-time error` occurs if:

-  Type of an object literal cannot be inferred from the context; or
-  The inferred type is not a class or interface type.
-  The inferred type has abstract methods (see :ref:`Abstract Methods`).
   **Note**. An abstract class without abstract methods can be used.

.. index::
   object literal expression
   type inference
   named class
   abstract method
   abstract class
   anonymous class
   context
   class type
   anonymous class
   interface type
   compile-time error
   inferred type

.. code-block:: typescript
   :linenos:

    let p = {name: "Bob", age: 25}
            // compile-time error, type cannot be inferred

|

.. _Object Literal of Class Type:

Object Literal of Class Type
=============================

.. meta:
    frontend_status: Done

If class type ``C`` is inferred from the context, then type of an object
literal is ``C``:

.. index::
   object literal
   class type
   type inference
   context

.. code-block:: typescript
   :linenos:

    class Person {
      name: string = ""
      age: number = 0
    }
    function foo(p: Person) { /*some code*/ }
    // ...
    let p: Person = {name: "Bob", age: 25} /* ok, variable type is
         used */
    foo({name: "Alice", age: 18}) // ok, parameter type is used

An identifier in each *name-value pair* must name a field of class ``C``,
or a field of any superclass of class ``C``.

A :index:`compile-time error` occurs if the identifier does not name an
*accessible member field* (see :ref:`Accessible`) in type ``C``:

.. index::
   identifier
   name-value pair
   field
   superclass
   class
   compile-time error
   accessible member field

.. code-block:: typescript
   :linenos:

    class Friend {
      name: string = ""
      private nick: string = ""
      age: number
      sex?: "male"|"female"
    }
    // compile-time error, nick is private:
    let f: Friend = {name: "Alexander", age: 55, nick: "Alex"}

A :index:`compile-time error` occurs if type of an expression in a
*name-value pair* is not assignable (see :ref:`Assignability`) to the
field type:

.. code-block:: typescript
   :linenos:

    let f: Friend = {name: 123} /* compile-time error - type of right hand-side
    is not assignable to the type of the left hand-side */

If some class fields have default values (see :ref:`Default Values for Types`)
or explicit initializers (see :ref:`Variable and Constant Declarations`), then
such fields can be skipped in the object literal.

.. index::
   expression
   name-value pair
   compatibility
   field type
   class field
   object literal
   initializer

.. code-block:: typescript
   :linenos:

    let f: Friend = {} /* OK, as name, nick, age, and sex have either default
                          value or explicit initializer */

If an object literal is to use class ``C``, then class ``C`` must have a
*parameterless* constructor (explicit or default) that is *accessible*
(see :ref:`Accessible`) in the class-composite context.

A :index:`compile-time error` occurs if:

-  ``C`` contains no parameterless constructor; or
-  No constructor is accessible (see :ref:`Accessible`).

These situations are presented in the examples below:

.. index::
   parameterless constructor
   accessibility
   context
   object literal

.. code-block:: typescript
   :linenos:

    class C {
      constructor (x: number) {}
    }
    // ...
    let c: C = {} /* compile-time error - no parameterless
           constructor */

.. code-block:: typescript
   :linenos:

    class C {
      private constructor () {}
    }
    // ...
    let c: C = {} /* compile-time error - constructor is not
        accessible */

If a class has accessors (see :ref:`Accessor Declarations`) for a property,
and its setter is provided, then this property can be used as a part of an
object literal. Otherwise, a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    class OK {
        set attr (attr: number) {}
    }
    const a: OK = {attr: 42} // OK, as the setter be called

    class Err {
        get attr (): number { return 42 }
    }
    const b: Err = {attr: 42} // compile-time error - no setter for 'attr'

.. index::
   accessor
   setter
   object literal

|

.. _Object Literal of Interface Type:

Object Literal of Interface Type
================================

.. meta:
    frontend_status: Done

If the interface type ``I`` is inferred from the context, then type of an
object literal is an anonymous class implicitly created for interface ``I``:

.. code-block:: typescript
   :linenos:

    interface Person {
      name: string
      age: number
    }
    let b: Person = {name: "Bob", age: 25}

In the example above, type of *b* is an anonymous class that contains the
same fields as the interface ``I`` properties.

.. index::
   interface type
   type inference
   context
   object literal
   anonymous class
   interface
   field

If some interface properties are of an optional type, then such properties can
be skipped in an object literal as their default values are *undefined* (see
:ref:`Default Values for Types`) according to the union type nature (see
:ref:`Variable Declarations`):

.. code-block:: typescript
   :linenos:

    interface Person {
      name: string
      age: number
      sex?: "male"|"female"
    }
    let b: Person = {name: "Bob", age: 25}
         // 'sex' field will have 'undefined' value

The interface type ``I`` must contain properties only. A :index:`compile-time error`
occurs if interface type ``I`` contains a method:

.. index::
   object literal
   interface type
   interface property
   undefined value
   union type
   inference type
   interface
   property
   method

.. code-block:: typescript
   :linenos:

    interface I {
      name: string
      foo()
    }
    let i : I = {name: "Bob"} // compile-time error, interface has methods

If an interface has accessors (see :ref:`Accessor Declarations`) for some
property, and the property is used in an object literal, then a
:index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    interface I1 {
        set attr (attr: number)
    }
    const a: I1 = {attr: 42} /* compile-time error - 'attr' cannot be used
                                 in object literal */

    interface I2 {
        get attr (): number
    }
    const b: I2 = {attr: 42} /* compile-time error - 'attr' cannot be used
                                 in object literal */

.. index::
   interface
   accessor
   property
   object literal

|

.. _Object Literal of Record Type:

Object Literal of ``Record`` Type
=================================

.. meta:
    frontend_status: Done

Generic type ``Record<Key, Value>`` (see :ref:`Record Utility Type`) is used
to map properties of a type (type ``Key``) to another type (type ``Value``).
A special form of object literal is used to initialize the value of such
type:

.. index::
   object literal
   generic type
   record type
   type property
   type value
   type key
   initialization
   value

.. code-block:: abnf

    recordLiteral:
       '{' keyValueSequence? '}'
       ;

    keyValueSequence:
       keyValue (',' keyValue)* ','?
       ;

    keyValue:
       expression ':' expression
       ;

The first expression in ``keyValue`` denotes a key and must be of type ``Key``.
The second expression denotes a value and must be of type ``Value``:

.. index::
   expression
   key
   value

.. code-block:: typescript

    let map: Record<string, number> = {
        "John": 25,
        "Mary": 21,
    }

    console.log(map["John"]) // prints 25

.. code-block:: typescript

    interface PersonInfo {
        age: number
        salary: number
    }
    let map: Record<string, PersonInfo> = {
        "John": { age: 25, salary: 10},
        "Mary": { age: 21, salary: 20}
    }

If a key is a union type consisting of literals or enum type, then all variants
must be listed in the object literal. Otherwise, a :index:`compile-time error`
occurs:

.. index::
   key
   union type
   literal
   object literal

.. code-block:: typescript

    let map: Record<"aa" | "bb", number> = {
        "aa": 1,
    } // compile-time error: "bb" key is missing

    enum Enum {A, B}
    const map1: Record<Enum, number> = {
        Enum.A: 1
    } // compile-time error: key "Enum.b" is missing


|

.. _Object Literal Evaluation:

Object Literal Evaluation
=========================

.. meta:
    frontend_status: Done

The evaluation of an object literal of type ``C`` (where ``C`` is either
a named class type or an anonymous class type created for the interface)
is to be performed by the following steps:

-  A parameterless constructor is executed to produce an instance *x* of
   class ``C``. The execution of the object literal completes abruptly
   if so does the execution of the constructor.

-  Name-value pairs of the object literal are then executed from left to
   right in the textual order they occur in the source code. The execution
   of a name-value pair includes the following:

   -  Evaluation of the expression; and
   -  Assignment of the value of expression to the corresponding field
      of *x* as its initial value. This rule also applies to *readonly* fields.

.. index::
   object literal
   evaluation
   named class
   anonymous class
   interface
   parameterless constructor
   constructor
   instance
   execution
   abrupt completion
   name-value pair
   field
   value
   expression
   assignment
   literal type
   readonly field

The execution of an object literal completes abruptly if so does
the execution of a name-value pair.

An object literal completes normally with the value of a newly
initialized class instance if so do all name-value pairs.

.. index::
   execution
   object literal
   abrupt completion
   normal completion
   name-value pair
   evaluation
   initialization
   class instance

|

.. _spread Expression:

Spread Expression
*****************

.. meta:
    frontend_status: Done

.. code-block:: abnf

    spreadExpression:
        '...' expression
        ;

*Spread expression* can be used only within an array literal (see
:ref:`Array Literal`) or argument passing. The *expression* must be of
array type (see :ref:`Array Types`) or tuple type (see :ref:`Tuple Types`).
Otherwise, a :index:`compile-time error` occurs.

A *spread expression* for arrays or tuples can be evaluated as follows:

-  By the compiler at compile time if *expression* is constant (see
   :ref:`Constant Expressions`);
-  At runtime otherwise.

An array or tuple referred by the *expression* is broken by the evaluation into
a sequence of values. This sequence is used where a *spread expression* is used.
It can be an assignment, a call of a function, method, or constructor.
A sequence of types of these values is the type of the *spread expression*.

.. index::
   spread expression
   array literal
   argument
   expression
   array type
   tuple type
   runtime
   compiler
   evaluation
   call
   function
   method
   constructor
   assignment

.. code-block:: typescript
   :linenos:

    let array1 = [1, 2, 3]
    let array2 = [4, 5]
    let array3 = [...array1, ...array2] // spread array1 and array2 elements
       // while building new array literal during compile-time
    console.log(array3) // prints [1, 2, 3, 4, 5]

    foo (...array2)  // spread array2 elements into arguments of the foo() call
    function foo (...array: number[]) {
      console.log (array)
    }

    run_time_spread_application1 (array1, array2) // prints [1, 2, 3, 42, 4, 5]
    function run_time_spread_application1 (a1: number[], a2: number[]) {
      console.log ([...a1, 42, ...a2])
        // array literal will be built at runtime
    }

    let tuple1: [number, string, boolean] = [1, "2", true]
    let tuple2: [number, string] = [4, "5"]
    let tuple3: [number, string, boolean, number, string] = [...tuple1, ...tuple2] // spread tuple1 and tuple2 elements
       // while building new tuple object during compile-time
    console.log(tuple3) // prints [1, "2", true, 4, "5"]

    bar (...tuple2)  // spread tuple2 elements into arguments of the foo() call
    function bar (...tuple: [number, string]) {
      console.log (tuple)
    }

    run_time_spread_application2 (tuple1, tuple2) // prints [1, "2", true, 42, 4, "5"]
    function run_time_spread_application2 (a1: [number, string, boolean], a2: [number, string]) {
      console.log ([...a1, 42, ...a2])
        // such array literal will be built at runtime
    }

**Note**. If an array is spread while calling a function, then an appropriate
parameter must be of the spread array kind. If an array is spread into a
sequence of ordinary parameters, then a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    let an_array = [1, 2]
    bar (...an_array) // compile-time error
    function bar (n1: number, n2: number) { ... }

**Note**. If a tuple is spread while calling a function, then an appropriate
parameter must be of the spread tuple kind. If a tuple is spread into a
sequence of ordinary parameters, then a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    let a_tuple: [number, string] = [1, "2"]
    bar (...a_tuple) // compile-time error
    function bar (n1: number, n2: string) { ... }

.. index::
   spread
   function call
   parameter
   tuple
   spread array
   array
   parameter

|

.. _Parenthesized Expression:

Parenthesized Expression
************************

.. meta:
    frontend_status: Done

.. code-block:: abnf

    parenthesizedExpression:
        '(' expression ')'
        ;

Type and value of a parenthesized expression are the same as those of
the contained expression.

.. index::
   parenthesized expression
   type
   value
   contained expression

|

.. _this Expression:

``this`` Expression
*******************

.. meta:
    frontend_status: Done

.. code-block:: abnf

    thisExpression:
        'this'
        ;

The keyword ``this`` can be used as an expression in the body of an instance
method of a class (see :ref:`Method Body`) or interface (see
:ref:`Default Interface Method Declarations`). The type of *this* expression
is the appropriate class or interface type.

It can be used in a lambda expression only if it is allowed in the context
in which the lambda expression occurs.

The keyword ``this`` in a *direct call* ``this(`` *arguments* ``)`` expression
can only be used in the explicit constructor call statement (see
:ref:`Explicit Constructor Call`).

The keyword ``this`` can also be used in the body of a function with receiver
(see :ref:`Functions with Receiver`). The type of *this* expression is the
declared type of the parameter ``this`` in a function.

A :index:`compile-time error` occurs if the keyword ``this`` appears elsewhere.

.. index::
   keyword this
   expression
   instance method
   method body
   class
   interface type
   lambda expression
   direct call expression
   constructor
   constructor call statement

The keyword ``this`` used as a primary expression denotes a value that is a
reference to the following:

-  Object for which the instance method is called; or
-  Object being constructed.

The parameter ``this`` in a lambda body and in the surrounding context denote
the same value.

The class of the actual object referred to at runtime can be ``T`` if ``T`` is
a class type, or a class assignable (see :ref:`Assignability`) to ``T``.

.. index::
   keyword this
   primary expression
   value
   instance method
   instance method call
   object
   lambda body
   surrounding context
   assignability
   class
   runtime
   class type
   class

|

.. _Field Access Expression:

Field Access Expression
***********************

.. meta:
    frontend_status: Done

*Field access expression* can access a field of an object to which an object
reference refers. The object reference can have different forms as described
in detail in :ref:`Accessing Current Object Fields` and in
:ref:`Accessing SuperClass Properties`.

.. index::
   field access expression
   access
   field
   object reference

.. code-block:: abnf

    fieldAccessExpression:
        objectReference ('.' | '?.') identifier
        ;

A *field access expression* that contains '``?.``' (see :ref:`Chaining Operator`)
is called *safe field access* because it handles nullish object references
safely.

If object reference evaluation completes abruptly, then so does the entire
field access expression.

An object reference used to access a field must be a non-nullish reference
type ``T``. Otherwise, a :index:`compile-time error` occurs.

A field access expression is valid if the identifier refers to an accessible
member field (see :ref:`Accessible`) in type ``T``. A :index:`compile-time error`
occurs otherwise.

Type of a *field access expression* is the type of a member field.

.. index::
   access
   field
   field access expression
   safe field access
   nullish object reference
   object reference
   abrupt completion
   non-nullish type
   reference type
   member field
   accessible member field

|

.. _Accessing Current Object Fields:

Accessing Current Object Fields
===============================

.. meta:
    frontend_status: Done

The result of a field access expression is computed at runtime as described
below.

a. *Static* field access (*objectReference* is evaluated in the form *typeReference*)

The evaluation of *typeReference* is performed. The result of a *field access
expression* of a static field in a class is as follows:

-  ``variable`` if the field is not ``readonly``. The resultant value can
   be changed later.

-  ``value`` if the field is ``readonly``, except where *field access* occurs
   in a class initializer (see :ref:`Class Initializer`).

.. index::
   access
   runtime
   field access expression
   object reference expression
   evaluation
   static field access
   static field
   field access expression
   field access
   class initializer
   field
   readonly
   class

b. *Instance* field access (*objectReference* is evaluated in the form *primaryExpression*)

The evaluation of *primaryExpression* is performed. The result of *field
access expression* of an instance field in a class or interface is as follows:

-  ``variable`` if the field is not ``readonly``. The resultant value can
   be changed later.

-  ``value`` if the field is ``readonly``, except where *field access* occurs
   in a constructor (see :ref:`Constructor Declaration`).

Only the *primaryExpression* type (not class type of an actual object
referred at runtime) is used to determine the field to be accessed.

.. index::
   instance field access
   field access
   field access expression
   interface
   readonly
   evaluation
   access
   runtime
   initializer
   instance initializer
   constructor
   field access
   class type

|

.. _Accessing SuperClass Properties:

Accessing SuperClass Properties
===============================

.. meta:
    frontend_status: None

The the form ``super.identifier`` is valid when accessing the superclass
property via accessor (see :ref:`Accessor Declarations`).
A :index:`compile-time error` occurs if identifier in 'super.identifier'
denotes a field.

.. code-block:: typescript
   :linenos:

    class Base {
       get property(): number { return 1 }
       set property(p: number) { }
       field = 1234
    }
    class Derived extends Base {
       get property(): number { return super.property } // OK
       set property(p: number) { super.property = 42 } // OK
       foo () {
          super.field = 42          // compile-time error
          console.log (super.field)  // compile-time error
       }
    }

.. index::
   access
   superclass property
   identifier
   field

|

.. _Method Call Expression:

Method Call Expression
**********************

.. meta:
    frontend_status: Done

A *method call expression* calls a static or instance method of a class or
an interface.

.. index::
   method call expression
   static method
   instance method
   class
   interface

.. code-block:: abnf

    methodCallExpression:
        objectReference ('.' | '?.') identifier typeArguments? arguments block?
        ;

The syntax form that has a block associated with the method call is a special
form called *trailing lambda call* (see :ref:`Trailing Lambdas` for details).

A method call with '``?.``' (see :ref:`Chaining Operator`) is called a
*safe method call* because it handles nullish values safely.

Resolving a method at compile time is more complicated than resolving a field
because method overloading (see :ref:`Class Method Overloading`) can occur.

There are several steps that determine and check the method to be called at
compile time (see :ref:`Step 1 Selection of Type to Use`,
:ref:`Step 2 Selection of Method`, and
:ref:`Step 3 Checking Method Modifiers`).

.. index::
   syntax
   block
   trailing lambda call
   method call
   safe method call
   nullish value
   method resolution
   method overloading
   compile time
   field resolution

|

.. _Step 1 Selection of Type to Use:

Step 1: Selection of Type to Use
================================

.. meta:
    frontend_status: Done

The *object reference* is used to determine the type in which to search for the
method. Three forms of *object reference* are possible:

.. table::
   :widths: 40, 60

   ============================== =================================================================
    **Form of Object Reference**   **Type to Use**
   ============================== =================================================================
   ``typeReference``               Type denoted by ``typeReference``.
   ``expression`` of type *T*      ``T`` if ``T`` is a class, interface, or union; ``T``’s constraint (:ref:`Type Parameter Constraint`) if ``T`` is a type parameter. A :index:`compile-time error` occurs otherwise.
   ``super``                       The superclass of the class that contains the method call.
   ============================== =================================================================

.. index::
   type
   type parameter
   object reference
   method
   constraint
   superclass
   method call

|

.. _Step 2 Selection of Method:

Step 2: Selection of Method
===========================

.. meta:
    frontend_status: Done

After the type to use is known, the method to call must be determined. |LANG|
supports overloading, and more than one method can be accessible under the
method name used in the call.

All accessible methods are called *potentially applicable candidates*, and
:ref:`Overload Resolution` is used to select the method to call. If *overload
resolution* can definitely select a single method, then this method is called.
Otherwise, a :index:`compile-time error` occurs as more than one applicable
method is available (no method to call, or ambiguity).

.. index::
   overload resolution
   method call
   potentially applicable candidate
   accessible method
   access

|

.. _Step 3 Checking Method Modifiers:

Step 3: Checking Method Modifiers
=================================

.. meta:
    frontend_status: Done

In this step, the single method to call is known, and the following set of
semantic checks must be performed:

-  If the method call has the form ``typeReference.identifier``, then
   ``typeReference`` refers to a class, and the method must be declared
   ``static``. Otherwise, a :index:`compile-time error` occurs.

-  If the method call has the form ``expression.identifier``, then the method
   must not be declared ``static``. Otherwise, a :index:`compile-time error`
   occurs.

-  If the method call has the form ``super.identifier``, then the method must
   not be declared ``abstract`` or ``static``. Otherwise, a
   :index:`compile-time error` occurs.

.. index::
   method call
   semantic check
   static method call
   abstract method call
   type argument

|

.. _Type of Method Call Expression:

Type of Method Call Expression
==============================

.. meta:
    frontend_status: None

Type of a *method call expression* is the return type of the method.

.. code-block:: typescript
   :linenos:

    class A {
       static method() { console.log ("Static method() is called") }
       method()        { console.log ("Instance method() is called") }
    }


    let x = A.method()     // compile-time error as void cannot be used as type annotation
    A.method ()            // OK
    let y = new A.method() // compile-time error as void cannot be used as type annotation
    new A.method()         // OK

.. index::
   method call expression
   method return type
   type annotation

|

.. _Function Call Expression:

Function Call Expression
************************

.. meta:
    frontend_status: Done

*Function call expression* is used to call a function (see
:ref:`Function Declarations`), a variable of a function type
(:ref:`Function Types`), or a lambda expression (see :ref:`Lambda Expressions`):

.. code-block:: abnf

    functionCallExpression:
        expression ('?.' | typeArguments)? arguments block?
        ;

A special syntactic form that contains a block associated with the function
call is called *trailing lambda call* (see :ref:`Trailing Lambdas` for details).

A :index:`compile-time error` occurs if the expression type is one of the
following:

-  Different than the function type;
-  Nullish type but without '``?.``' (see :ref:`Chaining Operator`).

.. index::
   function call expression
   function call
   function type
   trailing lambda call
   lambda expression
   expression type
   function type
   nullish type
   chaining operator

If the operator '``?.``' (see :ref:`Chaining Operator`) is present, and the
*expression* evaluates to a nullish value, then:

-  *Arguments* are not evaluated;
-  Call is not performed; and
-  Result of *functionCallExpression* is not produced as a consequence.

The function call is *safe* because it handles nullish values properly.

.. index::
   chaining operator
   expression
   evaluation
   nullish value
   argument
   semantic correctness check
   undefined
   function call

The following important situations depend on the form of expression in a call,
and require different semantic checks:

- The form of expression in the call is *qualifiedName*, and *qualifiedName*
  refers to an accessible function (:ref:`Function Declarations`), or to a set
  of accessible overloaded functions.

  In this case, all accessible functions are *potentially applicable candidates*.
  :ref:`Overload Resolution` is used to select the function to call.
  If *overload resolution* can definitely select a single function, then this
  function is called.
  Otherwise (i.e., if there is no function to call, or if there is ambiguity
  caused where more than one applicable function is available), a
  :index:`compile-time error` occurs.

.. index::
   call
   expression
   qualified name
   accessible function
   overloaded function
   overload resolution


- All other forms of expression.

  In this case, *overload resolution* is not required as the expression
  determines the entity to call unambiguously. Semantic check is performed
  in accordance with :ref:`Compatibility of Call Arguments`.

.. index::
   overload resolution
   expression
   semantic check
   compatibility
   function call
   potentially applicable candidate
   accessibility
   qualified name
   function

The example below represents different forms of function calls:

.. code-block:: typescript
   :linenos:

    function foo() { console.log ("Function foo() is called") }
    foo() // function call uses function name to call it

    call (foo)            // top-level function passed
    call ((): void => { console.log ("Lambda is called") }) // lambda is passed
    call (A.method)       // static method
    call ((new A).method) // instance method is passed

    class A {
       static method() { console.log ("Static method() is called") }
       method() { console.log ("Instance method() is called") }
    }

    function call (callee: () => void) {
       callee() // function call uses parameter name to call any functional object passed as an argument
    }

    ((): void => { console.log ("Lambda is called") }) () // function call uses lambda expression to call it

    let x = foo() // compile-time error as void cannot be used as type annotation

Type of a *function call expression* is the return type of the function.

.. index::
   function call
   function call expression
   return type
   function

|

.. _Indexing Expressions:

Indexing Expressions
********************

.. meta:
    frontend_status: Done

*Indexing expressions* are used to access elements of arrays (see
:ref:`Array Types`) and ``Record`` instances (see :ref:`Record Utility Type`).
Indexing expressions can also be applied to instances of indexable types (see
:ref:`Indexable Types`):

.. code-block:: abnf

    indexingExpression:
        expression ('?.')? '[' expression ']'
        ;

Any *indexing expression* has two subexpressions as follows:

-  *Object reference expression* before the left bracket; and
-  *Index expression* inside the brackets.

.. index::
   indexing expression
   indexable type
   access
   array element
   array type
   subexpression
   object reference expression
   index expression
   bracket

If the operator '``?.``' (see :ref:`Chaining Operator`) is present in an
indexing expression, then:

-  If an object reference expression is not of a nullish type, then the
   chaining operator has no effect.
-  Otherwise, object reference expression must be checked to nullish
   value. If the value is ``undefined`` or ``null``,
   then the evaluation of the entire surrounding *primary expression* stops.
   The result of the entire primary expression is then ``undefined``.

If no '``?.``' is present in an indexing expression, then object reference
expression must be an array type or the ``Record`` type. Otherwise, a
:index:`compile-time error` occurs.

.. index::
   chaining operator
   operator
   indexing expression
   object reference expression
   expression
   primary expression
   nullish type
   record type
   reference expression
   nullish value

|

.. _Array Indexing Expression:

Array Indexing Expression
=========================

.. meta:
    frontend_status: Partly
    todo: implement floating point index support - #14001

*Index expression* for array indexing must be of a numeric type (see
:ref:`Numeric Types`).

If an *index expression* is of type ``number`` or other floating-point type,
and the fractional part differs from 0, then errors occur as follows:

-  A runtime error, if the situation is identified during program execution;
   and
-  A :index:`compile-time error`, if the situation is detected during
   compilation.

.. index::
   array indexing
   numeric type
   index expression
   floating-point type
   runtime error
   compilation

A numeric types conversion (see :ref:`Primitive Types Conversions`) is
performed on an *index expression* to ensure that the resultant type is ``int``.
Otherwise, a :index:`compile-time error` occurs.

If the chaining operator '``?.``' (see :ref:`Chaining Operator`) is present,
and after its application the type of *object reference expression*
is an *array type*,
then it makes a valid *array reference expression*, and the type
of the array indexing expression is ``T``.

The result of an array indexing expression is a variable of type ``T`` (i.e., an
element of the array selected by the value of that *index expression*).

It is essential that, if type ``T`` is a reference type, then the fields of
array elements can be modified by changing the resultant variable fields:

.. index::
   numeric types conversion
   index expression
   chaining operator
   object reference expression
   array type
   array reference expression
   array indexing expression
   variable
   reference type


.. code-block:: typescript
   :linenos:

    let names: string[] = ["Alice", "Bob", "Carol"]
    console.log(name[1]) // prints Bob
    string[1] = "Martin"
    console.log(name[1]) // prints Martin

    class RefType {
        field: number = 42
    }
    const objects: RefType[] = [new RefType(), new RefType()]
    const object = objects [1]
    object.field = 777            // change the field in the array element
    console.log(objects[0].field) // prints 42
    console.log(objects[1].field) // prints 777

    let an_array = [1, 2, 3]
    let element = an_array [3.5] // Compile-time error
    function foo (index: number) {
       let element = an_array [index]
          // Runtime-time error if index is not integer
    }

An array indexing expression evaluated at runtime behaves as follows:

-  Object reference expression is evaluated first.
-  If the evaluation completes abruptly, then so does the indexing
   expression, and the index expression is not evaluated.
-  If the evaluation completes normally, then the index expression is evaluated.
   The resultant value of the object reference expression refers to an array.
-  If the index expression value of an array is less than zero, greater than
   or equal to that array’s *length*, then the ``ArrayIndexOutOfBoundsError``
   is thrown.
-  Otherwise, the result of the array access is a type ``T`` variable within
   the array selected by the value of the index expression.

.. code-block:: typescript
   :linenos:

    function setElement(names: string[], i: number, name: string) {
        names[i] = name // run-time error, if 'i' is out of bounds
    }

.. index::
   array indexing expression
   index expression
   evaluation
   runtime
   array
   object reference expression
   abrupt completion
   normal completion
   reference expression
   variable

|

.. _Record Indexing Expression:

Record Indexing Expression
==========================

.. meta:
    frontend_status: Done

*Index expression* for a ``Record<Key, Value>`` indexing (see
:ref:`Record Utility Type`) must be of type ``Key``.

The following two cases are to be considered separately:

1. Type ``Key`` is a union that contains literal types only;
2. Other cases.

**Case 1.** If type ``Key`` is a union that contains literal types only, then
an *index expression* can only be one of the literals listed in the type.
The result of the indexing expression is of type ``Value``.

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    type Keys = 'key1' | 'key2' | 'key3'

    let x: Record<Keys, number> = {
        'key1': 1,
        'key2': 2,
        'key3': 4,
    }
    let y = x['key2'] // y value is 2

.. index::
   index expression
   indexing expression
   key
   literal type
   literal
   value
   type

A :index:`compile-time error` occurs if an index expression is not a valid
literal:

.. code-block:: typescript
   :linenos:

    console.log(x['key4']) // compile-time error
    x['another key'] = 5 // compile-time error

The compiler guarantees that an object of ``Record<Key, Value>`` for this type
``Key`` contains values for all ``Key`` keys.

**Case 2.** An *index expression* has no restriction.
The result of an indexing expression is of type ``Value | undefined``.

.. index::
   index expression
   indexing expression
   literal
   compiler
   object
   key
   restriction


.. code-block-meta:

.. code-block:: typescript
   :linenos:

    let x: Record<number, string> = {
        1: "hello",
        2: "buy",
    }

    function foo(n: number): string | undefined {
        return x[n]
    }

    function bar(n: number): string {
        let s = x[n]
        if (s == undefined) { return "no" }
        return s!
    }

    foo(3) // prints "undefined"
    bar(3) // prints "no"

    let y = x[3]

.. index::
   index expression
   literal
   key
   compiler
   value
   indexing expression

Type of *y* in the code above is ``string | undefined``. The value of
*y* is ``undefined``.

An indexing expression evaluated at runtime behaves as follows:

-  Object reference expression is evaluated first.
-  If the evaluation completes abruptly, then so does the indexing
   expression, and the index expression is not evaluated.
-  If the evaluation completes normally, then the index expression is
   evaluated.
   The resultant value of the object reference expression refers to a ``record``
   instance.
-  If the ``record`` instance contains a key defined by the index expression,
   then the result is the value mapped to the key.
-  Otherwise, the result is the literal ``undefined``.

.. index::
   type
   value
   reference type
   key
   indexing expression
   index expression
   object reference expression
   abrupt completion
   normal completion
   literal
   record instance
   mapped value

|

.. _Chaining Operator:

Chaining Operator
*****************

.. meta:
    frontend_status: Done

The *chaining operator* '``?.``' is used to effectively access values of
nullish types. It can be used in the following contexts:

- :ref:`Field Access Expression`,
- :ref:`Method Call Expression`,
- :ref:`Function Call Expression`,
- :ref:`Indexing Expressions`.

If the value of the expression to the left of '``?.``' is ``undefined`` or
``null``, then the evaluation of the entire surrounding *primary expression*
stops. The result of the entire primary expression is then ``undefined``. Thus
the type of the entire primary expression is the union ``undefined`` |
*non-nullish type of the entire primary expression*:

.. index::
   chaining operator
   field access
   access
   value
   nullish type
   primary expression
   non-nullish type

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class Person {
        name: string
        spouse?: Person = undefined
        constructor(name: string) {
            this.name = name
        }
    }

    let bob = new Person("Bob")
    console.log(bob.spouse?.name) // prints "undefined"
       // type of bob.spouse?.name is undefined|string

    bob.spouse = new Person("Alice")
    console.log(bob.spouse?.name) // prints "Alice"
       // type of bob.spouse?.name is undefined|string

If an expression is not of a nullish type, then the chaining operator has
no effect.

A :index:`compile-time error` occurs if a chaining operator is placed in the
context where a variable is expected, e.g., in the left-hand-side expression of
an assignment (see :ref:`Assignment`) or expression
(see :ref:`Postfix Increment`, :ref:`Postfix Decrement`,
:ref:`Prefix Increment` or :ref:`Prefix Decrement`).

.. index::
   expression
   chaining operator
   nullish value
   nullish type
   assignment
   postfix
   prefix
   decrement
   increment
   function call
   method call
   primary expression
   evaluation
   access
   value

|

.. _New Expressions:

``New`` Expressions
*******************

.. meta:
    frontend_status: Done

There are two syntactical forms of the *new expression*:

.. code-block:: abnf

    newExpression:
        newClassInstance
        | newArrayInstance
        ;

Type of a *new expression* is ether ``class`` or ``array``.

A *new class instance expression* creates a new object that is an instance
of the specified class and it is described in full details below.

The creation of array instances is an experimental feature discussed in
:ref:`Array Creation Expressions`.

.. index::
   syntactical form
   expression
   expression type
   instance
   instantiation
   class instance creation expression
   array instance
   array creation expression

.. code-block:: abnf

    newClassInstance:
        'new' typeArguments? typeReference arguments?
        ;

*Class instance creation expression* specifies a class to be instantiated.
It optionally lists all actual arguments for the constructor.

.. code-block:: typescript
   :linenos:

    class A {
       constructor(p: number) {}
    }

    new A(5) // create an instance and call constructor
    const a = new A(6) /* create an instance, call constructor and store
                          created and initialized instance in 'a' */


*Class instance creation expression* can throw an error
(see :ref:`Error Handling`, :ref:`Constructor Declaration`).

.. index::
   class instance creation expression
   instantiation
   instance
   constructor
   initialization

The execution of a class instance creation expression is performed as follows:

-  New instance of class is created;
-  Constructor of class is called to fully initialize the created
   instance.

The validity of the constructor call is similar to the validity of the method
call as discussed in :ref:`Step 2 Selection of Method`, except the cases
discussed in :ref:`Constructor Body`.

A :index:`compile-time error` occurs if ``typeReference`` is a type parameter.

**Note**. If the *class instance creation expression* with no arguments is used
as the object reference in the method call expression, then empty parentheses
'*()*' are to be used.

.. code-block:: typescript
   :linenos:

    class A {  method() {} }

    new A.method()   // compile-time error
    new A().method() // OK
    let a = new A    // OK


.. index::
   class instance creation expression
   instance
   instantiation
   constructor
   initialization
   type parameter
   method call expression
   parenthesis

|

.. _Cast Expressions:

``Cast`` Expressions
********************

.. meta:
    frontend_status: Done

*Cast expression* applies *cast operator* ``as`` to an *expression* by issuing
a value of a specified type.

.. code-block:: abnf

    castExpression:
        expression 'as' type
        ;

.. code-block:: typescript
   :linenos:

    class X {}

    let x1 : X = new X()
    let ob : Object = x1 as Object
    let x2 : X = ob as X

The cast operator converts the value *V* of one type (as denoted by the
expression) at runtime to a value of another type.

The cast expression introduces the target type for the casting context (see
:ref:`Casting Contexts and Conversions`). The target type can be either
``type`` or ``typeReference``.

.. index::
   cast operator
   cast expression
   expression
   conversion
   value
   runtime
   casting context
   type

Cast expression type is always the target type.

The result of a cast expression is a value, not a variable (even if the operand
expression is a variable).

A :index:`compile-time error` occurs if the cast operator cannot convert the
compile-time type of the operand to the target type specified by the cast
operator.

If the casting conversion cannot be performed during program execution, then
``ClassCastError`` is thrown.

.. index::
   cast expression
   target type
   value
   variable
   operand expression
   variable
   operand value
   cast operator
   casting conversion

|

.. _InstanceOf Expression:

``InstanceOf`` Expression
*************************

.. meta:
    frontend_status: Done

.. code-block:: abnf

    instanceOfExpression:
        expression 'instanceof' type
        ;

Any ``instanceof`` expression is of type ``boolean``.

The expression operand of the operator ``instanceof`` must be of a reference
type. Otherwise, a :index:`compile-time error` occurs.

A :index:`compile-time error` occurs if ``type`` operand of the operator
``instanceof`` is one of the following:

   - Type parameter (see :ref:`Type Parameters`),
   - Primitive type (see :ref:`Primitive Types`),
   - Union type that contains type parameter after normalization
     (see :ref:`Union Types Normalization`),
   - *Generic type* (see :ref:`Generics`)---this temporary limitation
     is expected to be removed in the future (see
     :ref:`Generic and Function Types Peculiarities`).

.. index::
   instanceof expression
   operand
   operator
   instanceof operator
   reference type
   type parameter
   primitive type
   union type
   normalization
   generic type

If type of ``expression`` at compile time is assignable to ``type`` (see
:ref:`Assignability`), then the result of an ``instanceof`` expression
is ``true``.

Otherwise, an ``instanceof`` expression checks during program execution
whether type of the value the ``expression`` successfully evaluates to is
assignable to ``type`` (see :ref:`Assignability`).
If so, then the result of an ``instanceof`` expression is ``true``.
Otherwise, the result is ``false``.

If the expression evaluation causes an error, then the execution control is
transferred to a proper ``catch`` section or runtime system, and the result of
an ``instanceof`` expression cannot be determined.

.. index::
   instanceof expression
   assignability
   execution
   evaluation
   assignable type
   catch section

|

.. _TypeOf Expression:

``TypeOf`` Expression
*********************

.. meta:
    frontend_status: Done

.. code-block:: abnf

    typeOfExpression:
        'typeof' expression
        ;

Any ``typeof`` expression is of type ``string``. Its evaluation starts with the
``expression`` evaluation. If this evaluation causes an error, then the result
of a ``typeof`` expression cannot be determined. Otherwise, the value of a
``typeof`` expression is defined as follows:

1. Types defined at compile time

.. index::
   typeof expression
   type string
   evaluation
   compile time

+---------------------------------+-------------------------+-----------------------------+
|     **Type of Expression**      |   **Resulting String**  | **Code Example**            |
+=================================+=========================+=============================+
| ``number``                      | "number"                | .. code-block:: typescript  |
|                                 |                         |                             |
|                                 |                         |  let n: number = ...        |
|                                 |                         |  typeof n                   |
+---------------------------------+-------------------------+-----------------------------+
| ``string``                      | "string"                | .. code-block:: typescript  |
|                                 |                         |                             |
|                                 |                         |  let s: string = ...        |
|                                 |                         |  typeof s                   |
+---------------------------------+-------------------------+-----------------------------+

(table cont'd)

+---------------------------------+-------------------------+-----------------------------+
|     **Type of Expression**      |   **Resulting String**  | **Code Example**            |
+=================================+=========================+=============================+
| ``boolean``                     | "boolean"               | .. code-block:: typescript  |
|                                 |                         |                             |
|                                 |                         |  let b: boolean = ...       |
|                                 |                         |  typeof b                   |
+---------------------------------+-------------------------+-----------------------------+
| ``bigint``                      | "bigint"                | .. code-block:: typescript  |
|                                 |                         |                             |
|                                 |                         |  let b: bigint = ...        |
|                                 |                         |  typeof b                   |
+---------------------------------+-------------------------+-----------------------------+
| any class or interface          | "object"                | .. code-block:: typescript  |
|                                 |                         |                             |
|                                 |                         |  let a: Object = ...        |
|                                 |                         |  typeof a                   |
+---------------------------------+-------------------------+-----------------------------+
| any function type               | "function"              | .. code-block:: typescript  |
|                                 |                         |                             |
|                                 |                         |  let f: () => void = ...    |
|                                 |                         |  typeof f                   |
+---------------------------------+-------------------------+-----------------------------+
| ``undefined``                   | "undefined"             | .. code-block:: typescript  |
|                                 |                         |                             |
|                                 |                         |  typeof undefined           |
+---------------------------------+-------------------------+-----------------------------+
| ``null``                        | "object"                | .. code-block:: typescript  |
|                                 |                         |                             |
|                                 |                         |  typeof null                |
+---------------------------------+-------------------------+-----------------------------+
| ``T|null``, when ``T`` is a     | "object"                | .. code-block:: typescript  |
| class (but not Object -         |                         |                             |
| see next table),                |                         |  class C {}                 |
| interface or array              |                         |  let x: C | null = ...      |
|                                 |                         |  typeof x                   |
+---------------------------------+-------------------------+-----------------------------+
| ``enum``                        | "number" or "string",   | .. code-block:: typescript  |
|                                 | depending of constant   |                             |
|                                 | type                    |  enum C {R, G, B}           |
|                                 |                         |  let c: C = ...             |
|                                 |                         |  typeof c                   |
+---------------------------------+-------------------------+-----------------------------+
| Numeric types:                  | "number"                | .. code-block:: typescript  |
|                                 |                         |                             |
|                                 |                         |  let x: byte = ...          |
| ``byte``, ``short``, ``int``,   |                         |  typeof x                   |
| ``long``, ``float``, ``double`` |                         |  ...                        |
+---------------------------------+-------------------------+-----------------------------+

|

2. All other types evaluated at runtime

+------------------------+-----------------------------+
| **Type of Expression** | **Code Example**            |
+========================+=============================+
| Object                 | .. code-block:: typescript  |
|                        |                             |
|                        |  function f(o: Object) {    |
|                        |    typeof o                 |
|                        |  }                          |
+------------------------+-----------------------------+
| union type             | .. code-block:: typescript  |
|                        |                             |
|                        |  function f(p:A|B) {        |
|                        |    typeof p                 |
|                        |  }                          |
+------------------------+-----------------------------+
| type parameter         | .. code-block:: typescript  |
|                        |                             |
|                        |  class A<T|null|undefined> {|
|                        |     f: T                    |
|                        |     m() {                   |
|                        |        typeof this.f        |
|                        |     }                       |
|                        |     constructor(p:T) {      |
|                        |        this.f = p           |
|                        |     }                       |
|                        |  }                          |
+------------------------+-----------------------------+

.. index::
   union type
   type parameter


|

.. _Ensure-Not-Nullish Expressions:

Ensure-Not-Nullish Expression
*****************************

.. meta:
    frontend_status: Done

.. code-block:: abnf

    ensureNotNullishExpression:
        expression '!'
        ;

An *ensure-not-nullish expression* is a postfix expression with the operator
'``!``'. An *ensure-not-nullish expression* in the expression *e!* checks
whether *e* of a nullish type (see :ref:`Nullish Types`) evaluates to a
nullish value.

If the expression *e* is not of a nullish type, then the operator '``!``'
has no effect.

If the result of the evaluation of *e* is not equal to ``null`` or ``undefined``,
then the result of *e!* is the outcome of the evaluation of *e*.

If the result of the evaluation of *e* is equal to ``null`` or ``undefined``,
then ``NullPointerError`` is thrown.

Type of *ensure-not-nullish* expression is the non-nullish variant of
type of *e*.

.. index::
   ensure-not-nullish expression
   postfix expression
   prefix expression
   expression
   operator
   nullish type
   evaluation
   non-nullish variant
   nullish value
   null
   undefined

|

.. _Nullish-Coalescing Expression:

Nullish-Coalescing Expression
*****************************

.. meta:
    frontend_status: Done

.. code-block:: abnf

    nullishCoalescingExpression:
        expression '??' expression
        ;

*Nullish-coalescing expression* is a binary expression that uses the operator
'``??``', and checks whether the evaluation of the left-hand-side expression
equals the *nullish* value:

-  If so, then the right-hand-side expression evaluation is the result
   of a nullish-coalescing expression.
-  If not so, then the result of the left-hand-side expression evaluation is
   the result of a nullish-coalescing expression, and the right-hand-side
   expression is not evaluated (the operator '``??``' is thus *lazy*).

.. index::
   nullish-coalescing expression
   binary expression
   operator
   evaluation
   expression
   nullish value
   lazy operator

If the left-hand-side expression is not of a nullish type, then type of the
expression is a nullish-coalescing expression. Otherwise, type of a
nullish-coalescing expression is a normalized *union type*
(see :ref:`Union Types`) formed from the following:

- Non-nullish variant of the type of the left-hand-side expression; and
- Type of the right-hand-side expression.

The semantics of a nullish-coalescing expression is represented in the
following example:

.. code-block:: typescript
   :linenos:

    let x = expression1 ?? expression2

    let x$ = expression1
    if (x$ == null) {x = expression2} else x = x$!

    // Type of x is NonNullishType(expression1)|Type(expression2)

A :index:`compile-time error` occurs if the nullish-coalescing operator is
mixed with conditional-and or conditional-or operators without parentheses.

.. index::
   nullish type
   nullish-coalescing expression
   union type
   non-nullish type
   expression
   nullish-coalescing operator
   conditional-and operator
   conditional-or operator

|

.. _Unary Expressions:

Unary Expressions
*****************

.. meta:
    frontend_status: Done

.. code-block:: abnf

    unaryExpression:
        expression '++'
        | expression '--'
        | '++' expression
        | '--' expression
        | '+' expression
        | '-' expression
        | '~' expression
        | '!' expression
        ;

All expressions with unary operators (except postfix increment and postfix
decrement operators) group right-to-left for '``~+x``' to have the same meaning
as '``~(+x)``'.

Type of any *unary Expression* is the type of the ``expression`` provided.

.. index::
   unary expression
   unary operator
   expression
   postfix
   postfix
   increment operator
   decrement operator

.. _Postfix Increment:

Postfix Increment
=================

.. meta:
    frontend_status: Done

*Postfix increment expression* is an *expression* followed by the increment
operator '``++``'.

The *expression* must be *left-hand-side expression*
(see :ref:`Left-Hand-Side Expressions`), so it denotes a variable.

A :index:`compile-time error` occurs if type of the
the *expression* is not convertible (see :ref:`Implicit Conversions`) to a
numeric type (see :ref:`Numeric Types`).

Type of a *postfix increment expression* is the type of the variable. The
result of a *postfix increment expression* is a value, not a variable.

If the evaluation of the operand *expression* completes normally at runtime,
then:

-  The value *1* is added to the value of the variable by using necessary
   conversions (see :ref:`Primitive Types Conversions`); and
-  The sum is stored back into the variable.

.. index::
   postfix
   increment expression
   increment operator
   expression
   conversion
   variable
   numeric type
   convertible expression
   value
   operand
   normal completion

Otherwise, the *postfix increment expression* completes abruptly, and no
incrementation occurs.

The  value of the *postfix increment expression* is the value of the variable
*before* the new value is stored.

.. index::
   variable
   conversion
   numeric types conversion
   postfix
   increment expression
   abrupt completion
   expression
   variable
   postfix increment expression
   incrementation

|

.. _Postfix Decrement:

Postfix Decrement
=================

.. meta:
   frontend_status: Done
   todo: let a : Double = Double.Nan; a++; a--; ++a; --a; (assertion)

*Postfix decrement expression* is an expression followed by the decrement
operator '``--``'.

The *expression* must be *left-hand-side expression* (see
:ref:`Left-Hand-Side Expressions`). The *expression* denotes a variable.

A :index:`compile-time error` occurs if type of
the *expression* is not convertible (see :ref:`Implicit Conversions`) to a
numeric type (see :ref:`Numeric Types`).

Type of a postfix decrement expression is the type of the variable. The
result of a postfix decrement expression is a value, not a variable.

If evaluation of the operand expression completes at runtime, then:

.. index::
   postfix
   decrement expression
   decrement operator
   variable
   expression
   conversion
   runtime
   operand
   completion
   evaluation

-  The value *1* is subtracted from the value of the variable by using
   necessary conversions (see :ref:`Primitive Types Conversions`); and
-  The sum is stored back into the variable.

Otherwise, the *postfix decrement expression* completes abruptly, and
no decrementation occurs.

The value of the *postfix decrement expression* is the value of the variable
*before* the new value is stored.

.. index::
   subtraction
   value
   variable
   conversion
   abrupt completion
   numeric types conversion
   abrupt completion
   decrementation
   decrement expression
   postfix
   variable
   value

|

.. _Prefix Increment:

Prefix Increment
================

.. meta:
    frontend_status: Done

*Prefix increment expression* is an expression preceded by the operator
'``++``'.

The *expression* must be *left-hand-side expression* (see
:ref:`Left-Hand-Side Expressions`). The *expression* denotes a variable.

A :index:`compile-time error` occurs if the type of
the *expression* is not convertible (see :ref:`Implicit Conversions`) to a
numeric type (see :ref:`Numeric Types`).

Type of a prefix increment expression is the type of the variable. The
result of a prefix increment expression is a value, not a variable.

If evaluation of the operand *expression* completes normally at runtime, then:

.. index::
   prefix
   increment operator
   increment expression
   expression
   variable
   expression
   normal completion
   conversion
   convertibility

-  The value *1* is added to the value of the variable by using necessary
   conversions (see :ref:`Primitive Types Conversions`); and
-  The sum is stored back into the variable.

Otherwise, the *prefix increment expression* completes abruptly, and no
incrementation occurs.

The  value of the *prefix increment expression* is the value of the variable
*before* the new value is stored.

.. index::
   value
   variable
   conversion
   predefined type
   conversion
   abrupt completion
   prefix
   increment expression

|

.. _Prefix Decrement:

Prefix Decrement
================

.. meta:
    frontend_status: Done

*Prefix decrement expression* is an expression preceded by the operator
'``--``'.

The *expression* must be *left-hand-side expression* (see
:ref:`Left-Hand-Side Expressions`). The *expression* denotes a variable.

A :index:`compile-time error` occurs if type of
the *expression* is not convertible (see :ref:`Implicit Conversions`) to a
numeric type (see :ref:`Numeric Types`).

Type of a prefix decrement expression is the type of the variable. The
result of a prefix decrement expression is a value, not a variable.

.. index::
   prefix
   decrement operator
   decrement expression
   expression
   operator
   variable
   expression
   value

If evaluation of the operand *expression* completes normally at runtime, then:

-  The value *1* is subtracted from the value of the variable by using
   necessary conversions (see :ref:`Primitive Types Conversions`); and
-  The sum is stored back into the variable.

Otherwise, the *prefix decrement expression* completes abruptly, and no
decrementation occurs. The value of the *prefix decrement expression* remains
the value of the variable *before* a new value is stored.

.. index::
   evaluation
   runtime
   expression
   normal completion
   conversion
   decrement expression
   decrementation
   abrupt completion
   variable
   value
   prefix

|

.. _Unary Plus:

Unary Plus
==========

.. meta:
    frontend_status: Done

*Unary plus expression* is an expression preceded by the operator '``+``'.

Type of the operand *expression* with the unary operator '``+``' must
be convertible  (see :ref:`Implicit Conversions`) to a numeric type (see
:ref:`Numeric Types`). Otherwise, a :index:`compile-time error` occurs.

The numeric types conversion (see :ref:`Primitive Types Conversions`) is
performed on the operand to ensure that the resultant type is that of the
unary plus expression. The result of a unary plus expression is always a value,
not a variable (even if the result of the operand expression is a variable).

Type of the *unary plus expression* is the type of the expression provided.

.. index::
   unary plus operator
   operand
   expression
   unary operator
   conversion
   numeric type
   numeric types conversion
   unary plus
   operator
   value
   variable

|

.. _Unary Minus:

Unary Minus
===========

.. meta:
    frontend_status: Done
    todo: let a : Double = Double.Nan; a = -a; (assertion)

*Unary minus expression* is an expression preceded by the operator '``-``'.

Type of the operand *expression* with the unary operator '``-``' must
be convertible (see :ref:`Implicit Conversions`) to a numeric type (see
:ref:`Numeric Types`). Otherwise, a :index:`compile-time error` occurs.

The numeric types conversion (see :ref:`Primitive Types Conversions`)
is performed on the operand to ensure that the resultant type is that of the
unary minus expression.
The result of a unary minus expression is a value, not a variable (even if the
result of the operand expression is a variable).

A unary numeric promotion performs the value set conversion (see
:ref:`Implicit Conversions`).

The unary negation operation is always performed on, and the result is drawn
from the same value set as the promoted operand value.

Type of the *unary minus expression* is the type of the expression provided.


.. index::
   unary minus
   operand
   unary operator
   operator
   conversion
   convertibility
   numeric type
   numeric types conversion
   expression
   operand
   normal completion
   value
   variable
   unary numeric promotion
   value set conversion
   unary negation operation
   promoted operand value

Further value set conversions are then performed on the same result.

The value of a unary minus expression at runtime is the arithmetic negation
of the promoted value of the operand.

The negation of integer values is the same as subtraction from zero. The |LANG|
programming language uses two’s-complement representation for integers. The
range of two’s-complement value is not symmetric. The same maximum negative
number results from the negation of the maximum negative *int* or *long*.
In that case, an overflow occurs but throws no error. For any integer value
*x*, *-x* is equal to *(~x)+1*.

The negation of floating-point values is *not* the same as subtraction from
zero (if *x* is *+0.0*, then *0.0-x* is *+0.0*, however *-x* is *-0.0*).

A unary minus merely inverts the sign of a floating-point number. Special
cases to consider are as follows:

-  Operand ``NaN`` results in ``NaN`` (``NaN`` has no sign).
-  Operand infinity results in the infinity of the opposite sign.
-  Operand zero results in zero of the opposite sign.

.. index::
   value set conversion
   conversion
   unary minus
   negation
   promoted value
   operand
   operation
   integer value
   subtraction
   two’s-complement representation
   two’s-complement value
   overflow
   floating-point value
   subtraction
   floating-point number
   infinity
   NaN

|

.. _Bitwise Complement:

Bitwise Complement
==================

.. meta:
    frontend_status: Done

*Bitwise complement expression* is an expression preceded by the operator '``~``'.

Type of the operand *expression* with the unary operator '``~``' must be
convertible (see :ref:`Implicit Conversions`) to a primitive integer type.
Otherwise, a :index:`compile-time error` occurs.

Thus the type of the *Bitwise complement expression* is a primitive integer.

The numeric types conversion (see :ref:`Primitive Types Conversions`)
is performed on the operand to ensure that the resultant type is that of the
unary bitwise complement expression.

The result of a unary bitwise complement expression is a value, not a variable
(even if the result of the operand expression is a variable).

The value of a unary bitwise complement expression at runtime is the bitwise
complement of the promoted value of the operand. In all cases, *~x* equals
*(-x)-1*.

.. index::
   bitwise complement expression
   numeric type
   numeric types conversion
   operator
   complement operator
   expression
   operand
   unary operator
   conversion
   convertibility
   primitive type
   integer type
   unary bitwise complement expression
   variable
   runtime
   promoted value

|

.. _Logical Complement:

Logical Complement
==================

.. meta:
    frontend_status: Done

*Logical complement expression* is an expression preceded by the operator
'``!``'.

Type of the operand *expression* with the unary '``!``' operator must be
``boolean`` or type mentioned in
:ref:`Extended Conditional Expressions`. Otherwise, a
:index:`compile-time error` occurs.

The unary logical complement expression’s type is ``boolean``.

The value of a unary logical complement expression is ``true`` if the (possibly
converted) operand value is ``false``, and ``false`` if the operand value
(possibly converted) is ``true``.

.. index::
   logical complement operator
   expression
   operand
   operand value
   operator
   unary operator
   boolean type
   compile-time error
   unary logical complement expression
   predefined numeric types conversion

|

.. _Multiplicative Expressions:

Multiplicative Expressions
**************************

.. meta:
    frontend_status: Done

Multiplicative expressions use *multiplicative operators* '``*``', '``/``',
and '``%``':

.. code-block:: abnf

    multiplicativeExpression:
        expression '*' expression
        | expression '/' expression
        | expression '%' expression
        ;

Multiplicative operators group left-to-right.

Type of each operand in a multiplicative operator must be convertible (see
:ref:`Contexts and Conversions`) to a numeric type (see :ref:`Numeric Types`).
Otherwise, a :index:`compile-time error` occurs.

The numeric types conversion (see :ref:`Primitive Types Conversions`)
is performed on both operands to ensure that the resultant type is the type of
the multiplicative expression.

The result of a unary bitwise complement expression is a value, not a
variable (even if the operand expression is a variable).

.. index::
   multiplicative expression
   conversion
   convertibility
   context
   conversion
   numeric type
   multiplicative operator
   multiplicative expression
   numeric type
   value
   unary bitwise complement expression
   operand expression
   variable
   numeric type
   numeric types conversion
   multiplicative operator
   operand expression

|

.. _Multiplication:

Multiplication
==============

.. meta:
    frontend_status: Done
    todo: If either operand is NaN, the result should be NaN, but result is -NaN
    todo: Multiplication of an infinity by a zero should be NaN, but result is - NaN

The binary operator '``*``' performs multiplication, and returns the product of
its operands.

Multiplication is a commutative operation if operand expressions have no
side effects.

Integer multiplication is associative when all operands are of the same type.

Floating-point multiplication is not associative.

Type of a *multiplication expression* is the 'heaviest' (see
:ref:`Numeric Types`) type of its operands.

If overflow occurs during integer multiplication, then:

-  The result is the low-order bits of the mathematical product as represented
   in some sufficiently large two’s-complement format.
-  The sign of the result can be other than the sign of the mathematical
   product of the two operand values.

A floating-point multiplication result is determined in compliance with the
IEEE 754 arithmetic:

.. index::
   multiplication
   binary operator
   multiplication
   operand
   commutative operation
   expression
   side effect
   integer multiplication
   associativity
   two’s-complement format
   floating-type multiplication
   operand value
   low-order bit
   IEEE 754
   overflow

-  The result is ``NaN`` if:

   -  Either operand is ``NaN``;
   -  Infinity is multiplied by zero.

-  If the result is not ``NaN``, then the sign of the result is as follows:

   -  Positive, where both operands have the same sign; and
   -  Negative, where the operands have different signs.

-  If infinity is multiplied by a finite value, then the multiplication results
   in a signed infinity (the sign is determined by the rule above).
-  If neither ``NaN`` nor infinity is involved, then the exact mathematical
   product is computed.

   The product is rounded to the nearest value in the chosen value set by
   using the IEEE 754 *round-to-nearest* mode. The |LANG| programming
   language requires gradual underflow support as defined by IEEE 754
   (see :ref:`Floating-Point Types and Operations`).

   If the magnitude of the product is too large to represent, then the
   operation overflows, and the result is an appropriately signed infinity.

The evaluation of a multiplication operator '``*``' never throws an error
despite possible overflow, underflow, or loss of information.

.. index::
   NaN
   infinity
   operand
   finite value
   multiplication
   signed infinity
   round-to-nearest
   rounding
   underflow
   floating-point type
   floating-point operation
   overflow
   evaluation
   multiplication operator
   error
   loss of information
   IEEE 754
   rounding

|

.. _Division:

Division
========

.. meta:
   frontend_status: Done
   todo: If either operand is NaN, the result should be NaN, but result is -NaN
   todo: Division of infinity by infinity should be NaN, but result is - NaN
   todo: Division of a nonzero finite value by a zero results should be signed infinity, but "Floating point exception(core dumped)" occurs

The binary operator '``/``' performs division and returns the quotient of its
left-hand-side and right-hand-side operands (``dividend`` and ``divisor``
respectively).

Integer division rounds toward *0*, i.e., the quotient of integer operands
*n* and *d*, after a numeric types conversion on both (see
:ref:`Primitive Types Conversions` for details), is
the integer value *q* with the largest possible magnitude that
satisfies :math:`|d\cdot{}q|\leq{}|n|`.

**Note**. The integer value *q* is:

-  Positive, where \|n| :math:`\geq{}` \|d|, and *n* and *d* have the same sign;
   but
-  Negative, where \|n| :math:`\geq{}` \|d|, and *n* and *d* have opposite signs.

.. index::
   division operator
   binary operator
   operand
   dividend
   divisor
   round-toward-zero
   integer division
   integer operand
   numeric types conversion
   numeric type
   integer value

Only a single special case does not comply with this rule: the integer overflow
occurs, and the result equals the dividend if the dividend is a negative
integer of the largest possible magnitude for its type, while the divisor
is *-1*. No error is thrown in this case despite the overflow. However, if the
divisor value is *0* in an integer division, then ``ArithmeticError`` is
thrown.

The result of a floating-point division is determined in compliance with the
IEEE 754 arithmetic:

-  The result is ``NaN`` if:

   -  Either operand is NaN;
   -  Both operands are infinity; or
   -  Both operands are zero.

.. index::
   integer overflow
   dividend
   negative integer
   floating-point division
   divisor
   overflow
   integer division
   floating-point division
   NaN
   infinity
   operand
   IEEE 754

-  If the result is not ``NaN``, then the sign of the result is:

   -  Positive, where both operands have the same sign; or
   -  Negative, where the operands have different signs.

-  Division produces a signed infinity (the sign is determined by
   the rule above) if:

   -  Infinity is divided by a finite value; and
   -  A nonzero finite value is divided by zero.

-  Division produces a signed zero (the sign is determined by the
   rule above) if:

   -  A finite value is divided by infinity; and
   -  Zero is divided by any other finite value.

.. index::
   NaN
   operand
   division
   signed infinity
   finite value

-  If neither ``NaN`` nor infinity is involved, then the exact mathematical
   quotient is computed.

   If the magnitude of the product is too large to represent, then the
   operation overflows, and the result is an appropriately signed infinity.

The quotient is rounded to the nearest value in the chosen value set by
using the IEEE 754 *round-to-nearest* mode. The |LANG| programming
language requires gradual underflow support as defined by IEEE 754 (see
:ref:`Floating-Point Types and Operations`).

The evaluation of a floating-point division operator '``/``' never throws an
error despite possible overflow, underflow, division by zero, or loss of
information.

The type of the *division expression* is an integer or a floating-point number.


.. index::
   infinity
   NaN
   overflow
   floating-point division
   round-to-nearest
   rounding
   underflow
   floating-point type
   floating-point operation
   loss of information
   division
   division operator
   IEEE 754

|

.. _Remainder:

Remainder
=========

.. meta:
    frontend_status: Done
    todo: If either operand is NaN, the result should be NaN, but result is -NaN
    todo: if the dividend is an infinity, or the divisor is a zero, or both, the result should be NaN, but this is -NaN

The binary operator '``%``' yields the remainder of its operands (``dividend``
as the left-hand-side, and ``divisor`` as the right-hand-side operand) from an
implied division.

The remainder operator in |LANG| accepts floating-point operands (unlike in
C and C++).

The remainder operation on integer operands produces a result value, i.e.,
:math:`(a/b)*b+(a\%b)` equals *a*. The numeric type conversion on remainder
operation is discussed in :ref:`Primitive Types Conversions`.

.. index::
   binary operator
   remainder operator
   dividend
   divisor
   division
   numeric types conversion
   conversion
   floating-point operand
   remainder operation
   value
   integer operand
   numeric type

This equality holds even in the special case where the dividend is a negative
integer of the largest possible magnitude of its type, and the divisor is *-1*
(the remainder is then *0*). According to this rule, the result of the remainder
operation can only be one of the following:

-  Negative if the dividend is negative; or
-  Positive if the dividend is positive.

The magnitude of the result is always less than that of the divisor.

If the value of the divisor for an integer remainder operator is *0*, then
``ArithmeticError`` is thrown.

The result of a floating-point remainder operation as computed by the operator
'``%``' is different than that produced by the remainder operation defined by
IEEE 754. The IEEE 754 remainder operation computes the remainder from a rounding
division (not a truncating division), and its behavior is different from that
of the usual integer remainder operator. On the contrary, |LANG| presumes that
the operator '``%``' behaves on floating-point operations in the same manner as
the integer remainder operator (comparable to the C library function *fmod*).
The standard library (see :ref:`Standard Library`) routine ``Math.IEEEremainder``
can compute the IEEE 754 remainder operation.

.. index::
   dividend
   equality
   magnitude
   negative integer
   divisor
   remainder operator
   remainder operation
   truncation
   integer remainder
   value
   floating-point remainder operation
   floating-point operation
   division
   truncation
   rounding
   routine
   IEEE 754

The result of a floating-point remainder operation is determined in compliance
with the IEEE 754 arithmetic:

-  The result is ``NaN`` if:

   -  Either operand is ``NaN``;
   -  The dividend is infinity;
   -  The divisor is zero; or
   -  The dividend is infinity, and the divisor is zero.

-  If the result is not ``NaN``, then the sign of the result is the same as the
   sign of the dividend.
-  The result equals the dividend if:

   -  The dividend is finite, and the divisor is infinity; or
   -  If the dividend is zero, and the divisor is finite.

.. index::
   floating-point remainder operation
   remainder operation
   NaN
   infinity
   divisor
   dividend
   IEEE 754

-  If infinity, zero, or ``NaN`` are not involved, then the floating-point remainder
   *r* from the division of the dividend *n* by the divisor *d* is determined
   by the mathematical relation :math:`r=n-(d\cdot{}q)`, where *q* is an
   integer that is only:

   -  Negative if :math:`n/d` is negative, or
   -  Positive if :math:`n/d` is positive.

-  The magnitude of *q* is the largest possible without exceeding the
   magnitude of the true mathematical quotient of *n* and *d*.

The evaluation of the floating-point remainder operator '``%``' never throws
an error, even if the right-hand operand is zero. Overflow, underflow, or
loss of precision cannot occur.

The type of the *remainder expression* is an integer or floating-point number.


.. index::
   infinity
   NaN
   floating-point remainder
   remainder operator
   dividend
   integer
   loss of precision
   operand
   magnitude
   underflow
   error
   overflow
   loss of precision

|

.. _Additive Expressions:

Additive Expressions
********************

.. meta:
    frontend_status: Done

Additive expressions use *additive operators* '``+``' and '``-``':

.. code-block:: abnf

    additiveExpression:
        expression '+' expression
        | expression '-' expression
        ;

Additive operators group left-to-right.

If either operand of the operator is '``+``' of type ``string``, then the
operation is a string concatenation (see :ref:`String Concatenation`). In all
other cases, type of each operand of the operator '``+``' must be
convertible (see :ref:`Primitive Types Conversions`) to a numeric type (see
:ref:`Numeric Types`). Otherwise, a :index:`compile-time error` occurs.

Type of each operand of the binary operator '``-``' must be convertible
(see :ref:`Primitive Types Conversions`) to a numeric type (see
:ref:`Numeric Types`) in all cases. Otherwise, a :index:`compile-time error`
occurs.

Type of *Additive expression* is ``string`` or the 'heaviest' (see
:ref:`Numeric Types`) type of its operands.


.. index::
   additive expression
   additive operator
   operand
   string
   string concatenation
   operator
   conversion
   numeric type
   binary operator

|

.. _String Concatenation:

String Concatenation
====================

.. meta:
    frontend_status: Done

If one operand of an expression is of type ``string``, then the string
conversion (see :ref:`String Operator Contexts`) is performed on the other
operand at runtime to produce a string.

String concatenation produces a reference to a ``string`` object that is a
concatenation of two operand strings. The left-hand-side operand characters
precede the right-hand-side operand characters in a newly created string.

If the expression is not a constant expression (see :ref:`Constant Expressions`),
then a new ``string`` object is created (see :ref:`New Expressions`).

.. index::
   string concatenation
   string
   operand
   string conversion
   operator context
   runtime
   operand string
   expression
   constant expression
   object

|

.. _Additive Operators for Numeric Types:

Additive Operators for Numeric Types
====================================

.. meta:
   frontend_status: Done
   todo: The sum of two infinities of opposite sign should be NaN, but it is -NaN

The primitive types conversion (see :ref:`Primitive Types Conversions`)
performed on a pair of operands ensures that both operands are of a numeric
type. If the conversion fails, then a :index:`compile-time error` occurs.

The binary operator '``+``' performs addition and produces the sum of such
operands.

The binary operator '``-``' performs subtraction and produces the difference
of two numeric operands.

Type of an additive expression performed on numeric operands is the
largest type (see :ref:`Numeric Types`) to which operands of that
expression are converted.

If the promoted type is ``int`` or ``long``, then integer arithmetic is
performed.
If the promoted type is ``float`` or ``double``, then floating-point arithmetic
is performed.

.. index::
   additive operator
   primitive type
   primitive types conversion
   conversion
   numeric type
   numeric operand
   binary operator
   promoted type
   integer arithmetic
   floating-point arithmetic
   integer
   type operand
   addition
   subtraction
   expression

If operand expressions have no side effects, then addition is a commutative
operation.

If all operands are of the same type, then integer addition is associative.

Floating-point addition is not associative.

If overflow occurs on an integer addition, then:

-  Result is the low-order bits of the mathematical sum as represented in
   a sufficiently large two’s-complement format.
-  Sign of the result is different than that of the mathematical sum of
   the operands’ values.

The result of a floating-point addition is determined in compliance with the
IEEE 754 arithmetic as follows:

.. index::
   operand expression
   expression
   side effect
   addition
   commutative operation
   operation
   low-order bit
   two’s-complement format
   operand value
   overflow
   floating-point addition
   associativity
   IEEE 754

-  The result is ``NaN`` if:

   -  Either operand is ``NaN``; or
   -  The operands are two infinities of the opposite signs.

-  The sum of two infinities of the same sign is the infinity of that sign.
-  The sum of infinity and a finite value equals the infinite operand.
-  The sum of two zeros of opposite sign is positive zero.
-  The sum of two zeros of the same sign is zero of that sign.
-  The sum of zero and a nonzero finite value is equal to the nonzero operand.
-  The sum of two nonzero finite values of the same magnitude and opposite sign
   is positive zero.
-  If infinity, zero, or ``NaN`` are not involved, and the operands have the
   same sign or different magnitudes, then the exact sum is computed
   mathematically.

If the magnitude of the sum is too large to represent, then the operation
overflows. The result is an appropriately signed infinity.

.. index::
   NaN
   infinity
   signed infinity
   magnitude
   operand
   infinite operand
   infinite value
   nonzero operand
   finite value
   positive zero
   negative zero
   overflow
   operation overflow

Otherwise, the sum is rounded to the nearest value within the chosen value set
by using the IEEE 754 *round-to-nearest* mode. The |LANG| programming language
requires gradual underflow support as defined by IEEE 754 (see
:ref:`Floating-Point Types and Operations`).

When applied to two numeric type operands (see :ref:`Numeric Types`), the
binary operator '``-``' performs subtraction, and returns the difference of
such operands (``minuend`` as left-hand-side, and ``subtrahend`` as the
right-hand-side operand).

The result of *a-b* is always the same as that of *a+(-b)* in both integer and
floating-point subtraction.

The subtraction from zero for integer values is the same as negation. However,
the subtraction from zero for floating-point operands and negation is *not*
the same (if *x* is *+0.0*, then *0.0-x* is *+0.0*; however *-x* is *-0.0*).

The evaluation of a numeric additive operator never throws an error despite
possible overflow, underflow, or loss of information.

.. index::
   round-to-nearest mode
   rounding
   value set
   underflow
   floating-point type
   floating-point operation
   floating-point subtraction
   floating-point operand
   subtraction
   integer subtraction
   integer value
   loss of information
   numeric type operand
   binary operator
   subtraction
   negation
   overflow
   additive operator
   error
   IEEE 754

|

.. _Shift Expressions:

Shift Expressions
*****************

.. meta:
    frontend_status: Done
    todo: spec issue: uses 'L' postfix in example "(n >> s) + (2L << ~s)", we don't have it

*Shift expressions* use *shift operators* '``<<``' (left shift), '``>>``'
(signed right shift), and '``>>>``' (unsigned right shift). The value to be
shifted is the left-hand-side operand in a shift operator, and the
right-hand-side operand specifies the shift distance:

.. code-block:: abnf

    shiftExpression:
        expression '<<' expression
        | expression '>>' expression
        | expression '>>>' expression
        ;

Shift operators group left-to-right.

Numeric types conversion (see :ref:`Primitive Types Conversions`) is performed
separately on each operand to ensure that both operands are of primitive
integer type.

Thus, *shift expression* is of primitive integer type.

**Note**. If the initial type of one or both operands is ``double`` or
``float``, then such operand or operands are  first truncated to the appropriate
integer type. If both operands are of type ``bigint``, then shift operator
is applied to bigint operands.

A :index:`compile-time error` occurs if either operand in a shift operator
(after unary numeric promotion) is not a primitive ``integer`` or ``bigint``
type.

.. index::
   shift
   shift expression
   shift distance
   shift operator
   signed right shift
   unsigned right shift
   operand
   shift distance
   numeric type
   predefined numeric types conversion
   numeric types conversion
   unary numeric promotion
   truncation
   truncated operand
   primitive type
   integer type
   bigint

The shift expression type is the promoted type of the left-hand-side operand.

If the left-hand-side operand is of the promoted type ``int``, then only five
lowest-order bits of the right-hand-side operand specify the shift distance
(as if using a bitwise logical AND operator '``&``' with the mask value *0x1f*
or *0b11111* on the right-hand-side operand). Thus, it is always within the
inclusive range of *0* through *31*.

If the left-hand-side operand is of the promoted type ``long``, then only six
lowest-order bits of the right-hand-side operand specify the shift distance
(as if using a bitwise logical AND operator '``&``' with the mask value *0x3f*
or *0b111111* the right-hand-side operand). Thus, it is always within the
inclusive range of *0* through *63*.

Shift operations are performed on the two’s-complement integer
representation of the value of the left-hand-side operand at runtime.

The value of *n* ``<<`` *s* is *n* left-shifted by *s* bit positions. It is
equivalent to multiplication by two to the power *s* even in case of an
overflow.

.. index::
   shift expression
   promoted type
   operand
   shift distance
   bitwise logical AND operator
   mask value
   value
   truncation
   integer division
   shift operation
   multiplication
   overflow
   two’s-complement integer
   left shift
   runtime
   zero-extension
   shift

The value of *n* ``>>`` *s* is *n* right-shifted by *s* bit positions with
sign-extension. The resultant value is :math:`floor(n / 2s)`. If *n* is
non-negative, then it is equivalent to truncating integer division (as computed
by the integer division operator by 2 to the power *s*).

The value of *n* ``>>>`` *s* is *n* right-shifted by *s* bit positions with
zero-extension, where:

-  If *n* is positive, then the result is the same as that of *n* ``>>`` *s*.
-  If *n* is negative, and type of the left-hand-side operand is ``int``,
   then the result is equal to that of the expression (*n* ``>>`` *s*) ``+`` (*2* ``<<`` *s*).
-  If *n* is negative, and type of the left-hand-side operand is ``long``,
   then the result is equal to that of the expression (*n* ``>>`` *s*) ``+`` (*2L* ``<<`` *s*).

.. index::
   value
   sign-extension
   right shift
   truncation
   integer division
   operator
   zero-extension
   operand
   expression

|

.. _Relational Expressions:

Relational Expressions
**********************

.. meta:
    frontend_status: Done
    todo: if either operand is NaN, then the result should be false, but Double.NaN < 2 is true, and assertion fail occurs with opt-level 2. (also fails with INF)
    todo: Double.POSITIVE_INFINITY > 1 should be true, but false (also fails with opt-level 2)

Relational expressions use *relational operators* '``<``', '``>``', '``<=``',
and '``>=``'.

.. code-block:: abnf

    relationalExpression:
        expression '<' expression
        | expression '>' expression
        | expression '<=' expression
        | expression '>=' expression
        ;

Relational operators group left-to-right.

A relational expression is always of type ``boolean``.

Two kinds of relational expressions are described below. The kind of a
relational expression depends on types of operands. It is a
:index:`compile-time error` if at least one type of operands is different from
types described below.

.. index::
   numerical relational operator
   relational operator
   relational expression
   boolean type
   expression
   operand
   type

|

.. _Numerical Relational Operators:

Numerical Relational Operators
==============================

.. meta:
    frontend_status: Done

Type of each operand in a numerical relational operator must be convertible
(see :ref:`Implicit Conversions`) to a numeric type (see :ref:`Numeric Types`).
Otherwise, a :index:`compile-time error` occurs.

Numeric types conversions (see :ref:`Primitive Types Conversions`) are
performed on each operand as follows:

-  Signed integer comparison if the converted type of the operand is
   ``int`` or ``long``.

-  Floating-point comparison if the converted type of the operand is
   ``float`` or ``double``.

.. index::
   numerical relational operator
   operand
   conversion
   numeric type
   numeric types conversion
   predefined numeric types conversion
   signed integer comparison
   floating-point comparison
   converted type

The comparison of floating-point values drawn from any value set must be accurate.

A floating-point comparison must be performed in accordance with the IEEE 754
standard specification as follows:

-  The result of a floating-point comparison is false if either operand is ``NaN``.

-  All values other than ``NaN`` must be ordered with the following:

   -  Negative infinity less than all finite values; and
   -  Positive infinity greater than all finite values.

-  Positive zero equals negative zero.

.. index::
   floating-point value
   floating-point comparison
   comparison
   NaN
   finite value
   infinity
   negative infinity
   positive infinity
   positive zero
   negative zero
   IEEE 754

Based on the above presumption, the following rules apply to integer operands,
or floating-point operands other than ``NaN``:

-  The value produced by the operator '``<``' is ``true`` if the value of the
   left-hand-side operand is less than that of the right-hand-side operand.
   Otherwise, the value is ``false``.
-  The value produced by the operator '``<=``' is ``true`` if the value of the
   left-hand-side operand is less than or equal to that of the right-hand-side
   operand. Otherwise, the value is ``false``.
-  The value produced by the operator '``>``' is ``true`` if the value of the
   left-hand-side operand is greater than that of the right-hand-side operand.
   Otherwise, the value is ``false``.
-  The value produced by the operator '``>=``' is ``true`` if the value of the
   left-hand-side operand is greater than or equal to that of the right-hand-side
   operand. Otherwise, the value is ``false``.

.. index::
   integer operand
   floating-point operand
   NaN
   operator
   value

|

.. _String Relational Operators:

String Relational Operators
===========================

.. meta:
    frontend_status: Done

Results of all string comparisons are defined as follows:

-  Operator '``<``' delivers ``true`` if the string value of the left-hand-side
   operand is lexicographically less than the string value of the right-hand-side
   operand, or ``false`` otherwise.
-  Operator '``<=``' delivers ``true`` if the string value of the left-hand-side
   operand is lexicographically less than or equal to the string value of the
   right-hand-side operand, or ``false`` otherwise.
-  Operator '``>``' delivers ``true`` if the string value of the left-hand-side
   operand is lexicographically greater than the string value of the
   right-hand-side operand, or ``false`` otherwise.
-  Operator '``>=``' delivers ``true`` if the string value of the left-hand-side
   operand is lexicographically greater than or equal to the string value of
   the right-hand operand, or ``false`` otherwise.

.. index::
   operator
   string comparison
   string value

|

.. _Bigint Relational Operators:

Bigint Relational Operators
===========================

.. meta:
    frontend_status: Done

Type of each operand in a bigint relational operator must be ``bigint``.
Otherwise, a :index:`compile-time error` occurs.

All bigint relational operators compare bigint values.

.. index::
   bigint comparison
   relational operator
   bigint
   operand
   operator
   value

|

.. _Boolean Relational Operators:

Boolean Relational Operators
============================

.. meta:
    frontend_status: Done

Results of all boolean comparisons are defined as follows:

-  Operator '``<``' delivers ``true`` if the left-hand-side operand is ``false``
   and the right-hand-side operand is true, or ``false`` otherwise.
-  Operator '``<=``' delivers ``true`` if the left-hand-side operand is ``false``
   and the right-hand-side operand is ``true`` or ``false``, or ``false`` otherwise.
-  Operator '``>``' delivers ``true`` if the left-hand-side operand is ``true``
   and the right-hand-side operand is ``false``, or ``false`` otherwise.
-  Operator '``>=``' delivers ``true`` if the left-hand-side operand is ``true``
   and the right-hand-side operand is ``false`` or ``true``, or ``false`` otherwise.

.. index::
   operator
   operand
   relational operator
   boolean comparison

|

.. _Enumeration Relational Operators:

Enumeration Relational Operators
================================

.. meta:
    frontend_status: Done

If both operands are of the same Enumeration type (see :ref:`Enumerations`),
then :ref:`Numerical Relational Operators` or :ref:`String Relational Operators`
are used depending on the kind of enumeration constant value
( :ref:`Enumeration Integer Values` or :ref:`Enumeration String Values`).
Otherwise, a :index:`compile-time error` occurs.

.. index::
   enumeration operator
   enumeration constant
   value
   string value
   relational operator
   boolean comparison
   constant value

|

.. _Equality Expressions:

Equality Expressions
********************

.. meta:
    frontend_status: Done

Equality expressions use *equality operators* '``==``', '``===``', '``!=``',
and '``!==``':

.. code-block:: abnf

    equalityExpression:
        expression ('==' | '===' | '!=' | '!==') expression
        ;

Any equality expression is of type ``boolean``. The result of operators '``==``'
and '``===``' is ``true`` if operands are *equal* as shown below. Otherwise, the
result is ``false``.

Equality operators group left-to-right.
Equality operators are commutative if operand expressions cause no side
effects.

Equality operators are similar to relational operators, except for their
lower precedence (:math:`a < b==c < d` is ``true`` if both :math:`a < b`
and :math:`c < d` have the same ``truth`` value).

.. index::
   equality operator
   equality expression
   boolean type
   relational operator

In all cases, ``a != b`` produces the same result as ``!(a == b)``, and
``a !== b`` produces the same result as ``!(a === b)``.

The result of operators '``==``' and '``===``' is the same in all cases,
except when comparing the values ``null`` and ``undefined`` (see
:ref:`Reference Equality`).

The variant of equality evaluation to be used depends on types of the
operands used as follows:

-  *Value equality* is applied to entities of primitive types
   (see :ref:`Primitive Types`),
   type ``string`` (see :ref:`Type string`), type ``bigint`` (see
   :ref:`Type bigint`), and enumeration types (see :ref:`Enumerations`).
-  *Reference Equality based on actual (dynamic) type* is applied to values of
   type ``Object`` (:ref:`Type Object`), values of union types
   (:ref:`Union Types`), and type parameters (:ref:`Type Parameters`).
-  *Reference equality* is applied to entities of all other reference types
   (see :ref:`Reference Types`).

.. index::
   operator
   value
   value equality
   primitive type
   value type
   enumeration type
   bigint
   reference equality
   object
   type parameter

Operators '``===``' and '``==``', or '``!==``' and '``!=``' are used for:

- :ref:`Numerical Equality Operators` if operands are of numeric types (see
  :ref:`Numeric Types`), or type ``char``;

- :ref:`String Equality Operators` if both operands are of type ``string``;

- :ref:`Bigint Equality Operators` if both operands are of type ``bigint``;

- :ref:`Boolean Equality Operators` if both operands are of type ``boolean``;

- :ref:`Enumeration Equality Operators` if both operands are of enumeration type;

- :ref:`Reference Equality based on actual type` if at least one operand is of
  ``Object`` type or union type, or is a type parameter;

- :ref:`Reference Equality` if both operands are of compatible reference types,
  except types ``string``, ``bigint``, ``Object``, union types, and type
  parameters;

- :ref:`Extended Equality with null or undefined` if one operand is ``null`` or
  ``undefined``.

- Otherwise, a :index:`compile-time error` occurs.

.. code-block:: typescript
   :linenos:

   // Entities of value types are not comparable between each other
   5 == "5" // compile-time error
   5 == true // compile-time error
   "5" == true // compile-time error

.. index::
   value equality
   comparison
   operand
   operator
   numerical equality
   enumeration
   equality operator
   numeric type
   string
   equality operator
   boolean type
   type parameter
   object
   reference type
   union type
   type parameter

|

.. _Numerical Equality Operators:

Numerical Equality Operators
============================

.. meta:
    frontend_status: Done

Type of each operand in a numerical equality operator must be convertible
(see :ref:`Implicit Conversions`) to a numeric type (see :ref:`Numeric Types`).
Otherwise, a :index:`compile-time error` occurs.

A widening conversion can occur (see :ref:`Widening Primitive Conversions`)
if type of one operand is smaller than type of the other operand (see
:ref:`Numeric Types`).

If the converted type of the operands is ``int`` or ``long``, then an
integer equality test is performed.

If the converted type is ``float`` or ``double``, then a floating-point
equality test is performed.

The floating-point equality test must be performed in accordance with the
following IEEE 754 standard rules:

.. index::
   numerical equality
   value equality
   operator
   numeric type
   numeric types conversion
   converted type
   floating-point equality test
   operand
   conversion
   integer equality test
   IEEE 754
   widening
   primitive conversion

-  The result of '``==``' or '``===``' is ``false`` but the result of '``!=``'
   is ``true`` if either operand is ``NaN``.

   The test ``x != x`` or ``x !== x`` is ``true`` only if *x* is ``NaN``.

-  Positive zero equals negative zero.

-  Equality operators consider two distinct floating-point values unequal
   in any other situation.

   For example, if one value represents positive infinity, and the other
   represents negative infinity, then each compares equal to itself and
   unequal to all other values.

Based on the above presumptions, the following rules apply to integer operands
or floating-point operands other than ``NaN``:

-  If the value of the left-hand-side operand is equal to that of the
   right-hand-side operand, then the operator '``==``' or '``===``' produces
   the value ``true``. Otherwise, the result is ``false``.

-  If the value of the left-hand-side operand is not equal to that of the
   right-hand-side operand, then the operator '``!=``' or '``!==``' produces
   the value ``true``. Otherwise, the result is ``false``.

The following example illustrates *numerical equality*:

.. code-block:: typescript
   :linenos:

   5 == 5 // true
   5 != 5 // false

   5 === 5 // true

   5 == new Number(5) // true
   5 === new Number(5) // true

   5 == 5.0 // true

.. index::
   NaN
   value equality
   floating-point value
   floating-point operand
   numerical equality
   positive infinity
   negative infinity
   positive zero
   negative zero
   equality operator
   integer operand

|

.. _String Equality Operators:

String Equality Operators
=========================

.. meta:
    frontend_status: Done

Type of one operand must be of type ``string``, other operand must
be convertible (see :ref:`Implicit Conversions`) to ``string`` type.

Two strings are equal if they represent the same sequence of characters:

.. code-block:: typescript
   :linenos:

   "abc" == "abc" // true
   "abc" === "ab" + "c" // true

   function foo(s: string) {
      console.log(s == "hello")
   }
   foo("hello") // prints "true"

.. index::
   value equality
   string
   string equality operator
   operand
   convertibility

|

.. _Bigint Equality Operators:

Bigint Equality Operators
==========================

.. meta:
    frontend_status: Done

*Bigint equality* is used for operands of type ``bigint``.

Two ``bigints`` are equal if they have the same value:

.. code-block:: typescript
   :linenos:

   let x = 2n
   x == 2n // true

.. index::
   value equality
   bigint equality operator
   bigint
   equality operator
   boolean equality
   operand
   value

|

.. _Boolean Equality Operators:

Boolean Equality Operators
==========================

.. meta:
    frontend_status: Done

*Boolean equality* is used for operands of type ``boolean``.

If both operands are
either ``true`` or ``false``, then the result of ':math:`==`' or ':math:`===`'
is ``true``. Otherwise, the result is ``false``.

If both operands are either ``true`` or ``false``, then the result of '``!=``'
or '``!==``' is ``false``. Otherwise, the result is ``true``.

.. index::
   value equality
   boolean type
   boolean equality
   boolean equality operator
   equality operator
   value equality operator
   operand

|

.. _Enumeration Equality Operators:

Enumeration Equality Operators
==============================

.. meta:
    frontend_status: Done

If both operands are of the same enumeration type (see :ref:`Enumerations`),
then :ref:`Numerical Equality Operators` or :ref:`String Equality Operators`
are used depending on the kind of enumeration constant value
(:ref:`Enumeration Integer Values` or :ref:`Enumeration String Values`).
Otherwise, a :index:`compile-time error` occurs.

.. index::
   value equality
   enumeration type
   enumeration constant value
   constant value

|

.. _Reference Equality Based on Actual Type:

Reference Equality Based on Actual Type
=======================================

.. meta:
    frontend_status: Done

If an operand of an equality operator is of type ``Object``, or a union type,
or is a type parameter, then the operator is evaluated at runtime, and is based
on the actual type of this operand. If the other operand is of a type other
than that above, then the static type of this operand is used for evaluation.

If actual types of objects are compatible, then the corresponding evaluation of
equality operator is used. Otherwise, the result of the operators '``==``' and
'``===``' is ``false``.

.. index::
   reference equality
   union type
   operand
   compatibility
   equality operator
   object
   type parameter
   evaluation
   operator

|

.. _Object Type Equality Operators:

Object Type Equality Operators
------------------------------

.. meta:
    frontend_status: Done

A value of type ``Object`` can be compared to a value of any reference type.

The following example illustrates an equality with a value of type ``Object``:

.. code-block:: typescript
   :linenos:

    function equToString(a: Object, b: string): boolean {
        return a == b
    }

    equToString("aa", "aa") // true, string equality
    equToString(1,  "aa") // false, not compatible types

    function equ(a: Object, b: Object): boolean {
        return a == b
    }

    equ(1, 1) // true, numerical equality
    equ(1, 2) // false, numerical equality

    equ("aa", "aa") // true, string equality
    equ(1, "aa") // false, not compatible types

.. index::
   object type
   equality operator
   value
   compatible type
   compatibility

**Note**. The actual type of an ``Object`` value can be none of the following:

- Union type, as only the current value of a union type variable can be assigned
  to an ``Object`` variable;

- Type parameter, if it has no type constraint (see
  :ref:`Type Parameter Constraint`) as in the example below:

.. code-block:: typescript
   :linenos:

    function check(a: Object) {}

    class G<A, B extends number> {
        foo(x: A, y: B) {
            check(x) // compile-type error, A is not assignable to Object
            check(y) // ok, B is assignable to Object (as it is, at least, Number)
        }
    }

.. index::
   value
   union type
   variable
   type constraint
   assignment
   constraint
   type parameter
   object type

|

.. _Union Equality Operators:

Union Equality Operators
------------------------

Where one operand is of type ``T``:sub:`1`, and the other operand is of type
``T``:sub:`2`, while ``T``:sub:`1`, ``T``:sub:`2`, or both are a union type,
then a :index:`compile-time error` occurs if ``T``:sub:`1` and ``T``:sub:`2`
have no overlap (i.e., if no value belongs to both ``T``:sub:`1` and
``T``:sub:`2`).

**Note**. Any union type has an overlap with a value of type ``Object``.

The following example illustrates an equality with values of two union types:

.. code-block:: typescript
   :linenos:

    function f1(x: number | string, y: boolean | null): boolean {
        return x == y // compile-time error, types have no overlap
    }

    function f2(x: number | string, y: boolean | "abc"): boolean {
        return x == y // ok, types have overlap - value "abc"
    }

.. index::
   union equality
   equality operator
   type
   operand
   union type
   value
   object type

If actual types of values are compatible, then the corresponding evaluation of
an equality operator is used. Otherwise, the result of the operators '``==``'
and '``===``' is ``false``:

.. code-block:: typescript
   :linenos:

    function equ(x: number | string, y: string): boolean {
        return x == y
    }

    console.log(equ("aa", "aa")) // string equality: prints true
    console.log(equ("ab", "aa")) // string equality: prints false
    console.log(equ(1, "aa")) // different types: prints false

.. index::
   compatibility
   evaluation
   equality operator
   value
   operator

|

.. _Type Parameter Equality Operators:

Type Parameter Equality Operators
---------------------------------

.. meta:
    frontend_status: Done

If one operand is a type parameter, then the other operand can be of any
reference type, including type parameter.

If actual object types are compatible, then the corresponding evaluation of an
equality operator is used. Otherwise, the result of the operators '``==``' and
'``===``' is ``false``:

.. code-block:: typescript
   :linenos:

    function equ<A>(x: A, y: A): boolean {
        return x == y
    }

    console.log(equ<string>("aa", "aa")) // string equality: prints true
    console.log(equ<number>(1, 2)) // numerical equality: prints false

.. index::
   equality operator
   type parameter
   operand
   reference type
   compatibility
   evaluation

|

.. _Reference Equality:

Reference Equality
==================

.. meta:
    frontend_status: Partly
    todo: adapt latest specification changes

Reference equality compares operands of two reference types except types
``string``, ``bigint``, ``Object``, union types, and type parameters. The
extended semantics is discussed in :ref:`Extended Equality with null or undefined`.

A :index:`compile-time error` occurs if:

-  Any operand is not of a reference type;

-  There is no implicit conversion (see :ref:`Implicit Conversions`) that
   can convert type of either operand to the type of the other operand.

The result of '``==``' or '``===``' is ``true`` if both operand values:

-  Are ``null``;
-  Are ``undefined``; or
-  Refer to the same object, array, or function.

In addition, the result of the '``==``' operator is ``true`` if one operand
value is ``null``, and the other operand value is ``undefined``. Otherwise,
the result is ``false``. This semantics is illustrated by the example below:

.. index::
   reference equality
   reference type
   union type
   extended equality
   operand
   entity
   null
   undefined
   array
   operator
   operand value
   semantics

.. code-block:: typescript
   :linenos:

   class X {}
   new X() == new X() // false, two different object of class X
   new X() === new X() // false, the same
   let x1 = new X()
   let x2 = x1
   x1 == x2 // true, as x1 and x2 refer to the same object
   x1 === x2 // true, the same

   new Number(5) === new Number(5) // true, value equality is used
   new Number(5) == new Number(6) // false, value equality is used

   null == undefined // true
   null === undefined // false

|

.. _Extended Equality with null or undefined:

Extended Equality with ``null`` or ``undefined``
================================================

.. meta:
    frontend_status: Done

|LANG| provides extended semantics for equalities with ``null`` and ``undefined``
to ensure better alignment with |TS|.

Any entity can be compared to ``null`` by using the operators '``==``' and
'``===``'. This comparison can return ``true`` only for the entities of
*nullable* types if they actually have the ``null`` value during program
execution. In all other cases the comparison to ``null`` returns ``false``.

.. index::
   extended equality
   semantics
   nullable type
   null
   operator
   entity

Operators  '``!=``' and '``!==``' return ``true`` for any entity of
*non-nullable* types, and for *nullable* entities if they actually have no
``null`` value during program execution.

These situations are to be known at compile time.

Similarly, an equality comparison to ``undefined`` returns ``false`` if the
variable being compared is neither type ``undefined`` nor a union type with
``undefined`` as one of its types.

The following comparisons evaluate to ``false`` at compile time:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   5 == null // false
   5 == undefined // false
   ((): void => {}) == null // false

   class X {}
   new X() == null // false

The following comparison is evaluated at runtime:

.. code-block:: typescript
   :linenos:

    function foo<T> (p1: string | null, p2: T) {
        console.log (p1 == undefined, p1 == null, p2 == undefined, p2 == null)
    }
    let nullable: string|null = "a string"
    let undefinedable: Object|undefined = undefined

    foo (nullable, undefinedable) // Output: false false true true

.. index::
   entity
   non-nullable type
   nullable type
   null
   variable
   equality
   comparison
   undefined
   evaluation
   runtime

|

.. _Bitwise and Logical Expressions:

Bitwise and Logical Expressions
*******************************

.. meta:
    frontend_status: Done

The *bitwise operators* and *logical operators* are as follows:

-  AND operator '``&``';
-  Exclusive OR operator '``^``'; and
-  Inclusive OR operator '``|``'.

.. code-block:: abnf

    bitwiseAndLogicalExpression:
        expression '&' expression
        | expression '^' expression
        | expression '|' expression
        ;

These operators have different precedence. The operator '``&``' has the highest,
while '``|``' has the lowest precedence.

Operators group left-to-right. Each operator is commutative if the
operand expressions have no side effects, and associative.

The bitwise and logical operators can compare two operands of a numeric
type, or two operands of the ``boolean`` type. Otherwise, a
:index:`compile-time error` occurs.

.. index::
   bitwise operator
   logical operator
   bitwise expression
   logical expression
   type boolean
   operand expression
   exclusive OR operator
   inclusive OR operator
   AND operator
   commutative operator
   boolean type
   side effect
   numeric type
   associativity
   operator

|

.. _Integer Bitwise Operators:

Integer Bitwise Operators
=========================

.. meta:
    frontend_status: Done

The numeric types conversion (see :ref:`Primitive Types Conversions`)
is performed first on the operands of operator '``&``', '``^``', or '``|``'
if both operands are of a type convertible (see :ref:`Implicit Conversions`)
to a primitive integer type.

**Note**. If the initial type of one or both operands is ``double`` or
``float``, then that operand or operands are truncated first to the appropriate
integer type. If both operands are of type ``bigint``, then no conversion is
required.

A bitwise operator expression type is the converted type of its operands.

The resultant value of '``&``' is the bitwise AND of the operand values.

The resultant value of '``^``' is the bitwise exclusive OR of the operand values.

The resultant value of '``|``' is the bitwise inclusive OR of the operand values.

Thus, *integer bitwise expression* is of primitive integer type.


.. index::
   integer bitwise operator
   numeric types conversion
   numeric type
   conversion
   convertibility
   types conversion
   bitwise exclusive OR operand
   bitwise inclusive OR operand
   bitwise AND operand
   expression type
   operand value
   integer type
   conversion
   truncation

|

.. _Boolean Logical Operators:

Boolean Logical Operators
=========================

.. meta:
    frontend_status: Done

Type of the bitwise operator expression is ``boolean``. Both operands of
operator '``&``', '``^``', or '``|``' must be of type ``boolean``.

If both operand values are ``true``, then the resultant value of '``&``' is
``true``. Otherwise, the result is ``false``.

If the operand values are different, then the resultant value of ‘``^``’ is
``true``. Otherwise, the result is ``false``.

If both operand values are ``false``, then the resultant value of ‘``|``’ is
``false``. Otherwise, the result is ``true``.

Thus, *boolean logical expression* is of the boolean type.

.. index::
   boolean operator
   logical operator
   bitwise operator expression
   conversion
   numeric types conversion
   numeric type
   operand value
   boolean logical expression
   boolean type

|

.. _Conditional-And Expression:

Conditional-And Expression
**************************

.. meta:
    frontend_status: Done

The *conditional-and* operator '``&&``' is similar to '``&``' (see
:ref:`Bitwise and Logical Expressions`) but evaluates its right-hand-side
operand only if the value of the left-hand-side operand is ``true``.

The results of computation of '``&&``' and '``&``' on ``boolean`` operands are
the same, but the right-hand-side operand in '``&&``' cannot be evaluated.

.. code-block:: abnf

    conditionalAndExpression:
        expression '&&' expression
        ;

A *conditional-and* operator groups left-to-right.

A *conditional-and* operator is fully associative as regards both the result
value and side effects (i.e., the evaluations of the expressions *((a)* ``&&``
*(b))* ``&&`` *(c)* and *(a)* ``&&`` *((b)* ``&&`` *(c))* produce the same
result, and the same side effects occur in the same order for any *a*, *b*, and
*c*).

.. index::
   conditional-and operator
   conditional-and expression
   bitwise expression
   logical expression
   boolean operand
   conditional evaluation
   evaluation
   expression

A *conditional-and* expression is always of type ``boolean`` except the
extended semantics (see :ref:`Extended Conditional Expressions`).
A *conditional-and* expression with extended semantics can be of the first
expression type.

Each operand of the *conditional-and* operator must be of type ``boolean``,
or of a type mentioned in :ref:`Extended Conditional Expressions`.
Otherwise, a :index:`compile-time error` occurs.

The left-hand-side operand expression is first evaluated at runtime.

If the resultant value is ``false``, then the value of the *conditional-and*
expression is ``false``. The evaluation of the right-hand-side operand
expression is omitted.

If the value of the left-hand-side operand is ``true``, then the
right-hand-side expression is evaluated. 
The resultant value is the value of the *conditional-and*
expression.

.. index::
   conditional-and expression
   conditional-and operator
   compile-time error
   boolean type
   predefined numeric types conversion
   numeric types conversion
   numeric type
   evaluation

|

.. _Conditional-Or Expression:

Conditional-Or Expression
*************************

.. meta:
    frontend_status: Done

The *conditional-or* operator '``||``' is similar to '``|``' (see
:ref:`Integer Bitwise Operators`) but evaluates its right-hand-side operand
only if the value of its left-hand-side operand is ``false``.

.. code-block:: abnf

    conditionalOrExpression:
        expression '||' expression
        ;

A *conditional-or* operator groups left-to-right.

A *conditional-or* operator is fully associative as regards both the result
value and side effects (i.e., the evaluations of the expressions *((a)* ``||``
*(b))* ``||`` *(c)* and *(a)* ``||`` *((b)* ``||`` *(c))* produce the same
result, and the same side effects occur in the same order for any *a*, *b*,
and *c*).

A *conditional-or* expression is always of type ``boolean``  except the
extended semantics (see :ref:`Extended Conditional Expressions`).
A *conditional-or* expression with extended semantics can be of the first
expression type.

.. index::
   conditional-or expression
   conditional-or operator
   integer bitwise expression
   associativity
   expression
   side effect
   evaluation
   boolean type
   semantics
   boolean type
   extended semantics

Each operand of the *conditional-or* operator must be of type ``boolean``
or type mentioned in :ref:`Extended Conditional Expressions`.
Otherwise, a :index:`compile-time error` occurs.

The left-hand-side operand expression is first evaluated at runtime. 

If the resultant value is ``true``, then the value of the *conditional-or*
expression is ``true``, and the evaluation of the right-hand-side operand
expression is omitted.

If the resultant value is ``false``, then the right-hand-side expression is
evaluated. 
The resultant value is the value of the *conditional-or* expression.

The computation results of '``||``' and '``|``' on ``boolean`` operands are
the same, but the right-hand-side operand in '``||``' cannot be evaluated.

.. index::
   conditional-or expression
   conditional-or operator
   runtime
   boolean type
   expression
   boolean operand
   numeric types conversion
   numeric type
   semantics
   conditional evaluation

|

.. _Assignment:

Assignment
**********

.. meta:
    frontend_status: Done

All *assignment operators* group right-to-left (i.e., :math:`a=b=c` means
:math:`a=(b=c)`. The value of *c* is thus assigned to *b*, and then the value
of *b* to *a*).

.. code-block:: abnf

    assignmentExpression:
        lhsExpression assignmentOperator rhsExpression
        ;

    assignmentOperator
        : '='
        | '+='  | '-='  | '*='   | '='  | '%='
        | '<<=' | '>>=' | '>>>='
        | '&='  | '|='  | '^='
        ;

    lhsExpression:
        expression
        ;

    rhsExpression:
        expression
        ;

The first operand in an assignment operator represented by *lhsExpression* must
be *left-hand-side expression* (see :ref:`Left-Hand-Side Expressions`). This
first operand denotes a variable.

.. index::
   assignment
   assignment operator
   operand
   variable
   expression

Type of the variable is the type of the assignment expression.

The result of the *assignment expression* at runtime is not a variable itself
but the value of a variable after the assignment.

.. index::
   variable
   assignment
   assignment expression
   value
   runtime

|

.. _Simple Assignment Operator:

Simple Assignment Operator
==========================

.. meta:
    frontend_status: Done

The form of a simple assignment expression is ``E1 = E2``.

A :index:`compile-time error` occurs if type of the right-hand-side operand
(*rhsExpression*) is not assignable (see :ref:`Assignability`) to
the type of the variable. Otherwise, the expression is evaluated at runtime in
one of the following ways:

1. If the left-hand-side operand *lhsExpression* is a field access expression
   *e.f* (see :ref:`Field Access Expression`), possibly enclosed in parentheses,
   then:

   #. *lhsExpression* *e* is evaluated: if the evaluation of *e*
      completes abruptly, then so does the assignment expression.
   #. Right-hand-side operand *rhsExpression* is evaluated: if the evaluation
      completes abruptly, then so does the assignment expression.
   #. Value of the right-hand-side operand as computed above is assigned
      to the variable denoted by *e.f*.

.. index::
   simple assignment operator
   operator
   assignable type
   assignability
   access
   field access
   field access expression
   runtime
   abrupt completion
   evaluation
   assignment expression
   variable

2. If the left-hand-side operand is an array reference expression (see
   :ref:`Array Indexing Expression`), possibly enclosed in parentheses, then:

   #. Array reference subexpression of the left-hand-side operand is evaluated.
      If this evaluation completes abruptly, then so does the assignment
      expression. In that case, the right-hand-side operand and the index
      subexpression are not evaluated, and the assignment does not occur.
   #. If the evaluation completes normally, then the index subexpression of the
      left-hand-side operand is evaluated. If this evaluation completes abruptly,
      then so does the assignment expression. In that case, the right-hand-side
      operand is not evaluated, and the assignment does not occur.
   #. If the evaluation completes normally, then the right-hand-side operand is
      evaluated. If this evaluation completes abruptly, then so does the
      assignment expression, and the assignment does not occur.
   #. If the evaluation completes normally, but the value of the index
      subexpression is less than zero, or greater than, or equal to the
      *length* of the array, then ``ArrayIndexOutOfBoundsError`` is thrown,
      and the assignment does not occur.
   #. Otherwise, the value of the index subexpression is used to select an
      element of the array referred to by the value of the array reference
      subexpression.

      That element is a variable of type ``SC``. If ``TC`` is type of the
      left-hand-side operand of the assignment operator determined at compile
      time, then there are two options:


      - If ``TC`` is a primitive type, then ``SC`` can only be the same as ``TC``.

        The value of the right-hand-side operand is converted to the type of the
        selected array element. The value set conversion (see
        :ref:`Implicit Conversions`) is performed to convert it to the
        appropriate standard value set (not an extended-exponent value set).
        The result of the conversion is stored into the array element.
      - If ``TC`` is a reference type, then ``SC`` can be the same as ``TC`` or
        a type that extends or implements ``TC``.

.. index::
   operand
   array reference expression
   parenthesis
   array indexing expression
   reference subexpression
   assignment
   assignment expression
   abrupt completion
   subexpression
   evaluation
   array element
   array length
   variable
   conversion
   array element
   value set
   extended exponent
   reference type
   assignable class
   assignability
   runtime

3. If the left-hand-side operand is a record access expression (see
   :ref:`Record Indexing Expression`), possibly enclosed in parentheses, then:

   #. Object reference subexpression of the left-hand-side operand is evaluated.
      If this evaluation completes abruptly, then so does the assignment
      expression.
      In that case, the right-hand-side operand and the index subexpression are
      not evaluated, and the assignment does not occur.
   #. If the evaluation completes normally, the index subexpression of the
      left-hand-side operand is evaluated. If this evaluation completes abruptly,
      then so does the assignment expression.
      In that case, the right-hand-side operand is not evaluated, and the
      assignment does not occur.
   #. If the evaluation completes normally, the right-hand-side operand is
      evaluated. If this evaluation completes abruptly, then so does the
      assignment expression. In that case, the assignment does not occur.
   #. Otherwise, the value of the index subexpression is used as the ``key``.
      In that case, the right-hand-side operand is used as the ``value``, and
      the key-value pair is stored in the record instance.

.. index::
   operand
   record access expression
   parenthesis
   access expression
   reference subexpression
   index subexpression
   assignment
   assignment expression
   evaluation
   value
   key-value pair
   record instance
   normal completion
   abrupt completion
   key


If none of the above is true, then the following three steps are required:

#. Left-hand-side operand is evaluated to produce a variable. If the
   evaluation completes abruptly, then so does the assignment expression.
   In that case, the right-hand-side operand is not evaluated, and the
   assignment does not occur.

#. If the evaluation completes normally, then the right-hand-side operand is
   evaluated. If the evaluation completes abruptly, then so does the assignment
   expression. In that case, the assignment does not occur.

#. If that evaluation completes normally, then the value of the right-hand-side
   operand is converted to the type of the left-hand-side variable.
   In that case, the result of the conversion is stored into the variable.
   A :index:`compile-time error` occurs if type of the left-hand-side
   variable is one of the following:

   - ``readonly`` array (see :ref:`Readonly Parameters`), while the
     converted type of the right-hand-side operand is a non-``readonly`` array;
   - ``readonly`` tuple (see :ref:`Readonly Parameters`), while the
     converted type of the right-hand-side operand is a non-``readonly`` tuple.

.. index::
   evaluation
   operand
   assignment expression
   assignment
   abrupt completion
   normal completion
   conversion
   variable
   readonly array
   readonly tuple
   converted type

|

.. _Compound Assignment Operators:

Compound Assignment Operators
=============================

.. meta:
    frontend_status: Done

A compound assignment expression in the form *E1 op= E2* is equivalent to
*E1 = ((E1) op (E2)) as T*, where ``T`` is type of *E1*, except that *E1*
is evaluated only once. This expression can be evaluated at runtime in one
of the following ways:

1. If the left-hand-side operand expression is not an indexing expression:

   -  The left-hand-side operand is evaluated to produce a variable. If the
      evaluation completes abruptly, then so does the assignment expression.
      In that case, the right-hand-side operand is not evaluated, and no
      assignment occurs.

   -  If the evaluation completes normally, then the value of the left-hand-side
      operand is saved, and the right-hand-side operand is evaluated. If the
      evaluation completes abruptly, then so does the assignment expression.
      In that case, no assignment occurs.

   -  If the evaluation completes normally, then the saved value of the
      left-hand-side variable, and the value of the right-hand-side operand are
      used to perform the binary operation as indicated by the compound
      assignment operator. If the operation completes abruptly, then so does
      the assignment expression. In that case, no assignment occurs.

   -  If the evaluation completes normally, then the result of the binary
      operation converts to the type of the left-hand-side variable.
      The result of such conversion is stored into the variable.

.. index::
   compound assignment operator
   assignment operator
   indexing expression
   evaluation
   expression
   runtime
   operand
   variable
   assignment
   abrupt completion
   normal completion
   assignment expression
   binary operation
   conversion

2. If the left-hand-side operand expression is an array reference expression (see
   :ref:`Array Indexing Expression`), then:

   -  Array reference subexpression of the left-hand-side operand is evaluated.
      If the evaluation completes abruptly, then so does the assignment
      expression. In that case, the right-hand-side operand and the index
      subexpression are not evaluated, and no assignment occurs.

   -  If the evaluation completes normally, then the index subexpression of the
      left-hand-side operand is evaluated. If the evaluation completes abruptly,
      then so does the assignment expression. In that case, the right-hand-side
      operand is not evaluated, and no assignment occurs.

   -  If the evaluation completes normally, the value of the array
      reference subexpression refers to an array, and the value of the
      index subexpression is less than zero, greater than, or equal to
      the *length* of the array, then ``ArrayIndexOutOfBoundsError`` is
      thrown. In that case, no assignment occurs.

   -  If the evaluation completes normally, then the value of the index
      subexpression is used to select an array element referred to by
      the value of the array reference subexpression. The value of this
      element is saved, and then the right-hand-side operand is evaluated.
      If the evaluation completes abruptly, then so does the assignment
      expression. In that case, no assignment occurs.

   -  If the evaluation completes normally, consideration must be given
      to the saved value of the array element selected in the previous
      step. While this element is a variable of type ``S``, and ``T`` is
      type of the left-hand-side operand of the assignment operator
      determined at compile time:


      - If ``T`` is a primitive type, then ``S`` is the same as ``T``.

        The saved value of the array element, and the value of the
        right-hand-side operand are used to perform the binary operation of the
        compound assignment operator.

        If this operation completes abruptly, then so does the assignment
        expression. In that case, no assignment occurs.

        If this evaluation completes normally, then the result of the binary
        operation converts to the type of the selected array element.
        The result of the conversion is stored into the array element.

      - If ``T`` is a reference type, then it must be ``string``.

        ``S`` must also be a ``string`` because the class ``string`` is the
        *final* class. The saved value of the array element, and the value of
        the right-hand operand are used to perform the binary operation (string
        concatenation) of the compound assignment operator '``+=``'. If
        this operation completes abruptly, then so does the assignment
        expression. In that case, no assignment occurs.

      - If the evaluation completes normally, then the ``string`` result of
        the binary operation is stored into the array element.

.. index::
   value
   array element
   operand
   expression
   array reference expression
   array indexing expression
   reference subexpression
   evaluation
   assignment subexpression
   index subexpression
   normal completion
   abrupt completion
   array length
   assignment
   subexpression
   variable
   assignment operator
   primitive type
   array element
   value
   operand
   binary operation
   assignment expression
   assignment
   conversion
   array element
   compound assignment operator
   string
   evaluation
   array
   string concatenation

3. If the left-hand-side operand expression is a record access expression (see
   :ref:`Record Indexing Expression`):

   -  The object reference subexpression of the left-hand-side operand is
      evaluated. If this evaluation completes abruptly, then so does the
      assignment expression. In that case, the right-hand-side operand and
      the index subexpression are not evaluated, and no assignment occurs.

   -  If this evaluation completes normally, then the index subexpression of
      the left-hand operand is evaluated. If the evaluation completes abruptly,
      then so does the assignment expression. In that case, the right-hand-side
      operand is not evaluated, and no assignment occurs.

   -  If this evaluation completes normally, the value of the object reference
      subexpression and the value of index subexpression are saved, then the
      right-hand-side operand is evaluated. If the evaluation completes
      abruptly, then so does the assignment expression. In that case, no
      assignment occurs.

   -  If this evaluation completes normally, the saved values of the object
      reference subexpression and index subexpression (as the *key*) are used
      to get the *value* that is mapped to the *key* (see
      :ref:`Record Indexing Expression`), then this *value* and the value of the
      right-hand-side operand are used to perform the binary operation as
      indicated by the compound assignment operator. If the operation
      completes abruptly, then so does the assignment expression. In that case,
      no assignment occurs.

    - If the evaluation completes normally, then the result of the binary
      operation is stored as the key-value pair in the record instance
      (as in :ref:`Simple Assignment Operator`).

.. index::
   record access expression
   operand expression
   record indexing expression
   object reference subexpression
   operand
   index subexpression
   evaluation
   assignment expression
   abrupt completion
   normal completion
   assignment
   object reference subexpression
   reference subexpression
   index subexpression
   key
   key-value pair
   record indexing expression
   indexing expression
   record instance
   value
   compound assignment operator
   binary operation

|

.. _Left-Hand-Side Expressions:

Left-Hand-Side Expressions
==========================

.. meta:
    frontend_status: Done

*Expression* is a *left-hand-side expression* if it is one of the following:

-  Named variable;
-  Field or setter resultant from a field access (see
   :ref:`Field Access Expression`); or
-  Array or record element access (see :ref:`Indexing Expressions`).

A :index:`compile-time error` occurs if:

-  *Expression* contains the chaining operator '``?.``' (see
   :ref:`Chaining Operator`);
-  *Expression* result is not a variable.

.. index::
   expression
   named variable
   field
   setter
   field access
   array element
   record element
   access
   indexing expression
   chaining operator
   variable

|


.. _Conditional Expressions:

Conditional Expressions
***********************

.. meta:
    frontend_status: Done
    todo: implement full LUB support (now only basic LUB implemented)

The conditional expression '``? :``' uses the boolean value of the first
expression to decide which of the other two expressions to evaluate:

.. code-block:: abnf

    conditionalExpression:
        expression '?' expression ':' expression
        ;

The conditional operator '``? :``' groups right-to-left (i.e., the meaning of
:math:`a?b:c?d:e?f:g` and :math:`a?b:(c?d:(e?f:g))` is the same).

The conditional operator '``? :``' consists of three operand expressions
with the separators '``?``' between the first and the second expression, and
'``:``' between the second and the third expression.

A :index:`compile-time error` occurs if the first expression is not of type
``boolean``, or a type mentioned in
:ref:`Extended Conditional Expressions`.

.. index::
   conditional expression
   boolean value
   expression
   conditional operator
   operand
   operand expression
   separator
   boolean type
   extended conditional expression

Type of the conditional expression is determined as the union of types of the
second and the third expressions further normalized in accordance with the
process discussed in :ref:`Union Types Normalization`. If the second and the
third expressions are of the same type, then this is the type of the
conditional expression.

The following steps are performed as the evaluation of a conditional expression
occurs at runtime:

#. The operand expression of a conditional expression is evaluated first.

#. If the value of the first operand is ``true``, then the second operand
   expression is evaluated. Otherwise, the third operand expression is
   evaluated. The result of successful evaluation is the result of the
   conditional expression.

The examples below represent different scenarios with standalone expressions:

.. code-block:: typescript
   :linenos:

    class A {}
    class B extends A {}

    condition ? new A() : new B() // A | B => A

    condition ? 5 : 6             // int

    condition ? "5" : 6           // "5" | int

.. index::
   conditional expression
   normalization
   union type
   evaluation
   operand expression
   conversion
   standalone expression

|

.. _String Interpolation Expressions:

String Interpolation Expressions
********************************

.. meta:
    frontend_status: Done

'*String interpolation expression*' is a multiline string literal (a string
literal delimited with backticks, see :ref:`Multiline String Literal` for
details) that contains at least one *embedded expression*:

.. code-block:: abnf

    stringInterpolation:
        '`' (BacktickCharacter | embeddedExpression)* '`'
        ;

    embeddedExpression:
        '${' expression '}'
        ;

An '*embedded expression*' is an expression specified inside curly braces
preceded by the *dollar sign* '``$``'. A string interpolation expression is of
type ``string`` (see :ref:`Type String`).

When evaluating a *string interpolation expression*, the result of each
embedded expression substitutes that embedded expression. An embedded
expression must be of type ``string``. Otherwise, the implicit conversion
to ``string`` takes place in the same way as with the string concatenation
operator (see :ref:`String Concatenation`):

.. index::
   string interpolation expression
   multiline string
   string literal
   backtick
   string type
   expression
   string
   curly brace
   concatenation
   embedded expression
   string concatenation operator
   implicit conversion
   embedded expression

.. code-block:: typescript
   :linenos:

    let a = 2
    let b = 2
    console.log(`The result of ${a} * ${b} is ${a * b}`)
    // prints: The result of 2 * 2 is 4

The string concatenation operator can be used to rewrite the above example
as follows:

.. code-block:: typescript
   :linenos:

    let a = 2
    let b = 2
    console.log("The result of " + a + " * " + b + " is " + a * b)

An embedded expression can contain nested multiline strings.

.. index::
   string concatenation operator
   nested multiline string
   multiline string
   embedded expression

|

.. _Lambda Expressions:

Lambda Expressions
******************

.. meta:
    frontend_status: Done

*Lambda expression* fully defines an instance of a function type (see
:ref:`Function Types`) by providing optional ``async`` mark, type parameters
(see :ref:`Type Parameters`), mandatory lambda signature, and its body. The
definition of *lambda expression* is generally similar to that of a function
declaration (see :ref:`Function Declarations`), except that a lambda expression
has no function name specified, and can have types of parameters omitted:

.. code-block:: abnf

    lambdaExpression:
        annotationUsage?
        ('async'|typeParameters)? lambdaSignature '=>' lambdaBody
        ;

    lambdaBody:
        expression | block
        ;

    lambdaSignature:
        '(' lambdaParameterList? ')' returnType?
        | identifier
        ;

    lambdaParameterList:
        lambdaParameter (',' lambdaParameter)* (',' restParameter)? ','?
        | restParameter ','?
        ;

    lambdaParameter:
        annotationUsage? (lambdaRequiredParameter | lambdaOptionalParameter)
        ;

    lambdaRequiredParameter:
        identifier (':' type)?
        ;

    lambdaOptionalParameter:
        identifier '?' (':' type)?
        ;

    lambdaRestParameter:
        '...' lambdaRequiredParameter
        ;

The usage of annotations is discussed in :ref:`Using Annotations`.

.. index::
   lambda expression
   instance
   function type
   async mark
   type parameter
   lambda signature
   function declaration
   annotation

The examples of usage are presented below:

.. code-block:: typescript
   :linenos:

    (x: number): number => { return Math.sin(x) } // block as lambda body
    (x: number) => Math.sin(x)                    // expression as lambda body
    <T> (x: T, y: T) => { let local = x }         // generic lambda
    e => e                                        // shortest form of lambda

A *lambda expression* evaluation creates an instance of a function type (see
:ref:`Function Types`) as described in detail in
:ref:`Runtime Evaluation of Lambda Expressions`.

.. index::
   lambda expression
   function type
   instance

|

.. _Lambda Signature:

Lambda Signature
================

.. meta:
    frontend_status: Done

Similarly to function declarations (see :ref:`Function Declarations`),
a *lambda signature* is composed of formal parameters and
optional return types. Unlike function declarations,
type annotations of formal parameters can be omitted.

.. code-block:: typescript
   :linenos:

    function foo<T> (a: (p1: T, ...p2: T[]) => T) {}
    // All calls to foo pass valid lambda expressions in different forms
    foo (e => e)
    foo ((e1, e2) => e1)
    foo ((e1, e2: Object) => e1)
    foo ((e1: Object, e2) => e1)
    foo ((e1: Object, e2, e3) => e1)
    foo ((e1: Object, ...e2) => e1)

    foo (<Object>(e1: Object, e2: Object) => e1)

    function bar<T> (a: (...p: T[]) => T) {}
    // Type can be omitted for the rest parameter
    bar ((...e) => e)

    function goo<T> (a: (p?: T) => T) {}
    // Type can be omitted for the optional parameter
    goo ((e?) => e)


The specification of scope is discussed in :ref:`Scopes`, and shadowing details
of formal parameter declarations in :ref:`Shadowing by Parameter`.

A :index:`compile-time error` occurs if:

- Lambda expression declares two formal parameters with the same name.
- Formal parameter contains no type provided, and type cannot be derived
  by type inference.


.. index::
   lambda signature
   return type
   lambda expression
   function declaration
   type annotation
   type inference
   annotation
   scope
   shadow parameter
   shadowing
   parameter declaration
   evaluation
   argument expression
   initialization
   variable
   execution
   lambda body

|

.. _Lambda Body:

Lambda Body
===========

.. meta:
    frontend_status: Done

*Lambda body* can be a single expression or a block (see :ref:`Block`).
Similarly to the body of a method or a function, a lambda body describes the
code to be executed when a lambda expression call occurs (see
:ref:`Function Call Expression`).

The meanings of names, and of the keywords ``this`` and ``super`` (along with
the accessibility of the referred declarations) are the same as in the
surrounding context. However, lambda parameters introduce new names.

If a *lambda body* is a single expression, then it is equivalent to the block
with one return statement that returns that single expression
*{ return singleExpression }*.

If any local variable or formal parameter of the surrounding context is
used but not declared in a lambda body, then the local variable or formal
parameter is *captured* by the lambda.

If an instance member of the surrounding type is used in the lambda body
defined in a method, then ``this`` is *captured* by the lambda.

A :index:`compile-time error` occurs if a local variable is used in a lambda
body but is neither declared in nor assigned before it.

If *lambda signature* return type is not ``void`` (see :ref:`Type void`), and
the execution path of the lambda body has no return statement (see
:ref:`Return Statements`) or no single expression as a body, then a
:index:`compile-time error` occurs.

.. index::
   lambda body
   lambda expression
   keyword this
   keyword super
   runtime
   evaluation
   method body
   function body
   lambda call
   surrounding context
   accessibility
   lambda body
   instance member
   surrounding type
   return statement
   assignment

|

.. _Lambda Expression Type:

Lambda Expression Type
======================

.. meta:
    frontend_status: Done

*Lambda expression type* is a function type (see :ref:`Function Types`)
that has the following:

-  Lambda parameters (if any) as parameters of the function type; and

-  Lambda return type as the return type of the function type.

**Note**. Lambda return type can be inferred from the *lambda body* and thus
the return type can be dropped off.

 .. code-block:: typescript
    :linenos:

      const lambda = () => { return 123 }  // Type of the lambda is () => int
      const int_var: int = lambda()


.. index::
   lambda expression type
   function type
   lambda parameter
   lambda return type
   inference
   lambda body

|

.. _Runtime Evaluation of Lambda Expressions:

Runtime Evaluation of Lambda Expressions
========================================

.. meta:
    frontend_status: Done

The evaluation of a lambda expression itself never causes the execution of the
lambda body. If completing normally at runtime, the evaluation of a lambda
expression produces a reference to an allocated and initialized new instance
of a function type (see :ref:`Function Types`) that corresponds to the lambda
signature. In that case, it is similar to the evaluation of a class instance
creation expression (see :ref:`New Expressions`).

If the available space is not sufficient for a new instance to be created,
then the evaluation of the lambda expression completes abruptly, and
``OutOfMemoryError`` is thrown.

During a lambda expression evaluation, the captured values of the
lambda expression are saved to the internal state of the created instance.

.. index::
   runtime evaluation
   lambda expression
   lambda body
   execution
   initialization
   function type
   lambda signature
   normal completion
   instance creation expression
   initialization
   allocation
   instance
   abrupt completion
   error
   captured value
   internal state
   lambda expression evaluation


+-----------------------------------------------+--------------+
| **Source Fragment**                           | **Output**   |
+===============================================+==============+
| .. code-block:: typescript                    ||             |
|    :linenos:                                  |              |
|                                               |              |
|      function foo() {                         |              |
|      let y: int = 1                           | 2            |
|      let x = () => { return y+1 }             |              |
|      console.log(x())                         |              |
|      }                                        |              |
+-----------------------------------------------+--------------+

The variable 'y' is *captured* by the lambda.

The captured variable is not a copy of the original variable. If the
value of the variable captured by the lambda changes, then the original
variable is implied to change:

.. index::
   captured by lambda
   lambda
   variable
   captured variable
   original variable

+-----------------------------------------------+--------------+
| **Source Fragment**                           | **Output**   |
+===============================================+==============+
| .. code-block:: typescript                    ||             |
|    :linenos:                                  |              |
|                                               |              |
|     function foo() {                          |              |
|     let y: int = 1                            | 1            |
|     let x = () => { y++ }                     |              |
|     console.log(y)                            | 2            |
|     x()                                       |              |
|     console.log(y)                            |              |
|     }                                         |              |
+-----------------------------------------------+--------------+

In order to make lambdas behave as required, the language implementation
can act as follows:

-  Replace the captured variable’s type for a proxy class that contains an
   original reference (x: T for x: Proxy<T>; x.ref = original-ref) if that
   captured variable is of non-primitive type.

If the captured variable is defined as ``const``, then 
proxying is not required.

If the captured formal parameter can be neither boxed nor proxied, then
the implementation can require addition of a local variable as follows:

.. index::
   lambda
   implementation
   primitive type
   proxy class
   captured variable
   captured variable type
   non-primitive type
   proxying
   local variable
   variable

+-----------------------------------+-----------------------------------+
| **Source Code**                   | **Pseudo Code**                   |
+===================================+===================================+
| .. code-block:: typescript        | .. code-block:: typescript        |
|    :linenos:                      |    :linenos:                      |
|                                   |                                   |
|     function foo(y: int) {        |     function foo(y: int) {        |
|     let x = () => { return y+1 }  |     let y$: Int = y               |
|     console.log(x())              |     let x = () => { return y$+1 } |
|     }                             |     console.log(x())              |
|                                   |     }                             |
+-----------------------------------+-----------------------------------+

|

.. _Constant Expressions:

Constant Expressions
********************

.. meta:
    frontend_status: Done

*Constant expressions* are expressions with values that can be evaluated at
compile time.

.. code-block:: abnf

    constantExpression:
        expression
        ;

A *constant expression* is an expression of a primitive type (see
:ref:`Primitive Types`), or of type ``string``, or of enum type (see
:ref:`Enumerations`) that completes normally while being composed only of the
following:

-  Literals of a primitive type, and literals of type ``string`` (see
   :ref:`Literals`);

-  Conversions to primitive types and conversions to type ``string`` (see
   :ref:`Cast Expressions`);

-  Enumeration type constants;

-  Unary operators '``+``', '``-``', '``~``', and '``!``', but not '``++``'
   or '``--``' (see :ref:`Unary Plus`, :ref:`Unary Minus`,
   :ref:`Prefix Increment`, and :ref:`Prefix Decrement`);

-  Multiplicative operators '``*``', '``/``', and '``%``' (see
   :ref:`Multiplicative Expressions`);

-  Additive operators '``+``' and '``-``' (see :ref:`Additive Expressions`);

-  Shift operators '``<<``', '``>>``', and '``>>>``' (see
   :ref:`Shift Expressions`);

-  Relational operators '``<``', '``<=``', '``>``', and '``>=``' (see
   :ref:`Relational Expressions`);

-  Equality operators '``==``' and '``!=``' (see :ref:`Equality Expressions`);

-  Bitwise and logical operators '``&``', '``^``', and '``|``' (see
   :ref:`Bitwise and Logical Expressions`);

-  Conditional-and operator '``&&``' (see :ref:`Conditional-And Expression`),
   and conditional-or operator '``||``' (see :ref:`Conditional-Or Expression`);

-  Ternary conditional operator '``? :``' (see :ref:`Conditional Expressions`);

-  Parenthesized expressions (see :ref:`Parenthesized Expression`) that contain
   constant expressions;

-  Simple names or qualified names that refer to constants (see
   :ref:`Constant Declarations`) with constant expressions as initializers,
   declared in the same compilation unit.

.. index::
   constant expression
   primitive type
   string type
   enumeration type
   conversion
   normal completion
   literal
   cast expression
   unary operator
   increment operator
   decrement operator
   prefix
   multiplicative operator
   multiplicative expression
   shift operator
   relational operator
   equality operator
   bitwise operator
   logical operator
   ternary conditional operator
   conditional operator
   conditional-and operator
   conditional-or operator
   parenthesized expression
   constant expression
   simple name
   constant variable
   qualified name
   initializer
   compilation unit

.. raw:: pdf

   PageBreak
