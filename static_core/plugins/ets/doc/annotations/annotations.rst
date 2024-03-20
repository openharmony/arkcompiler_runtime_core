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

Annotations
###########

An *annotation* is a special language element that changes the semantics of
the declarations it is applied to by adding metadata.

The example below illustrates the declaring and using of an annotation:

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
The annotation usage can include arguments as in the example above.

For an annotation to be used, its name must be prefixed with the sympol ``@``
(e.g., ``@MyAnno``). 
Spaces and line separators between the sympol ``@`` and the name are not allowed:

.. code-block:: 
   :linenos:

    ClassAuthor({authorName: "Bob"}) // compile-time error, no '@'
    @ ClassAuthor({authorName: "Bob"}) // compile-time error, space is forbidden

A compile-time error occurs if the annotation
name is not accessible at the place of usage. An annotation declaration can be
exported and used in other compilation units.

Multiple annotations can be applied to one declaration:

.. code-block:: typescript
   :linenos:

    @MyAnno()
    @ClassAuthor({authorName: "John Smith"})
    class MyClass {/*body*/}


As annotations is not |TS| feature in can be used only in ``.ets/.d.ets`` files.

|

.. _Declaration of User-Defined Annotation:

Declaration of User-Defined Annotation
======================================

The definition of a *user-defined annotation* is similar to that of an
interface where the keyword ``interface`` is prefixed with the symbol ``@``:

.. code-block:: abnf

    userDefinedAnnotationDeclaration:
        annotationModifier? '@interface' Identifier '{' annotationField* '}'
        ;
    annotationModifier:
        'export'
        ;
    annotationField:
        Identifier ':' TypeNode initializer?
        ;
    initializer:
        '=' constantExpression
        ;
    constantExpression:
        Expression
        ;

A ``TypeNode`` in the annotation field is restricted (see :ref:`Types of Annotation Fields`).

The default value of an *annotation field* can be specified 
using *initializer* as *constant expression*. A compile-time occurs in the value
of this expression can not be evaluated in compile-time.

An *user-defined annotation* must be defined at top-level,
otherwise a compile-time error occurs.

The name of an *user-defined annotation* can not coincide with a name of other entity.

.. code-block:: typescript
   :linenos:

    @interface Position {/*properties*/}
  
    class Position {/*body*/} // compile-time error: duplicate identifier

An annotation declaration does not define a type, so a type alias
can not be applied to the annotation:

.. code-block:: typescript
   :linenos:

    @interface Position {}
    type Pos = Position // compile-time error

|

.. _Types of Annotation Fields:

Types of Annotation Fields
==========================

The choice of types for annotation fields is limited to the types listed below:

- Type ``number``;
- Type ``boolean``;
- Type ``string``;
- Enumeration types;
- Array of above types, e.g., ``string[]``.

A compile-time error occurs if any other type is used as type of an *annotation
field*.

.. _Using of User-Defined Annotation:

Using of User-Defined Annotation
================================

The following syntax is used to apply an
annotation to a declaration, 
and to define the values of annotation properties:

.. code-block:: abnf

    userDefinedAnnotationUsage:
        '@' qualifiedName userDefinedAnnotationParamList? 
        ;
    qualifiedName:
        Identifier ('.' Identifier)*
        ;
    userDefinedAnnotationParamList:
        '(' ObjectLiteralExpression? ')'
        ;

The current version of the language allows to use annotations only
for class declarations and method declarations.

An annotation declaration is presented in the example below:

.. code-block:: typescript
   :linenos:

    @interface ClassPreamble {
        authorName: string
        revision: number = 1
    }
    @interface MyAnno{}

Annotation usage is presented in the example below:

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John", revision: 2})
    class C1 {/*body*/}

    @ClassPreamble({authorName: "Bob"}) // default value for revision = 1
    class C2 {/*body*/}

    @MyAnno()
    class C3 {/*body*/}


The order of properties does not matter in an annotation usage:

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John", revision: 2})
    // the same as:
    @ClassPreamble({revision: 2, authorName: "John"})
    

When using an annotation, all fields without default values must be listed.
Otherwise, a compile-time error occurs:

.. code-block:: typescript
   :linenos:

    @ClassPreamble() // compile-time error, authorName is not defined
    class C1 {/*body*/}


If a field of an array type is defined for an annotation, then the array
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

The parentheses after the annotation name can be omitted,
if there is no need to set annotation properties:

.. code-block:: typescript
   :linenos:

    @MyAnno
    class C4 {/*body*/}

.. _Exporting and Importing Annotations:

Exporting and Importing Annotations
===================================

An annotation can be exported and imported in the same way as any other program entity
with some restrictions, see below.

.. code-block:: typescript
   :linenos:

    // a.ets
    export @interface MyAnno {}

    // b.ets
    import * as ns from "./a"

    @ns.MyAnno
    class C {/*body*/}

Unqualified import is also allowed:

.. code-block:: typescript
   :linenos:

    // b.ets
    import { MyAnno } from "./a"

    @MyAnno
    class C {/*body*/}

As an annotation is not a type, it is forbidden to export or import 
using ``export type`` or ``import type`` notations:

.. code-block:: typescript
   :linenos:

    import type { MyAnno } from "./a" // compile-time error 


The following cases are forbidden for annotations:

- Export default,

- Import default,

- Rename in import.

.. code-block:: typescript
   :linenos:

    import {MyAnno as Anno} from "./a" // compile-time error


.. _Annotations in .d.ets Files:

Annotations in .d.ets Files
===========================

Annotations can be defined in .d.ets and used if imported.

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export @interface MyAnno {}

    // b.ets
    import { MyAnno } from "./a"

    @MyAnno
    class C {/*body*/}


A compile-time error occurs if an annotation
is applied to an ambient declaration:

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export @interface MyAnno {}
    
    @MyAnno // compile-time error
    declare class C {}

The reason is that ambient declarations do not introduce new entities
but provide type information for entities that exist somewhere else, and
are included in a program by external means. As a result, applying an
annotation to an ambient declaration makes no sense as it has no influence
on a "real" entity.
