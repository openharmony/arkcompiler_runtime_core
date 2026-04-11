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


.. _Names, Declarations and Scopes:

名称、声明与作用域
################################################################################

.. meta:
    frontend_status: Done

本章介绍三个彼此相关的概念：

- 名称；
- 声明；
- 作用域。

|LANG| 程序中的每个实体，例如变量、常量、类、类型、函数、方法等，都是通过声明引入的。实体声明定义了该实体的名称。这个名称随后可在程序文本中用于引用该实体。声明会把实体名称绑定到某个 :ref:`Scopes` 上。作用域决定了新实体的可访问性，以及它是通过限定名还是简单名被引用。

.. index::
   variable
   constant
   class
   type
   function
   method
   scope
   accessibility
   declaration
   entity
   simple name
   unqualified name
   qualified name
   name

|

.. _Names:

名称
********************************************************************************

.. meta:
    frontend_status: Done

名称是由一个或多个标识符构成的序列。名称可用于引用任何已声明实体。名称在语法上有两种形式：

- 简单名，由单个标识符组成；
- 限定名，由一串标识符构成，之间以 ``'.'`` 分隔。

这两种情况都由以下语法规则覆盖：

.. code-block:: abnf

    qualifiedName:
      identifier ('.' identifier )*
      ;

在限定名 *N.x* 中，*N* 是简单名，``x`` 是跟在一串以 ``'.'`` 分隔的标识符之后的标识符。*N* 可以表示以下对象：

- 某个模块名（见 :ref:`Module Declarations`），该模块通过 ``import * as N`` 的方式引入（见 :ref:`Bind All with Qualified Access`），此时 ``x`` 表示该模块导出的实体；
- 某个类（见 :ref:`Classes`）或接口类型（见 :ref:`Interfaces`），此时 ``x`` 表示它的静态成员；
- 某个类或接口类型变量，此时 ``x`` 表示它的实例成员。

.. index::
   name
   entity
   simple name
   qualified access
   exported entity
   interface type variable
   interface type
   interface
   class
   static member
   qualified name
   identifier
   reference type
   variable
   field
   method
   token
   separator
   instance member

|

.. _Declarations:

声明
********************************************************************************

.. meta:
    frontend_status: Done

声明会在适当的*声明作用域*中引入一个具名实体（参见 :ref:`Scopes`），例如：

.. index::
   named entity
   declared entity
   declaration scope

- :ref:`命名空间声明 <Namespace Declarations>`；
- :ref:`类型声明 <Type Declarations>`；
- :ref:`变量和常量声明 <Variable and Constant Declarations>`；
- :ref:`函数声明 <Function Declarations>`；
- :ref:`类 <Classes>`；
- :ref:`接口 <Interfaces>`；
- :ref:`枚举 <Enumerations>`；
- :ref:`常量或变量声明 <Constant or Variable Declarations>`；
- :ref:`顶层声明 <Top-Level Declarations>`；
- :ref:`显式重载声明 <Explicit Overload Declarations>`；
- :ref:`注解 <Annotations>`；
- :ref:`ambient 声明 <Ambient Declarations>`；
- :ref:`访问器声明 <Accessor Declarations>`；
- :ref:`带接收者的函数 <Functions with Receiver>`。

同一声明作用域中的每个声明（参见 :ref:`Scopes`）都必须彼此可区分。

所谓“可区分”，是指这些声明具有：

- 不同的名称；或
- 不同的签名（参见 :ref:`Declaration Distinguishable by Signatures`）。

.. index::
   distinguishable declaration
   declaration scope
   name
   signature

可区分声明的示例如下：

.. code-block:: typescript
   :linenos:

    const PI = 3.14
    const pi = 3
    function Pi() {}
    type IP = number[]
    class A {
        static method() {}
        method() {}
        field: number = PI
        static field: number = PI + pi
    }

如果某个声明在名称上不可区分，并且也不是合法重载
（参见 :ref:`Declaration Distinguishable by Signatures`），就会产生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    // Compile-time error, the constant and the function have the same name.
    const PI = 3.14
    function PI() { return 3.14 }

    // Compile-time error, the type and the variable have the same name.
    class Person {}
    let Person: Person

    // Compile-time error, the field and the method have the same name.
    class C {
        counter: number
        counter(): number {
          return this.counter
        }
    }

    /* compile-time error, name of the declaration clashes with the predefined
        type or standard library entity name. */
    let number: number = 1
    let String = true
    function Record () {}
    interface Object {}
    let Array = 42


    /* compile-time error, ambient and non-ambient declarations refer to the
       same entity in a single module
    */
    declare function foo()
    function foo() {}

.. index::
   declaration
   distinguishable functions

|

.. _Declaration Distinguishable by Signatures:

按签名区分的声明
================================================================================

.. meta:
    frontend_status: None

以下这些同名声明，如果其签名不是 :ref:`Overload-Equivalent Signatures`，那么可以通过签名加以区分：

