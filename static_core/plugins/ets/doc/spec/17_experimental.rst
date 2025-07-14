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

The *array creation* feature introduced in
:ref:`Resizable Array Creation Expressions`
enables users to dynamically create objects of array type by using runtime
expressions that provide the array size. This addition is useful to other
array-related features of the language, such as array literals.
This feature can also be used to create multidimensional arrays.

Overloading functions, methods, or constructors is a practical and convenient
way to write program actions that are similar in logic but different in
implementation. |LANG| uses :ref:`Overload Declarations` as an innovative
form of *managed overloading*.

.. index::
   implementation
   array creation
   runtime expression
   array
   array literal
   constructor
   function
   method
   array type
   runtime
   array size
   multidimensional array
   function overloading
   method overloading
   implementation
   constructor overloading

Section :ref:`Native Functions and Methods` introduces practically important
and useful mechanisms for the inclusion of components written in other languages
into a program written in |LANG|.

Sections :ref:`Final Classes` and :ref:`Final Methods`
discuss the well-known feature that
in many OOP languages provides a way to restrict class inheritance and method
overriding. Making a class *final* prohibits defining classes derived from it,
whereas making a method *final* prevents it from overriding in derived classes.

Section :ref:`Adding Functionality to Existing Types` discusses the way to
add new functionality to an already defined type.

Section :ref:`Enumeration Methods` adds methods to declarations of the
enumeration types. Such methods can help in some kinds of manipulations
with ``enums``.

.. index::
   native function
   native method
   class inheritance
   implementation
   function overloading
   method overloading
   method overriding
   final class
   method final
   OOP (object-oriented programming)
   inheritance
   enum
   class
   interface
   inheritance
   derived class
   enumeration method
   functionality

The |LANG| language supports writing concurrent applications in the form of
*coroutines* (see :ref:`Coroutines (Experimental)`) that allow executing
functions concurrently.

There is a basic set of language constructs that support concurrency. A function
to be launched asynchronously is marked by adding the modifier ``async``
to its declaration. In addition, any function or lambda expression can be
launched as a separate thread explicitly by using the launch function from
the standard library.

.. index::
   coroutine
   modifier async
   async
   lambda expression
   concurrency
   launch function
   asynchronous launch

|

.. _Type char:

Type ``char``
*************

.. meta:
    frontend_status: Partly

Values of ``char`` type are Unicode code points.

.. list-table::
   :width: 100%
   :widths: 15 60
   :header-rows: 1

   * - Type
     - Type's Set of Values
   * - ``char`` (32-bits)
     - Symbols with codes from \U+0000 to \U+10FFFF (maximum valid Unicode code
       point) inclusive

Predefined constructors, methods, and constants for ``char`` type are
parts of the |LANG| :ref:`Standard Library`.

.. index::
   char type
   Unicode code point
   set of values
   predefined constructor
   predefined method
   predefined constant

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

The syntax of *character literal* is presented below:

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
   value
   character
   syntax
   escape sequence
   single quote
   type char
   value

|

.. _Character Equality Operators:

Character Equality Operators
============================

.. meta:
    frontend_status: Partly
    todo: need to adapt the implementation to the latest specification

*Value equality* is used for operands of type ``char``.

If both operands represent the same Unicode code point,
then the result of ':math:`==`' or ':math:`===`'
is ``true``. Otherwise, the result is ``false``.

.. index::
   character
   value
   char type
   Unicode code point
   equality operator
   value equality operator
   operand

|

.. _Fixed-Size Array Types:

Fixed-Size Array Types
**********************

.. meta:
    frontend_status: Partly

*Fixed-size array type*, written as ``FixedArray<T>``, is the built-in type
characterized by the following:

-  Any instance of array type contains elements. The number of elements is known
   as *array length*, and can be accessed by using the ``length`` property.
-  Array length is a non-negative integer number.
-  Array length is set once at runtime and cannot be changed after that.
-  Array element is accessed by its index. *Index* is an integer number
   starting from *0* to *array length minus 1*.
-  Accessing an element by its index is a constant-time operation.
-  If passed to a non-|LANG| environment, an array is represented as a contiguous
   memory location.
-  Type of each array element is assignable to the element's type specified
   in the array declaration (see :ref:`Assignability`).

*Fixed-size arrays* differ from *resizable arrays* as follows:

- Fixed-size array length is set once to achieve better performance;
- Fixed-size arrays have no methods defined;
- Fixed-size arrays have several constructors (see
  :ref:`Fixed-Size Array Creation`);
- Fixed-size arrays are not compatible with *resizable arrays*.

Incompatibility between a resizable array and a fixed-size array is represented
by the example below:

.. code-block:: typescript
   :linenos:

    function foo(a: FixedArray<number>, b: Array<number>) {
        a = b // compile-time error
        b = a // compile-time error
    }

.. index::
   resizable array
   fixed-size array
   fixed-size array type
   built-in type
   instance
   array type
   length property
   array length
   runtime
   access
   index
   integer number
   constant-time operation
   memory location
   assignability
   array declaration
   compatibility
   incompatibility

|

.. _Fixed-Size Array Creation:

Fixed-Size Array Creation
=========================

.. meta:
    frontend_status: Partly

*Fixed-size array* can be created by using :ref:`Array Literal` or
the constructors defined for type ``FixedArray<T>``.

Using *array literal* to create an array is represented by the example below:

.. code-block:: typescript
   :linenos:

    let a : FixedArray<number> = [1, 2, 3]
      /* create array with 3 elements of type number */
    a[1] = 7 /* put 7 as the 2nd element of the array, index of this element is 1 */
    let y = a[2] /* get the last element of array 'a' */
    let count = a.length // get the number of array elements
    y = a[3] // Will lead to runtime error - attempt to access non-existing array element

.. index::
   fixed-size array type
   array length
   array literal
   constructor
   fixed-size array
   integer
   array element
   access
   assignability
   resizable array
   runtime error

Several constructors can be called to create a ``FixedArray<T>`` instance as
follows:

- ``constructor(len: int)``, if type ``T`` has either a default value (see
  :ref:`Default Values for Types`) or a constructor that can be called with
  no argument provided:

.. code-block:: typescript
   :linenos:

    // type ``number`` has a default value:
    let a = new FixedArray<number>(3) // creates array [0.0, 0.0, 0.0]

    class C {
        constructor (n?: number) {}
    }
    let b = new FixedArray<C>(2) // creates array [new C(), new C()]

- ``constructor(len: int, elem: T)`` for any ``T``. The constructor creates an
  array instance filled with a single value ``elem``:

.. code-block:: typescript
   :linenos:

    let a = new FixedArray<string>(3, "a") // creates array ["a", "a", "a"]

- ``constructor(len: int, elems: (inx: int) => T)`` for any ``T``. The
  constructor creates an array instance where each *i* element is evaluated
  as a result of the ``elems`` call with argument *i*:

