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

.. _Annotations:

Annotations
###########

.. meta:
    frontend_status: Done

*Annotation* is a special language element that changes the semantics of
the declaration to which it is applied by adding metadata.

The example below illustrates how an annotation is declared and used:

.. code-block:: typescript
   :linenos:

    // Annotation declaration:
    @interface ClassAuthor {
        authorName: string
    }

    // Annotation use:
    @ClassAuthor({authorName: "Bob"})
    class MyClass {/*body*/}

The annotation *ClassAuthor* in the example above adds metadata to
the class declaration.

An annotation must be placed immediately before the declaration to which it is
applied. An annotation can include arguments as in the example above.

For an annotation to be used, the name of the annotation must be prefixed with
the character '``@``' (e.g., ``@MyAnno``). No white space and line separator is
allowed between the character '``@``' and the name:

.. index::
   annotation
   semantics
   metadata
   declaration
   class declaration
   prefix
   space
   line separator
   argument

.. code-block::
   :linenos:

    ClassAuthor({authorName: "Bob"}) // compile-time error, no '@'
    @ ClassAuthor({authorName: "Bob"}) // compile-time error, space is forbidden

A :index:`compile-time error` occurs if the annotation name is not accessible
(see :ref:`Accessible`) at the place of usage. An annotation declaration can be
exported and used in other compilation units.

Multiple annotations can be applied to a single declaration:

.. code-block:: typescript
   :linenos:

    @MyAnno()
    @ClassAuthor({authorName: "John Smith"})
    class MyClass {/*body*/}

.. index::
   annotation
   access
   accessibility
   annotation declaration
   compilation unit

|

.. _Declaring Annotations:

Declaring Annotations
*********************

.. meta:
    frontend_status: Done

Declaring an *annotation* is similar to declaring an interface where the
keyword ``interface`` is prefixed with the character '``@``'.

The syntax of *annotation declaration* is presented below:

.. code-block:: abnf

    annotationDeclaration:
        '@interface' identifier '{' annotationField* '}'
        ;
    annotationField:
        identifier ':' type constInitializer?
        ;
    constInitializer:
        '=' constantExpression
        ;

As any other declared entity, an annotation can be exported by using the
keyword ``export``.

*Type* in the annotation field is restricted (see :ref:`Types of Annotation Fields`).

The default value of an *annotation field* can be specified by using
*initializer* as *constant expression*. A :index:`compile-time error`
occurs if the value of this expression cannot be evaluated at compile time.

.. index::
   annotation
   interface
   keyword interface
   prefix
   keyword export
   annotation field
   constant expression
   compile time
   initializer
   type

*Annotation* must be defined at the top level. Otherwise, a
:index:`compile-time error` occurs.

*Annotation* cannot be extended as inheritance is not supported.

The name of an *annotation* cannot coincide with the name of another entity:

.. code-block:: typescript
   :linenos:

    @interface Position {/*properties*/}

    class Position {/*body*/} // compile-time error: duplicate identifier

An annotation declaration defines no type, and no type alias can be applied to
the annotation or used as an interface:

.. code-block:: typescript
   :linenos:

    @interface Position {}
    type Pos = Position // compile-time error

    class A implements Position {} // compile-time error

.. index::
   annotation
   type alias
   inheritance
   annotation declaration
   interface
   entity

|

.. _Types of Annotation Fields:

Types of Annotation Fields
==========================

.. meta:
    frontend_status: Done

The choice of *types for annotation fields* is limited to the following:

- :ref:`Numeric Types`;
- Type ``boolean`` (see :ref:`Boolean Types and Operations`);
- :ref:`Type string`;
- Enumeration types (see :ref:`Enumerations`);
- Array of the above types (e.g., ``string[]``), including multidimensional
  arrays (e.g., ``string[][]``).

A :index:`compile-time error` occurs if any other type is used as the type of
an *annotation field*.

.. index::
   annotation field
   type for annotation field
   numeric type
   boolean type
   type boolean
   string
   type string
   enumeration type
   array
   multidimensional array

|

.. _Using Annotations:

Using Annotations
*****************

.. meta:
    frontend_status: Done

The following syntax is used to apply an annotation to a declaration,
and to define the values of annotation fields:

.. code-block:: abnf

    annotationUsage:
        '@' qualifiedName annotationValues?
        ;
    annotationValues:
        '(' (objectLiteral | constantExpression)? ')'
        ;

An annotation declaration is presented in the example below:

.. code-block:: typescript
   :linenos:

    @interface ClassPreamble {
        authorName: string
        revision: number = 1
    }
    @interface MyAnno{}

