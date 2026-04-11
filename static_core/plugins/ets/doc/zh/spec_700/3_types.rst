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


.. _Types:

类型
################################################################################

.. meta:
    frontend_status: Partly

本章介绍“类型”这一概念。它是 |LANG| 以及其他编程语言中的基本概念之一。
下文讨论 |LANG| 中采用的类型分类方式，以及在该语言编写的程序中使用类型的各个方面。

通常，某个实体的类型被定义为该实体（变量）可取的*值*的集合，以及适用于该实体的、该类型所支持的*运算符*的集合。

|LANG| 是一种静态类型语言。这意味着每个已声明实体以及每个表达式的类型在编译期都是已知的。实体的类型要么由开发者显式指定，要么由编译器隐式推断（见 :ref:`Type Inference`）。

|LANG| 内建的类型称为*预定义类型*（见 :ref:`Predefined Types`）。

由开发者引入、声明并定义的类型称为*用户定义类型*。所有*用户定义类型*都必须以 |LANG| 源代码形式给出完整的类型声明。

.. index::
   statically typed language
   expression
   compile time
   type inference
   compiler
   predefined type
   user-defined type
   type declaration
   source code
   value

|LANG| 的类型可概括如下表：


   ========================= =========================
   预定义类型                用户定义类型
   ========================= =========================
   ``byte``, ``short``,      类类型，
   ``int``,  ``long``,       接口类型，
   ``float``, ``double``,    元组类型，
   ``number``,               联合类型，
   ``boolean``, ``char``,    函数类型，

   ``string``,               枚举类型，

   ``bigint``,               类型参数，

   ``Any``, ``Object``,      字符串字面量类型

   ``never``, ``void``,

   ``undefined``, ``null``,
   ``Array<T>`` 或 ``T[]``,
   ``FixedArray<T>``,
   ``ValueArray<T>``
   ========================= =========================

.. note::
   类型 ``number`` 是 ``double`` 的别名。

.. index::
   user-defined type
   predefined type
   class type
   interface type
   array type
   fixed array type
   tuple type
   union type
   literal type
   function type
   type parameter
   enumeration type
   alias

大多数*预定义类型*都带有别名，以提升与 |TS| 的兼容性，如下所示：


+--------------+---------------+
| Primary Name | Alias         |
+==============+===============+
| ``number``   |   ``Number``  |
+--------------+---------------+
| ``byte``     |   ``Byte``    |
+--------------+---------------+
| ``short``    |   ``Short``   |
+--------------+---------------+
| ``int``      |   ``Int``     |
+--------------+---------------+
| ``long``     |   ``Long``    |
+--------------+---------------+
| ``float``    |   ``Float``   |
+--------------+---------------+
| ``double``   |   ``Double``  |
+--------------+---------------+
| ``boolean``  |   ``Boolean`` |
+--------------+---------------+
| ``char``     |   ``Char``    |
+--------------+---------------+
| ``string``   |   ``String``  |
+--------------+---------------+
| ``bigint``   |   ``BigInt``  |
+--------------+---------------+
| ``Object``   |   ``object``  |
+--------------+---------------+

在所有情况下都推荐使用*预定义类型*的主名称。

.. index::
   predefined type
   primary name
   alias
   compatibility

|

.. _Predefined Types:

预定义类型
********************************************************************************

.. meta:
    frontend_status: Done

预定义类型包括：

-  :ref:`值类型 <Value Types>`
-  :ref:`Any 类型 <Type Any>`
-  :ref:`Object 类型 <Type Object>`
-  :ref:`never 类型 <Type never>`
-  :ref:`undefined / void 类型 <Type undefined or void>`
-  :ref:`null 类型 <Type null>`
-  :ref:`string 类型 <Type string>`
-  :ref:`bigint 类型 <Type bigint>`
-  :ref:`数组类型 <Array Types>` （``Array<T>``、``T[]`` 或 ``FixedArray<T>``）
-  :ref:`Function 类型 <Type Function>`

.. index::
   value
   type
   predefined type
   any
   Object
   never
   void
   undefined
   null
   string
   bigint
   array

|

.. _User-Defined Types:

用户定义类型
********************************************************************************

.. meta:
    frontend_status: Done

用户定义类型包括：

-  类类型（见 :ref:`Classes`）
-  接口类型（见 :ref:`Interfaces`）
-  枚举类型（见 :ref:`Enumerations`）
-  :ref:`Function Types`
-  :ref:`Tuple Types`
-  :ref:`Union Types`
-  :ref:`Type Parameters`
-  :ref:`Literal Types`

.. index::
   user-defined type
   class type
   interface type
   enumeration type
   function type
   union type
   type parameter
   literal type

|

.. _Using Types:

类型的使用
********************************************************************************

.. meta:
    frontend_status: Done

源代码可以通过以下方式引用一个类型：

-  对以下对象使用类型引用：

   + :ref:`Named Types`，或
   + 类型别名（见 :ref:`Type Alias Declaration`）；

-  对以下对象使用就地类型声明：

   + :ref:`Array Types`，
   + :ref:`Tuple Types`，
   + :ref:`Function Types`，
   + :ref:`Function Types with Receiver`，
   + :ref:`Keyof Types`，
   + :ref:`Union Types`，或
   + 括号中的类型

.. index::
   named type
   type alias
   in-place type declaration
   type reference
   array type
   function type
   function type with receiver
   union type
   tuple type
   type in parentheses

``type`` 的语法如下：

.. code-block:: abnf

    type:
        annotationUsageWithParentheses?
        ( typeReference
        | 'readonly'? arrayType
        | 'readonly'? tupleType
        | functionType
        | functionTypeWithReceiver
        | unionType
        | keyofType
        | StringLiteral
        )
        | '(' type ')'
        ;

注解的使用方式会在后续 :ref:`Using Annotations` 章节中讨论。

带有前缀 ``readonly`` 的类型会在后续 :ref:`Readonly Array Types` 和 :ref:`Readonly Tuple Types` 小节中讨论。

类型使用方式示例如下：

.. code-block:: typescript
   :linenos:

    let n: number   // Using identifier as a predefined value type name
    let o: Object   // Using identifier as a predefined class type name
    let a: number[] // Using array type
    let t: [number, number] // Using tuple type
    let f: ()=>number      // Using function type
    let u: number|string    // Using union type
    let l: "xyz"            // Using string literal type

    class C { n = 1; s = "aa"}
    let k: keyof C  // Using keyof to build union type


.. let f1: ()=>number      // Using function type
   let f2: <T>(p: T)=>T    // Using generic function type


如果某个类型由数组、函数或联合类型组合而成，就需要使用括号来明确所需的类型结构。没有括号时，用于构造联合类型的符号 ``'|'`` 具有最低优先级，示例如下：

.. index::
   annotation
   prefix readonly
   readonly type
   array type
   tuple type
   identifier
   function type
   union type
   type structure
   construct
   precedence
   parentheses

.. code-block:: typescript
   :linenos:

    // A nullish array with elements of type string:
    let a: string[] |  undefined
    let s: string[] = []

    a = s    // OK
    a = undefined // OK, a is nullish type

    // An array with elements whose types are string or undefined:
    let b1: (string | undefined)[]
    b1 = undefined // Error, b1 is an array and is not nullish type
    b1 = ["aa", undefined] // OK

    // string or array of undefined elements:
    let b2: string | undefined[]
    b2 = undefined // Error, b2 - string or array of undefined - not nullish
    b2 = [undefined, undefined] // OK

    // A function type that returns string or undefined
    let c: () => string | undefined
    c = undefined // error, c is not nullish
    c = (): string | undefined => { return undefined } // OK

    // (A function type that returns string) or undefined
    let d: (() => string) | undefined
    d = undefined // OK, d is nullish
    d = (): string => { return "hi" } // OK


位于类型前面的注解始终需要括号（如上方语法片段所示）。这样做是为了避免注解括号与类型声明中的括号发生歧义：

.. code-block:: typescript
   :linenos:

    let var_name1: @my_annotation() (A|B) // OK
    let var_name2: @my_annotation (A|B)  // Compile-time error

.. index::
   string
   null
   parentheses

|

.. _Named Types:

命名类型
********************************************************************************

.. meta:
    frontend_status: Done

命名类型包括类、接口、枚举、别名、类型参数以及预定义类型（见 :ref:`Predefined Types`，内建数组除外）。其他类型，即数组类型、函数类型和联合类型，默认都是匿名的，除非为其定义了别名。对应的命名类型通过以下形式引入：

-  类声明（见 :ref:`Classes`），
-  接口声明（见 :ref:`Interfaces`），
-  枚举声明（见 :ref:`Enumerations`），
-  类型别名声明（见 :ref:`Type Alias Declaration`），以及
-  类型参数声明（见 :ref:`Type Parameters`）。

带有类型参数的类、接口和类型别名属于泛型类型（见 :ref:`Generics`）。不带类型参数的命名类型属于非泛型类型。

类型引用（见 :ref:`Type References`）通过指定类型名，并在适用时提供用于替换命名类型类型参数的类型实参，来引用命名类型。

.. index::
   named type
   class
   interface
   enumeration
   alias
   type parameter
   predefined type
   function
   array
   union type
   built-in array
   anonymous type
   class declaration
   interface declaration
   enumeration declaration
   type alias declaration
   type parameter declaration
   type reference
   generic type
   non-generic type
   type argument
   type parameter

|

.. _Type References:

类型引用
********************************************************************************

.. meta:
    frontend_status: Done

类型引用通过以下任一方式引用某个类型：

-  简单类型名或限定类型名（见 :ref:`Names`），
-  类型别名（见 :ref:`Type Alias Declaration`）。

当类型引用引用某个泛型类或接口类型时，只有当它构成该泛型的合法实例化时才是有效的。其类型实参既可以显式提供，也可以基于默认值隐式给出。

.. index::
   type reference
   type name
   type parameter
   simple type name
   qualified type name
   identifier
   type alias
   type argument
   interface type
   generic class
   instantiation

``type reference`` 的语法如下：

.. code-block:: abnf

    typeReference:
        qualifiedName typeArguments?
        ;

