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


.. _Namespaces and Modules:

命名空间与模块
######################

.. meta:
    frontend_status: Done

|LANG| 中的程序被组织为可编译元素序列，这种形式称为 *模块*（见 :ref:`Module Declarations`）。
模块是一种特殊的命名空间形式，称为 *顶层命名空间*。模块还可以包含嵌套命名空间
（见 :ref:`Namespace Declarations`）。

|

.. _Module Declarations:

模块声明
*******************

.. meta:
    frontend_status: Done

每个模块都会创建自己的作用域（见 :ref:`Scopes`）。
如果未显式导出，模块中的变量、函数、类、接口或其他声明，只能在该作用域内访问
（见 :ref:`Accessible`）。所有导出实体的类型都必须显式给出
（详见 :ref:`Export Directives`）。

从模块导出的变量、函数、类、接口或其他声明，在被其他模块使用之前必须先被导入。

.. note::
   只有导出的声明才对第三方工具以及其他编程语言编写的程序可见。

模块可以由一个或多个文件组成（见 :ref:`Multifile Module`）。

所有 *模块* 都存储在文件系统或数据库中（见 :ref:`Modules in Host System`）。

一个 *模块* 可以选择性地包含以下部分：

#. 定义模块名的模块头；
#. 导入指令，用于在当前模块中使用导入进来的声明；
#. 顶层声明；
#. 顶层语句；
#. 再导出指令。

*模块* 的语法如下：

.. code-block:: abnf

    moduleDeclaration:
        moduleHeader? importDirective* (topDeclaration | topLevelStatements | exportDirective)*
        ;

模块能否被其他模块导入，由构建系统决定。

每个模块都可以直接使用标准库中定义的一组已导出实体（见 :ref:`Standard Library Usage`）。

.. code-block:: typescript
   :linenos:

    // Hello, world! module
    function main() {
      console.log("Hello, world!") // console is defined in the standard library
    }

如果一个模块至少包含一个顶层环境声明（见 :ref:`Ambient Declarations`），
那么它的其他所有声明也必须是环境声明，并且不得存在顶层语句
（见 :ref:`Top-Level Statements`）。否则会发生 :index:`compile-time error`。

.. code-block:: typescript
   :linenos:

    declare let x: number
    function main() {}
    // Compile-time error, ambient and non-ambient declarations are mixed


.. index::
   module
   module header
   import directive
   imported declaration
   module
   entity
   top-level declaration
   top-level statement
   re-export directive
   import
   console
   syntax
   standard library
   console

|

.. _Module Header:

模块头
*************

.. meta:
    frontend_status: Done

*模块头* 定义可选修饰符 ``export`` 和 *模块名*。

*模块头* 的语法如下：

.. code-block:: abnf

    moduleHeader:
        'declare'? 'export'? 'module' moduleName
        ;

    moduleName:
        StringLiteral
        ;

如果某个 *模块头* 带有 ``declare`` 修饰符，则整个模块都是环境模块。否则会发生
:index:`compile-time error`。

``export`` 修饰符由构建系统使用，用来使某个模块可被其他模块访问（见 :ref:`Accessible`），
通过导入（见 :ref:`Import Directives`）方式使用。

*模块头* 的用法见下例：

.. code-block:: typescript
   :linenos:

    export module "x"
    import {A} from "some module"
    export class B extends A {}

.. index::
    module header
    module name

|

.. _Namespace Declarations:

命名空间声明
**********************

.. meta:
    frontend_status: Done

*命名空间声明* 引入一个带名称的实体容器，其中包含若干可区分名称的实体。
每个命名空间都会创建自己的作用域（见 :ref:`Scopes`）。
如果未显式导出，命名空间中的变量、函数、类、接口或其他声明只能在该作用域中访问
（见 :ref:`Accessible`）。对导出实体的使用必须带上命名空间名限定。

*命名空间声明* 的语法如下：

.. code-block:: abnf

    namespaceDeclaration:
        'namespace' qualifiedName
        '{' (topDeclaration | topLevelStatements | staticBlock | exportDirective)* '}'
        ;

命名空间可以包含顶层语句或静态块，它们共同构成命名空间初始化器。
该初始化器只会在程序中至少使用了某个导出的命名空间成员时执行
（有关细节见 :ref:`Static Initialization`）。

*静态块* 计划在未来某个版本的 |LANG| 中被弃用，建议改用顶层语句。
一个命名空间中只允许一个 *静态块*。否则会发生 :index:`compile-time error`。

命名空间的用法如下所示：

.. code-block:: typescript
   :linenos:

    namespace NS1 {
        export function foo(): void {  }
        export let variable: int = 1234
        export const constant: int = 1234
        export let someVar: string

        // Will be executed before any use of NS1 members
        someVar = "some string"
        console.log("Init for NS1 done")
        export function bar(): void {}
    }

    namespace NS2 {
        export const constant:int = 1
        // Will never be executed since NS2 members are never used
        console.log("Init for NS2 done")
        export function bar(): void {}
    }

    export function bar(): void {}  // That is a different bar()

    if (NS1.variable == NS1.constant) {
        NS1.variable = 4321
    }
    NS1.bar()  // namespace bar() is called
    bar()      // top-level bar() is called

.. index::
   namespace
   namespace declaration
   qualified name
   qualifier
   access
   entity
   syntax
   export
   qualified name
   initializer block
   namespace variable
   static initialization
   call


.. note::
   导出的命名空间实体在同一模块中、命名空间外部可以以 *qualifiedName* 形式使用。
   任意命名空间实体在命名空间内部都可以并且通常都是不带限定地使用，也就是不写命名空间名。
   在命名空间内部，只有当实体被导出时，才允许对该实体使用 *qualifiedName*。
   无论在命名空间内外，对未导出实体使用 *qualifiedName* 都会导致 :index:`compile-time error`：

   .. code-block:: typescript
      :linenos:

       namespace NS {
           export let a: number = 1
           let b = 2

           export function foo(): void {
               let v: number
               v = a // OK, no qualification
               v = NS.a // OK, `a` exported
           }

           export function bar(): void {
               let v: number
               v = b  // OK, no qualification
               v = NS.b // Compile-time error, `b` not exported
           }
       }

       NS.a = 1 // OK,  `NS.a` exported
       NS.b = 1 // Compile-time error, `NS.b` not exported

