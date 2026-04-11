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


.. _Enumerations:

枚举
################################################################################

.. meta:
    frontend_status: Done

枚举类型 ``enum`` 指定一种独立的用户定义类型，并带有一组具名的只读成员，用来定义其可能的枚举值。

*枚举声明* 的语法如下：

.. code-block:: abnf

    enumDeclaration:
        'const'? 'enum' identifier enumBaseType? '{' enumMemberList? '}'
        ;

    enumBaseType:
        ':' type
        ;

    enumMemberList:
        enumMember (',' enumMember)* ','?
        ;

    enumMember:
        identifier initializer?
        ;

    initializer:
        '=' expression
        ;

.. index::
   enumeration
   type enum
   user-defined type
   named member
   value
   enumeration declaration
   syntax

如果 *枚举声明* 带有前缀关键字 ``const``，则会产生 :index:`compile-time error`。这一限制只是暂时的，``const enum`` 的语义会在未来版本的 |LANG| 中提供。

枚举中的所有成员名都必须唯一，否则会产生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    enum E3 { A = 5, A = 77 }      // Compile-time error
    enum E4 { A = 5, B = 5 }       // OK, values can be the same

为了与 |TS| 兼容，支持空 ``enum`` 这种边界情况。

.. code-block:: typescript
   :linenos:

    enum Empty {} // OK

枚举成员的值可以通过初始化表达式显式设置，也可以隐式设置（参见 :ref:`Initialization of Enumeration Members`）。

.. index::
   conversion
   enumeration type
   numeric type
   string type
   type enumeration
   conversion
   type string
   expression
   enum
   compatibility

访问枚举成员时，除初始化表达式外，都必须使用类型名限定：

.. code-block:: typescript
   :linenos:

    enum Color { 
        Red, 
        Green, 
        Blue, 
        Default = Red // Qualification can be omitted
    }
    let c: Color = Color.Red // Qualification is mandatory

.. index::
   qualification
   access

下面这些运算符可作用于枚举类型操作数：

- 关系运算符（参见 :ref:`Enumeration Relational Operators`）；
- 相等运算符（参见 :ref:`Equality Expressions`）。

隐式转换（参见 :ref:`Enumeration to Numeric Type Conversion`、:ref:`Enumeration to string Type Conversion`）会根据枚举基类型，把枚举值转换为数值类型或 ``string`` 类型：

.. code-block:: typescript
   :linenos:

    enum Color { Red, Green, Blue }
    let x: number = Color.Red + 1 // OK, implicit conversion to 'int'

    enum T { A = "hello", B = "Bye" }

    let s: string = T.A // OK, implicit conversion to 'string'

如果枚举类型被导出，则所有枚举成员也会连同其强制限定访问形式一起导出。

例如，如果 *Color* 被导出，那么 ``Color.Red`` 这样的成员也会与限定名 ``Color`` 一起导出。

.. _Enumeration Base Type:

枚举基类型
********************************************************************************

.. meta:
    frontend_status: Done

每个枚举都有一个 *枚举基类型*。基类型可以显式设置（参见 :ref:`Enumeration with Explicit Base Type`），也可以从初始化器推断，规则如下：

- 如果至少有一个成员没有初始化器，则推断类型为 `int`。
  所有初始化器的类型都必须可赋给 `int`（参见 :ref:`Assignability`），否则会产生 :index:`compile-time error`。
- 如果所有初始化表达式的类型都可赋给 `int`，则推断类型为 `int`。
- 如果所有初始化表达式的类型都可赋给 `string`，则推断类型为 `string`。
- 否则，会产生 :index:`compile-time error`。

这部分会在 :ref:`Initialization of Enumeration Members` 中进一步讨论。

基类型推断示例如下：

.. code-block:: typescript
   :linenos:

    enum E1 { A, B }     // OK, base type is 'int'
    enum E2 { A = 5, B } // OK, base type is 'int'

    enum E3 { A, B = "hello"}     // Compile-time error, base type cannot be inferred
    enum E4 { A = 5, B = "hello"} // Compile-time error, base type cannot be inferred

|

.. _Enumeration with Explicit Base Type:

显式基类型的枚举
********************************************************************************

.. meta:
    frontend_status: Done

*枚举基类型* 可以显式设置，如下所示：

.. code-block:: typescript
   :linenos:

    enum DoubleEnum: double { A = 0.0, B = 1, C = 3.14159 }
    enum ByteEnum: byte { A = 0, B = 1, C = 3 }

在以下情况下会产生 :index:`compile-time error`：

- 显式类型不是数值类型或字符串类型；
- 显式成员初始化器的类型不可赋给基类型（参见 :ref:`Assignability`）；
- 某个枚举成员没有显式初始化器，且基类型不是整数类型（参见 :ref:`Initialization of Enumeration Members`）。

