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

.. _Compilation Units:

Compilation Units
#################

.. meta:
    frontend_status: Done

Programs are structured as sequences of elements ready for compilation, i.e.,
compilation units. Each compilation unit creates its own scope (see
:ref:`Scopes`). The compilation unit’s variables, functions, classes,
interfaces, or other declarations are only accessible (see :ref:`Accessible`)
within such scope if not explicitly exported.

A variable, function, class, interface, or other declarations exported from
some compilation unit must be imported first by the compilation unit that
needs to use them.

.. Only exported declarations are available for the 3rd party tools and programs written in other programming languages.

There are three kinds of compilation units:

- *Separate modules* (discussed below),
- *Declaration modules* (discussed in detail in :ref:`Declaration Modules`), and
- *Packages* (discussed in detail in :ref:`Packages`).


The syntax of *compilation units* is presented below:

.. code-block:: abnf

    compilationUnit:
        separateModuleDeclaration
        | packageDeclaration
        | declarationModule
        ;

    packageDeclaration:
        packageModule+
        ;

All modules (both separate modules and packages) are stored in a file
system or a database (see :ref:`Compilation Units in Host System`).

.. index::
   compilation unit
   compilation
   scope
   variable
   function
   class
   interface
   declaration
   access
   accessibility
   export
   separate module
   package
   declaration module
   storage
   file system
   database

|

.. _Separate Modules:

Separate Modules
****************

.. meta:
    frontend_status: Done

*Separate module* is a module without a package header. A *separate module*
can optionally consist of the following four parts:

#. Import directives that enable referring imported declarations in a module;

#. Top-level declarations;

#. Top-level statements; and

#. Re-export directives.


The syntax of *separate module* is presented below:

.. code-block:: abnf

    separateModuleDeclaration:
        importDirective* (topDeclaration | topLevelStatements | exportDirective)*
        ;

Every module can directly use all exported entities from the core packages of
the standard library (see :ref:`Standard Library Usage`).

.. code-block:: typescript
   :linenos:

    // Hello, world! module
    function main() {
      console.log("Hello, world!") // console is defined in the standard library
    }

.. index::
   separate module
   package header
   import directive
   imported declaration
   module
   entity
   package
   top-level declaration
   top-level statement
   re-export directive
   import
   console
   standard library
   core package

|

.. _Import Directives:

Import Directives
*****************

.. meta:
    frontend_status: Partly
    todo: syntax is updated

*Import directives* make entities exported from other compilation units (see
:ref:`Compilation Units`) available for use in the current compilation unit by
using different binding forms. These directives have no effect during the
program execution.

An import declaration has the following two parts:

-  Import path that determines which compilation unit to import from;

-  Import bindings that define what entities, and in what form (either
   qualified or unqualified) the current compilation unit can use.

.. index::
   import directive
   compilation unit
   export
   entity
   binding
   module
   import declaration
   import path
   import binding
   qualified form
   unqualified form

The syntax of *import directives* is presented below:

.. code-block:: abnf

    importDirective:
        'import' 'type'? bindings 'from' importPath
        ;

    bindings:
        defaultBinding
        | (defaultBinding ',')? allBinding
        | (defaultBinding ',')? selectiveBindings
    ;

    allBinding:
        '*' bindingAlias
        ;

    bindingAlias:
        'as' identifier
        ;

    defaultBinding:
        identifier
        ;

    selectiveBindings:
        nameBinding (',' nameBinding)*
        ;

    nameBinding:
        identifier bindingAlias?
        | 'default' 'as' identifier
        ;

    importPath:
        StringLiteral
        ;

Each binding adds a declaration or declarations to the scope of a module
or a package (see :ref:`Scopes`). Any declaration added so must be
distinguishable in the declaration scope (see
:ref:`Distinguishable Declarations`).

Import with ``type`` modifier is discussed in :ref:`Import Type Directive`.

A :index:`compile-time error` occurs if:

-  Declaration added to the scope of a module or a package by a binding is
   not distinguishable;
-  Compilation unit imports itself directly: ``importPath`` refers to the
   file in which the current module is stored;
-  Import in a package module refers to a file which contains a package
   module.


.. code-block:: typescript
   :linenos:

    // File1
    package X
    import * as XX from "File2" /* This is in fact import of the same
                                   compilation unit. Compile-time error! */
    import * as YY from "File3" /* This is import of one package module but not
                                   the entire package. Compile-time error! */

    // File2
    package X
    import * as ZZ fromn "File2" // Import the same file. Compile-time error!

    // File3
    package Y


