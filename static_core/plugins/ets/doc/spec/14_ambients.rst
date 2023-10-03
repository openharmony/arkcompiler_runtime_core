.. _Ambients:

Ambient Declarations
####################

Ambient declarations are a way of specifying entities that are declared
somewhere else.

Ambient declarations do not introduce new entities---as regular declarations
do---but instead provide type information for the entities that are included
in a program by external means.

Ambient declarations cannot include executable code, and consequently have
no initializers.

Ambient functions, methods, and constructors have no body.

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
        ( ambientVariableDeclaration 
        | ambientFunctionDeclaration
        | ambientClassDeclaration
        | ambientInterfaceDeclaration
        | ambientNamespaceDeclaration
        | enumDeclaration
        | typeAlias
        )
        ;

A compile-time error occurs if the modifier 'declare' is used in a context
that is already ambient:

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

Ambient Variable Declarations
*****************************

.. code-block:: abnf

    ambientVariableDeclaration:
        ('let' | 'const') ambientVarList ';'
        ;

    variableDeclarationList:
        ambientVar (',' ambientVar)*
        ;

    ambientVar:
        identifier ':' type 
        ;

The type annotation is mandatory for each ambient variable.

.. index::
   ambient variable declaration
   type annotation

|

Ambient Function Declarations
*****************************

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

A compile-time error occurs if:

-  An explicit return type is not specified for an ambient function declaration;
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
    

**Note**: 'async' modifier cannot be used in an ambient context.

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

Ambient Class Declarations
**************************

.. code-block:: abnf

    ambientClassDeclaration:
        classModifier? 'class' identifier typeParameters?
        classExtendsClause? implementsClause?
        ambientClassBodyDeclaration*
        ;

    ambientClassBodyDeclaration:
        accessModifier?
        ( ambientFieldDeclaration 
        | ambientConstructorDeclaration
        | ambientMethodDeclaration
        | ambientAccessorDeclaration
        )
        ;
    

Ambient field declarations have no initializers.

.. index::
   ambient field declaration
   initializer

.. code-block:: abnf

    ambientFieldDeclaration:
        fieldModifier* ('let' | 'const') identifier ':' type
        ;

Ambient constructor, method, and accessor declarations have no bodies.

.. code-block:: abnf

    ambientConstructorDeclaration:
        'constructor' '(' parameterList? ')' throwMark?
        ;

    ambientMethodDeclaration:
        ambientMethodOverloadSignature*
         methodModifier* identifier signature
        ;

    ambientMethodOverloadSignature:
        methodModifier* identifier signature ';'
        ;

    ambientAccessorDeclaration:
        accessorModifier
        ( 'get' identifier '(' ')' returnType 
        | 'set' identifier '(' parameter ')'
        )
        ;       
        

|

Ambient Interface Declarations
******************************

.. code-block:: abnf

    ambientInterfaceDeclaration:
        'interface' identifier typeParameters?
        interfaceExtendsClause? '{' interfaceMember* '}'
        ;

|

Ambient Namespace Declarations
******************************

.. code-block:: abnf

    ambientNamespaceDeclaration:
        'namespace' Identifier '{' ambentNamespaceElement* '}'
        ;

    ambentNamespaceElement:
        'export'? 
        ( ambientVariableDeclaration
        | ambientFunctionDeclaration
        | ambientClassDeclaration
        | ambientInterfaceDeclaration
        | ambientNamespaceDeclaration
        | enumDeclaration
        | typeAlias 
        )
        ;

.. raw:: pdf

   PageBreak


