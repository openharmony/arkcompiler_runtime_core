.. _Types:

Types
#####

.. meta:
    frontend_status: Partly

|LANG| is a statically typed language, and the type of every declared entity
and every expression is known at compile time because it is either set
explicitly by a developer, or inferred implicitly by the compiler.

There are two categories of types:

#. *value types* (see :ref:`Value Types`), and

#. *reference types* (see :ref:`Reference Types`).

The types integral to |LANG| are called *predefined types* (see
:ref:`Predefined Types`).

The types introduced, declared, and defined by a developer are called
*user-defined types*.
All *user-defined types* must always have a complete type definition
presented as source code in |LANG| language.

.. index::
   statically typed language
   expression
   compile time
   value type
   reference type
   implicit inference
   compiler
   predefined type
   user-defined type
   type definition
   source code

|

.. _Predefined Types:

Predefined Types
****************

.. meta:
    frontend_status: Partly
    todo: unsigned types are not supported yet
    todo: void type is not supported yet(void as reference type)

The set of the predefined types includes the following:

-  basic numeric value type: ``number`` 

-  additional numeric value types: ``byte``, ``short``, ``int``, ``long``,
   ``float``, and ``double``

-  boolean value type: ``boolean``

-  character value type: ``char``

-  predefined reference string type: ``string``

-  predefined reference BigInt type: ``bigint``

-  predefined class types: ``Object``, ``String``, ``never``, and ``void``

-  each predefined value type corresponds to a predefined class type that wraps
   the value type: ``Number``, ``Byte``, ``Short``, ``Int``, ``Long``,
   ``Float``, ``Double``, ``Char``, and ``Boolean``.


The numeric value types, character, and boolean value types are called
*primitive types*. Primitive type names are reserved, and cannot be used
for user-defined type names.

The type ``double`` is an alias to ``number``; the type ``Double`` is an alias
to ``Number``.

.. index::
   predefined type
   basic numeric type
   numeric type
   user-defined type
   type definition
   alias
   value type
   class type
   primitive type
   type alias
   wrapping

|

.. _User-Defined Types:

User-Defined Types
******************

.. meta:
    frontend_status: Done

The *user-defined* types include the following:

-  class types (see :ref:`Classes`),
-  interface types (see :ref:`Interfaces`),
-  enumeration types (see :ref:`Enumerations`),
-  array types (see :ref:`Array Types`),
-  function types (see :ref:`Function Types`),
-  union types (see :ref:`Union Types`), and
-  type parameters (see :ref:`Generic Parameters`).

.. index::
   user-defined type
   class type
   interface type
   enumeration type
   array type
   function type
   union type
   type parameter

|

.. _Types by Category:

Types by Category
*****************

.. meta:
    frontend_status: Partly
    todo: nullable types are not supported yet

The following table summarizes all |LANG| types:

+---------------------------------------------------+-----------------------------------+
|              Predefined Types                     |         User-Defined Types        |
+=========================+=========================+================+==================+
| Value Types             |     Reference Types     |   Value Types  |  Reference Types |
| (Primitive Types)       |                         |                |                  |
+-------------------------+-------------------------+----------------+------------------+
|  ``number``,            | ``Number``,             | ``enum`` types | class types,     |
|  ``byte``, ``short``,   | ``Byte``, ``Short``,    |                | interface types, |
|  ``int``, ``long``,     | ``Int``, ``Long``,      |                | array types,     |
|  ``float``, ``double``, | ``Float``, ``Double``,  |                | function types,  |
|  ``char``, ``boolean``  | ``Char``, ``Boolean``,  |                | union types,     |
|                         | ``Object``, ``never``,  |                | type parameters  |
|                         | ``void``, ``null``,     |                |                  |
|                         | ``bigint``, ``string``, |                |                  |
|                         | ``BigInt``, ``String``  |                |                  |
+-------------------------+-------------------------+----------------+------------------+

.. index::
   class type
   primitive type
   reference type
   value type
   interface type
   array type
   union type
   type parameter

|

.. _Using Types:

Using Types
***********

.. meta:
    frontend_status: Done

The following can refer to a type in a source code:

-  reserved name for a primitive type;
-  type reference for a named type (see :ref:`Named Types`) or a type alias
   (see :ref:`Type Alias Declaration`);
-  an in-place type definition for an array type (see :ref:`Array Types`), a
   function type (see :ref:`Function Types`), or a union type (see
   :ref:`Union Types`).

.. index::
   reserved name
   primitive type
   type alias
   type reference
   array type
   function type
   union type

.. code-block:: abnf

    type:
        predefinedType
        | typeReference
        | arrayType
        | functionType
        | unionType
        | '(' type ')'
        ;

Example:

.. code-block:: typescript
   :linenos:

    let b: boolean  // using primitive value type name
    let n: number   // using primitive value type name
    let o: Object   // using predefined class type name
    let a: number[] // using array type

Parentheses in types (where a type is a combination of array, function, or
union types) are used to specify the required type structure.
Without parentheses, the symbol ``'|'`` that constructs a union type
has the lowest precedence.

.. index::
   array type
   function type
   union type
   type structure
   symbol
   construct
   precedence

Example:

