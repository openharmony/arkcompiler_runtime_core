..
    Copyright (c) 2021-2025 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _Names, Declarations and Scopes:

Names, Declarations and Scopes
##############################

.. meta:
    frontend_status: Done

This chapter introduces the following three mutually-related notions:

-  Names,
-  Declarations, and
-  Scopes.

Each entity in an |LANG| program---a variable, a constant, a class,
a type, a function, a method, etc.---is introduced via a *declaration*.
An entity declaration defines a *name* of the entity. The name is used to
refer to the entity further in the program text. The declaration binds the
entity name with the *scope* (see :ref:`Scopes`). The scope affects the
accessibility of a new entity, and how it can be referred to by its qualified
or simple (unqualified) name.

.. index::
   variable
   constant
   class
   type
   function
   method
   scope
   accessibility
   declaration
   entity
   simple name
   unqualified name
   qualified name
   name

|

.. _Names:

Names
*****

.. meta:
    frontend_status: Done

A name is a sequence of one or more identifiers. A name allows referring to
any declared entity. Names can have two syntactical forms:

    - *Simple name* that consists of a single identifier;
    - *Qualified name* that consists of a sequence of identifiers with the
      token '``.``' as separator.

Both situations are covered by the below syntax rule:

.. code-block:: abnf

    qualifiedName:
      identifier ('.' identifier )*
      ;

In a qualified name *N.x* (where *N* is a simple name, and ``x`` is an
identifier that can follow a sequence of identifiers separated with '``.``'
tokens), *N* can name the following:

-  Name of a compilation unit (see :ref:`Compilation Units`) that is introduced
   as a result of ``import * as N`` (see :ref:`Bind All with Qualified Access`)
   with ``x`` to name the exported entity;

-  A class or interface type (see :ref:`Classes`, :ref:`Interfaces`) with ``x``
   to name its static member;

-  A class or interface type variable with ``x`` to name its instance member.

.. index::
   name
   entity
   simple name
   compilation unit
   qualified access
   exported entity
   interface type variable
   interface type
   interface
   class
   static member
   qualified name
   identifier
   package member
   reference type
   package
   variable
   field
   method
   token
   separator
   instance member

|

.. _Declarations:

Declarations
************

.. meta:
    frontend_status: Done

A declaration introduces a named entity in an appropriate *declaration scope*
(see :ref:`Scopes`).

.. index::
   named entity
   declared entity
   declaration scope

|

.. _Distinguishable Declarations:

Distinguishable Declarations
****************************

.. meta:
    frontend_status: Done

Each declaration in the declaration scope must be *distinguishable*.

A :index:`compile-time error` occurs otherwise.

Declarations are *distinguishable* if they have:

-  Different names,
-  Different signatures (see :ref:`Declaration Distinguishable by Signatures`).

.. index::
   distinguishable declaration
   declaration scope
   name
   signature

The examples below represent declarations distinguishable by names:

.. code-block:: typescript
   :linenos:

    const PI = 3.14
    const pi = 3
    function Pi() {}
    type IP = number[]
    class A {
        static method() {}
        method() {}
        field: number = PI
        static field: number = PI + pi
    }

If a declaration is not distinguishable by name (except a valid overloading as
in :ref:`Function, Method and Constructor Overloading` and
:ref:`Declaration Distinguishable by Signatures`), then a
:index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    // compile-time error: The constant and the function have the same name.
    const PI = 3.14
    function PI() { return 3.14 }

    // compile-time error: The type and the variable have the same name.
    class Person {}
    let Person: Person

    // compile-time error: The field and the method have the same name.
    class C {
        counter: number
        counter(): number {
          return this.counter
        }
    }

    /* compile-time error: Name of the declaration clashes with the predefined
        type or standard library entity name. */
    let number: number = 1
    let String = true
    function Record () {}
    interface Object {}
    let Array = 42

    // Functions have the same name but are distinguishable by signatures
    function foo() {}
    function foo(p: number) {}

.. index::
   distinguishable declaration
   overloading
   distinguishable functions

|

.. _Scopes:

Scopes
******

.. meta:
    frontend_status: Done

Different entity declarations introduce new names in different *scopes*. Scope
(see :ref:`Scopes`) is the region of program text where an entity is declared,
along with other regions it can be used in. The following entities are always
referred to by their qualified names only:

- Class and interface members (both static and instance ones);
- Entities imported via qualified import; and
- Entities declared in namespaces (see :ref:`Namespace Declarations`).

Other entities are referred to by their simple (unqualified) names.

Entities within the scope are accessible (see :ref:`Accessible`).

