.. _Semantic Rules:

Semantic Rules
##############

This Chapter contains semantic rules to be used throughout the Specification
document.

Note that the description of the rules is more or less informal.

Some details are omitted to make the understanding easier. See the
formal language description for more information.

|

.. _Subtyping:

Subtyping
*********

The *subtype* relationships are the binary relationships of types.

The subtyping relation of *S* as a subtype of *T* is recorded as *S* <: *T*
and means that any object of type *S* can be safely used in any context
in place of an object of type *T*.

By the definition of the *S* <: *T*, the type *T* belongs to the set of
*supertypes* of type *S*.

Such set of *supertypes* includes all *direct supertypes* (see below) and all
their respective *direct supertypes*.

.. index::
   subtyping
   binary relationship
   object
   type
   direct supertype
   supertype

More formally speaking, the set is obtained by reflexive and transitive
closure over the direct supertype relation.

The *direct supertypes* of a non-generic class or interface type *C* are all
of the following:

-  The direct superclass of *C* (as mentioned in its extension clause, see
   :ref:`Class Extension Clause`), or the type *Object* if *C* has no extension
   clause specified.

-  The direct superinterfaces of *C* (as mentioned in *C*’ implementation
   clause, see :ref:`Class Implementation Clause`).

-  The type *Object* if *C* is an interface type with no direct superinterfaces
   (see :ref:`Superinterfaces and Subinterfaces`).


.. index::
   direct supertype
   direct superclass
   reflexive closure
   transitive closure
   non-generic class
   extension clause
   implementation clause
   superinterface
   Object
   direct superinterface
   class extension
   subinterface

The *direct supertypes* of the generic type *C* <*F*:sub:`1`,..., *F*:sub:`n`>
(for a generic class or interface type declaration *C* <*F*:sub:`1`,..., *F*:sub:`n`>
with *n*>0) are all of the following:

-  The direct superclass of *C* <*F*:sub:`1`,..., *F*:sub:`n`>.

-  The direct superinterfaces of *C* <*F*:sub:`1`,..., *F*:sub:`n`>.

-  The type *Object* if *C* <*F*:sub:`1`,..., *F*:sub:`n`> is a generic
   interface type with no direct superinterfaces.


The direct supertype of a type parameter is the type specified as the
constraint of that type parameter.

.. index::
   direct supertype
   generic type
   generic class
   interface type declaration
   direct superinterface
   type parameter
   superclass
   superinterface
   bound
   Object

|

.. _Least Upper Bound:

Least Upper Bound
*****************

.. meta:
    frontend_status: Done

The notion of the *least upper bound* (*LUB*) is used where a single type
must be found that is a common supertype for a set of reference types.

The word *least* means that the most specific supertype must be found,
and that there is no other shared supertype that is a subtype of LUB.

A single type is the LUB for itself.

In a set (*T*:sub:`1`,..., *T*:sub:`k`) that contains at least two types, the
LUB is determined as follows:

.. index::
   least upper bound (LUB)
   common supertype
   subtype

-  The set of supertypes *ST*:sub:`i` is determined for each type in the set;

-  The intersection of the *ST*:sub:`i` sets is calculated. Note that the
   intersection always contains the *Object* and thus cannot be empty.

-  The most specific type is selected from the intersection.


A compile-time error occurs of any types in the original set
(*T*:sub:`1`,..., *T*:sub:`k`) are not reference types.

.. index::
   compile-time error
   supertype
   intersection
   Object
   least upper bound (LUB)
   common supertype
   subtype
   most specific type
   reference type

|

.. _Override-Equivalent Signatures:

Override-Equivalent Signatures
******************************

Two functions, methods or constructors *M* and *N* have the *same signature* if
their names and type parameters (if any) are the same (see :ref:`Generic Declarations`),
and their formal parameter types are also the same (after the formal parameter
types of *N* are adapted to the type parameters of *M*).

Signatures *s*:sub:`1` and *s*:sub:`2` are *override-equivalent* only if
*s*:sub:`1` and *s*:sub:`2` are the same.

A compile-time error occurs if:

-  A package declares two functions with override-equivalent signatures.

-  A class declares the following:


   -  two methods with override-equivalent signatures.

   -  two constructors with override-equivalent signatures.

.. index::
   override-equivalent signature
   function
   method
   constructor
   signature
   type parameter
   generic declaration
   formal parameter type

|

.. _Overload Signature Compatibility:

Overload Signature Compatibility
********************************

|

.. _Compatibility Features:

Compatibility Features
**********************

To support smooth |TS| compatibility some features were added into |LANG|.
In most cases it is not recommended to use such features while doing
the |LANG| programming.

.. index::
   overload signature compatibility
   compatibility

|

.. _Extended Conditional Expressions:

Extended Conditional Expressions
================================

For better alignment with the semantics of conditional-and and conditional-or
expressions, an extended semantics from them is introduced for |LANG|. It
affects the semantics of conditional expressions (see :ref:`Conditional Expressions`),
the ``while`` and ``do`` statements (see :ref:`While Statements and Do Statements`),
the ``for``  statements (see :ref:`For Statements`), and ``if`` statements (see :ref:`if Statements`).
The approach is based on the concept of truthiness, which extends the Boolean
logic to operands and the result of non-Boolean types. The value of any valid
expression of non-void type can be treated as *Truthy* or *Falsy* depending on
the kind of the value type.

See the table below for details.

.. index::
   extended conditional expression
   semantic alignment
   conditional-and expression
   conditional-or expression
   conditional expression
   while statement
   do statement
   for statement
   if statement
   truthiness
   Boolean
   truthy
   falsy
   value type

+------------------+--------------------+--------------------+-----------------------------+
| Value type       | When Falsy         | When Truthy        | |LANG| code                 |
+==================+====================+====================+=============================+
| string           | "" empty string    | non-empty string   | if (stringExpr.length()==0) |
+------------------+--------------------+--------------------+-----------------------------+
| number           | 0 or NaN           | any other number   | if (numericExpr == 0)       |
+------------------+--------------------+--------------------+-----------------------------+
| nullishExpr      | == null            | != null            | if (nullishExpr == null)    |
+------------------+--------------------+--------------------+-----------------------------+
| nonNullishExpr   | never              | always             | n/a                         |
+------------------+--------------------+--------------------+-----------------------------+

The actual extended semantics of the conditional-and and conditional-or
expressions is described in the following truth tables (assuming 'A' and 'B'
are any valid expressions):

+---------+----------+
| A       | !A       |
+=========+==========+
| Falsy   | true     |
+---------+----------+
| Truthy  | false    |
+---------+----------+


+---------+---------+--------+---------+
| A       | B       | A && B | A || B  |
+=========+=========+========+=========+
| Falsy   | Falsy   | A      | B       |
+---------+---------+--------+---------+
| Falsy   | Truthy  | A      | B       |
+---------+---------+--------+---------+
| Truthy  | Falsy   | B      | A       |
+---------+---------+--------+---------+
| Truthy  | Truthy  | B      | A       |
+---------+---------+--------+---------+

The example below illustrates how this approach works in practice.
Non-zero number is truthy and loop works till it becomes falsy - zero.

.. code-block:: typescript
   :linenos:

    for (let i = 10; i; i--) {
       console.log (i)
    }
    /* And the output will be 
         10
         9
         8
         7
         6
         5
         4
         3
         2
         1
     */

.. index::
   truthy
   falsy
   NaN
   nullish expression
   numeric expression
   conditional-and expression
   conditional-or expression
   loop


.. raw:: pdf

   PageBreak