- 同一声明作用域中的同名函数（见 :ref:`Implicit Function Overloading`）；
- 同一个类中的同名方法（见 :ref:`Implicit Method Overloading`）；
- 同一个类中的构造函数。

.. index::
   distinguishable declaration
   signature
   function
   overloading
   constructor

如果两个在不同作用域中声明的同名函数最终在同一作用域内都 :ref:`Accessible`，则会产生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    import {foo, goo as bar} from "some module"

    function foo() {} // Compile-time error, duplicate declaration
    function bar() {} // Compile-time error, duplicate declaration

可通过签名区分的函数示例如下：

.. code-block:: typescript
   :linenos:

      function foo() {}
      function foo(x: number) {}
      function foo(x: number[]) {}
      function foo(x: string) {}

若函数签名重载等价，则会产生 :index:`compile-time error`，例如：

.. code-block:: typescript
   :linenos:

      // Functions have overload-equivalent signatures
      function foo(x: number) {}
      function foo(y: number) {}

      // Functions have overload-equivalent signatures because of type erasure
      function bar(x: Array<number>) {}
      function bar(x: Array<string>) {}

.. index::
   distinguishable function
   function
   signature

|

.. _Scopes:

作用域
********************************************************************************

.. meta:
    frontend_status: Done

不同实体声明会在不同作用域中引入新名称。作用域是程序文本中的一个区域，实体在此区域中被声明，并可在该区域及其他可见区域中使用。以下实体始终只能通过限定名引用：

- 类和接口成员，包括静态成员和实例成员；
- 通过限定导入引入的实体；
- 在命名空间（见 :ref:`Namespace Declarations`）中声明的实体。

其他实体则通过简单名引用。

作用域中的实体是 :ref:`Accessible` 的。

.. index::
   scope
   entity
   entity declaration
   class member
   interface member
   class
   member
   static member
   instance member
   qualified name
   qualified import
   namespace
   namespace declaration
   simple name
   access
   simple name
   unqualified name
   accessible scope
   variable
   constant
   function call
   accessibility

实体的作用域层级取决于其声明所处的上下文：

.. _module-access:

.. meta:
    frontend_status: Partly

- 模块级作用域只适用于 :ref:`Module Declarations`。常量和变量从各自的声明点起到模块末尾都属于可访问状态（见 :ref:`Accessible`）。其他实体则在整个模块级作用域内都可访问。如果被导出，该名称也可以在其他模块中访问。

.. _namespace-access:

.. meta:
    frontend_status: Partly

- 命名空间级作用域只适用于 :ref:`Namespace Declarations`。常量和变量从各自声明点起到命名空间末尾都属于可访问状态（见 :ref:`Accessible`），并且包含所有嵌套命名空间。其他实体则在整个命名空间级作用域内都可访问，同样覆盖嵌套命名空间。如果被导出，则必须带命名空间名限定才能在命名空间外访问。

.. index::
   module level scope
   module
   access
   name
   declaration
   namespace
   namespace level scope

.. _class-access:

.. meta:
    frontend_status: Done

- 在 :ref:`Classes` 内部声明的名称，处于类级作用域。它在类内属于可访问状态（见 :ref:`Accessible`），并且在某些情况下，依据 :ref:`Access Modifiers`，也可在类外或通过派生类访问。

  访问类内部名称时，使用以下限定之一：

  - 关键字 ``this`` 或 ``super``；
  - 类实例表达式，用于实例成员；
  - 类名，用于静态成员。

  从类外访问时，使用以下限定之一：

  - 保存该值的表达式；
  - 对类实例的引用，用于实例成员；
  - 类名，用于静态成员。

  |LANG| 允许静态实体和实例实体使用同一个标识符作为名称。二者可通过上下文区分：类名表示静态实体，而表示实例的表达式表示实例实体。

.. index::
   class level scope
   accessibility
   access modifier
   keyword super
   keyword this
   expression
   value
   method
   name
   access
   modifier
   derived class
   declaration
   class instance
   instance entity
   static entity

.. _interface-access:

.. meta:
    frontend_status: Done

- 在 :ref:`Interfaces` 内部声明的名称，处于接口级作用域。它在接口内部和外部都属于可访问状态（见 :ref:`Accessible`），默认访问级别为 ``public``。

.. index::
   name
   declaration
   class level scope
   interface level scope
   interface
   access

.. _class-or-interface-type-parameter-access:

.. meta:
    frontend_status: Done

- 类或接口声明中的类型参数名，其作用域是整个声明本身，但不包含静态成员声明。

.. index::
   name
   declaration
   static member
   static member declaration
   scope
   type parameter

.. _function-type-parameter-access:

.. meta:
    frontend_status: Done

- 函数声明中的类型参数名，其作用域是整个函数声明，也就是函数类型参数作用域。

.. index::
   parameter name
   function declaration
   function type parameter scope
   scope

.. _function-access:

.. meta:
    frontend_status: Done

