..
    Copyright (c) 2021-2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _RT Language Representation in Binary File:

Language Representation in Binary File
**************************************

This chapter describes how the |LANG| compiler translates different language
types and constructs into bytecode.

.. note::
   The |LANG| *type erasure* rules take precedence over the translation
   rules discussed in this chapter. It means that the *type erasure* rules are
   applied first for any |LANG| type ``T``, and then the *effective type* of ``T``
   is translated into the binary representation in accordance with the rules below.

|

.. _RT Value Types:

Value Types
===========

The |LANG| *value types* can be represented in a binary file in **one** of the following two forms depending on the context:

- ``PrimitiveType`` in :ref:`Type Descriptor <RT Type Descriptor>`, :ref:`FieldType <RT FieldType>`, or :ref:`Shorty <RT Shorty>`; or
- Corresponding predefined *reference type*.

The :ref:`Type Descriptor <RT Type Descriptor>` of a *reference type* type is ``RefType``.
Mapping between the |LANG| type, *primitive type* and :ref:`Fully Qualified Name <RT Fully Qualified Name>`
of a *reference type* is presented in the table below:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Primitive
     - Reference
   * - boolean
     - ``u1``
     - ``std.core.Boolean``
   * - char
     - ``u16``
     - ``std.core.Char``
   * - byte
     - ``i8``
     - ``std.core.Byte``
   * - short
     - ``i16``
     - ``std.core.Short``
   * - int
     - ``i32``
     - ``std.core.Int``
   * - long
     - ``i64``
     - ``std.core.Long``
   * - float
     - ``f32``
     - ``std.core.Float``
   * - number,double
     - ``f64``
     - ``std.core.Double``

.. _RT Selecting Between Primitive And Reference:

Selecting Between Primitive And Reference
-----------------------------------------

Each time one of *value types* is used, the |LANG| compiler tries
to lower it to its corresponding *primitive type* representation by default.
However, *reference type* representation is required in some cases.
A comprehensive list of such scenarios is presented below:

- *Component types* of a *union type* must be *reference types*, i.e., any
  *value type* in a *union type* is represented as a *reference type* in a binary
  file.
- If a *value type* is used as a constraint of a generic parameter, then it is also
  represented as a *reference type* in a binary file.
- If a default value for a parameter of a method or a function is of a *value type*,
  then the type of that parameter is represented with *reference type* in a binary file.

|

.. _RT Type Any:

Type ``Any``
============

The |LANG| type ``Any`` is represented in a binary file with the predefined
:ref:`Class <RT Class>`. The :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is ``RefType`` with
:ref:`Fully Qualified Name <RT Fully Qualified Name>` ``Y``.

|

.. _RT Type Object:

Type ``Object``
===============

The |LANG| type ``Object`` is represented in a binary file with the predefined
:ref:`Class <RT Class>`. The :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is ``RefType`` with
:ref:`Fully Qualified Name <RT Fully Qualified Name>` ``std.core.Object``.

|

.. _RT Type never:

Type ``never``
==============

The |LANG| type ``never`` is represented in a binary file with the predefined
:ref:`Class <RT Class>`. The :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is ``RefType`` with
:ref:`Fully Qualified Name <RT Fully Qualified Name>` ``N``.

|

.. _RT Type void:

Type ``void``
=============

The |LANG| type ``void`` is represented in a binary file as the primitive type ``void``
in the :ref:`Shorty <RT Shorty>`.

|

.. _RT Type null:

Type ``null``
=============

The |LANG| type ``null`` is represented in a binary file with the predefined
:ref:`Class <RT Class>`. The :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is ``RefType`` with
:ref:`Fully Qualified Name <RT Fully Qualified Name>` ``std.core.Null``.

|

.. _RT Type string:

Type ``string``
===============

The |LANG| type ``string`` is represented in a binary file with the predefined
:ref:`Class <RT Class>`. The :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is ``RefType`` with
:ref:`Fully Qualified Name <RT Fully Qualified Name>` ``std.core.String``.

|

.. _RT Type bigint:

Type ``bigint``
===============