.. index::
   binding
   declaration
   module
   package
   scope
   distinguishable declaration
   declaration scope
   import directive

|

.. _Bind All with Qualified Access:

Bind All with Qualified Access
==============================

.. meta:
    frontend_status: Done

Import binding ``* as A`` binds the single named entity *A* to the
declaration scope of the current module.

A qualified name consisting of *A* and the name of entity ``A.name`` is used
to access any entity exported from the compilation unit as defined by the
*import path*.

+---------------------------------+--+-------------------------------+
|   Import                        |  |   Usage                       |
+=================================+==+===============================+
|                                                                    |
+---------------------------------+--+-------------------------------+
| .. code-block:: typescript      |  | .. code-block:: typescript    |
|                                 |  |                               |
|     import * as Math from "..." |  |     let x = Math.sin(1.0)     |
+---------------------------------+--+-------------------------------+

This form of import is recommended because it simplifies the reading and
understanding of the source code when all exported entities are prefixed with
the name of the imported compilation unit.

.. index::
   import binding
   import
   qualified name
   entity
   declaration scope
   module
   entity
   name
   access
   export
   compilation unit
   import path

|

.. _Default Import Binding:

Default Import Binding
======================

.. meta:
    frontend_status: Done

Default import binding allows importing a declaration exported from some
module as default export. Knowing the actual name of a declaration is not
required as the new name is given at importing.
A :index:`compile-time error` occurs if another form of import is used to
import an entity initially exported as default.

There are two forms of *default import binding*:

- Single identifier;
- Special form of selective import with the keyword ``default``.

.. code-block:: typescript
   :linenos:

    import DefaultExportedItemBindedName from ".../someFile"
    import {default as DefaultExportedItemNewName} from  ".../someFile"
    function foo () {
      let v1 = new DefaultExportedItemBindedName()
      // instance of class 'SomeClass' to be created here
      let v2 = new DefaultExportedItemNewName()
      // instance of class 'SomeClass' to be created here
    }

    // SomeFile
    export default class SomeClass {}

    // Or
    class SomeClass {}
    export default SomeClass

.. index::
   import binding
   entity
   import
   declaration
   export
   module

|

.. _Selective Binding:

Selective Binding
=================

.. meta:
    frontend_status: Done


*Selective binding* allows to bind an entity exported as *identifier*,
or an entity exported by default (see :ref:`Default Import Binding`).

Binding with *identifier* binds an exported entity with the name
*identifier* to the declaration scope of the current module. If no *binding
alias* is present, then the entity is added to the declaration scope under
the original name. Otherwise, the identifier specified in *binding alias*
is used. In the latter case, the bounded entity is no longer accessible (see
:ref:`Accessible`) under the original name.

If the *identifier* denotes several exported overloaded functions (see
:ref:`Function, Method and Constructor Overloading`), then all these functions
are added to the declaration scope. This situation is represented in the
examples below:

.. index::
   import binding
   simple name
   identifier
   export
   name
   declaration scope
   overloaded function
   entity
   access
   accessibility
   bound entity
   binding

.. code-block:: typescript
   :linenos:

    export const PI = 3.14
    export function sin(d: number): number {}

The import path of the module is now irrelevant:

+---------------------------------+--+--------------------------------------+
|   Import                        |  |   Usage                              |
+=================================+==+======================================+
|                                                                           |
+---------------------------------+--+--------------------------------------+
| .. code-block:: typescript      |  | .. code-block:: typescript           |
|                                 |  |                                      |
|     import {sin} from "..."     |  |     let x = sin(1.0)                 |
|                                 |  |     let f: float = 1.0               |
+---------------------------------+--+--------------------------------------+
|                                                                           |
+---------------------------------+--+--------------------------------------+
| .. code-block:: typescript      |  | .. code-block:: typescript           |
|                                 |  |                                      |
|     import {sin as Sine} from " |  |     let x = Sine(1.0) // OK          |
|         ..."                    |  |     let y = sin(1.0) /* Error ‘sin’  |
|                                 |  |        is not accessible */          |
+---------------------------------+--+--------------------------------------+

A single import statement can list several names:

+-------------------------------------+--+---------------------------------+
|   Import                            |  |   Usage                         |
+=====================================+==+=================================+
|                                                                          |
+-------------------------------------+--+---------------------------------+
| .. code-block:: typescript          |  | .. code-block:: typescript      |
|                                     |  |                                 |
|     import {sin, PI} from "..."     |  |     let x = sin(PI)             |
+-------------------------------------+--+---------------------------------+
|                                                                          |
+-------------------------------------+--+---------------------------------+
| .. code-block:: typescript          |  | .. code-block:: typescript      |
|                                     |  |                                 |
|     import {sin as Sine, PI} from " |  |     let x = Sine(PI)            |
|       ..."                          |  |                                 |
+-------------------------------------+--+---------------------------------+