.. code-block:: typescript
   :linenos:

    // a nullable array with elements of type string:
    let a: string[] | null
    let s: string[] = []
    a = s    // ok
    a = null // ok, a is nullable

    // an array with elements whose types are string or null:
    let b: (string | null)[]
    b = null // error, b is an array and is not nullable
    b = ["aa", null] // ok

    // a function type that returns string or null
    let c: () => string | null
    c = null // error, c is not nullable
    c = (): string | null => { return null } // ok

    // (a function type that returns string) or null
    let d: (() => string) | null
    d = null // ok, d is nullable
    d = (): string => { return "hi" } // ok

|

.. _Named Types:

Named Types
***********

.. meta:
    frontend_status: None

Classes, interfaces, enumerations, and unions are the named types that are
introduced by class declarations (see :ref:`Classes`), interface
declarations (see :ref:`Interfaces`), enumeration declarations
(see :ref:`Enumerations`), and union declarations (see :ref:`Union Types`)
respectively.

Classes and interfaces that have type parameters are *generic types*
(see :ref:`Generics`). Named types without type parameters are
*non-generic types*.

The *type references* (see :ref:`Type References`) refer to named types by
specifying their type names, and (where applicable) type arguments to be
substituted for the type parameters of the named type.

.. index::
   named type
   class
   interface
   enumeration
   union
   class declaration
   interface declaration
   enumeration declaration
   union declaration
   generic type
   generics
   type argument
   type parameter

|

.. _Type References:

Type References
***************

A type reference uses a type name (either *simple* or *qualified*, see
:ref:`Names`), or a type alias (see :ref:`Type Alias Declaration`) to
refer to a type.

Optionally, each identifier in the name or alias can be followed by type
arguments (see :ref:`Type Arguments`) if the referred type is a class, or an
interface type.

.. index::
   type reference
   type name
   simple type name
   qualified type name
   identifier
   type alias
   type argument
   interface type

.. code-block:: abnf

    typeReference:
        typeReferencePart ('.' typeReferencePart)*
        ;

    typeReferencePart:
        Identifier typeArguments?
        ;

.. code-block:: typescript
   :linenos:

    let map: Map<string, number>

|

.. _Value Types:

Value Types
***********

.. meta:
    frontend_status: Partly
    todo: minor issue/feature - float/double literal parser in libc/libstdc++ can't parse everything, eg: 4.9E-324

The predefined type (see :ref:`Floating-Point Types and Operations`), integer
types (see :ref:`Integer Types and Operations`), floating-point types (see
:ref:`Floating-Point Types and Operations`), the boolean type (see
:ref:`Boolean Types and Operations`), the character type (see
:ref:`Character Types and Operations`), and user-defined enumeration types (see
:ref:`Enumerations`) are *value types*.

Such types’ values do *not* share state with other values.

.. index::
   value type
   predefined type
   integer type
   floating-point type
   boolean type
   character type
   enumeration

|

.. _Integer Types and Operations:

Integer Types and Operations
============================

+-----------+--------------------------------------------------------------------+--------------------------+
| Type      | Type's Set of Values                                               | Corresponding Class Type |
+===========+====================================================================+==========================+
| ``byte``  | All signed 8-bit integers (:math:`-2^7` to :math:`2^7-1`)          | ``Byte``                 |
+-----------+--------------------------------------------------------------------+--------------------------+
| ``short`` | All signed 16-bit integers (:math:`-2^{15}` to :math:`2^{15}-1`)   | ``Short``                |
+-----------+--------------------------------------------------------------------+--------------------------+
| ``int``   | All signed 32-bit integers (:math:`-2^{31}` to :math:`2^{31} - 1`) | ``Int``                  |
+-----------+--------------------------------------------------------------------+--------------------------+
| ``long``  | All signed 64-bit integers (:math:`-2^{63}` to :math:`2^{63} - 1`) | ``Long``                 |
+-----------+--------------------------------------------------------------------+--------------------------+
| ``bigint``| All integers with no limits                                        | ``BigInt``               |
+-----------+--------------------------------------------------------------------+--------------------------+

|LANG| provides a number of operators to act on integer values:

-  Comparison operators that produce a value of type *boolean*:

   +  Numerical comparison operators '<', '<=', '>' and '>=' (see :ref:`Numerical Comparison Operators`);
   +  Numerical equality operators '==' and '!=' (see :ref:`Value Equality Operators`);

-  Numerical operators that produce a value of type ``int`` or ``long``:

   + Unary plus '+' and minus '-' operators (see :ref:`Unary Plus` and :ref:`Unary Minus`);
   + Multiplicative operators '\*', '/' and '%' (see :ref:`Multiplicative Expressions`);
   + Additive operators '+' and '-' (see :ref:`Additive Expressions`);
   + Increment operator '++' used as prefix (see :ref:`Prefix Increment`)
     or postfix (see :ref:`Postfix Increment`);
   + Decrement operator '--' used as prefix (see :ref:`Prefix Decrement`)
     or postfix (see :ref:`Postfix Decrement`);
   + Signed and unsigned shift operators '<<', '>>' and '>>>' (see 
     :ref:`Shift Expressions`);
   + Bitwise complement operator '~' (see :ref:`Bitwise Complement`);
   + Integer bitwise operators '&', '^' and '\|' (see :ref:`Integer Bitwise Operators`);

-  Conditional operator '?:' (see :ref:`Conditional Expressions`);
-  Cast operator (see :ref:`Cast Expressions`), which converts an integer
   value to a value of any specified numeric type;
-  The string concatenation operator '+' (see :ref:`String Concatenation`),
   which (if a ``string`` operand and an integer operand are both available)
   converts the integer operand to a ``string`` (the character of a ``char``
   operand, or the decimal form of a ``byte``, ``short``, ``int`` or ``long``
   operand), and then creates a concatenation of two strings as a new ``string``.