.. note::
   一个命名空间若要在另一个模块中使用，则它本身必须被导出：

   .. code-block:: typescript
      :linenos:

       // File1
       namespace Space1 {
           export function foo(): void { ... }
           export let variable: int = 1234
           export const constant: int = 1234
       }
       export namespace Space2 {
           export function foo(p: number): void { ... }
           export let variable: int = "1234"
       }

       // File2
       import {Space2 as Space1} from "File1"

       // Compile-time error - there is no variable or constant called 'constant'
       if (Space1.variable == Space1.constant) {
            // Compile-time error - incorrect assignment as type 'number'
            // is not compatible with type 'string'
           Space1.variable = 4321
       }
       Space1.foo()     // Compile-time error - there is no function 'foo()'
       Space1.foo(1234) // OK

.. index::
   namespace
   module
   variable
   constant
   function
   compatibility
   string
   embedded namespace

.. note::
   允许嵌套命名空间：

   .. code-block:: typescript
      :linenos:

       namespace ExternalSpace {
           export function foo(): void { ... }
           export let variable: number = 1234
           export namespace EmbeddedSpace {
               export const constant: int = 1234
           }
       }

       if (ExternalSpace.variable == ExternalSpace.EmbeddedSpace.constant) {
           ExternalSpace.variable = 4321
       }


.. note::
   在单个模块中，如果多个命名空间具有相同的命名空间名，则它们的导出声明会合并为一个命名空间。
   如果发生重复，则会导致 :index:`compile-time error`。
   同名的导出声明和未导出声明也同样属于 :index:`compile-time error`。
   参与合并的命名空间中最多只能有一个带初始化器。否则会发生
   :index:`compile-time error`。

.. index::
   embedded namespace
   namespace
   namespace name
   module
   export
   declaration
   exported declaration
   non-exported declaration
   initializer

.. code-block:: typescript
   :linenos:

    // One source file
    namespace A {
        export function foo(): void { console.log ("1st A.foo() exported") }
        function bar(): void {  }
        export namespace C {
            export function too(): void { console.log ("1st A.C.too() exported") }
        }
    }

    namespace B {  }

    namespace A {
        export function goo(): void {
            A.foo() // calls exported foo()
            foo()   /* calls exported foo() as well as all A namespace
                       declarations are merged into one */
            A.C.moo()
        }
        //export function foo(): void {  }
        // Compile-time error as foo() was already defined

        // function foo() { console.log ("2nd A.foo() non-exported") }
        // Compile-time error as foo() was already defined as exported
    }

    namespace A.C {
        export function moo(): void {
            too() // too()  accessible when namespace C and too() are both exported
            A.C.too()
        }
    }

    A.goo()

    // File
    namespace A {
        export function foo(): void { ... }
        export function bar(): void { ... }
    }

    namespace A {
        function goo() { bar() }  // exported bar() is accessible in the same namespace
        export function foo(): void { ... }  // Compile-time error as foo() was already defined
    }

.. index::
   namespace
   export function
   qualified name
   notation
   shortcut notation
   embedded namespace
   access
   accessibility
   export function
   initializer

.. note::
   命名空间名可以是限定名。这是嵌套命名空间的简写形式，如下所示：

   .. code-block:: typescript
      :linenos:

       namespace A.B {
           /*some declarations*/
       }

   上述代码等价于：

   .. code-block:: typescript
      :linenos:

       namespace A {
           export namespace B {
             /*some declarations*/
           }
       }

   下例说明了这种写法的使用：

   .. code-block:: typescript
      :linenos:

       namespace A.B.C {
           export function foo(): void { ... }
       }

       A.B.C.foo() // Valid function call, as 'B' and 'C' are implicitly exported

   当命名空间以限定名形式发生合并时，所有限定层级都会被合并。
   如果发生重复，则会出现 :index:`compile-time error`。

   .. code-block:: typescript
      :linenos:

       namespace A {
           function foo() { ... } // #1
           export namespace B {
              export function foo(): void { ... } // #2
           }
       }

       namespace A.B {
           export function foo(): void { ... } // #3
       }

       // Declarations of functions #2 and #3 lead to a compile-time error,
       //   duplicated declaration of function A.B.foo()

       // While function foo() in namespace A is a valid declaration



如果某个环境命名空间（见 :ref:`Ambient Namespace Declarations`）定义在某个模块（见 :ref:`Module Declarations`）中，
那么该环境命名空间中的所有声明在该模块的所有声明和顶层语句中都是可访问的。

.. code-block:: typescript
   :linenos:

    declare namespace A {
        function foo(): void
        type X = Array<number>
    }

    A.foo() // Valid function call, as 'foo' is accessible for top-level statements
    function foo () {
        A.foo() // Valid function call, as 'foo' is accessible here as well
    }
    class C {
        method () {
            A.foo() // Valid function call, as 'foo' is accessible here too
            let x: A.X = [] // Type A.X can be used
        }
    }

.. index::
   namespace
   export namespace
   module
   ambient namespace
   declaration
   accessible declaration
   access
   accessibility
   top-level statement
   module

|

.. _Import Directives:

导入指令
*****************

.. meta:
    frontend_status: Partly
    todo: syntax is updated

*导入指令* 通过不同的绑定形式，使从其他模块导出的实体
（见 :ref:`Module Declarations`）可以在当前模块中使用。
这些指令在程序执行期间不起作用。

一个导入声明包含以下两部分：

- 导入路径，用于确定从哪个模块导入；
- 导入绑定，用于定义当前模块可以以什么形式使用哪些实体
  （限定形式或非限定形式）。

.. index::
   import directive
   export
   entity
   binding
   module
   directive
   import declaration
   import path
   import binding
   qualified form
   unqualified form
   syntax

*导入指令* 的语法如下：

.. code-block:: abnf

    importDirective:
        'import' 'type'? bindings 'from' importPath
        ;

    bindings:
        defaultBinding
        | (defaultBinding ',')? allBinding
        | (defaultBinding ',')? selectiveBindings
    ;

    allBinding:
        '*' bindingAlias
        ;

    bindingAlias:
        'as' identifier
        ;

    defaultBinding:
        'type'? identifier
        ;

    selectiveBindings:
        nameBinding (',' nameBinding)*
        ;

    nameBinding:
        'type'? identifier bindingAlias?
        | 'default' 'as' identifier
        ;

    importPath:
        StringLiteral
        ;

每个 binding 都会向模块作用域中添加一个或多个实体（见 :ref:`Scopes`）。
任何被添加的实体都必须在声明作用域（见 :ref:`Declarations`）中可区分。

带有 ``type`` 修饰符的导入将在 :ref:`Import Type Directive` 中讨论。

如果出现以下情形，则会发生 :index:`compile-time error`：

- 在 ``import`` 指令之前出现了非 import 指令、声明或语句；
- 通过 binding 加入模块作用域的实体不可区分；
- 模块直接导入其自身，并且 ``importPath`` 指向当前模块所在文件；或者
- 使用了 ``import type``，同时某个 binding 里也使用了 ``type``。

