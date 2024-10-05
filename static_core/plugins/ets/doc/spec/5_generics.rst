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

Class, interface, type alias, method, function, and lambda are program entities
that can be parameterized in |LANG| by one or several types. An entity so
parameterized introduces a *generic declaration* (called *a generic* for
brevity).

Types used as generic parameters in a generic are called *type parameters*
(see :ref:`Type Parameters`).

A *generic* must be instantiated in order to be used. *Generic instantiation*
is the action that converts a *generic* into a real program entity (non-generic
class, interface, union, array, mehtod, function, or lambda), or into another
*generic instantiation*. Instantiation (see :ref:`Generic Instantiations`) can
be performed either explicitly or implicitly.

|LANG| has special types that look similar to generics syntax-wise, and allow
creating new types during compilation (see :ref:`Utility Types`).


.. index::
   entity
   parameterization
   generic declaration
   generic
   explicit instantiation
   instantiation
   program entity
   generic parameter
   type parameter
   generic instantiation

|

.. _Type Parameters:

Type Parameters
***************

.. meta:
    frontend_status: Done

The type parameter is declared in the type parameter section. It can be used as
an ordinary type inside a *generic*. 

Syntactically, a type parameter is an unqualified identifier with a proper
scope (see :ref:`Scopes` for the scope of type parameters). Each type parameter
can have a *constraint* (see :ref:`Type Parameter Constraint`). A type
parameter can have a default type (see :ref:`Type Parameter Default`), and can
specify its *in-* or *out-* variance (see :ref:`Type Parameter Variance`).

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
   variance

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
        'extends' typeReference | keyofType | unionType
        ;

    typeParameterDefault:
        '=' typeReference ('[]')?
        ;

A generic class, interface, type alias, method, function, or lambda defines a
set of parameterized classes, interfaces, unions, arrays, methods, functions, or
lambdas respectively (see :ref:`Generic Instantiations`).
One type argument can define only one set for each possible parameterization of
the type parameter section.

.. index::
   generic declaration
   generic class
   generic interface
   generic function
   generic lambda
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
   parameterization

No type parameter has a default value, and initialization is mandatory for
variables and fields of a type parameter (see :ref:`Field Initialization`):

.. code-block:: typescript
   :linenos:
   
    class G<T> {
        field: T // compile-time error, field is not initialized
        function foo() {
            let t: T // compile-time error, variable is not initialized
        }
    }


.. index::
    default value
    field initialization

|

.. _Type Parameter Constraint:

Type Parameter Constraint
=========================

.. meta:
    frontend_status: Partly
    todo: support keyof constraint #17436

If possible instantiations need to be constrained, then an individual
*constraint* can be set for every type parameter.

A constraint of any type parameter can follow the keyword ``extends``.
The constraint is denoted as a type reference, type ``keyof``, or a union type.
If no constraint is declared, then the type parameter is not compatible with
``Object``, and has no methods or fields available for use. Lack of constraint
effectively means ``extends Object|null|undefined``. 
If type parameter *T* has type constraint *S*, then the actual type of the
generic instantiation must be compatible with *S* (see :ref:`Type Compatibility`).
If the constraint *S* is a non-nullish type (see :ref:`Nullish Types`), then
*T* is non-nullish too. If the type parameter is constrained with the
``keyof T``, then valid instantiations of this parameter can be the values of
the union type created from string names of *T* or the union type itself:

.. index::
   type parameter constraint
   keyword extends
   type argument
   generic instantiation
   instantiation
   constraint

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base { }
    class SomeType { }

    class G<T extends Base> { }
    
    let x = new G<Base>      // OK
    let y = new G<Derived>   // OK
    let z = new G<SomeType>  // Compile-time : SomeType is not compatible with Base

    class H<T extends Base|SomeType> {}
    let h1 = new H<Base>     // OK
    let h2 = new H<Derived>  // OK
    let h3 = new H<SomeType> // OK
    let h4 = new H<Object>   // Compile-time : Object is not compatible with Base|SomeType

    class Exotic<T extends 1|2|3> {}
    let e1 = new Exotic<2>   // OK
    let e2 = new Exotic<64>  // Compile-time : 64 is not compatible with 1|2|3

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

A :index:`compile-time error` occurs if a type parameter in the type parameter
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
    class Derived extends Base { }
    class SomeType { }
  
    class G<T, S extends T> {}
    
    let x: G<Base, Derived>  // correct: the second argument directly
                             // depends on the first one
    let y: G<Base, SomeType> // error: SomeType does not depend on Base

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

.. _Type Parameter Default:

Type Parameter Default
======================

.. meta:
    frontend_status: Done

