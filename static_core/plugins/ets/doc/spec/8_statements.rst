.. _Statements:

Statements
##########

Statements are designed to control the execution.

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
        | assertStatement
        ;

|

.. _Normal and Abrupt Statement Execution:

Normal and Abrupt Statement Execution
*************************************

.. meta:
    frontend_status: Done

The actions that every statement performs in a normal mode of execution are
specific for the particular kind of that statement. The normal modes of
evaluation for each kind of statement are described in the following
sections.

A statement execution is considered to *complete normally* if the desired
action is performed without throwing an exception or error.

On the contrary, a statement is said to *complete abruptly* if its
execution caused an exception or error to be thrown.

.. index::
   statement execution
   statement
   execution
   normal mode of execution
   statement execution
   normal completion
   abrupt completion
   exception
   error

   
|

.. _Expression Statements:

Expression Statements
*********************

.. meta:
    frontend_status: Done

Any expression can be used as statement.

.. code-block:: abnf

    expressionStatement:
        expression
        ;

The execution of such statements leads to the execution of the expression,
and the result of such execution is discarded.

.. index::
   expression statement
   expression
   execution

|

.. _Block:

Block
*****

.. meta:
    frontend_status: Done

A sequence of statements enclosed in balanced braces forms a *block*.

.. code-block:: abnf

    block:
        '{' statement* '}'
        ;

The execution of a block means that all block statements except type
declarations are executed one by one sequentially in the textual order
of their appearance within the block, unless exception, error or return
occurs.

If a block is the body of a *functionDeclaration* or a *classMethodDeclaration*
which was declared with no return type or with the return type *void*, then the
block may contain no return statement at all. Such a block is equivalent to a
block ending in the statement *return void*, and is executed accordingly.

.. index::
   sequence of statements
   block
   execution
   block statement
   type declaration
   exception
   error
   return
   return type

|

.. _Local Declarations:

Local Declarations
******************

.. meta:
    frontend_status: Partly
    todo: type declaration (class, interface, enum)

Local declarations define new mutable or immutable variables or
types within the enclosing context.

*Let* and *const* declarations have the initialization part that
presumes execution, and actually act as statements.

.. code-block:: abnf

    localDeclaration:
        variableDeclaration
        | constantDeclaration
        | typeDeclaration
        ;

The visibility of a local declaration name is determined by the function
(method) and block scopes rules (see :ref:`Scopes`).

.. index::
   local declaration
   immutable variable
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
    todo: ambiguous wording in the spec: "Any 'else' matches the first 'if' of an if statement" - what first means?

``If`` statements allow executing alternative statements (if provided) under
certain conditions.

.. code-block:: abnf

    ifStatement:
        'if' '(' expression ')' statement1
        ('else' statement2)?
        ;

An expression represents the condition, and if it is successfully evaluated
as *true*, then statement1 is executed; otherwise, statement2 is executed (if
provided). A compile-time error occurs unless the type of the expression is
*boolean*.

Any ``else`` matches the first ``if`` of an ``if`` statement.

.. index::
   if statement
   execution
   statement
   expression
   evaluation
   compile-time error

.. code-block:: typescript
   :linenos:
 
    if (Cond1)
    if (Cond2) statement1
    else statement2 // Executes only if: Cond1 && !Cond2

A list of statements in braces (see :ref:`Block`) is used to combine the ``else``
part with the first ``if``:

.. code-block:: typescript
   :linenos:

    if (Cond1) {
      if (Cond2) statement1
    }
    else statement2 // Executes if: !Cond1

|

.. _Loop Statements:

``Loop`` Statements
*******************

.. meta:
    frontend_status: Done

|LANG| has four kinds of loops. Each kind of loops can have an optional label
which can be used only by ``break`` and ``continue`` statements contained in
the body of the loop. A label is characterized by the *identifier* as shown below:

.. index::
   loop statement
   loop
   optional label
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

|

.. _While Statements and Do Statements:

``While`` Statements and ``Do`` Statements
******************************************

.. meta:
    frontend_status: Done

``While`` statements and ``do`` statements evaluate an *expression* and
execute a *statement* repeatedly till the *expression* value is *true*.
The key difference is that a *whileStatement* first evaluates and checks the
*expression* value, and a *doStatement* starts by executing the *statement*:

.. index::
   while statement
   do statement
   expression
   expression value
   execution
   statement

