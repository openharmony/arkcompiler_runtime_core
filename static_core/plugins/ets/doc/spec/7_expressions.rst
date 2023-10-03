.. _Expressions:

Expressions
###########

.. meta:
    frontend_status: Partly

This chapter describes the meanings of expressions and the rules for the
evaluation of  expressions (except for the expressions related to coroutines,
see :ref:`Create and Launch a Coroutine` for ``launch`` expressions and
:ref:`Awaiting a Coroutine` for ``await`` expressions).

.. index::
   evaluation
   expression
   coroutine
   launch expression
   await expression

.. code-block:: abnf

    expression:
        literal
        | qualifiedName
        | arrayLiteral
        | objectLiteral
        | voidExpression
        | parenthesizedExpression
        | thisExpression
        | methodCallExpression
        | fieldAccessExpression
        | indexingExpression
        | functionCallExpression
        | newExpression
        | castExpression
        | instanceOfExpression
        | ensureNotNullishExpression
        | nullishCoalescingExpression
        | unaryExpression
        | multiplicativeExpression
        | additiveExpression
        | shiftExpression
        | relationalExpression
        | equalityExpression
        | bitwiseAndLogicalExpression
        | conditionalAndExpression
        | conditionalOrExpression
        | assignmentExpression
        | conditionalExpression
        | stringInterpolation
        | lambdaExpression
        | dynamicImportExpression
        | launchExpression
        | awaitExpression
        ;

The grammar rules below introduce the productions to be used by other forms
of expression productions:

.. index::
   production
   expression production

.. code-block:: abnf

    potentiallyNullishExpression:
        expression '?'?
        ;

    objectReference:
        typeReference
        | (typeReference '.')? super
        | potentiallyNullishExpression
        ;

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

The *potentiallyNullishExpression* introduces an expression which can be
evaluated to a nullish value (``null`` or ``undefined``) if the expression
is of the nullish type; it implies that—while skipping the evaluation of
any surrounding parts—the entire surrounding expression is immediately
evaluated to ``undefined``. Otherwise, the evaluation of such expression
is guaranteed to be a non-nullish result of the enclosed expression evaluation.

.. index::
   expression
   evaluation
   nullish value
   nullish type
   surrounding expression
   non-nullish result
   expression evaluation

The *objectReference* refers to class or interface in the first two cases,
and thus allows handling static members. The last case refers to an
instance variable of class or interface type, unless the expression within
*potentiallyNullishExpression* is evaluated to ``undefined``.

The *arguments* refers to a list of arguments of a call. The last argument
can be prefixed by the spread operator '``...``'.

.. index::
   interface
   class
   static member
   instance variable
   argument
   expression
   evaluation
   prefix
   spread operator

|

.. _Chaining Operator:

Chaining Operator
*****************

.. meta:
    frontend_status: Partly

The term *optional chaining operator* (*'?.'*) is used as it effectively
covers accesses to an object's property, or calls to functions. If the object
to the left of (*'?.'*) is *undefined* or *null*, then the evaluation of the
entire surrounding expression is dropped, and *undefined* is used as the result
of the expression.

.. index::
   chaining operator
   access
   object property
   function call
   evaluation
   surrounding expression
   expression

|

.. _Evaluation of Expressions:

Evaluation of Expressions
*************************

.. meta:
    frontend_status: Done
    todo: needs more investigation, too much failing CTS tests (mostly tests are buggy)

The result of a program expression *evaluation* denotes the following:

-  A variable (the term *variable* is used here in the general, non-terminological
   sense to denote a modifiable lvalue in the left-hand side of an assignment);
   or
-  A value (results found in all other places).

.. index::
   evaluation
   expression
   variable
   lvalue
   assignment

A variable or a value are equally considered the *value of the expression*
if further evaluation requires such a value.

Expressions can contain assignments, increment operators, decrement operators,
method calls, and function calls; as a result, the evaluation of an expression
can produce side effects.

.. index::
   variable
   value
   evaluation
   expression

An expression can occur inside:

-  A declaration of type (class or interface), i.e. within a
   field initializer, static initializer, constructor declaration,
   method declaration, or annotation; or
-  A function body; or
-  An annotation of package declarations, or top-level declarations.


*Constant expressions* (see :ref:`Constant Expressions`) are the expressions
with values that can be determined at compile time.

.. index::
   expression
   declaration of type
   class
   interface
   field initializer
   static initializer
   constructor declaration
   method declaration
   annotation
   function body
   package declaration
   top-level declaration
   constant expression
   compile time

|

.. _Type of Expression:

Type of Expression
==================

.. meta:
    frontend_status: Done

The type of an expression that denotes a variable or a value is known at
compile time.

The type of a standalone expression (like just a + b) can be determined
entirely from the content of that expression; the type of any other expression
can be influenced by its target type (see :ref:`Contexts and Conversions`).
The rules that explain how the type of an expression can be determined are
provided below.

.. index::
   type
   expression
   value
   variable
   compile time
   standalone expression
   target type
   context
   conversion

The value of a type *T* expression is always suitable for the assignment to
a type *T* variable only if:

-  The expression value type is compatible (see :ref:`Compatible Types`)
   with the expression type (see :ref:`Reference Types` for the variables of
   a reference type); and
-  The value stored in a variable is compatible with the variable type.

.. index::
   value
   expression
   compatible type
   reference type
   variable
   variable type
   type compatibility

**Note**: *final* classes (see :ref:`Class Modifiers Final Classes`) cannot
have subclasses; if a class type *F* expression is declared *final*, then
only a class *F* object can be its value.

.. index::
   final class
   class modifier
   subclass
   expression
   object
   value

|

.. _Normal and Abrupt Completion of Expression Evaluation:

Normal and Abrupt Completion of Expression Evaluation
=====================================================

.. meta:
    frontend_status: Done

Every expression in a normal mode of evaluation requires certain computational
steps. The normal modes of evaluation for each kind of expression are described
in the following sections.

An expression evaluation *completes normally* if all computational steps
are performed without throwing an exception or error.

On the contrary, an expression *completes abruptly* if the expression
evaluation throws an exception or error.

The information about the causes of an abrupt completion can be available
in the value attached to the exception or error object.

.. index::
   normal completion
   abrupt completion
   evaluation
   expression
   error
   exception
   value

The predefined operators throw runtime errors as follows:

-  If an array reference expression has the ``null`` value, then an array
   access expression throws *NullPointerError*.
-  If an array reference expression has the ``null`` value, then an
   *indexing expression* (see :ref:`Indexing Expression`) throws
   *NullPointerError*.
-  If an array index expression has a value that is negative, greater than,
   or equal to the length of the array, then an *indexing expression* (see
   :ref:`Indexing Expression`) throws *ArrayIndexOutOfBoundsError*.
-  If a cast cannot be performed at runtime, then cast expressions (see
   :ref:`Cast Expressions`) throw *ClassCastError*.
-  If the right-hand operand expression has the zero value, then integer
   division (see :ref:`Division`), or integer remainder (see :ref:`Remainder`)
   operators throw *ArithmeticError*.
-  If the boxing conversion (see :ref:`Predefined Numeric Types Conversions`)
   occurs, then the assignment to an array element of a reference type (see
   :ref:`Array Literal`), method call expression (see
   :ref:`Method Call Expression`), or prefix/postfix increment/decrement (see
   :ref:`Unary Expressions`) operators throw *OutOfMemoryError*.
-  If the type of an array element is incompatible with the value that
   is being assigned, then the assignments to an array element of a
   reference type (see :ref:`Array Literal`) throw *ArrayStoreError*.

.. index::
   predefined operator
   runtime error
   array reference expression
   value
   array access expression
   error
   array index expression
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

Possible hard-to-predict and hard-to-handle linkage and virtual machine errors
can cause errors in the course of an expression evaluation.

An abrupt completion of a subexpression evaluation results in the following:

.. index::
   linkage
   virtual machine error
   error
   expression
   evaluation
   abrupt completion
   subexpression

-  Immediate abrupt completion of the expression that contains such a
   subexpression (if the evaluation of the entire expression requires
   the evaluation of that contained subexpression); and
-  Cancellation of all subsequent steps of the normal mode of evaluation.

.. index::
   abrupt completion
   expression
   subexpression
   evaluation

The terms ‘complete normally’ and ‘complete abruptly’ can also denote a
normal and an abrupt completion of the execution of statements (see
:ref:`Normal and Abrupt Statement Execution`). A statement can complete
abruptly for a variety of reasons in addition to an exception or error thrown.

.. index::
   normal completion
   abrupt completion
   execution
   statement
   error
   exception

|

.. _Order of Expression Evaluation:

Order of Expression Evaluation
==============================

.. meta:
    frontend_status: Done

The operands of an operator are evaluated from left to right in accordance with
the rules below:

-  Any right-hand operand is evaluated only after the full evaluation of the
   left-hand operand of a binary operator.

   If using a compound-assignment operator (see :ref:`Simple Assignment Operator`),
   the evaluation of the left-hand operand includes the following:


   - Remembering the variable denoted by the left-hand operand,
   - Fetching the value of that variable for the subsequent evaluation
     of the right-hand operand, and
   - Saving such value.

   If the evaluation of the left-hand operand completes abruptly, then no
   part of the right-hand operand is evaluated.

-  Any part of the operation can be executed only after the full evaluation
   of every operand of an operator (except conditional operators '&&', '||',
   and '?:').

   The execution of a binary operator that is an integer division '/' (see
   :ref:`Division`), or integer remainder '%' (see :ref:`Remainder`) can
   throw *ArithmeticError* only after the evaluations of both operands
   complete normally.
-  The |LANG| programming language follows the order of evaluation as indicated
   explicitly by parentheses, and implicitly by the precedence of operators.
   This rule particularly applies for infinity and NaN values of floating-point
   calculations.
   |LANG| considers integer addition and multiplication as provably associative;
   however, floating-point calculations must not be naively reordered because
   they are unlikely to be computationally associative (even though they appear
   to be mathematically associative).


.. index::
   operand
   order of evaluation
   expression
   operator
   evaluation
   binary operator
   compound-assignment operator
   simple assignment operator
   variable
   value
   abrupt completion
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

+---------------------------------+-------------------------+-------------------+
|         **Operator**            |   **Precedence**        | **Associativity** |
+=================================+=========================+===================+
| postfix increment and decrement | :math:`++ --`           | left to right     |
+---------------------------------+-------------------------+-------------------+
| prefix increment and decrement, | :math:`++` :math:`--`   | right to left     |
| and unary                       | :math:`+` :math:`-` ! ~ |                   |
+---------------------------------+-------------------------+-------------------+
| multiplicative                  | `\*` / %                | left to right     |
+---------------------------------+-------------------------+-------------------+
| additive                        | :math:`+` :math:`-`     | left to right     |
+---------------------------------+-------------------------+-------------------+
| cast                            | as                      | left to right     |
+---------------------------------+-------------------------+-------------------+
| shift                           | << >>  >>>              | left to right     |
+---------------------------------+-------------------------+-------------------+
| relational                      |  < > <= >= instanceof   | left to right     |
+---------------------------------+-------------------------+-------------------+
| equality                        |  == !=                  | left to right     |
+---------------------------------+-------------------------+-------------------+
| bitwise AND                     | &                       | left to right     |
+---------------------------------+-------------------------+-------------------+
| bitwise exclusive OR            | ^                       | left to right     |
+---------------------------------+-------------------------+-------------------+
| bitwise inclusive OR            | |                       | left to right     |
+---------------------------------+-------------------------+-------------------+
| logical AND                     | &&                      | left to right     |
+---------------------------------+-------------------------+-------------------+
| logical OR                      | ||                      | left to right     |
+---------------------------------+-------------------------+-------------------+
| null-coalescing                 | ??                      | left to right     |
+---------------------------------+-------------------------+-------------------+
| ternary                         | ?:                      | right to left     |
+---------------------------------+-------------------------+-------------------+
| assignment                      | = += :math:`-=` %=      | right to left     |
|                                 | :math:`*=` :math:`/=`   |                   |
|                                 | ``&=`` ``^=`` ``|=``    |                   |
|                                 | <<= >>= >>>=            |                   |
+---------------------------------+-------------------------+-------------------+

|

.. _Evaluation of Arguments:

Evaluation of Arguments
=======================

.. meta:
    frontend_status: Done

An evaluation of arguments always progresses from left to right up to the first
error, or through the end of the expression, i.e., any argument expression is
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
when they from time to time cause exceptional conditions.
The evaluation order of the following expressions needs a specific explanation:

-  Class instance creation expressions (see :ref:`New Expressions`);
-  Array creation expressions (see :ref:`Array Creation Expressions`);
-  Indexing expressions (see :ref:`Indexing Expression`);
-  Method call expressions (see :ref:`Method Call Expression`);
-  Assignments involving indexing (see :ref:`Assignment`);
-  Lambda expressions (see :ref:`Lambda Expressions`).

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

Literals (see :ref:`Literals`) denote fixed and unchanging value.

Types of literals are determined as follows:

+--------------------+--------------------------------------------------+
| **Literal**        | **Type of Literal Expression**                   |
+====================+==================================================+
| Integer            | ``int`` if the value can be represented by       |
|                    | the 32-bit type, otherwise ``long``              |
+--------------------+--------------------------------------------------+
| Floating-point     | ``double``                                       |
+--------------------+--------------------------------------------------+
| Boolean            | ``boolean``                                      |
| (true, false)      |                                                  |
+--------------------+--------------------------------------------------+
| Char               | ``char``                                         |
+--------------------+--------------------------------------------------+
| String             | ``string``                                       |
+--------------------+--------------------------------------------------+
| Null (null)        | ``null``                                         |
+--------------------+--------------------------------------------------+

|

.. _Qualified Name:

Qualified Name
**************

.. meta:
    frontend_status: Done

A *qualifiedName* is an expression that consists of dot-separated names which
refer to a variable imported from some package, or field, or property of some
class. A *qualifiedName* without a qualification refers to a local variable
of the surrounding function’s or method’s parameter.

.. index::
   qualified name
   expression
   dot-separated name
   imported variable
   qualification
   package
   field
   class property
   local variable
   surrounding function
   method parameter

See the examples below for illustration:

.. code-block:: typescript
   :linenos:

    import * as packageName from "someFile"

    class Type {}

    function foo (parameter: Type) {
      let local: Type = parameter /* here 'parameter' is the
          expression in the form of qualified name */
      local = new Type () /* here 'local' is the expression in the
          form of qualified name */
      local = packageName.someGlobalVariable /* here qualifiedName
          refers to a global variable imported from some package */
    }

|

.. _Array Literal:

Array Literal
*************

.. meta:
    frontend_status: Done
    todo: let x : int = [1,2,3][1] - valid?
    todo: let x = ([1,2,3][1]) - should be CTE, but it isn't
    todo: implement it properly for invocation context to get type from the context, not from the first element

An *array literal* is an expression that can be used to create an array, and
provide some initial values.