- 在函数体或方法体内声明的名称，其作用域从声明点开始，持续到函数体或方法体末尾。这一作用域也适用于函数参数名和方法参数名。

.. index::
   scope
   function body declaration
   method body declaration
   method scope
   function scope
   method parameter name

.. _block-access:

.. meta:
    frontend_status: Done

- 在代码块内部声明的名称，其作用域从声明点开始，持续到该代码块末尾，即块级作用域。

.. index::
   block
   body
   point of declaration
   block scope

.. code-block:: typescript
   :linenos:

    function foo() {
        let x = y // Compile-time error - y is not accessible yet
        let y = 1
    }

两个名称的作用域可以重叠，例如语句嵌套时。如果两个名称的作用域重叠，则：

- 内层声明优先；
- 无法访问外层同名实体。

类、接口和枚举成员只能通过把点运算符 ``'.'`` 作用到实例或类型上来访问，不能以其他方式访问。

.. index::
   name
   scope
   overlap
   nested statement
   innermost declaration
   precedence
   access
   class member
   interface member
   enum member
   instance
   dot operator

|

.. _Accessible:

可访问性
********************************************************************************

.. meta:
    frontend_status: Done

如果某个实体属于当前 :ref:`Scopes`，则认为它是可访问的。这意味着该实体名称可以用于以下目的：

- 类型名可用于声明变量、常量、参数、类字段或接口属性；
- 函数名或方法名可用于调用该函数或方法；
- 变量名可用于读取或修改该变量的值；
- 通过 :ref:`Bind All with Qualified Access` 方式导入得到的模块名，可用于访问该模块导出的实体。

.. index::
   accessible entity
   accessibility
   scope
   function name
   method name
   value
   module
   qualified access
   import
   bind all
   entity
   export
   exported entity

|

.. _Type Declarations:

类型声明
********************************************************************************

.. meta:
    frontend_status: Done

:ref:`Interfaces`、:ref:`Classes`、:ref:`Enumerations` 以及 :ref:`Type Alias Declaration`，统称为类型声明。

类型声明的语法如下：

.. code-block:: abnf

    typeDeclaration:
        classDeclaration
        | interfaceDeclaration
        | enumDeclaration
        | typeAlias
        ;

.. index::
   type declaration
   interface declaration
   class declaration
   enum declaration
   alias
   type alias
   type declaration

|

.. _Type Alias Declaration:

类型别名声明
================================================================================

.. meta:
    frontend_status: Done

类型别名通过以下方式提供更有意义、也更简洁的记法：

- 为匿名类型命名，例如数组类型、函数类型和联合类型；
- 为现有类型提供替代名称。

类型别名的作用域是模块级或命名空间级。当前上下文中的所有类型别名名称都必须遵守 :ref:`Declarations` 一节中的唯一性规则。

.. index::
   type alias
   anonymous type
   array
   declaration
   function
   union type
   scope
   context
   alias
   name

类型别名的语法如下：

.. code-block:: abnf

    typeAlias:
        'type' identifier typeParameters? '=' type
        ;

可以按如下方式为匿名类型提供有意义的名称：

.. code-block:: typescript
   :linenos:

    type Matrix = number[][]
    type Handler = (s: string, no: number) => string
    type Predicate<T> = (x: T) => boolean
    type NullishNumber = number | undefined

如果现有类型名过长，则可以通过类型别名引入更短的新名称，特别是对泛型类型而言：

.. code-block:: typescript
   :linenos:

    type Dictionary = Map<string, string>
    type MapOfString<T> = Map<T, string>

类型别名只是一个新名称，它既不会改变原始类型的语义，也不会引入一个全新的类型。

.. code-block:: typescript
   :linenos:

    type Vector = number[]
    function max(x: Vector): number {
        let m = x[0]
        for (let v of x)
            if (v > m) m = v
        return m
    }

    let x: Vector = [2, 3, 1]
    console.log(max(x)) // output: 3

.. index::
   alias
   anonymous type
   name
   type alias
   name
   generic type

类型别名可以在其右侧类型表达式中递归引用自身。

在 ``type A = something`` 这类定义中，只有以下两种情况允许递归使用 *A*：

- 作为数组元素类型：``type A = A[]``；
- 作为泛型类型实参：``type A = C<A>``。

.. code-block:: typescript
   :linenos:

    type A = A[] // OK, used as element type

    class C<T> { /*body*/}
    type B = C<B> // OK, used as a type argument

    type D = string | Array<D> // OK

其他任何用法，包括无法解析的循环引用，都会产生 :index:`compile-time error`，因为编译器无法获得足够信息来确定该别名：

.. code-block:: typescript
   :linenos:

    type E = E          // Compile-time error
    type F = string | F // Compile-time error
    type C<T> = T
    type A = C<A>       // Compile-time error

.. index::
   type alias
   alias
   recursive reference
   type alias declaration
   array element
   type argument
   generic type
   compiler

