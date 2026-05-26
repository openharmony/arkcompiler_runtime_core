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


.. _Expressions:

表达式
################################################################################

.. meta:
    frontend_status: Done

本章说明表达式的含义，以及在 |LANG| 中对表达式进行类型检查、求值和运行时处理的规则，
实验性特性（见 :ref:`Lambda Expressions with Receiver`）除外。
表达式既包括字面量、引用、调用、创建、索引、赋值和 lambda，也包括各类一元、
二元和条件表达式。

表达式相关的核心问题包括：

- 表达式的语法结构；
- 表达式的类型如何确定；
- 子表达式的求值顺序；
- 正常完成与异常完成；
- 运算符优先级与结合性；
- 各种特殊表达式的运行时语义。

.. index::
   evaluation
   expression
   syntax
   runtime semantics
   type checking

表达式的语法如下：

.. code-block:: abnf

       expression:
           primaryExpression
           | instanceOfExpression
           | castExpression
           | typeOfExpression
           | nullishCoalescingExpression
           | spreadExpression
           | unaryExpression
           | binaryExpression
           | assignmentExpression
           | ternaryConditionalExpression
           | stringInterpolation
           | lambdaExpression
           | lambdaExpressionWithReceiver
           | awaitExpression
           ;
       primaryExpression:
           literal
           | namedReference
           | arrayLiteral
           | objectLiteral
           | recordLiteral
           | thisExpression
           | parenthesizedExpression
           | methodCallExpression
           | fieldAccessExpression
           | indexingExpression
           | functionCallExpression
           | newExpression
           | ensureNotNullishExpression
           ;
       binaryExpression:
           multiplicativeExpression
           | exponentiationExpression
           | additiveExpression
           | shiftExpression
           | relationalExpression
           | equalityExpression
           | bitwiseAndLogicalExpression
           | conditionalAndExpression
           | conditionalOrExpression
           ;

.. index::
   coroutine
   lambda expression with receiver
   syntax

下面的语法产生式会在其他表达式规则中复用：

.. code-block:: abnf

       objectReference:
           typeReference
           | 'super'
           | primaryExpression
           ;

``objectReference`` 指以下三种形式之一：

- 处理静态成员的类；
- 用于访问超类构造函数或超类中被覆写方法版本的 ``super``；
- 求值后指向某个变量位置的 *primaryExpression*，除非其求值方式被链式运算符 ``?.``
  改变，见 :ref:`Chaining Operator`。

若 *primaryExpression* 的形式是 ``thisExpression``，则 ``this?.`` 会导致
:index:`compile-time error`。若其形式是 ``super``，则 ``super?.`` 也会导致
:index:`compile-time error`。

.. index::
   static member
   class
   access constructor
   superclass
   method
   overriding
   variable
   chaining operator
   pattern
   super

|

.. _Operators:

运算符
*******************************************************************************

表达式通常由运算符及其操作数组成。括号可用于改变默认的求值顺序。
*运算符* 或 *运算符记号* 是对操作数执行加法、减法等各种动作的记号
（参见 :ref:`Operators and Punctuators`）。根据操作数个数，运算符可以分为：

- 一元运算符：只有一个操作数；
- 二元运算符：有两个操作数；
- 三元运算符：有三个操作数。

某些运算符既可以是一元运算符，也可以是二元运算符。

每个运算符都有 *优先级* 和 *结合性*。当一个表达式中出现多个运算符时，
这两个属性决定语法归约与求值顺序：

- *优先级* 决定不同运算符之间谁先参与求值；
- *结合性* 决定同一优先级运算符按从左到右还是从右到左结合。

例如，下面代码里 ``+`` 的优先级高于 ``=``, 因而先计算加法；
而 ``=`` 是右结合的，所以赋值从右向左进行：

.. code-block:: typescript
   :linenos:

      let a: number = 1, b: number
      b = a = a+10
      console.log(b) // prints '11'

.. note::
   括号 ``( )`` 是优先级最高的 *分组运算符*，可显式改变表达式的求值顺序。

运算符的完整优先级与结合性列表见 :ref:`Operator Precedence`。

.. _Operator Precedence:

运算符优先级
===============================================================================

.. meta:
    frontend_status: Partly

不同运算符具有不同的优先级与结合性。
在没有显式括号时，编译器会按照规范给出的优先级表进行语法归约与类型检查。

.. index::
   precedence
   operator precedence
   operator
   associativity

.. list-table::
   :width: 100%
   :widths: 35 50 15
   :header-rows: 1

   * - 运算符类别
     - 优先级
     - 结合性
   * - 分组
     - ``'()'``
     - 不适用
   * - 成员访问与链式访问
     - ``'.'``、``'?.'``
     - 从左到右
   * - 访问与调用
     - ``'[]'``、``'.'``、``'()'``、``new``
     - 不适用
   * - 后缀自增与自减
     - ``'++'``、``'--'``
     - 不适用
   * - 后缀 !（确保非空值运算符）
     - ``'!'``
     - 不适用
   * - 前缀自增与自减、一元正负号、前缀 !（逻辑非）、按位取反、``typeof``、``await``
     - ``'++'``、``'--'``、``'+'``、``'-'``、``'!'``、``'~'``、``typeof``、``await``
     - 不适用
   * - 幂运算
     - ``'**'``
     - 从右到左
   * - 乘法类
     - ``'*'``、``'/'``、``'%'``
     - 从左到右
   * - 加法类
     - ``'+'``、``'-'``
     - 从左到右
   * - 类型转换
     - ``as``
     - 从左到右
   * - 移位
     - ``'<<'``、``'>>'``、``'>>>'``
     - 从左到右
   * - 关系
     - ``'<'``、``'>'``、``'<='``、``'>='``、``instanceof``
     - 从左到右
   * - 相等
     - ``'=='``、``'!='``
     - 从左到右
   * - 按位与
     - ``'&'``
     - 从左到右
   * - 按位异或
     - ``'^'``
     - 从左到右
   * - 按位或
     - ``'|'``
     - 从左到右
   * - 逻辑与
     - ``'&&'``
     - 从左到右
   * - 逻辑或
     - ``'||'``
     - 从左到右
   * - 空值合并
     - ``'??'``
     - 从左到右
   * - 三元
     - ``condition?whenTrue:whenFalse``
     - 从右到左
   * - 赋值
     - ``'='``、``'+='``、``'-='``、``'%='``、``'*='``、``'/='``、``'&='``、``'^='``、``'|='``、``'<<='``、``'>>='``、``'>>>='``
     - 从右到左

.. index::
   precedence
   bitwise operator
   null-coalescing operator
   assignment
   shift operator
   cast operator
   equality operator
   postfix operator
   increment operator
   decrement operator
   prefix operator
   logical operator
   relational operator
   exponentiation
   member access
   chaining
   access
   call
   ternary operator
   unary operator
   typeof operator
   await operator

|

.. _Evaluation of Expressions:

表达式求值
*******************************************************************************

.. meta:
    frontend_status: Done

表达式求值的结果可以是以下两类之一：

- 变量。这里的“变量”按一般意义使用，表示赋值左侧可修改的左值位置；
- 值。也就是表达式在运行时得到的普通结果值。

在后续求值需要值时，变量与值都同样按“表达式的值”参与进一步计算。
表达式的静态类型在编译期确定（见 :ref:`Type of Expression`），而实际值在运行期产生。

.. index::
   evaluation
   expression
   variable
   lvalue
   value
   assignment

.. note::
   表达式是由运算符和操作数组成、用于描述某种计算的结构。表达式求值可能产生副作用。
   任意表达式都可以作为更复杂表达式的子表达式。例如，``f() * 3`` 中的函数调用本身
   就是乘法表达式的子表达式。

:ref:`Constant Expressions` 是值可在编译期确定的表达式。

.. index::
   variable
   value
   evaluation
   expression
   type
   side effect
   constant expression
   compile time
   assignment
   increment operator
   decrement operator
   method call
   function call

.. _Type of Expression:

表达式的类型
===============================================================================

.. meta:
    frontend_status: Done

|LANG| 中的每个表达式都有类型，且表达式类型在编译期确定。

在多数上下文里，表达式都必须与上下文预期类型兼容。这个预期类型称为
*目标类型*。如果上下文中没有目标类型，该表达式就是*独立表达式*：

.. code-block:: typescript
   :linenos:

       let a = expr // no target type is available

       function foo() {
           expr // no target type is available
       }

否则，该表达式就是*非独立表达式*：

.. index::
   inferred type
   expression
   evaluation
   compile time
   compatibility
   type inference
   compatible expression
   standalone expression
   non-standalone expression
   context
   target type

.. code-block-meta:
   skip

.. code-block:: typescript
   :linenos:

       let a: number = expr // target type of 'expr' is number

       function foo(s: string) {}
       foo(expr) // target type of 'expr' is string

某些表达式无法仅通过自身确定类型（见 :ref:`Type Inference`），例如对象字面量（见 :ref:`Object Literal`）。
如果这类表达式被当作独立表达式使用，则会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       class P { x: number, y: number }

       let x = { x: 10, y: 10 } // standalone object literal - compile time error
       let y: P = { x: 10, y: 10 } // OK, type of object literal is inferred

表达式类型的确定需要完成以下步骤：

#. 收集类型推断所需信息，如类型注解、泛型约束等；
#. 执行 :ref:`Type Inference`；
#. 若前一步仍未得到类型，且表达式是广义字面量（包括 :ref:`Array Literal`），
   则尝试仅根据表达式本身推导类型。

.. index::
   expression
   standalone expression
   expression type
   context
   evaluation
   array literal
   inferred type
   string
   type annotation
   generic
   constraint
   type inference
   object
   literal

若上述步骤都不能产生合适的表达式类型，则发生 :index:`compile-time error`。

若表达式类型是 ``readonly``，则目标类型也必须是 ``readonly``；否则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

         let readonly_array: readonly number[] = [1, 2, 3]

         foo1(readonly_array) // OK
         foo2(readonly_array) // Compile-time error

         function foo1 (p: readonly number[]) {}
         function foo2 (p: number[]) {}

         let writable_array: number [] = [1, 2, 3]
         foo1 (writable_array) // OK, as always safe

.. index::
   expression
   expression type
   readonly
   target type
   type

.. _Normal and Abrupt Completion of Expression Evaluation:

表达式求值的正常完成与异常完成
===============================================================================

.. meta:
    frontend_status: Done

表达式求值往往包含多个计算步骤。若所有必要步骤都在没有抛出错误的情况下完成，
则称该表达式*正常完成*。若任一步骤抛出错误并中止求值，则称为*异常完成*。
异常完成的原因体现在所抛出的错误对象上。

.. index::
   normal completion
   abrupt completion
   evaluation
   expression
   error
   error object
   value

表达式求值可能在以下情况下产生 :index:`runtime error`：

- 若数组索引值为负，或大于等于数组长度，则 :ref:`Array Indexing Expression`
  抛出 ``RangeError``；
- 若写入定长数组元素的值类型不是该元素类型的子类型，则 :ref:`Assignment`
  抛出 ``ArrayStoreError``；
- 若 :ref:`Cast Expression` 在运行时无法完成转换，则抛出 ``ClassCastError``；
- 若右侧表达式的值为 0，则整数除法或整数余数运算
  （见 :ref:`Division` 与 :ref:`Remainder`）抛出 ``ArithmeticError``。

.. index::
   runtime error
   predefined operator
   evaluation
   expression
   operator evaluation
   expression evaluation
   operator
   array indexing expression
   RangeError
   fixed-size array
   ArrayStoreError
   cast expression
   ClassCastError
   integer division
   integer remainder
   ArithmeticError
   array element
   array element type
   assignment

表达式求值中的错误还可能来自链接阶段或虚拟机内部、且较难预测的运行时故障。

若某个子表达式异常完成，则：

- 若外层表达式的求值依赖该子表达式，则外层表达式立即异常完成；
- 正常求值路径上的后续步骤全部取消。

*正常完成* 与 *异常完成* 这两个术语也可用于语句执行
（见 :ref:`Normal and Abrupt Statement Execution`）。

.. index::
   normal completion
   abrupt completion
   execution
   statement
   virtual machine
   error
   expression
   subexpression
   evaluation
   linkage

.. _Order of Expression Evaluation:

表达式求值顺序
===============================================================================

.. meta:
    frontend_status: Done

子表达式通常按从左到右的顺序求值，下面的示例准确体现了这一顺序：

- 二元运算符的右操作数必须在左操作数完全求值后才能开始求值：

  .. code-block:: typescript
     :linenos:

           function arg(s: string): int { console.log(s); return 1; }

           arg("left") +  arg("right")
           // Prints:
           //    left
           //    right

- 三元运算符总是先求值条件，然后再根据条件值，只求值 ``whenTrue`` 或 ``whenFalse`` 其中一支：

  .. code-block:: typescript
     :linenos:

           function arg(s: string): int { console.log(s); return 1; }

           arg("condition True") ? arg("left 1") :  arg("right 1")
           arg("condition False")*0 ? arg("left 2") :  arg("right 2")
           // Prints:
           //    condition True
           //    left 1
           //    condition False
           //    right 2

- :ref:`Conditional-And Expression` 和 :ref:`Conditional-Or Expression`
  的操作数按从左到右求值，并根据左操作数的值决定是否跳过右操作数：

  .. code-block:: typescript
     :linenos:

           function arg(s: string): int { console.log(s); return 1; }

           // Conditional And
           arg("left And (true)") && arg("right And")
           console.log("---")
           arg("left And (false)")*0 && arg("right And")
           console.log("---")

           // Conditional Or
           arg("left Or (true)") && arg("right Or")
           console.log("---")
           arg("left Or (false)")*0 && arg("right Or")
           console.log("---")

           // Print
           //    left And (true)
           //    right And
           //    ---
           //    left And (false)
           //    ---
           //    left Or (true)
           //    ---
           //    left Or (false)
           //    right Or

- 赋值运算符是右结合的，因此包含多个赋值的表达式要额外区分“求值顺序”和“赋值顺序”：

  .. code-block:: typescript
     :linenos:

           class A {
              constructor(s:string) { console.log(s); this.tag = s; }
              tag: string
           }
           function f(s: string) { return new A(s); }

           console.log( f("left").tag = f("middle").tag = f("right").tag )
           // print
           //    left
           //    middle
           //    right
           //    right


  这个例子的求值过程是：

  - 三个子表达式仍按从左到右求值；其中最左和中间两个子表达式的求值结果都是左值表达式；
  - 之后由于赋值运算符右结合，真正的写入动作从右向左执行；
  - 也就是说，最右侧子表达式的值会先写入中间左值，再把中间表达式结果写入最左侧左值。

|LANG| 同时遵守括号显式给出的求值顺序，以及运算符优先级隐式规定的求值顺序。
这一规则尤其适用于浮点计算中的无穷大与 ``NaN`` 场景。|LANG| 可以把整数加法和乘法
视为可证明结合；但浮点计算即便在数学上看似结合，也不能据此在实现中随意重排。

.. index::
   operand
   operator
   assignment operator
   abrupt completion
   normal completion
   evaluation
   conditional operator
   binary operator
   integer division
   integer remainder
   expression
   compound-assignment operator
   precedence
   operator precedence
   infinity
   NaN value
   floating-point calculation
   associativity
   parentheses

.. index::
   order of evaluation
   subexpression
   argument evaluation
   short-circuit evaluation

.. _Evaluation of Arguments:

实参求值
===============================================================================

.. meta:
    frontend_status: Done

调用中的实参总是从左到右求值，直到遇到第一个错误，或直到整个表达式结束。
也就是说，任何实参表达式都要等到其左侧所有实参表达式正常完成后，才会开始求值。
这条规则适用于方法调用、构造函数调用、类实例创建以及函数调用中的逗号分隔实参。

若左侧实参表达式异常完成，则右侧实参表达式不会再被求值。

.. code-block:: typescript
   :linenos:

      function arg(s: string): int { console.log(s); return 1; }
      function errArg(s: string, v: int): int { console.log(s); return 1/v }
      function test(a: int, b: int, c: int) {}

      test(arg("left"), arg("middle"), arg("right"))
      test(errArg("errArg (left)", 0), arg("middle"), arg("right"))
      // Prints:
      //    left
      //    middle
      //    right
      //    errArg (left)
      //    ... divideByZero runtime error reported


.. index::
   evaluation
   argument
   error
   expression
   normal completion
   method call
   constructor call
   function call
   abrupt completion

.. _Evaluation of Other Expressions:

其他表达式的求值
===============================================================================

.. meta:
    frontend_status: Done

上述一般规则不足以覆盖某些会间歇触发异常条件的表达式。
以下表达式的求值顺序需要额外说明：

- :ref:`New Expressions`；
- :ref:`Resizable Array Creation Expressions`；
- :ref:`Indexing Expressions`；
- :ref:`Method Call Expression`；
- 涉及索引的 :ref:`Assignment`；
- :ref:`Lambda Expressions`。

.. index::
   evaluation
   expression
   method call expression
   class instance
   array
   indexing expression
   assignment
   lambda expression
   resizable array

|

.. _Literal:

字面量
*******************************************************************************

.. meta:
    frontend_status: Done

字面量（见 :ref:`Literals`）表示固定且不变化的值。字面量的类型就是表达式的类型。
对于数值字面量，其类型通过 :ref:`Type Inference for Constant Expressions` 推断。

.. index::
   literal
   expression
   value
   type

.. index::
   literal
   array literal
   object literal
   string literal
   numeric literal

.. _Named Reference:

命名引用
*******************************************************************************

.. meta:
    frontend_status: Done

表达式可以具有*命名引用*的形式，其语法如下：

.. code-block:: abnf

       namedReference:
         qualifiedName typeArguments?
         ;

命名引用表达式的类型，就是该命名引用所指向实体的类型。

*qualifiedName*（见 :ref:`Names`）是由点分隔名称组成的表达式。
若 ``qualifiedName`` 只包含一个标识符，则它是*简单名*。

.. index::
   expression
   named reference
   qualified name
   syntax
   entity
   dot-separated name
   simple name

简单名可以指向：

- 当前模块中声明的实体，例如变量、常量、函数或访问器；
- 外围代码块、函数体、方法体或 lambda 体中声明的变量、常量或形参。

非简单名的 ``qualifiedName`` 可以指向：

- 从模块导入的实体；
- 从命名空间导出的实体；
- 某个类或接口的成员；
- :ref:`Record Indexing Expression` 的某种特殊语法形式。

若提供了 ``typeArguments``，则 ``qualifiedName`` 必须是合法的泛型方法或泛型函数实例化；
否则发生 :index:`compile-time error`。若 ``qualifiedName`` 指向的名称未定义或不可访问，
同样发生 :index:`compile-time error`。

若命名引用指向函数名，则它是 :ref:`Function Reference`；
若它指向方法名，则它是 :ref:`Method Reference`。

.. index::
   simple name
   entity
   declaration
   module
   variable
   constant declaration
   parameter
   function
   method
   imported entity
   exported entity
   namespace
   class
   interface
   record indexing expression
   instantiation
   generic method
   generic function
   function reference
   method reference

.. _Function Reference:

函数引用
===============================================================================

*函数引用* 指向某个已声明或已导入的函数。函数引用的类型由函数签名导出：

.. code-block:: typescript
   :linenos:

      function foo(n: number): string { return n.toString() }
      let func = foo // type of func is '(n: number) => string'
      let x = func(1)  // foo() called via reference

函数引用也可以指向泛型函数，但只有在显式给出
:ref:`Explicit Generic Instantiations` 时才合法；否则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       function gen<T> (x: T) {}

       let a = gen<string> // OK
       let b = gen // Compile-time error, no explicit type arguments

.. index::
   function reference
   function signature
   declared function
   imported function
   generic function
   generic instantiation
   type argument

若把重载函数名称（见 :ref:`Implicit Function Overloading`）或
显式重载名称（见 :ref:`Explicit Function Overload`）直接当作函数引用使用，
也会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       function bar(n: number) {}
       function bar(s: string) {}

       let b = bar // Compile-time error, overloaded function name

       function foo1(n: number) {}
       function foo2(s: string) {}
       overload foo { foo1, foo2 }

       foo(1)          // OK, overload call
       let x = foo     // Compile-time error, explicit overload name
       let y = foo2    // OK, ref to foo2

.. _Method Reference:

方法引用
===============================================================================

*方法引用* 指向类或接口的*静态方法*或*实例方法*。方法引用的类型由方法签名导出：

.. code-block:: typescript
   :linenos:

       class C {
         static foo(n: number) {}
         bar (s: string): boolean { return true }
       }

       // Method reference to a static method
       const m1 = C.foo  // type of 'm1' is (n: number) => void

       // Method reference to an instance method
       const m2 = new C().bar // type of 'm1' is (s: string) => boolean

.. index::
   method reference
   static method
   instance method
   class
   interface
   method signature

若方法引用指向实例方法，则该命名引用会与使用到的实例绑定：

.. code-block:: typescript
   :linenos:

       class C {
           field = 123
           method(): number { return this.field }
       }
       let c1 = new C
       let c2 = new C
       let m1 = c1.method // 'c1' is bounded
       let m2 = c2.method // 'c2' is bounded
       c1.field = 42
       console.log (m1(), m2()) // Outputs: 42 123

泛型方法只有在显式给出泛型实例化（见 :ref:`Explicit Generic Instantiations`）时
才能作为方法引用使用，否则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       class C {
           gen<T> (x: T) {}
       }

       let a = new C().gen<string> // OK
       let b = new C().gen // Compile-time error, no explicit type arguments

.. index::
   method reference
   instance method
   named reference
   class
   interface
   generic method
   generic instantiation
   method signature
   compile-time error

若把*重载方法*名称直接用作方法引用，也会发生 :index:`compile-time error`（见 :ref:`Implicit Method Overloading`、:ref:`Explicit Class Method Overload` 和 :ref:`Explicit Class Method Overload`）：

.. code-block:: typescript
   :linenos:

       class C {
           foo1(n: number) {}
           foo2(s: string) {}
           overload foo { foo1, foo2 }

           function bar(n: number) {}
           function bar(s: string) {}
       }

       let c = new C()
       let f = c.foo // Compile-time error
       let b = c.bar // Compile-time error

.. index::
   type argument
   method
   explicit overload
   named reference

|

.. _Array Literal:

数组字面量
*******************************************************************************

.. meta:
    frontend_status: Done

*数组字面量* 是一种在某些情况下用于创建数组或元组的表达式，其所有元素值都显式写出。

其语法如下：

.. code-block:: abnf

       arrayLiteral:
           '[' expressionSequence? ']'
           ;

       expressionSequence:
           expression (',' expression)* ','?
           ;

数组字面量是由 ``'['`` 和 ``']'`` 包围、以逗号分隔的*初始化表达式*列表。
最后一个表达式后的尾逗号会被忽略：

.. index::
   array literal
   array
   tuple
   expression
   value
   syntax
   initializer expression
   trailing comma
   bracket

.. code-block:: typescript
   :linenos:

       let x = [1, 2, 3] // OK
       let y = [1, 2, 3,] // OK, trailing comma is ignored

方括号中初始化表达式的数量决定要构造的数组长度。若内存分配成功，就会创建一个指定长度的数组，
并用初始化表达式的值初始化所有元素。

.. index::
   initializer expression
   array length
   array initializer
   array element
   initialization
   value

若出现以下任一情况，数组字面量求值会异常完成：

- 没有足够内存用于创建新数组，并抛出 ``OutOfMemoryError``；
- 某个初始化表达式异常完成。

.. index::
   evaluation
   abrupt completion

.. index::
   initializer expression
   execution
   value
   nested literal
   array element
   array literal expression
   array literal
   array type

初始化表达式按从左到右执行。第 *n* 个初始化表达式为数组的第 ``n-1`` 个元素提供值。
数组字面量可以嵌套，也就是数组元素本身也可以是数组字面量。

数组字面量表达式的类型按以下规则推断：

- 若存在上下文，则优先从上下文推断；若成功，则数组字面量类型就是推断结果，
  否则发生 :index:`compile-time error`；
- 若不存在上下文，则从数组元素类型推断（见 :ref:`Array Type Inference from Types of Elements`）。

.. index::
   type inference
   inferred type
   context
   array literal
   array element

.. _Array Literal Type Inference from Context:

基于上下文的数组字面量类型推断
===============================================================================

.. meta:
    frontend_status: Done

若上下文已经给出目标类型，例如 :ref:`Resizable Array Types`、:ref:`Tuple Types`、
:ref:`Fixed-size Array Types`、值数组类型（见 :ref:`Value Array Types`）、接口类型或 :ref:`Union Types`，
则数组字面量会优先按上下文进行类型推断。
如果上下文与字面量结构不兼容，则会发生 :index:`compile-time error`。

可用于提供上下文的典型场景如下：

.. code-block:: typescript
   :linenos:

       let a: number[] = [1, 2, 3] // OK, variable type in a declaration is used
       a = [4, 5] // OK, variable type is used

       let b = [1, 2, 3] as number[]    // OK, cast target type is used

       function min(x: number[]): number {
         let m = x[0]
         for (let v of x) {
           if (v < m) m = v
         }
         return m
       }
       min([1., 3.14, 0.99]); // OK, parameter type is used

       // Array of array initialization
       type Matrix = number[][]
       let m: Matrix = [
           [1, 2], [3, 4] // OK, element type is used
       ]

       class aClass {}
       //
       let b1: Array <aClass> = [new aClass, new aClass]
       let b2: Array <number> = [1, 2, 3]
       let b3: FixedArray<number> = [1, 2]
         /* Type of literal is inferred from the context
            taken from b1, b2 and b3 declarations */

.. index::
   type
   inferred type
   type inference
   variable
   variable declaration
   assignment
   cast expression
   call parameter type
   array initialization
   array literal
   literal
   context
   array


若上下文类型不是以下任一类型，则发生 :index:`compile-time error`：

- ``Any``；
- ``Object``；
- 元组类型；
- 定长数组类型；
- 值数组类型；
- 可变长数组类型；
- 可变长数组类型的超接口；
- 至少包含上述某一成员的联合类型。

若上下文类型是 ``Any`` 或 ``Object``，则退回到
:ref:`Array Type Inference from Types of Elements`：

