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

.. _Names, Declarations and Scopes:

Names, Declarations and Scopes
##############################

.. meta:
    frontend_status: Partly

This chapter introduces three mutually-related notions: names,
declarations, and scopes.

Each entity in a |LANG| program---a variable, a constant, a class,
a type, a function, a method, etc.---is introduced via *declaration*.
An entity declaration assigns a *name* to the entity declared.
Further in the program text, the name is used to refer to the entity.

Each declaration is valid (i.e., available and known) only within its
*scope*. Informally, scope is the region of the program text where the
entity is declared, and where it can be referred to by its simple
(unqualified) name. See :ref:`Scopes` for more details.

.. index::
   variable
   constant
   class
   type
   function
   method
   scope
   declaration
   entity
   assignment
   simple name
   unqualified name

.. _Names:

Names
*****

.. meta:
    frontend_status: Done
    todo: A qualified name N.x may be used to refer to a member of package... If N names a package, then x is member of that package
    todo: Do we raeely want to support std.core.Double? If yes, it should be clariffied in 14.6 Import declaration section

A name refers to any declared entity.

*Simple* names consist of a single identifier; *qualified* names consist
of a sequence of identifiers with the '.' tokens as separators.

A qualified name *N.x* (where *N* is a simple or qualified name, and ``x``
is an identifier) can refer to a *member* of a package, or a reference type
(see :ref:`Reference Types`). *N* can name:

-  A package, of which ``x`` is a member.

-  A reference type, or a variable of a reference type (see
   :ref:`Reference Types`), and then ``x`` names a member (a field or a
   method) of that type.

.. index::
   name
   entity
   simple name
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

.. code-block:: abnf

    qualifiedName:
      (Identifier '.')* Identifier
      ;

|

.. _Declarations:

Declarations
************

.. meta:
    frontend_status: Done

Declarations introduce named entities in appropriate **declaration scope**
(see :ref:`Scopes`). Any declaration in its declaration scope must be
*distinguishable*.

.. index::
   named entity
   declared entity
   declaration scope

|

.. _Distinguishable Declarations:

Distinguishable Declarations
****************************

.. meta:
    frontend_status: Partly
    todo: const PI = 3.14;function PI():float{return 3.14};let a = PI(); error: TypeError: Unresolved reference PI
    todo: const PI:()=>int = ():int=>{return 5};function PI():float{return 3.14};let a = PI(); error: a is 5
    todo: need scpec clarification

Declarations must be distinguishable in a declaration scope. A
:index:`compile-time error` occurs otherwise.

Declarations are distinguishable if they:

-  Have different names;

-  Are distinguishable by signatures (see
   :ref:`Declaration Distinguishable by Signatures`).

.. index::
   distinguishable declaration
   declaration scope
   name
   signature

Example of declarations distinguishable by names:

.. code-block:: typescript
   :linenos:

    const PI = 3.14
    const pi = 3
    function Pi() {}
    type IP = number[]

Example of declarations that are indistinguishable by names, and cause a
:index:`compile-time error`:

.. code-block:: typescript
   :linenos:

    // The constant and the function have the same name.
    const PI = 3.14                   // compile-time error
    function PI() { return 3.14 }     // compile-time error

    // The type and the variable have the same name.
    class P type Person = P           // compile-time error
    let Person: Person                // compile-time error

    // The field and the method have the same name.
    class C {
        counter: number               // compile-time error
        counter(): number {           // compile-time error
          return this.counter
        }
    }

.. index::
   distinguishable declaration

|

.. _Scopes:

Scopes
******

.. meta:
    frontend_status: Partly
    todo: there are some TBD in spec

The **scope** of a name is the region of program code within which the entity
declared by that name can be referred to without the qualification of the name.
In other words, the name is accessible in some context if it can be used in this
context by its *simple* name.

The nature of usage of a scope depends on the kind of the name.
A type name can be used to declare variables or constants; a name of a function
can be used to call that function.

.. index::
   scope
   entity
   name qualification
   access
   simple name
   variable
   constant
   function call

The scope of a name depends on the context the name is declared in:

.. _package-access:

-  A name declared on the package level (*package level scope*) is accessible
   throughout the entire package. The name can be accessed in other packages
   if exported.

.. index::
   name
   declaration
   package level scope
   module level scope
   access
   module

.. _module-access:

-  *Module level scope* is applicable for separate modules only. A name
   declared on the module level is accessible throughout the entire module.
   The name can be accessed in other packages if exported.

