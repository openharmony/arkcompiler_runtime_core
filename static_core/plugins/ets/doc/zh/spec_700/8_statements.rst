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


.. _Statements:

语句
##########

.. meta:
    frontend_status: Done

*语句* 用于控制执行流程。

有些语句很简单，例如单个 expressionStatement（见 :ref:`Expression Statements`）；
而有些语句则包含多行程序代码，例如 :ref:`Block` 或 :ref:`if Statements`。

.. code-block:: typescript

    ...
    i++    // Statement consists of single expression
    ...
    if (i > 100) // 'if' statement starts
    {          // 'block' statement starts, it is a part of 'if' statement
        i %= 100        // Statement belonging to 'block'
        console.log(i)  // Another statement belonging to 'block'
    }   //  'block' statement ends, 'if' statement end

.. note::
   语句与表达式的区别在于：:ref:`Expressions` 会求得某个类型的值，而语句不会。

   从文法规则角度看，任何以分号 ``;`` 结束的表达式都会形成一个 *expressionStatement*。
   当表达式被用作语句时，缺失的分号会自动补齐。

   一个语句可以由一个或多个表达式组成，也可以完全不包含表达式。
   例如，语句 ``i = 1`` 由一个赋值表达式组成，该表达式推断出的类型是 ``int``，
   求值得到的值是 ``1``。

*语句* 的语法如下：

.. code-block:: abnf

    statement:
        expressionStatement
        | block
        | constantOrVariableDeclaration
        | ifStatement
        | loopStatement
        | breakStatement
        | continueStatement
        | returnStatement
        | switchStatement
        | throwStatement
        | tryStatement
        ;


|LANG| 中的语句列表如下：

.. list-table::
   :width: 100%
   :widths: 30 70
   :header-rows: 1

   * - Statement
     - Reference
   * - Expression statements
     - :ref:`Expression Statements`
   * - '{}' (block)
     - :ref:`Block`
   * - `let`, `const` (variable or constant declarations)
     - :ref:`Constant Or Variable Declarations`
   * - if-then-else
     - :ref:`if Statements`
   * - `while`, `do`, `for`, `for-of`
     - :ref:`Loop Statements`
   * - break
     - :ref:`break Statements`
   * - continue
     - :ref:`continue Statements`
   * - return
     - :ref:`return Statements`
   * - switch
     - :ref:`switch Statements`
   * - throw
     - :ref:`throw Statements`
   * - try-catch-finally
     - :ref:`try Statements`

|

.. _Normal and Abrupt Statement Execution:

正常与突然的语句执行
*************************************

.. meta:
    frontend_status: Done

每种语句在正常执行模式下所执行的动作都取决于该语句的具体种类。
各类语句的正常求值模式将在后续各节中说明。

如果某条语句在未抛出错误的情况下完成了预期动作，则其执行被认为是 *正常完成*；
反之，如果语句导致错误被抛出，则其执行被认为是 *突然完成*。

.. index::
   statement
   execution
   control
   evaluation
   error
   statement execution
   normal completion
   abrupt completion
   mode of evaluation

|

.. _Expression Statements:

表达式语句
*********************

.. meta:
    frontend_status: Done

任何表达式都可以被用作语句。

*表达式语句* 的语法如下：

.. code-block:: abnf

    expressionStatement:
        expression
        ;

语句的执行会导致该表达式被执行，而该执行结果会被丢弃。

.. index::
   statement
   expression
   expression statement
   syntax
   execution

|

.. _Block:

块
*****

.. meta:
    frontend_status: Done

由一对配平的大括号括起来的一系列语句（见 :ref:`Statements`）构成一个 *块*。

*块语句* 的语法如下：

.. code-block:: abnf

    block:
        '{' statement* '}'
        ;

块的执行意味着：除类型声明之外，块中的所有语句都会按照其在块内出现的文本顺序依次执行，
直到发生错误抛出（见 :ref:`Errors`），或者遇到返回（见 :ref:`Return Statements`）。

如果某个块是 ``functionDeclaration`` 的函数体（见 :ref:`Function Declarations`），
或者是隐式或显式声明返回类型为 ``void`` 的 ``classMethodDeclaration`` 的函数体（见 :ref:`Method Declarations`），
且其返回类型为 :ref:`Type undefined or void`，
那么该块可以完全不包含 ``return`` 语句。这样的块等价于以 ``return`` 语句结束的块，
并按同样方式执行。