.. index::
   explicit type
   enum member
   integer type
   non-explicit type
   integer value
   assignability
   numeric type
   string type

.. code-block:: typescript
   :linenos:

    enum DoubleEnum: double { A = 0.0, B = 1, C = 3.14159 } // OK
    enum LongEnum: long { A = 0, B = 1, C = 3 } // OK

    enum Wrong1: double { A, B, C } // Compile-time error, must be explicitly initialized
    enum Wrong2: long { A = 1, B = "abc" } // Compile-time error, not assignable to 'long'

|

.. _Initialization of Enumeration Members:

枚举成员的初始化
********************************************************************************

.. meta:
    frontend_status: Done

枚举成员值可以通过初始化表达式显式设置。如果省略初始化表达式，那么 *枚举基类型* 必须是整数类型，否则会产生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    enum E1 { A, B, C }    // OK, base type is 'int'
    enum E2: long { K, L } // OK, base type is 'long'

    enum E3 { A = "a", B }   // Compile-time error, wrong base type
    enum E4: string { A, B } // Compile-time error, base type is not an integer type
    enum E5: double { C, D } // Compile-time error, base type is not an integer type

如果所有成员都省略初始化器，则第一个成员的值为 0，其他成员的值为前一个成员的值加 1。

如果只有部分成员显式设置了值，则按以下规则处理：

- 如果某个初始化器不是常量表达式（参见 :ref:`Constant Expressions`），则其后的所有成员都必须显式初始化，否则会产生 :index:`compile-time error`；
- 第一个且没有显式初始化器的成员取得零值；
- 带显式初始化器的成员使用该显式值；
- 不是第一个且没有显式初始化器的成员，取其前一个常量成员的值加 1。

下面示例中，``Red`` 的值为 0，``Blue`` 的值为 5，``Green`` 的值为 6：

.. code-block:: typescript
   :linenos:

    enum Color { Red, Blue = 5, Green }

.. index::
   int type
   constant
   value

下面示例说明了在非常量初始化器之后必须继续使用显式初始化器：

.. code-block:: typescript
   :linenos:

    function foo() { return 1 }

    enum Wrong {
        A,
        B = foo(),
        C // Compile-time error, must have explicit initializer
    }

|

.. _Enumeration Methods:

枚举方法
********************************************************************************

.. meta:
    frontend_status: Done

可以通过枚举类型的值去索引该枚举类型名，从而得到成员名称：

.. code-block:: typescript
   :linenos:

    enum Color { Red, Green = 10, Blue }
    let c: Color = Color.Green
    console.log(Color[c]) // Prints: Green

如果多个枚举成员具有相同的值，则文本上最后出现的成员具有优先级：

.. code-block:: typescript
   :linenos:

   enum E { One = 1, one = 1, oNe = 1 }
   console.log(E[E.fromValue(1)]) // Prints: oNe
   console.log(E.fromValue(1).getName()) // Prints: oNe

每个枚举类类型都具备以下静态方法：

- ``static values()`` 返回按声明顺序排列的枚举成员数组；
- ``static getValueOf(name: string)`` 返回给定名称对应的枚举成员；若不存在则抛出错误；
- ``static fromValue(value: T)`` 中，``T`` 为枚举基类型。该方法返回具有给定值的枚举成员；若不存在则抛出错误。

.. index::
   enumeration method
   static method
   enumeration type
   enumeration member
   value

.. code-block:: typescript
   :linenos:

      enum Color { Red, Green, Blue = 5 }
      let colors = Color.values()
      // colors[0] is the same as Color.Red

      let red = Color.getValueOf("Red")

      Color.fromValue(5) // OK, returns Color.Blue
      Color.fromValue(6) // Throws runtime error

.. index::
   value
   conversion
   type
   string type
   method

枚举类型实例的方法包括：

- ``toString()``，把枚举成员的值转换为 ``string`` 类型；
- ``valueOf()``，根据枚举基类型返回该枚举成员的数值或 ``string`` 值；
- ``getName()``，返回枚举成员名称。

.. code-block:: typescript
   :linenos:

    enum Color { Red, Green = 10, Blue }
    let c: Color = Color.Green
    console.log(c.toString()) // Prints: 10

.. code-block-meta:

.. code-block:: typescript
   :linenos:

      enum Color { Red, Green = 10, Blue }
      let c: Color = Color.Green
      console.log(c.valueOf()) // Prints 10
      console.log(c.getName()) // Prints Green

.. note::
   ``c.toString()`` 与 ``c.valueOf().toString()`` 返回相同的值。

.. index::
   instance
   method
   enumeration type
   value
   name
   enumeration member

|

.. raw:: pdf

   PageBreak
