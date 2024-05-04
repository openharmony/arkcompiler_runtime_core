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

.. _Modules and Compilation Units:

Compilation Units, Packages, and Modules
########################################

.. meta:
    frontend_status: Done

Programs are structured as sequences of elements ready for compilation, i.e.,
compilation units. Each compilation unit creates its own scope (see
:ref:`Scopes`). The compilation unit’s variables, functions, classes,
interfaces, or other declarations are only accessible within such scope if
not explicitly exported.

A variable, function, class, interface, or other declarations exported from a
different compilation unit must be imported first.

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
   scope
   variable
   function
   class
   interface
   declaration
   access
   import
   separate module
   package
   file system
   database
   host system
   
|

.. _Separate Modules:

Separate Modules
****************

.. meta:
    frontend_status: Done

A *separate module* is a module without a package header. A *separate module*
can optionally consist of the following four parts:

#. Import directives that enable referring imported declarations in a module;

#. Top-level declarations;

#. Top-level statements; and

#. Re-export directives.


.. code-block:: abnf

    separateModuleDeclaration:
        importDirective* (topDeclaration | topLevelStatements | exportDirective)*
        ;

Every module implicitly imports (see :ref:`Implicit Import`) all exported
entities from essential kernel packages of the standard library (see
:ref:`Standard Library`).

All entities from these packages are accessible as simple names, like the
*console* variable:

.. code-block:: typescript
   :linenos:

    // Hello, world! module
    function main() {
      console.log("Hello, world!")
    }

.. index::
   separate module
   package header
   import directive
   imported declaration
   module
   top-level declaration
   top-level statement
   re-export directive
   import
   predefined package
   standard library
   entity
   access
   simple name
   console variable

|

.. _Separate Module Initializer:

Separate Module Initializer
***************************

.. meta:
    frontend_status: Partly

If used for import, a *separate module* is initialized only once with the
details listed in :ref:`Compilation Unit Initialization`. The initialization process
is performed in the following steps:

- If the separate module has variable or constant declaraions (see
  :ref:`Variable and Constant Declarations`), then their initializers are
  executed to ensure that they all have valid initial values;
- If the separate module has top-level statements (see :ref:`Top-Level Statements`),
  then they are also executed.

.. index::
   initializer
   separate module
   initialization
   variable declration
   constant declaration

|

.. _Compilation Units in Host System:

Compilation Units in Host System
**********************************

.. meta:
    frontend_status: Partly
    todo: Implement compiling a package module as a single compilation unit - #16267

Modules and packages are created and stored in a manner that is determined by a
host system. The exact manner modules and packages are stored in a file
system is determined by a particular implementation of the compiler and other
tools.

In a simple implementation:

-  A module (package module) is stored in a single file.

-  Files that correspond to a package module are stored in a single folder.

-  A folder can store several separate modules (one source file to contain a
   separate module or a package module).

-  A folder that stores a single package must not contain separate module
   files or package modules from other packages.

.. index::
   compilation unit
   host system
   module
   package
   file system
   implementation
   package module
   file
   folder
   source file
   separate module

|

.. _Import Directives:

Import Directives
*****************

.. meta:
    frontend_status: Partly
    todo: implement type binding - #14586

Import directives make entities exported from other compilation units (see
also :ref:`Declaration Modules`) avaialble for use in the current compilation
unit by using different binding forms.

An import declaration has the following two parts:

-  Import path that determines a compilation unit to import from;

-  Import binding that defines what entities, and in what form---qualified
   or unqualified---can be used by the current compilation unit.

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
        'import' fileBinding|selectiveBindigns|defaultBinding|typeBinding
        'from' importPath
        ;

    fileBinding:
        '*' importAlias
        | qualifiedName '.' '*'
        ;

    selectiveBindigns:
        '{' importBinding (',' importBinding)* '}'
        ;

    defaultBinding:
        Identifier
        ;

    typeBinding:
        'type' selectiveBindigns
        ;

    importBinding:
        qualifiedName importAlias?
        ;

    importAlias:
        'as' Identifier
        ;

    importPath:
        StringLiteral
        ;

Each binding adds a declaration or declarations to the scope of a module
or a package (see :ref:`Scopes`).
Any declaration added so must be distinguishable in the declaration scope (see
:ref:`Distinguishable Declarations`). Otherwise, a :index:`compile-time error`
occurs.

It is noteworthy that import directives are handled by the compiler
during compilation and have no effect during program execution.

