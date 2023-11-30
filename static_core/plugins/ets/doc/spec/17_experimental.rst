..
    Copyright (c) 2021-2023 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _Experimental Features:

Experimental Features
#####################

.. meta:
    frontend_status: Partly

This Chapter introduces the |LANG| features that are considered a part of
the language, but have no counterpart in |TS|, and are therefore not
recommended to those who need a single source code for Typescript
and |LANG|.

Some features introduced in this Chapter are still under discussion. They can
be removed from the final version of the |LANG| specification. Once a feature
introduced in this Chapter is approved and/or implemented, the corresponding
section will be moved appropriately to the body of the specification.

The *array creation* feature introduced in :ref:`Array Creation Expressions`
enables users to dynamically create objects of array types by using runtime
expressions that provide the array size. This is a useful addition to other
array-related features of the language, such as array literals.

The construct can also be used to create multi-dimensional arrays.

The feature *function and method overloading* is supported in many
(if not all) modern programming languages. Overloading functions/methods
is a practical and convenient way to write program actions that are similar
in logic but different in implementation.

.. index::
   implementation
   array creation
   runtime expression
   array
   array literal
   construct
   function overloading
   method overloading

The |LANG| language supports (as an experimental feature at the moment) two
semantically and syntactically different implementations of overloading: the
|TS|-like implementation, and that of other languages. See
:ref:`Function and Method Overloading` for more details.

Section :ref:`Native Functions and Methods` introduces practically important
and useful mechanisms for the inclusion of components written in other languages
into a program written in |LANG|.

Section :ref:`Final Classes and Methods` discusses the well-known feature that
in many OOP languages provides a way to restrict class inheritance and method
overriding.

Making a class *final* prohibits defining classes derived from it, whereas
making a method *final* prevents it from overriding in derived classes.

Section :ref:`Extension Functions` defines the ability to extend a class or an
interface with new functionality without having to inherit from the class. This
feature can be used for GUI programming (:ref:`Support for GUI Programming`).

Section :ref:`Enumeration Methods` adds methods to declarations of the
enumeration types. Such methods can help in some kinds of manipulations
with *enums*.

.. index::
   implementation
   function overloading
   method overloading
   class final
   method final
   OOP (object-oriented programming)
   inheritance

Section :ref:`Exceptions` discusses the powerful, commonly used mechanism for
the processing of various kinds of unexpected events and situations that break
the ‘ordinary’ program logic. There are constructs to raise (‘throw’) exceptions,
‘catch’ them along the dynamic sequence of function calls, and handle them.
Also, some support for exceptions is provided by the classes from the standard
library (see :ref:`Standard Library`).

**Note**: The exceptions mechanism is sometimes deprecated for being too
time-consuming and unsafe. Some modern languages do not support the
exceptions mechanism as discussed in this section. That is why the expediency
of adding this feature to the language is still under discussion.

The |LANG| language supports writing concurrent applications in the form of
*coroutines* (see :ref:`Coroutines`) that allow executing functions
concurrently, while the *channels* through which the coroutines can produce
results are asynchronous.

There is a basic set of language constructs that support concurrency. A function
that is to be launched asynchronously is marked by adding the ``async`` modifier
to its declaration. In addition, any function---or lambda expression---can be
launched as a separate thread explicitly by using the launch expression.

.. index::
   exception
   construct
   coroutine
   channel
   function
   async modifier
   launch expression
   launch

The ``await`` statement is introduced to synchronize functions launched as
threads. The generic class ``Promise<T>`` from the standard library (see
:ref:`Standard Library`) is used to exchange information between threads.
The class can be handled as an implementation of the channel mechanism.
The class provides a number of methods to manipulate the values produced
by threads.

Section :ref:`Packages` discusses a well-known and proven language feature
intended to organize large pieces of software that typically consist of many
components. *Packages* allow developers to construct a software product
as a composition of subsystems, and organize the development process in a way
that is appropriate for independent teams to work in parallel.

.. index::
   await statement
   function
   launch
   generic class
   standard library
   implementation
   channel
   package
   construct

*Package* is the language construct that combines a number of declarations,
and makes them parts of an independent compilation unit.

The *export* and *import* features are used to organize communication
between *packages*. An entity exported from one package becomes known to---
and accessible in---another package which imports that feature. Various
options are provided to simplify export/import, e.g., by defining
non-exported, i.e., ‘*internal*’ declarations that are not accessible from
the outside of the package.

In addition, the |LANG| supports the *package* initialization semantics that
makes a *package* even more independent from the environment.

In addition to the notion of *generic constructs*, the *declaration-site
variance* feature is also considered. The idea of the feature is briefly
described below, and in greater detail in :ref:`Generics Declaration-Site Variance`.

.. index::
   package
   construct
   declaration
   compilation unit
   export
   import
   internal declaration
   non-exported declaration
   access
   initialization
   declaration-site variance

Normally, two different argument types that specialize a generic class are
handled as different and unrelated types (*invariance*). |LANG| proposes
to extend the rule, and to allow such specializations become base classes and
derived classes (*covariance*), or vice versa (*contravariance*), depending on
the relationship of inheritance between argument types.

Special markers are used to specify the *declaration-site variance*.
The markers are to be added to generic parameter declarations.

The practices of some languages (e.g., Scala) have proven the usefulness of
this powerful mechanism. However, its practical usage can be relatively
difficult. Therefore, whether to add this feature to the language or not
is still under consideration.

.. index::
   generic class
   argument type
   invariance
   contravariance
   covariance
   generic parameter declaration
   inheritance
   derived class
   base class
   declaration-site variance

|

.. _Char Literals:

Char Literals
*************

.. meta:
    frontend_status: Done

A *char literal* represents the following:

-  A value with a single character; or
-  A single escape sequence preceded by the characters 'single quote' (U+0027)
   and 'c' (U+0063), and followed by a 'single quote' U+0027).

|

