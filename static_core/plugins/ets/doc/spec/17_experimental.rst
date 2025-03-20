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

.. _Experimental Features:

Experimental Features
#####################

.. meta:
    frontend_status: Partly

This Chapter introduces the |LANG| features that are considered parts of
the language, but have no counterpart in |TS|, and are therefore not
recommended to those who seek a single source code for |TS| and |LANG|.

Some features introduced in this Chapter are still under discussion. They can
be removed from the final version of the |LANG| specification. Once a feature
introduced in this Chapter is approved and/or implemented, the corresponding
section is moved to the body of the specification as appropriate.

The *array creation* feature introduced in :ref:`Array Creation Expressions`
enables users to dynamically create objects of array type by using runtime
expressions that provide the array size. This addition is useful to other
array-related features of the language, such as array literals.

The construct can also be used to create multidimensional arrays.

The feature *function and method overloading* is supported in many
(if not all) modern programming languages. Overloading functions/methods
is a practical and convenient way to write program actions that are similar
in logic but different in implementation. It is discussed in detail in section
:ref:`Function, Method and Constructor Overloading`.

.. index::
   implementation
   array creation
   runtime expression
   array
   array literal
   construct

Section :ref:`Native Functions and Methods` introduces practically important
and useful mechanisms for the inclusion of components written in other languages
into a program written in |LANG|.

Section :ref:`Final Classes and Methods` discusses the well-known feature that
in many OOP languages provides a way to restrict class inheritance and method
overriding. Making a class *final* prohibits defining classes derived from it,
whereas making a method *final* prevents it from overriding in derived classes.

Section :ref:`Adding Functionality to Existing Types` discusses the way to
add new functionality to an already defined type. This feature can be used
for GUI programming (:ref:`Support for GUI Programming`).

Section :ref:`Enumeration Methods` adds methods to declarations of the
enumeration types. Such methods can help in some kinds of manipulations
with ``enums``.

.. index::
   implementation
   function overloading
   method overloading
   final class
   method final
   OOP (object-oriented programming)
   inheritance
   enum
   class
   interface
   inheritance
   derived class

The |LANG| language supports writing concurrent applications in the form of
*coroutines* (see :ref:`Coroutines`) that allow executing functions
concurrently.

There is a basic set of language constructs that support concurrency. A function
to be launched asynchronously is marked by adding the modifier ``async``
to its declaration. In addition, any function---or lambda expression---can be
launched as a separate thread explicitly by using the launch function from
the standard library.

.. index::
   construct
   coroutine
   channel
   function
   async modifier
   lambda expression
   concurrency
   async modifier

The ``await`` statement is introduced to await Promise resolving.
The generic class ``Promise<T>`` from the standard library (see
:ref:`Standard Library`) is used to access result of Promise resolving or
``async`` function execution.

Section :ref:`Packages` discusses a well-known and proven language feature
intended to organize large pieces of software that typically consist of many
components. *Packages* allow developers to construct a software product
as a composition of subsystems, and organize the development process in a way
that is appropriate for independent teams to work in parallel.

.. index::
   await statement
   function
   generic class
   standard library
   implementation
   channel
   package
   construct

*Package* is the language construct that combines a number of declarations,
and makes them parts of an independent compilation unit.

The *export* and *import* features are used to organize communication between
*packages*. An entity exported from one package becomes known to and
accessible (see :ref:`Accessible`) in another package which imports that
feature. Various options are provided to simplify export/import, e.g., by
defining non-exported, i.e., ``internal`` declarations that are not accessible
(see :ref:`Accessible`) from the outside of the package.

In addition, |LANG| supports the *package* initialization semantics that
makes a *package* even more independent from its environment.

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
   semantics

|

.. _Character Type and Literals:

Character Type and Literals
***************************

|

.. _Character Literals:

Character Literals
==================

.. meta:
    frontend_status: Done

*Character literal* represents the following:

-  Value consisting of a single character; or
-  Single escape sequence preceded by the characters *single quote* (U+0027)
   and '*c*' (U+0063), and followed by a *single quote* U+0027).