.. index::
   binding
   declaration
   module
   package
   declaration scope


.. _Bind All with Qualified Access:

Bind All with Qualified Access
==============================

.. meta:
    frontend_status: Done

The import binding ``* as A`` binds the single named entity *A* to the
declaration scope of the current module.

A qualified name, which consists of *A* and the name of entity ``A.name``,
is used to access any entity exported from the compilation unit as defined
by the *import path*.

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
understanding of the source code.

.. index::
   import binding
   qualified access
   named entity
   declaration scope
   module
   qualified name
   entity
   name
   access
   export
   compilation unit
   import path
   source code

|

.. _Simple Name Binding:

Simple Name Binding
===================

.. meta:
    frontend_status: Done

The import binding ``qualifiedName`` has two cases as follows:

-  A simple name (like ``foo``); or

-  A name containing several identifiers (like ``A.foo``).


The import binding ``ident`` binds an exported entity with the name ``ident``
to the declaration scope of the current module. The name ``ident`` can only
correspond to several entities, where ``ident`` denotes several overloaded
functions (see :ref:`Function and Method Overloading`).

The import binding ``ident as A`` binds an exported entity (entities) with the
name *A* to the declaration scope of the current module.

The bound entity is not accessible as ``ident`` because this binding does not
bind ``ident``.

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
   bound entity
   binding

This is shown in the following module:

.. code-block:: typescript
   :linenos:

    export const PI = 3.14
    export function sin(d: number): number {}

The module’s import path is now irrelevant:

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
|         ..."                    |  |     let y = sin(1.0) /* Error ‘y’ is |
|                                 |  |        not accessible */             |
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

The same bound entities can use several import bindings. The same bound
entities can use one import directive, or several import directives with
the same import path.

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

