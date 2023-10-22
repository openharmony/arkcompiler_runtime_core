..
    Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

Lexical Elements
################

.. meta:
    frontend_status: Done

This chapter discusses the lexical structure of the |LANG| programming language
and the analytical conventions.

|

.. _Unicode Characters:

Use of Unicode Characters
*************************

.. meta:
    frontend_status: Done

The |LANG| programming language uses characters of the Unicode Character
set [1]_ as its terminal symbols, and represents text in sequences of
16-bit code units using the Unicode UTF-16 encoding.

The term *Unicode code point* is used in this specification only where such
representation is relevant to refer the reader to Unicode Character set and
UTF-16 encoding. Where such representation is irrelevant to the discussion,
the generic term *character* is used.

.. index::
   terminal symbol
   character
   Unicode character
   Unicode code point

|

.. _Lexical Input Elements:

Lexical Input Elements
**********************

.. meta:
    frontend_status: Done

The language has lexical input elements of the following types:
:ref:`White Spaces`, :ref:`Line Separators`, :ref:`Tokens` and :ref:`Comments`.

|

.. _White Spaces:

White Spaces
************

.. meta:
    frontend_status: Done

White spaces are lexical input elements that separate tokens from one another
to improve the source code readability and avoid ambiguities. White spaces are
ignored by the syntactic grammar.

White spaces include the following:

- Space (U+0020),

- Horizontal tabulation (U+0009),

- Vertical tabulation (U+000B),

- Form feed (U+000C),

- No-break space (U+00A0),

- Zero width no-break space (U+FEFF).

White spaces can occur within comments but not within any kind of token.

.. index::
   lexical input element
   source code
   white space
   syntactic grammar
   comment

|

.. _Line Separators:

Line Separators
***************

.. meta:
    frontend_status: Done

Line separators are lexical input elements that divide sequences of Unicode
input characters into lines and separate tokens from one another to improve
the source code readability.

Line separators include the following:

- Newline character (U+000A or ASCII <LF>),

- Carriage return character (U+000D or ASCII <CR>),

- Line separator character (U+2028 or ASCII <LS>),

- Paragraph separator character (U+2029 or ASCII <PS>).

Any sequence of line separators is considered a single separator.

|

.. _Tokens:

Tokens
******

.. meta:
    frontend_status: Done

Tokens form the vocabulary of the language. There are four classes of tokens:
:ref:`Identifiers`, :ref:`Keywords`, :ref:`Operators and Punctuators`,
and :ref:`Literals`.

Token is the only lexical input element which can act as a terminal symbol
of the syntactic grammar. In the process of tokenization, the next token is
always the longest sequence of characters that form a valid token. White
spaces (see :ref:`White spaces`) are ignored except when they separate
tokens that would otherwise merge into a single token.

In many cases line separators are treated as white spaces except where line
separators have special meanings. See :ref:`Semicolons` for more details.

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

Identifiers
***********

.. meta:
    frontend_status: Done
    todo: Identifier starting with '\$\$' is invalid now, double dollar is an operator now, but single dollar and double dollar in middle of the identifier is still valid.

An identifier is a sequence of one or more valid Unicode characters. The
Unicode grammar of identifiers is based on character properties
specified by the Unicode Standard.

The first character in an identifier must be '\$', '\_', or any Unicode
code point with the Unicode property 'ID_Start'[2]_. Other characters
must be Unicode code points with the Unicode property, or one of the following
characters: '\$' (\\U+0024), 'Zero Width Non-Joiner' (<ZWNJ>, \\U+200C) or
'Zero Width Joiner' (<ZWNJ>, \\U+200D).

.. index::
   identifier
   Unicode Standard
   identifier
   Unicode code point
   Unicode character
   
.. code-block:: abnf

    Identifier:
        IdentifierStart IdentifierPart \*
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
        | <ZWNJ>
        | <ZWJ>
        | '\\' EscapeSequence
        ;

|

.. _Keywords:

Keywords
********

.. meta:
    frontend_status: Partly
    todo: not yet added: inner
    todo: type: hard keyword in spec, soft keyword in implementation
    todo: of, default: soft keyword in spec, hard keyword in implementation

*Keywords* are the reserved words that have permanently predefined meanings
in the language.

The following keywords are reserved in any context (*hard keywords*) and
cannot be used as identifiers:

.. index::
   keyword
   reserved word
   hard keyword
   soft keyword
   identifier
   context
   
