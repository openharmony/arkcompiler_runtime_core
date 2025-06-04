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

.. _Implementation Details:

Implementation Details
######################

.. meta:
    frontend_status: Partly
    todo: Implement Type.for in stdlib

Important implementation details are discussed in this section.

.. _Import Path Lookup:

Import Path Lookup
******************

.. meta:
    frontend_status: Done

If an import path ``<some path>/name`` is resolved to a path in the folder
'*name*', then  the compiler executes the following lookup sequence:

-   If the folder contains the file ``index.ets``, then this file is imported
    as a separate module written in |LANG|;

-   If the folder contains the file ``index.ts``, then this file is imported
    as a separated module written in |TS|;

-   Otherwise, the compiler imports the package constituted by files
    ``name/*.ets``.

.. index::
   implementation
   import path
   compiler
   lookup sequence
   module
   package

|

.. _Compilation Units in Host System:

Compilation Units in Host System
**********************************

.. meta:
    frontend_status: Done

Modules and packages are created and stored in a manner determined by the
host system. The exact manner modules and packages are stored in a file
system is determined by a particular implementation of the compiler and other
tools.

As an example, a simple implementation is characterised by the following:

-  Module (package module) is stored in a single file.

-  All package modules are stored in files of the same folder.

-  Folder that can store several separate modules (one source file to contain a
   separate module or a package module).

-  Folder that stores a single package must not contain separate module
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

.. _How to get type via reflection:

How to Get Type Via Reflection
******************************

.. meta:
    frontend_status: None

The |LANG| standard library (see :ref:`Standard Library`) provides a pseudo
generic static method ``Type.for<T>()`` to be processed by the compiler in a
specific way during compilation. A call to this method allows getting a
variable of type ``Type`` that represents type ``T`` at runtime. Type ``T`` can
be any valid type.

.. code-block:: typescript
   :linenos:

    let type_of_int: Type = Type.for<int>()
    let type_of_string: Type = Type.for<String>()
    let type_of_array_of_int: Type = Type.for<int[]>()
    let type_of_number: Type = Type.for<number>()
    let type_of_Object: Type = Type.for<Object>()

    class UserClass {}
    let type_of_user_class: Type = Type.for<UserClass>()

    interface SomeInterface {}
    let type_of_interface: Type = Type.for<SomeInterface>()

.. index::
   pseudo generic static method
   static method
   compiler
   variable
   runtime

|

.. _Generic and Function Types Peculiarities:

Generic and Function Types Peculiarities
****************************************

Current compiler and runtime implementations use type erasure, and thus affect
the manner of behavior of generics and function types. It is expected to change
in the future. A particular example can be found under the last bullet of
the compile-time errors list in :ref:`InstanceOf Expression`.

.. index::
   generic
   function type
   compiler
   runtime implementation
   conversion

|

.. _Keyword struct and ArkUI:

Keyword ``struct`` and ArkUI
****************************

.. meta:
    frontend_status: Done

The current compiler reserves the keyword ``struct``
because it is used in legacy ArkUI code.
This keyword can be used as a replacement for the keyword ``class``
in :ref:`Class declarations`.
Class declarations marked with the keyword ``struct``
are processed by the ArkUI plugin
and replaced with class declarations that use specific ArkUI types.

.. index::
   compiler
   struct
   ArkUI
   compiler plugin

|

.. OutOfMemoryError for Primitive Type Operations:

``OutOfMemoryError`` for Primitive Type Operations
**************************************************

Execution of some primitive type operations (like
increment, decrement and assignment)
can throw ``OutOfMemoryError`` (see :ref:`Error Handling`)
if allocation of new object is required
but the available memory is not sufficient to perform it.

.. _Uniqueness of Functional Objects:

Uniqueness of Functional Objects
********************************

.. meta:
    frontend_status: Done

|TS| and |LANG| handle function objects differently, and the equality test can
perform differently. The difference can be eliminated in the future versions of
|LANG|.

.. code-block:: typescript
   :linenos:

    function foo() {}
    foo == foo  // true in Typescript while may be false in ArkTS
    const f1 = foo
    const f2 = foo
    f1 == f2 // true in Typescript while may be false in ArkTS


.. index::
   function object
   equality test

|

.. raw:: pdf

   PageBreak