.. code-block:: typescript
   :linenos:

       let a: Object = [1, 2, 3] // OK, array literal is of int[] type

若上下文类型是元组类型（见 :ref:`Tuple Types`），则必须同时满足：

- 表达式个数与元组成员数相同；
- 每个位置上的表达式类型都可赋值（见 :ref:`Assignability`）给该位置成员类型。

否则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       let tuple: [number, string] = [1, "hello"] // OK
       let incorrect: [number, string] = ["hello", 1] // Compile-time error

若上下文类型是定长数组（见 :ref:`Fixed-size Array Types`）、值数组或
可变长数组类型（见 :ref:`Resizable Array Types`，含 :ref:`Readonly Array Types`），
则每个表达式都必须可赋值给元素类型，否则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       let a: FixedArray<string> = ["hello", "world"] // OK
       let b: FixedArray<string> = [1, 2]             // Compile-time error
       let c: FixedArray<Object> = [1, "hello"]       // OK

若上下文类型是值数组类型，则每个表达式都必须可赋值给元素类型，
否则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       let a: ValueArray<int> = [1, 2]    // OK
       let b: ValueArray<double> = [1, 2] // OK
       let b: ValueArray<int> = [3.14]    // Compile-time error

若上下文类型是可变长数组类型，则每个表达式都必须可赋值给元素类型，
否则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       let a: Array<string> = ["aa", "bb"]     // OK
       let b: string[] = ["aa", "bb"]          // OK
       let c: readonly string[] = ["aa", "bb"] // OK

       let d: string[] = ["aa", 2]             // Compile-time error

       let o: Object[] = ["aa", 2]             // OK

若上下文类型是接口 ``I``，则：

- 如果 ``I`` 是某个可变长数组类型的泛型超接口 ``I<T>``，则数组字面量可视为
  ``Array<T>`` 的实例。若每个表达式都可赋值给 ``T``，则字面量类型为 ``I<T>``；否则发生 :index:`compile-time error`；
- 如果 ``I`` 是某个可变长数组类型的非泛型超接口，则先按
  :ref:`Array Type Inference from Types of Elements` 推导，再把结果视为 ``I``；
- 否则发生 :index:`compile-time error`。

.. code-block:: typescript
   :linenos:

       interface SomeI {}
       let a = [1, 2] as SomeI // Compile-time error, SomeI is not a superinterface of Array

       let b: ConcatArray<number> = [1, 2]  // OK, instance of Array<number>
       let c: ConcatArray<string> = [1, 2]  // Compile-time error, int is not assignable to string
       let d: ArrayLike<Object> = [1, "aa"] // OK, instance of Array<Object>

若上下文类型是联合类型（见 :ref:`Union Types`），则会对联合中每个候选类型重复执行
:ref:`Array Literal Type Inference from Context`。
如果只有一个候选类型推断成功，则采用它；否则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

      let union_1: string[] | [int, int] = [1, 2]
      // OK, literal type is a tuple [int, int]

      let union_2: number[] | [number, number] = [1, 2]
      // Error, as both union types accept literal [1, 2] as valid values

      let union_3: (number | string )[] | [(number | string), number] = ["xxx", 2]
      // Error, as both union types accept literal ["xxx", 2] as valid values

      let union_4: (number | string )[] | [(number | string), number] | string = "xxx"
      // OK, as only type string matches the type of "xxx" literal

      let union_5: (number | string )[] | [(number | string), number, boolean] | string = [5, 5]
       // OK, literal type is an array (number | string )[]

.. index::
   tuple
   type
   context
   literal expression
   compatibility
   union type
   type inference
   inferred type
   variable
   variable declaration
   assignment
   cast expression
   call parameter type
   array initialization

.. _Array Type Inference from Types of Elements:

基于元素类型的数组类型推断
===============================================================================

.. meta:
    frontend_status: Done

如果不存在可用上下文，则数组字面量的类型根据其元素类型集合推导。
在这种情况下，语言会构造适当的数组类型或联合元素类型。

若没有上下文，也就是数组字面量类型无法从上下文推断（见 :ref:`Type of Expression`），
则 ``[expr1, ..., exprN]`` 的类型按以下算法从初始化表达式推导：

#. 若数组字面量为空，则类型无法推断，发生 :index:`compile-time error`；
#. 若任意一个元素表达式的类型无法确定，则数组字面量类型无法推断，发生 :index:`compile-time error`；
#. 若所有初始化表达式都具有相同类型 ``T``，则数组字面量类型为 ``T[]``；
#. 若所有初始化表达式都是数值类型（见 :ref:`Numeric Types`），则数组字面量类型为 ``number[]``；
#. 否则，数组字面量类型构造成 ``(T1 | ... | TN)[]``，其中 ``Ti`` 是第 ``i`` 个元素表达式的类型。
   在构造该联合类型后，还要继续执行以下规则：

   - 若 ``Ti`` 是字面量类型，则先把它替换成对应的超类型；
   - 若 ``Ti`` 是由字面量类型组成的联合类型，则先把其中每个字面量成员都替换成各自超类型；
   - 完成上述替换后，再应用 :ref:`Union Types Normalization`。

这意味着，这里的推断不是简单地把所有元素类型并起来，
而是要先消除只在字面量层面存在的过窄差异，再形成规范化后的数组元素类型。

.. index::
   context
   type inference
   inferred type
   array element
   array literal
   type
   initialization expression
   expression
   numeric type
   union type normalization
   union type
   supertype
   literal type

.. code-block:: typescript
   :linenos:

       type A = number
       let u : "A" | "B" = "A"

       let a = []                        // Compile-time error, type cannot be inferred
       let b = ["a"]                     // type is string[]
       let c = [1, 2, 3]                 // type is int[]
       let d = [1, 2.1, 3]               // type is number[]
       let e = ["a" + "b", 1, 3.14]      // type is (string | number)[]
       let f = [u]                       // type is string[]
       let g = [(): void => {}, new A()] // type is (() => void | A)[]

|

.. _Object Literal:

对象字面量
*******************************************************************************

.. meta:
    frontend_status: Done

对象字面量由花括号中的字段与方法定义组成。
对象字面量的整体类型可能是某个类类型、接口类型、``Record`` 类型，
也可能构造出匿名类实例。

对象字面量是一种可用于创建类实例的表达式。在许多场景下，它比
:ref:`New Expressions` 更方便。

对象字面量的一种特殊形式称为 *record literal*，用于创建
:ref:`Record Utility Type` 的实例。语法和其他细节见
:ref:`Object Literal of Record Type`。

其语法如下：

.. code-block:: abnf

       objectLiteral:
          '{' objectLiteralMembers? '}'
          ;

       objectLiteralMembers:
          objectLiteralMember (',' objectLiteralMember)* ','?
          ;

       objectLiteralMember:
          objectLiteralField | objectLiteralMethod
          ;

       objectLiteralField:
          identifier ':' expression
          ;

       objectLiteralMethod:
          identifier typeParameters? signature block
          ;


对象字面量由逗号分隔的成员列表构成，并用大括号包围。尾逗号会被忽略。
对象字面量成员既可以是字段，也可以在接口类型场景中是方法。
详细规则见 :ref:`Object Literal of Class Type` 和 :ref:`Object Literal of Interface Type`。

.. index::
   object literal
   expression
   instance
   class
   class instance
   instance creation expression
   comma-separated list
   trailing comma
   identifier
   expression

对象字面量字段采用 ``identifier: expression`` 形式，对象字面量方法则是完整的
``public`` 方法声明。类类型对象字面量只能包含字段；接口类型对象字面量既可以
包含属性赋值，也可以包含方法实现（参见 :ref:`Object Literal of Interface Type`）。

.. code-block:: typescript
   :linenos:

       class Person {
         name: string = ""
         age: number = 0
       }
       let b: Person = {name: "Bob", age: 25}
       let a: Person = {name: "Alice", age: 18, } // OK, trailing comma is ignored
       let c: Person | number = {name: "Mary", age: 17} // Literal is of type Person

.. index::
   object literal
   comma-separated list
   name-value pair
   curly brace
   trailing comma
   identifier
   expression

对象字面量表达式的类型总是从上下文推导得到的某个类 ``C``。这个 ``C`` 可能是具名类（见 :ref:`Object Literal of Class Type`），
也可能是针对某个接口类型隐式创建的匿名类（见 :ref:`Object Literal of Interface Type`）。

.. index::
   object literal expression
   inferred type
   class type
   interface type
   anonymous class
   context

若出现以下任一情况，则发生 :index:`compile-time error`：

- 无法从上下文推断对象字面量的类型（示例见 :ref:`Type of Expression`）；
- 推断结果不是类类型或接口类型；
- 上下文是联合类型，且对象字面量同时能匹配多个候选分量类型；
- 对象字面量声明了上下文类型中不存在的新成员。

.. code-block:: typescript
   :linenos:

       let p = {name: "Bob", age: 25}
               // Compile-time error, type cannot be inferred

       class A { field = 1 }
       class B { field = 2 }

       let q: A | B = {field: 6}
               // Compile-time error, type cannot be inferred as the literal
               // fits both A and B

       let u: A = { field: 1, otherField: 2 }
               // Compile-time error, cannot declare a new member in the literal

.. index::
   object literal expression
   type inference
   inferred type
   class type
   interface type
   context
   union type
   object literal

.. _Object Literal of Class Type:

类类型对象字面量
===============================================================================

.. meta:
    frontend_status: Done

若上下文推导出对象字面量应当匹配某个类类型，则其字段名必须与该类中的可初始化成员
对应，并满足类型兼容与构造规则。


.. index::
   object literal
   class type
   type inference
   context

若上下文推断出类类型 ``C``，则对象字面量的类型就是 ``C``：

.. code-block:: typescript
   :linenos:

       class Person {
         name: string = ""
         age: number = 0
       }
       function foo(p: Person) { /*some code*/ }
       // ...
       let p: Person = {name: "Bob", age: 25} /* OK, variable type is
            used */
       foo({name: "Alice", age: 18}) // OK, parameter type is used

.. index::
   identifier
   field
   class
   compile-time error
   accessible member field
   type

对象字面量字段中的标识符必须命名 ``C`` 的某个可访问成员字段（见 :ref:`Accessible`），否则发生
:index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       class Friend {
         name: string = ""
         protected soname: string = ""
         private nick: string = ""
         age: number
         sex?: "male"|"female"
       }
       // Compile-time error, nick is private:
       let f: Friend = {name: "Alexander", age: 55, nick: "Alex"}
       // Compile-time error, soname is protected:
       let g: Friend = {name: "Alexander", age: 55, soname: "Reed"}

若对象字面量字段的表达式类型不能赋值（见 :ref:`Assignability`）给相应字段类型，也会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       let f: Friend = {name: 123} /* compile-time error, type of right hand-side
       is not assignable to the type of the left hand-side */

只有拥有默认值（见 :ref:`Default Values for Types`）或显式初始化器（见 :ref:`Variable and Constant Declarations`）的类字段才能在对象字面量中省略；否则发生 :index:`compile-time error`。
此外，类 ``C`` 还必须拥有显式或默认的无参构造函数，或全部参数都是可选参数（见 :ref:`Optional Parameters`）的构造函数，
且该构造函数在类复合上下文中可访问（见 :ref:`Accessible`）：

.. code-block:: typescript
   :linenos:

       let f: Friend = {} /* OK, as name, nick, age, and sex have either default
                             value or explicit initializer */

       class C {
           f1: number
           f2: string
           f3!: Object
       }
       let c1: C = {f2: "xyz", f3: new Object} // OK, f1 type has a default value
       let c2: C = {f2: "xyz"} // Compile-time error, f3 value is not provided

.. code-block:: typescript
   :linenos:

       class C {
         constructor (p: number) {}
       }
       // ...
       let c: C = {} /* compile-time error, no parameterless
              constructor */

.. code-block:: typescript
   :linenos:

       class C {
         private constructor () {}
       }
       // ...
       let c: C = {} /* compile-time error, constructor is not
           accessible */

.. code-block:: typescript
   :linenos:

       class C {
         constructor (p?: number) {}
       }
       // ...
       let c: C = {} // OK as constructor of has an optional parameter

.. code-block:: typescript
   :linenos:

       class C {
       }
       // ...
       let c: C = {} // OK as default constructor is added


.. index::
   parameterless constructor
   accessible constructor
   optional parameter
   default constructor
   default value
   class field
   initializer
   omitted field

若对象字面量显式设置类的 ``readonly`` 字段，则发生 :index:`compile-time error`；
若类中只存在 getter 而没有 setter，则对应名称也不能在对象字面量中赋值。

.. code-block:: typescript
   :linenos:

       class C {
           field1 = 123
           readonly field2: number
           readonly field3: string
           constructor () {
               this.field3 = ""
           }
       }

       // OK, type ``number`` has default value, field3 set in ctor
       let c: C = { field1: 654 }

       // Error: field2 and field3 are readonly, cannot be set explicitly
       let d: C = { field1: 654, field2: 3, field3: "text" }


如果类中某个名称对应的是 setter 访问器（见 :ref:`Class Accessor Declarations`），那么对象字面量可以使用该名称为属性赋值；
若只有 getter 而没有 setter，则会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       class OK {
           set attr (attr: number) {}
       }
       const a: OK = {attr: 42} // OK, as the setter be called

       class Err {
           get attr (): number { return 42 }
       }
       const b: Err = {attr: 42} // Compile-time error, no setter for 'attr'

.. index::
   readonly field
   setter
   getter
   accessor
   object literal field
   class accessor
   compile-time error


.. index::
   object literal
   class type
   type inference
   context
   assignability
   field type
   parameterless constructor
   accessible constructor
   accessor
   setter
   object literal field

.. _Object Literal of Interface Type:

接口类型对象字面量
===============================================================================

.. meta:
    frontend_status: Done

若上下文推导出对象字面量应匹配某个接口类型，则它必须为所有必需属性和方法
提供实现，并满足可选属性与访问器契约。


.. index::
   interface type
   type inference
   inferred type
   context
   object literal
   anonymous class
   interface
   field

若上下文推断出接口类型 ``I``，则对象字面量的实际类型是为 ``I`` 隐式创建的匿名类：

.. code-block:: typescript
   :linenos:

       interface Person {
         name: string
         set surname(s: string)
         get age(): number
       }
       let b: Person = {name: "Bob", surname: "Doe", age: 25}

在上例中，``b`` 的实际运行时类型是为接口 ``Person`` 隐式创建的匿名类。
该匿名类会包含与接口属性对应的字段，例如：

- ``name: string``
- ``surname: string``
- ``age: number``

若属性以 getter 形式定义，则对应字段类型取 getter 的返回类型；
若属性以 setter 形式定义，则对应字段类型取 setter 的参数类型；
若同时定义 getter 和 setter，则两者类型必须一致，否则根据
:ref:`Implementing Required Interface Properties` 会发生
:index:`compile-time error`。

任意可选属性都可以在对象字面量中省略，此时该属性的值为 ``undefined``：

.. code-block:: typescript
   :linenos:

       interface Person {
         name: string
         age: number
         sex?: "male"|"female"
       }
       let b: Person = {name: "Bob", age: 25}
            // 'sex' field has the value 'undefined'

.. index::
   optional property
   non-optional property
   undefined value
   property
   object literal
   compile-time error
   default value

非可选属性不能省略，即使该属性类型本身存在默认值（见 :ref:`Default Values for Types`）也不行；否则会发生
:index:`compile-time error`。

接口类型对象字面量必须为所有没有默认实现的方法提供实现，且这些方法都表现为 ``public``：

.. code-block:: typescript
   :linenos:

       interface I {
         print_name (name: string): void
         print_something() { console.log ("Something") }
       }
       let i: I = {
         print_name (name: string) { console.log(name) }
         // No need to implement print_something()
       }
       i.print_name ("Alice")
       i.print_something()


可选属性可以省略，并会被设为 ``undefined``；非可选属性不能省略。
对象字面量还必须为所有没有默认实现的接口方法提供实现。

在对象字面量方法体中引用 ``this``，表示引用这个匿名类实例本身。
对象字面量还可以提供与接口方法 override-compatible 的实现（见 :ref:`Override-Compatible Signatures`）：

.. code-block:: typescript
   :linenos:

       class Base {}
       class Drv1 extends Base {}
       class Drv2 extends Base {}

       interface A {
           foo (p: Drv1): Base
           foo (p: Drv2): Base
       }
       const a1: A = { foo(p: Base): Drv1 {} }
          /* OK, foo(p: Base) implements both foo (p: Drv1): base and foo (p: Drv2): Base */

       const a2: A = { // OK
          foo(p: Drv1): Drv1 { return new Drv1 } // implements foo (p: Drv1): Base
          foo(p: Drv2): Drv2 { return new Drv2 } // implements foo (p: Drv2): Base
       }

.. code-block:: typescript
   :linenos:

       interface I { method(i: I): I }
       const i: I = { method(i: I): I { return this } }


.. index::
   object literal method
   method
   default implementation
   override-compatible signature
   public method
   this reference
   compile-time error

若接口类型对象字面量引入了新方法，则发生 :index:`compile-time error`。

.. code-block:: typescript
   :linenos:

       interface I {}
       const i: I = { foo(): void {} } // Compile-time error

若接口属性不是可选属性，则对象字面量必须显式提供该属性，即使其类型本身具有默认值。
若接口中同时存在 getter 和 setter，则两者类型必须一致；否则根据
:ref:`Implementing Required Interface Properties` 发生编译时错误。

如果接口方法本身带默认实现，则对象字面量既可以直接使用接口实现，也可以覆盖它：

.. code-block:: typescript
   :linenos:

      interface I {
         method(): void { console.log ("method() from I is called") }
      }

      // Valid literal of anonymous class type using interface method
      const i1: I = {}
      i1.method()

      // Valid literal of anonymous class type using own method declaration
      const i2: I = {
         method(): void { console.log ("method() from object literal is called") }
      }

接口属性在对象字面量中总是通过“赋值一个值”的方式初始化，但接口中的属性定义形式（见 :ref:`Interface Properties`）
决定了后续如何使用该属性：

.. code-block:: typescript
   :linenos:

       interface I1 {
           set attr (attr: number)
       }
       const i1: I1 = {attr: 42} // OK, 'attr' is writable property
       console.log (i1.attr) // Compile-time error as attr has no getter

       interface I2 {
           get attr (): number
       }
       const i2: I2 = {attr: 42} /* OK, 'attr' is in fact a getter which always returns 42 */
       i2.attr = 666 // Compile-time error as attr is readonly
       console.log (i2.attr) // OK, output is 42

       interface I3 {
           readonly attr: number
       }
       const i3: I3 = {attr: 42} /* OK, same as above */
       i3.attr = 666 // Compile-time error as attr is readonly
       console.log (i3.attr) // OK, output is 42

       interface I4 {
           attr: number
       }
       const i4: I4 = {attr: 42} /* OK, getter and setter work with object literal field */
       i4.attr = 666 // OK
       console.log (i4.attr) // OK


.. index::
   interface property
   getter
   setter
   readonly property
   mutable property
   property access
   attribute


.. index::
   interface type
   inferred type
   context
   object literal
   anonymous class
   optional property
   non-optional property
   interface property
   undefined value
   method
   default implementation
   override-compatible signature
   object literal method

.. _Object Literal of Record Type:

``Record`` 类型对象字面量
===============================================================================

.. meta:
    frontend_status: Done

:ref:`Record Utility Type` ``Record<Key, Value>`` 类型的对象字面量使用键值对形式初始化。
键必须满足 ``Key`` 类型要求，值必须满足 ``Value`` 类型要求。

``Record<Key, Value>`` 使用一种特殊对象字面量来初始化，其语法如下：

.. index::
   object literal
   utility type
   record type
   type property
   value type
   key type
   initialization
   value

.. code-block:: abnf

       recordLiteral:
          '{' recordLiteralElementSequence? '}'
          ;

       recordLiteralElementSequence:
          recordLiteralElement (',' recordLiteralElement)* ','?
          ;

       recordLiteralElement:
          expression ':' expression
          |
          spreadExpression
          ;

.. index::
   expression
   key
   value

当 *recordLiteralElement* 表示为一对表达式时，它体现键值对语义。

``keyValue`` 中第一个表达式表示键，必须属于 ``Key``；第二个表达式表示值，
必须属于 ``Value``：

.. code-block:: typescript

       let map: Record<string, number> = {
           "John": 25,
           "Mary": 21,
       }

       console.log(map["John"]) // prints 25

.. code-block:: typescript

       interface PersonInfo {
           age: number
           salary: number
       }
       let map: Record<string, PersonInfo> = {
           "John": { age: 25, salary: 10},
           "Mary": { age: 21, salary: 20}
       }

若键类型是字面量联合，则对象字面量中必须列出所有键；否则发生
:index:`compile-time error`。

.. code-block:: typescript

       let map: Record<"aa" | "bb", number> = {
           "aa": 1,
       } // Compile-time error, "bb" key is missing

当 *recordLiteralElement* 表示为展开表达式时，其类型必须为
``Record<Key, Value>``，且 ``Key`` 类型必须与当前 ``Record`` 类型的 ``Key`` 类型匹配。
此外，被展开 ``Record`` 的 ``Value`` 类型必须是当前 ``Record`` 类型 ``Value`` 类型的子类型。
如果这些条件不满足，则发生 :index:`compile-time error`。展开表达式用于把被展开
``Record`` 中的元素填充到当前 ``Record`` 中。

示例如下：

.. code-block:: typescript

    let map1: Record<string, number> = {"aa": 1, "bb": 2}
    let map2: Record<string, number> = {...map1, "cc": 3}
    // map2 will be the record with {"aa": 1, "bb": 2, "cc": 3} values

    class B {}
    class A extends B {}
    let map3: Record<string, A> = {"aa": new A, "bb": new A}
    let map4: Record<string, B> = {...map3, "cc": new B}
    // map4 will be the record with {"aa": new A, "bb": new A, "cc": new B} values

    let map5: Record<string, number> = {...map3, "cc": 123}
    // Compile-time error, type of Value of map3 is not a subtype of Value of map5

    let map6: Record<number, B> = {...map3, 2: new B}
    // Compile-time error, type of Key of map6 is different from the type of Key of map3

.. index::
   object literal
   generic type
   utility type
   record type
   key type
   value type
   initialization
   key
   value
   union type
   literal

.. _Object Literal Evaluation:

对象字面量求值
===============================================================================

.. meta:
    frontend_status: Done

对象字面量求值会先创建目标对象，再按字段与方法定义顺序处理初始化逻辑。
若某个成员初始化异常完成，则整个对象字面量求值也异常完成。

更具体地说，对于类型为 ``C`` 的对象字面量（``C`` 可以是具名类，也可以是为接口
隐式生成的匿名类），求值步骤如下：

1. 调用类 ``C`` 的无参构造函数来初始化类实例 ``x``；
2. 按源码中的文本顺序，从左到右处理所有 *object literal fields*：

   - 先求值字段对应的表达式；
   - 若求值成功，则把该值赋给 ``x`` 中对应字段作为初始值；
   - 若求值失败，则整个对象字面量异常完成。

因此，只要构造函数执行或任意字段表达式执行异常，整个对象字面量的求值都会异常完成。
只有当类实例成功创建、构造函数执行成功，并且所有字段初始化都成功时，
对象字面量求值才算正常完成。

.. index::
   object literal
   evaluation
   named class
   class
   anonymous class
   interface
   parameterless constructor
   constructor
   instance
   execution
   abrupt completion
   field

对象字面量方法本身不会在对象创建时立即执行；它们只是作为生成类 ``C`` 的成员定义保留下来。
只有在后续显式调用这些方法时，方法体才会被求值。

.. index::
   object literal
   evaluation
   class
   anonymous class
   constructor
   instance
   abrupt completion
   normal completion
   initial value
   textual order
   field initializer
   method declaration

|

.. _Spread Expression:

展开表达式
*******************************************************************************

.. meta:
    frontend_status: Done

展开表达式把可迭代对象、数组、元组或其他允许的结构拆分为一系列元素或实参。
它可用于数组字面量、调用参数以及某些对象构造场景。

展开表达式只能出现在数组字面量（见 :ref:`Array Literal`）、实参传递位置
（见 :ref:`Rest Parameter`），或 ``Record`` 类型对象字面量中
（见 :ref:`Object Literal of Record Type`）。否则发生 :index:`compile-time error`。

其语法如下：

.. code-block:: abnf

       spreadExpression:
           '...' expression
           ;

被展开的 ``expression`` 必须是可迭代类型（见 :ref:`Iterable Types`）或元组类型
（见 :ref:`Tuple Types`）。否则发生 :index:`compile-time error`。

展开表达式的求值分为两种情况：

- 若 ``expression`` 是常量表达式（见 :ref:`Constant Expressions`），则可由编译器在编译期处理；
- 否则在运行时求值。

求值结果是一个值序列。这个序列会被用于数组字面量构造、函数/方法/构造函数调用等场景，
而展开表达式本身的类型，就是该序列中各值类型形成的序列类型。

.. index::
   spread expression
   array literal
   argument
   iterable type
   tuple type
   syntax
   runtime
   compile time
   call
   function
   method
   constructor
   assignment
   sequence of values
   rest parameter

展开表达式和语法上相似的 *rest parameter* 含义不同：

- 展开表达式负责**生成**一段值序列；
- rest 参数负责**接收**一段值序列，并把这些值存入数组或元组。

.. code-block:: typescript
   :linenos:

      function f(...a: number[]) {} // Rest, put caller values into an array

      f(1,2)    // Thanks to Rest, we can put some values directly ...
      let arglist: number[] = [1, 2, 3]
      f(...arglist)  // or  say that all values from 'arglist' must be substituted


若被展开对象只在运行时才确定，那么新的数组字面量也在运行时构造：

