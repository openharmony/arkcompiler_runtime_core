..
    Copyright (c) 2026 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _Build System:

Build System
############

.. meta:
    frontend_status: Partly


The Build System of the |LANG| language defines the following:

- :ref:`Package Definition` - introduces the concept of a package as a
  distributable unit composed of one or more modules.
- :ref:`Module Visibility` - determines whether a module is accessible from
  other packages based on the presence of the ``export`` modifier in the module
  header (e.g.: ``export module "name"``). Modules without ``export`` modifier
  are internal to the package.
- :ref:`Runtime Name Formation` - defines the fully qualified name (see
  :ref:`RT Fully Qualified Name`) for each entity. This name is used for
  runtime identification and access control.
- :ref:`ImportPath Resolution Rules` - defines how an *importPath* (see
  :ref:`Import Path`) is resolved to an actual module.

.. index::
    build system

|

.. _Package Definition:

Package Definition
******************

.. meta:
    frontend_status: Done

*Package* is defined as an entity that groups
together one or more modules.
*Package* is a unit of distribution. Each module (see
:ref:`Module Declarations`) belongs to exactly one package.

A package is described by a configuration file that provides the build
system with all information necessary to compile, link, and distribute
the package. The file must specify at least the following:

- *Package name* - a string that uniquely identifies the package in the build.
  This name is used as a prefix when forming runtime names (see :ref:`Runtime
  Name Formation`).
- *Set of source files* - the list of |LANG| files that constitute the package.
  The build system compiles these files and combines them into the binary file
  (see :ref:`RT Binary File Format`).
- *Dependencies* - a list of other packages that this package depends on. The
  build system uses this information to locate those packages during
  compilation and to ensure that their exported declarations are available for
  *importPath* (see :ref:`Import Path`).
- *ImportPath resolution rules* - rules that affect how *importPath*
  (see :ref:`Import Path`) is resolved to modules within the package
  or in external packages. The rules are applied by the build system
  during compilation to locate imported modules.

.. index::
    package

|

.. _Module Visibility:

Module Visibility
*****************

.. meta:
    frontend_status: Done

*Module visibility* determines whether a module can be accessed from modules
residing in other packages. The visibility of a module is determined by the
``export`` modifier in the module header (see :ref:`Module Header`). If the
module does not have a module header (see :ref:`Module Header`), the visibility
of the module is determined in the configuration file.

The module can be *exported* or *internal*:

- A module declared with the ``export`` modifier is *exported*. Such a module
  is accessible from modules of other packages.
- A module declared without the ``export`` modifier is *internal*. It is
  accessible only from other modules belonging to the same package.

If an attempt is made to import an *internal* module from
another package, a then :index:`compile-time error` occurs.

An *exported* module is represented in the example below:

.. code-block:: typescript
   :linenos:

    export module "x";
    // module contents

An *internal* module is represented in the example below:

.. code-block:: typescript
   :linenos:

    module "y";
    // module contents

The build system enforces that the public API of a package is any declaration
exported from an exported module that does not refer to entities that are not
exported. Specifically, if the signature of an exported function, method,
class, or interface includes a type that is not exported (e.g., a type defined
in an internal module), then a :index:`compile-time error` occurs. This rule is
consistent with the general requirement for exported declarations as discussed
in detail in :ref:`Exported Declarations`. The build system verifies this
property at compile time, ensuring that the package's public interface remains
self-contained, and that runtime visibility checks (see :ref:`RT Verification`)
never fail due to such leaks.

*Internal* type leaks protection from a package is
represented in the example below:

.. code-block:: typescript
   :linenos:

    // file1.ets
    module "y"          // Internal module "y" (not exported)
    export class InternalClass {}

    // file2.ets
    export module "x"   // Exported module "x"
    import { InternalClass } from "./file1";       // OK, import is allowed

    export function foo(x: InternalClass): void {} // compile-time error, exported function signature contains reference to an unexported type

.. index::
    internal module

|

.. _Runtime Name Formation:

Runtime Name Formation
**********************

.. meta:
    frontend_status: Done

The build system is responsible for constructing the runtime name (see
:ref:`RT Runtime Name`) of each entity.

