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


.. _Lexical Elements:

词法元素
################################################################################

.. meta:
    frontend_status: Done

本章讨论 |LANG| 编程语言的词法结构。

|

.. _Unicode Characters:

Unicode 字符的使用
********************************************************************************

.. meta:
    frontend_status: Done

|LANG| 编程语言使用 Unicode 字符集 [1]_ 中的字符作为其终结符。Unicode 的 UTF-16 编码以 16 位代码单元序列表示文本。

在本规范中，仅当需要涉及 Unicode 字符集和 UTF-16 编码这种表示方式时，才使用术语 *Unicode code point*。在这种表示方式与讨论无关的地方，则使用更通用的术语 *character*。

.. index::
   terminal symbol
   character
   Unicode character
   Unicode code point

|

.. _Lexical Input Elements:

词法输入元素
********************************************************************************

.. meta:
    frontend_status: Done

该语言具有以下几类*词法输入元素*：

-  :ref:`White Spaces`
-  :ref:`Line Separators`
-  :ref:`Tokens`
-  :ref:`Comments`

.. index::
   white space
   line separator
   token
   comment
   lexical input

|

.. _White Spaces:

空白符
********************************************************************************

.. meta:
    frontend_status: Done

空白符是将 token 相互分隔开的词法输入元素。空白符包括：

- 空格（U+0020）；
- 水平制表符（U+0009）；
- 垂直制表符（U+000B）；
- 换页符（U+000C）；
- 不换行空格（U+00A0）；以及
- 零宽不换行空格（U+FEFF）。

空白符提高源代码可读性并有助于避免歧义。空白符会被语法文法忽略（参见 :ref:`Grammar Summary`）。空白符不会出现在单个 token 内部，但可以出现在注释内部。

.. index::
   lexical input
   source code
   white space
   syntactic grammar
   comment
   token
   space
   horizontal tabulation
   form feed
   no-break space
   zero-width no-break space

|

.. _Line Separators:

行分隔符
********************************************************************************

.. meta:
    frontend_status: Done

行分隔符是将 token 相互分隔、并将 Unicode 输入字符序列划分为多行的词法输入元素。行分隔符包括：

- 换行符（U+000A 或 ASCII <LF>）；
- 回车符（U+000D 或 ASCII <CR>）；
- 行分隔符字符（U+2028 或 ASCII <LS>）；以及
- 段分隔符字符（U+2029 或 ASCII <PS>）。

行分隔符提高源代码可读性。任意一串连续的行分隔符都被视为一个分隔符。

.. index::
   lexical input
   newline character
   carriage return character
   line separator character
   paragraph separator character

除在具有特殊含义的场景外，行分隔符通常按空白符处理。详见 :ref:`Semicolons`。

|

.. _Tokens:

Token
********************************************************************************

.. meta:
    frontend_status: Done

Token 构成语言的词汇表。Token 分为四类：

-  :ref:`Identifiers`
-  :ref:`Keywords`
-  :ref:`Operators and Punctuators`
-  :ref:`Literals`

*Token* 是唯一可以作为语法文法终结符的词法输入元素（见 :ref:`Grammar Summary`）。在词法切分过程中，下一个 token 总是由能够构成合法 token 的最长字符序列组成。Token 之间通过空白符（见 :ref:`White Spaces`）、运算符或标点符号（见 :ref:`Operators and Punctuators`）分隔。空白符会被语法文法忽略（见 :ref:`Grammar Summary`）。

.. index::
   line separator
   lexical input element
   Unicode input character
   token
   tokenization
   white space
   source code
   identifier
   keyword
   operator
   punctuator
   literal
   terminal symbol
   syntactic grammar

|

.. _Identifiers:

标识符
********************************************************************************

.. meta:
    frontend_status: Done

*Identifier* 是由一个或多个合法 Unicode 字符组成的序列。标识符的 Unicode 文法基于 Unicode 标准中规定的字符属性。

标识符的首字符必须是 ``'$'``、``'_'``，或任意具有 Unicode 属性 ``ID_Start`` 的 Unicode 码点 [2]_。其余字符必须是具有相应 Unicode 属性的码点，或下列字符之一：

