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

.. _Ambient Declarations:

Ambient Declarations
####################

.. meta:
    frontend_status: Done

*Ambient declaration* specifies an entity that is declared somewhere else.
Ambient declarations:

-  Provide type information for entities included in a program from an external
   source.
-  Introduce no new entities like regular declarations do.
-  Cannot include executable code, and thus have no initializers.

Ambient functions, methods, and constructors have no bodies.

.. index::
   ambient declaration
   declaration
   module
   entity
   executable code
   initializer
   initialization
   ambient function
   ambient method
   ambient constructor
   function
   method
   constructor

The syntax of *ambient declaration* is presented below:

.. code-block:: abnf

    ambientDeclaration:
        'declare'
        ( ambientConstantDeclaration
        | ambientFunctionDeclaration
        | ambientClassDeclaration
        | ambientInterfaceDeclaration
        | ambientNamespaceDeclaration
        | ambientAnnotationDeclaration
        | 'const'? enumDeclaration
        | typeAlias
        )
        ;

An ambient enumeration type declaration can be prefixed by the keyword
``const`` for |TS| compatibility. It has no influence on the declared type.

A :index:`compile-time error` occurs if the modifier ``declare`` is used in a
context that is already ambient:

.. code-block:: typescript
   :linenos:

    declare namespace A{
        declare function foo(): void // compile-time error
    }

.. index::
   ambient declaration
   enumeration type
   context
   modifier declare
   declare
   prefix
   keyword const
   compatibility
   ambient

|

.. _Ambient Constant Declarations:

Ambient Constant Declarations
*****************************

.. meta:
    frontend_status: Done

The syntax of *ambient constant declaration* is presented below:

.. code-block:: abnf

    ambientConstantDeclaration:
        'const' ambientConstList ';'
        ;

    ambientConstList:
        ambientConst (',' ambientConst)*
        ;

    ambientConst:
        identifier ((':' type) | ('=' (IntegerLiteral|FloatLiteral|StringLiteral|MultilineStringLiteral)))
        ;

An initializer expression for an ambient constant must be a numeric or string
literal. The meaning of the literal is to define the type of the ambient
constant, while the actual value must be provided when a non-ambient declaration
is available.

.. index::
   ambient constant
   declaration
   type annotation
   initializer expression
   ambient constant
   non-ambient declaration
   string literal
   numeric literal

|

.. _Ambient Function Declarations:

Ambient Function Declarations
*****************************

.. meta:
    frontend_status: Done

The syntax of *ambient function declaration* is presented below:

.. code-block:: abnf

    ambientFunctionDeclaration:
        'function' identifier
        typeParameters? signature
        ;

A :index:`compile-time error` occurs if explicit return type for an ambient
function declaration is not specified.

.. index::
   ambient function declaration
   type annotation
   return type
   function
   function declaration

.. code-block:: typescript
   :linenos:

    declare function foo(x: number): void // ok
    declare function bar(x: number) // compile-time error

Ambient functions cannot have parameters with default values but can have
optional parameters.

Ambient function declarations cannot specify function bodies.

.. code-block:: typescript
   :linenos:

    declare function foo(x?: string): void // ok
    declare function bar(y: number = 1): void // compile-time error

**Note**. The modifier ``async`` cannot be used in an ambient context.

.. index::
   ambient function declaration
   ambient function
   value
   parameter
   default value
   modifier async
   async
   function body
   ambient context

|

.. _Ambient Class Declarations:

Ambient Class Declarations
**************************

.. meta:
    frontend_status: Done

The syntax of *ambient class declaration* is presented below:

.. code-block:: abnf

    ambientClassDeclaration:
        'class'|'struct' identifier typeParameters?
        classExtendsClause? implementsClause?
        '{' ambientClassBodyDeclaration* '}'
        ;

    ambientClassBodyDeclaration:
        ambientAccessModifier?
        ( ambientFieldDeclaration
        | ambientConstructorDeclaration
        | ambientMethodDeclaration
        | ambientAccessorDeclaration
        | ambientIndexerDeclaration
        | ambientCallSignatureDeclaration
        | ambientIterableDeclaration
        )
        ;

    ambientAccessModifier:
        'public' | 'protected'
        ;

