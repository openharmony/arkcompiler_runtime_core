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
    todo: Implement Type.from in stdlib

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
    as a separate module written in |TS|;

-   Otherwise, the compiler imports the package constituted by files
    ``name/*.ets``.

.. index::
   implementation
   import path
   path
   folder
   file
   compiler
   lookup sequence
   module
   separate module
   package

|

.. _Compilation Units in Host System:

Compilation Units in Host System
********************************

.. meta:
    frontend_status: Done

Modules and packages are created and stored in a manner determined by the
host system. The exact manner modules and packages are stored in a file
system is determined by a particular implementation of the compiler and other
tools.

A simple implementation is summarized in the following example:

-  Module (package module) is stored in a single file.

-  All package modules are stored in files of the same folder.

-  Folder that can store several separate modules (one source file to contain a
   separate module or a package module).

-  Folder that stores a single package must not contain separate module
   files or package modules from other packages.

.. index::
   compilation unit
   host system
   storage
   storage management
   module
   package
   separate module
   file system
   implementation
   package module
   file
   folder
   source file

|

.. _How to get type via reflection:

How to Get Type Via Reflection
******************************

.. meta:
    frontend_status: None

The |LANG| standard library (see :ref:`Standard Library`) provides a
pseudogeneric static method ``Type.from<T>()`` to be processed by the compiler
in a specific way during compilation. A call to this method allows getting a
value of type ``Type`` that represents type ``T`` at runtime. 

.. code-block:: typescript
   :linenos:

    let type_of_int: Type = Type.from<int>()
    let type_of_string: Type = Type.from<String>()
    let type_of_array_of_int: Type = Type.from<int[]>()
    let type_of_number: Type = Type.from<number>()
    let type_of_Object: Type = Type.from<Object>()

    class UserClass {}
    let type_of_user_class: Type = Type.from<UserClass>()

    interface SomeInterface {}
    let type_of_interface: Type = Type.from<SomeInterface>()

.. index::
   pseudogeneric static method
   static method
   compiler
   method call
   call
   method
   variable
   runtime

There are some restrictions on type ``T``, see :ref:`Standard Library` for 
details.

|

.. _Generic and Function Types Peculiarities:

Generic and Function Types Peculiarities
****************************************

The current compiler and runtime implementations use type erasure.
Type erasure affects the behavior of generics and function types. It is
expected to change in the future. A particular example is provided in the last
bullet point in the list of compile-time errors in :ref:`InstanceOf Expression`.

.. index::
   generic
   function type
   compiler
   runtime implementation
   type erasure
   instanceof expression

|

.. _Keyword struct and ArkUI:

Keyword ``struct`` and ArkUI
****************************

.. meta:
    frontend_status: Done

The current compiler reserves the keyword ``struct`` because it is used in
legacy ArkUI code. This keyword can be used as a replacement for the keyword
``class`` in :ref:`Class declarations`. Class declarations marked with the
keyword ``struct`` are processed by the ArkUI plugin and replaced with class
declarations that use specific ArkUI types.

.. index::
   compiler
   keyword struct
   keyword class
   class declaration
   ArkUI plugin
   ArkUI type
   ArkUI code

|

.. OutOfMemoryError for Primitive Type Operations:

``OutOfMemoryError`` for Primitive Type Operations
**************************************************

The execution of some primitive type operations (e.g., increment, decrement, and
assignment) can throw ``OutOfMemoryError`` (see :ref:`Error Handling`) if
allocation of a new object is required but the available memory is not
sufficient to perform it.

.. index::
   primitive type
   primitive type operation
   operation
   increment
   decrement
   assignment
   error
   allocation
   object
   available memory

.. raw:: pdf

   PageBreak