Type parameters of generic types can have defaults. This situation allows
dropping a type argument when a particular type of instantiation is used.
However, a :index:`compile-time error` occurs if:

- A type parameter without a default type follows a type parameter with a
  default type in the declaration of a generic type;
- Type parameter default refers to a type parameter defined after the current
  type parameter.

The application of this concept to both classes and functions is presented
in the examples below:

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
 
    function foo <T1 = T2, T2 = T1> () {}
    // That is a compile-time error, 
    // as T1's default refers to T2, which is defined after the T1
    // T2's default is valid as it refers to already defined type parameter T1

|

.. _Type Parameter Variance:

Type Parameter Variance
=======================

.. meta:
    frontend_status: Partly
    todo: Implement semantic checks, now in/out modifiers are only parsed ang ignored.

Normally, two different argument types used to instantiate a generic class or
interface are handled as different and unrelated types (*invariance*). |LANG|
supports type parameter variance that allows such instantiations become base
classes and derived classes (:ref:`Covariance`), or vice versa
(:ref:`Contravariance`), depending on the relationship of inheritance between
argument types.

.. index::
   generic class
   argument type
   invariance
   contravariance
   covariance
   inheritance
   derived class
   base class


Special markers are used to specify the *declaration-site variance*. The
markers are to be added to generic parameter declarations. These markers are
expressed as keywords ``in`` or ``out`` (a *variance modifier* that specifies
the variance of the type parameter).

Type parameters with the keyword ``out`` are *covariant* (see
:ref:`Covariance`), and can be used in the out-position only:

   - Methods may have ``out`` type parameters as return types
   - Fields of ``out`` type parameters as type should be ``readonly``.

Otherwise a :index:`compile-time error` occurs.

Type parameters with the keyword ``in`` are *contravariant* (see
:ref:`Contravariance`), and can be used in the in-position only:

   - Methods may have ``in`` type parameters as parameter types

Otherwise a :index:`compile-time error` occurs.

Type parameters with no variance modifier are implicitly *invariant*, and can
occur in any position.

.. code-block:: typescript
   :linenos:

    class X<in T1, out T2, T3> {
       // T1 can be used in in-position only
       foo (p: T1) {...} 

       // T2 can be used in out-position only
       bar(): T2 {...}   
       readonly fld1: T2 

       // T3 can be used in any position (in-out, write-read)
       fld2: T3 
       method (p: T3): T3 {...}
    } 


In case of function types (see :ref:`Function Types`) variance interleaving
occurs. 

.. code-block:: typescript
   :linenos:

    class X<in T1, out T2> {
       foo (p: T1): T2 {...}                           // in - out
       foo (p: (p: T2)=> T1) {...}                     // out - in
       foo (p: (p: (p: T1)=>T2)=> T1) {...}            // in - out - in
       foo (p: (p: (p: (p: T2)=> T1)=>T2)=> T1) {...}  // out - in - out - in
       // and further more
    } 


.. index::
   generic
   declaration-site variance
   type parameter
   keyword in
   keyword out
   variance modifier
   in-position
   out-position

A :index:`compile-time error` occurs if a function, method, or constructor
type parameters have a variance modifier specified.

*Variance* is used to describe the subtyping (see :ref:`Subtyping`) operation
on parameterized types (see :ref:`Generics`). The variance of the corresponding
type parameter *F* defines the subtyping between ``T<A>`` and ``T<B>`` (in the
case of declaration-site variance with two different types ``A`` <: ``B``) as
follows:

-  Covariant :ref:`Covariance` (*out F*): ``T<A>`` <: ``T<B>``;
-  Contravariant :ref:`Contravariance` (*in F*): ``T<A>`` :> ``T<B>``;
-  Invariant (default) (*F*).

.. index::
   type parameter
   variance modifier
   function
   method
   constructor
   variance
   covariance
   contravariance
   invariance
   type-parameterized declaration
   parameterized type
   subtyping
   declaration-site variance

|

.. _Generic Instantiations:

Generic Instantiations
**********************

.. meta:
    frontend_status: Done

As mentioned before, a generic class, interface, type alias, method, function,
or lambda declaration defines a set of corresponding non-generic entities. A
generic entity must be *instantiated* in order to make it a non-generic entity.
The result of instantiation is a *real*, non-parameterized program entity, e.g.,
class, interface, union, array, method, function, or lambda that is handled in a
usual way. Instantiation can also lead to a new definition of a generic entity.

Conceptually, a generic class, an interface, a type alias, a method, a
function, or a lambda defines a set of non-generics classes, interfaces,
unions, arrays, methods, functions, or lambdas  respectively.