.. code-block:: abnf

    arrayLiteral:
        '[' expressionSequence? ']'
        ;

    expressionSequence:
        expression (',' expression)* ','?
        ;

An array literal is a comma-separated list of *initializer expressions*
enclosed between '[' and ']'. A trailing comma after the last expression
in an array literal is ignored.

.. index::
   array literal
   expression
   value
   comma-separated list
   initializer expression


.. code-block:: typescript
   :linenos:

    let x = [1, 2, 3] // ok
    let y = [1, 2, 3,] // ok, trailing comma is ignored

The number of initializer expressions enclosed in braces of the array
initializer determines the length of the array to be constructed.

If sufficient space is allocated for a new array, then a one-dimensional
array of the specified length is created, and all elements of the array
are initialized to the values specified by initializer expressions.

.. index::
   initializer expression
   array initializer
   array
   one-dimensional array
   array element
   initialization
   initializer expression
   value

On the contrary, the evaluation of the array initializer completes abruptly if:

-  The space allocated for the new array is insufficient, and
   *OutOfMemoryError* is thrown; or
-  Some initialization expression completes abruptly.

.. index::
   evaluation
   array initializer
   abrupt completion
   array
   error
   initialization expression

Initializer expressions are executed from left to right. The *n*’th expression
specifies the value of the *n-1*’th element of the array.

Array literals can be nested, i.e., the initializer expression that specifies
an array element can be an array literal if that element is of an *array* type.

The type of an array literal is inferred by the following rules:

.. index::
   initializer expression
   execution
   value
   array element
   array literal
   array type
   type inference

-  If the type can be inferred from the context, then the type of an array
   literal is the inferred type `T[]`.
-  Otherwise, the type is inferred from the types of its elements.

.. index::
   type inference
   context
   array literal
   array element

|

.. _Type Inference from Context:

Type Inference from Context
===========================

.. meta:
    frontend_status: Done

The type of an array literal can be inferred from the context, including
explicit type annotation of a variable declaration, left-hand part type
of an assignment, call parameter type, or type of a cast expression:

.. index::
   type inference
   context
   array literal
   type
   type annotation
   variable declaration
   assignment
   call parameter type
   cast expression

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

    // ...
    type Matrix = number[][]
    let m: Matrix = [[1, 2], [3, 4], [5, 6]]

All valid conversions are applied to the initializer expression, i.e., each
initializer expression type must be compatible (see :ref:`Compatible Types`)
with the array element type; otherwise, a compile-time error occurs.

.. index::
   conversion
   initializer expression
   compatible type
   type compatibility
   array element
   type
   compile-time error

.. code-block:: typescript
   :linenos:

    let value: number = 2
    let list: Object[] = [1, value, "hello", new Error()] // ok

In the example above, the first literal and 'value' are implicitly boxed
to *Number*, and the types of a string literal and the instance of type
*Error* are compatible (see :ref:`Compatible Types`) with Object because
the corresponding classes are inherited from Object.

.. index::
   literal
   boxing
   string literal
   instance
   error
   type compatibility
   compatible type
   inheritance

If the type used in the context is a *tuple type* (see :ref:`Tuple Types`) 
then the type of the array literal will be a tuple type if types of all
literal expressions are compatible with tuple type elements at respective
positions.

.. code-block:: typescript
   :linenos:

    let tuple: [number, string] = [1, "hello"] // ok
    
    let incorrect: [number, string] = ["hello", 1] // compile-time error

|

.. _Type Inference from Types of Elements:

Type Inference from Types of Elements
=====================================

.. meta:
    frontend_status: Done

If the type of an array literal *[* *expr*:sub:`1`, ... , *expr*:sub:`N` *]*
cannot be inferred from the context, then it is inferred from the
initialization expressions by using the following algorithm:

#. If there is no expression (*N == 0*), then the type is *Object[]*

#. If the type of the expression cannot be determined, then the type of the
   array literal cannot be inferred, and a compile-time error occurs.

#. If each initialization expression is of some numeric type, then the
   type is *number[]*.

#. If all initialization expressions are of the same type *T*, then the
   type is *T[]*.

#. Otherwise the type is constructed as the union type *T*:sub:`1` | ... |
   *T*:sub:`N`, where *T*:sub:`i` is the type of *expr*:sub:`i`.
   Union type normalization (see :ref:`Union Types Normalization`) is applied
   to this union type.

.. index::
   type inference
   array element
   type
   array literal
   context
   initialization expression
   expression
   compile-time error
   numeric type
   union type normalization
   union type

.. code-block:: typescript
   :linenos:

    let a = [] // type is Object[]
    let b = ["a"] // type is string[]
    let c = [1, 2, 3] // type is number[]
    let d = ["a", 1, 3.14] // type is (string | number)[]
    let e = [(): void => {}, new A()] // type is (() => void | A)[]
    


|

.. _Object Literal:

Object Literal
***************

.. meta:
    frontend_status: Partly

An *object literal* is an expression that can be used to create a class
instance, and provide some initial values. It can be used in place of a class
instance creation expression (see :ref:`New Expressions`) because it is more
convenient in some cases.

.. index::
   object literal
   expression
   instance
   class
   class instance creation expression

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
enclosed in '{' and '}'. A trailing comma after the last pair is ignored. Each
*name-value pair* consists of an identifier and an expression:

.. index::
   object literal
   comma-separated list
   name-value pair
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

The type of an object literal is always some class *C* that is inferred from
the context. A type inferred from the context can be either a named class (see
:ref:`Object Literal of Class Type`), or an anonymous class created for the
inferred interface type (see :ref:`Object Literal of Interface Type`).

A compile-time error occurs if the type of an object literal cannot be inferred
from the context, or the inferred type is not a class or an interface type.

.. index::
   object literal
   inference
   context
   class type
   anonymous class
   interface type
   compile-time error
   inferred type

.. code-block:: typescript
   :linenos:

    let p = {name: "Bob", age: 25} /* compile-time error, type is
        not inferred */

|

.. _Object Literal of Class Type:

Object Literal of Class Type
=============================

.. meta:
    frontend_status: Done

If the class type *C* is inferred from the context, then the type of object
literal is *C*:

.. index::
   object literal
   class type
   inference
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


An identifier in each *name-value pair* must name a field of the class *C*,
or a field of any superclass of class *C*.

A compile-time error occurs if the identifier does not name an *accessible
member field* (:ref:`Scopes`) in the type *C*.

.. index::
   identifier
   name-value pair
   field
   superclass
   class
   compile-time error
   accessible member field
   scope

.. code-block:: typescript
   :linenos:

    class Friend {
      name: string = ""
      private nick: string = ""
      age: number = 0
    }
    // compile-time error, nick is private:
    let f: Friend = {name: "aa", age: 55, nick: "bb"}

A compile-time error occurs if the type of an expression in a *name-value
pair* is not compatible (see :ref:`Compatible Types`) with the field type.

.. code-block:: typescript
   :linenos:

    let f: Friend = {name: 123 /* compile-time error - type of right hand-side
    is not compatible to the type of the left hand-side */

If class *C* is to be used in an object literal, then it must have a
*parameterless* constructor (explicit or default) that is *accessible*
in the class composite context.

A compile-time error occurs if:

-  *C* does not contain a parameterless constructor; or
-  No constructor is accessible.

.. index::
   compile-time error
   expression
   type
   name-value pair
   compatible type
   type compatibility
   field type
   accessible constructor
   parameterless constructor
   class composite context
   object literal
   access

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

|

.. _Object Literal of Interface Type:

Object Literal of Interface Type
================================

If the interface type *I* is inferred from the context, then the type of the
object literal is an anonymous class implicitly created for interface *I*:

.. code-block:: typescript
   :linenos:

    interface Person {
      name: string = ""
      age: number = 0
    }
    let b : Person = {name: "Bob", age: 25}

In this example, the type of *b* is an anonymous class that contains the
same fields as the interface *I*.

A compile-time error occurs if the interface type *I* has methods.
It must contain fields only.

.. index::
   object literal
   interface type
   inference
   context
   anonymous class
   interface
   anonymous class
   field
   method
   compile-time error occurs

.. code-block:: typescript
   :linenos:

    interface I {
      name: string = ""
      foo(): void
    }
    let i : I = {name: "Bob"} // compile-time error, interface has methods

|

.. _Object Literal of Record Type:

Object Literal of Record Type
=============================

The generic Record<Key, Value> type (see :ref:`Record Utility Type`) is used
to map the properties of a type (*Key* type) to another type (*Value* type).
A special form of an object literal is used to initialize the value of such
type.

.. index::
   object literal
   generic record type
   record utility type
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

The first expression in *keyValue* denotes a key, and must be of type *Key*;
the second expression denotes a value, and must be of type *Value*.

.. index::
   expression
   type Key
   type Value
   value

.. code-block:: typescript

    let map: Record<string, number> = {
        "John": 25,
        "Mary": 21,
    }
    
    console.log(map["John"]) // prints 25

|

.. code-block:: typescript

    interface PersonInfo {
        age: number
        salary: number
    }
    let map: Record<string, PersonInfo> = {
        "John": { age: 25, salary: 10},
        "Mary": { age: 21, salary: 20}
    }


If a key is a union type consisting of literals, then all variants must be
listed in the object literal; otherwise, a compile-time error occurs:

.. index::
   key
   union type
   literal
   object literal
   compile-time error

.. code-block:: typescript

    let map: Record<"aa" | "bb", number> = {
        "aa": 1,
    } // compile-time error: "bb" key is missing

|

.. _Object Literal Evaluation:

Object Literal Evaluation
=========================

.. meta:
    frontend_status: Done

The evaluation of an object literal of type *C*, where *C* is either
a named class type, or an anonymous class type created for the interface,
is to be done by the following steps:

-  A parameterless constructor is executed to produce an instance *x* of
   the class *C*. The execution of the object literal completes abruptly
   if so does the execution of the constructor.

-  Name-value pairs of the object literal are then executed from left to
   right in the textual order they occur in the source code. The execution
   of a name-value pair includes the following:

   -  Evaluation of the expression; and
   -  Assigning the value of the expression to the corresponding field
      of *x*.

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

The execution of the object literal completes abruptly if so does
the execution of a name-value pair.

The object literal completes normally with the value of the newly
initialized class instance if all name-value pairs complete normally.

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

.. _void Expression:

``void`` Expression
*******************

.. code-block:: abnf

    voidExpression:
        'void'
        ;

The type of *voidExpression* is of type *void* (see :ref:`void Type`).

The evaluation of *voidExpression* results in a single object of type *void*
(see :ref:`void Type`).

.. index::
   expression
   void
   type void
   evaluation

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

The type and the value of a parenthesized expression are the same as those of
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
        (typeReference '.')? 'this'
        ;

The keyword ``this`` can only be used as an expression in the body of an
instance method of a class, *enum*, or interface.

It can be used in a lambda expression only if it is allowed in the
context the lambda expression appears in.

The keyword ``this`` in a direct call expression: ``this(...)`` can only
be used in the explicit constructor call statement.

A compile-time error occurs if the keyword ``this`` appears elsewhere.

.. index::
   compile-time error
   keyword this
   expression
   instance method
   method body
   class
   enum
   interface
   lambda expression
   direct call expression
   explicit constructor call statement

The keyword ``this`` used as a primary expression denotes a value that is a
reference to the following:

-  Object for which the instance method is called; or
-  Object being constructed.


The value denoted by ``this`` in a lambda body and in the surrounding context
is the same.

The class of the actual object referred to at runtime can be *T* if *T*
is a class type, or a class that is a subtype of *T*.

.. index::
   keyword this
   primary expression
   value
   instance method
   instance method call
   object
   lambda body
   surrounding context
   class
   runtime
   subtype
   class type
   class

|

.. _Qualified this:

Qualified ``this``
==================

The value of a qualified expression in the form *typeReference.this* is the
*n*’th lexically enclosing instance of ``this``, where *T* is the type of the
expression denoted by *typeReference*, and *n* is an integer (provided that
*T* is the *n*’th lexically enclosing type declaration of the class or
interface that qualified expression appears in).

A compile-time error occurs if the qualified ``this`` expression occurs in
a class or interface other than class *T*.

.. index::
   qualified this
   value
   qualified expression
   lexically enclosing instance
   expression type
   integer
   lexically enclosing type declaration
   class
   interface
   expression

|

.. _Method Call Expression:

Method Call Expression
**********************

.. meta:
    frontend_status: Done

A method call expression calls a static or instance method of a class, or
an interface.

.. index::
   method call expression
   static method
   instance method
   class
   interface

.. code-block:: abnf

    methodCallExpression:
        objectReference '.' identifier typeArguments? (arguments | arguments? block)
        ;

The syntax form which has a block associated with the method call is a special
form called trailing lambda call and described here (see :ref:`Trailing Lambda`).

A compile-time error occurs if *typeArguments* is present, and any of type
arguments are wildcards (see :ref:`Type Arguments`).

A method call with '?.' (see :ref:`Chaining Operator`) in *objectReference* is
called a safe method call because it handles nullish values safely.

Resolving a method at compile time is more complicated than resolving a field
because method overloading (see :ref:`Class Method Overloading`) can occur.

There are several steps that determine and check the method to be called at
compile time (see :ref:`Step 1 Selection of Type to Use`,
:ref:`Step 2 Selection of Method`, and
:ref:`Step 3 Semantic Correctness Check`).

.. index::
   compile-time error
   type argument
   wildcard
   method call
   chaining operator
   safe method call
   nullish value
   method resolution
   compile time
   field resolution
   method overloading
   semantic correctness check

|

.. _Step 1 Selection of Type to Use:

Step 1: Selection of Type to Use
================================

.. meta:
    frontend_status: Done

The object reference, and the method identifier are used to determine the
type in which to search the method.

The following options must be considered:

+----------------------------------+-----------------------------------------------+
| Form of object reference         | Type to use                                   |
+==================================+===============================================+
| *typeReference.identifier*       | Type denoted by *typeReference*.              |
+----------------------------------+-----------------------------------------------+
| *expression.identifier*, where   | *T* if *T* is a class or interface,           |
| *expression* is of type *T*      | *T*’s constraint                              |
|                                  | (:ref:`Type Parameter Constraint`) if *T* is  |
|                                  | a type parameter. A compile-time error occurs |
|                                  | otherwise.                                    |
+----------------------------------+-----------------------------------------------+
| *super.identifier*               | The superclass of the class that contains     |
|                                  | the method call.                              |
+----------------------------------+-----------------------------------------------+
| *typeReference.super.identifier* | The superclass of *typeReference*.            |
+----------------------------------+-----------------------------------------------+

.. index::
   type
   object reference
   method identifier
   compile-time error
   expression
   identifier
   interface
   superclass
   class
   method call
   type parameter constraint

|

.. _Step 2 Selection of Method:

Step 2: Selection of Method
===========================

.. meta:
    frontend_status: Done

After the type to use is known, the call method must be determined.

The goal is to select one from all potentially applicable methods.

As there is more than one applicable method, the *most specific* method must
be selected.

The method selection process is described below:

.. index::
   method selection
   call method
   type
   most specific method
   applicable method

#. All potentially applicable methods (i.e., all methods with the given name
   that are accessible at the point of call) must be found.