+-----------------------------+----------------------------+------------------------------+
| **Case**                    | **Sample**                 | **Rule**                     |
+=============================+============================+==============================+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | Ok. The compile-time         |
| without an alias in several |      import {sin, sin}     | warning is recommended.      |
| bindings.                   |         from "..."         |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is used explicitly   |                            | Ok. No warning.              |
| without alias in one        |     import {sin}           |                              |
| binding.                    |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | Ok. Both the name and        |
| without alias and implicitly|     import {sin}           | qualified name can be used:  |
| with alias.                 |        from "..."          |                              |
|                             |                            | sin and M.sin are            |
|                             |     import * as M          | accessible.                  |
|                             |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | Ok. Only alias is accessible |
| with alias.                 |                            | for the name, but not the    |
|                             |     import {sin as Sine}   | original one:                |
|                             |       from "..."           |                              |
|                             |                            | - Sine is accessible;        |
|                             |                            | - sin is not accessible.     |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly        |                            | Ok. Both variants can be     |
| used with alias and         |                            | used:                        |
| implicitly with alias.      |     import {sin as Sine}   |                              |
|                             |        from "..."          | - Sine is accessible;        |
|                             |                            |                              |
|                             |     import * as M          | - M.sin is accessible.       |
|                             |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | Compile-time error.          |
| with alias several times.   |                            | Or warning?                  |
|                             |     import {sin as Sine,   |                              |
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
module as default export. Knowing the actual name of the declaration is not
required as the new name is given at importing.
A :index:`compile-time error` occurs if another form of import is used to
import an entity initially exported as default.

.. code-block:: typescript
   :linenos:

    import DefaultExportedItemBindedName from ".../someFile"
    function foo () {
      let v = new DefaultExportedItemBindedName()
      // instance of class 'SomeClass' be created here
    }

    // SomeFile
    export default class SomeClass {}

.. index::
   import binding
   default import binding
   import
   declaration
   default export
   module

|

.. _Type Binding:

Type Binding
============

.. meta:
    frontend_status: None
    todo: implement type binding - #14586

Type import binding allows importing only the type declarations exported from
some module or package. These declarations can be exported normally, or by
using the *export type* form. The difference between *import* and
*import type* is that the first form imports all top-level declarations
which were exported, and the second imports only exported types.

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


|

.. _Import Path:

Import Path
===========

.. meta:
    frontend_status: Done

Import path is a string literal---represented as a combination of the
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
   folder. The source code of the package is all the |LANG| source files in
   the folder.

-  Otherwise, the compiler imports the module that the import path refers to.
   The source code of the module is the file with the extension provided
   within the import path, or---if none is so provided---appended by the
   compiler.


.. index::
   import binding
   import path
   string literal
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

Resolving a *relative import* is relative to the importing file. *Relative
import* is used on compilation units to maintain their relative location.

.. code-block:: typescript
   :linenos:

    import * as Utils from "./mytreeutils"

Other import paths are *non-relative* as in the examples below:

.. code-block:: typescript
   :linenos:

    "/net/http"
    "std/components/treemap"

Resolving a *non-relative path* depends on the compilation environment. The
definition of the compiler environment can be particularly provided in a
configuration file or environment variables.

The *base URL* setting is used to resolve a path that starts with '/'.
*Path mapping* is used in all other cases. Resolution details depend on
the implementation.

For example, the compilation configuration file can contain the following lines:

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
   imported file
   compilation unit
   relative location
   non-relative import path
   configuration file
   environment variable
   resolving
   base URL
   path mapping
   resolution
   implementation

|

.. _Implicit Import:

Implicit Import
***************

.. meta:
    frontend_status: Done
    todo: now core, containers, math and time are also imported because of stdlib internal dependencies
    todo: fix stdlib and tests, then import only core by default
    todo: add escompat to spec and default

Any compilation unit implicitly imports all entities exported from the
essential kernel packages of the standard library(see :ref:`Standard Library`).
All entities exported from these packages can be accessed as simple names.

.. code-block:: typescript
   :linenos:

    function main() {

      let myException = new Exception { ... }
        // class 'Exception' is defined in the standard library

      console.log("Hello")
        // variable 'console' is defined in the standard library too

    }

.. index::
   compilation unit
   import
   exported entity
   package
   access
   simple name

|

.. _Declaration Modules:

Declaration Modules
*******************

.. meta:
    frontend_status: None

A *declaration module* is a special kind of compilation units that can be
imported by using :ref:`Import Directives`. A declaration module contains
:ref:`Ambient Declarations` and :ref:`Type Alias Declaration` only.

Ambient declarations defined in the declaration module must be fully declared
elsewhere.

.. code-block:: abnf

    declarationModule:
        importDirective* 
        ( 'export'? ambientDeclaration
        | 'export'? typeAlias
        | selectiveExportDirective
        )*
        ;

The following example shows how ambient functions can be declared and exported:

.. code-block:: typescript
   :linenos:

    declare function foo()
    export declare function goo()
    export { foo }

The exact manner declaration modules are stored in the file system, and how
they differ from separate modules is determined by a particular implementation.

|

.. _Compilation Unit Initialization:

Compilation Unit Initialization
*******************************

.. meta:
    frontend_status: None

A compilation unit is a separate module or a package that is initialized (see
:ref:`Separate Module Initializer` and :ref:`Package Initializer`) once
before the first use of an entity (function, variable, or type) exported
from the compilation unit.
If a compilation unit has any import directive (see :ref:`Import Directives`)
but the imported entities are not actually used, then the imported compilation
unit (separate or package) is initialized before the entry point (see
:ref:`Program Entry Point`) code starts.
If different compilation units are not connected by import, then the order
of initialization of the compilation units is not determined.

.. index::
   binding
   declaration
   module
   package
   declaration scope
   import construction


|

.. _Top-Level Declarations:

Top-Level Declarations
**********************

.. meta:
    frontend_status: Done

*Top-level declarations* declare top-level types (``class``, ``interface``,
or ``enum``), top-level variables, constants, or functions. Top-level
declarations can be exported.

.. code-block:: abnf

    topDeclaration:
        ('export' 'default'?)?
        ( typeDeclaration
        | variableDeclarations
        | constantDeclarations
        | functionDeclaration
        | extensionFunctionDeclaration
        )
        ;

.. code-block:: typescript
   :linenos:

    export let x: number[], y: number

.. index::
   top-level type declaration
   top-level type
   class
   interface
   enum
   variable
   constant
   function
   export

|

.. _Exported Declarations:

Exported Declarations
=====================

.. meta:
    frontend_status: Done

Top-level declarations can use export modifiers that make the declarations
accessible in other compilation units by using import (see :ref:`Import Directives`).
The declarations not marked as exported can be used only inside the compilation
unit they are declared in.

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

.. index::
   exported declaration
   top-level declaration
   export modifier
   export
   declared name
   compilation unit
   default export scheme
   import

|

.. _Export Directives:

Export Directives
*****************

.. meta:
    frontend_status: Partly
    todo: implement type binding - #14586

The *export directive* allows the following:

-  Specifying a selective list of exported declarations with optional
   renaming; or
-  Re-exporting declarations from other compilation units.


.. code-block:: abnf

    exportDirective:
        selectiveExportDirective | reExportDirective | exportTypeDirective
        ;

.. index::
   export directive
   exported declaration
   renaming
   re-export
   compilation unit

|

.. _Selective Export Directive:

Selective Export Directive
==========================

.. meta:
    frontend_status: Partly
    todo: export with alias isn't work properly

In addition, each exported declaration can be marked as *exported* by
explicitly listing the names of exported declarations. Renaming is optional.

.. code-block:: abnf

    selectiveExportDirective:
        'export' selectiveBindigns
        ;

An export list directive uses the same syntax as an import directive with
*selective bindings*:

.. code-block:: typescript
   :linenos:

    export { d1, d2 as d3}

The above directive exports 'd1' by its name, and 'd2' as 'd3'. The name 'd2'
is not accessible in the modules that import this module.

.. index::
   selective export directive
   exported declaration
   renaming
   export list directive
   import directive
   selective binding
   module
   access

|

.. _Export Type Directive:

Export Type Directive
=====================

.. meta:
    frontend_status: Partly
    todo: implement type binding - #14586

In addition to export that is attached to some declaration, a programmer can
use the *export type* directive in order to do the following:

-  Export as a type a particular class or interface already declared; or
-  Export an already declared type under a different name.

The appropriate syntax is presented below:

.. code-block:: abnf

    exportTypeDirective:
        'export' 'type' selectiveBindigns
        ;

If a class or an interface is exported in this manner, then its usage is
limited similarly to the limitations described for *import type* directives
(see :ref:`Type Binding`).

A compile-time error occurs if a class or interface is declared exported,
but then *export type* is applied to the same class or interface name.

The following example is an illustration of how this can be used:

.. code-block:: typescript
   :linenos:

    class A {}

    export type {A}  // export already declared class type

    export type MyA = A // name MyA is declared and exported

    export type {MyA} // compile-time error as MyA was already exported

|

.. _Re-Export Directive:

Re-Export Directive
===================

.. meta:
    frontend_status: Partly
    todo: export with alias isn't work properly

In addition to exporting what is declared in the module, it is possible to
re-export declarations that are part of other modules' export. Only
limited re-export possibilities are currently supported.

It is possible to re-export a particular declaration or all declarations
from a module. When re-exporting, new names can be given. This action is
similar to importing but with the opposite direction.

The appropriate grammar is presented below:

.. code-block:: abnf

    reExportDirective:
        'export' ('*' | selectiveBindigns) 'from' importPath
        ;

The following examples illustrate the re-exporting in practice:

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
        statements
        ;

A module can contain any number of top-level statements that logically
merge into a single sequence in the textual order:

.. code-block:: typescript
   :linenos:

      statements_1
      /* top-declarations */
      statements_2

The sequence above is equal to the following:

.. code-block:: typescript
   :linenos:

      /* top-declarations */
      statements_1; statements_2

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
- If a separate module is used a program, then top-level statements are used as
  a program entry point (see :ref:`Program Entry Point`). If the separate
  module has the ``main`` function, then it is executed after the execution of
  the top-level statements.

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

A :index:`compile-time error` occurs if a top-level statement contains a
return statement (:ref:`Expression Statements`).

.. index::
   top-level statement
   execution
   function
   access
   top-level variable
   separate module
   statement
   output
   return statement

|

.. _Program Entry Point:

Program Entry Point
*******************

.. meta:
    frontend_status: Partly

Separate modules can act as programs (applications). The two kinds of program
(application) entry points are as follows:

- Top-level statements (see :ref:`Top-Level Statements`);
- Top-level ``main`` function (see below).

Thus, a separate module may have:

- Only a top-level ``main`` function (that is the entry point);
- Only top-level statements (that are entry points);
- Both top-level ``main`` function and statements (same as above, plus ``main``
  is called after the top-level statement execution is completed).

The top-level ``main`` function must have either no parameters, or one
parameter of string  type ``[]`` that provides access to the arguments of
program command-line. Its return type is either ``void`` or ``int``.
No overloading is allowed for an entry point function.

Different forms of valid and invalid entry points are shown in the example
below:

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    function main() {
      // Option 1: a return type is inferred, must be void or int
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


.. index::
   top-level function
   top-level main function
   program entry point
   application entry point
   parameter
   string
   return type
   void
   int
   overloading
   entry point function
   entry point

.. raw:: pdf

   PageBreak

