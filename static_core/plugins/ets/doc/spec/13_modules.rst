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
   separate module
   package
   declaration module

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

|

.. _Separate Module Initializer:

Separate Module Initializer
***************************

.. meta:
    frontend_status: Done

*Separate module* used for import is initialized only once with the
details listed in :ref:`Compilation Unit Initialization`. The initialization
process is performed in the following steps:

- If the separate module has variable or constant declarations (see
  :ref:`Variable and Constant Declarations`), then declaration initializers are
  executed to ensure that all declarations have valid initial values;
- If the separate module has top-level statements (see :ref:`Top-Level Statements`),
  then the statements are also executed.

.. index::
   initializer
   separate module
   initialization
   variable declaration
   compilation unit
   constant declaration
   top-level statement

|

.. _Import Directives:

Import Directives
*****************

.. meta:
    frontend_status: Done

*Import directives* make entities exported from other compilation units (see
also :ref:`Declaration Modules`) available for use in the current compilation
unit by using different binding forms.

An import declaration has the following two parts:

-  Import path that determines which compilation unit to import from;

-  Import binding that defines what entities, and in what form---qualified
   or unqualified---the current compilation unit can use.

Alternatively, a module can be imported without binding simply in order to
run the initialization code.

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

.. code-block:: abnf

    importDirective:
        'import' allBinding|selectiveBindings|defaultBinding|typeBinding
        'from'   importPath
        ;

    allBinding:
        '*' bindingAlias
        ;

    selectiveBindings:
        '{' nameBinding (',' nameBinding)* ','? '}'
        ;

    defaultBinding:
        identifier | ( '{' 'default' 'as' identifier '}' )
        ;

    typeBinding:
        'type' selectiveBindings
        ;

    nameBinding:
        identifier bindingAlias?
        ;

    bindingAlias:
        'as' identifier
        ;

    importPath:
        StringLiteral
        ;

Each binding adds a declaration or declarations to the scope of a module
or a package (see :ref:`Scopes`). Any declaration added so must be
distinguishable in the declaration scope (see
:ref:`Distinguishable Declarations`). A :index:`compile-time error` occurs if:

-  A declaration added to the scope of a module or a package by a binding is
   not distinguishable;
-  ``importPath`` refers to the file in which the current module is stored.

**Note**: Import directives are handled by the compiler during compilation, and
have no effect during program execution. Though they ensure that imported
entities are initialized before use in the current compilation unit.

.. index::
   binding
   declaration
   module
   package
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
| **Import**                      |  | **Usage**                     |
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

.. _Simple Name Binding:

Simple Name Binding
===================

.. meta:
    frontend_status: Done

The import binding ``identifier`` binds an exported entity with the name
``identifier`` to the declaration scope of the current module. The name
``identifier`` can only correspond to multiple entities with ``identifier``
denoting several overloaded functions (see
:ref:`Function, Method and Constructor Overloading`).

The import binding ``identifier as A`` binds an exported entity (entities) with
the name *A* to the declaration scope of the current module. The bounded entity
is no longer accessible (see :ref:`Accessible`) under the name ``identifier``.
It is shown in the following examples:


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
| **Import**                      |  | **Usage**                            |
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
| **Import**                          |  | **Usage**                       |
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
| **Case**                    | **Sample**                 | **Rule**                     |
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
   compile-time error
   name
   import
   alias
   access
   
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

.. _Type Binding:

Type Binding
============

.. meta:
    frontend_status: Done

*Type import binding* allows importing only the type declarations exported from
a module or a package. These declarations can be exported normally, or by
using the *export type* form. The difference between *import* and
*import type* is that *import* imports all exported top-level declarations,
while *import type* imports only exported types.

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
    let c2 = new Class2() // OK, the same

.. index::
   import binding
   declaration
   module
   package
   export type
   import type
   top-level declaration

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

The compiler uses the following rule to define the kind of imported
compilation units, and the exact placement of the source code:

-  If import path refers to a folder denoted by the last name in the resolved
   file path, then the compiler imports the package that resides in the
   folder. The source code of the package is comprised of all the |LANG| source
   files in the folder.

-  Otherwise, the compiler imports the module that the import path refers to.
   The source code of the module is the file with the extension provided
   within the import path, or---if none is so provided---appended by the
   compiler.

.. index::
   import binding
   import path
   string
   string literal
   slash character
   alpha-numeric character
   compilation unit
   file system
   file path
   relative import path
   non-relative import path
   resolution
   host system
   source code
   package
   module
   folder
   extension
   resolving
   filename

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

