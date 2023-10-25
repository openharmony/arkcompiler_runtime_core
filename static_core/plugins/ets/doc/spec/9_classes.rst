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

.. _Classes:

Classes
#######

.. meta:
    frontend_status: Done

Class declarations introduce new reference types, and describe the manner
of their implementation.

Classes can be *top-level* and local (see :ref:`Local Classes`).

A class body declares:

-  Members,
-  Instance,
-  Static initializers, and
-  Constructors.


The body of the declaration of a member (see :ref:`Class Members`)
comprises the scope of a declaration (see :ref:`Scopes`).

Class members include:

-  Fields,
-  Methods, and
-  Accessors.

.. index::
   class declaration
   reference type
   implementation
   class body
   field
   method
   accessor
   constructor
   instance
   member
   static initializer
   scope

Class members can be *declared* or *inherited*.

Every member is associated with the class declaration it is declared in.

Field, method, accessor, and constructor declarations can have access modifiers
(see :ref:`Access Modifiers`):

-  *Public*,
-  *Protected*,
-  *Internal*, or
-  *Private*.


A newly declared field can hide a field declared in a superclass or
superinterface.

A newly declared method can hide, implement, or override a method
declared in a superclass or superinterface.

.. index::
   class declaration
   access modifier
   field declaration
   method declaration
   accessor declaration
   constructor declaration
   hiding
   implementation
   overriding
   superclass
   superinterface
   

|

.. _Class Declarations:

Class Declarations
******************

.. meta:
    frontend_status: Done

Every class declaration defines a **Class type**, i.e., a new named
reference type.

The class name is specified by the *identifier* inside a class declaration.

If *typeParameters* are defined in a class declaration, then that class
is a *generic class* (see :ref:`Generic Declarations`).

.. index::
   class declaration
   class type
   reference type
   identifier
   generic class
   scope
   
.. code-block:: abnf

    classDeclaration:
        classModifier? 'class' identifier typeParameters?
          classExtendsClause? implementsClause? classBody
        ;

    classModifier:
        'abstract' | 'final'
        ;

The scope of a class declaration is specified in :ref:`Scopes`.

.. code-block:: typescript
   :linenos:

    class Point {
      public x: number
      public y: number
      public constructor(x : number, y : number) {
        this.x = x
        this.y = y
      }
      public length(): number {
        return Math.sqrt(this.x * this.x + this.y * this.y)
      }
      static origin = new Point(0, 0)
    }

|

.. _Class Modifiers Abstract Classes:

Class Modifiers: Abstract Classes
=================================

.. meta:
    frontend_status: Done

A class that is incomplete or considered incomplete is *abstract*.

A non-abstract subclass of an *abstract* class can be instantiated; as a
result, a constructor for the *abstract* class, and field initializers
for non-static fields of that class are executed.

A method that is declared but not yet implemented is *abstract*. Only
*abstract* classes can have *abstract* methods.

A :index:`compile-time error` occurs if:

-  An attempt is made to create an instance of an *abstract* class.
-  A non-*abstract* class has an *abstract* method.

.. index::
   modifier
   abstract
   method
   non-abstract
   class
   subclass
   instance
   instantiation
   constructor
   initializer
   non-static
   field
   execution
   implementation
   abstract method
   final

|

.. _Class Modifiers Final Classes:

Class Modifiers: Final Classes
==============================

.. meta:
    frontend_status: Done

Final classes are described in the experimental section (see
:ref:`Final Classes`).

.. index::
   modifier
   class
   final

|

.. _Local Classes:

Local Classes
=============

Local classes are defined between balanced braces in a group of zero or more
statements (i.e., in a *block* that is a *method body*, a ``for`` loop, or an
``if`` clause).

A local interface can be a normal interface, but not an annotation interface.
A local interface cannot declare static members.

.. index::
   local
   statement
   block
   method body
   loop
   clause
   static
   annotation
   interface
   
Local classes have access to instance members of the enclosing class and local
variables if such are declared *constant* (i.e., a variable or parameter whose
value remains unchanged after initialization).

A local class captures a local variable, or the parameter it accesses of the
enclosing function or method.

.. index::
   local
   class
   instance
   enclosing class
   enclosing function
   enclosing method
   variable
   access
   initialization
   constant
   parameter
   value

A local class can only:

-  Declare members or initializers.
-  Refer to a *static* member of the enclosing class in a *static* method
   (static members must be *constant variables*, i.e., variables of a primitive
   type, or type *String* that is declared *constant* and initialized with
   a compile-time constant expression).
-  Be referred to by a simple name (neither a qualified nor a canonical
   name), i.e., if a canonical name is required, then a local class cannot
   be considered.


A :index:`compile-time error` occurs if a local class or interface declaration
has:

-  A variable that is not constant.
-  A member variable that is not defined as *static*.
-  A name that is used to declare a new local class or interface (unless that
   local class or interface is declared within a class or interface declaration).
-  A local class or an interface declaration that has access modifier *public*,
   *protected*, or *private*.

.. index::
   local
   class
   initializer
   static
   enclosing class
   compile-time constant expression
   interface
   constant variable
   primitive type
   string
   simple name
   qualified name
   canonical name
   declaration
   access modifier
   
A :index:`compile-time error` occurs if the direct superclass of a local
class is *final*.

A local class cannot be nested.

A local class and interface declarations are not statements but must also be
immediately contained by a block.

.. index::
   class
   final
   local
   
The scope of a local class declaration encompasses its entire declaration (not
its body only), i.e., the definition of the local class *Cyclic* is indeed
cyclic because it extends itself rather than *Global.Cyclic*. Consequently,
the declaration of the local class *Cyclic* is rejected at compile time.

*Local* class names cannot be redeclared within the same method (constructor,
initializer, or function as the case may be); a :index:`compile-time error`
occurs if a method uses the declaration *local* more than once.

.. index::
   declaration
   declaration body
   class
   local
   compile time

|

.. _Class Extension Clause:

Class Extension Clause
======================

.. meta:
    frontend_status: Done

All classes except class *Object* can contain the *extends* clause which
specifies the *base class*, or the *direct superclass* of the current class.
A class that has no *extends* clause, and is not *Object*, is assumed to have
the *extends* *Object* clause.

.. index::
   class
   Object
   clause
   direct superclass
   base class
   

.. code-block:: abnf

    classExtendsClause:
        'extends' typeReference
        ;

A :index:`compile-time error` occurs if:

-  *extends* clause appears in the definition of the class *Object*
   which is the top of the types hierarchy, and has no superclass.

-  *typeReference* names a class type that is not accessible (see
   :ref:`Scopes`).