#. If there are several overloaded methods, the overload resolution must be
   performed without boxing/unboxing, and with no consideration to the *rest*
   (see :ref:`Rest Parameter`) and *optional* (see :ref:`Optional Parameters`)
   parameters.

#. If the method is not selected, the overload resolution must be performed
   with boxing/unboxing.

#. If the method is still not selected, the overload resolution must be
   performed with boxing/unboxing, and with consideration to the *rest*
   (see :ref:`Rest Parameter`) and *optional* (see :ref:`Optional Parameters`)
   parameters.

.. index::
   method selection
   call method
   applicable method
   overloaded method
   access
   method
   point of call
   overload resolution
   boxing
   unboxing
   rest parameter
   optional parameter

A compile-time error occurs if:

-  There is no method to select; or
-  There are more than one applicable methods.

.. index::
   compile-time error
   method selection
   applicable method

|

.. _Step 3 Semantic Correctness Check:

Step 3: Semantic Correctness Check
==================================

.. meta:
    frontend_status: Done

At this step, the single method to call (the *most specific* method) is known,
and the following set of semantic checks must be performed:

-  If the method call has the form *typeReference.identifier*, then the method
   must be declared ``static``; a compile-time error occurs otherwise.

-  If the method call has the form *expression.identifier*, then the method
   must not be declared ``static``; a compile-time error occurs otherwise.

-  If the method call has the form *super.identifier*, then the method must
   not be declared ``abstract``; a compile-time error occurs otherwise.

-  If the last argument of a method call has the spread operator '``...``',
   then *objectReference* that follows such argument must refer to an array
   whose type is compatible (see :ref:`Compatible Types`) with the type
   specified in the last parameter of the method declaration.

.. index::
   semantic correctness check
   most specific method
   method call
   static method call
   compile-time error
   abstract method call
   type argument
   method declaration
   argument
   spread operator
   compatible type
   type compatibility

|

.. _Field Access Expressions:

Field Access Expressions
************************

.. meta:
    frontend_status: Done

A field access expression can access a field of an object that is referred to
by the value of the object reference. The object reference value can have
different forms described in detail in :ref:`Accessing Current Object Fields`
and :ref:`Accessing Superclass Fields`.

.. index::
   field access expression
   access
   field
   value
   object reference
   superclass

.. code-block:: abnf

    fieldAccessExpression:
        objectReference '.' identifier
        ;

This object reference cannot denote a package, class type, or interface type.

Otherwise, the meaning of that expression is determined by the same rules as
the meanings of qualified names.

A field access in which *objectReference* contains '?.' (see :ref:`Chaining Operator`)
is called *safe field access* because it handles nullish values safely.

If object reference evaluation completes abruptly, then so does the entire
field access expression.

.. index::
   object reference
   package
   class type
   interface type
   expression
   qualified name
   reference evaluation
   safe field access
   nullish value
   field access
   field access expression

|

.. _Accessing Current Object Fields:

Accessing Current Object Fields
===============================

.. meta:
    frontend_status: Done

An object reference used for Field Access must be a non-nullish reference
type *T*; a compile-time error occurs otherwise.

Field access expression is valid if the identifier refers to a single
accessible member field in type *T*.

A compile-time error occurs if:

-  The identifier names several accessible member fields (see :ref:`Scopes`)
   in type *T*;
-  The identifier does not name an accessible member field in type *T*.

.. index::
   access
   object field
   field access
   non-nullish type
   reference type
   compile-time error
   member field
   identifier
   accessible member field

The result of the field access expression is computed at runtime as follows:

a. For a *static* field:

-  The result of an object reference expression evaluation is discarded.

-  The result is the value of the specified class variable in the class or
   interface that is the type of the object reference expression if the
   field is a non-blank *const* field.
-  The result is a variable (the specified class variable in the class
   that is the object reference type) if the field is a blank *const*, or
   is not *const*, while the field access occurs in a class variable
   initializer, or a static initializer.

.. index::
   field access expression
   runtime
   object reference expression
   evaluation
   static field
   interface
   class variable
   type
   const field
   field
   variable
   class
   static initializer
   variable initializer

b. For a non-*static* field:

-  The object reference expression is evaluated.

-  The result is the value of the named member field in type *T* found
   in the object referred to by the object reference expression value if
   the field is a non-blank *const* field.

-  The result is a variable (the named member field in type *T* found in
   the object referred to by the object reference expression value) if
   the field is a blank *const*, or is not *const*, while the field access
   occurs in an instance variable initializer, instance initializer,
   or constructor.

Only the object reference type (not the class type of an actual object
referred at runtime) is used to determine the field to be accessed.

.. index::
   non-static field
   object reference expression
   evaluation
   access
   runtime
   initializer
   instance initializer
   constructor
   field access
   reference type
   class type

|

.. _Accessing Superclass Fields:

Accessing Superclass Fields
===========================

.. meta:
    frontend_status: Done

A field access expression cannot denote a package, class type, or interface
type. Otherwise, the meaning of that expression is determined by the same
rules as the meaning of a qualified name.

The form *super.identifier* refers to the field named *identifier* of the
current object, while such current object is viewed as an instance of the
superclass of the current class).

The form *T.super.identifier* refers to the field named *identifier* of the
lexically enclosing instance corresponding to *T*, while such instance is
viewed as an instance of the superclass of  *T*.

The forms that use the keyword ``super`` are valid only in:

-  Instance methods;
-  Instance initializers;
-  Constructors of a class; or
-  Initializers of an instance variable of a class.

.. index::
   access
   superclass field
   expression
   package
   class type
   interface type
   qualified name
   identifier
   instance
   superclass
   constructor
   instance variable
   keyword super
   lexically enclosing instance
   instance initializer
   initializer

A compile-time error occurs if forms with the keyword ``super``:

-  Occur elsewhere;
-  Occur in the declaration of class *Object* (since *Object*
   has no superclass).


The field access expression *super.f* is handled in the same way as the
expression *this.f* in the body of class *S*; assuming that *super.f*
appears within class *C*, *f* is accessible in *S* from class *C* (see
:ref:`Scopes`) while:

-  The direct superclass of *C* is class *S*;
-  The direct superclass of the class denoted by *T* is a class with *S*
   as its fully qualified name.


A compile-time error occurs otherwise, and particularly if the current class
is not *T*.

.. index::
   compile-time error
   keyword super
   Object
   superclass
   field access expression
   access
   direct superclass
   qualified name

|

.. _Indexing Expression:

Indexing Expression
*******************

.. meta:
    frontend_status: Partly

An indexing expression is used to access elements of arrays (see
:ref:`Array Types`), and of the *Record* instances (see
:ref:`Record Utility Type`).

.. code-block:: abnf

    indexingExpression:
        expression ('?.')? '[' expression ']'
        ;

An indexing expression contains two subexpressions: *object reference
expression* before the left bracket, and *index expression* inside the
brackets.

.. index::
   indexing expression
   access
   array element
   array type
   record instance
   record utility type
   subexpression
   object reference expression
   index expression

If '?.' (see :ref:`Chaining Operator`) is present in an indexing expression,
then:

-  The type of the object reference expression must be a nullish type based
   on an array type, or on the *Record* type; a compile-time error occurs
   otherwise.
-  The object reference expression must be checked to evaluate to nullish
   value; if it does, then the entire *indexingExpression* equals *undefined*.


If no '?.' is present in an indexing expression, then the type of object
reference expression must be an array type, or the *Record* type; a
compile-time error occurs otherwise.

.. index::
   chaining operator
   indexing expression
   object reference expression
   nullish type
   record type
   compile-time error
   reference expression
   evaluation
   nullish value

|

.. _Array Indexing Expression:

Array Indexing Expression
=========================

.. meta:
    frontend_status: Done

For an array indexing, a type of *index expression* must be of a numeric type.

A numeric types conversion (see :ref:`Predefined Numeric Types Conversions`)
is performed on *index expression* to ensure that the resultant type is *int*.
A compile-time error occurs otherwise.

If the type of *object reference expression* after applying of optional '?.'
operator is an array type `T[ ]`, then the type of the indexing expression
is *T*.

The result of an indexing expression is a type *T* variable (i.e., a variable
within the array selected by the value of that *index expression*).

It is essential that, if type *T* is a reference type, then the fields of array
elements can be changed by changing the resultant variable fields.

An illustration is given in the example below:

.. code-block:: typescript
   :linenos:
   
    let names: string[] = ["Alice", "Bob", "Carol"]   
    console.log(name[1]) // prints Bob
    string[1] = "Martin"
    console.log(name[1]) // prints Martin

    class refType {
        field: number = 666
    }
    const objects: RefType[] = [new RefType(), new RefType()]
    const object = objects [1]
    object.field = 777            // change the field in the array element
    console.log(objects[0].filed) // prints 666
    console.log(objects[1].filed) // prints 777


.. index::
   array indexing expression
   array element
   indexing expression
   array indexing
   object reference expression
   optional operator
   array type
   index expression
   numeric type
   numeric types conversion
   predefined numeric types conversion
   compile-time error
   variable
   const
   reference expression


An array indexing expression evaluated at runtime behaves as follows:

-  First, the object reference expression is evaluated.
-  If the evaluation completes abruptly, then so does the indexing
   expression, and the index expression is not evaluated.
-  If the evaluation completes normally, then the index expression is evaluated.
   The resultant value of the object reference expression refers to an array.
-  For an array, if the index expression value is less than zero, greater
   than or equal to the array’s *length*, then *ArrayIndexOutOfBoundsError*
   is thrown.
-  Otherwise, the result of the array access is the type *T* variable within
   the array selected by the value of the index expression.

.. code-block:: typescript
   :linenos:

    function setElement(names: string[], i: number, name: string) {
        names[i] = name // run-time error, if 'i' is out of bounds
    }

.. index::
   array indexing
   indexing expression
   index expression
   array indexing expression
   object reference expression
   abrupt completion
   normal completion
   reference expression
   array
   error

|

.. _Record Indexing Expression:

Record Indexing Expression
==========================

For a *Record<Key, Value>* indexing, a type of *index expression* must be of the
*Key* type. The indexing expression’s result is of type *Value | undefined*.

.. code-block:: typescript
   :linenos:
   
    let x: Record<number, string> = {
        1: "hello",
        2: "buy", 
    }

    let y = x[3]

In the code above, the type of *y* is *string | undefined*, and the value of
*y* is *undefined*.

An indexing expression evaluated at runtime behaves as follows:

-  First, the object reference expression is evaluated.
-  If the evaluation completes abruptly, then so does the indexing
   expression, and the index expression is not evaluated.
-  If the evaluation completes normally, then the index expression is
   evaluated.
   The resultant value of the object reference expression refers to a record
   instance.
-  If the key, defined by the index expression, exists in the record instance,
   then the result is the value mapped to that key.
-  Otherwise, the result is the *undefined* literal.

.. index::
   record index expression
   evaluation
   runtime
   undefined
   type
   value
   reference type
   type Key
   indexing expression
   index expression
   object reference expression
   abrupt completion
   normal completion
   literal
   record instance
   key

|

.. _Function Call Expression:

Function Call Expression
************************

.. meta:
    frontend_status: Partly

A *function call expression* is used to call a function (see
:ref:`Function Types`), or a lambda expression (see :ref:`Lambda Expressions`).

.. code-block:: abnf

    functionCallExpression:
        expression ('?.' | typeArguments)? (arguments| arguments? block)
        ;

The special syntactic form which has a block associated with the function
call is called *trailing lambda call*. It is described in detain in
:ref:`Trailing Lambda`.

A compile-time error occurs if:

-  The *typeArguments* clause is present, and any of the type arguments are
   wildcards (see :ref:`Type Arguments`).
-  The *expression* type is different from the function type.
-  The *expression* type is nullish but no '?.' (see :ref:`Chaining Operator`)
   is present.

.. index::
   function call expression
   function call
   lambda expression
   compile-time error
   type argument
   wildcard
   expression type
   function type
   nullish type
   chaining operator

If '?.' (see :ref:`Chaining Operator`) is present, and the *expression*
evaluates to a nullish value, then the *arguments* are not evaluated,
the call is not performed, and the result of the *functionCallExpression*
is *undefined* . Such function call is *safe* because it handles nullish
values properly.

:ref:`Step 1 Selection of Function` and :ref:`Step 2 Semantic Correctness Check`
below specify the steps to follow to determine what function is being called.

.. index::
   chaining operator
   expression
   evaluation
   nullish value
   semantic correctness check
   undefined
   function call

|

.. _Step 1 Selection of Function:

Step 1: Selection of Function
=============================

.. meta:
    frontend_status: Done

One function must be selected from all potentially applicable functions as a
function can be overloaded.

The *most specific* function must be selected where there are more than one
applicable functions.

The function selection process is described below:

.. index::
   function selection
   overloaded function
   applicable function

#. All potentially applicable functions (i.e., all functions with the given
   name that are accessible at the point of call) must be found.

#. If there are several overloaded functions, the overload resolution must be
   performed without boxing/unboxing, and with no consideration to the *rest*
   (see :ref:`Rest Parameter`) and *optional* (see :ref:`Optional Parameters`)
   parameters.

#. If the function is not selected, the overload resolution must be performed
   with boxing/unboxing.

#. If the function is not selected, the overload resolution must be performed
   with boxing/unboxing, and with consideration to the *rest* (see
   :ref:`Rest Parameter`) and *optional* (see :ref:`Optional Parameters`)
   parameters.


.. index::
   potentially applicable function
   applicable function
   function
   access
   point of call
   overloaded function
   overload resolution
   boxing
   unboxing
   rest parameter
   optional parameter

A compile-time error occurs if:

-  There is no function to select; or

-  There are more than one applicable functions.

.. index::
   compile-time error
   function
   function selection
   applicable function

|

.. _Step 2 Semantic Correctness Check:

Step 2: Semantic Correctness Check
==================================

The single function to call is known at this step, and the following
semantic checks must be performed:

-  If the last argument of the function call has the spread operator '``...``',
   then *objectReference* that follows such argument must refer to an array
   whose type is compatible (see :ref:`Compatible Types`) with the type
   specified in the last parameter of the function declaration.

.. index::
   semantic correctness check
   function
   semantic check
   argument
   spread operator
   array
   compatible type
   type compatibility
   function declaration
   parameter

|

.. _New Expressions:

New Expressions
***************

.. meta:
    frontend_status: Done

The operation **new** instantiates an object of a *class* or *array* type.

.. code-block:: abnf

    newExpression:
        newClassInstance
        | newArrayInstance
        ;

A *class instance creation expression* creates new objects that are instances
of classes.

|LANG| also supports the creation of array instances as an experimental feature
(see :ref:`Array Creation Expressions`).

.. index::
   expression
   instantiation
   class instance creation expression
   class
   array
   object
   instance
   creation
   array instance
   array creation expression