-  ``'$'`` (\\U+0024)
-  ``'``Zero-Width Non-Joiner``'`` (``<ZWNJ>``, \\U+200C)
-  ``'``Zero-Width Joiner``'`` (``<ZWJ>``, \\U+200D)

.. index::
   identifier
   Unicode Standard
   identifier
   Unicode code point
   Unicode character
   zero-width joiner
   zero-width non-joiner

.. code-block:: abnf

    Identifier:
      IdentifierStart IdentifierPart*
      ;

    IdentifierStart:
      UnicodeIDStart
      | '$'
      | '_'
      | '\\' EscapeSequence
      ;

    IdentifierPart:
      UnicodeIDContinue
      | '$'
      | ZWNJ
      | ZWJ
      | '\\' EscapeSequence
      ;

    ZWJ:
     '\u200C'
    ;
    ZWNJ:
     '\u200D'
    ;

    UnicodeIDStart
      : Letter
      | ['$']
      | '\\' UnicodeEscapeSequence;

    UnicodeIDContinue
      : UnicodeIDStart
      | UnicodeDigit
      | '\u200C'
      | '\u200D';

    UnicodeEscapeSequence:
      'u' HexDigit HexDigit HexDigit HexDigit
      | 'u' '{' HexDigit HexDigit+ '}'
      ;

    Letter
      : UNICODE_CLASS_LU
      | UNICODE_CLASS_LL
      | UNICODE_CLASS_LT
      | UNICODE_CLASS_LM
      | UNICODE_CLASS_LO
      ;
    UnicodeDigit
      : UNICODE_CLASS_ND
      ;

Unicode 字符类别 *UNICODE_CLASS_LU*、*UNICODE_CLASS_LL*、*UNICODE_CLASS_LT*、*UNICODE_CLASS_LM*、*UNICODE_CLASS_LO* 和 *UNICODE_CLASS_ND* 会在 :ref:`Grammar Summary` 中详细讨论。

|

.. _Keywords:

关键字
********************************************************************************

.. meta:
    frontend_status: Done

*Keywords* 是在 |LANG| 中具有永久预定义含义的保留字。关键字区分大小写。下列四个表给出了按类型分组后的关键字精确拼写。

1. *Hard keywords* 在任何上下文中都是保留字。硬关键字不能作为标识符使用：

.. index::
   keyword
   reserved word
   hard keyword
   soft keyword
   identifier
   context

+--------------------+-------------------+------------------+------------------+
|                    |                   |                  |                  |
+====================+===================+==================+==================+
|   ``abstract``     |   ``enum``        |   ``let``        |   ``super``      |
+--------------------+-------------------+------------------+------------------+
|   ``as``           |   ``export``      |   ``native``     |   ``this``       |
+--------------------+-------------------+------------------+------------------+
|   ``async``        |   ``extends``     |   ``new``        |   ``throw``      |
+--------------------+-------------------+------------------+------------------+
|   ``await``        |   ``false``       |   ``null``       |   ``true``       |
+--------------------+-------------------+------------------+------------------+
|   ``break``        |   ``final``       |   ``overload``   |   ``try``        |
+--------------------+-------------------+------------------+------------------+
|   ``case``         |   ``for``         |   ``override``   |   ``typeof``     |
+--------------------+-------------------+------------------+------------------+
|   ``class``        |   ``function``    |   ``package``    |   ``undefined``  |
+--------------------+-------------------+------------------+------------------+
|   ``const``        |   ``if``          |   ``private``    |   ``while``      |
+--------------------+-------------------+------------------+------------------+
|   ``constructor``  |   ``implements``  |   ``protected``  |                  |
+--------------------+-------------------+------------------+------------------+
|   ``continue``     |   ``import``      |   ``public``     |                  |
+--------------------+-------------------+------------------+------------------+
|   ``default``      |   ``in``          |   ``return``     |                  |
+--------------------+-------------------+------------------+------------------+
|   ``do``           |   ``instanceof``  |   ``static``     |                  |
+--------------------+-------------------+------------------+------------------+
|   ``else``         |   ``interface``   |   ``switch``     |                  |
+--------------------+-------------------+------------------+------------------+