.. code-block:: typescript
   :linenos:

    let map: Map<string, number> // Map<string, number> is the type reference

    class A<T> {...}
    class C<T> {
       field1: A<T>  // A<T> is a class type reference - class type reference
       field2: A<number> // A<number> is a type reference - class type reference
       foo (p: T) {} // T is a type reference - type parameter
       constructor () { /* some body to init fields */ }
    }

    type MyType<T> = A<T>[]
    let x: MyType<number> = [new A<number>, new A<number>]
      // MyType<number> is a type reference  - alias reference
      // A<number> is a type reference - class type reference

如果某个类型引用是通过类型别名（见 :ref:`Type Alias Declaration`）来引用某个类型，那么编译器在所有涉及类型处理的场景里都可以把该类型别名替换为其对应的非别名类型，而且这种替换可能是递归的。

.. code-block:: typescript
   :linenos:

   type T1 = Object
   type T2 = number
   function foo(t1: T1, t2: T2)  {
       t1 = t2      // Type compatibility test will use Object and number
       t2 = t2 + t2 // Operator validity test will use type number not T2
   }

.. index::
   type reference
   type alias
   non-aliased type
   type
   recursive replacement
   replacement
   compatibility
   Object
   operator validity test

|

.. _Value Types:

值类型
********************************************************************************

.. meta:
    frontend_status: Done

值类型包括预定义整数类型（见 :ref:`Integer Types and Operations`）、浮点类型（见
:ref:`Floating-Point Types and Operations`）、布尔类型（见
:ref:`Type boolean`）以及字符类型（见 :ref:`Type char`）。

这些类型的值不会与其他值共享状态。

.. note::

   :ref:`Unary Expressions` 和 :ref:`Binary Expressions` 中的相关表格，总结了操作数类型的合法组合，以及一元和二元运算的结果类型信息。

.. index::
   value type
   predefined type
   integer type
   floating-point type
   boolean type
   character type
   enumeration
   user-defined type
   enumeration type
   value
   state

|

.. _Numeric Types:

数值类型
================================================================================

.. meta:
    frontend_status: Done

数值类型包括整数类型和浮点类型（见 :ref:`Integer Types and Operations` 和 :ref:`Floating-Point Types and Operations`）。

每一种数值类型都有其各自的值集合。表达式中会通过 :ref:`Widening Numeric Conversions`，把较小类型的值转换为较大类型的值。

数值类型层级定义如下：

- ``byte`` < ``short`` < ``int`` < ``long`` < ``float`` < ``double``，其中最小类型是 ``byte``，最大类型是 ``double``。

类型 ``bigint`` 不属于这一层级。不会发生从数值类型到 ``bigint`` 的隐式转换。要从数值类型的值创建 ``bigint``，必须使用 ``BigInt`` 类（:ref:`Standard Library` 的一部分）的方法。

数值类型由 :ref:`Standard Library` 中的类表示。这意味着数值类型在本质上具有类类型特征，全部都是 ``Object`` 的子类型，因此可以在任何需要类名的场合使用。

.. code-block:: typescript
   :linenos:

    let a_number = new number
    let a_byte = new byte
    let an_integer = new int
    console.log (a_number, a_byte, an_integer)
    // Output is: 0 0 0


.. index::
   integer type
   floating-point type
   numeric type
   value
   double
   float
   long
   int
   short
   byte
   bigint

|

.. _Integer Types and Operations:

整数类型与运算
================================================================================

.. meta:
    frontend_status: Done

+------------+--------------------------------------------------------------------+
| Type       | Corresponding Set of Values                                        |
+============+====================================================================+
| ``byte``   | All signed 8-bit integers (:math:`-2^7` to :math:`2^7-1`)          |
+------------+--------------------------------------------------------------------+
| ``short``  | All signed 16-bit integers (:math:`-2^{15}` to :math:`2^{15}-1`)   |
+------------+--------------------------------------------------------------------+
| ``int``    | All signed 32-bit integers (:math:`-2^{31}` to :math:`2^{31} - 1`) |
+------------+--------------------------------------------------------------------+
| ``long``   | All signed 64-bit integers (:math:`-2^{63}` to :math:`2^{63} - 1`) |
+------------+--------------------------------------------------------------------+
| ``bigint`` | All integers with no limits                                        |
+------------+--------------------------------------------------------------------+

.. note::

   ``bigint`` 严格来说不是数值类型，但它处理的是任意精度整数值（见 :ref:`type bigint`），因此会在相应小节中单独详述。

|LANG| 提供了若干用于处理整数值的运算符，概括如下：

-  产生 ``boolean`` 类型结果的比较运算符：

   + 数值关系运算符 "<"、"<="、">" 和 ">="（见 :ref:`Numeric Relational Operators`）
   + 数值相等运算符 "==" 和 "!="（见 :ref:`Numeric Equality Operators`）

-  幂运算符 ``'**'`` 的一种变体（见 :ref:`Exponentiation Expression`），其结果类型为 ``bigint``；

-  产生 ``int``、``long`` 或 ``bigint`` 类型结果的数值运算符：

   + 一元正号 "+"（见 :ref:`Unary Plus`）与负号 "-"（见 :ref:`Unary Minus`）；
   + 乘法类运算符 "*"、"/" 和 "%"（见 :ref:`Multiplicative Expressions`）；
   + 加法类运算符 "+" 和 "-"（见 :ref:`Additive Expressions`）；
   + 作为前缀或后缀使用的自增运算符 "++"（见 :ref:`Prefix Increment`、:ref:`Postfix Increment`）；
   + 作为前缀或后缀使用的自减运算符 "--"（见 :ref:`Prefix Decrement`、:ref:`Postfix Decrement`）；
   + 有符号和无符号移位运算符 "<<"、">>" 和 ">>>"（见 :ref:`Shift Expressions`）；
   + 按位取反运算符 "~"（见 :ref:`Bitwise Complement`）；
   + 整数按位运算符 "&"、"^" 和 "|"（见 :ref:`Integer Bitwise Operators`）；

-  三元条件运算符 "?:"
   （见 :ref:`Ternary Conditional Expressions`、:ref:`Extended Conditional Expressions`）；
-  字符串拼接运算符 ``'+'``（见 :ref:`String Concatenation`）：如果一个操作数是 ``string``，另一个是整数类型，则先把整数操作数转换成十进制字符串，再生成新的 ``string`` 结果。

.. index::
   byte
   short
   boolean
   int
   long
   bigint
   integer value
   comparison operator
   ternary conditional operator
   numeric relational operator
   numeric equality operator
   equality operator
   numeric operator
   type reference
   type name
   simple type name
   qualified type name
   type alias
   type argument
   interface type
   postfix
   prefix
   unary operator
   additive operator
   multiplicative operator
   increment operator
   numeric relational operator
   numeric equality operator
   decrement operator
   signed shift operator
   unsigned shift operator
   bitwise complement operator
   integer bitwise operator
   conditional operator
   cast operator
   integer value
   numeric type
   string concatenation operator
   operand
   string

如果某个二元整数运算（移位表达式除外）的任一操作数类型为 ``long``，而另一个操作数是更小的类型，则会先使用数值转换（见 :ref:`Widening Numeric Conversions`）把第二个操作数提升为 ``long``。在这种情况下：

-  运算实现使用 64 位精度；
-  数值运算的结果类型为 ``long``。

如果两个操作数都不是 ``long``，且任一操作数的类型不是 ``int``，则会先把该操作数通过数值转换提升为 ``int``。在这种情况下：

-  运算实现使用 32 位精度；
-  数值运算的结果类型为 ``int``。

整数类型与 ``boolean`` 之间不允许相互转换。不过，在某些场景中，整数类型的值可以用作逻辑条件（见 :ref:`Extended Conditional Expressions`）。

整数运算符不会指示溢出或下溢。

如果整数除法运算符（见 :ref:`Division`）``'/'`` 或整数求余运算符（见 :ref:`Remainder`）``'%'`` 的右操作数为零，则整数运算符可能抛出 ``ArithmeticError``。这一情况会在错误处理相关部分（见 :ref:`Error Handling`）中讨论。

.. index::
   constructor
   method
   constant
   operand
   numeric promotion
   predefined numeric types conversion
   numeric type
   widening
   long
   int
   boolean
   integer type
   cast
   operator
   overflow
   underflow
   division operator
   remainder operator
   error
   increment operator
   decrement operator
   additive expression
   error
   integer operator


整数类型的预定义构造器、方法和常量都属于 |LANG| 标准库（见 :ref:`Standard Library`）的一部分。

.. index::
   predefined constructor
   predefined method
   predefined constant
   integer type

|

.. _Floating-Point Types and Operations:

浮点类型与运算
================================================================================

.. meta:
    frontend_status: Done

+----------------+--------------------------------------+
| 类型           | 对应的值集合                         |
+================+======================================+
| ``float``      | 所有 IEEE 754 32 位浮点数            |
+----------------+--------------------------------------+
| ``number``、   | 所有 IEEE 754 64 位浮点数            |
| ``double``     |                                      |
+----------------+--------------------------------------+

.. index::
   IEEE 754
   floating-point number
   floating-point type
   number type

|LANG| 提供了若干用于处理浮点类型值的运算符，概括如下：

-  产生 ``boolean`` 类型结果的比较运算符：

   + 数值关系运算符 "<"、"<="、">" 和 ">="（见 :ref:`Numeric Relational Operators`）；
   + 数值相等运算符 "==" 和 "!="（见 :ref:`Numeric Equality Operators`）；

-  产生 ``float`` 或 ``double`` 类型结果的数值运算符：

   + 一元正号 "+"（见 :ref:`Unary Plus`）与负号 "-"（见 :ref:`Unary Minus`）；
   + 幂运算符 ``'**'`` 的一种变体（见 :ref:`Exponentiation Expression`），其结果类型为 ``double``；
   + 乘法类运算符 "*"、"/" 和 "%"（见 :ref:`Multiplicative Expressions`）；
   + 加法类运算符 "+" 和 "-"（见 :ref:`Additive Expressions`）；
   + 作为前缀或后缀使用的自增运算符 "++"（见 :ref:`Prefix Increment`、:ref:`Postfix Increment`）；
   + 作为前缀或后缀使用的自减运算符 "--"（见 :ref:`Prefix Decrement`、:ref:`Postfix Decrement`）；