.. code-block:: abnf

    newClassInstance:
        'new' typeArguments? typeReference arguments? classBody?
        ;

A *class instance creation expression* specifies a class to be instantiated,
and optionally lists all actual arguments for the constructor.

A *class instance creation expression* can throw an error as specified in
:ref:`Errors Handling`.

A class instance creation expression is *standalone* if it has no assignment
or call context (see :ref:`Assignment and Call Contexts`).

A class is *instantiated* when a class instance creation expression creates an
instance of that class. The *class instantiation* involves determining:

-  A class to be instantiated;
-  A constructor to be called to create that new instance.

The validity of the constructor call is similar to the validity of the method
call as described in :ref:`Step 3 Semantic Correctness Check`.

.. index::
   class instance creation expression
   instantiation
   argument
   constructor
   error
   instance creation expression
   instance
   error
   expression
   standalone expression
   assignment context
   call context
   class instance
   constructor
   method validity
   semantic correctness check

|

.. _Cast Expressions:

Cast Expressions
****************

.. meta:
    frontend_status: Done

``Cast expressions`` apply *cast operator*  '``as``' to some ``expression``
by issuing a value of the specified ``type``.

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
:ref:`Casting Contexts`). The target type can be either *type* or
*typeReference*.

.. index::
   cast operator
   expression
   conversion
   value
   runtime
   casting context
   cast expression

A cast expression type is always the target type.

The result of a cast expression is a value, not a variable (even if the operand
expression is a variable).

The casting conversion (see :ref:`Casting Conversions`) converts the operand
value at runtime to the target type specified by the cast operator (if needed).

A compile-time error occurs if the casting conversion (see :ref:`Casting Conversions`)
cannot convert the operand’s compile-time type to the target type specified by
the cast operator.

If the ``as`` cast cannot be performed during program execution, then the
*ClassCastError* is thrown.

.. index::
   cast expression
   target type
   value
   variable
   operand expression
   variable
   casting conversion
   operand value
   compile-time type
   cast operator
   execution
   error

|

.. _InstanceOf Expression:

InstanceOf Expression
*********************

.. meta:
    frontend_status: Partly

.. code-block:: abnf

    instanceOfExpression:
        expression 'instanceof' type
        ;

Any ``instanceof`` *expression* is of type *boolean*.

The *expression* operand of the operator ``instanceof`` must be of a
reference type; a compile-time error occurs otherwise.

An ``instanceof`` *expression* checks, during the program execution, whether
the type of the value the expression successfully evaluates to is compatible
to ``type``.
If so, then the result of the  ``instanceof`` *expression* is ``true``;
otherwise, the result is ``false``.
If the expression evaluation causes exception or error, then execution
control is transferred to a proper ``catch`` section or runtime system,
and the result of the ``instanceof`` *expression* cannot be determined.

.. index::
   instanceof expression
   expression
   operand
   reference type
   compile-time error
   execution
   evaluation
   type compatibility
   compatible type
   catch section
   runtime
   control transfer
   execution control
   boolean
   exception
   error

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

An *ensure-not-nullish expression* is a postfix expression with the operator '!'.
An *ensure-not-nullish expression* in the expression *e!* checks whether *e* of
the nullish type (see :ref:`Nullish Types`) evaluates to the *nullish* value.

If the result of the evaluation of *e* is:

-  Not equal to *null* or *undefined*, then the result of *e!* is the outcome
   of the evaluation of *e*;
-  Equal to *null* or *undefined*, then *NullPointerError* is thrown.

A compile-time error occurs if *e* is not a nullish type.

The type of *ensure-not-nullish* expression is the non-nullish variant of the
type of *e*.

.. index::
   ensure-not-nullish expression
   postfix
   prefix
   expression
   operator
   nullish type
   evaluation
   nullish value
   null
   undefined
   error
   compile-time error
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

A *nullish-coalescing expression* is a binary expression that uses the operator
'``??``', and checks whether the evaluation of the left-hand-side expression
equals the *nullish* value:

-  If so, then the right-hand-side expression evaluation is the result
   of a nullish-coalescing expression.
-  If not so, then the left-hand-side expression evaluation result is
   the result of a nullish-coalescing expression, and the right-hand-side
   expression is not evaluated (the operator '``??``' is thus **lazy**).

.. index::
   nullish-coalescing expression
   binary expression
   operator
   evaluation
   expression
   nullish value
   lazy operator

A compile-time error occurs if the left-hand-side expression is not a
reference type.

The type of a nullish-coalescing expression is the *least upper bound* (see
:ref:`Least Upper Bound`) of the non-nullish variant of the types of the
left-hand-side and right-hand-side expressions.

The following example represents the semantics of a nullish-coalescing
expression:

.. code-block:: typescript
   :linenos:

    let x = expression1 ?? expression2

    let x = expression1
    if (x == null) x = expression2

A compile-time error occurs if the nullish-coalescing operator is mixed
with conditional-and or conditional-or operators without parentheses.

.. index::
   compile-time error
   reference type
   nullish-coalescing expression
   least upper bound (LUB)
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
        | expression '––'
        | '++' expression
        | '––' expression
        | '+' expression
        | '–' expression
        | '~' expression
        | '!' expression
        ;

All expressions with unary operators (except postfix increment and postfix
decrement operators) group right-to-left for '~+x' to have the same meaning
as '~(+x)'.

.. index::
   unary expression
   expression
   unary operator
   postfix
   postfix
   increment operator
   decrement operator

|

.. _Postfix Increment:

Postfix Increment
=================

.. meta:
    frontend_status: Done

A *postfix increment expression* is an expression followed by a ':math:`++`'
increment operator.

A compile-time error occurs if the type of the variable resultant from the
*expression* is not convertible (see :ref:`Kinds of Conversion`) to a numeric
type.

The type of a postfix increment expression is the type of the variable. The
result of a postfix increment expression is a value, not a variable.

If the evaluation of the operand expression completes normally at runtime, then:

-  The value *1* is added to the value of the variable by using necessary
   conversions (see :ref:`Predefined Numeric Types Conversions`); and
-  The sum is stored back into the variable.

.. index::
   postfix
   increment operator
   postfix increment expression
   expression
   conversion
   variable
   compile-time error
   convertible expression
   value
   operand
   normal completion
   runtime


Otherwise, the postfix increment expression completes abruptly, and no
incrementation occurs.

The  value of the postfix increment expression is the value of the variable
*before* the new value is stored.

.. index::
   variable
   conversion
   predefined numeric types conversion
   postfix
   increment
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

A *postfix decrement expression* is an expression followed by a ':math:`--`'
decrement operator.

A compile-time error occurs if the type of the variable resultant from the
*expression* is not convertible (see :ref:`Kinds of Conversion`) to a numeric
type.

The type of a postfix decrement expression is the type of the variable. The
result of a postfix decrement expression is a value, not a variable.

If evaluation of the operand expression completes at runtime, then:

.. index::
   postfix
   decrement
   operator
   postfix decrement expression
   compile-time error
   variable
   expression
   conversion
   runtime
   operand
   completion
   evaluation

-  The value *1* is subtracted from the value of the variable by using
   necessary conversions (see :ref:`Predefined Numeric Types Conversions`); and
-  The sum is stored back into the variable.

Otherwise, the postfix decrement expression completes abruptly, and
no decrementation occurs.

The value of the postfix decrement expression is the value of the variable
*before* the new value is stored.

.. index::
   subtraction
   value
   variable
   conversion
   predefined numeric types conversion
   abrupt completion
   decrementation
   postfix decrement expression
   postfix
   decrement expression
   variable
   value

|

.. _Prefix Increment:

Prefix Increment
================

.. meta:
    frontend_status: Done

A *prefix increment expression* is an expression preceded by a ':math:`++`'
operator.

A compile-time error occurs if the type of the variable resultant from the
*expression* is not convertible (see :ref:`Kinds of Conversion`) to a numeric
type.

The type of a prefix increment expression is the type of the variable. The
result of a prefix increment expression is a value, not a variable.

If evaluation of the operand expression completes normally at runtime, then:

.. index::
   prefix increment operator
   prefix increment expression
   expression
   prefix
   increment operator
   evaluation
   increment expression
   variable
   runtime
   expression
   normal completion
   conversion

-  The value *1* is added to the value of the variable by using necessary
   conversions (see :ref:`Predefined Numeric Types Conversions`); and
-  The sum is stored back into the variable.

Otherwise, the prefix increment expression completes abruptly, and no
incrementation occurs.

The  value of the  prefix increment expression is the value of the variable
*before* the new value is stored.

.. index::
   value
   variable
   conversion
   predefined numeric types conversion
   numeric type
   abrupt completion
   prefix increment expression
   prefix
   increment expression

|

.. _Prefix Decrement:

Prefix Decrement
================

.. meta:
    frontend_status: Done

A *prefix decrement expression* is an expression preceded by a ':math:`--`'
operator.

A compile-time error occurs if the type of the variable resultant from the
*expression* is not convertible (see :ref:`Kinds of Conversion`) to a numeric
type.

The type of a prefix decrement expression is the type of the variable. The
result of a prefix decrement expression is a value, not a variable.

.. index::
   prefix decrement operator
   prefix decrement expression
   expression
   prefix
   decrement operator
   operator
   variable
   expression
   value

If evaluation of the operand expression completes normally at runtime, then:

-  The value *1* is subtracted from the value of the variable by using
   necessary conversions (see :ref:`Predefined Numeric Types Conversions`);
   and
-  The sum is stored back into the variable.

Otherwise, the prefix decrement expression completes abruptly, and no
decrementation occurs.

The value of the  prefix decrement expression is the value of the variable
*before* the new value is stored.

.. index::
   evaluation
   expression
   operand
   normal completion
   predefined numeric types conversion
   numeric type
   decrement
   abrupt completion
   variable
   prefix
   decrement
   expression
   prefix decrement expression

|

.. _Unary Plus:

Unary Plus
==========

.. meta:
    frontend_status: Done

The type of the operand *expression* with the unary ':math:`+`' operator must
be convertible  (see :ref:`Kinds of Conversion`) to a primitive numeric type;
a compile-time error occurs otherwise.

The numeric types conversion (see :ref:`Predefined Numeric Types Conversions`)
is performed on the operand to ensure that the resultant type is that of the
unary plus expression. The result of a unary plus expression is always a value,
not a variable (even if the result of the operand expression is a variable).

.. index::
   unary plus operator
   operand
   expression
   unary operator
   conversion
   primitive type
   numeric type
   compile-time error
   numeric types conversion
   predefined numeric types conversion
   unary plus expression
   expression
   operator
   value
   variable
   operand expression

|

.. _Unary Minus:

Unary Minus
===========

.. meta:
    frontend_status: Done
    todo: let a : Double = Double.Nan; a = -a; (assertion)

The type of the operand *expression* with the unary ':math:`--`' operator must
be convertible (see :ref:`Kinds of Conversion`) to a primitive numeric type; a
compile-time error occurs otherwise.

The numeric types conversion (see :ref:`Predefined Numeric Types Conversions`)
is performed on the operand to ensure that the resultant type is that of the
unary minus expression. 
The result of a unary minus expression is a value, not a variable (even if the
result of the operand expression is a variable).

A unary numeric promotion performs the value set conversion (see
:ref:`Kinds of Conversion`).

The unary negation operation is always performed on, and its result is drawn
from the same value set as the promoted operand value.

.. index::
   unary minus operation
   operand
   unary operator
   conversion
   primitive type
   numeric type
   predefined numeric types conversion
   expression
   operand
   normal completion
   value
   variable
   conversion
   unary numeric promotion
   value set conversion
   unary negation operation
   promoted operand value

Further value set conversions are then performed on that same result.

The value of a unary minus expression at runtime is the arithmetic negation
of the promoted value of the operand.

The negation of integer values is the same as subtraction from zero. The |LANG|
programming language uses two’s-complement representation for integers. The
range of two’s-complement value is not symmetric, and that same maximum
negative number results from the negation of the maximum negative *int* or
*long*. In such case an overflow occurs but throws no exception or error.
For any integer value *x*, *-x* is equal to '``(~x)+1``'.

The negation of floating-point values is *not* the same as subtraction from
zero (if *x* is *+0.0*, then *0.0-x* is *+0.0*, however *-x* is *-0.0*).

A unary minus merely inverts the sign of a floating-point number. Special
cases to consider are as follows:

-  The operand NaN results in NaN (NaN has no sign).
-  The operand infinity results in the infinity of the opposite sign.
-  The operand zero results in zero of the opposite sign.

.. index::
   value set conversion
   unary minus expression
   runtime
   negation
   promoted value
   operand
   operation
   integer
   value
   subtraction
   two’s complement representation
   two’s complement value
   overflow
   exception
   error
   floating-point value
   subtraction
   unary minus
   floating-point number
   infinity
   NaN

|

.. _Bitwise Complement:

Bitwise Complement
==================

.. meta:
    frontend_status: Done

The type of the operand *expression* with the unary '~' operator must be
convertible (see :ref:`Kinds of Conversion`) to a primitive integer type; a
compile-time error occurs otherwise.

The numeric types conversion (see :ref:`Predefined Numeric Types Conversions`)
is performed on the operand to ensure that the resultant type is that of the
unary bitwise complement expression.

The result of a unary bitwise complement expression is a value, not a variable
(even if the result of the operand expression is a variable).

The value of a unary bitwise complement expression at runtime is the bitwise
complement of the promoted value of the operand. In all cases, '``~x``' equals
'``(-x)-1``'.

.. index::
   bitwise complement operator
   complement operator
   expression
   operand
   unary operator
   conversion
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

The type of the operand *expression* with the unary '``!``' operator must be
*boolean* or *Boolean*; a compile-time error occurs otherwise.

The unary logical complement expression’s type is *boolean*.

The unboxing conversion (see :ref:`Predefined Numeric Types Conversions`) is
performed on the operand at runtime if needed.

The value of a unary logical complement expression is ``true`` if the
(possibly converted) operand value is ``false``, and ``false`` if the operand
value (possibly converted) is ``true``.

.. index::
   logical complement operator
   expression
   operand
   unary operator
   boolean
   Boolean
   compile-time error
   unary logical complement expression
   unboxing conversion
   boxing conversion
   predefined numeric types conversion
   numeric type

|

.. _Multiplicative Expressions:

Multiplicative Expressions
**************************

.. meta:
    frontend_status: Done

The operators '\*', '/', and '%' are *multiplicative operators*.

.. code-block:: abnf

    multiplicativeExpression:
        expression '*' expression
        | expression '/' expression
        | expression '%' expression
        ;

The multiplicative operators group left-to-right.

The type of each operand in a multiplicative operator must be convertible (see
:ref:`Contexts and Conversions`) to a primitive numeric type; a compile-time
error occurs otherwise.

The numeric types conversion (see :ref:`Predefined Numeric Types Conversions`)
is performed on both operands to ensure that the resultant type is the type of
the multiplicative expression.