2. 预定义类型的名称及其别名也属于 *hard keywords*，不能用作标识符：

+---------------+---------------+
| Primary Name  | Alias         |
+===============+===============+
| ``Any``       |               |
+---------------+---------------+
| ``bigint``    | ``BigInt``    |
+---------------+---------------+
| ``boolean``   | ``Boolean``   |
+---------------+---------------+
| ``byte``      | ``Byte``      |
+---------------+---------------+
| ``char``      | ``Char``      |
+---------------+---------------+
| ``double``    | ``Double``    |
+---------------+---------------+
| ``float``     | ``Float``     |
+---------------+---------------+
| ``int``       | ``Int``       |
+---------------+---------------+
| ``long``      | ``Long``      |
+---------------+---------------+
| ``number``    | ``Number``    |
+---------------+---------------+
| ``Object``    | ``object``    |
+---------------+---------------+
| ``short``     | ``Short``     |
+---------------+---------------+
| ``string``    | ``String``    |
+---------------+---------------+
| ``void``      |               |
+---------------+---------------+

3. *Soft keywords* 只在特定上下文中具有保留含义；在其他位置，它们仍然是合法标识符：

.. index::
   keyword
   soft keyword
   identifier

+--------------------+--------------------+
|                    |                    |
+====================+====================+
|      ``catch``     |     ``namespace``  |
+--------------------+--------------------+
|      ``declare``   |     ``of``         |
+--------------------+--------------------+
|      ``finally``   |     ``out``        |
+--------------------+--------------------+
|      ``from``      |    ``readonly``    |
+--------------------+--------------------+
|      ``get``       |    ``set``         |
+--------------------+--------------------+
|      ``keyof``     |    ``type``        |
+--------------------+--------------------+
|      ``module``    |                    |
+--------------------+--------------------+

4. 下列标识符也被视为 *soft keywords*，保留供 |LANG| 将来使用，或当前已在 |TS| 中作为软关键字使用：

.. index::
   identifier
   soft keyword

+---------------+---------------+---------------+---------------+----------------+
|               |               |               |               |                |
+===============+===============+===============+===============+================+
|    ``is``     |   ``memo``    |   ``struct``  |    ``var``    |  ``yield``     |
+---------------+---------------+---------------+---------------+----------------+

|

.. _Operators and Punctuators:

运算符与标点符号
********************************************************************************

.. meta:
    frontend_status: Done

*Operators* 是表示对值执行各种操作的 token，例如加法、减法、比较等。关键字 ``instanceof`` 和 ``typeof`` 也起运算符作用。

*Punctuators* 是用于分隔、结束或以其他方式组织程序元素和组成部分的 token。标点符号包括逗号、分号、圆括号、方括号等。

运算符和标点符号由下列字符序列表示：

.. index::
   operator
   token
   value
   addition
   subtraction
   comparison
   punctuator
   semicolon
   parentheses
   comma
   square bracket
   keyword

+----------+----------+----------+----------+----------+----------+----------+
+----------+----------+----------+----------+----------+----------+----------+
|  ``+``   |  ``&``   |  ``+=``  | ``|=``   | ``&=``   |  ``<``   |  ``?.``  |
+----------+----------+----------+----------+----------+----------+----------+
|  ``-``   |  ``|``   |  ``-=``  | ``^=``   | ``&&``   |  ``>``   |  ``!``   |
+----------+----------+----------+----------+----------+----------+----------+
|  ``*``   |  ``^``   |  ``*=``  | ``<<=``  | ``||``   |  ``===`` |  ``<=``  |
+----------+----------+----------+----------+----------+----------+----------+
|  ``/``   |  ``>>``  |  ``/=``  | ``>>=``  | ``++``   |  ``==``  |  ``>=``  |
+----------+----------+----------+----------+----------+----------+----------+
|  ``%``   |  ``<<``  |  ``%=``  | ``>>>=`` | ``--``   |  ``=``   |  ``...`` |
+----------+----------+----------+----------+----------+----------+----------+
|  ``(``   |  ``)``   |  ``[``   | ``]``    | ``{``    |  ``}``   |  ``??``  |
+----------+----------+----------+----------+----------+----------+----------+
|  ``,``   |  ``;``   |  ``.``   | ``:``    | ``!=``   |  ``!==`` |  ``**``  |
+----------+----------+----------+----------+----------+----------+----------+
|  ``**=`` |          |          |          |          |          |          |
+----------+----------+----------+----------+----------+----------+----------+