Complex cases with several bindings mixed on one import path are discussed
below in :ref:`Several Bindings for One Import Path`.

.. index::
   import statement
   import path
   binding

|

.. _Import Type Directive:

Import Type Directive
=====================

.. meta:
    frontend_status: Partly
    todo: no CTE for type import

An import directive can have ``type`` modifier exclusively to be more
compatibile with |TS| syntactically, see also :ref:`Export Type Directive`.
|LANG| does not support any additional semantic checks for entities
imported using *import type* directives.

The following code illustrates semantic checks done by |TS|
but not by |LANG| compiler:

.. code-block:: typescript
   :linenos:

    // File module.ets
    console.log ("Module initialization code")

    export class Class1 {/*body*/}

    class Class2 {}
    export type {Class2}

    // MainProgram.ets

    import {Class1} from "./module.ets"
    import type {Class2} from "./module.ets"

    let c1 = new Class1() // OK
    let c2 = new Class2() // Compile-time error in Typescript, OK in ArkTS

.. index::
   import binding
   export type
   import
   import type

|

.. _Import Path:

Import Path
===========

.. meta:
    frontend_status: Done

*Import path* is a string literal---represented as a combination of the
slash character '``/``' and a sequence alpha-numeric characters---that
determines how an imported compilation unit must be placed.

The slash character '``/``' is used in import paths irrespective of the host
system. The backslash character is not used in this context.

In most file systems, an import path looks like a file path. *Relative* (see
below) and *non-relative* import paths have different *resolutions* that map
the import path to a file path of the host system.

.. index::
   import binding
   import path
   alpha-numeric character
   import
   compilation
   import path
   context
   file system
   relative import path
   non-relative import path
   resolution

The compiler uses the following rule to define the kind of imported
compilation units, and the exact placement of the source code:

-  If *import path* refers to a folder denoted by the last name in the resolved
   file path, then the compiler imports the package that resides in the
   folder. The source code of the package is comprised of all the |LANG| source
   files in the folder.

   Otherwise, the compiler imports the module the *import path* refers to.
   The source code of the module is the file with the extension provided
   within the import path, or---if none is so provided---appended by the
   compiler.

-  If *import path* refers to both a declaration module and a separate module or
   package with the same name, then the reference to the separate module or
   package prevails.

.. index::
   compilation unit
   import path
   file path
   import
   resolution
   host system
   source code
   package
   module
   folder
   extension
   resolving
   filename
   separate module

A *relative import path* starts with '``./``' or '``../``' as in the following
examples:

.. code-block:: typescript
   :linenos:

    "./components/entry"
    "../constants/http"

Resolving *relative import* is relative to the importing file. *Relative
import* is used on compilation units to maintain their relative location.

.. code-block:: typescript
   :linenos:

    import * as Utils from "./mytreeutils"

Other import paths are *non-relative* as in the examples below:

.. code-block:: typescript
   :linenos:

    "/net/http"
    "std/components/treemap"

.. index::
   relative import path
   relative import
   non-relative import

Resolving a *non-relative path* depends on the compilation environment. The
definition of the compiler environment can be particularly provided in a
configuration file or environment variables.

The *base URL* setting is used to resolve a path that starts with '``/``'.
*Path mapping* is used in all other cases. Resolution details depend on
the implementation. For example, the compilation configuration file can contain
the following lines:

.. code-block:: typescript
   :linenos:

    "baseUrl": "/home/project",
    "paths": {
        "std": "/arkts/stdlib"
    }

In the example above, ``/net/http`` is resolved to ``/home/project/net/http``,
and ``std/components/treemap`` to ``/arkts/stdlib/components/treemap``.

File name, placement, and format are implementation-specific.

.. index::
   relative import path
   non-relative import path
   compilation environment
   compiler environment
   imported file
   compilation unit
   relative location
   configuration file
   environment variable
   resolving
   base URL
   path mapping
   resolution
   implementation

|

.. _Several Bindings for One Import Path:

Several Bindings for One Import Path
====================================

.. meta:
    frontend_status: Done

The same bound entities can use the following:

- Several import bindings,
- One import directive, or several import directives with the same import path:

+---------------------------------+-----------------------------------+
|                                 |                                   |
+---------------------------------+-----------------------------------+
|                                 | .. code-block:: typescript        |
| In one import directive         |                                   |
|                                 |     import {sin, cos} from "..."  |
+---------------------------------+-----------------------------------+
|                                 | .. code-block:: typescript        |
| In several import directives    |                                   |
|                                 |     import {sin} from "..."       |
|                                 |     import {cos} from "..."       |
+---------------------------------+-----------------------------------+

No conflict occurs in the above example, because the import bindings
define disjoint sets of names.

The order of import bindings in an import declaration has no influence
on the outcome of the import.

The rules below prescribe what names must be used to add bound entities
to the declaration scope of the current module if multiple bindings are
applied to a single name:

.. index::
   import binding
   bound entity
   import directive
   import path
   import declaration
   import outcome
   declaration scope
   entity
   binding

+-----------------------------+----------------------------+------------------------------+
|   Case                      |   Sample                   |   Rule                       |
+=============================+============================+==============================+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | OK. The compile-time         |
| without an alias in several |      import {sin, sin}     | warning is recommended.      |
| bindings.                   |         from "..."         |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is used explicitly   |                            | OK. No warning.              |
| without alias in one        |     import {sin}           |                              |
| binding.                    |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | OK. Both the name and        |
| without alias, and          |     import {sin}           | qualified name can be used:  |
| implicitly with alias.      |        from "..."          |                              |
|                             |                            | sin and M.sin are            |
|                             |     import * as M          | accessible.                  |
|                             |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | OK. Only alias is accessible |
| with alias.                 |                            | for the name, but not the    |
|                             |     import {sin as Sine}   | original name:               |
|                             |       from "..."           |                              |
|                             |                            | - Sine is accessible;        |
|                             |                            | - sin is not accessible.     |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly        |                            | OK. Both options can be      |
| used with alias, and        |                            | used:                        |
| implicitly with alias.      |     import {sin as Sine}   |                              |
|                             |        from "..."          | - Sine is accessible;        |
|                             |                            |                              |
|                             |     import * as M          | - M.sin is accessible.       |
|                             |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | OK. Both aliases are         |
| with alias several times.   |                            | accessible. But warning can  |
|                             |     import {sin as Sine,   | be displayed.                |
|                             |        sin as SIN}         |                              |
|                             |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+

.. index::
   name
   import
   alias
   access

|

.. _Standard Library Usage:

Standard Library Usage
**********************

.. meta:
    frontend_status: Done
    todo: now core, containers, math and time are also imported because of stdlib internal dependencies
    todo: fix stdlib and tests, then import only core by default
    todo: add escompat to spec and default

All entities exported from the core packages of the standard library (see
:ref:`Standard Library`) are accessible as simple names (see :ref:`Accessible`)
in any compilation unit across all its scopes. Using these names as
programmer-defined entities causes to a :index:`compile-time error` in
accordance to :ref:`Distinguishable Declarations`.

.. code-block:: typescript
   :linenos:

    console.log("Hello, world!")
        // variable 'console' is defined in the standard library

.. index::
   compilation unit
   entity
   export
   accessibility
   package
   access
   simple name
   standard library
   access
   distinguishable declaration

|

.. _Declaration Modules:

Declaration Modules
*******************

.. meta:
    frontend_status: Done

*Declaration module* is a special kind of compilation units that can be
imported by using :ref:`Import Directives`. A declaration module contains
:ref:`Ambient Declarations` and :ref:`Type Alias Declaration` only. An ambient
declaration declared in a declaration module must be fully defined elsewhere.

The syntax of *declaration module* is presented below:

.. code-block:: abnf

    declarationModule:
        importDirective*
        ( 'export'? ambientDeclaration
        | selectiveExportDirective
        )*
        ;

.. index::
   declaration module
   compilation unit
   import
   ambient declaration
   declaration module

The following example shows how ambient functions can be declared and exported:

.. code-block:: typescript
   :linenos:

    declare function foo()
    export declare function goo()
    export { foo }

Optional usage of the keyword ``export`` means that a particular declaration
is used by other exported declarations. However, it is not exported on its own,
and cannot be used by modules that import this declaration module:

.. index::
   declaration
   export
   keyword export
   declaration module
   ambient function

.. code-block:: typescript
   :linenos:

   // module with implementation
   class A {} // It is not exported
   export class B {
     public a: A = new A // the field is exported but its type is not
   }
   export function process_field (p: A) {}

   // declaration module should look like
   declare class A {}
   export declare class B {
     public a: A // the field is exported but its type is not
   }
   export function process_field (p: A)

   // Module which uses B and process_field
   import * as m from "path_to_declaration_module"

   let b = new m.B  // B instance is created
   m.process_field (b.a) // exported field is passed to function as an argument

   let a = new m.A // compile-time error as A is not exported

