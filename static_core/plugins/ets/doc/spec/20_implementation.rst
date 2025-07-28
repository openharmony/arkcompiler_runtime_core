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
    as a module written in |LANG|;

-   If the folder contains the file ``index.ts``, then this file is imported
    as a module written in |TS|.


.. index::
   implementation
   import path
   path
   folder
   file
   compiler
   lookup sequence
   module

|

.. _Compilation Units in Host System:

Compilation Units in Host System
********************************

.. meta:
    frontend_status: Done

Modules and libraries are created and stored in a manner that is
determined by the host system. The exact manner modules and libraries are stored
in a file system is determined by a particular implementation of the compiler
and other tools.

A simple implementation is summarized as follows:

-  Module is stored in a single file.

-  Folder that can store several modules (one source file to contain a module).

-  Library description files are named "lib.ets". Source files are stored in
   folders with a predefined structure.

.. index::
   compilation unit
   host system
   storage
   storage management
   module
   file system
   implementation
   file
   folder
   source file

|

.. _Getting Type Via Reflection:

Getting Type Via Reflection
***************************

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

If type ``T`` used as type argument is affected by :ref:`Type Erasure`, then
the function returns value of type ``Type`` for *effective type* of ``T``
but not for ``T`` itself:

.. code-block:: typescript
   :linenos:

    let type_of_array1: Type = Type.from<int[]>() // value of Type for Array<> 
    let type_of_array2: Type = Type.from<Array<number>>() // the same Type value

|

.. _Ensuring Module Initialization:

Ensuring Module Initialization
******************************

.. meta:
    frontend_status: None

The |LANG| standard library (see :ref:`Standard Library`) provides a top-level
function ``initModule()`` with one parameter of ``string`` type. A call to this
function ensures that the module referred by the argument is available and its
initialization (see :ref:`Static Initialization`) is performed. An argument
should be a string literal otherwise a :index:`compile-time error` occurs.

The current module has no access to the exported declarations of the module
referred by the argument. If such module is not available or any other runtime
issue occurs then a proper exception is thrown.

.. code-block:: typescript
   :linenos:

    initModule ("@ohos/library/src/main/ets/pages/Index")

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

|

.. _Make a Bridge Method for Overriding Method:

Make a Bridge Method for Overriding Method
******************************************

.. meta:
    frontend_status: None

Situations are possible where the compiler must create an additional bridge
method to provide a type-safe call for the overriding method in a subclass of
a generic class. Overriding is based on *erased types* (see :ref:`Type Erasure`).
The situation is represented in the following example:

.. code-block:: typescript
   :linenos:

    class B<T extends Object> {
        foo(p: T) {}
    }
    class D extends B<string> {
        foo(p: string> {} // original overriding method
    }

In the example above, the compiler generates a *bridge* method with the name
``foo`` and signature ``(p: Object)``. The *bridge* method acts as follows:

-  Behaves as an ordinary method in most cases, but is not accessible from
   the source code, and does not participate in overloading;

-  Applies narrowing to argument types inside its body to match the parameter
   types of the original method, and invokes the original method.

The use of the *bridge* method is represented by the following code:

.. code-block:: typescript
   :linenos:

    let d = new D()
    d.foo("aa") // original method from 'D' is called
    let b: B<string> = d
    b.foo("aa") // bridge method with signature (p: Object) is called
    // its body calls original method, using (p as string) to check the type of the argument

More formally, a bridge method ``m(C``:sub:`1` ``, ..., C``:sub:`n` ``)``
is created in ``D``, in the following cases:

- Class ``B`` comprises type parameters
  ``B<T``:sub:`1` ``extends C``:sub:`1` ``, ..., T``:sub:`n` ``extends C``:sub:`n` ``>``;
- Subclass ``D`` is defined as ``class D extends B<X``:sub:`1` ``, ..., X``:sub:`n` ``>``;
- Method ``m`` of class ``D`` overrides ``m`` from ``B`` with type parameters in signature,
  e.g., ``(T``:sub:`1` ``, ..., T``:sub:`n` ``)``;
- Signature of the overriden method ``m`` is not ``(C``:sub:`1` ``, ..., C``:sub:`n` ``)``.


|

.. _Runtime Evaluation of Lambda Expressions Implementation:

Runtime Evaluation of Lambda Expressions Implementation
=======================================================

.. meta:
    frontend_status: Done


In order to make lambdas behave as required (see
:ref:`Runtime Evaluation of Lambda Expressions`), the language implementation
can act as follows:

-  If a captured variable is of a non-value type (see :ref:`Value Types`), then
   replace the captured variable type for a proxy class that contains an
   original reference (``x: T`` for ``x: Proxy<T>; x.ref = original-ref``).
-  If the captured variable is defined as ``const``, then proxying is not
   required.
-  If the captured formal parameter cannot be proxied, then the implementation
   can require adding of a local variable as shown in the table below.

.. index::
   lambda
   implementation
   runtime evaluation
   non-value type
   reference
   captured formal parameter
   predefined value type
   proxy class
   captured variable
   captured variable type
   proxying
   local variable
   variable
   source code
   pseudo code

+-----------------------------------+-----------------------------------+
|   Source Code                     |   Pseudo Code                     |
+===================================+===================================+
| .. code-block:: typescript        | .. code-block:: typescript        |
|    :linenos:                      |    :linenos:                      |
|                                   |                                   |
|     function foo(y: int) {        |     function foo(y: int) {        |
|     let x = () => { return y+1 }  |     let y$: Int = y               |
|     console.log(x())              |     let x = () => { return y$+1 } |
|     }                             |     console.log(x())              |
|                                   |     }                             |
+-----------------------------------+-----------------------------------+




.. raw:: pdf

   PageBreak