.. code-block:: abnf

    whileStatement:
        'while' '(' expression ')' statement
        ;

    doStatement
        : 'do' statement 'while' '(' expression ')'
        ;

|

.. _For Statements:

``For`` Statements
******************

.. meta:
    frontend_status: Done

.. index::
   for statement

.. code-block:: abnf

    forStatement:
        'for' '(' forInit? ';' expression? ';' forUpdate? ')' statement
        ;

    forInit:
        expressionSequence
        | variableDeclarations
        ;

    forUpdate:
        expressionSequence
        ;

.. code-block:: typescript
   :linenos:

    // existing variable
    let i: number
    for (i = 1; i < 10; i++) {
      console.log(i)
    }

    // new variable, explicit type:
    for (let i: number = 1; i < 10; i++) {
      console.log(i)
    }

    // new variable, implicit type
    // inferred from variable declaration
    for (let i = 1; i < 10; i++) {
      console.log(i)
    }

|

.. _For-Of Statements:

``For-Of`` Statements
*********************

.. meta:
    frontend_status: Done

``For-Of`` loops iterate elements of *array* or *string*:

.. index::
   for-of statement
   loop
   array
   string

.. code-block:: abnf

    forOfStatement:
        'for' '(' forVariable 'of' expression ')' statement
        ;

    forVariable:
        identifier | ('let' | 'const') identifier (':' type)?
        ;


A compile-time error occurs if the type of an expression is not an
*array* or *string*.

The execution of the ``For-Of`` loop starts with the evaluation of ``expression``,
and,  if successful, then the resultant *string* or *array* is used for 
loop iterations (execution of the ``statement``). For every such iteration
the *forVariable* is set to successive elements of the *array* or *string*.

.. index::
   compile-time error
   expression
   type
   array
   string
   for-of loop
   evaluation
   loop iterations
   statement
   array
   string

If *forVariable* has the modifiers ``let`` or ``const``, then a new variable
is used inside the loop; otherwise, the variable as declared above.
The modifier ``const`` prohibits assignments into *forVariable*,
while ``let`` allows modifications.

As an experimental feature, an explicit type annotation for a *for variable*
(see :ref:For-of Type Annotation) is allowed.

.. index::
   modifier
   let modifier
   const modifier
   assignment
   for-of type annotation
   type annotation

.. code-block:: typescript
   :linenos:

    // existing variable 'ch'
    let ch : char
    for (ch of "a string object") {
      console.log(ch)
    }

    // new variable 'ch', its type is
    // inferred from expression
    for (let ch of "a string object") {
      console.log(ch)
    }

    // new variable 'element', its type is
    // inferred from expression, and it 
    // cannot be assigned with a new value
    // in the loop body
    for (const element of [1, 2, 3]) {
      console.log(element)
    }


|

.. _Break Statements:

``Break``  Statements
*********************

.. meta:
    frontend_status: Done
    todo: break with label causes compile time assertion

``Break`` statements transfer control out of enclosing *loopStatement* or
*switchStatement*.

.. index::
   break statement
   control transfer

.. code-block:: abnf

    breakStatement:
        'break' identifier?
        ;

A ``break`` statement with a label *identifier* transfers control out of the
enclosing statement with the same label *identifier*. Such statement must
be found within the body of the surrounding function or method; a compile-time
error occurs otherwise.

A statement without a label transfers control out of the innermost enclosing
``switch``, ``while``, ``do``, ``for`` or ``for-of`` statement.

A compile-time error occurs if a *breakStatement* is not found within
the *loopStatment* or *switchStatement*.

.. index::
   break statement
   identifier
   control transfer
   enclosing statement
   surrounding function
   surrounding method
   innermost enclosing statement
   switch statement
   while statement
   do statement
   for statement
   for-of statement
   compile-time error
   loop

|

.. _Continue Statements:

``Continue`` Statements
***********************

.. meta:
    frontend_status: Done
    todo: continue with label causes compile time assertion

``Continue`` statements stop the execution of the current loop iteration
and transfer control to the next iteration with proper checks of the loop exit
conditions that depend on the kind of the loop.

.. code-block:: abnf

    continueStatement:
        'continue' identifier?
        ;

A ``continue`` statement with the label *identifier* transfers control out
of the enclosing loop statement with the same label *identifier*. Such
a statement must be found within the body of the surrounding function or
method; a compile-time error occurs otherwise.

A compile-time error occurs if a *continueStatement* is not found within
the *loopStatment*.