-  There is a cycle in the ‘extends’ graph.

-  *typeReference* is an alias of a *primitive* or *enum* type.

-  Any of the type arguments of *typeReference* is a wildcard type argument.


Class extension implies that a class inherits all members of the direct
superclass, while private members are not accessible within the current class.

.. index::
   class
   Object
   superclass
   type
   enum type
   primitive type
   class type
   extends clause
   extends graph
   wildcard
   type argument
   inheritance

.. code-block:: typescript
   :linenos:

    class Base {
      // All methods are mutually accessible in the class where
          they were declared
      public publicMethod () {
        this.protectedMethod()
        this.privateMethod()
      }
      protected protectedMethod () {
        this.publicMethod()
        this.privateMethod()
      }
      private privateMethod () {
        this.publicMethod();
        this.protectedMethod()
      }
    }
    class Derived extends Base {
      foo () {
        this.publicMethod()    // OK
        this.protectedMethod() // OK
        this.privateMethod()   // compile-time error: no such
            method
      }
    }

The transitive closure of a *direct subclass* relationship is the *subclass*
relationship. Class *A* can be a subclass of class *C* if:

-  *A* is the direct subclass of *C*; or

-  There is some class *B* of which *A* is a subclass, and *B* is in turn a
   subclass of *C* (the definition applies recursively).


Class *C* is a *superclass* of class *A* if *A* is its subclass.

.. index::
   transitive closure
   direct subclass
   subclass relationship
   subclass
   class

|

.. _Class Implementation Clause:

Class Implementation Clause
===========================

.. meta:
    frontend_status: Done

The names of interfaces that are direct superinterfaces of a declared
class are listed in the class declaration of the *implements* clause.

.. code-block:: abnf

    implementsClause:
        'implements' interfaceTypeList
        ;

    interfaceTypeList:
        typeReference (',' typeReference)*
        ;

A :index:`compile-time error` occurs if:

-  *typeReference* fails to name an accessible interface type (see
   :ref:`Scopes`).

-  Any type argument of *typeReference* is a wildcard type argument.

-  An interface is repeated as a direct superinterface in a single
   *implements* clause (even if that interface is named differently).

.. index::
   class declaration
   implementation
   accessible interface type
   type argument
   wildcard
   interface
   direct superinterface
   implements clause

For the class declaration *C* <*F*:sub:`1`,..., *F*:sub:`n`> (:math:`n\geq{}0`,
:math:`C\neq{}Object`):

- Direct superinterfaces of the class type *C* <*F*:sub:`1`,..., *F*:sub:`n`>
  are the types specified in the *implements* clause of the declaration of *C*
  (if there is the *implements* clause).


For a generic class declaration *C* <*F*:sub:`1`,..., *F*:sub:`n`> (*n* > *0*):

-  *Direct superinterfaces* of the parameterized class type *C*
   < *T*:sub:`1`,..., *T*:sub:`n`> are all types *I* <*U*:sub:`1`:math:`\theta{}`
   ,..., *U*:sub:`k`:math:`\theta{}`>, if:

    - *T*:sub:`i` (:math:`1\leq{}i\leq{}n`) is a type;
    - *I* <*U*:sub:`1`,..., *U*:sub:`k`> is the direct superinterface of
      *C* <*F*:sub:`1`,..., *F*:sub:`n`>; and
    - :math:`\theta{}` is the substitution [*F*:sub:`1`:= *T*:sub:`1`,...,
      *F*:sub:`n`:= *T*:sub:`n`].

.. index::
   class declaration
   parameterized class type
   generic class
   direct superinterface
   implements clause

Interface type *I* is a superinterface of class type *C* if *I* is:

-  A direct superinterface of *C*; or
-  A superinterface (see :ref:`Superinterfaces and Subinterfaces` defines
   superinterface of an interface) of *J* which is in turn a direct
   superinterface of *C*; or
-  A superinterface of the direct superclass of *C*.


A class *implements* all its superinterfaces.

A :index:`compile-time error` occurs if a class is at the same time a
subtype of:

-  Two interface types that represent different instantiations of the same
   generic interface (see :ref:`Generic Declarations`); or
-  The instantiation of a generic interface, and a raw type that names the
   a generic interface.

.. index::
   class type
   direct superinterface
   superinterface
   interface
   superclass
   class
   subtype
   interface type
   instantiation
   generic interface
   raw type

Non-*abstract* classes are not allowed to have *abstract* methods (see
:ref:`Abstract Methods`).

If a class is not declared *abstract*, then:

-  Any *abstract* member method of each direct superinterface is implemented
   (see :ref:`Overriding by Instance Methods`) by a declaration in that class;
-  The declaration of the existing method is inherited from a direct superclass,
   or a direct superinterface.


If a default method (see `Default Method Declarations`) of a class
superinterface is not inherited, then that default method can:

-  Be overridden by a class method, and
-  Behave as specified in its default body.


A single method declaration in a class is allowed to implement methods of one
or more superinterfaces.

A :index:`compile-time error` occurs if the names of a class field, and of
the method from one of superinterfaces that class implements are the same.

.. index::
   class type
   abstract class
   abstract method
   superinterface
   implementation
   overriding
   declaration
   class field
   method declaration
   inheritance
   superclass
   implementation
   method body

|

.. _Implementing Interface Properties:

Implementing Interface Properties
=================================

.. meta:
    frontend_status: Partly

A class must implement all properties from all interfaces (see
:ref:`Implementing Interface Properties`) which are defined as a getter, a
setter, or both. Providing implementation for the property in the form of
a field is not necessary.

.. code-block:: typescript
   :linenos:

    interface Style {
      get color(): string
      set color(s: string)
    }

    class StyleClassOne implements Style {
      color: string = ""
    }

    class StyleClassTwo implements Style {
      private color_: string = ""

      get color(): string {
        return this.color_
      }

      set color(s: string) {
        this.color_ = s
      }
    }

.. index::
   class
   implementation
   getter
   setter
   field

|

.. _Class Body:

Class Body
**********

.. meta:
    frontend_status: Partly
    todo: inner class, inner interface, inner enum declaration

A *class body* can contain declarations of members: fields, methods, accessors,
types (classes and interfaces), declarations of constructors and static
initializers for the class.

.. code-block:: abnf

    classBody:
        '{' 
           classBodyDeclaration* classInitializer? classBodyDeclaration*
        '}'
        ;

    classBodyDeclaration:
        accessModifier?
        ( constructorDeclaration
        | classFieldDeclaration
        | classMethodDeclaration
        | classAccessorDeclaration
        )
        ;