.. code-block:: typescript
   :linenos:

      let array1: int[] = [1, 2, 3]
      let array2: FixedArray<int> = [4, 5]

      // A literal contains two spread expressions with arrays of variable and fixed size
      let array3: int[] = [...array1, ...array2] // spread array1 and array2 elements
                                           // while building new array literal at compile time
      console.log(array3) // prints [1, 2, 3, 4, 5]

      function foo (...array: int[]) {
         console.log (array)
      }

      // The next two calls are equivalent
      foo(...array2)
      foo (...[...array2])  // spread array2 elements into arguments of the foo() call

      // recall,  'array3 = [ ..array1, ..array2])' (see above)
      // next two calls are also equivalent
      foo(...array3)
      foo(...[...array1, ...array2])

      function run_time_spread_application1 (a1: int[], a2: FixedArray<int>) {
         console.log ([...a1, 42, ...a2])
         // Array literal is built at runtime
      }
      run_time_spread_application1 (array1, array2) // prints [1, 2, 3, 42, 4, 5]


数组类型的展开同时支持 :ref:`Resizable Array Types` 和
:ref:`Fixed-Size Array Types`。两者可以在数组字面量和函数调用中任意组合：

.. code-block:: typescript
   :linenos:

      let a: int[] = [1, 2, 3]

      function foo (...p: int[]) {
         p[1] = 4
         console.log ("inside foo()", p)
      }
      foo(...a)  // prints 'inside foo() 1,4,3'
      console.log ("outside foo()", a) // prints 'outside foo() 1,2,3'


展开表达式始终复制值，而不会把源数组别名直接传给目标位置：

.. code-block:: typescript
   :linenos:

      let a: readonly int[] = [1, 2, 3]
      let b: int[] = [1, 2, 3]

      // 'readonly' array in spread expr, can modify target array Elements
      let rw: int[] = [...a]
      rw[1] = 1 // OK
      function foo(...p_rw: int[]) {
         p_rw[1] = 1 // OK
      }

      // RW array in spread, readonly target
      let ro: readonly int[] = [...b]
         ro[1] = 1 // Compile-time error
      function foo(...p_ro: readonly int[]) {
         p_ro[1] = 1 // Compile-time error
      }


因此，源数组上的 ``readonly`` 属性（见 :ref:`Readonly Array Types`）不会自动传递给展开后的目标数组。
如果目标需要是只读数组，则必须在目标数组或 rest 参数上显式声明 ``readonly``：

.. code-block:: typescript
   :linenos:

      function accept_spreads_with_rest_parameter (...args: number[]) {
          console.log (args)
      }
      let arr = [1, 2, 3]
      accept_spreads_with_rest_parameter (...arr, ...arr)
         // Output: 1 2 3 1 2 3

      function g1() { return [1, 2] }
      function g2() { return [3, 4, 5] }
      accept_spreads_with_rest_parameter (...g1(), ...g2())
         // Output: 1 2 3 4 5


若展开表达式用于传递调用实参（见 :ref:`Rest Parameter`），则多个连续的展开表达式会拼接成一个单一值序列：

.. code-block:: typescript
   :linenos:

       let tuple1: [number, string, boolean] = [1, "2", true]
       let tuple2: [number, string] = [4, "5"]
        // spread tuple1 and tuple2 elements
       let tuple3: [number, string, boolean, number, string] = [...tuple1, ...tuple2]
          // while building new tuple object at compile time
       console.log(tuple3) // prints [1, 2, true, 4, 5]

       function bar (...tuple: [number, string]) {
         console.log (tuple)
       }
       bar (...tuple2)  // spread tuple2 elements into arguments of the foo() call

       function run_time_spread_application2 (a1: [number, string, boolean], a2: [number, string]) {
         console.log ([...a1, 42, ...a2])
           // Such an array literal is built at runtime
       }
       run_time_spread_application2 (tuple1, tuple2) // prints [1, 2, true, 42, 4, "5"]


元组也支持展开：

.. code-block:: typescript
   :linenos:

       class A<T> implements Iterable<T|undefined> { // variables of type A can be spread
           // To check code with TS, comment line with  `$_iterator()`
           // and uncomment one with `[Symbol.iterator]()`
           $_iterator(): Iterator<T|undefined>  {
           // [Symbol.iterator](): Iterator<T|undefined>  {
             return new MyIteratorResult<T|undefined>(this.data)
           }
           private data: T[]
           constructor (...data: T[]) { this.data = data }
       }
       class MyIteratorResult <T> implements Iterator<T|undefined> {
           private data: T[]
           private index: number = 0
           next(): IteratorResult<T|undefined> {
               if (this.index >= this.data.length) {
                  return MyIteratorResult.end_of_sequence
               } else {
                  this.current_element.value = this.data[this.index++]
                  return this.current_element
               }
           }
           constructor (data: T[]) { this.data = data }
           private static end_of_sequence: IteratorResult<undefined> = {done: true, value: undefined}
           private current_element: IteratorResult<T|undefined> = {done: false, value: undefined}
       }
       function display<T> (...p: T[]) { console.log (p) }
       display (... new A<number> (1, 2, 3))        // Spread A with numbers
       display (... new A<string> ("aaa", "bbb"))   // Spread A with strings
       display (... new A<Object> (1, "aaa", true)) // Spread A with any objects
       display (... new A<undefined>)               // Spread A with no objects

       type UnionOfIterable = A<number> | new A<string>
       function show (...p: UnionOfIterable) { console.log (p) }
       show (... new A<number> (1, 2, 3))        // Spread A with numbers
       show (... new A<string> ("aaa", "bbb"))   // Spread A with strings


对于实现了 ``Iterable`` 的类实例，也允许使用展开表达式把迭代结果拆成值序列：

.. code-block:: typescript
   :linenos:

          function foo1 (n1: number, n2: number) // non-rest parameters
             { ... }
          let an_array = [1, 2]
          foo1 (...an_array) // Compile-time error

          function foo2 (n1: number, n2: string)  // non-rest parameters
             { ... }
          let a_tuple: [number, string] = [1, "2"]
          foo2 (...a_tuple) // Compile-time error

.. note::
   在调用点展开实参时，对应形参必须是 rest 参数（见 :ref:`Rest Parameter`）。
   不能把展开结果直接喂给一组普通非可选参数，否则发生 :index:`compile-time error`。

.. index::
   call site
   argument
   spread
   rest parameter
   parameter
   tuple
   array
   non-optional parameter
   iterable type
   readonly array

|

.. _Parenthesized Expression:

括号表达式
*******************************************************************************

.. meta:
    frontend_status: Done

括号表达式的语法如下：

.. code-block:: abnf

       parenthesizedExpression:
           '(' expression ')'
           ;

括号表达式不会改变被包裹表达式的值与类型，只会显式改变语法分组与求值顺序。

.. index::
   parenthesized expression
   syntax
   value
   type
   contained expression

|

.. _this Expression:

``this`` 表达式
*******************************************************************************

.. meta:
    frontend_status: Done

关键字 ``this`` 在实例方法、访问器、构造函数、允许使用 ``this`` 的字段初始化器、
以及某些 lambda/receiver 场景中表示当前对象。
在显式构造函数调用语句（见 :ref:`Explicit Constructor Call`）中，
``this(`` *arguments* ``)`` 形式的直接调用也使用 ``this`` 关键字。
在其他不允许的上下文中使用 ``this`` 会产生 :index:`compile-time error`。

其语法如下：

.. code-block:: abnf

       thisExpression:
           'this'
           ;

``this`` 可以在以下位置作为表达式使用：

- 类实例方法体中（见 :ref:`Method Body`）；
- 接口默认方法体中（见 :ref:`Default Interface Method Declarations`）；
- 对象字面量方法体中（见 :ref:`Object Literal`）；
- 构造函数体中（见 :ref:`Constructor Body`）；
- 允许使用 ``this`` 的字段初始化器中（见 :ref:`Field Initialization`）；
- 带 receiver 的函数体中（见 :ref:`Functions with Receiver`）；
- 以及允许继承外围 ``this`` 语义的 lambda 中。

若方法声明在对象字面量中，则 ``this`` 的类型是该对象字面量推导得到的类型。
若 ``this`` 出现在带 receiver 的函数中，则它的类型就是该函数 ``this`` 参数声明的类型。

在 lambda 中，若外围上下文本来允许使用 ``this``，那么 lambda 里的 ``this``
与外围上下文中的 ``this`` 表示同一个值。

.. index::
   this keyword
   expression
   instance method
   constructor
   lambda expression
   function with receiver
   declared type
   object literal
   context

.. index::
   this keyword
   primary expression
   value
   instance method
   instance method call
   object
   parameter
   lambda body
   context
   subclass
   subtyping
   class
   runtime
   class type

.. code-block:: typescript
   :linenos:

       interface anInterface {
           method() {
              this // type of 'this' is anInterface
           }
       }
       class aClass implements anInterface {
           method() {
              this // type of 'this' is aClass
           }
           field = (): void => {
              this // type of 'this' is aClass
           }
       }
       class AnotherClass {
           anotherMethod() {
               const obj: aClass = { // Object literal
                 method () {
                     this // type of 'this' is aClass
                 },
                 field: (): void => {
                     this // type of 'this' is AnotherClass
                 }
               }
           }
       }

`this` 作为主表达式时，运行时指向以下两类对象之一：

- 当前实例方法调用所针对的对象；
- 正在构造中的对象。

若 ``this`` 出现在 lambda 中，则 lambda 体内的 ``this`` 与外围上下文中的 ``this``
表示同一个值，而不会因为 lambda 自身而重新绑定。

运行时 ``this`` 所引用对象的实际类可以正好是静态类型 ``T``，也可以是 ``T`` 的某个子类
（参见 :ref:`Subtyping`）。

.. index::
   this keyword
   primary expression
   runtime
   object being constructed
   lambda body
   subclass

|

.. _Field Access Expression:

字段访问表达式
*******************************************************************************

.. meta:
    frontend_status: Done

字段访问表达式用于读取对象或类型上的字段和访问器。
对象引用必须可访问且与目标成员匹配；若访问链中出现空值，还要结合 chaining 规则。

.. index::
   field access expression
   access
   field
   object reference
   superclass
   syntax

字段访问表达式既可以访问类静态字段（见 :ref:`Accessing Static Fields`），
也可以访问对象引用所指向对象上的实例字段。对象引用的具体形式会进一步落到
:ref:`Accessing Current Object Fields` 或
:ref:`Accessing SuperClass Accessors` 的规则上。

其语法如下：

.. code-block:: abnf

       fieldAccessExpression:
           objectReference ('.' | '?.') identifier
           ;

包含运算符 "?."
（见 :ref:`链式运算符 <Chaining Operator>`）的字段访问称为安全字段访问（safe field access），因为它会对 nullish 对象引用做安全处理。

若对象引用求值异常完成，则整个字段访问表达式也异常完成。
用于字段访问的对象引用必须是非 nullish 的引用类型 ``T``；否则发生
:index:`compile-time error`。

当 ``identifier`` 能解析为类型 ``T`` 中某个可访问成员字段（见 :ref:`Accessible`）时，字段访问合法；
否则发生 :index:`compile-time error`。字段访问表达式的类型，就是该成员字段的类型。

如果 ``identifier`` 命中的是类（见 :ref:`Class Accessor Declarations`）或接口类型（见 :ref:`Interface Properties`）
中定义的访问器，则在读取位置会调用 getter，在赋值左侧位置会调用 setter。

因此，字段访问既可能解析为真实字段，也可能解析为访问器调用；这一点由静态类型 ``T``
中的成员定义决定，而不是由运行时对象的实际类临时决定。

.. index::
   field access expression
   access
   field
   safe field access
   object reference
   abrupt completion
   non-nullish type
   identifier
   accessible member field
   accessibility
   accessor
   getter
   setter
   reference type

.. _Accessing Static Fields:

访问静态字段
===============================================================================

.. meta:
    frontend_status: Done

访问静态字段时，对象引用部分应当是类型引用。
静态字段的结果属于类级存储，而非某个具体对象实例。

更具体地说，访问静态字段时，``objectReference`` 必须采取 ``typeReference`` 形式。

静态字段访问表达式的结果是以下两者之一：

- 若字段不是 ``readonly``，结果是一个后续仍可修改的 ``variable``；
- 若字段是 ``readonly``，结果是一个 ``value``，但在静态初始化块（见 :ref:`Static Initialization`）中访问时除外。

.. index::
   access
   static field access
   static field
   field access expression
   readonly field
   initializer block
   runtime
   evaluation
   variable
   value

.. _Accessing Current Object Fields:

访问当前对象字段
===============================================================================

.. meta:
    frontend_status: Done

当前对象字段访问以 ``this`` 或等价实例引用为基础。
最终访问到的成员既可能是字段，也可能是与该名称关联的访问器。

访问当前对象字段时，``objectReference`` 采取 ``primaryExpression`` 形式。
字段访问表达式的运行时求值从 ``primaryExpression`` 的求值开始。

求值结果可能对应：

- 非 ``readonly`` 字段，此时结果是后续仍可修改的 ``variable``；
- ``readonly`` 字段，此时结果是 ``value``，但在构造函数（见 :ref:`Constructor Declaration`）中访问时除外；
- getter 调用，当字段访问不处于赋值左侧时；
- setter 调用，当字段访问处于赋值左侧时。

决定访问哪个字段或属性时，只看 ``primaryExpression`` 的静态类型，
而不看运行时实际对象的类。

.. index::
   instance field access
   field access expression
   field access
   primary expression
   readonly field
   getter
   setter
   static type
   runtime
   variable
   value
   class type
   constructor

.. _Accessing SuperClass Accessors:

访问超类访问器
===============================================================================

.. meta:
    frontend_status: Done

``super.identifier`` 形式主要用于访问超类中的访问器成员。
若标识符无法解析到合法的超类成员，则会产生编译时错误。

更准确地说，``super.identifier`` 只允许用来访问超类访问器
（见 :ref:`Class Accessor Declarations`）。如果 ``identifier`` 指向的是字段，
则发生 :index:`compile-time error`。

.. code-block:: typescript
   :linenos:

       class Base {
          get property(): number { return 1 }
          set property(p: number) { }
          field = 1234
       }
       class Derived extends Base {
          get property(): number { return super.property } // OK
          set property(p: number) { super.property = 42 }  // OK
          foo () {
             super.field = 42           // Compile-time error
             console.log (super.field)  // Compile-time error
          }
       }

.. index::
   superclass accessor
   super
   accessor
   compile-time error

|

.. _Method Call Expression:

方法调用表达式
*******************************************************************************

.. meta:
    frontend_status: Done

方法调用表达式用于调用实例方法、静态方法以及某些特殊成员。

.. index::
   method call expression
   static method
   instance method
   function with receiver
   class
   interface
   call

它可以调用：

- 类的静态方法；
- 类或接口的实例方法；
- 带 receiver 的函数（参见 :ref:`Functions with Receiver`）。

其语法如下：

.. code-block:: abnf

       methodCallExpression:
           objectReference ('.' | '?.') identifier typeArguments? callArguments
           ;

*调用实参* 见 :ref:`Call Arguments`。

使用运算符 "?."
（见 :ref:`链式运算符 <Chaining Operator>`）的方法调用称为安全方法调用，因为它会安全处理 nullish 值。
编译期会按以下三个步骤确定并检查目标实体：

- :ref:`Step 1 Selection of Type to Use`
- :ref:`Step 2 Selection of Entity to Call`
- :ref:`Step 3 Checking Modifiers`

.. index::
   method call expression
   static method
   instance method
   function with receiver
   class
   interface
   call
   syntax
   method call
   chaining operator
   safe method call
   nullish value
   compile time

.. _Step 1 Selection of Type to Use:

步骤 1：选择要使用的类型
===============================================================================

.. meta:
    frontend_status: Done

首先根据对象引用或类型引用确定在哪个类型中查找候选方法。
可接受的 *对象引用* 形式有三种：

.. table::
   :widths: 30, 70

   ============================== =================================================================
    对象引用形式                     用于查找的方法所属类型
   ============================== =================================================================
   ``typeReference``               ``typeReference`` 所表示的类型必须是类类型；
                                   否则发生 :index:`compile-time error`。
   ``super``                       包含当前方法调用的类的直接超类。
   类型为 *T* 的表达式             若 ``T`` 是类、接口或联合类型，则使用 ``T``；
                                   若 ``T`` 是类型参数，则使用其约束
                                   （参见 :ref:`Type Parameter Constraint`）；
                                   否则发生 :index:`compile-time error`。
   ============================== =================================================================

.. index::
   type
   type parameter
   object reference
   method
   constraint
   superclass
   method call
   class
   interface
   union

.. _Step 2 Selection of Entity to Call:

步骤 2：选择要调用的实体
===============================================================================

.. meta:
    frontend_status: Partly

在目标类型确定后（见 :ref:`Step 1 Selection of Type to Use`），需要依据名称、可访问性、是否重载以及实参与形参兼容关系，
构造候选集合。候选集合取决于对象引用形式、用于查找的类型以及方法名：

.. table::
   :widths: 30, 70

   ================================= =================================================================
    对象引用形式                       候选调用实体集合
   ================================= =================================================================
   ``typeReference`` of type *T*      类 *T* 中名为 ``identifier`` 的静态方法。
   ``super``                          当前类直接超类中名为 ``identifier`` 的实例方法。
   类型为 *T* 的表达式                类或接口 *T* 中名为 ``identifier`` 的实例方法，
                                      以及 receiver 类型为 *T* 的同名带 receiver 函数（见 :ref:`Functions with Receiver`）。
                                      若 *T* 是联合类型，则使用公共实例成员集合
                                      （参见 :ref:`Access to Common Union Members`）。
   ================================= =================================================================

若候选集合为空，即没有任何可调用实体，则发生 :index:`compile-time error`。
若候选集合包含多个实体，则使用 :ref:`Overload Resolution` 进行选择；
方法调用场景下重载集的形成见 :ref:`Overload Set at Method Call`。

若选中的目标是实例方法，则运行时还会按
:ref:`Dynamic resolution of method calls` 进行动态分派。

因此，方法调用的语义可以分成两层：编译期先做候选选择与重载决议，
运行期再对实例方法执行动态分派。

.. index::
   overload resolution
   call
   method call
   candidate set
   dynamic dispatch

.. _Step 3 Checking Modifiers:

步骤 3：检查修饰符
===============================================================================

.. meta:
    frontend_status: Done

在此步骤中，待调用的具体方法已经唯一确定。接下来必须根据方法调用的具体形式，
继续执行一组修饰符语义检查，以确认是否违反 ``abstract``、``private``、
``protected``、``static`` 等规则。具体如下：

- ``typeReference.identifier``
  目标方法必须声明为 ``static``，否则发生 :index:`compile-time error`。
- ``expression.identifier``
  目标方法不得声明为 ``static``，否则发生 :index:`compile-time error`。
- ``super.identifier``
  目标方法不得声明为 ``abstract`` 或 ``static``，否则发生
  :index:`compile-time error`。

方法调用的实参语义检查统一按 :ref:`Compatibility of Call Arguments` 进行。

.. index::
   method
   method modifier
   call
   class
   static method
   method call
   semantic check
   static method call
   abstract method
   abstract method call

.. _Type of Method Call Expression:

方法调用表达式的类型
===============================================================================

.. meta:
    frontend_status: Done

方法调用表达式的类型就是被调用方法的返回类型。
示例如下：

.. code-block:: typescript
   :linenos:

       class A {
          static method() { console.log ("Static method() is called") }
          method()        { console.log ("Instance method() is called") }
       }


       let x = A.method()     // Compile-time error as void cannot be used as type annotation
       A.method ()            // OK
       let y = new A().method() // Compile-time error as void cannot be used as type annotation
       new A().method()         // OK

若使用安全方法调用 ``?.``，而对象引用在运行时求值为 nullish 值，
则调用实参与方法体都不会继续执行，整个调用以安全短路方式结束。

.. index::
   method call expression
   candidate set
   overload resolution
   dynamic dispatch
   safe method call
   return type

|

.. _Function Call Expression:

函数调用表达式
*******************************************************************************

.. meta:
    frontend_status: Done

函数调用表达式作用于函数值、函数引用、可调用类型或其他允许调用的实体。
它用于调用函数（参见 :ref:`Function Declarations`）、函数类型变量
（参见 :ref:`Function Types`）或 lambda 表达式（参见
:ref:`Lambda Expressions`）。

其语法如下：

.. code-block:: abnf

       functionCallExpression:
           expression ('?.' | typeArguments)? callArguments
           ;

*调用实参* 见 :ref:`Call Arguments`。

若被调用表达式的类型满足以下任一条件，则发生 :index:`compile-time error`：

- 不是函数类型；
- 是 nullish 类型且未使用 ``?.`` （参见 :ref:`Chaining Operator`）。

若使用 ``?.`` （见 :ref:`Chaining Operator`）且该表达式在运行时求值得到 nullish 值，则：

- 不再求值 *调用实参*；
- 不执行调用；
- 不产生函数调用表达式的结果值。

这就是 *安全函数调用* 的语义：它通过短路方式安全处理 nullish 值。

.. index::
   function call expression
   expression
   function call
   function type
   trailing lambda call
   lambda expression
   expression type
   syntax
   nullish type
   chaining operator
   block

若调用中的表达式形式是 *qualifiedName*，并且它引用的是重载函数
（参见 :ref:`Implicit Function Overloading` 和 :ref:`Explicit Function Overload`），
则必须通过 :ref:`Overload Resolution` 选择要调用的具体函数。

若没有任何可调用函数可选，则发生 :index:`compile-time error`。

调用的语义检查按 :ref:`Compatibility of Call Arguments` 进行。

.. index::
   call
   qualified name
   overload resolution
   explicit overload declaration
   function
   semantic check
   compatibility
   call argument

不同形式的函数调用如下：

.. code-block:: typescript
   :linenos:

       function foo() { console.log ("Function foo() is called") }
       foo() // function call uses function name to call it

       call (foo)            // top-level function passed
       call ((): void => { console.log ("Lambda is called") }) // lambda is passed
       call (A.method)       // static method
       call ((new A).method) // instance method is passed

       class A {
          static method() { console.log ("Static method() is called") }
          method() { console.log ("Instance method() is called") }
       }

       function call (callee: () => void) {
          callee() // function call uses parameter name to call any functional object passed as an argument
       }

       ((): void => { console.log ("Lambda is called") }) () // function call uses lambda expression to call it

       let x = foo() // Compile-time error as void cannot be used as type annotation

.. index::
   call
   expression
   qualified name
   overload resolution
   explicit overload declaration
   function
   semantic correctness check

函数调用表达式的类型，是被调用函数的返回类型。

同一个调用位置既可以直接调用顶层函数，也可以调用参数中传入的函数值、
lambda 表达式、静态方法引用或实例方法引用。只要目标的静态类型满足函数类型要求，
函数调用表达式就统一按相同的调用规则和实参兼容规则处理。

.. index::
   function call expression
   function call
   call
   static method
   instance method
   function type
   nullish type
   safe function call
   overload resolution
   return type
   function
   parameter
   functional object
   argument
   callee
   lambda expression
   type annotation

.. _Call Arguments:

调用实参
===============================================================================

.. meta:
    frontend_status: Done

调用实参的语法由位置参数、可选参数、rest 参数与展开表达式共同构成。
参数兼容性由语义章节中的 Compatibility of Call Arguments 给出。

它描述的是一次调用中按文本顺序写出的实参列表。实参数量是否允许省略、
是否允许额外值、展开表达式能否出现，以及 trailing lambda 是否可以写在右括号之后，
最终都由被调用实体的形参列表和
:ref:`Compatibility of Call Arguments` 一起决定。

其语法如下：

.. code-block:: abnf

       callArguments:
           '(' argumentSequence? ')' trailingLambda?
           ;

       argumentSequence:
           expression (',' expression)* ','?
           ;

`callArguments` 语法规则表示调用实参列表。只有对应某个 *rest parameter* 的实参，
才允许是展开表达式（见 :ref:`Spread Expression`）。

*trailing lambda call* 是一种特殊调用形式，其中实参列表包含 *trailing lambda*
（详见 :ref:`Trailing Lambdas`）。

.. index::
   argument
   call argument
   syntax
   expression
   call
   spread expression
   trailing lambda
   rest parameter
   argument sequence
   parameter list

|

.. _Indexing Expressions:

索引表达式
*******************************************************************************

.. meta:
    frontend_status: Done

索引表达式 ``obj[index]`` 包含两个子表达式：对象引用与索引值。
不同的被索引目标会触发不同的静态与运行时规则。

索引表达式用于访问数组元素（见 :ref:`Array Types`）、字符串（见 :ref:`Type string`）元素、
:ref:`Record Utility Type` 实例和其他可索引类型（见 :ref:`Indexable Types`）实例。

其语法如下：

.. code-block:: abnf

       indexingExpression:
           expression ('?.')? '[' expression ']'
           ;

任何索引表达式都包含两个子表达式：

- 左方括号之前的对象引用表达式；
- 方括号中的索引表达式。

.. index::
   indexing expression
   indexable type
   access
   array element
   string
   record
   utility type
   array type
   subexpression
   object reference expression
   index expression
   bracket

若索引表达式中使用了 ``?.`` 运算符（见 :ref:`Chaining Operator`），则：

- 如果对象引用表达式不是 nullish 类型，链式运算符没有效果；
- 否则要先检查对象引用表达式的值是否为 ``undefined`` 或 ``null``。
  若是，则整个外围 primary expression 的求值停止，结果为 ``undefined``。

若未使用 ``?.``，则对象引用表达式必须是数组类型或 ``Record`` 类型；
否则发生 :index:`compile-time error`。

.. index::
   chaining operator
   operator
   indexing expression
   object reference expression
   primary expression
   nullish type
   array type
   record type
   reference expression
   nullish value

.. _Array Indexing Expression:

数组索引表达式
===============================================================================

.. meta:
    frontend_status: Partly

数组索引要求索引值属于允许的整数语义范围。
若索引越界，则在运行时可能抛出相应错误。

数组索引中的索引表达式必须属于 ``byte``、``short`` 或 ``int`` 这些整数类型，
否则发生 :index:`compile-time error`。

.. index::
   array indexing
   integer type
   index expression
   runtime error
   compilation

``byte`` 和 ``short`` 会先做 :ref:`Widening Numeric Conversions` 提升到 ``int``；若整数类型提升后结果不是 ``int``，则发生 :index:`compile-time error`。
其他数值类型（如 ``long``、``float``、``double``/``number``）必须显式调用 :ref:`Standard Library` 中的转换方法：