如果 ``importPath`` 指向一个不含 |LANG| 代码、其语法起始符对应空产生式的文件，
则会出现 :index:`compile-time warning`。

.. index::
   binding
   declaration
   module
   scope
   declaration
   declaration scope
   import directive
   type
   type modifier
   modifier
   storage
   import type

|

.. _Bind All with Qualified Access:

通过限定访问绑定全部实体
==============================

.. meta:
    frontend_status: Done

导入绑定 ``* as A`` 会把单个具名实体 *A* 绑定到当前模块的声明作用域中。

由 *A* 和实体名组成的限定名 ``A.name`` 用于访问由导入路径所定义模块中导出的任意实体。

+---------------------------------+--+-------------------------------+
|   Import                        |  |   Usage                       |
+=================================+==+===============================+
|                                                                    |
+---------------------------------+--+-------------------------------+
| .. code-block:: typescript      |  | .. code-block:: typescript    |
|                                 |  |                               |
|     import * as Math from "..." |  |     let x = Math.sin(1.0)     |
+---------------------------------+--+-------------------------------+

推荐使用这种导入形式，因为当所有导出实体都以前缀模块名的形式出现时，源码更容易阅读和理解。

.. index::
   import binding
   import
   binding
   qualified name
   entity
   declaration scope
   module
   name
   access
   export
   import path

|

.. _Default Import Binding:

默认导入绑定
======================

.. meta:
    frontend_status: Done

默认导入绑定允许把模块中的默认导出声明导入进来。
使用时不需要知道声明的实际名称，因为导入时会给它起一个新名字。
如果对最初以 default 形式导出的实体使用了其他导入形式，则会发生
:index:`compile-time error`。

*默认导入绑定* 有两种形式：

- 单个标识符；
- 使用关键字 ``default`` 的 selective import 特殊形式。

.. code-block:: typescript
   :linenos:

    // Module 1
    import DefaultExportedItemBindedName from "Module 2"
    import {default as DefaultExportedItemNewName} from "Module 2"
    function foo () {
      let v1 = new DefaultExportedItemBindedName()
      // instance of class 'SomeClass' to be created here
      let v2 = new DefaultExportedItemNewName()
      // instance of class 'SomeClass' to be created here
    }

    // Module 2
    export default class SomeClass {}

    // Module 3 - the same semantics as in Module 2
    class SomeClass {}
    export default SomeClass

   // Module 4
   export {default} from "Module 2" // Module 4 re-exports default export of Module 2
   export {default as SomeClassNewName} from "Module 2"
      // Module 4 re-exports default export of Module 2 as a new exported name


.. index::
   import binding
   entity
   import
   declaration
   export
   module
   default keyword
   identifier
   selective import

|

.. _Selective Binding:

选择性绑定
=================

.. meta:
    frontend_status: Done

*选择性绑定* 允许绑定以 *identifier* 形式导出的实体，
或者绑定默认导出的实体（见 :ref:`Default Import Binding`）。

使用 *identifier* 的绑定会把名为 *identifier* 的导出实体绑定到当前模块的声明作用域中。
如果不存在 *binding alias*，则该实体以原名加入声明作用域。
否则，使用 *binding alias* 中给出的标识符。
在后者情况下，被绑定实体不再能以原名访问（见 :ref:`Accessible`）。

如果某个标识符指向一个重载函数（见 :ref:`Overloading`），则所有可访问的重载函数都会被导入。
导出的显式函数重载（见 :ref:`Explicit Function Overload`）则以其自身导出名导入。

.. code-block:: typescript
   :linenos:

    // File1
    export function foo(p: number): void {} // #1
    export function foo(p: string): void {} // #2
    export function fooString(p: string): void {}
    export function fooBoolean(p: Boolean): void {}
    export overload bar {fooBoolean, fooString}

    function foo() {} // #3

    // File2
    import {foo} from "File1"  // all exported 'foo' are imported
    import {bar} from "File1"  // exported explicit overload is imported by its own name
    foo(5)          // #1 is called
    foo("a string") // #2 is called
    bar(true)       // fooBoolean is called
    bar("a string") // fooString is called
    foo()           // Compile-time error, as #3 is not exported

使用导出实体的 *选择性绑定* 如下例所示：

.. index::
   import binding
   simple name
   identifier
   export
   call
   name
   declaration scope
   overloaded function
   entity
   access
   accessibility
   bound entity
   selective binding
   binding

.. code-block:: typescript
   :linenos:

    export const PI: number = 3.14
    export function sin(d: number): number {}

.. note::
   以下示例中，用 ``"..."`` 代替模块的导入路径：

+---------------------------------+--+--------------------------------------+
|   Import                        |  |   Usage                              |
+=================================+==+======================================+
|                                                                           |
+---------------------------------+--+--------------------------------------+
| .. code-block:: typescript      |  | .. code-block:: typescript           |
|                                 |  |                                      |
|     import {sin} from "..."     |  |     let x = sin(1.0)                 |
|                                 |  |     let f: float = 1.0               |
+---------------------------------+--+--------------------------------------+
|                                                                           |
+---------------------------------+--+--------------------------------------+
| .. code-block:: typescript      |  | .. code-block:: typescript           |
|                                 |  |                                      |
|     import {sin as Sine} from " |  |     let x = Sine(1.0) // OK          |
|         ..."                    |  |     let y = sin(1.0) /* Error ‘sin’  |
|                                 |  |        is not accessible */          |
+---------------------------------+--+--------------------------------------+

一条导入指令可以列出多个名称：

+-------------------------------------+--+---------------------------------+
|   Import                            |  |   Usage                         |
+=====================================+==+=================================+
|                                                                          |
+-------------------------------------+--+---------------------------------+
| .. code-block:: typescript          |  | .. code-block:: typescript      |
|                                     |  |                                 |
|     import {sin, PI} from "..."     |  |     let x = sin(PI)             |
+-------------------------------------+--+---------------------------------+
|                                                                          |
+-------------------------------------+--+---------------------------------+
| .. code-block:: typescript          |  | .. code-block:: typescript      |
|                                     |  |                                 |
|     import {sin as Sine, PI} from " |  |     let x = Sine(PI)            |
|       ..."                          |  |                                 |
+-------------------------------------+--+---------------------------------+

对同一导入路径使用多个绑定的复杂情况，将在 :ref:`Several Bindings for One Import Path`
中讨论。

.. index::
   import directive
   import path
   binding
   import

|

.. _Import Type Directive:

Import Type 指令
=====================

