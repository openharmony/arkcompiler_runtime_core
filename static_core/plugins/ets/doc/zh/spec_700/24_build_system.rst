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


.. _Build System:

构建系统
################################################################################

.. meta:
    frontend_status: Partly

|LANG| 语言的构建系统定义了以下内容：

- :ref:`Package Definition`，它引入了包的概念，即由一个或多个模块组成、可分发的单元。
- :ref:`Module Visibility`，它根据模块头中是否存在修饰符 ``export`` 来判断模块能否被其他包访问。例如：``export module "name"``。没有 ``export`` 修饰符的模块属于包内部模块。
- :ref:`Runtime Name Formation`，它为每个实体定义全限定的运行时名称（见 :ref:`RT Fully Qualified Name`）。该名称用于运行时识别和访问控制。
- :ref:`ImportPath Resolution Rules`，它规定 importPath（见 :ref:`Import Path`）如何被解析到实际模块。

.. index::
    build system
    module
    module visibility
    export modifier
    internal module
    entity
    qualified name
    runtime
    access control

|

.. _Package Definition:

包定义
********************************************************************************

.. meta:
    frontend_status: Done

*Package* 被定义为将一个或多个模块（见 :ref:`Module Declarations`）分组在一起的实体。*Package* 是分发单位。每个模块都且仅属于一个包。

包通过配置文件描述，该文件向构建系统提供编译、链接和分发该包所需的全部信息。文件需指定以下内容：

- *Package name*，即在构建中唯一标识该包的字符串。该名称在形成运行时名称（见 :ref:`Runtime Name Formation`）时作为前缀使用；
- *Source files*，即构成该包的 |LANG| 文件集合。构建系统会编译这些文件，并将其合并为二进制文件（见 :ref:`RT Binary File Format`）；
- Dependencies，即该包依赖的其他包集合。构建系统利用这些信息在编译期间定位相关包，并确保它们的导出声明可供 importPath（见 :ref:`Import Path`）使用；
- ImportPath resolution rules，即影响 importPath（见 :ref:`Import Path`）如何解析为包内模块或外部包中模块的规则。构建系统会在编译期间应用这些规则来定位被导入模块。

.. index::
    package
    module

|

.. _Module Visibility:

模块可见性
********************************************************************************

.. meta:
    frontend_status: Done

*模块可见性* 决定某个模块能否被其他包中的模块访问。模块可见性由模块头（见 :ref:`Module Header`）中的修饰符 ``export`` 决定。如果模块没有模块头（见 :ref:`Module Header`），那么其可见性与“不带 ``export`` 修饰符的模块”相同。

模块可以是 *exported* 或 *internal*：

- *Exported* 是指使用修饰符 ``export`` 声明的模块。导出模块可被其他包中的模块访问。
- *Internal* 是指未使用修饰符 ``export`` 声明的模块。内部模块只能被同一包中的其他模块访问。

试图从其他包导入 *internal* 模块会导致 :index:`compile-time error`。

带有 *module header* 的 *exported* 模块示例如下：

.. code-block:: typescript
   :linenos:

    export module "x"
    // module contents

带有 *module header* 的 *internal* 模块示例如下：

.. code-block:: typescript
   :linenos:

    module "y"
    // module contents

没有 *module header* 的 *internal* 模块示例如下：

.. code-block:: typescript
   :linenos:

    // module contents

在构建系统中，包的 *public API* 指从导出模块导出的任意声明（详见 :ref:`Exported Declarations`）。如果某个导出函数、方法、类或接口的签名中包含未导出的类型，例如定义在内部模块中的类型，则会产生 :index:`compile-time error`。构建系统会在编译期验证这一性质，以确保包的公共接口保持自包含，不会在运行时可见性检查（见 :ref:`RT Verification`）中因泄漏内部类型而失败。

下面的示例说明了如何防止 *internal* 类型通过包接口泄漏：

.. code-block:: typescript
   :linenos:

    // file1.ets
    module "y"          // Internal module "y" (not exported)
    export class InternalClass1 {}

    // file2.ets        // Internal module (not exported)
    export class InternalClass2 {}

    // file3.ets
    export module "x"   // Exported module "x"
    import { InternalClass1 } from "./file1"        // OK, import is allowed
    import { InternalClass2 } from "./file2"        // OK, import is allowed

    export function foo(x: InternalClass1): void {} // Compile-time error, exported function signature contains reference to an unexported type
    export function foo(x: InternalClass2): void {} // Compile-time error, exported function signature contains reference to an unexported type

.. index::
    internal module

|

.. _Runtime Name Formation:

运行时名称形成
********************************************************************************

.. meta:
    frontend_status: Done

构建系统负责构造每个实体的运行时名称（见 :ref:`RT Runtime Name`）。

根据包名、模块名（见 :ref:`Module Header`）和实体的限定名形成运行时名称（见 :ref:`RT Runtime Name`）的规则如下：

.. code-block:: text
   :linenos:

    if M is not set:
        M = F

    if M is an empty string, and Q is not set:
        R = P
    if M is an empty string:
        R = P + ":" + Q
    else:
        R = P + ":" + M + "." + Q

    R = R.replace("/", ".")