.. index::
   statement
   balanced braces
   block
   syntax
   error
   execution
   block statement
   type declaration
   return
   return type
   declaration body
   return statement

|

.. _Constant Or Variable Declarations:

常量或变量声明
*********************************

.. meta:
    frontend_status: Done

*常量或变量声明* 在其外围上下文中定义新的可变变量或不可变变量。

``let`` 和 ``const`` 声明都包含需要执行的初始化部分，因此它们实际上表现为语句。

.. index::
   variable declaration
   constant declaration
   let declaration
   const declaration

*常量或变量声明* 的语法如下：

.. code-block:: abnf

    constantOrVariableDeclaration:
        annotationUsage?
        ( variableDeclaration
        | constantDeclaration
        )
        ;

声明名的可见性由外围模块、命名空间、函数或方法，以及块作用域规则共同决定
（见 :ref:`Scopes`）。为避免歧义，本规范专门在以下小节中对这些实体作进一步讨论：

- :ref:`if Statements`；
- :ref:`For Statements`；
- :ref:`For-Of Statements`。

任意声明都可以遮蔽同一外围作用域中的另一个同名声明（若存在）。

.. code-block:: typescript
   :linenos:

    let item: number = 123
    {
       const item: string = "123"
       // This 'item' is of type 'string'
    }
    // This 'item' is of type 'number'

    function foo (item: boolean) {
       // this 'item' is of type 'number'
       let item: number[] = [] // Compile-time error as parameter 'item' and
                               // local variable 'item' lead to duplication as
                               // they are in the same scope
    }


注解的用法见 :ref:`Using Annotations`。

.. index::
   enclosing context
   context
   mutable variable
   immutable variable
   initialization
   syntax
   execution
   function
   method
   surrounding function
   surrounding method
   block scope
   if statement
   for statement
   for-of statement
   annotation


|

.. _if Statements:

``if`` 语句
******************

.. meta:
    frontend_status: Done

``if`` 语句允许在特定条件下执行备选语句（若提供）。

*if 语句* 的语法如下：

.. code-block:: abnf

    ifStatement:
        'if' '(' expression ')' thenStatement
        ('else' elseStatement)?
        ;

    thenStatement:
        statement
        ;

    elseStatement:
        statement
        ;

表达式的类型必须是 ``boolean``，或属于 :ref:`Extended Conditional Expressions` 所允许的类型。否则会发生
:index:`compile-time error`。

.. index::
   if statement
   statement
   syntax
   expression
   type
   boolean type
   conditional expression

如果某个表达式成功求值为 ``true``，则执行 ``thenStatement``。
否则，如果提供了 ``elseStatement``，则执行该语句。

在 ``if`` 语句中，任意 ``else`` 都对应最近的前置 ``if``：

.. code-block:: typescript
   :linenos:

    if (Cond1)
    if (Cond2) statement1
    else statement2 // Executes only if: Cond1 && !Cond2

可以使用 :ref:`Block` 将 ``else`` 部分与最初的 ``if`` 组合起来，如下所示：

.. code-block:: typescript
   :linenos:

    if (Cond1) {
      if (Cond2) statement1
    }
    else statement2 // Executes if: !Cond1

如果 ``thenStatement`` 或 ``elseStatement`` 是任意一种语句但不是块（见 :ref:`Block`），
那么不会为该语句创建块作用域（见 :ref:`Scopes`）。

.. code-block:: typescript
   :linenos:

    function foo(Cond1: boolean) {
      if (Cond1) let x: number = 1
      x = 2 // OK

      if (Cond1) {
        let x: number = 10;   // OK, then-block scope
        let y: number = x;
      }
      else {
        let x: number = 20   // OK, no conflict, else-block scope
        y = x;           // Compile-time error, no 'y' in scope
      }

      console.log(x)  // OK, prints 2
      console.log(y)  // Compile-time error, 'y' unknown
    }

.. index::
   if statement
   statement
   expression
   evaluation
   block
   block scope
   scope
   else-block
   then-block

|

.. _Loop Statements:

循环语句
***************

.. meta:
    frontend_status: Done

|LANG| 有四种循环。每一种循环都可以选择性地使用某个 *identifier* 作为标签。
这个 *identifier* 只能被循环体中包含的 :ref:`Break Statements` 和
:ref:`Continue Statements` 使用。