.. meta:
    frontend_status: Partly
    todo: no CTE for type import

导入指令可以带 ``type`` 修饰符，其唯一目的在于获得更好的 |TS| 语法兼容性
（另见 :ref:`Export Type Directive`）。
|LANG| 不会对通过 *import type* 指令导入的实体执行额外的语义检查。

|TS| 中由编译器执行而 |LANG| 中不执行的语义检查如下所示：

.. code-block:: typescript
   :linenos:

    // File module.ets
    console.log ("Module initialization code")

    export class Class1 {/*body*/}

    class Class2 {}
    export type {Class2}

    // MainProgram.ets

    import {Class1} from "./module.ets"
    import type {Class2} from "./module.ets"

    let c1 = new Class1() // OK
    let c2 = new Class2() // Compile-time error in |TS|, OK in |LANG|

``type`` 的另一种形式，是把 ``type`` 附着到名称绑定上。
这样就可以把普通导入和 ``type`` 导入混合使用。

.. code-block:: typescript
   :linenos:

    // File module.ets
    console.log ("Module initialization code")

    class Class1 {/*body*/}
    class Class2 {}
    export {Class1, type Class2}

    // MainProgram.ets

    import {Class1, type Class2 } from "./module.ets"

    let c1 = new Class1() // OK
    let c2 = new Class2() // Compile-time error in |TS|, OK in |LANG|

.. index::
   import binding
   import directive
   import
   import type
   import type directive
   type modifier
   semantic check
   syntax
   compatibility
   name binding
   binding
   export type
   compiler
   module
   general import
   type import

|

.. _Import Path:

导入路径
===========

.. meta:
    frontend_status: Done

*导入路径* 是一个字符串字面量，用于决定从何处以及以何种方式查找被导入模块。

*导入路径* 可以包含以下内容：

- 初始点 ``.`` 或双点 ``..``，后跟斜杠 ``/``；
- 一个或多个路径组件（其字符子集与大小写敏感性必须遵循宿主文件系统的路径规则）；
- 用于分隔路径组件的斜杠字符。

在导入路径中，无论宿主系统如何，都统一使用斜杠字符 ``/``。
此上下文中不使用反斜杠字符。

在多数文件系统中，导入路径看起来像文件路径。
*相对* 与 *非相对* 导入路径拥有不同的 *解析* 规则，
这些规则会把导入路径映射到宿主系统中的文件路径。

.. index::
   import binding
   string literal
   import path
   alpha-numeric character
   import
   compilation
   import path
   context
   file system
   relative import path
   non-relative import path
   resolution
   path component
   case sensitivity
   subset
   file path
   path rule
   slash character
   backslash character

编译器使用自己的算法，根据导入路径定位模块源码。
如果导入路径未指定文件扩展名，则编译器可以按照自身规则和优先级补上某些扩展名。
如果导入路径指向一个文件夹，则如何处理由具体编译器决定。
如果编译器无法确定性地定位模块源码，则会发生 :index:`compile-time error`。

.. index::
   compiler
   import path
   source
   module
   folder
   extension
   file

*相对导入路径* 以 ``./`` 或 ``../`` 开头。相对路径示例如下：

.. code-block:: typescript
   :linenos:

    "./components/entry"
    "../constants/http"

*相对导入* 的解析相对于导入方文件进行。
它用于保持模块之间的相对位置关系。

.. code-block:: typescript
   :linenos:

    import * as Utils from "./mytreeutils"

其他导入路径都属于 *非相对*。

*非相对路径* 的解析依赖于编译环境。
编译器环境的定义通常可以在配置文件或环境变量中提供。

*base URL* 设置用于解析以 ``/`` 开头的路径。
其他所有情况则使用 *路径映射*。
解析细节由实现决定。例如，编译配置文件可以包含：

.. code-block:: typescript
   :linenos:

    "baseUrl": "/home/project",
    "paths": {
        "std": "/arkts/stdlib"
    }

在上述示例中，``/net/http`` 会被解析为 ``/home/project/net/http``，
而 ``std/components/treemap`` 会被解析为 ``/arkts/stdlib/components/treemap``。

文件名、放置位置和格式都依赖于具体实现。

如果上述配置生效，则第一个路径在应用 ``baseUrl`` 后会直接映射到文件系统；
第二个路径中的 ``std`` 会被替换为 ``/arkts/stdlib``。
非相对路径示例如下：

.. code-block:: typescript
   :linenos:

    "/net/http"
    "std/components/treemap"

.. index::
   relative import path
   relative path
   non-relative import path
   non-relative path
   compilation environment
   compiler environment
   imported file
   relative location
   configuration file
   environment variable
   resolving
   base URL
   path mapping
   resolution
   implementation
   treemap
   file system


|

.. _Several Bindings for One Import Path:

同一导入路径的多个绑定
====================================

.. meta:
    frontend_status: Done

同一组被绑定实体可以通过以下方式使用：

- 多个导入绑定；
- 一条导入指令，或多条具有相同导入路径的导入指令：

+---------------------------------+-----------------------------------+
|                                 |                                   |
+---------------------------------+-----------------------------------+
|                                 | .. code-block:: typescript        |
| In one import directive         |                                   |
|                                 |     import {sin, cos} from "..."  |
+---------------------------------+-----------------------------------+
|                                 | .. code-block:: typescript        |
| In several import directives    |                                   |
|                                 |     import {sin} from "..."       |
|                                 |     import {cos} from "..."       |
+---------------------------------+-----------------------------------+

上例中不会发生冲突，因为这些导入绑定定义了互不相交的名称集合。

在导入声明中，导入绑定的顺序不会影响导入结果。

下列规则规定了：当多个绑定应用于同一个名称时，必须使用哪些名称把被绑定实体添加到当前模块的声明作用域中。

.. index::
   import binding
   bound entity
   import directive
   import path
   import declaration
   import
   import outcome
   declaration scope
   scope
   entity
   binding
   module
   name

