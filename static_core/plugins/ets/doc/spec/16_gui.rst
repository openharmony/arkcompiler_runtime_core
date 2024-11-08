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

.. _Support for GUI Programming:

Support for GUI Programming
###########################

This section describes the built-in mechanisms that |LANG| provides to create
graphical user interface (GUI) programs. The section is based on the ArkUI
Declarative Core Language Specification available at
<https://gitee.com/arkui-finland/arkui-edsl-core-spec/blob/master/arkui-core-spec.md>.

ArkUI provides a set of extensions of the standard to declaratively describe
the programs’ GUI, and the interaction between the GUI components. |LANG| aims
to adopt ArkUI syntax and semantics as long as they do not contradict |LANG|.

This section is actively under development, and some of its parts can be still
underspecified. In such cases please refer to the original specification.

.. index::
   GUI programming

|

.. _GUI Structs:

GUI Structs
***********

.. meta:
    frontend_status: Partly
    
*GUI structs* are used to define UI components. From the language
perspective, a GUI struct is a restricted form of a non-primitive type
that is designed to define GUI expressively and efficiently.

Each GUI struct is required to implement its *builder* (i.e., the method
responsible for the visual rendering of components).

.. index::
    GUI struct
    struct type
    builder
    implementation
    visual rendering
    method

.. code-block:: abnf

    guiStructDeclaration:
        guiEntryAnnotation? guiComponentAnnotation 'struct' identifier
        guiStructBody
        ;

    guiStructBody:
        '{'
        guiStructBodyDeclaration*
        guiMainComponentBuilderDeclaration
        guiStructBodyDeclaration*
        '}'
        ;

    guiStructBodyDeclaration:
        guiAccessModifier?
        (
        | guiStructFieldDeclaration
        | guiLifeCycleCallbackDeclaration
        | guiCustomComponentBuilderDeclaration
        | classFieldDeclaration
        | classMethodDeclaration
        )
        ;

    guiAccessModifier:
        'private'
        ;

    guiStructFieldDeclaration:
        guiStructFieldAnnotationDeclaration
        variableDeclaration
        ;

    guiStructFieldAnnotationDeclaration:
        guiBuilderParamAnnotation
        | ( guiDataSynchronizationAnnotation guiWatchAnnotation? )
        ;

    guiDataSynchronizationAnnotation:
        guiConsumeAnnotation
        | guiLinkAnnotation
        | guiLocalStorageLinkAnnotation
        | guiLocalStoragePropAnnotation
        | guiObjectLinkAnnotation
        | guiPropAnnotation
        | guiProvideAnnotation
        | guiStateAnnotation
        | guiStorageLinkAnnotation
        | guiStoragePropAnnotation
        ;

    guiMainComponentBuilderDeclaration:
        guiAccessModifier?
        'build'
        '(' ')' (':' 'void')? block
        ;

    guiCustomComponentBuilderDeclaration:
        guiBuilderAnnotation
        guiAccessModifier?
        identifier
        '(' ')' (':' 'void')? block
        ;

    guiLifeCycleCallbackDeclaration:
        guiAccessModifier?
        ( 'aboutToAppear' | 'aboutToDisappear' )
        '(' ')' ':' 'void' block
        ;

|

.. _Builder Function Syntax Conventions:

Builder Function Syntax Conventions
***********************************

The following syntax conventions apply to any builder function (component’s
main builder, component’s custom builder, or stand-alone global custom builder):

-  The required result of ``C({...})`` for any predefined or custom component
   *C* is to initialize and render the component with the data from the
   ``{...}`` block. Specific semantics depends on the implementation. For
   illustrative purposes, it can be expressed as ``(new C({...})).build()``,
   where the object literal ``{...}`` is handled as an initializer of the
   component’s fields.

-  The required result of ``C() {...}`` for any predefined or custom component
   *C* is to initialize and render the component by passing the data
   from the ``{...}`` block to the component’s builder function. Specific
   semantics depends on the implementation. For illustrative purposes, it can
   be expressed as ``new C().build({...})``, where the ``{...}`` block is
   handled as a lambda to be passed as an argument to the builder.

.. index::
   builder
   builder function
   initializer
   initialization
   object literal
   literal
   implementation
   rendering
   argument
   block
   semantics
   component field
   component function
   lambda

|

.. _Builder Function Restrictions:

Builder Function Restrictions
*****************************

Restrictions apply to any builder function (component’s main builder,
component’s custom builder, or stand-alone global custom builder). The
following is not allowed:

-  Declaring local variables;

-  Constructing new objects; and

-  Function calls, except the following:

    -  Calling builders by name.

    -  Calling builders by the reference stored in the
       ``@BuilderParam``-annotated struct field.

    -  Calling a predefined builder ``ForEach`` for iterative rendering.

    -  Calling a function that does not mutate the program state (note that all
       logging functions mutate the state, and thus are prohibited).

    - Using conditional ``if ... else`` syntax.

.. index::
    builder
    builder function
    builder function restriction
    local variable
    construction
    conditional rendering
    iterative rendering
    rendering
    function call
    predefined builder
    builder call
    global custom builder
    call

|

.. _Additional Features:

Additional Features
*******************

|

.. _Methods Returning this:

Methods Returning ``this``
==========================

.. meta:
    frontend_status: Done

A return type of an instance method of a class or a struct can be ``this``.
It means that the return type is the class or struct type the method belongs to.
 
The extended grammar for a method signature (see :ref:`Signatures`) is as
follows:

.. code-block:: abnf

    returnType:
        ':' (type | 'this')
        ;


The only result that is allowed to be returned from such a method is ``this``:

.. code-block:: typescript
   :linenos:

    class C {
        foo(): this {
            return this
        }
    }

.. index::
    return type
    instance method
    struct
    class
    method signature
    signature
    subclass

The return type of an overridden method in a subclass must also be ``this``:

.. code-block:: typescript
   :linenos:

    class D extends C {
        foo(): this {
            return this
        }
    }

    let x = new C().foo() // type of 'x' is 'C'
    let y = new D().foo() // type of 'y' is 'D'

Otherwise, a :index:`compile-time error` occurs.

|

.. raw:: pdf

   PageBreak
