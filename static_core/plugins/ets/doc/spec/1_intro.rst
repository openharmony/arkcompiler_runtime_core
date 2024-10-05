..
    Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

The |LANG| language combines and supports features that are in use in many
well-known programming languages, where these tools have already proven
helpful and powerful.

|LANG| supports imperative, object-oriented, functional, and generic
programming paradigms, and combines them safely and consistently.

At the same time, |LANG| does not support features that allow software
developers to write dangerous, unsafe, or inefficient code. In particular,
the language uses the strong static typing principle. It allows no dynamic
type changes, as object types are determined by their declarations. Their
semantic correctness is checked at compile time.

The following major aspects characterize the |LANG| language as a whole:

-  Object orientation

   The |LANG| language supports conventional class-based, *object-oriented
   approach* to programming (OOP). The major notions of this approach are
   as follows:

   -  Classes with single inheritance,
   -  Interfaces as abstractions to be implemented by classes, and
   -  Virtual functions (class members) with a dynamically dispatched
      overriding mechanism.


   Common in many (if not all) modern programming languages, object orientation
   enables powerful, flexible, safe, clear, and adequate software design.

.. index::
   object
   object orientation
   object-oriented
   OOP (object-oriented programming)
   inheritance
   overriding
   abstraction

-  Modularity

   The |LANG| language supports the *component programming approach*. It
   presumes that software is designed and implemented as a composition
   of *compilation units*. A compilation unit is typically represented as
   a *module* or as a *package*.

   A module in |LANG| is a standalone, independently compiled unit that
   combines various programming resources (types, classes, functions, and so
   on). A module can communicate with other modules by exporting all or some
   of its resources to, or importing from other modules.

   This feature provides a high level of software development process and
   software maintainability, supports flexible module reuse and efficient
   version control.

.. index::
   modularity
   compilation unit
   module
   package

-  Genericity

   Some program entities in |LANG| can be *type-parameterized*. This means that
   an entity can represent a very high-level (abstract) concept. Providing more
   concrete type information makes the entity instantiated for a particular use
   case.

   A classical illustration is the notion of a list that represents the
   ‘idea’ of an abstract data structure. This abstract notion can be turned
   into a concrete list by providing additional information (i.e., the type of
   list elements).

   Supported by many programming languages, a similar feature (*generics* or
   *templates*) serves as the basis of the generic programming paradigm. It
   enables making programs and program structures more generic and reusable.

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
   range of devices. The language ecosystem is a developer-friendly and uniform
   programming environment for a range of popular platforms (called
   *cross-platform development*). It can generate optimized applications
   capable of operating under the limitations of lightweight devices, or
   realizing the full potential of any specific target hardware.

.. index::
   multi-targeting

|LANG| is designed as a part of the modern language manifold. To provide an
efficient and safely executable code, the language takes flexibility and
power from |TS| and its predecessor JavaScript, and the static
typing principle from Java and Kotlin. The overall design keeps the |LANG|’
syntax style similar to that of those languages, and some of its important
constructs are almost identical to theirs on purpose.

In other words, there is a significant *common subset* of features of |LANG|
on the one hand, and of |TS|, JavaScript, Java, and Kotlin on the other.
Consequently, the |LANG|’ style and constructs are no puzzle for the |TS| and
Java users who can sense the meaning of most constructs of the new language
even if not understand them completely.

This stylistic and semantic similarity permits smoothly migrating the
applications originally written in |TS|, Java, or Kotlin to |LANG|.

Like its predecessors, |LANG| is a relatively high-level language. It means
that the language provides no access to low-level machine representations.
As a high-level language, |LANG| supports automatic storage management. It
means that dynamically created objects are deallocated automatically soon
after they are no longer available, and deallocating them explicitly is not
required.

|LANG| is not merely a language, but rather a comprehensive software
development ecosystem that facilitates the creation of software solutions
in various application domains.

The |LANG| ecosystem includes the language along with its compiler,
accompanying documents, guidelines, tutorials, the standard library
(see :ref:`Standard Library`), and a set of additional tools that perform
automatic or semi-automatic transition from other languages (currently,
|TS| and Java) to |LANG|.