.. code-block:: abnf

      CharLiteral:
          'c\'' SingleQuoteCharacter '\''
          ;

      SingleQuoteCharacter:
          ~['\\\r\n]
          | '\\' EscapeSequence
          ;

Examples:

.. code-block:: typescript
   :linenos:

      c'a'
      c'\n'
      c'\x7F'
      c'\u0000'

*Char literals* are of type *char*.

.. index::
   char literal
   character
   escape sequence
   single quote
   type char

|

.. _Array Creation Expressions:

Array Creation Expressions
**************************

.. meta:
    frontend_status: Done

An *array creation expression* creates new objects that are instances of arrays.
The *array literal* expression is used to create an array instance, and to
provide some initial values (see :ref:`Array Literal`).

.. code-block:: typescript
   :linenos:

      newArrayInstance:
          'new' typeReference dimensionExpression+
          ;

      dimensionExpression:
          '[' expression ']'
          ;

.. code-block:: typescript
   :linenos:

      let x = new number[2][2] // create 2x2 matrix

An *array creation expression* creates an object that is a new array with the
elements of the type specified by *typeReference*.

The type of each *dimensionExpression* must be convertible (see
:ref:`Primitive Types Conversions`) to an integer type.
A :index:`compile-time error` occurs otherwise.

A numeric conversion (see :ref:`Primitive Types Conversions`) is
performed on each *dimensionExpression* to ensure that the resultant type
is *int*. A :index:`compile-time error` occurs otherwise.

A :index:`compile-time error` occurs if any *dimensionExpression* is a
constant expression that is evaluated at compile time to a negative integer
value.

.. code-block:: typescript
   :linenos:

      let x = new number[-3] // compile-time error

A :index:`compile-time error` occurs if *typeReference* refers to a class
that does not contain an accessible parameterless constructor, or if
*typeReference* has no a default value.

.. code-block:: typescript
   :linenos:

      let x = new string[3] // compile-time error: string has no default value


.. index::
   array creation expression
   object
   instance
   array
   array literal
   array instance
   initial value
   conversion
   integer type
   numeric conversion
   type int

|

.. _Runtime Evaluation of Array Creation Expressions:

Runtime Evaluation of Array Creation Expressions
************************************************

.. meta:
    frontend_status: Partly

The evaluation of an array creation expression at runtime is performed
as follows:

#. The dimension expressions are evaluated. The evaluation is performed
   left-to-right; if any expression evaluation completes abruptly, then
   the expressions to the right of it are not evaluated.

#. The values of dimension expressions are checked. If the value of any
   *dimExpr* expression is less than zero, then *NegativeArraySizeException*
   is thrown.

#. Space for the new array is allocated. If the available space is not
   sufficient to allocate the array, then *OutOfMemoryError* is thrown,
   and the evaluation of the array creation expression completes abruptly.

#. When a one-dimensional array is created, each element of that array
   is initialized to its default value if the type default value is defined
   (:ref:`Default Values for Types`).
   If the default value for an element type is not defined, but the element
   type is a class type, then its *parameterless* constructor is used to
   create each element’s value. 

#. When a multi-dimensional array is created, the array creation effectively
   executes a set of nested loops of depth *n-1*, and creates an implied
   array of arrays.

.. index::
   array
   constructor
   expression
   evaluation
   default value
   parameterless constructor
   class type
   initialization
   nested loop

|

.. _Enumeration SuperType:

Enumeration Super Type
**********************

.. meta:
    frontend_status: Partly

Any *enum* type has class type *Object* as its supertype. This allows
polymorphic assignments into *Object* type variables. The *instanceof*
check can be used to get enumeration variable back by applying 'as' conversion.

.. code-block:: typescript
   :linenos:

    enum Commands { Open = "fopen", Close = "fclose" }
    let c: Commands = Commands.Open
    let o: Object = c // Autoboxing of enum type to its reference version
    // Such reference version type has no name, but can be detected by instanceof
    if (o.instanceof (Commands)) {
       c = o as Commands // And explicitly converted back by 'as' conversion
    }

.. index::
   enum type
   class type
   Object
   supertype
   polymorphic assignment
   type variable
   enumeration variable
   conversion

|

.. _Enumeration Types Conversions:

Enumeration Types Conversions
=============================

.. meta:
    frontend_status: Done

Every *enum* type is compatible (see :ref:`Compatible Types`) with type
*Object* (see :ref:`Enumeration SuperType`). Every variable of *enum* type can
thus be assigned into a variable of type *Object*.

.. index::
   enum type
   compatibility
   Object
   variable
   assignment
   mutable variable


|


.. _Statements Experimental:

Statements
**********

.. meta:
    frontend_status: Done

|


.. _For-of Type Annotation:

For-of Type Annotation
======================

.. meta:
    frontend_status: Done

An explicit type annotation is allowed for a *for variable*:

.. code-block:: typescript
   :linenos:

      // explicit type is used for a new variable,
      let x: number[] = [1, 2, 3]
      for (let n: number of x) {
        console.log(n)
      }

.. index::
   explicit type annotation

|

.. _Multiple Clauses in Statements:

Multiple Clauses in Statements
===============================

.. meta:
    frontend_status: Done

When an exception or an error is thrown in the ``try`` block, or in a *throwing*
(see :ref:`Throwing Functions`) or *rethrowing* (:ref:`Rethrowing Functions`)
function called from the ``try`` block, the control is transferred to
the first *catch* clause if the statement has at least one *catch* clause
that can catch that exception or error. If no *catch* clause is found, then
*exception* or *error* is propagated to the surrounding scope.

**Note**: An exception handled within a *non-throwing* function (see
:ref:`Non-Throwing Functions`) is never propagated outside that function.

A *catch* clause has two parts:

-  An exception parameter that provides access to the object associated
   with the exception or the error occurred; and

-  A block of code that is to handle the situation.

.. index::
   exception
   error
   throwing function
   rethrowing function
   non-throwing function
   try block
   control transfer
   catch clause
   propagation
   surrounding scope
   exception parameter
   access

*Default catch clause* is a *catch* clause with the exception parameter type
omitted. Such a *catch* clause handles any exception or error that is not
handled by any previous clause. The type of that parameter is of the class
*Object*.

A :index:`compile-time error` occurs if:

-  The default *catch* clause is not the last *catch* clause in a ``try``
   statement.

-  The type reference of an exception parameter (if any) is neither the
   class *Exception* or *Error*, nor a class derived from *Exception* or
   *Error*.

.. code-block:: typescript
   :linenos:

      class ZeroDivisor extends Exception {}

      function divide(a: int, b: int): int throws {
        if (b == 0) throw new ZeroDivisor()
        return a / b
      }

      function process(a: int; b: int): int {
        try {
          let res = divide(a, b)

          // further processing ...
        }
        catch (d: ZeroDivisor) { return MaxInt }
        catch (e) { return 0 }
      }

.. index::
   default catch clause
   exception
   exception parameter
   error
   Exception
   Error
   try statement
   derived class

All exceptions that the ``try`` block can throw are caught by the function
'process'. Special handling is provided for the *ZeroDivisor* exception,
and the handling of other *exceptions* and *errors* is different.

*Catch* clauses do not handle every possible *exception* or *error*
that can be thrown by the code in the ``try`` block. If no *catch* clause
can handle the situation, then *exception* or *error* is propagated to
the surrounding scope.

**Note**: If a ``try`` statement (*default catch clause*) is placed inside
a *non-throwing* function (see :ref:`Non-Throwing Functions`), then 
*exception* is never propagated.

.. index::
   exception
   try block
   propagation
   try statement
   default catch clause
   non-throwing function

If a *catch* clause contains a block that corresponds to the *error*'s
parameter, then it can only handle that *error*.

The type of the *catch* clause parameter in a *default catch clause* is
omitted. The *catch* clause can handle any *exceptions* or *errors*
unhandled by the previous clauses.

The type of a *catch* clause parameter (if any) must be of the class *Error*
or *Exception*, or of another class derived from *Exception* or *Error*.

.. index::
   exception
   error
   catch clause
   default catch clause
   derived class
   Error
   Exception

.. code-block:: typescript
   :linenos:

        function process(a: int; b: int): int {
        try {
          return a / b
        }
        catch (x: DivideByZeroError) { return MaxInt }
      }

A *catch* clause handles the *DivideByZeroError* at runtime. Other errors
are propagated to the surrounding scope if no *catch* clause is found.

.. index::
   catch clause
   runtime
   error
   propagation
   surrounding scope

|

.. _Assert Statements Experimental:

``Assert`` Statements
=====================

.. meta:
    frontend_status: Done

An ``assert`` statement can have one or two expressions. The first expression
is of type *boolean*; the optional second expression is of type *string*. A
:index:`compile-time error` occurs if the types of the expressions fail to match.

.. code-block:: abnf

      assertStatement:
          'assert' expression (':' expression)?
          ;

*Assertions* control mechanisms that are not part of |LANG|, yet the
language allows having assertions either *enabled* or *disabled*.

.. index::
   assert statement
   assertion
   expression
   boolean
   string

The execution of the *enabled* assertion starts from the evaluation of the
*boolean* expression. An error is thrown if the expression evaluates to
``false``. The second expression is then evaluated (if provided). Its
value passes as the *error* argument.

The execution of the *disabled* assertion has no effect whatsoever.

.. index::
   assertion
   execution
   boolean
   evaluation
   argument
   value

.. code-block:: typescript
   :linenos:

      assert p != null
      assert f.IsOpened() : "file must be opened" + filename
      assert f.IsOpened() : makeReportMessage()

|

.. _Function and Method Overloading:

Function and Method Overloading
===============================

.. meta:
    frontend_status: Done

Like the |TS| language, |LANG| supports overload signatures that allow
specifying several headers for a function or method with different signatures.
Most other languages support a different form of overloading that specifies
a separate body for each overloaded header.

Both approaches have their advantages and disadvantages. The |LANG|
experimental approach allows for improved performance as a specific body
is executed at runtime.

.. index::
   function overloading
   method overloading
   overload signature
   header
   function
   method
   signature
   overloaded header
   execution
   runtime

|

.. _Function Overloading:

Function Overloading
====================

.. meta:
    frontend_status: Done

If a declaration scope declares two functions with the same name but
different signatures that are not *override-equivalent* (see
:ref:`Override-Equivalent Signatures`), then the functions' name is
*overloaded*.

This fact is not difficult, and cannot cause a :index:`compile-time error`
on its own.
No specific relationship is required between the return types, or between the
*throws* clauses of the two functions with the same name but different
signatures that are not *override-equivalent*.

When calling a function, the number of actual arguments (and any explicit type
arguments) and compile-time types of arguments is used at compile time to
determine the signature of the function being called (see
:ref:`Function Call Expression`).

.. index::
   function overloading
   declaration scope
   signature
   name
   override-equivalent signature
   overloaded function name
   return type
   throws clause
   argument
   actual argument
   explicit type argument
   function call


|

.. _Class Method Overloading:

Class Method Overloading
========================

.. meta:
    frontend_status: Done

If two methods within a class have the same name, and their signatures are not
*override-equivalent*, then the methods' name is considered *overloaded*.

An *overloaded* method name cannot cause a :index:`compile-time error`
on its own.

If the signatures of two methods with the same name are not *override-equivalent*,
then the return types of those methods, or the *throws* or *rethrows* clauses
of those methods can have any kind of relationship.

A number of actual arguments, explicit type arguments, and compile-time types
of the arguments is used at compile time to determine the signature of the
method being called (see :ref:`Method Call Expression`, and
:ref:`Step 2 Selection of Method`).

In the case of an instance method, the actual method being called is determined
at runtime by using the dynamic method lookup (see :ref:`Method Call Expression`)
provided by the runtime system.

.. index::
   class method overloading
   signature
   override-equivalent signature
   throws clause
   rethrows clause
   explicit type argument
   actual argument
   method call
   instance method
   runtime
   dynamic method lookup

|

.. _Interface Method Overloading:

Interface Method Overloading
============================

.. meta:
    frontend_status: Done

If two methods of an interface (declared or inherited in any combination)
have the same name but different signatures that are not *override-equivalent*
(see :ref:`Inheriting Methods with Override-Equivalent Signatures`), then
such method name is considered *overloaded*.

However, this causes no :index:`compile-time error` on its own, because no
specific relationship is required between the return types, or between the
*throws* clauses of the two methods.

.. index::
   interface method overriding
   interface
   method
   override-equivalent signature
   inherited method
   overloaded method
   method inheritance
   declared method
   return type
   throws clause
   signature

|

.. _Constructor Overloading:

Constructor Overloading
=======================

.. meta:
    frontend_status: Done

The constructor overloading behaves identically to the method overloading (see
:ref:`Class Method Overloading`). Each class instance creation expression (see
:ref:`New Expressions`) resolves the overloading at compile time.

.. index::
   constructor overloading
   method overloading
   class instance creation expression

|

.. _Declaration Distinguishable by Signatures:

Declaration Distinguishable by Signatures
=========================================

.. meta:
    frontend_status: Done

Declarations with the same name are distinguishable by signatures if:

-  They are functions with the same name, but their signatures are not
   *override-equivalent* (see :ref:`Function Overloading`).

-  They are methods with the same name, but their signatures are not
   *override-equivalent* (see :ref:`Class Method Overloading`, and
   :ref:`Interface Method Overloading`).

.. index::
   signature
   function overloading
   override-equivalent signature
   interface method overloading
   class method overloading


The example below is of functions distinguishable by signatures:

.. code-block:: typescript
   :linenos:

      function foo() {}
      function foo(x: number) {}
      function foo(x: number[]) {}
      function foo(x: string) {}

The example below is of functions undistinguishable by signatures that cause a
:index:`compile-time error`:

.. index::
   function
   signature

.. code-block:: typescript
   :linenos:

      // Functions have override-equivalent signatures
      function foo(x: number) {}
      function foo(y: number) {}

      // Functions have override-equivalent signatures
      function foo(x: number) {}
      type MyNumber = number
      function foo(x: MyNumber) {}

|

.. _Native Functions and Methods:

Native Functions and Methods
****************************

.. meta:
    frontend_status: Done

|

.. _Native Functions:

Native Functions
================

.. meta:
    frontend_status: Done

A *native* function implemented in a platform-dependent code is typically
written in another programming language (e.g., *C*).

A :index:`compile-time error` occurs if a *native* function has a body.

.. index::
   native function
   implementation
   platform-dependent code

|

.. _Native Methods Experimental:

Native Methods
==============

.. meta:
    frontend_status: Done

*Native* methods are methods implemented in a platform-dependent code written
in another programming language (e.g., *C*).

A :index:`compile-time error` occurs if:

-  A method declaration contains the keyword ``abstract`` along with the
   keyword ``native``.

-  A *native* method has a body (see :ref:`Method Body`) that is a block
   instead of a simple semicolon or empty body.

.. index::
   native method
   implementation
   platform-dependent code
   keyword native
   method body
   block
   method declaration
   keyword abstract

|

.. _Final Classes and Methods:

Final Classes and Methods
*************************

.. meta:
    frontend_status: Done

|

.. _Final Classes:

Final Classes
=============

.. meta:
    frontend_status: Done

A class may be declared *final* to prevent its extension. A class declared
*final* cannot have subclasses, and no method of a *final* class can be
overridden.

A :index:`compile-time error` occurs if the *extends* clause of a class
declaration contains another class that is *final*.

.. index::
   final class
   method
   overriding
   class
   class extension
   extends clause
   class declaration
   subclass

|

.. _Final Methods Experimental:

Final Methods
=============

.. meta:
    frontend_status: Done

A method can be declared *final* to prevent it from being overridden (see
:ref:`Overriding by Instance Methods`) or hidden in subclasses.

A :index:`compile-time error` occurs if:

-  A method declaration contains the keyword ``abstract`` or ``static``
   along with the keyword ``final``.

-  A method declared *final* is overridden.

.. index::
   final method
   overriding
   instance method
   hiding
   subclass
   method declaration
   keyword abstract
   keyword static
   keyword final

|

.. _Default and Static Interface Methods:

Default and Static Interface Methods
************************************

.. meta:
    frontend_status: Done

|

.. _Default Method Declarations:

Default Method Declarations
============================

.. meta:
    frontend_status: Done

.. code-block:: abnf

    interfaceDefaultMethodDeclaration:
        'private'? identifier signature block
        ;

A default method can be explicitly declared *private* in an interface body.

A block of code that represents the body of a default method in an interface
provides a default implementation for any class if such class does not override
the method to implement the interface.

.. index::
   default method
   method declaration
   private
   implementation
   default method body
   interface body
   default implementation
   overriding

|

.. _Static Method Declarations:

*Static* Method Declarations
============================

.. meta:
    frontend_status: Done

.. code-block:: abnf

    interfaceStaticMethodDeclaration:
        'static' 'private'? identifier signature block
        | 'private'? 'static' identifier signature block
        ;

A *static* method in an interface body can be explicitly declared *private*.

*Static* interface method calls refer to no particular object.

In contrast to default methods, *static* interface methods are not instance
methods.

.. index::
   static method declaration
   static method
   interface body
   private
   static interface method
   default method
   instance method
   
A :index:`compile-time error` occurs if:

-  The body of a *static* method attempts to use the keywords ``this`` or
   ``super`` to refer to the current or a supertype object.

-  The header or body of a *static* method of an interface contains the
   name of any surrounding declaration’s type parameter.

.. index::
   static method body
   keyword this
   keyword super
   static method header
   static method body
   interface
   type parameter
   surrounding declaration

|

.. _Extension Functions:

Extension Functions
*******************

.. meta:
    frontend_status: Partly
    todo: static extension functions, import/export of them, extension function for primitive types

The *extension function* mechanism allows using a special form of top-level
functions as class or interface extensions. Syntactically, *extension* is the
addition of a new functionality.

*Extensions* can be called in the usual way as if they were methods of the
original class. However, *extensions* do not actually modify the classes they
extend. No new member is inserted into a class; only new *extension functions*
are callable with the *dot-notation* on variables of the class.

*Extension functions* are dispatched statically; what *extension function*
is being called is already known at compile time based on the receiver type
specified in the extension function declaration.

.. index::
   function
   class extension
   interface extension
   functionality
   function call
   original class
   class member
   extension function
   callable function
   dot-notation
   receiver type
   extension function declaration

*Extension functions* specify names, signatures, and bodies:

.. code-block:: abnf

    extensionFunctionDeclaration:
        'static'? 'function' typeParameters? typeReference '.' identifier
        signature block
        ;

The keyword ``this`` inside an extension function corresponds to the receiver
object (i.e., *typeReference* before the dot).

Class or interface referred by typeReference, and *private* or *protected*
members are not accessible within the bodies of their *extension functions*.
Only *public* members can be accessed:

.. index::
   keyword this
   extension function
   receiver object

.. code-block:: typescript
   :linenos:

      class A {
          foo () { ... this.bar() ... } 
                       // Extension function bar() is accessible
          protected member_1 ...
          private member_2 ...
      }
      function A.bar () { ... 
         this.foo() // Method foo() is accessible as it is public
         this.member_1 // Compile-time error as member_1 is not accessible
         this.member_2 // Compile-time error as member_2 is not accessible
         ...
      }                              
      let a = new A()
      a.foo() // Ordinary class method is called
      a.bar() // Class extension function is called

*Extension functions* can be generic as illustrated by the example below:

.. code-block:: typescript
   :linenos:

     function <T> B<T>.foo(p: T) {
          console.log (p)
     }
     function demo (p1: B<SomeClass>, p2: B<BaseClass>) {
         p1.foo (new SomeClass())
           // Type inference should determine the instantiating type
         p2.foo <BaseClass>(new DerivedClass())
          // Explicit instantiation
     }

*Extension functions* are top-level functions, and can call each other. The
form of such calls depends on whether static was used while declaring or not.
This affects the kind of receiver to be used for the call. A *static extension
function* requires the name of the type (class or interface). A *non-static
extension function* requires a variable as receiver:

.. code-block:: typescript
   :linenos:

      class A {
          foo () { ...
             this.bar() // non-static extension function is called with this.
             A.goo() // static extension function is called with class name receiver
             ...
          }
      }
      function A.bar () { ... 
         this.foo() // Method foo() is called
         A.goo() // Other static extension function is called with class name receiver
         ...
      }                              
      static function A.goo () { ... 
         this.foo() // Compile-time error as instance members are not accessible
         this.bar() // Compile-time error as instance extension functions are not acessible
         ...
      }
      let a = new A()
      a.foo() // Ordinary class method is called
      a.bar() // Class instance extension function is called
      A.goo() // Static extension function is called

*Extension functions* are dispatched statically and remain active for all
derived classes until the next definition of the *extension function* for the
derived class is found:

|

.. code-block:: typescript
   :linenos:

      class Base { ... }
      class Derived extends Base { ... }
      function Base.foo () { console.log ("Base.foo is called") }
      function Derived.foo () { console.log ("Derived.foo is called") }

      let b: Base = new Base()
      b.foo() // `Base.foo is called` to be printed
         b = new Derived()
      b.foo() // `Base.foo is called` to be printed
      let d: Derived = new Derived()
      d.foo() // `Derived.foo is called` to be printed

*Extension functions* can be:

-  Put into a compilation unit other than class or interface; and
-  Imported by using a name of the *extension function*.

.. code-block:: typescript
   :linenos:

      // file a.ets
      import {bar} from "a.ets" // import name 'bar'
      class A {
          foo () { ...
             this.bar() // non-static extension function is called with this.
             A.goo() // static extension function is called with class name receiver
             ...
          } 
      }

      // file ext.ets
      import {A} from "a.ets" // import name 'A'
      function A.bar () { ... 
         this.foo() // Method foo() is called
         ...
      }

If an *extension function* and a type method have the same name and signature,
then calls to that name are routed to the method:

.. code-block:: typescript
   :linenos:

      class A {
          foo () { console.log ("Method A.foo is called") } 
      }
      function A.foo () { console.log ("Extension A.foo is called") }
      let a = new A()
      a.foo() // Method is called, `Method A.foo is called` to be printed out

The precedence between methods and *extension functions* can be expressed
by the following formula:

  derived type instance method <
  base type instance method <
  derived type extension function <
  base type extension function.

In other words, the priority of standard object-oriented semantics is higher
than that of type extension functions:

|

.. code-block:: typescript
   :linenos:

      class Base {
         foo () { console.log ("Method Base.foo is called") }
      }
      class Derived extends Base {
         override foo () { console.log ("Method Derived.foo is called") }
      }
      function Base.foo () { console.log ("Extension Base.foo is called") }
      function Derived.foo () { console.log ("Extension Derived.foo is called") }

      let b: Base = new Base()
      b.foo() // `Method Base.foo is called` to be printed
      b = new Derived()
      b.foo() // `Method Derived.foo is called` to be printed
      let d: Derived = new Derived()
      d.foo() // `Method Derived.foo is called` to be printed

If an *extension function* and another top-level function have the same name
and signature, then calls to this name are routed to a proper function in
accordance with the form of the call. *Extension functions* cannot be called
without a receiver as they have access to ``this``.

.. code-block:: typescript
   :linenos:

      class A { ... }
      function A.foo () { console.log ("Extension A.foo is called") }
      function foo () { console.log ("Top-level foo is called") }
      let a = new A()
      a.foo() // Extension function is called, `Extension A.foo is called` to be printed out
      foo () // Top-level function is called, `Top-level foo is called` to be printed out


|

.. _Trailing Lambda:

Trailing Lambda
***************

.. meta:
    frontend_status: Done

The *trailing lambda* mechanism allows using a special form of function
or method call when the last parameter of a function or a method is of
function type, and the argument is passed as a lambda using the ``{}``
notation.

Syntactically, the *trailing lambda* looks as follows:

.. index::
   trailing lambda
   function call
   method call
   function parameter
   method parameter
   lambda
   function type

.. code-block:: typescript
   :linenos:

      class A {
          foo (f: ()=>void) { ... } 
      }

      let a = new A()
      a.foo() { console.log ("method lambda argument is activated") }
      // method foo receives last argument as an inline lambda

The formal syntax of the *trailing lambda* is presented below:

|

.. code-block:: abnf

    trailingLambdaCall:
        ( objectReference '.' identifier typeArguments? 
        | expression ('?.' | typeArguments)?
        )
        arguments block
        ;


Currently, no parameter can be specified for the trailing lambda. A
compile-time error occurs otherwise.

**Note**: If a call is followed by a block, and the function or method
being called has no last function type parameter, then such block is
handled as an ordinary block of statements but not as a lambda function.

In case of other ambiguities---e.g., when a function or method call has
the last parameter, which can be optional, of a function type---the syntax
production that starts with '{' following the function or method call is
handled as the *trailing lambda*.
If other semantics is needed, then a separating semicolon ';' can be used.
It means that the function or the method is to be called without the last
argument (see :ref:`Optional Parameters`).

.. code-block:: typescript
   :linenos:

      class A {
          foo (p?: ()=>void) { ... } 
      }

      let a = new A()
      a.foo() { console.log ("method lambda argument is activated") }
      // method foo receives last argument as an inline lambda

      a.foo(); { console.log ("that is the block code") }
      // method 'foo' is called with 'p' parameter set to 'undefined'
      // ';' allows to specify explicitly that '{' starts the block

      function bar(f: ()=>void) { ... }

      bar() { console.log ("function lambda argument is activated") }
      // function 'bar' receives last argument as an inline lambda,
      bar(); { console.log ("that is the block code") }
      // function 'bar' is called with 'p' parameter set to 'undefined'

.. index::
   trailing lambda
   compile-time error
   call
   block
   statement
   function
   method
   lambda function
   function type parameter

.. code-block:: typescript
   :linenos:

     function foo (f: ()=>void) { ... }
     function bar (n: number) { ... }

     foo() { console.log ("function lambda argument is activated") }
     // function foo receives last argument as an inline lambda,

     bar(5) { console.log ("after call of 'bar' this block is executed") }

     foo(() => { console.log ("function lambda argument is activated") }) 
     { console.log ("after call of 'foo' this block is executed") }
     /* here, function foo receives lambda as an argument and a block after
      the call is just a block, not a trailing lambda. */

|

.. _Enumeration Methods:

Enumeration Methods
*******************

.. meta:
    frontend_status: Partly

There are several static methods available to handle each enumeration type:

-  'values()' returns an array of enumeration constants in the order of
   declaration.
-  'valueOf(name: string)' returns an enumeration constant with the given
   name, or throws an error if no constant with such name exists.

.. index::
   enumeration method
   static method
   enumeration type
   enumeration constant
   error
   constant

.. code-block:: typescript
   :linenos:

      enum Color { Red, Green, Blue }
      let colors = Color.values()
      //colors[0] is the same as Color.Red
      let red = Color.valueOf("Red")

There is an additional method for instances of any enumeration type:

-  'getValue()' returns a value of enumeration constant which is
   either of ``int`` or ``string`` type.

.. code-block:: typescript
   :linenos:

      enum Color { Red, Green = 10, Blue }
      let c: Color = Color.Green
      console.log(c.getValue()) // prints 10

**Note**: ``c.toString()`` returns the same value as ``c.getValue()``; its
type must be converted to *string* for enumeration constants of a numeric type.

.. index::
   instance
   enumeration type
   value
   numeric type
   enumeration constant
   type int
   type string

|

.. _Exceptions:

Exceptions
**********

.. meta:
    frontend_status: Done

*Exception* is the base class of all exceptions. *Exception* is used to
define a new exception, or any class derived from the *Exception* as the
base of a class:

.. code-block:: typescript
   :linenos:

      class MyException extends Exception { ... }

.. index::
   exception
   base class
   Exception

A :index:`compile-time error` occurs if a generic class is a direct or
indirect subclass of *Exception*.

An *exception* is thrown explicitly with the ``throw`` statement.

When an *exception* is thrown, the surrounding piece of code is to handle it by
correcting the problem, trying an alternative approach, or informing the user.

There are two ways to process an *exception*:

-  Propagating the exception from a function to the code that calls that
   function (see :ref:`Throwing Functions`);

-  Using a ``try`` statement to handle the exception (see :ref:`Try Statements`).

.. index::
   exception
   base class
   Exception
   try statement
   propagation
   function
   throwing function
   function call

|

.. _Throwing Functions:

Throwing Functions
==================

.. meta:
    frontend_status: Done

The keyword ``throws`` is used at the end of a signature to indicate that a
function (this notion here includes methods, constructors, or lambdas) can
throw an exception. A function ending with ``throws`` is called a
*throwing function*. The function type can also be marked as ``throws``.

.. index::
   keyword throws
   throwing function
   signature
   method
   constructor
   lambda
   function
   exception
   function type
   throws mark

.. code-block:: typescript
   :linenos:

      function canThrow(x: int): int throws { ... }

A *throwing function* can propagate exceptions to the scope from which
it is called. The propagation of an *exception* occurs if:

-  The call of a *throwing function* is not enclosed in a ``try`` statement; or

-  The enclosed ``try`` statement does not contain a clause that can catch the
   exception.


In the example below, the function call is not enclosed in a ``try``
statement; any exception raised by *canThrow* function is propagated:

.. index::
   throwing function
   propagation
   exception
   scope
   function call
   try statement

.. code-block:: typescript
   :linenos:

      function propagate1(x: int): int throws {
        return y = canThrow(x) // exception is propagated
      }


In the example below, the ``try`` statement can catch only ``this`` exceptions.
Any exception raised by *canThrow* function---but for *MyException* itself, and
any exception derived from *MyException*---is propagated:

.. index::
   try statement
   this
   exception
   propagation

.. code-block:: typescript
   :linenos:

      function propagate2(x: int): int throws {
        try {
          return y = canThrow(x) //
        }
        catch (e: MyException) /*process*/ }
          return 0
      }