.. index::
   scope
   entity
   entity declaration
   class member
   interface member
   class
   member
   static member
   instance member
   qualified name
   qualified import
   namespace
   namespace declaration
   simple name
   access
   simple name
   unqualified name
   accessible scope
   variable
   constant
   function call
   accessibility

The scope of an entity depends on the context the entity is declared in:

.. _package-access:

-  Name declared on the package level (*package level scope*) is accessible
   (see :ref:`Accessible`) throughout the entire package. The name can be
   accessed (see :ref:`Accessible`) in other packages or modules if exported.

.. index::
   name declaration
   package level scope
   module level scope
   access
   module
   package
   module

.. _module-access:

-  *Module level scope* is applicable to separate modules only. A name
   declared on the module level is accessible (see :ref:`Accessible`)
   throughout the entire module. If exported, a name can be accessed in other
   compilation units.

.. _namespace-access:

.. meta:
    frontend_status: Done

-  *Namespace level scope* is applicable to namespaces only. A name
   declared in a namespace is accessible (see :ref:`Accessible`)
   throughout the entire namespace and in all embedded namespaces. If exported,
   a name can be accessed outside the namespace with mandatory namespace name
   qualification.

.. index::
   module level scope
   module
   access
   name
   declaration
   compilation unit
   namespace
   namespace level scope

.. _class-access:

-  A name declared inside a class (*class level scope*) is accessible (see
   :ref:`Accessible`) in the class and sometimes, depending on the access
   modifier (see :ref:`Access Modifiers`), outside the class, or by means of a
   derived class.

   Access to names inside the class is qualified with one of the following:

   -  Keywords ``this`` or ``super``;
   -  Class instance expression for the names of instance entities; or
   -  Name of the class for static entities.

   Outside access is qualified with one of the following:

   -  The expression the value stores;
   -  A reference to the class instance for the names of instance entities; or
   -  Name of the class for static entities.

.. index::
   class level scope
   accessibility
   access modifier
   keyword super
   keyword this
   expression
   value
   method
   name
   access
   modifier
   derived class
   declaration
   class instance
   instance entity
   static entity

.. _interface-access:

-  A name declared inside an interface (*interface level scope*) is accessible
   (see :ref:`Accessible`) inside and outside that interface (default
   ``public``).

.. index::
   name
   declaration
   class level scope
   interface level scope
   interface
   access

.. _class-or-interface-type-parameter-access:

-  *The scope of a type parameter* name in a class or interface declaration
   is that entire declaration, excluding static member declarations.

.. index::
   name
   declaration
   static member
   static member declaration
   scope
   type parameter

.. _function-type-parameter-access:

-  The scope of a type parameter name in a function declaration is that
   entire declaration (*function parameter scope*).

.. index::
   parameter name
   function declaration
   function parameter scope
   scope

.. _function-access:

-  The scope of a name declared immediately inside the body of a function
   or a method declaration is the body of that declaration from the point of
   declaration and up to the end of the body (*method* or *function scope*).
   This scope is also applied to function or method parameter names.

.. index::
   scope
   function body declaration
   method body declaration
   method scope
   function scope
   method parameter name

.. _block-access:

-  The scope of a name declared inside a statement block is the body of
   the statement block from the point of declaration and up to the end
   of the block (*block scope*).

.. index::
   statement block
   body
   point of declaration
   block scope

.. code-block:: typescript
   :linenos:

    function foo() {
        let x = y // compile-time error - y is not accessible yet
        let y = 1
    }

Scopes of two names can overlap (e.g., when statements are nested). If scopes
of two names overlap, then:

-  The innermost declaration takes precedence; and
-  Access to the outer name is not possible.

Class, interface, and enum members can only be accessed by applying the dot
operator '``.``' to an instance. Accessing them otherwise is not possible.

.. index::
   name
   scope
   overlap
   nested statement
   innermost declaration
   precedence
   access
   class member
   interface member
   enum member
   instance
   dot operator

|

.. _Accessible:

Accessible
**********

.. meta:
    frontend_status: Done

Entity is considered accessible if it belongs to the current scope (see
:ref:`Scopes`) and means that its name can be used for different purposes as
follows:

- Type name is used to declare variables, constants, parameters, class fields,
  or interface properties;
- Function or method name is used to call the function or method;
- Variable name is used to read or change the value of the variable;
- Name of a compilation unit introduced as a result of import with Bind All with
  Qualified Access (see :ref:`Bind All with Qualified Access`) is used to deal
  with exported entities.

.. index::
   accessible entity
   accessibility
   scope
   function name
   method name
   value
   compilation unit
   qualified access
   import
   bind all
   entity
   export
   exported entity

|

.. _Type Declarations:

Type Declarations
*****************

.. meta:
    frontend_status: Done

