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


.. _RT Runtime Type System:

运行时类型系统
********************************************************************************

|LANG| 运行时处理以下类型：

- *基本类型*：``void``、``u1``、``i8``、``u8``、``i16``、``u16``、``i32``、``u32``、``f32``、``f64``、``i64``、``u64``、``any``；
- *引用类型*：其余所有类型。所有 *引用类型* 在二进制文件中都有对应的 :ref:`ForeignClass <RT ForeignClass>` 或 :ref:`Class <RT Class>`。

|LANG| 运行时还会进一步区分上述 *引用类型* 组别：

- ``Any``：运行时类型系统的顶层类型。它对应某个 :ref:`Class <RT Class>`，其 :ref:`Type Descriptor <RT Type Descriptor>` 匹配到带有 :ref:`Fully Qualified Name <RT Fully Qualified Name>` ``Y`` 的 ``RefType``。**注意：** 这与基本类型中的 ``any`` 并不相同。
- ``never``：运行时类型系统的底层类型。它对应某个 :ref:`Class <RT Class>`，其 :ref:`Type Descriptor <RT Type Descriptor>` 匹配到带有 :ref:`Fully Qualified Name <RT Fully Qualified Name>` ``N`` 的 ``RefType``。
- *数组类型*：表示某种 *组件类型* 数组的类型。它对应某个 :ref:`Class <RT Class>`，其 :ref:`Type Descriptor <RT Type Descriptor>` 匹配 ``ArrayType``。
- *联合类型*：表示其 *组件类型* 联合的类型。它对应某个 :ref:`Class <RT Class>`，其 :ref:`Type Descriptor <RT Type Descriptor>` 匹配 ``UnionType``。
- ``Object``：所有 *用户定义类型* 和 *数组类型* 的基类型。它对应某个 :ref:`Class <RT Class>`，其 :ref:`Type Descriptor <RT Type Descriptor>` 匹配到带有 :ref:`Fully Qualified Name <RT Fully Qualified Name>` ``std.core.Object`` 的 ``RefType``。
- *用户定义类型*：不属于上述任何组别的 *引用类型*。

这些类型中的每一种，在运行时都用名为 ``Class`` 的结构实例表示。每个唯一类型只对应一个 ``Class`` 结构实例，并落入上述某一类别中。

|

.. _RT Subtyping:

子类型关系
================================================================================

*引用类型* 与 *基本类型* 构成如下类型层次：

::

                         Any
               __________/ \__________
              /                       \
   *Primitive types*                Object
             |                ________/ \________
             |               /                   \
             |        *Array types*      *User-defined types*        *Union types*
             |               \___                 |                   ___/
             |                   \                |                  /
             |                   +-----------------------------------+
             |                   | language reference type hierarchy |
             |                   +-----------------------------------+
              \__________   _____________________/
                         \ /
                        never

关于 *基本类型* 的子类型关系，成立的规则是：只有当两个 *基本类型* 完全相同，其中一个才是另一个的子类型。

对于 *数组类型*、*联合类型* 和 *用户定义类型*，其子类型关系由 |LANG| 的子类型规则规定。