+---------------+---------------+---------------+---------------+
|               |               |               |               |
+===============+===============+===============+===============+
| abstract      | else          | interface     | return        |
+---------------+---------------+---------------+---------------+
| as            | enum          | internal      | static        |
+---------------+---------------+---------------+---------------+
| assert        | export        | launch        | switch        |
+---------------+---------------+---------------+---------------+
| async         | extends       | let           | super         |
+---------------+---------------+---------------+---------------+
| await         | false         | native        | this          |
+---------------+---------------+---------------+---------------+
| break         | final         | new           | throw         |
+---------------+---------------+---------------+---------------+
| case          | for           | null          | true          |
+---------------+---------------+---------------+---------------+
| class         | function      | override      | try           |
+---------------+---------------+---------------+---------------+
| const         | if            | package       | type          |
+---------------+---------------+---------------+---------------+
| constructor   | implements    | private       | while         |
+---------------+---------------+---------------+---------------+
| continue      | import        | protected     |               |
+---------------+---------------+---------------+---------------+
| do            | instanceof    | public        |               |
+---------------+---------------+---------------+---------------+

The following words have special meaning in certain contexts (*soft
keywords*) but are valid identifiers elsewhere:

.. index::
   keyword
   soft keyword
   identifier

+---------------+---------------+---------------+---------------+
|               |               |               |               |
+===============+===============+===============+===============+
| catch         | from          | out           | struct        |
+---------------+---------------+---------------+---------------+
| declare       | get           | readonly      | throws        |
+---------------+---------------+---------------+---------------+
| default       | in            | rethrows      |               |
+---------------+---------------+---------------+---------------+
| finally       | of            | set           |               |
+---------------+---------------+---------------+---------------+

The following words cannot be used as user-defined type names but are
not otherwise restricted:

.. index::
   user-defined type name

+---------------+---------------+---------------+---------------+
|               |               |               |               |
+===============+===============+===============+===============+
| boolean       | double        | number        | void          |
+---------------+---------------+---------------+---------------+
| byte          | float         | short         |               |
+---------------+---------------+---------------+---------------+
| bigint        | int           | string        |               |
+---------------+---------------+---------------+---------------+
| char          | long          | undefined     |               |
+---------------+---------------+---------------+---------------+

The following identifiers are also treated as keywords and are reserved for
the future use (or used in TS):

.. index::
   identifier
   keyword

+---------------+---------------+---------------+---------------+
|               |               |               |               |
+===============+===============+===============+===============+
| is            | typeof        | var           | yield         |
+---------------+---------------+---------------+---------------+

Keywords are always lowercase.

|

.. _Operators and Punctuators:

Operators and Punctuators
*************************

.. meta:
    frontend_status: Partly
    todo: note: ?? and ?. are not implemented yet

*Operators* are tokens that denote various actions on values. Examples are
addition, subtraction, comparisons and others.

*Punctuators* are tokens that serve for separating, completing or otherwise
organizing program elements and parts. The examples are commas, semicolons,
parentheses, square brackets, etc.

The following character sequences represent operators and punctuators:

.. index::
   operator
   token
   value
   addition
   subtraction
   comparison
   punctuator

+------+------+------+------+------+------+------+-----+-----+
+------+------+------+------+------+------+------+-----+-----+
|      |      |      |  &=  |      |  ==  |  ??  |     |     |
+------+------+------+------+------+------+------+-----+-----+
|  \+  |   &  |  \+= |  \|= |      |  <   |  ?.  |  (  |  )  |
+------+------+------+------+------+------+------+-----+-----+
|  \-  |  \|  |  \-= |  \^= |  &&  |  >   |  !.  |  [  |  ]  |
+------+------+------+------+------+------+------+-----+-----+
|  \*  |  \^  |  \*= |  <<= | \||  |  === |  <=  |  {  |  }  |
+------+------+------+------+------+------+------+-----+-----+
|  /   |  >>  |  /=  |  >>= | \++  |  =   |  >=  |  ,  |  ;  |
+------+------+------+------+------+------+------+-----+-----+
|  %   |  <<  |  %=  | >>>= | \--  |  !   | \... | \.  | \:  |
+------+------+------+------+------+------+------+-----+-----+

|

.. _Literals:

Literals
********

.. meta:
    frontend_status: Partly

*Literals* are representations of certain value types.

See :ref:`Char Literals` for the experimental *char literal*.

.. index::
   literal
   value type
   char

|
   
.. _Integer Literals:

Integer Literals
================

.. meta:
    frontend_status: Done
    todo: note: let number: long=0xFFFFFFFF --> 0xFFFFFFFF FFFFFFFF, because the int typed -1 is sign extended. 0xFFFFFFFF is not interpreted as 4294967295, decause it does not fit in an int

*Integer literals* represent numbers that do not have a decimal point or
an exponential part. Integer literals can be written with bases 16
(hexadecimal), 10 (decimal), 8 (octal) and 2 (binary).

.. index::
   integer
   literal
   hexadecimal
   decimal
   octal
   binary
   
   