+-----------------------------+----------------------------+------------------------------+
|   Case                      |   Sample                   |   Rule                       |
+=============================+============================+==============================+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | OK. The compile-time         |
| without an alias in several |      import {sin, sin}     | warning is recommended.      |
| bindings.                   |         from "..."         |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is used explicitly   |                            | OK. No warning.              |
| without alias in one        |     import {sin}           |                              |
| binding.                    |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | OK. Both the name and        |
| without alias, and          |     import {sin}           | qualified name can be used:  |
| implicitly with alias.      |        from "..."          |                              |
|                             |                            | sin and M.sin are            |
|                             |     import * as M          | accessible.                  |
|                             |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | OK. Only alias is accessible |
| with alias.                 |                            | for the name, but not the    |
|                             |     import {sin as Sine}   | original name:               |
|                             |       from "..."           |                              |
|                             |                            | - Sine is accessible;        |
|                             |                            | - sin is not accessible.     |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly        |                            | OK. Both options can be      |
| used with alias, and        |                            | used:                        |
| implicitly with alias.      |     import {sin as Sine}   |                              |
|                             |        from "..."          | - Sine is accessible;        |
|                             |                            |                              |
|                             |     import * as M          | - M.sin is accessible.       |
|                             |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+
|                             | .. code-block:: typescript |                              |
| A name is explicitly used   |                            | OK. Both aliases are         |
| with alias several times.   |                            | accessible. But warning can  |
|                             |     import {sin as Sine,   | be displayed.                |
|                             |        sin as SIN}         |                              |
|                             |        from "..."          |                              |
+-----------------------------+----------------------------+------------------------------+

.. index::
   name
   import
   alias
   access
   binding
   qualified name
   accessibility

|

.. _Top-Level Declarations:

顶层声明
**********************

.. meta:
    frontend_status: Done

*顶层声明* 用于声明顶层类型（``class``、``interface``、``enum``，见 :ref:`Type Declarations`）、顶层变量（见 :ref:`Variable Declarations`）、常量（见 :ref:`Constant Declarations`）、函数（见 :ref:`Function Declarations`）、重载（见 :ref:`Explicit Function Overload`）、命名空间（见 :ref:`Namespace Declarations`），或其他声明（见 :ref:`Ambient Declarations`、:ref:`Annotations`、:ref:`Accessor Declarations`、:ref:`Functions with Receiver`）。
顶层声明可以被导出。

*顶层声明* 的语法如下：

.. code-block:: abnf

    topDeclaration:
        ('export' 'default'?)?
        annotationUsage?
        ( typeDeclaration
        | variableDeclarations
        | constantDeclarations
        | functionDeclaration
        | explicitFunctionOverload
        | namespaceDeclaration
        | ambientDeclaration
        | annotationDeclaration
        | accessorDeclaration
        | functionWithReceiverDeclaration
        )
        ;

.. code-block:: typescript
   :linenos:

    export let x: number[], y: number

.. index::
   top-level declaration
   top-level type
   top-level variable
   class
   interface
   enum
   variable
   constant
   constant declaration
   namespace
   export
   function
   variable declaration
   type declaration
   function declaration
   accessor declaration
   function with receiver
   accessor with receiver
   explicit function overload
   namespace
   namespace declaration
   declaration
   ambient declaration
   annotation
   syntax

注解的用法见 :ref:`Using Annotations`。

|

.. _Exported Declarations:

已导出声明
*********************

.. meta:
    frontend_status: Done

顶层声明可以使用 ``export`` 修饰符，使其能够在其他模块中通过导入方式被访问（见 :ref:`Accessible`）
（见 :ref:`Import Directives`）。对于某个顶层声明，也可以通过导出指令
（见 :ref:`Export Directives`）达到相同效果。未以上述方式导出的声明只能在其声明所在模块内部使用。

.. code-block:: typescript
   :linenos:

    export class Point {}
    export let Origin: Point = new Point(0, 0)
    export function Distance(p1: Point, p2: Point): number {
      // ...
    }

.. index::
   top-level declaration
   exported declaration
   export modifier
   access
   accessible declaration
   declaration
   accessibility
   module
   import directive
   import

如果某个声明以另一个已导出声明的名称被导出，则会发生 :index:`compile-time error`。

当 *导出指令* 使用 *selectiveBindings* 或 *bindingAlias* 为某个声明指定新名字，
而该新名字与另一已导出声明的名字冲突时，就会出现这种情况，如下所示：

.. code-block:: typescript
   :linenos:

    export function foo(): void {}
    function bar(): void {}
    export {bar as foo} // Compile-time error, entity named 'foo' is already exported

使用 :ref:`Re-Export Directive` 时同样会出现该错误：

.. code-block:: typescript
   :linenos:

    // file1.ets
    export class A {}

    // file2.ets
    export class A {}

    // Another file
    export * from "./file1"
    export * from "./file2" // Compile-time error, entity named 'A' is already exported

此外，只允许使用默认导出指令导出一个顶层声明。
这样在导入时就可以不写其声明名（细节见 :ref:`Default Import Binding`）。
如果有多于一个顶层声明被标记为 ``default``，则会发生 :index:`compile-time error`。

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    export default let PI: number = 3.141592653589

.. index::
   top-level declaration
   export
   default export directive
   declaration
   name
   import
   import binding

另一种受支持的 *export default* 形式，是把一个表达式作为默认导出目标。
这种导出实际上意味着：创建一个匿名常量变量，其值等于该表达式的求值结果。
这种导出只能通过为该匿名常量指定一个导入名的方式导入。否则会发生
:index:`compile-time error`。

.. code-block:: typescript
   :linenos:

    // File1
    class A {
      foo () {}
    }
    export default new A

    // File2
    import {default as a} from "File1"

    a.foo()  // Calling method foo() of class A where 'a' is an instance of type A
    a = new A // Compile-time error as 'a' is a constant variable

    // File3
    import * as a from "File1" /* compile-time error, such form of import
                                  cannot be used for the default export */


|LANG| 禁止任何这类已导出声明：其一旦被导入后，会导致模块访问某个未导出的实体。
这一规则同时适用于全局声明与命名空间声明。
导出函数、变量、常量、类的 public/protected 成员、默认接口方法、接口 getter 和 setter 的类型，
在适用时都必须显式给出。

.. note::
   上面“已导出函数、..., 方法, ... 的类型”不仅指返回类型，还包括实体的完整签名，
   其中也包括参数类型等适用部分。比如 setter 没有返回类型
   （甚至不是 ``void``），但它的参数类型仍必须显式导出。

在当前模块中声明，并且被用于某个导出声明的可访问部分中的任何实体，
都必须要么是 |LANG| 直接可用的实体（例如内建类型），要么也必须被导出。
否则会发生 :index:`compile-time error`。

.. note::
   这里“导出声明的可访问部分”指的是一组在其他模块导入该声明后也能被使用到的实体。
   例如，导出类中的 public 和 protected 成员属于这一集合，而 private 成员不属于。

.. note::
   对显式类型和显式导出的上述要求，同样适用于 :ref:`ambient declarations`。

以下情况会违反上述规则，因此触发编译时错误：

- 导出常量、变量，或导出类中的非私有字段，没有显式类型。
  导出函数、导出接口中的方法，或导出类中的非私有方法，没有显式返回类型。
  顶层或命名空间中的导出 getter（见 :ref:`Accessor declarations`）没有显式返回类型；

