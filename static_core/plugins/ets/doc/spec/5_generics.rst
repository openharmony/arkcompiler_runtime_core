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

.. _Generics:

Generics
########

.. meta:
    frontend_status: Partly

Class, interface, method, constructor, and function are program entities
that can be generalized in the |LANG| language. Generalization is
parameterizing an entity by one or several types. A generalized
entity is introduced by a *generic declaration* (called *generic*
for brevity).

.. index::
   entity
   generalization
   parameterization
   generic declaration
   generic

|

.. _Generic Declarations:

Generic Declarations
********************

.. meta:
    frontend_status: Partly

Types used as generic parameters in a generic are called *type parameters*.

A *generic* must be instantiated in order to be used. *Generic instantiation*
is the action that converts a *generic* into a real program entity: ordinary
class, interface, function, etc.
Instantiation can be performed either explicitly or implicitly.

*Explicit* generic instantiation is the language construct that specifies
real types, which substitute type parameters of a *generic*. Real types
specified in the instantiation are called *type arguments*.

.. index::
   generic
   explicit instantiation
   instantiation
   conversion
   program entity
   generic declaration
   generic parameter
   type parameter
   generic instantiation
   conversion
   construct
   type argument

In an *implicit* instantiation, type arguments are not specified explicitly.
They are inferred from the context the generic is referred in. Implicit
instantiation is possible only for functions and methods.

The result of instantiation is a *real*, non-parameterized program entity:
class, interface, method, constructor, or function. The entity is handled
exactly as an ordinary class, interface, method, constructor, or function.

Conceptually, a generic class, an interface, a method, a constructor, or a
function defines a set of classes, interfaces, methods, constructors, or
functions respectively (see :ref:`Generic Instantiations`).

.. index::
   implicit instantiation
   instantiation
   type argument
   context
   non-parameterized entity
   method
   class
   interface
   constructor
   function

|

.. _Generic Parameters:

Generic Parameters
******************

.. meta:
    frontend_status: Done

A class, an interface, or a function must be parameterized by at least one
*type parameter* to be a *generic*. The type parameter is declared in the type
parameter section. It can be used as an ordinary type inside a *generic*.

Syntactically, a type parameter is an unqualified identifier with the proper
scope (see :ref:`Scopes` for the scope of type parameters). Each type parameter
can have a *constraint* (see :ref:`Type Parameter Constraint`). A type
parameter can have a default type (see :ref:`Type Parameter Default`).

.. index::
   generic parameter
   generic
   class
   interface
   function
   parameterization
   type parameter
   unqualified identifier
   scope
   constraint
   default type
   type parameter

.. code-block:: abnf

    typeParameters:
        '<' typeParameterList '>'
        ;

    typeParameterList:
        typeParameter (',' typeParameter)*
        ;

    typeParameter:
        ('in' | 'out')? identifier constraint? typeParameterDefault?
        ;

    constraint:
        'extends' typeReference | keyofType
        ;

    typeParameterDefault:
        '=' typeReference
        ;

A generic class, interface, method, constructor, or function defines a set
of parameterized classes, interfaces, methods, constructors, or functions
respectively (see :ref:`Generic Instantiations`). One type argument can define
only one set for each possible parameterization of the type parameter section.

.. index::
   generic declaration
   generic class
   generic interface
   generic function
   generic instantiation
   class
   interface
   function
   instantiation
   type parameter
   ordinary type
   parameterized class
   parameterized interface
   parameterized function
   type-parameterized declaration
   argument
   parameterization

|

.. _Type Parameter Constraint:

Type Parameter Constraint
*************************

.. meta:
    frontend_status: Partly
    todo: overloading functions with bound, and resolving call for correct overload
    todo: Checking of boundaries on call site
    todo: Further checks on multiple parameter bounds
    todo: Implement union type support for constraints
    todo: Adapt spec change: T without constraint doesn't mean "T extends Object|null" anymore.


If a type parameter has restrictions, or *constraints*, then such constraints
must be followed by the corresponding type argument in a generic instantiation.

In every type parameter, a constraint can follow the keyword ``extends``. The
constraint is denoted as a single type parameter *T*. If no constraint is
declared, then the type parameter is not compatible with ``Object``, and
has no methods or fields available for use. Lack of constraint effectively
means ``extends Object|null|undefined``.
If type parameter *T* has type constraint *S*, then the actual type of the
generic instantiation must be a subtype (see :ref:`Subtyping`) of *S*. If the
constraint *S* is a non-nullish type (see :ref:`Nullish Types`), then *T* is
non-nullish too. If the type parameter is constrained with the ``keyof T``,
then valid instantiations of this parameter can be the values of the union type
created from string names of *T* or the union type itself:

.. index::
   type parameter constraint
   keyword extends
   type argument
   generic instantiation
   instantiation
   constraint
   subtype

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base { }
    class SomeType { }

    class G<T extends Base> { }
    
    let x: G<Base>      // correct
    let y: G<Derived>   // also correct
    let z: G<SomeType>  // error: SomeType is not a subtype of Base

    class A {
      f1: number = 0
      f2: string = ""
      f3: boolean = false
    }
    class B<T extends keyof A> {}
    let b1 = new B<'f1'>    // OK
    let b2 = new B<'f0'>    // Compile-time error as "f0" does not satisfy the constraint 'keyof A'
    let b3 = new B<keyof A> // OK

A type parameter of a generic can *depend* on another type parameter
of the same generic.

If *S* constrains *T*, then the type parameter *T* *directly depends*
on the type parameter *S*, while *T* directly depends on the following:

-  *S*; or
-  Type parameter *U* that depends on *S*.

A compile-time error occurs if a type parameter in the type parameter
section depends on itself.

.. index::
   type parameter
   generic declaration
   type parameter
   unqualified identifier
   generic declaration
   constraint
   compile-time error

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived { }
    class SomeType { }
  
    class G<T, S extends T> {}
    
    let x: G<Base, Derived>  // correct: the second argument directly
                            // depends on the first one
    let y: G<Base, SomeType> // error: SomeType doesn't depend on Base

    class A0<T> {
       data: T
       constructor (p: T) { this.data = p }
       foo () {
          let o: Object = this.data // error: as type T is not compatible with Object
          console.log (this.data.toString()) // error: type T has no methods or fields
       }
    }

    class A1<T extends Object> extends A0<T> {
       constructor (p: T) { this.data = p }
       override foo () {
          let o: Object = this.data // OK!
          console.log (this.data.toString()) // OK!
       }
    }

|

.. _Generic Instantiations:

Generic Instantiations
**********************

.. meta:
    frontend_status: Partly

As mentioned before, a generic class, interface, or function declaration
defines a set of corresponding non-generic entities. A generic entity
must be *instantiated* in order to get a non-generic entity out of it.
The explicit instantiation is specified by providing a list of *type arguments*
that substitute corresponding type parameters of the generic:

.. index::
   instantiation
   generic entity
   non-generic entity
   function declaration
   type argument
   type parameter
   generic

*G* <``T``:sub:`1`, ``...``, ``T``:sub:`n`>

---where <``T``:sub:`1`, ``...``, ``T``:sub:`n`> is the list of type arguments
for the generic declaration *G*.

..
   lines 312, 314, 336 - initially the type was *T*:sub:`1`, ``...``, *T*:sub:`n`
   lines 321, 322 - initially *C*:sub:`1`, ``...``, *C*:sub:`n` and *T*:sub:`1`, ``...``, *T*:sub:`n` 

If ``C``:sub:`1`, ``...``, ``C``:sub:`n` is the constraint for the corresponding
type parameters ``T``:sub:`1`, ``...``, ``T``:sub:`n` of a generic declaration,
then *T*:sub:`i` is a subtype (see :ref:`Subtyping`) of each constraint type
*C*:sub:`i`. All subtypes of the type listed in the corresponding constraint
have each type argument *T*:sub:`i` of the parameterized declaration ranging
over them.

.. index::
   type argument
   type parameter
   generic declaration
   parameterized declaration
   subtype
   constraint

A generic instantiation *G* <``T``:sub:`1`, ``...``, ``T``:sub:`n`> is
*well-formed* if **all** of the following is true:

-  The generic declaration name is *G*.
-  The number of type arguments equals that of *G*’s type parameters.
-  All type arguments are subtypes (see :ref:`Subtyping`) of a corresponding
   type parameter constraint.

A compile-time error occurs if an instantiation is not well-formed.

Unless explicitly stated otherwise in appropriate sections, this specification
discusses generic versions of class type, interface type, or function.

Any two generic instantiations are considered *provably distinct* if:

-  Both are parameterizations of distinct generic declarations; or
-  Any of their type arguments is provably distinct.

.. index::
   instantiation
   generic instantiation
   well-formed declaration
   generic declaration
   type argument
   type parameter
   subtype
   type parameter constraint
   compile-time error
   class type
   interface type
   function
   provably distinct instantiation
   parameterization
   distinct generic declaration
   distinct argument

|

.. _Type Parameter Default:

Type Parameter Default
**********************

