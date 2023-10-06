..  Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

Introduction
############

This document presents complete information on the new common-purpose,
multi-paradigm programming language called |LANG|.

|

.. _Common Description:

Common Description
******************

The |LANG| language combines and supports many features that have been in use in
many well-known programming languages, and have proven useful and powerful tools.

|LANG| supports multiple programming paradigms as imperative, object-oriented,
functional and generic approaches, and combines them safely and consistently.

At the same time, |LANG| does not support features that allow software
developers to write dangerous, unsafe and inefficient code. In particular,
the language uses the strong static typing principle that allows no dynamic
type changes as object types are determined by their declarations and checked
for semantic correctness at compile time.

The following major aspects characterize the |LANG| language as a whole:

-  Object orientation

   The |LANG| language supports conventional class-based, *object-oriented
   approach* to programming (OOP). The major notions of this approach are
   classes with single inheritance, interfaces as abstractions to be
   implemented by classes, and virtual functions (class members) with a
   dynamically dispatched overriding mechanism. Common in many (if not
   all) modern programming languages, object orientation enables
   powerful, flexible, safe, clear and adequate software design.

.. index::
   object
   object orientation
   object-oriented
   OOP (object-oriented programming)
   inheritance
   overriding
   abstraction

-  Modularity

   The |LANG| language supports the *component programming approach* which
   presumes that software is designed and implemented as a composition
   of modules or packages. A package in |LANG| is a standalone, independently
   compiled unit that combines various programming resources (types,
   classes, functions and so on). A package can communicate with other
   packages by exporting all or some of its resources to, or importing from
   other packages. This feature provides a high level of software
   development process and software maintainability, supports flexible
   module reuse and efficient version control.

.. index::
   modularity
   module
   package

-  Genericity

   Some program entities in |LANG| can be *type-parameterized*. This means that
   an entity can represent a very high-level (abstract) concept, and
   providing more concrete information makes it specialized for a
   particular use case. A classical illustration is the notion of a list
   that represents the ‘idea’ of an abstract data structure. Providing
   additional information (the type of list elements) can turn this
   abstract notion into a concrete list.

   Supported by many programming languages, a similar feature (‘generics’
   or ‘templates’) serves as the basis of the generic programming
   paradigm, and enables making programs and program structures more
   generic and reusable.

.. index::
   abstract concept
   abstract notion
   abstract data structure
   genericity
   type parameterization
   compile-time feature
   generics
   template

-  Multi-targeting

   |LANG| provides an efficient application development solution for a wide
   range of devices. The language ecosystem is a developer-friendly, uniform
   programming environment for a range of popular platforms (cross-platform
   development). It can generate optimized applications capable of operating
   under the limitations of lightweight devices, or realizing the full
   potential of any specific target hardware.

.. index::
   multi-targeting
   
|LANG| is designed as a part of the modern language manifold. To provide an
efficient and safely executable code, the language takes flexibility and
power from TypeScript and its predecessor JavaScript, and the static
typing principle from Java and Kotlin.

The |LANG|’ overall design keeps its syntax style similar to those languages,
and some of its important constructs are almost identical to theirs on purpose.

In other words, there is a significant *common subset* of features of |LANG|
on the one hand, and of TypeScript, JavaScript, Java and Kotlin on the other.
Consequently, the |LANG|’ style and constructs are no puzzle for the TypeScript
and Java users who can sense the meaning of most constructs of the new
language even if not understand them completely.

This stylistic and semantic similarity permits migrating the applications,
originally written in TypeScript, Java or Kotlin, smoothly to |LANG|.

Like its predecessors, |LANG| is a relatively high-level language. It means
that the language provides no access to low-level machine representations.
As a high-level language, |LANG| supports automatic storage management:
dynamically created objects are deallocated automatically soon after they
are no longer available, and deallocating them explicitly is not required.

|LANG| is not merely a language, but rather a comprehensive software
development ecosystem that facilitates the creation of software solutions
in various application domains.

The |LANG| ecosystem includes the language itself along with its compiler,
accompanying documents, guidelines, tutorials, the standard library
(see :ref:`Standard Library`), and a set of additional tools that perform
automatic or semi-automatic transition from other languages (currently,
TypeScript and Java) to |LANG|.

.. index::
   object
   migration
   automatic transition

|

.. _Lexical and Syntactic Notation:

Lexical and Syntactic Notation
******************************

This section introduces the notation (known as *context-free grammar*)
that this specification uses to define the lexical and syntactic
structure of a program.

.. index::
   context-free grammar

The |LANG| lexical notation defines a set of productions (rules) that specify
the structure of the elementary language parts called tokens. All tokens are
defined in :ref:`Lexical Elements`. The set of tokens (identifiers, keywords,
numbers/numeric literals, operator signs, delimiters), special characters
(white spaces and line separators) and comments comprises the language’s
*alphabet*.

.. index::
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

The tokens defined by the lexical grammar are terminal symbols of
the syntactic notation, which defines a set of productions starting from the
goal symbol *compilationUnit* (see :ref:`Modules and Compilation Units`)---a
sentence that consists of a single distinguished nonterminal, and
describes how sequences of tokens can form syntactically correct programs.

.. index::
   production
   nonterminal
   lexical grammar
   syntactic notation
   goal symbol
   compilation unit
   module
   nonterminal

Lexical and syntactic grammars are defined as a range of productions,
each comprised of an abstract symbol (*nonterminal*) as its *left-hand
side* and a sequence of one or more non-terminal and *terminal* symbols
as its *right-hand side*. Each production includes the ':' character as
a separator between the left-hand and the right-hand sides, and the ';'
character as its end marker.