Any declaration within the class (inherited or immediately declared) has
a class scope fully defined in :ref:`Scopes`.

.. index::
   class body
   declaration
   member
   field
   method
   accessor
   type
   class
   interface
   constructor
   static initializer
   inheritance
   scope

|

.. _Class Members:

Class Members
*************

.. meta:
    frontend_status: Done

The class type members are as follows:

-  Members inherited from their direct superclass (see :ref:`Inheritance`),
   except class *Object* that cannot have a direct superclass.
-  Members inherited from a direct superinterface (see
   :ref:`Superinterfaces and Subinterfaces`).
-  Members declared in the class body (see :ref:`Class Body`).


The class members declared *private* are not inherited by subclasses of
that class.

.. index::
   class type
   inheritance
   member
   direct superclass
   Object
   direct superinstance
   class body
   private
   subclass
   
Class members declared *protected* or *public* are inherited by subclasses
that are declared in a package other than the package containing the class
declaration.

Constructors and class initializers are not members, and cannot be inherited.

Members can be class field (see :ref:`Field Declarations`), method (see
:ref:`Method Declarations`), and accessors (see :ref:`Accessor Declarations`).
Method is an ordered 4-tuple consisting of type parameters, argument types,
return type, and *throws*/*rethrows* clause, where:

#. Type parameter is the declaration of any type parameters of the
   method member.
#. Argument type is a list of the types of arguments applicable to the
   method member.
#. Return type is the return type of the method member.
#. *throws* or *rethrows* clause is an indication of a member method’s
   ability to raise exception.


All names in the declaration scope (see :ref:`Scopes`) must be unique, i.e.,
fields and methods cannot have the same name.

.. index::
   class
   member
   protected
   public
   inheritance
   subclass
   package
   declaration
   constructor
   initializer
   field
   method
   accessor
   return type
   argument type
   throws clause
   rethrows clause
   4-tuple
   type parameter
   declaration scope

|

.. _Access Modifiers:

Access Modifiers
****************

.. meta:
    frontend_status: Partly

Access modifiers define how a class member or a constructor can be accessed.

Modifiers *private*, *internal*, *internal protected*, *protected*, or *public*
explicitly specify the desired accessibility of class members and constructors.

.. code-block:: abnf

    accessModifier:
        'private'
        | 'internal' 'protected'?
        | 'protected'
        | 'public'
        ;

If no explicit modifier is provided, then a class member or a constructor
is implicitly declared *public* by default.

.. index::
   access modifier
   member
   constructor
   private
   public
   accessibility

|

.. _Private Access Modifier:

Private Access Modifier
=======================

.. meta:
    frontend_status: Done
    todo: only parsing is implemented, but checking isn't implemented yet, need libpandafile support too

The modifier *private* indicates that a class member or a constructor is
accessible within their declaring class, i.e., *private* member or
constructor *m* declared in a class *C* can be accessed only within the
class body of *C*.

.. code-block:: typescript
   :linenos:

    class C {
      private count: number
      getCount(): number {
        return this.count // ok
      }
    }

    function increment(c: C) {
      c.count++ // compile-time error – 'count' is private
    }

.. index::
   modifier
   private
   class member
   constructor
   accessibility
   declaring class
   class body

|

.. _Internal Access Modifier:

Internal Access Modifier
========================

Final methods are described in the experimental section (see
:ref:`Internal Access Modifier Experimental`).

|

.. _Protected Access Modifier:

Protected Access Modifier
=========================

.. meta:
    frontend_status: Done

The modifier *protected* indicates that a class member or a constructor is
accessible only within its declaring class, and classes derived from that
declaring class, i.e., a protected member *M* declared in a class *C* can be
accessed only within the class body of *C*, or of a class derived from *C*.

.. code-block:: typescript
   :linenos:

    class C {
      protected count: number
       getCount(): number {
         return this.count // ok
       }
    }

    class D extends C {
      increment() {
        this.count++ // ok, D is derived from C
      }
    }

    function increment(c: C) {
      c.count++ // compile-time error – 'count' is not accessible
    }

.. index::
   modifier
   method
   protected
   constructor
   accessibility
   class body
   declaring class


A member or a constructor with both *internal* (see above) and *protected*
modifier can be accessed as *internal* or *protected*.

|

.. _Public Access Modifier:

Public Access Modifier
======================

.. meta:
    frontend_status: Done
    todo: spec needs to be clarified - "The only exception and panic here is that the type the member or constructor belongs to must also be accessible"

The modifier *public* indicates that a class member or a constructor can be
accessed everywhere, provided that the type that member or constructor
belongs to is also accessible.

.. index::
   modifier
   protected
   access
   public
   constructor

|

.. _Field Declarations:

Field Declarations
******************

.. meta:
    frontend_status: Partly
    todo: issue when accessing hidden super class property using super
    todo: more work - when interface fields are implemented

*Field declarations* are data members in class instances.

.. code-block:: abnf

    classFieldDeclaration:
        fieldModifier*
        ( variableDeclaration
        | constantDeclaration
        )
        ;

    fieldModifier:
        'static' | 'readonly'
        ;

A :index:`compile-time error` occurs if:

-  A field modifier is used more than once in a field declaration.
-  The name of a field declared in the body of a class declaration is already
   used for another field or method in the same declaration.

A field declared by a class with a certain name *hides* any accessible
declaration of fields if they have the same name in superclasses and
superinterfaces of the class.

.. index::
   field declaration
   data member
   class instance
   field modifier
   class declaration
   hiding
   access
   superclass
   superinterface
   class declaration body
   
If a hidden field is *static*, then it can be accessed with a superclass or
superinterface qualification. Otherwise, a field access expression with the
keyword *super* (see :ref:`Field Access Expressions`), or a cast to a
superclass type can be used.

A class inherits all non-*private* fields of the superclass and superinterfaces
from its direct superclass and direct superinterfaces if those are not hidden
by a declaration in the class and accessible (see :ref:`Scopes`) to code in the
class.

A subclass can access a *private* field of a superclass if both classes are
members of the same class. However, a subclass cannot inherit a private field.

A class can inherit more than one field or property with the same name from
its superinterfaces, or from both its superclass and superinterfaces. However,
a :index:`compile-time error` occurs if an attempt is made to refer
to such a field or property by its simple name within the body of the class.

The same field or property declaration can be inherited from an interface in
more than one way. In that case, the field or property is considered
to be inherited only once, and referring to it by its simple name causes no
ambiguity.

.. index::
   qualified name
   access
   class body
   hiding
   hidden field
   static field
   field access expression
   keyword super
   superclass
   type
   inheritance
   subclass
   private
   property declaration

|

.. _Static Fields:

Static Fields
=============

.. meta:
    frontend_status: Done

A *static field* is instantiated when the class is initialized, and is
always declared static. A *static field* can have only one instantiation,
irrespective of how many instances of that class (even if zero) are
eventually created.

A new field is called non-*static* if it is created for, and associated with
a newly-created instance of a class or its superclasses. A non-*static* field
is not declared *static*.

.. index::
   static field
   instantiation
   instance
   initialization
   class
   superclass
   non-static field

|

.. _Readonly Constant Fields:

Readonly (Constant) Fields
==========================

.. meta:
    frontend_status: Done

A *readonly field* has *readonly* modifier, and is initialized only once. No
change of its value is allowed after the initialization.

Static fields and non-*static* fields can be declared *readonly*.

A :index:`compile-time error` occurs unless:

-  A blank *readonly* field is initialized by a static field (see
   :ref:`Class Initializer`) of its declared class, if any.

-  A blank *readonly* non-static field is initialized as a result of execution
   of every class constructor (see :ref:`Constructor Declaration`).

A blank *readonly* non-static field is to be initialized as a result of
execution of any class constructor. Otherwise, a :index:`compile-time error`
occurs.

.. index::
   readonly field
   constant field
   initialization
   modifier
   static field
   non-static field
   execution
   constructor

|

.. _Field Initialization:

Field Initialization
====================

.. meta:
    frontend_status: Done

An initializer in a non-*static* field declaration has the semantics of
an assignment (see :ref:`Assignment`) to the declared variable.

The following rules apply to an initializer in a *static* field declaration:

-  A :index:`compile-time error` occurs if the initializer uses the keyword
   ``this`` or the keyword ``super`` while calling a method (see
   :ref:`Method Call Expression`), or accessing a field (see
   :ref:`Field Access Expressions`).
-  The initializer is evaluated, and the assignment is performed only once
   when the class is initialized at runtime.


**Note**: Constant fields are initialized before all other *static* fields.

Constant fields initialization never uses default values (see
:ref:`Default Values for Types`).

An initializer in a non-*static* field declaration:

-  Can use the keyword ``this`` to access or refer to the current object, and
   the keyword ``super`` to access a superclass object.
-  Is evaluated at runtime, and has its assignment performed each time an
   instance of the class is created.

.. index::
   initializer
   non-static field
   field declaration
   constant field
   initialization
   keyword this
   keyword super
   assignment
   variable
   access
   superclass
   object
   assignment
   evaluation
   creation
   access
   static field
   instance
   class

Additional restrictions (as specified in :ref:`Exceptions and Errors Inside Field Initializers`)
apply to variable initializers that refer to fields that cannot yet be
initialized.

References to a field (even if the field is in the scope) can be restricted.
The rules applying to the restrictions on forward references to fields (if the
reference textually precedes the field declaration) and self-references (if
the field is used within its own initializer) are provided below.

A :index:`compile-time error` occurs in a reference to a *static* field *f*
declared in class or interface *C* if:

-  such reference is used in *C*’s *static* initializer (see
   :ref:`Class Initializer`) or *static* field initializer (see
   :ref:`Field Initialization`);
-  such reference is used before *f*’s declaration, or within *f*’s own
   declaration initializer;
-  no such reference is present on the left-hand side of an assignment
   expression (see :ref:`Assignment`);
-  *C* is the innermost class or interface enclosing such reference.


A :index:`compile-time error` occurs in a reference to a non-*static* field *f*
declared in class *C* if:

-  such reference is used in *C*’s non-*static* field initializer;
-  such reference is used before *f*’s declaration, or within *f*’s own
   declaration initializer;
-  no such reference is present on the left-hand side of an assignment
   expression (see :ref:`Assignment`);
-  *C* is the innermost class or interface enclosing such reference.

.. index::
   restriction
   exception
   error
   initializer
   variable
   field
   interface
   expression
   assignment
   reference
   non-static field
   static field
   innermost class
   innermost interface
   enclosing

|

.. _Method Declarations:

Method Declarations
*******************

.. meta:
    frontend_status: Done
    todo: spec issue: synchronized isn't specified at all, consequently noyt supported yet
    todo: spec issue: native and override are mutually exclusive - shouldn't be and used in stdlib
    todo: some corner cases needs to be fixed (revealed by CTS tests)

*Methods* declare executable code that can be called.

.. code-block:: abnf

    classMethodDeclaration:
        methodOverloadSignature*
        methodModifier* identifier signature block?
        ;

    methodModifier:
        'abstract'
        | 'static'
        | 'final'
        | 'override'
        | 'native'
        ;

Method *overload signatures* allow calling a method in different ways.

The *identifier* of *classMethodDeclaration* is the method name that can be
used to refer to the method (see :ref:`Method Call Expression`).

A :index:`compile-time error` occurs if:

-  A method modifier appears more than once in a method declaration.
-  The body of a class declaration declares a method if the method's name
   is already used for a field in this declaration.
-  The body of a class declaration declares two same-name methods with
   override-equivalent signatures (see :ref:`Override-Equivalent Signatures`)
   as its members.

.. index::
   method declaration
   overload signature
   identifier
   method
   method modifier
   class declaration
   override-equivalent signature
   class declaration body

|

.. _Class Static Methods:

Class (Static) Methods
======================

.. meta:
    frontend_status: Done

A method declared static is a *class method*.

A :index:`compile-time error` occurs if:

-  A method declaration contains another keyword (``abstract``, ``final``, or
   ``override``) along with the keyword ``static``.
-  The header or body of a class method includes the name of a surrounding
   declaration’s type parameter.


Class methods are always called with no reference to a particular object. That
is why a :index:`compile-time error` occurs if keywords ``this`` or ``super``
are used inside a static method.

.. index::
   static method
   keyword this
   keyword super
   keyword abstract
   keyword final
   keyword override
   keyword static
   class method header
   class method body
   type parameter

|

.. _Instance Methods:

Instance Methods
================

.. meta:
    frontend_status: Done

A method that is not declared *static* is called an *instance method*, or a
non-*static* method.

An instance method is always called with respect to an object, which becomes
the current object that the keyword ``this`` refers to during the execution
of the method body.

.. index::
   static method
   instance method
   non-static method
   keyword this
   method body

|

.. _Abstract Methods:

Abstract Methods
================

.. meta:
    frontend_status: Done

An *abstract* method declaration introduces the method as a member along
with its signature but without an implementation.

Non-*abstract* methods can be referred to as *concrete* methods.

A :index:`compile-time error` occurs if:

-  An *abstract* method is marked as *private*.
-  A method declaration contains another keyword (``static``, ``final``, or
   ``native``) along with the keyword ``abstract``.


A :index:`compile-time error` occurs unless:

-  The *abstract* method *m* declaration appears directly within an *abstract*
   class *A*.
-  Every non-*abstract* subclass of *A* (see
   :ref:`Class Modifiers Abstract Classes`) provides an implementation for *m*.

An *abstract* method can be overridden by another *abstract* method declaration
provided by an *abstract* class.

A :index:`compile-time error` occurs if an *abstract* method overrides a
non-*abstract* instance method.

.. index::
   abstract method declaration
   abstract method
   non-abstract instance method
   non-abstract method
   signature
   keyword abstract
   keyword static
   keyword final
   keyword native
   private
   abstract class
   overriding
   

|

.. _Final Methods:

Final Methods
=============

.. meta:
    frontend_status: Done

Final methods are described in the experimental section (see
:ref:`Native Methods Experimental`).

|

.. _Override Methods:

Override Methods
================

.. meta:
    frontend_status: Done

The keyword ``override`` indicates that an instance method in a superclass is
overridden by the corresponding instance method from a subclass (see
:ref:`Overriding by Instance Methods`).

The use of ``override`` is optional.

A :index:`compile-time error` occurs if:

-  Method marked with ``override`` does not override a method from a superclass.
-  Method declaration that contains the keyword ``override`` also contains
   keywords ``abstract`` or ``static``.


If the signature of the overridden method contains parameters with default
values (see :ref:`Optional Parameters`), then the overriding method always
uses the default parameter values of the overridden method.

A :index:`compile-time error` occurs if a parameter in an overriding method
contains the default value.

See :ref:`Overriding by Instance Methods` for the specific rules of overriding.

.. index::
   keyword override
   keyword abstract
   keyword static
   final method
   signature
   overriding
   method
   superclass
   instance
   subclass
   default value
   overridden method
   overriding method

|

.. _Native Methods:

Native Methods
==============

.. meta:
    frontend_status: Done

Native methods are described in the experimental section (see
:ref:`Native Methods Experimental`).

|

.. _Methods Overload Signatures:

Method Overload Signatures
==========================

The |LANG| allows specifying a method that can be called in different ways by
writing *overload signatures*, i.e., by writing several method headers which
have the same name and different signatures, and are followed by one
implementation function. See also :ref:`Function Overload Signatures` for
*function overload signatures*.

.. index::
   native method
   method overload
   overload signature
   implementation
   function overload signature
   method overload signature

.. code-block:: abnf

    methodOverloadSignature:
        methodModifier* identifier signature ';'
        ;

A :index:`compile-time error` occurs if the method implementation is not
present, or does not immediately follow the declaration.

A call of a method with overload signatures is always a call of the
implementation method.

The example below has two overload signatures defined: one is parameterless,
and the other has one parameter:

.. index::
   method implementation
   method declaration
   method overload signature
   overload signature

.. code-block:: typescript
   :linenos:

    class C {
        foo(): void; /*1st signature*/
        foo(x: string): void; /*2nd signature*/
        foo(x?: string): void {
            console.log(x)
        }
    }
    let c = new C()
    c.foo() // ok, 1st signature is used
    c.foo("aa") // ok, 2nd signature is used