How declaration modules are stored in the file system, and whether the manner
of storage of a declaration module differs from the manner of storage of other
modules is determined by a particular implementation.

.. index::
   declaration
   declaration module
   file system
   storage
   implementation

|

.. _Top-Level Declarations:

Top-Level Declarations
**********************

.. meta:
    frontend_status: Done

*Top-level declarations* declare top-level types (``class``, ``interface``,
or ``enum`` see :ref:`Type Declarations`), top-level variables (see
:ref:`Variable Declarations`), constants (see :ref:`Constant Declarations`),
functions (see :ref:`Function Declarations`), or namespaces (see
:ref:`Namespace Declarations`). Top-level declarations can be exported.

The syntax of *top-level declarations* is presented below:

.. code-block:: abnf

    topDeclaration:
        ('export' 'default'?)?
        annotationUsage?
        ( typeDeclaration
        | variableDeclarations
        | constantDeclarations
        | functionDeclaration
        | functionWithReceiverDeclaration
        | accessorWithReceiverDeclaration
        | namespaceDeclaration
        | ambientDeclaration
        | annotationDeclaration
        )
        ;

.. code-block:: typescript
   :linenos:

    export let x: number[], y: number

.. index::
   top-level declaration
   top-level type
   top-level variable
   class
   interface
   enum
   variable
   constant
   constant declaration
   namespace
   export
   function
   variable declaration
   type declaration

The usage of annotations is discussed in :ref:`Using Annotations`.

|

.. _Exported Declarations:

Exported Declarations
=====================

.. meta:
    frontend_status: Done

Top-level declarations can use export modifiers that make the declarations
accessible (see :ref:`Accessible`) in other compilation units by using import
(see :ref:`Import Directives`). The declarations not marked as exported can be
used only inside the compilation unit they are declared in.

.. code-block:: typescript
   :linenos:

    export class Point {}
    export let Origin = new Point(0, 0)
    export function Distance(p1: Point, p2: Point): number {
      // ...
    }

.. index::
   top-level declaration
   exported declaration
   export modifier
   access
   accessible declaration
   accessibility
   compilation unit
   import directive
   declaration

In addition, only one top-level declaration can be exported by using the default
export directive. It allows specifying no declared name when importing (see
:ref:`Default Import Binding` for details). A :index:`compile-time error`
occurs if more than one top-level declaration is marked as ``default``.

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    export default let PI = 3.141592653589

.. index::
   top-level declaration
   export
   default export directive
   import
   import binding

Another supported form of *export default* is using an expression as export
default target. This export directive effectively means that an anonymous
constant variable is created with a value equal to the value of the expression
evaluation result. The export can be imported only by providing a name for the
constant variable that is exported by using this export directive. Otherwise, a
:index:`compile-time error` occurs.

.. code-block:: typescript
   :linenos:

    // File1
    class A {
      foo () {}
    }
    export default new A

    // File2
    import {default as a} from "File1"

    a.foo()  // Calling method foo() of class A where 'a' is an instance of type A
    a = new A // Compile-time error as 'a' is a constant variable

    // File3
    import * as a from "File1" /* Compile-time error: such form of import
                                  cannot be used for the default export */


.. index::
   exported declaration
   top-level declaration
   modifier export
   constant variable
   evaluation result
   export
   export default
   export directive
   accessibility
   declaration
   export
   declared name
   compilation unit
   default export directive
   import

|

.. _Namespace Declarations:

Namespace Declarations
**********************

.. meta:
    frontend_status: Done

*Namespace declaration* introduces the qualified name to be used as a
qualifier for access to each exported entity of the namespace.

The syntax of *namespace declarations* is presented below:

.. code-block:: abnf

    namespaceDeclaration:
        'namespace' qualifiedName
        '{' topDeclaration* initializerBlock? topDeclaration* '}'
        ;

Namespace can have an initializer block (see :ref:`Static Initialization`).

An usage example is presented below:

.. code-block:: typescript
   :linenos:

    namespace NS1 {
        export function foo() { ... }
        export let variable = 1234
        export const constant = 1234
        export let someVar: SomeType
        static {
            someVar = new SomeType
        }
        export bar
    }

    function bar() {}

    if (NS1.variable == NS1.constant) {
        NS1.variable = 4321
    }
    NS1.bar()

.. index::
   namespace
   namespace declaration
   qualifier
   access
   entity
   export
   qualified name
   initializer block
   namespace variable

**Note**. A namespace must be exported to be used in another compilation unit.