.. index::
   loop statement
   loop
   loop label
   break statement
   continue statement
   identifier

*循环语句* 的语法如下：

.. code-block:: abnf

    loopStatement:
        (identifier ':')?
        whileStatement
        | doStatement
        | forStatement
        | forOfStatement
        ;

如果标签 *identifier* 未在 ``loopStatement`` 中使用，或者在循环体内的 :ref:`Lambda Expressions` 中使用，
则会发生 :index:`compile-time error`。

.. code-block:: typescript
   :linenos:

    label: for (i = 1; i < 10; i++) {
        const f1 = () => {
            while (true) {
                continue label // Compile-time error
            }
        }
        const f2 = () => {
            do
                break label // Compile-time error
            while (true)
        }
    }


.. index::
   loop statement
   loop
   syntax
   lambda
   lambda expression
   loop body
   label
   identifier

|

.. _While Statements and Do Statements:

``while`` 语句与 ``do`` 语句
******************************************

.. meta:
    frontend_status: Done

``while`` 语句和 ``do`` 语句都会求值一个表达式，并在该表达式值为 ``true`` 时反复执行对应语句。
两者的关键区别在于：``whileStatement`` 从求值并检查表达式值开始，而 ``doStatement`` 从先执行语句开始。

*while 与 do 语句* 的语法如下：

.. code-block:: abnf

    whileStatement:
        'while' '(' expression ')' statement
        ;

    doStatement
        : 'do' statement 'while' '(' expression ')'
        ;

表达式类型必须是 ``boolean``，或属于 :ref:`Extended Conditional Expressions` 所允许的类型。
否则会发生 :index:`compile-time error`。

.. index::
   while statement
   do statement
   evaluation
   expression
   expression value
   execution
   statement
   syntax
   while statement
   do statement
   boolean type
   type
   extended conditional expression

|

.. _For Statements:

``for`` 语句
******************

.. meta:
    frontend_status: Done

*for 语句* 的语法如下：

.. code-block:: abnf

    forStatement:
        'for' '(' forInit? ';' forContinue? ';' forUpdate? ')' statement
        ;

    forInit:
        expressionSequence
        | variableDeclarations
        ;

    forContinue:
        expression
        ;

    forUpdate:
        expressionSequence
        ;

*forContinue* 表达式的类型必须是 ``boolean``，或者属于 :ref:`Extended Conditional Expressions` 所允许的类型。
否则会发生 :index:`compile-time error`。

.. code-block:: typescript
   :linenos:

    // The existing variable is used as a loop index variable
    let i: number
    for (i = 1; i < 10; i++) {
      console.log(i)
    }

    // A new variable is declared as a loop index variable with its type
    // explicitly specified
    for (let i: int = 1; i < 10; i++) {
      console.log(i)
    }

    // A new variable is declared as a loop index variable with its type
    // inferred from its initialization part of the declaration
    for (let i = 1; i < 10; i++) {
      console.log(i)
    }

.. index::
   for statement
   syntax
   variable
   declaration
   loop index variable
   type
   inferred type
   initialization

在 *forInit* 部分声明的变量具有循环作用域。它可以在 ``forContinue`` 表达式、``forUpdate``
表达式、单条语句形式的循环体，或者块形式的循环体中使用：

.. code-block:: typescript
   :linenos:

    // forInit declaration and no body block
    let k: int = 0
    for (let i: int = 1; i < 10; i++)
      k += i
    console.log(k)
    // i =  k  // Compile-time error if uncommented
    let i: int = k  // OK

|

.. _For-Of Statements:

``for-of`` 语句
*********************

.. meta:
    frontend_status: Partly
    todo: type of element for strings

``for-of`` 循环会迭代某个 :ref:`Iterable Types` 实例的元素，并在这些元素可用的情况下执行循环体。

*for-of 语句* 的语法如下：

.. code-block:: abnf

    forOfStatement:
        'for' '(' forVariable 'of' expression ')' statement
        ;

    forVariable:
        identifier | ('let' | 'const') identifier (':' type)?
        ;

如果表达式的类型不可迭代，则会发生 :index:`compile-time error`。

``for-of`` 循环的执行从对 ``expression`` 求值开始。
随后，如果求值成功，则在每次循环迭代中，*forVariable* 都会被设置为迭代器前进后得到的下一个元素，
并执行循环体（即 ``statement``）。