Ambient field declarations have no initializers.

.. index::
   ambient field declaration
   initializer

The syntax of *ambient field declaration* is presented below:

.. code-block:: abnf

    ambientFieldDeclaration:
        ambientFieldModifier* identifier ':' type
        ;

    ambientFieldModifier:
        'static' | 'readonly'
        ;

Ambient constructor, method, and accessor declarations have no bodies.

Their syntax is presented below:

.. code-block:: abnf

    ambientConstructorDeclaration:
        'constructor' parameters
        ;

    ambientMethodDeclaration:
        ambientMethodModifier* identifier signature
        ;

    ambientMethodModifier:
        'static'
        ;

    ambientAccessorDeclaration:
        ambientMethodModifier*
        ( 'get' identifier '(' ')' returnType
        | 'set' identifier '(' parameter ')'
        )
        ;

.. index::
   constructor
   method
   accessor
   ambient accessor declaration

|

.. _Ambient Indexer:

Ambient Indexer
===============

.. meta:
    frontend_status: Done

*Ambient indexer declarations* specify the indexing of a class instance
in an ambient context. The feature is provided for |TS| compatibility:

The syntax of *ambient indexer declaration* is presented below:

.. code-block:: abnf

    ambientIndexerDeclaration:
        'readonly'? '[' identifier ':' indexType ']' returnType
        ;

The following restriction applies: Only one *ambient indexer declaration* is
allowed in an ambient class declaration.

.. index::
   ambient indexer declaration
   indexing
   class instance
   ambient context
   compatibility
   restriction
   ambient class declaration

.. code-block:: typescript
   :linenos:

    declare class C {
        [index: number]: number
    }

**Note**. *Ambient indexer declaration* is supported in ambient contexts only.
If written in |LANG|, ambient class implementation must conform to
:ref:`Indexable Types`.

.. index::
   ambient indexer declaration
   ambient context
   ambient class
   implementation
   indexable type

|

.. _Ambient Call Signature:

Ambient Call Signature
======================

.. meta:
    frontend_status: Done

*Ambient call signature* declarations are used to specify *callable types*
in an ambient context. The feature is provided for |TS| compatibility:

The syntax of *ambient call signature declaration* is presented below:

.. code-block:: abnf

    ambientCallSignatureDeclaration:
        signature
        ;

.. code-block:: typescript
   :linenos:

    declare class C {
        (someArg: number): boolean
    }

**Note**. *Ambient class signature declaration* is supported in ambient contexts
only. If written in |LANG|, ambient class implementation must conform to
:ref:`Callable Types with $_invoke Method`.

The following restriction applies: only one *ambient call signature* is allowed
in an ambient class declaration.

.. index::
   ambient call signature declaration
   ambient call signature
   callable type
   ambient context
   compatibility
   restriction

|

.. _Ambient Iterable:

Ambient Iterable
================

.. meta:
    frontend_status: Done

*Ambient iterable declaration* indicates that a class instance is iterable
in an ambient context. The feature is provided for |TS| compatibility:

The syntax of *ambient iterable declaration* is presented below:

.. code-block:: abnf

    ambientIterableDeclaration:
        '[Symbol.iterator]' '(' ')' returnType
        ;


The following restrictions apply:

- *returnType* must be a type that implements ``Iterator`` interface defined
  in :ref:`Standard Library`.
- Only one *ambient iterable declaration* is allowed in an ambient class
  declaration.

.. code-block:: typescript
   :linenos:

    declare class C {
        [Symbol.iterator]: CIterator
    }

**Note**. *Ambient iterable declaration* is supported in ambient contexts only.
If written in |LANG|, ambient class implementation must conform to
:ref:`Iterable Types`.

.. index::
   ambient iterable declaration
   class instance
   iterable class instance
   ambient context
   compatibility
   return type
   implementation
   interface