.. code-block:: typescript
   :linenos:

       const a = ["Alice", "Bob", "Carol"]
       function demo (l: long, f: float, d: double, n: number) {
           console.log (
              a[l.toInt()], a[f.toInt()],
              a[d.toInt()], a[n.toInt()]
           ) // OK to access array using index expression conversion methods
       }


若存在链式运算符 ``?.``（见 :ref:`Chaining Operator`），且应用后对象引用表达式的类型仍为数组类型，
则该表达式是合法的数组引用表达式，其结果类型为 ``T``。

数组索引表达式的结果是类型 ``T`` 的变量位置，也就是由索引值选出的数组元素。

.. index::
   conversion
   type
   numeric types conversion
   widening conversion
   index expression
   chaining operator
   numeric type
   object reference expression
   method
   class
   array

若 ``T`` 是引用类型，则通过这个结果值可以继续修改数组元素内部字段：

.. code-block:: typescript
   :linenos:

       let names: string[] = ["Alice", "Bob", "Carol"]
       console.log(names[1]) // prints Bob
       names[1] = "Martin"
       console.log(names[1]) // prints Martin

       console.log (names["1"]) // Compile-time error as index of non-numeric type

       class RefType {
           field: number = 42
       }
       const objects: RefType[] = [new RefType(), new RefType()]
       const obj = objects [1]
       obj.field = 777            // change the field in the array element
       console.log(objects[0].field) // prints 42
       console.log(objects[1].field) // prints 777

       let an_array = [1, 2, 3]
       let element = an_array [3.5] // Compile-time error as index is not integer
       function foo (index: number) {
          let element = an_array [index] // Compile-time error as index is not integer
       }

运行时，数组索引按如下顺序执行：

- 先求值对象引用表达式；
- 若其异常完成，则整个索引表达式异常完成，且索引表达式不再求值；
- 否则求值索引表达式；
- 若索引值小于 0 或大于等于数组长度，则抛出 ``RangeError``；
- 否则结果就是数组中对应元素的变量位置。

.. code-block:: typescript
   :linenos:

       function setElement(names: string[], i: int, name: string) {
           names[i] = name // runtime error, if 'i' is out of bounds
       }

.. index::
   array indexing
   integer type
   index expression
   runtime error
   widening conversion
   array type
   array indexing expression
   reference type
   RangeError
   object reference expression
   abrupt completion
   variable
   field

.. _String Indexing Expression:

字符串索引表达式
===============================================================================

.. meta:
    frontend_status: Partly

字符串索引会返回 ``string`` 类型结果。
若索引超出合法范围，结果和错误行为由语言定义的字符串索引规则决定。

字符串索引中的索引表达式也必须是 ``byte``、``short`` 或 ``int``，
并遵循与 :ref:`Array Indexing Expression` 相同的规则。

.. index::
   string indexing
   index expression
   integer type
   array indexing expression
   string
   string length
   value
   type

若索引值小于 0 或大于等于字符串长度，则抛出 ``RangeError``。
字符串索引表达式的结果类型是 ``string``。

.. code-block:: typescript
   :linenos:

       console.log("abc"[1]]) // prints: b
       console.log("abc"[3]]) // runtime exception

.. note::
   字符串值不可变，因此不能通过索引修改字符串元素：

   .. code-block:: typescript
      :linenos:

             let x = "abc"
             x[1] = "d" // Compile-time error, string value is immutable

.. index::
   string indexing
   index expression
   integer type
   array indexing expression
   string
   string length
   value
   type
   string type
   string value
   string element
   indexing

.. _Record Indexing Expression:

``Record`` 索引表达式
===============================================================================

.. meta:
    frontend_status: Done

对 :ref:`Record Utility Type` ``Record<Key, Value>`` 的索引既可能得到 ``Value``，
也可能得到 ``Value | undefined``，具体取决于键类型与完整性信息。

对 ``Record<Key, Value>`` 的索引要分两种情况：

1. 若 ``Key`` 是只包含字面量类型的联合，则索引表达式只能是这些字面量之一（否则发生 :index:`compile-time error`），
   结果类型是 ``Value``；
2. 其他情况中，索引表达式没有该限制，结果类型是 ``Value | undefined``。

.. code-block:: typescript
   :linenos:

       type Keys = 'key1' | 'key2' | 'key3'

       let x: Record<Keys, number> = {
           'key1': 1,
           'key2': 2,
           'key3': 4,
       }
       let y = x['key2'] // y value is 2

.. code-block:: typescript
   :linenos:

       console.log(x['key4']) // Compile-time error
       x['another key'] = 5 // Compile-time error

编译器会保证对于这种 ``Key`` 类型，``Record<Key, Value>`` 对象一定为所有合法键都持有值。

.. index::
   index expression
   indexing expression
   record type
   utility type
   value
   key type
   literal type
   union type
   literal
   type
   compile-time error

.. index::
   index expression
   key type
   indexing expression
   literal
   compiler
   restriction

在第二种情形中，索引表达式没有额外限制，结果类型是 ``Value | undefined``。

.. code-block:: typescript
   :linenos:

       let x: Record<number, string> = {
           1: "hello",
           2: "buy",
       }

       function foo(n: number): string | undefined {
           return x[n]
       }

       function bar(n: number): string {
           let s = x[n]
           if (s == undefined) { return "no" }
           return s!
       }

       foo(3) // prints "undefined"
       bar(3) // prints "no"

       let y = x[3]

.. index::
   index expression
   literal
   key
   string
   compiler
   value

上例中，``y`` 的类型是 ``string | undefined``，其值为 ``undefined``。

运行时，``Record`` 索引表达式按如下顺序求值：

- 先求值对象引用表达式；
- 若对象引用求值异常完成，则整个索引表达式异常完成，且索引表达式不再求值；
- 若对象引用正常完成，则继续求值索引表达式；
- 若 ``record`` 中存在该键，则结果是映射到该键的值；
- 否则结果是字面量 ``undefined``。

当索引表达式本身是 ``string`` 类型的标识符时，``Record`` 索引还可以写成字段访问形式（见 :ref:`Field Access Expression`）：

.. code-block:: typescript
   :linenos:

       type Keys = 'key1' | 'key2' | 'key3'

       let x: Record<Keys, number> = {
           'key1': 1,
           'key2': 2,
           'key3': 4,
       }
       console.log(x.key2) // the same as console.log(x['key2'])
       x.key2 = 8          // the same as x['key2'] = 8
       console.log(x.key2) // the same as console.log(x['key2'])


.. index::
   string
   undefined
   evaluation
   expression
   type
   value
   key
   record indexing expression
   object reference expression
   abrupt completion
   normal completion
   record instance
   mapped value
   field
   field access expression
   identifier
   string type

|

.. _Chaining Operator:

链式运算符
*******************************************************************************

.. meta:
    frontend_status: Done

链式运算符 ``?.`` 用于在可能为 ``null`` 或 ``undefined`` 的对象上安全地访问字段、
方法、函数调用或索引。它可用于以下上下文：

- :ref:`Field Access Expression`
- :ref:`Method Call Expression`
- :ref:`Function Call Expression`
- :ref:`Indexing Expressions`

.. index::
   chaining operator
   field access
   access
   value
   nullish type
   context
   function call
   indexing expression
   expression
   undefined
   null
   method call

若 ``expr?.`` 中 ``expr`` 的类型是 *nullish type*，并且运行时求值结果为 ``undefined`` 或
``null``，则整个外围 *primary expression* 的求值会立刻停止，结果为 ``undefined``。
此时整个 primary expression 的类型为 ``undefined | T``，其中 ``T`` 是该 primary expression
原本的非 nullish 类型。

.. index::
   chaining operator
   field access
   access
   value
   nullish type
   context
   function call
   indexing expression
   expression
   undefined
   null
   method call
   primary expression
   non-nullish type

.. code-block:: typescript
   :linenos:

       class Person {
           name: string
           spouse?: Person = undefined
           constructor(name: string) {
               this.name = name
           }
       }

       let bob = new Person("Bob")
       console.log(bob.spouse?.name) // prints "undefined"
          // type of bob.spouse?.name is undefined|string

       bob.spouse = new Person("Alice")
       console.log(bob.spouse?.name) // prints "Alice"
          // type of bob.spouse?.name is undefined|string

若 ``expr?.`` 中 ``expr`` 不是 nullish 类型，则链式运算符没有语义效果，也不会改变整个
primary expression 的类型：

.. code-block:: typescript
   :linenos:

       function foo(s1: string, s2: string | undefined) {
           let a = s1?.[0] // 's' is of non-nullish type, type of 'a' is string
           let b = s2?.[0] // type of 'b' is string | undefined
       }

链式运算符只能用于实例方法调用；对静态方法使用 ``?.`` 虽然语法上可写，但会导致
:index:`compile-time error`：

.. code-block:: typescript
   :linenos:

      class A {
         static f(): string {return "" }
         g(): string  { return "" }
      }

      let s: string|undefined

      s = A?.f()            // static method, compile-time error

      let b = new A
      s = b?.g()            // non-static method, OK

若在期望变量位置的上下文里使用链式运算符，也会发生 :index:`compile-time error`，
例如赋值左值（见 :ref:`Assignment`）或自增/自减表达式的操作数
（见 :ref:`Postfix Increment`、:ref:`Postfix Decrement`、:ref:`Prefix Increment`、
:ref:`Prefix Decrement`）。

若编译器能在编译期确定链前表达式恒为 nullish 值，或恒为非 nullish 值，
则会发出 :index:`compile-time warning`：

.. code-block:: typescript
   :linenos:

       class C { f = 1}

       let c = new C()
       c?.f // warning, expression is always non-nullish

       let d: C | undefined = undefined
       d?.f // warning, expression is always evaluated as undefined

|

.. _New Expressions:

``new`` 表达式
*******************************************************************************

.. meta:
    frontend_status: Done

``new`` 表达式有两种语法形式：创建类实例，或创建数组实例。它的结果类型因此只能是
类类型或数组类型。数组实例创建是一项实验性特性（参见 :ref:`Resizable Array Creation Expressions`）。

.. index::
   new expression
   expression
   expression type
   class
   array
   instance
   instantiation
   class instance expression
   object
   array instance
   array creation expression

.. code-block:: abnf

       newExpression:
           newClassInstance
           | newArrayInstance
           ;

其中，类实例创建表达式的语法如下：

.. code-block:: abnf

       newClassInstance:
           'new' typeReference typeArguments? arguments?
           ;

.. index::
   class instance expression
   class instance creation expression
   syntax
   instantiation
   instance
   class
   constructor
   argument
   initialization

类实例创建表达式给出要实例化的类，并可选地给出构造函数的全部实参。
其求值分两步进行：

1. 创建该类的新实例；
2. 调用构造函数以完成初始化。

.. code-block:: typescript
   :linenos:

       class A {
          constructor(p: number) {}
       }

       new A(5) // create an instance and call constructor
       const a = new A(6) /* create an instance, call constructor and store
                             created and initialized instance in 'a' */


构造函数调用的有效性与方法调用类似（详见 :ref:`Step 2 Selection of Entity to Call`），
差异部分由 :ref:`Constructor Body` 规定。若 ``typeReference`` 是类型形参，则发生 :index:`compile-time error`。

类实例创建表达式也可能抛出错误（见 :ref:`Error Handling`、:ref:`Constructor Declaration`）。

当 new 表达式指向 FixedArray、Array 或派生自 Array 的类，且其元素类型是某个类类型时，
该表达式也属于一种特殊的数组创建表达式，详见下列实验性特性小节：

- :ref:`Fixed-Size Array Creation`
- :ref:`Resizable Array Creation Expressions`

试图创建元素类型为类型形参的 ``FixedArray`` 会导致 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       class A<T> {
          foo (element: T) {
             const a1 = new T[5] (element)  // OK, resizable array with 5 elements
                                            // is created
             const a2 = new FixedArray<T> (5, element) // compile-time error, fixed-size
                                            // array with 5 elements of type T cannot be
                                            // created
          }
       }


.. note::
   若无参的类实例创建表达式被用作方法调用中的对象引用，则必须显式写出空括号
   ``()``，否则 `new` 会和后续的成员访问结合出不同语义。

   .. code-block:: typescript
      :linenos:

             class A {  method() {} }

             new A.method()   // Compile-time error
             new A().method() // OK
             (new A).method() // OK
             let a = new A    // OK


.. index::
   new expression
   class instance creation expression
   array creation expression
   constructor
   constructor call
   instantiation
   type parameter
   parentheses

|

.. _instanceof Expression:

``instanceof`` 表达式
*******************************************************************************

.. meta:
    frontend_status: Done

``instanceof`` 表达式的语法如下：

.. code-block:: abnf

       instanceOfExpression:
           expression 'instanceof' type
           ;

形如 ``expr instanceof T`` 的表达式类型始终为 ``boolean``。

.. index::
   syntax
   instanceof expression
   boolean
   operand
   operator
   instanceof operator

若对 ``expr`` 求值得到的值的 *实际类型* 是 ``T`` 的子类型，则结果为 ``true``；否则结果为
``false``。子类型关系见 :ref:`Subtyping`。

若类型 ``T`` 在 :ref:`Type Erasure` 后不是“保留到 undefined”的类型，则会发生
:index:`compile-time error`。

泛型类型（见 :ref:`Generics`）可以作为 ``instanceof`` 的右操作数，但只能以类型名形式出现，参见
:ref:`Type References`。这时真正参与检查的是该泛型类型擦除（:ref:`Type Erasure`）后的形式。

.. code-block:: typescript
   :linenos:

      class A<T> {}
      class B<T> extends A<T> {}

      function foo<T>(a: A<T>) {
         let c = a as B<T>   // OK
         let x = new B<string> // OK, explicit type parameter
         console.log(x instanceof B)        // OK
         console.log(x instanceof B<T>)     // Compile-time error, B<T> is not preserved up to undefined

         if(a instanceof B) {  // OK, type of instanceof is used for type
                               // checking in `if` clause
            let b = a as B<T>  // OK
         }
      }

      let a = new B<string>()
      foo(a)

若编译器在编译期就能证明某个 ``instanceof`` 表达式运行时恒为 ``true`` 或恒为 ``false``，
则会发出 :index:`compile-time warning`：

.. code-block:: typescript
   :linenos:

       class C {}
       class D extends C{}
       class E {}

       function foo(d: D) {
           console.log(d instanceof C) // warning, expression is always true
           console.log(d instanceof E) // warning, expression is always false
       }

当上下文需要时，``instanceof`` 中的类型信息还会参与后续类型检查。

.. index::
   subtype
   type
   evaluation
   subtyping
   type erasure
   type reference
   semantic check
   type cast
   generic type
   type name
   type parameter

|

.. _Cast Expression:

类型转换表达式
*******************************************************************************

.. meta:
    frontend_status: Done

类型转换表达式的语法如下：

.. code-block:: abnf

       castExpression:
           expression 'as' type
           ;

形如 ``expr as target`` 的表达式把 ``expr`` 的求值结果视为 ``target`` 类型，因此整个
转换表达式的类型始终是目标类型 ``target``。

.. code-block:: typescript
   :linenos:

       class X {}

       let x1 : X = new X()
       let ob : Object = x1 as Object // Object is the target type
       let x2 : X = ob as X // X is the target type

.. index::
   cast expression
   target type
   operand
   cast operator

若 ``target`` 是 ``never``，则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       1 as never // Compile-time error

.. index::
   never type
   target type

若 ``target`` 在 :ref:`Type Erasure` 后不是“保留到 undefined”的类型，则会发出
:index:`compile-time warning`：

.. code-block:: typescript
   :linenos:

       class X<out T> { }
       function foo(p1: X<Object>) {
           p1 as X<number> // Compile-time warning - such cast converison is type unsafe
       }


根据 ``expr`` 的形态，转换语义分为两类：

- 数值字面量（见 :ref:`Numeric Literals`）、:ref:`数组字面量 <Array Literal>`、
  :ref:`对象字面量 <Object Literal>` 适用
  :ref:`Type Inference in Cast Expression`；
- 其他表达式适用 :ref:`Runtime Checking in Cast Expression`。

.. index::
   numeric literal
   object literal
   array literal
   type inference

.. _Type Inference in Cast Expression:

类型转换中的类型推断
===============================================================================

.. meta:
    frontend_status: Partly

对 ``expr as target`` 表达式，会考虑 ``expr`` 与 ``target`` 的以下组合：

- ``expr`` 是数值字面量，细节见 :ref:`Type Inference for Constant Expressions`；
- ``expr`` 是 :ref:`Array Literal`，且 ``target`` 是数组类型或元组类型，
  细节见 :ref:`Array Literal Type Inference from Context`；
- ``expr`` 是 :ref:`Object Literal`，且 ``target`` 是类类型、接口类型或
  :ref:`Record Utility Type`，细节见 :ref:`Object Literal` 各子节。

.. index::
   cast expression
   type inference
   expression
   numeric literal
   array literal
   object literal
   class type
   interface type
   record type
   utility type

这类转换的结果是：以 ``target`` 为上下文对 ``expr`` 做类型推断。该转换自身不会单独引发
:index:`runtime error`；但数组字面量元素或对象字面量属性在求值时，仍可能由其子表达式触发
运行时错误。

数值字面量转换的例子：

.. code-block:: typescript
   :linenos:

      let x = 1 as byte // OK
      let y = 128 as byte // Compile-time error

数组字面量转换的例子：

.. code-block:: typescript
   :linenos:

      let a = [1, 2] as double[] // OK, [1.0, 2.0]
      let b = [1, 2] as double // Compile-time error, wrong target type
      let c = [1, "cc"] as double[] // Compile-time error, wrong element type
      let d = [1, "cc"] as [double, string] // OK, cast to the tuple type
      let e = [1.0, "cc"] as [int, string] // Compile-time error, wrong element type

.. note::
   对数组字面量元素应用的是 *assignability* 检查。

.. index::
   inferred type
   evaluation
   runtime error
   cast
   assignability
   tuple type

对象字面量的示例见 :ref:`Object Literal`。

.. _Runtime Checking in Cast Expression:

类型转换中的运行时检查
===============================================================================

.. meta:
    frontend_status: Partly

若 :ref:`Type Inference in Cast Expression` 不适用，则 ``expr as target`` 会检查
``expr`` 的实际类型是否是 ``target`` 的子类型（见 :ref:`Subtyping`）。

若 ``expr`` 的 *实际类型* 是 ``target`` 的子类型（见 :ref:`Subtyping`），则转换结果就是 ``expr`` 的求值结果；
否则运行时抛出 ``ClassCastError``。

.. index::
   runtime error
   type erasure
   runtime
   effective type

若 ``target`` 在 :ref:`类型擦除 <Type Erasure>` 后不是“保留到 undefined”的类型，则运行时检查针对的是
它的有效类型（详见 :ref:`类型擦除 <Type Erasure>`）。由于有效类型比原始 ``target`` 更宽，这种转换可能在检查阶段通过，但后续按更精确
类型使用该值时仍可能触发 ``ClassCastError``。

语义上，这类 ``as`` 与 :ref:`instanceof Expression` 密切相关：

.. index::
   runtime check
   target type
   subtype
   subtyping
   type erasure
   effective type

- 若 ``x instanceof T`` 为 ``true``，则 ``x as T`` 一定成功，不会产生
  :index:`runtime error`；
- 若 ``x instanceof T`` 为 ``false``，则 ``x as T`` 在运行时一定抛出
  ``ClassCastError``。

.. code-block:: typescript
   :linenos:

       function foo (x: Object) {
           x as string
       }

       foo("aa") // OK
       foo(1)    // runtime error is thrown in foo by 'as' operator application

:ref:`instanceof Expression` 可以用来避免此类运行时错误。此外，:ref:`instanceof Expression` 在很多场景下可以让显式转换变得不必要：

.. code-block:: typescript
   :linenos:

       class Person {
           name: string
           constructor (name: string) { this.name = name }
       }

       function printName(x: Object) {
           if (x instanceof Person) {
               // no need to cast, access to 'name' is type-safe here
               console.log(x.name)
           } else {
               console.log("not a Person")
           }
       }

       printName(new Person("Bob")) // output: Bob
       printName(1)                 // output: not a Person

.. index::
   runtime error
   operator
   expression
   cast conversion

若编译器在编译期已经能确定某个转换恒成功或恒抛出 ``ClassCastError``，则会发出
:index:`compile-time warning`：

.. code-block:: typescript
   :linenos:

       class C {}
       class D extends C {}
       class E extends C {}

       let a: C = new D()
       a as D // Compile-time warning, cast always succeeds
       a as E // Compile-time warning, cast always throws ClassCastError

|

.. _typeof Expression:

``typeof`` 表达式
*******************************************************************************

.. meta:
    frontend_status: Done

``typeof`` 表达式用于获得表达式的类型类别信息。其语法如下：

.. code-block:: abnf

       typeOfExpression:
           'typeof' expression
           ;

任何 ``typeof`` 表达式的类型都是 ``string``。若 ``typeof`` 指向重载函数或重载方法的名称，
则发生 :index:`compile-time error`。

``typeof`` 的求值先对内部 ``expression`` 求值。若内部求值异常终止，则整个 ``typeof``
也异常终止；否则结果按以下规则确定。

.. index::
   syntax
   typeof expression
   expression
   string type
   evaluation
   compile time
   value

1. 某些情形下，结果在编译期就已知

当操作数的静态类型已经足以唯一确定结果时，``typeof`` 的值可在编译期直接确定。典型情况如下：

+--------------------------------------+------------------------------+-----------------------------+
| 表达式类型                           | ``typeof`` 结果              | 代码示例                    |
+======================================+==============================+=============================+
| ``string``                           | ``"string"``                 | .. code-block:: typescript  |
|                                      |                              |                             |
|                                      |                              |  let s: string = ...        |
|                                      |                              |  typeof s                   |
+--------------------------------------+------------------------------+-----------------------------+
| ``boolean``                          | ``"boolean"``                | .. code-block:: typescript  |
|                                      |                              |                             |
|                                      |                              |  let b: boolean = ...       |
|                                      |                              |  typeof b                   |
+--------------------------------------+------------------------------+-----------------------------+
| ``bigint``                           | ``"bigint"``                 | .. code-block:: typescript  |
|                                      |                              |                             |
|                                      |                              |  let b: bigint = ...        |
|                                      |                              |  typeof b                   |
+--------------------------------------+------------------------------+-----------------------------+
| 任意类或接口类型                     | ``"object"``                 | .. code-block:: typescript  |
|                                      |                              |                             |
|                                      |                              |  let a: Object = ...        |
|                                      |                              |  typeof a                   |
+--------------------------------------+------------------------------+-----------------------------+
| 任意函数类型                         | ``"function"``               | .. code-block:: typescript  |
|                                      |                              |                             |
|                                      |                              |  let f: () => void = ...    |
|                                      |                              |  typeof f                   |
+--------------------------------------+------------------------------+-----------------------------+
| ``undefined``、``void``              | ``"undefined"``              | .. code-block:: typescript  |
|                                      |                              |                             |
|                                      |                              |  typeof undefined           |
|                                      |                              |  typeof void                |
+--------------------------------------+------------------------------+-----------------------------+
| ``null``                             | ``"object"``                 | .. code-block:: typescript  |
|                                      |                              |                             |
|                                      |                              |  typeof null                |
+--------------------------------------+------------------------------+-----------------------------+
| ``T | null``，其中 ``T`` 为类        | ``"object"``                 | .. code-block:: typescript  |
| （但不是 ``Object``，见下一张表）    |                              |                             |
| 、接口或数组类型                     |                              |  class C {}                 |
|                                      |                              |  let x: C | null = ...      |
|                                      |                              |  typeof x                   |
+--------------------------------------+------------------------------+-----------------------------+
| 枚举类型                             | 枚举底层类型名称             | .. code-block:: typescript  |
|                                      |                              |                             |
|                                      |                              |  enum C {R, G, B}           |
|                                      |                              |  let c: C = ...             |
|                                      |                              |  typeof c // "int"          |
+--------------------------------------+------------------------------+-----------------------------+
| ``number``、``double``               | ``"number"``                 | .. code-block:: typescript  |
|                                      |                              |                             |
|                                      |                              |  let n: number = ...        |
|                                      |                              |  typeof n                   |
+--------------------------------------+------------------------------+-----------------------------+
| 其他数值类型：``byte``、             | ``"byte"``、``"short"``、    | .. code-block:: typescript  |
| ``short``、``int``、``long``、       | ``"int"``、``"long"`` 或     |                             |
| ``float``                            | ``"float"``，取决于表达式    |  let x: byte = ...          |
|                                      | 的类型                       |  typeof x // "byte"         |
+--------------------------------------+------------------------------+-----------------------------+
| ``char``                             | ``"char"``                   | .. code-block:: typescript  |
|                                      |                              |                             |
|                                      |                              |  let x: char = ...          |
|                                      |                              |  typeof x                   |
+--------------------------------------+------------------------------+-----------------------------+

2. 另一些情形下，结果需要运行时类型信息

这类情况包括 ``Object``、联合类型和类型参数等。此时静态类型不足以唯一确定结果，
``typeof`` 的值必须由运行时实际承载的类型决定：

+------------------------+-----------------------------+
| 表达式类型             | 代码示例                    |
+========================+=============================+
| ``Object``             | .. code-block:: typescript  |
|                        |                             |
|                        |  function f(o: Object) {    |
|                        |    typeof o                 |
|                        |  }                          |
+------------------------+-----------------------------+
| 联合类型               | .. code-block:: typescript  |
|                        |                             |
|                        |  function f(p: A | B) {     |
|                        |    typeof p                 |
|                        |  }                          |
+------------------------+-----------------------------+
| 类型参数               | .. code-block:: typescript  |
|                        |                             |
|                        |  class A<T|null|undefined> {|
|                        |     f: T                    |
|                        |     m() {                   |
|                        |        typeof this.f        |
|                        |     }                       |
|                        |     constructor(p:T) {      |
|                        |        this.f = p           |
|                        |     }                       |
|                        |  }                          |
+------------------------+-----------------------------+

这时返回的是运行时实际类型的名称，而不是静态注解中的并集名称或上界名称。