An interface declaration (see :ref:`Interfaces`), a class declaration (see
:ref:`Classes`), an enum declaration (see :ref:`Enumerations`), or a type alias
(see :ref:`Type Alias Declaration`) are type declarations.

The syntax of *type declration* is presented below:

.. code-block:: abnf

    typeDeclaration:
        classDeclaration
        | interfaceDeclaration
        | enumDeclaration
        | typeAlias
        ;

.. index::
   type declaration
   interface declaration
   class declaration
   enum declaration
   alias
   type alias
   type declaration

|

.. _Type Alias Declaration:

Type Alias Declaration
======================

.. meta:
    frontend_status: Done

Type aliases enable using meaningful and concise notations by providing the
following:

-  Names for anonymous types (array, function, and union types); or
-  Alternative names for existing types.

Scopes of type aliases are package, module, or namespace level scopes. Names of
all type aliases must follow the uniqueness rules of
:ref:`Distinguishable Declarations` in the current context.

.. index::
   type alias
   anonymous type
   array
   distinguishable declaration
   function
   union type
   scope
   context
   alias
   name

The syntax of *type alias* is presented below:

.. code-block:: abnf

    typeAlias:
        'type' identifier typeParameters? '=' type
        ;

Meaningful names can be provided for anonymous types as follows:

.. code-block:: typescript
   :linenos:

    type Matrix = number[][]
    type Handler = (s: string, no: number) => string
    type Predicate<T> = (x: T) => boolean
    type NullableNumber = number | null

If the existing type name is too long, then a shorter new name can be
introduced by using type alias (particularly for a generic type).

.. code-block:: typescript
   :linenos:

    type Dictionary = Map<string, string>
    type MapOfString<T> = Map<T, string>

A type alias acts as a new name only. It neither changes the original type
meaning nor introduces a new type.

.. code-block:: typescript
   :linenos:

    type Vector = number[]
    function max(x: Vector): number {
        let m = x[0]
        for (let v of x)
            if (v > m) v = m
        return m
    }

    function main() {
        let x: Vector = [3, 2, 1]
        console.log(max(x)) // ok
    }

.. index::
   alias
   anonymous type
   name
   type alias
   name
   generic type

Type aliases can be recursively referenced inside the right-hand side of a type
alias declaration.

In a type alias defined as ``type A = something``, *A* can be used recursively
if it is one of the following:

-  Array element type: ``type A = A[]``; or
-  Type argument of a generic type: ``type A = C<A>``.

.. code-block:: typescript
   :linenos:

    type A = A[] // ok, used as element type

    class C<T> { /*body*/}
    type B = C<B> // ok, used as a type argument

    type D = string | Array<D> // ok

Any other use causes a :index:`compile-time error`, because the compiler
does not have enough information about the defined alias:

.. code-block:: typescript
   :linenos:

    type E = E // compile-time error
    type F = string | E // compile-time error

.. index::
   type alias
   alias
   recursive reference
   type alias declaration
   array element
   type argument
   generic type
   compiler

The same rules apply to a generic type alias defined as
``type A<T> = something``:

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    type A<T> = Array<A<T>> // ok, A<T> is used as a type argument
    type A<T> = string | Array<A<T>> // ok

    type A<T> = A<T> // compile-time error

A :index:`compile-time error` occurs if a generic type alias is used without
a type argument:

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    type A<T> = Array<A> // compile-time error

**Note**. There is no restriction on using a type parameter *T* in
the right side of a type alias declaration. The following code
is valid:

.. code-block:: typescript
   :linenos:

    type NodeValue<T> = T | Array<T> | Array<NodeValue<T>>;

.. index::
   alias
   generic type
   type argument
   type alias
   type parameter

|

.. _Variable and Constant Declarations:

Variable and Constant Declarations
**********************************

.. meta:
    frontend_status: Done

.. _Variable Declarations:

Variable Declarations
=====================

.. meta:
    frontend_status: Partly
    todo: arrays never have default values 
    todo: raise error for non initialized arrays: let x: number[];console.log(x)
    todo: fix grammar change - ident '?' is not allowed, readonly is not here

A non-ambient *variable declaration* introduces a new variable which is in fact
a named storage location. A declared variable must be assigned an initial value
before the first usage. The initial value is assigned either as a part of the
declaration or in various forms via initialization.

The syntax of *variable declarations* is presented below:

.. code-block:: abnf

    variableDeclarations:
        'let' variableDeclarationList
        ;

    variableDeclarationList:
        variableDeclaration (',' variableDeclaration)*
        ;

    variableDeclaration:
        identifier ':' type initializer?
        | identifier initializer
        ;

    initializer:
        '=' expression
        ;

When a variable is introduced by a variable declaration, type ``T`` of the
variable is determined as follows:

-  ``T`` is the type specified in a type annotation (if any) of the declaration.

   - If the declaration also has an initializer, then the initializer expression
     type must be assignable to ``T`` (see :ref:`Assignability with Initializer`).

-  If no type annotation is available, then ``T`` is inferred from the
   initializer expression (see :ref:`Type Inference from Initializer`).

An ambient variable declaration must have *type* but has no *initializer*.
Otherwise, a :index:`compile-time error` occurs.


.. index::
   variable declaration
   declaration
   name
   named store location
   variable
   type annotation
   initialization
   initializer expression
   assignability
   inference
   annotation
   inference
   variable declaration
   value
   declaration

.. code-block:: typescript
   :linenos:

    let a: number // ok
    let b = 1 // ok, type 'int' is inferred
    let c: number = 6, d = 1, e = "hello" // ok

    // ok, type of lambda and type of 'f' can be inferred
    let f = (p: number) => b + p
    let x // compile-time error -- either type or initializer

Every variable in a program must have an initial value before it can be used:

- If the *initializer* of a variable is specified explicitly, then its
  execution produces the initial value for this variable.

- Otherwise, the following situations are possible:

   + If the type of a variable is ``T``, and ``T`` has a *default value*
     (see :ref:`Default Values for Types`), then the variable is initialized
     with the default value.
   + If a variable has no default value, then a value must be set by the
     :ref:`Simple Assignment Operator` before attempting to use the variable.

.. index::
   value
   method parameter
   function parameter
   method
   default value
   assignment operator
   assignment
   function
   initializer
   initialization
   cyclic dependency
   variable
   initializer expression
   non-initialized variable

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

   let a = b // a uses b for its initialization
   let b = a // b uses a for its initialization

   class A {
     a = this.b // a uses b for its initialization
     b = this.a // b uses a for its initialization
   }

|

.. _Constant Declarations:

Constant Declarations
=====================

.. meta:
    frontend_status: Done

A *constant declaration* introduces a named variable with a mandatory
explicit value. The value of a constant cannot be changed by an assignment
expression (see :ref:`Assignment`). If the constant is an object or array, then
object fields or array elements can be modified.

The syntax of *constant declarations* is presented below:

.. code-block:: abnf

    constantDeclarations:
        'const' constantDeclarationList
        ;

    constantDeclarationList:
        constantDeclaration (',' constantDeclaration)*
        ;

    constantDeclaration:
        identifier (':' type)? initializer
        ;

If a constant declaration belongs to the package (see :ref:`Packages`), then its
syntax and semantics can be slightly extended (see :ref:`Constants in packages`).

The type ``T`` of a constant declaration is determined as follows:

-  If ``T`` is the type specified in a type annotation (if any) of the
   declaration, then the initializer expression must be assignable to
   ``T`` (see :ref:`Assignability with Initializer`).
-  If no type annotation is available, then ``T`` is inferred from the
   initializer expression (see :ref:`Type Inference from Initializer`).
-  If '``?``' is used after the name of the constant, then the type of the
   constant is ``T | undefined``, regardless of whether ``T`` is identified
   explicitly or via type inference.

.. index::
   constant declaration
   variable
   constant
   value
   assignment expression
   object
   array
   type
   constant declaration
   type annotation
   inferred type
   initializer expression
   type inference

.. code-block:: typescript
   :linenos:

    const a: number = 1 // ok
    const b = 1 // ok, int type is inferred
    const c: number = 1, d = 2, e = "hello" // ok
    const x // compile-time error -- initializer is mandatory
    const y: number // compile-time error -- initializer is mandatory

|

.. _Assignability with Initializer:

Assignability with Initializer
==============================

.. meta:
    frontend_status: Done

If a variable or constant declaration contains type annotation ``T`` and
initializer expression *E*, then the type of *E* must be assignable to ``T``
(see :ref:`Assignability`).

.. index::
   initializer expression
   assignment-like contexts
   annotation
   constant declaration
   type
   assignability
   type annotation

|

.. _Type Inference from Initializer:

Type Inference from Initializer
===============================

.. meta:
    frontend_status: Done

The type of a declaration that contains no explicit type annotation is inferred
from the initializer expression as follows:

-  In a variable declaration (not in a constant declaration, though), if the
   initializer expression is of a literal type, then the literal type is
   replaced for its supertype, if any (see :ref:`Subtyping for Literal Types`).
   If the initializer expression is of a union type that contains literal types,
   then each literal type is replaced for its supertype (see
   :ref:`Subtyping for Literal Types`), and then normalized (see
   :ref:`Union Types Normalization`).

-  Otherwise, the type of a declaration is inferred from the initializer
   expression.

