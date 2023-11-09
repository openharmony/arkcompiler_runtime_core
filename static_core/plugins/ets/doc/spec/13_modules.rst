..
    Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

Compilation Units, Packages and Modules
#######################################

.. meta:
    frontend_status: Done

Programs are structured as sequences of elements ready for compilation, i.e.,
compilation units. Each compilation unit creates its own scope (see
:ref:`Scopes`), and the compilation unit’s variables, functions, classes,
interfaces, or other declarations are only accessible within such scope unless
explicitly exported.

A variable, function, class, interface, or other declarations exported from a
different compilation unit must be imported first.

Compilation units are **separate modules** or **packages**. Packages are
described in the experimental section (see :ref:`Packages`).

.. code-block:: abnf

    compilationUnit:
        separateModuleDeclaration
        | packageDeclaration
        ;

    packageDeclaration:
        packageModule+
        ;

All modules are stored in a file system or a database (see
:ref:`Compilation Units in Host System`).

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

A *separate module* is a module that has no package header, and consists of
four optional parts:

#. Import directives that enable referring imported declarations in a module;

#. Top-level declarations; 

#. Top-level statements; and

#. Re-export directives.


.. code-block:: abnf

    separateModuleDeclaration:
        importDirective* (topDeclaration | topLevelStatements | exportDirective)*
        ;

Every module automatically imports all exported entities from essential kernel
packages (‘std.core’ and 'escompat') of the standard library (see :ref:`Standard Library`).

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

.. _Compilation Units in Host System:

Compilation Units in Host System
**********************************

.. meta:
    frontend_status: Partly

The manner modules and packages are created and stored is determined by a
host system.

The exact way modules and packages are stored in a file system is
determined by a particular implementation of the compiler and other
tools.

In a simple implementation:

-  A module (package module) is stored in a single file.

-  Files corresponding to a package module are stored in a single folder.

-  A folder can store several separate modules (one source file to contain a
   separate module or a package module).

-  A folder that stores a single package must contain neither separate module
   files nor package modules from other packages.

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

Import directives import entities exported from other compilation units, and
provide such entities with bindings in the current module.

Two parts of an import declaration are as follows:

#. Import path that determines a compilation unit to import from;

#. Import binding that defines what entities, and in what form---qualified
   or unqualified---can be used by the imported compilation unit.

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
        '*' importAlias?
        | qualifiedName '.' '*'
        ;

    selectiveBindigns:
        '{' importBinding ('.' importBinding)* '}'
        ;

    defaultBinding:
        'default' Identifier
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
Any added declaration must be distinguishable in the declaration scope (see
:ref:`Distinguishable Declarations`). Otherwise, a :index:`compile-time error`
occurs.

Some import constructions are specific for packages, and are described in the
Experimental section (see :ref:`Packages`).

.. index::
   binding
   declaration
   module
   package
   declaration scope
   import construction

|

.. _Bind All with Unqualified Access:

Bind All with Unqualified Access
================================

.. meta:
    frontend_status: Partly

Unless an alias is set, the import binding '\*' binds all entities exported
from the compilation unit defined by the *import path* to the declaration scope
of the current module as simple names.

+-------------------------------+--+-------------------------------+
| Import                        |  |  Usage                        |
+===============================+==+===============================+
|                                                                  |
+-------------------------------+--+-------------------------------+
| .. code-block:: typescript    |  | .. code-block:: typescript    |
|                               |  |                               |
|     import * from "..."       |  |     let x = sin(1.0)          |
+-------------------------------+--+-------------------------------+

.. index::
   import binding
   alias
   entity
   export
   compilation unit
   import path
   declaration scope
   simple name
   module

|

.. _Bind All with Qualified Access:

Bind All with Qualified Access
==============================

.. meta:
    frontend_status: Done

The import binding '\* as A' binds the single named entity 'A' to the
declaration scope of the current module.

A qualified name that consists of 'A' and the name of entity '*A.name*'
are used to access any entity exported from the compilation unit defined
by the *import path*.

+---------------------------------+--+-------------------------------+
| Import                          |  |  Usage                        |
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

There are two cases of the import binding '*qualifiedName*' as follows:

-  A simple name (like *foo*); or

-  A name containing several identifiers (like *A.foo*).


The import binding '*ident*' binds an exported entity with the name '*ident*'
to the declaration scope of the current module. The name '*ident*' can only
correspond to several entities, where '*ident*' denotes several overloaded
functions (see :ref:`Function and Method Overloading`).