The call of ``c.foo()`` is executed as a call of the implementation method with
the ``null`` argument, while the call of ``c.foo(x)`` is executed as a call of
the implementation method with an argument.

A :index:`compile-time error` occurs if the signature of method implementation
is not *overload signature-compatible* with each overload signature. It means
that a call of each overload signature must be replaceable for the correct
call of the implementation method. Using optional parameters (see
:ref:`Optional Parameters`) or *least upper bound* types (see
:ref:`Least Upper Bound`) can achieve this.
See :ref:`Overload Signature Compatibility` for the exact semantic rules.

A :index:`compile-time error` occurs unless all of the following requirements
are met:

-  Overload signatures and the implementation method have the same access
   modifier (*public*, *private*, or *protected*).
-  All overload signatures and the implementation method are *static* or
   *non-static*.
-  All overload signatures and the implementation method are *final* or
   *non-final*.
-  Overload signatures are not *native* (however, *native* implementation
   method is allowed).
-  Overload signatures are not *abstract*.

.. index::
   execution
   call
   signature
   overload signature-compatible
   overload signature
   access modifier
   public
   private
   protected
   abstract
   native implementation method
   final implementation method
   non-final implementation method
   static implementation method
   non-static implementation method
   least upper bound

|

.. _Method Body:

Method Body
===========