.. index::
   module level scope
   module
   access
   name
   declaration

.. _class-access:
  
-  A name declared within a class (*class level scope*) is accessible in that
   class and sometimes, depending on the access modifier, outside the class or
   by methods of derived classes.

.. index::
   class level scope
   method
   name
   access
   modifier
   derived class
   declaration
   
.. _interface-access:

-  A name declared within an interface (*interface level scope*) is accessible
   inside and outside that interface (default public).
   
.. index::
   name
   declaration
   class level scope
   interface level scope
   interface
   access
   default public

.. _enum-access:

-  *Enum level scope*: as every enumeration defines a type within a package or
   module then its scope is identical to the package or module level scope.
   All enumeration constants have the same scope as the enumeration itself.

.. index::
   name
   declaration
   enum level scope
   enumeration
   enumeration constant
   package
   module
   scope

.. _class-or-interface-type-parameter-access:

-  *The scope of a type parameter* name in a class or interface declaration
   is that entire declaration, excluding static member declarations.

.. index::
   name
   declaration
   static member

.. _function-type-parameter-access:

-  The scope of a type parameter name in a function declaration is that
   entire declaration (*function parameter scope*).

.. index::
   parameter name
   function declaration
   function parameter scope

.. _function-access:

-  The scope of a name declared immediately within the body of a function
   (method) declaration is the body of that function declaration (*method
   or function scope*) from the place of declaration, and up to the end of
   the body.

.. index::
   scope
   function body declaration
   method body declaration
   method scope
   function scope

.. _block-access:

-  The scope of a name declared within a statement block is the body of
   such statement block from the point of declaration, and down to the end
   of the block (*block scope*).

.. index::
   statement block
   body
   point of declaration
   block scope

.. code-block:: typescript
   :linenos:

    function foo() {
        let x = y // compile-time error – y is not accessible
        let y = 1
    }

Scopes of two names can overlap (e.g., when statements are nested); in
that case the name with the innermost declaration takes precedence, and
access to the outer name is not accessible.

Class, interface and enum members are not directly accessible in a scope;
in order to access them, the dot operator '.' must be applied to an instance.

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

.. _Type Declarations:

Type Declarations
*****************

.. meta:
    frontend_status: Done

An interface declaration (see :ref:`Interfaces`), a class declaration (see
:ref:`Classes`), or an enum declaration (see :ref:`Enumerations`) are type
declarations.

.. code-block:: abnf

    typeDeclaration:
        classDeclaration
        | interfaceDeclaration
        | enumDeclaration
        ;

.. index::
   type declaration
   interface declaration
   class declaration
   enum declaration

|

.. _Type Alias Declaration:

Type Alias Declaration
**********************

.. meta:
    frontend_status: Partly
    todo: type alias can be as local declaration now, but the spec says it can be only topDeclaration
    todo: type alias name shouldn't be handled as variable name (eg: type foo = Double; let foo : int = 0 --> now error)

Type aliases enable using meaningful and concise notations by providing:

-  Names for anonymous types (array, function, union types); or
-  Alternative names for existing types.


Scopes of type aliases are package or module level scopes. Names
of all type aliases must be unique across all types in the current
context.

.. index::
   type alias
   anonymous type
   array
   function
   union type
   scope
   package level scope
   module level scope
   name

.. code-block:: abnf

    typeAlias:
        'type' identifier typeParameters? '=' type
        ;

Meaningful names can be provided for anonymous types as follows:

.. code-block:: typescript
   :linenos:

    type Matrix = number[][]
    type Handler = (s: string, no: number) => string
    type Predicate<T> = (x: T) => Boolean
    type NullableNumber = Number | null

If the existing type name is too long, then it is reasonable to introduce
a shorter new name by using type alias, particularly for a generic type.

.. code-block:: typescript
   :linenos:

    type Dictionary = Map<string, string>
    type MapOfString<T> = Map<T, string>

A type alias introduces no new type but acts only as a new name, while its
meaning remains the same as the original type’s.

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

Type aliases can be recursively referenced inside the right-hand side of a
type alias declaration, see :ref:`Recursive Type Aliases`.


.. index::
   anonymous type
   type alias
   generic type

|

.. _Recursive Type Aliases:

Recursive Type Aliases
======================

For a type alias defined as *type A = something*, *A* can be used recursively
if it is used as one of the following:

-  Array element type: *type A = A[]*; or
-  Type argument of some generic type: type A = C<A>.

.. code-block:: typescript
   :linenos:

    type A = A[] // ok, used as element type

    class C<G> { /*body*/}
    type B = C<B> // ok, used as a type argument

    type D = string | Array<D> // ok


Any other usage causes a compile-time error, as the compiler
does not have enough information about the defined alias:

.. code-block:: typescript
   :linenos:

    type E = E // compile-time error
    type F = string | E // compile-time error


Exactly the same rules apply to a generic type alias defined as
*type A<G> = something*.

.. code-block:: typescript
   :linenos:

    type A<G> = Array<A<G>> // ok, A<G> is used as a type argument
    type A<G> = string | Array<A<G>> // ok

    type A<G> = A<G> // compile-time error


A compile-time error occurs if a generic type alias is used without
a type argument:

.. code-block:: typescript
   :linenos:
   
    type A<G> = Array<A> // compile-time error

**Note**: there is no restriction on using a type parameter *G* in
the right side of a type alias declaration, and the following code
is valid:

.. code-block:: typescript
   :linenos:

    type NodeValue<G> = G | Array<G> | Array<NodeValue<G>>; 

|

.. _Variable and Constant Declarations:

Variable and Constant Declarations
**********************************

.. meta:
    frontend_status: Partly

|

.. _Variable Declarations:

Variable Declarations
=====================

.. meta:
    frontend_status: Done
    todo: spec issue: missing the default value for unsigned types - but would be better to remove entirely, just reference to 3.6 Default Valuew
    todo: es2panda bug: A local variable must be explicitly given a value before ifis used, by either
    todo: "Every variable in program must have a value before its value is used" - Can't be guaranteed in compile time that a non-nullable array component is initialized. initialization or assignment. But we got no error if don't init a primitive typed local var.

A *variable declaration* introduces a new named variable, which can be assigned
with an initial value.

.. code-block:: abnf

    variableDeclarations:
        'let' varDeclarationList
        ;

    variableDeclarationList:
        variableDeclaration (',' variableDeclaration)*
        ;

    variableDeclaration:
        identifier ('?')? ':' type initializer? 
        | identifier initializer
        ;

    initializer:
        '=' expression
        ;

The type *T* of a variable introduced by a variable declaration is determined
as follows:

-  *T* is the type specified in a type annotation (if any) of the declaration.
   If *'?'* is used after the name of the variable, then the variable's type is
   *type | undefined.* If the declaration also has an initializer, then the
   initializer expression must be compatible with *T* (see :ref:`Type Compatibility with Initializer`).

-  If no type annotation is available, then *T* is inferred from the
   initializer expression (see :ref:`Type Inference from Initializer`).

.. index::
   variable declaration
   named variable
   initial value
   variable
   type annotation
   initializer expression
   compatibility
   inference

.. code-block:: typescript
   :linenos:

    let a: number // ok
    let b = 1 // ok, number type is inferred
    let c: number = 6, d = 1, e = "hello" // ok

    // ok, type of lambda and type of 'f' can be inferred
    let f = (p: number) => b + p
    let x // compile-time error -- either type or initializer

Every variable in a program must have an initial value before its value is used.

There are several ways to identify such initial value:

-  An initial value is explicitly specified by using an *initializer*.
-  Each method or function parameter is initialized to the corresponding
   argument value provided by the caller of that method or function.
-  Each constructor parameter is initialized to the corresponding
   argument value provided by:
   
   + Class instance creation expression (see :ref:`New Expressions`), or
   + Explicit constructor call (see :ref:`Explicit Constructor Call`).

-  An exception parameter is initialized to the thrown object (see
   :ref:`Throw Statements`) that represents exception or error.
-  Each class, instance, local variable, or array element is initialized with
   a *default value* (see :ref:`Default Values for Types`) when it is created.

Otherwise, a variable is not initialized, and a :index:`compile-time error`
occurs.

If an initializer expression is provided, then additional restrictions apply
to the content of such expression as described in :ref:`Exceptions and Initialization Expression`.

.. index::
   initial value
   initializer
   method parameter
   function parameter
   argument value
   method caller
   function caller
   constructor parameter
   initialization
   instance creation expression
   explicit constructor call
   exception parameter
   exception
   error
   class
   instance
   local variable
   array element
   default value
   initializer expression
   restriction

|

.. _Constant Declarations:

Constant Declarations
=====================