.. index::
   for-of statement
   loop
   instance
   iterable class
   iterable interface
   iterable type
   expression
   type
   array
   string
   for-of loop
   evaluation
   loop iteration
   iteration
   statement

如果 *forVariable* 带有修饰符 ``let`` 或 ``const``，那么 *forVariable* 会声明一个仅在循环体内部可访问的新变量。
否则，会使用在其他位置已声明的变量。

修饰符 ``const`` 禁止向 *forVariable* 赋值，而修饰符 ``let`` 允许修改。

在循环内部声明的 *forVariable*，其类型会被推断为被迭代元素的类型，也就是：

- ``T``，如果被迭代的是 ``Array<T>`` 或 ``FixedArray<T>`` 实例；
- ``string``，如果被迭代的是某个 ``string`` 值；
- 迭代器的类型实参，如果被迭代的是某个 *可迭代* 类型实例。

如果 *forVariable* 在循环外部声明，那么被迭代元素的类型必须 :ref:`可赋值 <Assignability>` 给该变量的类型。
否则会发生 :index:`compile-time error`。

.. index::
   compile-time error
   modifier
   modifier let
   let
   loop
   loop scope
   loop body
   instance
   iteration
   iterable type
   accessibility
   declaration
   inferred type
   modifier const
   const
   variable
   assignment
   modification
   for-of type annotation
   annotation
   iterator

.. code-block:: typescript
   :linenos:

    // The existing variable 's'
    let s : string
    for (s of "a string object") {
      console.log(s)
    }

    // New variable 's', its type is inferred from the expression after 'of'
    for (let s of "a string object") {
      console.log(s)
    }

    // New variable 'element', its type is inferred from the expression after 'of'
    // as 'const' it cannot be assigned a new value in the loop body
    for (const element of [1, 2, 3]) {
      console.log(element)
      element = 66 // Compile-time error as 'element' is 'const'
    }

显式为 *forVariable* 添加类型注解目前仍是 :ref:`For-of Explicit Type Annotation` 实验性特性。

.. index::
   annotation
   inferred type
   expression
   assignment

|

.. _Break Statements:

``break`` 语句
*********************

.. meta:
    frontend_status: Done
    todo: break with label causes compile time assertion

``break`` 语句会把控制流转移出外围的 ``loopStatement`` 或 ``switchStatement``。
如果 ``break`` 语句出现在 ``loopStatement`` 或 ``switchStatement`` 之外，则会发生
:index:`compile-time error`。

.. index::
   break statement
   control transfer
   compile-time error
   control transfer
   switch statement
   loop statement
   syntax

*break 语句* 的语法如下：

.. code-block:: abnf

    breakStatement:
        'break' identifier?
        ;

带标签 *identifier* 的 ``break`` 语句会把控制流转移出带有相同标签 *identifier* 的外围语句。
如果不存在带有该标签的外围循环语句（位于外围函数或方法体之内），则会发生
:index:`compile-time error`。

不带标签的语句会把控制流转移出最内层外围的 ``switch``、``while``、``do``、``for`` 或 ``for-of`` 语句。
如果 ``breakStatement`` 放在 ``loopStatement`` 或 ``switchStatement`` 之外，
则会发生 :index:`compile-time error`。

下面给出带标签和不带标签的 ``break`` 语句示例：

.. code-block:: typescript
   :linenos:

    // A single iteration
    while (true) {
      console.log("iteration")  // Gets printed exactly once
      break;
    }

    let a: number = 0
    outer:
      do {
        for (a = 0; a < 10; a++) {
            if (a == 1) break outer
            console.log("inner")    // Gets printed once only
        }
        console.log(a) // Never reached
      } while (true)   // Condition never used

.. index::
   break statement
   label
   identifier
   control transfer
   statement
   enclosing statement
   surrounding function
   surrounding method
   function
   method
   label
   switch statement
   while statement
   do statement
   for statement
   for-of statement
   loop statement

|

.. _Continue Statements:

``continue`` 语句
***********************

.. meta:
    frontend_status: Done
    todo: continue with label causes compile time assertion

``continue`` 语句会终止当前循环迭代的执行，并把控制流转移到下一次迭代，
同时保持循环退出条件的相应检查仍然生效。