-  产生 ``int`` 或 ``long`` 类型结果的数值运算符：

   + 有符号和无符号移位运算符 "<<"、">>" 和 ">>>"（见 :ref:`Shift Expressions`）；
   + 按位取反运算符 "~"（见 :ref:`Bitwise Complement`）；
   + 整数按位运算符 "&"、"^" 和 "|"（见 :ref:`Integer Bitwise Operators`）；

-  三元条件运算符 "?:"
   （见 :ref:`Ternary Conditional Expressions`、:ref:`Extended Conditional Expressions`）；
-  字符串拼接运算符 ``'+'``（见 :ref:`String Concatenation`）：如果一个操作数是 ``string``，另一个是浮点类型，则先把浮点操作数转换成十进制字符串表示（且不丢失信息），再生成新的 ``string`` 结果。

.. index::
   floating-point type
   floating-point number
   operator
   value
   exponentiation operator
   ternary conditional operator
   numeric relational operator
   numeric equality operator
   comparison operator
   boolean type
   numeric operator
   float
   double
   unary operator
   unary plus operator
   unary minus operator
   multiplicative operator
   multiplicative expression
   additive operator
   prefix
   postfix
   increment operator
   decrement operator
   signed shift operator
   shift expression
   unsigned shift operator
   cast operator
   bitwise complement operator
   integer bitwise operator
   conditional operator
   string concatenation operator
   operand
   numeric type
   string
   decimal form
   loss of information
   concatenation

当满足以下条件之一时，某个运算称为浮点运算：

-  ``exponentiation operator`` 的两个操作数都是数值类型；或
-  某个二元运算符至少有一个操作数是浮点类型，即使另一个操作数是整数类型，只要该运算符不是字符串拼接。

数值操作数参与幂运算符时，运算实现始终使用 64 位浮点算术。必要时，操作数会先转换为 ``double``。该数值运算符的结果类型为 ``double``。

对其他浮点运算，适用以下规则：

-  如果数值运算符的至少一个操作数类型为 ``double``，则运算实现使用 64 位浮点算术，结果类型为 ``double``。
-  如果第一个操作数类型为 ``double``，而另一个操作数不是，则先使用数值转换（见 :ref:`Widening Numeric Conversions`）将另一个操作数提升为 ``double``。
-  如果两个操作数都不是 ``double``，则运算实现使用 32 位浮点算术，结果类型为 ``float``。
-  如果第一个操作数类型为 ``float``，而另一个操作数不是，则先使用数值转换将另一个操作数提升为 ``float``。

任意浮点类型（见 :ref:`Numeric Types`）的值都可以与任意数值类型之间进行强制类型转换（见 :ref:`Numeric Casting Conversions`）。

.. index::
   constructor
   method
   constant
   integer
   standard library
   operation
   floating-point operation
   predefined numeric types conversion
   string concatenation
   numeric type
   operand
   implementation
   float
   double
   numeric promotion
   numeric operator
   binary operator
   floating-point type

浮点类型与 ``boolean`` 之间不允许相互转换。不过，在某些场景中，浮点类型的值可以用作逻辑条件（见 :ref:`Extended Conditional Expressions`）。

浮点数上的运算中，除求余运算符（见 :ref:`Remainder`）外，都按 IEEE 754 标准的要求工作。例如，|LANG| 要求支持 IEEE 754 的非规格化浮点数以及渐进下溢。这些特性有助于证明特定数值算法的期望性质。若计算结果是非规格化数，浮点运算不会把它直接“冲刷”为零。

|LANG| 要求浮点算术的行为应当等价于：每个浮点运算符的浮点结果都会按结果精度进行舍入。不精确结果会被舍入到最接近无限精确结果的可表示值。|LANG| 使用“舍入到最近值”原则，这是 IEEE 754 的默认舍入模式；若存在两个同样接近的可表示值，则优先选择最低有效位为零的那个。

.. index::
   cast
   floating-point type
   floating-point number
   boolean type
   numeric type
   numeric types conversion
   widening
   operand
   implementation
   numeric promotion
   remainder operator
   gradual underflow
   underflow
   flush to zero
   round to nearest
   rounding mode
   denormalized number
   nearest value
   IEEE 754

|LANG| 使用“朝零舍入”把浮点值转换为整数值。在这种情况下，其行为等价于截断小数部分并丢弃尾数位。“朝零舍入”的结果，是最接近无限精确结果且绝对值不大于它的那个格式值。

浮点运算发生上溢时会产生带符号的无穷大。

浮点运算发生下溢时会产生非规格化值或带符号的零。

没有数学上确定结果的浮点运算会产生 ``NaN``。

任何带有 ``NaN`` 操作数的数值运算，其结果都是 ``NaN``。

浮点类型的预定义构造器、方法和常量都属于 |LANG| 标准库（见 :ref:`Standard Library`）的一部分。

.. index::
   round toward zero
   conversion
   predefined numeric types conversion
   numeric type
   truncation
   truncated number
   rounding toward zero
   mantissa bit
   denormalized value
   NaN
   numeric operation
   increment operator
   decrement operator
   error
   overflow
   underflow
   signed zero
   signed infinity
   integer
   floating-point operation
   floating-point operator
   floating-point value
   floating-point type
   throw
   predefined constructor
   predefined method
   predefined constant

|

.. _Type boolean:

类型 ``boolean``
================================================================================

.. meta:
    frontend_status: Done

类型 ``boolean`` 表示逻辑值 ``true`` 和 ``false``。

布尔运算符包括：

-  相等运算符（见 :ref:`Equality Expressions`）；
-  逻辑取反运算符 "!"（见 :ref:`Logical Complement`）；
-  逻辑运算符 "&"、"^" 和 "|"（见 :ref:`Boolean Logical Operators`）；
-  条件与运算符 "&&"（见 :ref:`Conditional-And Expression`）与条件或运算符 "||"（见 :ref:`Conditional-Or Expression`）；
-  三元条件运算符 "?:"
   （见 :ref:`Ternary Conditional Expressions`）；
-  字符串拼接运算符 ``'+'``（见 :ref:`String Concatenation`）：它会先把 ``boolean`` 类型的操作数转换成 ``string`` 类型（``true`` 或 ``false``），再生成新的 ``string`` 结果。

类型 ``boolean`` 是 :ref:`Standard Library` 的一部分类类型。这意味着 ``boolean`` 是 ``Object`` 的子类型，因此可以在任何需要类名的场合使用。

.. code-block:: typescript
   :linenos:

    let a_boolean = new boolean
    console.log (a_boolean)
    // Output is: false
    let o: Object = a_boolean // OK

.. index::
   boolean
   Boolean
   relational operator
   complement operator
   logical operator
   conditional-and operator
   conditional-or operator
   ternary conditional operator
   ternary conditional expression
   string concatenation operator
   floating-point expression
   comparison
   conversion
   nonzero value
   concatenation
   string

|

.. _Reference Types:

引用类型
********************************************************************************

.. meta:
    frontend_status: Done

引用类型可以是以下几类之一：

-  类类型（见 :ref:`Type Object` 和 :ref:`Classes`）；
-  接口类型（见 :ref:`Interfaces`）；
-  :ref:`Array Types`；
-  :ref:`Fixed-Size Array Types`；
-  :ref:`Tuple Types`；
-  :ref:`Function Types`；
-  :ref:`Union Types`；
-  :ref:`Literal Types`；
-  :ref:`Type Any`；
-  :ref:`Type string`；
-  :ref:`Type bigint`；
-  :ref:`Type never`；
-  :ref:`Type null`；
-  :ref:`Type undefined or void`；以及
-  :ref:`Type Parameters`。

.. index::
   reference type
   class type
   interface type
   array type
   fixed-size array type
   function type
   union type
   string type
   literal type
   never type
   null type
   undefined type
   void type
   type parameter

本文后续使用术语 ``Object`` 来表示由引用类型变量或常量（见 :ref:`Variable and Constant Declarations`）所指向的任意实例。

多个引用可以指向同一个对象。

对象可以具有状态。类实例的状态存储在其字段中。数组（见 :ref:`Array Types`）或元组（见 :ref:`Tuple Types`）对象的状态存储在其元素中。

如果两个引用类型变量都包含指向同一个对象的引用，并且其中任意一个变量修改了该对象的状态，则这种状态变化对另一个变量也是可见的。

.. index::
   object
   subtype
   state
   array element
   variable
   instance
   reference

|

.. _Type Any:

类型 ``Any``
********************************************************************************

.. meta:
    frontend_status: Partly

类型 ``Any`` 是一种预定义类型，也是所有类型的超类型。``Any`` 还是一种预定义的空值型（见 :ref:`Nullish Types`），也就是说，它尤其是 :ref:`Type undefined or void` 与 :ref:`Type null` 的超类型。

类型 ``Any`` 没有任何方法或字段。

.. _Type Object:

类型 ``Object``
********************************************************************************

.. meta:
    frontend_status: Done

类型 ``Object`` 是一种预定义类类型。除 :ref:`Type undefined or void`、:ref:`Type null`、:ref:`Nullish Types`、:ref:`Type Parameters`，以及包含类型参数的某些 :ref:`Union types` 之外，它是所有类型的超类型（见 :ref:`Subtyping`）。

类型 ``Object`` 是类型 ``Any`` 的子类型（见 :ref:`Type Any`）。``Object`` 的所有子类型都会继承 ``Object`` 类的全部方法（见 :ref:`Inheritance`）；``Object`` 类的方法在 :ref:`Standard Library` 中有完整描述。

本文档示例中使用的 ``toString`` 方法会返回任意对象的字符串表示。

.. index::
   class
   interface
   string type
   bigint type
   array
   union
   function type
   enum type
   method
   interface
   array
   type parameter
   union type
   inheritance
   string

|

.. _Type never:

类型 ``never``
********************************************************************************

.. meta:
    frontend_status: Done

类型 ``never`` 可以赋值给任意类型（见 :ref:`Assignability`）。

类型 ``never`` 没有实例。``never`` 主要用于以下场景：

- 返回类型：用于那些永远不会返回值、而是在完成操作时抛出错误的函数或方法。
- 变量类型：用于那些永远得不到值的变量。不过，左右两侧类型都为 ``never`` 的赋值语句是合法的。
- 参数类型：用于阻止某个函数或方法的函数体被实际执行。