.. index::
   byte
   short
   int
   long
   bigint
   Byte
   Short
   Int
   Long
   BigInt
   integer value
   comparison operator
   numerical comparison operator
   numerical equality operator
   equality operator
   numerical operator
   type reference
   type name
   simple type name
   qualified type name
   type alias
   type argument
   interface type
   postfix
   prefix
   unary operator
   unary operator
   additive operator
   multiplicative operator
   increment operator
   numerical comparison operator
   numerical equality operator
   decrement operator
   signed shift operator
   unsigned shift operator
   bitwise complement operator
   integer bitwise operator
   conditional operator
   cast operator
   integer value
   specific numeric type
   string concatenation operator
   operand

The classes ``Byte``, ``Short``, ``Int``, and ``Long`` also predefine other
constructors, methods and constants that are part of the |LANG| standard
library (see :ref:`Standard Library`).

If one operand is not ``long``, then the numeric promotion (see
:ref:`Predefined Numeric Types Conversions`) must be used first to widen
it to type ``long``.

If no operand is of type ``long``, then:

-  the operation implementation uses 32-bit precision;
-  the result of such numerical operator is of type ``int``.


If any operand is not ``int``, then the numeric promotion must be used
first to widen it to type ``int``.

Any integer type value can be cast to or from any numeric type.

Casts between the integer types and ``boolean`` are not possible.

The integer operators cannot indicate an overflow or an underflow.

An integer operator can throw errors (see :ref:`Errors Handling`) as follows:

-  An integer division operator '/' (see :ref:`Division`), and an
   integer remainder operator '%' (see :ref:`Remainder`) throw the
   ``ArithmeticError`` if their right-hand operand is zero.
-  An increment operator '++' and a decrement operator '--' (see
   :ref:`Additive Expressions`) throw the ``OutOfMemoryError`` if boxing
   conversion (see :ref:`Predefined Numeric Types Conversions`) is required
   but the available memory is not sufficient to perform it.

.. index::
   Byte
   Short
   Int
   Long
   constructor
   method
   constant
   operand
   numeric promotion
   predefined numeric types conversion
   numeric type
   widening
   long
   int
   boolean
   integer type
   numeric type
   cast
   operator
   overflow
   underflow
   division operator
   remainder operator
   error
   increment operator
   decrement operator
   additive expression
   boxing conversion

|

.. _Floating-Point Types and Operations:

Floating-Point Types and Operations
===================================

+-------------+--------------------------------------------------------+--------------------------+
| Type        | Type's Set of Values                                   | Corresponding Class Type |
+=============+========================================================+==========================+
| ``float``   | The set of all IEEE-754 32-bit floating-point numbers  | ``Float``                |
+-------------+--------------------------------------------------------+--------------------------+
| ``number``, | The set of all IEEE-754 64-bit floating-point numbers  | ``Number``               |
| ``double``  |                                                        | ``Double``               |
+-------------+--------------------------------------------------------+--------------------------+

A number of |LANG| operators act on the floating-point type values:

-  Comparison operators that produce a type ``boolean`` value:

   - Numerical comparison operators '<', '<=', '>' and '>=' (see
     :ref:`Numerical Comparison Operators`);
   - Numerical equality operators '==' and '!=' (see
     :ref:`Value Equality Operators`);

-  Numerical operators that produce values of type ``float`` or ``double``:

   + Unary plus '+' and minus '-' operators (see :ref:`Unary Plus` and :ref:`Unary Minus`);
   + Multiplicative operators '\*', '/' and '%' (see :ref:`Multiplicative Expressions`);
   + Additive operators '+' and '-' (see :ref:`Additive Expressions`);
   + Increment operator '++' used both as prefix (see :ref:`Prefix Increment`)
     and postfix (see :ref:`Postfix Increment`);
   + Decrement operator '--' used both as prefix (see :ref:`Prefix Decrement`)
     and postfix (see :ref:`Postfix Decrement`);
   + Signed and unsigned shift operators '<<', '>>' and '>>>' (see :ref:`Shift Expressions`);
   + Bitwise complement operator '~' (see :ref:`Bitwise Complement`);
   + Integer bitwise operators '&', '^' and '\|' (see :ref:`Integer Bitwise Operators`);
   
- Conditional operator '?:' (see :ref:`Conditional Expressions`);

-  Cast operator (see :ref:`Cast Expressions`), which converts a floating-point
   value to a value of any specified numeric type;
-  The string concatenation operator '+' (see :ref:`String Concatenation`),
   which (if both a ``string`` operand and a floating-point operand are present)
   converts the floating-point operand to a ``string`` with a value represented
   in the decimal form (without the loss of information), and then creates a
   concatenation of the two strings as a new ``string``.


.. index::
   floating-point type
   floating-point number
   operator
   numerical comparison operator
   numerical equality operator
   comparison operator
   boolean type
   numerical operator
   float
   double
   unary operator
   unary plus operator
   unary minus operator
   multiplicative operator
   additive operator
   prefix
   postfix
   increment operator
   decrement operator
   signed shift operator
   unsigned shift operator
   cast operator
   bitwise complement operator
   integer bitwise operator
   conditional operator
   string concatenation operator
   operand
   numeric type
   string

The classes ``Float`` and ``Double``, and the math library also predefine other
constructors, methods and constants which are part of the |LANG| standard
library (see :ref:`Standard Library`).