*continue 语句* 的语法如下：

.. code-block:: abnf

    continueStatement:
        'continue' identifier?
        ;

.. index::
   continue statement
   statement
   execution
   loop
   loop iteration
   control transfer
   iteration
   exit condition
   label
   syntax

不带标签的 ``continue`` 语句会把控制流转移到外围 ``loop`` 语句的下一次迭代。
如果在外围函数或方法体中不存在外围 ``loop`` 语句，则会发生 :index:`compile-time error`。

带标签 *identifier* 的 ``continue`` 语句会把控制流转移到带有相同标签 *identifier*
的外围循环语句的下一次迭代。
如果在外围函数或方法体中不存在带有相同标签 *identifier* 的外围循环语句，
则会发生 :index:`compile-time error`。

下面给出带标签和不带标签的 ``continue`` 语句示例：

.. index::
   continue statement
   control transfer
   statement
   iteration
   surrounding function
   surrounding method
   enclosing statement
   execution
   label
   label identifier
   exit condition
   loop statement
   surrounding function
   control transfer
   identifier
   function
   method

.. code-block:: typescript
   :linenos:

    // continue     // Compile-time error if uncommented

    // Continue without label
    // Prints 0, 1, 2, 4 (3 skipped)
    for (let a: int = 0; a < 5; a++){
      if (a == 3) continue
      console.log("a = " + a)
    }

    let a: number
    outer:
      do {
        for (a = 0; a < 10; a++) {
            if (a > 1) continue outer
            console.log("inner")    // Gets printed twice only
        }
        console.log("Outer") // Never reached
      } while (false)


|

.. _return Statements:

``return`` 语句
*********************

.. meta:
    frontend_status: Done
    todo: return voidExpression

``return`` 语句会终止当前函数、方法、构造器或 lambda 的执行，并将控制返回给调用方。
对于函数、方法和 lambda 调用，执行 ``return`` 语句意味着该调用返回一个值，
该值被定义为 *返回表达式* 求值得到的结果。


*return 语句* 的语法如下：

.. code-block:: abnf

    returnStatement:
        'return' expression?
        ;

如果未提供 *expression*，则 *return 语句* 在语义上等价于 ``return undefined``。

无 *expression* 的普通形式 ``return`` 可以出现在以下任一位置：

- 构造器体内部；
- 返回类型为 :ref:`Type undefined or void` 的函数、方法或 lambda 体内部，
  或者返回类型为包含 ``void`` 或 ``undefined`` 的联合类型的函数、方法或 lambda 体内部
  （见 :ref:`Union Types`）；
- 返回类型为 ``Promise<void>`` 的异步函数、方法或 lambda 体内部
  （见 :ref:`Asynchronous execution`）。

否则会发生 :index:`compile-time error`。

如果返回表达式的类型不 :ref:`可赋值 <Assignability>` 给外围函数、方法或 lambda 的返回类型，也会发生
:index:`compile-time error`。

其语义见下例：

.. code-block:: typescript
   :linenos:

    return // Compile-time error, top-level statements cannot contain a return statement
    namespace NS {
       return // Compile-time error, top-level statements cannot contain a return statement
    }
    function f1 () {} // OK, no return statement equals the form 'return undefined'
    function f2 () { return } // OK, 'return' equals the form 'return undefined'
    function f3 (): void { return undefined } // Full syntax form

    function f4(cond: boolean): Base {
         if (cond) {
              return new Derived // OK, as Derived is assignable to Base
         } else {
              return new Object // Compile-time error, as Object is not assignable to Base
         }
    }

    class A {
        constructor () {
             return  // OK
        }
        constructor (p: number) {
             return undefined  // Compile-time error, such syntax form is forbidden
        }
    }


.. index::
   return statement
   expression
   function body
   lambda body
   method body
   return type
   statement
   top-level statement
   function
   method
   void type
   return type
   class
   constructor
   constructor body
   return type
   statement
   syntax
   return expression
   function
   method
   lambda
   execution
   termination
   surrounding function
   function
   surrounding method
   method
   constructor
   evaluation

|

.. _Switch Statements:

``switch`` 语句
*********************

.. meta:
    frontend_status: Done
    todo: non literal constant expression () in case ==> causes an assertion error
    todo: when there is only a default clause in switchBlock then the default's statements/block are not executed