.. meta:
    frontend_status: Done

A *method body* is a block of code that implements a method. A semicolon, or
an empty body (i.e., no body at all) indicate the lack of implementation.

An *abstract* or *native* method must have an empty body.

A :index:`compile-time error` particularly occurs if:

-  The body of an *abstract* or *native* method declaration is a block.
-  A method declaration is neither *abstract* nor *native*, but its body
   is empty, or is a semicolon.


See :ref:`Return Statements` for the rules that apply to *return* statements
in a method body.

A :index:`compile-time error` occurs if a method is declared to have a return
type, but its body can complete normally (see :ref:`Normal and Abrupt Statement Execution`).

.. index::
   method body
   block
   implementation
   implementation method
   abstract method
   native method
   method declaration
   return statement
   return type
   
|

.. _Inheritance:

Inheritance
===========

.. meta:
    frontend_status: Done

Class *C* inherits from its direct superclass all concrete methods *m* (both
*static* and *instance*) that meet all of the following conditions:

-  *m* is a member of *C*’s direct superclass;
-  *m* is *public*, *protected*, or *internal* in the same package as *C*;
-  No signature of a method declared in *C* is a subsignature (see
   :ref:`Override-Equivalent Signatures`) of the signature of *m*.


Class *C* inherits from its direct superclass and direct superinterfaces all
*abstract* and *default* methods *m* (see `Default Method Declarations`)
that meet the following conditions:

-  *m* is a member of *C*’s direct superclass or direct superinterface *D*;
-  *m* is *public*, *protected*, or *internal* in the same package as *C*;
-  No method declared in *C* has a signature that is a subsignature (see
   :ref:`Override-Equivalent Signatures`) of the signature of *m*;
-  No signature of a concrete method inherited by *C* from its direct
   superclass is a subsignature of the signature of *m*;
