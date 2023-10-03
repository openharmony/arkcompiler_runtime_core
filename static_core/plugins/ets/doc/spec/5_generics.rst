.. _Generics:

Generics
########

.. meta:
    frontend_status: Partly

Class, interface, method, constructor and function are program entities
that can be generalized in the |LANG| language. Generalization is
parametrizing an entity by one or several types. A generalized
entity is introduced by a *generic declaration* (also called *generic*
for brevity).

.. index::
   entity
   generalization
   parameterization
   generic declaration
   generic

.. _Generic Declarations:

Generic Declarations
********************

.. meta:
    frontend_status: Partly

Types used as generic parameters in a generic are called *type parameters*.

A generic must be instantiated in order to be used.
*Generic instantiation* is the action that converts a generic into
a *real* program entity: ordinary class, interface, function, etc.
Instantiation can be performed either explicitly or implicitly.

*Explicit* generic instantiation is the language construct that specifies
real types, which substitute type parameters of a generic. Real types
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

In an *implicit* instantiation, type arguments are not specified explicitly,
but are inferred from the context the generic is referred in.
Implicit instantiation is possible only for functions and methods.

The result of instantiation is a *real*, non-parametrized program entity:
class, interface, method, constructor, or function. Such entity is treated
exactly as an ordinary class, interface, method, constructor, or function.

Conceptually, a generic class, an interface, a method, a constructor or a
function defines a set of classes, interfaces, methods, constructors, or
functions respectively (see :ref:`Generic Instantiations`).

.. index::
   implicit instantiation
   instantiation
   type argument
   context
   non-parametrized entity
   method
   class
   interface
   constructor
   function

.. _Generic Parameters:

Generic Parameters
******************

.. meta:
    frontend_status: Done

A class, an interface, or a function must be parameterized by at least one
type parameter to be *generic*. The type parameter is declared in the type
parameter section, and can be used as an ordinary type inside a generic.

Syntactically, the type parameter is an unqualified identifier.
For the scope of type parameters see :ref:`Scopes`.

Each type parameter has a *constraint* (see :ref:`Type Parameter Constraint`).

Type parameters can also have default types (see :ref:`Type Parameter Default`).

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
        'extends' typeReference
        ;

    typeParameterDefault:
        '=' typeReference
        ;

A generic class, interface, method, constructor, or function defines a set
of parameterized classes, interfaces, methods, constructors, or functions
respectively (see :ref:`Generic Instantiations`). One type argument can only
define one such set for each possible parametrization of the type parameter
section.

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
    todo: Checking of bounaries on call site
    todo: Further checks on multiple parameter bounds

A type parameter can be restricted. In that case, the corresponding type
argument in the generic instantiation must follow some restrictions, or
*constraints*.

Every type parameter’s constraint follows the keyword ``extends``.
A constraint is denoted as a single type parameter *T*; if no constraint
is declared, then the type is assumed to be ``Object | null``.
If the type parameter *T* has the type constraint *S*, then the actual
type of the generic instantiation must be a subtype of *S*.

.. index::
   type parameter constraint
   keyword extends
   type argument
   generic instantiation
   instantiation
   constraint
   subtype

Example:

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived { }
    class SomeType { }
    
    class G<T extends Base> { }
    
    let x: G<Base>      // correct
    let y: G<Derived>   // also correct
    let z: G<SomeType>  // error: SomeType is not a subtype of Base

A type parameter of a generic can *depend* on some other type parameter
of the same generic.

If *S* constrains *T*, then the type parameter *T* *directly depends*
on the type parameter *S*, while *T* directly depends on:

-  *S*; or
-  A type parameter *U* that depends on *S*.

A compile-time error occurs if a type parameter in the type parameter
section depends on itself.

.. index::
   type parameter
   generic declaration
   type parameter
   unqualified identifier
   scope
   generic declaration
   constraint
   compile-time error

|

.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived { }
    class SomeType { }
  
    class G<T, S extends T> {}
    
    let x: G<Base,Derived>  // correct: the second argument directly
                            // depends on the first one
    let y: G<Base,SomeType> // error: SomeType doesn't depend on Base

|

.. _Generic Instantiations:

Generic Instantiations
**********************

.. meta:
    frontend_status: Partly