The result of a unary bitwise complement expression is a value, not a
variable (even if the operand expression is a variable).

.. index::
   multiplicative expression
   convertibility
   context
   conversion
   primitive type
   numeric type
   multiplicative operator
   multiplicative expression
   primitive type
   numeric type
   value
   unary bitwise complement expression
   operand expression
   variable
   predefined numeric types conversion
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

The binary operator '\*' performs multiplication, and returns the product of
its operands.

Multiplication is a commutative operation unless the operand expressions
have side effects.

Integer multiplication is associative when all operands are of the same type.

Floating-point multiplication is not associative.

If overflow occurs during integer multiplication, then:

-  The result is the low-order bits of the mathematical product as represented
   in some sufficiently large two’s-complement format.
-  The sign of the result can be other than the sign of the mathematical
   product of the two operand values.


A floating-point multiplication result is determined in compliance with the
IEEE 754 arithmetic:

.. index::
   multiplication operator
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

-  The result is NaN if:

   -  Either operand is NaN;
   -  Infinity is multiplied by zero.


-  If the result is not NaN, the sign of the result is:

   -  Positive if both operands have the same sign; and
   -  Negative if the operands have different signs.


-  If infinity is multiplied by a finite value, then the multiplication results
   in a signed infinity (the sign is determined by the rule above).
-  If neither NaN nor infinity is involved, then the exact mathematical product
   is computed.

   The product is rounded to the nearest value in the chosen value set by
   using the IEEE 754 '*round-to-nearest*' mode. The |LANG| programming
   language requires gradual underflow support as defined by IEEE 754
   (see :ref:`Floating-Point Types and Operations`).

   If the magnitude of the product is too large to represent, then the
   operation overflows, and the result is an appropriately signed infinity.


The evaluation of a multiplication operator '\*' never throws an error despite
possible overflow, underflow, or loss of information.

.. index::
   NaN
   infinity
   operand
   finite value
   multiplication
   signed infinity
   round-to-nearest
   underflow
   floating-point type
   floating-point operation
   overflow
   evaluation
   multiplication operator
   error
   loss of information

|

.. _Division:

Division
========

.. meta:
   frontend_status: Done
   todo: If either operand is NaN, the result should be NaN, but result is -NaN
   todo: Division of an infinity by a infinity should be NaN, but result is - NaN
   todo: Division of a nonzero finite value by a zero results should be signed infinity, but "Floating point exception(core dumped)" occurs

The binary operator '/' performs division, and returns the quotient of its
left-hand and right-hand operands (*dividend* and *divisor* respectively).

Integer division rounds toward *0*, i.e., the quotient of integer operands
*n* and *d*, after a numeric types conversion on both (see
:ref:`Predefined Numeric Types Conversions` for details), is
an integer value *q* with the largest possible magnitude that
satisfies :math:`|d\cdot{}q|\leq{}|n|`.

Note that *q* is:

-  Positive when \|n| :math:`\geq{}` \|d|, and *n* and *d* have the same sign,
   but
-  Negative when \|n| :math:`\geq{}` \|d|, and *n* and *d* have opposite signs.


.. index::
   division operator
   binary operator
   operand
   dividend
   divisor
   round-toward-zero
   integer division
   predefined numeric types conversion
   numeric type
   integer value

Only one special case does not comply with this rule: the integer overflow
occurs, and the result equals the dividend if the dividend is a negative
integer of the largest possible magnitude for its type, while the divisor
is *-1*.

This case throws no exception or error despite the overflow. However, if in
an integer division the divisor value is *0*, then an *ArithmeticError* is
thrown.

A floating-point division result is determined in compliance with the IEEE 754
arithmetic:

-  The result is NaN if:

   -  Either operand is NaN;
   -  Both operands are infinity.
   -  Both operands are zero.


.. index::
   integer overflow
   dividend
   negative integer
   floating-point division
   divisor
   exception
   error
   overflow
   integer division
   floating-point division
   NaN
   infinity
   operand

-  If the result is not NaN, the sign of the result is:

   -  Positive if both operands have the same sign; and
   -  Negative if the operands have different signs.


-  The division results in a signed infinity (the sign is determined by
   the rule above) if:

   -  Infinity is divided by a finite value;
   -  A nonzero finite value is divided by zero.


-  The division results in a signed zero (the sign is determined by the
   rule above) if:

   -  A finite value is divided by infinity;
   -  Zero is divided by any other finite value.

.. index::
   NaN
   operand
   division
   signed infinity
   finite value

-  If neither NaN nor infinity is involved, then the exact mathematical
   quotient is computed.

   If the magnitude of the product is too large to represent, then the
   operation overflows, and the result is an appropriately signed infinity.


The quotient is rounded to the nearest value in the chosen value set by
using the IEEE 754 '*round-to-nearest*' mode. The |LANG| programming
language requires gradual underflow support as defined by IEEE 754 (see
:ref:`Floating-Point Types and Operations`).

The evaluation of a floating-point division operator '/' never throws an error
despite possible overflow, underflow, division by zero, or loss of information.

.. index::
   infinity
   NaN
   overflow
   floating-point division
   round-to-nearest
   underflow
   floating-point types
   floating-point operation
   error
   exception
   loss of information
   division
   division operator

|

.. _Remainder:

Remainder
=========

.. meta:
    frontend_status: Done
    todo: If either operand is NaN, the result should be NaN, but result is -NaN
    todo: if the dividend is an infinity, or the divisor is a zero, or both, the result should be NaN, but this is -NaN

The binary operator '%' yields the remainder of its operands (*dividend* as
left-hand, and *divisor* as the right-hand operand) from an implied division.

The remainder operator in |LANG| accepts floating-point operands (unlike in
C and C++).

The remainder operation on integer operands (for the numeric type conversion
on both see :ref:`Predefined Numeric Types Conversions`) produces a result
value, i.e., :math:`(a/b)*b+(a\%b)` equals *a*.


.. index::
   remainder operator
   dividend
   divisor
   predefined numeric types conversion
   conversion
   floating-point operand
   remainder operation
   value
   integer operand
   implied division
   numeric types conversion
   numeric type
   conversion

This equality holds even in the special case where the dividend is a negative
integer of the largest possible magnitude of its type, and the divisor is *-1*
(the remainder is then *0*).

According to this rule, the result of the remainder operation can only be:

-  Negative if the dividend is negative, and
-  Positive if the dividend is positive.


The magnitude of the result is always less than that of the divisor.

If the value of the divisor for an integer remainder operator is *0*, then
the *ArithmeticError* is thrown.

A floating-point remainder operation result as computed by the operator '%' is
different than that produced by the remainder operation defined by IEEE 754.
The IEEE 754 remainder operation computes the remainder from a rounding
division (not a truncating division), and its behavior is different from that
of the usual integer remainder operator. Unlike that, |LANG| presumes that on
floating-point operations the operator '%' behaves in the same manner as the
integer remainder operator (comparable to the C library function *fmod*). The
standard library (see :ref:`Standard Library`) routine *Math.IEEEremainder*
can compute the IEEE 754 remainder operation.

.. index::
   dividend
   negative integer
   divisor
   remainder operation
   integer remainder
   value
   floating-point remainder operation
   floating-point operation
   truncating division
   rounding division

The result of a floating-point remainder operation is determined in compliance
with the IEEE 754 arithmetic:

-  The result is NaN if:

   -  Either operand is NaN;
   -  The dividend is infinity;
   -  The divisor is zero;
   -  The dividend is infinity, and the divisor is zero.


-  If the result is not NaN, the sign of the result is the same as the
   sign of the dividend.
-  The result equals the dividend if:

   -  The dividend is finite, and the divisor is infinity;
   -  If the dividend is zero, and the divisor is finite.

.. index::
   floating-point remainder operation
   remainder operation
   NaN
   infinity
   divisor
   dividend

-  If infinity, zero or NaN are not involved, then the floating-point remainder
   *r* from the division of the dividend *n* by the divisor *d* is determined
   by the mathematical relation :math:`r=n-(d\cdot{}q)`, in which *q* is an
   integer that is only:

   -  Negative if :math:`n/d` is negative, and
   -  Positive if :math:`n/d` is positive.


-  The magnitude of *q* is the largest possible without exceeding the
   magnitude of the true mathematical quotient of *n* and *d*.


The evaluation of the floating-point remainder operator '%' never throws
an error, even if the right-hand operand is zero. Overflow, underflow. or
loss of precision cannot occur.

.. index::
   infinity
   NaN
   floating-point remainder
   remainder operator
   dividend
   loss of precision
   operand

|

.. _Additive Expressions:

Additive Expressions
********************

.. meta:
    frontend_status: Done

The operators '+' and '-' are the *additive operators*:

.. code-block:: abnf

    additiveExpression:
        expression '+' expression
        | expression '-' expression
        ;

The additive operators group left-to-right.

If either operand of the operator is '+' of type *string*, then the operation
is a string concatenation. In all other cases, the type of each operand of the
operator '+' must be convertible (see :ref:`Kinds of Conversion`) to a
primitive numeric type; a compile-time error occurs otherwise.

The type of each operand of the binary operator '-' in all cases must be
convertible (see :ref:`Kinds of Conversion`) to a primitive numeric type;
a compile-time error occurs otherwise.

.. index::
   additive expression
   additive operator
   operand
   type string
   string concatenation
   operator
   conversion
   primitive type
   numeric type
   compile-time error
   binary operator

|

.. _String Concatenation:

String Concatenation
====================

.. meta:
    frontend_status: Done

If one operand of an expression is of type *string*, then the string
conversion (see :ref:`Operator Contexts`) is performed on the other operand
at runtime to produce a string.

String concatenation produces a reference to a *string* object that is a
concatenation of two operand strings. The left-hand operand characters precede
the right-hand operand characters in the newly created string.

If the expression is not a constant expression (see :ref:`Constant Expressions`),
then a new *string* object is created (see :ref:`New Expressions`).

.. index::
   string concatenation operator
   operand
   type string
   string conversion
   operator context
   runtime
   operand string
   precedence
   expression
   constant expression

|

.. _Additive Operators for Numeric Types:

Additive Operators '+' and '-' for Numeric Types
================================================

.. meta:
   frontend_status: Done
   todo: The sum of two infinities of opposite sign should be NaN, but it is -NaN

The binary operator '+' applied to two numeric type operands performs addition,
and produces the sum of such operands.

The binary operator '-' performs subtraction, and produces the difference of
two numeric operands.

The numeric types conversion (see :ref:`Predefined Numeric Types Conversions`)
is performed on the operands.

The type of an additive expression on numeric operands is the promoted type of
that expression’s operands. If such promoted type is:

-  *int* or *long*, then integer arithmetic is performed;
-  *float* or *double*, then floating-point arithmetic is performed.

.. index::
   additive operator
   numeric type
   binary operator
   type operand
   addition
   subtraction
   numeric operand
   predefined numeric types conversion
   floating-point arithmetic
   integer arithmetic
   promoted type
   expression
   additive expression

If operand expressions have no side effects, then addition is a commutative
operation.

If all operands are of the same type, then integer addition is associative.

Floating-point addition is not associative.

If overflow occurs on an integer addition, then:

-  The result is the low-order bits of the mathematical sum as represented in
   a sufficiently large two’s-complement format.
-  The sign of the result is different than that of the mathematical sum of
   the operands’ values.


The result of a floating-point addition is determined in compliance with the
IEEE 754 arithmetic:

.. index::
   operand expression
   expression
   side effect
   addition
   commutative operation
   operation
   two’s-complement format
   operand value
   overflow
   floating-point addition
   associativity

-  The result is NaN if:

   -  Either operand is NaN;
   -  The operands are two infinities of opposite sign.


-  The sum of two infinities of the same sign is the infinity of that sign.
-  The sum of infinity and a finite value equals the infinite operand.
-  The sum of two zeros of opposite sign is positive zero.
-  The sum of two zeros of the same sign is zero of that sign.
-  The sum of zero and a nonzero finite value is equal to the nonzero operand.
-  The sum of two nonzero finite values of the same magnitude and opposite sign
   is positive zero.
-  If infinity, zero, or NaN are not involved, and the operands have the same
   sign or different magnitudes, then the exact sum is computed mathematically.


If the magnitude of the sum is too large to represent, then the operation
overflows, and the result is an appropriately signed infinity.

.. index::
   NaN
   infinity
   signed infinity
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
using the IEEE 754 '*round-to-nearest mode*'. The |LANG| programming language
requires gradual underflow support as defined by IEEE 754 (see
:ref:`Floating-Point Types and Operations`).

When applied to two numeric type operands, the binary operator '-' performs
subtraction, and returns the difference of such operands (*minuend* as left-hand,
and *subtrahend* as the right-hand operand).

The result of *a-b* is always the same as that of *a+(-b)* in both integer and
floating-point subtraction.

The subtraction from zero for integer values is the same as negation. However,
the subtraction from zero for floating-point operands and negation is *not*
the same (if *x* is *+0.0*, then *0.0-x* is *+0.0*, however *-x* is *-0.0*).

The evaluation of a numeric additive operator never throws an error despite
possible overflow, underflow, or loss of information.

.. index::
   round-to-nearest mode
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
   negation
   overflow
   additive operator
   error

|

.. _Shift Expressions:

Shift Expressions
*****************

.. meta:
    frontend_status: Done
    todo: spec issue: uses 'L' postfix in example "(n >> s) + (2L << ~s)", we don't have it

The operators '<<' (left shift), '>>' (signed right shift), and '>>>' (unsigned
right shift) are the *shift operators*. The value to be shifted is the left-hand
operand in a shift operator; the right-hand operand specifies the shift distance.

.. code-block:: abnf

    shiftExpression:
        expression '<<' expression
        | expression '>>' expression
        | expression '>>>' expression
        ;

The shift operators group left-to-right.

Numeric types conversion (see :ref:`Predefined Numeric Types Conversions`)
is performed separately on each operand to ensure that both operands are of
primitive integer type. Note that if the initial type of one or both operands
is ``double`` or ``float``, then such operand or operands are truncated to
appropriate integer type first.

A compile-time error occurs if either operand in a shift operator (after unary
numeric promotion) is not a primitive integer type.

.. index::
   shift expression
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
   primitive integer type

The shift expression type is the promoted type of the left-hand operand.

If the left-hand operand is of the promoted type *int*, then only five
lowest-order bits of the right-hand operand specify the shift distance
(as if using a bitwise logical AND operator '&' with the mask value *0x1f*
or *0b11111* on the right-hand operand), and thus it is always within
the inclusive range of *0* through *31*.

If the left-hand operand is of the promoted type *long*, then only six
lowest-order bits of the right-hand operand specify the shift distance
(as if using a bitwise logical AND operator '&' with the mask value *0x3f*
or *0b111111* the right-hand operand), and thus it is always within the
inclusive range of *0* through *63*.

Shift operations are performed on the two’s-complement integer
representation of the value of the left operand at runtime.