-  No method :math:`m'` that is a member of *C*’s direct superclass or
   *C*’s direct superinterface *D*' (while :math:`m'` is distinct from *m*,
   and :math:`D'` from *D*) overrides the declaration of the method *m* from
   :math:`D'` (see :ref:`Overriding by Instance Methods` for class method
   overriding, and :ref:`Overriding by Instance Methods in Interfaces` for
   interface method overriding).


No class can inherit *private* or *static* methods from its superinterfaces.

.. index::
   inheritance
   direct superclass
   static method
   instance method
   public
   protected
   package
   signature
   subsignature
   override-equivalent signature
   default method
   abstract method
   direct superinterface
   interface method overriding
   private method
   static method

|

.. _Overriding by Instance Methods:

Overriding by Instance Methods
==============================

.. meta:
    frontend_status: Done

The instance method  *m*:sub:`C` (inherited by, or declared in class
*C*) overrides another method *m*:sub:`A` (declared in class *A*)
if **all** the following is true:

-  *C* is a subclass of *A*, and
-  *C* does not inherit *m*:sub:`A`, and
-  The signature of *m*:sub:`C` is a subsignature of the signature
   of *m*:sub:`A`,


and also if one of the following is also true:

-  *m*:sub:`A` is *public*, or
-  *m*:sub:`A` is *protected*, or
-  *m*:sub:`A` is *internal* in the same package as *C*, while:

    -  Either *C* declares *m*:sub:`C`, or
    -  *m*:sub:`A` is a member of the direct superclass of *C*,

-  *m*:sub:`A` is declared  with package access, and *m*:sub:`C` overrides:

    -  *m*:sub:`A` from a superclass of *C*, or
    -  method :math:`m'` from *C*, where :math:`m'` is distinct from both
         *m*:sub:`C` and *m*:sub:`A` (i.e., :math:`m'` overrides *m*:sub:`A`
         from a superclass of *C*).


.. index::
   instance method
   overriding
   subclass
   inheritance
   signature
   subsignature
   public
   protected
   abstract method
   non-abstract method
   implementation

Non-*abstract* *m*:sub:`C` implements *m*:sub:`A` from *C* if it overrides an
*abstract* method *m*:sub:`A`.

A :index:`compile-time error` occurs if the overridden method *m*:sub:`A` is
static.

An instance method *m*:sub:`C` (inherited by, or declared in class *C*)
overrides another method *m*:sub:`I` (declared in interface *I*) from *C* if:

-  *I* is a superinterface of *C*; and
-  *m*:sub:`I` is not static; and
-  *C* does not inherit *m*:sub:`I`; and
-  The signature of *m*:sub:`C` is a subsignature of the signature of
   *m*:sub:`I` (see :ref:`Override-Equivalent Signatures`); and
-  *m*:sub:`I` is *public*.


A method call expression (see :ref:`Method Call Expression`) containing the
keyword ``super`` can be used to access an overridden method.

Accessing an overridden method with a qualified name, or a cast to a superclass
type is not effective.

Among the methods that override each other, return types can vary if they are
reference types. The specialization of a return type to a subtype (i.e.,
*covariant returns*) is based on the concept of *return-type-substitutability*.

For example, the method declaration *d*:sub:`1` with return type *R*:sub:`1` is
*return-type-substitutable* for another method *d*:sub:`2` with return type
*R*:sub:`2` if:

-  *R*:sub:`1` is a primitive type (*R*:sub:`2` is then identical to
   *R*:sub:`1`); or

-  *R*:sub:`1` is a reference type (*R*:sub:`1` adapted to type parameters
   of *d*:sub:`2` is then a subtype of *R*:sub:`2`).

.. index::
   abstract method
   non-abstract method
   implementation
   overriding
   instance method
   superinterface
   static method
   inheritance
   signature
   subsignature
   keyword super
   qualified name
   overridden method
   superclass type
   return type
   reference type
   return-type-substitutability
   covariant return
   primitive type
   subtype
   type parameter
  
|

.. _Hiding by Class Methods:

Hiding by Class Methods
=======================

.. meta:
    frontend_status: Done

A *static* method *m* declared in, or inherited by a class *C* *hides* any
method :math:`m'` (where the signature of *m* is a subsignature of the
signature of :math:`m'` as described in :ref:`Override-Equivalent Signatures`)
in its superclasses and superinterfaces.

A hidden method is not directly accessible (see :ref:`Scopes`) to code in *C*.
However, a hidden method can be accessed by using a qualified name, or a method
call expression (see :ref:`Method Call Expression`) that contains the keyword
``super`` or a cast to a superclass type.

A :index:`compile-time error` occurs if a *static* method hides an *instance*
method.

.. index::
   hiding
   static method
   inheritance
   method
   signature
   override-equivalent signature
   superclass
   superinterface
   hidden method
   scope
   access
   qualified name
   method call expression
   keyword super
   superclass type
   instance method
   cast

|

.. _Requirements in Overriding and Hiding:

Requirements in Overriding and Hiding
=====================================

.. meta:
    frontend_status: Done

The method declaration *d*:sub:`1` with return type *R*:sub:`1` can override or
hide the declaration of another method *d*:sub:`2` with return type *R*:sub:`2`
if *d*:sub:`1` is return-type-substitutable (see
:ref:`Requirements in Overriding and Hiding` and
:ref:`Overriding by Instance Methods`) for *d*:sub:`2`. Otherwise, a
:index:`compile-time error` occurs.

A method that overrides or hides another method (including the methods that
implement *abstract* methods defined in interfaces) cannot change *throws* or
*rethrows* clauses of the overridden or hidden method.

A :index:`compile-time error` occurs if a type declaration *T* has a member
method *m*:sub:`1`, but there is also a method *m*:sub:`2`, declared in *T*
or a supertype of *T*, for which all of the following is true:

-  *m*\ :sub:`1`\ and *m*\ :sub:`2`\ use the same name; and
-  *m*\ :sub:`2`\ is accessible from *T* (see :ref:`Scopes`); and
-  *m*\ :sub:`1`\’s signature is not a subsignature (see
   :ref:`Override-Equivalent Signatures`) of *m*\ :sub:`2`\’s signature.

.. index::
   overriding
   hiding
   method declaration
   return type
   return-type-substitutability
   abstract method
   interface
   throws clause
   rethrows clause
   hidden method
   overridden method
   access
   signature
   subsignature
   override-equivalent signature

The access modifier of an overriding or hiding method must provide no less
access than was provided in the overridden or hidden method.

A :index:`compile-time error` occurs if:

-  The overridden or hidden method is *public*, and the overriding or hiding
   method is *not* *public*.
-  The overridden or hidden method is *protected*, and the overriding or hiding
   method is *not* *protected* or *public*.