.. meta:
    frontend_status: Done
    todo: no CTE in case of top-level "const x:int;" without initializer

A *constant declaration* introduces a named variable with a mandatory
explicit value.

The value of a constant cannot be changed by using an assignment expression
(see :ref:`Assignment`). However, if the constant is an object or array, then
its properties or items can be modified.

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

The type *T* of a constant declaration is determined as follows:

-  If *T* is the type specified in a type annotation (if any) of the
   declaration, then the initializer expression must be compatible with
   that *T* (see :ref:`Type Compatibility with Initializer`).
-  If no type annotation is available, then *T* is inferred from the
   initializer expression (see :ref:`Type Inference from Initializer`).
-  If *'?'* is used after the name of the constant, then the type of the
   constant is *T | undefined*, regardless of whether *T* is identified
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
   type annotation
   initializer expression
   compatibility
   inference

.. code-block:: typescript
   :linenos:

    const a: number = 1 // ok
    const b = 1 // ok, int type is inferred
    const c: number = 1, d = 2, e = "hello" // ok
    const x // compile-time error -- initializer is mandatory
    const y: number // compile-time error -- initializer is mandatory

Additional restrictions on the content of the initializer expression are
described in :ref:`Exceptions and Initialization Expression`.

|

.. _Type Compatibility with Initializer:

Type Compatibility with Initializer
===================================

.. meta:
    frontend_status: Done

If a variable or constant declaration contains the type annotation *T* and the
initializer expression *E*, then the type of *E* must be equal to that of *T*;
otherwise, one of the following assertions must be true:

+-----------------------+----------------------------------+-------------------------------------+
| **T is**              | **E is**                         |  **Assertion**                      |
+=======================+==================================+=====================================+
| One of integer types  | integer literal or compile-time  | Value of *E* is in bound of         |
|                       | constant expression of some      | type *T*                            |
|                       | integer type                     |                                     |
+-----------------------+----------------------------------+-------------------------------------+
| Type *char*           | integer literal                  | Value of *E* is in bounds of type   |
|                       |                                  | *T*                                 |
+-----------------------+----------------------------------+-------------------------------------+
| Float type (*float*   | floating-point literal or        | Value of *E* is in bounds of type   |
| or *double*)          | compile-time constant expression | *T*. *This conversion can lead to   |
|                       | of a float type                  | the loss of precision, see          |
|                       |                                  | “Narrowing Primitive Conversion”.*  |
+-----------------------+----------------------------------+-------------------------------------+
| Class type            | of a class type                  | Type of *E* is derived class of *T* |
+-----------------------+----------------------------------+-------------------------------------+

Unless at least one of these conditions is fulfilled, then an error is thrown.

.. index::
   compile-time error
   type compatibility
   initializer expression
   initializer
   exception
   integer literal
   bound
   conversion
   type
   narrowing
   derived class
   class type
   float type
   constant expression
   type annotation
   assertion

|

.. _Type Inference from Initializer:

Type Inference from Initializer
===============================

.. meta:
    frontend_status: Partly
    todo: spec issue: "If initializer expression is a null literal('null') the compiler error should be reported". Why is it striked out? "let a = null" should be CTE A: scpec will be changed, a will have "Object|null tyoe"

The type of a declared entity is:

-  The type of the initializer expression if a variable or constant
   declaration contains no explicit type annotation; or

-  *Object \| null* if the initializer expression is a null literal
   (*null*).

.. index::
   type
   entity
   type inference
   initializer
   variable declaration
   constant declaration
   type annotation
   initializer expression
   null literal
   Object

|

.. _Function Declarations:

Function Declarations
*********************

.. meta:
    frontend_status: Partly

**Function declarations** specify names, signatures, and bodies when
introducing **named functions**.

.. code-block:: abnf

    functionDeclaration:
        functionOverloadSignature*
        modifiers? 'function' identifier
        typeParameters? signature block?
        ;

    modifiers:
        'native' | 'async'
        ;

Function *overload signature* allows calling a function in different ways (see
:ref:`Function Overload Signatures`).

In a **native function** (see :ref:`Native Functions`), the body is omitted.

If a function is declared as **generic** (see :ref:`Generics`), then type
parameters must be  specified.

Native functions are described in the experimental section (see
:ref:`Native Functions`).

Functions must be declared on the top-level (see :ref:`Top-Level Statements`).

Function expressions must be used to define lambdas (see
:ref:`Lambda Expressions`).