The value of *n* << *s* is *n* left-shifted by *s* bit positions. It is
quivalent to multiplication by two to the power *s* even in case of an
overflow.

.. index::
   shift expression
   promoted type
   operand
   shift distance
   bitwise logical AND operator
   mask value
   shift operation
   multiplication
   overflow
   two’s-complement integer representation
   runtime

The value of *n* >> *s* is *n* right-shifted by *s* bit positions with
sign-extension; the resultant value is :math:`floor(n / 2s)`. If *n* is
non-negative, then it is equivalent to truncating integer division (as computed
by the integer division operator by 2 to the power *s*).

The value of *n* >>> *s* is *n* right-shifted by *s* bit positions with
zero-extension, where:

-  If *n* is positive, then the result is the same as that of *n* >> *s*.
-  If *n* is negative, and the type of the left-hand operand is *int*, then
   the result is equal to that of the expression (*n* >> *s*) + (*2* << *s*).
-  If *n* is negative, and the type of the left-hand operand is *long*, then
   the result is equal to that of the expression (*n* >> *s*) + (*2L* << *s*).

.. index::
   sign-extension
   truncating integer division
   integer division operator
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

The numerical comparison operators '<', '>', '<=', and '>=' are *relational
operators*.

.. code-block:: abnf

    relationalExpression:
        expression '<' expression
        | expression '>' expression
        | expression '<=' expression
        | expression '>=' expression
        ;

The relational operators group left-to-right.

A relational expression is always of type *boolean*.

.. index::
   numerical comparison operator
   relational operator
   relational expression
   boolean

|

.. _Numerical Comparison Operators:

Numerical Comparison Operators <, <=, >, and >=
===============================================

.. meta:
    frontend_status: Done

The type of each operand in a numerical comparison operator must be convertible
(see :ref:`Kinds of Conversion`) to a primitive numeric type; a compile-time
error occurs otherwise.

Numeric types conversions (see :ref:`Predefined Numeric Types Conversions`) are
performed on each operand as follows:

-  Signed integer comparison if the converted type of the operand is *int* or *long*.

-  Floating-point comparison if the converted type of the operand is *float* or *double*.

.. index::
   numerical comparison operator
   operand
   conversion
   compile-time error
   numeric type
   numeric types conversion
   predefined numeric types conversion
   signed integer comparison
   floating-point comparison
   converted type

The comparison of floating-point values drawn from any value set must be accurate.

According to the IEEE 754 standard specification, the result of a floating-point
comparison is:

-  False if either operand is NaN.
-  All values other than NaN must be ordered with

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

Based on the above presumption, the following rules apply to integer operands,
or floating-point operands other than NaN:

-  The value produced by the operator '<' is ``true`` if the value of the
   left-hand operand is less than that of the right-hand operand, and ``false``
   otherwise.
-  The value produced by the operator '<=' is ``true`` if the value of the
   left-hand operand is less than or equal to that of the right-hand
   operand, and ``false`` otherwise.
-  The value produced by the operator '>' is ``true`` if the value of the
   left-hand operand is greater than that of the right-hand operand, and
   ``false`` otherwise.
-  The value produced by the operator '>=' is ``true`` if the value of the
   left-hand operand is greater than or equal to that of the right-hand operand,
   and ``false`` otherwise.

.. index::
   integer operand
   floating-point operand
   NaN
   operator

|

.. _Equality Expressions:

Equality Expressions
********************

.. meta:
    frontend_status: Partly

The |LANG| programming language has two forms of equality expressions:

.. code-block:: abnf

    equalityExpression:
        expression ('===' | '==') expression
        | expression ('!==' | '!=') expression
        ;

The two forms have the same semantics regardless of the number of the symbols
'=' used in the operator. The form of the operator '===' is identical to '==',
and '!==' is identical to '!='.
The exceptions are as follows:

- "5" == 5 vs. "5" === 5
- null == undefined vs. null === undefined

.. index::
   equality expression
   undefined
   null

.. code-block:: typescript
   :linenos:

   console.log (null == undefined) // true
   console.log (null === undefined) // false

   cmp2("5", 5) // true
   cmp3 ("5", 5) // false
   function cmp2 (o1: Object, o2: Object) {
     console.log ( o1 == o2 )
   }
   function cmp3 (o1: Object, o2: Object) {
     console.log ( o1 === o2 )
   }


Any equality expression is of type *boolean*.

The equality operators group left-to-right.

The equality operators are commutative if operand expressions cause no side
effects.

The equality operators are similar to the relational operators except for
their lower precedence (:math:`a < b==c < d` is ``true`` if both :math:`a < b`
and :math:`c < d` have the same *truth* value).

The equality operator semantics has two kinds:

-  *reference equality test*
-  *value equality test*

.. index::
   equality expression
   boolean
   equality operator
   commutative
   operand expression
   side effect
   relational operator
   precedence
   reference equality test
   value equality test

The application of this semantics depends on what kinds of objects are being
compared:

- All entities of the class, function and array types are compared by using
  the reference equality operators (see :ref:`Reference Equality Operators`),
- All entities of primitive types and their boxed versions are compared by
  using the value equality operators (see :ref:`Value Equality Operators`),
  and applying conversions if required.

.. index::
   object
   reference equality operator
   entity
   primitive type entity
   boxing
   boxed version
   value equality operator
   conversion

This semantics is illustrated by the example below:

.. code-block:: typescript
   :linenos:

   5 == 5 // true
   "a string" == "a string" // true
   5 == 5.0 // true

   class X {}
   new X() == new X() // false, two different object of class X
   let x1 = new X()
   let x2 = x1
   x1 == x2 // true, as x1 and x2 refer to the same object

   [1, 2, 3] == [1, 2, 3] // false, two different array objects
   let y1 = [1, 2, 3]
   let y2 = y1
   y1 == y2 // true, as y1 and y2 refer to the same array

   ((): void => {}) == ((): void => {}) // false, two different functions
   let z1 = (): void => {}
   let z2 = z1
   z1 == z2 // true, as z1 and z2 refer to the same function

   // Note - part of experimental features!
   5 == new Int(5) // true
   new Int(5) == new Double(5) // true

Any entity can be compared with *null*, but such comparison can return ``true``
only for the entities of *nullable* types if they actually have the *null*
value during the program execution. Tests of all other kinds return ``false``,
which is to be known during compilation.

.. index::
   entity
   comparison
   null
   nullable type
   execution
   compilation

.. code-block:: typescript
   :linenos:

   // Compile-time checks

   5 == null // false
   "a string" == null // false
   5.0 == null // false

   class X {}
   new X() == null // false

   [1, 2, 3] == null // false

   ((): void => {}) == null // false

   // Runtime checks

   let x: X | null = null
   x == null // true, as x was assigned with null

   let y: int[] | null = null
   y == null // true, as y was assigned with null

   let z: (() => void) | null = (): void => {}
   z == null // false, as z was assigned with a function

   // Note - part of experimental features!
   null == new Int(5) // false
   new Double(5) == null // false

   let i: Int|null = null
   i == null // true, runtime check   

|

.. _Reference Equality Operators:

Reference Equality Operators
============================

.. meta:
    frontend_status: Partly

The reference equality operator can compare two reference type operands
(entities of the class, function and array types).

A compile-time error occurs if:

-  Any operand is not of a reference type.

-  Casting conversion (see :ref:`Casting Contexts`) cannot convert the type
   of either operand to the type of the other.


The result of ':math:`==`' at runtime is ``true`` if both operand values:

-  Are ``null``; or
-  Refer to the same object, array, or function.


The result is ``false`` otherwise.

The result of ':math:`!=`' is ``false`` if both operand values:

-  Are ``null``; or
-  Refer to the same object, array, or function.


The result is ``true`` otherwise.

.. index::
   reference equality operator
   reference type operand
   entity
   class
   function
   array type
   compile-time error
   reference type
   casting conversion
   casting context
   conversion
   operand value
   object
   array

|

.. _Value Equality Operators:

Value Equality Operators
========================

.. meta:
    frontend_status: Partly

Value equality operators can compare two operands that are:

-  Convertible to a numeric type;
-  Of type *boolean*, or *Boolean*; or
-  Of type *char*, or *Char*.


A compile-time error occurs otherwise.

.. index::
   compile-time error
   value equality operator
   comparison
   operand
   conversion
   numeric type
   boolean
   Boolean
   Char
   char

|

.. _Value Equality Operators for Numeric Types:

Value Equality Operators for Numeric Types
==========================================

.. meta:
    frontend_status: Partly

The numeric types conversion (see :ref:`Predefined Numeric Types Conversions`)
is performed on the operands of a value equality operator if:

-  Both are of a numeric type; or
-  One is of a numeric type, and the other is convertible to a numeric type.

If the converted type of the operands is ``int`` or ``long``, then an
integer equality test is performed .

If the converted type is *float* or *double*, then a floating-point equality
test is performed.

The floating-point equality test must follow the IEEE 754 standard rules:

.. index::
   value equality operator
   numeric type
   numeric types conversion
   predefined numeric types conversion
   converted type
   floating-point equality test
   operand
   conversion
   integer equality test

-  The result of ':math:`==`' is ``false`` but the result of ':math:`!=`' is
   ``true`` if either operand is NaN.

   The test :math:`x != x` is ``true`` only if *x* is NaN.

-  Positive zero equals negative zero.

-  The equality operators consider two distinct floating-point values unequal
   in any other situation.

   For example, if one value represents positive infinity, and the other
   represents negative infinity, then each compares equal to itself, and
   unequal to all other values.

Based on the above presumptions, the following rules apply to integer operands,
or floating-point operands other than NaN:

-  If the value of the left-hand operand is equal to that of the right-hand
   operand, then the ':math:`==`' operator produces the value ``true``.
   Otherwise, the result is ``false``.

-  If the value of the left-hand operand is not equal to that of the right-hand
   operand, then the ':math:`!=`' operator produces the value ``true``.
   Otherwise, the result is ``false``.

.. index::
   NaN
   positive zero
   negative zero
   equality operator
   floating-point value
   floating-point operand
   positive infinity
   negative infinity
   comparison
   operand
   operator

|

.. _Value Equality Operators for Boolean Types:

Value Equality Operators for Boolean Types
==========================================

.. meta:
    frontend_status: Partly

The operation is a *boolean equality* if:

-  Both operands of a value equality operator are of type *boolean*; or

-  One operand is of type *boolean*, and the other is of type *Boolean*.

The boolean equality operators are associative.

If one operand is of type *Boolean*, then the unboxing conversion (see
:ref:`Predefined Numeric Types Conversions`) must be performed.

If both operands (after the unboxing conversion if required) are either
``true`` or ``false``, then the result of ':math:`==`' is ``true``.
Otherwise, the result is ``false`` otherwise.

If both operands are either ``true`` or ``false``, then the result of
':math:`!=`' is false``.
Otherwise, the result is ``true``.

.. index::
   value equality operator
   type boolean
   type Boolean
   boolean equality
   value equality operator
   associativity
   operand
   unboxing conversion
   numeric type
   predefined numeric types conversion

|

.. _Value Equality Operators for Character Types:

Value Equality Operators for Character Types
============================================

.. meta:
    frontend_status: Partly

The operation is a *character equality* if:

-  Both operands of a value equality operator are of type *char*; or

-  One operand is of type *char*, and the other is of type *Char*.

The character equality operators are associative.

If one operand is of the *Char* type, then the unboxing conversion
(see :ref:`Predefined Numeric Types Conversions`) must be performed.

If both operands (after the unboxing conversion where required) contain
the same character code, then the result of ':math:`==`' is ``true``.
Otherwise, the result is ``false``.

If both operands contain different character codes, then the result
of ':math:`!=`' is ``false``.
Otherwise, the result is ``true``.

.. index::
   value equality operator
   equality operator
   character type
   operation
   character equality
   associativity
   unboxing conversion
   predefined numeric types conversion
   numeric types
   character code

|

.. _Bitwise and Logical Expressions:

Bitwise and Logical Expressions
*******************************

.. meta:
    frontend_status: Done

The *bitwise operators* and *logical operators* are as follows:

-  AND operator '&';
-  Exclusive OR operator '^'; and
-  Inclusive OR operator '\|'.


.. code-block:: abnf

    bitwiseAndLogicalExpression:
        expression '&' expression
        | expression '^' expression
        | expression '|' expression
        ;

These operators have different precedence. The operator '&' has the highest,
and ':math:`|`' has the lowest precedence.

The operators group left-to-right. Each operator is commutative if the
operand expressions have no side effects, and associative.

The bitwise and logical operators can compare two operands of a numeric
type, or two operands of the *boolean* type. A compile-time error occurs
otherwise.

.. index::
   bitwise operator
   logical operator
   bitwise expression
   logical expression
   type boolean
   compile-time error
   operand expression
   precedence
   exclusive OR operator
   inclusive OR operator
   AND operator
   commutative operator
   side effect
   numeric type
   associativity

|

.. _Integer Bitwise Operators:

Integer Bitwise Operators &, ^, and |
=====================================

.. meta:
    frontend_status: Done

The numeric types conversion (see :ref:`Predefined Numeric Types Conversions`)
is first performed on the operands of an operator '&', '^', or '\|' if both
such operands are of a type convertible (see :ref:`Kinds of Conversion`) to a
primitive integer type. Note that if the initial type of one or both operands
is ``double`` or ``float``, then that operand or operands are truncated to the
appropriate integer type first.

A bitwise operator expression type is the converted type of its operands.

The resultant value of '&' is the bitwise AND of the operand values.

The resultant value of '^' is the bitwise exclusive OR of the operand values.

The resultant value of '\|' is the bitwise inclusive OR of the operand values.


.. index::
   integer bitwise operator
   numeric types conversion
   predefined numeric types conversion
   bitwise exclusive OR operand
   bitwise inclusive OR operand
   bitwise AND operand
   primitive integer type
   primitive type
   integer type
   conversion

|

.. _Boolean Logical Operators:

Boolean Logical Operators &, ^, and |
=====================================

.. meta:
    frontend_status: Done

The type of the bitwise operator expression is *boolean* if both operands of a
'&', '^', or '\|' operator are of type *boolean* or *Boolean*. In any case,
the unboxing conversion (see :ref:`Predefined Numeric Types Conversions`) is
performed on the operands as necessary.

If both operand values are ``true``, then the resultant value of '&' is ``true``.
Otherwise, the result is ``false``.

If the operand values are different, then the resultant value of ‘^’ is ``true``.
Otherwise, the result is ``false``.

If both operand values are ``false``, then the resultant value of is ``false``.
Otherwise, the result is ``true``.

.. index::
   boolean logical operator
   bitwise operator expression
   unboxing conversion
   conversion
   predefined numeric types conversion
   numeric types conversion
   numeric type
   operand value

|

.. _Conditional-And Expression:

Conditional-And Expression
**************************

.. meta:
    frontend_status: Done

The *conditional-and* operator '&&' is similar to '&' (see
:ref:`Bitwise and Logical Expressions`) but evaluates its right-hand
operand only if the left-hand operand’s value is ``true``.