.. index::
   lexical grammar
   syntactic grammar
   abstract symbol
   non-terminal symbol
   terminal symbol
   character
   separator
   end marker

Grammars draw the terminal symbols from a fixed width form. Starting from the
:index:`goal symbol`, grammars specify the language itself, i.e., the set of
possible sequences of terminal symbols that can result from repeatedly
replacing any nonterminal in the sequence for a right-hand side of the
production, to which that nonterminal is the left-hand side.

.. index::
   goal symbol
   nonterminal
   terminal symbol

Grammars can use the following additional symbols---sometimes called
metasymbols---in the right-hand side of a grammar production along
with terminal and non-terminal symbols:

-  Vertical line '\|' to specify alternatives.

-  Question mark '?' to specify the optional (zero- or one-time) appearance
   of the preceding terminal or non-terminal.

-  Asterisk '\*' to mark a *terminal* or *non-terminal* that can appear zero
   or more times.

-  Brackets '(' and ')' to enclose any sequence of terminals and/or
   nonterminals marked with the '?' or '\*' metasymbols.

.. index::
   terminal
   terminal symbol
   nonterminal
   non-terminal symbol
   goal symbol
   metasymbol
   grammar production

Such additional symbols specify the structuring rules for terminal and
non-terminal sequences, but are not part of the terminal symbol sequences
that comprise the resultant program text.

The production below is an example that specifies a list of expressions:

.. code-block:: abnf

    expressionList:
      expression (',' expression)* ','?
      ;

This production introduces the following structure defined by the
non-terminal ``expressionList``: the expression list must consist of the
sequence of *expression*\ s separated by the ‘,’ terminal symbol. The
sequence must have at least one *expression*, and the list is optionally
terminated by the ‘,’ terminal symbol.

All grammar rules are presented in the Grammar section of this specification.

.. index::
   terminal
   expression
   grammar rule


Terms and Definitions
*********************

This section contains the alphabetical list of important terms found in the
Specification with their |LANG|-specific definitions. Such definitions are
not generic and can differ significantly from the definitions of same terms
as used in other languages, application areas or industries.

.. glossary::
   :sorted:

   expression
     -- a formula for calculating values. An expression has the syntactic
     form that is a composition of operators and parentheses, where
     parentheses are used to change the order of calculation. By default,
     the order of calculation is determined by operator preferences.

   operator (in programming languages)
     -- a syntactic construct that denotes an elementary calculation within an
     expression. Normally, an operator consists of an operator sign and of
     one or more operands.

     In unary operators that have a single operand, the operator sign can be
     placed either in front of an operand (*prefix* unary operator), or after
     the operand (*postfix* unary operator). If two operands are available,
     then the operator sign can be placed between two operands (*infix*
     binary operator). A conditional operator with three operands is called
     *ternary*.


     Some operators have special notations; for example, the indexing
     operator, while formally being a binary operator, has a conventional
     form like a[i].


     Some languages treat operators as “syntactic sugar”---a conventional
     version of a more common construct, i.e., *function call*. Therefore,
     an operator like ``a+b`` is conceptually treated as the call ``+(a,b)``,
     where the operator sign plays the role of the function name, and the
     operands are function call arguments.

   operation sign
     -- a language token that signifies an operator and conventionally
     denotes a usual mathematical operator, for example, '+' for additional
     operator, '/' for division etc. However, some languages allow using
     identifiers to denote operators, and/or arbitrarily combining characters
     that are not tokens in the alphabet of that language, i.e., operator
     signs.

   operand
     -- an argument of an operation. Syntactically, operands have the form of
     simple or qualified identifiers that refer to variables or members of
     structured objects. In turn, operands can be operators whose preferences
     (“priorities”) are higher that the preference of the given operator.

   operation
     -- the informal notion that means an action, or a process of operator
     evaluation.

   metasymbol
     -- additional symbols '\|', '?', '\*', '(' and ')' that can be used
     along with terminal and non-terminal symbols in the right-hand side
     of a grammar production.

   goal symbol
     -- sentence that consists of a single distinguished nonterminal
     (*compilationUnit*) and describes how sequences of tokens can form
     syntactically correct programs.

   token
     -- an elementary part of a programming language: identifier, keyword,
     operator and punctuator, and literal. Tokens are lexical input elements
     that form the vocabulary of a language, and can act as terminal symbols
     of the language's syntactic grammar.

   tokenization
     -- the establishing of tokens in the process of codebase reading by
     a machine. The process of tokenization presumes finding the longest
     sequence of characters that form a valid token.

   terminal symbol
     -- a syntactically invariable token, i.e. a syntactic notation defined
     directly by the invariable form of the lexical grammar that defines a
     set of productions starting from the :term:`goal symbol`.

   terminal
     -- see *terminal symbol*.

   non-terminal symbol
     -- token that is syntactically variable and is the result of successive
     application of the production rules.

   context-free grammar
      -- grammar in which the left-hand side of each production rule consists
      of only a single nonterminal symbol.

   nonterminal
     -- see *non-terminal symbol*.

   variable
     -- see *variable declaration*.

   variable declaration
     -- declaration that introduces a new named variable to which a
     modifiable initial value can be assigned.

   constant
     -- see *constant declaration*.

   constant declaration
     -- declaration that introduces a new variable to which an inmutable
     initial value can be assigned only once at the time of instantiation.


.. raw:: pdf

   PageBreak