.. index::
   function declaration
   name
   signature
   named function
   body
   function overload signature
   function call
   native function
   generic function
   type parameter
   top-level statement
   lambda

|

.. _Signatures:

Signatures
==========

.. meta:
    frontend_status: Done

A signature defines parameters, and the return type (see :ref:`Return Type`)
of a function, method, or constructor.

.. code-block:: abnf

    signature:
        '(' parameterList? ')' returnType? throwMark?
        ;

    returnType:
        ':' type
        ;

    throwMark:
        'throws' | 'rethrows'
        ;

See :ref:`Throwing Functions` for the details of ``throws`` marks, and
:ref:`Rethrowing Functions` for the details of ``rethrows`` marks.

Overloading (see :ref:`Function and Method Overloading`) is supported for
functions and methods. The signatures of functions and methods are important
for their unique identification.

.. index::
   signature
   parameter
   return type
   function
   method
   constructor
   throwing function
   rethrowing function
   throws mark
   rethrows mark
   function overloading
   method overloading
   identification

|

.. _Parameter List:

Parameter List
==============

.. meta:
    frontend_status: Partly

A signature contains a *parameter list* that specifies an identifier and a
type of each parameter. Every parameter’s type must be explicitly defined.
The last parameter of a function can be a *rest parameter*
(see :ref:`Rest Parameter`).

.. code-block:: abnf

    parameterList:
        parameter (',' parameter)* (',' restParameter)?
        | restParameter
        ;

    parameter:
        identifier ':' type
        | optionalParameter
        ;

    restParameter:
        '...' parameter
        ;

An *optional parameter* (see :ref:`Optional Parameters`) allows to omit the
corresponding argument when calling a function. If a parameter is not
*optional*, then each function call must contain an argument corresponding to
that parameter. Non-optional parameters are called the *required parameters*.

The function below has *required parameters*:

.. code-block:: typescript
   :linenos:

    function power(base: number, exponent: number): number {
      return Math.pow(base, exponent)
    }
    power(2, 3) // both arguments are required in the call

A :index:`compile-time error` occurs if an *optional parameter* precedes a
*required parameter* in the parameter list.

.. index::
   signature
   parameter list
   identifier
   parameter type
   function
   rest parameter
   optional parameter
   argument
   non-optional parameter
   required parameter

|

.. _Optional Parameters:

Optional Parameters
===================

There are two forms of *optional parameters*:

.. code-block:: abnf

    optionalParameter:
        identifier ':' type '=' expression
        | identifier '?' ':' type
        ;

The first form contains an expression that specifies a *default value*. Such
parameter is called a *parameter with default values*. If the argument
corresponding to that parameter is omitted in a function call, then the
value of the parameter is default.

.. index::
   optional parameter
   expression
   default value
   parameter with default values
   argument
   function call
   default value

.. code-block:: typescript
   :linenos:

    function pair(x: number, y: number = 7)
    {
        console.log(x, y)
    }
    pair(1, 2) // prints: 1 2
    pair(1) // prints: 1 7

The second form is a short notation for a parameter of a union type
*T* | ``undefined`` with the default value ``undefined``. 
It means that *identifier '?' ':' type* is equivalent to
*identifier ':' type | undefined = undefined*.
If a type is of the value type kind, then (similar to :ref:`Union Types`)
implicit boxing must be applied in the following manner:
*identifier '?' ':' valueType* is equivalent to
*identifier ':' referenceTypeForValueType | undefined = undefined*.

.. index::
   notation
   parameter
   union type
   undefined
   default value
   identifier
   value type
   union type
   implicit boxing
   function

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
    function foo2 (p: Number | undefined = undefined) {}

    foo1() // 'p' has 'undefined' value
    foo1(5) // 'p' has an integer value
    foo2() // 'p' has 'undefined' value
    foo2(5) // 'p' has an integer value

|

.. _Rest Parameter:

Rest Parameter
==============

A *rest parameter* allows functions or methods to take unbounded numbers
of arguments.

*Rest parameters* have the '``...``' symbol mark before a parameter name.

.. code-block:: typescript
   :linenos:

    function sum(...numbers: number[]): number {
      let res = 0
      for (let n of numbers)
        res += n
      return res
    }

A :index:`compile-time error` occurs if a rest parameter:

-  Is not the last parameter in a parameter list;
-  Has a type that is not an array type.

A function that has a rest parameter of type ``T[]`` can accept any
number of arguments of type ``T``.