If the type of the initializer expression cannot be inferred, then a
:index:`compile-time error` occurs (see :ref:`Object Literal`):

.. index::
   type
   declaration
   annotation
   type inference
   initializer
   subtyping
   supertype
   normalization
   type inference
   inferred type
   type annotation
   initializer expression
   literal type

.. code-block:: typescript
   :linenos:

    let a = null                // type of 'a' is null
    let aa = undefined          // type of 'aa' is undefined
    let arr = [null, undefined] // type of 'arr' is (null | undefined)[]

    let cond: boolean = /*some initialization*/

    let b = cond ? 1 : 2         // type of 'b' is int
    let c = cond ? 3 : 3.14      // type of 'c' is double
    let d = cond ? "one" : "two" // type of 'd' is string
    let e = cond ? 1 : "one"     // type of 'e' is int | string

    const bb = cond ? 1 : 2         // type of 'bb' is int
    const cc = cond ? 3 : 3.14      // type of 'cc' is double
    const dd = cond ? "one" : "two" // type of 'dd' is "one" | "two"
    const ee = cond ? 1 : "one"     // type of 'ee' is int | "one"

    let f = {name: "aa"} // compile-time error: type unknown

    declare let   x1 = 1 // compile-time error: ambient variable cannot have initializer
    declare const x2 = 1 // type of 'x2' is int
    let           x3 = 1 // type of 'x3' is int
    const         x4 = 1 // type of 'x4' is int

    declare let   s1 = "1" // compile-time error: ambient variable cannot have initializer
    declare const s2 = "1" // type of 's2' is "1"
    let           s3 = "1" // type of 's3' is string
    const         s4 = "1" // type of 's4' is "1"


|

.. _Function Declarations:

Function Declarations
*********************

.. meta:
    frontend_status: Done

*Function declarations* specify names, signatures, and bodies when
introducing *named functions*. An optional function body is a block
(see :ref:`Block`).

The syntax of *function declarations* is presented below:

.. code-block:: abnf

    functionDeclaration:
        modifiers? 'function' identifier
        typeParameters? signature block?
        ;

    modifiers:
        'native' | 'async'
        ;

Functions must be declared on the top level (see :ref:`Top-Level Statements`).

If a function is declared *generic* (see :ref:`Generics`), then its type
parameters must be specified.

The modifier ``native`` indicates that the function is a *native function* (see
:ref:`Native Functions` in Experimental Features). If a *native function* has a
body, then a :index:`compile-time error` occurs.

Functions with the modifier ``async`` are discussed in :ref:`Async Functions`.

.. index::
   function declaration
   name
   signature
   named function
   function body
   block
   body
   function call
   native function
   generic function
   type parameter
   top-level statement

|

.. _Signatures:

Signatures
==========

.. meta:
    frontend_status: Done

A signature defines parameters and the return type (see :ref:`Return Type`)
of a function, method, or constructor.

The syntax of *signature* is presented below:

.. code-block:: abnf

    signature:
        '(' parameterList? ')' returnType?
        ;

Overloading (see :ref:`Function, Method and Constructor Overloading`) is supported for
functions, methods and constructors. The signatures of overloaded entities are important
for their unique identification.

.. index::
   signature
   parameter
   return type
   function
   method
   constructor
   function overloading
   method overloading
   function overloading
   overloaded entity
   identification

|

.. _Parameter List:

Parameter List
==============

.. meta:
    frontend_status: Partly
    todo: change parser as grammar rules, are changed - rest can be after optional, annotation for rest

A signature may contain a *parameter list* that specifies an identifier of
each parameter name, and the type of each parameter. The type of each
parameter must be defined explicitly. If the *parameter list* is omitted, then
the function or the method has no parameters.

The syntax of *parameter list* is presented below:

.. code-block:: abnf

    parameterList:
        parameter (',' parameter)* (',' restParameter)? ','?
        | restParameter ','?
        ;

    parameter:
        annotationUsage? (requiredParameter | optionalParameter)
        ;

    requiredParameter:
        identifier ':' type
        ;

If a parameter is *required*, then each function or method call must contain
an argument corresponding to that parameter.
The function below has *required parameters*:

.. code-block:: typescript
   :linenos:

    function power(base: number, exponent: number): number {
      return Math.pow(base, exponent)
    }
    power(2, 3) // both arguments are required in the call

Several parameters can be *optional*, allowing to omit
corresponding arguments in a call (see :ref:`Optional Parameters`).

A :index:`compile-time error` occurs if an *optional parameter* precedes a
*required parameter* in the parameter list.

The last parameter of a function or a method can be a single *rest parameter*
(see :ref:`Rest Parameter`).