The operation is a floating-point operation if at least one of the operands in
a binary operator is of the floating-point type, even if the other operand is
integer.

If at least one operand of the numerical operator is of type ``double``, 
then the operation implementation uses 64-bit floating-point arithmetic, and
the result of the numerical operator is a value of type ``double``.

If the other operand is not ``double``, then the numeric promotion (see
:ref:`Predefined Numeric Types Conversions`) must be used first to widen it
to type ``double``.

If no operand is of type ``double``, then the operation implementation
uses 32-bit floating-point arithmetic, and the result of the numerical
operator is a value of type ``float``.

If the other operand is not ``float``, then the numeric promotion must be
used first to widen it to type ``float``.

Any floating-point type value can be cast to or from any numeric type.

.. index::
   Float
   Double
   class
   constructor
   method
   constant
   operation
   floating-point operation
   predefined numeric types conversion
   numeric type
   operand
   implementation
   float
   double
   numeric promotion
   numerical operator
   binary operator
   floating-point type

Casts between floating-point types and type ``boolean`` are not
possible.

Operators on floating-point numbers (except the remainder operator, see
:ref:`Remainder`) behave in compliance with the IEEE 754 Standard.
For example, |LANG| requires the support of IEEE 754 *denormalized*
floating-point numbers and *gradual underflow* that make it easier to prove
a particular numerical algorithms’ desirable properties. Floating-point
operations do not '*flush to zero*' if the calculated result is a
denormalized number.

|LANG| requires that floating-point arithmetic behave as if the floating-point
result of every floating-point operator is rounded to the result precision. An
*inexact* result is rounded to the representable value nearest to the infinitely
precise result; |LANG| uses the '*round to nearest*' principle (the default
rounding mode in IEEE-754), and prefers the representable value with the least
significant bit zero out of any two equally near representable values.

.. index::
   cast
   floating-point type
   floating-point number
   numeric type
   numeric types conversion
   widening
   operand
   implementation
   numeric promotion
   remainder operator
   gradual underflow
   flush to zero
   round to nearest
   rounding mode
   denormalized number

|LANG| uses '*round toward zero*' to convert a floating-point value to an
integer (see :ref:`Predefined Numeric Types Conversions`), which in this
case acts as if the number is truncated, and the mantissa bits discarded.
The format’s value closest to, and no greater in magnitude than the infinitely
precise result is chosen as the result of *rounding toward zero*.

A floating-point operation with overflow produces a signed infinity.

A floating-point operation with underflow produces a denormalized value
or a signed zero.

A floating-point operation with no mathematically definite result
produces NaN.

All numeric operations with a NaN operand result in NaN.

A floating-point operator (the increment '++' operator and decrement '--'
operator, see :ref:`Additive Expressions`) can throw the *OutOfMemoryError*
(see :ref:`Errors Handling`) if boxing conversion (see
:ref:`Predefined Numeric Types Conversions`) is needed but the available
memory is not sufficient to perform it.

.. index::
   round toward zero
   conversion
   predefined numeric types conversion
   numeric type
   truncation
   truncated number
   rounding toward zero
   denormalized value
   NaN
   numeric operation
   increment operator
   decrement operator
   error
   boxing conversion
   overflow
   underflow
   signed zero
   signed infinity
   integer
   floating-point operation
   floating-point operator
   floating-point value
   throw
   
|

.. _Numeric Types Hierarchies:

Numeric Types Hierarchies
=========================

Integer types and floating-point types are the *numeric types*.

Larger types include smaller types (of their values):

-  ``double`` > ``float`` > ``long`` > ``int`` > ``short`` > ``byte``
-  ``bigint`` > ``long`` > ``int`` > ``short`` > ``byte``

A smaller type’s value can be assigned to a larger type’s variable.

The type *bigint* is an exception because any integer value or some string
type values can be converted into *bigint* by using built-in functions
*BigInt(anyInteger: long): bigint* or 
*BigInt(anyIntegerString: string): bigint*.

.. index::
   numeric type
   exception
   floating-point type
   assignment
   variable
   double
   float
   long
   int
   short
   byte
   bigint
   long
   int
   short
   byte
   bigint
   string
   BigInt


|

.. _Boolean Types and Operations:

Boolean Types and Operations
============================

The type ``boolean`` represents logical values ``true`` and ``false`` that
correspond to the class type ``Boolean``.

The boolean operators are as follows:

-  Relational operators '==' and '!=' (see :ref:`Relational Expressions`);
-  Logical complement operator '!' (see :ref:`Logical Complement`);
-  Logical operators '&', '^' and '|' (see :ref:`Integer Bitwise Operators`);
-  Conditional-and operator '&&' (see :ref:`Conditional-And Expression`) and
   conditional-or operator '||' (see :ref:`Conditional-Or Expression`);
-  Conditional operator '?:' (see :ref:`Conditional Expressions`);
-  String concatenation operator '+' (see :ref:`String Concatenation`) 
   that converts the boolean operand to a string (``true`` or ``false``),
   and then creates a concatenation of the two strings as a new string.


The conversion of an integer or floating-point expression *x* to a boolean
value must follow the *C* language convention that any nonzero value is
converted to ``true``, and the value of zero is converted to ``false``.
In other words, the result of conversion of expression *x* to the boolean
type is always the same as the result of comparison *x != 0*.