|

.. _Ambient Interface Declarations:

Ambient Interface Declarations
******************************

.. meta:
    frontend_status: Done

The syntax of *ambient interface declaration* is presented below:

.. code-block:: abnf

    ambientInterfaceDeclaration:
        'interface' identifier typeParameters?
        interfaceExtendsClause?
        '{' ambientInterfaceMember* '}'
        ;

    ambientInterfaceMember
        : interfaceProperty
        | interfaceMethodDeclaration
        | ambientIndexerDeclaration
        | ambientIterableDeclaration
        ;

*Ambient interface* can contain additional members in the same manner as
an ambient class (see :ref:`Ambient Indexer`, and :ref:`Ambient Iterable`).

.. index::
   ambient interface
   ambient interface declaration
   ambient class

|

.. _Ambient Namespace Declarations:

Ambient Namespace Declarations
******************************

.. meta:
    frontend_status: Done

Namespaces are used to logically group multiple entities. |LANG| supports
*ambient namespaces* for better |TS| compatibility. |TS| often uses ambient
namespaces to specify the platform API or a third-party library API.

The syntax of *ambient namespace declaration* is presented below:

.. code-block:: abnf

    ambientNamespaceDeclaration:
        'namespace' identifier '{' ambientNamespaceElement* '}'
        ;

    ambientNamespaceElement:
        ambientNamespaceElementDeclaration | exportDirective
    ;

    ambientNamespaceElementDeclaration:
        'export'?
        ( ambientConstantDeclaration
        | ambientFunctionDeclaration
        | ambientClassDeclaration
        | ambientInterfaceDeclaration
        | ambientNamespaceDeclaration
        | 'const'? enumDeclaration
        | typeAlias
        )
        ;

An *enumeration type declaration* can be prefixed with the keyword ``const``
for |TS| compatibility. The prefix has no influence on the declared type.
Only exported entities can be accessed outside a namespace.

Namespaces can be nested:

.. code-block:: typescript
   :linenos:

    declare namespace A {
        export namespace B {
            export function foo(): void;
        }
    }

A namespace is not an object but merely a scope for entities that can be
accessed by using qualified names only.

.. index::
   namespace
   ambient namespace
   entity
   compatibility
   platform API
   third-party library API
   ambient iterable declaration
   qualified name
   access
   keyword const
   enumeration type declaration
   prefix
   declared type

If an ambient namespace is imported from a declaration module, then all
ambient namespace declarations are accessible (see :ref:`Accessible`) across
all declarations and top-level statements of the current module.

.. code-block:: typescript
   :linenos:

    // File1.d.ets
    export declare namespace A { // namespace itself must be exported
        function foo(): void
        type X = Array<number>
    }

    // File2.ets
    import {A} from 'File1.d.ets'

    A.foo() // Valid function call, as 'foo' is accessible for top-level statements
    function foo () {
        A.foo() // Valid function call, as 'foo' is accessible here as well
    }
    class C {
        method () {
            A.foo() // Valid function call, as 'foo' is accessible here too
            let x: A.X = [] // Type A.X can be used
        }
    }

.. index::
   ambient namespace
   ambient namespace declaration
   declaration module
   accessible declaration
   access
   accessibility
   top-level statement
   module

|

.. _Implementing Ambient Namespace Declaration:

Implementing Ambient Namespace Declaration
==========================================

.. meta:
    frontend_status: Done

If an *ambient namespace* is implemented in |LANG|, a namespace with the
same name must be declared (see :ref:`Namespace Declarations`) as the
top-level declaration of a compilation unit. All namespace names of a nested
namespace (i.e. a namespace embedded into another namespace) must be the same
as in ambient context.

A compilation unit that implements a namespace is the unit for which the
declaration module is built (see :ref:`Declaration Modules`).

.. index::
   ambient namespace declaration
   ambient namespace
   entity
   implementation
   namespace declaration
   declaration
   top-level declaration
   compilation unit
   ambient context
   nested namespace
   declaration module

.. raw:: pdf

   PageBreak