.. index::
   signature
   parameter list
   identifier
   parameter name
   type
   function
   method
   method call
   parameter
   rest parameter
   argument
   required parameter
   optional parameter
   prefix readonly

If a parameter type is prefixed with ``readonly``, then there are additional
restrictions on the parameter as described in :ref:`Readonly Parameters`.

|

.. _Readonly Parameters:

Readonly Parameters
===================

.. meta:
    frontend_status: Done

If the parameter type is ``readonly`` array or tuple type, then 
no assignment and no function or method call can modify
elements of this array or tuple.
Otherwise, a :index:`compile-time error` occurs:

.. code-block:: typescript
   :linenos:

    function foo(array: readonly number[], tuple: readonly [number, string]) {
        let element = array[0] // OK, one can get array element
        array[0] = element // compile-time error, array is readonly

        element = tuple[0] // OK, one can get tuple element
        tuple[0] = element // compile-time error, tuple is readonly
    }

Any assignment of readonly parameters and variables must follow the limitations
stated in :ref:`Type of Expression`.

.. index::
   readonly parameter
   parameter type
   prefix readonly
   array type
   tuple type
   array
   assignment
   conversion

|

.. _Optional Parameters:

Optional Parameters
===================

.. meta:
    frontend_status: Done

*Optional parameters* can be of two forms as follows:

.. code-block:: abnf

    optionalParameter:
        identifier ':' type '=' expression
        | identifier '?' ':' type
        ;

The first form contains an expression that specifies a *default value*. It is
called a *parameter with default value*. The value of the parameter is set
to the *default value* if the argument corresponding to that parameter is
omitted in a function or method call:

.. index::
   optional parameter
   expression
   default value
   parameter with default value
   argument
   function call
   method call

.. code-block:: typescript
   :linenos:

    function pair(x: number, y: number = 7)
    {
        console.log(x, y)
    }
    pair(1, 2) // prints: 1 2
    pair(1) // prints: 1 7

The second form is a short-cut notation and ``identifier '?' ':' type``
effectively means that ``identifier`` has type ``T | undefined`` with the
default value ``undefined``.

.. index::
   notation
   undefined
   default value
   identifier

For example, the following two functions can be used in the same way:

.. code-block:: typescript
   :linenos:

    function hello1(name: string | undefined = undefined) {}
    function hello2(name?: string) {}

    hello1() // 'name' has 'undefined' value
    hello1("John") // 'name' has a string value
    hello2() // 'name' has 'undefined' value
    hello2("John") // 'name' has a string value

    function foo1 (p?: number) {}
    function foo2 (p: number | undefined = undefined) {}

    foo1()  // 'p' has 'undefined' value
    foo1(5) // 'p' has a numeric value
    foo2()  // 'p' has 'undefined' value
    foo2(5) // 'p' has a numeric value

|

.. _Rest Parameter:

Rest Parameter
==============

.. meta:
    frontend_status: Done

*Rest parameters* allow functions, methods, constructors, or lambdas to take
arbitrary numbers of arguments. *Rest parameters* have the ``spread`` operator
'``...``' as a prefix before the parameter name.

The syntax of *rest parameter* is presented below:

.. code-block:: abnf

    restParameter:
        annotationUsage? '...' identifier ':' type
        ;

A :index:`compile-time error` occurs if a rest parameter:

-  Is not the last parameter in a parameter list;
-  Has a type that is neither an array type nor a tuple type.

A call of entity with a rest parameter of array type ``T[]``
(or ``FixedArray<T>``) can accept any number of arguments
of types that are assignable (see :ref:`Assignability`) to ``T``:

.. index::
   rest parameter
   function
   method
   parameter name
   rest parameter
   array type
   parameter list
   array type
   tuple type
   assignability
   lambda
   constructor
   argument
   prefix

.. code-block:: typescript
   :linenos:

    function sum(...numbers: number[]): number {
      let res = 0
      for (let n of numbers)
        res += n
      return res
    }

    sum() // returns 0
    sum(1) // returns 1
    sum(1, 2, 3) // returns 6

If an argument of array type ``T[]`` is to be passed to a call of entity
with the rest parameter, then the spread expression (see
:ref:`Spread Expression`) must be used with the ``spread`` operator '``...``'
as a prefix before the array argument:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    function sum(...numbers: number[]): number {
      let res = 0
      for (let n of numbers)
        res += n
      return res
    }

    let x: number[] = [1, 2, 3]
    sum(...x) // spread an array 'x'
       // returns 6

.. index::
   argument
   prefix
   spread operator
   function
   method
   array argument
   array type
   entity
   spread expression

