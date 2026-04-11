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


.. _Contexts and Conversions:

上下文与转换
################################################################################

.. meta:
    frontend_status: Done

本章定义表达式上下文，以及可在不同上下文中施加到表达式上的转换。

上下文可以分为以下几类：

- :ref:`赋值类上下文 <Assignment-like Contexts>`；
- :ref:`字符串运算符上下文 <String Operator Contexts>`，带有 ``string`` 拼接（运算符 ``'+'``）；
- :ref:`数值运算符上下文 <Numeric Operator Contexts>`，带所有数值运算符（``'+'``、``'-'`` 等）。

.. index::
   context
   conversion
   expression
   string
   assignment-like context
   numeric operator
   concatenation

|


.. _Assignment-like Contexts:

赋值类上下文
********************************************************************************

.. meta:
    frontend_status: Partly
    todo: Need to adapt es2panda implementation after assignment and call contexts are unified

*赋值类上下文* 包括：

- *声明上下文*，用于给带显式类型注解的变量（参见 :ref:`Variable Declarations`）、常量（参见 :ref:`Constant Declarations`）或字段（参见 :ref:`Field Declarations`）设置初始值；
- *赋值上下文*，用于把某个表达式值赋给变量（参见 :ref:`Assignment`）；
- *调用上下文*，用于把实参值赋给函数、方法、构造器或 lambda 调用的对应形式参数（参见 :ref:`Function Call Expression`、:ref:`Method Call Expression`、:ref:`Explicit Constructor Call` 和 :ref:`New Expressions`）；
- *返回上下文*，用于指定函数、方法或 lambda 调用的返回值（参见 :ref:`return Statements`）；
- *复合字面量上下文*，用于把表达式值设置给数组元素（参见 :ref:`Array Literal Type Inference from Context`）、类字段或接口字段（参见 :ref:`Object Literal`）。

.. index::
   assignment
   assignment-like context
   assignment context
   call context
   variable declaration
   constant declaration
   constant
   field
   field declaration
   assignment
   assignment context
   expression value
   expression
   conversion
   function call
   constructor call
   lambda call
   method call
   call context
   type
   type inference
   interface field
   formal parameter
   array literal
   object literal
   initial value
   value
   variable
   constant
   composite literal context
   function
   method
   constructor
   expression value
   array element

示例如下：

.. code-block:: typescript
   :linenos:

      // declaration contexts:
      let x: number = 1
      const str: string = "done"
      class C {
        f: string = "aa"
      }

      // assignment contexts:
      x = str.length
      new C().f = "bb"
      function foo<T1, T2> (p1: T1, p2: T2) {
        let t1: T1 = p1
        let t2: T2 = p2
      }

      // call contexts:
      function foo(s: string) {}
      foo("hello")

      // composite literal contexts:
      let a: number[] = [str.length, 11]

在所有这些情况下，表达式类型都必须可赋给 *目标类型*（参见 :ref:`Assignability`）。*可赋值性* 允许使用某种 :ref:`Implicit Conversions`。如果没有适用的转换，则产生 :index:`compile-time error`。

.. index::
   expression type
   expression
   target type
   assignability
   conversion

|

.. _String Operator Contexts:

字符串运算符上下文
********************************************************************************

.. meta:
    frontend_status: Done

*字符串上下文* 只适用于二元运算符 ``'+'`` 中的非 ``string`` 操作数，且另一侧操作数必须是 ``string``。

对非 ``string`` 操作数执行 *字符串转换* 的规则如下：

- 整数类型操作数（参见 :ref:`Integer Types and Operations`）会被转换为 ``string``，其值采用十进制表示；
- 浮点类型操作数（参见 :ref:`Floating-Point Types and Operations`）会被转换为 ``string``，其值采用十进制表示且不丢失信息；
- ``boolean`` 类型操作数会被转换为 ``string`` 值 ``true`` 或 ``false``；
- 若枚举类型（参见 :ref:`Enumerations`）的值是字符串基类型，则相应枚举值可转换为 ``string``；
- 对于具有空值的空值型操作数：

  - ``null`` 转换为字符串 ``null``；
  - ``undefined`` 转换为字符串 ``undefined``。

- 对引用类型操作数，或值不是字符串的 ``enum`` 类型操作数，则通过调用 ``toString()`` 完成转换。