.. code-block:: abnf

    DecimalIntegerLiteral:
      '0'
      | [1-9] ('_'? [0-9])* 
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
      '0' [oO] ( [0-7] | [0-7] [0-7_]* [0-7] )
      ;

    BinaryIntegerLiteral:
      '0' [bB] ( [01] | [01] [01_]* [01] )
      ;

Examples:

.. code-block:: typescript
   :linenos:

    153 // decimal literal
    1_153 // decimal literal
    0xBAD3 // hex literal
    0xBAD_3 // hex literal
    0o777 // octal literal
    0b101 // binary literal

The underscore character '_' can be used after a base prefix or between
successive digits in order to denote an integer literal and improve
readability. Underscore characters in such positions do not change the
values of literals. However, an underscore character must not be used as
the very first and the very last symbol of an integer literal.

.. index::
   prefix
   value
   literal
   integer
   underscore character

Integer literals are of type *int* if the value can be represented by a
32-bit number; it is of type *long* otherwise. In variable and constant
declarations, an integer literal can be implicitly converted to another
*integer* or *char* type (see :ref:`Type Compatibility with Initializer`). In
all other places an explicit cast must be used (see :ref:`Cast Expressions`).

.. index::
   integer literal
   int
   long
   constant declaration
   variable declaration
   integer literal
   char
   explicit cast
   implicit conversion
   cast expression

|

.. _Floating-Point Literals:

Floating-Point Literals
=======================

.. meta:
    frontend_status: Partly
    todo: let d = 9999.0009E-9994 --> Inf, but should be 0
    todo: let d = 4.9e-324; (in stdlib Double.ets) --> Inf, but should be 0x000000000000001 double

*Floating-point literals* represent decimal numbers and consist of a
whole-number part, a decimal point, a fraction part and
an exponent.
   
.. code-block:: abnf

    FloatLiteral:
        DecimalIntegerLiteral '.' FractionalPart? ExponentPart?
        | '.' FractionalPart ExponentPart?
        | DecimalIntegerLiteral ExponentPart
        ;

    ExponentPart:
        [eE] [+-]? DecimalIntegerLiteral
        ;

    FractionalPart:
        [0-9]
        | [0-9] [0-9_]* [0-9]
        ;

Examples:

.. code-block:: typescript
   :linenos:

    3.14
    3.141_592
    .5
    1e10

In order to denote a floating-point literal, the underscore character '\_' can
be used after a base prefix or between successive digits for readability.
Underscore characters in such positions do not change the values of literals.
However, the underscore character must not be used as the very first and very
last symbol of an integer literal.

A floating-point literal is of type *double* (the type *number* is
an alias to *double*). In variable and constant declarations, a
floating-point literal can be implicitly converted to type *float*
(see :ref:`Type Compatibility with Initializer`).

.. index::
   floating-point literal
   prefix
   underscore character
   implicit conversion
   constant declaration

|

.. _BigInt Literals:

BigInt Literals
===============

.. meta:
    frontend_status: None

*BigInt literals* represent integer numbers with unlimited number of digits.
BigInt literals use decimal base only. It is a sequence of digits which ends
with the symbol 'n'.

.. code-block:: abnf

    BigIntLiteral:
      '0n'
      | [1-9] ('_'? [0-9])* 'n'
      ;

Examples:

.. code-block:: typescript

    153n // bigint literal
    1_153n // bigint literal

The underscore character '_' can be used between successive digits in order
to denote a bigint literal and improve readability. Underscore characters in
such positions do not change the values of literals. However, an underscore
character must not be used as the very first and the very last symbol
of a bigint literal.

BigInt literals are always of type *bigint*. 

Built-in functions as shown below

.. code-block:: typescript

    BigInt (other: string): bigint
    BigInt (other: long): bigint

allow converting strings that represent numbers, or any integer values into
*bigint* ones.

.. index::
   integer
   BigInt literal
   underscore character
   static function

Two other static functions allow to take *bitsCount* lower bits of the
BigInt number and return them as a result. Signed and unsigned versions
are available:

.. code-block:: typescript

    BigInt.asIntN(bitsCount: long, bigIntToCut: bigint): bigint
    BigInt.asUintN(bitsCount: long, bigIntToCut: bigint): bigint

|

.. _Boolean Literals:

Boolean Literals
================

.. meta:
    frontend_status: Done

There are two *Boolean literal* values represented by the keywords
``true`` and ``false``.

.. code-block:: abnf
   :linenos:

    BooleanLiteral:
        ’true’ | ’false’
        ;

Boolean literals are of type *boolean*.

.. index::
   keyword
   Boolean literal

|

.. _String Literals:

String Literals
===============

.. meta:
    frontend_status: Done
    todo: "" sample is invalid: SyntaxError: Newline is not allowed in strings

*String literals* consist of zero or more characters enclosed between
single or double quotes. A special form of string literals is
*template literal* (see :ref:`Template Literals`).