.. _Non-Throwing Functions:

Non-Throwing Functions
======================

.. meta:
    frontend_status: Done

A *non-throwing function* is a function (this notion here includes methods,
constructors, or lambdas) not marked as ``throws``. Any exceptions inside a
*non-throwing function* must be handled inside the function.

A :index:`compile-time error` occurs if not **all** of the following
requirements are met:

-  The call of a *throwing function* is enclosed in a ``try`` statement;

-  The enclosing ``try`` statement has a default *catch* clause.

.. index::
   non-throwing function
   throwing function
   function
   method
   constructor
   lambda
   throws mark
   try statement
   catch clause
   

.. code-block:: typescript
   :linenos:

      // non-throwing function
      function cannotThrow(x: int): int {
        return y = canThrow(x) // compile-time error
      }

      function cannotThrow(x: int): int {
        try {
          return y = canThrow(x) //
        }
        catch (e: MyException) { /* process */ }
        // compile-time error – default catch clause is required
      }

|

.. _Rethrowing Functions:

Rethrowing Functions
====================

.. meta:
    frontend_status: Done

A *rethrowing function* is a function that accepts a *throwing function* as a
parameter, and is marked with the keyword ``rethrows``.

The body of such function must not contain any ``throw`` statement that is
not handled by a ``try`` statement within that body. A function with unhandled
``throw`` statements must be marked with the keyword ``throws`` but not
``rethrows``.

