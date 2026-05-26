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


.. _RT Verification:

验证
********************************************************************************

尽管 |LANG| 编译器能够保证用户程序在结构上正确、不会违反 |LANG| 类型系统，并且只生成满足 :ref:`二进制文件格式 <RT Binary File Format>` 全部静态与结构约束的二进制文件，但 |LANG| 运行时无法保证被加载的二进制文件一定由该编译器生成，或一定是格式正确的。

.. note::
   编译期检查的另一个问题是不同版本二进制文件之间可能发生冲突。假设依赖模块 ``B`` 的模块 ``A`` 已成功编译。自 ``A`` 编译后，模块 ``B`` 的定义可能发生了破坏 :ref:`二进制兼容性 <Binary Compatibility>` 的变化。此类冲突只能在运行时通过验证过程被检测出来。

由于上述问题，|LANG| 运行时要求对所有已加载的二进制文件执行验证，并以运行时错误的形式拒绝所有未通过验证的二进制文件。

验证要保证程序的二进制表示在结构上正确，并且在任何方面都不违反 |LANG| 类型系统。例如，验证会检查某个 final 类是否随后又被继承；被调用方法的参数类型与参数个数是否与方法定义一致；以及指令是否违反 |LANG| 类型系统。

如果验证期间发生错误，则会在触发该验证的程序位置抛出错误。

|

.. _RT Verification Process:

验证过程
================================================================================

根据实现不同，验证过程可能发生在：

- 运行时，即在 :ref:`Class <RT Class>` 加载时；
- 静态阶段，即在程序链接序列已知且先于执行的情况下。

对这两种方式而言，验证都包含以下主要步骤：

- 依赖实体验证（参见 :ref:`Dependent Entity Verification <RT Dependent Entity Verification>`）；
- 类型系统验证（参见 :ref:`Type System Verification <RT Type System Verification>`）；
- 程序控制流验证（参见 :ref:`Control Flow Verification <RT Control Flow Verification>`）；
- 程序抽象解释（参见 :ref:`Abstract Interpretation <RT Abstract Interpretation>`）。

对于 :ref:`Dependent Entity Verification <RT Dependent Entity Verification>` 和 :ref:`Type System Verification <RT Type System Verification>`，验证单元是 :ref:`Class <RT Class>`。对于 :ref:`Abstract Interpretation <RT Abstract Interpretation>` 和 :ref:`Control Flow Verification <RT Control Flow Verification>`，验证单元是 :ref:`Method <RT Method>`。

.. note::
   每个 :ref:`Method <RT Method>` 的代码都彼此独立地进行验证。

|

.. _RT Dependent Entity Verification:

依赖实体验证
================================================================================

此步骤验证 :ref:`Class <RT Class>` 的所有依赖实体都能够被解析，并且在验证时不会产生错误。如果任一依赖实体无法被解析，或者其自身验证失败，则会抛出错误。

|

.. _RT Type System Verification:

类型系统验证
================================================================================

此步骤验证 :ref:`Class <RT Class>` 声明是否违反现有的静态类型系统，该类型系统既包含 |LANG| 类型，也包含其他用户定义类型。此步骤的检查包括但不限于：

- 检查 :ref:`Class <RT Class>` 没有去扩展某个 final 的 :ref:`Class <RT Class>`；
- 检查 :ref:`Class <RT Class>` 有权限访问其扩展的 :ref:`Class <RT Class>`；
- 等等。

如果此步骤中的任一检查失败，则会抛出错误。

|

.. _RT Control Flow Verification:

控制流验证
================================================================================

此步骤验证 :ref:`Method <RT Method>` 的控制流是否一致。此步骤的检查包括但不限于：

- 所有控制流指令的目标都必须是某条指令的起始位置；
- :ref:`Method <RT Method>` 的控制流不能到达其主体之外；
- 控制流只能通过专门指令到达异常处理器主体；
- :ref:`Method <RT Method>` 的控制流结束处必须以恰当的指令结束；
- 等等。

如果此步骤中的任一检查失败，则会抛出错误。

|

.. _RT Abstract Interpretation:

抽象解释
================================================================================

此步骤检查 :ref:`Method <RT Method>` 指令在类型上一致。抽象解释按指令逐条推进，在 :ref:`Method <RT Method>` 数据流中维护带有推断类型的实际状态。抽象解释会考虑每条指令如何影响 :ref:`Method <RT Method>` 数据流中的类型，并正确处理在给定指令处不同数据流的汇合。

此步骤的检查包括但不限于：

- 检查每个被访问的 :ref:`Field <RT Field>` / :ref:`Class <RT Class>` / :ref:`Method <RT Method>` 是否具有相应访问权限；
- 检查作为指令输入 / 方法参数所期望的类型，是否通过子类型关系与实际类型正确关联；
- 检查从 :ref:`Method <RT Method>` 返回的值类型是否与其签名匹配；
- 检查异常处理器的正确性；
- 等等。

如果此步骤中的任一检查失败，则会抛出错误。

.. _RT Verifier Type System:

验证器类型系统
--------------------------------

下面列出验证器内建类型的类型层次。

*用户定义类型* 的子类型关系由 :ref:`Runtime Type System <RT Runtime Type System>` 规定。

::

                                                Top
                                  ______________/ \___________
                                 /                            \
                            Primitive                      Reference
            ___________________/ \______________               |
           /                                    \              |
        bits32                                 bits64        Object
       __/ \____                             ___/ \___         |
      /         \                           /         \        |
  Float32        |                      Float64        |   FixedArray
     |           |                         |           |       |
    f32          |                        f64          |       |
     |       Integral32                    |       Integral64  |
     |      ____/|\_______                 |       ___/ \___   |
     |     /     |        \                |      /         \  |
     |    i32    |         |               |     i64       u64 |
     |    |     u32    Integral16          |     |           | |
     |    |\_____|__   ___/|\_______       |     |           | |
     |    |      |  \ /    |        \      |     |           | |
     |    |      |  i16    |         |     |     |           | |
     |    |       \__|_____|_.       |     |     |           | |
     |     \_________|____ | |       |     |     |           | |
     |               |    \|/        |     |     |           | |
     |               |    u16    Integral8 |     |           | |
     |               |\____|__   ___/|     |     |           | |
     |               |     |  \ /    |     |     |           | |
     |               |     |   i8    |     |     |           | |
     |               |      \__|_____|_.   |     |           | |
     |                \________|____ | |   |     |           | |
     |                         |    \|/    |     |           | |
     |                         |    u8     |     |           | |
     |                          \___ |     |     |           | |
     |                              \|     |     |           | |
     |                               u1    |     |           | |
     |                                \_   |     |   _______/  |
      \________________________________ \  |     |  / ________/
                                       \ | |     | | /
                                        null_reference