.. code-block:: abnf

      CharLiteral:
          'c\'' SingleQuoteCharacter '\''
          ;

      SingleQuoteCharacter:
          ~['\\\r\n]
          | '\\' EscapeSequence
          ;

The examples are presented below:

.. code-block:: typescript
   :linenos:

      c'a'
      c'\n'
      c'\x7F'
      c'\u0000'

*Character literals* are of type ``char``.

.. index::
   char literal
   character literal
   escape sequence
   single quote
   type char

|

.. _Character Type and Operations:

Character Type and Operations
=============================

.. meta:
    frontend_status: Partly
    todo: need to adapt the implementation to the latest specification

+-----------+----------------------------------+------------------------------+
| **Type**  | **Type's Set of Values**         | **Corresponding Class Type** |
+===========+==================================+==============================+
| ``char``  | Symbols with codes from \U+0000  | *Char*                       |
| (32-bits) | to \U+10FFFF (maximum valid      |                              |
|           | unicode code point) inclusive    |                              |
+-----------+----------------------------------+------------------------------+

|LANG| provides a number of operators to act on character values as discussed
below.

All character operators are identical to integer operators (see
:ref:`Integer Types and Operations`) for they handle character values as
integers of type ``int`` (see :ref:`Widening Primitive Conversions`).

The class ``Char`` provides constructors, methods, and constants that are
parts of the |LANG| standard library (see :ref:`Standard Library`).

.. index::
   char
   Char
   boolean
   equality operator
   constructor
   method
   constant

|

.. _Fixed Array Types:

Fixed Array Types
*****************

.. meta:
    frontend_status: Partly

*Fixed array type* is the built-in type characterized by the following:

-  Any object of array type contains elements, and the number of such elements
   is known as *array length*.
-  Array length is a non-negative integer number.
-  Array length is set once at runtime and can not be changed after that.
-  Array element is accessed by its index. *Index* is an integer number
   starting from *0* to *array length minus 1*.
-  Accessing an element by its index is a constant-time operation.
-  If passed to non-|LANG| environment, an array is represented as a contiguous
   memory location.
-  Type of each array element is assignable to the element's type specified
   in the array declaration (see :ref:`Assignability`).

*Fixed arrays* differ from *resizable arrays* as follows:

- *Fixed array* length is set once to achieve better performance;
- *Fixed arrays* have no methods defined;
- *Fixed arrays* are not compatible with *resizable arrays*.

*Fixed array* can be created by using :ref:`Array Literal`.

The examples are presented below:

.. code-block:: typescript
   :linenos:

    let a : FixedArray<number> = [1, 2, 3]
      /* allocate array with 3 elements of type number */
    a[1] = 7 /* put 7 as the 2nd element of the array, index of this element is 1 */
    let y = a[2] /* get the last element of array 'a' */
    let count = a.length // get the number of array elements
    y = a[3] // Will lead to runtime error - attempt to access non-existing array element

.. index::
   fixed array type
   fixed array
   integer
   array element
   element type
   access


TBD: example for incompatibility between array and fixed array.

|

.. _Array Creation Expressions:

Array Creation Expressions
**************************

.. meta:
    frontend_status: Done

*Array creation expression* creates new objects that are instances of arrays.
The *array literal* expression is used to create an array instance, and to
provide initial values (see :ref:`Array Literal`).

.. code-block:: abnf

      newArrayInstance:
          'new' arrayElementType dimensionExpression+ (arrayElement)?
          ;

      arrayElementType:
          typeReference
          | '(' type ')'
          ;

      dimensionExpression:
          '[' expression ']'
          ;

      arrayElement:
          '(' expression ')'
          ;

.. code-block:: typescript
   :linenos:

      let x = new number[2][2] // create 2x2 matrix

*Array creation expression* creates an object that is a new array with the
elements of the type specified by ``arrayElelementType``.

The type of each *dimensionExpression* must be convertible (see
:ref:`Primitive Types Conversions`) to an integer type. Otherwise,
a :index:`compile-time error` occurs.

A numeric conversion (see :ref:`Primitive Types Conversions`) is performed
on each *dimensionExpression* to ensure that the resultant type is ``int``.
Otherwise, a :index:`compile-time error` occurs.

A :index:`compile-time error` occurs if any *dimensionExpression* is a
constant expression that is evaluated to a negative integer value at compile
time.

.. index::
   array creation expression
   conversion
   integer
   type
   value
   numeric conversion
   type int
   constant expression

If the type of any *dimensionExpression* is ``number`` or other floating-point
type, and its fractional part is different from '0', then errors occur as
follows:

- Compile-time error, if the situation is identified during compilation; and
- Runtime error, if the situation is identified during program execution.

If ``arrayElement`` is provided, then the type of the ``expression`` can be
as follows:

- Type of array element denoted by ``arrayElelementType``, or
- Lambda function with the return type equal to the type of array element
  denoted by ``arrayElelementType`` and the parameters of type ``int``, and the
  number of parameters equal to the number of array dimensions.

.. index::
   type
   floating-point type
   runtime error
   compilation
   expression
   lambda function
   array
   parameter
   array

Otherwise, a :index:`compile-time error` occurs.

.. code-block:: typescript
   :linenos:

      let x = new number[-3] // compile-time error

      let y = new number[3.141592653589]  // compile-time error

      foo (3.141592653589)
      function foo (size: number) {
         let y = new number[size]  // runtime error
      }

A :index:`compile-time error` occurs if ``arrayElelementType`` refers to a
class that does not contain an accessible (see :ref:`Accessible`) parameterless
constructor, or constructor with all parameters of the second form of optional
parameters (see :ref:`Optional Parameters`), or if ``type`` has no default
value:

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

      let x = new string[3] // compile-time error: string has no default value

      class A {
         constructor (p1?: number, p2?: string) {}
      }
      let y = new A[2] // OK, as all 3 elements of array will be filled with
      // new A() objects

A :index:`compile-time error` occurs if ``arrayElelementType`` is a type
parameter:

.. code-block:: typescript
   :linenos:

      class A<T> {
         foo() {
            new T[2] // compile-time error: cannot create an array of type parameter elements
         }
      }

.. index::
   accessibility
   constructor
   array

The creation of an array with a known number of elements is presented below:

.. code-block:: typescript
   :linenos:

      class A {}
         // It has no default value or parameterless constructor defined

      let array_size = 5

      let array1 = new A[array_size] (new A)
         /* Create array of 'array_size' elements and all of them will have
            initial value equal to an object created by new A expression */

      let array2 = new A[array_size] ((index): A => { return new A })
         /* Create array of `array_size` elements and all of them will have
            initial value equal to the result of lambda function execution with
            different indices */

      let array2 = new A[2][3] ((index1, index2): A => { return new A })
         /* Create two-dimensional array of 6 elements total and all of them will
            have initial value equal to the result of lambda function execution with
            different indices */

The creation of exotic arrays with different kinds of element types is presented
below:

.. code-block:: typescript
   :linenos:

      let array_of_union = new (Object|null) [5] // filled with null
      let array_of_functor = new (() => void) [5] ( (): void => {})
      type aliasTypeName = number []
      let array_of_array = new aliasTypeName [5] ( [3.141592653589] )

The creation of fixed arrays is presented below:

.. code-block:: typescript
   :linenos:

      let fixed_array : FixedArray<number> = new number[5]

|

.. _Runtime Evaluation of Array Creation Expressions:

Runtime Evaluation of Array Creation Expressions
================================================

.. meta:
    frontend_status: Partly
    todo: initialize array elements properly - #14963, #15610

The evaluation of an array creation expression at runtime is performed
as follows:

#. The dimension expressions are evaluated. The evaluation is performed
   left-to-right. If any expression evaluation completes abruptly, then
   the expressions to the right of it are not evaluated.

#. The values of dimension expressions are checked. If the value of any
   ``dimExpr`` expression is less than zero, then ``NegativeArraySizeError`` is
   thrown.

#. Space for the new array is allocated. If the available space is not
   sufficient to allocate the array, then ``OutOfMemoryError`` is thrown,
   and the evaluation of the array creation expression completes abruptly.

#. When a one-dimensional array is created, each element of that array
   is initialized to its default value if type default value is defined
   (:ref:`Default Values for Types`).
   If the default value for an element type is not defined, but the element
   type is a class type, then its *parameterless* constructor is used to
   create the value of each element.

#. When a multidimensional array is created, the array creation effectively
   executes a set of nested loops of depth *n-1*, and creates an implied
   array of arrays.

.. index::
   array
   constructor
   abrupt completion
   expression
   runtime
   evaluation
   default value
   parameterless constructor
   class type
   initialization
   nested loop

|

.. _Indexable Types:

Indexable Types
***************

.. meta:
    frontend_status: Done

If a class or an interface declares one or two functions with names ``$_get``
and ``$_set``, and signatures *(index: Type1): Type2* and *(index: Type1,
value: Type2)* respectively, then an indexing expression (see
:ref:`Indexing Expressions`) can be applied to variables of such types:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class SomeClass {
       $_get (index: number): SomeClass { return this }
       $_set (index: number, value: SomeClass) { }
    }
    let x = new SomeClass
    x = x[1] // This notation implies a call: x = x.$_get (1)
    x[1] = x // This notation implies a call: x.$_set (1, x)

If only one function is present, then only the appropriate form of index
expression (see :ref:`Indexing Expressions`) is available:

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    class ClassWithGet {
       $_get (index: number): ClassWithGet { return this }
    }
    let getClass = new ClassWithGet
    getClass = getClass[0]
    getClass[0] = getClass // Error - no $_set function available

    class ClassWithSet {
       $_set (index: number, value: ClassWithSet) { }
    }
    let setClass = new ClassWithSet
    setClass = setClass[0] // Error - no $_get function available
    setClass[0] = setClass

.. index::
   function
   signature
   indexing expression
   variable
   index expression
   string

Type ``string`` can be used as a type of the index parameter:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class SomeClass {
       $_get (index: string): SomeClass { return this }
       $_set (index: string, value: SomeClass) { }
    }
    let x = new SomeClass
    x = x["index string"]
       // This notation implies a call: x = x.$_get ("index string")
    x["index string"] = x
       // This notation implies a call: x.$_set ("index string", x)

Functions ``$_get`` and ``$_set`` are ordinary functions with compiler-known
signatures. The functions can be used like any other function.
The functions can be abstract, or defined in an interface and implemented later.
The functions can be overridden and provide a dynamic dispatch for the indexing
expression evaluation (see :ref:`Indexing Expressions`). The functions can be
used in generic classes and interfaces for better flexibility. A
:index:`compile-time error` occurs if these functions are marked as ``async``.

.. index::
   index parameter
   function
   compiler
   signature
   overriding
   interface
   implementation
   indexing expression
   evaluation
   flexibility
   async function
   generic class

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    interface ReadonlyIndexable<K, V> {
       $_get (index: K): V
    }

    interface Indexable<K, V> extends ReadonlyIndexable<K, V> {
       $_set (index: K, value: V)
    }

    class IndexableByNumber<V> extends Indexable<number, V> {
       private data: V[] = []
       $_get (index: number): V { return this.data [index] }
       $_set (index: number, value: V) { this.data[index] = value }
    }

    class IndexableByString<V> extends Indexable<string, V> {
       private data = new Map<string, V>
       $_get (index: string): V { return this.data [index] }
       $_set (index: string, value: V) { this.data[index] = value }
    }

    class BadClass extends IndexableByNumber<boolean> {
       override $_set (index: number, value: boolean) { index / 0 }
    }

    let x: IndexableByNumber<boolean> = new BadClass
    x[42] = true // This will be dispatched at runtime to the overridden
       // version of the $_set method
    x.$_get (15)  // $_get and $_set can be called as ordinary
       // methods

|

.. _Iterable Types:

Iterable Types
**************

.. meta:
    frontend_status: Done

A class or an interface can be made *iterable*. It means that instances of the
class or the interface can be used in ``for-of`` statements (see
:ref:`For-Of Statements`).

Some type ``C`` is *iterable* if it declares a parameterless function with name
``$_iterator`` and return type that is assignable (see :ref:`Assignability`)
to type ``Iterator`` as defined in the standard library (see
:ref:`Standard Library`). It guarantees that the object returned is of the
class type which implements ``Iterator`` and allows traversing an object of
class type ``C``. The example below defines *iterable* class ``C``:

.. index::
   iterable type
   class
   interface
   instance
   for-of statement
   parameterless function
   compatibility
   iterable class

.. code-block:: typescript
   :linenos:

      class C {
        data: string[] = ['a', 'b', 'c']
        $_iterator() { // Function type is inferred from its body
          return new CIterator(this)
        }
      }

      class CIterator implements Iterator<string> {
        index = 0
        base: C
        constructor (base: C) {
          this.base = base
        }
        next(): IteratorResult<string> {
          return {
            done: this.index >= this.base.data.length,
            value: this.index >= this.base.data.length ? undefined : this.base.data[this.index++]
          }
        }
      }

      let c = new C()
      for (let x of c) {
            console.log(x)
      }

In the example above, class ``C`` function ``$_iterator`` returns
``CIterator<string>`` that implements ``Iterator<string>``. If executed,
this code prints out the following:

.. code-block:: typescript

    "a"
    "b"
    "c"

The function ``$_iterator`` is an ordinary function with a compiler-known
signature. The function can be used like any other function. It can be
abstract or defined in an interface to be implemented later. A
:index:`compile-time error` occurs if this function is marked as ``async``.

.. index::
   function
   class
   string
   iterator
   compiler
   signature
   async function

**Note**. To support the code compatible with |TS|, the name of the function
``$_iterator`` can be written as ``[Symbol.iterator]``. In this case, the class
``iterable`` looks as follows:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

      class C {
        data: string[] = ['a', 'b', 'c'];
        [Symbol.iterator]() {
          return new CIterator(this)
        }
      }

The use of the name ``[Symbol.iterator]`` is considered deprecated.
It can be removed in the future versions of the language.

.. index::
   compatibility
   function
   iterator
   class

|

.. _Callable Types:

Callable Types
**************

.. meta:
    frontend_status: Partly
    todo: add $_ to names

A type is *callable* if the name of the type can be used in a call expression.
A call expression that uses the name of a type is called a *type call
expression*. Only class type can be callable. To make a type
callable, a static method with the name ``$_invoke`` or ``$_instantiate`` must
be defined or inherited:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class C {
        static $_invoke() { console.log("invoked") }
    }
    C() // prints: invoked
    C.$_invoke() // also prints: invoked

In the above example, ``C()`` is a *type call expression*. It is the short
form of the normal method call ``C.$_invoke()``. Using an explicit call is
always valid for the methods ``$_invoke`` and ``$_instantiate``.

.. index::
   callable type
   call expression
   expression
   type call expression
   class type
   method call
   instantiation

**Note**. Only a constructor---not the methods ``$_invoke`` or
``$_instantiate``---is called in a *new expression*:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class C {
        static $_invoke() { console.log("invoked") }
        constructor() { console.log("constructed") }
    }
    let x = new C() // constructor is called

The methods ``$_invoke`` and ``$_instantiate`` are similar but have differences as
discussed below.

A :index:`compile-time error` occurs if a callable type contains both methods
``invoke`` and ``$_instantiate``.

.. index::
   method
   instantiation

|

.. _Callable Types with $_invoke Method:

Callable Types with ``$_invoke`` Method
=======================================

.. meta:
    frontend_status: Done

The method ``$_invoke`` can have an arbitrary signature. The method can be used
in a *type call expression* in either case above. If the signature has
parameters, then the call must contain corresponding arguments.

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class Add {
        static $_invoke(a: number, b: number): number {
            return a + b
        }
    }
    console.log(Add(2, 2)) // prints: 4

.. index::
   callable type
   signature
   method
   type call expression
   argument

|

.. _Callable Types with $_instantiate Method:

Callable Types with ``$_instantiate`` Method
============================================

.. meta:
    frontend_status: Done

The method ``$_instantiate`` can have an arbitrary signature by itself.
If it is to be used in a *type call expression*, then its first parameter
must be a ``factory`` (i.e., it must be a *parameterless function type
returning some class type*).
The method can have or not have other parameters, and those parameters can
be arbitrary.

In a *type call expression*, the argument corresponding to the ``factory``
parameter is passed implicitly:

.. code-block:: typescript
   :linenos:

    class C {
        static $_instantiate(factory: () => C): C {
            return factory()
        }
    }
    let x = C() // factory is passed implicitly

    // Explicit call of '$_instantiate' requires explicit 'factory':
    let y = C.$_instantiate(() => { return new C()})

.. index::
   method
   signature
   type call expression
   factory
   parameterless function type
   struct type

If the method ``$_instantiate`` has additional parameters, then the call must
contain corresponding arguments:

.. code-block:: typescript
   :linenos:

    class C {
        name = ""
        static $_instantiate(factory: () => C, name: string): C {
            let x = factory()
            x.name = name
            return x
        }
    }
    let x = C("Bob") // factory is passed implicitly

A :index:`compile-time error` occurs in a *type call expression* with type ``T``,
if:

- ``T`` has neither method ``$_invoke`` nor  method ``$_instantiate``; or
- ``T`` has the method ``$_instantiate`` but its first parameter is not
  a ``factory``.

.. index::
   type call expression
   instantiation
   method
   parameter

.. code-block-meta:
    expect-cte

.. code-block:: typescript
   :linenos:

    class C {
        static $_instantiate(factory: string): C {
            return factory()
        }
    }
    let x = C() // compile-time error, wrong '$_instantiate' 1st parameter

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

An explicit type annotation is allowed for a *for-variable*:

.. code-block:: typescript
   :linenos:

      // explicit type is used for a new variable,
      let x: number[] = [1, 2, 3]
      for (let n: number of x) {
        console.log(n)
      }

.. index::
   type annotation
   for-variable
   for-of type annotation

|

.. _Function, Method and Constructor Overloading:

Function, Method and Constructor Overloading
********************************************

.. meta:
    frontend_status: Done

As many other languages do, |LANG| supports overloading. Overloading allows
declaring several functions or methods with the same name but different
signatures.
|LANG| does not support |TS| overload signatures that allow
specifying several headers for a function or method with a single body but
different signatures (see :ref:`TS Overload Signatures`).

The |LANG| approach delivers better performance because no extra checks
are performed during the execution of a specific body at runtime.

.. index::
   function overloading
   method overloading
   header
   function
   method
   signature
   overloaded header
   runtime

|

.. _Function Overloading:

Function Overloading
====================

.. meta:
    frontend_status: Done

If a declaration scope declares and/or imports two or more functions with the
same name but different signatures that are not *overload-equivalent* (see
:ref:`Overload-Equivalent Signatures`), then such functions are *overloaded*.
Function overload declarations cause no :index:`compile-time error` on their
own.

No specific relationship is required between the return types of the two
functions with the same name but different signatures that are not
*overload-equivalent* (see :ref:`Overload-Equivalent Signatures`).

When calling an overloaded function, the number of actual arguments (and any
explicit type arguments) and compile-time argument types are used at compile
time to determine exactly which one is to be called (see
:ref:`Function Call Expression`).

.. index::
   function overloading
   declaration scope
   signature
   name
   overload-equivalence
   overload-equivalent signature
   overloaded function name
   return type
   argument
   type argument
   function call
   compile-time error

|

.. _Class Method Overloading:

Class Method Overloading
========================

.. meta:
    frontend_status: Done

If two or more methods within a class have the same name, and their signatures
are not *overload-equivalent* (see :ref:`Overload-Equivalent Signatures`), then
such methods are considered *overloaded*.

Method overloading declarations cause no :index:`compile-time error` on their
own, except where possible instantiation causes an *overload-equivalent* (see
:ref:`Overload-Equivalent Signatures`) method in the instantiated class or
interface:

.. index::
   class method overloading
   class
   signature
   overload-equivalent signature
   overload equivalence
   overloading
   method
   instantiation
   interface

.. code-block:: typescript
   :linenos:

     class Template<T> {
        foo (p: number) { ... }
        foo (p: T) { ... }
     }
     let instantiation: Template<number>
       // Leads to two *overload-equivalent* methods

     interface ITemplate<T> {
        foo (p: number)
        foo (p: T)
     }
     function foo (instantiation: ITemplate<number>) { ... }
       // Leads to two *overload-equivalent* methods

If signatures of two or more methods with the same name are not
*overload-equivalent* (see :ref:`Overload-Equivalent Signatures`), then return
types of those methods can have any kind of relationship.

When calling an overloaded method, the number of actual arguments (and any
explicit type arguments) and compile-time argument types are used at compile
time to determine exactly which one is to be called (see
:ref:`Method Call Expression` and :ref:`Step 2 Selection of Method`).

.. index::
   signature
   overload-equivalent signature
   overload equivalence
   type argument
   argument type
   method call
   instance method

|

.. _Constructor Overloading:

Constructor Overloading
=======================

.. meta:
    frontend_status: Done

Constructor overloading behavior is identical to that of method overloading (see
:ref:`Class Method Overloading`). Each class instance creation expression (see
:ref:`New Expressions`) resolves the constructor overloading call if any at
compile time.

.. index::
   constructor overloading
   method overloading
   class instance
   creation expression
   compile time

|

.. _Declaration Distinguishable by Signatures:

Declaration Distinguishable by Signatures
=========================================

.. meta:
    frontend_status: Done

Same-name declarations are distinguishable by signatures if such
declarations are one of the following:

-  Functions with the same name and signatures that are not
   *overload-equivalent* (see :ref:`Overload-Equivalent Signatures` and
   :ref:`Function Overloading`).

-  Methods with the same name and signatures that are not
   *overload-equivalent* (see :ref:`Overload-Equivalent Signatures`,
   :ref:`Class Method Overloading`, and :ref:`Interface Method Overloading`).

-  Constructors of the same class and signatures that are not
   *overload-equivalent* (see :ref:`Overload-Equivalent Signatures` and
   :ref:`Constructor Overloading`).

.. index::
   distinguishable declaration
   signature
   function
   overloading
   overload-equivalent signature
   overload-equivalence
   constructor

The example below represents the functions distinguishable by signatures:

.. code-block:: typescript
   :linenos:

      function foo() {}
      function foo(x: number) {}
      function foo(x: number[]) {}
      function foo(x: string) {}

The following example represents the functions indistinguishable by signatures
that cause a :index:`compile-time error`:

.. code-block:: typescript
   :linenos:

      // Functions have overload-equivalent signatures
      function foo(x: number) {}
      function foo(y: number) {}

      // Functions have overload-equivalent signatures
      function foo(x: number) {}
      type MyNumber = number
      function foo(x: MyNumber) {}

.. index::
   distinguishable function
   function
   signature

|

.. _TS Overload Signatures:

|TS| Overload Signatures
========================

.. meta:
    frontend_status: None

|LANG| does not support overload signatures in |TS|-style where several function
headers are followed by a single body. Each overloaded function, method, or
constructor is required to have a separate body.

The following code is valid in |TS| but causes a compile-time error in |LANG|:

.. code-block-meta:
   expect-cte

.. code-block:: typescript
   :linenos:

    function foo(): void 
    function foo(x: string): void
    function foo(x?: string): void {
        /*body*/
    }

The following code is valid in |LANG|
(see :ref:`Function, Method and Constructor Overloading`):

.. code-block-meta:
   not-subset

.. code-block:: typescript
   :linenos:

    function foo(): void {
      /*body1*/
    }
    function foo(x: string): void {
      /*body2*/
    }

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

*Native function* is a function marked with the keyword ``native`` (see
:ref:`Function Declarations`).

*Native function* implemented in a platform-dependent code is typically written
in another programming language (e.g., *C*). A :index:`compile-time error`
occurs if a native function has a body.

.. index::
   keyword native
   function
   native function
   implementation
   platform-dependent code
   compile-time error
   function body

|

.. _Native Methods Experimental:

Native Methods
==============

.. meta:
    frontend_status: Done

*Native method* is a method marked with the keyword ``native`` (see
:ref:`Method Declarations`).

*Native methods* are the methods implemented in a platform-dependent code
written in another programming language (e.g., *C*).

A :index:`compile-time error` occurs if:

-  Method declaration contains the keyword ``abstract`` along with the
   keyword ``native``.

-  *Native method* has a body (see :ref:`Method Body`) that is a block
   instead of a simple semicolon or empty body.

-  *Native method* is defined in a local class (see
   :ref:`Local Classes and Interfaces`).

.. index::
   native method
   implementation
   platform-dependent code
   keyword native
   method body
   block
   method declaration
   keyword abstract
   semicolon
   empty body

|

.. _Native Constructors:

Native Constructors
===================

.. meta:
    frontend_status: Done

*Native constructor* is a constructor marked with the keyword ``native`` (see
:ref:`Constructor Declaration`).

*Native constructors* are the constructors implemented in a platform-dependent
code written in another programming language (e.g., *C*).

A :index:`compile-time error` occurs if a *native constructor*:

-  Has a non-empty body (see :ref:`Constructor Body`).

-  Is defined in a local class (see :ref:`Local Classes and Interfaces`).

.. index::
   native constructor
   platform-dependent code
   keyword native
   non-empty body

|

.. _Final Classes and Methods:

Final Classes and Methods
*************************

.. meta:
    frontend_status: Done

.. _Final Classes Experimental:

Final Classes
=============

.. meta:
    frontend_status: Done

A class can be declared ``final`` to prevent extension, i.e., a class declared
``final`` cannot have subclasses. No method of a ``final`` class can be
overridden.

If a class type ``F`` expression is declared *final*, then only a class ``F``
object can be its value.

A :index:`compile-time error` occurs if the ``extends`` clause of a class
declaration contains another class that is ``final``.

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

A method can be declared ``final`` to prevent it from being overridden (see
:ref:`Overloading and Overriding`) in subclasses.

A :index:`compile-time error` occurs if:

-  The method declaration contains the keyword ``abstract`` or ``static``
   along with the keyword ``final``.

-  A method declared ``final`` is overridden.

.. index::
   final method
   overriding
   instance method
   subclass
   method declaration
   keyword abstract
   keyword static
   keyword final

|

.. _Default Interface Method Declarations:

Default Interface Method Declarations
*************************************

.. meta:
    frontend_status: Done

.. code-block:: abnf

    interfaceDefaultMethodDeclaration:
        'private'? identifier signature block
        ;

A default method can be explicitly declared ``private`` in an interface body.

A block of code that represents the body of a default method in an interface
provides a default implementation for any class if such class does not override
the method that implements the interface.

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

.. _Adding Functionality to Existing Types:

Adding Functionality to Existing Types
**************************************

.. meta:
    frontend_status: Done

|LANG| supports adding functions and accessors to already defined types. The
usage of functions so added looks the same as if they are methods and accessors
of these types. The mechanism is called :ref:`Functions with Receiver`
and :ref:`Accessors with Receiver`. This feature is often used to add new
functionality to a class or interface without having to inherit from this class
or implement this interface. However, it can be used not only for classes and
interfaces but also for other types.

Moreover, :ref:`Function Types with Receiver` and
:ref:`Lambda Expressions with Receiver` can be defined and used to make the
code more flexible.

|

.. _Functions with Receiver:

Functions with Receiver
=======================

.. meta:
    frontend_status: Done

*Function with receiver* declaration is a top-level declaration
(see :ref:`Top-Level Declarations`) that looks almost the same as
:ref:`Function Declarations`, except that the first parameter is mandatory,
and the keyword ``this`` is used as its name:

.. code-block:: abnf

    functionWithReceiverDeclaration:
        'function' identifier typeParameters? signatureWithReceiver block
        ;

    signatureWithReceiver:
        '(' receiverParameter (', ' parameterList)? ')' returnType? throwMark?
        ;

    receiverParameter:
        annotationUsage? 'this' ':' type
        ;

A *function with receiver* can be called in the following two ways:

-  Making a function call (see :ref:`Function Call Expression`), and
   passing the first parameter in the usual way;

-  Making a method call (see :ref:`Method Call Expression`) with
   no argument provided for the first parameter, and using the
   ``objectReference`` before the function name as the first argument.

.. code-block:: typescript
   :linenos:

      class C {}

      function foo(this: C) {}
      function bar(this: C, n: number): void {}

      let c = new C()

      // as a function call:
      foo(c)
      bar(c, 1)

      // as a method call:
      c.foo()
      c.bar(1)

      interface D {}
      function foo1(this: D) {}
      function bar1(this: D, n: number): void {}

      function demo (d: D) {
         // as a function call:
         foo(d)
         bar(d, 1)

         // as a method call:
         d.foo()
         d.bar(1)
      }



The keyword ``this`` can be used inside a *function with receiver*. It
corresponds to the first parameter. The type of ``this`` parameter is
called the *receiver type* (see :ref:`Receiver Type`).

If the *receiver type* is a class or interface type, then ``private`` or
``protected`` members are not accessible (see :ref:`Accessible`) within the
body of a *function with receiver*. Only ``public`` and  ``internal`` members
can be accessed. The ``internal`` members can be accessed only when *function
with receiver* and class or interface type are declared in the same compilation
unit:

.. index::
   keyword this
   function with receiver
   receiver type
   public member
   private member
   protected member
   access

.. code-block:: typescript
   :linenos:

      class A {
          foo () { ... this.bar() ... }
                       // function bar() is accessible here
          protected member_1 ...
          private member_2 ...
      }
      function bar(this: A) { ...
         this.foo() // Method foo() is accessible as it is public
         this.member_1 // Compile-time error as member_1 is not accessible
         this.member_2 // Compile-time error as member_2 is not accessible
         ...
      }
      let a = new A()
      a.foo() // Ordinary class method is called
      a.bar() // Function with receiver is called

A :index:`compile-time error` occurs is the name of a *function with receiver*
is the same as the name of an accessible instance method or field
of the receiver type.
It means that a *function with receiver* cannot overload a method
defined for the receiver type.

.. code-block:: typescript
   :linenos:

      class A {
          foo () { ...  }
      }

      function foo(this: A) { ... } // Compile-time error

A function with reciever can overload a global function with
the same name. A compile-time error occurs if signatures of
a function with receiver is overload-equivalent
(see Overload-Equivalent Signatures) to such overloaded function.

.. code-block:: typescript
   :linenos:

      function foo(this: A) { ... }
      function foo(param: A) { ... } /* Compile-time error as it is a
                                        non-distinguishable declaration */

*Function with receiver* can be generic as in the following example:

.. index::
   function with receiver
   public method
   overload
   receiver type

.. code-block:: typescript
   :linenos:

     function foo<T>(this: B<T>, p: T) {
          console.log (p)
     }
     function demo (p1: B<SomeClass>, p2: B<BaseClass>) {
         p1.foo(new SomeClass())
           // Type inference should determine the instantiating type
         p2.foo<BaseClass>(new DerivedClass())
          // Explicit instantiation
     }

*Functions with receiver* are dispatched statically. What function is being
called is known at compile time based on the receiver type specified in the
declaration. A *function with receiver* can be applied to the receiver of any
derived class until it is redefined within the derived class:

.. code-block:: typescript
   :linenos:

      class Base { ... }
      class Derived extends Base { ... }

      function foo(this: Base) { console.log ("Base.foo is called") }
      function foo(this: Derived) { console.log ("Derived.foo is called") }

      let b: Base = new Base()
      b.foo() // `Base.foo is called` to be printed
      b = new Derived()
      b.foo() // `Base.foo is called` to be printed
      let d: Derived = new Derived()
      d.foo() // `Derived.foo is called` to be printed

As illustrated by the following examples, a *function with receiver* can be
defined in a compilation unit other than the one that defines the receiver type:

.. code-block:: typescript
   :linenos:

      // file a.ets
      class A {
          foo() { ... }
      }

      // file ext.ets
      import {A} from "a.ets" // name 'A' is imported
      function bar(this: A) () {
         this.foo() // Method foo() is called
      }

|

.. _Receiver Type:

Receiver Type
=============

.. meta:
    frontend_status: Done

*Receiver type* is the type of the *receiver parameter* in a function,
function type, and lambda with receiver. A *receiver type* may be an interface
type, a class type, an array type, or a type parameter. Otherwise, a
:index:`compile-time error` occurs.

Using an array type as *receiver type* is illustrated by the example below:

.. code-block:: typescript
   :linenos:

      function addElements(this: number[], ...s: number[]) {
       ...
      }

      let x: number[] = [1, 2]
      x.addElements(3, 4)

|

.. _Accessors with Receiver:

Accessors with Receiver
=======================

.. meta:
    frontend_status: Done

*Accessor with receiver* declaration is a top-level declaration (see
:ref:`Top-Level Declarations`) that can be used as class or interface accessor
(see :ref:`Accessor Declarations`) for a specified receiver type:

.. code-block:: abnf

    accessorWithReceiverDeclaration:
          'get' identifier '(' receiverParameter ')' returnType block
        | 'set' identifier '(' receiverParameter ',' parameter ')' block
        ;

A get-accessor (getter) must have a single *receiver parameter* and an explicit
return type.

A set-accessor (setter) must have a *receiver parameter*, one other parameter,
and no return type.

The use of getters and setters looks the same as the use of fields:

.. code-block:: typescript
   :linenos:

      class Person {
        firstName: string
        lastName: string
        constructor (first: string, last: string) {...}
        ...
      }

      get fullName(this: C): string {
        return this.LastName + ' ' + this.FirstName
      }

      let c = new C("John", "Doe")

      // as a method call:
      console.log(c.fullName) // output: 'Doe John'
      c.fullName = "new name" // compile-time error, as setter is not defined

A :index:`compile-time error` occurs if an accessor is used in the form of
a function or a method call.

|

.. _Function Types with Receiver:

Function Types with Receiver
============================

.. meta:
    frontend_status: Done

*Function type with receiver* specifies the signature of a function or lambda
with receiver. It is almost the same as *function type* (see :ref:`Function Types`),
except that the first parameter is mandatory, and the keyword ``this`` is used
as its name:

.. code-block:: abnf

    functionTypeWithReceiver:
        '(' receiverParameter (',' ftParameterList)? ')' ftReturnType
        ;

The type of a *receiver parameter* is called the *receiver type* (see
:ref:`Receiver Type`).

.. code-block:: typescript
   :linenos:

      class A {...}

      type FA = (this: A) => boolean
      type FN = (this: number[], max: number) => number

*Function type with receiver* can be generic as in the following example:

.. code-block:: typescript
   :linenos:

      class B<T> {...}

      type FB<T> = (this: B<T>, x: T): void
      type FBS = (this: B<string>, x: string): void

The usual rule of function type compatibility (see :ref:`Function Types Conversions`)
is applied to *function type with receiver*, and parameter names are ignored.

.. code-block:: typescript
   :linenos:

      class A {...}

      type F1 = (this: A) => boolean
      type F2 = (a: A) => boolean

      function foo(this: A): boolean {}
      function goo(a: A): boolean {}

      let f1: F1 = foo // ok
      f1 = goo // ok

      let f2: F2 = goo // ok
      f2 = foo // ok
      f1 = f2 // ok

The sole difference is that only an entity of *function type with receiver* can
be used in :ref:`Method Call Expression`. The definitions from the previous
example are reused in the example below:

.. code-block:: typescript
   :linenos:

      let a = new A()
      a.f1() // ok, function type with receiver
      f1(a)  // ok

      a.f2() // compile-time error
      f2(a) // ok

|

.. _Lambda Expressions with Receiver:

Lambda Expressions with Receiver
================================

.. meta:
    frontend_status: Done

*Lambda expression with receiver* defines an instance of a *function type with
receiver* (see :ref:`Function Types with Receiver`). It looks almost the same
as an ordinary lambda expression (see :ref:`Lambda Expressions`), except that
the first parameter is mandatory, and the keyword ``this`` is used as its name:

.. code-block:: abnf

    lambdaExpressionWithReceiver:
        annotationUsage? typeParameters?
        '(' receiverParameter (',' lambdaParameterList)? ')'
        returnType? throwMark? '=>' lambdaBody
        ;

The usage of annotations is discussed in :ref:`Using Annotations`.

The keyword ``this`` can be used inside a *lambda expression with receiver*,
It corresponds to the first parameter:

.. code-block:: typescript
   :linenos:

      class A { name = "Bob" }

      let show = (this: A): void {
          console.log(this.name)
      }

The use of lambda is illustrated by the example below:

.. code-block:: typescript
   :linenos:

      class A {
        name: string
        constructor (n: string) {
            this.name = n
        }
      }

      function apply(aa: A[], f: (this: A) => void) {
        for (let a of aa) {
            a.f()
        }
      }

      let aa: A[] = [new A("aa"), new A("bb")]
      foo(aa, (this: A) => { console.log(this.name)} ) // output: "aa" "bb"


**Note**. If *lambda expression with receiver* is declared in a class or
interface, then the use of ``this`` in a lambda body always refers to the first
lambda parameter, but not to ``this`` of the surrounding class or interface.

.. code-block:: typescript
   :linenos:

      class B {
        foo() { console.log ("foo() from B is called") }
      }
      class A {
        foo() { console.log ("foo() from A is called") }
        bar() {
            let lambda = (this: B): void => { this.foo() }
            new B().lambda()
        }
      }
      new A().bar() // Output is 'foo() from B is called'

|

.. _Implicit this in Lambda with Receiver Body:

Implicit ``this`` in Lambda with Receiver Body
==============================================

.. meta:
    frontend_status: Done

Implicit ``this`` can be used in the body of *lambda expression with receiver*
when accessing the following:

- Instance methods, fields, and accessories of lambda receiver type (see
  :ref:`Receiver Type`); or
- Functions with receiver (see :ref:`Functions with Receiver`) of the same
  receiver type.

In other words, prefix ``this.`` in such cases can be omitted. This feature
is added to |LANG| to improve DSL support. It is illustrated by the following
examples:

.. code-block:: typescript
   :linenos:

     class C {
       name: string = ""
       foo(): void {}
     }

     function process(context: (this: C) => void) {}

     process(
        (this: C): void => {
            this.foo()   // ok - normal call
            foo()        // ok - implicit 'this'
            name = "Bob" // ok - implicit 'this'
        }
     )

The same applies if *lambda expression with receiver* is defined as
*trailing lambda* (see :ref:`Trailing Lambdas`). In this case, lambda signature
is inferred from the context:

.. code-block:: typescript
   :linenos:

     process() {
        this.foo() // ok - normal call
        foo()      // ok - implicit 'this'
     }

The example above represents the use of implicit ``this`` when calling a function
with receiver:

.. code-block:: typescript
   :linenos:

     function bar(this: C) {}
     function otherBar(this: OtherClass) {}

     process() {
        bar()      // ok -  implicit 'this'
        otherBar() // compile-time error, wrong type of implicit 'this'
     }

If a simple name used in a lambda body can be resolved as instance method,
field or accessor of the receiver type, and as another entity in the current
scope at the same time, then a :index:`compile-time error` occurs to prevent
ambiguity and improve readability.

|

.. _Trailing Lambdas:

Trailing Lambdas
****************

.. meta:
    frontend_status: Done

The *trailing lambda* mechanism allows using a special form of function
or method call when the last parameter of a function or a method is of
function type, and the argument is passed as a lambda using the ``{}``
notation. The *trailing lambda* syntactically looks as follows:

.. index::
   trailing lambda
   function call
   method call
   parameter
   function type
   method
   parameter
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

.. code-block:: abnf

    trailingLambdaCall:
        ( objectReference '.' identifier typeArguments?
        | expression ('?.' | typeArguments)?
        )
        arguments block
        ;

Currently, no parameter can be specified for the trailing lambda,
except a receiver parameter (see :ref:`Lambda Expressions with Receiver`).
Otherwise, a :index:`compile-time error` occurs.

**Note**. If a call is followed by a block, and the function or method
being called has no last function type parameter, then such block is
handled as an ordinary block of statements but not as a lambda function.

In case of other ambiguities (e.g., when a function or method call has the
last parameter, which can be optional, of a function type), a syntax
production that starts with '{' following the function or method call is
handled as the *trailing lambda*.
If other semantics is needed, then the semicolon '``;``' separator can be used.
It means that the function or the method is to be called without the last
argument (see :ref:`Optional Parameters`).

.. index::
   trailing lambda
   parameter
   block
   function
   method
   function type
   lambda function
   lambda
   semicolon
   call

.. code-block:: typescript
   :linenos:

      class A {
          foo (p?: ()=>void) { ... }
      }

      const a = new A()

      a.foo() { console.log ("method lambda argument is activated") }
      // method 'foo' receives last argument as an inline lambda

      a.foo(); { console.log ("that is the block code") }
      // method 'foo' is called with 'p' parameter set to 'undefined'
      // ';' allows to specify explicitly that '{' starts the block

      function bar(f?: ()=>void) { ... }

      bar() { console.log ("function lambda argument is activated") }
      // function 'bar' receives last argument as an inline lambda,
      bar(); { console.log ("that is the block code") }
      // function 'bar' is called with 'f' parameter set to 'undefined'

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

Several static methods are available to handle each enumeration type as follows:

-  Method ``values()`` returns an array of enumeration constants in the order of
   declaration.
-  Method ``getValueOf(name: string)`` returns an enumeration constant with the
   given name, or throws an error if no constant with such name exists.

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

There are additional methods for instances of any enumeration type:

-  Method ``valueOf()`` returns an ``int`` or ``string`` value of an enumeration
   constant depending on the type of the enumeration constant.

-  Method ``getName()`` returns the name of an enumeration constant.

.. code-block-meta:

.. code-block:: typescript
   :linenos:

      enum Color { Red, Green = 10, Blue }
      let c: Color = Color.Green
      console.log(c.valueOf()) // prints 10
      console.log(c.getName()) // prints Green

**Note**. Methods ``c.toString()`` and ``c.valueOf().toString()`` return the
same value.

.. index::
   instance
   enumeration type
   value
   enumeration constant
   type int
   type string

|

.. _Errors and Initialization Expression:

Errors and Initialization Expression
====================================

.. meta:
    frontend_status: Done

If code of an *initialization expression* of *variable declaration* (see
:ref:`Variable Declarations`), *constant declaration* (see
:ref:`Constant Declarations`), or *class field initializer* (see
:ref:`Field Initialization`) contains unhandled ``throw`` statement (see
:ref:`Throw Statements`), then the program terminates (see :ref:`Program Exit`).

These situations are represented by the examples below:

.. code-block:: typescript
   :linenos:

    const const_decl = foo(true)
    class A {
        field_decl = foo(false)
    }


    function foo(toggle: boolean) {
        if (toggle)
            throw new Error // No surrounding 'try'
        return true
    }



.. index::
   variable declaration
   initialization expression
   constant declaration
   field initializer
   expression
   initialization
   variable
   constant
   function call
   statement

|

.. _Coroutines:

Coroutines
**********

.. meta:
    frontend_status: Partly

A function or lambda can be a *coroutine*. |LANG| supports *basic coroutines*,
*structured coroutines*.
Basic coroutines are used to create and launch a coroutine; the result is then
to be awaited.

See :ref:`Standard Library`.

.. index::
   structured coroutine
   basic coroutine
   coroutine

|

.. _Async Functions and Methods:

Async Functions and Methods
***************************

.. meta:
    frontend_status: Done


.. _Async Functions:

``Async`` Functions
===================

.. meta:
    frontend_status: Done

``Async`` functions are implicit coroutines that can be called as regular
functions. ``Async`` functions can be neither ``abstract`` nor ``native``.

The return type of an ``async`` function must be ``Promise<T>``. 
Returning values of types ``Promise<T>`` and ``T``
from ``async`` functions is allowed.

Using return statement without an expression is allowed if the return type
is ``Promise<void>``.
*No-argument* return statement can be added implicitly as the last statement
of the function body if there is no explicit return statement in a function
with the return ``Promise<void>``.

**Note**. Using type ``Promise<void>`` is not recommended as this type is
supported for the sake of backward |TS| compatibility only.

.. index::
   function async
   coroutine
   return type
   function body
   backward compatibility
   annotation
   no-argument return statement
   async function
   return statement
   compatibility

|

.. _Experimental Async Methods:

Experimental ``Async`` Methods
==============================

.. meta:
    frontend_status: Done

The method ``async`` is an implicit coroutine that can be called as a regular
method. ``Async`` methods can be neither ``abstract`` nor ``native``.

The return type of an ``async`` method must be ``Promise<T>``. 
Returning values of types ``Promise<T>`` and ``T``
from ``async`` methods is allowed.

Using return statement without an expression is allowed if the return type
is ``Promise<void>``.
*No-argument* return statement can be added implicitly as the last statement
of the methods body if there is no explicit return statement in a method
with return type ``Promise<void>``.

**Note**. Using this annotation is not recommended as this type of methods
is supported for the sake of backward |TS| compatibility only.

.. index::
   async method
   coroutine
   return type
   function body
   compatibility
   no-argument return statement
   annotation
   abstract method
   native method

|

.. _Packages:

Packages
********

.. meta:
    frontend_status: Partly
    todo: Implement compiling a package module as a single compilation unit - #16267

One or more *package modules* form a package:

.. code-block:: abnf

      packageDeclaration:
          packageModule+
          ;

*Packages* are stored in a file system or a database (see
:ref:`Compilation Units in Host System`).

A *package* can consist of several package modules if all such modules
have the same *package header*:

.. index::
   package module
   package
   file system
   database
   package header
   module

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
          topDeclaration | initializerBlock
          ;

A :index:`compile-time error` occurs if:

-  *Package module* contains no package header; or
-  Package headers of two package modules in the same package have
   different identifiers.

Every *package module* can directly use all exported entities from the core
packages of the standard library (see :ref:`Standard Library Usage`).

A *package module* can directly access all top-level entities declared in all
modules that constitute the package.

If a top declaration in any package module contains an initializer for a
variable or a constant, then the form of the initializer must be
*constantExpression*. Otherwise, a :index:`compile-time error` occurs.
Initializer block is to be used for initialization to ensure an explicit order
of initialization.

.. code-block:: typescript
   :linenos:

   package P
     let v1 = foo() // Compile-time error as call to foo() is not a constant expression
     function foo() { return 1 }

     let v2 = 2 + 3 * 4 // OK

     let v2: number
     static {
        v2 = foo() // OK
     }




.. index::
   package module
   package header
   package
   identifier
   core package
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
    todo: Implement in libpandafile, implement semantic, now it is parsed and ignored - #16088

The modifier ``internal`` indicates that a class member, a constructor, or
an interface member is accessible (see :ref:`Accessible`) within its
compilation unit only. If the compilation unit is a package (see
:ref:`Packages`), then ``internal`` members can be used in any
*package module*. If the compilation unit is a separate module (see
:ref:`Separate Modules`), then ``internal`` members can be used within this
module.

.. index::
   modifier
   access modifier
   accessibility
   interface
   class member
   constructor
   access
   package module
   module

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

.. index::
   member
   constructor
   internal modifier
   access

|

.. _Package Initializer:

Package Initializer
===================

.. meta:
    frontend_status: Partly

Among all *package modules* there can be one to contain a code that performs
initialization actions (e.g., setting initial values for variables across all
package modules) as described in detail in
:ref:`Compilation Unit Initialization` and in :ref:`Initializer Block`.

A :index:`compile-time error` occurs if a package contains *package initializer*
in more than one source file.

.. code-block:: typescript
   :linenos:

      // Source file 1
      package P
         static { // P initializer part one
         }
         function foo() {}
         static { // P initializer part two
         }


      // Source file 2
      package P
         static {} // compile-time error as initializer in a different source file



.. index::
   package initializer
   package module
   initialization
   compilation unit
   variable

|

.. _Import and Overloading of Function Names:

Import and Overloading of Function Names
========================================

.. meta:
    frontend_status: Done

While importing functions, the following situations can occur:

-  Different imported functions have the same name but different signatures, or
   a function (functions) of the current module and an imported function
   (functions) have the same name but different signatures. This situation is
   called *overloading*. All such functions are accessible (see :ref:`Accessible`).

-  A function (functions) of the current module and an imported function
   (functions) have the same name and an overload-equivalent signature (see
   :ref:`Overload-Equivalent Signatures`). This situation causes a
   :index:`compile-time error` as declarations are duplicated. Qualified import
   or alias in import can be used to access the imported entity.

.. index::
   import
   function
   overloading
   function name
   function
   imported function
   signature
   module
   access
   accessibility

The two situations are illustrated by the examples below:

.. code-block-meta:
   skip

.. code-block:: typescript
   :linenos:

      // Overloading case
      package P1
      function foo(p: int) {}

      package P2
      function foo(p: string) {}

      // Main module
      import {foo} from "path_to_file_with_P1"
      import {foo} from "path_to_file_with_P2"

      function foo (p: double) {}

      function main() {
        foo(5) // Call to P1.foo(int)
        foo("A string") // Call to P2.foo(string)
        foo(3.141592653589) // Call to local foo(double)
      }

      // Declaration duplication case
      package P1
         function foo() {}
      package P2
         function foo() {}
      // Main program
      import {foo} from "path_to_file_with_P1"
      import {foo} from "path_to_file_with_P2" /* Error: duplicating
          declarations imported*/
      function foo() {} /* Error: duplicating declaration identified
          */
      function main() {
        foo() // Error: ambiguous function call
        // But not a call to local foo()
        // foo() from P1 and foo() from P2 are not accessible
      }

|

.. _Generics Experimental:

Generics Experimental
*********************

.. meta:
    frontend_status: Done

|

.. _NonNullish Type Parameter:

NonNullish Type Parameter
=========================

.. meta:
    frontend_status: Partly

If some generic class has a type parameter with a nullish union type constraint,
then special syntax can be used for type annotation to get a non-nullish
version of the type parameter variable. The example below illustrates this
possibility:

.. index::
   generic class
   type parameter
   nullish union type
   constraint
   annotation
   non-nullish type
   variable
   parameter

.. code-block:: typescript
   :linenos:

      class A<T> {  // in fact it extends Object|null|undefined
          foo (p: T): T! { // foo returns non-nullish value of p
             return p!
          }
      }

      class B<T extends SomeType | null> {
          foo (p: T): T! { // foo returns non-nullish value of p
             return p!
          }
      }

      class C<T extends SomeType | undefined> {
          foo (p: T): T! { // foo returns non-nullish value of p
             return p!
          }
      }

      let a = new A<Object>
      let b = new B<SomeType>
      let c = new C<SomeType>

      let result: Object = new Object  // Type of result is non-nullish !
      result = a.foo(result)
      result = b.foo(new SomeType)
      result = c.foo(new SomeType)

      // All such assignments are type-valid as well
      result = a.foo(void)      // void! => never
      result = b.foo(null)      // null! => never
      result = c.foo(undefined) // undefined! => never

.. raw:: pdf

   PageBreak