|

.. _Literals:

字面量
********************************************************************************

.. meta:
    frontend_status: Done

*Literals* 是 :ref:`预定义类型 <Predefined Types>` 或
:ref:`字面量类型 <Literal Types>` 的值。

.. code-block:: abnf

    Literal:
      IntegerLiteral
      | FloatLiteral
      | BigIntLiteral
      | BooleanLiteral
      | StringLiteral
      | MultilineStringLiteral
      | NullLiteral
      | UndefinedLiteral
      | CharLiteral
      ;

下面将分别详细说明每一种字面量。实验性的 ``char literal`` 在对应的 :ref:`char Literals` 说明中讨论。

.. index::
   literal
   char

|

.. _Numeric Literals:

数值字面量
================================================================================

.. meta:
    frontend_status: Done

*Numeric literals* 包括整数和浮点数字面量。

|

.. _Integer Literals:

整数字面量
================================================================================

.. meta:
    frontend_status: Done

整数字面量表示既不包含小数点也不包含指数部分的数字。整数字面量可以使用 16 进制（hexadecimal）、10 进制（decimal）、8 进制（octal）和 2 进制（binary）书写，如下所示：

.. index::
   integer
   literal
   hexadecimal
   decimal
   octal
   binary
   radix

.. code-block:: abnf

    IntegerLiteral:
      DecimalIntegerLiteral
      | HexIntegerLiteral
      | OctalIntegerLiteral
      | BinaryIntegerLiteral
      ;

    DecimalIntegerLiteral:
      '0'
      | DecimalDigitNotZero ('_'? DecimalDigit)*
      ;

    DecimalDigit:
      [0-9]
      ;

    DecimalDigitNotZero:
      [1-9]
      ;

    HexIntegerLiteral:
      '0' [xX]  ( HexDigit
      | HexDigit (HexDigit | '_')* HexDigit
      )
      ;

    HexDigit:
      [0-9a-fA-F]
      ;

    OctalIntegerLiteral:
      '0' [oO] ( OctalDigit
      | OctalDigit (OctalDigit | '_')* OctalDigit )
      ;

    OctalDigit:
      [0-7]
      ;

    BinaryIntegerLiteral:
      '0' [bB] ( BinaryDigit
      | BinaryDigit (BinaryDigit | '_')* BinaryDigit )
      ;

    BinaryDigit:
      [0-1]
      ;

不同进制的整数字面量示例如下：

.. code-block:: typescript
   :linenos:

    153 // decimal literal
    1_153 // decimal literal
    0xBAD3 // hex literal
    0xBAD_3 // hex literal
    0o777 // octal literal
    0b101 // binary literal

连续数字之间可以插入下划线字符 ``'_'`` 以提高可读性。位于这些位置的下划线不会改变字面量的值。但是，下划线既不能位于整数字面量的最开头，也不能位于其最后。

.. index::
   underscore character
   compiler-internal integer type

整数字面量的类型是 compiler-internal integer type（见
:ref:`Specifics of Constant Expressions Evaluation`），该类型用于常量表达式求值。
:ref:`Type Inference for Constant Expressions` 总是在常量表达式求值后应用，
用于把数值常量表达式推断为某个预定义数值类型。

如果整数字面量是常量表达式的一部分（见 :ref:`Constant Expressions`），
其值可以超出 ``long`` 的范围，但整个整数常量表达式的值必须落在 ``long``
范围内，或者由上下文决定类型。

下面的示例说明了常量表达式如何通过类型推断确定类型。注意 ``'-'`` 不是整数字面量的一部分，
而是 :ref:`Unary Minus` 运算符：