.. index::
   object
   migration
   automatic transition

|

.. _Lexical and Syntactic Notation:

Lexical and Syntactic Notation
******************************

This section introduces the notation known as *context-free grammar*. It is
used in this specification to define the lexical and syntactic structure of
a program.

.. index::
   context-free grammar

The |LANG| lexical notation defines a set of productions (rules) that specify
the structure of the elementary language parts called *tokens*. All tokens are
defined in :ref:`Lexical Elements`. The set of tokens (identifiers, keywords,
numbers/numeric literals, operator signs, delimiters), special characters
(white spaces and line separators), and comments comprises the language’s
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

The tokens defined by the lexical grammar are terminal symbols of syntactic
notation. Syntactic notation defines a set of productions starting from the
goal symbol *compilationUnit* (see :ref:`Modules and Compilation Units`). It is
a sentence that consists of a single distinguished nonterminal, and describes
how sequences of tokens can form syntactically correct programs.

.. index::
   production
   nonterminal
   lexical grammar
   syntactic notation
   goal symbol
   compilation unit
   module
   nonterminal

Lexical and syntactic grammars are defined as a range of productions. Each
production:

- Is comprised of an abstract symbol (*nonterminal*) as its left-hand side,
  and a sequence of one or more *nonterminal* and *terminal* symbols as its
  *right-hand side*.
- Includes the '``:``' character as a separator between the left- and 
  right-hand sides, and the '``;``' character as the end marker.

.. index::
   lexical grammar
   syntactic grammar
   abstract symbol
   nonterminal symbol
   terminal symbol
   character
   separator
   end marker

Grammars draw terminal symbols from a fixed width form. Starting from the
goal symbol, grammars specify the language itself, i.e., the set of possible
sequences of terminal symbols that can result from repeatedly replacing
any nonterminal in the left-hand-side sequence for a right-hand side of the
production.

.. index::
   goal symbol
   nonterminal
   terminal symbol

Grammars can use the following additional symbols---sometimes called
*metasymbols*---in the right-hand side of a grammar production along
with terminal and nonterminal symbols:

-  Vertical line '``|``' to specify alternatives.

-  Question mark '``?``' to specify an optional (zero- or one-time) occurrence
   of the preceding terminal or nonterminal.

-  Asterisk '``*``' to mark a *terminal* or *nonterminal* that can occur zero
   or more times.

-  Parentheses '``(``' and '``)``' to enclose any sequence of terminals and/or
   nonterminals marked with the '``?``' or '``*``' metasymbols.

.. index::
   terminal
   terminal symbol
   nonterminal
   goal symbol
   metasymbol
   grammar production

Such additional symbols specify the structuring rules for terminal and
nonterminal sequences. However, they are not part of the terminal symbol
sequences that comprise the resultant program text.

The production below is an example that specifies a list of expressions:

.. code-block:: abnf

    expressionList:
      expression (',' expression)* ','?
      ;

This production introduces the following structure defined by the
nonterminal *expressionList*. The expression list must consist of a
sequence of *expression*\ s separated by the terminal ‘``,``’ symbol. The
sequence must have at least one *expression*. The list is optionally
terminated by the terminal ‘``,``’ symbol.

All grammar rules are presented in the Grammar section (see
:ref:`Grammar Summary`) of this specification.

.. index::
   terminal
   expression
   grammar rule


Terms and Definitions
*********************

This section contains the alphabetical list of important terms found in the
Specification, and their |LANG|-specific definitions. Such definitions are
not generic and can differ significantly from the definitions of same terms
as used in other languages, application areas, or industries.