同样的规则也适用于泛型类型别名 ``type A<T> = something``：

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    type A<T> = Array<A<T>> // OK, A<T> is used as a type argument
    type A<T> = string | Array<A<T>> // OK

    type A<T> = A<T> // Compile-time error

如果泛型类型别名在使用时没有提供类型实参，也会产生 :index:`compile-time error`：

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    type A<T> = Array<A> // Compile-time error

.. note::
   在类型别名声明右侧使用类型参数 *T* 本身没有限制。如下代码是合法的：

.. code-block:: typescript
   :linenos:

    type NodeValue<T> = T | Array<T> | Array<NodeValue<T>>;

.. index::
   alias
   generic type
   type argument
   type alias
   type parameter

类型别名的某个类型参数可以依赖同一泛型中的另一个类型参数。

.. index::
   type parameter
   generic

.. code-block:: typescript
   :linenos:

    type X<T, S extends T> = T | S

如果类型参数列表中的某个类型参数直接或间接依赖其自身，则会产生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

   type X<T extends T> = T // circular dependency
   type Y<T extends R, R extends T>  = T // circular dependency
   type Z<T extends R, R extends T | undefined> = T // circular dependency

|

.. _Variable and Constant Declarations:

变量和常量声明
********************************************************************************

.. meta:
    frontend_status: Done

.. _Variable Declarations:

变量声明
================================================================================

.. meta:
    frontend_status: Partly
    todo: arrays never have default values
    todo: raise error for non initialized arrays: let x: number[];console.log(x)
    todo: fix grammar change - ident '?' is not allowed, readonly is not here

非 ambient 变量声明会引入一个新变量。变量本质上是一个具名存储位置。声明出的变量在首次使用前必须先获得初始值。这个初始值要么作为声明的一部分给出，要么通过各种初始化形式赋予。

变量声明的语法如下：

.. code-block:: abnf

    variableDeclarations:
        'let' variableDeclarationList
        ;

    variableDeclarationList:
        variableDeclaration (',' variableDeclaration)*
        ;

    variableDeclaration:
        identifier ':' type initializer?
        | identifier initializer
        ;

    initializer:
        '=' expression
        ;

当某个变量通过变量声明引入时，其类型 ``T`` 按如下方式确定：

- 如果声明里有类型注解，则 ``T`` 就是该注解指定的类型。

  - 如果声明同时还带有初始化器，那么初始化器表达式的类型必须满足 :ref:`Assignability with Initializer`，也就是可赋值给 ``T``。

- 如果没有类型注解，则 ``T`` 从初始化器表达式中推断（见 :ref:`Type Inference from Initializer`）。

ambient 变量声明必须符合 :ref:`Ambient Constant or Variable Declarations` 的规则：必须有类型而不能有初始化器，否则会产生 :index:`compile-time error`。

.. index::
   variable declaration
   declaration
   name
   named store location
   variable
   type annotation
   initialization
   initializer expression
   assignability
   inference
   annotation
   inference
   variable declaration
   value
   declaration

.. code-block:: typescript
   :linenos:

    let a: number // OK
    let b = 1 // OK, type 'int' is inferred
    let c: number = 6, d = 1, e = "hello" // OK

    // OK, type of lambda and type of 'f' can be inferred
    let f = (p: number) => b + p
    let x // Compile-time error, either type or initializer

程序中的每个变量在使用前都必须先有初始值：

- 如果变量显式指定了初始化器，那么执行该初始化器表达式就会产生变量的初始值。
- 否则，可能有以下情况：

  + 如果变量类型为 ``T``，且 ``T`` 具有 :ref:`Default Values for Types`，那么该变量会使用默认值初始化。
  + 如果变量没有默认值，那么在变量被使用之前，必须先通过 :ref:`Simple Assignment Operator` 为它赋值。

.. index::
   value
   method parameter
   function parameter
   method
   default value
   assignment operator
   assignment
   function
   initializer
   initialization
   cyclic dependency
   variable
   initializer expression
   non-initialized variable

|

.. _Constant Declarations:

常量声明
================================================================================

.. meta:
    frontend_status: Done

常量声明会引入一个具有强制显式值的具名变量。常量的值不能通过 :ref:`Assignment` 表达式改变。不过，如果常量本身是对象或数组，那么对象字段或数组元素仍然可以被修改。

常量声明的语法如下：

.. code-block:: abnf

    constantDeclarations:
        'const' constantDeclarationList
        ;

    constantDeclarationList:
        constantDeclaration (',' constantDeclaration)*
        ;

    constantDeclaration:
        identifier (':' type)? initializer
        ;

常量声明的类型 ``T`` 按如下方式确定：

- 如果声明中带有类型注解，则初始化器表达式必须满足 :ref:`Assignability with Initializer`，即可赋值给该注解指定的 ``T``；
- 如果没有类型注解，则 ``T`` 从初始化器表达式中推断（见 :ref:`Type Inference from Initializer`）。