- 导出函数或 accessor 在签名中使用了未导出实体。
  导出变量或常量把未导出实体作为类型使用。
  注意：把未导出实体用作 :ref:`Optional Parameters` 的默认值是允许的；

- 导出泛型实体将未导出声明用作类型实参、类型参数约束或类型参数默认值
  （即使该类型实参未在其他导出部分中使用也同样不允许）；

- 导出类或接口在 ``extends`` 子句中使用未导出的类或接口，
  或导出类在 ``implements`` 子句中使用未导出的接口。
  public/protected 类字段将未导出实体作为类型使用。
  导出类中的 public/protected 方法在签名中使用未导出实体；

- 导出的类型别名声明使用了未导出的类型；

- 应用于导出声明的注解未被导出，或其本身使用了未导出的类型；

- 导出重载中包含一个或多个未导出实体。


下列例子展示了这些情况：

1. 没有显式类型或显式返回类型的导出声明。

   .. code-block:: typescript
      :linenos:

      // // functions, variables, constants
      export let a: int = 1 // OK
      export let b = 1 // Compile-time error - no explicit type

      export const c: int = 1 // OK
      export const d = 1 // Compile-time error - no explicit type

      export function foo(): void {} // OK

      export function bar() {}  // Compile-time error, no return type

      // // Fields and methods
      export class C {
         // static and instance fields and methods (public and protected)
         x = 1   // Compile-time error, no explicit type for a public field
         protected xx = 1   // Compile-time error, no explicit type for a protected field
         static v = 1 // Compile-time error, no explicit type for public field
         y: int = 1   // OK
         private z = 1 // OK - not a public/protected field

         foo() { return 1 } // Compile-time error, no explicit return type
                          // for a public method

         static bar() {} // Compile-time error, no explicit
                         // return type for a public static method
         static bar_ok(): void {} // OK


         protected i_bar() { return 1 } // Compile-time error, no return
                                      // type for a protected method
         protected bar_ok(): int { return 1 } // OK

         private baz() { return "hello" } // OK, baz() is private
      }

      // // Interfaces
      // OK
      export interface I {
         get_count(): number { return 1; }
         set_count(n: number): void {}
      }

      // Compile-time error, no explicit return types for get_count and set_count
      export interface J {
         get_count() { return 1; }
         set_count(n: number) {}
      }

      // // getters
      let _counter: int = 1  // OK
      let _name = "empty"         // OK
      export get counter() { return _counter } // Compile-time error, no return type
      export get name(): string { return _name } // OK

      export namespace NS {
         get name() { return "Bob" } // OK since not exported
         export get age(): int { return 1 }  // OK
         export get sex() { return "male" } // Compile-time error, no explicit
                                            // return type
      }


2. 导出函数、变量、常量或 accessor 使用了未导出实体。

   .. code-block:: typescript
      :linenos:

      class A { constructor(a: A) {}; };

      // The following declarations cause compile-time errors
      // because 'A' is not exported
      export let v: A
      export function foo (p: A): A { return p; }
      export const x3: A = new A()
      export get val(): A { return new A() }
      export set val(a: A) {}

      export class B { constructor(B: B) {}; };
      class C extends B {};

      // Next is OK, can use an unexported default as an optional parameter
      export function bar(p: B = new C() ) {}

  |


3. 导出泛型使用了未导出实体。

   .. code-block:: typescript
      :linenos:

      class Arg {};

      // OK since T is a type parameter
      export class G<T> {}

      // Compile-time error, type parameter default 'Arg' not exported
      export function foo<T = Arg>(): void {}
      // Compile-time error, type parameter constraint 'Arg' not exported
      export function foo<T extends Arg>(): void {}

  |

4. 导出类或接口在 public/protected 字段类型或 public/protected 方法签名中使用了未导出实体。

   .. code-block:: typescript
      :linenos:

      class C { constructor() {} ; };
      interface I {};

      // // unexported type in extends/implements
      // Compile-time error, 'C' and 'I' must be exported
      export class C1 extends C implements I {}
      // Compile-time error, 'I' must be exported
      export interface I1 extends I {}

      // // unexported entity inside a declaration
      export class C1 {
         // // Compile-time errors due to unexported 'C'
         f: C = new C;
         doIt(): C { return new C(); }
         protected tryMe(): C { return new C(); }

         // // OK, unexported 'C' can be used in private members/fields
         private secret(): C { return new C(); }
      }

  |


5. 导出的类型别名声明引用了未导出类型。

   .. code-block:: typescript
     :linenos:

      class C {};

      // Compile-time error, 'C' must be exported
      export type A = C

   |

6. 应用于导出声明的注解未导出，或其自身使用了未导出类型。

   .. code-block:: typescript
      :linenos:

      type Version = number[];

      // compile time error, ``Version`` not exported
      export @interface deprecated {
                        fromVersion: Version;
                     }

      export @deprecated([1, 1]) function bar() {};

      |

7. 导出重载中包含一个或多个未导出实体：

   .. code-block:: typescript
      :linenos:

      function foo(): void  {};
      export function bar(): void  {};

      // Compile-time error, `foo` not exported
      export overload baz { foo, bar }

   |


.. index::
   exported declaration
   expression
   top-level declaration
   modifier export
   constant variable
   evaluation result
   export
   default target
   export target
   export directive
   accessibility
   declaration
   export
   declared name
   default export directive
   import
   value

|

.. _Export Directives:

导出指令
*****************

.. meta:
    frontend_status: Done

*导出指令* 允许：

- 指定一个选择性导出声明列表，并可选择重命名；
- 指定一个声明名；
- 导出一个类型；或
- 从其他模块再导出声明。

*导出指令* 的语法如下：

.. code-block:: abnf

    exportDirective:
        selectiveExportDirective
        | singleExportDirective
        | exportTypeDirective
        | reExportDirective
        ;

有关已导出声明的限制，可参见 :ref:`Exported declarations` 中的示例说明。

.. index::
   export directive
   export
   declaration
   exported declaration
   renaming
   re-export
   re-exporting declaration
   module
   syntax

.. index::
   default method
   exported interface
   explicit type
   module with ambient declaration
   ambient declaration

|

.. _Selective Export Directive:

选择性导出指令
==========================

.. meta:
    frontend_status: Done

顶层声明可以通过 *选择性导出指令* 成为已导出声明。
该指令显式给出要导出的声明名列表，并允许通过重命名使这些声明以新名字导出。

*选择性导出指令* 的语法如下：

.. code-block:: abnf

    selectiveExportDirective:
        'export' selectiveBindings
        ;