.. index::
   typeof expression
   syntax
   expression
   string type
   compile time
   runtime
   runtime type
   overloaded function
   overloaded method
   union type
   type parameter
   object type
   enumeration
   actual type

|

.. _Ensure-Not-Nullish Expressions:

非空确保表达式
*******************************************************************************

.. meta:
    frontend_status: Done

非空确保表达式是带后缀运算符 ``!`` 的表达式。对 ``e!``，若 ``e`` 具有 nullish 类型
（见 :ref:`Nullish Types`），则该运算会在运行时检查它是否确实不是 nullish 值。

其语法如下：

.. code-block:: abnf

       ensureNotNullishExpression:
           expression '!'
           ;

若 ``expr!`` 中 ``expr`` 的求值结果既不是 ``null`` 也不是 ``undefined``，则表达式结果就是
``expr`` 的求值结果；否则抛出 ``NullPointerError``。

``expr!`` 的类型是 ``expr`` 类型的非 nullish 变体。

.. note::
   若 ``expr`` 本身不是 nullish 类型，则后缀 ``!`` 不产生实际效果。

.. index::
   ensure-not-nullish expression
   postfix
   expression
   operator
   nullish type
   evaluation
   non-nullish variant
   null
   undefined

若编译器能在编译期确定某个非空确保表达式在运行时恒返回非 nullish 值，或恒对 nullish 值求值，
则会发出 :index:`compile-time warning`。后一种情形下，运行时一定抛出 ``NullPointerError``：

.. code-block:: typescript
   :linenos:

       class C { f = 1}

       let c = new C()
       c!.f // Compile-time warning, expression is always non-nullish, operator '!' is ignored

       let d: C | undefined = undefined
       d!.f // Compile-time warning, operator '!' always throws 'NullPointerError', as it is applied to nullish value
            // runtime: throws 'NullPointerError'

|

.. _Nullish-Coalescing Expression:

空值合并表达式
*******************************************************************************

.. meta:
    frontend_status: Done

空值合并表达式是使用运算符 ``??`` 的二元表达式。其语法如下：

.. code-block:: abnf

       nullishCoalescingExpression:
           expression '??' expression
           ;

它会检查左操作数的求值结果是否为 *nullish* 值：

- 若左操作数结果为 ``null`` 或 ``undefined``，则结果是右操作数的求值结果；
- 否则结果就是左操作数的求值结果，而右操作数不会被求值，因此 ``??`` 是一个惰性运算符。

.. index::
   nullish-coalescing expression
   binary expression
   operator
   evaluation
   nullish value
   lazy operator

空值合并表达式的类型，是以下两部分组成并归一化后的联合类型（见 :ref:`Union Types`）：

- 左操作数类型的非 nullish 变体；
- 右操作数的类型。

其语义可由下列等价代码说明：

.. code-block:: typescript
   :linenos:

       let x = lhs_expression ?? rhs_expression

       let x$ = lhs_expression
       if (x$ == null || x == undefined) {x = rhs_expression} else x = x$!

       // Type of x is NonNullishType(lhs_expression)|Type(rhs_expression)

若 ``??`` 与条件与 ``&&`` 或条件或 ``||`` 在没有括号的情况下混用，则发生
:index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       function  foo(n: boolean | undefined, a: boolean, b: boolean) {
           n ?? a || b   // error, '??' and '||' operations cannot be mixed without parentheses
           n ?? (a || b) // OK
       }

.. index::
   nullish type
   union type
   non-nullish type
   conditional-and operator
   conditional-or operator
   nullish-coalescing operator

若编译器能在编译期确定结果恒来自左操作数或恒来自右操作数，则会发出
:index:`compile-time warning`：

.. code-block:: typescript
   :linenos:

       let a = 1
       let b = undefined

       a ?? 2 // warning, left-hand-side expression is always used
       b ?? 3 // warning, right-hand-side expression is always used


|

.. _Unary Expressions:

一元表达式
*******************************************************************************

.. meta:
    frontend_status: Done

一元表达式包含后缀自增/自减、前缀自增/自减、一元正负号、按位取反和逻辑取反。
其语法如下：

.. code-block:: abnf

       unaryExpression:
           expression '++'
           | expression '--'
           | '++' expression
           | '--' expression
           | '+' expression
           | '-' expression
           | '~' expression
           | '!' expression
           ;

除后缀自增和后缀自减外，其余一元运算符都按从右到左结合，因此 ``~+x`` 与 ``~(+x)``
具有相同语义。

一元表达式的结果类型不一定与操作数类型相同，其主要规则如下：

+------------------------+-----------------------------------------------+---------------------------+
| **一元运算符**         | **操作数类型**                                | **结果类型**              |
+========================+===============================================+===========================+
| ``++``、``--``         | ``byte``                                      | ``byte``                  |
| （前缀或后缀）         +-----------------------------------------------+---------------------------+
|                        | ``short``                                     | ``short``                 |
|                        +-----------------------------------------------+---------------------------+
|                        | ``int``                                       | ``int``                   |
|                        +-----------------------------------------------+---------------------------+
|                        | ``long``                                      | ``long``                  |
|                        +-----------------------------------------------+---------------------------+
|                        | ``float``                                     | ``float``                 |
|                        +-----------------------------------------------+---------------------------+
|                        | ``double``                                    | ``double``                |
|                        +-----------------------------------------------+---------------------------+
|                        | ``bigint``                                    | ``bigint``                |
+------------------------+-----------------------------------------------+---------------------------+
| 一元 ``+``、``-``      | ``byte``、``short``、``int``                  | ``int``                   |
|                        +-----------------------------------------------+---------------------------+
|                        | ``long``                                      | ``long``                  |
|                        +-----------------------------------------------+---------------------------+
|                        | ``float``                                     | ``float``                 |
|                        +-----------------------------------------------+---------------------------+
|                        | ``double``                                    | ``double``                |
|                        +-----------------------------------------------+---------------------------+
|                        | ``bigint``                                    | ``bigint``                |
+------------------------+-----------------------------------------------+---------------------------+
| ``~``                  | ``byte``、``short``、``int``、``float``       | ``int``                   |
| （按位取反）           +-----------------------------------------------+---------------------------+
|                        | ``long``、``double``                          | ``long``                  |
|                        +-----------------------------------------------+---------------------------+
|                        | ``bigint``                                    | ``bigint``                |
+------------------------+-----------------------------------------------+---------------------------+
| ``!``                  | ``boolean``，或见                             | ``boolean``               |
| （逻辑取反）           | :ref:`Extended Conditional Expressions`       |                           |
+------------------------+-----------------------------------------------+---------------------------+

.. index::
   unary expression
   unary operator
   postfix increment
   postfix decrement
   prefix increment
   prefix decrement
   logical complement
   bitwise complement
   type

.. _Postfix Increment:

后缀自增
===============================================================================

.. meta:
    frontend_status: Done

后缀自增表达式是表达式后跟运算符 ``++``。

该表达式必须是左值表达式（见 :ref:`Left-Hand-Side Expressions`），并且指代变量。若其类型不是数值类型（见 :ref:`Numeric Types`）或 ``bigint``，
则发生 :index:`compile-time error`。

后缀自增表达式的类型与变量本身相同，但表达式结果是一个值，而不是变量。

若操作数表达式在运行时正常完成，则：

- 将与变量同类型的值 ``1`` 加到该变量当前值上；
- 把结果写回变量。

否则，后缀自增表达式异常终止，且不会发生递增。

后缀自增表达式的值是写回 *之前* 的旧值。

.. code-block:: typescript

     let a: short  = 1
     let b: float  = 1.5f
     let c: bigint = 1n

     a++ // result '1', 'a' becomes '2' ('1 + 1')
     b++ // result '1.5f', 'b' becomes '2.5f'  ('1.5f + 1f')
     c++ // result '1n', 'c' becomes '2n' ('1n + 1n')


.. index::
   postfix
   postfix increment
   expression
   increment expression
   increment operator
   left-hand-side expression
   conversion
   variable
   type
   numeric type
   bigint
   evaluation
   value
   operand

.. _Postfix Decrement:

后缀自减
===============================================================================

.. meta:
    frontend_status: Done

后缀自减表达式是表达式后跟运算符 ``--``。

该表达式必须是左值表达式（见 :ref:`Left-Hand-Side Expressions`），并且指代变量。若其类型不是 :ref:`Numeric Types` 或 ``bigint``，
则发生 :index:`compile-time error`。

后缀自减表达式的类型与变量相同，但结果是值，而不是变量。

.. index::
   postfix
   decrement expression
   decrement operator
   numeric type
   variable
   value
   expression
   conversion
   runtime
   operand
   completion
   evaluation

若操作数表达式在运行时正常完成，则：

- 从变量当前值中减去一个同类型的值 ``1``；
- 将减法结果写回变量。

否则，后缀自减表达式异常终止，且不会发生递减。

后缀自减表达式的值是写回 *之前* 的旧值。

也就是说，``x--`` 在更大表达式里贡献的是旧值，而变量 ``x`` 自身在此之后才保存减一后的新值。

.. code-block:: typescript

     let a: short  = 1
     let b: float  = 1.5f
     let c: bigint = 1n

     a-- // result '1', 'a' becomes '0' ('1 - 1')
     b-- // result '1.5f', 'b' becomes '0.5f'  ('1.5f - 1f')
     c-- // result '1n', 'c' becomes '0n' ('1n - 1n')

.. index::
   variable
   numeric types conversion
   postfix
   decrement expression
   decrement operator
   abrupt completion
   expression
   type
   numeric type
   value
   operand
   evaluation
   decrementation

.. index::
   postfix decrement
   decrement expression
   decrement operator
   left-hand-side expression
   variable
   numeric type
   bigint
   evaluation
   runtime
   abrupt completion
   conversion
   old value

.. _Prefix Increment:

前缀自增
===============================================================================

.. meta:
    frontend_status: Done

前缀自增表达式是表达式前带运算符 ``++``。

该表达式必须是左值表达式（见 :ref:`Left-Hand-Side Expressions`），并且指代变量。若其类型不是 :ref:`Numeric Types` 或 ``bigint``，
则发生 :index:`compile-time error`。

前缀自增表达式的类型与变量相同，但结果是值，不是变量。

.. index::
   prefix
   increment operator
   increment expression
   expression
   operator
   variable
   value

若操作数表达式在运行时正常完成，则：

- 将与变量同类型的值 ``1`` 加到变量当前值上；
- 把结果写回变量。

若操作数求值异常终止，则整个表达式异常终止，且不会发生递增。

前缀自增表达式的值是写回 *之后* 的新值。

.. code-block:: typescript

     let a: short  = 1
     let b: float  = 1.5f
     let c: bigint = 1n

     ++a // result '2', 'a' becomes '2' ('1 + 1')
     ++b // result '2.5f', 'b' becomes '2.5f'  ('1.5f + 1f')
     ++c // result '2n', 'c' becomes '2n' ('1n + 1n')


.. index::
   prefix increment
   increment expression
   variable
   evaluation
   numeric type
   bigint

.. _Prefix Decrement:

前缀自减
===============================================================================

.. meta:
    frontend_status: Done

前缀自减表达式是表达式前带运算符 ``--``。

该表达式必须是左值表达式（见 :ref:`Left-Hand-Side Expressions`），并且指代变量。若其类型不是 :ref:`Numeric Types` 或 ``bigint``，
则发生 :index:`compile-time error`。

前缀自减表达式的类型与变量相同，但结果是值，不是变量。

.. index::
   prefix
   decrement operator
   decrement expression
   expression
   operator
   variable
   value

若操作数表达式在运行时正常完成，则：

- 从变量当前值中减去一个同类型的值 ``1``；
- 把结果写回变量。

若操作数求值异常终止，则整个表达式异常终止，且不会发生递减。

前缀自减表达式的值是写回 *之后* 的新值。

.. code-block:: typescript

     let a: short  = 1
     let b: float  = 1.5f
     let c: bigint = 1n

     --a // result '0', 'a' becomes '0' ('1 - 1')
     --b // result '0.5f', 'b' becomes '0.5f'  ('1.5f - 1f')
     --c // result '0n', 'c' becomes '0n' ('1n - 1n')

.. index::
   prefix decrement
   decrement expression
   variable
   evaluation
   numeric type
   bigint

.. _Unary Plus:

一元正号
===============================================================================

.. meta:
    frontend_status: Done

一元正号表达式是前置运算符 ``+`` 作用于某个表达式。

其操作数必须能够通过 :ref:`Implicit Conversions` 转换为 :ref:`Numeric Types`，
或本身就是 ``bigint``；否则发生 :index:`compile-time error`。

一元正号表达式的结果始终是值，而不是变量。

在应用一元 ``+`` 之前会先执行数值 widening。结果类型规则如下：

- 对 ``byte``、``short``、``int``，结果类型为 ``int``；
- 对 ``long``、``float``、``double``、``bigint``，结果类型与操作数一致。

也就是说，一元正号不会改变数值本身；它只是在继续求值前，先按本节规则完成数值提升。

.. index::
   unary plus operator
   unary plus expression
   operator
   operand
   conversion
   convertible type
   numeric type
   numeric widening
   value

.. _Unary Minus:

一元负号
===============================================================================

.. meta:
    frontend_status: Done

一元负号表达式是前置运算符 ``-`` 作用于某个表达式。

其操作数必须能够转换为 :ref:`Numeric Types`，或本身是 ``bigint``；否则发生
:index:`compile-time error`。

在应用一元负号前会先执行 :ref:`Widening Numeric Conversions`。结果类型规则如下：

- 对 ``byte``、``short``、``int``，结果类型为 ``int``；
- 对 ``long``、``float``、``double``、``bigint``，结果类型保持不变。

一元负号表达式的结果始终是值，而不是变量；即使操作数表达式的结果本身是变量，也不例外。

.. index::
   unary minus
   operand
   unary operator
   operator
   conversion
   convertibility
   numeric type
   numeric types conversion
   expression
   operand value

一元取负总是在提升后的操作数值所在的同一 value set 上执行；随后才可能对同一个结果继续执行
value set conversion。

运行时，一元负号表达式的值就是提升后操作数值的算术相反数。

对整数，一元负号等价于从零中减去该值。ArkTS 使用二进制补码表示整数，因此其值域并不对称；
最小负整数取反时仍得到同一个最小负整数。此时会发生溢出，但不会抛错；对任意整数值 ``x``，
都有 ``-x == (~x) + 1``。

对 ``bigint``，取负与从 ``0n`` 中减去该值等价。

对浮点数，一元负号并不总与 ``0.0 - x`` 完全等价；它只是翻转符号位。特殊情况包括：

- ``NaN`` 取负仍为 ``NaN``；
- 正无穷和负无穷互换；
- ``+0.0`` 与 ``-0.0`` 在取负后互换。

因此，一元负号不会引入新的算术异常；它只是依据操作数类型产生相反数，
并在浮点场景下保留 IEEE 754 对 ``NaN``、无穷大和有符号零的处理方式。

.. index::
   unary minus
   negation
   operand
   numeric type
   bigint
   two's complement
   overflow
   unary numeric promotion
   value set conversion
   arithmetic negation
   promoted operand value
   promoted value
   NaN
   infinity
   value
   variable
   operation
   floating-point number
   signed zero
   runtime

.. _Bitwise Complement:

按位取反
===============================================================================

.. meta:
    frontend_status: Done

按位取反运算符 ``~`` 作用于数值类型或 ``bigint``。

若操作数类型为 ``double`` 或 ``float``，则先分别截断为 ``long`` 或 ``int``；
若操作数类型为 ``byte`` 或 ``short``，则先提升为 ``int``；若操作数是 ``bigint``，
则不需要转换。

结果类型规则如下：

- 对 ``byte``、``short``、``int``、``float``，结果类型为 ``int``；
- 对 ``long``、``double``，结果类型为 ``long``；
- 对 ``bigint``，结果类型为 ``bigint``。

按位取反表达式的结果始终是值，而不是变量；即使操作数表达式的结果本身是变量，也不例外。

运行时结果是完成上述截断或提升后的操作数值的逐位补码。对所有情况都有 ``~x == (-x) - 1``。

.. code-block:: typescript

     let b: byte  = 2
     let s: short  = 2
     let i: int = 2
     let f: float = 2.0f

     let l: long  = 2
     let d: double  = 2.0

     let B: bigint = 2n

     let rb = ~b
     console.log(rb, typeof rb) // prints '-3 int'
     let rs = ~s
     console.log(rs, typeof rs) // prints '-3 int'
     let ri = ~i
     console.log(ri, typeof ri) // prints '-3 int'
     let rf = ~f
     console.log(rf, typeof rf) // prints '-3 int'

     let rl = ~l
     console.log(rl, typeof rl) // prints '-3 long'
     let rd = ~d
     console.log(rd, typeof rd) // prints '-3 long'

     let rB = ~B
     console.log(rB, typeof rB) // prints '-3 bigint'


.. index::
   bitwise complement
   bitwise complement expression
   operator
   expression
   operand value
   numeric type
   bigint type
   truncation
   conversion
   runtime
   value
   variable
   bigint

.. _Logical Complement:

逻辑取反
===============================================================================

.. meta:
    frontend_status: Done

逻辑取反表达式是前置运算符 ``!`` 作用于某个表达式。

操作数类型必须是 ``boolean``，或者属于 :ref:`Extended Conditional Expressions`
允许的扩展条件类型；否则发生 :index:`compile-time error`。

逻辑取反表达式的类型始终是 ``boolean``。

其运行时语义是：若操作数值（必要时按扩展条件规则转换后）为 ``false``，则结果为 ``true``；
若操作数值为 ``true``，则结果为 ``false``。

.. index::
   logical complement operator
   logical complement
   logical complement expression
   conditional expression
   extended conditional expression
   operand
   boolean type
   value

|

.. _Binary Expressions:

二元表达式
*******************************************************************************

.. meta:
    frontend_status: Done

二元表达式的基本形式是 *expression 1* ``op`` *expression 2*，其中 ``op`` 为二元运算符，
左右两侧是操作数表达式。其语法如下：

.. code-block:: abnf

       binaryExpression:
           multiplicativeExpression
           | exponentiationExpression
           | additiveExpression
           | shiftExpression
           | relationalExpression
           | equalityExpression
           | bitwiseAndLogicalExpression
           | conditionalAndExpression
           | conditionalOrExpression
           ;

本章后续分别规定各个子类的类型组合、结果类型和求值语义。若某组操作数类型不在语言允许的
组合之内，则在编译期可判定时发生 :index:`compile-time error`，否则在运行时触发
:index:`runtime error`。

二元表达式中最重要的规则是：

- 运算符的优先级决定语法归约顺序；
- 子表达式的求值顺序仍受 :ref:`Order of Expression Evaluation` 约束；
- 各类运算对操作数类型有严格限制，不满足时要么编译期报错，要么在运行时抛错；
- 某些运算在值语义上有专门的短路、溢出、舍入或类型提升规则。

二元表达式的结果有时是值，有时会参与后续左值相关语义，但这一点并不改变其静态类型规则：
静态类型始终由当前运算符及操作数类型组合决定，运行时只负责产生该规则对应的实际值或错误。

英文规范在这里给出了完整的类型组合表。未在表中列出的组合，如果能在编译期识别，
则发生 :index:`compile-time error`；否则在运行时触发 :index:`runtime error`。

+--------------------+------------------------------------+----------------------------------+----------------------+
| **运算符**         | **一个操作数的类型**               | **另一个操作数的类型**           | **结果类型**         |
+====================+====================================+==================================+======================+
| 乘法 / 除法 / 取余 | byte、short、int                   | byte、short、int                 | int                  |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | long                               | 除 float 或 double 外的任意      | long                 |
|                    |                                    | 数值类型                         |                      |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | float                              | 除 double 外的任意数值类型       | float                |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | double                             | 任意数值类型                     | double               |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | bigint                             | bigint                           | bigint               |
+--------------------+------------------------------------+----------------------------------+----------------------+
| 指数运算           | 任意数值类型                       | 任意数值类型                     | double               |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | bigint                             | bigint                           | bigint               |
+--------------------+------------------------------------+----------------------------------+----------------------+
| +                  | string                             | 可转换为字符串                   | string               |
+--------------------+------------------------------------+----------------------------------+----------------------+
| + / -              | byte、short、int                   | byte、short、int                 | int                  |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | long                               | 除 float 或 double 外的任意      | long                 |
|                    |                                    | 数值类型                         |                      |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | float                              | 除 double 外的任意数值类型       | float                |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | double                             | 任意数值类型                     | double               |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | bigint                             | bigint                           | bigint               |
+--------------------+------------------------------------+----------------------------------+----------------------+
| << / >>            | bigint                             | bigint                           | bigint               |
+--------------------+------------------------------------+----------------------------------+----------------------+
| << / >> / >>>      | byte、short、int、float            | 任意数值类型                     | int（见注 2）        |
+                    +------------------------------------+----------------------------------+----------------------+
|                    | long、double                       | 任意数值类型                     | long（见注 2）       |
+--------------------+------------------------------------+----------------------------------+----------------------+
| < / <= / > / >=    | string、字符串字面量类型           | string、字符串字面量类型         | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | boolean                            | boolean                          | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | 数值类型                           | 数值类型                         | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | 枚举（数值值）                     | 枚举（数值值）                   | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | 枚举（字符串值）                   | 枚举（字符串值）                 | boolean              |
+--------------------+------------------------------------+----------------------------------+----------------------+
| == / === / != / !==| string、字符串字面量类型           | string、字符串字面量类型         | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | boolean                            | boolean                          | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | 数值类型                           | 数值类型                         | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | bigint                             | bigint                           | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | char                               | char                             | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | 枚举                               | 枚举                             | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | 函数                               | 函数                             | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | null、undefined                    | null、undefined                  | boolean              |
+--------------------+------------------------------------+----------------------------------+----------------------+
| & / ^ / |          | boolean                            | boolean                          | boolean              |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | byte、short、int、float            | byte、short、int、float          | int                  |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | long、double                       | long、double                     | long                 |
|                    +------------------------------------+----------------------------------+----------------------+
|                    | bigint                             | bigint                           | bigint               |
+--------------------+------------------------------------+----------------------------------+----------------------+
| && / ||            | boolean                            | boolean                          | boolean              |
+                    +------------------------------------+----------------------------------+----------------------+
|                    | 其他类型，见注 5                   | 其他类型，见注 5                 | 见注 5               |
+--------------------+------------------------------------+----------------------------------+----------------------+

.. note::
   对上表补充说明如下：

   1. 除移位运算符外，操作数顺序不会影响结果类型。
   2. 数值类型移位运算 ``<<``、``>>``、``>>>`` 的结果类型只取决于第一个
      （左侧）操作数的提升后类型，而不取决于第二个（右侧）操作数的类型。
   3. 相等运算符 ``==``、``===``、``!=``、``!==`` 对任意类型都可以书写，
      但表中明确列出的组合才有本章定义的专门语义。
   4. 含 ``null`` 或 ``undefined`` 的相等比较，见
      :ref:`Extended Equality with null or undefined`。
   5. 非 ``boolean`` 操作数上的 ``&&`` / ``||``，见
      :ref:`Extended Conditional Expressions`。

对于常见运算符，结果类型可概括如下：

- ``*``、``/``、``%``：要求双方都是 ``bigint`` 或都可转换为数值类型；
- ``**``：要么双方都是 ``bigint``，要么双方都按数值幂运算处理；
- ``+``：若任一侧为 ``string``，则执行字符串拼接；否则按数值加法或 ``bigint`` 加法处理；
- ``<<``、``>>``、``>>>``：结果类型取决于左操作数提升后的类型；
- ``<``、``<=``、``>``、``>=``：结果恒为 ``boolean``；
- ``==``、``===``、``!=``、``!==``：结果恒为 ``boolean``；
- ``&&``、``||``：布尔场景返回 ``boolean``，扩展条件表达式场景遵守专门规则。

除此之外，还有几条对整章都成立的规则：

- 除移位运算符外，左右操作数的先后顺序通常不影响结果类型；
- 对 ``<<``、``>>``、``>>>``，结果类型只取决于左操作数提升后的类型；
- 相等运算符对任意类型都可以书写，但只有本章明确列出的组合具有专门语义；
- 与 ``null`` / ``undefined`` 相关的特殊相等语义见
  :ref:`Extended Equality with null or undefined`；
- ``&&`` / ``||`` 在非 ``boolean`` 操作数上的类型与值语义见
  :ref:`Extended Conditional Expressions`。

因此，阅读本章后续各节时，可以把“操作数是否合法”“结果类型是什么”“运行时是否短路/
是否可能异常完成”视为每一种二元运算都要单独回答的三个核心问题。

.. index::
   binary expression
   binary operator
   operand
   expression
   compile-time error
   runtime error
   evaluation order
   type restriction
   type promotion
   result type
   shift operator
   equality operator
   extended equality
   extended conditional expression

.. _Multiplicative Expressions:

乘法类表达式
===============================================================================

.. meta:
    frontend_status: Done

乘法类表达式使用 ``*``、``/`` 和 ``%``。其语法如下：

.. code-block:: abnf

       multiplicativeExpression:
           expression '*' expression
           | expression '/' expression
           | expression '%' expression
           ;

乘法类运算符按从左到右结合。两个操作数必须满足以下条件之一：

- 都是 ``bigint``；
- 都可转换为数值类型，参见 :ref:`Numeric Operator Contexts` 和 :ref:`Numeric Types`。

否则发生 :index:`compile-time error`。

若两个操作数都是 ``bigint``，则不做隐式转换，结果类型恒为 ``bigint``：

.. code-block:: typescript
   :linenos:

      // OK, the inferred type of 'i' is ``bigint``
      let i = 1n * 2n
      // Compile-time error, one operand is not of 'bigint' type
      let i = 1n * 2

.. index::
   multiplicative expression
   multiplicative operator
   convertible type
   numeric type
   numeric types conversion
   widening numeric conversion
   compile-time error

若操作数属于数值类型（见 :ref:`Numeric Operator Contexts`），则先执行数值提升（见 :ref:`Widening Numeric Conversions`）。结果类型由提升后的最大类型决定：