.. index::
   boolean
   Boolean
   relational operator
   complement operator
   logical operator
   conditional-and operator
   conditional-or operator
   conditional operator
   string concatenation operator
   floating-point expression
   comparison
   conversion

|

.. _Character Types and Operations:

Character Types and Operations
==============================

+-----------+--------------------------------------------------------------------+--------------------------+
| Type      | Type's Set of Values                                               | Corresponding Class Type |
+===========+====================================================================+==========================+
| ``char``  | Symbols with codes from \U+0000 to \U+FFFF inclusive, that is,     | ``Char``                 |
|           | from 0 to 65,535                                                   |                          |
+-----------+--------------------------------------------------------------------+--------------------------+

|LANG| provides a number of operators to act on character values:

-  Comparison operators that produce a value of type *boolean*:

   +  Character comparison operators '<', '<=', '>' and '>=' (see :ref:`Numerical Comparison Operators`);
   +  Character equality operators '==' and '!=' (see :ref:`Value Equality Operators`);

-  Character operators that produce a value of type ``char``;

   + Unary plus '+' and minus '-' operators (see :ref:`Unary Plus` and :ref:`Unary Minus`);
   + Additive operators '+' and '-' (see :ref:`Additive Expressions`);
   + Increment operator '++' used both as prefix (see :ref:`Prefix Increment`)
     and postfix (see :ref:`Postfix Increment`);
   + Decrement operator '--' used both as prefix (see :ref:`Prefix Decrement`)
     and postfix (see :ref:`Postfix Decrement`);

-  Conditional operator '?:' (see :ref:`Conditional Expressions`);
-  The string concatenation operator '+' (see :ref:`String Concatenation`),
   which (if a string operand and character operand are available) converts
   the character operand to a string and then creates a concatenation of the
   two strings as a new string.

The class ``Char`` also provides other constructors, methods and constants
which are part of the |LANG| standard library (see :ref:`Standard Library`).

.. index::
   char
   Char
   boolean
   comparison operator
   equality operator
   unary operator
   additive operator
   increment operator
   postfix
   prefix
   decrement operator
   conditional operator
   concatenation operator
   operand
   constructor
   method
   constant

|

.. _Reference Types:

Reference Types
***************

.. meta:
    frontend_status: Partly

*Reference types* can be of the following kinds:

-  class types (see :ref:`Classes`);
-  interface types (see :ref:`Interfaces`);
-  array types (see :ref:`Array Types`);
-  function types (see :ref:`Function Types`);
-  union types (see :ref:`Union Types`);
-  string type (see :ref:`String Type`);
-  never type (see :ref:`never Type`), null type (see :ref:`null Type`), 
   undefined type (see :ref:`undefined Type`), 
   void type (see :ref:`void Type`); and
-  type parameters (see :ref:`Generic Parameters`).

.. index::
   class type
   interface type
   array type
   function type
   union type
   string type
   never type
   undefined type
   void type
   type parameter

|

.. _Objects:

Objects
=======

An *object* can be a *class instance*, *function instance*, or an *array*.

The pointers to these objects are called *references* or reference values.

A class instance creation expression (see :ref:`New Expressions`) explicitly
creates a class instance.

Referring to a declared function by its name, qualified name, or lambda
expression (see :ref:`Lambda Expressions`) explicitly creates a function
instance.

An array creation expression explicitly creates an array (see
:ref:`Array Creation Expressions`).

A string literal initialization explicitly creates a string.

Other expressions can implicitly create a class instance (see
:ref:`New Expressions`), or an array (see :ref:`Array Creation Expressions`).

.. index::
   object
   instance
   array
   reference value
   function instance
   class instance
   reference
   lambda expression
   qualified name
   name
   declared function
   array creation
   expression
   literal
   initialization

The operators on references to objects of class, interface and type
parameter are as follows:

-  Field access that uses a qualified name or a field access expression (see
   :ref:`Field Access Expressions`);
-  Call expression (see :ref:`Method Call Expression` and :ref:`Function Call Expression`);
-  Cast expression (see :ref:`Cast Expressions`);
-  String concatenation operator (see :ref:`String Concatenation`)
   that---given a *string* operand and a reference---calls the *toString*
   method of the referenced object, converts the reference to a *string*,
   and creates a concatenation of the two strings as a new ``string``;
-  ``instanceof`` operator (see :ref:`InstanceOf Expression`);
-  Reference equality operators '==' and '!=' (see :ref:`Reference Equality Operators`);
-  Conditional operator '?:' (see :ref:`Conditional Expressions`).


Multiple references to an object are possible. Most objects have state. The
state is stored in the field if an object is an instance of class, or in a
variable that is an element of an array object. If two variables contain
references to the same object, and the state of that object is modified in
one variable’s reference, then the state so modified can be seen in the
other variable’s reference.

.. index::
   operator
   object
   class
   interface
   type parameter
   field access
   qualified name
   method call expression
   function call expression
   field access expression
   cast expression
   call expression
   concatenation operator
   conversion
   reference equality operator
   conditional operator
   state
   array element
   variable
   field
   instance
   reference

|

.. _Object Class Type:

``Object`` Class Type
=====================

.. meta:
    frontend_status: Done

The class ``Object`` is a supertype of all other classes, interfaces, string,
arrays, unions, function types, and enum types.

Thus all of them inherit (see :ref:`Inheritance`) the class ``Object``’s
methods as summarized below:

-  The method ``equals`` defines a notion of object equality, which is
   based on the comparison of values rather than references.
