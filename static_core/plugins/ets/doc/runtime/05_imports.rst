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

.. _Runtime Imports:

Runtime Imports
###############

This chapter describes runtime import and initialization rules.

.. _Initialization Granularity And Execution Order:

Initialization Granularity And Execution Order
==============================================

- Each module, namespace and each nested namespace has its own top-level scope.
  According to the |LANG| language specification, namespaces with same name
  within a single top-level scope are merged.

- Each top-level scope has a runtime entity with an initializer. Initializer
  executes all the top-level statements and variable declarations sequentially.

- Each class has its own initializer. Initializer sequentially executes all
  static properties initializers and code inside the ``static`` block.

Top-level scope and class initializers are invoked on the first access to the
representing runtime entity. In particular:

- Access to exported variable or function of top-level scope triggers the
  initializer.

- Access to nested namespace, class or module import does **not** trigger the
  initializer.

Initializers are executed **exactly once** and are concurrency-safe. In
particular, each initializer is guarded with a lock and all concurrent entities
wait until the initialization is complete.

.. _Initialization Laziness:

Initialization Laziness
=======================

The **laziness** of |LANG| 2.0 source code imported by |LANG| 1.0 is regulated
with |LANG| 1.0 import code itself, because the |LANG| 2.0 simply builds the
module object when the module object is requested for the first time.

Exisitng mainline already limits the usage of the sideeffects in top-level
scopes. We may consider an option that will bind the initialization of nested
scopes and trigger initialization on import for the application debug.

.. _Comprehensive Initializer Example:

Comprehensive Initializer Example
=================================

Let's assume the following source code in ``appsrc/mod.ets``:

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

Intialization is done as follows:

.. code-block:: typescript
    :linenos:

    import {A, foo} from "module1" // no sideeffects

    foo()  // call to function "foo" from "module1" toplevel triggers "module1.GLOBAL"
           // at this point "module1.A" is not affected
    A.prop // access to static property of A triggers "module1.A"

    // call to this function triggers the surrounding toplevel scope intializer
    export function bar() {}

    // instantiation "new A()" DOES NOT trigger
    // the surrounding toplevel scope initializer
    export class A { }

.. _Handling Binaries And Loading Classes:

Handling Binaries And Loading Classes
#####################################

While the build system may mandate some forms of distribution and rules of
packaging, |LANG| runtime subsystem relies only on binary executable files:

- Internal runtime loading APIs work **only** with binary executable files
  and do not change / patch classes / modules or names on load.
- Binary executable files for |LANG| 2.0 do not have the concept of the common
  ``ohmurl``, every class itself is independent and the name of every class is
  fully qualified.

The rules are as follows:

All entities (functions, records) inside the executable binary file have fully
qualified names which are not modified by the runtime on load events. In other
words, executable binary file is "flat" in terms of runtime names.

- Executable binary file by itself has no associated name or entrypoint,
  nor is associated with any source file.

- Executable binary file can be moved within the filesystem and runtime is not
  sensitive to the actual file system path to the executable binary file.

- Several executable binary files *.abc* can be merged togethe, and it doesn't
  affect the load procedure.

- Executable binary files are processed **only** within a fixed **load context**

|LANG| runtime determines the load context with custom class loading policies:

- Which executable binary file to locate in filesystem or extract and store
  into the context is a custom class loading policy.

|LANG| runtime defines at least two built-in load contexts:

- ``boot``: the core language and common platform libraries,
    set up in **AppSpawn**.

- ``app``: the context of the application files.

All the files for these context have to be explicitly provided on **AppSpawn**
and app startup. No any filesystem resolution for executable binary files on
core runtime side.

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
    final class ABCFile {
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


.. code-block:: typescript
    :linenos:

    /*
     * appspawn code, pre-fork, created in runtime from bootload files
     * specified by appspawn: ["stdlib.abc", "ohosapi.abc", "arkui.abc"]
     */
    let boot = getBootRuntimeLinker()

    /*
     * appspawn code, post-fork, create a new 'app' context RuntimeLinker
     * specifing all the files that are in intial ability har
     */
    let app = new AbcRuntimeLinker(boot, ["app.abc"])

    /* load a feature dynamically */
    app.addAbcFiles(["feature.abc"])

    /* load an isolated resource */
    let isolated = new AbcRuntimeLinker(boot, ["dynamics.abc"])

Resolution request from runtime has two inputs:

- Runtime class descriptor, which is computed from the **runtime name** used by
  language APIs: reflection, class loading, dynamic import
- Load context

.. _Load Contexts:

Load Contexts
=============

Each class is assoiated with the context it was loaded to. Context may list
classes, and a class has a link back to the context it was loaded into.

All references to classes and methods from bytecode are lazily resolved with
the following rules:

- Obtain a class which owns the bytecode.

- Obtain a context of the class, perform fast lookup if the class is already
  available in the context.

- Obtain an associated loader and perform resoltuion specified by the custom
  loader code.

- Loader **may** delegate the request to its **parent** loader

- Loader **may** be backed by a set of executable binary files, this loader is
  backed by the runtime implementation.

Overall, the core runtime does not rely on the particular distribution format
unless it is explicitly told which binaries to load.

.. _Application Load Contexts:

Application Load Contexts
=========================

Loading contexts for executable binary files form a tree structure, and the
resolution delegates the request to the parent linker if the class can not be
loaded, while parent linker does not child context to resolve existing
references in parent class.

**FIXME: picture**

This approach isolates separate loading contexts, allowing to load different
(or clashing) versions of the same module within in the single running
application. It is the natural way to load potentially conflicting or
independent parts of the application within independent class loaders.

**FIXME: splitload**

**FIXME: repackaging**

.. _Runtime Loading Limitations:

Runtime Loading Limitations
===========================

- |LANG| 2.0 to |LANG| 2.0 scenarios are handled with the class loading APIs.

- |LANG| 1.0 to 2.0 scenarios handled with explicit APIs provided by interop.

- |LANG| 2.0 to 1.0 scenarios handled with explicit APIs provided by interop,