- 若任一操作数为 ``double``，结果类型为 ``double``；
- 否则若任一操作数为 ``float``，结果类型为 ``float``；
- 否则若任一操作数为 ``long``，结果类型为 ``long``；
- 否则结果类型为 ``int``。

.. code-block:: typescript
   :linenos:

      // Code below prints true 4 times
      let byte1: byte = 1
      let byte2: byte = 1
      let long1: long = 1
      let float1: float = 1
      let double1: double = 1

      let res_byte = byte1 * byte2  // int
      console.log(res_byte instanceof int)

      let res_long = byte1 * long1  // long
      console.log(res_long instanceof long)

      let res_float = byte1 * float1 // float
      console.log(res_float instanceof float)

      let res_double = byte1 * double1 // double
      console.log(res_double instanceof double)


.. index::
   value
   variable
   operand expression
   multiplication
   division
   remainder
   promoted type

.. index::
   multiplicative expression
   multiplicative operator
   numeric type
   bigint
   widening numeric conversion
   type inference
   promotion

.. _Multiplication:

乘法
===============================================================================

.. meta:
    frontend_status: Done

二元运算符 ``*`` 对两个操作数求积。数值乘法表达式的类型是其操作数经过
:ref:`Widening numeric conversions` 之后的最大类型（见 :ref:`Numeric Types`）。

- ``bigint`` 乘法满足结合律；
- 当整数操作数具有同一类型时，整数乘法满足结合律；
- 若操作数表达式没有副作用，则乘法满足交换律；
- 浮点乘法不满足结合律。

若整数乘法溢出，则结果取数学乘积在足够大二进制补码表示中的低位部分，结果符号可能与数学乘积
的符号不同。

.. index::
   multiplication
   binary operator
   operand
   commutative operation
   expression
   operand expression
   side effect
   integer
   integer multiplication
   associativity
   two's-complement format
   floating-type multiplication
   operand value
   low-order bit

浮点乘法结果按 IEEE 754 算术确定：

- 任一操作数为 ``NaN``，或无穷大与零相乘时，结果为 ``NaN``；
- 若结果不是 ``NaN``，则两操作数同号时结果为正，异号时结果为负；
- 无穷大与有限非零值相乘得到带符号的无穷大；
- 若结果幅值过大而无法表示，则运算上溢并得到带适当符号的无穷大；
- 若没有 ``NaN`` 或无穷大参与，则先计算精确数学乘积；
- 之后再按 IEEE 754 的 round-to-nearest 模式，把该乘积舍入到所选 value set 中最接近的值。

|LANG| 对浮点乘法要求支持 IEEE 754 规定的渐进下溢
（见 :ref:`Floating-Point Types and Operations`）。
乘法运算即使发生上溢、下溢或精度损失，也不会因为这些原因抛出错误。

.. code-block:: typescript
   :linenos:

    let i: int = 6
    let j: int = -3
    let f: float = 2.5f
    let d: double = 0.0

    let r1 = i * j
    let r2 = f * 4
    let r3 = f * d

    let bi1 = 7n
    let bi2 = 8n
    let r4 = bi1 * bi2

.. index::
   multiplication
   overflow
   IEEE 754
   NaN
   infinity
   associativity
   commutativity
   round-to-nearest mode
   gradual underflow
   floating-point multiplication
   finite value
   multiplication result

.. _Division:

除法
===============================================================================

.. meta:
    frontend_status: Done

二元运算符 ``/`` 返回左操作数（被除数）与右操作数（除数）的商。
数值除法表达式的结果类型是其操作数经过 :ref:`Widening Numeric Conversions` 之后的最大
数值类型（见 :ref:`Numeric Types`）。

``bigint`` 除法与整数除法都向 *0* 舍入。也就是说，商是满足
:math:`|d\cdot{}q|\leq{}|n|` 的最大幅值整数。

- 对 ``bigint``，若除数是 ``0n``，则编译期可判定时发生 :index:`compile-time error`，
  否则运行时触发 :index:`runtime error`；
- 对整数类型，若除数在编译期可知为 ``0``，则发生 :index:`compile-time error`，
  否则运行时抛出 ``ArithmeticError``；
- 对整数除法，唯一不完全服从上述规则的特殊情况是最小负整数除以 ``-1``，此时结果等于被除数，
  即使发生溢出也不抛错。

.. note::
   当整数商为正时，被除数和除数同号；商为负时，被除数和除数异号。


.. index::
   division operator
   binary operator
   operand
   dividend
   divisor
   integer division
   integer operand
   numeric types conversion
   widening numeric conversion
   numeric type
   integer value

也就是说，整数除法始终按朝零截断的方式得到商。唯一的特殊溢出情形是
“最小负整数除以 ``-1``”，此时结果等于被除数本身，且不会抛出错误。


.. index::
   integer overflow
   integer division
   overflow
   dividend
   divisor
   special case

浮点除法遵循 IEEE 754 规则：

- 任一操作数为 ``NaN``、两个操作数同为无穷大、或两个操作数同为零时，结果为 ``NaN``；
- 结果非 ``NaN`` 时，符号由两操作数符号决定；
- 无穷大除以有限值、或非零有限值除以零，得到带符号无穷大；
- 有限值除以无穷大、或零除以有限值，得到带符号零；
- 其余情况下按数学商计算，并按 round-to-nearest 模式舍入。

换句话说：

- 两个操作数同号时，结果符号为正；
- 两个操作数异号时，结果符号为负；
- 若既没有 ``NaN`` 也没有无穷大参与，则先计算精确数学商，再决定是否需要舍入。

若结果幅值过大而无法表示，则运算上溢并得到带适当符号的无穷大。
|LANG| 按 IEEE 754 要求支持渐进下溢（见 :ref:`Floating-Point Types and Operations`）。


.. index::
   floating-point division
   IEEE 754
   NaN
   infinity
   signed infinity
   finite value
   positive zero
   negative zero
   signed zero
   overflow
   underflow
   loss of information
   floating-point operation

浮点除法即使发生除零、溢出、下溢或精度损失，也不会因此抛错。数值类型的除法表达式的结果类型，
是其操作数经过 :ref:`Widening numeric conversions` 后的最大数值类型。

也就是说，只要操作数属于数值类型，``/`` 的结果类型始终是数值提升后的最大类型；
除 ``bigint`` / 整数除零这类明确错误条件外，浮点除法不会因为 IEEE 754 特例而抛出运行时错误。

.. code-block:: typescript
   :linenos:

    let a: int = 7
    let b: int = 3
    let c: double = 7.0
    let d: double = 0.0

    let q1 = a / b
    let q2 = c / 2
    let q3 = c / d
    let q4 = 7n / 3n

.. index::
   division
   dividend
   divisor
   ArithmeticError
   bigint
   IEEE 754
   NaN
   infinity
   signed infinity
   finite value
   positive zero
   negative zero
   overflow
   underflow
   round-to-zero
   integer overflow
   floating-point division
   round-to-nearest mode
   loss of information
   floating-point operation
   division operator
   rounding

.. _Remainder:

余数
===============================================================================

.. meta:
    frontend_status: Done

运算符 ``%`` 返回隐含除法之后的余数，其中左操作数为被除数，右操作数为除数。
余数表达式的类型是其操作数经过 :ref:`Widening Numeric Conversions` 之后的最大
数值类型（见 :ref:`Numeric Types`）。

对 ``bigint``，余数满足 :math:`(a/b)*b+(a\%b)=a`，且不进行隐式转换。
若除数为 ``0n``，运行时触发 :index:`runtime error`。

对整数操作数，余数同样满足 :math:`(a/b)*b+(a\%b)=a`。在执行余数运算之前，
按 :ref:`Widening Numeric Conversions` 进行数值提升：

- ``byte``、``short``、``int``、``float`` 产生 ``int`` 结果；
- ``long``、``double`` 产生 ``long`` 结果。

.. index::
   binary operator
   operand
   remainder operator
   dividend
   divisor
   division
   numeric types conversion
   conversion
   floating-point operand
   remainder operation
   value
   integer operand
   numeric type
   widening numeric conversion

整数余数还有两个重要约束：

- 结果符号与被除数相同；
- 结果幅值始终小于除数幅值。

即使被除数是该类型可表示的最小负整数而除数是 ``-1``，上述恒等式仍然成立；
此时余数为 ``0``。

若整数除数在编译期可知为 ``0``，则发生 :index:`compile-time error`；
否则运行时抛出 ``ArithmeticError``。

.. index::
   integer remainder
   remainder
   integer operator
   remainder operation
   integer operand
   negative integer
   integer overflow

浮点余数与 IEEE 754 定义的 remainder 并不相同。ArkTS 的 ``%`` 更接近整数余数语义，
类似 C 库中的 ``fmod``；若需要 IEEE 754 remainder，应使用标准库（见 :ref:`Standard Library`）
的 ``Math.IEEEremainder``。

对浮点余数：

- 任一操作数为 ``NaN``、被除数为无穷大、除数为零时，结果为 ``NaN``；
- 结果非 ``NaN`` 时，其符号与被除数相同；
- 若被除数有限且除数为无穷大，或被除数为零且除数有限，则结果等于被除数；
- 其余情况下，结果满足 :math:`r = n - (d \cdot q)`，其中 *q* 为朝零截断得到的商。

这里的 *q* 还满足以下约束：

- 当 :math:`n/d` 为负时，*q* 为负；
- 当 :math:`n/d` 为正时，*q* 为正；
- *q* 的幅值是不超过真实数学商幅值的最大整数。

也就是说，浮点余数更接近通常的整数余数语义，而不是 IEEE 754 定义的 rounding remainder。

.. index::
   floating-point remainder operation
   remainder operation
   operand
   NaN
   infinity
   divisor
   dividend
   IEEE 754

浮点余数不会抛出错误，也不会因上溢、下溢或精度损失失败。

因此，ArkTS 的 ``%`` 在浮点场景下更接近 C 标准库中的 ``fmod``，
而不是 IEEE 754 的 rounding remainder。

对整数余数同样成立的一点是：结果只能与被除数同号，且其绝对值严格小于除数的绝对值。

.. code-block:: typescript
   :linenos:

    let a: int = 7
    let b: int = 3
    let c: double = 7.5

    let r1 = a % b
    let r1n = (-7) % 3
    let r2 = c % 2.0
    let r3 = 7n % 3n

.. index::
   remainder
   dividend
   divisor
   ArithmeticError
   floating-point remainder
   IEEE 754
   Math.IEEEremainder
   truncation
   rounding
   magnitude
   value
   infinity
   NaN
   bigint
   runtime error
   standard library
   floating-point operation
   loss of precision
   truncation
   integer remainder
   floating-point remainder operation

.. _Exponentiation Expression:

幂表达式
===============================================================================

.. meta:
    frontend_status: None

幂表达式使用二元运算符 ``**``。

该运算符把第一个操作数作为底数，把第二个操作数作为指数。

其语法如下：

.. code-block:: abnf

       exponentiationExpression:
           expression '**' expression
           ;

``**`` 只有两种合法变体：

- 两个操作数都是 :ref:`Type bigint`；
- 两个操作数都属于 :ref:`Numeric types`，此时凡是不是 ``double`` 的操作数都会先转换为 ``double``。

任何其他类型组合都会导致 :index:`compile-time error`。

当指数为 ``0n`` 时，结果恒为 ``1n``，包括 ``0n ** 0n`` 这一情形。

若第二个 ``bigint`` 操作数为负数，则在编译期可判定时发生 :index:`compile-time error`，
否则运行时触发 :index:`runtime error`。

.. code-block:: typescript
   :linenos:

      let a: bigint = 2n
      let b: double = 2
      let c: int = 2
      let d: int = 10

      let v = a ** 2n // OK 'bigint' ** 'bigint'
      let u = a ** 0n // OK 'bigint' ** 'bigint'
      let w = a ** -1n // Compile-time error, exponent must be non-negative

      let x = a ** c // Compile-time error, 'c' is not 'bigint'
      let y = b ** d // 'd' is converted to 'double'
      let z = c ** d // both 'c' and 'd' are converted to 'double'


对数值操作数，``**`` 的语义等价于 ``Math.pow()``。这一变体本身既不会引发
:index:`compile-time error`，也不会引发 :index:`runtime error`。

对数值幂运算，需要遵守 IEEE 754 的若干特殊情形：

.. note::
   由于数值操作数都会隐式转换为 ``double``，下面所说的“整数”实际上指小数部分为零的
   ``double`` 值，例如 ``-2.0``。

   - ``x ** +/-0`` 恒为 ``1``，即使 ``x`` 是 ``NaN``；
   - ``+/-0 ** y`` 在 ``y`` 为负奇整数时结果为 ``+/-Infinity``；
   - ``+/-0 ** -Infinity`` 为 ``+Infinity``；
   - ``+/-0 ** +Infinity`` 为 ``+0``；
   - ``+/-0 ** y`` 在 ``y`` 为有限正奇整数时为 ``+/-0``；
   - ``-1 ** +Infinity`` 为 ``1``；
   - ``+1 ** y`` 对任意 ``y`` 都为 ``1``，即使 ``y`` 是 ``NaN``；
   - 当 ``-1 < x < 1`` 时，``x ** +Infinity`` 为 ``+0``；
   - 当 ``x < -1`` 或 ``x > 1`` 时，``x ** +Infinity`` 为 ``+Infinity``；
   - 当 ``-1 < x < 1`` 时，``x ** -Infinity`` 为 ``+Infinity``；
   - 当 ``x < -1`` 或 ``x > 1`` 时，``x ** -Infinity`` 为 ``+0``；
   - ``+Infinity ** y`` 在 ``y < 0`` 时为 ``+0``，在 ``y > 0`` 时为 ``+Infinity``；
   - ``-Infinity ** y`` 在 ``y`` 为有限负奇整数时为 ``-0``；
   - ``-Infinity ** y`` 在 ``y`` 为有限正奇整数时为 ``-Infinity``；
   - ``-Infinity ** y`` 在 ``y < 0`` 且不是奇整数时为 ``+0``；
   - ``-Infinity ** y`` 在 ``y > 0`` 且不是奇整数时为 ``+Infinity``；
   - ``+/-0 ** y`` 在 ``y < 0`` 且不是奇整数时为 ``+Infinity``；
   - ``+/-0 ** y`` 在 ``y > 0`` 且不是奇整数时为 ``+0``；
   - 当 ``x`` 是有限负数且 ``y`` 是有限非整数时，``x ** y`` 为 ``NaN``。

.. index::
   exponentiation
   binary operator
   operand
   base
   exponent
   bigint
   Math.pow
   NaN
   Infinity

.. _Additive Expressions:

加法类表达式
===============================================================================

.. meta:
    frontend_status: Done

加法类表达式使用加法运算符 ``+`` 和减法运算符 ``-``。其语法如下：

.. code-block:: abnf

       additiveExpression:
           expression '+' expression
           | expression '-' expression
           ;

加法类运算符按从左到右结合。

当使用 ``+`` 时，适用以下规则：

- 若任一操作数类型为 ``string``，则执行字符串拼接，见 :ref:`String Concatenation`；
- 若两个操作数类型都是 ``bigint``，则不做隐式转换，结果类型为 ``bigint``；
- 其他情况要求两个操作数都能通过 :ref:`Widening Numeric Conversions` 转换为
  :ref:`Numeric Types`。

否则发生 :index:`compile-time error`。

当使用 ``-`` 时，适用以下规则：

- 要么两个操作数都是 ``bigint``；
- 要么两个操作数都能转换为数值类型（见 :ref:`Widening Numeric Conversions` 和 :ref:`Numeric Types`）。

否则同样发生 :index:`compile-time error`。

合法的加法类表达式，其结果类型按下列规则决定：

- 若任一操作数是 ``string``，结果类型为 ``string``；
- 若两个操作数都是 ``bigint``，结果类型为 ``bigint``；
- 若两个操作数都可转换为数值类型，则结果类型由数值 widening 后的最大数值类型决定，
  规则与 :ref:`Multiplicative Expressions` 一致。

.. index::
   additive expression
   additive operator
   syntax
   string type
   operand
   string concatenation
   widening numeric conversion
   numeric type
   binary operator

.. _String Concatenation:

字符串拼接
===============================================================================

.. meta:
    frontend_status: Done

若表达式的一侧操作数类型为 ``string``，则运行时会先对另一侧操作数执行字符串转换，
参见 :ref:`String Operator Contexts`。

字符串拼接的结果是一个新的 ``string`` 对象引用，其内容等于左右两个操作数字符串按顺序连接：
左操作数的字符位于前，右操作数的字符位于后。

若该表达式不是 :ref:`Constant Expressions`，则运行时会创建新的 ``string`` 对象，
相关对象创建语义见 :ref:`New Expressions`。

.. index::
   string concatenation
   string type
   string object
   operand string
   operand
   string conversion
   operator context
   runtime
   constant expression
   object

.. _Additive Operators for Numeric Types:

数值类型的加法类运算符
===============================================================================

.. meta:
    frontend_status: Done

当 ``+`` 或 ``-`` 作用于数值类型时，首先执行数值提升（见 :ref:`Widening Numeric Conversions`）；若提升失败，则发生
:index:`compile-time error`。


.. index::
   additive operator
   conversion
   numeric types conversion
   numeric widening conversion
   numeric type
   numeric operand
   binary operator
   operand
   addition
   additive expression
   promoted type
   promoting
   integer arithmetic
   floating-point arithmetic
   integer
   expression

``+`` 计算两操作数之和，``-`` 计算差。结果类型是提升后的最大数值类型（见 :ref:`Numeric Types`，
另见 :ref:`Multiplicative Expressions` 的示例）：

- ``int`` 或 ``long`` 场景执行整数算术；
- ``float`` 或 ``double`` 场景执行浮点算术。

因此，数值加减法首先要解决“操作数被提升到什么类型”，
再根据该提升后类型进入整数算术或浮点算术语义。


.. index::
   operand expression
   expression
   side effect
   addition
   integer addition
   commutative operation
   associative operation
   floating-point addition
   integer overflow
   two's complement
   low-order bits

若操作数表达式没有副作用，则加法满足交换律；当所有整数操作数具有同一类型时，加法满足结合律。
浮点加法不满足结合律。

若整数加法发生溢出，则结果等于数学和在足够大二进制补码表示中的低位部分。
此时结果符号可能与数学和的符号相反。


.. index::
   IEEE 754
   floating-point addition
   NaN
   infinity
   signed infinity
   finite value
   positive zero
   negative zero
   overflow
   underflow
   round-to-nearest mode
   value set
   rounding
   loss of information

浮点加法遵循 IEEE 754：

- 任一操作数为 ``NaN``，或两个无穷大符号相反时，结果为 ``NaN``；
- 同号无穷大相加仍为该符号的无穷大；
- 无穷大与有限值相加得到无穷大；
- 正零与负零相加得到正零；
- 同号零相加得到该符号的零；
- 零与有限非零值相加，结果等于该有限非零操作数；
- 两个幅值相同、符号相反的有限非零值相加，结果为正零；
- 其余情况下按数学和计算，并按 round-to-nearest 模式舍入。

若结果幅值太大而无法表示，则发生上溢，结果是带适当符号的无穷大。
否则，结果舍入到当前 value set 中最接近的值。|LANG| 按 IEEE 754 要求支持渐进下溢（见 :ref:`Floating-Point Types and Operations`）。

这意味着只要参与的是数值类型（见 :ref:`Numeric Types`），加减法的重点就是 promoted type、IEEE 754 特殊值规则、
以及整数溢出后的二进制补码低位结果，而不是运行时异常。

对数值减法，``a - b`` 与 ``a + (-b)`` 在整数和浮点减法中具有相同的数学意义，
但对浮点零值，``0.0 - x`` 与一元负号 ``-x`` 的结果符号不总是相同。

例如，若 ``x`` 为 ``+0.0``，则 ``0.0 - x`` 的结果仍为 ``+0.0``，
而一元负号 ``-x`` 的结果是 ``-0.0``。这是浮点减法与一元取负不完全等价的边界情形。

数值加减法即使发生溢出、下溢或精度损失，也不会因此抛出错误。

这一点对 ``+`` 和 ``-`` 都成立：语言允许结果因为整数溢出、浮点舍入或精度损失而发生变化，
但不会仅因为这些现象本身抛出运行时错误。

.. code-block:: typescript
   :linenos:

    let i1: int = 5
    let i2: int = 7
    let d1: double = 5.0
    let d2: double = -5.0

    let s1 = i1 + i2
    let s2 = d1 + d2
    let s3 = d1 - d2
    let s4 = 3n + 4n
    let s5 = 3n - 4n

.. index::
   additive operator
   numeric addition
   numeric subtraction
   widening numeric conversion
   overflow
   underflow
   IEEE 754
   associativity
   commutativity
   side effect
   two's complement
   round-to-nearest mode
   finite value
   positive zero
   negative zero
   infinity
   NaN
   bigint
   value set
   integer arithmetic
   floating-point arithmetic
   subtraction
   negation
   promoted type
   rounding

.. _Shift Expressions:

移位表达式
===============================================================================

.. meta:
    frontend_status: Done

移位表达式使用 ``<<``、``>>`` 和 ``>>>``。左操作数是被移位值，右操作数表示移位距离。

.. code-block:: abnf

       shiftExpression:
           expression '<<' expression
           | expression '>>' expression
           | expression '>>>' expression
           ;

.. index::
   shift
   shift expression
   shift distance
   shift operator
   signed right shift
   unsigned right shift
   operand
   syntax
   numeric type
   bigint type

移位运算按从左到右结合。两个操作数必须都是数值类型，或都是 ``bigint``：

- 若存在 ``float`` 或 ``double``，先分别截断为 ``int`` 或 ``long``；
- 左操作数若为 ``byte`` 或 ``short``，先提升为 ``int``；
- 若两个操作数都是 ``bigint``，则不做转换；
- 一个是 ``bigint``、另一个是普通数值类型，会导致 :index:`compile-time error`；
- 对 ``bigint`` 使用 ``>>>`` 也会导致 :index:`compile-time error`。

结果类型取决于左操作数提升后的类型。若左操作数提升为 ``int``，则只取右操作数最低
5 位作为移位距离；若左操作数提升为 ``long``，则只取最低 6 位作为移位距离。

换句话说，``int`` 类型移位距离总落在 ``0`` 到 ``31`` 之间，``long`` 类型移位距离总落在
``0`` 到 ``63`` 之间。

.. index::
   shift expression
   promoted type
   promotion
   lowest-order bit
   shift distance
   bitwise logical AND operator
   mask value
   value
   truncation
   integer division

运行时在左操作数的二进制补码表示上执行移位：

- ``n << s`` 等价于乘以 :math:`2^s`，即使溢出也如此；
- ``n >> s`` 为带符号右移，使用符号扩展；
- ``n >>> s`` 为无符号右移，使用零扩展。

更具体地说：

- 当 ``n`` 非负时，``n >>> s`` 与 ``n >> s`` 的结果相同；
- 当 ``n`` 为负且左操作数类型为 ``int`` 时，``n >>> s`` 等价于
  ``(n >> s) + (2 << ~s)``；
- 当 ``n`` 为负且左操作数类型为 ``long`` 时，``n >>> s`` 等价于
  ``(n >> s) + ((2 as long) << ~s)``。

.. code-block:: typescript
   :linenos:

    let i: int = -8
    let j: int = 1
    let l: long = 8

    let r1 = i << j
    let r2 = i >> j
    let r3 = i >>> j
    let r4 = l << 2

    let bi = 8n
    let r5 = bi >> 1n

移位表达式只改变位模式，不会因为上溢、被截断的移位距离或符号扩展而抛出错误。
真正的静态限制只来自操作数类型不兼容，例如 ``bigint`` 与普通数值类型混用，
或对 ``bigint`` 使用 ``>>>``。

.. index::
   shift expression
   shift operator
   signed right shift
   unsigned right shift
   shift distance
   truncation
   bigint
   zero-extension
   sign-extension
   two's-complement integer
   promotion
   lowest-order bit
   mask value
   left shift
   right shift
   zero-extension
   overflow

.. _Relational Expressions:

关系表达式
===============================================================================

.. meta:
    frontend_status: Done

关系表达式使用 ``<``、``>``、``<=`` 和 ``>=``。其语法如下：

.. code-block:: abnf

       relationalExpression:
           expression '<' expression
           | expression '>' expression
           | expression '<=' expression
           | expression '>=' expression
           ;

关系运算符按从左到右结合，任意关系表达式的结果类型都为 ``boolean``。
关系表达式的具体语义取决于操作数类型；若操作数类型不属于本章规定的几类比较之一，
则发生 :index:`compile-time error`。

也就是说，关系表达式并不是单一规则，而是一组按操作数类型区分的比较规则。
只有当两个操作数的类型组合落入下文列出的合法类别时，该表达式才有定义的比较语义。

.. index::
   relational expression
   relational operator
   boolean type
   syntax
   expression
   operand
   operand type
   type

.. _Numeric Relational Operators:

数值关系运算符
===============================================================================

.. meta:
    frontend_status: Done

数值关系运算符要求每个操作数都属于 :ref:`Numeric Types`，并遵守
:ref:`Numeric Conversions for Relational and Equality Operands` 中规定的转换规则。
否则发生 :index:`compile-time error`。

从任意 value set 中取出的浮点值在比较时都必须保持精确。
一旦进入浮点比较分支，就必须额外遵守 IEEE 754 对 ``NaN``、无穷大和有符号零的规则。


.. index::
   numeric relational operator
   operand
   convertible type
   conversion
   numeric type
   numeric types conversion
   signed integer comparison
   floating-point comparison
   converted type

转换后：

- 若操作数类型为 ``int`` 或 ``long``，执行有符号整数比较；
- 若操作数类型为 ``float`` 或 ``double``，执行浮点比较。


.. index::
   floating-point value
   floating-point comparison
   comparison
   NaN
   finite value
   infinity
   negative infinity
   positive infinity
   positive zero
   negative zero
   IEEE 754

浮点比较遵守 IEEE 754：

- 任一操作数为 ``NaN`` 时，比较结果总是 ``false``；
- 除 ``NaN`` 外的值必须形成全序，其中负无穷小于所有有限值，正无穷大于所有有限值；
- 正零与负零视为相等。