In general, annotation field values are set by an *object literal*. In a
special case, annotation field values are set by using an expression (see
:ref:`Using Single Field Annotations`).

All values in an *object literal* must be constant expressions. Otherwise,
a :index:`compile-time error` occurs.

.. index::
   annotation
   annotation declaration
   syntax
   declaration
   annotation field
   object literal
   value
   expression

The usage of annotation is presented in the example below. The annotations in
this example are applied to class declarations:

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John", revision: 2})
    class C1 {/*body*/}

    @ClassPreamble({authorName: "Bob"}) // default value for revision = 1
    class C2 {/*body*/}

    @MyAnno()
    class C3 {/*body*/}

Annotations can be applied to the following:

- :ref:`Top-Level Declarations`;

- Class members (see :ref:`Class Body`) or interface members (see
  :ref:`Interface Body`);

- Type usage (see :ref:`Using Types`);

- Parameters (see :ref:`Parameter List` and :ref:`Optional Parameters`);

- Lambda expression (see :ref:`Lambda Expressions` and
  :ref:`Lambda Expressions with Receiver`);

- :ref:`Local Declarations`.

.. index::
   annotation
   declaration
   class declaration
   top-level declaration
   class
   interface
   method
   parameter
   lambda expression
   function

Otherwise, a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:


    function foo () {
           @MyAnno() let local = 1 // compile-time error
    }

Repeatable annotations are not supported, i.e., an annotation cannot be applied
to an entity more than once:

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John"})
    @ClassPreamble({authorName: "Bob"}) // compile-time error
    class C {/*body*/}

When using an annotation, the order of values has no significance:

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John", revision: 2})
    // the same as:
    @ClassPreamble({revision: 2, authorName: "John"})

When using an annotation, all fields without default values must be listed.
Otherwise, a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    @ClassPreamble() // compile-time error, authorName is not defined
    class C1 {/*body*/}

.. index::
   annotation
   array literal
   array type
   value
   field

If a field of an array type for an annotation is defined, then its value is set
by using the array literal syntax:

.. code-block:: typescript
   :linenos:

    @interface ClassPreamble {
        authorName: string
        revision: number = 1
        reviewers: string[]
    }

    @ClassPreamble(
        {authorName: "Alice",
        reviewers: ["Bob", "Clara"]}
    )
    class C3 {/*body*/}

If setting annotation properties is not required, then parentheses can be
omitted after the annotation name:

.. code-block:: typescript
   :linenos:

    @MyAnno
    class C4 {/*body*/}

.. index::
   field
   array type
   annotation
   array literal
   parenthesis
   annotation name

|

.. _Using Single Field Annotations:

Using Single Field Annotations
==============================

.. meta:
    frontend_status: Done

If annotation declaration defines only one field, then it can be used with a
short notation to specify just one expression instead of an object literal:

.. code-block:: typescript
   :linenos:

    @interface deprecated{
        fromVersion: string
    }

    @deprecated("5.18")
    function foo() {}

    @deprecated({fromVersion: "5.18"})
    function goo() {}

A short notation and a notation with an object literal behave in exactly the
same manner.

.. index::
   field annotation
   annotation declaration
   field
   notation
   expression
   object literal

|

.. _Exporting and Importing Annotations:

Exporting and Importing Annotations
***********************************

.. meta:
    frontend_status: Done

An annotation can be exported and imported. However, a few forms of export and
import directives are supported.

An annotation declaration to be exported must be marked with the keyword
``export`` as follows:

.. code-block:: typescript
   :linenos:

    // a.ets
    export @interface MyAnno {}

If an annotation is imported as a part of an imported module, then the
annotation is accessed by its qualified name:

.. code-block:: typescript
   :linenos:

    // b.ets
    import * as ns from "./a"

    @ns.MyAnno
    class C {/*body*/}

.. index::
   export
   import
   annotation
   annotation declaration
   keyword export
   import directive
   imported module
   qualified name
   access
   unqualified import

Unqualified import is also allowed:

.. code-block:: typescript
   :linenos:

    // b.ets
    import { MyAnno } from "./a"

    @MyAnno
    class C {/*body*/}

An annotation is not a type. Using ``export type`` or ``import type`` notations
to export or import annotations is forbidden:

.. code-block:: typescript
   :linenos:

    import type { MyAnno } from "./a" // compile-time error

Annotations are forbidden in the following cases:

- Export default,

- Import default,

- Rename in export, and

- Rename in import.

.. index::
   export type
   import type
   annotation
   type
   notation
   import annotation
   export default
   import default

.. code-block:: typescript
   :linenos:

    import {MyAnno as Anno} from "./a" // compile-time error

|

.. _Ambient Annotations:

Ambient Annotations
*******************

