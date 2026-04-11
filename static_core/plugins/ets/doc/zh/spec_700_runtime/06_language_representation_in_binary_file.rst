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


.. _RT Language Representation in Binary File:

二进制文件中的语言表示
********************************************************************************

本章描述 |LANG| 编译器如何将不同的语言类型和构造翻译为字节码。

.. note::
   |LANG| 的 *类型擦除* 规则优先于本章讨论的翻译规则。
   也就是说，对于任意 |LANG| 类型 ``T``，首先应用 *类型擦除* 规则，然后再按照下述规则，
   将 ``T`` 的 *有效类型* 翻译为二进制表示。

|

.. _RT Value Types:

值类型
================================================================================

|LANG| 的 *值类型* 可根据上下文，以如下两种形式之一表示在二进制文件中：

- :ref:`Type Descriptor <RT Type Descriptor>`、:ref:`FieldType <RT FieldType>` 或 :ref:`Shorty <RT Shorty>` 中的 ``PrimitiveType``；
- 对应的预定义 *引用类型*。

某个 *引用类型* 的 :ref:`Type Descriptor <RT Type Descriptor>` 匹配 ``RefType``。|LANG| 类型、*基本类型* 与该
*引用类型* 的 :ref:`全限定名 <RT Fully Qualified Name>` 之间的对应关系如下表所示：

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Primitive
     - Reference
   * - boolean
     - ``u1``
     - ``std.core.Boolean``
   * - char
     - ``u16``
     - ``std.core.Char``
   * - byte
     - ``i8``
     - ``std.core.Byte``
   * - short
     - ``i16``
     - ``std.core.Short``
   * - int
     - ``i32``
     - ``std.core.Int``
   * - long
     - ``i64``
     - ``std.core.Long``
   * - float
     - ``f32``
     - ``std.core.Float``
   * - number,double
     - ``f64``
     - ``std.core.Double``

.. _RT Selecting Between Primitive And Reference:

在基本表示与引用表示之间进行选择
--------------------------------------------------------------------------------

每次使用某个 *值类型* 时，|LANG| 编译器默认都会尝试将其降级为对应的 *基本类型* 表示。
但在某些情况下，必须使用 *引用类型* 表示。完整场景如下：

- *联合类型* 的 *组成类型* 必须是 *引用类型*；也就是说，联合类型中的任意 *值类型* 在二进制文件中都表示为 *引用类型*；
- 如果某个 *值类型* 被用作泛型参数的约束，那么它在二进制文件中同样表示为 *引用类型*；
- 如果方法或函数参数的默认值是某个 *值类型*，那么该参数的类型在二进制文件中表示为 *引用类型*；
- 当某个类字段作为某个泛型类字段的实例化结果被继承，且该字段类型是类型参数时，该字段使用 *引用* 类型表示；
- 如果某个方法从泛型类或接口继承而来且没有被重写，那么在原始定义中类型为类型参数的那些参数，在二进制文件中表示为 *引用* 类型；
  方法的返回类型也是如此。另一方面，如果该方法被重写，则其参数和返回类型表示为 *基本类型*；
- 带有延迟初始化语义的类字段在二进制文件中表示为 *引用类型* 字段，并以 ``undefined`` 值初始化；
- *值类型* 的 ``valueOf`` 方法返回类型表示为 *引用*；
- 其他所有情况都选择 *基本* 表示。

.. code-block:: typescript
   :linenos:

    abstract class B<T> {
       n: number = 0.
       x: T

       f(i: number, y: T): T { return y }
       g(y: T): T { return y}
    }
    class D extends B<number> {
       override g: number = 1.
       override g(y: number) { return y }
    }
    let d = new D
    d.n                       // represented as primitive
    d.x                       // represented as reference
    let v1 = d.f(2., 3.)      // 2. is represented as primitive, 3. as reference
                              // function return is a reference,
                              // unboxed before assigning to v1
    let v2 = d.g(4.)          // 4. is represented as primitive,
                              // returned as primitive
    let v3 = (d as B).g(5.)   // 5. is represented as reference,
                              // return value is reference,
                              // unboxed before assigning to v3


|

.. _RT Type Any:

类型 ``Any``
================================================================================

|LANG| 类型 ``Any`` 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>`
匹配带有 :ref:`全限定名 <RT Fully Qualified Name>` ``Y`` 的 ``RefType``。

|

.. _RT Type Object:

类型 ``Object``
================================================================================

|LANG| 类型 ``Object`` 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>`
匹配带有 :ref:`全限定名 <RT Fully Qualified Name>` ``std.core.Object`` 的 ``RefType``。

|

.. _RT Type never:

类型 ``never``
================================================================================