A call of entity with a rest parameter of tuple type
[``T``:sub:`1` ``, ..., T``:sub:`n`] can accept only ``n`` arguments of types
that are assignable (see :ref:`Assignability`) to the corresponding
``T``:sub:`i`:

.. index::
   rest parameter
   function
   lambda
   assignability
   method
   parameter name
   tuple type
   parameter list
   type
   argument

.. code-block:: typescript
   :linenos:

    function sum(...numbers: [number, number, number]): number {
      return numbers[0] + numbers[1] + numbers[2]
    }

    sum()          // compile-time error: wrong number of arguments, 0 instead of 3
    sum(1)         // compile-time error: wrong number of arguments, 1 instead of 3
    sum(1, 2, "a") // compile-time error: wrong type of the 3rd argument
    sum(1, 2, 3)   // returns 6

If an argument of tuple type [``T``:sub:`1` ``, ..., T``:sub:`n`]
is to be passed to a call of entity with the rest parameter,
then a spread expression (see :ref:`Spread Expression`)
must have the ``spread`` operator '``...``' as a
prefix before the tuple argument:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    function sum(...numbers: [number, number, number]): number {
      return numbers[0] + numbers[1] + numbers[2]
    }

    let x: [number, number, number] = [1, 2, 3]
    sum(...x) // spread tuple 'x'
       // returns 6

.. index::
   argument
   prefix
   spread expression
   function
   rest parameter
   tuple argument
   spread operator

If an argument of fixed-size array type ``FixedArray<T>`` is to be passed to a
function or a method with the rest parameter, then the spread expression (see
:ref:`Spread Expression`) must be used with the ``spread`` operator '``...``'
as a prefix before the fixed-size array argument:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    function sum(...numbers: Array<number>): number {
      let res = 0
      for (let n of numbers)
        res += n
      return res
    }

    let x: FixedArray<number> = [1, 2, 3]
    sum(...x) // spread an fixed-size array 'x'
       // returns 6

.. index::
   argument
   fixed-size array type
   rest parameter
   prefix
   spread operator
   spread expression
   function
   method
   fixed-size array argument


|

.. _Shadowing by Parameter:

Shadowing by Parameter
======================

.. meta:
    frontend_status: Done

If the name of a parameter is identical to the name of a top-level variable
accessible (see :ref:`Accessible`) within the body of a function or a method
with that parameter, then the name of the parameter shadows the name of the
top-level variable within the body of that function or method:

.. code-block:: typescript
   :linenos:

    class T1 {}
    class T2 {}
    class T3 {}

    let variable: T1
    function foo (variable: T2) {
        // 'variable' has type T2 and refers to the function parameter
    }
    class SomeClass {
      method (variable: T3) {
        // 'variable' has type T3 and refers to the method parameter
      }
    }

.. index::
   shadowing
   parameter
   accessibility
   top-level variable
   access
   function body
   method body
   name
   function
   method
   function parameter
   method parameter

|

.. _Return Type:

Return Type
===========

.. meta:
    frontend_status: Done

Function or method return type defines the static type of the result of the
function or method execution (see :ref:`Function Call Expression` and
:ref:`Method Call Expression`). During the execution, the function or method
can produce a value of a type assignable (see :ref:`Assignability`) to the
return type.

The syntax of *return type* is presented below:

.. code-block:: abnf

    returnType:
        ':' (type | 'this')
        ;

If function or method return type is not ``void`` (see :ref:`Type void`), and
the execution path of the function or method body has no return statement (see
:ref:`Return Statements`), then a :index:`compile-time error` occurs.

A special form of return type with the keyword ``this`` as type annotation can
be used in class instance methods only (see :ref:`Methods Returning this`).

If function or method return type is not specified, then it is inferred from
its body (see :ref:`Return Type Inference`). If there is no body, then the
function or method return type is ``void`` (see :ref:`Type void`).

.. index::
   return type
   function
   method
   static type
   assignable type
   assignability
   return statement
   method body
   type void
   execution path
   return statement
   inferred type
   type inference

|

.. _Return Type Inference:

Return Type Inference
=====================

.. meta:
    frontend_status: Done

An omitted function or method return type can be inferred from the function,
or the method body. If the return type is omitted in a native function (see
:ref:`Native Functions`), then a :index:`compile-time error` occurs.

The current version of |LANG| allows inferring return types at least under
the following conditions:

-  If there is no return statement, or if all return statements have no
   expressions, then the return type is ``void`` (see :ref:`Type void`).
-  If there are *k* return statements (where *k* is 1 or more) with
   the same type expression *R*, then ``R`` is the return type.
