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


.. _Introduction:

引言
################################################################################

本文档完整介绍名为 |LANG| 的新型通用、多范式编程语言。

|

.. _Common Description:

总体描述
********************************************************************************

|LANG| 语言融合并支持了许多知名编程语言中已经被证明具有帮助且功能强大的特性。

|LANG| 支持命令式、面向对象、函数式和泛型等编程范式，并以安全且一致的方式将它们结合起来。

同时，|LANG| 不支持那些会让软件开发者编写出危险、不安全或低效代码的特性。尤其是，该语言采用强静态类型原则。对象类型由其声明决定，不允许发生动态类型变化。语义正确性在编译期进行检查。

|LANG| 被设计为现代语言体系的一部分。为了提供高效且可安全执行的代码，该语言从 |TS| 及其前身 |JS| 中吸收灵活性和表达能力，从 Java 与 Kotlin 中吸收静态类型原则。总体设计使 |LANG| 的语法风格与这些语言保持相似，其中一些重要构造更是有意与它们几乎完全一致。

换言之，|LANG| 与 |TS|、|JS|、Java 以及 Kotlin 具有显著的*共同子集*。因此，对于 |TS| 和 Java 用户而言，|LANG| 的风格与构造并不陌生；即使尚未完全理解，他们通常也能直观感知这门新语言中大多数构造的含义。

.. index::
   construct
   syntax
   common subset

这种风格和语义上的相似性，使得原本使用 |TS|、Java 或 Kotlin 编写的应用能够较为平滑地迁移到 |LANG|。

与其前身一样，|LANG| 是一种相对高层的语言。这意味着该语言不提供对底层机器表示的直接访问。作为高层语言，|LANG| 支持自动存储管理，即所有动态创建的对象都会在不再可达后不久自动释放，无需显式释放。

|LANG| 不仅仅是一门语言，更是一个完整的软件开发生态系统，用于支持多种应用领域中的软件解决方案开发。

|LANG| 生态包含语言本身及其编译器、配套文档、指南、教程、标准库（见 :ref:`Standard Library`），以及一组附加工具。这些工具可以将其他语言（当前为 |TS| 和 Java）自动或半自动地迁移到 |LANG|。

整体来看，|LANG| 语言具有以下特征：

-  **面向对象**

   |LANG| 支持基于类和接口的*面向对象编程*（OOP）方法。该方法中的主要概念包括：

   - 具有单继承的类；
   - 作为抽象、由类实现的接口；以及
   - 具备重写和动态分派机制的方法（类实例方法或接口方法）。

   面向对象几乎存在于所有现代编程语言中。它使软件设计更强大、更灵活、更安全，也更清晰且更贴合实际需求。

.. index::
   object
   object orientation
   object-oriented
   OOP (object-oriented programming)
   inheritance
   overriding
   abstraction
   dynamically dispatched overriding

-  **模块化**

   |LANG| 支持*模块化编程*方法。该方法假定软件是以*模块*或*顶层命名空间*的组合形式进行设计和实现的。

   *模块* 聚合了多种编程资源（类型、类、函数等）。模块可以通过导出其全部或部分资源，或从其他模块导入资源，与其他模块交互。

.. index::
   modular programming
   module
   namespace

-  **泛型化**

   |LANG| 中的一些程序实体可以进行*类型参数化*。这意味着一个实体能够表示一个非常高层次（抽象）的概念。为其提供更具体的类型信息，就构成了该实体在特定用例中的实例化。

   一个经典示例是列表这一概念，它表示一种抽象数据结构的“思想”。通过提供附加信息（即列表元素的类型），就可以把这种抽象概念转化为具体的列表。

   许多编程语言支持的类似特性（*generics* 或 *templates*）可以让程序和程序结构更加通用、更可复用，并且构成泛型编程范式的基础。

.. index::
   abstract concept
   abstract notion
   abstract data structure
   genericity
   type parameterized entity
   compile-time feature
   program entity
   generic
   template

-  **多目标支持**

   |LANG| 为广泛的设备提供高效的应用开发方案。面向开发者友好的 |LANG| 生态是一种*跨平台开发*环境，为众多常见平台提供统一的编程体验。它既可以生成能够适应轻量级设备约束的优化应用，也可以充分发挥特定目标硬件的全部能力。

.. index::
   multi-targeting
   cross-platform development
   high-level language
   low-level representation
   target-specific hardware
   storage management
   dynamically created object
   deallocation
   migration
   automatic transition
   semi-automatic transition