.. index::
   rethrowing function
   throwing function
   non-throwing function
   function parameter
   keyword throws
   keyword rethrows
   try statement
   throw statement

Both a *throwing* and a *non-throwing* function can be an argument of a
*rethrowing function* *foo* that is being called.

If a *throwing function* is an argument, then the calling of *foo* can
throw an exception.

This rule is exception-free, i.e., a *non-throwing* function used as a call
argument cannot throw an exception:

.. code-block:: typescript
   :linenos:

        function foo (action: () throws) rethrows {
        action()
      }

      function canThrow() {
        /* body */
      }

      function cannotThrow() {
        /* body */
      }

      // calling rethrowing function:
        foo(canThrow) // exception can be thrown 
        foo(cannotThrow) // exception-free

A call is exception-free if:

-  Function *foo* has several parameters of a function type marked
   with *throws*; and

-  All actual arguments of the call to *foo* are non-throwing.

However, the call can raise an exception, and is handled as any other
*throwing function* call if at least one of the actual function arguments
is *throwing*.

It implies that a call to *foo* within the body of a *non-throwing* function
must be guaranteed with a ``try-catch`` statement.

.. index::
   function
   exception-free call
   function type parameter
   throws mark
   throwing function
   non-throwing function
   try-catch statement

.. code-block:: typescript
   :linenos:

      function mayThrowContext() throws {
        // calling rethrowing function:
        foo(canThrow) // exception can be thrown
        foo(cannotThrow) // exception-free
      }

      function neverThrowsContext() {
        try {
          // calling rethrowing function:
          foo(canThrow) // exception can be thrown
          foo(cannotThrow) // exception-free
        }
        catch (e) {
          // To handle the situation
        }
      }