.. code-block:: typescript
   :linenos:

    let a = new FixedArray<int>(3, (inx: int) => 3 - inx )
    // creates array [3, 2, 1]

.. index::
   constructor
   call
   default value
   value
   argument
   array instance
   array
   instance

|

.. _Resizable Array Creation Expressions:

Resizable Array Creation Expressions
************************************

.. meta:
    frontend_status: Done

*Array creation expression* creates new objects that are instances of *resizable
arrays* (see :ref:`Resizable Array Types`). An array instance can be created
alternatively by using :ref:`Array literal`.

The syntax of *array creation expression* is presented below:

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

.. index::
   resizable array
   array creation expression
   object
   instance
   array
   array instance
   array literal

*Array creation expression* creates an object that is a new array with the
elements of the type specified by ``arrayElelementType``.

The type of each *dimension expression* must be assignable (see
:ref:`Assignability`) to an ``int`` type. Otherwise,
a :index:`compile-time error` occurs.

A :index:`compile-time error` occurs if any *dimension expression* is a
constant expression that is evaluated to a negative integer value at compile
time.

.. index::
   array creation expression
   array
   type
   dimension expression
   conversion
   integer
   integer type
   int type
   assignability
   type
   integer value
   type int
   constant expression
   compile time

If the type of any *dimension expression* is ``number`` or other floating-point
type, and its fractional part is other than '0', then errors occur as
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
   dimension expression
   number
   floating-point type
   fractional part
   compile time
   compile-time error
   runtime error
   compilation
   expression
   array element
   lambda function
   array
   parameter


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

.. index::
   class
   accessibility
   access
   parameterless constructor
   constructor
   parameter
   optional parameter
   default value

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

      class C{
        constructor (n: number) {}
      }
      let x = new C[3] // compile-time error: no parameterless constructor

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
   compile-time error
   type parameter
   array

The creation of an array with a known number of elements is presented below:

.. code-block:: typescript
   :linenos:

      class A {
        constructor (x: number) {}
      }
      // A has no default value or parameterless constructor

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

.. index::
   array
   array creation
   parameterless constructor
   default value
   exotic array
   type
   lambda function
   index

.. code-block:: typescript
   :linenos:

      let array_of_union = new (Object|undefined) [5] // filled with undefined
      let array_of_functor = new (() => void) [5] ( (): void => {})
      type aliasTypeName = number []
      let array_of_array = new aliasTypeName [5] ( [3.141592653589] )

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
   dimension expression is less than zero, then ``NegativeArraySizeError``
   is thrown.

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
   array creation
   array creation expression
   evaluation
   dimension expression
   constructor
   abrupt completion
   expression
   space allocation
   one-dimensional array
   multidimensional array
   class type
   runtime
   runtime evaluation
   evaluation
   default value
   parameterless constructor
   class type
   initialization
   nested loop
   array of arrays

|

.. _Enumerations Experimental:

Enumerations Experimental
*************************

Several experimental features described below are available for enumerations.

|

.. _Enumeration with Explicit Type:

Enumeration with Explicit Type
==============================

.. meta:
    frontend_status: None

*Enumeration with explicit type* uses the following syntax:

.. code-block:: abnf

    enumDeclaration:
        'const'? 'enum' identifier ':' type '{' enumConstantList? '}'
        ;

All enumeration constants of a declared enumeration are of the *explicit type*
specified in the declaration, i.e., the *explicit type* is the
*enumeration base type* (see :ref:`Enumerations`).

.. index::
   enumeration base type
   enumeration with explicit type
   syntax
   enumeration constant
   enumeration
   declaration
   explicit type

If *explicit type* is an integer type then omitted values for constants allowed,
the same rules applied as for enum with non-explicit type (see :ref:`Enumeration Integer Values`).

A :index:`compile-time error` occurs in the following situations:

- *Explicit type* is different from any numeric or string type.
- Enumeration constant has no value and *Explicit type* is not an integer type.
- Enumeration constant type is not assignable (see :ref:`Assignability`)
  to the *explicit type*.

.. index::
   explicit type
   enum constant
   integer type
   non-explicit type
   integer value
   enumeration constant
   assignability
   numeric type
   string type
   value
   type
   syntax

.. code-block:: typescript
   :linenos:

    enum DoubleEnum: double { A = 0.0, B = 1, C = 3.141592653589 } // OK
    enum LongEnum: long { A = 0, B = 1, C = 3 } // OK

    enum IncorrectEnum1: double { A, B, C } // compile-time error
    enum IncorrectEnum2: double { A = 1.0, B = 2, C = "a string" } // compile-time error

|

.. _Enumeration Methods:

Enumeration Methods
===================

.. meta:
    frontend_status: Done

Several static methods are available to handle each enumeration type
as follows:

-  Method ``static values()`` returns an array of enumeration constants
   in the order of declaration.
-  Method ``static getValueOf(name: string)`` returns an enumeration constant
   with the given name, or throws an error if no constant with such name
   exists.
-  Method ``static fromValue(value: T)``, where ``T`` is the base type
   of the enumeration, returns an enumeration constant with a given value, or
   throws an error if no constant has such a value.

.. index::
   enumeration method
   static method
   enumeration type
   enumeration constant
   constant
   value

.. code-block:: typescript
   :linenos:

      enum Color { Red, Green, Blue = 5 }
      let colors = Color.values()
      //colors[0] is the same as Color.Red

      let red = Color.getValueOf("Red")

      Color.fromValue(5) // ok, retuns Color.Blue
      Color.fromValue(6) // throws runtime error

Additional methods for instances of an enumeration type are as follows:

-  Method ``valueOf()`` returns a numeric or ``string`` value of an enumeration
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
   method
   enumeration type
   value
   enumeration constant


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

If only one function is present, then only the appropriate form of indexing
expression (see :ref:`Indexing Expressions`) is available:

.. index::
   indexable type
   interface
   class
   declaration
   function name
   function
   signature
   indexing expression
   variable
   type

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

Type ``string`` can be used as a type of the index parameter:

.. index::
   function
   indexing expression
   string
   string type
   type
   index parameter

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
   function
   ordinary function
   compiler
   compiler-known signature
   abstract function
   signature
   overriding
   interface
   implementation
   dynamic dispatch
   implementation
   indexing expression
   indexing expression evaluation
   generic class
   generic interface
   evaluation
   flexibility
   async function
   generic class
   generic interface
   function

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

A class or an interface is *iterable* if it implements the interface ``Iterable``
defined in the :ref:`Standard Library`, and thus has an accessible parameterless
method with the name ``$_iterator`` and a return type that is a subtype (see
:ref:`Subtyping`) of type ``Iterator`` as defined in the :ref:`Standard Library`.
It guarantees that an object returned by the ``$_iterator`` method is of the
type which implements ``Iterator``, and thus allows traversing an object of the
*iterable* type.