.. meta:
    frontend_status: Done

Type parameters of generic types can have defaults. This situation allows
dropping a type argument when a particular type of instantiation is used.
However, a compile-time error occurs if a type parameter without a
default type follows a type parameter with a default type in the
declaration of a generic type.

The examples below illustrate this for both classes and functions:

.. index::
   type parameter
   generic type
   type argument
   type parameter default
   instantiation
   class
   function
   compile-time error


.. code-block-meta:
    expect-cte:

.. code-block:: typescript
   :linenos:

    class SomeType {}
    interface Interface <T1 = SomeType> { }
    class Base <T2 = SomeType> { }
    class Derived1 extends Base implements Interface { }
    // Derived1 is semantically equivalent to Derived2
    class Derived2 extends Base<SomeType> implements Interface<SomeType> { }

    function foo<T = number>(): T {
        // ...
    }
    foo() // this call is semantically equivalent to the call below
    foo<number>()

    class C1 <T1, T2 = number, T3> {}
    // That is a compile-time error, as T2 has default but T3 does not

    class C2 <T1, T2 = number, T3 = string> {}
    let c1 = new C2<number>          // equal to C2<number, number, string>
    let c2 = new C2<number, string>  // equal to C2<number, string, string>
    let c3 = new C2<number, Object, number> // all 3 type arguments provided

|

.. _Type Arguments:

Type Arguments
**************

.. meta:
    frontend_status: Partly
    todo: implement "Type Argument Variance" fully

Type arguments can be reference types or wildcards.

If a value type is specified as a type argument in the generic instantiation,
then the boxing conversion applies to the type (see :ref:`Boxing Conversions`).

.. code-block:: abnf

    typeArguments:
        '<' typeArgumentList '>'
        ;

A compile-time error occurs if type arguments are omitted in a parameterized
function.

.. index::
   type argument
   reference type
   wildcard
   boxing conversion
   numeric type
   predefined numeric types conversion
   raw type
   parameterized function
   compile-time error

.. code-block:: abnf

    typeArgumentList:
        typeArgument (',' typeArgument)*
        ;

    typeArgument:
        typeReference
        | arrayType
        | wildcardType
        ;

    wildcardType:
        'in' typeReference
        | 'out' typeReference?
        ;


.. _Type Argument Variance:

Type Argument Variance
======================

.. meta:
    frontend_status: Partly
    todo: implement semantic, now in/out is only parsed and ignored

The variance for type arguments can be specified with wildcards (*use-site
variance*). It allows changing type variance of an *invariant* type parameter.

**Note**: This description of *use-site variance* modifiers is tentative.
The details are to be specified in the future versions of |LANG|.

The syntax to signify a *covariant* :ref:`Covariance` type argument, or a
wildcard with an upper bound (*T* is a ``typeReference``) is as follows:

.. index::
   variance
   type argument
   wildcard
   use-site variance
   modifier
   type variance
   invariant type parameter
   covariant type parameter
   upper bound

-  ``out`` *T*

   This syntax restricts the methods available, and allows accessing only
   the methods that do not use *T*, or use *T* in out-position.

The syntax to signify a contravariant :ref:`Contravariance` type argument, or
a wildcard with a lower bound (*T* is a ``typeReference``) is as follows:

-  ``in`` *T*

   This syntax restricts the methods available, and allows accessing only
   the methods that do not use *T*, or use *T* in in-position.

.. index::
   method
   access
   out-position
   contravariant type argument
   wildcard
   lower bound
   in-position

The unbounded wildcard ``out``, and the wildcard ``out Object | null`` are
equivalent.

A compile-time error occurs if:

-  A wildcard is used in a parameterization of a function; or
-  A *covariant* :ref:`Covariance` wildcard is specified for a *contravariant*
   :ref:`Contravariance` type parameter; or
-  A *contravariant* wildcard is specified for a *covariant* :ref:`Covariance`
   type parameter.

.. index::
   compile-time error
   unbounded wildcard
   wildcard
   covariant wildcard
   contravariant wildcard
   function parameterization
   contravariant type parameter
   covariant type parameter

The rules below apply to the subtyping (see :ref:`Subtyping`) of two
non-equivalent types *A* <: *B*, and an invariant type parameter *F* in
case of use-site variance:

-  ``T <out A>`` <: ``T <out B>``;
-  ``T <in A>`` :> ``T <in B>``;
-  ``T* <A>`` <: ``T <out A>``;
-  ``T <A>`` <: ``T <in A>``.

.. index::
   subtyping
   invariant type parameter
   use-site variance

Any two type arguments are considered *provably distinct* if:

-  The two arguments are not of the same type, and neither is a type parameter
   nor a wildcard; or
-  One type argument is a type parameter or a wildcard with an upper bound
   of *S*, the other *T* is not a type parameter and not a wildcard, and
   neither is a subtype (see :ref:`Subtyping`) of the other ; or
-  Each type argument is a type parameter, or wildcard with upper bounds
   *S* and *T*, and neither is a subtype (see :ref:`Subtyping`) of the other.

.. index::
   provably distinct type argument
   type parameter
   wildcard
   subtype
   upper bound
   type argument

|

.. _Utility Types:

Utility Types
*************

|LANG| supports several embedded types, called *utility* types.
They allow constructing new types, and extend their functionality.

.. index::
   embedded type
   utility type
   extended functionality

|

.. _Partial Utility Type:

Partial Utility Type
====================

.. meta:
    frontend_status: None

Type ``Partial<T>`` constructs a type with all properties of *T* set to
optional. *T* must be a class or an interface type:

.. code-block:: typescript
   :linenos:

    interface Issue {
        title: string
        description: string
    }

    function process(issue: Partial<Issue>) {
        if (issue.title != undefined) { 
            /* process title */
        }
    }
    
    process({title: "aa"}) // description is undefined

In the example above, type ``Partial<Issue>`` is transformed to a distinct
type that is analogous:

.. code-block:: typescript
   :linenos:

    interface /*some name*/ {
        title?: string
        description?: string
    }

|

.. Required Utility Type:

Required Utility Type
=====================

.. meta:
    frontend_status: None

Type ``Required<T>`` is opposite to ``Partial<T>``.
It constructs a type with all properties of *T* set to
be required (not optional). *T* must be a class or an interface type.

.. code-block:: typescript
   :linenos:

    interface Issue {
        title?: string
        description?: string
    }

    let c: Required<Issue> = { // CTE: 'description' should be defined
        title: "aa"
    }
    


The type defined in the example above, the type ``Required<Issue>``
is transformed to a distinct type that is analogous:

.. code-block:: typescript
   :linenos:

    interface /*some name*/ {
        title: string
        description: string
    }

|

.. _Readonly Utility Type:

Readonly Utility Type
=====================

.. meta:
    frontend_status: None

Type ``Readonly<T>`` constructs a type with all properties of *T* set to
readonly. It means that the properties of the constructed value cannot be
reassigned. *T* must be a class or an interface type:

.. code-block:: typescript
   :linenos:

    interface Issue {
        title: string
    }

    const myIssue: Readonly<Issue> = {
        title: "One"
    };

    myIssue.title = "Two" // compile-time error: readonly property

|

.. _Record Utility Type:

Record Utility Type
===================

.. meta:
    frontend_status: Partly
    todo: implement record indexing - #13845

Type ``Record<K, V>`` constructs a container that maps keys (of type *K*)
to values (of type *V*).

The type *K* is restricted to ``number`` types, type ``string``, union types
constructed from these types, and literals of these types.

A compile-time error occurs if any other type, or literal of any other type
is used in place of this type:

.. index::
   record utility type
   value
   container
   union type
   number type
   string type
   literal
   compile-time error

.. code-block:: typescript
   :linenos:

    type R1 = Record<number, string> // ok
    type R2 = Record<boolean, string> // compile-time error
    type R3 = Record<1 | 2, string> // ok
    type R4 = Record<"salary" | "bonus", number> // ok
    type R4 = Record<1 | true, number> // compile-time error

There are no restrictions on type *V*. 

A special form of object literals is supported for instances of type ``Record``
(see :ref:`Object Literal of Record Type`).

Access to ``Record<K, V>`` values is performed by an *indexing expression*
like *r[index]*, where *r* is an instance of type ``Record``, and *index*
is the expression of type *K*. The result of an indexing expression is of type
*V* if *K* is a union that contains literal types only. Otherwise, it is of
type ``V | undefined``. See :ref:`Record Indexing Expression` for details.

.. index::
   object literal
   instance
   Record type
   access
   indexing expression
   index expression

.. code-block:: typescript
   :linenos:
   
    type Keys = 'key1' | 'key2' | 'key3'
   
    let x: Record<Keys, number> = {
        'key1': 1,
        'key2': 2,
        'key3': 4,
    }
    console.log(x['key2']) // prints 2
    x['key2'] = 8
    console.log(x['key2']) // prints 8

In the example above, *K* is a union of literal types. The result of an
indexing expression is of type *V*. In this case it is ``number``.


.. raw:: pdf

   PageBreak