The |LANG| type ``bigint`` is represented in a binary file with the predefined
:ref:`Class <RT Class>`. The :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is ``RefType`` with
:ref:`Fully Qualified Name <RT Fully Qualified Name>` ``std.core.BigInt``.

|

.. _RT Resizable Array Types:

Resizable Array Types
=====================

The |LANG| *resizable array types* are represented in a binary file by the
predefined :ref:`Class <RT Class>`. The :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is
``RefType`` with :ref:`Fully Qualified Name <RT Fully Qualified Name>` ``std.core.Array``.

|

.. _RT Fixed-Size Array Types:

Fixed-Size Array Types
======================

The |LANG| *fixed-size array types* are represented in a binary file by a predefined :ref:`Class <RT Class>`.
Each distinct *fixed-size array type* has a unique correspondent predefined :ref:`Class <RT Class>`.
The :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is ``ArrayType``
with :ref:`Fully Qualified Name <RT Fully Qualified Name>` of the component
type being the :ref:`Fully Qualified Name <RT Fully Qualified Name>` of the *component type* of a
*fixed-size array type*.

|

.. _RT Functional Types:

Functional Types
================

The |LANG| *functional types* are represented in a binary file by a
predefined :ref:`Class <RT Class>` that depends on the arity of the *functional type*
and on the presence of a *rest parameter* in the signature.
The :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` for *functional types* without a
*rest parameter* is ``RefType`` with the :ref:`Fully Qualified Name <RT Fully Qualified Name>`
``std.core.Function0``, ``std.core.Function1``, ..., ``std.core.Function16``, or ``std.core.FunctionN``.
Exact :ref:`Type Descriptor <RT Type Descriptor>` depends on the arity of the *functional type*.
However, the :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` for *functional types* with a
*rest parameter* is ``RefType`` with the :ref:`Fully Qualified Name <RT Fully Qualified Name>`
``std.core.FunctionR0``, ``std.core.FunctionR1``, ..., ``std.core.FunctionR15`` or ``std.core.FunctionR16``.
Exact :ref:`Type Descriptor <RT Type Descriptor>` depends on the arity of the *functional type*.

|

.. _RT Functional Objects:

Functional Objects
==================

The |LANG| *functional objects* are represented in a binary file by a predefined
:ref:`Class <RT Class>`. Each *functional object* has a unique correspondent predefined :ref:`Class <RT Class>`.
``fields`` of this :ref:`Class <RT Class>`
contain references captured by the *functional object*. 
``methods`` of this :ref:Class <Class> contain auxiliary functions needed to invoke the
*functional object*. The :ref:`Method <RT Method>` that contains the body of the *functional object*
is added to the ``methods`` of the enclosing class of the
*functional object*. It is to allow the body of the *functional object* to access private members of the enclosing class.

.. note::
   If the body of a *functional object* changes the fields of the
   enclosing class or module, and if these fields are of a *value type*, then the fields
   are promoted to their *reference type* representation as discussed in
   :ref:`Value Types <RT Value Types>`.

|

.. _RT Async:

Async
=====

TBD, since the semantics will be reworked with introduction of the stackless
coroutines.

|

.. _RT Class Types:

Class Types
===========

.. _RT Class Declaration:

Class Declaration
-----------------

Each *class declaration* is lowered to a :ref:`Class <RT Class>` with an *unqualified name*
equal to the *class* name in the source code. ``fields`` of this :ref:`Class <RT Class>`
correspond to all *class field declarations* of the *class*. ``methods`` of this
:ref:`Class <RT Class>` correspond to all *class method declarations* and
*class accessor declarations* of the *class*. Additionally, for each
*constructor declaration* of the *class* a method is generated with the name
``<ctor>`` and for all *static block* in *class* the single *static method*
is generated with name ``<cctor>``.

.. _RT Class Extension Clause:

Class Extension Clause
----------------------

*Direct superclass* of the *class* is stored in the ``super_class_off`` field
of the :ref:`Class <RT Class>`.

.. _RT Class Implementation Clause:

Class Implementation Clause
---------------------------

*Direct superinterfaces* of the *class* stored in the ``class_data`` field of
the :ref:`Class <RT Class>`. ``INTERFACES`` tag is used to store *direct superinterfaces*.

