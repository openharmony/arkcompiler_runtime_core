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

.. Annotations:

Annotations
###########

.. meta:
    frontend_status: Partly

An *annotation* is a special language element that changes the semantics of
the declaration it is applied to by adding metadata.

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

The annotation *ClassAuthor* in the example above adds meta information to
the class declaration.

An annotation must be placed immediately before the declaration it is applied to.
An annotation can include arguments as in the example above.

For an annotation to be used, the name of the annotation must be prefixed with
the character '``@``' (e.g., ``@MyAnno``). Spaces and line separators between
the character '``@``' and the name are not allowed:

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
at the place of usage. An annotation declaration can be exported and used in
other compilation units.

Multiple annotations can be applied to a single declaration:

.. code-block:: typescript
   :linenos:

    @MyAnno()
    @ClassAuthor({authorName: "John Smith"})
    class MyClass {/*body*/}

.. index::
   annotation
   access
   compilation unit

|

.. _Declaring Annotations:

Declaring Annotations
*********************

.. meta:
    frontend_status: Partly

The declaration of a *annotation* is similar to that of an interface where the
keyword ``interface`` is prefixed with the character '``@``':

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

A *type* in the annotation field is restricted (see :ref:`Types of Annotation Fields`).

The default value of an *annotation field* can be specified by using
*initializer* as *constant expression*. A :index:`compile-time error`
occurs if the value of this expression cannot be evaluated at compile time.

.. index::
   annotation
   interface
   keyword interface
   prefix
   annotation field
   constant expression
   compile time
   initializer
   keyword export
   type

The *annotation* must be defined at the top level. Otherwise, a
:index:`compile-time error` occurs.

An *annotation* cannot be extended as inheritance is not supported.

The name of an *annotation* cannot coincide with another entity name:

.. code-block:: typescript
   :linenos:

    @interface Position {/*properties*/}

    class Position {/*body*/} // compile-time error: duplicate identifier

An annotation declaration does not define a type, and a type alias can be
neither applied to the annotation, nor used as an interface:

.. code-block:: typescript
   :linenos:

    @interface Position {}
    type Pos = Position // compile-time error

    class A implements Position {} // compile-time error

.. index::
   annotation
   type alias
   interface
   entity

|

.. _Types of Annotation Fields:

Types of Annotation Fields
==========================

.. meta:
    frontend_status: Partly

The choice of types for annotation fields is limited to the following:

- Numeric types (:ref:`Numeric Types Hierarchy`);
- Type ``boolean``;
- Type ``string``;
- Enumeration types;
- Array of above types, e.g., ``string[]``, including multi-dimensional arrays, like ``string[][]``.

A :index:`compile-time error` occurs if any other type is used as type of an
*annotation field*.

.. index::
   annotation field
   numeric type
   string
   enumeration type
   array

|

.. _Using Annotations:

Using Annotations
*****************

.. meta:
    frontend_status: Partly

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
   annotation field
   object literal
   expression

The usage of annotation is presented in the example below:

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John", revision: 2})
    class C1 {/*body*/}

    @ClassPreamble({authorName: "Bob"}) // default value for revision = 1
    class C2 {/*body*/}

    @MyAnno()
    class C3 {/*body*/}

The current version of |LANG| allows using annotations only with the following:

- Non-abstract class declarations;
- Method declarations in non-abstract classes; and
- Function declaration.

.. index::
   annotation
   non-abstract class
   declaration
   method
   function

Otherwise, a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    @MyAnno()
    abstract class A {} // compile-time error

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

If a field of an array type for an annotation is defined, then the array
literal syntax is used to set its value:

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

Parentheses after the annotation name can be omitted if setting annotation
properties is not required:

.. code-block:: typescript
   :linenos:

    @MyAnno
    class C4 {/*body*/}

.. index::
   field
   array type
   annotation
   array literal

|

.. _Using Single Field Annotations:

Using Single Field Annotations
==============================

.. meta:
    frontend_status: Partly

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

The behavior of a short notation is exactly the same as that of a notation
with an object literal.

.. index::
   field annotation
   notation
   expression
   object literal

|

.. _Exporting and Importing Annotations:

Exporting and Importing Annotations
***********************************

An annotation can be exported and imported. However, a few forms of export and
import directives are supported.

To export an annotation, its declaration must be marked with the keyword
``export``:

.. code-block:: typescript
   :linenos:

    // a.sts
    export @interface MyAnno {}


If an annotation is imported as part of an imported module, then the annotation
is accessed by its qualified name:

.. code-block:: typescript
   :linenos:

    // b.sts
    import * as ns from "./a"

    @ns.MyAnno
    class C {/*body*/}

.. index::
   export
   import
   annotation
   import directive
   imported module
   qualified name
   access
   unqualified import

Unqualified import is also allowed:

.. code-block:: typescript
   :linenos:

    // b.sts
    import { MyAnno } from "./a"

    @MyAnno
    class C {/*body*/}

An annotation is not a type. Exporting or importing annotations by using
``export type`` or ``import type`` notations is forbidden:

.. code-block:: typescript
   :linenos:

    import type { MyAnno } from "./a" // compile-time error


Annotations are forbidden in the following cases:

- Export default,

- Import default,

- Rename in export, and

- Rename in import.

.. index::
   export
   import
   annotation
   type
   notation

.. code-block:: typescript
   :linenos:

    import {MyAnno as Anno} from "./a" // compile-time error

|

.. _Ambient Annotations:

Ambient Annotations
*******************

Ambient annotations can be specified in :ref:`Declaration Modules` only.

.. code-block:: abnf

    ambientAnnotationDeclaration:
        'declare' annotationDeclaration
        ;

Such a declaration does not introduce a new annotation but provides type
information to use an annotation that must be defined elsewhere.
A runtime error occurs if no annotation corresponds to the ambient annotation
used in the program.

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

    // a.d.sts
    export declare @interface NameAnno{name: string = ""}

    // a.sts
    export @interface NameAnno{name: string = ""} // ok

The code in the example below is incorrect because the ambient declaration is
not identical to the annotation declaration:

.. code-block:: typescript
   :linenos:

    // a.d.sts
    export declare @interface VersionAnno{version: number} // initialization is missing

    // a.sts
    export @interface VersionAnno{version: number = 1}


An ambient declaration can be imported and used in exactly the same manner
as a regular annotation:

.. code-block:: typescript
   :linenos:

    // a.d.sts
    export declare @interface MyAnno {}

    // b.sts
    import { MyAnno } from "./a"

    @MyAnno
    class C {/*body*/}

If an annotation is applied to an ambient declaration in the *.d.sts* file (see
the example below), then the annotation is to be applied to the implementation
declaration manually, because the annotation is not automatically applied to
the declaration that implements the ambient declaration:

.. code-block:: typescript
   :linenos:

    // a.d.sts
    export declare @interface MyAnno {}

    @MyAnno
    declare class C {}

.. index::
   annotation declaration
   import
   annotation
   ambient declaration
   declaration

|

.. _Standard Annotations:

Standard Annotations
********************

*Standard annotation* is an annotation that is defined in :ref:`Standard Library`,
or implicitly defined in the compiler (*built-in annotation*).
A *standard annotation* is usually known to the compiler. A *standard annotation*
modifies semantics of the declaration it is applied to.

.. index::
   standard annotation
   annotation
   compiler
   built-in annotation
   semantics
   declaration

|

.. raw:: pdf

   PageBreak