.. code-block:: typescript
   :linenos:

    function foo (): never {
       // function foo never returns
       throw new Error("foo() never returns")
    }

    let x: never = foo() // x never gets a value

    function bar (p: never) {
       // function bar body is never executed
    }

    bar (foo()) // bar is never called as foo() call never returns

.. index::
   never type
   instance
   return type
   method
   error
   throw
   variable
   assignment
   parameter
   function
   return
   value

|

.. _Type undefined or void:

类型 ``undefined`` 或 ``void``
********************************************************************************

.. meta:
    frontend_status: Done

类型 ``undefined`` 只有一个值，即 ``undefined``（见 :ref:`Undefined Literal`）。
类型 ``void`` 是类型 ``undefined`` 的别名，因此它也只有同一个值 ``undefined``。
类型 ``undefined`` 和 ``void`` 可以互换使用。

类型别名 ``void`` 通常用作返回类型，用于强调某个函数、方法或 lambda 可以包含不带表达式的
:ref:`Return Statements`，或者根本没有返回语句：

.. code-block:: typescript
   :linenos:

    function f0 (): void {
        // OK, no return statement
    }

    function f1 (): void {
        return // OK
    }

    function f2 (): void {
        return undefined // OK
    }

    function f3 (): undefined {
        return undefined // OK
    }

    type FunctionWithNoParametersType = () => void

    let funcTypeVariable: FunctionWithNoParametersType = (): void => {}

``undefined`` 和 ``void`` 都可以作为类型实参来实例化泛型类型、函数或方法：

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

   class A<T> {}
   let a1 = new A<undefined>() // OK
   let a2 = new A<void>()      // OK

   function foo<T>(x: T) {}
   foo<undefined>(undefined) // OK
   foo<void>(undefined)      // OK

   foo<void>(void) // Compile-time error, 'void' cannot be used as a value

.. index::
   keyword undefined
   undefined literal
   instantiation
   generic type

|

.. _Type null:

类型 ``null``
********************************************************************************

.. meta:
    frontend_status: Done

类型 "null" 的唯一值是字面量 "null"（见 :ref:`Null Literal`）。

.. note::

   - 类型 ``null`` 主要是为了兼容 |TS|。
   - 在 |TS| 和 |JS| 中，通常认为使用 ``undefined`` 替代 ``null`` 是更好的实践。
   - 如果没有明确要求，通常不建议把 ``null`` 用作类型注解或用在 :ref:`Nullish Types` 中，更推荐使用 ``undefined``。
   - 类型 ``undefined`` 通常比类型 ``null`` 具有更好的性能。

.. index::
   null type
   null literal
   nullish type

|

.. _Type string:

类型 ``string``
********************************************************************************

.. meta:
    frontend_status: Done

类型 ``string`` 的值就是所有字符串字面量，例如 ``abc``。``string`` 使用 Unicode UTF-16 代码单元来存储字符序列。

``string`` 对象是不可变的：对象一旦创建，其值就不能再改变。``string`` 对象的值可以被共享。

类型 ``string`` 具有双重语义：

- 如果被创建、赋值或作为参数传递，``string`` 的行为类似引用类型（见 :ref:`Reference Types`）。
- 对所有适用于 ``string`` 的运算（见 :ref:`String Concatenation`、:ref:`Equality Expressions` 和 :ref:`String Relational Operators`），``string`` 又按照值类型处理。

因此，从引用类型语义来看，``string`` 是一种类类型。对应的类属于 :ref:`Standard Library`，``string`` 也是 ``Object`` 的子类型，因此可以在任何需要类名的场合使用。

此外，``string`` 还是可迭代的（见 :ref:`Iterable Types`），因此可以在任何需要可迭代类型的场合使用。

.. code-block:: typescript
   :linenos:

    let a_string = new string
    console.log (a_string)
    // Output is: <empty_string>
    let o: Object = a_string // OK

.. index::
   type string
   value
   Unicode code unit
   string literal
   literal
   character
   sequence
   string
   object
   dual semantics
   reference type
   expression
   equality
   relational operator

对 ``string`` 值可执行的运算包括：

- 访问属性 ``length`` 会返回 ``int`` 类型的字符串长度值。字符串长度是非负整数，并且在运行时一旦确定后就不能更改。
- 拼接运算符 ``'+'``（见 :ref:`String Concatenation`）会产生 ``string`` 类型的值。如果结果不是常量表达式（见 :ref:`Constant Expressions`），字符串拼接运算符可以隐式创建新的 ``string`` 对象。
- 对字符串值进行索引（见 :ref:`String Indexing Expression`）会返回 ``string`` 类型的值，并且也可能隐式创建新的 ``string`` 对象。

字符串值可以包含任意字符。也就是说，没有任何字符会被用来标识字符串结束。值为 ``\0`` 的字符在字符串内部也只是普通字符，如下例所示：

.. code-block:: typescript
   :linenos:

   console.log("a\0b".length) // output: 3

无论何种情况，都推荐使用 ``string``，尽管名称 ``String`` 也指向同一个类型。

.. index::
   string value
   access
   string type
   string literal
   string object
   string concatenation
   integer
   runtime
   indexing
   character
   reference type
   concatenation operator
   value type

|

.. _Type bigint:

类型 ``bigint``
********************************************************************************

.. meta:
    frontend_status: Done

|LANG| 内建了 ``bigint`` 类型，用于处理理论上任意大的整数。``bigint`` 类型的值可以表示大于 ``long`` 最大值的数字。``bigint`` 采用任意精度算术，并且不属于 :ref:`Numeric Types` 层级。其结果包括：

- ``bigint`` 类型与其他数值类型之间不存在隐式转换。
- 当关系运算符一侧操作数为 ``bigint``、另一侧为其他类型时，该运算非法，并会产生 :index:`compile-time error`。
- 当二元算术表达式一侧操作数为 ``bigint``、另一侧为其他类型时，该表达式非法，并会产生 :index:`compile-time error`。
- ``bigint`` 与非 ``bigint`` 进行相等性比较时，结果总是 ``false``，并会产生编译时告警。

``bigint`` 类型的值可以通过以下方式创建：

- ``bigint`` 字面量（见 :ref:`Bigint Literals`）；或
- 通过调用标准库类 ``BigInt`` 的方法或构造器（见 :ref:`Standard Library`），把数值类型的值转换过来。

与 ``string`` 类似，``bigint`` 也具有双重语义：

- 如果被创建、赋值或作为参数传递，``bigint`` 的行为类似引用类型（见 :ref:`Reference Types`）。
- 对所有适用于 ``bigint`` 的运算来说，``bigint`` 按值类型处理，也就是说，这类值不会与其他值共享状态。

无论何种情况，都推荐使用 ``bigint``，尽管名称 ``BigInt`` 也指向同一个类型。为了兼容 |TS|，可以借助 ``BigInt`` 的静态方法来创建 ``bigint`` 对象。

.. code-block:: typescript
   :linenos:

   let b1: bigint = new BigInt(5) // for Typescript compatibility
   let b2: bigint = 123n

类型 ``bigint`` 是一种类类型，其对应类属于 :ref:`Standard Library` 的一部分。这意味着 ``bigint`` 是 ``Object`` 的子类型，因此可以在任何需要类名的场合使用。

.. code-block:: typescript
   :linenos:

    let a_bigint = new bigint
    console.log (a_bigint)
    // Output is: 0
    let o: Object = a_bigint // OK

.. index::
   bigint type
   built-in type
   arbitrary large integer
   integer
   long type
   bigint literal
   value type
   type annotation
   compatibility
   method
   static method
   numeric type
   value

|

.. _Literal Types:

字面量类型
********************************************************************************

.. meta:
    frontend_status: Partly

字面量类型与部分 |LANG| 字面量（见 :ref:`Literals`）相对应。它们的类型名与其值名，也就是字面量本身，完全相同。|LANG| 只支持以下字面量类型：

- 字符串字面量类型；
- ``null``；以及
- ``undefined``。

.. code-block:: typescript
   :linenos:

    let a: "string literal" = "string literal"
    let b: null = null
    let c: undefined = undefined

    printThem (a, b, c)
    function printThem (p1: "string literal", p2: null, p3: undefined) {
        console.log (p1, p2, p3)
    }

对于 ``null`` 和 ``undefined`` 字面量类型，不存在专门的运算。

.. index::
   literal type
   truncation
   operation
   null type
   undefined type
   type name
   value name
   literal
   string

|

.. _String Literal Types:

字符串字面量类型
================================================================================

.. meta:
    frontend_status: Done

字符串字面量类型变量上的运算，与其超类型 "string"（见 :ref:`Type string`）上的运算完全相同。
运算结果的类型，也与该运算在超类型中的规定一致。

.. code-block:: typescript
   :linenos:

    let s0: "string literal" = "string literal"
    let s1: string = s0 + s0   // + for string returns string

.. index::
   literal type
   string
   variable
   supertype
   subtyping
   operation type

|

.. _Array Types:

数组类型
********************************************************************************

.. meta:
    frontend_status: Partly

数组类型表示一种数据结构，它可以包含任意个非负数量的元素，而这些元素的类型都必须是数组声明中指定类型的子类型。|LANG| 支持以下两种预定义数组类型：

- :ref:`Resizable Array Types`；
- :ref:`Fixed-Size Array Types`，该能力目前属于实验特性。

在大多数场景下，推荐使用可变长数组类型。如果性能是首要要求，则可以使用定长数组类型。

定长数组与可变长数组的区别在于：

- 定长数组的长度只会设置一次，以获得更好的性能。
- 定长数组没有定义任何方法。

任意数组类型都是类类型，其对应类属于 :ref:`Standard Library`。这意味着数组类型是 ``Object`` 的子类型，因此可以在任何需要类名的场合使用。

此外，数组类型也是可迭代的（见 :ref:`Iterable Types`），因此可以在任何需要可迭代类型的场合使用。

.. note::
   本规范中的“数组类型”一词同时适用于可变长数组类型和定长数组类型，“数组值”和“数组实例”也同理。
   可变长数组与定长数组之间不能互相赋值。

.. index::
   array length
   array type
   array value
   array instance
   resizable array type
   fixed-size array

|

.. _Resizable Array Types:

可变长数组类型
================================================================================

.. meta:
    frontend_status: Partly

可变长数组类型是一种内建类型，具有以下特征：

