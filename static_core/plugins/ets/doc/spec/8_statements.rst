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

.. _Statements:

Statements
##########

.. meta:
    frontend_status: Done

Statements are designed to control execution:

.. code-block:: abnf

    statement:
        expressionStatement
        | block
        | localDeclaration
        | ifStatement
        | loopStatement
        | breakStatement
        | continueStatement
        | returnStatement
        | switchStatement
        | throwStatement
        | tryStatement
        ;

|

.. _Normal and Abrupt Statement Execution:

Normal and Abrupt Statement Execution
*************************************

.. meta:
    frontend_status: Done

The actions that every statement performs in a normal mode of execution are
specific for the particular kind of statement. Normal modes of
evaluation for each kind of statement are described in the following
sections.

A statement execution is considered to *complete normally* if the desired
action is performed without an error being thrown. On the contrary, a statement
execution is considered to *complete abruptly* if it causes an error thrown.

.. index::
   statement
   execution
   statement execution
   normal completion
   abrupt completion

|

.. _Expression Statements:

Expression Statements
*********************

.. meta:
    frontend_status: Done

Any expression can be used as a statement:

.. code-block:: abnf

    expressionStatement:
        expression
        ;

The execution of a statement leads to the execution of the expression. The
result of such execution is discarded.

.. index::
   statement
   expression
   execution

|

.. _Block:

Block
*****

.. meta:
    frontend_status: Done

A sequence of statements (see :ref:`Statements`) enclosed in balanced braces
forms a *block*:

.. code-block:: abnf

    block:
        '{' statement* '}'
        ;

The execution of a block means that all block statements, except type
declarations, are executed one after another in the textual order of their
appearance within the block until an erroneous situation is thrown (see
:ref:`Errors`), or return (see :ref:`Return Statements`) occurs.

If a block is the body of a ``functionDeclaration`` (see
:ref:`Function Declarations`) or a ``classMethodDeclaration`` (see
:ref:`Method Declarations`) declared implicitly or explicitly with
return type ``void`` (see :ref:`Type void`), then the block can contain no
return statement at all. Such a block is equivalent to one that ends in a
``return`` statement, and is executed accordingly.

.. index::
   statement
   balanced brace
   block
   execution
   block statement
   type declaration
   return
   return type

|

.. _Local Declarations:

Local Declarations
******************

.. meta:
    frontend_status: Done

*Local declarations* define new mutable or immutable variables or types within
the enclosing context.

``Let`` and ``const`` declarations have the initialization part that presumes
execution, and actually act as statements:

.. code-block:: abnf

    localDeclaration:
        annotationUsage?
        ( variableDeclaration
        | constantDeclaration
        | typeDeclaration
        )
        ;

The visibility of a local declaration name is determined by the surrounding
function or method, and by the block scope rules (see :ref:`Scopes`).

The usage of annotations is discussed in :ref:`Using Annotations`.

.. index::
   local declaration
   let declaration
   const declaration
   mutable variable
   immutable variable
   initialization
   execution
   function
   method
   block scope

|

.. _if Statements:

``if``  Statements
******************

.. meta:
    frontend_status: Done
    todo: ambiguous wording in the spec: "Any 'else' corresponds to the first 'if' of an if statement" - what first means?

An ``if`` statement allows executing alternative statements (if provided) under
certain conditions:

.. code-block:: abnf

    ifStatement:
        'if' '(' expression ')' thenStatement
        ('else' elseStatement)?
        ;

    thenStatement:
        statement
        ;

    elseStatement:
        statement
        ;

Type of expression must be ``boolean``, or a type mentioned in
:ref:`Extended Conditional Expressions`. Otherwise, a
:index:`compile-time error` occurs.

If an expression is successfully evaluated as ``true``, then *thenStatement* is
executed. Otherwise, *elseStatement* is executed (if provided).

Any ``else`` corresponds to the first ``if`` of an ``if`` statement:

.. code-block:: typescript
   :linenos:

    if (Cond1)
    if (Cond2) statement1
    else statement2 // Executes only if: Cond1 && !Cond2

A list of statements in braces (see :ref:`Block`) is used to combine the
``else`` part with the first ``if``:

.. code-block:: typescript
   :linenos:

    if (Cond1) {
      if (Cond2) statement1
    }
    else statement2 // Executes if: !Cond1

.. index::
   if statement
   statement
   expression
   evaluation
   compile-time error

|

.. _Loop Statements:

Loop Statements
***************

.. meta:
    frontend_status: Done

|LANG| has four kinds of loops. A loop of each kind can have an optional loop
label that can be used only by ``break`` and ``continue`` statements contained
in the body of the loop. The label is characterized by an *identifier* as shown
below:

.. index::
   loop statement
   loop
   loop label
   break statement
   continue statement
   identifier

.. code-block:: abnf

    loopStatement:
        (identifier ':')?
        whileStatement
        | doStatement
        | forStatement
        | forOfStatement
        ;

An ``identifier`` at the beginning of a loop statement denotes a *label*
that can be used in :ref:`Break Statements` and :ref:`Continue Statements`.

A :index:`compile-time error` occurs if the label *identifier* is not used
within ``loopStatement``.

|

.. _While Statements and Do Statements:

``while`` Statements and ``do`` Statements
******************************************

.. meta:
    frontend_status: Done

A ``while`` statement and a ``do`` statement evaluate an expression and
execute the statement repeatedly till the expression value is ``true``.
The key difference is that *whileStatement* starts from evaluating and
checking the expression value, and *doStatement* starts from executing
the statement:

.. code-block:: abnf

    whileStatement:
        'while' '(' expression ')' statement
        ;

    doStatement
        : 'do' statement 'while' '(' expression ')'
        ;

Type of expression must be ``boolean``, or a type mentioned in
:ref:`Extended Conditional Expressions`.
Otherwise, a :index:`compile-time error` occurs.

.. index::
   while statement
   do statement
   expression
   expression value
   execution
   statement

|

.. _For Statements:

``for`` Statements
******************

.. meta:
    frontend_status: Done

.. code-block:: abnf

    forStatement:
        'for' '(' forInit? ';' forContinue? ';' forUpdate? ')' statement
        ;

    forInit:
        expressionSequence
        | variableDeclarations
        ;

    forContinue:
        expression
        ;

    forUpdate:
        expressionSequence
        ;

Type of *forContinue* expression must be ``boolean``, or a type
mentioned in :ref:`Extended Conditional Expressions`. Otherwise, a
:index:`compile-time error` occurs.

.. code-block:: typescript
   :linenos:

    // existing variable is used as a loop index variable
    let i: number
    for (i = 1; i < 10; i++) {
      console.log(i)
    }

    // new variable is declared as a loop index variable with its type
    // explicitly specified
    for (let i: number = 1; i < 10; i++) {
      console.log(i)
    }

    // new variable is declared as loop index variable with its type
    // inferred from its initialization part of the declaration
    for (let i = 1; i < 10; i++) {
      console.log(i)
    }

.. index::
   for statement

|

.. _For-Of Statements:

``for-of`` Statements
*********************

.. meta:
    frontend_status: Done

A ``for-of`` loop iterates elements of ``array`` or ``string``, or an instance
of *iterable* class or interface (see :ref:`Iterable Types`):

.. code-block:: abnf

    forOfStatement:
        'for' '(' forVariable 'of' expression ')' statement
        ;

    forVariable:
        identifier | ('let' | 'const') identifier (':' type)?
        ;

A :index:`compile-time error` occurs if the type of an expression is not
``array``, ``string``, or an iterable type.

The execution of a ``for-of`` loop starts from the evaluation of ``expression``.
If the evaluation is successful, then the resultant expression is used for
loop iterations (execution of the ``statement``). On each iteration,
*forVariable* is set to successive elements of the ``array``, ``string``, or
the result of class iterator advancing.

.. index::
   for-of statement
   loop
   array
   string
   compile-time error
   expression
   type
   array
   string
   for-of loop
   evaluation
   loop iterations
   statement

If *forVariable* has the modifiers ``let`` or ``const``, then a new variable
is used inside the loop. Otherwise, the variable is as declared above.
The modifier ``const`` prohibits assignments into *forVariable*,
while ``let`` allows modifications.