The import binding '*ident* as A' binds an exported entity (entities) with the
name '*A*' to the declaration scope of the current module.

The bound entity is not accessible as '*ident*' because this binding does not
bind '*ident*'.

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
| Import                          |  |  Usage                               |
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

One import statement can list several names:

+-------------------------------------+--+---------------------------------+
| Import                              |  | Usage                           |
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
| Case                        | Sample                     | Rule                         |
+=============================+============================+==============================+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | Ok. The compile-time         |
| without an alias in several |      import {sin, sin}     | warning is recommended.      |
| bindings.                   |         from "..."         |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is used explicitly   |                            | Ok. No warning.              |
| without alias in one binding|     import {sin}           |                              |
| and implicitly without an   |        from "..."          |                              |
| alias in another binding.   |                            |                              |
|                             |     import * from "..."    |                              |
|                             |                            |                              |
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
| with alias and implicitly   |                            | for the name, but not the    |
| without alias.              |     import {sin as Sine}   | original one:                |
|                             |       from "..."           |                              |
|                             |                            | - Sine is accessible;        |
|                             |     import * from "..."    |                              |
|                             |                            | - sin is not accessible.     |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly        |                            | Ok. Both variants            |
| used with alias and         |                            |   can be used:               |
| implicitly with alias.      |     import {sin as Sine}   |                              |
|                             |        from "..."          | - Sine is accessible;        |
|                             |                            |                              |
|                             |     import * as M          | - M.sin is accessible.       |
|                             |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | Compile-time error.          |
| with alias several times.   |                            | Or warning?                  |
|                             |     import                 |                              |
|                             |        {sin as Sine},      |                              |
|                             |        sin as SIN          |                              |
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
    frontend_status: Partly

Default import binding allows to import a declaration exported from some
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

Type import binding allows importing only the type declarations exported from
some module or package. These declarations can be exported normally, or by
using the *export type* form. The difference between *import* and 
*import type* is that the the first form imports all top-level declarations
which were exported while the second one imports only exported types.

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
    frontend_status: Partly

Import path is a string literal---represented as a combination of the
slash character '/' and a sequence alpha-numeric characters---that determines
how an imported compilation unit must be placed.

The slash character '/' is used in import paths irrespective of the host system.
The backslash character is not used in this context.

In most file systems, an import path looks like a file path; *relative* (see
below) and *non-relative* import paths have different *resolutions* that map
the import path to a file path of the host system.

The compiler uses the following rule to define the kind of imported
compilation units, and the exact placement of the source code:

-  If import path refers to a folder denoted by the last name in the resolved
   file path, then the compiler imports the package which resides in that
   folder. The source code of the package is all the |LANG| source files in
   that folder.

-  Otherwise, the compiler imports the module that the import path refers to.
   The source code of the module is the file with the extension provided
   within the import path, or——if none is so provided——appended by the compiler.


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

A **relative import path** starts with './' or '../' as in the following
examples:

.. code-block:: typescript
   :linenos:

    "./components/entry"
    "../constants/http"

Resolving a *relative import* is relative to the importing file. *Relative
import* is used for compilation units to maintain their relative location.

.. code-block:: typescript
   :linenos:

    import * as Utils from "./mytreeutils"

Other import paths are **non-relative** as in the examples below:

.. code-block:: typescript
   :linenos:

    "/net/http"
    "std/components/treemap"

Resolving a *non-relative path* depends on the compilation environment. The
definition of the compiler environment can be particularly provided in a
configuration file or environment variables.

The *base URL* setting is used to resolve a path that starts with '/';
*path mapping* is used in all other cases. Resolution details depend on
the implementation.

For example the compilation configuration file may contain the following lines:
(file name, placement, and format are implementation-specific)

.. code-block:: typescript
   :linenos:

    "baseUrl": "/home/project",
    "paths": {
        "std": "/sts/stdlib"
    }

In the example above, '/net/http' is resolved to '/home/project/net/http',
and 'std/components/treemap' to '/sts/stdlib/components/treemap'.

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

.. _Default Import:

Default Import
**************

.. meta:
    frontend_status: Done
    todo: now core, containers, math and time are also imported because of stdlib internal dependencies
    todo: fix stdlib and tests, then import only core by default
    todo: add escompat to spec and default

A compilation unit automatically imports all entities exported from the
predefined package ‘std.core’. All entities from this package can be accessed
as simple names.