The rules below define how a runtime name (see :ref:`RT Runtime Name`) is formed
from the package name, the module name (see :ref:`Module Header`),
and the qualified name of an entity.

Let:

- ``P`` - the name of the package containing the entity (see
  :ref:`Package Definition`).
- ``M`` - the module name declared in the module header (see
  :ref:`Module Header`).
- ``Q`` - the qualified name of the entity within its module, consisting of
  dot-separated identifiers (see :ref:`Names`) with the exception of functions,
  variables, methods and static methods.
- ``F`` - the relative path to the file in the package (see
  :ref:`Package Definition`).
- ``R`` - the runtime name (see :ref:`RT Runtime Name`) of the entity.

.. code-block:: text
   :linenos:

    If M is not set:
        M = F

    General algorithm:
        R = P + ":" + M + "." + Q

    If M is empty string:
        R = P + ":" + Q

    If M is empty string and Q is empty:
        R = P

Forming a runtime name is represented in the example below:

.. code-block:: typescript
   :linenos:

    // packageName: pkg1

    // file1.ets
    export module ""            // Runtime Name: "pkg1"

    export class A {}           // Runtime Name: "pkg1:A"
    export namespace ns1 {      // Runtime Name: "pkg1:ns1"
        export namespace ns2 {  // Runtime Name: "pkg1:ns1.ns2"
            exprot class B {}   // Runtime Name: "pkg1:ns1.ns2.B"
        }
        export class C {}       // Runtime Name: "pkg1:ns1.C"
    }

    // file2.ets
    export module "x"           // Runtime Name: "pkg1:x"

    export class A {}           // Runtime Name: "pkg1:x.A"
    export namespace ns1 {      // Runtime Name: "pkg1:x.ns1"
        export namespace ns2 {  // Runtime Name: "pkg1:x.ns1.ns2"
            exprot class B {}   // Runtime Name: "pkg1:x.ns1.ns2.B"
        }
        export class C {}       // Runtime Name: "pkg1:x.ns1.C"
    }

    // file3.ets
    export class A {}           // Runtime Name: "pkg1:file3.A"
    export namespace ns1 {      // Runtime Name: "pkg1:file3.ns1"
        export namespace ns2 {  // Runtime Name: "pkg1:file3.ns1.ns2"
            exprot class B {}   // Runtime Name: "pkg1:file3.ns1.ns2.B"
        }
        export class C {}       // Runtime Name: "pkg1:file3.ns1.C"
    }

|

.. _ImportPath Resolution Rules:

ImportPath Resolution Rules
***************************

.. meta:
    frontend_status: Done

The build system resolves importPath (see :ref:`Import Path`) according to the
following rules, which depend on whether the target module belongs to the same
package or to a different package.

|

.. _Imports Within The Same Package:

Imports Within the Same Package
===============================

A module can import another module that resides
in the same package by using one of the following:

- *A file-relative path* as defined :ref:`Import Path`. The path is resolved
  relative to the location of the importing file.
- *A module name* prefixed with ``//``. Such a name denotes the *module name*
  declared in the target module header (see :ref:`Module Header`).
  For example, if a module has the header module ``"ui"``, then it can be
  imported from another module in the same package as ``import { ... } from
  "//ui"``. The build system resolves this name by locating the module with
  the matching identifier within the package. If no module with the given
  identifier exists, then a :index:`compile-time error` occurs.

|

.. _Imports From Other Packages:

Imports From Other Packages
===========================

A module can import a module from a different package
only by using the full module name:

.. code-block:: text
   :linenos:

    FullModuleName: "PackageName/ModuleName"

.. Import path resolution

where:

- ``PackageName`` is the name of the target package as declared in its
  configuration file (see :ref:`Package Definition`).
- ``ModuleName`` is the module identifier of the target module (see
  :ref:`Module Header`). If the target module is an empty string, then the
  ``ModuleName`` part is omitted, and the *importPath* consists solely of the
  ``PackageName``.

The build system resolves such an import by consulting the dependency
information in the configuration file of the importing package (see
:ref:`Package Definition`). It locates the target package among the declared
dependencies and then finds a module with the given identifier (or an unnamed
module if the module name is an empty string) within that package. If the
target package is not listed as a dependency, or if the specified module does
not exist in that package, then a :index:`compile-time error` occurs.