-  The method ``hashCode`` returns the integer hash code of the object.
-  The method ``toString`` returns a string representation of the
   object.

.. index::
   class type
   function call expression
   field access expression
   cast expression
   concatenation operator
   operand
   reference
   method
   object
   object class type
   call expression   
   instanceof operator
   supertype
   interface
   array
   inheritance
   hash code

|

.. _string Type:

``string`` Type
===============

.. meta:
    frontend_status: Done

The ``string`` type stores sequences of characters as Unicode UTF-16 code
units.

*String* is the predefined type.

The type ``string`` includes all string literals, e.g., ``'abc'``. The value
of a string object cannot be changed after it is created, i.e., a string
object is immutable, and can be shared.

If the result is not a constant expression (see :ref:`Constant Expressions`),
then the string concatenation operator '+' (see :ref:`String Concatenation`)
implicitly creates a new string object.

While the ``String`` type is the alias to ``string``, it is recommended to
use ``string`` in all cases.

.. index::
   string type
   Unicode code unit
   compiler
   predefined type
   extended semantics
   literal
   constant expression
   concatenation operator
   alias

|

.. _never Type:

``never`` Type
==============

The class ``never`` is a subclass (see :ref:`Subtyping`) of any other class.

The ``never`` class has no instances, and is used to represent values that
never exist (a function with this return type never returns but only throws
an error or exception).

.. index::
   subtyping
   class
   instance
   error
   exception
   function
   return type
   string literal
   string object
   constant expression
   concatenation operator
   alias
   subclass
   instance
   value

|

.. _void Type:

``void`` Type
=============

The ``void`` type has a single value, and is typically used as the return
type if:

-  a function returns no value of another type (similarly to type ``Unit``
   in some other languages), or
-  ``void`` is the type argument that instantiates a generic type where a
   specific type parameter argument value is irrelevant.

.. index::
   error
   exception
   return type
   type argument
   instantiation
   generic type
   type parameter argument


The code below:

.. code-block:: typescript
   :linenos:

      function foo ()

is equivalent to the following:

.. code-block:: typescript
   :linenos:

      function foo (): void

      class A<G>
      let a = new A<void>()

|

.. _Array Types:

Array Types
===========

.. meta:
    frontend_status: Partly
    todo: Inherited methods from baseclass - Object can't be invoked now

*Array type* is the built-in type which is characterized by the following:

-  any object of array type contains elements indexed by integer position
   starting from 0;
-  access to any array element is performed at the same time;
-  while being passed to non-|LANG| environment, an array is represented
   as a contiguous memory location;
-  types of all arrays elements are upper-bounded by the element type
   specified in the array declaration.

.. index::
   array type
   array element
   access
   array

Two basic operations with array elements are taking elements out of and
putting elements into an array. Both operations use the ``[ ]`` operator
and index expression for that.
Another important operation implemented as the read-only field ``length``
which allows to know the number of elements in the array.

The syntax for built-in array type is below:

.. index::
   array element
   index expression
   operator

.. code-block:: abnf

    arrayType:
       type '[' ']'
       ;

A family of array types that are part of the standard library is described
in full in the library documentation. A common characteristic of such types
is that the ``[ ]`` operator can be applied to variables of all array types
and their derived ones as well.

.. index::
   array type
   variable
   operator

Examples:

.. code-block:: typescript
   :linenos:

    let a : number[] = [0, 0, 0, 0, 0] 
      /* allocate array with 5 elements of type number */
    a[1] = 7 /* put 7 as the 2nd element of the array, index of this element is 1 */
    let y = a[4] /* get the last element of array 'a' */
    let count = a.length // get the number of array elements

A type alias can set a name for an array type (see :ref:`Type Alias Declaration`):

.. code-block:: typescript
   :linenos:

    type Matrix = number[][] /* Two-dimensional array */

As an object, an array is assignable to a variable of type Object:

.. code-block:: typescript
   :linenos:

    let a: number[] = [1, 2, 3]
    let o: Object = a

.. index::
   alias
   array type
   object
   array
   assignment
   variable

|

.. _Function Types:

Function Types
==============

.. meta:
    frontend_status: Done

A *function type* can be used to express the expected signature of a function.
A function type consists of the following:

-  list of parameters (can be empty);
-  optional return type;
-  optional keyword ``throws``.

.. index::
   array element
   type alias
   array type
   type Object
   keyword throws
   function type
   signature

.. code-block:: abnf

    functionType:
        '(' ftParameterList? ')' ftReturnType 'throws'?
        ;

    ftParameterList:
        ftParameter (',' ftParameter)\* (',' restParameter)?
        | restParameter
        ;

    ftParameter:
        identifier ':' type
        ;

    restParameter:
        '...' ftParameter
        ;

    ftReturnType:
        '=>' type
        ;

The *rest* parameter is described in :ref:`Rest Parameter`.

.. index::
   rest parameter

.. code-block:: typescript
   :linenos:

    let binaryOp: (x: number, y: number) => number
    function evaluate(f: (x: number, y: number) => number) { }

A type alias can set a name for a function type (see :ref:`Type Alias Declaration`).

.. index::
   type alias
   function type

.. code-block:: typescript
   :linenos:

    type BinaryOp = (x: number, y: number) => number
    let op: BinaryOp

The type ``void`` (see :ref:`void Type`) is the implied return type of a
function type if *ftReturnType* is omitted.

If the function type contains the '``throws``' mark (see
:ref:`Throwing Functions`), then it is the *throwing function type* .