How declaration modules are stored in the file system and if the storage scheme
of declaration modules differs from the way other modules are stored is
determined by the particular implementation.

.. index::
   declaration
   declaration module
   implementation

|

.. _Compilation Unit Initialization:

Compilation Unit Initialization
*******************************

.. meta:
    frontend_status: Done

*Compilation unit* is a separate module (see :ref:`Separate Module Initializer`)
or a package (see :ref:`Package Initializer`) that is initialized once before
an entity (function, variable, or type), exported from the compilation unit,
is used for the first time.
If a *compilation unit* has an import directive (see :ref:`Import Directives`)
but the imported entities are not actually used, then the imported compilation
unit (separate or package) is initialized before the entry point code (see
:ref:`Program Entry Point`) starts.
If different compilation units are not connected by import, then the order
of compilation unit initialization is not determined.
If there is a cyclic dependency between top-level variable declarations, then a
:index:`compile-time error` occurs.

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    // Source file 1
    import {x} from "Source file 2"
    let y = x // y uses x for its initialization

    // Source file 2
    import {y} from "Source file 1"
    let x = y // x uses y for its initialization

.. index::
   compilation unit
   initialization
   entity
   function
   variable
   type
   import directive
   entry point
   binding
   declaration
   module
   package
   declaration scope
   top-level declaration
   variable

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
   function
   export

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

In addition, only one top-level declaration can be exported by using the default
export scheme. It allows specifying no declared name when importing (see
:ref:`Default Import Binding` for details). A :index:`compile-time error`
occurs if more than one top-level declaration is marked as ``default``.

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    export default let PI = 3.141592653589


One more form of *export default* is supported where expression is put as the
export default target. It can be imported only with providing a name for the 
constant variable exported with help of this scheme. Otherwise a
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

    a.foo()  // Calling method foo() of class A where 'a' is the instance of type A
    a = new A // Compile-time error as 'a' is a constant variable

    // File3
    import * as a from "File1" /* Compile-time error: such form of import
                                  cannot be used for the default export */
 

.. index::
   exported declaration
   top-level declaration
   export modifier
   accessibility
   declaration
   export
   declared name
   compilation unit
   default export scheme
   import

|

.. _Namespace Declarations:

Namespace Declarations
**********************

.. meta:
    frontend_status: Done

*Namespace declaration* introduces the qualified name to be used as a
qualifier for access to each exported entity of the namespace. The appropriate
syntax is presented below:

.. code-block:: abnf

    namespaceDeclaration:
        'namespace' qualifiedName
        '{' topDeclaration* initializerBlock? topDeclaration* '}'
        ;

Namespace can have an initializer block to ensure that all namespace variables
receive initial values. Initialization details are based on
:ref:`Compilation Unit Initialization`, except the parts related to import which
is not applicable to namespaces and :ref:`Initializer Block`.

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
    }

    if (NS1.variable == NS1.constant) {
        NS1.variable = 4321
    }

.. index::
   namespace declaration
   qualifier
   access
   entity
   export
   namespace

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

**Note**. Namespaces with identical namespace names in a single compilation
  unit merge their exported declarations into a single namespace. A duplication
  causes a :index:`compile-time error`. Exported and non-exported declarations
  having the same name is also considered a :index:`compile-time error`.
  Only one of merging namespaces can have an initializer. Otherwise, a
  :index:`compile-time error` occurs.

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

.. code-block:: typescript
   :linenos:

    namespace A.B {
        /*some declarations*/
    }

The code above is the shortcut version of the following code:

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
a separate module (see :ref:`Separate Modules`) then all ambient namespace
declarations are accessible across all declarations and top-level statements of
the separate module.

.. code-block:: typescript
   :linenos:

    declare namespace A {
        function foo(): void
        type X = Array<number>
    }

    A.foo() // Valid function call, as 'foo' is acessible for top-level statements
    function foo () {
        A.foo() // Valid function call, as 'foo' is acessible here as well
    }
    class C {
        method () {
            A.foo() // Valid function call, as 'foo' is acessible here too
            let x: A.X = [] // Type A.X can be used
        }
    }



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
   declaration
   directive
   renaming
   import directive
   selective binding
   module
   access

|

.. _Single Export Directive:

Single Export Directive
=======================

.. meta:
    frontend_status: Done

*Single export directive* allows specifying the declaration to be exported from
the current compilation unit by using the declaration's own name or anonymously. 
Synatx for the both cases is presented below: 