-  If there are *k* return statements (where *k* is 2 or more) with
   expressions of types ``T``:sub:`1`, ``...``, ``T``:sub:`k`, then ``R`` is the
   *union type* (see :ref:`Union Types`) of these types (``T``:sub:`1` | ... |
   ``T``:sub:`k`), and its normalized version (see :ref:`Union Types Normalization`)
   is the return type.
-  If a function or a method is ``async`` (see :ref:`Async Functions and Methods`), 
   the return type is inferred by using the rules above, and the return type ``T``
   is not ``Promise``, then the return type is assumed to be ``Promise<T>``.

Future compiler implementations are to infer the return type in more cases.
The example below represents type inference:

.. index::
   return type
   function
   method
   function return type
   method return type
   native function
   type inference
   inferred type
   method body
   return statement
   normalization
   type expression
   normalization
   expression
   function
   implementation
   compiler
   union type

.. code-block:: typescript

    // Explicit return type
    function foo(): string { return "foo" }

    // Implicit return type inferred as string
    function goo() { return "goo" }

    class Base {}
    class Derived1 extends Base {}
    class Derived2 extends Base {}

    function bar (condition: boolean) {
        if (condition)
            return new Derived1()
        else
            return new Derived2()
    }
    // Return type of bar will be Derived1|Derived2 union type

    function boo (condition: boolean) {
        if (condition) return 1
    }
    // That is a compile-time error as there is an execution path with no return

If the compiler fails to recognize a particular type inference case, then
a corresponding :index:`compile-time error` occurs.

.. index::
   inference
   compiler
   type inference
   inferred type

|

.. _Overload Signatures:

Overload Signatures
*******************

.. meta:
    frontend_status: None

|LANG| allows specifying a function, a method and a constructor that can
have several *overload signatures* followed by one implementation body.

A call of an entity with overload signatures is always a call of the
implementation body. If the implementation body is missing, or does
not immediately follow the declaration, then a :index:`compile-time error`
occurs.

The example below has two overload signatures defined for a function:

.. index::
   function
   overload signature
   function header
   implementation function
   implementation

.. code-block:: typescript
   :linenos:

    function foo(): void           // 1st signature
    function foo(x: string): void  // 2nd signature
    function foo(x: Object|null|undefined): void // 3rd - implementation signature
    {
        console.log(x)
    }

    foo()          // ok, call fits 1st and 3rd signatures
    foo("aa")      // ok, call fits 2nd and 3rd signatures
    foo(undefined) // ok, call fits the 3rd signature

The call of ``foo()`` is executed as a call of the implementation function
with the ``undefined`` argument. The call of ``foo(x)`` is executed as a call
of the implementation function with the ``x`` argument.

.. _Function Overload Signatures:

Function Overload Signatures
============================

.. meta:
    frontend_status: None

The syntax of a *function with overload signatures*
is presented below (see also :ref:`Function Declarations`):

.. code-block:: abnf

    functionWithOverloadSignatures:
        functionOverloadSignature*
        functionDeclaration

    functionOverloadSignature:
      'async'? 'function' identifier typeParameters? signature
      ;

The semantic rules for *overload signatures* and its implementation body
are described in :ref:`Overload Signatures Correctness Check`.

If not all overload signatures are either exported or non-exported, then a
:index:`compile-time error` occurs.

.. index::
   call
   implementation function
   argument null
   argument undefined
   execution
   signature
   function
   implementation
   overload signature
   compatibility

|

.. _Class Method Overload Signatures:

Class Method Overload Signatures
================================

.. meta:
    frontend_status: None

The syntax of a *method with overload signatures*
is presented below (see also :ref:`Method Declarations`):

.. code-block:: abnf

    classMethodWithOverloadSignatures:
        methodOverloadSignature*
        classMethodDeclaration

    methodOverloadSignature:
        methodModifier* identifier signature
        ;

A :index:`compile-time error` occurs if the method implementation is not
present, or does not immediately follow the declaration.

The semantic rules for *overload signatures* and its implementation body
are described in :ref:`Overload Signatures Correctness Check`.


.. _Constructor Overload Signatures:

Constructor Overload Signature
================================

.. meta:
    frontend_status: None

The syntax of a *constructor with overload signatures*
is presented below (see also :ref:`Constructor Declaration`):

.. code-block:: abnf

    constructorWithOverloadSignatures:
        constructorOverloadSignature*
        constructorDeclaration:
        ;

    constructorOverloadSignature:
        accessModifier? 'constructor' signature
        ;

A :index:`compile-time error` occurs if at least two different overload
signatures or implementation signatures have different *access modifiers*.

The semantic rules for *overload signatures* and its implementation body
are described in :ref:`Overload Signatures Correctness Check`.

.. _Overload Signatures Correctness Check:

Overload Signatures Correctness Check
=====================================

TBD

|

.. raw:: pdf

   PageBreak