As mentioned before, a generic class, interface, or function declaration
defines a set of corresponding non-generic entities. A generic  entity
must be *instantiated* in order to get a non-generic entity out of it.
The instantiation is specified by providing a list of *type arguments*
that substitute corresponding type parameters of the generic:

.. index::
   instantiation
   generic entity
   non-generic entity
   function declaration
   type argument
   type parameter
   generic

*G* < *T*:sub:`1`, ``...``, *T*:sub:`n`>

where <*T*:sub:`1`, ``...``, *T*:sub:`n`> is the list of type arguments
for the generic declaration *G*.

If *C*:sub:`1`, ``...``, *C*:sub:`n` is the constraint for the corresponding
type parameters *T*:sub:`1`, ``...``, *T*:sub:`n` of a generic declaration,
then *T*:sub:`i` is a subtype of each constraint type *C*:sub:`i`
(see :ref:`Subtyping`). All such subtypes of the type listed in the
corresponding constraint have each type argument *T*:sub:`i` of the
parameterized declaration ranging over them.

.. index::
   type argument
   type parameter
   generic declaration
   parameterized declaration
   subtype
   constraint

A generic instantiation *G* < *T*:sub:`1`, ``...``, *T*:sub:`n`> is
*well-formed* if all of the following is true:

-  The generic declaration name is *G*.
-  The number of type arguments equals that of *G*’s type parameters.
-  All type arguments are subtypes of a corresponding type parameter constraint.

A compile-time error occurs if an instantiation is not well-formed.

The generic version is included (unless explicitly excluded) with a class type,
an interface type, or a function in this specification.

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
    frontend_status: Partly

Type parameters of generic types can have defaults, and optional type
parameters are also supported. This situation allows dropping a type
argument when a particular type of instantiation is used.
However, a compile-time error occurs if a type parameter without a
default value follows a type parameter with a default value in the
declaration of a generic type.

The examples below illustrate this for both classes and functions.

.. index::
   type parameter
   generic type
   type argument
   default type parameter
   optional type parameter
   instantiation
   class
   function
   compile-time error
   default value

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

Type arguments can be reference types or wildcards.

If a value type is specified as a type argument in the generic instantiation,
then the boxing conversion applies to that type (see
:ref:`Predefined Numeric Types Conversions`).

.. code-block:: abnf

    typeArguments:
        '<' typeArgumentList? '>'
        ;

A *typeArgument* denotes a raw type (see :ref:`Raw Types`) unless a
*typeArgumentList* is provided.

A compile-time error occurs if type arguments are omitted in a parametrized
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

The variance for type arguments can be specified with wildcards (*use-site
variance*). It allows specifying the type variance of the corresponding type
argument, and changing the type variance of an *invariant* type parameter.

**NOTE**: This description of use-site variance modifiers is tentative.
The details are to be specified in the future language versions.

The syntax to signify a covariant type argument, or a wildcard with an
upper bound (*T* is a *typeReference*) is as follows:

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

   This syntax restricts the methods available so that only the methods
   that do not use *T*, or use *T* in out-position can be accessed.

The syntax to signify a contravariant type argument, or a wildcard with a
lower bound (*T* is a *typeReference*) is as follows:

-  ``in`` *T*

   This syntax restricts the methods available so that only the methods
   that do not use *T*, or use *T* in in-position can be accessed.

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

-  A wildcard is used in a parameterization of a function;
-  A covariant wildcard is specified for a contravariant type parameter;
-  A contravariant wildcard is specified for a covariant type parameter.

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

-  *T* <out *A*> <: *T* <out *B*>
-  *T* <in *A*> :> *T* <in *B*>
-  *T* <*A*> <: *T* <out *A*>
-  *T* <*A*> <: *T* <in *A*>

.. index::
   subtyping
   invariant type parameter
   use-site variance

Any two type arguments are considered *provably distinct* if:

-  The two arguments are not of the same type, and neither is a type parameter
   nor a wildcard; or
-  One type argument is a type parameter, or a wildcard with an upper bound
   of *S*, the other *T* is not a type parameter and not a wildcard, and
   neither is a subtype of another (see :ref:`Subtyping`); or