.. meta:
    frontend_status: Done

The syntax of *ambient annotations* is presented below:

.. code-block:: abnf

    ambientAnnotationDeclaration:
        'declare' annotationDeclaration
        ;

Such a declaration does not introduce a new annotation but provides type
information that is required to use the annotation. The annotation itself
must be defined elsewhere. A runtime error occurs if no annotation corresponds
to the ambient annotation used in the program.

An ambient annotation and the annotation that implements it must be exactly
identical, including field initialization:

.. index::
   ambient annotation
   declaration
   annotation
   type
   runtime error
   field initialization

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface NameAnno{name: string = ""}

    // a.ets
    export @interface NameAnno{name: string = ""} // ok

The code in the example below is incorrect because the ambient declaration is
not identical to the annotation declaration:

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface VersionAnno{version: number} // initialization is missing

    // a.ets
    export @interface VersionAnno{version: number = 1}

An ambient declaration can be imported and used in exactly the same manner
as a regular annotation:

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface MyAnno {}

    // b.ets
    import { MyAnno } from "./a"

    @MyAnno
    class C {/*body*/}

If an annotation is applied to an ambient declaration in the *.d.ets* file (see
the example below), then the annotation is to be applied to the implementation
declaration manually, because the annotation is not automatically applied to
the declaration that implements the ambient declaration:

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface MyAnno {}

    @MyAnno
    declare class C {}

.. index::
   annotation declaration
   import
   annotation
   ambient declaration
   declaration
   implementation

|

.. _Standard Annotations:

Standard Annotations
********************

.. meta:
    frontend_status: Done

*Standard annotation* is an annotation that is defined in :ref:`Standard Library`,
or implicitly defined in the compiler (*built-in annotation*).
*Standard annotation* is usually known to the compiler. It modifies the
semantics of the declaration it is applied to.

.. index::
   standard annotation
   annotation
   compiler
   built-in annotation
   semantics
   declaration

|

.. _Retention Annotation:

Retention Annotation
====================

.. meta:
    frontend_status: Done

*@Retention* is a standard annotation that is used to annotate a declaration
of another annotation.
A :index:`compile-time error` occurs if it is used in other places.

The annotation has a single field ``policy`` of type ``string``. It is typically
used as follows:

.. code-block:: typescript
   :linenos:

    @Retention({policy: "RUNTIME"})
    @interface MyAnno {} // this annotation uses "RUNTIME" policy

    @MyAnno //
    class C {}

.. index::
   retention annotation
   standard annotation
   annotation
   declaration
   declaration annotation
   field

The value of this field determines at which point an annotation is used,
and discarded after use.
The retention policies can be of three types:

- "SOURCE"

  Annotations that use "SOURCE" policy are processed at compile time, and are
  discarded after compilation;

- "BYTECODE"

  Metadata specified in annotations that use "BYTECODE" policy are saved into
  the bytecode file, but are discarded at runtime.

- "RUNTIME"

  Metadata specified in annotations that use "RUNTIME" policy are saved into
  the bytecode file, are retained and can be accessed at runtime.

The default retention policy is "BYTECODE".

A :index:`compile-time error` occurs if any other string literal is used as
the value of ``policy`` field.

As ``@Retention`` has a single field, it can be used with a short notation
(see :ref:`Using Single Field Annotations`) as follows:

.. code-block:: typescript
   :linenos:

    @Retention("SOURCE")
    @interface Author {name: string} // this annotation uses "SOURCE" policy

.. index::
   source
   runtime
   value
   field
   compile time
   bytecode
   metadata
   annotation
   policy
   bytecode file
   string literal
   notation

|

.. _Runtime Access to Annotations:

Runtime Access to Annotations
*****************************

.. meta:
    frontend_status: None

For an annotation with *retention policy* (see :ref:`Retention Annotation`)
``BYTECODE`` or ``RUNTIME`` an abstract class with the name of the annotation
is implicitly declared. All fields of this class are ``readonly``.
If a field is of an array type, the array type is also ``readonly``.

For the following annotation:

.. code-block:: typescript
   :linenos:

    @Retention("RUNTIME")
    @interface MyAnno {
        name: string
        attrs: number[]
    }

the abstract class is declared:

.. code-block:: typescript
   :linenos:

    abstract class MyAnno {
        readonly name: string
        readonly attrs: readonly number[]
    }

The following example shows the use of such class:

.. code-block:: typescript
   :linenos:

    @MyAnno({name: "someName", attr: [1, 2]})
    class A {}

    let my: MyAnno = // call of reflection library to get instance of annotation for type A
    console.log(my.name) // output: someName

.. raw:: pdf

   PageBreak