- 任何可变长数组类型对象都包含元素。元素个数称为数组长度，可通过属性 ``length`` 访问。
- 数组长度是非负整数。
- 数组长度可以在运行时设置和修改。
- 数组元素通过索引访问。索引是从 ``0`` 到“数组长度减 1”范围内的整数。
- 通过索引访问元素是常数时间操作。
- 若传递到非 |LANG| 环境中，数组会表示为一段连续内存区域。
- 每个数组元素的类型都必须可赋值（见 :ref:`Assignability`）给数组声明中指定的元素类型。

.. index::
   resizable array type
   built-in type
   access
   array length
   non-negative integer number
   constant-time operation
   array type
   integer
   array element
   element type
   array declaration
   contiguous memory location
   assignability
   array declaration
   memory location
   access
   array

元素类型为 ``T`` 的可变长数组类型有两种语法形式：

- ``T[]``
- ``Array<T>``

第一种形式使用如下语法：

.. code-block:: abnf

    arrayType:
       type '[' ']'
       ;

.. note::
   ``T[]`` 和 ``Array<T>`` 表示完全相同、不可区分的类型（见 :ref:`Type Identity`）。

.. index::
   type identity
   element type
   syntax
   resizable array type
   type identity

对数组元素最基本的两种操作，是通过运算符 ``[]`` 从数组中取出元素，或向数组中放入元素。

同样的语法也可用于某些 :ref:`Indexable Types`，其中一部分属于 :ref:`Standard Library`。

数组中的元素个数可以通过访问属性 ``length`` 获得。数组长度也可以在运行时通过 :ref:`Standard Library` 中定义的方法进行设置和修改。

数组可以通过 :ref:`Array Literal`、:ref:`Resizable Array Creation Expressions`，或 :ref:`Standard Library` 中定义的构造器来创建。

|LANG| 允许为 ``length`` 赋新值来缩短数组，以提供更好的 |TS| 兼容性。以下情况会导致错误：

- 该值是 ``number`` 或其他浮点类型，且其小数部分不为 0；
- 该值小于 0；
- 该值大于原先的长度。

上述情况会导致以下错误结果：

- 如果在运行时，即程序执行期间识别出该情况，则会产生运行时错误；
- 如果在编译期间检测到该情况，则会产生 :index:`compile-time error`。

.. index::
   method
   array length
   array element
   access
   operator
   syntax
   indexable type
   resizable array
   compatibility
   floating-point type
   value
   runtime
   property length
   standard library

下面展示数组操作示例：

.. code-block:: typescript
   :linenos:

    let a : number[] = [0, 0, 0, 0, 0]
      /* allocate array with 5 elements of type number */
    a[1] = 7 /* put 7 as the 2nd element of the array, index of this element is 1 */
    let y = a[4] /* get the last element of array 'a' */
    let count = a.length // get the number of array elements
    a.length = 3 // shrink array
    y = a[2] // OK, 2 is the index of the last element now
    y = a[3] // Runtime error, attempt to access a non-existing array element

    let b: Array<number> = a // 'b' points to the same array as 'a'

类型别名可以为数组类型命名（见 :ref:`Type Alias Declaration`）：

.. code-block:: typescript
   :linenos:

    type Matrix = number[][] /* array of array of numbers */

数组作为对象时，也可以赋值给 ``Object`` 类型变量：

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    let a: number[] = [1, 2, 3]
    let o: Object = a

.. index::
   alias
   array operation
   array element
   access
   type alias
   assignability
   array type
   object
   array
   assignment
   variable

|

.. _Readonly Array Types:

只读数组类型
================================================================================

.. meta:
    frontend_status: Partly

只读数组类型是不可变的，也就是说：

- 只读数组类型变量的长度不能修改；
- 只读数组类型的元素在初始赋值之后，既不能直接修改，也不能通过函数或方法调用间接修改。

否则就会产生 :index:`compile-time error`。

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    let x: readonly number [] = [1, 2, 3]
    x[0] = 42 // Compile-time error, array itself is readonly

元素类型为 ``T`` 的只读数组类型有以下两种语法形式：

- ``readonly T[]``
- ``ReadonlyArray<T>``

这两种形式表示完全相同、不可区分的类型（见 :ref:`Type Identity`）。

.. note::
   对于“数组的数组”，其中所有数组都会是 ``readonly``。

.. index::
   prefix readonly
   readonly array type
   array length
   assignment
   function call
   method call
   syntax
   array
   initial value

|

.. _Tuple Types:

元组类型
********************************************************************************

.. meta:
    frontend_status: Done

元组类型是一种引用类型，它由一组固定的其他类型构成。

元组类型的语法如下：

.. code-block:: abnf

    tupleType:
        '[' (type (',' type)* ','?)? ']'
        ;

元组类型的值，是一组与该元组类型中各组成类型一一对应的值。该组值的数量等于元组类型声明中的类型数量，而声明中类型的顺序则决定了每个位置上对应值的类型。

这意味着元组的每个元素都拥有各自独立的类型。访问元组元素时，使用的也是运算符 ``[]``，其方式与访问数组元素相似。

索引表达式的操作数必须是整数类型的常量表达式（见 :ref:`Constant Expressions`）。第一个元组元素的索引是 ``0``：

.. code-block:: typescript
   :linenos:

   let tuple: [number, number, string, boolean, Object] =
              [     6,      7,  "abc",    true,    42]
   tuple[0] = 42
   console.log (tuple[0], tuple[4]) // `42 42` be printed

元组中的元素个数称为元组长度，可以通过属性 ``length`` 访问：

.. code-block:: typescript
   :linenos:

   let tuple: [number, string]  = [1, "" ]
   console.log(tuple.length) // output: 2

对于 :ref:`Type Tuple`，使用属性 ``length`` 是有意义的。

.. index::
   tuple type
   syntax
   reference type
   assignability
   operator
   object
   class
   reference type
   value
   type declaration
   array element
   index expression
   constant expression
   square bracket
   compatibility
   access

任意元组类型都是 :ref:`Type Tuple` 的子类型。元组之间的子类型关系会在 :ref:`Subtyping for Tuple Types` 中讨论。

|

.. _Readonly Tuple Types:

只读元组类型
================================================================================

.. meta:
    frontend_status: Done

如果某个元组类型带有前缀 ``readonly``，那么它的元素在初始赋值之后，既不能直接修改，也不能通过函数或方法调用进行修改。否则会产生 :index:`compile-time error`，例如：

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    let x: readonly [number, string] = [1, "abc"]
    x[0] = 42 // Compile-time error, tuple itself is readonly

.. index::
   prefix
   readonly
   tuple
   assignment
   tuple type
   initial value
   function call
   method call

|

.. _Type Tuple:

类型 ``Tuple``
================================================================================

.. meta:
    frontend_status: None

类型 ``Tuple`` 是任意元组类型的抽象超类。

.. code-block:: typescript
   :linenos:

    let pair: [number, string] = [1, "abc"]

    let a: Tuple = pair // OK, subtyping

空元组类型与 ``Tuple`` 完全相同：

.. code-block:: typescript
   :linenos:

    let empty: [] = [] // empty tuple with no elements in it

类型 ``Tuple`` 会在 :ref:`Type Erasure` 后保留，并且可用于 :ref:`instanceof Expression` 和 :ref:`Cast Expression`。

不能直接访问 ``Tuple`` 值中的元素。此时可以改用如下签名的方法 ``unsafeGet`` 来获取元素值：

.. code-block:: typescript
   :linenos:

    unsafeGet(index: int): Any

在以下情况下，调用 ``unsafeGet`` 会产生运行时错误：

- ``index`` 小于零；
- ``index`` 大于或等于实际的元组长度。

.. index::
   runtime error

``unsafeGet`` 的用法示例如下：

.. code-block:: typescript
   :linenos:

    function log_1(x: Object) {
        if (x instanceof Tuple) {
            console.log(x.unsafeGet(1))
        }
    }

    let a: [string, string] = ["aa", "bb"]
    log_1(a) // OK, output: 2, "bb"

    let b: [string] = ["aa"]
    log_1(b)     // Runtime error in ``unsafeGet`` call

``Tuple`` 值中的任何元素都不能被修改。|LANG| 不支持此类修改，因为它可能在执行过程中某个不可预测的位置导致运行时错误。

|

.. _Function Types:

函数类型
********************************************************************************

.. meta:
    frontend_status: Done

函数类型可用于表达某个函数期望具有的签名。一个函数类型由以下部分组成：

- 可选的类型参数；
- 参数列表，可以为空；
- 可选的返回类型。

.. index::
   function
   function type
   function signature
   signature
   return type
   parameter list

函数类型的语法如下：

.. code-block:: abnf

    functionType:
        '(' ftParameterList? ')' ftReturnType
        ;

    ftParameterList:
        ftParameter (',' ftParameter)* (',' ftRestParameter)?
        | ftRestParameter
        ;

    ftParameter:
        identifier ('?')? ':' type
        ;

    ftRestParameter:
        '...' ftParameter
        ;

    ftReturnType:
        '=>' type
        ;

``rest`` 参数会在 :ref:`Rest Parameter` 中说明。

.. code-block:: typescript
   :linenos:

    let binaryOp: (x: number, y: number) => number
    function evaluate(f: (x: number, y: number) => number) { }

类型别名（见 :ref:`Type Alias Declaration`）可以为函数类型命名：

.. index::
   alias
   rest parameter
   type alias
   function type
   syntax

.. code-block:: typescript
   :linenos:

    type BinaryOp = (x: number, y: number) => number
    let op: BinaryOp

如果某个函数类型中的参数名带有 ``?`` 标记，那么该参数以及其后所有参数都会成为可选参数。否则就会产生 :index:`compile-time error`。此时，该参数的实际类型是“参数类型”和 ``undefined`` 类型构成的联合类型，并且该参数没有默认值。

.. code-block:: typescript
   :linenos:

    type FuncTypeWithOptionalParameters = (x?: number, y?: string) => void
    let foo: FuncTypeWithOptionalParameters
        = ():void => {}          // OK, arguments are just ignored
    foo = (p: number):void => {} // Compile-time error, call with zero arguments is invalid
    foo = (p?: number):void => {} // OK, call with zero or one argument is valid
    foo = (p1: number, p2?: string):void => {} // Compile-time error, call with zero arguments is invalid
    foo = (p1?: number, p2?: string):void => {} // OK

    foo()
    foo(undefined)
    foo(undefined, undefined)
    foo(42)
    foo(42, undefined)
    foo(42, "a string")

    type IncorrectFuncTypeWithOptionalParameters = (x?: number, y: string) => void
       // Compile-time error, no mandatory parameter can follow an optional parameter

    function bar (
       p1?: number,
       p2:  number|undefined
    ) {
       p1 = p2 // OK
       p2 = p1 // OK
       // Types of p1 and p2 are identical
    }