Function types assignability is described in :ref:`Assignment and Call Contexts`,
and conversions in :ref:`Function Types Conversions`.

.. index::
   function type
   return type
   type void
   throwing function
   throwing function type
   throws mark

|

.. _null Type:

``null`` Type
=============

The ``null`` type’s single value is represented by the keyword  ``null``
(see :ref:`Null Literal`).

Using the type ``null`` as type annotation is not recommended, except in
nullish types (see :ref:`Nullish Types`).

.. index::
   type null
   null literal
   keyword null
   type annotation
   nullish type

|

.. _undefined Type:

``undefined`` Type
==================

The ``undefined`` type’s single value is represented by the keyword
``undefined`` (see :ref:`Undefined Literal`).

Using the type ``undefined`` as type annotation is not recommended,
except in nullish types, see :ref:`Nullish Types`.

.. index::
   type undefined
   keyword undefined
   literal
   annotation
   nullish type

|

.. _Union Types:

Union Types
===========

.. code-block:: abnf

    unionType:
        type|literal ('|' type|literal)*
        ;

A *union* type is a reference type created as a combination of other
types or values. Values of union types can be valid values of all
types and values of literals the union was created from.

A :index:`compile-time error` occurs if the type in the right-hand-side
of the union type declaration leads to a circular reference.

If a *union* uses one of primitive types (see Primitive types in
:ref:`Types by Category`), then automatic boxing occurs to keep the reference
nature of the type.

The reduced form of union types allows defining a type which has only
one value as in the following example:

.. index::
   union type
   reference type
   circular reference
   union
   primitive type
   literal
   primitive type
   automatic boxing
   
.. code-block:: typescript
   :linenos:

   type T = 3
   let t1: T = 3 // OK
   let t2: T = 2 // Compile-time error

Below is a typical example of unit types usage:

.. code-block:: typescript
   :linenos:

    class Cat {
      // ...
    }
    class Dog {
      // ...
    }
    class Frog {
      // ...
    }
    type Animal = Cat | Dog | Frog | number
    // Cat, Dog, and Frog are some types (class or interface ones)

    let animal: Animal = new Cat()
    animal = new Frog() 
    animal = 42
    // One may assign the variable of the union type with any valid value

Different mechanisms can be used to get values of particular types from a union.

For example:

.. code-block:: typescript
   :linenos:

    class Cat { sleep () {}; meow () {} }
    class Dog { sleep () {}; bark () {} }
    class Frog { sleep () {}; leap () {} }

    type Animal = Cat | Dog | Frog | number

    let animal: Animal = new Cat()
    if (animal instanceof Frog) { // animal is of type Frog here
        let frog: Frog = animal as Frog // Use as conversion here
        animal.leap()
        frog.leap()
        // As a result frog leaps twice
    }

    animal.sleep () // Any animal can sleep


Below is an example for primitive types:

.. code-block:: typescript
   :linenos:

    type Primitive = number | boolean
    let p: Primitive = 7
    if (p instanceof Number) { // type of 'p' is Number here
       let i: number = p as number // Explicit conversion from Primitive to number
    }

For values:

.. code-block:: typescript
   :linenos:

    type BMW_ModelCode = 325 | 530 | 735
    let bmw: BMW_ModelCode = 325
    if (bmw == 325){
       bmw = 525
    } else if (bmw == 525){
       bmw = 735
    } else {
       // pension :-)
    }

|

.. _Union Types Normalization:

Union Types Normalization
-------------------------

Union types normalization allows minimizing the number of types and literals
within a union type while keeping the type's safety. Some types or literals
can also be replaced for more general types.

Formally, union type *T*:sub:`1` | ... | *T*:sub:`N`, where N > 1, can be
reduced to type *U*:sub:`1` | ... | *U*:sub:`M`, where M <= N, or even to
non-union type *V*.

The normalization process presumes performing the following steps one after
another:

.. index::
   union type
   non-union type
   normalization
   literal

#. Identical types within the union type are replaced for a single type.
#. Identical literals within the union type are replaced for a single literal.
#. If at least one type in the union is *Object* then the whole union type is
   reduced to *Object* type.
#. Otherwise, if there is at least one numeric type among the numeric union
   types or numeric literals, then all such types or literals are replaced
   for type *number*.
#. Unless any of the above is true:

-  If there are two types *T1* and *T2* within the union type, and *T1* is
   compatible (see :ref:`Compatible Types`) with *T2*, then only *T2* remains
   in the union type.
-  if *T2* is compatible (see :ref:`Compatible Types`) with *T1*, then *T1*
   remains in the union type.
-  The last step is performed recursively until no more mutually compatible
   types remain, or the union type is reduced to a single type.

.. index::
   union type
   literal non-union type
   normalization
   literal
   Object type
   numeric union type
   compatible type
   type compatibility

As a result of this process, the normalized union type remains.

The below examples highlight how this works:

.. code-block:: typescript
   :linenos:

    1 | 1 | 1  =>  1
    number | number => number
    1 | number | number => number
    1 | string | number => string | number
    1 | Object => Object
    AnyType | Object | AnyType => Object
    class Base {}
    class Derived1 extends Base {}
    class Derived2 extends Base {}   
    Base | Derived1 => Base
    Derived1 | Derived2 => Derived1 | Derived2 // No changes!
    int | double | short => number

The |LANG| compiler applies such normalization while processing union types
and handling the type inference for array literals (see
:ref:`Type Inference from Types of Elements`).