-  Each type argument is a type parameter, or wildcard with upper bounds
   *S* and *T*, and neither is a subtype of another (see :ref:`Subtyping`).

.. index::
   provably distinct type argument
   type parameter
   wildcard
   subtype
   upper bound
   type argument

|

.. _Raw Types:

Raw Types
*********

.. code-block:: abnf

    rawType:
        identifier '<' '>'
        ;

**Note**: Raw types are added below to simplify the migration from other
languages that support this notion. Future versions of the language are
to disallow the use of raw types.

A raw type is one of the following:

-  The reference type formed by taking a generic type declaration’s name
   without the accompanying type argument list.
-  An array type with a raw type element.
-  A non-*static* member type of a raw type *R* not inherited from an
   *R*’s superclass or superinterface.

.. index::
   raw type
   migration
   reference type
   generic type declaration
   type argument
   array type
   raw type
   non-static member type
   inheritance
   superclass
   superinterface
   member type

Only a generic class, or interface type can be a raw type.

Raw type superclasses and superinterfaces are the raw versions of respective
superclasses and superinterfaces of any generic type instantiations.

The type of a constructor (see :ref:`Constructor Declaration`), instance method
(see :ref:`Instance Methods` for classes, :ref:`Interface Method Declarations`
and `Default Method Declarations` for interfaces), and non-*static* field (see
:ref:`Field Declarations`) of a raw type *C*, which is not inherited from
respective superclasses or superinterfaces, is the raw version of its type
in the generic declaration corresponding to *C*.

The type of a *static* method or *static* field of a raw type *C*, and the type
of a method or field in the generic declaration corresponding to *C* are the
same.

.. index::
   generic class
   interface type
   raw type
   raw type superclass
   raw type superinterface
   instantiation
   superclass
   superinterface
   generic type
   constructor
   instance method
   interface method declaration
   default method declaration
   interface
   non-static field
   field declaration
   inheritance
   static method
   static field
   generic declaration

A compile-time error occurs if:

-  Type arguments are passed to a non-*static* type member of a raw type
   that is not inherited from its superclasses or superinterfaces.
-  An attempt is made to use a type member of a parameterized type as a
   raw type.

.. index::
   compile-time error
   type argument
   non-static type member
   raw type
   inheritance
   superclass
   superinterface
   type member
   parameterized type

A class’ supertype can be a raw type.

Member access to a class is treated as normal, while member access to a
supertype is treated as that to a raw type. Calls to ``super`` are treated
as method calls to raw types in the class constructor.

The use of raw types is a concession for the sake of compatibility with
legacy code, and is to be disallowed in future versions of the language.

The use of a raw type always results in compile-time diagnostics in order
to ensure that potential typing rules violations are flagged.

.. index::
   supertype
   raw type
   member access
   class
   member access
   supertype
   call
   method call
   class constructor
   compatibility


|

.. _Utility Types:

Utility Types
*************

The |LANG| supports several embedded types, called "utility" types.
They allow to construct new types, and extend their functionality.

.. index::
   embedded type
   utility type
   extended functionality

|

.. _Partial Utility Type:

Partial Utility Type
====================

The type ``Partial<*T*>`` constructs a type with all properties of *T* set to
optional. *T* must be a class or an interface type.

.. index::
   interface type

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

In the example above, the type ``Partial<Issue>`` is transformed to a distinct
type that is analogous to the type:

.. code-block:: typescript
   :linenos:

    interface /*some name*/ {
        title?: string
        description?: string
    }

|

.. _Record Utility Type:

Record Utility Type
===================

The type *Record<K, V>* constructs a container that maps keys (of type *K*)
to values (of type *V*).

The type *K* is restricted to ``number`` types, ``string`` types, union types
constructed from these types, and also literals of such types.

A compile-time error occurs if any other type, or literal of any other type
is used as this type.

There are no restrictions on the type *V*. 

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


A special form of object literals is supported for instances of *Record*
types (see :ref:`Object Literal of Record Type`).

Accessing to ``Record<``*K*``, ``*V*``>`` values is done by the *indexing
expression* like *r[index]*, where *r* is an instance of the type ``Record``,
and *index* is the expression of the type *K*. The result of an indexing
expression is of the type *V*.

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

.. raw:: pdf

   PageBreak


