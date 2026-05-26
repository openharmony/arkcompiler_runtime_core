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


.. _Binary Compatibility:

二进制兼容性
################################################################################

.. meta:
    frontend_status: Partly

|LANG| 允许将同一个可执行程序的源代码分块编译。这一特性允许源代码作者彼此独立地开发和分发代码。不过，要想让已有程序在无需重新编译全部构成源代码的情况下继续可运行，就必须满足一定限制。

*二进制兼容性* 不同于 *源代码兼容性*。后者允许依赖方源代码在不产生任何编译时错误的情况下继续编译。某些 *源代码兼容* 的修改可能并不 *二进制兼容*，反之亦然。

.. note::
   某些 *二进制不兼容* 的修改可能会破坏执行环境中的类型系统不变式，使二进制代码无法执行。在这种情况下，运行时验证系统（见 :ref:`Verification <RT Verification>`）负责检测并拒绝这类二进制代码。

本章将 *二进制兼容* 的源代码修改定义为：在所有相关 *类型* 和 *命名空间* 仍然可加载、可解析的前提下，允许所有依赖程序保持这一性质的修改。

若依赖某个 *类型* 或 *命名空间* 的代码本身及其成员都无需在运行中的程序里执行，且该实体的静态初始化能够完成，则称该 *类型* 或 *命名空间* 是 *可加载的*。若该实体不可加载，其静态初始化就不会完成，而是抛出错误。

若某个 *类型* 或 *命名空间* 的所有成员，例如 *命名空间* 变量和函数，或 *类* 字段与 *方法*，都能在没有解析错误的情况下被访问，则称该 *类型* 或 *命名空间* 是 *可解析的*。*可解析* 的类型一定也是 *可加载* 的。

.. code-block:: typescript
   :linenos:

    // source1.ets
    export interface I {
      m(): void
    }

    // source2.ets
    export namespace N {

      // The type I must be loadable, otherwise the f and the whole N is not loadable.
      //
      // Example case: the definition of I is removed during the change in
      // the source1.ets, while the code of function f was not recompiled.
      export function f(i: I) {

        // I.m must be resolvable for the actual type of I, otherwise an attempt
        // to call i.m will throw a runtime error.
        //
        // Example case: the class that was compiled with an older source code
        // of source1.ets, where I does not declare the method m, does not provide
        // an implementation of the method I.m.
        return i.m()
      }
    }

    // app.ets

    class A implements I {
      m() { }
    }

    try {
      N.f(new A())
    } catch (e) {
      // Some platform-specific errors may appear if the binary compatibility
      // was broken by changes in source1.ets or source2.ets
      //
      // While the recovery for the running program may be possible in some
      // circumstances, this is a detail of the platform whether such
      // program can run or not, and whether it will be terminated during
      // the execution or not.
    }

源代码修改可能在执行期间产生特定错误，而这些错误在没有经历中间源码修改、一次性整体编译的程序中可能不会出现。由此产生的错误及其条件，可能导致某个实体仍然 *可解析*，或者不可解析但 *可加载*，或者甚至 *不可加载*。例如：

- 如果从源代码中删除了某个 *public* 或 *exported* 类型，则访问该类型的代码会失效，使某些依赖代码变得 *不可加载*；
- 如果方法定义缺失，或多个方法定义发生冲突，则 *动态方法解析* 可能失败。这种失败会在调用该方法时抛出错误，而相关类本身仍然 *可加载*，只是不可 *解析*；
- 类型层级结构的修改可能导致 *类型循环*。构成该循环的类型都是 *不可加载* 的。

.. _Classification of Source Code Changes:

源代码修改的分类
********************************************************************************

对于 *Method Declarations* 与 *Function Declarations* 的函数体、*Top-level Statements*、*Top-level Variables* 与 *static class fields* 的初始化表达式：

- 如果某个 *Method* 或 *Function* 声明、*field* 或 *variable* 的 *Effective Type* 未受影响，则该修改不会影响 *二进制兼容性*；
- 否则，请参见其他小节关于 *Effective Type* 变化的规定。

对于 *Namespace Definitions*：

不会影响二进制兼容性的修改包括：

- 重命名或删除未 *exported* 的 *namespace* 成员；
- 增加新的 *namespace* 成员；
- 增加新的 *function* 或 *variable*；
- 增加新的成员类型定义，例如 *class*、*interface*、*enum* 或 *type alias*。

会破坏二进制兼容性的部分修改包括：

- 修改或删除某个 *exported* *namespace* 的名称；
- 修改某个 *exported* *function* 或 *variable* 的 *Effective Type*；
- 修改或删除某个 *exported* *function* 或 *variable* 的名称；
- 修改或删除某个 *exported* *class*、*interface*、*enum*、*type alias* 或 *namespace* 的名称。

对于 *Class* 和 *Interface Declarations*：

不会影响二进制兼容性的修改包括：

- 重新排列 *extends* 或 *implements* 子句中的类型顺序；
- 在类型层级中插入此前不存在且不含成员的接口或类；
- 重新排列成员，例如 *fields*、*methods*；
- 增加、删除或重命名 *private* 成员；
- 向 *class* 增加新 *field*。

可能使部分已编译代码变得不可 *解析* 的修改包括：

- 如果某个派生类包含多个同名方法定义，则以下情况可能在方法解析期间引发冲突：

  - 向某个 *exported* 类增加新的 *public* 方法；
  - 将某个方法上移到类层级的更高位置；
  - 向接口增加新的默认方法或可选属性，也可能在方法解析期间造成冲突。

- 向接口增加 *required property* 或 *method*，但未为其提供 *default implementation*；
- 向已有 *exported* *abstract* 类增加新的 *abstract* 方法。

可能使部分已编译代码变得不可 *加载* 的修改包括：

- 修改某个 *public* 或 *protected* *method* 或 *field* 的 *Effective Type*；
- 修改或删除某个 *public* 或 *protected* *method* 或 *field* 的名称；
- 为已有 *exported* 类增加 *final* 修饰符。

对于 *enumeration declarations*：

不会影响二进制兼容性的修改包括：

- 增加新的枚举常量；
- 在保持类型不变的前提下修改枚举常量的值。

.. raw:: pdf

   PageBreak