函数类型可赋值性的更多细节会在 :ref:`Subtyping for Function Types` 中讨论。

.. index::
   function type
   parameter name
   parameter type
   undefined type
   assignability
   context
   conversion
   mandatory parameter
   optional parameter
   subtyping

|

.. _Type Function:

类型 ``Function``
================================================================================

.. meta:
    frontend_status: Done

类型 ``Function`` 是所有函数类型的直接超接口。

``Function`` 类型的值不能被直接调用。开发者必须改用 ``unsafeCall`` 方法。该方法会检查 ``Function`` 类型参数的个数和类型，并在参数合法时调用底层函数值。

.. code-block:: typescript
   :linenos:

   function foo(n: number) {}

   let f: Function = foo

   f(1) // Compile-time error, cannot be called

   f.unsafeCall(3.14) // correct call and execution
   f.unsafeCall() // Runtime error, wrong number of arguments

类型 ``Function`` 的另一个重要属性是 ``name``。它是一个字符串，按以下规则包含与该函数对象关联的名称：

- 如果函数对象对应的是某个函数或方法，则关联名称就是该函数或方法的名称；
- 如果某个 lambda 被赋给 ``Function`` 类型变量，则关联名称就是该变量名；
- 否则，该字符串为空。

.. index::
   function type
   predefined type
   direct superinterface
   value
   method
   argument
   runtime error
   assignment
   function object
   lambda
   string

.. code-block:: typescript
   :linenos:

   function print_name (f: Function) {
      console.log (f.name)
   }

   function foo() {}
   print_name (foo) // Output: "foo"

   class A {
      static sm() {}
      m() {}
   }
   print_name (A.sm)      // Output: "sm"
   print_name (new A().m) // Output: "m"

   let x: Function = (): void => {}
   print_name (x) // Output: "x"

   let y = x
   print_name (y) // Output: "x"

   print_name (():void=>{}) // Output: ""

``unsafeCall`` 方法、``name`` 属性，以及 ``Function`` 类型的其他方法和属性声明，都包含在 :ref:`Standard Library` 中。

.. index::
   property
   method
   Function type

|

.. _Union Types:

联合类型
********************************************************************************

.. meta:
   frontend_status: Partly
   todo: fix grammar - two types are mandatory
   todo: support string literal in union
   todo: implement using common fields and methods, fix related issues

联合类型是一种引用类型，它由其他类型组合而成。

联合类型的语法如下：

.. code-block:: abnf

    unionType:
        type '|' type ('|' type)*
        ;

联合类型的值，必须是构成该联合的任一类型的合法值。

如果联合类型声明右侧的某个类型会导致循环引用，则会产生 :index:`compile-time error`。

如果联合中的某个类型是函数类型（见 :ref:`Function Types`），但没有用括号括起来，也会产生 :index:`compile-time error`。

.. index::
   union type
   reference type
   type declaration
   circular reference
   union
   declaration
   circular reference

联合类型的典型用法如下：

.. code-block:: typescript
   :linenos:

   type OperationResult = "Done" | "Not done"
   function do_action(): OperationResult {
      if (someCondition) {
         return "Done"
      } else {
         return "Not done"
      }
   }

   class Cat {
      // ...
   }
   class Dog {
     // ...
   }
   class Frog {
      // ...
   }
   type Animal = Cat | Dog | Frog | number
   // Cat, Dog, and Frog are some types (class type or interface type)

   let animal: Animal = new Cat()
   animal = new Frog()
   animal = 42
   // One may assign the variable of the union type with any valid value

    enum StringEnum {One = "One", Two = "Two"}

    type Union1 = string | StringEnum // OK, to be reduced during normalization

    type Invalid = string | () => string | number // Compile-time error, function type with no parenthesis around
    type Valid1 = string | (() => string) | number // OK
    type Valid21 = string | (() => string | number) // OK

.. index::
   union type
   class type
   interface type
   value
   normalization

可以通过不同机制，从联合类型中取出某个具体类型的值，例如：

.. code-block:: typescript
   :linenos:

    class Cat { sleep () {}; meow () {} }
    class Dog { sleep () {}; bark () {} }
    class Frog { sleep () {}; leap () {} }

    type Animal = Cat | Dog | Frog

    let animal: Animal = new Cat()
    if (animal instanceof Frog) {
        // animal is of type Frog here, conversion can be used:
        let frog: Frog = animal as Frog
        frog.leap()
    }

    animal.sleep () // Any animal can sleep

.. index::
   type
   value
   union
   conversion

预定义类型的联合示例如下：

.. code-block:: typescript
   :linenos:

    type Predefined = number | boolean
    let p: Predefined = 7
    if (p instanceof number) {
       // type of 'p' is number here
    }

字面量类型的联合示例如下：

.. code-block:: typescript
   :linenos:

    type BMW_ModelCode = "325" | "530" | "735"
    let car_code: BMW_ModelCode = "325"
    if (car_code == "325"){
       car_code = "530"
    } else if (car_code == "530"){
       car_code = "735"
    } else {
       // pension :-)
    }

.. index::
   literal type
   predefined type
   union type
   conversion
   literal value
   value

|

.. _Union Types Normalization:

联合类型归一化
================================================================================

.. meta:
   frontend_status: Partly
   todo: depends on literal types, maybe issues can occur for now

联合类型归一化的目标是在保持类型安全的前提下，尽量减少联合中的类型数量，并在必要时用更一般的类型替换更具体的类型。

联合类型 ``T``:sub:`1` | ... | ``T``:sub:`N`（其中 ``N`` > 1）在形式上可以被化简为 ``U``:sub:`1` | ... | ``U``:sub:`M`（其中 ``M`` <= ``N``），甚至进一步化简为某个非联合类型 *V*。在后一种情况下，*V* 可以是预定义值类型，也可以是字面量类型。

归一化过程按以下顺序逐步执行：

.. index::
   union type
   type safety
   non-union type
   normalized union type
   normalization
   literal type

#. 把所有嵌套联合类型线性化。
#. 把所有类型别名（递归别名除外）递归展开为非别名类型。
#. 联合中的相同类型会合并为单个类型，并考虑 ``readonly`` 类型标志的优先级。
#. 如果联合中包含 ``string``，则移除其中所有字符串字面量类型。
#. 如果联合中包含 ``never``，则移除 ``never``。

上述过程会递归重复执行，直到再也无法继续应用这些步骤为止。

.. index::
   union type
   nested union type
   linearization
   non-nullish type
   never type
   union type
   type alias
   numeric type
   numeric literal type
   readonly
   Any type
   alias
   non-alias
   literal type
   Object type
   subtyping

归一化的结果是一个规范化后的联合类型。示例如下：

.. code-block:: typescript
   :linenos:

    ( T1 | T2) | (T3 | T4) // normalized as T1 | T2 | T3 | T4. Linearization

    type A = A[] | string  // No changes. Recursive type alias is kept

    type B = number
    type C = string
    type D = B | C // normalized as number | string. Type aliases are unfolded

    number | number // normalized as number. Identical types elimination

    (number[]) | (readonly number[]) // normalized as readonly number[]. Readonly version wins

    "1" | string | number // normalized as  string | number. Literal type value belongs to another type values

    class Base {}
    class Derived extends Base {}
    Base | Derived // normalized as Base | Derived (no change)

|LANG| 编译器会在处理联合类型，以及处理数组字面量的类型推断（见 :ref:`Array Type Inference from Types of Elements`）时应用归一化。

.. index::
   normalization
   union type
   normalized union type
   array literal
   type inference
   array literal
   linearization
   string
   readonly

|

.. _Access to Common Union Members:

访问联合类型的公共成员
================================================================================

.. meta:
    frontend_status: Partly

设 ``u`` 是联合类型 ``T``:sub:`1` | ... | ``T``:sub:`N` 的变量。若满足以下条件，|LANG| 支持访问 ``u.m`` 这样的公共成员：

- 每个 ``T``:sub:`i` 都是接口类型或类类型；
- 每个 ``T``:sub:`i` 都有一个名为 ``m`` 的非静态成员；
- 对任意 ``T``:sub:`i` 而言，``m`` 必须满足以下其一：

  - 具有相同签名的方法或访问器；
  - 类型相同的字段。

否则会产生 :index:`compile-time error`。

.. index::
   interface type
   method
   class type
   accessor
   signature
   field

.. code-block:: typescript
   :linenos:

    class A {
        n = 1
        s = "aa"
        foo() {}
        goo(n: number) {}
        static foo () {}
    }
    class B {
        n = 2
        s = 3.14
        foo() {}
        goo() {}
        static foo () {}
    }

    let u: A | B = new A

    let x = u.n // OK, common field
    u.foo() // OK, common method

    console.log(u.s) // Compile-time error, field types differ
    u.goo() // Compile-time error, signatures differ

    type AB = A | B
    AB.foo() // Compile-time error, foo() is a static method

.. index::
   field
   signature
   method

如果在某个 ``T``:sub:`i` 中，名称 ``m`` 是重载名（见 :ref:`Overloading`），也会产生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    class C {
        overload foo { foo1, foo2 }
        foo1(a: number): void {}
        foo2(a: string): void {}
    }
    class D {
        foo(a: number): void {}
        foo2(a: string): void {}
    }

    function test(x: C | D) {
        x.foo() // Compile-time error, 'foo' in C is the explicit overload
        x.foo2("aa") // OK, 'foo2' in both C and D is a method
    }

|

.. _Keyof Types:

``Keyof`` 类型
================================================================================

.. meta:
   frontend_status: Done

``Keyof`` 类型是联合类型的一种特殊形式，它通过关键字 ``keyof`` 构造而成。``keyof`` 作用于类类型（见 :ref:`Classes`）或接口类型（见 :ref:`Interfaces`），得到的新类型是该类或接口的所有可访问（见 :ref:`Accessible`）成员名构成的联合，这些成员名以字符串字面量类型表示。