.. index::
   constant declaration
   variable
   constant
   value
   assignment expression
   object
   array
   type
   constant declaration
   type annotation
   inferred type
   initializer expression
   type inference

.. code-block:: typescript
   :linenos:

    const a: number = 1 // OK
    const b = 1 // OK, int type is inferred
    const c: number = 1, d = 2, e = "hello" // OK
    const x // Compile-time error, initializer is mandatory
    const y: number // Compile-time error, initializer is mandatory

|

.. _Validity of Initializer:

初始化器的有效性
================================================================================

.. meta:
    frontend_status: None

如果变量或常量的初始化器表达式引用了某个变量或常量，而该变量或常量的声明在文本上位于当前声明之后，则会产生 :index:`compile-time error`。

.. code-block:: typescript
   :linenos:

    const a: number = b // Compile-time error
    let   b = 1
    let   c: number = d // Compile-time error
    const d = 123

|

.. _Assignability with Initializer:

初始化器上的可赋值性
================================================================================

.. meta:
    frontend_status: Done

如果变量或常量声明包含类型注解 T，并且其初始化器表达式的类型为 E，则类型 E 必须可赋值给类型 T
（见 :ref:`Assignability`）。

.. index::
   initializer expression
   annotation
   type
   assignability
   type annotation

|

.. _Type Inference from Initializer:

从初始化器进行类型推断
================================================================================

.. meta:
    frontend_status: Done

当声明中没有显式类型注解时，其类型按如下方式从初始化器表达式推断：

- 在变量声明中，如果初始化器表达式的类型是字面量类型，则会按照 :ref:`Subtyping for Literal Types` 用其超类型替换该字面量类型。如果初始化器表达式的类型是包含字面量类型的联合类型，则会先把每个字面量类型替换为其超类型，然后再进行 :ref:`Union Types Normalization`。
- 否则，声明的类型直接从初始化器表达式推断。
- 对于 :ref:`Ternary Conditional Expressions` ``condition ? expr1 : expr2``：

  - 如果 ``condition`` 能在编译期求值，那么整个表达式的类型会先按 :ref:`Subtyping for Literal Types` 处理，再从 ``expr1`` 或 ``expr2`` 推断，具体取决于 ``condition`` 的真假；
  - 否则，三元表达式的类型是 ``expr1`` 与 ``expr2`` 推断类型组成的规范化联合类型。

如果初始化器表达式的类型无法推断，就会产生 :index:`compile-time error`。

.. index::
   type
   declaration
   annotation
   type inference
   initializer
   subtyping
   supertype
   type inference
   inferred type
   type annotation
   initializer expression
   literal type

.. code-block:: typescript
   :linenos:

    // Get boolean value unknown at compile time
    function cond(): boolean { return Math.random() < 0.5 ? true : false; }

    let a = null                // Type of 'a' is null
    let aa = undefined          // Type of 'aa' is undefined
    let arr = [null, undefined] // Type of 'arr' is (null | undefined)[]

    let b = cond() ? 1 : 2    // Type of 'b' is int

    let c = cond() ? 3 : 3.14 // Type of 'c' is int | double
    let c1 = true ? 3 : 3.14  // Type of 'c1' is int
    let c2 = false ? 3 : 3.14 // Type of 'c1' is double

    let d = cond() ? "one" : "two" // Type of 'd' is string

    let e = cond() ? 1 : "one" // Type of 'e' is int | string
    let e1 = true ? 1 : "one"  // Type of 'e1' is int
    let e2 = false ? 1 : "one" // Type of 'e2' is string

    const bb  = cond() ? 1 : 2     // Type of 'bb' is int

    const cc  = cond() ? 1 : 3.14  // Type of 'cc' is int | double
    const cc1 = true   ? 1 : 3.14  // Type of 'cc1' is int
    const cc2 = false  ? 1 : 3.14  // Type of 'cc2' is double

    const dd  = cond() ? "one" : "two" // Type of 'dd'  is "one" | "two"
    const dd1 = true   ? "one" : "two" // Type of 'dd1' is "one"
    const dd2 = cond() ? "one" : "two" // Type of 'dd2' is "two"

    const ee = cond() ? 1 : "one" // Type of 'ee' is int | "one"

    let f = {name: "aa"} // Compile-time error, type of object literal is not inferred

    let   x1 = 1 // Type of 'x1' is int
    const x2 = 1 // Type of 'x2' is int

    let   s1 = "1" // Type of 's1' is string
    const s2 = "1" // Type of 's2' is "1"

.. note::

    对 :ref:`Ambient Constant or Variable Declarations` 而言，如果存在初始化器，会产生 :index:`compile-time error`。

|

.. _Function Declarations:

函数声明
********************************************************************************

.. meta:
    frontend_status: Done

函数声明在引入具名函数时指定其名称、签名和函数体。函数体是可选的；若存在，其形式为一个 :ref:`Block`。

函数声明的语法如下：