.. index::
   continue statement
   execution
   loop statement
   surrounding function
   control transfer
   identifier
   identifier

|

.. _Return Statements:

``Return`` Statements
*********************

.. meta:
    frontend_status: Done
    todo: return voidExpression

``Return`` statements can have an expression or none.

.. code-block:: abnf

    returnStatement:
        'return' expression?
        ;

A 'return expression' statement can only occur inside a function, method
or constructor body.

.. index::
   return statement
   expression
   return expression
   function
   method
   constructor

A ``return`` statement (without expression) can occur in any place where
statements are allowed, except top-level statements  (see
:ref:`Top-Level Statements`). This form is also valid for functions or methods
with the return type ``void`` because it is semantically equivalent to the
statement ``return void``.

A compile-time error occurs if:

-  a ``return`` statement occurs in 
   top-level statements (see :ref:`Top-Level Statements`).
-  a ``return`` statement in class initializers (see :ref:`Class Initializer`)
   and constructors (see :ref:`Constructor Declaration`) has an expression.
-  a ``return`` statement in a function or a method with a non-``void`` return
   type has no expression.

.. index::
   compile-time error
   return statement
   expression
   statement
   top-level statement
   function
   method
   return type
   class initializer
   constructor declaration

The execution of a *returnStatement* leads to the termination of the
surrounding function or method. The resultant value is the evaluated
*expression* (if provided), or otherwise ``void``.

In case of constructors, class initializers and top-level statements, the
control leaves the scope of the construction mentioned but no result is
required. Other statements of the surrounding function or method body,
class initializer or top-level statement are not executed.

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

|

.. _Switch Statements:

``Switch`` Statements
*********************

.. meta:
    frontend_status: Done
    todo: non literal constant expression () in case ==> causes an assertion error
    todo: when there is only a default clause in switchBlock then the default's statements/block are not executed
    todo: spec issue: optional identifier before the switch - it should be clarified it can be a label for break stmt

``Switch`` statements transfer control to statements or block by using the
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
        : 'case' expression ':' (statement+ | block)?
        ;

    defaultClause
        : 'default' ':' (statement+ | block)?
        ;

The switch *expression* type must be of type *char, byte, short, int, long,
Char, Byte, Short, Int, Long, string* or *enum*.


.. index::
   expression type
   constant expression
   enum constant
   char
   byte
   short
   int
   long
   Char
   Byte
   Short
   Int
   Long

A compile-time error occurs unless all of the following is true:

-  Every case expression type associated with a ``switch`` statement is
   compatible (see :ref:`Compatible Types`) with the
   type of the ``switch`` statement’s expression.

-  For a ``switch`` statement expression of type *enum*, every case
   expression associated with the ``switch`` statement is of type *enum*.

-  No two case expressions associated with the ``switch`` statement have
   identical values.

-  None of the case expressions associated with the ``switch`` statement is *null*.

.. index::
   expression
   switch statement
   type compatibility
   constant
   null statement

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

The execution of a ``switch`` statement starts from the switch *expression*
evaluation. If the evaluation result is of type *Char*, *Byte*, *Short* or
*Int*, then the unboxing conversion follows.

Otherwise, the value of the switch expression is compared repeatedly to the
value of each case expression.

If a case expression value equals the switch expression value in terms of the
':math:`==`' operator, then the case label *matches*.

However, if the expression value is a *string*, then the equality for strings
determines the equality.

.. index::
   execution
   switch statement
   expression
   evaluation
   Char
   Byte
   Short
   Int
   unboxing conversion
   Expression
   constant
   operator
   string

|

.. _Throw Statements:

``Throw`` Statements
********************

.. meta:
    frontend_status: Done

``Throw`` statements cause an exception or error to be thrown (see
:ref:`Errors Handling`). They immediately transfer control and can exit multiple
statements, constructors, functions and method calls until a ``try`` statement
(see :ref:`Try Statements`) is found that catches the thrown value; if no such
try statement is found, then the *UncatchedExceptionError* is thrown.

.. code-block:: abnf

    throwStatement:
        'throw' expression
        ;

The *expression*’s type must be assignable (see :ref:`Assignment`) to the
type *Exception* or *Error*. A compile-time error occurs otherwise.

This implies that thrown object is never *null*.

It is necessary to check at compile time that a *throw* statement which
throws the exception is placed in a ``try`` block of a ``try`` statement
or in a *throwing function* (see :ref:`Throwing Functions`). Errors can
be thrown at any place in the code.