``keyof`` 类型的语法如下：

.. code-block:: abnf

    keyofType:
        'keyof' typeReference
        ;

.. index::
   keyof type
   union type
   keyof keyword
   interface type
   semantics

如果 ``typeReference`` 既不是类类型也不是接口类型，则会产生 :index:`compile-time error`。示例如下：

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    class A {
       field: number
       method() {}
    }
    type KeysOfA = keyof A // "field" | "method"
    let a_keys: KeysOfA = "field" // OK
    a_keys = "any string different from field or method"
      // Compile-time error, invalid value for the type KeysOfA

如果一个类或接口为空，则它的 ``keyof`` 类型等价于 ``never``：

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class A {} // Empty class
    type KeysOfA = keyof A // never

.. index::
   class
   interface type
   never type
   keyof type

|

.. _Nullish Types:

空值型
********************************************************************************

.. meta:
    frontend_status: Done

|LANG| 支持空值型。它本质上是联合类型（见 :ref:`Union Types`）的一种特定形式。以下联合可用于表示类型 ``T`` 的空值版本：

- ``T | undefined``；
- ``T | null``；或
- ``T | undefined | null``。

.. note::

    在空值型中，推荐优先使用 ``T | undefined``。具体原因可参考 :ref:`Type null` 一节中的说明。

除 :ref:`Type Any` 之外的所有预定义类型，以及所有用户定义类型，都是非空值类型。非空值类型在运行时不能取 ``null`` 或 ``undefined``。

声明为 ``T | null`` 的变量，可以保存类型 ``T`` 及其派生类型的值，也可以保存值 ``null``。

声明为 ``T | undefined`` 的变量，可以保存类型 ``T`` 及其派生类型的值，也可以保存值 ``undefined``。

声明为 ``T | null | undefined`` 的变量，可以保存类型 ``T`` 及其派生类型的值，也可以保存 ``undefined`` 或 ``null``。

空值型是一种引用类型（见 :ref:`Union Types`）。值为 ``null`` 或 ``undefined`` 的引用称为*空值*。

如果某个操作在是否存在空值的情况下都安全，例如把一个空值重新赋给另一个空值，那么它可以直接用于空值型。

.. index::
   union type
   user-defined type
   type declaration
   type inference
   array literal
   nullish type
   non-nullish type
   predefined type declaration
   user-defined type declaration
   undefined value
   runtime
   derived type
   reference type
   nullish value
   nullish-safe option
   null safety
   access
   assignment
   re-assignment

处理空值型 ``T`` 时，可使用以下空值安全选项：

- 使用安全操作：

   - 安全方法调用（见 :ref:`Method Call Expression`）；
   - 安全字段访问表达式（见 :ref:`Field Access Expression`）；
   - 安全索引表达式（见 :ref:`Indexing Expressions`）；
   - 安全函数调用（见 :ref:`Function Call Expression`）；

- 将 ``T | null`` 或 ``T | undefined`` 转换为 ``T``：

   - :ref:`Cast Expression`；
   - ensure-not-nullish 表达式（见 :ref:`Ensure-Not-Nullish Expressions`）；

- 当存在空值时提供一个替代值：

   - 空值合并表达式（见 :ref:`Nullish-Coalescing Expression`）。

.. note::
   空值型与 ``Object`` 类型不兼容：

.. code-block:: typescript
   :linenos:

   function nullish (
      o: Object, nullish1: null, nullish2: undefined, nullish3: null|undefined,
      nullish4: AnyClassOrInterfaceType|null|undefined
   ) {
      o = nullish1 /* compile-time error, type 'null' is not compatible with
                      Object */
      o = nullish2 /* compile-time error, type 'undefined' is not compatible
                      with Object */
      o = nullish3 /* compile-time error, type 'null|undefined' is not
                      compatible with Object */
      o = nullish4 /* compile-time error, type
                      'AnyClassOrInterfaceType|null|undefined' is not
                      compatible with Object */
   }

.. index::
   method call
   field access expression
   indexing expression
   function call
   cast expression
   ensure-not-nullish expression
   nullish-coalescing expression
   nullish-safe option
   nullish value
   nullish type
   safe operation
   safe method call
   safe field access
   safe indexing expression
   safe function call
   conversion
   compatibility

|

.. _Utility Types:

工具类型
********************************************************************************

.. meta:
    frontend_status: Done

|LANG| 支持若干内建类型，称为工具类型。工具类型可以通过调整初始类型的属性来构造新类型，其记法与泛型相同。如果初始类型是类或接口，那么构造出的工具类型也按类或接口类型处理。

所有工具类型名称都可以在任意模块及其所有作用域中作为简单名称访问（见 :ref:`Accessible`）。若把这些名字拿来定义用户自定义实体，会产生 :index:`compile-time error`（参见 :ref:`Declarations`）。下面按字母顺序给出工具类型列表。

.. index::
   built-in type
   class
   interface
   accessibility
   module
   user-defined entity
   declaration
   utility type

|

.. _Awaited Utility Type:

Awaited 工具类型
================================================================================

.. meta:
    frontend_status: None

类型 ``Awaited<T>`` 用于构造一个不再包含 ``Promise`` 的类型。它类似于 ``async`` 函数中的 ``await``，或者 ``Promise`` 上的 ``.then()`` 方法。类型中的 ``Promise`` 会被递归移除，直到遇到泛型、函数、数组或元组类型为止。如果 ``T`` 的声明中根本不包含 ``Promise``，那么 ``Awaited<T>`` 会保持 ``T`` 不变。

如果 ``Awaited<T>`` 中的 ``T`` 是类型参数，那么 ``Awaited<T>`` 的子类型关系由 ``T`` 的子类型关系决定。也就是说，若 ``T`` 是 ``U`` 的子类型，则 ``Awaited<T>`` 也是 ``Awaited<U>`` 的子类型。示例如下：

.. code-block:: typescript
   :linenos:

    type A = Awaited<Promise<string>>           // type A is string

    type B = Awaited<Promise<Promise<number>>>  // type B is number

    type C = Awaited<boolean | Promise<number>> // type C is boolean | number

    type D = Awaited <Object>                   // type D is Object

    type E = Awaited<Promise<Promise<number>|Promise<string>|Promise<boolean>>>
                                                // type E is number|string|boolean

    type F = Awaited<Promise<(p: Promise<string>) => Promise<number>>>
                                                // type F is (p: Promise<string>) => Promise<number>>

    type G = Awaited<Promise<Array<Promise<number>>>>
                                                // type F is Array<Promise<number>>

    function foo <T extends SuperType> (p: Awaited<T>) {}
    function bar <T extends SubType> (p: Awaited<T>) {
        foo (p) // is a valid call as Awaited<T extends SubType> <: Awaited<T extends SuperType>
    }

.. index::
   utility type
   awaited
   promise
   async function
   method

|

.. _NonNullable Utility Type:

NonNullable 工具类型
================================================================================

.. meta:
    frontend_status: None

类型 "NonNullable<T>" 通过排除 "null"（见 :ref:`Type null`）和 "undefined"
（见 :ref:`Type undefined or void`）来构造一个新类型。它可以形式化写成（见 :ref:`Difference Types`）：

``NonNullable<T> = T - null - undefined``。

这意味着：

- 如果类型 ``T`` 中本就既不包含 ``null`` 也不包含 ``undefined``，那么 ``NonNullable<T>`` 会保持 ``T`` 不变；
- ``NonNullable<null>``、``NonNullable<undefined>``，以及对 ``null`` 与 ``undefined`` 联合类型应用 ``NonNullable<>`` 的结果都是 ``never``；
- ``NonNullable<Any> = Any - null - undefined``。

``NonNullable<T>`` 的用法示例如下：

.. code-block:: typescript
   :linenos:

    type X = Object | null | undefined
    type Y = NonNullable<X> // Type of 'Y' is Object

    class A <T> {
      field: NonNullable<T> // This is a non-nullable version of the type parameter
      constructor (field: NonNullable<T>) {
        this.field = field
      }
    }

    const a = new A<Object|undefined> (new Object)
    a.field // Type of field is Object

.. index::
   utility type
   null type
   undefined type
   field

|

.. _Partial Utility Type:

Partial 工具类型
================================================================================

.. meta:
    frontend_status: Done

类型 ``Partial<T>`` 会构造一个新类型，使 ``T`` 的所有字段（见 :ref:`Field Declarations`）以及以字段形式表示的属性（见 :ref:`Interface Properties`）都变成可选。``T`` 必须是类类型或接口类型，否则会产生 :index:`compile-time error`。``T`` 的任何方法都不会成为 ``Partial<T>`` 的一部分，getter 和 setter 也不例外。示例如下：

.. code-block:: typescript
   :linenos:

    interface Issue {
        title: string
        description: string
    }

    function process(issue: Partial<Issue>) {
        if (issue.title != undefined) {
            /* process title */
        }
    }

    process({title: "aa"}) // description is undefined

上例中的 ``Partial<Issue>`` 会被转换为一个不同但结构类似的类型：

.. code-block:: typescript
   :linenos:

    interface /*some name*/ {
        title?: string
        description?: string
    }

.. index::
   type
   property
   optional property
   class type
   interface type
   method
   getter
   setter
   distinct type

类型 ``T`` 不可赋值（见 :ref:`Assignability`）给 ``Partial<T>``，``T`` 与 ``Partial<T>`` 之间也不存在子类型关系（见 :ref:`Subtyping`）。``Partial<T>`` 类型的变量应当使用合法的对象字面量进行初始化。

如果 ``U`` 是 ``T`` 的子类型，那么 ``Partial<U>`` 也是 ``Partial<T>`` 的子类型。

.. note::
   如果类 ``T`` 定义了自定义 getter、setter 或两者兼有，那么在 ``Partial<T>`` 变量上使用对象字面量时，这些成员都不会被调用。对象字面量拥有其自身的内建 getter 和 setter 来修改其变量，示例如下：

.. code-block:: typescript
   :linenos:

    interface I {
        property: number
    }
    class A implements I {
        _property: number
        set property(property: number) {
            console.log ("Setter called")
            this._property = property
        }
        get property(): number {
            console.log ("Getter called");
            return this._property
        }
    }

    function foo (partial: Partial<A>) {
        partial.property = 42 // Setter to be called
        console.log(partial.property) // Getter to be called
    }

    foo ({property: 1}) // No getter or setter from class A is called
    // 42 is printed as object literal has its own setter and getter