.. code-block:: abnf

    functionDeclaration:
        modifiers? 'function' identifier
        typeParameters? signature block?
        ;

    modifiers:
        'native' | 'async'
        ;

函数必须在顶层声明（见 :ref:`Top-Level Statements`）。

如果某个函数被声明为 :ref:`Generics` 泛型函数，那么其类型参数必须显式给出。

修饰符 ``native`` 表示该函数是原生函数（见 :ref:`Native Functions`）。如果原生函数带有函数体，则会产生 :index:`compile-time error`。

带有修饰符 ``async`` 的函数会在 :ref:`Async Functions` 中讨论。

.. index::
   function declaration
   name
   signature
   named function
   function body
   block
   body
   function call
   native function
   generic function
   type parameter
   top-level statement

|

.. _Signatures:

签名
================================================================================

.. meta:
    frontend_status: Done

签名定义了函数、方法或构造函数的参数以及 :ref:`Return Type`。

签名的语法如下：

.. code-block:: abnf

    signature:
        '(' parameterList? ')' returnType?
        ;

.. index::
   signature
   parameter
   return type
   function
   method
   constructor

|

.. _Parameter List:

参数列表
================================================================================

.. meta:
    frontend_status: Partly
    todo: change parser as grammar rules, are changed - rest can be after optional, annotation for rest

签名可以包含参数列表。参数列表为每个参数指定参数名标识符以及参数类型。每个参数的类型都必须显式给出。如果省略参数列表，则函数或方法不带参数。

参数列表的语法如下：

.. code-block:: abnf

    parameterList:
        parameter (',' parameter)* (',' restParameter)? ','?
        | restParameter ','?
        ;

    parameter:
        annotationUsage? (requiredParameter | optionalParameter)
        ;

    requiredParameter:
        identifier ':' type
        ;
    optionalParameter:
        identifier ':' type '=' expression
        | identifier '?' ':' type
        ;

如果某个参数是必选参数，那么每次函数或方法调用都必须提供与之对应的实参。:ref:`Optional Parameters` 会在专门小节中说明。下面的函数包含必选参数：

.. code-block:: typescript
   :linenos:

    function power(base: number, exponent: number): number {
      return Math.pow(base, exponent)
    }
    power(2, 3) // both arguments are required in the call

多个参数可以是可选参数，因此调用时可以省略对应实参。

如果某个可选参数出现在必选参数之前，会产生 :index:`compile-time error`。

函数或方法的最后一个参数可以是唯一的 :ref:`Rest Parameter`。

.. index::
   signature
   parameter list
   identifier
   parameter name
   type
   function
   method
   method call
   parameter
   rest parameter
   argument
   required parameter
   optional parameter
   prefix readonly

如果参数类型带有前缀 ``readonly``，则该参数会受到 :ref:`Readonly Parameters` 所述的额外限制。

|

.. _Readonly Parameters:

只读参数
================================================================================

.. meta:
    frontend_status: Done

如果参数类型是 ``readonly`` 数组类型或元组类型，那么任何赋值操作、函数调用或方法调用都不能修改该数组或元组的元素。否则会产生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    function foo(array: readonly number[], tuple: readonly [number, string]) {
        let element = array[0] // OK, one can get array element
        array[0] = element // Compile-time error, array is readonly

        element = tuple[0] // OK, one can get tuple element
        tuple[0] = element // Compile-time error, tuple is readonly
    }

对只读参数和只读变量的赋值，都必须遵循 :ref:`Type of Expression` 中的限制。

.. index::
   readonly parameter
   parameter type
   prefix readonly
   array type
   tuple type
   array
   assignment
   conversion

|

.. _Optional Parameters:

可选参数
================================================================================

.. meta:
    frontend_status: Done

可选参数有以下两种形式：

.. code-block:: abnf

    optionalParameter:
        identifier ':' type '=' expression
        | identifier '?' ':' type
        ;

第一种形式包含一个指定默认值的 ``expression``。这种参数称为“带默认值的参数”。在函数类型和调用点上，它都表现为 :ref:`Optional parameters`。如果对应实参为 ``undefined``，也就是参数被省略或显式传入 ``undefined``，则使用默认值：

.. index::
   optional parameter
   expression
   default value
   parameter with default value
   argument
   function call
   method call

.. code-block:: typescript
   :linenos:

    function pair(x: number, y: number = 7)
    {
        console.log(x, y)
    }
    pair(1, 2) // prints: 1 2
    pair(1) // prints: 1 7
    pair(1, undefined) // prints: 1 7

.. note::
   作为实参传入的 ``undefined``，不会改变函数、方法或构造函数体内该参数的类型。

第二种形式是简写形式，``identifier '?' ':' type`` 实际上等价于让该标识符具有 ``T | undefined`` 类型，并带默认值 ``undefined``。

.. index::
   notation
   undefined
   default value
   identifier

例如，下列两个函数的使用方式完全相同：