Explicit type annotation of *forVariable* is allowed as an experimental
feature (see :ref:`For-of Type Annotation`).

.. index::
   modifier
   let modifier
   const modifier
   variable
   assignment
   modification
   for-of type annotation
   annotation

.. code-block-meta:
    not-subset

.. code-block:: typescript
   :linenos:

    // existing variable 'ch'
    let ch : char
    for (ch of "a string object") {
      console.log(ch)
    }

    // new variable 'ch', its type is inferred from expression after 'of'
    for (let ch of "a string object") {
      console.log(ch)
    }

    // new variable 'element', its type is inferred from expression after 'of',
    // and it cannot be assigned with a new value in the loop body
    for (const element of [1, 2, 3]) {
      console.log(element)
      element = 66 // Compile-time error as 'element' is 'const'
    }

|

.. _Break Statements:

``break``  Statements
*********************

.. meta:
    frontend_status: Done
    todo: break with label causes compile time assertion

A ``break`` statement transfers control out of the enclosing *loopStatement*
or *switchStatement*:

.. index::
   break statement
   control transfer
   switch statement
   loop statement

.. code-block:: abnf

    breakStatement:
        'break' identifier?
        ;

A ``break`` statement with the label *identifier* transfers control out of the
enclosing statement with the same label *identifier*. If there is no enclosing
loop statement with the same label identifier (within the body of the
surrounding function or method), then a :index:`compile-time error` occurs.

A statement without a label transfers control out of the innermost enclosing
``switch``, ``while``, ``do``, ``for``, or ``for-of`` statement. If
``breakStatement`` is placed outside *loopStatement* or *switchStatement*, then
a :index:`compile-time error` occurs.

.. index::
   break statement
   label
   identifier
   control transfer
   statement
   function
   method
   label
   switch statement
   while statement
   do statement
   for statement
   for-of statement
   compile-time error
   loop statement

|

.. _Continue Statements:

``continue`` Statements
***********************

.. meta:
    frontend_status: Done
    todo: continue with label causes compile time assertion

A ``continue`` statement stops the execution of the current loop iteration,
and transfers control to the next iteration. Appropriate checks of loop
exit conditions depend on the kind of the loop.

.. code-block:: abnf

    continueStatement:
        'continue' identifier?
        ;

A ``continue`` statement with the label *identifier* transfers control
to the next iteration of the enclosing loop statement
with the same label *identifier*.
If there is no enclosing loop statement with the same label *identifier*
(within the body of the surrounding function or method),
then a :index:`compile-time error` occurs.

.. index::
   continue statement
   execution
   label
   exit condition
   loop statement
   surrounding function
   control transfer
   identifier
   continue statement
   function

|

.. _Return Statements:

``return`` Statements
*********************

.. meta:
    frontend_status: Done
    todo: return voidExpression

A ``return`` statement can have or not have an expression.

.. code-block:: abnf

    returnStatement:
        'return' expression?
        ;

A *return expression* statement can only occur inside a function or a method body.

.. index::
   return statement
   expression
   return expression
   function
   method
   method body
   constructor

A ``return`` statement (with no expression) can occur in one of the following
situations:

- Inside a class initializer;
- Inside a constructor body; or
- Inside a function or a method body with return type ``void`` (see
  :ref:`Type void`).

A :index:`compile-time error` occurs if a ``return`` statement is found in:

-  Top-level statements (see :ref:`Top-Level Statements`);
-  Class initializers (see :ref:`Class Initializer`) and constructors (see
   :ref:`Constructor Declaration`), where it has an expression;
-  Functions or methods with return type ``void`` (see :ref:`Type void`)
   that have an expression;
-  Functions or methods with a non-``void`` return type that have no
   expression.

.. index::
   compile-time error
   return statement
   expression
   statement
   top-level statement
   function
   method
   return type
   class
   initializer
   constructor
   constructor declaration

The execution of *returnStatement* leads to the termination of the
surrounding function or method. If an *expression* is provided,
the resultant value is the evaluated *expression*.