String literals are of type *string*, which is a predefined reference
type (see :ref:`String Type`).

.. index::
   string literal
   template literal
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

Normally, characters in string literals represent themselves. However,
certain non-graphic characters can be represented by explicit specifications
or Unicode codes. Such constructs are called *escape sequences*.

Graphic characters within a string literal, e.g., single quotes '``'``', double
quotes '``"``', backslashes '``\``' and some others, can also be represented by
escape sequences.

.. index::
   string literal
   escape sequence
   backslash
   single quote
   double quotes

An escape sequence always starts with the backslash character '``\``', followed
by one of the following characters:

-  ``"`` (double quote, U+0022)

-  ``'`` (single quote, U+0027)

-  ``b`` (backspace, U+0008)

-  ``f`` (form feed, U+000c)

-  ``n`` (linefeed, U+000a)

-  ``r`` (carriage return, U+000d)

-  ``t`` (horizontal tab, U+0009)

-  ``v`` (vertical tab, U+000b)

-  ``\\`` (backslash, U+005c)

-  ``x`` and two hexadecimal digits, like ``7F``

-  ``u`` and four hexadecimal digits, forming fixed Unicode escape
   sequence, like ``\u005c``

-  ``u{`` and at least one hexadecimal digit, followed by ``}``, forming
   a bounded Unicode escape sequence, like ``\u{5c}``

-  any single character, except digits from ‘1’ to ‘9’ and characters
   ‘x’, ‘u’, CR and LF.

Examples:

.. code-block:: typescript
   :linenos:

    let s1 = 'Hello, world!'
    let s2 = "Hello, world!"
    let s3 = "\\"
    let s4 = ""
    let s5 = "don’t do it"
    let s6 = 'don\'t do it'
    let s7 = 'don\u0027t do it'

|

.. _Template Literals:

Template Literals
=================

.. meta:
    frontend_status: Done

Multi-line string literals that can include embedded expressions are called
*template literals*.

A *template literal* with an embedded expression is a *template string*.

A *template string* is not exactly a literal because its value cannot be
evaluated at compile time. The evaluation of a template string is called
*string interpolation* (see :ref:`String Interpolation Expressions`).

.. index::
   string literal
   template literal
   template string
   string interpolation
   multi-line string

.. code-block:: abnf

    TemplateLiteral:
        '`' (BacktickCharacter | embeddedExpression)* '`'
        ;

    BacktickCharacter:
        ~[`\\\r\n\]
        | '\\' EscapeSequence
        | LineContinuation
        ;

See :ref:`String Interpolation Expressions` for the grammar of *embeddedExpression*.

An example of a multi-line string is below:

.. code-block:: typescript
   :linenos:

    let sentence = `This is an example of
                    a multi-line string, 
                    which should be enclosed in 
                    backticks`

Template literals are of type *string*, which is a predefined reference
type (see :ref:`string Type`).

|

.. _Null Literal:

Null Literal
========================

.. meta:
    frontend_status: Partly

There is only one literal (called *null literal*) that denotes a reference
without pointing at any entity. It is represented by the keyword ``null``. 

.. code-block:: abnf

    NullLiteral:
        'null' 
        ;

The null literal denotes the null reference which represents an absence
of a value, and is a valid value only for types *T* | ``null``, (see
:ref:`Nullish Types`). The null literal is of type *null* (see
:ref:`null Type`) and is, by definition, the only value of this type.

.. index::
   null literal
   null reference
   nullish type
   null type

|

.. _Undefined Literal:

Undefined Literal
========================

There is only one literal (called *undefined literal*) that denotes a reference
with a value that is not defined. It is the only value of type *undefined*
(see :ref:`undefined Type`), and is represented by the keyword ``undefined``.

.. code-block:: abnf

    UndefinedLiteral:
        'undefined'
        ;

.. index::
   undefined literal
   undefined type
   keyword

|

.. _Comments:

Comments
********

.. meta:
    frontend_status: Partly
    todo: Q: "Comments may be nested" - do we need nested multiline comments? It is not yet supported

A piece of text added in the stream to document and compliment the source
code is a *Comment*. Comments are insignificant for the syntactic grammar.

*Line comments* start with the character sequence '//' and stop at the end of
the line.

*Multi-line comments* start with the character sequence '/\*' and stop with
the first subsequent character sequence '\*/'.

Comments can be nested.

.. index::
   comment
   syntactic grammar
   multi-line comment
   nested comment

|

.. _Semicolons:

Semicolons
**********

.. meta:
    frontend_status: Done

In most cases declarations and statements are terminated by a line separator
(see :ref:`Line Separators`). In some cases a semicolon must be used to separate
syntax productions written in one line, or to avoid ambiguity.

.. index::
   declaration
   statement
   line separator
   syntax production

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