``switch`` 语句会根据 ``switch`` 表达式的值成功求值后的结果，将控制流转移到某条语句或某个块。

.. index::
   switch statement
   control transfer
   statement
   block
   evaluation
   switch expression

*switch 语句* 的语法如下：

.. code-block:: abnf

    switchStatement:
        (identifier ':')? 'switch' '(' expression ')' switchBlock
        ;

    switchBlock
        : '{' caseClause* defaultClause? caseClause* '}'
        ;

    caseClause
        : 'case' expression ':' statement*
        ;

    defaultClause
        : 'default' ':' statement*
        ;

``switch`` 表达式可以是任意类型。

如果提供了可选的 identifier，那么 ``break`` 语句就可以将控制流转移出嵌套的 ``switch``
或 ``loop`` 语句。

.. index::
   syntax
   switch statement
   switch expression
   expression type
   identifier
   control transfer
   nested statement
   switch statement
   loop statement
   break statement

如果至少有一个 case 表达式的类型不 :ref:`可赋值 <Assignability>` 给 ``switch`` 语句表达式的类型，则会发生
:index:`compile-time error`。

.. index::
   expression
   expression type
   switch statement
   assignability

.. code-block:: typescript
   :linenos:

    let arg: string = prompt("Enter a value?");
    switch (arg) {
      case '0':
      case '1':
        console.log('One or zero')
        break
      case '2':
        console.log('Two')
        break
      default:
        console.log('An unknown value')
    }

    class A {}
    let a: A| null = assignIt()
    switch (a) {
      case null:
      case null: // One may have several case clauses with the same expression in
        console.log ("a is null")
        break
      case new A:
        console.log ("Never matches as new A is a new unique object")
        break
      default:
        console.log ("a is A")
    }
    function assignIt () {
        return new A
    }


``switch`` 语句的执行从对 ``switch`` 表达式求值开始。

随后，``switch`` 表达式的值会被反复拿来与 case 表达式的值比较。比较从上至下进行，
直到出现第一个 *匹配*。当某个 case 表达式的值在 ``==`` 运算符意义下等于 ``switch`` 表达式的值时，
就发生一次 *匹配*。此时，执行会转移到发生匹配的 *caseClause* 所包含的语句集合。
如果该语句集合执行了 ``break`` 语句（见 :ref:`Break statements`），那么整个 ``switch`` 语句终止。
如果没有执行 ``break`` 语句，那么执行会继续穿过后续所有 *caseClause* 与 *defaultClause*
中的语句，直到遇到第一个 ``break`` 语句，或直到 ``switch`` 语句结束。

如果在存在 *defaultClause* 的情况下没有任何 *匹配* 发生，那么执行会转移到 *defaultClause*
中的语句。

.. index::
   expression
   break
   object
   function
   execution
   switch statement
   switch expression
   expression value
   execution transfer
   evaluation
   constant
   operator
   string
   match
   break statement

|

.. _throw Statements:

``throw`` 语句
********************

.. meta:
    frontend_status: Done

``throw`` 语句会导致一个 *错误* 对象被创建并抛出（见 :ref:`Error Handling`）。
它会立即转移控制流，并且可以越过多层语句、构造器、函数和方法调用，直到找到能够捕获该抛出值的
``try`` 语句（见 :ref:`Try Statements`）。如果没有找到 ``try`` 语句，则会抛出
``UncaughtExceptionError``。

*throw 语句* 的语法如下：

.. code-block:: abnf

    throwStatement:
        'throw' expression
        ;

表达式的类型必须 :ref:`可赋值 <Assignability>` 给 ``Error`` 类型。否则会发生 :index:`compile-time error`。

这意味着 ``null`` 和 ``undefined`` 都不能被抛出。

错误可以在代码中的任意位置被抛出。

.. index::
   throw statement
   error object
   thrown value
   thrown object
   control transfer
   statement
   method
   method call
   function
   constructor
   try block
   try statement
   value
   assignment
   assignability
   expression
   assignability
   error
   type

|

.. _Try Statements:

``try`` 语句
******************

.. meta:
    frontend_status: Done

``try`` 语句运行一段代码块，并提供可选的 ``catch`` 子句，以处理该代码块执行期间可能发生的错误
（见 :ref:`Error Handling`）。

.. index::
   try statement
   block
   catch clause

*try 语句* 的语法如下：