-  The overridden or hidden method has *internal* access, and the
   overriding or hiding method is *private*.

.. index::
   overriding method
   hiding method
   access modifier
   overridden method
   hidden method
   public method
   protected method
   private method
   internal access

|

.. _Inheriting Methods with Override-Equivalent Signatures:

Inheriting Methods with Override-Equivalent Signatures
======================================================

.. meta:
    frontend_status: Done

A class can inherit multiple methods with override-equivalent signatures (see
:ref:`Override-Equivalent Signatures`).

A :index:`compile-time error` occurs if a class *C* inherits the following:

-  Concrete method whose signature is override-equivalent with another
   method that *C* inherited; or
-  Default method whose signature is override-equivalent with another method
   that *C* inherited, unless there is an abstract method, declared in a
   superclass of *C* and inherited by *C*, that is override-equivalent
   with both methods.


An *abstract* class can inherit all the methods, assuming that a set of
override-equivalent methods consists of at least one *abstract* method, and
zero or more default methods.

A :index:`compile-time error` occurs unless one of the inherited methods is
return-type-substitutable for every other inherited method (except *throws*
and *rethrows* clauses that cause no error in this case).

The same method declaration can be inherited from an interface in a number
of ways, causing no :index:`compile-time error` on its own.

.. index::
   inheriting method
   override-equivalent signature
   inheritance
   abstract method
   superclass
   return-type-substitutability
   inherited method
   throws clause
   rethrows clause
   interface
   method declaration

|

.. _Accessor Declarations:

Accessor Declarations
*********************

.. meta:
    frontend_status: Done

Accessors are often used instead of fields to add additional control for
operations of getting or setting a field value. An accessor can be either
a getter or a setter.

.. code-block:: abnf

    classAccessorDeclaration:
        accessorModifier
        ( 'get' identifier '(' ')' returnType block?
        | 'set' identifier '(' parameter ')' block?
        )
        ;

    accessorModifier:
        'abstract'
        | 'static'
        | 'final'
        | 'override'
        ;

Accessor modifiers are a subset of method modifiers. The allowed accessor
modifiers have exactly the same meaning as the corresponding method modifiers.
See :ref:`Abstract Methods` for *abstract*, :ref:`Class Static Methods` for
*static*, :ref:`Final Methods` for *final*, and :ref:`Override Methods` for
*override*.

.. index::
   access declaration
   field
   field value
   accessor
   getting
   setting
   getter
   setter
   expression
   accessor modifier
   method modifier
   abstract
   static method
   final method
   override method

.. code-block:: typescript
   :linenos:

    class Person {
      private _age: number = 0
      get age(): number { return this._age }
      set age(a: number) {
        if (a < 0) { throw new Error("wrong age") }
        this._age = a
      }
    }

Each *get* accessor (getter) must have neither parameters nor an explicit
return type.
Each *set* accessor (setter) must have a single parameter and no return value.

The use of getters and setters looks the same as the use of fields.

.. code-block:: typescript
   :linenos:

    class Person {
      private _age: number = 0
      get age(): number { return this._age }
      set age(a: number) {
        if (a < 0) { throw new Error("wrong age") }
        this._age = a
      }
    }

    let p = new Person()
    p.age = 25 // setter is called
    if (p.age > 30) { // getter is called
      // do something
    }

A class can define a getter, a setter, or both. If both a getter and a
setter are defined, then they must have the same accessor modifiers.
Otherwise, a :index:`compile-time error` occurs.

Accessors can be backed by a private field (as in the example above),
or have no such backing.

.. index::
   accessor
   getter
   setter
   explicit return type
   return value
   parameter
   private field
   class
   accessor modifier

.. code-block:: typescript
   :linenos:

    class Person {
      name: string = ""
      surname: string = ""
      get fullName(): string {
        return this.surname + " " + this.name
      }
    }

|

.. _Class Initializer:

Class Initializer
*****************

.. meta:
    frontend_status: Done

When a class is initialized, the *class initializer* declared in the class
is executed. Class initializers (along with field initializers for static
fields as described in :ref:`Field Initialization`) ensure that all static
fields receive their initial values before the first use.

.. code-block:: typescript
   :linenos:

    classInitializer
        : 'static' block
        ;

A :index:`compile-time error` occurs if a class initializer contains:

-  A *return <expression>* statement (see :ref:`Return Statements`).
-  A ``throw`` statement (see :ref:`Throw Statements`) with no ``try``
   statement (see :ref:`Try Statements`) to handle the surrounding context.
-  Keywords ``this`` (see :ref:`this Expression`) or ``super`` (see
   :ref:`Method Call Expression` and :ref:`Field Access Expressions`), or any
   type of a variable declared outside the class initializer.


Restrictions of class initializers’ ability to refer to static fields (even
those within the scope) are specified in :ref:`Exceptions and Errors Inside Field Initializers`.
Class initializers cannot throw exceptions for they are effectively
non-throwing functions (see :ref:`Non-Throwing Functions`).

.. index::
   class initializer
   execution
   static field
   field initialization
   initial value
   return expression statement
   throw statement
   try statement
   keyword this
   keyword super
   method call
   field access
   restriction
   scope
   exception
   error
   non-throwing function

|

.. _Constructor Declaration:

Constructor Declaration
***********************

.. meta:
    frontend_status: Done
    todo: Explicit Constructor Call - "Qualified superclass constructor calls" - not implemented, need more investigation (inner class)

*Constructors* are used to create objects that are instances of class.

.. code-block:: abnf

    constructorDeclaration:
        'constructor' '(' parameterList? ')' throwMark? constructorBody
        ;

A constructor declaration starts with the keyword ``constructor``, and has no
name. In any other respect, a constructor declaration is similar to a method
declaration with no result.

Constructors are called by class instance creation expressions (see
:ref:`New Expressions`), by conversions and concatenations caused by the string
concatenation operator ':math:`+`' (see :ref:`String Concatenation`), and by
explicit constructor calls from other constructors (see :ref:`Constructor Body`).

Access to constructors is governed by access modifiers (see
:ref:`Access Modifiers` and :ref:`Scopes`). Declaring a constructor
inaccessible can prevent class instantiation.

A :index:`compile-time error` occurs if two constructors in a class are
declared, and have identical signatures.

See :ref:`Throwing Functions` for ``throws`` mark, and
:ref:`Rethrowing Functions` for ``rethrows`` mark.

.. index::
   constructor
   constructor declaration
   object
   creation
   instance
   instance creation
   instance creation expression
   expression
   class
   keyword constructor
   class instance
   concatenation
   conversion
   string concatenation operator
   explicit constructor call
   throwing function
   rethrowing function
   throws mark
   rethrows mark
   scope
   access modifier
   access
   class instantiation
   signature