In case of constructors, class initializers, and top-level statements, the
control is transferred out of the scope of the construction in question, but
no result is required. Other statements of the surrounding function, method
body, class initializer, or top-level statement are not executed.

.. index::
   execution
   termination
   surrounding function
   surrounding method
   constructor
   class initializer
   top-level statement
   control transfer
   expression
   evaluation
   method body
   class initializer
   top-level statement
   return statement

|

.. _Switch Statements:

``switch`` Statements
*********************

.. meta:
    frontend_status: Done
    todo: non literal constant expression () in case ==> causes an assertion error
    todo: when there is only a default clause in switchBlock then the default's statements/block are not executed
    todo: spec issue: optional identifier before the switch - it should be clarified it can be a label for break stmt

A ``switch`` statement transfers control to a statement or a block by using the
result of successful evaluation of the value of a ``switch`` expression.

.. index::
   switch statement
   control transfer
   statement
   block
   evaluation
   switch expression

.. code-block:: abnf

    switchStatement:
        (identifier ':')? 'switch' '(' expression ')' switchBlock
        ;

    switchBlock
        : '{' caseClause* defaultClause? caseClause* '}'
        ;

    caseClause
        : 'case' expression ':' statement*
        ;

    defaultClause
        : 'default' ':' statement*
        ;

The ``switch`` expression type must be of type ``char``, ``byte``, ``short``,
``int``, ``long``, ``string``,
or ``enum``.

.. index::
   expression type
   constant expression
   enum constant
   char
   byte
   short
   int
   long

A :index:`compile-time error` occurs if not **all** of the following is true:

-  Every case expression type is
   assignable (see :ref:`Assignability`) to the type of the ``switch``
   statement expression.

-  In a ``switch`` statement expression of type ``enum``, every case expression
   associated with the ``switch`` statement is of type ``enum``.

-  No two case constant expressions (see :ref:`Constant Expressions`)
   have identical values.

-  No case expression is ``null``.

.. index::
   expression
   switch statement
   compatibility
   constant
   null expression

.. code-block:: typescript
   :linenos:

    let arg = prompt("Enter a value?");
    switch (arg) {
      case '0':
      case '1':
        alert('One or zero')
        break
      case '2':
        alert('Two')
        break
      default:
        alert('An unknown value')
    }

The execution of a ``switch`` statement starts from the evaluation of the
``switch`` expression.

The value of the ``switch`` expression is compared repeatedly to the
value of case expressions starting from the top till the first *match*. The
*match* means that particular case expression value equals the value of the
``switch`` expression in terms of the operator '``==``'.  However, if the
expression value is of type  ``string``, then the equality for strings
determines the equality.

So, in case of *match* execution is transferred to the set of statements of
the *caseClause* where match occurred. If this set of statements executes
*break* statement then the whole ``switch`` statement terminates. If no *break*
statement was executed then execution continues through all remaining
*caseClause*s as well as *defaultClause* at last if it is present.
If no *match* occurred and *defaultClause* is present then it is executed.


.. index::
   execution
   switch statement
   expression
   evaluation
   constant
   operator
   string

|

.. _Throw Statements:

``throw`` Statements
********************

.. meta:
    frontend_status: Done

A ``throw`` statement causes an *error* object to be created and raised
(see :ref:`Error Handling`). It immediately transfers control, and can exit
multiple statements, constructors, functions, and method calls until a ``try``
statement (see :ref:`Try Statements`) is found that catches the value thrown.
If no ``try`` statement is found, then ``UncaughtExceptionError`` is thrown.

.. code-block:: abnf

    throwStatement:
        'throw' expression
        ;

The expression type must be assignable (see :ref:`Assignment`) to type
``Error``. Otherwise, a :index:`compile-time error` occurs.

This implies that the object thrown is never ``null``.

Errors can be thrown at any place in the code.

.. index::
   throw statement
   thrown value
   thrown object
   control transfer
   statement
   method
   function
   constructor
   try block
   try statement
   assignment
   compile-time error

|

.. _Try Statements:

``try`` Statements
******************

.. meta:
    frontend_status: Done

A ``try`` statement runs blocks of code, and provides sets of catch clauses
to handle different errors (see :ref:`Error Handling`).