如果没有适用转换，则会产生 :index:`compile-time error`。

在这一上下文中，目标类型始终是 ``string``。

.. index::
   string context
   non-string operand
   binary operator
   string operand
   string conversion
   conversion
   reference type
   integer type
   operand
   floating-point type
   loss of information
   enumeration type
   string type
   nullish type
   boolean
   decimal
   string conversion
   operand null
   operator undefined
   method call
   context

.. code-block:: typescript
   :linenos:

    console.log("" + null) // prints "null"
    console.log("value is " + 123) // prints "value is 123"
    console.log("BigInt is " + 123n) // prints "BigInt is 123"
    console.log(15 + " steps") // prints "15 steps"
    let x: string | null | undefined = null
    console.log("string is " + x) // prints "string is null"
    x = undefined
    console.log("string is " + x) // prints "string is undefined"

|

.. _Numeric Operator Contexts:

数值运算符上下文
********************************************************************************

.. meta:
    frontend_status: Done

数值上下文适用于算术运算符的操作数。数值上下文使用数值类型转换（参见 :ref:`Widening Numeric Conversions`），并确保每个实参表达式都能转换为目标类型 ``T``，从而为 ``T`` 类型的值定义算术运算。

若枚举的基类型是数值类型，则枚举类型操作数（参见 :ref:`Enumerations`）也可以在数值上下文中使用。在这种情况下，该操作数的类型被视为与枚举基类型相同。

.. index::
   numeric context
   arithmetic operator
   predefined type
   numeric type
   conversion
   argument expression
   target type
   string conversion
   string context
   type int

数值上下文包括以下形式：

- :ref:`Unary Expressions`；
- :ref:`Exponentiation expression`；
- :ref:`Multiplicative Expressions`；
- :ref:`Additive Expressions`；
- :ref:`Shift Expressions`；
- :ref:`Relational Expressions`；
- :ref:`Equality Expressions`；
- :ref:`Bitwise and Logical Expressions`；
- :ref:`Conditional-And Expression`；
- :ref:`Conditional-Or Expression`。

.. index::
   numeric context
   expression
   unary expression
   multiplicative expression
   additive expression
   shift expression
   relational expression
   equality expression
   bitwise expression
   logical expression
   conditional-and expression
   conditional-or expression

|

.. _Numeric Conversions for Relational and Equality Operands:

关系与相等操作数的数值转换
================================================================================

 .. meta:
     frontend_status: Done

关系运算符和相等运算符（参见 :ref:`Relational Expressions` 和 :ref:`Equality Expressions`）允许对不同大小的数值类型操作数进行隐式转换。此类转换的具体细节在 :ref:`Widening numeric conversions` 和 :ref:`Specifics of Numeric Operator Contexts` 中讨论。
  
下面用关系运算符 ``'<'`` 说明这种情况：

.. code-block:: typescript
   :linenos:

   let a: int = 1
   let b: long = 0

   if (b<a) { // 'a' converted to 'long' prior to comparison
      ;
   }


.. index::
   numeric conversion
   numeric types conversion
   widening numeric conversion
   operand
   numeric type
   conversion
   
|

.. _char Conversions for Relational and Equality Operands:

关系与相等操作数中的 ``char`` 转换
================================================================================

 .. meta:
     frontend_status: Done

关系运算符和相等运算符（参见 :ref:`Relational Expressions` 和 :ref:`Equality Expressions`）允许在另一侧操作数为数值类型时，对 ``char`` 类型操作数进行隐式转换。

``char`` 类型操作数会被提升为以下之一：

- 如果另一侧操作数是 ``byte``、``char`` 或 ``int``，则提升为 ``int``；
- 否则提升为另一侧操作数的类型。

下面用关系运算符 ``'<'`` 说明这种情况：

.. code-block:: typescript
   :linenos:

    function foo(c: char, b: byte, i: int, l: long, d: double) {
        c < b // 'c' and 'b' are converted to 'int' prior to comparison
        c < i // 'c' is converted to 'int' prior to comparison
        c < l // 'c' is converted to 'long' prior to comparison
        c < d // 'c' is converted to 'double' prior to comparison
    }

.. index::
   char conversion
   widening numeric conversion
   operand
   numeric type
   conversion   

|

.. _Implicit Conversions:

隐式转换
********************************************************************************