|

.. _Exceptions and Initialization Expression:

Exceptions and Initialization Expression
========================================

.. meta:
    frontend_status: Partly

A *variable declaration* (see :ref:`Variable Declarations`) or a *constant
declaration* (see :ref:`Constant Declarations`) expression used to initialize
a variable or constant must not have calls to functions that can *throw* or
*rethrow* exceptions if the declaration is not within a statement that handles
all exceptions.

See :ref:`Throwing Functions` and :ref:`Rethrowing Functions` for details.

.. index::
   variable declaration
   exception
   initialization expression
   constant declaration
   expression
   initialization
   variable
   constant
   function call
   throw exception
   rethrow exception
   statement
   throwing function
   rethrowing function

|

.. _Exceptions and Errors Inside Field Initializers:

Exceptions and Errors Inside Field Initializers
===============================================

.. meta:
    frontend_status: Partly

Class field initializers cannot call *throwing* or *rethrowing* functions.

See :ref:`Throwing Functions` and :ref:`Rethrowing Functions` for details.

.. index::
   exception
   error
   field initializer
   throwing function
   rethrowing function

|

.. _Coroutines:

Coroutines
**********

.. meta:
    frontend_status: Partly

A function or lambda can be a *coroutine*. |LANG| supports **basic coroutines**,
**structured coroutines*,* and communication **channels**.
Basic coroutines are used to create and launch a coroutine; the result is then
to be awaited.