.. index::
   throw statement
   thrown value
   thrown object
   exception
   error
   control transfer
   statement
   method
   function
   constructor
   try block
   try statement
   throwing function
   assignment
   compile-time error

|

.. _Try Statements:

``Try`` Statements
******************

.. meta:
    frontend_status: Done

``Try`` statements run blocks of code and provide sets of catch clauses
to handle different exceptions and errors (see :ref:`Errors Handling`).

.. index::
   try statement
   block
   catch clause
   exception
   error

.. code-block:: abnf

    tryStatement:
          'try' block catchClauses finallyClause?
          ;

    catchClauses:
          typedCatchClause* catchClause?
          ;

    catchClause:
          'catch' '(' identifier ')' block
          ;

    typedCatchClause:
          'catch' '(' identifier ':' typeReference ')' block
          ;

    finallyClause:
          'finally' block
          ;

The |LANG| programming language supports *multiple typed catch clauses* as
an experimental feature (see :ref:`Try Statements`).

A ``try`` statement must contain either a ``finally`` clause or at least one
``catch`` clause; a compile-time error occurs otherwise.

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
   exception
   error
   block

|

.. _Catch Statements:

``Catch`` Clause
================

.. meta:
    frontend_status: Done

A ``catch`` clause consists of two parts:

-  a *catch identifier* that provides access to the object associated with
   the error occurred, and

-  a block of code that is to handle the situation.

The type of a *catch identifier* is *Object*.

.. index::
   catch clause
   catch identifier
   access
   error
   block
   catch identifier
   Object

See :ref:`Multiple Clauses in Statements` for the details of *typed catch
clause*.

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
   error

|

.. _Finally Clause:

``Finally`` Clause
==================

.. meta:
    frontend_status: Done

A ``finally`` clause defines the set of actions in the form of a block to be
executed without regard to how (normally or abruptly) a ``try-catch`` completes.

.. code-block:: abnf

    finallyClause:
        'finally' block
        ;

The ``finally`` block is executed without regard to how (by reaching
*exception*, *error*, *return* or *try-catch* end) the program control
is transferred out. A ``finally`` block is particularly useful to ensure
proper resource management: any required actions (e.g., flush buffers and
close file descriptors) can be performed while leaving the try-catch.

.. index::
   finally clause
   block
   execution
   try-catch
   normal completion
   abrupt completion
   finally block
   execution
   exception
   error
   return
   try-catch
   exception
   flush buffer
   file descriptor

.. code-block:: typescript

    class SomeResource {
      // some API
      // ...
      close() : void {}
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

``Try`` Statement Execution
===========================

.. meta:
    frontend_status: Done

#. The ``try`` block and the entire ``try`` statement completes normally if
   no ``catch`` block is executed.

   The execution of a ``try`` block completes abruptly if an *exception* or
   *error* is thrown inside the ``try`` block.

   ``Catch`` clauses are checked in the textual order of their position in the
   source code.

.. index::
   try statement
   execution
   try block
   normal completion
   abrupt completion
   error
   catch clause
   exception

#. The execution of a ``try`` block completes abruptly if an *exception* or
   *error* *x* is thrown inside the ``try`` block.
   If the runtime type of *x* is compatible (see :ref:`Compatible Types`) with
   an *exception* class of the *exception* parameter (i.e., the ``catch``
   clause matches *x*), and the execution of the body of the ``catch`` clause
   completes normally, then the entire ``try`` statement completes normally.
   Otherwise, the ``try`` statement completes abruptly.

#. If no ``catch`` clause can handle the *exception* or *error*, then those
   propagate to the surrounding scope. If the surrounding scope is a function,
   method or constructor, then the execution depends on whether the surrounding
   scope is a *throwing function* (see :ref:`Throwing Functions`). If so, then
   the exception propagates to the caller context. Otherwise, the
   *UncatchedExceptionError* is thrown.

.. index::
   execution
   try block
   abrupt completion
   normal completion
   try block
   exception
   runtime
   compatible type
   catch clause
   exception
   exception parameter
   try statement
   error
   type compatibility
   compatible type
   propagation
   surrounding scope
   function
   method
   constructor
   throwing function
   caller context

|

.. _Assert Statements:

``Assert`` Statements
*********************

.. meta:
    frontend_status: Done

The ``assert`` statements are described in the experimental section (see
:ref:`Assert Statements Experimental`).

.. index::
   assert statement

.. raw:: pdf

   PageBreak