.. meta:
   frontend_status: Done
   todo: Narrowing Reference Conversion - note: Only basic checking available, not full support of validation
   todo: String Conversion - note: Implemented in a different but compatible way: spec - toString(), implementation: StringBuilder
   todo: Forbidden Conversion - note: Not exhaustively tested, should work

本节描述所有允许的隐式转换。每一种转换都只能在特定上下文中使用。例如，当用于初始化局部变量的表达式处于 :ref:`Assignment-like Contexts` 时，就由该上下文的规则决定表达式要隐式选用哪一种转换。

.. index::
   identity conversion
   conversion
   context
   local variable
   assignment
   assignment-like context
   conversion
   expression
   variable

|

.. _Widening Numeric Conversions:

拓宽数值转换
================================================================================

.. meta:
    frontend_status: Partly

*拓宽数值转换* 会把一个数值类型的值（参见 :ref:`Numeric Types`）转换为：

- 更大的数值类型；或
- 联合类型（参见 :ref:`Widening Numeric to a Union Type`）。

这种转换永远不会导致运行时错误。

.. code-block:: typescript
   :linenos:

    function foo(l: long) {}
    function bar(d: double) {}

    let b: byte = 1
    let s: short = 2
    let i: int = 3

    foo(b) // byte to long conversion
    foo(s) // short to long conversion
    foo(i) // int to long conversion

    let f: float = 3.14f

    bar(i) // int to double conversion
    bar(f) // float to double conversion

.. index::
   widening
   numeric conversion
   conversion
   numeric type
   value
   byte
   short
   int
   long
   float
   integer type

所有 *拓宽数值转换* 见下表：

+------------------+------------------------------------------------------+
| From Type        | To Type                                              |
+==================+======================================================+
| ``byte``         | ``short``, ``int``, ``long``, ``float``, ``double``  |
+------------------+------------------------------------------------------+
| ``short``        | ``int``, ``long``, ``float``, ``double``             |
+------------------+------------------------------------------------------+
| ``int``          | ``long``, ``float``, or ``double``                   |
+------------------+------------------------------------------------------+
| ``long``         | ``float`` or ``double``                              |
+------------------+------------------------------------------------------+
| ``float``        | ``double``                                           |
+------------------+------------------------------------------------------+

上述转换不会导致数值整体量级信息丢失。只有在正确使用 IEEE 754 的“最近值舍入”模式时，从整数类型转换到浮点类型才可能丢失某些最低有效位。得到的浮点值会被正确舍入到原整数值附近。


.. index::
   conversion
   numeric value
   floating-point type
   integer type
   conversion
   round-to-nearest mode
   runtime error
   IEEE 754
   widening
   numeric conversion
   rounding

|

.. _Widening Numeric to a Union Type:

拓宽数值到联合类型
================================================================================

.. meta:
    frontend_status: Done

如果联合类型 ``U``:sub:`1` ``| ... | U``:sub:`n` 中存在唯一一个数值类型 ``U``:sub:`i`，且它比当前值类型更大，则数值 ``v`` 会被转换为该 ``U``:sub:`i`。否则将产生 :index:`compile-time error`。

.. note::
   在拓宽到联合类型 *之前*，多数情况下会先应用以下语义规则：

    - 若 ``v`` 是数字字面量，则应用 :ref:`Type Inference for Constant Expressions`；

    - 若 ``U``:sub:`i` 与该值类型相同，则应用 :ref:`Subtyping for Union Types`。

   如果其中某条规则能够成功应用，则无需再进行拓宽转换。

所有情况见下例：

.. code-block:: typescript
   :linenos:

   let s: short = 1
   let i: int = 2

   let u: byte | int = 256 // OK, type inference for numeric literal
   console.log(u instanceof int) // output: true

   u = i // OK, subtyping
   console.log(u instanceof int) // output: true

   u = s // OK, widening to union type, short => int conversion
   console.log(u instanceof int) // output: true

|

.. _Enumeration to Numeric Type Conversion:

枚举到数值类型的转换
================================================================================

.. meta:
    frontend_status: Done

如果 *枚举基类型* 是数值类型，则枚举类型的值可被转换为：

- 等于或大于该 *枚举基类型* 的数值类型；或
- 在考虑 *枚举基类型* 后的联合类型（参见 :ref:`Widening Numeric to a Union Type`）。