.. index::
   union type
   normalization
   array literal
   type inference
   array literal

|

.. _Nullish Types:

Nullish Types
==============

.. meta:
    frontend_status: Partly

|LANG| has nullish types that are in fact a special form of the union types
(see :ref:`Union Types`).

.. code-block:: abnf

    nullishType:
        type '|' ( 'null' | 'undefined' )
        ;

All predefined and user-defined type declarations create non-nullish
types that cannot have a ``null`` or ``undefined`` value at runtime.

Use *T* \| ``null`` or *T* \| ``undefined`` as the type to specify a
nullish version of type *T*. 

A variable declared to have the type *T* \| ``null`` can hold values of
type *T* and its derived types, or the value ``null``. Such type is called
a *nullable type*.

A variable declared to have the type *T* \| ``undefined`` can hold values
of type *T* and its derived types, or the value ``undefined``.

A :index:`compile-time error` occurs if *T* is not a reference type.
A reference which is ``null`` or ``undefined`` is called *nullish* value.

An operation that is safe with no regard to the presence or absence of 
nullish values (e.g., re-assigning one nullable value to another) can
be used as is for nullish types.

The following nullish-safe options exist for operations on nullish type *T*
that can potentially violate null safety (e.g., access to a property):

.. index::
   union type
   type inference
   array literal
   nullish type
   nullable type
   non-nullish type
   predefined type declaration
   user-defined type declaration
   undefined value
   runtime
   derived type
   reference type
   nullish value
   nullish-safe option
   null safety
   access
   assignment
   re-assignment

-  Using safe operations:

   -  safe method call (for details see :ref:`Method Call Expression`);
   -  safe field access expression (for details see :ref:`Field Access Expressions`);
   -  safe indexing expression (for details see :ref:``Indexing Expression``);
   -  safe function call (for details see :ref:`Function Call Expression`);

-  Downcasting from *T* \| ``null`` or *T* \| ``undefined`` to *T*:

   -  cast expression (for details see :ref:`Cast Expressions`);
   -  ensure-not-nullish expression (for details see :ref:`Ensure Not-Nullish Expressions`);

-  Supplying a default value to use if a nullish value is present:

   -  nullish-coalescing expression (for details see :ref:`Nullish-Coalescing Expression`).

.. index::
   method call
   field access expression
   indexing expression
   function call
   downcasting
   cast expression
   ensure-not-nullish expression
   nullish-coalescing expression
   nullish value
   cast expression

|

.. _DynamicObject Type:

DynamicObject Type
==================

.. meta:
    frontend_status: None

The interface *DynamicObject* is used to provide seamless interoperability
with dynamic languages as Javascript and TypeScript and
to support advanced language features such as *dynamic import*
(see :ref:`Dynamic Import`).

This interface is defined in the standard library (see
:ref:`Standard Library`). 

It is a common interface for a set of wrappers 
(also defined in the standard library) that provide access 
to underlying objects. 
The *DynamicObject* instance cannot be created directly,
only an instance of a specific wrapper object can be instantiated. 
For example, the result of *dynamic import* expression (see :ref:`Dynamic Import Expression`) 
is the instance of dymanic object implementation class that wraps an object that contains 
exported entities of the imported module.

The DynamicObject is the predefined type and the compiler treats 
some operations applied to an object of this type in a special way:

- field access
- method call
- indexing access
- new
- cast 

.. _DynamicObject Field Access:

DynamicObject Field Access
--------------------------

A field access expression *D.F* where *D* is of type DynamicObject is treated 
as the access to property of underlying object.

.. code-block:: typescript
   :linenos:

   function foo(d: DynamicObject) {
      console.log(d.f1) // access of the property named "f1" of underlying object
      d.f1 = 5 // set a value of the property named "f1"
   }

A wrapper can raise an error if a property with the specified name is not exist
in the undefined object or if the type of assigned value is not compatible with
the type of property.


    
|

.. _Default Values for Types:

Default Values for Types
************************

.. meta:
    frontend_status: Partly

**Note**: this is part of the experimental mode of the |LANG|.

Some types use so-called *default values* for variables without explicit
initialization (see :ref:`Variable Declarations`), including the following:

-  all primitive types (see the table below).
-  nullable reference types with the default value ``null`` (see :ref:`Literals`).


All other types, including reference types and enumeration types, have no
default values.

A variable of such types must be initialized explicitly with a value
before its first use.

The primitive types’ default values are as follows:

.. index::
   default value
   variable
   explicit initialization
   nullable reference type
   primitive type
   reference type
   enumeration type

+----------------+--------------------+
|    Data Type   |   Default Value    |
+================+====================+
| ``number``     | 0 as ``number``    |
+----------------+--------------------+
| ``byte``       | 0 as ``byte``      |
+----------------+--------------------+
| ``short``      | 0 as ``short``     |
+----------------+--------------------+
| ``int``        | 0 as ``int``       |
+----------------+--------------------+
| ``long``       | 0 as ``long``      |
+----------------+--------------------+
| ``float``      | +0.0 as ``float``  |
+----------------+--------------------+
| ``double``     | +0.0 as ``double`` |
+----------------+--------------------+
| ``char``       | '\u0000'           |
+----------------+--------------------+
| ``boolean``    | ``false``          |
+----------------+--------------------+

.. index::
   number
   byte
   short
   int
   long
   float
   double
   char
   boolean

.. raw:: pdf

   PageBreak