The computation results of '&&' and '&' on *boolean* operands are
the same unless the right-hand operand expression is evaluated
conditionally.

.. code-block:: abnf

    conditionalAndExpression:
        expression '&&' expression
        ;

The *conditional-and* operator groups left-to-right.

The *conditional-and* operator is fully associative as regards both the result
value and side effects (i.e., the evaluations of the expressions *((a)* &&
*(b))* && *(c)* and *(a)* && *((b)* && *(c))* produce the same result, and the
same side effects occur in the same order for any *a*, *b*, and *c*).

.. index::
   conditional-and operator
   conditional-and expression
   bitwise expression
   logical expression
   boolean operand
   conditional evaluation
   evaluation
   expression

A *conditional-and* expression is always of type *boolean*.

Each operand of the *conditional-and* operator must be of type *boolean*, or
*Boolean*; a compile-time error occurs otherwise.

The left-hand operand expression is first evaluated at runtime. If the result
is of the *Boolean* type, then the unboxing conversion (see
:ref:`Predefined Numeric Types Conversions`) is performed as follows:

-  If the resultant value is ``false``, then the value of the *conditional-and*
   expression is ``false``, and the evaluation of the right-hand operand
   expression is omitted.

-  If the value of the left-hand operand is ``true``, then the right-hand
   expression is evaluated. If the result of the evaluation is of type
   *Boolean*, then it is subjected to the unboxing conversion (see
   :ref:`Predefined Numeric Types Conversions`). The resultant value is the
   value of the *conditional-and* expression.

.. index::
   conditional-and expression
   conditional-and operator
   compile-time error
   unboxing conversion
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

The *conditional-or* operator ':math:`||`' is similar to ':math:`|`' (see
:ref:`Integer Bitwise Operators`) but evaluates its right-hand operand
only if the value of its left-hand operand is ``false``.

.. code-block:: abnf

    conditionalOrExpression:
        expression '||' expression
        ;

The *conditional-or* operator groups left-to-right.

The *conditional-or* operator is fully associative as regards both the result
value and side effects (i.e., the evaluations of the expressions *((a)* \|\|
*(b))* \|\| *(c)* and *(a)* \|\| *((b)* \|\| *(c))* produce the same result,
and the same side effects occur in the same order for any *a*, *b*, and *c*).

A *conditional-or* expression is always of type *boolean*.

.. index::
   conditional-or expression
   conditional-or operator
   integer bitwise expression
   associativity
   expression
   side effect
   evaluation

Each operand of the *conditional-or* operator must be of type *boolean*, or
*Boolean*; a compile-time error occurs otherwise.

The left-hand operand expression is first evaluated at runtime. If the
result is of the *Boolean* type, then the *unboxing conversion ()* is performed
as follows:

-  If the resultant value is ``true``, then the value of the *conditional-or*
   expression is ``true``, and the evaluation of the right-hand operand
   expression is omitted.

-  If the resultant value is ``false``, then the right-hand expression is
   evaluated. If the result of the evaluation is of type *Boolean*, then
   the *unboxing conversion* (see :ref:`Predefined Numeric Types Conversions`)
   is performed. The resultant value is the value of the *conditional-or*
   expression.

The computation results of '\|\|' and '\|' on the *boolean* operands are the
same unless the right-hand operand expression is evaluated conditionally.

.. index::
   conditional-or expression
   conditional-or operator
   compile-time error
   runtime
   Boolean type
   unboxing conversion
   expression
   boolean operand
   predefined numeric types conversion
   numeric types conversion
   numeric type
   conditional evaluation

|

.. _Assignment:

Assignment
**********

.. meta:
    frontend_status: Partly
    todo: nullable field access
    todo: nullable field access

All *assignment operators* group right-to-left (i.e., :math:`a=b=c` means
:math:`a=(b=c)`---and thus assign the value of *c* to *b*, and then the value
of *b* to *a*).

.. code-block:: abnf

    assignmentExpression:
        expression1 assignmentOperator expression2
        ;

    assignmentOperator
        : '='
        | '+='  | '-='  | '*='   | '='  | '%='
        | '<<=' | '>>=' | '>>>='
        | '&='  | '|='  | '^='
        ;

The result of the first operand in an assignment operator (represented by
expression1) must be one of the following:

-  A named variable, such as a local variable, or a field of the current
   object or class; or
-  A computed variable resultant from a field access (see
   :ref:`Field Access Expressions`); or
-  An array or record component access (see :ref:``Indexing Expression``).

.. index::
   assignment
   assignment operator
   operand
   field
   variable
   local variable
   class
   object
   field access
   array
   field access expression
   indexing expression
   record component access

A compile-time error occurs if *expression1* contains the optional chaining
operator '``?.``' (see :ref:`Chaining Operator`).

A compile-time error occurs if the result of *expression1* is not a variable.

The type of the variable is the type of the assignment expression.

The result of the assignment expression at runtime is not itself a variable
but the value of a variable after the assignment.

.. index::
   compile-time error
   chaining operator
   variable
   assignment
   expression
   runtime

|

.. _Simple Assignment Operator:

Simple Assignment Operator
==========================

.. meta:
    frontend_status: Partly

A compile-time error occurs if the type of the right-hand operand
(*expression2*) is not compatible (see :ref:`Compatible Types`) with
the type of the variable (see :ref:`Generic Parameters`). Otherwise,
the expression is evaluated at runtime in one of the following ways:

#. If the left-hand operand *expression1* is a field access expression
   *e.f* (see :ref:`Field Access Expressions`), possibly enclosed in a
   pair of parentheses, then:

   #. *expression1* *e* is evaluated: if the evaluation of *e*
      completes abruptly, then so does the assignment expression.
   #. The right-hand operand *expression2* is evaluated: if the evaluation
      completes abruptly, then so does the assignment expression.
   #. The value of the right-hand operand as computed above is assigned
      to the variable denoted by *e.f*.

.. index::
   simple assignment operator
   compile-time error
   compatible type
   type compatibility
   field access
   field access expression
   runtime
   abrupt completion
   evaluation
   assignment expression
   variable

#. If the left-hand operand is an array access expression (see
   :ref:`Array Indexing Expression`), possibly enclosed in a pair of
   parentheses, then:

   #. The array reference subexpression of the left-hand operand is evaluated.
      If this evaluation completes abruptly, then so does the assignment
      expression. In that case, the right-hand operand and the index
      subexpression are not evaluated, and the assignment does not occur.
   #. If the evaluation completes normally, then the index subexpression of
      the left-hand operand is evaluated. If this evaluation completes
      abruptly, then so does the assignment expression. In that case, the
      right-hand operand is not evaluated, and the assignment does not occur.
   #. If the evaluation completes normally, then the right-hand operand is
      evaluated. If this evaluation completes abruptly, then so does the
      assignment expression, and the assignment does not occur.
   #. If the evaluation completes normally, but the value of the index
      subexpression is less than zero, or greater than, or equal to the
      *length* of the array, then *ArrayIndexOutOfBoundsError* is thrown,
      and the assignment does not occur.
   #. Otherwise, the value of the index subexpression is used to select an
      element of the array referred to by the value of the array reference
      subexpression.


      Such element is a variable of type *SC*; if *TC* is the type of
      the left-hand operand of the assignment operator determined at compile
      time, there are two options:

      - If *TC* is a primitive type, then *SC* can only be the same as *TC*.


        The value of the right-hand operand is converted to the type of the
        selected array element, and the value set conversion (see
        :ref:`Kinds of Conversion`) is performed to convert it to the
        appropriate standard value set (not an extended-exponent value set).
        The result of that conversion is stored into the array element.
      - If *TC* is a reference type, then *SC* can be the same as *TC* or
        a type that extends or implements *TC*.

        If an |LANG| compiler cannot guarantee at compile time that the array
        element is exactly of type *TC*, then a check must be performed
        at runtime to ensure that the class *RC* (assuming that *RC* is the
        class of the object referred to by the value of the right-hand operand
        at runtime) is compatible (see :ref:`Type Compatibility with Initializer`)
        with the actual type *SC* of the array element.

        If that class *RC* is not assignable to type *SC*, then
        *ArrayStoreError* is thrown, and the assignment does not occur.

        Otherwise, the reference value of the right-hand operand is stored in
        the selected array element.

.. index::
   array access expression
   array indexing expression
   array reference subexpression
   evaluation
   abrupt completion
   normal completion
   assignment
   assignment expression
   index subexpression
   reference subexpression
   variable
   assignment operator
   compile time
   primitive type
   operand
   conversion
   extended-exponent value set
   standard value set
   reference type
   class
   type compatibility
   compatible type
   array element
   error
   reference value

#. If the left-hand operand is a record access expression (see
   :ref:``Record Indexing Expression``), possibly enclosed in a pair of
   parentheses, then:

   #. The object reference subexpression of the left-hand operand is evaluated.
      If this evaluation completes abruptly, then so does the assignment
      expression. In that case, the right-hand operand and the index
      subexpression are not evaluated, and the assignment does not occur.
   #. If the evaluation completes normally, the index subexpression of the
      left-hand operand is evaluated. If this evaluation completes abruptly,
      then so does the assignment expression. In that case, the right-hand
      operand is not evaluated, and the assignment does not occur.
   #. If the evaluation completes normally, the right-hand operand is evaluated.
      If this evaluation completes abruptly, then so does the assignment
      expression. In that case, the assignment does not occur.
   #. Otherwise, the value of the index subexpression is used as the *key*.
      In that case, the right-hand operand is used as the *value*, and the
      key-value pair is stored in the record instance.

.. index::
   operand
   record access expression
   record indexing expression
   object reference subexpression
   index subexpression
   assignment expression
   evaluation
   value
   key-value pair
   record instance
   normal completion
   abrupt completion
   key

|

If none of the above is true, then three steps are required:

#. The left-hand operand is evaluated to produce a variable. If the
   evaluation completes abruptly, then so does the assignment expression.
   In that case, the right-hand operand is not evaluated, and the assignment
   does not occur.

#. If the evaluation completes normally, then the right-hand operand is
   evaluated. If the evaluation completes abruptly, then so does the assignment
   expression. In that case, the assignment does not occur.

#. If that evaluation completes normally, then the value of the right-hand
   operand is converted to the type of the left-hand variable. In that case,
   the result of the conversion is stored into the variable.

.. index::
   evaluation
   assignment expression
   abrupt completion
   normal completion
   conversion
   variable
   expression

|

.. _Compound Assignment Operators:

Compound Assignment Operators
=============================

.. meta:
    frontend_status: Partly

A compound assignment expression in the form *E1 op= E2* is equivalent to
*E1 = ((E1) op (E2)) as T* (where *T* is the type of *E1*, except that *E1*
is evaluated only once). This expression can be evaluated at runtime in one
of the two ways as follows:

#. If the left-hand operand expression is not an indexing expression:

   -  The left-hand operand is evaluated to produce a variable. If the
      evaluation completes abruptly, then so does the assignment expression.
      In that case, the right-hand operand is not evaluated, and the
      assignment does not occur.

   -  If the evaluation completes normally, then the value of the left-hand
      operand is saved, and the right-hand operand is evaluated. If the
      evaluation completes abruptly, then so does the assignment expression.
      In that case, the assignment does not occur.

   -  If the evaluation completes normally, then the saved value of the
      left-hand variable, and the value of the right-hand operand are
      used to perform the binary operation as indicated by the compound
      assignment operator. If the operation completes abruptly, then so
      does the assignment expression. In that case, the assignment
      does not occur.

   -  If the evaluation completes normally, then the result of the binary
      operation converts to the type of the left-hand variable. The result
      of such conversion is stored into the variable.

.. index::
   compound assignment operator
   evaluation
   expression
   runtime
   operand
   indexing expression
   variable
   assignment
   abrupt completion
   normal completion
   assignment expression
   binary operation
   conversion

#. If the left-hand operand expression is an array access expression (see
   :ref:`Array Indexing Expression`), then:

   -  The array reference subexpression of the left-hand operand is
      evaluated. If the evaluation completes abruptly, then so does
      the assignment expression. In that case, the right-hand operand
      and the index subexpression are not evaluated, and the assignment
      does not occur.

   -  If the evaluation completes normally, then the index subexpression
      of the left-hand operand is evaluated. If the evaluation completes
      abruptly, then so does the assignment expression. In that case,
      the right-hand operand is not evaluated, and the assignment does
      not occur.

   -  If the evaluation completes normally, the value of the array
      reference subexpression refers to an array, and the value of the
      index subexpression is less than zero, greater than, or equal to
      the *length* of the array, then *ArrayIndexOutOfBoundsError* is
      thrown. In that case, the assignment does not occur.

   -  If the evaluation completes normally, then the value of the index
      subexpression is used to select an array element referred to by
      the value of the array reference subexpression. The value of this
      element is saved, and then the right-hand operand is evaluated.
      If the evaluation completes abruptly, then so does the assignment
      expression. In that case, the assignment does not occur.

   -  If the evaluation completes normally, consideration must be given
      to the saved value of the array element selected in the previous
      step. While this element is a variable of type *S*, and *T* is
      the type of the left-hand operand of the assignment operator
      determined at compile time:

      - If *T* is a primitive type, then *S* is the same as *T*.

        The saved value of the array element, and the value of the right-hand
        operand are used to perform the binary operation of the compound
        assignment operator.

        If this operation completes abruptly, then so does the assignment
        expression. In that case, the assignment does not occur.

        If this evaluation completes normally, then the result of the binary
        operation converts to the type of the selected array element. The
        result of the conversion is stored into the array element.

      - If *T* is a reference type, then it must be *string*.

        *S* must also be a *string* because the class *string* is the *final*
        class. The saved value of the array element, and the value of the
        right-hand operand are used to perform the binary operation (string
        concatenation) of the compound assignment operator (that must be
        ':math:`+=`'). If this operation completes abruptly, then so does the
        assignment expression. In that case, the assignment does not occur.

        If this evaluation completes normally, then the *string* result of
        the binary operation is stored into the array element.

.. index::
   expression
   operand
   access expression
   array indexing expression
   array reference subexpression
   evaluation
   abrupt completion
   normal completion
   index subexpression
   assignment expression
   array
   error
   assignment
   index subexpression
   primitive type
   evaluation
   binary operation
   conversion
   array element

#. If the left-hand operand expression is a record access expression (see
   :ref:`Record Indexing Expression`):

   -  The object reference subexpression of the left-hand operand is
      evaluated. If this evaluation completes abruptly, then so does the
      assignment expression. In that case, the right-hand operand and
      the index subexpression are not evaluated, and the assignment does
      not occur.

   -  If this evaluation completes normally, the index subexpression of
      the left-hand operand is evaluated. If the evaluation completes
      abruptly, then so does the assignment expression. In that case,
      the right-hand operand is not evaluated, and the assignment does
      not occur.

   -  If this evaluation completes normally, the the value of the object
      reference subexpression and the value of index subexpression are saved,
      then the right-hand operand is evaluated. If the evaluation completes
      abruptly, then so does the assignment expression. In that case, the
      assignment does not occur.

   -  If this evaluation completes normally, the saved values of the
      object reference subexpression and index subexpression (as the *key*)
      are used to get the *value* that is mapped to the *key* (see
      :ref:`Record Indexing Expression`), then this *value*, and the value
      of the right-hand operand are used to perform the binary operation as
      indicated by the compound assignment operator. If the operation
      completes abruptly, then so does the assignment expression. In that
      case, the assignment does not occur.

   -  If this evaluation completes normally, then the result of the binary
      operation is stored as the key-value pair in the record instance
      (as in :ref:`Simple Assignment Operator`).

