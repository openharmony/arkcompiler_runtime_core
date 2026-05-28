..
    Copyright (c) 2021-2026 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _Standard Library:

Standard Library
################

.. meta:
    frontend_status: Partly

The Standard Library of the |LANG| language defines the required set of types,
variables, constants, functions, and annotations that provide APIs for effective
and convenient programming.

A detailed description of all elements of the standard library is covered in
a separate document that is a part of the |LANG| distribution package. This
document also defines a set of exported entities directly accessible
in all |LANG| modules.

|

.. _Standard Library Usage:

Standard Library Usage
**********************

.. meta:
    frontend_status: Partly

A set of entities exported from the standard library (see
:ref:`Standard Library`)
is accessible as simple names (see :ref:`Accessible`) at module scope and
in nested scopes if not redefined.

.. code-block:: typescript
   :linenos:

    console.log("Hello, world!") // OK, 'console' is defined in the standard library

Names of predefined types listed in :ref:`Keywords` cannot be redefined.
Otherwise, a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    let void = 1 // Compile-time error

Other types, defined in the standard library, can be redefined in a nested
scope, but not in a module scope. A :index:`compile-time error` occurs in the
second case.

Functions, constants and variables can be redefined in a module scope
or in a nested scope. If a name is redefined in a certain scope, it makes
a standard library entity with this name inaccessible (shadowed)
in that scope.

.. code-block:: typescript
   :linenos:

    let console = 1 
    console.log("hello") // Compile-time error, 'console' is redefined

Explicit import of an entity from the standard library can be used to access
the redefined entity:

.. code-block:: typescript
   :linenos:

    import {console as out} from "std"

    let console = 1

    out.log("hello") // OK, output: "hello"

.. index::
   entity
   export
   accessibility
   simple name
   standard library

.. raw:: pdf

   PageBreak