.. index::
   try statement
   block
   catch clause

.. code-block:: abnf

    tryStatement:
          'try' block catchClause? finallyClause?
          ;

    catchClause:
          'catch' '(' identifier ')' block
          ;

    finallyClause:
          'finally' block
          ;

A ``try`` statement must contain either a ``finally`` clause, or a
``catch`` clause. Otherwise, a :index:`compile-time error` occurs.

If the ``try`` block completes normally, then no action is taken, and no
``catch`` clause block is executed.

If an error is thrown in the ``try`` block directly or indirectly, then the
control is transferred to the ``catch`` clause.

.. index::
   catch clause
   typed catch clause
   try statement
   try block
   normal completion
   compile-time error
   control transfer
   finally clause
   block

|

.. _Catch Statements:

``catch`` Clause
================

.. meta:
    frontend_status: Done

A ``catch`` clause consists of two parts:

-  A *catch identifier* that provides access to an object associated with
   the *error* thrown; and

-  A block of code that handles the error.

The type of *catch identifier* inside the block is ``Error`` (see
:ref:`Error Handling`).

.. index::
   catch clause
   catch identifier
   access
   block
   catch identifier
   Object


.. index::
   typed catch clause

.. code-block:: typescript
   :linenos:

    class ZeroDivisor extends Error {}

    function divide(a: number, b: number): number {
      if (b == 0)
        throw new ZeroDivisor()
      return a / b
    }

    function process(a: number, b: number): number {
      try {
        let res = divide(a, b)

        // further processing ...
      }
      catch (e) {
        return e instanceof ZeroDivisor? -1 : 0
      }
    }

A ``catch`` clause handles all errors at runtime. It returns '*-1*' for
the ``ZeroDivisor``, and '*0*'  for all other errors.

.. index::
   catch clause
   runtime

|

.. _Finally Clause:

``finally`` Clause
==================

.. meta:
    frontend_status: Done

A ``finally`` clause defines the set of actions in the form of a block to be
executed without regard to whether a ``try-catch`` completes normally or
abruptly.

.. code-block:: abnf

    finallyClause:
        'finally' block
        ;

A ``finally`` block is executed without regard to how (by reaching ``return``
or ``try-catch`` end or raising new *error*) the program control is
transferred out. The ``finally`` block is particularly useful to ensure
proper resource management.

Any required actions (e.g., flush buffers and close file descriptors)
can be performed while leaving the ``try-catch``:

.. index::
   finally clause
   block
   execution
   try-catch
   normal completion
   abrupt completion
   finally block
   execution
   return
   try-catch

.. code-block:: typescript

    class SomeResource {
      // some API
      // ...
      close() {}
    }

    function ProcessFile(name: string) {
      let r = new SomeResource()
      try {
        // some processing
      }
      finally {
        // finally clause will be executed after try-catch is
            executed normally or abruptly
        r.close()
      }
    }

|

.. _Try Statement Execution:

``try`` Statement Execution
===========================

.. meta:
    frontend_status: Done

#. A ``try`` block and the entire ``try`` statement complete normally if no
   ``catch`` block is executed. The execution of a ``try`` block completes
   abruptly if an error is thrown inside the ``try`` block. 

#. The execution of a ``try`` block completes abruptly if error *x* is thrown
   inside the ``try`` block. If the ``catch`` clause is present and the
   execution of the body of the ``catch`` clause completes normally, then the
   entire ``try`` statement completes normally. Otherwise, the ``try``
   statement completes abruptly.

#. If no ``catch`` clause in place then the error is propagated to the
   surrounding scope until it reaches the scope with the ``catch`` clause which
   handles the error. If no such scope exits then ``UncaughtExceptionError`` is
   thrown and program execution terminates (see :ref:`Program Exit`).

#. If ``finally`` clause in place and its execution completes abruptly then the
   ``try`` statement completes abruptly as well.

.. index::
   try statement
   execution
   try block
   normal completion
   abrupt completion
   error
   catch clause
   runtime
   catch clause
   compatibility
   propagation
   surrounding scope
   function
   method
   constructor
   caller context

.. raw:: pdf

   PageBreak
