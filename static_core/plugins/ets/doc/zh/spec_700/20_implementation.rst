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


.. _Implementation Details:

实现细节
################################################################################

.. meta:
    frontend_status: Partly
    todo: Implement Type.from in stdlib

本节讨论若干重要的实现细节。

.. _Import Path Lookup:

导入路径查找
********************************************************************************

.. meta:
    frontend_status: Done

如果某个导入路径 ``<some path>/name`` 被解析到目录 ``name`` 中的某个路径，则编译器按如下顺序查找：

- 如果该目录包含文件 ``index.ets``，则将该文件作为以 |LANG| 编写的模块导入；
- 如果该目录包含文件 ``index.ts``，则将该文件作为以 |TS| 编写的模块导入。

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

.. _Modules in Host System:

宿主系统中的模块
********************************************************************************

.. meta:
    frontend_status: Done

模块的创建和存储方式由宿主系统决定。模块在文件系统中的具体存储方式，取决于编译器及其他工具的具体实现。

一种简单实现是将每个模块存储在单独的文件中。

.. index::
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

.. _Getting Runtime Type Via Reflection:

通过反射获取运行时类型
********************************************************************************

.. meta:
    frontend_status: None

|LANG| 标准库（见 :ref:`Standard Library`）提供了一个伪泛型静态方法 ``Class.from<T>()``。编译器会在编译期间以特定方式处理该方法调用。调用此方法可以获取一个 ``Class`` 类型的值，在运行时表示类型 ``T``。

.. code-block:: typescript
   :linenos:

    let type_of_int: Class = Class.from<int>()
    let type_of_string: Class = Class.from<string>()
    let type_of_number: Class = Class.from<number>()
    let type_of_Object: Class = Class.from<Object>()

    class UserClass {}
    let type_of_user_class: Class = Class.from<UserClass>()

    interface SomeInterface {}
    let type_of_interface: Class = Class.from<SomeInterface>()

.. index::
   pseudogeneric static method
   static method
   compiler
   method call
   call
   method
   compilation
   variable
   runtime
   value
   type

如果作为类型实参使用的 ``T`` 会受到 :ref:`Type Erasure` 影响，那么该函数返回的 ``Class`` 值对应的是 ``T`` 的有效类型，而不是 ``T`` 本身：

.. code-block:: typescript
   :linenos:

    let type_of_array1: Class = Class.from<int[]>() // value of Class for Array<> 
    let type_of_array2: Class = Class.from<Array<number>>() // the same Class value

.. index::
   type argument
   type erasure
   function
   array

|

.. _Ensuring Module Initialization:

确保模块完成初始化
********************************************************************************

.. meta:
    frontend_status: None

|LANG| 标准库（见 :ref:`Standard Library`）提供了一个顶层函数 ``initModule()``，它只有一个 ``string`` 类型参数。该参数值会在编译期按照导入路径解析规则（见 :ref:`ImportPath Resolution Rules`）求值。调用该函数可以确保参数所指模块可用，并且其初始化过程（见 :ref:`Static Initialization`）已经执行。参数必须是字符串字面量，否则会产生 :index:`compile-time error`。

当前模块无法访问参数所指模块的导出声明。如果该模块不可用，或发生其他运行时问题，则会抛出相应异常。

.. code-block:: typescript
   :linenos:

    initModule ("@ohos/library/src/main/ets/pages/Index")

.. index::
   module initialization
   initialization
   top-level function
   parameter
   sting literal
   module
   argument
   function
   argument
   access
   declaration
   runtime
   exception

|

.. _Generic and Function Types Peculiarities:

泛型与函数类型的特殊性
********************************************************************************

当前编译器和运行时实现采用类型擦除。类型擦除会影响泛型和函数类型的行为，未来预期会有所变化。一个具体示例见 :ref:`instanceof Expression` 中关于编译时错误列表的最后一条说明。

.. index::
   generic
   function type
   compiler
   runtime implementation
   type erasure
   instanceof expression

|

.. _Keyword struct and ArkUI:

关键字 ``struct`` 与 ArkUI
********************************************************************************

.. meta:
    frontend_status: Done

当前编译器保留关键字 ``struct``，因为旧版 ArkUI 代码会使用它。该关键字可以在 :ref:`Class declarations` 中作为 ``class`` 的替代写法。使用 ``struct`` 标记的类声明会由 ArkUI 插件处理，并被替换为使用特定 ArkUI 类型的类声明。

.. index::
   compiler
   struct keyword
   class keyword
   class declaration
   ArkUI plugin
   ArkUI type
   ArkUI code

|

.. _Make a Bridge Method for Overriding Method:

为覆写方法生成桥接方法
********************************************************************************

.. meta:
    frontend_status: None

在某些情况下，编译器必须额外生成一个桥接方法，以便在泛型类子类中为覆写方法提供类型安全的调用。覆写基于擦除后的类型（见 :ref:`Type Erasure`）进行。如下例所示：

.. code-block:: typescript
   :linenos:

    class B<T extends Object> {
        foo(p: T) {}
    }
    class D extends B<string> {
        foo(p: string) {} // original overriding method
    }

在上述示例中，编译器会生成一个名为 ``foo``、签名为 ``(p: Object)`` 的桥接方法。该桥接方法的行为如下：

.. index::
   bridge method
   overriding method
   erased type
   subclass
   compiler
   type-safe call
   generic class
   type erasure
   signature

- 在大多数情况下，它表现为普通方法，但源代码中无法访问，也不参与重载；
- 它会在方法体内部把实参类型收窄到原始方法的参数类型，然后调用原始方法。

桥接方法的使用可由以下代码表示：

.. code-block:: typescript
   :linenos:

    let d = new D()
    d.foo("aa") // original method from 'D' is called
    let b: B<string> = d
    b.foo("aa") // bridge method with signature (p: Object) is called
    // its body calls original method, using (p as string) to check the type of the argument

更形式化地说，在以下情况下，会在 ``D`` 中创建桥接方法 ``m(C``:sub:`1` ``, ..., C``:sub:`n` ``)``：

- 类 ``B`` 含有类型参数 ``B<T``:sub:`1` ``extends C``:sub:`1` ``, ..., T``:sub:`n` ``extends C``:sub:`n` ``>``；
- 子类 ``D`` 定义为 ``class D extends B<X``:sub:`1` ``, ..., X``:sub:`n` ``>``；
- 类 ``D`` 的方法 ``m`` 覆写了 ``B`` 中带有类型参数签名的方法 ``m``，例如 ``(T``:sub:`1` ``, ..., T``:sub:`n` ``)``；
- 被覆写方法 ``m`` 的签名不是 ``(C``:sub:`1` ``, ..., C``:sub:`n` ``)``。

.. index::
   ordinary method
   access
   source code
   overloading
   argument type
   method
   bridge method
   type parameter
   subclass
   overriding
   signature
   overridden method

.. raw:: pdf

   PageBreak