选择性导出指令使用与导入指令相同的 *selective bindings*：

.. code-block:: typescript
   :linenos:

    export { d1, d2 as d3}

上面的指令会以原名导出 ``d1``，并以 ``d3`` 这个名字导出 ``d2``。
在导入该模块的其他模块中，名字 ``d2`` 将不可访问（见 :ref:`Accessible`）。

.. index::
   selective export directive
   selective export
   top-level declaration
   export
   export directive
   declaration
   directive
   renaming
   import directive
   selective binding
   module
   access
   accessibility

|

.. _Single Export Directive:

单项导出指令
=======================

.. meta:
    frontend_status: Partly
    todo: changes in export syntax

*单项导出指令* 允许使用声明自身的名称，或匿名方式，来指定当前模块中要导出的声明。

*单项导出指令* 的语法如下：

.. code-block:: abnf

    singleExportDirective:
        'export'
        ( 'type'? identifier
        | 'default' (expression | identifier)
        | '{' identifier 'as' 'default' '}'
        )
        ;

.. index::
   export directive
   declaration
   export
   declaration name
   module
   syntax

如果存在 ``default``，则当前模块中最多只能有一条这种导出指令。
否则会发生 :index:`compile-time error`。

下例中的指令会按原名导出变量 ``v``：

.. code-block:: typescript
   :linenos:

    export v
    let v: int = 1


下例中的指令会把类 ``A`` 作为默认导出导出：

.. code-block:: typescript
   :linenos:

    class A {}
    export default A
    export {A as default} // such syntax is also acceptable

.. index::
   export directive
   module
   directive
   syntax

下例中的指令会匿名导出一个常量变量：

.. code-block:: typescript
   :linenos:

    class A {}
    export default new A


当 *identifier* 所引用的声明本身是导入来的声明时，*单项导出指令* 会表现为再导出。

.. code-block:: typescript
   :linenos:

    import {v} from "some location"
    export v

.. index::
   export
   directive
   constant variable
   export directive
   re-export
   declaration
   identifier
   import

|

.. _Export Type Directive:

Export Type 指令
=====================

.. meta:
    frontend_status: Done

导出指令可以带有 ``type`` 修饰符，其唯一目的在于获得更好的 |TS| 语法兼容性
（另见 :ref:`Import Type Directive`）。

*export type 指令* 的语法如下：

.. code-block:: abnf

    exportTypeDirective:
        'export' 'type' selectiveBindings
        ;

如果某个绑定引用的不是类型，则会发生 :index:`compile-time error`。

与 *export* 指令相比，|LANG| 不会对通过 *export type* 指令导出的实体执行其他额外语义检查。

.. index::
   export
   declaration
   export type
   export directive
   semantic check
   entity
   directive
   binding
   type
   syntax

|

.. _Re-Export Directive:

再导出指令
===================

.. meta:
    frontend_status: Partly
    todo: syntax was changed

除了导出模块中自己声明的内容之外，也可以再导出其他模块导出的声明。
可以从某个模块再导出某个特定声明，也可以再导出全部声明。
在再导出时，还可以赋予其新名字。
这一行为类似导入，只不过方向相反。

*再导出指令* 的语法如下：

.. code-block:: abnf

    reExportDirective:
        'export'
        ('*' bindingAlias?
        | selectiveBindings
        | '{' 'default' bindingAlias? '}'
        )
        'from' importPath
        ;

.. index::
   export
   module
   declaration
   re-export declaration
   re-export
   re-export directive
   import

``importPath`` 不能指向当前模块所在文件。否则会发生 :index:`compile-time error`。

如果再导出的声明在当前模块作用域中不可区分（见 :ref:`Declarations`），则会发生 :index:`compile-time error`。

再导出的实践示例如下：

.. code-block:: typescript
   :linenos:

    export * from "path_to_the_module" // re-export all exported declarations
    export * as qualifier from "path_to_the_module"
       // re-export all exported declarations with qualification
    export { d1, d2 as d3} from "path_to_the_module"
       // re-export particular declarations some under new name
    export {default} from "path_to_the_module"
       // re-export default declaration from the other module
    export {default as name} from "path_to_the_module"
       // re-export default declaration from the other module under 'name'

.. index::
   import path
   module
   storage
   re-export
   re-exported declaration
   declaration
   scope

|

.. _Top-Level Statements:

顶层语句
********************

.. meta:
    frontend_status: Done

模块可以包含一系列语句，这些语句在逻辑上构成一个统一的语句序列。

*顶层语句* 的语法如下：

.. code-block:: abnf

    topLevelStatements:
        statement*
        ;

.. index::
   top-level statement
   module
   statement
   syntax

模块可以包含任意数量的顶层语句，这些语句在逻辑上会按文本顺序合并成一个序列：

.. code-block:: typescript
   :linenos:

      statements_1
      /* top-declarations except constant and variable declarations */
      statements_2

上述序列等价于：

.. code-block:: typescript
   :linenos:

      /* top-declarations except constant and variable declarations */
      statements_1; statements_2


下例表示了这种情形：

.. index::
   module
   top-level statement
   variable declaration
   constant declaration
   declaration

.. code-block:: typescript
   :linenos:


   // The actual text combination of the statements and declarations
   console.log ("Start of top-level statements")
   type A = number | string
   let a: A = 56
   function foo () {
      console.log (a)
   }
   a = "a string"


   // The logically ordered text - declarations then statements
   type A = number | string
   function foo () {
      console.log (a)
   }
   console.log ("Start of top-level statements")
   let a: A = 56
   a = "a string"

.. index::
   top-level statement
   declaration
   module
   statement

- 如果某个模块被其他模块导入，那么顶层语句的语义就是初始化该被导入模块。
  这意味着，所有顶层语句都只会执行一次，而且是在调用模块中任意其他函数之前，
  或访问该模块任意顶层变量之前执行；
- 如果模块被用作程序，那么顶层语句就用作程序入口点
  （见 :ref:`Program Entry Point`）。
  若顶层语句为空，则程序入口点同样为空且不执行任何操作。
  如果模块中存在 ``main`` 函数，则该函数会在顶层语句执行完成后再执行。

.. index::
   module
   imported module
   semantics
   top-level statement
   initialization
   import
   module
   call
   access
   accessibility
   program entry point
   function

.. code-block:: typescript
   :linenos:

      // Source file A
      { // Block form
        console.log ("A.top-level statements")
      }

      // Source file B
      import * as A from "Source file A "
      function main () {
         console.log ("B.main")
      }

输出如下：

A. Top-level statements,
B. Main.