A union of iterable types is also *iterable*. It means that instances of such
types can be used in ``for-of`` statements (see :ref:`For-Of Statements`).

An *iterable* class ``C`` is represented by the example below:

.. index::
   iterable class
   class
   iterable interface
   interface
   parameterless method
   access
   accessibility
   subtyping
   iterator
   instance
   for-of statement
   return type
   assignability
   type Iterator
   implementation
   iterable type
   union
   for-of statement
   object

.. code-block:: typescript
   :linenos:

      class C implements Iterable {
        data: string[] = ['a', 'b', 'c']
        $_iterator() { // Return type is inferred from the method body
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

In the example above, class ``C`` method ``$_iterator`` returns
``CIterator<string>`` that implements ``Iterator<string>``. If executed,
this code prints out the following:

.. code-block:: typescript

    "a"
    "b"
    "c"

The method ``$_iterator`` is an ordinary method with a compiler-known
signature. This method can be used like any other method. It can be
abstract or defined in an interface to be implemented later. A
:index:`compile-time error` occurs if this method is marked as ``async``.

.. index::
   type inference
   method
   class
   string
   iterator
   compiler-known signature
   compiler
   signature
   implementation
   async method

**Note**. To support the code compatible with |TS|, the name of the method
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
   compatible code
   method
   iterator
   iterable class

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
   type name
   expression
   type call expression
   callable class type
   callable type
   class type
   type call expression
   method call
   inheritance
   static method
   normal method call
   call
   explicit call
   method

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
   constructor
   method
   call
   new expression
   callable type

|

.. _Callable Types with $_invoke Method:

Callable Types with ``$_invoke`` Method
=======================================

.. meta:
    frontend_status: Done

The static method ``$_invoke`` can have an arbitrary signature. The method
can be used in a *type call expression* in either case above. If the signature
has parameters, then the call must contain corresponding arguments.

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
   static method
   callable type
   arbitrary signature
   signature
   parameter
   method
   type call expression
   argument
   instance method
   type

That a type contains the instance method ``$_invoke`` does not make the type
*callable*.

|

.. _Callable Types with $_instantiate Method:

Callable Types with ``$_instantiate`` Method
============================================

.. meta:
    frontend_status: Done

The static method ``$_instantiate`` can have an arbitrary signature by itself.
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
   static method
   callable type
   method
   signature
   arbitrary signature
   type call expression
   parameter
   factory parameter
   parameterless function type
   class type
   type call expression

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
   method
   call
   type call expression
   instantiation
   parameter
   callable type

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

That a type contains the instance method ``$_instantiate`` does not make the
type *callable*.

|

.. _Statements Experimental:

Statements
**********

.. meta:
    frontend_status: Done

|

.. _For-of Explicit Type Annotation:

For-of Explicit Type Annotation
===============================

.. meta:
    frontend_status: Partly
    todo: check assignability

An explicit type annotation is allowed for a *ForVariable*
(see :ref:`For-Of Statements`):

.. code-block:: typescript
   :linenos:

      // explicit type is used for a new variable,
      let x: number[] = [1, 2, 3]
      for (let n: number of x) {
        console.log(n)
      }

Type of elements in a ``for-of`` expression must be assignable
(see :ref:`Assignability`) to the type of the variable. Otherwise, a
:index:`compile-time error` occurs.

.. index::
   type annotation
   for-variable
   expression
   assignability
   variable
   for-of type annotation

|

.. _Overload Declarations:

Overload Declarations
*********************

.. meta:
    frontend_status: None

|LANG| supports both the |TS|-compatible *overload signatures* (see
:ref:`Declarations with Overload Signatures`), and an innovative form of
*managed overloading* that allows a developer to fully control the order of
selecting a specific entity to call from several overloaded entities.

The actual entity to be called is determined at compile time. Thus,
*overloading* is related to the *compile-time polymorphism by name*.
The semantic details are discussed in :ref:`Overloading`.

.. index::
    compile-time polymorphism
    polymorphism by name
    managed overloading
    overloading
    overload signature
    overloaded entity
    compile time
    compatibility

An *overload declaration* is used in *managed overloading* to
define a set and an order of the overloaded entities (functions, methods,
or constructors).

An *overload declaration* can be used for:

- Functions (see :ref:`Function Declarations`), including functions in
  namespaces;
- Class or interface methods (see :ref:`Method Declarations` and
  :ref:`Interface Method Declarations`); and
- :ref:`Ambient Declarations`.

An *overload declaration* starts with the keyword ``overload`` and
declares an *overload alias* for a set of explicitly listed entities as follows:

.. index::
    overload declaration
    managed overloading
    overloaded entity
    function
    method
    constructor
    namespace
    class method
    interface method
    method declaration
    ambient declaration
    overload keyword
    entity
    overload alias

.. code-block:: typescript
   :linenos:

    function max2(a: number, b: number): number {
        return  a > b ? a : b
    }
    function maxN(...a: number[]): number {
        // return max element
    }

    // declare 'max' as an ordered set of functions max2 and maxN
    overload max { max2, maxN }

    max(1, 2)     // max2 is called
    max(3, 2, 4)  // maxN is called
    max("a", "b") // compile-time error, no function to call

    maxN(1, 2)    // maxN is explicitly called

The semantics of an entity included into an *overload set* does not change.
Such entities follow the ordinary accessibility rules, and can be used
separately from an overload alias, e.g., called explicitly as follows:

.. code-block:: typescript
   :linenos:

    maxN(1, 2) // maxN is explicitly called
    max2(2, 3) // max2 is explicitly called

When calling an *overload alias*, entities from an *overload set* are checked
in the listed order, and the first entity with an appropriate signature is
called (see :ref:`Overload resolution for Overload Declarations` for detail).
A :index:`compile-time error` occurs if no entity with an appropriate signature
is available:

.. index::
    function
    semantics
    entity
    overload
    overload set
    accessibility
    overload alias
    overload set
    overload resolution
    overload declaration
    signature
    function call

.. code-block-meta:
    expect-cte

.. code-block:: typescript
   :linenos:

    max(1)    // maxN is called
    max(1, 2) // max2 is called, as is the first in order

    max("a", "b") // compile-time error, no function to call

It means that exactly one entity is selected for a call at the call site.
Otherwise, a :index:`compile-time error` occurs.

An overloaded entity in an *overload declaration* can be *generic* (see
:ref:`Generics`).

If during :ref:`Overload Resolution for Overload Declarations` *type arguments*
are provided explicitly in a call of an *overload alias* (see
:ref:`Explicit Generic Instantiations`), then consideration is given only to
the entities that have an equal number of *type parameters* and *type arguments*.

If *type arguments* are not provided explicitly (see
:ref:`Implicit Generic Instantiations`), then consideration is given to all
entities as represented in the example below:

.. index::
    function call
    overloaded entity
    overload declaration
    generic
    generic instantiation
    type argument
    type parameter
    overload resolution
    overload alias


.. code-block:: typescript
   :linenos:

    function foo1(s: string) {}
    function foo2<T>(x: T) {}

    overload foo { foo1, foo2 }

    foo("aa")   // foo1 is called
    foo(1) // foo2 is called, implicit generic instantiation
    foo<string>("aa") // foo2 is called


An entity can be listed in several *overload declarations*:

.. code-block:: typescript
   :linenos:

    function max2i(a: int, b: int): int {
        return  a > b ? a : b
    }
    function maxNi(...a: int[]): int {
        // return max element
    }
    function maxN(...a: number[]): number {
        // return max element
    }

    overload maxi { max2i, maxNi }
    overload max { max2i, maxNi, maxN }

.. index::
    entity
    overload declaration
    generic instantiation

|

.. _Function Overload Declarations:

Function Overload Declarations
==============================

.. meta:
    frontend_status: None

*Function overload declaration* allows declaring an *overload alias*
for a set of functions (see :ref:`Function Declarations`).

The syntax is presented below:

.. code-block:: abnf

    overloadFunctionDeclaration:
        'overload' identifier '{' qualifiedName (',' qualifiedName)* ','? '}'
        ;

.. index::
    function overload
    overload declaration
    function overload declaration
    overload alias
    function
    syntax


A :index:`compile-time error` occurs, if a *qualified name*:

- Does not refer to an accessible function; or

- Refers to a function with overload signatures (see
  :ref:`Function with Overload Signatures`).

A :index:`compile-time error` occurs, if an *overload alias* is exported
but an overloaded function is not:

.. code-block:: typescript
   :linenos:

    export function foo1(p: string) {}
    function foo2(p: number) {}
    export overload foo { foo1, foo2 } // compile-time error, 'foo2' is not exported
    overload bar { foo1, foo2 } // ok, as 'bar' is not exported

All overloaded functions must be in the same module or namespace scope (see
:ref:`Scopes`). Otherwise, a :index:`compile-time error` occurs. The erroneous
overload declarations are represented in the example below:

.. code-block:: typescript
   :linenos:

    import {foo1} from "something"

    function foo2() {}
    overload foo {foo1, foo2} // compile-time error

    namespace N {
        export function fooN() {}
        namespace M {
            export function fooM() {}
        }
        overload goo {M.fooM, fooN} // compile-time error
    }
    overload bar {foo2, N.fooN} // compile-time error

.. index::
    qualified name
    accessible function
    access
    function with overload signatures
    overload signature
    function
    export
    overload alias
    overloaded function
    module
    namespace
    namespace scope
    scope
    overload declaration

|

.. _Class Method Overload Declarations:

Class Method Overload Declarations
==================================

.. meta:
    frontend_status: None

*Method overload declaration* allows declaring an *overload alias*
as a class member (see :ref:`Class Members`)
for a set of static or instance methods (see :ref:`Method Declarations`).
The syntax is presented below:

.. code-block:: abnf

    overloadMethodDeclaration:
        overloadMethodModifier*
        'overload' identifier '{' identifier (',' identifier)* ','? '}'
        ;

    overloadMethodModifier: 'static' | 'async';

Using *method overload declaration* and calling an *overload alias* are
represented in the example below:

.. index::
    class method
    class member
    static method
    instance method
    method
    method overload
    syntax
    method overload declaration
    overload alias

.. code-block:: typescript
   :linenos:

    class Processor {
        overload process { processNumber, processString }
        processNumber(n: number) {/*body*/}
        processString(s: string) {/*body*/}
    }

    let c = new C()
    c.process(42) // calls processNumber
    c.process("aa") // calls processString

*Static overload alias* is represented in the example below:

.. code-block:: typescript
   :linenos:

    class C {
        static one(n: number) {/*body*/}
        static two(s: string) {/*body*/}
        static overload foo { one, two }
    }

A :index:`compile-time error` occurs if:

-  Method modifier is used more than once in an method overload declaration;

-  *Identifier* in the overloaded method list:

    - Does not refer to an accessible method (either declared or inherited)
      of the current class;
    - Refers to a method with overload signatures (see
      :ref:`Class Method with Overload Signatures`);

-  *Overload alias* is:

    - *Static* but the overloaded method is not;
    - *Non-static* but the overloaded method is not;
    - Marked ``async`` but the overloaded method is not; or
    - Not ``async`` but the overloaded method is.


.. index::
    static overload alias
    method modifier
    class
    method overload declaration
    identifier
    accessible method
    declaration
    inheritance
    overloaded method
    overload signature
    overload alias


*Overload alias* and overloaded methods can have different access modifiers.
A :index:`compile-time error` occurs if the *overload alias* is:

-  ``public`` but at least one overloaded method is not ``public``;

-  ``protected`` but at least one overloaded method is ``private``.


Valid and invalid overload declarations are represented in the example below:

.. index::
    overload alias
    overloaded method
    overload declaration
    access modifier
    public
    protected
    private

.. code-block:: typescript
   :linenos:

    class C {
        private foo1(x: number) {/*body*/}
        protected foo2(x: string) {/*body*/}
        public foo3(x: boolean) {/*body*/}
        foo4() {/*body*/} // implicitly public

        public overload foo { foo3, foo4 } // ok
        protected overload bar { foo2, foo3 } // ok
        private overload goo { foo1, foo2, foo3 } // ok

        public overload err1 {foo2, foo3} // compile-time error, foo2 is not public
        protected overload err2 {foo2, foo1} // compile-time error, foo1 is private
    }

Some or all overloaded functions can be ``native`` as follows:

.. code-block:: typescript
   :linenos:

    class C {
        native foo1(x: number)
        foo2(x: string) {/*body*/}
        overload foo { foo1, foo2 }
    }

If a subclass has an *overload declaration*, then the overload declaration
can be overridden in a subclass. If a superclass has no *overload declaration*,
then the declaration from the superclass in used.
If a subclass has an *overload declaration*, then the overload declaration must
list all methods overloaded in a superclass. Otherwise, a
:index:`compile-time error` occurs.
*Overload declaration* in a subclass can add new methods and change the order
of methods.

An *overload alias* is used like an ordinary class method except that it is
replaced in a call at compile time for one of overloaded methods that use the
type of *object reference*. The *overload declaration* in subtypes is
represented in the example below:

.. index::
    overloaded function
    native
    superclass
    overload declaration
    overriding
    subclass
    declaration
    superclass
    overloaded method
    overload alias
    object reference
    method


.. code-block:: typescript
   :linenos:

    class Base {
        overload process { processNumber, processString }
        processNumber(n: number) {/*body*/}
        processString(s: string) {/*body*/}
    }

    class D1 extends Base {
        // method is overridden
        override processNumber(n: number) {/*body*/}
        // overload declaration is inherited
    }

    class D2 extends Base {
        // method is added:
        processInt(n: int) {/*body*/}
        // new order for overloaded methods is specified:
        overload process { processInt, processNumber, processString }
    }

    new D1().process(1)   // calls processNumber from D1

    new D2().process(1)   // calls processInt from D2 (as it is listed earlier)
    new D2().process(1.0) // calls processNumber from Base (first appropriate)

Methods with special names (see :ref:`Indexable Types`, :ref:`Iterable Types`,
and :ref:`Callable Types`) can be overloaded like ordinary methods:

.. index::
    overloaded method
    overriding
    callable type
    inheritance
    ordinary method
    name

.. code-block:: typescript
   :linenos:

    class C {
        getByNumber(n: number): string {...}
        getByString(s: string): string {...}
        overload $_get { getByNumber, getByString }
    }

    let c = new C()

    c[1]     // getByNumber is used
    c["abc"] // getByString is used

If a class implements some interfaces with *overload declarations* for the
same alias, then a new *overload declaration* must include all overloaded
methods. Otherwise, a :index:`compile-time error` occurs.

.. index::
    overloaded method
    class
    interface
    overload declaration
    alias

.. code-block:: typescript
   :linenos:

    interface I1 {
        overload foo {f1, f2}
        // f1 and f2 are declared in I1
    }
    interface I2 {
        overload foo {f3, f4}
        // f3 and f4 are declared in I2
    }
    class C implements I1, I2 {
       // compile-time error as no new overload is defined
    }
    class D implements I1, I2 {
        overload foo { f2, f3, f1, f4 } // OK, as new overload is defined
    }
    class E implements I1, I2 {
        overload foo { f2, f4 } // compile-time error as not all methods are used
    }

    const i1: I1 = new D
    i1.foo(<arguments>) // call is valid if arguments fit first signature of {f1, f2} set

    const i2: I2 = new D
    i2.foo(<arguments>) // call is valid if arguments fit first signature of {f3, f4} set

    const d: D = new D
    d.foo(<arguments>) // call is valid if arguments fit first signature of {f2, f3, f1, f4} set

.. index::
    overloaded interface
    declaration
    method
    argument
    signature

|

.. _Interface Method Overload Declarations:

Interface Method Overload Declarations
======================================

.. meta:
    frontend_status: None

*Interface method overload declaration* allows declaring an *overload alias*
as an interface member (see :ref:`Interface Members`)
for a set of interface methods (see :ref:`Interface Method Declarations`).

The syntax is presented below:

.. code-block:: abnf

    overloadInterfaceMethodDeclaration:
        'overload' identifier '{' identifier (',' identifier)* ','? '}'
        ;

The use of a *method overload declaration* is represented in the example below:

.. code-block:: typescript
   :linenos:

    interface I {
        foo(): void
        bar(n?: string): void
        overload goo { foo, bar }
    }

    function example(i: I) {
        i.goo()        // calls i.foo()
        i.goo("hello") // calls i.bar("hello")
        i.bar()        // explicit call: i.bar(undefined)
    }

.. index::
    interface method
    overload alias
    overload declaration
    interface
    syntax
    method overload declaration


An *overload alias* is used like an ordinary interface method, except that in
a call it is replaced at compile time by one of overloaded methods by using
the type of *object reference*.

A class that implements an interface with an *overload alias* usually implements
all interface methods, except those having a default body (see
:ref:`Default Interface Method Declarations`):

.. code-block:: typescript
   :linenos:

   // Using interface overload declaration
   class C implements I {
        foo(): void {/*body*/}
        bar(n?: string): void {/*body*/}
   }

   let c = new C()
   c.goo() // calls c.foo()

.. index::
    overload alias
    ordinary method
    interface method
    call
    compile time
    overloaded method
    object reference
    type
    class
    implementation

An interface *overload alias* can be redefined in a class. In this case, the
*overload declaration* in the class must contain all methods overloaded in the
interface. Otherwise, a :index:`compile-time error` occurs.

.. code-block:: typescript
   :linenos:

   class D implements I {
        foo(): void {/*body*/}
        bar(n?: string): void {/*body*/}
        overload goo( bar, foo) // order is changes
   }

   let d = new D()
   d.goo() // d.bar(undefined) is used, as it is the first appropriate method

An *overload alias* defined in a superinterface can be redefined in a
subinterface. In this case, the *overload declaration* of the subinterface
must contain all methods overloaded in superinterface. Otherwise, a
:index:`compile-time error` occurs.

The *overload alias* defined in superinterfaces must be redefined
in a subinterface if several *overload declarations* for the same alias are
inherited into the interface, otherwise a :index:`compile-time error` occurs.

.. index::
    overload alias
    interface
    class
    overload declaraton
    superinterface
    method
    subinterface
    overloaded method
    alias
    interface

.. code-block:: typescript
   :linenos:

    interface I1 {
        overload foo {f1, f2}
        // f1 and f2 are declared in I1
    }
    interface I2 {
        overload foo {f3, f4}
        // f3 and f4 are declared in I2
    }
    interface I3 extends I1, I2 {
       // compile-time error as no new overload for 'foo' is defined
    }
    interface I4 extends I1, I2 {
        overload foo { f4, f1, f3, f2 } // OK, as new overload is defined
    }
    interface I5 extends I1, I2 {
        overload foo { f1, f3 } // compile-time error as not all mehtods are included
    }


|

.. _Constructor Overload Declarations:

Constructor Overload Declarations
=================================

.. meta:
    frontend_status: None

*Constructor overload declaration* allows declaring an *overload alias*
and setting an order of constructors for a call in a new expression.

The syntax is presented below:

.. code-block:: abnf

    overloadConstructorDeclaration:
        'overload' 'constructor' '{' identifier (',' identifier)* ','? '}'
        ;

This feature can be used if there are more then one constructors declared
in the class, and maximum one of them is anonymous (see
:ref:`Constructor Names`).

Only a single *constructor overload declaration* is allowed in a class.
Otherwise, a :index:`compile-time error` occurs.

*Overload alias* for constructors is used the same way as anonymous constructor
(see :ref:`New Expressions`).

The use of a *constructor overload declaration* is represented in the example
below:

.. index::
    overload declaration
    constructor
    constructor overload declaration
    syntax
    declaration
    overload alias
    constructor
    call
    expression
    class

.. code-block:: typescript
   :linenos:

    class BigFloat {
        constructor fromNumber(n: number) {/*body1*/}
        constructor fromString(s: string) {/*body2*/}

        overload constructor { fromNumber, fromString }
    }

    new BigFloat(1)      // fromNumber is used
    new BigFloat("3.14") // fromString is used


If a class has an anonymous constructor it is implicitly placed at first
position in a list of overloaded constructors:

.. code-block:: typescript
   :linenos:

    class C {
        constructor () {/*body*/}
        constructor fromString(s?: string) {/*body*/}

        overload constructor { fromString }
    }

    new C()                // anonymous constructor is used
    new C("abc")           // fromString is used
    new C.fromString("aa") // fromString is explicitly used

.. index::
    constructor
    overloaded constructor

A :index:`compile-time error` occurs if both *constructor overload declaration*
and constructor *overload signature* (see
:ref:`Declarations with Overload Signatures`) are used:

.. code-block:: typescript
   :linenos:

    class C {
        // overload signature
        constructor (n: number)
        constructor (b: boolean)
        constructor (...x: Any[]) {/*body1*/}

        constructor fromString(s: string) {/*body2*/}

        // overload declaration
        overload constructor { fromString }
        // compile-time error: mix of both overload schemes
    }

.. index::
    constructor overload
    constructor
    overload signature
    declaration

|

.. _Overload Alias Name Same As Function Name:

Overload Alias Name Same As Function Name
=========================================

.. meta:
    frontend_status: None
    
A name of a top-level *overload declaration* can be the same as the name of an
overloaded function. This situation is represented in the following example:

.. code-block:: typescript
   :linenos:

    function foo(n: number): number {/*body1*/}
    function fooString(s: number): string {/*body2*/}

    overload foo {foo, fooString}

    foo(1)    // overload alias is used to call 'foo'
    foo("aa") // overload alias is used to call 'fooString'

Using an *overload alias* causes no ambiguity for it is considered
at the call site only, i.e., an *overload alias* is **not** considered in the
following situations:

- List of the overloaded entities (see :ref:`Function Overload Declarations`);

- :ref:`Function Reference`.

.. code-block:: typescript
   :linenos:

    function foo(n: number): number {/*body1*/}
    function fooString(s: number): string {/*body2*/}
    overload foo {foo, fooString}

    let func1 = foo // function 'foo' is used, not overload alias

If the name of an *overload alias* is the same as the name of a function that
is not listed as an overloaded function, then a :index:`compile-time error`
occurs as follows:

.. code-block:: typescript
   :linenos:

    function foo(n: number) {/*body1*/}
    function fooString(s: number) {/*body2*/}
    function fooBoolean(b: boolean) {/*body3*/}

    overload foo { // compile-time error
        fooBoolean, fooString
    }

|

.. _Overload Alias Name Same As Method Name:

Overload Alias Name Same As Method Name
=======================================

.. meta:
    frontend_status: None
    
A name of a class or interface *overload declaration* can be the same as the
name of an overloaded method. As one example, a method defined in a superclass
can be used as one of overloaded methods in a same-name subclass *overload
declaration*. This important case is represented by the following example:

.. code-block:: typescript
   :linenos:

    class C {
        foo(n: number): number {/*body*/}
    }
    class D implements C {
        fooString(s: number): string {/*body*/}

        overload foo {
            foo, // method 'foo' from C
            fooString
        }
    }

    let d = new D()
    let c: C = d

    d.foo(1)    // overload alias is used to call 'foo' from C
    d.foo("aa") // overload alias is used to call 'fooString' from D
    c.foo(1)    // method 'foo' from is called (no overload)

.. index::
    overload alias
    overload alias name
    method name
    overload declaration
    overloaded method
    superclass
    subclass

If names of a method and of an *overload alias* are the same, then the method
can be overriden as usual:

.. code-block:: typescript
   :linenos:

    class C {
        foo(n: number): number {/*body*/}
    }
    class D implements C {
        foo(n: number): number {/*body*/} // method is overriden
        fooString(s: number): string {/*body*/}

        overload foo { foo, fooString }
    }

This feature is also valid in interfaces, or in an interface and a class that
implements the interface:

.. index::
    overload alias
    overload alias name
    method name
    overriding
    interface
    class
    implementation

.. code-block:: typescript
   :linenos:

    interface I {
        foo(n: number): number {/*body*/}
    }
    interface J extends I {
        fooString(s: number): string
        overload foo { foo, fooString }
    }

    class K implements I {
        foo(n: number): number {/*body*/}
        fooString(s: number): string {/*body*/}

        overload foo { foo, fooString }
    }

Using an *overload alias* causes no ambiguity for it is considered
at the call site only. An *overload alias* is **not** considered in the
following situations:

- :ref:`Overriding`;

- List of the overloaded entities (see :ref:`Class Method Overload Declarations`
  and :ref:`Interface Method Overload Declarations`);

- :ref:`Method Reference`.

.. index::
    number
    string
    overload
    overload alias
    call site
    overriding
    overloaded entity
    method reference

.. code-block:: typescript
   :linenos:

    class C {
        foo(n: number): number {/*body*/}
    }

    class D implements C {
        fooString(s: number): string {/*body*/}

        overload foo { foo, fooString }
    }

    let d = new D()
    let c: C = d

    let func1 = c.foo // method 'foo' is used
    let func2 = d.foo // method 'foo' is used, not overload alias

A :index:`compile-time error` occurs if the name of an *overload alias*
is the same as the name of a method (with the same static or non-static
modifier) that is not listed as an overloaded method as follows:

.. code-block:: typescript
   :linenos:

    class C {
        foo(n: number) {/*body*/}
        fooString(s: number) {/*body*/}
        fooBoolean(b: boolean) {/*body*/}

        overload foo { // compile-time error
            fooBoolean, fooString
        }
    }

.. index::
    number
    string
    method
    overload alias
    static modifier
    non-static modifier
    overloaded method

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
   native keyword
   function
   native function
   native method
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


.. index::
   native method
   implementation
   platform-dependent code
   native keyword
   method body
   block
   method declaration
   abstract keyword
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

A :index:`compile-time error` occurs if a *native constructor* has a non-empty
body (see :ref:`Constructor Body`).

.. index::
   native constructor
   constructor
   constructor declaration
   platform-dependent code
   native keyword
   implementation
   non-empty body

|

.. _Classes Experimental:

Classes Experimental
********************

.. meta:
    frontend_status: Done

|

.. _Final Classes:

Final Classes
=============

.. meta:
    frontend_status: Done

A class can be declared ``final`` to prevent extension, i.e., a class declared
``final`` can have no subclasses. No method of a ``final`` class can be
overridden.

If a class type ``F`` expression is declared *final*, then only a class ``F``
object can be its value.

A :index:`compile-time error` occurs if the ``extends`` clause of a class
declaration contains another class that is ``final``.

.. index::
   final class
   class
   class type
   extension
   method
   overriding
   class
   class extension
   extends clause
   class declaration

|

.. _Final Methods:

Final Methods
=============

.. meta:
    frontend_status: Done

A method can be declared ``final`` to prevent it from being overridden (see
:ref:`Overriding Methods`) in subclasses.

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
   abstract keyword
   static keyword
   final keyword

|

.. _Constructor Names:

Constructor Names
=================

.. meta:
    frontend_status: None

A :ref:`Constructor Declaration` allows a developer to set a name used to
explicitly specify constructor to call in :ref:`New Expressions`:

.. code-block:: typescript
   :linenos:

    class Temperature{
        // use specified scale:
        constructor Celsius(n: double)    {/*body1*/}
        constructor Fahrenheit(n: double) {/*body2*/}
    }

    new Temperature.Celsius(0)
    new Temperature.Fahrenheit(32)

If a constructor has a name, then a direct application of the constructor in
a new expression implies using the constructor name explicitly:

.. index::
   constructor name
   constructor declaration
   constructor
   expression

.. code-block:: typescript
   :linenos:

    class X{
        constructor ctor1(p: number) {/*body1*/}
        constructor ctor2(p: string) {/*body2*/}
    }

    new X(1)      // compile-time error
    new X("abs")  // compile-time error
    new X.ctor1(1)      // OK
    new X.ctor2("abs")  // OK

A :index:`compile-time error` occurs if a constructor name is used as a named
reference (see :ref:`Named Reference`) in any expression.

.. code-block:: typescript
   :linenos:

    class X{ 
        constructor foo() {}
    }
    const func = X.foo // Compile-time error

The feature is also important for :ref:`Constructor Overload Declarations`.

.. index::
   constructor name
   named reference
   expression
   constructor overload declaration
   overload declaration

|

.. _Default Interface Method Declarations:

Default Interface Method Declarations
*************************************

.. meta:
    frontend_status: Done

The syntax of *interface default method* is presented below:

.. code-block:: abnf

    interfaceDefaultMethodDeclaration:
        'private'? identifier signature block
        ;

A default method can be explicitly declared ``private`` in an interface body.

A block of code that represents the body of a default method in an interface
provides a default implementation for any class if such a class does not
override the method that implements the interface.

.. index::
   method declaration
   interface method declaration
   default method
   private method
   implementation
   interface
   block
   class
   method body
   interface body
   default implementation
   overriding
   syntax

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
functionality to a class or an interface without having to inherit from the
class or to implement the interface. However, it can be used not only for
classes and interfaces but also for other types.

Moreover, :ref:`Function Types with Receiver` and
:ref:`Lambda Expressions with Receiver` can be defined and used to make the
code more flexible.

.. index::
   functionality
   function
   type
   accessor
   method
   function with receiver
   accessor with receiver
   interface
   inheritance
   class
   function type
   lambda expression
   lambda expression with receiver
   flexibility

|

.. _Functions with Receiver:

Functions with Receiver
=======================

.. meta:
    frontend_status: Done

*Function with receiver* declaration is a top-level declaration
(see :ref:`Top-Level Declarations`) that looks almost the same as
:ref:`Function Declarations`, except that the first parameter is mandatory,
and the keyword ``this`` is used as its name.

The syntax of *function with receiver* is presented below:

.. code-block:: abnf

    functionWithReceiverDeclaration:
        'function' identifier typeParameters? signatureWithReceiver block
        ;

    signatureWithReceiver:
        '(' receiverParameter (', ' parameterList)? ')' returnType?
        ;

    receiverParameter:
        annotationUsage? 'this' ':' type
        ;

.. index::
   function with receiver
   function with receiver declaration
   top-level declaration
   function declaration
   parameter
   this keyword

*Function with receiver* can be called in the following two ways:

-  Making a function call (see :ref:`Function Call Expression`), and
   passing the first parameter in the usual way;

-  Making a method call (see :ref:`Method Call Expression`) with
   no argument provided for the first parameter, and using the
   ``objectReference`` before the function name as the first argument.

.. index::
   function with receiver
   function call
   parameter
   method call
   method call expression
   argument
   object reference
   function name


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
body of a *function with receiver*. Only ``public`` members can be accessed:

.. index::
   this keyword
   function with receiver
   receiver type
   public member
   private member
   protected member
   access
   accessibility
   parameter

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

A :index:`compile-time error` occurs if the name of a *function with receiver*
is the same as the name of an accessible (see :ref:`Accessible`) instance
method or field of the receiver type, i.e., a *function with receiver* cannot
overload a method defined for the receiver type:

.. code-block:: typescript
   :linenos:

      class A {
          foo () { ...  }
      }

      function foo(this: A) { ... } // Compile-time error

*Function with receiver* cannot have the same name as a global function.
Otherwise, a :index:`compile-time error` occurs.

.. code-block:: typescript
   :linenos:

      function foo(this: A) { ... }
      function foo() { ... } // Compile-time error

*Function with receiver* can be generic as in the following example:

.. index::
   function with receiver
   access
   accessibility
   instance method
   method
   field
   public method
   overload
   compile-time error
   overloaded function
   receiver type
   generic function
   global function

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

A *function with receiver* can be defined in a compilation unit other than the
one that defines the receiver type. This is represented in the following
examples:

.. index::
   function with receiver
   static dispatch
   function call
   compile time
   receiver type
   declaration
   receiver
   derived class
   class
   compilation unit

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

The use of array type as *receiver type* is presented in the example below:

.. code-block:: typescript
   :linenos:

      function addElements(this: number[], ...s: number[]) {
       ...
      }

      let x: number[] = [1, 2]
      x.addElements(3, 4)

.. index::
   receiver type
   receiver parameter
   type
   function
   function type
   lambda with receiver
   interface type
   class type
   array type
   type parameter
   array type

|

.. _Accessors with Receiver:

Accessors with Receiver
=======================

.. meta:
    frontend_status: Done

*Accessor with receiver* declaration is a top-level declaration (see
:ref:`Top-Level Declarations`) that can be used as class or interface accessor
(see :ref:`Accessor Declarations`) for a specified receiver type:

The syntax of *accessor with receiver* is presented below:

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

.. index::
   accessor with receiver
   accessor with receiver declaration
   receiver type
   accessor declaration
   top-level declaration
   class accessor
   interface accessor
   get-accessor
   setter
   getter
   set-accessor
   receiver parameter
   return type
   field

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

.. index::
   accessor
   function call
   method call

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

The syntax of *function type with receiver* is presented below:

.. code-block:: abnf

    functionTypeWithReceiver:
        '(' receiverParameter (',' ftParameterList)? ')' ftReturnType
        ;

The type of a *receiver parameter* is called the *receiver type* (see
:ref:`Receiver Type`).

.. index::
   function type with receiver
   signature
   function with receiver
   lambda with receiver
   function type
   this keyword
   parameter
   receiver type
   receiver parameter

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

The usual rule of function type compatibility (see
:ref:`Subtyping for Function Types`) is applied to
*function type with receiver*, and parameter names are ignored.

.. index::
   function type with receiver
   generic
   function type
   compatibility
   subtyping
   parameter name

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

.. index::
   entity
   function type with receiver
   method call
   expression
   compile-time error


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

The syntax of *lambda expression with receiver* is presented below:

.. code-block:: abnf

    lambdaExpressionWithReceiver:
        annotationUsage?
        '(' receiverParameter (',' lambdaParameterList)? ')'
        returnType? '=>' lambdaBody
        ;

The usage of annotations is discussed in :ref:`Using Annotations`.

The keyword ``this`` can be used inside a *lambda expression with receiver*,
It corresponds to the first parameter:

.. index::
   lambda expression with receiver
   lambda expression
   instance
   function type with receiver
   lambda expression
   parameter
   this keyword
   annotation

.. code-block:: typescript
   :linenos:

      class A { name = "Bob" }

      let show = (this: A): void {
          console.log(this.name)
      }

Lambda can be called in two syntactical ways represented by the example below:

.. code-block:: typescript
   :linenos:

      class A {
        name: string
        constructor (n: string) {
            this.name = n
        }
      }

      function foo(aa: A[], f: (this: A) => void) {
        for (let a of aa) {
            a.f() // first way
            f (a) // second way
        }
      }

      let aa: A[] = [new A("aa"), new A("bb")]
      foo(aa, (this: A) => { console.log(this.name)} ) // output: "aa" "bb"

.. index::
   lambda
   syntax
   constructor
   function
   class

**Note**. If *lambda expression with receiver* is declared in a class or
interface, then ``this`` use in the lambda body refers to the first lambda
parameter and not to the surrounding class or interface. Any lambda call
outside a class has to use the ordinary syntax of arguments as represented by
the example below:


.. code-block:: typescript
   :linenos:

      class B {
        foo() { console.log ("foo() from B is called") }
      }
      class A {
        foo() { console.log ("foo() from A is called") }
        bar() {
            let lambda1 = (this: B): void => { this.foo() } // local lambda
            new B().lambda1()
        }
        lambda2 = (this: B): void => { this.foo() } // class field lambda
      }
      new A().bar() // Output is 'foo() from B is called'
      new A().lambda2 (new B) // Argument is to be provided in its usual place

      interface I {
         lambda: (this: B) => void // Property of the function type
      }
      function foo (i: I) {
         i.lambda(new B) // Argument is to be provided in its usual place
      }


.. index::
   lambda expression with receiver
   class
   interface
   this keyword
   lambda body
   lambda parameter
   surrounding class
   surrounding interface
   syntax

|

.. _Implicit this in Lambda with Receiver Body:

Implicit ``this`` in Lambda with Receiver Body
==============================================

.. meta:
    frontend_status: Done

Implicit ``this`` can be used in the body of *lambda expression with receiver*
when accessing the following:

- Instance methods, fields, and accessors of lambda receiver type (see
  :ref:`Receiver Type`); or
- Functions with receiver (see :ref:`Functions with Receiver`) of the same
  receiver type.

In other words, prefix ``this.`` in such cases can be omitted. This feature
is added to |LANG| to improve DSL support. It is represented in the following
examples:

.. index::
   lambda expression with receiver
   this
   lambda
   accessor
   DSL support
   prefix
   instance
   method
   field
   lambda receiver type
   receiver type
   prefix

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

The example above represents the use of implicit ``this`` when calling a
function with receiver:

.. index::
   lambda expression with receiver
   trailing lambda
   lambda signature
   inference
   context
   call
   function with receiver

.. code-block:: typescript
   :linenos:

     function bar(this: C) {}
     function otherBar(this: OtherClass) {}

     process() {
        bar()      // ok -  implicit 'this'
        otherBar() // compile-time error, wrong type of implicit 'this'
     }

If a simple name used in a lambda body can be resolved as instance method,
field, or accessor of the receiver type, and as another entity in the current
scope at the same time, then a :index:`compile-time error` occurs to prevent
ambiguity and improve readability.

.. index::
   simple name
   lambda body
   instance method
   field
   accessor
   receiver type
   entity
   scope

|

.. _Trailing Lambdas:

Trailing Lambdas
****************

.. meta:
    frontend_status: Done

The *trailing lambda* is a special form of notation for function
or method call when the last parameter of a function or a method is of
function type, and the argument is passed as a lambda using the
:ref:`Block` notation. The *trailing lambda* syntactically looks as follows:

.. index::
   trailing lambda
   notation
   function call
   method call
   parameter
   function type
   method
   parameter
   lambda
   block notation

.. code-block:: typescript
   :linenos:

      class A {
          foo (f: ()=>void) { ... }
      }

      let a = new A()
      a.foo() { console.log ("method lambda argument is activated") }
      // method foo receives last argument as the trailing lambda


The syntax of *trailing lambda* is presented below:

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

A block immediately after a call is always handled as *trailing lambda*.
A :index:`compile-time error` occurs if the last parameter of the called entity
is not of a function type.

The semicolon '``;``' separator can be used between a call and a block to
indicate that the block does not define a *trailing lambda*. When calling an
entity with the last optional parameter (see :ref:`Optional Parameters`), it
means that the call must use the default value of the parameter.

.. index::
   trailing lambda
   syntax
   parameter
   receiver parameter
   optional parameter
   lambda expression with receiver
   block
   function type
   lambda
   semicolon
   separator
   default value
   call

.. code-block:: typescript
   :linenos:

      function foo (f: ()=>void) { ... }

      foo() { console.log ("trailing lambda") }
      // 'foo' receives last argument as the trailing lambda

      function bar(f?: ()=>void) { ... }

      bar() { console.log ("trailing lambda") }
      // function 'bar' receives last argument as the trailing lambda,
      bar(); { console.log ("that is the block code") }
      // function 'bar' is called with 'f' parameter set to 'undefined'

      function goo(n: number) { ... }

      goo() { console.log("aa") } // compile-time error
      goo(); { console.log("aa") } // ok


If there are optional parameters in front of an optional function type parameter,
then calling such a function or method can skip optional arguments and keep the
trailing lambda only. This implies that the value of all skipped arguments is
``undefined``.

.. code-block:: typescript
   :linenos:

    function foo (p1?: number, p2?: string, f?: ()=>string) {
        console.log (p1, p2, f?.())
    }

    foo()                           // undefined undefined undefined
    foo() { return "lambda" }       // undefined undefined lambda
    foo(1) { return "lambda" }      // 1 undefined lambda
    foo(1, "a") { return "lambda" } // 1 a lambda

.. index::
   optional parameter
   optional argument
   trailing lambda
   argument
   operional function
   function
   method
   function call
   method call
   string
   lambda

|

.. _Libraries:

Libraries
*********

.. meta:
    frontend_status: None

The syntax of a *library description* is presented below:

.. code-block:: abnf

    libraryDescription:
        (importDirective|reExportDirective)*
        ;


*Libraries* are constructed from modules or other libraries. They can control
what is exported from a library by using the import-and-then-export scheme.

*Libraries* are stored in a file system or a database (see
:ref:`Compilation Units in Host System`).


.. index::
   library
   module
   construct
   file system
   database
   compilation unit
   host system


.. raw:: pdf

   PageBreak