.. code-block:: typescript
   :linenos:

    function hello1(name: string | undefined = undefined) {}
    function hello2(name?: string) {}

    hello1() // 'name' has 'undefined' value
    hello1("John") // 'name' has a string value
    hello2() // 'name' has 'undefined' value
    hello2("John") // 'name' has a string value

    function foo1 (p?: number) {}
    function foo2 (p: number | undefined = undefined) {}

    foo1()  // 'p' has 'undefined' value
    foo1(5) // 'p' has a numeric value
    foo2()  // 'p' has 'undefined' value
    foo2(5) // 'p' has a numeric value

|

.. _Rest Parameter:

rest 参数
================================================================================

.. meta:
    frontend_status: Done

rest 参数允许函数、方法、构造函数或 lambda 接收任意数量的实参。rest 参数的参数名前带有 ``spread`` 运算符前缀 ``'...'``。

.. note::
   ``spread`` 运算符 ``'...'`` 在 |LANG| 中也会作为 :ref:`Spread Expression` 的前缀使用。因此，rest 参数和 spread expression 在语法上相似，但二者含义不同。

rest 参数的语法如下：

.. code-block:: abnf

    restParameter:
        annotationUsage? '...' identifier ':' type
        ;

在以下情况下，rest 参数会导致 :index:`compile-time error`：

- 它后面还跟着一个非 rest 参数；
- 它的类型既不是数组类型，也不是元组类型，且也不是受数组或元组约束的类型参数。

当实体具有 ``T[]`` 类型的 rest 参数时，它可以接受任意数量且类型满足 :ref:`Assignability`、即可赋值给 ``T`` 的实参：

.. index::
   rest parameter
   function
   method
   constructor
   lambda
   spread operator
   prefix
   parameter name
   syntax
   parameter list
   array type
   tuple type
   assignability
   argument

.. code-block:: typescript
   :linenos:

    function sum(...numbers: number[]): number {
      let res = 0
      for (let n of numbers)
        res += n
      return res
    }

    sum() // returns 0
    sum(1) // returns 1
    sum(1, 2, 3) // returns 6

如果要把 ``T[]`` 类型的数组作为实参传给带 rest 参数的调用，则必须在数组实参前使用前缀 ``'...'`` 的 :ref:`Spread Expression`：

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    function sum(...numbers: number[]): number {
      let res = 0
      for (let n of numbers)
        res += n
      return res
    }

    let x: number[] = [1, 2, 3]
    sum(...x) // spread an array 'x'
       // returns 6

.. index::
   argument
   prefix
   spread operator
   spread expression
   function
   array argument
   array type

若某个实体具有元组类型 ``[T1, ..., Tn]`` 的 rest 参数，则它只能接受 ``n`` 个实参，并且每个实参类型都必须分别满足 :ref:`Assignability`、即可赋值给对应的 ``Ti``：

.. index::
   call
   rest parameter
   tuple type
   type
   argument
   assignability

.. code-block:: typescript
   :linenos:

    function sum(...numbers: [number, number, number]): number {
      return numbers[0] + numbers[1] + numbers[2]
    }

    sum()          // Compile-time error, wrong number of arguments, 0 instead of 3
    sum(1)         // Compile-time error, wrong number of arguments, 1 instead of 3
    sum(1, 2, "a") // Compile-time error, wrong type of the 3rd argument
    sum(1, 2, 3)   // returns 6

声明一个“可选参数 + 元组 rest 参数”的函数在语法上是合法的，但通常没有实际意义；如果在调用时没有显式提供可选参数和 rest 参数，则会产生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    // optional tuple + rest tuple
    function g(opt?: [number, string], ...tail: [number,string]) { // OK
       // ...
    }

    g() // Compile-time error, no rest tuple
    g([1, "str"]) // Compile-time error, no rest tuple
    g([ 1, "str"], 1, "str") // OK

如果要把元组类型 ``[T1, ..., Tn]`` 的实参传给带 rest 参数的调用，同样必须在该元组实参前使用 ``'...'`` 前缀的 :ref:`Spread Expression`：

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    function sum(...numbers: [number, number, number]): number {
      return numbers[0] + numbers[1] + numbers[2]
    }

    let x: [number, number, number] = [1, 2, 3]
    sum(...x) // spread tuple 'x'
       // returns 6

.. index::
   optional parameter
   tuple type
   entity
   argument
   prefix
   spread expression
   function
   rest parameter
   tuple argument
   spread operator

如果要把定长数组 ``FixedArray<T>`` 作为实参传给带 rest 参数的函数或方法，也必须在该定长数组实参前使用 ``'...'`` 的 :ref:`Spread Expression`：

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    function sum(...numbers: Array<number>): number {
      let res = 0
      for (let n of numbers)
        res += n
      return res
    }

    let x: FixedArray<number> = [1, 2, 3]
    sum(...x) // spread an fixed-size array 'x'
       // returns 6