在上述前提下，对整数操作数以及除 ``NaN`` 之外的浮点操作数，适用以下规则：

- ``<`` 在左值小于右值时返回 ``true``；
- ``<=`` 在左值小于等于右值时返回 ``true``；
- ``>`` 在左值大于右值时返回 ``true``；
- ``>=`` 在左值大于等于右值时返回 ``true``。

换句话说，除 ``NaN`` 外，数值关系运算对整数值和浮点值都遵循通常的大小关系；
``NaN`` 是唯一会让四个关系运算结果全部为 ``false`` 的特殊值。

这里的关键差别在于：整数比较按有符号整数顺序执行；浮点比较则必须把 ``NaN``、
正负无穷以及有符号零这些 IEEE 754 特例一起考虑进去。

.. code-block:: typescript
   :linenos:

      // integer comparisons
      console.log( 1 < -3) // false
      console.log(-1 as long >= -1 as short) // true

      // floating-point comparisons
      console.log(1 <= 1.0f)  // true
      console.log(2 <= 1.0)   // false


.. index::
   numeric relational operator
   integer comparison
   floating-point comparison
   NaN
   infinity
   IEEE 754
   positive zero
   negative zero
   operand type
   integer operand
   floating-point operand
   operator
   value

.. _Bigint Relational Operators:

``bigint`` 关系运算符
===============================================================================

.. meta:
    frontend_status: Done

关系运算可以用于 :ref:`Type bigint` 值，只要满足以下任一条件：

- 两个操作数都是 ``bigint``；
- 一个操作数是 ``bigint``，另一个是普通数值类型。

.. index::
   relational operator
   operand
   numeric type
   numeric types conversion
   bigint type
   bigint comparison

第二种情形下，比较按下列规则执行：

- 若普通数值是整数类型，则先转换为 ``bigint``，再执行 ``bigint`` 比较；
- 若普通数值是 ``double``，则把 ``bigint`` 转换为 ``double`` 后进行浮点比较；
- 若普通数值是 ``float``，则两个操作数都转换为 ``double`` 后比较。

也就是说，``bigint`` 和普通整数类型混用时优先转成 ``bigint`` 比较；
与浮点类型混用时则退化为浮点比较。

.. code-block:: typescript
   :linenos:

       let b = 2n

       // bigint against bigint
       console.log(b < 3n)  // true
       console.log(b >= 3n) // false

       // bigint against an integer type
       console.log(b < 3)         // true, '2' is converted implicitly to bigint
       console.log(b < BigInt(3)) // true, the same with explicit conversion

      // bigint against double
      console.log(b < 3.0)               // true, 'b' is converted to double
      console.log(b.doubleValue() < 3.0) // the same with explicit conversion

      // bigint against float
      const f = 3.f
      console.log(b < f) // true, 'b' and 'f' are converted to bigint

.. index::
   bigint relational operator
   bigint comparison
   numeric conversion
   bigint
   double
   float
   integer type
   floating-point comparison

.. _String Relational Operators:

字符串关系运算符
===============================================================================

.. meta:
    frontend_status: Done

所有字符串比较的结果都按下列规则定义：

- 运算符 ``<``：当左操作数字符串值按词典序小于右操作数字符串值时返回 ``true``，否则返回 ``false``；
- 运算符 ``<=``：当左操作数字符串值按词典序小于或等于右操作数字符串值时返回 ``true``，否则返回 ``false``；
- 运算符 ``>``：当左操作数字符串值按词典序大于右操作数字符串值时返回 ``true``，否则返回 ``false``；
- 运算符 ``>=``：当左操作数字符串值按词典序大于或等于右操作数字符串值时返回 ``true``，否则返回 ``false``。

.. code-block:: typescript
   :linenos:

    console.log("aa" < "ab")
    console.log("aa" >= "aa")
    console.log("bb" < "ab")

.. index::
   operator
   string relational operator
   string comparison
   lexicographical order
   string value

.. _Boolean Relational Operators:

布尔关系运算符
===============================================================================

.. meta:
    frontend_status: Done

所有布尔比较的结果都按下列规则定义：

- 运算符 ``<``：仅当左操作数为 ``false`` 且右操作数为 ``true`` 时返回 ``true``，否则返回 ``false``；
- 运算符 ``<=``：

  - 当两个操作数都为 ``true`` 时返回 ``true``；
  - 当左操作数为 ``false`` 时，不论右操作数为何值都返回 ``true``；
  - 当左操作数为 ``true`` 且右操作数为 ``false`` 时返回 ``false``；

- 运算符 ``>``：仅当左操作数为 ``true`` 且右操作数为 ``false`` 时返回 ``true``，否则返回 ``false``；
- 运算符 ``>=``：

  - 当两个操作数都为 ``false`` 时返回 ``true``；
  - 当左操作数为 ``true`` 时，不论右操作数为何值都返回 ``true``；
  - 当左操作数为 ``false`` 且右操作数为 ``true`` 时返回 ``false``。

换句话说，``false`` 视为较小值，``true`` 视为较大值。

.. code-block:: typescript
   :linenos:

    console.log(false < true)
    console.log(true <= false)
    console.log(true >= false)

.. index::
   operator
   operand
   boolean relational operator
   boolean comparison
   relational operator
   boolean operand

.. _char Relational Operators:

``char`` 关系运算符
===============================================================================

.. meta:
    frontend_status: Done

``char`` 的关系比较按其 16 位码元值的数值顺序执行。具体比较语义与运算规则见
:ref:`char Operations`。

.. index::
   char relational operator
   code unit
   char operations

.. _Enumeration Relational Operators:

枚举关系运算符
===============================================================================

.. meta:
    frontend_status: Done

若两个操作数属于同一枚举类型（见 :ref:`Enumerations`），则根据枚举底层类型选择：

- 数值枚举使用 :ref:`Numeric Relational Operators`；
- 字符串枚举使用 :ref:`String Relational Operators`。

若两个操作数不是同一枚举类型，则发生 :index:`compile-time error`。

.. index::
   enumeration relational operator
   enumeration type
   base type
   string value
   relational operator
   numeric relational operator
   string relational operator
   compile-time error

.. _Equality Expressions:

相等表达式
===============================================================================

.. meta:
    frontend_status: Done

相等表达式使用 ``==``、``===``、``!=`` 和 ``!==``。其语法如下：

.. code-block:: abnf

       equalityExpression:
           expression ('==' | '===' | '!=' | '!==') expression
           ;

相等运算符按从左到右结合；若操作数表达式没有副作用，则它们满足交换律。
任何相等表达式的类型都是 ``boolean``，其优先级低于关系运算符。

.. index::
   equality operator
   equality expression
   syntax
   boolean type
   side effect
   commutative operator
   relational operator
   precedence



``a != b`` 与 ``!(a == b)`` 的结果始终一致，``a !== b`` 与 ``!(a === b)``
的结果也始终一致。除 ``null`` 与 ``undefined`` 的比较外，``==`` 与 ``===``
在本规范中的结果相同，差异见 :ref:`Extended Equality with null or undefined`。


.. index::
   operator
   comparison
   value
   evaluation

若两个操作数类型不属于本节列出的可比较类别，则发生 :index:`compile-time error`。

使用 ``==`` 或 ``===`` 的比较在以下情况下求值为 ``true``：

- 两个操作数都是 :ref:`Type boolean` 且值相同；
- 两个操作数都是 :ref:`Type string` 或字符串字面量类型（见 :ref:`String Literal Types`），且内容相同；
- 两个操作数都是 :ref:`Type bigint` 且值相同；
- 两个操作数都是 :ref:`Numeric Types`，且经过数值转换（见 :ref:`Widening numeric conversions`、:ref:`Numeric Conversions for Relational and Equality Operands`）后值相同（``NaN`` 除外，详见 :ref:`Numeric Equality Operators`）；
- 两个操作数都是 :ref:`Type char` 且表示相同的 16 位码元（见 :ref:`char Operations`）；
- 一个是 ``char``，另一个是数值类型，且二者数值相同（见 :ref:`char Operations`、:ref:`char Conversions for Relational and Equality Operands`）；
- 两个操作数都是枚举类型（见 :ref:`Enumerations`），且底层数值或字符串值相同；
- 两个函数引用指向同一函数对象（详见 :ref:`Function Type Equality Operators`）；
- 两个操作数具有相同类型并引用同一对象。


.. index::
   char type
   char operations
   enum type
   function reference
   function object
   object reference
   same object
   same type

若比较的是不同类别的值，则结果为 ``false``，除非适用
:ref:`Extended Equality with null or undefined` 的特殊规则。

相等比较始终基于操作数的**实际类型**在运行时求值，而不是只看静态声明类型：

.. code-block:: typescript
   :linenos:

       function equ(a: Object, b: Object): boolean {
           return a == b
       }

       equ(1, 1) // true, values are compared
       equ(1, 2) // false, value are compared

       equ("aa", "aa") // true, string contexts are compared
       equ(1, "aa")    // false, not compatible types

       interface I1 {}
       interface I2 {}

       function equ1 (i1: I1, i2: I2) {
          return i1 == i2 // to be resolved during program execution
       }
       class A implements I1, I2 {}
       const a = new A
       equ1 (a, a) // true, the same values


联合类型上的相等比较如下：

.. code-block:: typescript
   :linenos:

       function f1(x: number | string, y: boolean | undefined): boolean {
           return x == y // Compile-time warning, always evaluates to false
       }

       function f2(x: number | string, y: boolean | "abc"): boolean {
           // OK, can be evaluated as true
           return x == y
       }

.. index::
   compile-time warning
   warning
   runtime evaluation
   actual type

若编译器能证明某个相等表达式在运行时恒为 ``true`` 或恒为 ``false``，则会发出
:index:`compile-time warning`。不同枚举类型之间的比较也可能触发同类 warning，
因为它通常意味着非预期代码：

.. code-block:: typescript
   :linenos:

       5 == 5  // Compile-time warning, always true
       5 != 5  // Compile-time warning, always false
       5 === 5 // Compile-time warning, always true
       5 !== 5 // Compile-time warning, always false

       function  foo(b: boolean | undefined) {
           let n: number | boolean = 1
           b == n // Compile-time warning, expression is always false
                  // (type of `n` is `number` and does not overlap with type of `b`)
           n == 1 // Compile-time warning, expression is
                  // always true because `n` initialized with 1 and never modified
       }

       class B {
           f(): B|undefined { return undefined }
       }
       class D extends B {
           f(): D { return this }
       }

       function bar(c: B) {
           if (c instanceof D) {
               c.f() == undefined // warning, expression is always false
           }
       }

.. code-block:: typescript
   :linenos:


       enum E1 {A, B}
       enum E2 {C, D}

       function test (e1: E1, e2: E2) {
          e1 == e2 // Compile-time warning, appears to be unintentional
       }

.. index::
   equality expression
   equality operator
   boolean type
   precedence
   compile-time warning
   compile-time error
   union type
   enum comparison
   operand
   actual type
   function object
   object reference
   same type
   same object

.. _Numeric Equality Operators:

数值相等运算符
===============================================================================

.. meta:
    frontend_status: Done

数值相等运算符要求每个操作数都是 :ref:`Numeric Types`，或可按
:ref:`Numeric Conversions for Relational and Equality Operands`
转换为 :ref:`Type bigint`。否则发生 :index:`compile-time error`。

若一侧类型比另一侧更窄（见 :ref:`Numeric Types`），则可能先进行 :ref:`Widening Numeric Conversions`。转换后：

- 若结果类型为 ``int`` 或 ``long``，执行整数相等性比较；
- 若结果类型为 ``float`` 或 ``double``，执行浮点相等性比较。

.. index::
   numeric equality
   numeric equality operator
   widening conversion
   convertible type
   conversion
   value equality
   operator
   numeric type
   numeric types conversion

浮点相等性比较遵守 IEEE 754 规则：

- 任一操作数为 ``NaN`` 时，``==`` 和 ``===`` 为 ``false``，而 ``!=`` 和 ``!==`` 为 ``true``；
- 正零与负零相等；
- 除此之外，两个不同的浮点值均不相等；
- ``x != x`` 或 ``x !== x`` 仅在 ``x`` 为 ``NaN`` 时才为 ``true``；
- 正无穷和负无穷各自都与自身相等，但与其他任何值都不相等。

因此，对非 ``NaN`` 的整数或浮点值：

- 左右值相等时，``==`` 和 ``===`` 返回 ``true``；
- 左右值不等时，``!=`` 和 ``!==`` 返回 ``true``。

.. code-block:: typescript
   :linenos:

      5 == 5 // true
      5 != 5 // false

      5 === 5 // true

      5 == new Number(5) // true
      5 === new Number(5) // true

      5 == 5.0 // true

若两个操作数都是 ``bigint`` 且值相同，则 ``==`` 和 ``===`` 返回 ``true``；
否则返回 ``false``。若一侧是 ``bigint``、另一侧是普通数值类型，则比较结果为 ``false``。

换句话说，``bigint`` 与普通数值类型之间不会通过隐式数值转换进行相等性比较。

.. code-block:: typescript
   :linenos:

       function foo() {}
       function bar() {}
       function goo(p: number) {}

       foo == foo
       foo == bar
       foo == goo

       class A {
          method() {}
          static method() {}
          foo () {}
       }
       const a = new A
       a.method == a.method
       A.method == A.method

       const aa = new A
       a.method == aa.method /*
            as 'a' and 'aa' are different bounded objects */
       a.method == a.foo


.. index::
   numeric equality operator
   widening conversion
   floating-point equality
   integer equality
   NaN
   positive zero
   negative zero
   bigint
   IEEE 754
   implicit conversion
   positive infinity
   negative infinity
   operand
   converted type
   value equality

.. _Function Type Equality Operators:

函数类型相等运算符
===============================================================================

.. meta:
    frontend_status: Done

若两个操作数引用同一个函数对象，则函数相等比较返回 ``true``。
对方法引用而言，不仅引用的方法本体必须相同，其绑定实例也必须相同。

.. code-block:: typescript
   :linenos:

    function foo() {}
    function bar() {}
    function goo(p: number) {}

    foo == foo // true, same function object
    foo == bar // false, different function objects
    foo == goo // false, different function objects

    class A {
        method() {}
        static method() {}
        foo() {}
    }
    const a = new A()
    const aa = new A()

    a.method == a.method // true, same function object
    A.method == A.method // true, same function object
    a.method == aa.method /* false, different function objects
         as 'a' and 'aa' are different bounded objects */
    a.method == a.foo // false, different function objects

.. index::
   function type equality operator
   equality operator
   function object
   instance
   method reference
   bound instance
   bound object

.. _Extended Equality with null or undefined:

与 ``null`` 或 ``undefined`` 的扩展相等比较
===============================================================================

.. meta:
    frontend_status: Done

为更好地与 |TS| 对齐，|LANG| 为 ``null`` 与 ``undefined`` 提供扩展相等语义。

当相等表达式的一个操作数是 ``null``，另一个是 ``undefined`` 时：

- ``==`` 的结果是 ``true``；
- ``===`` 的结果是 ``false``；
- ``!=`` 的结果是 ``true``；
- ``!==`` 的结果是 ``false``。

.. code-block:: typescript
   :linenos:

       function foo(x: Object | null, y: Object | null | undefined) {
           console.log(x == y, x === y)
       }

       foo(null, undefined) // output: true, false
       foo(null, null)      // output: true, true


``null`` 与 ``undefined`` 的直接比较也是合法的：

.. code-block:: typescript
   :linenos:

       console.log(null == undefined)  // output: true
       console.log(null === undefined) // output: false

.. index::
   extended equality
   null
   undefined
   ts compatibility

.. _Bitwise and Logical Expressions:

位运算与逻辑表达式
===============================================================================

.. meta:
    frontend_status: Done

这一组表达式包括 ``&``、``^``、``|`` 三类按位/逻辑运算。其语法如下：

.. code-block:: abnf

       bitwiseAndLogicalExpression:
           expression '&' expression
           | expression '^' expression
           | expression '|' expression
           ;

这三个运算符的优先级不同，其中 ``&`` 最高、``|`` 最低；它们都按从左到右结合。
若操作数表达式没有副作用，这些运算符满足交换律和结合律。

允许的操作数组合只有两类：

- 两个 ``boolean``；
- 两个数值类型，或两个 ``bigint``。

否则发生 :index:`compile-time error`。

.. index::
   bitwise expression
   logical expression
   operator precedence
   compile-time error
   syntax
   boolean
   numeric type
   exclusive OR operator
   inclusive OR operator
   AND operator
   commutative operator
   associativity

.. _Integer Bitwise Operators:

整数位运算符
===============================================================================

.. meta:
    frontend_status: Done

整数位运算符是作用于数值类型（见 :ref:`Numeric types`）或 ``bigint`` 的 ``&``、``^``、``|``。
若两个操作数中有一方或双方不是合适的整数表示形式，则必须先完成本节规定的截断或提升。

- 若存在 ``double`` 或 ``float``，则先截断为相应整数类型；
- 若操作数是 ``byte`` 或 ``short``，则先提升为 ``int``；
- 若两个整数类型不同，则按 :ref:`Widening Numeric Conversions` 把较小类型提升为较大类型；
- 若两个操作数都是 ``bigint``，则不做转换；
- 若一个操作数为 ``bigint``、另一个为普通数值类型，则发生 :index:`compile-time error`。

结果类型与操作数提升后的类型相同。运行时结果分别是：

- ``&`` 返回按位与；
- ``^`` 返回按位异或；
- ``|`` 返回按位或。

.. index::
   integer bitwise operator
   numeric types conversion
   numeric type
   bigint
   bigint type
   truncation
   conversion
   widening numeric conversion
   convertibility
   integer type
   types conversion
   bitwise and
   bitwise or
   bitwise xor
   bitwise AND operand
   bitwise inclusive OR operand
   bitwise exclusive OR operand
   operand value
   expression type

.. _Boolean Logical Operators:

布尔逻辑运算符
===============================================================================

.. meta:
    frontend_status: Done

布尔逻辑运算符这里指作用于 ``boolean`` 操作数的 ``&``、``^`` 和 ``|``：

- ``a & b`` 仅当两个操作数都为 ``true`` 时结果为 ``true``；
- ``a ^ b`` 仅当两个操作数不同时时结果为 ``true``；
- ``a | b`` 仅当两个操作数都为 ``false`` 时结果为 ``false``。

因此，布尔逻辑表达式的结果类型始终是 ``boolean``。

.. index::
   boolean operator
   boolean logical operator
   boolean type
   operand value
   boolean logical expression
   logical and
   logical or
   logical xor

.. _Conditional-And Expression:

条件与表达式
===============================================================================

.. meta:
    frontend_status: Done

条件与运算符 ``&&`` 与 ``&``（见 :ref:`Bitwise and Logical Expressions`）在布尔结果上相同，但它只在左操作数为 ``true`` 时才会求值
右操作数，因此具有短路语义。其语法如下：

.. code-block:: abnf

       conditionalAndExpression:
           expression '&&' expression
           ;

``&&`` 按从左到右结合，并且在结果和值副作用两方面都满足结合律。

也就是说，``((a && b) && c)`` 与 ``(a && (b && c))`` 对任意 ``a``、``b``、``c``
都会产生相同的结果和值副作用顺序。

.. index::
   conditional-and operator
   conditional-and expression
   boolean operand
   syntax
   conditional evaluation
   evaluation
   expression
   side effect
   extended conditional expression

若不考虑扩展条件表达式语义（见 :ref:`Extended Conditional Expressions`），条件与表达式的类型恒为 ``boolean``。每个操作数都必须是
``boolean``，或属于 :ref:`Extended Conditional Expressions` 允许的类型；
否则发生 :index:`compile-time error`。

运行时先求值左操作数：

- 若结果为 ``false``，则整个表达式结果为 ``false``，右操作数不再求值；
- 若结果为 ``true``，则继续求值右操作数，并以其结果作为整个表达式结果。

.. index::
   conditional-and operator
   conditional-and expression
   short circuit
   associativity
   boolean type
   compile-time error
   evaluation
   expression
   side effect
   boolean operand

.. _Conditional-Or Expression:

条件或表达式
===============================================================================

.. meta:
    frontend_status: Done

条件或运算符 ``||`` 与 ``|``（见 :ref:`Integer Bitwise Operators`）在布尔结果上相同，但它只在左操作数为 ``false`` 时才会求值
右操作数，因此同样具有短路语义。其语法如下：

.. code-block:: abnf

       conditionalOrExpression:
           expression '||' expression
           ;

``||`` 按从左到右结合，并且在结果和值副作用两方面满足结合律。

也就是说，``((a || b) || c)`` 与 ``(a || (b || c))`` 对任意 ``a``、``b``、``c``
都会产生相同的结果和值副作用顺序。

.. index::
   conditional-or expression
   conditional-or operator
   operand
   syntax
   associativity
   expression
   side effect
   evaluation
   boolean type
   extended conditional expression

除扩展条件表达式（见 :ref:`Extended Conditional Expressions`）语义外，条件或表达式的类型恒为 ``boolean``。每个操作数都必须是 ``boolean``，
或属于 :ref:`Extended Conditional Expressions` 允许的类型；否则发生
:index:`compile-time error`。

运行时先求值左操作数：

- 若结果为 ``true``，则整个表达式结果为 ``true``，右操作数不再求值；
- 若结果为 ``false``，则继续求值右操作数，并以其结果作为整个表达式结果。

.. index::
   conditional-or operator
   conditional-or expression
   short circuit
   associativity
   boolean type
   compile-time error
   evaluation
   expression
   side effect

|

.. _Assignment:

赋值
*******************************************************************************

.. meta:
    frontend_status: Done

赋值表达式把右值结果赋给左值表达式。
所有赋值运算符都按从右到左结合，也就是说 :math:`a=b=c` 等价于
:math:`a=(b=c)`。因此，先把 ``c`` 的值赋给 ``b``，再把 ``b`` 的值赋给 ``a``。

其语法如下：

.. index::
   assignment
   assignment operator
   syntax
   assignment expression
   operand
   variable
   expression

.. code-block:: abnf

       assignmentExpression
           : lhsExpression assignmentOperator rhsExpression
           | destructuringAssignment
           ;

       assignmentOperator
           : '='
           | '+='  | '-='  | '*='   | '/='  | '%=' | '**='
           | '<<=' | '>>=' | '>>>='
           | '&='  | '|='  | '^='
           ;

       lhsExpression:
           expression
           ;

       rhsExpression:
           expression
           ;

赋值运算符的第一个操作数 ``lhsExpression`` 必须是 *左值表达式*
（参见 :ref:`Left-Hand-Side Expressions`），也就是某个变量位置。
``destructuringAssignment`` 的语义见 :ref:`Destructuring Assignment`。

赋值表达式本身的类型是被赋值变量的类型。运行时结果不是变量对象本身，
而是赋值完成后变量所保存的值。

因此，赋值表达式参与更大表达式求值时，贡献的是“赋值后的值”，不是变量位置本身。

.. index::
   assignment expression
   assigned value
   variable type
   runtime

.. _Simple Assignment Operator:

简单赋值运算符
===============================================================================

.. meta:
    frontend_status: Done

简单赋值 ``=`` 直接把右侧结果写入左值目标。

简单赋值表达式形如 ``lhsExpression = rhsExpression``。
下列情况会导致 :index:`compile-time error`：

- ``rhsExpression`` 的类型不可赋值（见 :ref:`Assignability`）给 ``lhsExpression`` 所引用变量的类型；
- ``lhsExpression`` 的类型为：

  - ``readonly`` 数组（见 :ref:`Readonly Parameters`），而 ``rhsExpression`` 转换后的类型是非 ``readonly`` 数组；
  - ``readonly`` 元组（见 :ref:`Readonly Parameters`），而 ``rhsExpression`` 转换后的类型是非 ``readonly`` 元组。

运行时，简单赋值会按左值种类执行不同流程：字段访问、数组索引、记录索引，
以及一般变量左值。它们都遵循同一原则：先按规定顺序求值左侧所需子表达式，
再求右值；若任一步骤异常完成，则本次赋值异常完成且不发生写入。

1. 若 lhsExpression 是字段访问表达式 e.f（见 :ref:`字段访问表达式 <Field Access Expression>`）：

   - 先求值 ``e``；若异常完成，则整个赋值表达式异常完成；
   - 再求值 ``rhsExpression``；若异常完成，则整个赋值表达式异常完成；
   - 若两步都正常完成，则先把 ``rhsExpression`` 的值转换为字段 ``f`` 的类型，
     再把转换结果写入该字段。

2. 若 ``lhsExpression`` 是数组引用表达式（见 :ref:`Array Indexing Expression`）：

   - 先求值数组引用子表达式；若异常完成，则索引子表达式与 ``rhsExpression`` 均不求值，
     且不发生赋值；
   - 若数组引用正常完成，则求值索引子表达式；若异常完成，则 ``rhsExpression`` 不再求值，
     且不发生赋值；
   - 若索引子表达式也正常完成，则求值 ``rhsExpression``；若异常完成，则不发生赋值；
   - 若索引值小于 0 或大于等于数组长度，则抛出 ``RangeError``，且不发生赋值；
   - 若 ``lhsExpression`` 表示固定长度数组元素，而 ``rhsExpression`` 的类型不是元素类型的子类型，
     则抛出 ``ArrayStoreError``，且不发生赋值；
   - 否则，以索引值选出数组元素，把 ``rhsExpression`` 的值转换为元素类型后写入该元素。

.. index::
   operand
   array reference expression
   array indexing expression
   reference subexpression
   assignment
   assignment expression
   abrupt completion
   normal completion
   subexpression
   evaluation
   array length
   variable
   conversion
   array element
   value set
   extended exponent
   reference type
   assignability
   runtime
   RangeError
   ArrayStoreError
   fixed-size array

3. 若 ``lhsExpression`` 是记录访问表达式（见 :ref:`Record Indexing Expression`）：

   - 先求值对象引用子表达式；若异常完成，则索引子表达式与 ``rhsExpression`` 均不求值，
     且不发生赋值；
   - 若对象引用正常完成，则求值索引子表达式；若异常完成，则 ``rhsExpression`` 不再求值，
     且不发生赋值；
   - 若索引子表达式也正常完成，则求值 ``rhsExpression``；若异常完成，则不发生赋值；
   - 否则，将索引子表达式的值作为 ``key``，把 ``rhsExpression`` 的值转换为记录值类型后
     作为 ``value``，并把这个 key-value 对写入记录实例。