.. code-block:: typescript
   :linenos:

    // File1
    namespace Space1 {
        export function foo() { ... }
        export let variable = 1234
        export const constant = 1234
    }
    export namespace Space2 {
        export function foo(p: number) { ... }
        export let variable = "1234"
    }

    // File2
    import {Space2 as Space1} from "File1"
    if (Space1.variable == Space1.constant) { // compile-time error - there is no variable or constant called 'constant'
        Space1.variable = 4321 // compile-time error - incorrect assignment as type 'number' is not compatible with type 'string'
    }
    Space1.foo()     // compile-time error - there is no function 'foo()'
    Space1.foo(1234) // OK

.. index::
   namespace
   compilation unit

**Note**. Embedded namespaces are allowed.

.. code-block:: typescript
   :linenos:

    namespace ExternalSpace {
        export function foo() { ... }
        export let variable = 1234
        export namespace EmbeddedSpace {
            export const constant = 1234
        }
    }

    if (ExternalSpace.variable == ExternalSpace.EmbeddedSpace.constant) {
        ExternalSpace.variable = 4321
    }

.. index::
   embedded namespace

**Note**. Namespaces with identical namespace names in a single compilation
unit merge their exported declarations into a single namespace. A duplication
causes a :index:`compile-time error`. Exported and non-exported declarations
with the same name are also considered a :index:`compile-time error`.
Only one of the merging namespaces can have an initializer. Otherwise, a
:index:`compile-time error` occurs.

.. index::
   namespace
   namespace name
   compilation unit
   export
   declaration
   initializer

.. code-block:: typescript
   :linenos:

    // One source file
    namespace A {
        export function foo() { console.log ("1st A.foo() exported") }
        function bar() {  }
        export namespace C {
            export function too() { console.log ("1st A.C.too() exported") }
        }
    }

    namespace B {  }

    namespace A {
        export function goo() {
            A.foo() // calls exported foo()
            foo()   /* calls exported foo() as well as all A namespace
                       declarations are merged into one */
            A.C.moo()
        }
        //export function foo() {  }
        // Compile-time error as foo() was already defined

        // function foo() { console.log ("2nd A.foo() non-exported") }
        // Compile-time error as foo() was already defined as exported
    }

    namespace A.C {
        export function moo() {
            too() // too()  accessible when namespace C and too() are both exported
            A.C.too()

        }
    }

    A.goo()

    // File1
    package P
    namespace A {
        export function foo() { ... }
        export function bar() { ... }
    }

    // File2
    package P
    namespace A {
        function goo() { bar() }  // exported bar() is accessible in the same namespace
        export function foo() { ... }  // Compile-time error as foo() was already defined
    }

    namespace X {
        static {}
    }
    namespace X {
        static {} // Compile-time error as only one initializer allowed
    }

**Note**. A namespace name can be a qualified name. It is a shortcut notation of
embedded namespaces as represented below:

.. index::
   namespace
   shortcut notation
   embedded namespace

.. code-block:: typescript
   :linenos:

    namespace A.B {
        /*some declarations*/
    }

The code above is the shortcut to the following code:

.. code-block:: typescript
   :linenos:

    namespace A {
        export namespace B {
          /*some declarations*/
        }
    }

This code illustrates the usage of declarations in the following case:

.. code-block:: typescript
   :linenos:

    namespace A.B.C {
        export function foo() { ... }
    }

    A.B.C.foo() // Valid function call, as 'B' and 'C' are implicitly exported

If an ambient namespace (see :ref:`Ambient Namespace Declarations`) belongs to
a separate module (see :ref:`Separate Modules`), then all ambient namespace
declarations are accessible across all declarations and top-level statements of
the separate module.

.. code-block:: typescript
   :linenos:

    declare namespace A {
        function foo(): void
        type X = Array<number>
    }

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
   namespace
   separate module
   ambient namespace
   declaration
   accessible declaration
   access
   accessibility
   top-level statement

|

.. _Export Directives:

Export Directives
*****************

.. meta:
    frontend_status: Done

*Export directive* allows the following:

-  Specifying a selective list of exported declarations with optional
   renaming; or
-  Specifying a name of one declaration; or
-  Exporting a type; or
-  Re-exporting declarations from other compilation units.

The syntax of *export directive* is presented below:

.. code-block:: abnf

    exportDirective:
        selectiveExportDirective
        | singleExportDirective
        | exportTypeDirective
        | reExportDirective
        ;

.. index::
   export directive
   export
   declaration
   renaming
   re-export
   re-exporting declaration
   compilation unit

|

.. _Selective Export Directive:

Selective Export Directive
==========================

.. meta:
    frontend_status: Done