|

.. _Lexical and Syntactic Notation:

词法与语法记号
********************************************************************************

本节介绍一种称为*上下文无关文法*（context-free grammar）的记号方式。本文规范使用这种记号来定义程序的词法结构和语法结构。

.. index::
   context-free grammar
   lexical structure
   syntactic structure

|LANG| 的词法记号定义了一组规则，即产生式，用来描述称为 *token* 的基本语言组成部分的结构。所有 token 都在 :ref:`Lexical Elements` 中定义。这些 token（标识符、关键字、数字/数值字面量、运算符号、分隔符）、特殊字符（空白符与行分隔符）以及注释，共同构成语言的*字母表*。

.. index::
   lexical notation
   production
   token
   lexical element
   identifier
   keyword
   number
   numeric literal
   operator sign
   line separator
   delimiter
   special character
   white space
   comment

词法文法中定义的 token 是语法记号中的终结符。语法记号定义了一组从目标符号 ``moduleDeclaration`` 开始的产生式。该目标符号（见 :ref:`Module Declarations`）是一个只包含单个特定非终结符的句子，用于描述 token 序列如何组成语法正确的程序。

.. index::
   production
   nonterminal
   lexical grammar
   syntactic notation
   goal symbol
   module
   nonterminal

词法文法和语法文法都被定义为若干产生式的集合。每个产生式由以下部分组成：

- 其左侧的抽象符号（*nonterminal*）；
- 其*右侧*由一个或多个 *nonterminal* 与 *terminal* 符号组成的序列；
- 作为左右两侧分隔符的字符 ``':'``；以及
- 作为结束标记的字符 ``';'``。

.. index::
   lexical grammar
   syntactic grammar
   abstract symbol
   nonterminal symbol
   terminal symbol
   character
   separator
   end marker

文法从目标符号开始，通过不断将左侧序列中的任一非终结符替换为对应产生式右侧的内容，从而规定语言，也就是所有可能得到的终结符序列集合。

.. index::
   goal symbol
   nonterminal
   terminal symbol
   sequence
   production

在文法产生式右侧，除终结符和非终结符外，还可以使用以下附加符号（有时称为 *metasymbols*）：

-  竖线 ``'|'``，表示可选项。

-  问号 ``'?'``，表示前一个终结符或非终结符可以出现零次或一次。

-  星号 ``'*'``，表示某个 *terminal* 或 *nonterminal* 可以出现零次或多次。

-  圆括号 ``'('`` 和 ``')'``，用于括起由元符号 ``'?'`` 或 ``'*'`` 标记的任意终结符和/或非终结符序列。

.. index::
   terminal
   terminal symbol
   nonterminal
   goal symbol
   metasymbol
   grammar production

这些元符号用于规定终结符和非终结符序列的组织规则，但它们本身不属于构成最终程序文本的终结符序列。

下面的示例给出了一个用于描述表达式列表的产生式：

.. code-block:: abnf

    expressionList:
      expression (',' expression)* ','?
      ;

该产生式引入了由非终结符 *expressionList* 定义的结构。表达式列表必须由若干个 *expression* 组成，并以终结符 ``','`` 分隔。该序列至少包含一个 *expression*。列表末尾可以选择性地再跟一个终结符 ``','``。

本规范的全部文法规则汇总于 :ref:`Grammar Summary` 章节。

.. index::
   structuring rule
   sequence
   terminal symbol
   expression
   grammar rule

|

术语与定义
********************************************************************************

本节按字母顺序列出规范中出现的重要术语，以及它们在 |LANG| 中的专门定义。这些定义并非通用定义，可能与这些术语在其他语言、应用领域或行业中的含义有显著差异。