|

.. _Formal Parameters:

Formal Parameters
=================

.. meta:
    frontend_status: Done

The syntax and semantics of a constructor’s formal parameters are identical
to those of a method.

|

.. _The Type of a Constructor:

The Type of a Constructor
=========================

.. meta:
    frontend_status: Done

A constructor type consists of its signature and optional *throw* or
*rethrow* clauses.

.. index::
   constructor parameter
   constructor type
   signature
   throws clause
   rethrows clause

|

.. _Constructor Body:

Constructor Body
================

.. meta:
    frontend_status: Done

The first statement in a constructor body can be an explicit call of another
same-class constructor, or of the direct superclass (see
:ref:`Explicit Constructor Call`).

.. code-block:: abnf

    constructorBody:
        '{' constructorCall? statement* '}'
        ;

    constructorCall:
        'this' arguments
        | 'super' arguments
        ;

.. code-block:: typescript
   :linenos:

    class Point {
      x: number
      y: number
      constructor(x: number, y: number) {
        this.x = x
        this.y = y
      }
    }

    class ColoredPoint extends Point {
      static readonly WHITE = 0
      static readonly BLACK = 1
      color: number
      constructor(x: number, y: number, color: number) {
        super(x, y) // calls base class constructor
        this.color = color
      }
    }

.. index::
   statement
   constructor body
   explicit call
   constructor
   direct superclass

A :index:`compile-time error` occurs if a constructor calls itself, directly or
indirectly---through a series of one or more explicit constructor calls---by
using ``this``.

The constructor body must implicitly begin with a superclass constructor
call '``super()``' (call of the constructor’s direct superclass that takes
no argument), unless the constructor body begins with an explicit constructor
call, and the constructor being declared is a part of the primordial class
*Object*.

A constructor body looks like a method body (see :ref:`Method Body`), except
that explicit constructor calls are possible, and explicit returning of a value
(see :ref:`Return Statements`) is prohibited.

However, a return statement (:ref:`Return Statements`) can be used in a
constructor body unless it includes an expression.

.. index::
   constructor call
   constructor body
   superclass
   direct superclass
   argument
   primordial class
   Object
   method body
   return statement
   expression
   this
   super()

|

.. _Explicit Constructor Call:

Explicit Constructor Call
=========================

.. meta:
    frontend_status: Done

There are two kinds of explicit constructor call statements:

-  *Alternate constructor calls* that begin with the keyword ``this``, and
   can be prefixed with explicit type arguments (used to call an alternate
   same-class constructor).
-  *Superclass constructor calls* (used to call a direct superclass
   constructor) called *Unqualified superclass constructor calls* that
   start with the keyword ``super``, and can be prefixed with explicit type
   arguments.


A :index:`compile-time error` occurs if the constructor body of an explicit
constructor call statement:

-  Refers to any non-static field or instance method; or
-  Uses ``this`` or ``super`` in any expression.

.. index::
   constructor call
   constructor call statement
   alternate constructor call
   keyword this
   superclass constructor call
   direct superclass constructor
   unqualified superclass constructor call statement
   keyword super
   prefix
   explicit type argument
   constructor body
   non-static field
   instance method
   superclass
   expression
   instantiation
   enclosing
   qualified superclass constructor call statement
   static context
   

An ordinary method call evaluates an alternate constructor call statement
left-to-right. The evaluation starts from arguments, proceeds to constructor,
and then the constructor is called.

The process of evaluation of a superclass constructor call statement is
performed as follows:

.. index::
   expression
   qualified superclass constructor call statement
   subclass
   access
   scope
   method call
   evaluation
   alternate constructor call statement
   argument
   constructor
   superclass constructor call statement

#. If instance *i* is created, then the following procedure is used to
   determine *i*'s immediately enclosing instance with respect to *S*
   (if available):

   -  If the declaration of *S* occurs in a static context, then *i* has no
      immediately enclosing instance with respect to *S*.

   -  If the superclass constructor call is unqualified, then *S* must be a
      local class.

      If *S* is a local class, then the immediately enclosing type declaration
      of *S* is *O*.

      If *n* is an integer (:math:`n\geq{}1`), and *O* is the *n*’th
      lexically enclosing type declaration of *C*, then *i*'s immediately
      enclosing instance with respect to *S* is the *n*’th lexically enclosing
      instance of ``this``.

.. index::
   instance
   creation
   enclosing instance
   static context
   superclass constructor call
   qualified superclass constructor call
   unqualified superclass constructor call
   enclosing type declaration
   integer
   lexically enclosing type declaration
   lexically enclosing instance
   expression
   evaluation

#. After *i*'s immediately enclosing instance with respect to *S* (if available)
   is determined, the evaluation of the superclass constructor call statement
   continues left-to-right. The arguments to the constructor are evaluated, and
   then the constructor is called.

#. If the superclass constructor call statement completes normally after all,
   then all non-static field initializers of *C* are executed. *I* is executed
   before *J* if a non-static field initializer *I* textually precedes another
   non-static field initializer *J*.


   Non-static field initializers are executed if the superclass constructor
   call:
   
   -  Has an explicit constructor call statement; or
   -  Is implicit.


   An alternate constructor call does not perform the implicit execution.

.. index::
   immediately enclosing instance
   evaluation
   superclass constructor call
   superclass constructor call statement
   argument
   constructor
   non-static field initializer
   execution
   alternate constructor call statement

.. _Default Constructor:

Default Constructor
===================

.. meta:
    frontend_status: Done

If a class contains no constructor declaration, then a default constructor
is implicitly declared.
Such a constructor provides default values to class fields with
default values.
The default constructor for a top-level class or local class
has the following form:

-  The access modifier of the default constructor and of the class is the same
   (if the class has no access modifier, then the default constructor has the
   *internal* access (see :ref:`Scopes`).

-  The default constructor has no *throws* or *rethrows* clauses.

-  If the primordial class *Object* is being declared, then the body of the
   default constructor is empty. Otherwise, the default constructor only
   calls the superclass constructor with no arguments.

A :index:`compile-time error` occurs if a default constructor is implicit, but
the superclass has no accessible constructor that:

-  Takes no argument; and
-  Has no *throws* or *rethrows* clauses.

.. index::
   default constructor
   constructor declaration
   field
   default value
   top-level class
   local class
   access modifier
   internal access
   throws clause
   rethrows clause
   primordial class
   Object
   accessible constructor

.. raw:: pdf

   PageBreak


