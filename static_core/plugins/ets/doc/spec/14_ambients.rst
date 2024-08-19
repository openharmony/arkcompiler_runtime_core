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

.. _Ambient Declarations:

Ambient Declarations
####################

.. meta:
    frontend_status: Done
    
Ambient declarations are the way to specify entities that are declared
somewhere else. Ambient declarations do not introduce new entities---as regular
declarations do---but instead provide type information for the entities that
are included in a program by external means. Ambient declarations cannot
include executable code. As a consequence, they have no initializers.

Ambient declaration can be specified in :ref:`Declaration Modules` only.

Ambient functions, methods, and constructors have no bodies.

.. index::
   ambient declaration
   entity
   execution
   initializer
   initialization
   ambient function
   method
   constructor


.. code-block:: abnf

    ambientDeclaration:
        'declare'
        ( ambientConstantDeclaration 
        | ambientFunctionDeclaration
        | ambientClassDeclaration
        | ambientInterfaceDeclaration
        | ambientNamespaceDeclaration
        | 'const'? enumDeclaration
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
   compile-time error
   context
   modifier
   ambient

|

.. _Ambient Constant Declarations:

Ambient Constant Declarations
*****************************

.. meta:
    frontend_status: Done
    
.. code-block:: abnf

    ambientConstantDeclaration:
        'const' ambientConstList ';'
        ;

    ambientConstList:
        ambientConst (',' ambientConst)*
        ;

    ambientConst:
        identifier (':' type)? initializer
        ;

The initializer expression for an ambient constant
must be a numeric or string literal.

.. index::
   ambient constant declaration
   type annotation

|

.. _Ambient Function Declarations:

Ambient Function Declarations
*****************************

.. meta:
    frontend_status: Done
    
.. code-block:: abnf

    ambientFunctionDeclaration:
        ambientFunctionOverloadSignature*
        'function' identifier
        typeParameters? signature
        ;

    ambientFunctionOverloadSignature:
        'declare'? 'function' identifier
          typeParameters? signature ';'
        ;        

A :index:`compile-time error` occurs if:

-  Explicit return type is not specified for an ambient function declaration;
-  Not all overload signatures are marked as ambient in top-level ambient
   overload signatures.

.. index::
   ambient function declaration
   compile-time error
   type annotation
   return type
   ambient function
   overload signature
   top-level ambient overload signature

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
    

**Note**: The modifier ``async`` cannot be used in an ambient context.

.. index::
   ambient function
   ambient function declaration
   ambient function parameter
   default value
   optional parameter
   modifier async
   function body
   ambient context

|

.. _Ambient Class Declarations:

Ambient Class Declarations
**************************

.. meta:
    frontend_status: Done
    
.. code-block:: abnf

    ambientClassDeclaration:
        'class' identifier typeParameters?
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


Ambient field declarations have no initializers:

.. index::
   ambient field declaration
   initializer

.. code-block:: abnf

    ambientFieldDeclaration:
        ambientFieldModifier* identifier ':' type
        ;

    ambientFieldModifier:
        'static' | 'readonly'
        ;       

Ambient constructor, method, and accessor declarations have no bodies:

.. code-block:: abnf

    ambientConstructorDeclaration:
        'constructor' parameters throwMark?
        ;

    ambientMethodDeclaration:
        ambientMethodOverloadSignature*
        ambientMethodModifier* identifier signature
        ;

    ambientMethodOverloadSignature:
        ambientMethodModifier* identifier signature ';'
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

|

.. _Ambient Indexer:

Ambient Indexer
===============

.. meta:
    frontend_status: Done
       
Ambient indexer declarations specify the indexing of a class instance
in an ambient context. This feature is provided for |TS| compatibility:

.. code-block:: abnf

    ambientIndexerDeclaration:
        'readonly'? '[' identifier ':' indexType ']' returnType
        ;

**Restriction**: *indexType* must be ``number``.

.. code-block:: typescript
   :linenos:

    declare class C {
        [index: number]: number
    }


**Note**: *Ambient indexer declaration* is supported in ambient contexts only.
If ambient class implementation is written in |LANG|, then it must conform to
:ref:`Indexable Types`.

.. _Ambient Call Signature:

Ambient Call Signature
======================

.. meta:
    frontend_status: Done
       
Ambient call signature declarations used to specify *callable types*
in an ambient context. This feature is provided for |TS| compatibility:

.. code-block:: abnf

    ambientCallSignatureDeclaration:
        signature
        ;

.. code-block:: typescript
   :linenos:

    declare class C {
        (someArg: number): boolean
    }

**Note**: *Ambient class signature declaration* is supported in ambient contexts
only. If ambient class implementation is written in |LANG|, then it must conform
to :ref:`Callable Types with Invoke Method`.

.. _Ambient Iterable:

Ambient Iterable
================

.. meta:
    frontend_status: Done
       
An ambient iterable declaration indicates that a class instance is iterable.
This feature is provided for |TS| compatibility, and can be used in ambient
contexts only:

.. code-block:: abnf

    ambientIterableDeclaration:
        '[Symbol.iterator]' '(' ')' returnType
        ;

**Restriction**: *returnType* must be a type that
implements ``Iterator`` interface defined in the standard library (see
:ref:`Standard Library`).

.. code-block:: typescript
   :linenos:

    declare class C {
        [Symbol.iterator]: CIterator
    }


**Note**: *Ambient iterable declaration* is supported in ambient contexts only.
If ambient class implementation is written in |LANG|, then it must conform to
:ref:`Iterable Types`.

|

.. _Ambient Interface Declarations:

Ambient Interface Declarations
******************************

.. meta:
    frontend_status: Done
    
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
        | ambientCallSignatureDeclaration
        | ambientIterableDeclaration
        ;

An ambient interface can contain additional members in the same manner as
an ambient class (see :ref:`Ambient Indexer`, :ref:`Ambient Call Signature`,
and :ref:`Ambient Iterable`).

|

.. _Ambient Namespace Declarations:

Ambient Namespace Declarations
******************************

.. meta:
    frontend_status: Done

Namespaces are used to logically group multiple entities. |LANG| supports
ambient namespaces to provide better |TS| compatibility as |TS| often uses
them to specify the platform API or a third-party library API.

.. code-block:: abnf

    ambientNamespaceDeclaration:
        'namespace' identifier '{' ambientNamespaceElement* '}'
        ;

    ambientNamespaceElement:
        ambientNamespaceElementDeclaration | selectiveExportDirective
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

An enumeration type declaration can be prefixed by the keyword ``const`` for
|TS| compatibility. It has no influence on the declared type.

Only exported entities can be accessed outside a namespace.

Namespace nesting is allowed:

.. code-block:: typescript
   :linenos:

    declare namespace A {
        export namespace B {
            export function foo(): void;
        }
    }

A namespace is not an object but just a scope for entities that can only be
accessed by using qualified names.

.. _Implementing Ambient Namespace Declaration:

Implementing Ambient Namespace Declaration
==========================================

:ref:`Annotation namespace` must be applied to classes and functions that
implement the corresponding entities as declared in the ambient namespace.


.. raw:: pdf

   PageBreak