.. code-block:: typescript
   :linenos:

    const max_int1 = 21474836477 // OK, type: int, value: max(int)
    const max_int2 = 0x7FFFFFFF  // OK, type: int, value: max(int)

    const min_int1 = - 2147483648 // OK, type: int, value: min(int)
    const min_int2 = - 0x80000000 // OK, type: int, value: min(int)

    const long1    =   0x80000000 // OK, type: long

    const err1: int = 2147483648 // Compile-time error, the value is out of range for 'int'

    const max_long = 0x7FFF_FFFF_FFFF_FFFF   // OK, type: long, value: max(long)
    const min_long = - 0x8000_0000_0000_0000 // OK, type: long, value: min(long)

    const err2 = 0x8000_0000_0000_0000 // Compile-time error, the value is too large
    const err3 = 9223372036854775808   // Compile-time error, the value is too large

    const max_long2 = 0x7FFF_FFFF_FFFF_FFFF + 10 - 10  // OK, type: long, value: max(long)

|

.. _Floating-Point Literals:

浮点数字面量
================================================================================

.. meta:
    frontend_status: Done

*Floating-point literals* 表示十进制数，由整数部分、小数点、小数部分、指数部分以及 ``float`` 类型后缀组成，形式如下：

.. code-block:: abnf

    FloatLiteral:
        DecimalIntegerLiteral '.' FractionalPart? ExponentPart? FloatTypeSuffix?
        | '.' FractionalPart ExponentPart? FloatTypeSuffix?
        | DecimalIntegerLiteral ExponentPart? FloatTypeSuffix
        ;

    ExponentPart:
        [eE] [+-]? DecimalIntegerLiteral
        ;

    FractionalPart:
        DecimalDigit
        | DecimalDigit (DecimalDigit | '_')* DecimalDigit
        ;
    FloatTypeSuffix:
        'f'
        ;

这一概念示例如下：

.. code-block:: typescript
   :linenos:

    3.14
    3.14f
    3.141_592
    .5
    1234f
    1e10
    1e10f

连续数字之间可以插入下划线字符 ``'_'`` 以提高可读性。位于这些位置的下划线不会改变字面量的值。但是，下划线既不能位于字面量的最开头，也不能位于其最后。

浮点数字面量属于与其匹配的浮点类型，规则如下：

- 如果存在 *float type suffix*，则类型为 ``float``；
- 否则类型为 ``double``（类型 ``number`` 是 ``double`` 的别名）。

如果浮点数字面量对其类型来说过大，则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    // Compile-time error, the value is too large for type float:
    3.4e39f

    // Compile-time error, the value is too large for type double:
    1.7e309

.. index::
   floating-point literal
   compile-time error
   prefix
   underscore character
   implicit conversion
   constant declaration
   decimal number
   radix
   readability

|

.. _Bigint Literals:

Bigint 字面量
================================================================================

.. meta:
    frontend_status: Partly
    todo: hex, octal, binary literals

*Bigint literals* 表示位数不受限制的整数。

Bigint 字面量的类型始终为 bigint（见 :ref:`Type bigint`）。

``bigint`` 字面量是在*整数字面量*后追加符号 ``'n'`` 得到的：

.. code-block:: abnf

    BigIntLiteral: DecimalIntegerLiteral 'n';


.. BigIntLiteral: IntegerLiteral 'n';


这一概念示例如下：

.. code-block:: typescript

    153n // bigint literal
    1_153n // bigint literal
    -153n // negative bigint literal

.. 0xBAD_3n // bigint literal in hexadecimal notation

连续数字之间可以插入下划线字符 ``'_'`` 以提高可读性。位于这些位置的下划线不会改变字面量的值。但是，下划线既不能位于 ``bigint`` 字面量的最开头，也不能位于其最后。

表示数字的字符串或任意整数值，都可以通过内建函数转换为 ``bigint``，如下所示：

.. code-block-meta:
    skip

.. code-block:: typescript

    BigInt(other: string): bigint
    BigInt(other: long): bigint

.. index::
   integer
   bigint literal
   underscore character
   readability
   string
   number
   integer value

有两个方法可以截取 ``bigint`` 数值的低 *bitsCount* 位并将其作为结果返回。它们分别提供有符号和无符号版本：

