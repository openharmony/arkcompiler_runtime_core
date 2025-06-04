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

.. _Runtime Names:

Runtime Names
#############

|LANG| runtime (including internal ``ClassLinker`` APIs, standard library
reflection and class loading APIs) and build-time (build system, frontend,
bytecode manipulation tools) both work with modules, classes and other entities
through **runtime names**.

A runtime name is formed according to the following rules:

- |LANG| compiler associates a **name** with a **group** of source code files
  compiled within a single compilation configuration file
  (``build-config.json``).

- |LANG| compiler limits relative imports to be in the **same group** of
  source code files.

- |LANG| compiler obtains the **current group name** from the current
  compilation configuration file.

- If the imported file is from the **external group** of source code files,
  the external group name is obtained from the corresponding external
  compilation configuration file.

.. code-block:: json
    :linenos:

    /* In compilation configuration file: */
    {
        "name": "@app",
        "paths": {
            "@ohos/apix": "../../somewhere/within/sdk",
            "@lib": "../../some/path"
        }
    }

.. code-block:: typescript
    :linenos:

    import { A } from "@ohos/apix/fs"           // "fs" is associated with the name "@ohos/apix"
    import { B } from "@lib/components/buttons" // "components/buttons" is associated with name "@lib"
    import { C } from "../bar"                  // "bar" is associated with the name "@app"

- |LANG| compiler computes the **module name** for each file according to the
  **group name** and the relative path. ``@ohos/apix/fs`` will have module
  name ``@ohos.apix.fs``.

- Module itself uses the same module name as its runtime name.

- Entities inside the module are prefixed with the module name. As shown below,
  classes are named `@ohos.apix.fs.A` and so on.

Modules within a well-formed build-system are guaranteed to not clash by runtime
name. Well-formed assumes parties do not assign same **group** name, ``.abc``
files are not patched.

.. _Comprehensive Runtime Names Example:

Comprehensive Runtime Names Example
===================================

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

Runtime names are generated as follows (illustrated with pseudo-code):

.. code-block:: typescript
    :linenos:

    // Build system maps `appsrc` to `@app`
    class @app.mod.GLOBAL { // module top-level
        static x001: F
        static x002: F
        static foo() {
            return new F()
        }
        static {
            @app.mod.GLOBAL.x001 = @app.mod.GLOBAL.foo()
            @app.mod.GLOBAL.x002 = @app.mod.GLOBAL.foo()
        }
    }

    class @app.mod.NS { // merged namespace NS
        static x1: F
        static {
            @app.mod.NS.x1 = @app.mod.GLOBAL.foo()
        }
    }

    class @app.mod.NS.C { // class C inside namespace NS
        static x2: F
        static {
            @app.mod.NS.C.x2 = @app.mod.GLOBAL.foo()
        }
    }

    class @app.mod.C { // class C inside module
        static x3: F
        static {
            @app.mod.C.x3 = @app.mod.GLOBAL.foo()
        }
    }