.. code-block:: abnf

    singleExportDirective:
        'export' 
        'default'? identifier |
        'default' expression
        ;


If ``default`` is added, then only one such export directive is possible in
the current compilation unit. Otherwise, a :index:`compile-time error` occurs.

The directive in the example below exports variable 'v' by its name:

.. code-block:: typescript
   :linenos:

    export v
    let v = 1

    class A {}
    export default A

The directive in the example below exports constant variable of some type
anonymously:

.. code-block:: typescript
   :linenos:

    class A {}
    export default new A


*Single export directive* works as re-export when declaration referred by
*identifier* was imported.

.. code-block:: typescript
   :linenos:

    import {v} from "some location"
    export v
   

.. index::
   export directive
   declaration
   export
   compilation unit
   default export scheme

|

.. _Export Type Directive:

Export Type Directive
=====================

.. meta:
    frontend_status: Done

In addition to export that is attached to some declaration, the *export type*
directive can be used in order to do the following:

-  Export *as a type* a particular class or interface already declared; or
-  Export an already declared type under a different name.

The appropriate syntax is presented below:

.. code-block:: abnf

    exportTypeDirective:
        'export' 'type' selectiveBindings
        ;

If a class or an interface is exported in this manner, then its usage is
limited similarly to the limitations described for *import type* directives
(see :ref:`Type Binding`).

If a class or interface is declared exported, but *export type* is applied
to the same class or interface name, then a :index:`compile-time error` occurs.
This situation is represented in the following example:

.. code-block:: typescript
   :linenos:

    class A {}

    export type {A}  // export already declared class type

    export type MyA = A // name MyA is declared and exported

    export type {MyA} // compile-time error as MyA was already exported

.. index::
   export directive
   export
   declaration
   export type
   class
   interface
   name
   import type

|

.. _Re-Export Directive:

Re-Export Directive
===================

.. meta:
    frontend_status: Done

In addition to exporting what is declared in the module, it is possible to
re-export declarations that are part of other modules' export. 
A particular declaration or all declarations can be re-exported from a module.
When re-exporting, new names can be given. This action is similar to importing
but has the opposite direction. The appropriate grammar is presented below:

.. code-block:: abnf

    reExportDirective:
        'export' ('*' | selectiveBindings) 'from' importPath
        ;

An ``importPath`` cannot refer to the file the current module is stored in.
Otherwise, a :index:`compile-time error` occurs.

The re-exporting practice is represented in the following examples:

.. code-block:: typescript
   :linenos:

    export * from "path_to_the_module" // re-export all exported declarations
    export { d1, d2 as d3} from "path_to_the_module"
       // re-export particular declarations some under new name

.. index::
   re-export
   re-export directive
   re-export declaration
   module

.. _Top-Level Statements:

Top-Level Statements
********************

.. meta:
    frontend_status: Done

A separate module can contain sequences of statements that logically
comprise one sequence of statements:

.. code-block:: abnf

    topLevelStatements:
        statement*
        ;

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


It is represented by the example below:


.. code-block:: typescript
   :linenos:


   // The actual text mixture of the statements and declarations 
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
   separate module
   statement
   top-level statement
   sequence

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
   module
   semantics
   top-level statement
   initialization
   import
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
appearance within the module until an erroneous situation is thrown (see
:ref:`Errors`), or last statement is executed.


.. index::
   top-level statement
   return statement

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

A separate module can have the following forms of entry point:

- Sole entry point function (``main`` or other as described below);
- Sole top-level statement (the first statement in the top-level statements
  acts as the entry point);
- Both top-level statement and entry point function (same as above, plus the
  function called after the top-level statement execution is completed).

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
   module
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

.. _Program Exit:

Program Exit
************

.. meta:
    frontend_status: Done

Separate modules and packages can act as programs (applications) and their entry
point is described above (see :ref:`Program Entry Point`).

*Program exit* takes place when:

- All top-level statements (for separate modules) and statements of the entry
  point function body, if any, complete normally.
- An unhandled error (see :ref:`Error Handling`) occurs during the program
  execution.

In both cases, the control is transferred to the |LANG| runtime system, which
ensures that all coroutines (see :ref:`Coroutines`) created during the program
execution are terminated.

If an error occurs, then the appropriate diagnostics is displayed. This is the
end of the *program exit* process.

.. index::
   exit
   top-level statement
   top-level function
   function body
   normal completion
   control
   module
   runtime system
   coroutine

.. raw:: pdf

   PageBreak