.. code-block:: typescript
   :linenos:

      // One source file
      console.log ("A.Top-level statements")
      function main () {
         console.log ("B.main")
      }

如果顶层语句中包含 return 语句（:ref:`Expression Statements`），则会发生 :index:`compile-time error`。

顶层语句的执行意味着：除类型声明外，所有语句都会按其在模块中出现的文本顺序依次执行，
直到抛出错误（见 :ref:`Errors`）或执行完最后一条语句。

因此，如果某条顶层语句引用了某个变量或常量，而该变量或常量的声明（见 :ref:`Variable and Constant Declarations`）在文本上位于当前语句之后，
则会发生 :index:`compile-time error`。

.. code-block:: typescript
   :linenos:

      console.log (a, b) // Compile-time error
      let a = 1
      const b = a

关于变量或常量声明有效性的细节，见 :ref:`Validity of Initializer`。

.. index::
   top-level statement
   return statement
   expression statement
   expression
   statement
   type declaration
   module
   error

|

.. _Multifile Module:

多文件模块
****************

*多文件模块* 是由多个源文件组成的模块，这些源文件具有相同的 *模块头*
（见 :ref:`Module Header`）。

如果两个 *模块头*（见 :ref:`Module Header`）拥有相同的 *moduleName*，但其 ``export`` 修饰符不同，
则会发生 :index:`compile-time error`。

*多文件模块* 会把该模块所有文件中的 :ref:`Import Directives`、
:ref:`Top-Level Declarations` 和 :ref:`Export Directives` 组合起来。

*多文件模块* 有如下限制：

- 如果顶层语句（见 :ref:`Top-Level Statements`）位于不同文件中，
  则会发生 :index:`compile-time error`。

正确的 *多文件模块* 示例：

.. code-block:: typescript
   :linenos:

    // file1
    export module "x"
    import {A} from "some module"
    export a

    // file2
    export module "mod1"
    let a = new A()

.. code-block:: typescript
   :linenos:

    // file1
    module "x"
    function foo() {}
    function bar() {}
    namespace NS1 {
        function foo() {}
        function bar() {}
    }

    // file2
    module "x"
    class A {}

错误的 *多文件模块* 示例：

.. code-block:: typescript
   :linenos:

    // file1
    module "y"
    let a = 8
    namespace NS1 {
        let a = 9
    }

    // file2
    module "y"
    let b = 4       // Compile-time error, the top-level statements located in several files
    namespace NS1 {
        let b = 6   // Compile-time error, the top-level statements located in several files
    }

.. index::
    multifile module

|

.. _Standard Library Usage:

标准库使用
**********************

.. meta:
    frontend_status: Done
    todo: now core, containers, math and time are also imported because of stdlib internal dependencies
    todo: fix stdlib and tests, then import only core by default
    todo: add escompat to spec and default

标准库（见 :ref:`Standard Library`）中导出的一组实体，
会在模块作用域及其嵌套作用域中以简单名的形式可访问（见 :ref:`Accessible`），只要这些名字没有被重新定义。
如果在模块作用域中把这些名字用作程序员自定义实体的名字，就会如 :ref:`Declarations` 中所述导致
:index:`compile-time error`。

.. code-block:: typescript
   :linenos:

    console.log("Hello, world!") // OK, 'console' is defined in the standard library

    let console = 5 // Compile-time error

.. index::
   entity
   export
   scope
   name
   accessibility
   access
   simple name
   standard library
   access
   declaration

|

.. _Program Entry Point:

程序入口点
*******************

.. meta:
    frontend_status: Done

模块可以充当程序（应用）。程序执行从 *程序入口点* 开始，它可以有以下两种形式之一：

- 模块中的顶层语句（见 :ref:`Top-Level Statements`）；或者
- 入口函数（见下文）。

.. index::
   module
   top-level statement
   return statement
   execution
   program entry point
   entry point function

模块可以具有以下入口点形式：

- 仅有入口函数（默认是 ``main``，也可以是下文所述的其他函数）；
- 仅有顶层语句（顶层语句中的第一条语句充当入口点）；
- 同时具有顶层语句和入口函数（先执行顶层语句，再调用入口函数）。

.. index::
   module
   entry point
   entry point function
   top-level statement
   statement

入口函数具有以下特征：

- 任意已导出的顶层函数都可以被用作入口函数。入口函数由编译器、执行环境，或二者共同选定；
- 入口函数必须要么没有参数，要么只有一个 ``FixedArray<string>`` 类型的参数，
  该参数用于访问程序命令行参数；
- 入口函数可以是异步函数（见 :ref:`Async Functions`）；
- 如果入口函数不是异步函数，则其返回类型必须是 ``void``（见 :ref:`Type undefined or void`）或 ``int``；
- 如果入口函数是异步函数，则其返回类型必须是 Promise<void> 或 Promise<int>
  （见 :ref:`Concurrency Promise Class`）；
- 入口函数不能被重载；
- 默认情况下，入口函数名为 ``main``。

.. index::
   entry point
   entry point function
   function
   compiler
   execution
   parameter
   string type
   access
   argument
   return type
   void type
   int type
   overloading
   top-level statements
   default

下例展示了合法和不合法的入口点形式：

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    function main() {
      // Option 1: a return type is inferred from the body of main().
      // It will be 'int' if the body has 'return' with the integer expression
      // and 'void' if no return at all in the body
    }

    function main(): void {
      // Option 2: explicit :void - no return in the function body required
    }

    function main(): int {
      // Option 3: explicit :int - return is required
      return 0
    }

    function main(): string { // Compile-time error, incorrect main signature
      return ""
    }

    function main(p: number) { // Compile-time error, incorrect main signature
    }

    async function main() {
      // Option 4: asynchronous entry point function. Its return type is
      // inferred from the body of main(). It will be 'Promise<int>' if the body
      // has 'return' with an expression that has 'Promise<int>' or integer
      // type. It will be 'Promise<void>' if there is no return at all in the
      // body.
    }

    async function main(): Promise<void> {
      // Option 5: asynchronous entry point function, explicit :Promise<void>
      // - no return in the function body required
    }

    async function main(): Promise<int> {
      // Option 6: asynchronous entry point function, explicit :Promise<int>
      // - return is required
    }

    // Option 7: top-level statement is the entry point
    console.log ("Hello, world!")

    // Option 8: top-level exported function
    export function entry(): void {}

    // Option 9: top-level exported function with command-line arguments
    export function entry(cmdLine: FixedArray<string>): void {}

.. index::
   entry point
   entry point function
   command-line argument
   signature
   function body
   inferred type
   integer expression
   function body

|

.. raw:: pdf

   PageBreak