.. code-block:: typescript

    asIntN(bitsCount: long, bigIntToCut: bigint): bigint
    asUintN(bitsCount: long, bigIntToCut: bigint): bigint

.. index::
   decimal
   radix

|

.. _Boolean Literals:

布尔字面量
================================================================================

.. meta:
    frontend_status: Done

*Boolean literal* 的值由两个关键字表示：``true`` 和 ``false``。

.. code-block:: abnf

    BooleanLiteral:
        'true' | 'false'
        ;

*Boolean literals* 的类型为 ``boolean``。

.. index::
   keyword
   Boolean literal
   literal value
   literal

|

.. _String Literals:

字符串字面量
================================================================================

.. meta:
    frontend_status: Done
    todo: "" sample is invalid: SyntaxError: Newline is not allowed in strings

*String literals* 由零个或多个字符组成，并包裹在单引号或双引号之间。*multiline string* 字面量（见 :ref:`Multiline String Literal`）是字符串字面量的一种特殊形式。

String 字面量的类型是与字面量本身对应的字面量类型。如果对该字面量应用了运算符，则该字面量类型会被替换为 string（见 :ref:`Type string`）。

.. index::
   string literal
   multiline string
   predefined reference type

.. code-block:: abnf

    StringLiteral:
        '"' DoubleQuoteCharacter* '"'
        | '\'' SingleQuoteCharacter* '\''
        ;

    DoubleQuoteCharacter:
        ~["\\\r\n]
        | '\\' EscapeSequence
        ;

    SingleQuoteCharacter:
        ~['\\\r\n]
        | '\\' EscapeSequence
        ;

    EscapeSequence:
        ['"bfnrtv0\\]
        | 'x' HexDigit HexDigit
        | 'u' HexDigit HexDigit HexDigit HexDigit
        | 'u' '{' HexDigit+ '}'
        | ~[1-9xu\r\n]
        ;

字符串字面量中的字符通常按其本身解释。不过，一些不可见字符可以通过显式写法或 Unicode 编码来表示，这类构造称为 *escape sequences*。

转义序列也可以在*字符串字面量*中表示可见字符，例如单引号（``'``）、双引号（``"``）、反斜杠（``\``）等。转义序列总是以反斜杠字符 ``'\'`` 开始，后接下列字符之一：

.. index::
   string literal
   escape sequence
   backslash
   single quote
   double quotes

-  ``"`` （双引号，U+0022）
-  ``'`` （中性单引号，U+0027）
-  ``b`` （退格，U+0008）
-  ``f`` （换页，U+000c）
-  ``n`` （换行，U+000a）
-  ``r`` （回车，U+000d）
-  ``t`` （水平制表符，U+0009）
-  ``v`` （垂直制表符，U+000b）
-  ``\`` （反斜杠，U+005c）
-  ``x`` 加两个十六进制数字（如 ``7F``）
-  ``u`` 加四个十六进制数字，构成固定长度 Unicode 转义序列，例如 ``\u005c``
-  ``u{`` 加至少一个十六进制数字，再跟 ``}``，构成有界 Unicode 转义序列，例如 ``\u{5c}``
-  除数字 ``1`` 至 ``9`` 以及字符 ``'x'``、``'u'``、``'CR'``、``'LF'`` 之外的任意单个字符

.. index::
   string literal
   escape sequence
   backslash
   horizontal tab
   form feed
   backspace
   vertical tab
   hexadecimal
   Unicode escape sequence

转义序列示例如下：

.. code-block:: typescript
   :linenos:

    let s1 = 'Hello, world!'
    let s2 = "Hello, world!"
    let s3 = "\\"
    let s4 = ""
    let s5 = "don’t worry, be happy"
    let s6 = 'don\'t worry, be happy'
    let s7 = 'don\u0027t worry, be happy'

|

.. _Multiline String Literal:

多行字符串字面量
================================================================================

.. meta:
    frontend_status: Done

*Multiline strings* 可以包含任意文本，并由反引号字符 ``'`` \` ``'`` 包围。反斜杠字符 ``'\'`` 用于转义其后的下一个字符。多行字符串可以包含任意字符，但不能包含未转义的反引号。行尾会按换行字符处理：

.. index::
   string literal
   multiline string literal
   multiline string
   string interpolation
   multiline string
   backtick
   escape
   newline
   character

.. code-block:: abnf

    MultilineStringLiteral:
        '`' (BackticksContentCharacter)* '`'
        ;

    BackticksContentCharacter:
        ~[`\\\r\n]
        | '\\' EscapeSequence
        | LineContinuation
        ;

     LineContinuation:
        '\\' [\r\n\u2028\u2029]+
        ;

*embeddedExpression* 的文法会在 :ref:`String Interpolation Expressions` 中描述。

多行字符串示例如下：

.. code-block:: typescript
   :linenos:

    let sentence1 =`This is an example of
                    a \`multiline\` string
                    to be enclosed in
                    backticks`

    let sentence2 = `This is an example of
    a \`multiline\` string
    to be enclosed in
    backticks`

    console.log(sentence1)
    console.log(sentence2)

.. note::
   行首空格既不会被压缩，也不会被裁剪。

输出如下：

::

  This is an example of
                  a `multiline` string
                  to be enclosed in
                  backticks

  This is an example of
  a `multiline` string
  to be enclosed in
  backticks


MultilineString 字面量的类型是与其字面量值对应的字面量类型。如果对其应用运算符，则该字面量类型会被替换为 string（见 :ref:`Type String`）。

.. index::
   multiline string
   operator
   literal
   literal type


|

.. _Undefined Literal:

``Undefined`` 字面量
================================================================================

.. meta:
    frontend_status: Done

*Undefined literal* 是类型 ``void`` 和 ``undefined``（见 :ref:`Type undefined or void`）中唯一的字面量，用于表示其值尚未定义的引用。*undefined literal* 由关键字 ``undefined`` 表示：

.. code-block:: abnf

    UndefinedLiteral:
        'undefined'
        ;

.. index::
   undefined literal
   type undefined
   type void
   keyword

|

.. _Null Literal:

``Null`` 字面量
================================================================================

.. meta:
    frontend_status: Done

*Null literal* 是类型 ``null``（见 :ref:`Type null`）中唯一的字面量，用于表示一个不指向任何实体的引用。null 字面量由关键字 ``null`` 表示：

.. code-block:: abnf

    NullLiteral:
        'null'
        ;

该值通常用于 ``T | null`` 这样的类型（见 :ref:`Nullish Types`）。

.. index::
   null literal
   null reference
   nullish type
   type null

|

.. _Comments:

注释
********************************************************************************

.. meta:
    frontend_status: Done

*Comment* 是加入到文本流中、用于说明和补充源代码的文本片段。注释对于语法文法（见 :ref:`Grammar Summary`）没有意义。

*Line comments* 以字符序列 ``'//'`` 开始，如下例所示，并在行分隔符处结束。这两个界定符之间可以出现任意字符或字符序列，但都会被忽略。

.. code-block:: typescript
   :linenos:

    // This is a line comment

*Multiline comments* 以字符序列 ``'/*'`` 开始，如下例所示，并在之后遇到的第一个字符序列 ``'*/'`` 处结束。它们之间允许出现任意字符或字符序列，但都会被忽略。

.. code-block:: typescript
   :linenos:

    /*
        This is a multiline comment
    */

注释不能嵌套。

.. index::
   comment
   syntactic grammar
   multiline comment

|

.. _Semicolons:

分号
********************************************************************************

.. meta:
    frontend_status: Done

声明和语句通常以行分隔符结束（见 :ref:`Line Separators`）。在某些情况下，必须使用分号来分隔写在同一行中的语法产生式，或者避免歧义。

.. index::
   declaration
   statement
   line separator
   syntax production
   semicolon

.. code-block:: typescript
   :linenos:

    function foo(x: number): number {
        x++;
        x *= x;
        return x
    }

    let i = 1
    i-i++ // one expression
    i;-i++ // two expressions

-------------

.. [1]
   Unicode Standard Version 15.0.0,
   https://www.unicode.org/versions/Unicode15.0.0/

.. [2]
   https://unicode.org/reports/tr31/

.. raw:: pdf

   PageBreak