如果某个类型参数受数组或元组类型约束，那么它也可以作为泛型中的 rest 参数类型：

.. code-block:: typescript
   :linenos:

    function sum<T extends Array<number>>(...numbers: T): number {
      let res = 0
      for (let n of numbers)
        res += n
      return res
    }

.. note::
   对 :ref:`Function Call Expression`、:ref:`Method Call Expression` 或 :ref:`Lambda Expressions` 的每次带 rest 参数调用，都意味着会从传入实参中新创建一个数组或元组。

.. code-block:: typescript
   :linenos:

    function rest_array(...array_parameter: number[]) {
       // array_parameter is a new array created from the arguments passed
       array_parameter[0] = 1234
       console.log (array_parameter[0]) // 1234 is the output
    }
    function rest_tuple(...tuple_parameter: [number, string]) {
       // tuple_parameter is a new tuple created from the arguments passed
       tuple_parameter[0] = 1234
       console.log (tuple_parameter[0]) // 1234 is the output
    }

    const array_argument: number[] =  [1,2,3,4]
    const tuple_argument: [number, string] =  [1,"234"]

    console.log (array_argument[0], tuple_argument[0]) // 1 1 is the output

    rest_array (...array_argument)
         // array_argument is spread into a sequence of its elements

    rest_tuple (...tuple_argument)
         // tuple_argument is spread into a sequence of its elements

    console.log (array_argument[0], tuple_argument[0]) // 1 1 is the output

.. index::
   argument
   fixed-size array type
   rest parameter
   prefix
   spread operator
   spread expression
   function
   method
   fixed-size array argument

|

.. _Shadowing by Parameter:

参数遮蔽
================================================================================

.. meta:
    frontend_status: Done

如果某个参数的名称与函数体或方法体内属于可访问状态（见 :ref:`Accessible`）的顶层变量名称相同，那么在该函数或方法体内，参数名会遮蔽顶层变量名：

.. code-block:: typescript
   :linenos:

    let x: number = 1
    function foo (x: string) {
        // 'x' refers to the parameter and has type string
    }
    class SomeClass {
      method (x: boolean) {
        // 'x' refers to the method parameter and has type boolean
      }
    }
    x++ // 'x' refers to the global variable

.. index::
   shadowing
   parameter
   accessibility
   access
   top-level variable
   access
   function body
   method body
   name
   function
   method
   function parameter
   method parameter
   boolean type

|

.. _Return Type:

返回类型
================================================================================

.. meta:
    frontend_status: Done

函数、方法或 lambda 的返回类型定义了该函数、方法或 lambda 执行后的结果类型。在 :ref:`Function Call Expression`、:ref:`Method Call Expression` 和 :ref:`Lambda Expressions` 的执行过程中，它们可以产生任何 :ref:`可赋值 <Assignability>` 给该返回类型的值。

返回类型的语法如下：

.. code-block:: abnf

    returnType:
        ':' (type | 'this')
        ;

如果函数、方法或 lambda 的返回类型既不是 :ref:`Type undefined or void`，也不是包含 ``void`` 或 ``undefined`` 的联合类型，并且其函数体中存在某条执行路径既没有 :ref:`Return Statements` 也没有 :ref:`Throw Statements`，则会产生 :index:`compile-time error`。

如果函数、方法或 lambda 的返回类型是 :ref:`Type never`，但存在某条执行路径上所有语句都能按 :ref:`Normal and Abrupt Statement Execution` 正常执行结束，也会产生 :index:`compile-time error`。

返回类型也支持使用关键字 ``this`` 作为特殊形式，但这种形式只能用于类实例方法（见 :ref:`Methods Returning this`）。

如果函数、方法或 lambda 未显式指定返回类型，则从其函数体按 :ref:`Return Type Inference` 推断；如果它根本没有函数体，则其返回类型为 :ref:`Type undefined or void`。

.. code-block:: typescript
   :linenos:

    function foo1 (): number {}  // Compile-time error, return or throw missing
    let foo2 =  (): number => {} // Compile-time error, return or throw missing

    function foo3 (): undefined {}  // OK, it returns 'undefined' value
    let foo4 =  (): undefined => {} // OK, it returns 'undefined' value

    function foo5 (): never {}  // Compile-time error, no throw or return never type function call
    let foo6 =  (): never => {} // Compile-time error, no throw or return never type function call

    function foo7 (): void {}  // OK, it returns undefined value
    let foo8 =  (): void => {} // OK, it returns undefined value

    function foo9 () {}   // OK, return type is void and return value is 'undefined'
    let foo10 =  () => {} // OK, return type is void and return value is 'undefined'

.. index::
   return type
   function
   method
   lambda
   function call
   function call expression
   method call expression
   lambda expression
   static type
   assignable type
   assignability
   return statement
   syntax
   method body
   type void
   execution path
   return statement
   inferred type
   type inference
   void type
   never type
   this keyword
   type annotation
   class instance method

|

.. raw:: pdf

   PageBreak