.. index::
   type
   assignability
   assignable type
   variable
   initialization
   object literal
   class
   user-defined getter
   built-in getter
   getter
   setter
   user-defined setter
   built-in setter
   property

|

.. _Required Utility Type:

Required 工具类型
================================================================================

.. meta:
    frontend_status: Done

类型 ``Required<T>`` 与 ``Partial<T>`` 相反，它会构造一个新类型，使 ``T`` 的所有字段（见 :ref:`Field Declarations`）以及以字段形式表示的属性（见 :ref:`Interface Properties`）都变为必选，也就是不再可选。``T`` 必须是类类型或接口类型，否则会产生 :index:`compile-time error`。``T`` 的任何方法都不会成为 ``Required<T>`` 的一部分，getter 和 setter 也不例外。示例如下：

.. code-block:: typescript
   :linenos:

    interface Issue {
        title?: string
        description?: string
    }

    let c: Required<Issue> = { // Compile-time error, 'description' should be defined
        title: "aa"
    }

上例中的 ``Required<Issue>`` 会被转换为一个不同但结构类似的类型：

.. code-block:: typescript
   :linenos:

    interface /*some name*/ {
        title: string
        description: string
    }

类型 ``T`` 不可赋值（见 :ref:`Assignability`）给 ``Required<T>``，``T`` 与 ``Required<T>`` 之间也不存在子类型关系（见 :ref:`Subtyping`）。``Required<T>`` 类型的变量应当使用合法的对象字面量进行初始化。

如果 ``U`` 是 ``T`` 的子类型，那么 ``Required<U>`` 也是 ``Required<T>`` 的子类型。

.. index::
   type
   interface type
   utility type
   assignability
   assignable type
   property
   required property
   method
   getter
   setter
   type
   object literal
   class type
   interface type
   distinct type
   initialization
   variable

|

.. _Readonly Utility Type:

Readonly 工具类型
================================================================================

.. meta:
    frontend_status: Done

类型 ``Readonly<T>`` 会构造一个新类型，使 ``T`` 的所有字段（见 :ref:`Field Declarations`）以及以字段形式表示的属性（见 :ref:`Interface Properties`）都带上 ``readonly``。这意味着该构造类型中的这些字段和属性都不能被重新赋值。``T`` 必须是类类型或接口类型，否则会产生 :index:`compile-time error`。``T`` 的任何方法都不会成为 ``Readonly<T>`` 的一部分，getter 和 setter 也不例外。示例如下：

.. code-block:: typescript
   :linenos:

    interface Issue {
        title: string
    }

    const myIssue: Readonly<Issue> = {
        title: "One"
    };

    myIssue.title = "Two" // Compile-time error, readonly property

.. index::
   type
   readonly type
   utility type
   type readonly
   constructed value
   method
   reassignment
   assignability
   assignable type
   property
   interface type
   getter
   setter

类型 ``T`` 可以赋值（见 :ref:`Assignability`）给 ``Readonly<T>``，因此允许如下赋值：

.. code-block:: typescript
   :linenos:

    class A {
       f1: string = ""
       f2: number = 1
       f3: boolean = true
    }
    let x = new A
    let y: Readonly<A> = x // OK

|

类型 ``T`` 是 ``Readonly<T>`` 的子类型（见 :ref:`Subtyping`）。

如果类型 ``U`` 是 ``T`` 的子类型，那么 ``Readonly<U>`` 也是 ``Readonly<T>`` 的子类型。

.. _Record Utility Type:

Record 工具类型
================================================================================

.. meta:
    frontend_status: Done

类型 ``Record<K, V>`` 构造一个把键映射到值的容器，其中键的类型是 ``K``，值的类型是 ``V``。

类型 ``K`` 仅限于数值类型（见 :ref:`Numeric Types`）、类型 ``string``、字符串字面量类型，以及由这些类型构成的联合类型。

如果使用了其他任何类型，或者其他类型的字面量来替代 ``K``，就会产生 :index:`compile-time error`。

其用法示例如下：

.. index::
   record utility type
   utility type
   value
   container
   restriction
   union type
   numeric type
   string type
   literal
   string literal type
   compile-time error
   type
   key
   union type

.. code-block:: typescript
   :linenos:

    type R1 = Record<number, Object>             // OK
    type R2 = Record<boolean, Object>            // Compile-time error
    type R3 = Record<"salary" | "bonus", Object> // OK
    type R4 = Record<"salary" | boolean, Object> // Compile-time error
    type R5 = Record<"salary" | number, Object>  // OK
    type R6 = Record<string | number, Object>    // OK

类型 ``V`` 没有限制。

对于 ``Record`` 类型实例，支持一种特殊形式的对象字面量（见 :ref:`Object Literal of Record Type`）。

对 ``Record<K, V>`` 的访问通过索引表达式 ``r[index]`` 完成，其中 ``r`` 是 ``Record`` 类型实例，``index`` 是类型为 ``K`` 的表达式（见 :ref:`Record Indexing Expression`）。

类型为 ``Record<K, V>`` 的变量可以使用合法的 ``Record`` 类型对象字面量（见 :ref:`Object Literal of Record Type`）初始化。当键表达式的类型与 ``K`` 兼容，且值表达式的类型与 ``V`` 兼容时，该字面量即为合法。

.. code-block:: typescript
   :linenos:

    type Keys = 'key1' | 'key2' | 'key3'

    let x: Record<Keys, number> = {
        'key1': 1,
        'key2': 2,
        'key3': 4,
    }
    console.log(x['key2']) // prints 2
    x['key2'] = 8
    console.log(x['key2']) // prints 8

上例中，``K`` 是字面量类型构成的联合，因此索引表达式的结果类型就是 ``V``，在这里即 ``number``。

.. index::
   restriction
   object literal
   literal
   instance
   Record type
   access
   indexing expression
   index expression
   index
   number
   expression
   variable
   compatibility
   value

|

.. _ReturnType Utility Type:

ReturnType 工具类型
================================================================================

.. meta:
    frontend_status: None

类型 ``ReturnType<T>`` 会根据函数类型 ``T``（见 :ref:`Function Types`）的返回类型构造一个新类型。如果传入的不是函数类型，且也不是 ``never``，则会产生 :index:`compile-time error`。示例如下：

.. code-block:: typescript
   :linenos:

   type MyString = ReturnType<()=> string> // OK
   type Incorrect = ReturnType<string>     // Compile-time error

   function foo<P extends Function, R = ReturnType<P>>() {}
   /* OK, default type for the second type parameter is the return type of
      the function type provided as the first type argument */

   foo<()=>number>()  // R is number

   type anAny = ReturnType<Function>  // anAny is Any
   type aNever = ReturnType<never>    // aNever is never

|

.. _Utility Type Private Fields:

工具类型中的私有字段
================================================================================

.. meta:
    frontend_status: Done

工具类型建立在其他类型之上。原始类型中的私有字段会保留在工具类型中，但它们不可访问（见 :ref:`Accessible`），也无法以任何方式访问。示例如下：

.. code-block:: typescript
   :linenos:

   function foo(): string {  // Potentially some side effect
      return "private field value"
   }

   class A {
      public_field = 444
      private private_field = foo()
   }

   function bar (part_a: Readonly<A>) {
      console.log (part_a)
   }

   bar ({public_field: 777}) // OK, object literal has no field `private_field`
   bar ({public_field: 777, private_field: ""}) // Compile-time error, incorrect field name

   bar (new A) // OK, object of type Readonly<A> has field `private_field`

.. index::
   utility type
   private field
   type
   access
   accessibility
   field name

|

.. _Nesting Utility Types:

嵌套工具类型
================================================================================

.. meta:
    frontend_status: Partly

如果需要同时使用多个工具类型，它们可以像下面这样嵌套：

.. code-block:: typescript
   :linenos:

   interface Issue {
     title?: string
   }

   const myIssue: Required<Readonly<Issue>> = {
      title: "One"
   };
   console.log(myIssue.title)  // Safe: required property
   myIssue.title = "Two" // Compile-time error, readonly property

.. index::
   utility type
   private field
   nesting
   readonly property
   required property
   type
   access
   accessibility

|

.. _Default Values for Types:

类型的默认值
********************************************************************************

.. meta:
    frontend_status: Done

.. note::
   该 |LANG| 特性目前属于实验特性。

所谓“默认值”，用于那些不要求显式初始化（见 :ref:`Variable Declarations`）的变量。以下类型具有默认值：

- :ref:`Value Types`；
- ``undefined`` 类型及其所有超类型。

所有其他类型，包括引用类型、枚举类型以及类型参数，都没有默认值。

值类型的默认值如下：

.. index::
   default value
   variable
   explicit initialization
   literal type
   undefined type
   type parameter
   reference type
   enumeration type
   initialization
   supertype

+--------------+--------------------+
| 数据类型     | 默认值             |
+==============+====================+
| ``number``   | 0 as ``number``    |
+--------------+--------------------+
| ``byte``     | 0 as ``byte``      |
+--------------+--------------------+
| ``short``    | 0 as ``short``     |
+--------------+--------------------+
| ``int``      | 0 as ``int``       |
+--------------+--------------------+
| ``long``     | 0 as ``long``      |
+--------------+--------------------+
| ``float``    | +0.0 as ``float``  |
+--------------+--------------------+
| ``double``   | +0.0 as ``double`` |
+--------------+--------------------+
| ``char``     | ``u0000``          |
+--------------+--------------------+
| ``boolean``  | ``false``          |
+--------------+--------------------+

值 ``undefined`` 是所有可以接受该值的类型的默认值。

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   class A {
     f1: string|undefined
     f2?: boolean
   }
   let a = new A()
   console.log (a.f1, a.f2)
   // Output: undefined, undefined

   let o1: Any
   let o2: void
   let o3: undefined
   console.log (o1, o2, o3)
   // Output: undefined, undefined, undefined

.. index::
   number
   byte
   short
   int
   long
   float
   double
   char
   boolean
   type
   null
   undefined type
   data type
   assignment

-------------

.. raw:: pdf

   PageBreak