.. index::
   rest parameter
   function
   method
   unbounded
   parameter name
   array type
   parameter list
   type
   argument

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

If an argument of type ``T[]`` is prefixed with the *spread* operator
'``...``', then only one argument can be accepted.

.. code-block:: typescript
   :linenos:

    function sum(...numbers: number[]): number {
      let res = 0
      for (let n of numbers)
        res += n
      return res
    }

    let x: number[] = [1, 2, 3]
    sum(...x) // returns 6

.. index::
   argument
   prefix
   spread operator

|

.. _Shadowing Parameters:

Shadowing Parameters
====================

.. meta:
    frontend_status: Done

If the name of the parameter is identical to the name of the top-level
variable accessible within the body of a function or a method with that
a parameter, then the name of the parameter shadows the name of the
top-level variable within the body of that function or method.

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
   shadowing parameter
   shadowing
   parameter
   top-level variable
   access
   function body
   method body
   name

|

.. _Return Type:

Return Type
===========

.. meta:
    frontend_status: Done

An omitted function, or method return type can be inferred from the function,
or the method body. A :index:`compile-time error` occurs if a return type is
omitted in a native function (see :ref:`Native Functions`).

The current version of |LANG| allows inferring return types at least under
the following conditions:

-  If there is no return statement, or if all return statements have no
   expressions, then the return type is *void* (see :ref:`void Type`).
-  If there is at least one return statement with an expression, and the
   type of each expression of each return statement is *R*, then the
   return type is *R*.
-  If there are *k* return statements (assuming that *k* is two, or more)
   with expressions of types (*T*:sub:`1`, ``...``, *T*:sub:`k`), and *R*
   is the *least upper bound* (see :ref:`Least Upper Bound`) of these types,
   then the return type is *R*.
-  If the function is *async*, the return type is inferred by using the rules
   above, and the type *T* is not *Promise* type, then the return type
   is *Promise<T>*.

Future compiler implementation can infer return type in more cases. If
the particular type inference case is not recognized by the compiler,
then a corresponding :index:`compile-time error` occurs.

See the example below for an illustration of type inference:

.. index::
   return type
   function return type
   method return type
   inference
   method body
   native function
   return statement
   expression
   least upper bound
   function
   implementation

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
            return new Derived1
        else
            return new Derived2
    }
    /* Return type of bar will be inferred as Base which is 
       LUB for Derived1 and Derived2 */

|

.. _Function Overload Signatures:

Function Overload Signatures
============================

.. meta:
    frontend_status: None

The |LANG| language allows specifying a function that can be called in
different ways by writing *overload signatures*, i.e., by writing several
function headers that have the same name but different signatures followed
by one implementation function. See also :ref:`Methods Overload Signatures`
for *method overload signatures*.

.. code-block:: abnf

    functionOverloadSignature:
      'async'? 'function' identifier
      typeParameters? signature ';'
      ;

A :index:`compile-time error` occurs if the function implementation is missing,
or does not immediately follow the declaration.

A call of a function with overload signatures is always a call of the
implementation function.

The example below has two overload signatures defined (one is parameterless,
and the other has one parameter):

.. index::
   function overload signature
   function
   overload signature
   function header
   signature
   implementation function
   implementation
   method overload signature

.. code-block:: typescript
   :linenos:

    function foo(): void; /*1st signature*/
    function foo(x: string): void; /*2nd signature*/
    function foo(x?: string): void {
        console.log(x)
    }

    foo() // ok, 1st signature is used
    foo("aa") // ok, 2nd signature is used

The call of ``foo()`` is executed as a call of the implementation function
with the ``null`` argument, while the call of ``foo(x)`` is executed as a call
of the implementation function with the ``x`` argument.

A :index:`compile-time error` occurs if the signature of function
implementation is not *overload signature-compatible* with each overload
signature. It means that a call of each overload signature must be replaceable
for the correct call of the implementation function. This can be achieved by
using optional parameters (see :ref:`Optional Parameters`), or *least upper
bound* types (see :ref:`Least Upper Bound`). See
:ref:`Overload Signature Compatibility` for the exact semantic rules.

A :index:`compile-time error` occurs unless all overload signatures are
exported or non-exported.

.. index::
   call
   implementation function
   null argument
   execution
   signature
   function
   implementation
   overload signature
   least upper bound
   compatibility

.. raw:: pdf

   PageBreak