.. index::
   record access expression
   operand expression
   record indexing expression
   object reference subexpression
   evaluation
   assignment expression
   abrupt completion
   normal completion
   assignment
   object reference subexpression
   index subexpression
   key
   key-value pair
   record indexing expression
   record instance
   value
   compound assignment operator
   binary operation

|

.. _Conditional Expressions:

Conditional Expressions
***********************

.. meta:
    frontend_status: Done
    todo: implement full LUB support (now only basic LUB implemented)

The conditional expression ':math:`? :`' uses the boolean value of the first
expression to decide which of the other two expressions to evaluate.

.. code-block:: abnf

    conditionalExpression:
        expression '?' expression ':' expression
        ;

The conditional operator ':math:`? :`' groups right-to-left (i.e.,
:math:`a?b:c?d:e?f:g`, and :math:`a?b:(c?d:(e?f:g))` have the same meaning).

The conditional operator ':math:`? :`' consists of three operand expressions
with the separators ':math:`?`' between the first and the second, and
':math:`:`' between the second and the third expression.

A compile-time error occurs unless the first expression is of type
*boolean* or *Boolean*.

Conditional expressions are classified by their second and third operand
expressions:

.. index::
   conditional expression
   boolean
   value
   expression
   evaluation
   conditional operator
   operand expression
   separator
   Boolean

#. *boolean conditional expressions* (the type of both the second and the
   third operand expressions are either *boolean* or *Boolean*);

#. *numeric conditional expressions* (both the second and the third operand
   expressions are of a numeric type); and

#. *reference conditional expressions* (all other combinations of
   operand expressions).


The following sections explain how the kind of a conditional expression
influences its type determination at runtime:

#. The first operand expression of a conditional expression is
   evaluated; the unboxing conversion is performed on the result if
   necessary.

#. The resultant *boolean* value is used to choose between the second
   and the third operand expressions as follows:

   -  If the value of the first operand is ``true``, then the second
      operand expression is chosen.

   -  If the value of the first operand is ``false``, then the third
      operand expression is chosen.

#. The chosen operand expression is evaluated, and the resultant value
   converts to the type of the conditional expression as determined by
   the rules below.

   Boxing or unboxing conversions are allowed for this conversion (see
   :ref:`Predefined Numeric Types Conversions`).

   The remaining operand expression is not evaluated in this particular
   evaluation of the conditional expression.

.. index::
   boolean conditional expression
   type boolean
   type Boolean
   numeric conditional expression
   reference conditional expression
   operand expression
   conditional expression
   runtime
   unboxing conversion
   conversion
   predefined numeric types conversion
   numeric types conversion
   numeric type

|

.. _Boolean Conditional Expressions:

Boolean Conditional Expressions
===============================

.. meta:
    frontend_status: Done

The type of a boolean conditional expression is:

-  *Boolean* if both the second and the third operands are of type *Boolean*.
-  *boolean* otherwise.

.. index::
   Boolean conditional expression
   conditional expression
   type Boolean
   type boolean

|

.. _Numeric Conditional Expressions:

Numeric Conditional Expressions
===============================

.. meta:
    frontend_status: Done

The type of a numeric conditional expression is:

-  Same as the type of the second and the third operands if those are
   of the same type.

-  *T* if either the second or the third operand is of the primitive
   type *T*, and the type of the other operand results from the boxing
   conversion (see :ref:`Predefined Numeric Types Conversions`) of *T*.

-  *short* if one operand’s type is *byte* or *Byte*, and the other’s
   *short* or *Short*.

-  *T* if one operand is of type *T* (where *T* is *byte*, *short*,
   or *char*), and the other operand is a constant expression (see
   :ref:`Constant Expressions`) of type *int* that has a value
   representable in type *T*.

-  *U* if one operand is of type *T* (where *T* is *Byte*, *Short*, or
   *Char*), and the other operand is a constant expression of type *int*
   that has a value representable in the type *U* that results from the
   unboxing conversion of *T*.

-  In other situations, the type of the conditional expression is the promoted
   type of the second and the third operands after the binary numeric promotion
   (see :ref:`Predefined Numeric Types Conversions`) of the operand types.

.. index::
   numeric conditional expression
   primitive type
   boxing conversion
   predefined numeric types conversion
   numeric types conversion
   numeric type
   conditional expression
   promoted type
   binary numeric promotion
   numeric promotion

|

.. _Reference Conditional Expressions:

Reference Conditional Expressions
=================================

.. meta:
    frontend_status: Done

The type of a reference conditional expression is:

-  Same as the type of the second and third operands if those are of the
   same type.

-  In other situations, the type of the conditional expression is the
   *least upper bound* type of *T*:sub:`1` and *T*:sub:`2` (where
   *T*:sub:`1` is the type resultant from the boxing conversion of the
   second operand’s type *S*:sub:`1`, and *T*:sub:`2` is the type resultant
   from the boxing conversion of the third operand’s type *S*:sub:`2`).

.. index::
   reference conditional expression
   operand
   conditional expression
   least upper bound
   boxing conversion

|

.. _String Interpolation Expressions:

String Interpolation Expressions
================================

.. meta:
    frontend_status: Done

A *string interpolation expression* is a template literal (a string literal
delimited with backticks, see :ref:`Template Literals` for details) that
contains at least one *embedded expression*.

.. code-block:: abnf

    stringInterpolation:
        '`' (BacktickCharacter | embeddedExpression)* '`'
        ;

    embeddedExpression:
        '${' expression '}'
        ;

An *embedded expression* is an expression specified inside the *dollar sign*
'$' and curly braces. The type of a string interpolation expression is *string*
(see :ref:`String Type`).

When evaluating a *string interpolation expression*,  the result of each
embedded expression substitutes that embedded expression. An embedded
expression must be of type *string*; otherwise, the implicit conversion
to *string* takes place in the same way as with the string concatenation
operator (see :ref:`String Concatenation`).

.. index::
   string interpolation expression
   template literal
   string literal
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

The example above can be rewritten as follows by using the string
concatenation operator:

.. code-block:: typescript
   :linenos:

    let a = 2
    let b = 2
    console.log("The result of " + a + " * " + b + " is " + a * b)

An embedded expression can contain nested template strings.

.. index::
   string concatenation operator
   nested template string
   template string
   embedded expression

|

.. _Lambda Expressions:

Lambda Expressions
******************

.. meta:
    frontend_status: Partly
    todo: capturing non-effectively final local variables
    todo: complete lambda and function type checks
    todo: there are still stability issues can cause compile time segmentation faults, incorrect bytecode generated

A *lambda expression* is a short block of code that takes in parameters, and
can return a value. Lambda expressions are generally similar to functions
but do not need names. Lambda expressions can be implemented immediately
within expressions.

.. code-block:: abnf

    lambdaExpression:
        signature '=>' lambdaBody
        ;

    lambdaBody:
        expression | block
        ;

.. code-block:: typescript
   :linenos:

    (x: number): number => { return Math.sin(x) }
    (x: number) => Math.sin(x) // expression as lambda body

A lambda expression evaluation:

-  Produces an instance of a function type (see :ref:`Function Types`).

-  Does *not* cause the execution of the expression body. However, the
   expression body can be executed later if a function call expression
   is used on the produced instance).


See :ref:`Throwing Functions` for the details of ``throws``, and
:ref:`Rethrowing Functions` for the details of ``rethrows`` marks.

.. index::
   lambda expression
   block of code
   parameter
   throwing function
   rethrowing function
   throws mark
   rethrows mark
   instance
   function type
   execution
   expression body
   instance

|

.. _Lambda Signature:

Lambda Signature
================

.. meta:
    frontend_status: Done

Formal parameters and return types (which compose lambda signature) of
lambda expressions and function declarations (see :ref:`Function Declarations`)
have the same syntax and semantics.

See :ref:`Scopes` for the specification of the scope, and
:ref:`Shadowing Parameters` for the shadowing  details of formal parameter
declarations.

A compile-time error occurs if a lambda expression declares two formal
parameters with the same name.

As a lambda expression is evaluated, the values of actual argument expressions
initialize the newly created parameter variables (each of the declared type)
before the execution of the lambda body.

.. index::
   lambda signature
   return type
   lambda expression
   function declaration
   scope
   shadow parameter
   shadowing
   parameter declaration
   compile-time error
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

A *lambda body* can be a single expression or a block (see :ref:`Block`).
Similar to the body of a method or a function, a lambda body describes the
code to be executed when a lambda call (see
:ref:`Runtime Evaluation of Lambda Expressions`) occurs.

The meanings of names, and of the keywords ``this`` and ``super`` (along with
the accessibility of the referred declarations) are the same as in the
surrounding context; however, lambda parameters introduce new names.

When a single expression, a *lambda body* is equivalent to the block
with one return statement which returns such single expression
``{ return singleExpression }``.

If completing normally, then a lambda body block is *value-compatible*.
A lambda body completing normally means that the statement of the form
``return expression`` is executed.

If any local variable, or formal parameter of the surrounding context is
used but is not declared in a lambda body, then such local variable, or formal
parameter is *captured* by the lambda.

If an instance member of surrounding type is used in the lambda body which
is defined in a method, then ``this`` is *captured* by the lambda.

A compile-time error occurs if a local variable is used in a lambda body but
is neither declared in it nor assigned before it.

.. index::
   lambda body
   keyword this
   keyword super
   runtime
   evaluation
   lambda expression
   method body
   function body
   lambda call
   surrounding context
   name
   access
   accessibility
   referred declaration
   expression
   value-compatible block
   normal completion
   captured by lambda
   surrounding context
   local variable
   lambda body block
   instance member
   surrounding type
   return expression
   execution
   compile-time error
   assignment

|

.. _Lambda Expression Type:

Lambda Expression Type
======================

.. meta:
    frontend_status: Done

A lambda expression type is a function type (see :ref:`Function Types`)
that has the following:

-  Lambda parameters (if any) as parameters of the function type; and

-  Lambda return type as return type of the function type.


Note: lambda return type may be ``void``, or inferred from the *lambda body*

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

The evaluation of a lambda expression is distinct from the execution of that
lambda body.

If completing normally at runtime, then the evaluation of a lambda expression
produces a reference to an object. In that case, it is similar to the evaluation
of a class instance creation expression.

The evaluation of a lambda expression:

-  Allocates and initializes a new instance of a class with the
   properties below; or

-  Refers to an existing instance of a class with the properties below.


The evaluation of the lambda expression completes abruptly, and an
*OutOfMemoryError* is thrown if a new instance is to be created
but the available space is not sufficient for it.

During a lambda expression evaluation, the captured values of that
lambda expression are saved to the internal state of the lambda.

.. index::
   runtime
   evaluation
   lambda expression
   execution
   normal completion
   instance creation expression
   initialization
   allocation
   instance
   class
   abrupt completion
   error
   captured value
   lambda internal state

+-----------------------------------------------+--------------+
| Source Fragment                               | Output       |
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
value of the variable captured by the lambda changes, then it is implied
that the original variable also changes:

.. index::
   captured by lambda
   variable
   captured variable
   original variable

+-----------------------------------------------+--------------+
| Source Fragment                               | Output       |
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

In order to cause the lambdas behave as required, the language implementation
can make the following replacements:

-  Primitive type for the corresponding boxed type (x: int to x: Int)
   if the captured variable is of a primitive type;

-  Captured variable’s type for a proxy class that contains an original
   reference (x: T for x: Proxy<T>; x.ref = original-ref) if such captured
   variable is of a non-primitive type.


If a captured variable is defined as 'const', then neither boxing nor
proxying is required.

If a formal parameter (which can be neither boxed nor proxied) is captured,
then the implementation can require the addition of a local variable as
follows:

.. index::
   lambda
   implementation
   primitive type
   boxed type
   captured variable
   captured variable type
   non-primitive type
   boxing
   proxying
   local variable

+-----------------------------------+-----------------------------------+
| Source Code                       | Pseudo Code                       |
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

.. _Dynamic Import Expression:

Dynamic Import Expression
*************************

TBD - grammar and link


|

.. _Constant Expressions:

Constant Expressions
********************

.. meta:
    frontend_status: Done

.. code-block:: abnf

    constantExpression:
        expression
        ;

A *constant expression* is an expression that denotes a value of a primitive
type, or a *string* that completes normally while being composed only of the
following:

-  Literals of a primitive type, and literals of type *string* (see
   :ref:`Literals`);

-  Casts to primitive types, and casts to the type *string* (see
   :ref:`Cast Expressions`);

-  Unary operators ':math:`+`', ':math:`-`', '~', and '!', but not ':math:`++`'
   or ':math:`--`' (see :ref:`Unary Plus`, :ref:`Unary Minus`,
   :ref:`Prefix Increment`, and :ref:`Prefix Decrement`);

-  Multiplicative operators ':math:`*`', ':math:`/`', and ':math:`\%`' (see
   :ref:`Multiplicative Expressions`);

-  Additive operators ':math:`+`' and ':math:`-`' (see :ref:`Additive Expressions`);

-  Shift operators '<<', '>>', and '>>>' (see :ref:`Shift Expressions`);

-  Relational operators '<', '<=', '>', and '>=' (see :ref:`Relational Expressions`);

-  Equality operators '``==``' and '``!=``' (see :ref:`Equality Expressions`);

-  Bitwise and logical operators '&', '^', and '\|' (see :ref:`Bitwise and Logical Expressions`);

-  Conditional-and operator '\&\&' (see :ref:`Conditional-And Expression`), and
   conditional-or operator '\||' (see :ref:`Conditional-Or Expression`);

-  Ternary conditional operator '? :' (see :ref:`Conditional Expressions`);

-  Parenthesized expressions (see :ref:`Parenthesized Expression`) that contain
   constant expressions;

-  Simple names that refer to constant variables;

-  Qualified names that have the form *typeReference'.'identifier*, and refer
   to constant variables.

.. index::
   constant expression
   primitive type
   type string
   normal completion
   literal
   cast expression
   unary operator
   increment operator
   decrement operator
   prefix


.. index::
   multiplicative operator
   multiplicative expression
   shift operator
   relational operator
   equality operator
   bitwise operator
   logical operator
   ternary conditional operator
   conditional-and operator
   conditional-or operator
   parenthesized expression
   constant expression
   simple name
   constant variable
   qualified name

.. raw:: pdf

   PageBreak