.. code-block:: abnf

    tryStatement:
          'try' block catchClause? finallyClause?
          ;

    catchClause:
          'catch' '(' identifier ')' block
          ;

    finallyClause:
          'finally' block
          ;

``try`` 语句必须包含以下之一：

- ``finally`` 子句；
- ``catch`` 子句；或
- 同时包含 ``finally`` 子句与 ``catch`` 子句。

否则会发生 :index:`compile-time error`。

如果 ``try`` 块正常完成，那么若存在 ``catch`` 子句块，则不会执行它。

如果在 ``try`` 块中直接或间接抛出错误，那么若存在 ``catch`` 子句，则控制流会转移到该 ``catch`` 子句。

.. index::
   syntax
   catch clause
   typed catch clause
   try statement
   try block
   normal completion
   control transfer
   finally clause
   block

|

.. _Catch Clause:

``catch`` 子句
================

.. meta:
    frontend_status: Done

``catch`` 子句由两部分组成：

- 一个 catch identifier，它提供对与所抛出错误关联对象的访问；以及
- 一个用于处理该错误的代码块。

块内 catch identifier 的类型是 Error（见 :ref:`Error Handling`）。

.. index::
   catch clause
   catch identifier
   access
   error
   block
   catch identifier
   Object

.. code-block:: typescript
   :linenos:

    class ZeroDivisor extends Error {}

    function divide(a: number, b: number): number {
      if (b == 0)
        throw new ZeroDivisor()
      return a / b
    }

    function process(a: number, b: number): number {
      try {
        let res = divide(a, b)

        // Further processing ...
        return res
      }
      catch (e) {
        return e instanceof ZeroDivisor? -1 : 0
      }
    }

``catch`` 子句会在运行时处理所有错误。对于 ``ZeroDivisor``，它返回 ``-1``；
对于其他所有错误，则返回 ``0``。

.. index::
   catch clause
   runtime
   runtime error
   function
   divisor


|

.. _Finally Clause:

``finally`` 子句
==================

.. meta:
    frontend_status: Done

``finally`` 子句以一个块的形式定义一组动作，不论 ``try-catch`` 是正常完成还是突然完成，
这些动作都会执行。

*finally 子句* 的语法如下：

.. code-block:: abnf

    finallyClause:
        'finally' block
        ;

无论程序控制是通过到达 ``return``、到达 ``try-catch`` 末尾，还是抛出新 *错误* 的方式离开，
``finally`` 块都会执行。``finally`` 块特别适合用于确保资源管理正确完成。

在离开 ``try-catch`` 时，可以执行任何必要动作（例如刷新缓冲区和关闭文件描述符）：

.. index::
   finally clause
   block
   execution
   try-catch
   normal completion
   abrupt completion
   syntax
   finally block
   execution
   return

.. code-block:: typescript

    class SomeResource {
      // Some class members
      // ...
      close() {}
    }

    function ProcessFile(name: string) {
      let r = new SomeResource()
      try {
        // Some processing
      }
      finally {
        // Finally clause is executed after try-catch
            completes normally or abruptly
        r.close()
      }
    }

|

.. _Try Statement Execution:

``try`` 语句执行
===========================

.. meta:
    frontend_status: Done

#. 如果没有执行任何 ``catch`` 块，则 ``try`` 块及整个 ``try`` 语句都正常完成。
   如果在 ``try`` 块中抛出了错误，则 ``try`` 块的执行会突然完成。

#. 如果在 ``try`` 块内抛出了错误 *x*，则 ``try`` 块的执行会突然完成。
   如果存在 ``catch`` 子句，且 ``catch`` 子句体执行正常完成，则整个 ``try`` 语句正常完成。
   否则，``try`` 语句突然完成。

#. 如果不存在 ``catch`` 子句，则该错误会沿着外围作用域和调用方作用域继续传播，
   直到到达具有 ``catch`` 子句、能够处理该错误的作用域。之后的步骤由执行环境定义。

#. 如果存在 ``finally`` 子句，且其执行突然完成，则 ``try`` 语句也会突然完成。

.. index::
   try statement
   execution
   try block
   normal completion
   abrupt completion
   error
   catch clause
   runtime
   statement
   catch clause
   assignability
   propagation
   surrounding scope
   caller scope
   scope
   environment

.. raw:: pdf

   PageBreak