.. _RT Class Field:

Class Field
-----------

Each *class field declaration* is lowered to a :ref:`Field <RT Field>` with the name
equal to the *field* name in the source code. Access modifiers of the
*field* are represented by the corresponding :ref:`Field Access Flags <RT Field Access Flags>`:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Modifier
     - Access Flag
   * - ``private``
     - ``ACC_PRIVATE``
   * - ``public``
     - ``ACC_PUBLIC``
   * - ``protected``
     - ``ACC_PROTECTED``
   * - ``static``
     - ``ACC_STATIC``
   * - ``readonly``
     - ``ACC_READONLY``

.. _RT Class Method:

Class Method
------------

Each *class method declaration* is lowered to a :ref:`Method <RT Method>` with the
name equal to the *method* name in the source code. Modifiers of the
*method* are represented by the corresponding :ref:`Method Access Flags <RT Method Access Flags>`:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Modifier
     - Access Flag
   * - ``private``
     - ``ACC_PRIVATE``
   * - ``public``
     - ``ACC_PUBLIC``
   * - ``protected``
     - ``ACC_PROTECTED``
   * - ``abstract``
     - ``ACC_ABSTRACT``
   * - ``final``
     - ``ACC_FINAL``
   * - ``native``
     - ``ACC_NATIVE``
   * - ``static``
     - ``ACC_STATIC``

.. _RT Class Accessor:

Class Accessor
--------------

Each *class accessor declaration* is lowered to a :ref:`Method <RT Method>`. For *getter*
of property ``propName`` :ref:`Method <RT Method>` name will be ``%%get-propName``. For
*setter* of property ``propName`` :ref:`Method <RT Method>` name will be
``%%set-propName``.

|

.. _RT Interface Types:

Interface Types
===============

.. _RT Interface Declaration:

Interface Declaration
---------------------

Each *interface declaration* is lowered to a :ref:`Class <RT Class>` with
an *unqualified name* equal to the *interface* name in the source code.
This :ref:`Class <RT Class>` is an *interface*
(``access_flags | ACC_INTERFACE == 1``,``access_flags | ACC_ABSTRACT == 1``).
``fields`` of this :ref:`Class <RT Class>` must be empty. ``methods`` of this :ref:`Class <RT Class>`
correspond to all *interface properties* and *interface method declarations* of
the *interface*.

.. _RT Superinterfaces And Subinterfaces:

Superinterfaces And Subinterfaces
---------------------------------

The representation of *direct superinterfaces* of an *interface* and of
:ref:`Class Implementation Clause <RT Class Implementation Clause>` is the same.

.. _RT Interface Property:

Interface Property
------------------

The binary representation of an *interface property* depends on the form in which it is declared.
The representation of a property declared as an explicit *accessor* is the same as of :ref:`Class Accessor <RT Class Accessor>`.
The representation of a property declared as a field is a *setter* and *getter* pair. The representation of both of these accessors
is the same as of the :ref:`Class Accessor <RT Class Accessor>`.

.. _RT Interface Method:

Interface Method
----------------

Each *interface method declaration* is lowered to a :ref:`Method <RT Method>` with the
name equal to the *method* name in the source code. This :ref:`Method <RT Method>` is an
*abstract method*.

|

.. _RT Enumeration Types:

Enumeration Types
=================

Each *enumeration declaration* is lowered to a :ref:`Class <RT Class>` with
an *unqualified name* equal to the *enumeration type* name in the source code.
``fields`` of this :ref:`Class <RT Class>` correspond to all
*enum constants* of the *enumeration type*.
``methods`` of this class correspond to the *enumeration methods* of the *enumeration type*.
Additionally, a single *static method* with name ``<cctor>`` is generated for this
:ref:`Class <RT Class>`. This *static method* corresponds to the *enumeration type* static constructor.
This :ref:`Class <RT Class>` extends the :ref:`Class <RT Class>` with
:ref:`Fully Qualified Name <RT Fully Qualified Name>` ``std.core.BaseEnum``.

|

.. _RT Tuple Types:

Tuple Types
===========