.. glossary::
   :sorted:

   compile-time error
     -- 编译器在程序代码中识别出会阻止代码生成的错误时显示的文本消息。

   compile-time warning
     -- 编译器在程序代码中发现某些逻辑不一致、需要重新审视设计和实际编码时显示的文本消息。

   expression
     -- 用于计算值的公式。表达式的语法形式是运算符与括号的组合，其中括号用于改变计算顺序。默认计算顺序由运算符优先级决定。

   operator (in programming languages)
     -- 该术语可具有以下含义：

     (1) 表示对某个值执行何种操作（加、减、比较等）的 token。

     (2) 表示表达式中某个基础计算的语法构造。运算符通常由运算符号和一个或多个操作数组成。

     只有一个操作数的一元运算符，其运算符号可以放在操作数之前或之后（分别称为 *prefix* 和 *postfix* 一元运算符）。位于两个操作数之间的运算符号称为 *infix* 二元运算符。拥有三个操作数的条件运算符称为 *ternary*。

     某些运算符具有特殊记法。例如，下标运算符在形式上是二元运算符，但通常写成 ``a[i]`` 这样的形式。

     某些语言将运算符视为 *syntactic sugar*，即某种更通用构造或 *function call* 的约定写法。因此，像 ``a+b`` 这样的运算符在概念上可视为调用 ``+(a,b)``，其中运算符号相当于函数名，操作数相当于函数调用参数。

   operation sign
     -- 表示运算符的语言 token，通常对应常见数学运算符，例如表示加法的 ``'+'``、表示除法的 ``'/'`` 等。不过，一些语言允许使用标识符表示运算符，和/或任意组合某一特定语言字母表中并非 token 的字符来表示运算符号。

   operand
     -- 运算的参数。在语法上，操作数可以是简单标识符或限定标识符，用于引用变量或结构化对象的成员。反过来，操作数本身也可以是优先级高于某个给定运算符的其他运算符。

   operation
     -- 表示运算符求值动作或过程的非正式概念。

   metasymbol
     -- 可与终结符和非终结符一起出现在文法产生式右侧的附加符号 ``'|'``、``'?'``、``'*'``、``'('`` 和 ``')'``。

   goal symbol
     -- 由单个特定非终结符（*moduleDeclaration*）构成的句子。*goal symbol* 描述 token 序列如何组成语法正确的程序。

   token
     -- 编程语言中的基本组成部分，即标识符、关键字、运算符和标点符号，或字面量。Token 是构成语言词汇表的词法输入元素，并且可以作为语言语法文法中的终结符。

   tokenization
     -- 在机器读取代码库的过程中，寻找能够构成合法 token 的最长字符序列（即*识别出*一个 token）的过程。

   punctuator
     -- 用于分隔、结束或以其他方式组织程序元素和组成部分的 token，例如逗号、分号、圆括号、方括号等。

   literal
     -- 某种类型的值的表示形式。

   comment
     -- 对语法文法无意义、但被加入到文本流中用于说明和补充源代码的文本片段。

   generic type
     -- 具有类型参数的命名类型（类或接口）。

   generic
     -- 参见 *generic type*。

   non-generic type
     -- 不具有类型参数的命名类型（类或接口）。

   non-generic
     -- 参见 *non-generic type*。

   type reference
     -- 通过指定类型名称以及在适用时指定类型实参，来引用命名类型的引用形式；这些实参将用于替换命名类型的类型参数。

   nullable type
     -- 声明为可取值 ``null`` 的变量，或 ``type T | null`` 这类类型；它可以持有类型 ``T`` 及其派生类型的值。

   nullish value
     -- 值为 ``null`` 或 ``undefined`` 的引用。

   simple name
     -- 仅由一个标识符组成的名称。

   qualified name
     -- 由若干标识符通过 token ``'.'`` 分隔组成的名称。

   name scope
     -- 程序代码中的某个区域，在该区域内，某个实体（由该名称声明）可以通过其简单名称在不加限定的情况下被访问或引用。

   function declaration
     -- 在引入具名函数时，用于指定名称、签名和函数体的声明。

   terminal symbol
     -- 语法上不可变的 token（即由词法文法中不可变形式直接定义的语法记号；该词法文法定义了一组从 :term:`goal symbol` 开始的产生式）。

   terminal
     -- 参见 *terminal symbol*。

   nonterminal symbol
     -- 通过连续应用产生式规则而得到的语法上可变 token。

   context-free grammar
      -- 每条产生式规则左侧仅由单个非终结符组成的文法。

   nonterminal
     -- 参见 *nonterminal symbol*。

   keyword
     -- 在语言中永久预定义其含义的*保留字*之一。

   variable
     -- 参见 *variable declaration*。

   variable declaration
     -- 引入一个新的具名变量的声明；可以为该变量赋予可修改的初始值。

   constant
     -- 参见 *constant declaration*。

   constant declaration
     -- 引入一个新变量的声明；该变量在实例化时只能被赋予一次不可变初始值。

   grammar
     -- 用于描述某种编程语言将哪些终结符与非终结符序列解释为合法的规则集合。

     文法是若干产生式的集合。每个产生式由左侧的抽象符号（非终结符）和右侧由非终结符与终结符构成的序列组成。每个产生式都包含字符 ``':'`` 作为左右两侧的分隔符，并包含字符 ``';'`` 作为结束标记。

   production
     -- 某种编程语言解释为合法的终结符与非终结符序列。

   white space
     -- 为提高源代码可读性并避免歧义而用于分隔 token 的词法输入元素。

   widening conversion
     -- 不会造成数值整体大小信息丢失的转换。

   casting conversion
     -- 将强制类型转换表达式中的操作数转换为显式指定类型的转换。

   method
     -- 由类型参数、参数类型和返回类型组成的有序三元组。

   abstract declaration
     -- 指定方法名称和签名的普通接口方法声明。

   overloading
     -- 一种语言特性，允许使用同一个名称调用多个具有不同签名和不同实现体的函数（广义上包括方法和构造函数）。

   module level scope
     -- 在模块级声明的名称具有模块级作用域。如果该名称从一个模块导出并在另一个模块中导入，则可以在该另一个模块中访问它。

   namespace level scope
     -- 在命名空间中声明的名称具有命名空间级作用域。若被导出，则可以在命名空间外访问。

   class level scope
     -- 在类内部声明的名称。借助访问修饰符或派生类，它可以在类内部以及某些情况下在类外部访问。

   interface level scope
     -- 在接口内部声明的名称被视为具有接口级作用域。它可以在接口内部和外部访问。

   function type parameter scope
     -- 函数声明中类型参数名称的作用域。它与整个函数声明一致。

   method scope
     -- 在方法声明或函数声明体内直接声明的名称的作用域。方法作用域从该名称的声明位置开始，一直到该方法声明或函数声明体的末尾。

   function scope
     -- 与 *method scope* 相同。

   type parameter scope
     -- 在类或接口中声明的类型参数名称的作用域。类型参数作用域与整个声明一致（静态成员声明中除外）。

   static member
     -- 与某个特定类实例无关的类成员。静态成员可以通过限定名称记法在整个程序中使用（限定部分为类名）。

   linearization
     -- 将联合类型中所有嵌套类型展开为一个不再包含联合类型的平面序列。

   fit into (v.)
     -- 属于某个实体，或可隐式转换为某个实体（见 :ref:`Widening Numeric Conversions`）。

   match (v.)
     -- 与某个实体相对应。

   own (adj.)
     -- 指在类、接口、类型等中以文本形式直接声明的成员，与从基类（超类）或基接口（超接口）继承而来的成员相对。

   supercomponent (base component, parent component)
     -- 另一个组件由其派生而来的组件。

   subcomponent (derived component, child component)
     -- 由另一个组件生成、继承且依赖于该组件的组件。

   array length
     -- 可变大小数组中的元素个数。

   resizable array type
     -- 一种由多个元素构成、并且其元素个数可在运行时改变的内建类型。

   fixed-size array type
     -- 一种由多个元素构成、并且其长度只能设置一次、以获得更好性能的内建类型。

   array type
     -- 由多个元素构成的类型。

   concurrent execution
     -- 异步执行或并行执行（见 :ref:`Execution Model`）。

   parallel execution
     -- 一种执行模式，其中某些 C_JOBS 运行在不同的 C_WORKERS 上，因此可以同时推进（见 :ref:`Execution Model`）。

   asynchronous execution
     -- 一种执行模式，其中多个 C_JOBS 共享同一个 C_WORKER（见 :ref:`Execution Model`）。

   |C_WORKER|
     -- 对平台所提供并行单元的抽象，例如操作系统线程（见 :ref:`Execution Model`）。

   |C_JOB|
     -- 一段可以与其他 |C_JOBS| 并发执行的代码，并通过语言提供的机制传递其返回值（见 :ref:`Execution Model`）。

   coroutine
     -- 即 ``C_CORO``，也就是带有挂起点的 ``C_JOB``。协程的执行可以被挂起，并在稍后恢复（见 :ref:`Execution Model`）。

.. narrowing conversion
     -- a conversion that causes a loss of information about the overall
     magnitude of a numeric value, and a potential loss of precision
     and range.
     
   function types conversion
     -- 一种将某个函数类型转换为另一个函数类型的转换。



.. raw:: pdf

   PageBreak