.. index::
   structured coroutine
   basic coroutine
   function
   lambda
   coroutine
   channel
   launch

|

.. _Create and Launch a Coroutine:

Create and Launch a Coroutine
=============================

.. meta:
    frontend_status: Done

The following expression is used to create and launch a coroutine:

.. code-block:: typescript
   :linenos:

      launchExpression: 'launch' expression;

A :index:`compile-time error` occurs if the expression is not a *function call
expression* (see :ref:`Function Call Expression`).

|

.. code-block:: typescript
   :linenos:

      let res = launch cof(10)

      // where 'cof' can be defined as:
      function cof(a: int): int {
        let res: int
        // Do something
        return res
      }

Lambda is used in the launch expression as follows:

.. code-block:: typescript
   :linenos:

      let res = launch (n: int) => { /* lambda body */(7)

.. index::
   expression
   coroutine
   launch
   function call expression
   lambda
   launch expression

The launch expression result is of type *Promise<T>*, where *T* is the return
type of the function being called:

.. code-block:: typescript
   :linenos:

      function foo(): int {}
      function bar() {}
      let resfoo = launch foo()
      let resbar = launch bar()

The type of *resfoo* in the example above is *Promise<int>*, and the
type of *resbar* is *Promise<void>*.

Similarly to |TS|, |LANG| supports the launching of a coroutine by calling
the function *async* (see :ref:`Async Functions`). No restrictions apply as to
from what scope to call the function *async*.

.. index::
   launch expression
   return type
   function call
   coroutine
   function async
   restriction

.. code-block:: typescript
   :linenos:

      async function foo(): Promise<int> {}

      // This will create and launch coroutine
      let resfoo = foo()

|

.. _Awaiting a Coroutine:

Awaiting a Coroutine
====================

.. meta:
    frontend_status: Done

The expressions *await* and *wait* are used while a previously launched
coroutine finishes and returns a value.

.. code-block:: abnf

      awaitExpresson:
        'await' expression
        ;

A :index:`compile-time error` occurs if the expression type is not *Promise<T>*.

.. index::
   expression await
   expression wait
   launch
   coroutine
   expression type

.. code-block:: typescript
   :linenos:

      let promise = launch (): int { return 1 } ()
      console.log(await promise) // output: 1

If the coroutine result must be ignored, then the expression statement
``await`` is used.

.. code-block:: typescript
   :linenos:

      function foo() { /* do something */ }
      let promise = launch foo()
      await promise

.. index::
   coroutine
   expression statement await

|

.. _The Promise T Class:

The Promise<T> Class
====================

.. meta:
    frontend_status: Partly

The class  *Promise<T>* represents the values returned by launch expressions.
The definition of type *Promise<T>* belongs the '*package std.core*'
of the standard library (see :ref:`Standard Library`).

The following methods are used:

-  *then* takes two arguments (the first argument is the callback used if the
   promise is fulfilled, and the second if it is rejected), and returns
   *Promise<U>*.

.. index::
   class
   value
   launch expression
   argument
   callback
   package
   standard library
   method

.. code-block:: abnf

        Promise<U> Promise<T>::then<U>(fullfillCallback :
            function
        <T>(val: T) : Promise<U>, rejectCallback : (err: Object)
        : Promise<U>)

-  *catch* is the alias for *Promise<T>*.then<U>((value: T) : U => {},
   onRejected).

.. code-block:: abnf

        Promise<U> Promise<T>::catch<U>(rejectCallback : (err:
            Object) : Promise<U>)

-  *finally* takes one argument (the callback called after *promise* is either
   fulfilled or rejected) and returns *Promise<T>*.

.. index::
   alias
   callback
   call

.. code-block:: abnf

        Promise<U> Promise<T>::finally<U>(finallyCallback : (
            Object:
        T) : Promise<U>)

|

.. _Structured Coroutines:

Structured Coroutines
=====================

|

.. _Channels Classes:

Channels Classes
================

*Channels* are used to send data between coroutines.

*Channels classes* are a part of the corouitnes-related package of the
standard library (see :ref:`Standard Library`).

.. index::
   channel class
   coroutine
   package

|

.. _Async Functions:

Async Functions
===============

.. meta:
    frontend_status: Partly

The function *async* is implicitly a coroutine that can be called as a
regular function.

The return type of an *async* function must be *Promise<T>* (see
:ref:`The Promise T Class`).
Returning values of types *Promise<T>* and *T* from the function *async*
is allowed.

Using return statement without expression is allowed if the return type
is *Promise<void>*.
*No-argument* return statement can be implicitly added as the last statement
of the function body if there is no explicit return statement in a function
with the return type *Promise<void>*.

**Note**: Using this annotation is not recommended because this type of
functions is only supported for the sake of backward |TS| compatibility.

.. index::
   function async
   coroutine
   return type
   function body
   backward compatibility
   annotation

|

.. _Packages:

Packages
********

.. meta:
    frontend_status: Partly

One or more *package modules* form a package.

.. code-block:: abnf

      packageDeclaration:
          packageModule+
          ;

*Packages* are stored in a file system or a database (see
:ref:`Compilation Units in Host System`).

A *package* can consist of several package modules if all such modules
have the same *package header*.

.. index::
   package module
   package
   file system
   database
   package header

.. code-block:: abnf

      packageModule:
          packageHeader packageModuleDeclaration
          ;

      packageHeader:
          'package' qualifiedName
          ;

      packageModuleDeclaration:
          importDirective* packageTopDeclaration*
          ;

      packageTopDeclaration:
          topDeclaration | packageInitializer
          ;

A :index:`compile-time error` occurs if:

-  A *package module* contains no package header; or

-  Package headers of two package modules in the same package have
   different identifiers.

A *package module* automatically imports all exported entities from essential
kernel packages (‘std.core’ and 'escompat') of the standard library (see
:ref:`Standard Library`). All entities from these packages are accessible
as simple names.

A *package module* can automatically access all top-level entities
declared in all modules that constitute the package.

.. index::
   package module
   package header
   package
   identifier
   import
   exported entity
   access
   top-level entity
   module
   standard library
   simple name

|

.. _Internal Access Modifier Experimental:

Internal Access Modifier
========================

.. meta:
    frontend_status: Partly

The modifier *internal* indicates that a class member or a constructor is
accessible within their compilation unit only. A compilation unit that is a
package can be used in any *package module* (see :ref:`Packages`).

.. index::
   modifier
   internal access modifier
   class member
   constructor
   access
   package module

.. code-block:: typescript
   :linenos:

      class C {
        internal count: int
        getCount(): int {
          return this.count // ok
        }
      }

      function increment(c: C) {
        c.count++ // ok
      }

A member or a constructor with both *internal* and *protected* modifiers (see
below) can be accessed as *internal* and *protected*.

.. index::
   member
   constructor
   internal modifier
   protected modifier
   access

|

.. _Package Initializer:

Package Initializer
===================

Among all *package modules* there can be one to contain a code that performs
initialization of global variables across all package modules.

The syntax is presented below:

.. index::
   package initializer
   package module
   initialization
   variable

.. code-block:: abnf

      packageInitializer:
          'static' block
          ;

A :index:`compile-time error` occurs if a package contains more than one
*package initializer*.

A *package initializer* is executed only once right before the first activation
of the package (calling an exported function or accessing an exported
global variable).

.. index::
   package initializer
   package
   execution
   exported function
   access
   exported global variable
   function call

|

.. _Sub-Entity Binding:

Sub-Entity Binding
==================

The import bindings '*qualifiedName*' (that consists of at least two
identifiers) or '*qualifiedName* as A' bind a sub-entity to the declaration
scope of the current module.

'L' is a *static* entity and the last identifier in the '*qualifiedName* A.B.L'.
L’s *public* access modifier is defined in the class or interface denoted in the
previous part of the '*qualifiedName*'. 'L' is accessible regardless of the
export status of the class or the interface it belongs to.

An entity (or—in the case of overloaded methods—entities) is bound by its
original name, or by an alias (if an alias is set). In the latter case, the
original name is not accessible.

.. index::
   sub-entity binding
   import binding
   identifier
   module
   declaration scope
   static entity
   public access modifier
   class
   interface
   access
   export status
   entity
   overloaded method
   alias

The following module can be considered as an example:

.. code-block:: typescript
   :linenos:

      class A {
        class B {
          public static L: int
        }
      }

The table below illustrates the import of this module:

+-----------------------------------+-+--------------------------------------+
| Import                            | | Usage                                |
+===================================+=+======================================+
| .. code-block:: typescript        | | .. code-block:: typescript           |
|                                   | |                                      |
|     import {A.B.L} from "..."     | |     if (L == 0) { ... }              |
+-----------------------------------+-+--------------------------------------+
| .. code-block:: typescript        | | .. code-block:: typescript           |
|                                   | |                                      |
|     import {A.B} from "..."       | |     let x = new B() // OK            |
|                                   | |     let y = new A() // Error: 'A' is |
|                                   | |        not accessible                |
+-----------------------------------+-+--------------------------------------+
| .. code-block:: typescript        | | .. code-block:: typescript           |
|                                   | |                                      |
|     import {A.B.L as X} from ".." | |     if (X == 0) { ... }              |
+-----------------------------------+-+--------------------------------------+
| .. code-block:: typescript        | | .. code-block:: typescript           |
|                                   | |                                      |
|     import {A.B as AB} from "..." | |     let x = new AB()                 |
+-----------------------------------+-+--------------------------------------+

This form of binding is included in the language specifically to simplify
the migration from the languages that support access to sub-entities as
simple names. This feature is to be used only for migration.

.. index::
   import
   access
   binding
   migration
   sub-entity

|

.. _All Static Sub-Entities Binding:

All Static Sub-Entities Binding
===============================

The import binding '*qualifiedName.\** ' binds all *public static* sub-entities
of the entity denoted by the *qualifiedName* to the declaration scope of the
current module.

The following module can be considered as an example:

.. index::
   import binding
   static sub-entity binding
   public static sub-entity
   declaration scope
   entity
   module

.. code-block:: typescript
   :linenos:

      class A {
        class Point {
          public static X: int
          public static Y: int
          public isZero(): boolean {}
        }
      }

The examples below illustrate the import of this module:

.. code-block:: typescript
   :linenos:

      // Import:
      import A.Point.* from "..."

.. code-block:: typescript
   :linenos:

      // Usage:
      import A.Point.* from "..."

      if ((X == 0) && (Y == 0)) { // OK
         // ...
      }

      let x = isZero() / Error: 'isZero' is not static

This form of binding is included in the |LANG| language specifically to
simplify the migration from the languages that support access to sub-entities
as simple names. This feature is to be used only for migration.

.. index::
   binding
   migration
   access
   sub-entity
   simple name

|

.. _Import and Overloading of Function Names:

Import and Overloading of Function Names
========================================

.. meta:
    frontend_status: Done

While importing functions, the following situations can occur:

-  Different imported functions have the same name but different signatures,
   or a function (functions) of the current module and an imported function
   (functions) have the same name but different signatures. That situation
   is called **overloading**.

-  A function (functions) of the current module and an imported function
   (functions) have the same name and signature. That situation is called
   **shadowing**.

.. index::
   import
   overloading
   function name
   function
   imported function
   signature
   module
   shadowing

|

.. _Overloading of Function Names:

Overloading of Function Names
=============================

.. meta:
    frontend_status: Done

**Overloading** is the situation when a compilation unit has access to several
functions with the same names (regardless of where such functions are declared).
The code can use all such functions if they have distinguishable signatures
(i.e., the functions are not override-equivalent):

.. code-block:: typescript
   :linenos:

      package P1
      function foo(p: int) {}

      package P2
      function foo(p: string) {}

      // Main module
      import * from "path_to_file_with_P1"
      import * from "path_to_file_with_P2"
      function foo (p: double) {}
      function main() {
        foo(5) // Call to P1.foo(int)
        foo("A string") // Call to P2.foo(string)
        foo(3.141592653589) // Call to local foo(double)
      }

.. index::
   overloading
   access
   function
   signature

|

.. _Shadowing of Function Names:

Shadowing of Function Names
===========================

.. meta:
    frontend_status: Done

**Shadowing** is the :index:`compile-time error` that occurs if an imported
function is identical to the function declared in the current compilation
unit (the same names and override-equivalent signatures), i.e., the
declarations are duplicated.

Qualified import or alias in import can be used to access the imported entity.

.. code-block:: typescript
   :linenos:

      package P1
         function foo() {}
      package P2
         function foo() {}
      // Main program
      import * from "path_to_file_with_P1"
      import * from "path_to_file_with_P2" /* Error: duplicating
          declarations imported*/
      function foo() {} /* Error: duplicating declaration identified
          */
      function main() {
        foo() // Error: ambiguous function call
        // But not a call to local foo()
        // foo() from P1 and foo() from P2 are not accessible
      }

.. index::
   shadowing
   function name
   imported function
   compilation unit
   override-equivalent signature
   qualified import
   alias
   import
   access
   imported entity

|

.. _Generics Declaration-Site Variance:

Generics: Declaration-Site Variance
***********************************

Optionally, a type parameter can have keywords ``in`` or ``out`` (a
*variance modifier*, which specifies the variance of the type parameter).

**NOTE**: This description of variance modifiers is preliminary. The details
are to be specified in the future versions of the |LANG| language.

Type parameters with the keyword ``out`` are *covariant*, and can be used in
the out-position only.

Type parameters with the keyword ``in`` are *contravariant*, and can be used
in the in-position only.

Type parameters with no variance modifier are implicitly *invariant*, and can
occur in any position.

.. index::
   generic
   declaration-site variance
   type parameter
   keyword in
   keyword out
   variance modifier
   variance modifier
   in-position
   out-position

A :index:`compile-time error` occurs if a function, method, or constructor
type parameters have a variance modifier specified.

*Variance* is used to describe the subtyping (see :ref:`Subtyping`) operation
on parameterized types (see :ref:`Generic Declarations`). The
variance of the corresponding type parameter *F* defines the subtyping between
*T<A>* and *T<B>* (in the case of declaration-site variance with two different
types *A* <: *B*) as follows:

-  Covariant (*out F*): *T<A>* <: *T<B>*;
-  Contravariant (*in F*): *T<A>* :> *T<B>*;
-  Invariant (default) (*F*).

.. index::
   type parameter
   variance modifier
   function
   method
   constructor
   variance
   covariance
   contravariance
   invariance
   type-parameterized declaration
   parameterized type
   subtyping
   declaration-site variance

.. raw:: pdf

   PageBreak