该转换永远不会导致运行时错误。

.. code-block:: typescript
   :linenos:

    enum IntegerEnum {a, b, c}
    let int_enum: IntegerEnum = IntegerEnum.b
    let int_value: int = int_enum // int_value will get the value 1
    let number_value: number = int_enum
       /* number_value will get the value 1 as a result of conversion
          to a larger numeric type */

    enum DoubleEnum: double {a = 1.0, b = 2.0, c = 3.141592653589}
    let d_enum: DoubleEnum = DoubleEnum.a
    let d_value: double = d_enum // d_value will get the value 1.0

.. index::
   enumeration type
   numeric base type
   base type
   conversion
   integer type
   constant
   type int

|

.. _Enumeration to string Type Conversion:

枚举到 ``string`` 类型的转换
================================================================================

.. meta:
    frontend_status: Done

若某个 *枚举* 的基类型为 ``string``，则其值可以转换为 ``string`` 类型，或转换为包含 ``string`` 的联合类型（参见 :ref:`Union Types`）。该转换永远不会导致运行时错误。

.. code-block:: typescript
   :linenos:

    enum StringEnum {a = "a", b = "b", c = "c"}
    let s_enum: StringEnum = StringEnum.a
    let s: string = s_enum // 's' will get the value of "a"

    let u: string | number = s_enum // 'u' will get the value of "a"

.. index::
   enumeration type
   string type
   conversion
   constant
   runtime error

|

.. _Numeric Casting Conversions:

数值强制类型转换
********************************************************************************

.. meta:
    frontend_status: Done

当 *目标类型* 和表达式类型都属于 ``numeric`` 时，会发生 *数值强制类型转换*。其上下文是使用 :ref:`Standard Library` 中定义的转换方法的场景。

下面示例展示了显式使用数值转换方法的情况：

.. code-block-meta:
   not-subset

.. code-block:: typescript
   :linenos:

    function process_int(an_int: int) { /* ... */ }

    let pi = 3.14
    process_int(pi.toInt())

数值强制类型转换永远不会导致运行时错误。

把 ``double`` 类型的操作数转换为目标类型 ``float`` 时，转换遵循 IEEE 754 的舍入规则。这种转换可能丢失精度或范围，结果包括：

- 非零 double 变成 float 零；
- 有限 double 变成 float 无穷大。

double 的 ``NaN`` 会转换成 float 的 ``NaN``。

double 的无穷大会转换为同号的浮点无穷大。

.. index::
   numeric casting conversion
   target type
   expression type
   numeric type
   double type
   float type
   compliance
   rounding rule
   float zero
   nonzero double
   float infinity
   infinity double
   floating-point infinity
   double infinity
   double NaN
   Nan
   float NaN
   IEEE 754
   rounding rule
   conversion
   infinity

把浮点类型操作数转换到 ``long`` 或 ``int`` 时，规则如下：

- 如果操作数是 ``NaN``，则结果为 0；
- 如果操作数是正无穷大，或对目标类型来说过大，则结果为该目标类型可表示的最大值；
- 如果操作数是负无穷大，或对目标类型来说过小，则结果为该目标类型可表示的最小值；
- 否则，结果为按 IEEE 754 “向零舍入” 模式处理后的值。

把浮点类型操作数转换为 ``byte`` 或 ``short`` 时，分两步进行：

- 先执行到 ``int`` 的转换；
- 再把 ``int`` 操作数转换为目标类型。

.. index::
   target type
   floating-point operand
   floating-point type
   long type
   int type
   NaN
   numeric conversion
   byte
   short
   positive infinity
   negative infinity
   casting conversion
   runtime error
   operand
   compliance
   IEEE 754
   NaN
   floating-point type
   floating-point infinity
   rounding rules
   round-toward-zero

从某个整数类型到更小整数类型 ``I`` 的数值强制类型转换，会丢弃除最低 *N* 位之外的所有位，其中 *N* 是表示类型 ``I`` 所用的位数。该转换可能丢失数值大小信息，结果值的符号也可能与原值不同。

.. index::
   IEEE 754
   floating-point type
   numeric casting conversion
   operand
   conversion
   positive infinity
   target type
   negative infinity
   casting conversion
   integer type
   conversion
   rounding rule
   numeric value

|

.. raw:: pdf

   PageBreak