.. code-block:: typescript
   :linenos:

    function main() {

      let myException = new Exception { ... }
        // class Exception is defined in the 'std.core' package

      console.log("Hello")
        // 'console' variable is defined in the 'std.core' package

    }

.. index::
   compilation unit
   import
   exported entity
   package
   access
   simple name

|

.. _Dynamic Import:

Dynamic Import
**************

TBD


|

.. _Top-level Declarations:

Top-level Declarations
**********************

.. meta:
    frontend_status: Done

*Top-level type declarations* declare top-level types (*class*,
*interface*, or *enum*), top-level variables, constants, or
functions, and can be exported.

.. code-block:: abnf

    topDeclaration:
        ('export' 'default'?)?
        ( typeDeclaration
        | typeAlias
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

Top-level declarations can use export modifiers to 'export' declared names.
A declared name is considered *private* if not exported; a name declared
*private* can be used only inside the compilation unit it is declared in.

.. code-block:: typescript
   :linenos:

    export class Point {}
    export let Origin = new Point(0, 0)
    export function Distance(p1: Point, p2: Point): number {
      // ...
    }

In addition, only one top-level declaration can be exported by using the default
export scheme. It allows to not specify the declared name at importing (see
:ref:`Default Import Binding` for details). A :index:`compile-time error`
occurs if more than one top-level declaration is marked as *default*.

.. code-block:: typescript
   :linenos:

    export default let PI = 3.141592653589

.. index::
   exported declaration
   top-level declaration
   export modifier
   export
   declared name
   private
   compilation unit
   default export scheme
   import

|

.. _Export Directives:

Export Directives
*****************

.. meta:
    frontend_status: None
    todo: Now all symbols are exported (not only one with export declaration) because of stdlib internal dependencies
    todo: Fix stdlib and test, then restrict exporting everything

The *export directive* allows specifying a selective list of exported
declarations with optional renaming, or re-exporting declarations from
other compilation units.

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

In addition, each exported declaration can be marked as *exported*
by explicitly listing the names of exported declarations, with
optional renaming.

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

In addition to export which is attached to some declaration a programmer may
wish to export as a type particular class or interface which was already
declared or export already declared type under a different name. That can be
done with help of *export type* directive. Its syntax is presented below.

.. code-block:: abnf

    exportTypeDirective:
        'export' 'type' selectiveBindigns
        ;

If class or interface was exported with this scheme then its usage is limited
similar to limitations described in *import type* directive section (see `:ref:Type Binding`).

It is a compile-time error if a class or interface was already declared as
exported and then later *export type* is applied for the same class or
interafce name.

Example below shows how this can be used

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

In addition to exporting what is declared in the module, it is possible to
re-export declarations that are part of export of the other modules. Only
limited re-export possibilities are currently supported.

It is possible to re-export particular declarations, or all declarations
from a module. When re-exporting, new names can be given, which is similar
to importing, but the direction of such action is opposite.

The appropriate grammar is presented below:

.. code-block:: abnf

    reExportDirective:
        'export' ('*' | selectiveBindigns) 'from' importPath
        ;


The examples below illustrate how re-export works in practice:

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

|

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

The above sequence is equal to the following:

.. code-block:: typescript
   :linenos:

      /* top-declarations */
      statements_1; statements_2

.. index::
   separate module
   statement
   top-level statement
   sequence

All top-level statements are executed only once before the call to any other
function, or access to any top-level variable of the separate module.
This also works when a function of the module is used as *program entry
point*.

.. code-block:: typescript
   :linenos:

      // Source file A
      {
        console.log ("A.top-level statements")
      }

      // Source file B
      import * from "Source file A "
      function main () {
         console.log ("B.main")
      }

The output will be:

A. Top-level statements;
B. Main.

A :index:`compile-time error` occurs if a top-level statement is a return
statement (:ref:`Expression Statements`).

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

.. _Program Entry Point main:

Program Entry Point (`main`)
****************************

.. meta:
    frontend_status: Done

A program (application) entry point is the top-level ``main`` function. The
function must have either no parameters, or one parameter of the string ``[]``
type. Its return type is either ``void``, or ``int``. No overloading is allowed
for the entry point function.

The example below illustrates different forms of valid and invalid entry points:

.. code-block:: typescript
   :linenos:

    function main() {
      // Option 1: skip the return type - identical to :void
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