An explicit generic instantiation is a language construct, which provides a
list of *type arguments* (see :ref:`Type Arguments`) that specify real types or
type parameters to substitute corresponding type parameters of a generic:

.. code-block:: typescript
   :linenos:

    class G<T> {}    // Generic class declaration
    let x: G<number> // Explicit class instantiation, type argument provided

    class A {
       method <T> () {}  // Generic method declaration
    }
    let a = new A()
    a.method<string> () // Explicit method instantiation, type argument provided

    function foo <T> () {} // Generic function declaration
    foo <string> () // Explicit function instantiation, type argument provided

    type MyArray<T> = T[] // Generic type declaration
    let array: MyArray<boolean> = [true, false] // Explicit array instantiation, type argument provided

    class X <T1, T2> {}
    // Different forms of explicit instantiations of class X producing new generic entities
    class Y<T> extends X<number, T> { // class Y extends X instantiated with number and T
       f1: X<Object, T> // X instantiated with Object and T
       f2: X<T, string> // X instantiated with T and string
    }

    let lambda = <T> (p: T) => { console.log (p) } // Generic lambda defined
    lambda<string> ("string argument") // Generic lambda instantiated and called


.. index::
   instantiation
   generic entity
   non-generic entity
   function declaration
   type argument
   type parameter
   generic

In the explicit generic instantiation *G* <``T``:sub:`1`, ``...``, ``T``:sub:`n`>,
*G* is the generic declaration, and  <``T``:sub:`1`, ``...``, ``T``:sub:`n`> is
the list of its type arguments.

..
   lines 312, 314, 336 - initially the type was *T*:sub:`1`, ``...``, *T*:sub:`n`
   lines 321, 322 - initially *C*:sub:`1`, ``...``, *C*:sub:`n` and *T*:sub:`1`, ``...``, *T*:sub:`n` 

If type parameters *T*:sub:`1`, ``...``, *T*:sub:`n` of a generic
declaration are constrained by the corresponding ``C``:sub:`1`, ``...``,
``C``:sub:`n`, then *T*:sub:`i` is compatible with each constraint type
*C*:sub:`i` (see :ref:`Type Compatibility`). All subtypes of the type listed
in the corresponding constraint have each type argument *T*:sub:`i` of the
parameterized declaration ranging over them.

.. index::
   type argument
   type parameter
   generic declaration
   parameterized declaration
   constraint

A generic instantiation *G* <``T``:sub:`1`, ``...``, ``T``:sub:`n`> is
*well-formed* if **all** of the following is true:

-  The generic declaration name is *G*.
-  The number of type arguments equals the number of type parameters of *G*.
-  All type arguments are compatible with the corresponding type parameter
   constraint (see :ref:`Type Compatibility`).

A :index:`compile-time error` occurs if an instantiation is not well-formed.

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

.. _Implicit Generic Instantiations:

Implicit Generic Instantiations
===============================

.. meta:
    frontend_status: Done

In an *implicit* instantiation, type arguments are not specified explicitly.
Such type arguments are inferred (see :ref:`Type Inference`) from the context
the generic is referred in as in the example below:

.. code-block:: typescript
   :linenos:

    function foo <G> (x: G, y: G) {} // Generic declaration
    foo (new Object, new Object)     // Implicit generic instantiation
      // based on argument types the type argument is inferred

Implicit instantiation is only possible for functions and methods.

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

.. _Type Arguments:

Type Arguments
==============

.. meta:
    frontend_status: Done

Type arguments can be reference types or array types.

If a value type is specified as a type argument in the generic instantiation,
then the boxing conversion applies to the type (see :ref:`Boxing Conversions`).

.. code-block:: abnf

    typeArguments:
        '<' typeArgumentList '>'
        ;

A :index:`compile-time error` occurs if type arguments are omitted in a
parameterized function.

.. index::
   type argument
   reference type
   boxing conversion
   parameterized function
   compile-time error

.. code-block:: abnf

    typeArgumentList:
        typeArgument (',' typeArgument)*
        ;

    typeArgument:
        typeReference
        | arrayType
        ;

|

.. _Utility Types:

Utility Types
*************

|LANG| supports several embedded types, called *utility* types. Utility types
allow constructing new types by adjusting properties of the initial types.

.. index::
   embedded type
   utility type
   extended functionality

|

.. _Partial Utility Type:

Partial Utility Type
====================

.. meta:
    frontend_status: Done

Type ``Partial<T>`` constructs a type with all properties of ``T`` set to
optional. ``T`` must be a class or an interface type. No method (including any
getter or setter) of ``T`` is part of the ``Partial<T>`` type.

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

In the example above, type ``Partial<Issue>`` is transformed to a distinct but
analogous type:

.. code-block:: typescript
   :linenos:

    interface /*some name*/ {
        title?: string
        description?: string
    }

Type ``T`` is not compatible with ``Partial<T>`` (see :ref:`Type Compatibility`),
and variables of ``Partial<T>`` are to be initialized with valid object
literals.

**Note**: If class ``T`` has a user-defined getter, setter, or both, then none
of those is called when object literal is used with ``Partial<T>`` variables.
Object literal has its own built-in getters and setters to modify its variables:

.. code-block:: typescript
   :linenos:

    interface I {
        property: number
    }
    class A implements I {
        set property(property: number) { console.log ("Setter called") ... }
        get property(): number { console.log ("Getter called") ... }
    }
    function foo (partial: Partial<A>) {
        partial.property = 666 // setter to be called
        console.log(partial.property) // getter to be called
    }
    foo ({property: new SomeType}) // No getter or setter from class A is called
    // 666 is printed as object literal has its own setter and getter

|

.. _Required Utility Type:

Required Utility Type
=====================

.. meta:
    frontend_status: Done

Type ``Required<T>`` is opposite to ``Partial<T>``, and constructs a type with
all properties of ``T`` set to required (i.e., not optional). ``T`` must be a
class or an interface type. No method (including any getter or setter) of ``T``
is part of the ``Required<T>`` type.

.. code-block:: typescript
   :linenos:

    interface Issue {
        title?: string
        description?: string
    }

    let c: Required<Issue> = { // CTE: 'description' should be defined
        title: "aa"
    }



In the example above, type ``Required<Issue>`` is transformed to a distinct
but analogous type:

.. code-block:: typescript
   :linenos:

    interface /*some name*/ {
        title: string
        description: string
    }

Type ``T`` is not compatible (see :ref:`Type Compatibility`) with
``Required<T>``, and variables of ``Required<T>`` are to be initialized with
valid object literals.


|

.. _Readonly Utility Type:

Readonly Utility Type
=====================

.. meta:
    frontend_status: Done

Type ``Readonly<T>`` constructs a type with all properties of ``T`` set to
readonly. It means that the properties of the constructed value cannot be
reassigned. ``T`` must be a class or an interface type. No method (including
any getter or setter) of ``T`` is part of the ``Readonly<T>`` type.


.. code-block:: typescript
   :linenos:

    interface Issue {
        title: string
    }

    const myIssue: Readonly<Issue> = {
        title: "One"
    };

    myIssue.title = "Two" // compile-time error: readonly property

Type ``T`` is compatible (see :ref:`Type Compatibility`) with ``Readonly<T>``,
and allows assignments as a consequence:


.. code-block:: typescript
   :linenos:

    class A {
       f1: string = ""
       f2: number = 1
       f3: boolean = true
    }
    let x = new A
    let y: Readonly<A> = x // OK


|

.. _Record Utility Type:

Record Utility Type
===================

.. meta:
    frontend_status: Partly
    todo: support literals in Record types - #13645

Type ``Record<K, V>`` constructs a container that maps keys (of type ``K``)
to values (of type ``V``).

Type ``K`` is restricted to ``number`` types, type ``string``, union types
constructed from these types, and literals of these types.

A :index:`compile-time error` occurs if any other type, or literal of any other
type is used in place of this type:

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

There are no restrictions on type ``V``.

A special form of object literals is supported for instances of type ``Record``
(see :ref:`Object Literal of Record Type`).

Access to ``Record<K, V>`` values is performed by an *indexing expression*
like *r[index]*, where *r* is an instance of type ``Record``, and *index*
is the expression of type ``K``. The result of an indexing expression is of type
``V`` if ``K`` is a union that contains literal types only. Otherwise, it is of
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

In the example above, ``K`` is a union of literal types. The result of an
indexing expression is of type ``V``. In this case it is ``number``.

|

.. _Utility Type Private Fields:

Utility Type Private Fields
===========================

.. meta:
    frontend_status: Done

As utility types are built on top of other types private fields of the initial
type stay in the utility type but they are not accessible and cannot be
accessed in any form.

.. code-block:: typescript
   :linenos:
   
   function foo(): string {  // Potentially some side effect 
      return "private field value"
   }

   class A {
      public_field = 444
      private private_field = foo()
   }

   function bar (part_a: Readonly<A>) {
      console.log (part_a)
   }

   bar ({public_field: 777}) // OK, object literal has no field `private_field`
   bar ({public_field: 777, private_field: ""}) // compile-time error, incorrect field name

   bar (new A) // OK, object of type Readonly<A> has field `private_field`

.. raw:: pdf

   PageBreak


