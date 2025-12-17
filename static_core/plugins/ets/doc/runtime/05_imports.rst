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

.. _Runtime Initialization:

Runtime Initialization
**********************

This section discusses the rules of runtime initialization.

.. _Initialization Granularity And Execution Order:

Initialization Granularity And Execution Order
==============================================

- Each module, namespace, and nested namespace has its own top-level scope.
  According to the |LANG| language specification, same-name namespaces
  within a single top-level scope are merged.
- Each top-level scope has a runtime entity with an initializer. An initializer
  executes all top-level statements and variable declarations one after another.
- Each class has its own initializer. An initializer executes all static
  property initializers and code inside a ``static`` block one after another.

Top-level scope and class initializers are invoked at the first access to a
representing runtime entity, in particular:

- Access to an exported variable or a function of a top-level scope triggers
  an initializer;
- Access to a nested namespace or class does **not** trigger an initializer.

Initializers are executed **only once**, and are concurrency-safe. In
particular, each initializer is guarded with a lock, and all concurrent entities
wait until an initialization completes.

|

.. _Initialization Laziness:

Initialization Laziness
=======================

The *laziness* of |LANG| 2.0 source code imported by |LANG| 1.0 is regulated
with the |LANG| 1.0 import code itself because |LANG| 2.0 simply builds a
module object when the module object is requested for the first time.

The existing mainline already limits the usage of side effects in top-level
scopes. Binding the initialization of nested scopes to a trigger initialization
of import can be considered optionally to debug an application.

|

.. _Comprehensive Initializer Example:

Comprehensive Initializer Example
=================================

If source code in ``appsrc/mod.ets`` is as follows:

.. code-block:: typescript
    :linenos:

    export function foo() {
        return new F()
    }
    export let x001 = foo()
    export let x002 = foo()

    export namespace NS {
        let x1 = foo()
    }
    export namespace NS {
        class C {
            static x2 = foo()
        }
    }

    export class C {
        static x3 = foo()
    }

--then the initialization is performed as represented below:

.. code-block:: typescript
    :linenos:

    import {A, foo} from "module1" // no side effects

    foo()  // call to function "foo" from "module1" top-level triggers "module1.GLOBAL"
           // at this point "module1.A" is not affected
    A.prop // access to static property of A triggers "module1.A"

    // call to this function triggers the surrounding top-level scope initializer
    export function bar() {}

    // instantiation "new A()" DOES NOT trigger
    // the surrounding top-level scope initializer
    export class A { }

|

.. _Handling Binaries and Loading Classes:

Handling Binaries and Loading Classes
=====================================

The build system can mandate some forms of distribution and rules of packaging,
but the |LANG| runtime subsystem relies only on binary executable files:

- Internal runtime loading APIs work with binary executable files **only**.
  They neither change nor patch classes/modules or names on load.
- Binary executable files for |LANG| 2.0 have no common ``ohmurl`` concept.
  Each class is independent, and the name of each class is fully qualified.

The following rules apply:

- All entities (functions, records) inside an executable binary file have fully
  qualified names that are not modified by runtime onload events, i.e., an
  executable binary file is *flat* in terms of runtime names.

- Executable binary file by itself has no associated name or entry point,
  and is not associated with any source file.

- Executable binary file can be moved within a file system, and runtime is not
  sensitive to the actual path to the executable binary file in a file system.

- Several executable binary files ``.abc`` can be merged without affecting the
  load procedure.

- Executable binary files are processed **only** within a fixed *load context*.

The |LANG| runtime uses a *custom class loading policy* to determine a load
context, i.e., whether an executable binary file is to be located within a file
system, or extracted and stored in a context.

The |LANG| runtime defines at least two built-in load contexts:

- ``boot``: the core language and common platform libraries set up in *AppSpawn*.

- ``app``: the context of application files.

All files for these contexts must be provided explicitly in *AppSpawn* and at
app startup. No file system resolution for executable binary files exists on
the core runtime side.