|LANG| 类型 ``never`` 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>`
匹配带有 :ref:`全限定名 <RT Fully Qualified Name>` ``N`` 的 ``RefType``。

|

.. _RT Type void:

类型 ``void``
================================================================================

|LANG| 类型 ``void`` 在二进制文件中表示为 :ref:`Shorty <RT Shorty>` 中的基本类型 ``void``。

|

.. _RT Type null:

类型 ``null``
================================================================================

|LANG| 类型 ``null`` 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>`
匹配带有 :ref:`全限定名 <RT Fully Qualified Name>` ``std.core.Null`` 的 ``RefType``。

|

.. _RT Type string:

类型 ``string``
================================================================================

|LANG| 类型 ``string`` 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>`
匹配带有 :ref:`全限定名 <RT Fully Qualified Name>` ``std.core.String`` 的 ``RefType``。

|

.. _RT Type bigint:

类型 ``bigint``
================================================================================

|LANG| 类型 ``bigint`` 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>`
匹配带有 :ref:`全限定名 <RT Fully Qualified Name>` ``std.core.BigInt`` 的 ``RefType``。

|

.. _RT Array types:

数组类型
================================================================================

.. _RT Resizable Array Types:

可变长数组类型
--------------------------------------------------------------------------------

|LANG| 的 *可变长数组类型* 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>`
匹配带有 :ref:`全限定名 <RT Fully Qualified Name>` ``std.core.Array`` 的 ``RefType``。

.. _RT Fixed-Size Array Types:

固定大小数组类型
--------------------------------------------------------------------------------

|LANG| 的 *固定大小数组类型* 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。每个不同的 *固定大小数组类型*
都有唯一对应的预定义 :ref:`Class <RT Class>`。该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>` 匹配 ``ArrayType``。
其 *组件类型* 的 :ref:`全限定名 <RT Fully Qualified Name>` 与该 *固定大小数组类型* 组件类型的
:ref:`全限定名 <RT Fully Qualified Name>` 相同。

|

.. _RT Tuple Types:

元组类型
================================================================================

|LANG| 的 *元组类型* 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>`
匹配 ``RefType``，其 :ref:`全限定名 <RT Fully Qualified Name>` 为 ``std.core.Tuple1``、
``std.core.Tuple2``、...、``std.core.Tuple16`` 或 ``std.core.TupleN``，具体取决于该
*元组类型* 的组成类型数量。

**约束**：如上所示，|LANG| 运行时不会区分组成类型数量大于 ``16`` 的元组类型。

|

.. _RT Functional Types:

函数类型
================================================================================

|LANG| 的 *函数类型* 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示，该 :ref:`Class <RT Class>` 取决于 *函数类型* 的参数个数，
以及签名中是否存在 *rest 参数*。
对于不含 *rest 参数* 的 *函数类型*，其 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>` 匹配 ``RefType``，
对应的 :ref:`全限定名 <RT Fully Qualified Name>` 为 ``std.core.Function0``、
``std.core.Function1``、...、``std.core.Function16`` 或 ``std.core.FunctionN``。
具体 :ref:`Type Descriptor <RT Type Descriptor>` 取决于该 *函数类型* 的参数个数。
而对于带有 *rest 参数* 的 *函数类型*，其 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>` 同样匹配 ``RefType``，
对应的 :ref:`全限定名 <RT Fully Qualified Name>` 为 ``std.core.FunctionR0``、
``std.core.FunctionR1``、...、``std.core.FunctionR15`` 或 ``std.core.FunctionR16``。
具体 :ref:`Type Descriptor <RT Type Descriptor>` 取决于该 *函数类型* 的参数个数。

.. _RT Functional Objects:

函数对象
--------------------------------------------------------------------------------

|LANG| 的 *函数对象* 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。每个 *函数对象* 都有唯一对应的预定义 :ref:`Class <RT Class>`。
该 :ref:`Class <RT Class>` 的 ``fields`` 包含由 *函数对象* 捕获的引用。
该 :ref:`Class <RT Class>` 的 ``methods`` 包含调用 *函数对象* 所需的辅助函数。
包含 *函数对象* 函数体的 :ref:`Method <RT Method>` 会被加入到该 *函数对象* 所在外围类的 ``methods`` 中，
从而允许该 *函数对象* 的函数体访问外围类的私有成员。

|

.. _RT Union Types:

联合类型
================================================================================

|LANG| 的 *联合类型* 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。每个 *联合类型* 都有唯一对应的预定义 :ref:`Class <RT Class>`。
该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>` 匹配 ``UnionType``，其中各个 *组成类型* 的 *qualified name*，
就是该 *联合类型* 各 *组成类型* 的 *qualified name*。

|

.. _RT Utility Types:

工具类型
================================================================================

.. _RT Awaited:

Awaited
--------------------------------------------------------------------------------

|LANG| 类型 ``Awaited`` 会在编译期被完全展开，因此不会出现在运行时。

.. _RT NonNullable:

NonNullable
--------------------------------------------------------------------------------

|LANG| 类型 ``NonNullable`` 会在编译期被完全展开，因此不会出现在运行时。

.. _RT Partial:

Partial
--------------------------------------------------------------------------------

|LANG| 类型 ``Partial`` 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。每个 ``Partial`` 类型都有唯一对应的预定义 :ref:`Class <RT Class>`。
该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>` 匹配 ``RefType``，其 *非限定名* 为 ``%%partial-typeName``，
其中 ``typeName`` 是 ``Partial`` 类型实参的 *非限定名*。

.. _RT Required:

Required
--------------------------------------------------------------------------------

|LANG| 类型 ``Required`` 会在编译期被完全展开，因此不会出现在运行时。

.. _RT Readonly:

Readonly
--------------------------------------------------------------------------------

|LANG| 类型 ``Readonly`` 会在编译期被完全展开，因此不会出现在运行时。

.. _RT Record:

Record
--------------------------------------------------------------------------------

|LANG| 类型 ``Record`` 在二进制文件中由预定义 :ref:`Class <RT Class>` 表示。该 :ref:`Class <RT Class>` 的 :ref:`Type Descriptor <RT Type Descriptor>`
匹配 *qualified name* 为 ``std.core.Record`` 的 ``RefType``。

.. _RT ReturnType:

ReturnType
--------------------------------------------------------------------------------

|LANG| 类型 ``ReturnType`` 会在编译期被完全展开，因此不会出现在运行时。

|

.. _RT Class Types:

类类型
================================================================================

.. _RT Class Declaration:

类声明
--------------------------------------------------------------------------------

每个 *类声明* 都会被降级为一个 :ref:`Class <RT Class>`，其 *非限定名* 等于源码中的 *类* 名。
该 :ref:`Class <RT Class>` 的 ``fields`` 对应于该 *类* 中的全部 *类字段声明*。
其 :ref:`Class <RT Class>` 的 ``methods`` 对应该 *类* 中的全部 *类方法声明* 和 *类 accessor 声明*。
另外，对该 *类* 的每个 *构造器声明*，都会生成一个名为 ``<ctor>`` 的方法；
对该 *类* 中的全部 *静态块*，则会生成一个名为 ``<cctor>`` 的单个 *静态方法*。

对于该类的每个构造器声明，都会生成一个名为 ``<ctor>`` 的 :ref:`Method <RT Method>`。
对于该类中的每个静态块，都会生成一个名为 ``<cctor>`` 的静态 :ref:`Method <RT Method>` （``access_flags | ACC_STATIC == 1``）。

.. _RT Class Extension Clause:

类扩展子句
--------------------------------------------------------------------------------

该 *类* 的 *直接超类* 存储在 :ref:`Class <RT Class>` 的 ``super_class_off`` 字段中。

.. _RT Class Implementation Clause:

类实现子句
--------------------------------------------------------------------------------

该 *类* 的 *直接超接口* 存储在 :ref:`Class <RT Class>` 的 ``class_data`` 字段中。标签 ``INTERFACES`` 用于存储
这些 *直接超接口*。

.. _RT Class Field:

类字段
--------------------------------------------------------------------------------

每个 *类字段声明* 都会被降级为一个 :ref:`Field <RT Field>`，其名称等于源码中的 *字段* 名。
*字段* 的访问修饰符使用对应的 :ref:`Field Access Flags <RT Field Access Flags>` 表示：

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Modifier
     - Access Flag
   * - ``private``
     - ``ACC_PRIVATE``
   * - ``public``
     - ``ACC_PUBLIC``
   * - ``protected``
     - ``ACC_PROTECTED``
   * - ``static``
     - ``ACC_STATIC``
   * - ``readonly``
     - ``ACC_READONLY``

.. _RT Class Method:

类方法
--------------------------------------------------------------------------------

每个 *类方法声明* 都会被降级为一个 :ref:`Method <RT Method>`，其名称等于源码中的 *方法* 名。
*方法* 的修饰符使用对应的 :ref:`Method Access Flags <RT Method Access Flags>` 表示：

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Modifier
     - Access Flag
   * - ``private``
     - ``ACC_PRIVATE``
   * - ``public``
     - ``ACC_PUBLIC``
   * - ``protected``
     - ``ACC_PROTECTED``
   * - ``abstract``
     - ``ACC_ABSTRACT``
   * - ``final``
     - ``ACC_FINAL``
   * - ``native``
     - ``ACC_NATIVE``
   * - ``static``
     - ``ACC_STATIC``

.. _RT Class Accessor:

类 accessor
--------------------------------------------------------------------------------

每个 *类 accessor 声明* 都会被降级为一个 :ref:`Method <RT Method>`。
属性 ``propName`` 的 getter 对应的 :ref:`Method <RT Method>` 名称为 ``%%get-propName``。
属性 ``propName`` 的 setter 对应的 :ref:`Method <RT Method>` 名称为 ``%%set-propName``。

|

.. _RT Interface Types:

接口类型
================================================================================

.. _RT Interface Declaration:

接口声明
--------------------------------------------------------------------------------

每个 *接口声明* 都会被降级为一个 :ref:`Class <RT Class>`，其 *非限定名* 等于源码中的 *接口* 名。
该 :ref:`Class <RT Class>` 是一个 *接口*（``access_flags | ACC_INTERFACE == 1``，``access_flags | ACC_ABSTRACT == 1``）。
该 :ref:`Class <RT Class>` 的 ``fields`` 必须为空。其 :ref:`Class <RT Class>` 的 ``methods`` 对应该 *接口* 的全部 *接口属性* 与 *接口方法声明*。

.. _RT Superinterfaces And Subinterfaces:

超接口与子接口
--------------------------------------------------------------------------------

*接口* 的 *直接超接口* 的表示方式，与 :ref:`Class Implementation Clause <RT Class Implementation Clause>` 中的表示方式相同。

.. _RT Interface Property:

接口属性
--------------------------------------------------------------------------------

接口 accessor 的表示方式，与 :ref:`Class Accessor <RT Class Accessor>` 的表示方式相同。

.. _RT Interface Method:

接口方法
--------------------------------------------------------------------------------

每个 *接口方法声明* 都会被降级为一个 :ref:`Method <RT Method>`，其名称等于源码中的 *方法* 名。
该 :ref:`Method <RT Method>` 是一个 *抽象方法*。

|

.. _RT Enumeration Types:

枚举类型
================================================================================

每个 *枚举声明* 都会被降级为一个 :ref:`Class <RT Class>`，其 *非限定名* 等于源码中的 *枚举类型* 名。
该 :ref:`Class <RT Class>` 的 ``fields`` 对应于该 *枚举类型* 的全部 *枚举常量*。
其 :ref:`Class <RT Class>` 的 ``methods`` 对应该 *枚举类型* 的 *枚举方法*。
另外，还会为该 :ref:`Class <RT Class>` 生成一个名为 ``<cctor>`` 的单个 *静态方法*。
该 *静态方法* 对应该 *枚举类型* 的静态构造器。
该 :ref:`Class <RT Class>` 扩展了 :ref:`全限定名 <RT Fully Qualified Name>` 为 ``std.core.BaseEnum`` 的 :ref:`Class <RT Class>`。

|

.. _RT Namespaces And Modules:

命名空间与模块
================================================================================

每个 *命名空间声明* 都会被降级为一个 :ref:`Class <RT Class>`，其 *非限定名* 等于源码中的 *命名空间* 名。
该 :ref:`Class <RT Class>` 是 *抽象的*（``access_flags | ACC_ABSTRACT == 1``），并带有一个
:ref:`全限定名 <RT Fully Qualified Name>` 为 ``ets.annotation.Module`` 的 :ref:`Annotation <RT Annotation>`。
该 :ref:`Class <RT Class>` 的 ``fields`` 对应该 *命名空间* 的全部 *变量声明* 和 *常量声明*。
其 ``methods`` 对应该 *命名空间* 的全部 *函数声明*、*accessor 声明*、
*带接收者的函数声明* 以及 *带接收者的 accessor 声明*。
此外，每个 *命名空间* 还会生成一个名为 ``<cctor>`` 的 *静态方法*，
它对应于该 *命名空间* 的静态初始化器。
顶层 *命名空间* 的名称由 *构建系统* 选择，而其二进制表示保持不变。

|

.. _RT Annotations:

注解
================================================================================

.. _RT Annotation Declaration:

注解声明
--------------------------------------------------------------------------------

每个 *注解声明* 都会被降级为一个 :ref:`Class <RT Class>`，其 *非限定名* 等于源码中的 *注解* 名。
该 :ref:`Class <RT Class>` 是一个 *注解*（``access_flags | ACC_ANNOTATION == 1``，``access_flags | ACC_ABSTRACT == 1``）。
该 :ref:`Class <RT Class>` 的 ``fields`` 对应该 *注解* 的全部 *注解字段*，其 ``methods`` 必须为空。

.. _RT Annotation Field:

注解字段
--------------------------------------------------------------------------------

每个 *注解字段* 在字节码中的表示方式，与 :ref:`Class field <RT Class field>` 的表示方式相同。