.. glossary::
   :sorted:

   expression
     -- a formula for calculating values. An expression has the syntactic
     form that is a composition of operators and parentheses, where
     parentheses are used to change the order of calculation. By default,
     the order of calculation is determined by operator preferences.

   operator (in programming languages)
     -- the term can have several meanings.

     (1) a token that denotes the action to be performed on a value (addition,
     subtraction, comparison, etc.).
     
     (2) a syntactic construct that denotes an elementary calculation within
     an expression. Normally, an operator consists of an operator sign and
     one or more operands.

     In unary operators that have a single operand, the operator sign can be
     placed either in front of (*prefix* unary operator) or after an operand
     (*postfix* unary operator).

     If both operands are available, then the operator sign can be placed
     between the two (*infix* binary operator). A conditional operator with
     three operands is called *ternary*.

     Some operators have special notations. For example, the indexing
     operator, while formally being a binary operator, has a conventional
     form like a[i].

     Some languages treat operators as *syntactic sugar*---a conventional
     version of a more common construct, i.e., *function call*. Therefore,
     an operator like ``a+b`` is conceptually treated as the call ``+(a,b)``,
     where the operator sign plays the role of the function name, and the
     operands are function call arguments.

   operation sign
     -- a language token that signifies an operator and conventionally
     denotes a usual mathematical operator, for example, '``+``' for addition
     operator, '``/``' for division, etc. However, some languages allow using
     identifiers to denote operators, and/or arbitrarily combining characters
     that are not tokens in the alphabet of that language, i.e., operator
     signs.

   operand
     -- an argument of an operation. Syntactically, operands have the form of
     simple or qualified identifiers that refer to variables or members of
     structured objects. In turn, operands can be operators whose preferences
     ('priorities') are higher than the preference of the given operator.

   operation
     -- an informal notion that means an action or a process of operator
     evaluation.

   metasymbol
     -- additional symbols '``|``', '``?``', '``*``', '``(``', and '``)``' that
     can be used along with terminal and nonterminal symbols in the right-hand
     side of a grammar production.

   goal symbol
     -- a sentence that consists of a single distinguished nonterminal
     (*compilationUnit*). The *goal symbol* describes how sequences of
     tokens can form syntactically correct programs.

   token
     -- an elementary part of a programming language: identifier, keyword,
     operator and punctuator, or literal. Tokens are lexical input elements
     that form the vocabulary of a language, and can act as terminal symbols
     of the language's syntactic grammar.

   tokenization
     -- finding the longest sequence of characters that forms a valid token
     (i.e., establishing a token) in the process of codebase reading
     by the machine.

   punctuator
     -- a token that serves for separating, completing, or otherwise organizing
     program elements and parts: commas, semicolons, parentheses, square
     brackets, etc.

   literal
     -- a representation of a certain value type.

   comment
     -- a piece of text, insignificant for the syntactic grammar, that is
     added to the stream in order to document and compliment the source code.

   primitive type
      -- numeric value types, character, and boolean value types whose names
      are reserved, and cannot be used for user-defined type names.

   generic type
     -- a named type (class or interface) that has type parameters.

   generic
     -- see *generic type*.

   non-generic type
     -- a named type (class or interface) that has no type parameters.

   non-generic
     -- see *non-generic type*.

   type reference
     -- references that refer to named types by specifying their type names,
     and type arguments, where applicable, to be substituted for type
     parameters of the named type.

   nullable type 
     -- a variable declared to have the value ``null``, or ``type T | null``
     that can hold values of type ``T`` and its derived types.

   nullish value
     -- a reference which is null or undefined.

   simple name
     -- a name that consists of a single identifier.
     
   qualified name
     -- a name that consists of a sequence of identifiers separated with the
     token ‘``.``’.

   scope of a name
     -- a region of program code within which an entity---as declared by
     that name---can be accessed or referred to by its simple name without
     any qualification.

   function declaration
     -- a declaration that specifies names, signatures, and bodies when
     introducing a named function.

   terminal symbol
     -- a syntactically invariable token (i.e., a syntactic notation defined
     directly by an invariable form of the lexical grammar that defines a
     set of productions starting from the :term:`goal symbol`).

   terminal
     -- see *terminal symbol*.

   nonterminal symbol
     -- a syntactically variable token that results from the successive
     application of the production rules.

   context-free grammar
      -- grammar in which the left-hand side of each production rule consists
      of only a single nonterminal symbol.

   nonterminal
     -- see *nonterminal symbol*.

   keyword
     -- one of the *reserved words* that have their meanings permanently
     predefined in the language.

   variable
     -- see *variable declaration*.

   variable declaration
     -- a declaration that introduces a new named variable a modifiable
     initial value can be assigned to.

   constant
     -- see *constant declaration*.

   constant declaration
     -- a declaration that introduces a new variable to which an immutable
     initial value can be assigned only once at the time of instantiation.

   grammar
     -- a set of rules that describe what possible sequences of terminal and
     nonterminal symbols a programming language interprets as correct.

     Grammar is a range of productions. Each production comprises an
     abstract symbol (nonterminal) as its left-hand side, and a sequence
     of nonterminal and terminal symbols as its right-hand side.
     Each production has the character ‘``:``’ as a separator between the
     left- and right-hand sides, and the character ‘``;``’ as the end
     marker.

   production
     -- a sequence of terminal and nonterminal symbols that a programming
     language interprets as correct.

   white space
     -- one of lexical input elements that separate tokens from one another
     in order to improve the source code readability and avoid ambiguities.

   overload signature
      -- signatures that have several function (or method) headers with the same
      name and different signatures, and are followed by one implementation.

   widening conversion
     -- a conversion that causes no loss of information about the overall
     magnitude of a numeric value.

   narrowing conversion
     -- a conversion that can cause a loss information about the overall
     magnitude of a numeric value, and potentially a loss of precision
     and range.

   function types conversion
     -- conversion of one function type to another.

   casting conversion
     -- conversion of an operand of a cast expression to an explicitly
     specified type.

   method
     -- ordered 4-tuple consisting of type parameters, argument types,
     return type, and a ``throws``/``rethrows`` clause.

   abstract declaration
     -- ordinary interface method declaration that specifies the method’s name
     and signature.

   truthiness
     -- concept that extends the Boolean logic to operands and results
     of non-Boolean types, and allows handling the value of a valid
     expression of a non-void type as ``Truthy`` or ``Falsy``, depending on
     the kind of the value type.

   default ``catch`` clause
     -- ``catch`` clause that has its exception parameter type omitted, and can
     handle any exception or error that is not handled by a preceding clause.

   overloading
     -- situation where different methods inherited by or declared in the same
     class or interface have the same name but different signatures.

   shadowing
     -- situation where a derived class introduces a field with the same name as
     that of its base class.

   package level scope
      -- a name that is declared on the package level, and accessible throughout
      the entire package, and in other packages if exported.

   module level scope
     -- a name that is applicable for separate modules only. It is accessible
     throughout the entire module and in other packages if exported.

   class level scope
     -- a name declared inside a class. It is accessible inside and sometimes---by
     means of an access modifier, or via a derived class---outside  that class.
  
   interface level scope
     -- a name declared inside an interface. It is accessible inside and outside
     that interface.

   function parameter scope
     -- the scope of a type parameter name in a function declaration. It is
     identical to that entire declaration.

   method scope
     -- the scope of a name declared immediately inside the body of a method
     (function) declaration. Method scope is identical to the body of that
     method (function) declaration from the place of declaration, and up to
     the end of the body.

   function scope
     -- same as *method scope*.

   type parameter scope
     -- the name of a type parameter declared in a class or an interface. The
     type parameter scope is identical to the entire declaration (except static
     member declarations).

   static member
     -- a class member that is not related to a particular class 
     instance. A static member can be used across an entire program by using
     a qualified name notation (qualification is the name of a class).

   linearization
     -- de-nesting of all nested types in a union type to present them in
     the form of a flat line that has no more union types included.

   fit (into)
     -- belong, or be implicitly convertible (see :ref:`Widening Primitive Conversions`)
     to an entity.

   match (v.)
     -- correspond to an entity.

   own (adj.)
     -- of a member textually declared in a class, interface, type, etc., as
     opposed to members inherited from base class (superclass), base interfaces
     (superinterface), base type (supertype), etc.

   supercomponent (base component, parent component)
     -- a component from which another component is derived.

   subcomponent (derived component, child component)
     -- a component produced by, inherited from, and dependent from another
     component.


.. raw:: pdf

   PageBreak