The |LANG| *tuple types* are represented in a binary file by a
predefined :ref:`Class <RT Class>`. :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is
``RefType`` with :ref:`Fully Qualified Name <RT Fully Qualified Name>` ``std.core.Tuple1``,
``std.core.Tuple2``, ..., ``std.core.Tuple16``, or ``std.core.TupleN``
that depends on the arity of the *tuple type*.

**Constraint**: As shown above, the |LANG| runtime does not distinguish between tuple types with an arity higher than ``16``.

|

.. _RT Union Types:

Union Types
===========

The |LANG| *union types* are represented in a binary file by a predefined :ref:`Class <RT Class>`.
Each *union type* has a unique correspondent predefined :ref:`Class <RT Class>`.
:ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is ``UnionType``
with *qualified names* of the *component types*
being the *qualified names* of the *component types* of the *union type*.

|

.. _RT Access To Common Union Members:

Access To Common Union Members
==============================

TBD, since the semantics will be reworked.

|

.. _RT Utility Types:

Utility Types
=============

.. _RT Awaited:

Awaited
-------

The |LANG| type ``Awaited`` is fully expanded at compile-time and does not appear
at runtime.

.. _RT NonNullable:

NonNullable
-----------

The |LANG| type ``NonNullable`` is fully expanded at compile-time and does not
appear at runtime.

.. _RT Partial:

Partial
-------

The |LANG| type ``Partial`` is represented in a binary file by a predefined :ref:`Class <RT Class>`.
Each ``Partial`` type has a unique correspondent predefined :ref:`Class <RT Class>`.
:ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is ``RefType`` with
the *unqualified name* ``%%partial-typeName``, where ``typeName`` is
an *unqualified name* of the ``Partial``'s type argument.

.. _RT Required:

Required
--------

The |LANG| type ``Required`` is fully expanded at compile-time and does not appear
at runtime.

.. _RT Readonly:

Readonly
--------

The |LANG| type ``Readonly`` is fully expanded at compile-time and does not appear
at runtime.

.. _RT Record:

Record
------

The |LANG| type ``Record`` is represented in a binary file by a predefined
:ref:`Class <RT Class>`. :ref:`Type Descriptor <RT Type Descriptor>` of this :ref:`Class <RT Class>` is ``RefType`` with
*qualified name* ``std.core.Record``.

.. _RT ReturnType:

ReturnType
----------

The |LANG| type ``ReturnType`` is fully expanded at compile-time and does not
appear at runtime.

|

.. _RT Namespaces And Modules:

Namespaces And Modules
======================

Each *namespace declaration* is lowered to a :ref:`Class <RT Class>` with an
*unqualified name* equal to the *namespace* name in the source code. This
:ref:`Class <RT Class>` is *abstract* (``access_flags | ACC_ABSTRACT == 1``), has
an :ref:`Annotation <RT Annotation>` with the :ref:`Fully Qualified Name <RT Fully Qualified Name>` ``ets.annotation.Module``.
``fields`` of this :ref:`Class <RT Class>` correspond to all *variable declarations* and
*constant declarations* of the *namespace*. ``methods`` of this :ref:`Class <RT Class>`
correspond to all *function declarations*, *accessor declarations*,
*function with receiver declarations*, and *accessor with receiver declarations*
of the *namespace*. Additionally, each *namespace* has a generated
*static method* with the name ``<cctor>``. It corresponds to the static
initializer of the *namespace*. A name for *top-level* namespace is chosen by the
*build system*, while the binary representation remains.

|

.. _RT Annotations:

Annotations
===========

.. _RT Annotation Declaration:

Annotation Declaration
----------------------

Each *annotation declaration* is lowered to a :ref:`Class <RT Class>` with
an *unqualified name* equal to the *annotation* name in the source code. This
:ref:`Class <RT Class>` is an *annotation* (``access_flags | ACC_ANNOTATION == 1``,
``access_flags | ACC_ABSTRACT == 1``). ``fields`` of this :ref:`Class <RT Class>`
correspond to all *annotation fields* of the *annotation*. ``methods`` of
this :ref:`Class <RT Class>` must be empty.

.. _RT Annotation Field:

Annotation Field
----------------

The representation of each *annotation field* and :ref:`Class field <RT Class field>` in bytecode is the same.