.. code-block:: typescript
    :linenos:

    // standard library
    abstract class RuntimeLinker {
        final native findLoadedClass(clsName: string): Class | null
        loadClass(name: string, init: boolean = false): Class
        abstract findAndLoadClass(name: string, init: boolean): Class | null
    }

    function getBootRuntimeLinker(): BootRuntimeLinker

    final class BootRuntimeLinker extends RuntimeLinker {
        private constructor()
        native override findAndLoadClass(clsName: string, init: boolean): Class | null

        static instance: BootRuntimeLinker
    }

    // runtime-specific API:
    export final class ABCFile {
        private constructor()
        static native loadAbcFile(runtimeLinker: RuntimeLinker, path: string): AbcFile
        native loadClass(runtimeLinker: RuntimeLinker, clsName: string, init: boolean): Class | null
        native getFilename(): string
    }

    // platform API
    class AbcRuntimeLinker extends RuntimeLinker {
        constructor(parentLinker: RuntimeLinker | null, paths: string[])
        addAbcFiles(paths: string[]): void
        override findAndLoadClass(clsName: string, init: boolean): Class | null

        parentLinker: RuntimeLinker
        abcFiles: ABCFile[] // formed from the `paths` list and define the load order
    }

    // platform API
    export class MemoryRuntimeLinker extends AbcRuntimeLinker {
        constructor(parentLinker: RuntimeLinker | undefined, rawAbcFiles: Array<FixedArray<byte>>)
    }

.. code-block:: typescript
    :linenos:

    /*
     * appspawn code, pre-fork, created in runtime from bootload files
     * specified by appspawn: ["stdlib.abc", "ohosapi.abc", "arkui.abc"]
     */
    let boot = getBootRuntimeLinker()

    /*
     * appspawn code, post-fork, create a new 'app' context RuntimeLinker
     * specifying all the files that are in initial ability HAR
     */
    let app = new AbcRuntimeLinker(boot, ["app.abc"])

    /* load a feature dynamically */
    app.addAbcFiles(["feature.abc"])

    /* load an isolated resource */
    let isolated = new AbcRuntimeLinker(boot, ["dynamics.abc"])

    /* native function to read binary files */
    native function loadRawFiles() : Array<FixedArray<byte>>

    /* load abc file from binary */
    let memLinker = new MemoryRuntimeLinker(boot, loadRawFiles());

Runtime resolution request has two inputs:

- Runtime class descriptor computed from the *runtime name* used by language
  APIs: reflection, class loading, and dynamic import;
- Load context.

|

.. _Load Contexts:

Load Contexts
=============

Each class is associated with the context it is loaded into. A context can list
classes, and a class has a link back to the context it is loaded into.

All references from bytecode to classes and methods are lazily resolved by
the following rules:

#. Obtain a class that owns the bytecode.

#. Obtain a context of the class, and perform fast lookup to check if the class
   is already available in the context.

#. Obtain an associated loader, and perform the resolution specified by the
   custom loader code.

#. Loader can delegate the request to its *parent* loader.

#. Loader can be backed by a set of executable binary files (such a loader is
   backed by the runtime implementation).

The core runtime relies on no particular distribution format unless it is
instructed explicitly what binaries to load.

|

.. _Application Load Contexts:

Application Load Contexts
=========================

Load contexts for executable binary files form a tree structure.
If a class cannot be loaded, then a request is delegated to a parent linker
during resolution. On the opposite, a parent linker cannot delegate references
existing in a parent class to a child context to resolve.

**FIXME: picture**

This approach isolates some load contexts and allows loading different and even
clashing versions of the same module within in a single running application.
A natural way to load independent or potentially conflicting parts of an
application is by using independent class loaders.

**FIXME: splitload**

**FIXME: repackaging**

|

.. _Runtime Loading Limitations:

Runtime Loading Limitations
===========================

- |LANG| 2.0 to |LANG| 2.0 scenarios are handled with class loading APIs.
- |LANG| 1.0 to 2.0 and |LANG| 2.0 to 1.0 scenarios are handled with explicit
  APIs provided by interop.