其中：

- ``M`` 是在模块头（见 :ref:`Module Header`）中声明的模块名；
- ``F`` 是包（见 :ref:`Package Definition`）内文件的相对路径，去掉其扩展名后的结果；
- ``R`` 是实体的运行时名称（见 :ref:`RT Runtime Name`）；
- ``P`` 是包含该实体的包名（见 :ref:`Package Definition`）；
- ``Q`` 是实体在模块中的限定名。该名称由点分隔的标识符（见 :ref:`Names`）组成，但函数、变量、方法和静态方法除外。

如果得到的 ``R`` 与同一包中其他实体的运行时名称（见 :ref:`RT Runtime Name`）相同，则会产生 :index:`compile-time error`。

运行时名称形成示例如下：

.. code-block:: typescript
   :linenos:

    // packageName: pkg1

    // src/file1.ets
    export module ""            // Runtime Name: "pkg1"

    export class A {}           // Runtime Name: "pkg1:A"
    export namespace ns1 {      // Runtime Name: "pkg1:ns1"
        export namespace ns2 {  // Runtime Name: "pkg1:ns1.ns2"
            exprot class B {}   // Runtime Name: "pkg1:ns1.ns2.B"
        }
        export class C {}       // Runtime Name: "pkg1:ns1.C"
    }

    // src/file2.ets
    export module "x"           // Runtime Name: "pkg1:x"

    export class A {}           // Runtime Name: "pkg1:x.A"
    export namespace ns1 {      // Runtime Name: "pkg1:x.ns1"
        export namespace ns2 {  // Runtime Name: "pkg1:x.ns1.ns2"
            exprot class B {}   // Runtime Name: "pkg1:x.ns1.ns2.B"
        }
        export class C {}       // Runtime Name: "pkg1:x.ns1.C"
    }

    // src/file3.ets
    export class A {}           // Runtime Name: "pkg1:src.file3.A"
    export namespace ns1 {      // Runtime Name: "pkg1:src.file3.ns1"
        export namespace ns2 {  // Runtime Name: "pkg1:src.file3.ns1.ns2"
            exprot class B {}   // Runtime Name: "pkg1:src.file3.ns1.ns2.B"
        }
        export class C {}       // Runtime Name: "pkg1:src.file3.ns1.C"
    }

|

.. _ImportPath Resolution Rules:

ImportPath 解析规则
********************************************************************************

.. meta:
    frontend_status: Done

构建系统按照以下规则解析 importPath（见 :ref:`Import Path`）。

**步骤 1**：构建系统先尝试在当前包内解析 *import path*，可采用以下方式之一：

- *文件相对路径*。如果 *import path* 以 ``./`` 或 ``../`` 开头，则它指向一个相对于导入方文件位置解析得到的文件路径。若文件不存在，则产生 :index:`compile-time error`。
- *模块名*。如果 *import path* 以 ``//`` 开头，则它指向当前包内某个在 *module header*（见 :ref:`Module Header`）中声明的 *module name*。若该模块不存在，则产生 :index:`compile-time error`。
- *完整模块名*。如果 *import path*（见 :ref:`Import Path`）等于 ``"package_name/module_name"``，其中 ``package_name`` 为当前包名，``module_name`` 为当前包中某个在 *module header*（见 :ref:`Module Header`）中声明的模块名，则该 *import path* 指向相应模块。

**步骤 2**：构建系统再尝试将 *import path* 解析为外部模块，可采用以下方式之一：

- *Aliases section*。如果 *import path* 等于 ``"alias_name/module_name"``，其中 ``alias_name`` 是某个外部包名称的别名，``module_name`` 是该包中某个模块名，则该 *import path* 指向相应模块。
- *Dependencies section*。如果 *import path* 等于 ``"package_name/module_name"``，其中 ``package_name`` 是外部包名称，``module_name`` 是该包中某个模块名，则该 *import path* 指向相应模块。

如果 *import path* 无法被解析，则会产生 :index:`compile-time error`。

下面示例展示了在单个包内解析导入的情况：

.. code-block:: typescript
   :linenos:

    // packageName: pkg1

    // src/file1.ets
    module "x"
    export function foo() {}

    // src/file2.ets
    export function foo() {}

    // src/file3.ets
    import {...} from "./file1"       // OK, importPath is resolved to "file1.ets" by the relative path
    import {...} from "./file2"       // OK, importPath is resolved to "file2.ets" by the relative path
    import {...} from "../src/file1"  // OK, importPath is resolved to "file1.ets" by the relative path
    import {...} from "../src/file2"  // OK, importPath is resolved to "file2.ets" by the relative path
    import {...} from "//x"           // OK, importPath is resolved to "x" by the module name
    import {...} from "//file2"       // Compile-time error, importPath refers to a non-existent module inside the package
    import {...} from "pkg1/x"        // OK, importPath is resolved to "x" by the full module name
    import {...} from "pkg1/file2"    // Compile-time error, importPath refers to a non-existent module inside the package

.. raw:: pdf

   PageBreak