.. index::
   operand
   record access expression
   record indexing expression
   reference subexpression
   index subexpression
   assignment
   key-value pair
   conversion
   runtime

4. 若以上情况都不满足，则：

   - 先把 ``lhsExpression`` 求值为变量；若异常完成，则 ``rhsExpression`` 不求值，且不发生赋值；
   - 若左值正常完成，再求值 ``rhsExpression``；若异常完成，则不发生赋值；
   - 若右值也正常完成，则把右值转换为左值变量类型，并将转换结果写入该变量。

也就是说，普通变量赋值同样严格遵守“左值先于右值”的求值顺序。
只要左值求值、右值求值或类型转换任一步骤异常完成，这次赋值都不会写回任何结果。

.. index::
   evaluation
   operand
   assignment expression
   assignment
   abrupt completion
   normal completion
   conversion
   variable

换句话说，简单赋值统一遵守“先求左值所需引用，再求右值，再写回”的顺序；
只要中间任一步骤异常完成，就不会发生实际写入。

.. index::
   simple assignment operator
   assignment operator
   assignability
   readonly array
   readonly tuple
   access
   runtime
   abrupt completion
   normal completion
   field
   field type
   evaluation
   assignment expression
   variable
   array indexing expression
   record indexing expression
   RangeError
   ArrayStoreError
   key-value pair
   conversion
   subexpression
   readonly array
   readonly tuple

不同左值形态的示例如下：

.. code-block:: typescript
   :linenos:

      // Case 1: field access lhsExpression
      class A { f: double }
      new A().f = 1

      // Case 2: array indexing lhsExpression
      let b: double[] = [1, 1, 1 ]
      b[1] = 2

      // Case 3: record indexing lhsExpression
      type Keys = 'key1' | 'key2' | 'key3'
      let x: Record<Keys, number> = { 'key1': 1, 'key2': 2, 'key3': 3 }
      x['key2'] = 8

      // None of {field access|array indexing|record indexing}
      let c: double
      c = 1
      c += 2

.. _Compound Assignment Operators:

复合赋值运算符
===============================================================================

.. meta:
    frontend_status: Done

复合赋值在语义上等价于“读取左值、执行某种运算、再写回左值”，
但 ``lhsExpression`` 只会求值一次。形式
``lhsExpression op= rhsExpression`` 可理解为：
``lhsExpression = ((lhsExpression) op (rhsExpression)) as T``，
其中 ``T`` 是 ``lhsExpression`` 的类型（见 :ref:`Simple Assignment Operator`）。

因此，复合赋值与简单赋值的核心差别不是“是否写回”，而是
写回前必须先把保存下来的左值旧值与右值一起执行一次由 ``op`` 指定的二元运算。
若该运算异常完成，则整个赋值表达式也异常完成，且不会发生写回。

运行时主要有三类路径：

1. ``lhsExpression`` 不是索引表达式时：

   - 先把 ``lhsExpression`` 求值为变量；若异常完成，则 ``rhsExpression`` 不求值，
     且不发生赋值；
   - 若左值正常完成，则保存其旧值，并求值 ``rhsExpression``；若异常完成，则不发生赋值；
   - 若两侧都正常完成，则以保存的左值旧值和 ``rhsExpression`` 的值执行复合赋值对应的
     二元运算；若运算异常完成，则整个赋值表达式异常完成；
   - 若二元运算正常完成，则把结果转换为左值变量类型并写回变量。

   也就是说，在非索引左值上，复合赋值与简单赋值的差别只在于：
   先取出旧值参与一次由 ``op`` 指定的二元运算，再把运算结果按左值变量类型转换后写回。

2. ``lhsExpression`` 是数组索引表达式（见 :ref:`Array Indexing Expression`）时：

   - 先求值数组引用子表达式；若异常完成，则索引子表达式和 ``rhsExpression`` 均不求值；
   - 若数组引用正常完成，则求值索引子表达式；若异常完成，则 ``rhsExpression`` 不再求值；
   - 若索引值越界，则抛出 ``RangeError``；
   - 若索引合法，则先取出该数组元素的旧值并保存，再求值 ``rhsExpression``；
   - 若 ``lhsExpression`` 的静态类型是预定义值类型，则以保存的数组元素值和 ``rhsExpression`` 的值
     执行二元运算，再把结果转换为数组元素类型后写回；
   - 若 ``lhsExpression`` 的静态类型是引用类型，则这里只允许 ``string``，
     此时复合赋值只能表现为字符串拼接语义。
   - 如果上述二元运算异常完成，则整个赋值表达式异常完成，且不发生写回。

    这里之所以只允许 ``string``，是因为数组元素若是引用类型，就必须对应 ``string``；
    而 ``string`` 是 ``final`` 类，因此数组元素旧值与右值只会参与字符串拼接这一种复合赋值语义。

    也就是说，对数组元素执行复合赋值时，左值只求值一次；随后使用保存下来的旧元素值
    与右值执行二元运算。若该二元运算或结果转换异常完成，则整个赋值表达式异常完成，
    且不会写回数组。

    若该二元运算正常完成，且当前分支属于 ``string`` 引用类型情形，则得到的 ``string`` 结果
    会直接写回被选中的数组元素。

.. index::
   value
   array element
   operand
   expression
   array reference subexpression
   array indexing expression
   evaluation
   index subexpression
   normal completion
   abrupt completion
   assignment
   subexpression
   variable
   assignment operator
   predefined value type
   binary operation
   assignment expression
   conversion
   compound assignment operator
   reference type
   string
   array
   string concatenation

3. ``lhsExpression`` 是记录索引表达式（见 :ref:`Record Indexing Expression`）时：

   - 先求值对象引用子表达式；若异常完成，则索引子表达式与 ``rhsExpression`` 均不求值；
   - 若对象引用正常完成，则求值索引子表达式；若异常完成，则 ``rhsExpression`` 不再求值；
   - 若两者都正常完成，则保存对象引用值和索引值，再求值 ``rhsExpression``；
   - 若 ``rhsExpression`` 也正常完成，则取出该键当前映射的值，与右值一起执行对应的二元运算；
   - 若运算正常完成，则把该结果按 :ref:`Simple Assignment Operator` 的规则
     转换为记录值类型，再把转换后的结果作为新的 key-value 对写回记录实例；
   - 若二元运算异常完成，则整个赋值表达式异常完成，且记录实例不被修改。

.. index::
   record access expression
   operand expression
   record indexing expression
   object reference subexpression
   operand
   index subexpression
   evaluation
   assignment expression
   abrupt completion
   normal completion
   assignment
   key
   key-value pair
   indexing expression
   record instance
   value
   compound assignment operator
   binary operation

也就是说，记录索引（:ref:`Record Indexing Expression`）上的复合赋值会先以保存下来的 key 查出当前 value，
再用这个旧 value 与右值执行二元运算，最后把运算结果按记录 value 类型转换后，
才重新写回同一个 key。

记录索引上的复合赋值和简单赋值一样，也遵守“先保存 key，再计算 value，再写回”的顺序，
因而不会重复求值对象引用或索引表达式。

若任一中间步骤异常完成，则整个赋值表达式异常完成且不发生写入。

因此，复合赋值与简单赋值一样，都保证左侧索引表达式只求值一次；
差别仅在于复合赋值会先把旧值与右值执行由 ``op`` 指定的二元运算，再把结果按左值类型写回。

.. code-block:: typescript
   :linenos:

    let c: double = 1
    c += 2

   let arr: int[] = [1, 2, 3]
   arr[1] *= 4

   type Keys = 'k1' | 'k2'
   let rec: Record<Keys, number> = { 'k1': 1, 'k2': 2 }
   rec['k2'] += 3

.. index::
   compound assignment operator
   compound assignment expression
   nullish-coalescing assignment
   assignment operator
   indexing expression
   operand
   variable
   assignment
   abrupt completion
   normal completion
   assignment expression
   binary operation
   RangeError
   ArrayStoreError
   runtime
   evaluation
   conversion
   saved value
   string concatenation
   record instance
   key-value pair
   array element
   record indexing expression
   object reference subexpression
   index subexpression
   left-hand-side variable
   binary operation result

.. _Left-Hand-Side Expressions:

左值表达式
===============================================================================

.. meta:
    frontend_status: Done

能够出现在赋值左侧的表达式称为左值表达式。
左值表达式是引用以下任一实体的表达式：

- 变量（参见 :ref:`Named Reference`）；
- 形参（见 :ref:`Named Reference`），但名称为 ``this`` 的参数除外；
- 字段访问（见 :ref:`Field Access Expression`）得到的字段或 setter；
- 数组元素或记录元素访问（参见 :ref:`Indexing Expressions`）。

若表达式不是上述任一形式，则发生 :index:`compile-time error`。

若表达式包含链式运算符 "?."
（见 :ref:`链式运算符 <Chaining Operator>`），或引用的是只读实体，则发生
:index:`compile-time error`。

换句话说，左值表达式必须真正表示某个可写位置，而不能只是普通计算结果。

.. index::
   expression
   named variable
   field
   setter
   field access
   array element
   record element
   access
   indexing expression
   chaining operator
   variable
   parameter
   readonly entity

|

.. _Ternary Conditional Expressions:

三元条件表达式
*******************************************************************************

.. meta:
    frontend_status: Done

三元表达式 ``cond ? a : b`` 根据条件值选择第二或第三个分支。
其类型通常由两个分支的类型共同决定。

其语法如下：

.. code-block:: abnf

       ternaryConditionalExpression:
           expression '?' expression ':' expression
           ;

三元条件运算符按从右到左结合，因此
:math:`a?b:c?d:e?f:g` 与 :math:`a?b:(c?d:(e?f:g))` 语义相同。

.. 若首个表达式的类型不是 ``boolean``，或不是
   :ref:`Extended Conditional Expressions` 中列举的类型，则发生 :index:`compile-time error`。

.. index::
   ternary conditional expression
   ternary conditional operator
   boolean value
   expression
   operand
   right-to-left associativity
   separator

表达式 ``condition ? whenTrue : whenFalse`` 的类型按下列规则确定：

- 若 ``condition`` 在编译期可知为 ``true``，结果类型为 ``whenTrue`` 的类型；
- 若 ``condition`` 在编译期可知为 ``false``，结果类型为 ``whenFalse`` 的类型；
- 若 ``condition`` 在编译期未知，结果类型为 ``whenTrue`` 与 ``whenFalse`` 类型形成的联合类型，
  并按 :ref:`Union Types Normalization` 继续归一化。

运行时先求值 ``condition``：

- 若结果为 ``true``，则只求值 ``whenTrue``；
- 若结果为 ``false``，则只求值 ``whenFalse``。

成功求值得到的分支结果，就是整个三元条件表达式的结果。

.. code-block:: typescript
   :linenos:

       class A {}
       class B extends A {}

       // Assuming value of `condition` is unknown at compile time
       condition ? new A() : new B() // A | B => A

       condition ? 5 : 6             // int

       condition ? "5" : 6           // "5" | int
       true      ? "5" : 6           // "5"
       false     ? "5" : 6           // int

.. index::
   ternary conditional expression
   ternary conditional operator
   union type normalization
   right-to-left associativity
   operand expression

|

.. _String Interpolation Expressions:

字符串插值表达式
*******************************************************************************

.. meta:
    frontend_status: Done

字符串插值允许在字符串模板中嵌入表达式。
嵌入表达式会按指定顺序求值，并按字符串转换规则拼接到最终结果中。

字符串插值表达式本质上是多行字符串字面量，也就是使用反引号包围
（详见 :ref:`Multiline String Literal`），并且至少包含一个 *嵌入表达式* 的字符串字面量。

其语法如下：

.. code-block:: abnf

       stringInterpolation:
           '`' (BackticksContentCharacter | embeddedExpression)* '`'
           ;

       embeddedExpression:
           '${' expression '}'
           ;

嵌入表达式是写在花括号内、并由字符 $ 引入的表达式。
字符串插值表达式的类型始终是 ``string`` ，见 :ref:`Type String`。

求值时，每个嵌入表达式的结果都会替换自身位置。
嵌入表达式若不是 ``string`` 类型，则会执行与 :ref:`String Concatenation`
相同的隐式字符串转换。

.. index::
   string interpolation expression
   multiline string
   string literal
   backtick
   string type
   syntax
   embedded expression
   curly brace
   implicit conversion
   string concatenation operator

.. code-block:: typescript
   :linenos:

       let a = 2
       let b = 2
       console.log(`The result of ${a} * ${b} is ${a * b}`)
       // prints: The result of 2 * 2 is 4

上一示例等价于以下字符串拼接：

.. code-block:: typescript
   :linenos:

       let a = 2
       let b = 2
       console.log("The result of " + a + " * " + b + " is " + a * b)

嵌入表达式内部还可以继续包含嵌套的多行字符串。

.. index::
   nested multiline string
   multiline string
   string concatenation
   embedded expression

|

.. _Lambda Expressions:

Lambda 表达式
*******************************************************************************

.. meta:
    frontend_status: Done

lambda 表达式定义匿名函数值，可捕获外围作用域中的变量和 ``this`` 语义
（在允许的上下文中）。它通过可选注解使用（见 :ref:`Using Annotations`）、可选 ``async`` 标记（见 :ref:`Async Lambdas`）、
必需的 lambda 签名以及函数体，完整定义一个函数类型实例（见 :ref:`Function Types`）。
lambda 表达式的定义方式与函数声明（见 :ref:`Function Declarations`）总体类似，
区别在于 lambda 表达式没有函数名称，且可以省略参数类型。

其语法如下：

.. code-block:: abnf

       lambdaExpression:
           annotationUsage? 'async'? lambdaSignature '=>' lambdaBody
           ;

       lambdaBody:
           expression | block
           ;

       lambdaSignature:
           '(' lambdaParameterList? ')' returnType?
           | identifier
           ;

       lambdaParameterList:
           lambdaParameter (',' lambdaParameter)* (',' restParameter)? ','?
           | restParameter ','?
           ;

       lambdaParameter:
           annotationUsage? (lambdaRequiredParameter | lambdaOptionalParameter)
           ;

       lambdaRequiredParameter:
           identifier (':' type)?
           ;

       lambdaOptionalParameter:
           identifier '?' (':' type)?
           ;

       lambdaRestParameter:
           '...' lambdaRequiredParameter
           ;

.. index::
   lambda expression
   instance
   function type
   async mark
   type parameter
   lambda signature
   lambda body
   function declaration
   optional annotation
   annotation

注解的使用示例如下（参见 :ref:`Using Annotations`）：

.. code-block:: typescript
   :linenos:

       (x: number): number => { return Math.sin(x) } // block as lambda body
       (x: number) => Math.sin(x)                    // expression as lambda body
       e => e                                        // shortest form of lambda

lambda 表达式求值会创建一个函数类型（:ref:`Function Types`）实例，具体运行时行为见
:ref:`Runtime Evaluation of Lambda Expressions`。

.. index::
   lambda expression
   function type
   async lambda
   lambda signature
   lambda body
   lambda parameter
   annotation
   instance

.. _Lambda Signature:

Lambda 签名
===============================================================================

.. meta:
    frontend_status: Done

lambda 签名定义参数列表、可选返回类型及其约束。
与函数声明（见 :ref:`Function Declarations`）类似，lambda 签名由形式参数和可选返回类型组成。
在某些情况下，可通过 :ref:`Type Inference` 省略形式参数的类型标注。

.. code-block:: typescript
   :linenos:

       // Lambda expressions has no type annotations

       const func: (p: Object) => Object = e => e

       function foo<T>(p: (p: T) => T) {}
       foo <Object> (e => e)

作用域规则见 :ref:`Scopes`，形参遮蔽规则见 :ref:`Shadowing by Parameter`。

若 lambda 声明了两个同名形参，或某个形参既未写出类型又无法由 :ref:`Type Inference` 推断，
则发生 :index:`compile-time error`。

.. index::
   lambda signature
   return type
   lambda expression
   function declaration
   type annotation
   formal parameter
   optional parameter
   type inference
   annotation
   scope
   shadow parameter
   shadowing
   parameter declaration
   evaluation

.. _Lambda Body:

Lambda 体
===============================================================================

.. meta:
    frontend_status: Done

lambda 体可以是表达式体，也可以是代码块体（见 :ref:`Block`）。
其名称解析、``this`` 与 ``super`` 的语义，以及被引用声明的可访问性，
都与外围上下文一致，但 lambda 形参会引入新名字。
lambda 体描述了调用 lambda 表达式（见 :ref:`Function Call Expression`）时执行的代码。
若在 lambda 体中使用了外围局部变量或形式参数而未重新声明，则该变量会被 lambda *捕获*。

若在方法中定义的 lambda 使用了外围类型的实例成员，则 ``this`` 也会被 lambda 捕获。
如果局部变量在 lambda 体中被使用，但既未在体内声明，也没有在使用前完成赋值，
则会发生 :index:`compile-time error`。

若 lambda 体是单个表达式，则：

- 若该表达式是返回类型为 ``void`` 的调用表达式，则等价于 ``{ expression }``；
- 否则等价于 ``{ return expression }``。

若 lambda 签名返回类型既不是 void
（见 :ref:`类型 undefined 或 void <Type undefined or void>`）也不是 never
（见 :ref:`类型 never <Type never>`），
但 lambda 体的某条执行路径没有 return 语句（见 :ref:`Return Statements`）且也不是单表达式体，
则发生 :index:`compile-time error`。

.. index::
   lambda body
   lambda
   expression
   block
   method body
   function body
   lambda expression
   lambda expression call
   captured variable
   this capture
   this keyword
   super keyword
   return statement
   call expression
   context
   accessibility
   lambda signature
   surrounding type
   runtime
   evaluation

.. _Lambda Expression Type:

Lambda 表达式的类型
===============================================================================

.. meta:
    frontend_status: Done

lambda 表达式的类型通常是某个函数类型（见 :ref:`Function Types`）或兼容的上下文目标类型。
它的核心类型始终是某个函数类型，其组成如下：

- 若存在 lambda 参数，则这些参数成为该函数类型的参数；
- lambda 的返回类型成为该函数类型的返回类型。

若返回类型可由 lambda 体推断，则可以省略显式返回类型。

.. note::
   这意味着在不少上下文里，lambda 只需要给出参数列表和函数体；
   只要返回类型能够从 lambda 体推断出来，就不必显式写出。

.. code-block:: typescript
   :linenos:

         const lambda = () => { return 123 }  // Type of the lambda is () => int
         const int_var: int = lambda()


.. index::
   lambda expression type
   function type
   lambda expression
   lambda parameter
   return type
   inferred type
   lambda body

.. _Runtime Evaluation of Lambda Expressions:

Lambda 表达式的运行时求值
===============================================================================

.. meta:
    frontend_status: Done

运行时求值 lambda 会创建可调用对象，并绑定其捕获环境。
lambda 表达式自身的求值不会执行 lambda 体；它只会在运行时创建新的函数类型（见 :ref:`Function Types`）实例，
并把外围变量按规则捕获进来。每次求值都会得到新的函数实例，
类似于类实例创建表达式（见 :ref:`New Expressions`）的求值方式。

若没有足够空间分配新的函数对象，则 lambda 表达式会异常完成，并抛出
``OutOfMemoryError``。

.. code-block:: typescript
   :linenos:

        function foo() {
           let y: int = 1
           let x = () => { return y+1 } // 'y' is *captured*.
           console.log(x())             // Output: 2
        }

.. index::
   runtime evaluation
   lambda expression
   lambda body
   execution
   initialization
   function type
   lambda signature
   normal completion
   instance creation expression
   allocation

被捕获的变量并不是原变量的拷贝；如果 lambda 修改了该变量，外围作用域中观察到的
也是同一个变量的变化：

.. code-block:: typescript
   :linenos:

        function foo() {
          let y: int = 1
          let x = () => { y++ } // 'y' is *captured*.
          console.log(y) // Output: 1
          x()
          console.log(y) // Output: 2
        }

.. index::
   captured by lambda
   lambda
   variable
   captured variable
   original variable

函数作用域中的捕获示例如下：

.. code-block:: typescript
   :linenos:

        function capturingFunction() { // Function scope
          let v: number = 0 // A captured variable
          return  (p: number) => {
              console.log ("Previous value: ", v, " new value: ", p)
              v = p
          }
        }

        const func1 = capturingFunction ()
        const func2 = capturingFunction ()
        // Note: func1 and func2 are two different function type instances

        func1(11) // Previous value: 0 new value: 11
        func2(22) // Previous value: 0 new value: 22
        func1(33) // Previous value: 11 new value: 33
        func2(44) // Previous value: 22 new value: 44
        /* Note:
              func1 calls work with their own version of variable 'v'
              func2 calls work with their own version of variable 'v'
        */

循环作用域中的捕获示例如下：

.. code-block:: typescript
   :linenos:

        const l = () => {}
        const storage = [l, l, l, l, l]  // fill array with some lambdas

        for (let index = 0; index < 5; index++) {
           storage [index] = () => { console.log ("Index ", index) }
           // Every lambda captures loop index variable
        }
        for (let index = 0; index < 5; index++) {
           storage[index]() // Captured indices printed
        }



.. index::
   runtime evaluation
   lambda expression
   function type instance
   captured variable
   allocation
   OutOfMemoryError
   function scope
   loop scope

|

.. _Constant Expressions:

常量表达式
*******************************************************************************

.. meta:
    frontend_status: Done

常量表达式是在编译期即可确定其值的表达式子集，可以是值类型（见 :ref:`Value Types`）或 ``string`` 类型。
若其求值 *异常完成*，则发生 :index:`compile-time error`。

其语法如下：

.. code-block:: abnf

       constantExpression:
           expression
           ;

常量表达式可以是值类型表达式，也可以是 ``string`` 类型表达式，但只能由以下成分构成：

- 预定义值类型字面量与 ``string`` 字面量，见 :ref:`Literals`；
- 引用外围 block、函数、方法或 lambda 体内常量的简单名称，且被引用常量声明的初始化器本身也是常量表达式；
- 一元运算符 ``+``、``-``、``~``、``!``，但不包括 ``++`` 和 ``--``，见
  :ref:`Unary Plus`、:ref:`Unary Minus`、:ref:`Prefix Increment`、:ref:`Prefix Decrement`；
- 到数值类型的 cast 转换，见 :ref:`Cast Expression`；
- 乘法类运算符 ``*``、``/``、``%``，见 :ref:`Multiplicative Expressions`；
- 加法类运算符 ``+``、``-``，见 :ref:`Additive Expressions`；
- 位移运算符 ``<<``、``>>``、``>>>``，见 :ref:`Shift Expressions`；
- 关系运算符 ``<``、``<=``、``>``、``>=``，见 :ref:`Relational Expressions`；
- 相等运算符 ``==``、``!=``，见 :ref:`Equality Expressions`；
- 位与逻辑运算符 ``&``、``^``、``|``，见 :ref:`Bitwise and Logical Expressions`；
- 条件与 ``&&`` 和条件或 ``||``，见 :ref:`Conditional-And Expression` 和 :ref:`Conditional-Or Expression`；
- 三元条件运算符 ``condition ? whenTrue : whenFalse``，见 :ref:`Ternary Conditional Expressions`；
- 其中包含常量表达式的括号表达式，见 :ref:`Parenthesized Expression`。

常量表达式的约束因此有两层：

- 语法上，表达式只能由上面列出的运算和操作数组成；
- 语义上，编译器必须能够在编译期把它完整求值出来。

若某个表达式虽然只使用了允许的运算符，但某个子表达式在编译期无法确定其值，
则整个表达式仍然不是常量表达式。反过来，只要所有子表达式都满足上述条件，
常量表达式就可以递归地组合出更复杂的编译期值。

.. index::
   constant expression
   expression
   evaluation
   compile time
   syntax
   constant expression
   constant declaration
   value type
   normal completion
   string type
   literal
   simple name
   surrounding function
   surrounding method
   surrounding lambda
   unary operator
   unary plus
   unary minus
   increment
   decrement
   casting conversion
   cast expression
   multiplicative expression
   multiplicative operator
   additive expression
   additive operator
   shift expression
   shift operator
   relational expression
   relational operator
   equality expression
   equality operator
   bitwise operator
   logical operator
   conditional operator
   conditional-and operator
   conditional-or operator
   ternary conditional operator
   parenthesized expression
   compile-time error
   surrounding block
   lambda body

.. _Specifics of Constant Expressions Evaluation:

常量表达式求值的特性
===============================================================================

.. meta:
    frontend_status: Done

若常量表达式包含作用于数值操作数的一元或二元运算符，则按以下规则求值：

- 若任一操作数类型为 ``double``，或运算符是幂运算符，则其余操作数先转换为 ``double``，
  结果类型也为 ``double``；
- 否则若任一操作数类型为 ``float``，则其余操作数先转换为 ``float``，
  结果类型为 ``float``；
- 否则，所有操作数都转换为某种能够表示任意精度整数的“大整数内部类型”，
  其能力至少不低于 ``bigint``，并且能覆盖超过 ``long`` 最大值的整数。

若常量表达式本身只是一个整数字面量，或一个整数类型常量，也会先转换为同样的内部大整数类型。

:ref:`Type Inference for Constant Expressions` 总是用于推断常量表达式的最终类型。
换句话说，上述“大整数类型”仅仅是编译器内部求值用的中间类型，运行时不会出现这种值。

对混合常量表达式，每个数值子表达式都按上面的规则独立求值；
也就是说，逻辑或条件运算本身不会改变这些数值子表达式内部的常量求值策略：

.. code-block:: typescript
   :linenos:

       const c = 3

       c > 1 && c*2 < 8 // numeric subexpressions: (c > 1) and (c*2 < 8)

.. index::
   constant expression evaluation
   numeric operand
   double
   float
   exponentiation operator
   internal compiler type
   big integer type
   type inference
   numeric subexpression

.. raw:: pdf

   PageBreak