Top-level declarations can be made *exported* by using a selective export
directive. The selective export directive provides an explicit list of names
of the declarations to be exported. Optional renaming allows having the
declarations exported with new names.

The syntax of *selective export directive* is presented below:

.. code-block:: abnf

    selectiveExportDirective:
        'export' selectiveBindings
        ;

A selective export directive uses the same *selective bindings* as an import
directive:

.. code-block:: typescript
   :linenos:

    export { d1, d2 as d3}

The above directive exports 'd1' by its name, and 'd2' as 'd3'. The name 'd2'
is not accessible (see :ref:`Accessible`) in the modules that import this
module.

.. index::
   selective export directive
   top-level declaration
   export
   export directive
   declaration
   directive
   renaming
   import directive
   selective binding
   module
   access
   accessibility

|

.. _Single Export Directive:

Single Export Directive
=======================

.. meta:
    frontend_status: Partly
    todo: changes in export syntax

*Single export directive* allows specifying the declaration to be exported from
the current compilation unit by using the declaration's own name, or anonymously.

The syntax of *single export directive* is presented below:

.. code-block:: abnf

    singleExportDirective:
        'export'
        ( identifier
        | 'default' (expression | identifier)
        | '{' identifier 'as' 'default' '}'
        )
        ;

.. index::
   export directive
   declaration
   compilation unit
   own name
   syntax

If ``default`` is present, then only one such export directive is possible in
the current compilation unit. Otherwise, a :index:`compile-time error` occurs.

The directive in the example below exports variable 'v' by its name:

.. code-block:: typescript
   :linenos:

    export v
    let v = 1


The directive in the example below exports class 'A' by its name as default
export:

.. code-block:: typescript
   :linenos:

    class A {}
    export default A
    export {A as default} // such syntax is also acceptable

.. index::
   export directive
   compilation unit
   directive

The directive in the example below exports a constant variable anonymously:

.. code-block:: typescript
   :linenos:

    class A {}
    export default new A


*Single export directive* acts as re-export when the declaration referred to by
*identifier* is imported.

.. code-block:: typescript
   :linenos:

    import {v} from "some location"
    export v

.. index::
   export
   directive
   constant variable
   export directive
   re-export
   identifier
   import

|

.. _Export Type Directive:

Export Type Directive
=====================

.. meta:
    frontend_status: Done

An export directive can have ``type`` modifier exclusively to be more
compatibile with |TS| syntactically, see also :ref:`Import Type Directive`.

The syntax of *export type directive* is presented below:

.. code-block:: abnf

    exportTypeDirective:
        'export' 'type' selectiveBindings
        ;


|LANG| does not support any additional semantic checks for entities
exported using *export type* directive.

.. index::
   export
   declaration
   export type
   export directive
   syntax

|

.. _Re-Export Directive:

Re-Export Directive
===================

.. meta:
    frontend_status: Partly
    todo: syntax was changed

In addition to exporting what is declared in the module, it is possible to
re-export declarations that are part of other modules' export.
A particular declaration or all declarations can be re-exported from a module.
When re-exporting, new names can be given. This action is similar to importing
but has the opposite direction.

The syntax of *re-export directive* is presented below:

.. code-block:: abnf

    reExportDirective:
        'export'
        ('*' bindingAlias?
        | selectiveBindings
        | '{' 'default' bindingAlias? '}'
        )
        'from' importPath
        ;

.. index::
   export
   module
   declaration
   re-export declaration
   re-export
   import

An ``importPath`` cannot refer to the file the current module is stored in.
Otherwise, a :index:`compile-time error` occurs.

The re-exporting practice is represented in the following examples:

.. code-block:: typescript
   :linenos:

    export * from "path_to_the_module" // re-export all exported declarations
    export * as qualifier from "path_to_the_module"
       // re-export all exported declarations with qualification
    export { d1, d2 as d3} from "path_to_the_module"
       // re-export particular declarations some under new name
    export {default} from "path_to_the_module"
       // re-export default declaration from the other module
    export {default as name} from "path_to_the_module"
       // re-export default declaration from the other module under 'name'

.. index::
   import path
   module
   storage
   re-export

|

.. _Top-Level Statements:

Top-Level Statements
********************

.. meta:
    frontend_status: Done

A separate module can contain sequences of statements that logically
comprise one sequence of statements.

The syntax of *top-level statements* is presented below:

.. code-block:: abnf

    topLevelStatements:
        statement*
        ;

.. index::
   top-level statement
   separate module
   module
   statement

A module can contain any number of top-level statements that logically
merge into a single sequence in the textual order:

.. code-block:: typescript
   :linenos:

      statements_1
      /* top-declarations except constant and variable declarations */
      statements_2

The sequence above is equal to the following:

.. code-block:: typescript
   :linenos:

      /* top-declarations except constant and variable declarations */
      statements_1; statements_2


This situation is represented by the example below:

.. index::
   top-level statement

.. code-block:: typescript
   :linenos:


   // The actual text combination of the statements and declarations
   console.log ("Start of top-level statements")
   type A = number | string
   let a: A = 56
   function foo () {
      console.log (a)
   }
   a = "a string"


   // The logically ordered text - declarations then statements
   type A = number | string
   function foo () {
      console.log (a)
   }
   console.log ("Start of top-level statements")
   let a: A = 56
   a = "a string"

.. index::
   top-level statement
   declaration
   separate module
   statement

- If a separate module is imported by some other module, then the semantics of
  top-level statements is to initialize the imported module. It means that all
  top-level statements are executed only once before a call to any other
  function, or before the access to any top-level variable of the separate
  module.
- If a separate module is used as a program, then top-level statements are used
  as a program entry point (see :ref:`Program Entry Point`). The set of
  top-level statements being empty implies that the program entry point is also
  empty and does nothing. If a separate module has the ``main`` function, then
  it is executed after the execution of the top-level statements.

.. index::
   separate module
   semantics
   top-level statement
   initialization
   import
   module
   call
   access
   accessibility
   entry point
   function

.. code-block:: typescript
   :linenos:

      // Source file A
      { // Block form
        console.log ("A.top-level statements")
      }

      // Source file B
      import * as A from "Source file A "
      function main () {
         console.log ("B.main")
      }

The output is as follows:

A. Top-level statements,
B. Main.

.. code-block:: typescript
   :linenos:

      // One source file
      console.log ("A.Top-level statements")
      function main () {
         console.log ("B.main")
      }

A :index:`compile-time error` occurs if top-level statements contain a
return statement (:ref:`Expression Statements`).

The execution of top-level statements means that all statements, except type
declarations, are executed one after another in the textual order of their
appearance within the module until an error situation is thrown (see
:ref:`Errors`), or last statement is executed.

.. index::
   top-level statement
   return statement
   statement
   type declaration
   module
   error

|

.. _Program Entry Point:

Program Entry Point
*******************

.. meta:
    frontend_status: Done

Separate modules or packages can act as programs (applications). Program
execution starts from the execution of a *program entry point* which can
be of the following two kinds:

- Top-level statements for separate modules (see :ref:`Top-Level Statements`); or
- Entry point function (see below).

.. index::
   separate module
   package
   top-level statement
   return statement
   execution
   entry point

A separate module can have the following forms of entry point:

- Sole entry point function (``main`` or other as described below);
- Sole top-level statement (the first statement in the top-level statements
  acts as the entry point);
- Both top-level statement and entry point function (same as above, plus the
  function called after the top-level statement execution is completed).

.. index::
   separate module
   entry point

A package can have a sole entry point function (``main`` or other as
described below).

Entry point functions have the following features:

- Any exported top-level function can be used as an entry point. An entry point
  is selected by the compiler, the execution environment, or both;
- Entry point function must either have no parameters, or have one parameter of
  type ``string[]`` that provides access to the arguments of a program command
  line;
- Entry point function return type is either ``void`` (see :ref:`Type void`) or
  ``int``;
- Entry point function cannot have overloading;
- Entry point function is called ``main`` by default.

.. index::
   package
   entry point
   function
   parameter
   string type
   access
   argument
   return type
   type void
   type int
   overloading
   top-level statements

The example below represents different forms of valid and invalid entry points:

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    function main() {
      // Option 1: a return type is inferred from the body of main().
      // It will be 'int' if the body has 'return' with the integer expression
      // and 'void' if no return at all in the body
    }

    function main(): void {
      // Option 2: explicit :void - no return in the function body required
    }

    function main(): int {
      // Option 3: explicit :int - return is required
      return 0
    }

    function main(): string { // compile-time error: incorrect main signature
      return ""
    }

    function main(p: number) { // compile-time error: incorrect main signature
    }

    // Option 4: top-level statement is the entry point
    console.log ("Hello, world!")

    // Option 5: top-level exported function
    export function entry() {}

    // Option 5: top-level exported function with command-line arguments
    export function entry(cmdLine: string[]) {}

    // Package example - outputs "Package init" then "Package main"
    package P
    function main () { console.log ("Package main")}
    static { console.log ("Package init") }

|

.. raw:: pdf

   PageBreak
