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

.. _Recipes:

Recipes
=======

.. _R001:

|CB_R| Objects with property names that are not identifiers are not supported
-----------------------------------------------------------------------------

|CB_RULE| ``arkts-identifiers-as-prop-names``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: LiteralAsPropertyName, ComputedPropertyName
    :fix: Replace property name with identifier

|CB_ERROR|

|LANG| does not support Objects with name properties that are numbers, or
computed values. String literals and constant enum string members are allowed
as property names.

Use classes to access data by property names.
Use arrays to access data by numeric indices.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    var x = {"name": 1, 2: 3}

    console.log(x["name"])
    console.log(x[2])

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class X {
        public name: number = 0
    }
    let x:X = {name: 1}
    console.log(x.name)

    let y = [1, 2, 3]
    console.log(y[2])

    // If you still need a container to store keys of different types,
    // use Map<Object, some_type>:
    let z = new Map<Object, number>()
    z.set("name", 1)
    z.set(2, 2)
    console.log(z.get("name"))
    console.log(z.get(2))

|CB_SEE|
~~~~~~~~

* :ref:`R002`
* :ref:`R029`
* :ref:`R059`
* :ref:`R060`
* :ref:`R066`
* :ref:`R144`

.. _R002:

|CB_R| ``Symbol()`` API is not supported
----------------------------------------

|CB_RULE| ``arkts-no-symbol``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SymbolType

|CB_ERROR|

|TS| has ``Symbol()`` API that can be used among other things to generate
unique property names at runtime. |LANG| does not support ``Symbol()`` API
because its most popular use cases make no sense in the statically typed
environment. In particular, the object layout is defined at compile time,
and cannot be changed at runtime.

|LANG| supports the usage of ``Symbol.iterator`` in iterable interfaces.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    const sym = Symbol()
    let o = {
       [sym]: "value"
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class SomeClass {
        public someProperty : string = ""
    }
    let o = new SomeClass()

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R029`
* :ref:`R059`
* :ref:`R060`
* :ref:`R066`
* :ref:`R144`

.. _R003:

|CB_R| Private '``#``' identifiers are not supported
----------------------------------------------------

|CB_RULE| ``arkts-no-private-identifiers``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: PrivateIdentifier

|CB_ERROR|

|LANG| does not use private identifiers starting with the character '``#``'.
Use the keyword ``private`` instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    /*
     * Such notation for private fields is not supported in ArkTS:
    class C {
        #foo: number = 42
    }
    */

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        private foo: number = 42
    }

.. _R004:

|CB_R| Use unique names for types and namespaces.
-------------------------------------------------

|CB_RULE| ``arkts-unique-names``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DeclWithDuplicateName

|CB_ERROR|

Names for all types (classes, interfaces, enums) and namespaces must be unique
and distinct from other names, e.g., variable names and function names.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let X: string
    type X = number[] // Type alias name and variable name are the same

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let X: string
    type T = number[] // X is not allowed here to avoid name collisions

.. _R005:

|CB_R| Use ``let`` instead of ``var``
-------------------------------------

|CB_RULE| ``arkts-no-var``
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: VarDeclaration

|CB_ERROR|

|LANG| does not support ``var``. Use ``let`` instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function f(shouldInitialize: boolean) {
        if (shouldInitialize) {
           var x = 10
        }
        return x
    }

    console.log(f(true))  // 10
    console.log(f(false)) // undefined

    let upper_let = 0
    {
        var scoped_var = 0
        let scoped_let = 0
        upper_let = 5
    }
    scoped_var = 5 // Visible
    scoped_let = 5 // Compile-time error

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function f(shouldInitialize: boolean): Object {
        let x: Object = new Object()
        if (shouldInitialize) {
            x = 10
        }
        return x
    }

    console.log(f(true))  // 10
    console.log(f(false)) // {}

    let upper_let = 0
    let scoped_var = 0
    {
        let scoped_let = 0
        upper_let = 5
    }
    scoped_var = 5
    scoped_let = 5 // Compile-time error

.. _R008:

|CB_R| Use explicit types instead of ``any``, ``unknown``
---------------------------------------------------------

|CB_RULE| ``arkts-no-any-unknown``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: AnyType, UnknownType, EsObjectType, EsObjectAssignment, EsObjectAccess

|CB_ERROR|

|LANG| does not support the types ``any`` and ``unknown``. Specify
types explicitly.

If your |LANG| code has to interoperate with the standard |TS| or |JS| code,
and no type information is available (or if type information is impossible
to obtain), then you can use a special ``ESObject`` type to work with dynamic
objects. Note that such objects must be avoided at all cost as they reduce
type checking (i.e., the code is less stable and more error-prone) and have
severe runtime overhead. The usage of ``ESObject`` still produces a warning
message.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let value1 : any
    value1 = true
    value1 = 42

    let value2 : unknown
    value2 = true
    value2 = 42

    // Let's assume that we have no information for external_function
    // because it is defined in JavaScript code:
    let something : any = external_function()
    console.log("someProperty of something:", something.someProperty)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let value_b: boolean = true // OR: let value_b = true
    let value_n: number = 42 // OR: let value_n = 42
    let value_o1: Object = true
    let value_o2: Object = 42

    // Let's assume that we have no information for external_function
    // because it is defined in JavaScript code:
    let something : ESObject = external_function()
    console.log("someProperty of something:", something.someProperty)

|CB_SEE|
~~~~~~~~

* :ref:`R145`

.. _R014:

|CB_R| Use ``class`` instead of a type with call signature
----------------------------------------------------------

|CB_RULE| ``arkts-no-call-signatures``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: CallSignature

|CB_ERROR|

|LANG| does not support call signatures in object types. Use classes instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    type DescribableFunction = {
        description: string
        (someArg: number): string // call signature
    }

    function doSomething(fn: DescribableFunction): void {
        console.log(fn.description + " returned " + fn(6))
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class DescribableFunction {
        description: string
        public invoke(someArg: number): string {
            return someArg.toString()
        }
        constructor() {
            this.description = "desc"
        }
    }

    function doSomething(fn: DescribableFunction): void {
        console.log(fn.description + " returned " + fn.invoke(6))
    }

    doSomething(new DescribableFunction())

|CB_SEE|
~~~~~~~~

* :ref:`R015`

.. _R015:

|CB_R| Use ``class`` instead of a type with constructor signature
-----------------------------------------------------------------

|CB_RULE| ``arkts-no-ctor-signatures-type``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ConstructorType

|CB_ERROR|

|LANG| does not support constructor signatures in object types. Use classes
instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class SomeObject {}

    type SomeConstructor = {
        new (s: string): SomeObject
    }

    function fn(ctor: SomeConstructor) {
        return new ctor("hello")
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class SomeObject {
        public f: string
        constructor (s: string) {
            this.f = s
        }
    }

    function fn(s: string): SomeObject {
        return new SomeObject(s)
    }

|CB_SEE|
~~~~~~~~

* :ref:`R014`

.. _R016:

|CB_R| Only one static block is supported
-----------------------------------------

|CB_RULE| ``arkts-no-multiple-static-blocks``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: MultipleStaticBlocks

|CB_ERROR|

|LANG| does not allow having several static blocks for class initialization.
Combine multiple static block statements into one static block.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        static s: string

        static {
            C.s = "aa"
        }
        static {
            C.s = C.s + "bb"
        }
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript


    class C {
        static s: string

        static {
            C.s = "aa"
            C.s = C.s + "bb"
        }
    }


.. _R017:

|CB_R| Indexed signatures are not supported
-------------------------------------------

|CB_RULE| ``arkts-no-indexed-signatures``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: IndexMember

|CB_ERROR|

|LANG| does not allow indexed signatures. Use arrays instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Interface with an indexed signature:
    interface StringArray {
        [index: number]: string
    }

    function getStringArray() : StringArray {
        return ["a", "b", "c"]
    }

    const myArray: StringArray = getStringArray()
    const secondItem = myArray[1]

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class X {
        public f: string[] = []
    }

    let myArray: X = new X()
    const secondItem = myArray.f[1]

.. _R019:

|CB_R| Use inheritance instead of intersection types
----------------------------------------------------

|CB_RULE| ``arkts-no-intersection-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: IntersectionType

|CB_ERROR|

Currently, |LANG| does not support intersection types. Use inheritance
as a workaround.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    interface Identity {
        id: number
        name: string
    }

    interface Contact {
        email: string
        phoneNumber: string
    }

    type Employee = Identity & Contact

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface Identity {
        id: number
        name: string
    }

    interface Contact {
        email: string
        phoneNumber: string
    }

    interface Employee extends Identity,  Contact {}

.. _R021:

|CB_R| ``this`` typing is supported only for methods with explicit ``this`` return
----------------------------------------------------------------------------------

|CB_RULE| ``arkts-this-typing``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ThisTyping

|CB_ERROR|

|LANG| allows type notation using the ``this`` keyword only for a return type
of an instance method of a class or struct.
Such methods can only return ``this`` explicitly (``return this``).

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        n: number = 0

        m(c: this) {
            console.log(c)
        }

        foo(): this {
            return this.bar();
        }

        bar(): this {
            return this;
        }
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        n: number = 0

        m(c: C) {
            console.log(c)
        }

        foo(): this {
            return this;
        }

        bar(): this {
            return this;
        }
    }

.. _R022:

|CB_R| Conditional types are not supported
------------------------------------------

|CB_RULE| ``arkts-no-conditional-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ConditionalType

|CB_ERROR|

|LANG| does not support conditional type aliases. Introduce a new type with
constraints explicitly, or rewrite logic using ``Object``. The keyword
``infer`` is not supported.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    type X<T> = T extends number ? T : never

    type Y<T> = T extends Array<infer Item> ? Item : never

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Provide explicit constraints within type alias
    type X1<T extends number> = T

    // Rewrite with Object. Less type control, need more type checks for safety
    type X2<T> = Object

    // Item has to be used as a generic parameter and need to be properly
    // instantiated
    type YI<Item, T extends Array<Item>> = Item

.. _R025:

|CB_R| Declaring fields in ``constructor`` is not supported
-----------------------------------------------------------

|CB_RULE| ``arkts-no-ctor-prop-decls``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ParameterProperties

|CB_ERROR|

|LANG| does not support declaring class fields in the ``constructor``.
Declare class fields inside the ``class`` declaration instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Person {
        constructor(
            protected ssn: string,
            private firstName: string,
            private lastName: string
        ) {
            this.ssn = ssn
            this.firstName = firstName
            this.lastName = lastName
        }

        getFullName(): string {
            return this.firstName + " " + this.lastName
        }
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        protected ssn: string
        private firstName: string
        private lastName: string

        constructor(ssn: string, firstName: string, lastName: string) {
            this.ssn = ssn
            this.firstName = firstName
            this.lastName = lastName
        }

        getFullName(): string {
            return this.firstName + " " + this.lastName
        }
    }

.. _R027:

|CB_R| Construct signatures are not supported in interfaces
-----------------------------------------------------------

|CB_RULE| ``arkts-no-ctor-signatures-iface``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ConstructorIface

|CB_ERROR|

|LANG| does not support construct signatures. Use methods instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    interface I {
        new (s: string): I
    }

    function fn(i: I) {
        return new i("hello")
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface I {
        create(s: string): I
    }

    function fn(i: I) {
        return i.create("hello")
    }

|CB_SEE|
~~~~~~~~

* :ref:`R015`

.. _R028:

|CB_R| Indexed access types are not supported
---------------------------------------------

|CB_RULE| ``arkts-no-aliases-by-index``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: IndexedAccessType

|CB_ERROR|

|LANG| does not support indexed access types. Use the type name instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    type Point = {x: number, y: number}
    type N = Point["x"] // is equal to number

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Point {x: number = 0; y: number = 0}
    type N = number

.. _R029:

|CB_R| Indexed access is not supported for fields
-------------------------------------------------

|CB_RULE| ``arkts-no-props-by-index``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: PropertyAccessByIndex
    :fix: Replace with dot notation

|CB_ERROR|

|LANG| does not support dynamic field declaration and access. Declare all
object fields immediately in the class. Access only those class fields
that are either declared in the class, or accessible via inheritance. Accessing
any other fields is prohibited, and causes compile-time errors.

To access a field, use ``obj.field`` syntax. Indexed access (``obj["field"]``)
is not supported, except all typed arrays found in the standard library (e.g.,
``Int32Array``) that support access to their elements through
``container[index]`` syntax, tuples, Record objects, and enums.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Point {
        x: number = 0
        y: number = 0
    }
    let p: Point = {x: 1, y: 2}
    console.log(p["x"])

    class Person {
        name: string = ""
        age: number = 0; // semicolon is required here
        [key: string]: string | number
    }

    let person: Person = {
        name: "John",
        age: 30,
        email: "***@example.com",
        phoneNumber: "18*********",
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Point {
        x: number = 0
        y: number = 0
    }
    let p: Point = {x: 1, y: 2}
    console.log(p.x)

    class Person {
        name: string
        age: number
        email: string
        phoneNumber: string

        constructor(name: string, age: number, email: string,
                    phoneNumber: string) {
            this.name = name
            this.age = age
            this.email = email
            this.phoneNumber = phoneNumber
        }
    }

    let person = new Person("John", 30, "***@example.com", "18*********")
    console.log(person["name"])         // Compile-time error
    console.log(person.unknownProperty) // Compile-time error

    let arr = new Int32Array(1)
    console.log(arr[0])

.. _R030:

|CB_R| Structural typing is not supported
-----------------------------------------

|CB_RULE| ``arkts-no-structural-typing``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: StructuralIdentity

|CB_ERROR|

Currently, |LANG| does not support structural typing, i.e., the compiler
cannot compare public APIs of two types and decide whether such types are
identical. Use other mechanisms (inheritance, interfaces, or type aliases)
instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    interface I1 {
        f(): string
    }

    interface I2 { // I2 is structurally equivalent to I1
        f(): string
    }

    class X {
        n: number = 0
        s: string = ""
    }

    class Y { // Y is structurally equivalent to X
        n: number = 0
        s: string = ""
    }

    let x = new X()
    let y = new Y()

    console.log("Assign X to Y")
    y = x

    console.log("Assign Y to X")
    x = y

    function foo(x: X) {
        console.log(x.n, x.s)
    }

    // X and Y are equivalent because their public API is equivalent.
    // Thus the second call is allowed:
    foo(new X())
    foo(new Y())

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface I1 {
        f(): string
    }

    type I2 = I1 // I2 is an alias for I1

    class B {
        n: number = 0
        s: string = ""
    }

    // D is derived from B, which explicitly set subtype / supertype relations:
    class D extends B {
        constructor() {
            super()
        }
    }

    let b = new B()
    let d = new D()

    console.log("Assign D to B")
    b = d // ok, B is the superclass of D

    // An attempt to assign b to d results in a compile-time error:
    // d = b

    interface Common {
       n: number
       s: string
    }

    // X implements interface Z, which makes relation between X and Y explicit.
    class X implements Common {
        n: number = 0
        s: string = ""
    }

    // Y implements interface Z, which makes relation between X and Y explicit.
    class Y implements Common {
        n: number = 0
        s: string = ""
    }

    let x: Common = new X()
    let y: Common = new Y()

    console.log("Assign X to Y")
    y = x // ok, both are of the same type

    console.log("Assign Y to X")
    x = y // ok, both are of the same type

    function foo(c: Common): void {
        console.log(c.n, c.s)
    }

    // X and Y implement the same interface, thus both calls are allowed:
    foo(new X())
    foo(new Y())

.. _R034:

|CB_R| Type inference in case of generic function calls is limited
------------------------------------------------------------------

|CB_RULE| ``arkts-no-inferred-generic-params``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: GenericCallNoTypeArgs

|CB_ERROR|

|LANG| allows omitting generic type parameters if it is possible to infer
a concrete type from the parameters passed to the function. A compile-time
error occurs otherwise. In particular, inference of generic type parameters
based only on function return types is prohibited.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function choose<T>(x: T, y: T): T {
        return Math.random() < 0.5 ? x : y
    }

    let x = choose(10, 20)   // OK, choose<number>(...) is inferred
    let y = choose("10", 20) // Compile-time error

    function greet<T>(): T {
        return "Hello" as T
    }
    let z = greet() // Type of T is inferred as "unknown"

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function choose<T>(x: T, y: T): T {
        return Math.random() < 0.5 ? x : y
    }

    let x = choose(10, 20)   // OK, choose<number>(...) is inferred
    let y = choose("10", 20) // Compile-time error

    function greet<T>(): T {
        return "Hello" as T
    }
    let z = greet<string>()

.. _R037:

|CB_R| RegExp literals are not supported
----------------------------------------

|CB_RULE| ``arkts-no-regexp-literals``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: RegexLiteral

|CB_ERROR|

Currently, |LANG| does not support RegExp literals. Use library call with
string literals instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let regex: RegExp = /bc*d/

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let regex: RegExp = new RegExp("/bc*d/")

.. _R038:

|CB_R| Object literal must correspond to some explicitly declared class or interface
------------------------------------------------------------------------------------

|CB_RULE| ``arkts-no-untyped-obj-literals``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ObjectLiteralNoContextType

|CB_ERROR|

|LANG| supports the usage of object literals if the compiler can infer
what classes or interfaces such literals correspond to.
A compile-time error occurs otherwise. Using literals to initialize classes
and interfaces is not supported specifically for the initialization of the
following:

* Anything that has ``any``, ``Object``, or ``object`` type;
* Classes or interfaces with methods;
* Classes that declare a ``constructor`` with parameters;
* Classes with ``readonly`` fields.

In addition, |LANG| supports the usage of object literals to initialize the
value of special type ``Record<K, V>``. The type ``K`` denotes an object key,
and is restricted to types ``number`` and ``string``, union types constructed
from these types, and literals of these types.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let o1 = {n: 42, s: "foo"}
    let o2: Object = {n: 42, s: "foo"}
    let o3: object = {n: 42, s: "foo"}

    let oo: Object[] = [{n: 1, s: "1"}, {n: 2, s: "2"}]

    class C2 {
        s: string
        constructor(s: string) {
            this.s = "s =" + s
        }
    }
    let o4: C2 = {s: "foo"}

    class C3 {
        readonly n: number = 0
        readonly s: string = ""
    }
    let o5: C3 = {n: 42, s: "foo"}

    abstract class A {}
    let o6: A = {}

    class C4 {
        n: number = 0
        s: string = ""
        f() {
            console.log("Hello")
        }
    }
    let o7: C4 = {n: 42, s: "foo", f : () => {}}

    class Point {
        x: number = 0
        y: number = 0
    }

    function id_x_y(o: Point): Point {
        return o
    }

    // Structural typing is used to deduce that p is Point:
    let p = {x: 5, y: 10}
    id_x_y(p)

    // A literal can be contextually (i.e., implicitly) typed as Point:
    id_x_y({x: 5, y: 10})

    let rec: Record<string, number> = { a: 1, b: 2 }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C1 {
        n: number = 0
        s: string = ""
    }

    let o1: C1 = {n: 42, s: "foo"}
    let o2: C1 = {n: 42, s: "foo"}
    let o3: C1 = {n: 42, s: "foo"}

    let oo: C1[] = [{n: 1, s: "1"}, {n: 2, s: "2"}]

    class C2 {
        s: string
        constructor(s: string) {
            this.s = "s =" + s
        }
    }
    let o4 = new C2("foo")

    class C3 {
        n: number = 0
        s: string = ""
    }
    let o5: C3 = {n: 42, s: "foo"}

    abstract class A {}
    class C extends A {}
    let o6: C = {} // or let o6: C = new C()

    class C4 {
        n: number = 0
        s: string = ""
        f() {
            console.log("Hello")
        }
    }
    let o7 = new C4()
    o7.n = 42
    o7.s = "foo"

    class Point {
        x: number = 0
        y: number = 0

        // constructor() is used before literal initialization
        // to create a valid object. Since there is no other Point constructors,
        // constructor() is automatically added by compiler
    }

    function id_x_y(o: Point): Point {
        return o
    }

    // Explicit type is required for literal initialization
    let p: Point = {x: 5, y: 10}
    id_x_y(p)

    // id_x_y expects Point explicitly
    // New instance of Point is initialized with the literal
    id_x_y({x: 5, y: 10})

    let rec: Record<string, number> = { "a": 1, "b": 2 }

|CB_SEE|
~~~~~~~~

* :ref:`R040`
* :ref:`R043`

.. _R040:

|CB_R| Object literals cannot be used as type declarations
----------------------------------------------------------

|CB_RULE| ``arkts-no-obj-literals-as-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ObjectTypeLiteral

|CB_ERROR|

|LANG| does not support the usage of object literals to declare
types in place. Declare classes and interfaces explicitly instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let o: {x: number, y: number} = {
        x: 2,
        y: 3
    }

    type S = Set<{x: number, y: number}>

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class O {
        x: number = 0
        y: number = 0
    }

    let o: O = {x: 2, y: 3}

    type S = Set<O>

|CB_SEE|
~~~~~~~~

* :ref:`R038`
* :ref:`R043`

.. _R043:

|CB_R| Array literals must contain elements of only inferable types
-------------------------------------------------------------------

|CB_RULE| ``arkts-no-noninferable-arr-literals``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ArrayLiteralNoContextType

|CB_ERROR|

Basically, |LANG| infers the type of an array literal as union type of its
contents. However, a compile-time error occurs if at least one element has
a non-inferable type (e.g., untyped object literal).

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let a = [{n: 1, s: "1"}, {n: 2, s : "2"}]

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        n: number = 0
        s: string = ""
    }

    let a1 = [{n: 1, s: "1"} as C, {n: 2, s : "2"} as C] // a1 is of type "C[]"
    let a2: C[] = [{n: 1, s: "1"}, {n: 2, s : "2"}]      // ditto

|CB_SEE|
~~~~~~~~

* :ref:`R038`
* :ref:`R040`

.. _R046:

|CB_R| Use arrow functions instead of function expressions
----------------------------------------------------------

|CB_RULE| ``arkts-no-func-expressions``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: FunctionExpression
    :fix: Convert to arrow function

|CB_ERROR|

|LANG| does not support function expressions. Use arrow functions instead
to specify explicitly.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let f = function (s: string) {
        console.log(s)
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let f = (s: string) => {
        console.log(s)
    }

.. _R050:

|CB_R| Class literals are not supported
---------------------------------------

|CB_RULE| ``arkts-no-class-literals``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ClassExpression

|CB_ERROR|

|LANG| does not support class literals. Introduce new named class types
explicitly.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    const Rectangle = class {
        constructor(height: number, width: number) {
            this.height = height
            this.width = width
        }

        height
        width
    }

    const rectangle = new Rectangle(0.0, 0.0)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Rectangle {
        constructor(height: number, width: number) {
            this.height = height
            this.width = width
        }

        height: number
        width: number
    }

    const rectangle = new Rectangle(0.0, 0.0)

.. _R051:

|CB_R| Classes cannot be specified in ``implements`` clause
-----------------------------------------------------------

|CB_RULE| ``arkts-implements-only-iface``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ImplementsClass

|CB_ERROR|

|LANG| does not allow specifying a class in implements clause. Only interfaces
can be specified.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
      foo() {}
    }

    class C1 implements C {
      foo() {}
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface C {
      foo(): void
    }

    class C1 implements C {
      foo() {}
    }

.. _R052:

|CB_R| Reassigning object methods is not supported
--------------------------------------------------

|CB_RULE| ``arkts-no-method-reassignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: MethodReassignment

|CB_ERROR|

|LANG| does not support re-assigning a method for objects. In the statically
types languages, the layout of objects is fixed, and all instances of the same
object must share the same code of each method.

To add specific behavior for certain objects, create separate wrapper functions,
or use inheritance.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        foo() {
            console.log("foo")
        }
    }

    function bar() {
        console.log("bar")
    }

    let c1 = new C()
    let c2 = new C()
    c2.foo = bar

    c1.foo() // foo
    c2.foo() // bar

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        foo() {
            console.log("foo")
        }
    }

    class Derived extends C {
        foo() {
            console.log("Extra")
            super.foo()
        }
    }

    function bar() {
        console.log("bar")
    }

    let c1 = new C()
    let c2 = new C()
    c1.foo() // foo
    c2.foo() // foo

    let c3 = new Derived()
    c3.foo() // Extra foo

.. _R053:

|CB_R| Only ``as T`` syntax is supported for type casts
-------------------------------------------------------

|CB_RULE| ``arkts-as-casts``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: TypeAssertion
    :fix: Replace to 'as' expression

|CB_ERROR|

|LANG| supports the keyword ``as`` as the only syntax for type casts.
Incorrect cast causes a compile-time error, or runtime ``ClassCastException``.
``<type>`` syntax for type casts is not supported.

Use the expression ``new ...`` instead of ``as`` to cast a **primitive** type
(e.g., a ``number`` or a ``boolean``) to a reference type.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Shape {}
    class Circle extends Shape {x: number = 5}
    class Square extends Shape {y: string = "a"}

    function createShape(): Shape {
        return new Circle()
    }

    let c1 = <Circle> createShape()

    let c2 = createShape() as Circle

    // No report is provided during compilation
    // nor during runtime if cast is wrong:
    let c3 = createShape() as Square
    console.log(c3.y) // undefined

    // Important corner case for casting primitives to the boxed counterparts:
    // The left operand is not properly boxed here at runtime
    // because "as" has no runtime effect in TypeScript
    let e1 = (5.0 as Number) instanceof Number // false

    // Number object is created and instanceof works as expected:
    let e2 = (new Number(5.0)) instanceof Number // true

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Shape {}
    class Circle extends Shape {x: number = 5}
    class Square extends Shape {y: string = "a"}

    function createShape(): Shape {
        return new Circle()
    }

    let c2 = createShape() as Circle

    // ClassCastException during runtime is thrown:
    let c3 = createShape() as Square

    // Number object is created, and instanceof works as expected:
    let e2 = (new Number(5.0)) instanceof Number // true

.. _R054:

|CB_R| JSX expressions are not supported
----------------------------------------

|CB_RULE| ``arkts-no-jsx``
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: JsxElement

|CB_ERROR|

Do not use JSX since no alternative is provided to rewrite it.

.. _R055:

|CB_R| Unary operators '``+``', '``-``', and '``~``' work only on numbers
-------------------------------------------------------------------------

|CB_RULE| ``arkts-no-polymorphic-unops``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: UnaryArithmNotNumber

|CB_ERROR|

|LANG| allows unary operators to work on numeric types only. A compile-time
error occurs if these operators are applied to a non-numeric type. Unlike in
|TS|, implicit casting of strings in this context is not supported, and must
be done explicitly.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let a = +5        // 5 as number
    let b = +"5"      // 5 as number
    let c = -5        // -5 as number
    let d = -"5"      // -5 as number
    let e = ~5        // -6 as number
    let f = ~"5"      // -6 as number
    let g = +"string" // NaN as number

    function returnTen(): string {
        return "-10"
    }

    function returnString(): string {
        return "string"
    }

    let x = +returnTen()    // -10 as number
    let y = +returnString() // NaN

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let a = +5        // 5 as number
    let b = +"5"      // Compile-time error
    let c = -5        // -5 as number
    let d = -"5"      // Compile-time error
    let e = ~5        // -6 as number
    let f = ~"5"      // Compile-time error
    let g = +"string" // Compile-time error

    function returnTen(): string {
        return "-10"
    }

    function returnString(): string {
        return "string"
    }

    let x = +returnTen()    // Compile-time error
    let y = +returnString() // Compile-time error

.. _R059:

|CB_R| ``delete`` operator is not supported
-------------------------------------------

|CB_RULE| ``arkts-no-delete``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DeleteOperator

|CB_ERROR|

|LANG| assumes that object layout is known at compile time, and cannot be
changed at runtime. Thus, the operation of deleting a property makes no sense.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Point {
        x?: number = 0.0
        y?: number = 0.0
    }

    let p = new Point()
    delete p.y

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // To mimic the original semantics, you can declare a nullable type,
    // and assign null to mark value absence:

    class Point {
        x: number | null = 0
        y: number | null = 0
    }

    let p = new Point()
    p.y = null

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R002`
* :ref:`R029`
* :ref:`R060`
* :ref:`R066`

.. _R060:

|CB_R| ``typeof`` operator is allowed only in expression contexts
-----------------------------------------------------------------

|CB_RULE| ``arkts-no-type-query``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: TypeQuery

|CB_ERROR|

|LANG| supports ``typeof`` operator only in the expression context. Using
``typeof`` to specify type notations is not supported.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let n1 = 42
    let s1 = "foo"
    console.log(typeof n1) // "number"
    console.log(typeof s1) // "string"
    let n2: typeof n1
    let s2: typeof s1

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let n1 = 42
    let s1 = "foo"
    console.log(typeof n1) // "number"
    console.log(typeof s1) // "string"
    let n2: number
    let s2: string

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R002`
* :ref:`R029`
* :ref:`R059`
* :ref:`R066`
* :ref:`R144`

.. _R065:

|CB_R| ``instanceof`` operator is partially supported
-----------------------------------------------------

|CB_RULE| ``arkts-instanceof-ref-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InstanceofUnsupported

|CB_ERROR|

In |TS|, the left-hand side of an ``instanceof`` expression must be of type
``any``, an object type, or a type parameter. Otherwise, the result is
``false``.

In |LANG|, the left-hand side expression can be of any reference type.
Otherwise, a compile-time error occurs. In addition, the left operand cannot
be a type in |LANG|.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class X {
        // ...
    }

    let a = (new X()) instanceof Object // true
    let b = (new X()) instanceof X      // true

    let c = X instanceof Object // true, left operand is a type
    let d = X instanceof X      // false, left operand is a type

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class X {
        // ...
    }

    let a = (new X()) instanceof Object // true
    let b = (new X()) instanceof X      // true

    let c = X instanceof Object // Compile-time error, left operand is a type
    let d = X instanceof X      // Compile-time error, left operand is a type

.. _R066:

|CB_R| ``in`` operator is not supported
---------------------------------------

|CB_RULE| ``arkts-no-in``
~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InOperator

|CB_ERROR|

|LANG| does not support the operator ``in``. This operator makes little
sense since the object layout is already known at compile time, and cannot
be modified at runtime. Use ``instanceof`` as a workaround if you still need
to check whether certain class members exist.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Person {
        name: string = ""
    }
    let p = new Person()

    let b = "name" in p // true

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        name: string = ""
    }
    let p = new Person()

    let b = p instanceof Person // true, and "name" is guaranteed to be present

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R002`
* :ref:`R029`
* :ref:`R059`
* :ref:`R060`
* :ref:`R144`

.. _R069:

|CB_R| Destructuring assignment is partially supported
------------------------------------------------------

|CB_RULE| ``arkts-no-destruct-assignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DestructuringAssignment

|CB_ERROR|

|LANG| supports destructuring assignment for arrays and tuples. Object
destructuring and spread operator are not supported. Use other idioms
(e.g., a temporary variable where applicable) as replacement.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let one, two;
    [one, two] = [1, 2];
    [one, two] = [two, one];

    let head, tail
    [head, ...tail] = [1, 2, 3, 4]

    class Point {
        x: number = 0.0
        y: number = 0.0
    }

    let x: number, y: number;
    ({x, y} = new Point()); // parentheses are required here

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let one, two;
    [one, two] = [1, 2];
    [one, two] = [two, one];

    let data: Number[] = [1, 2, 3, 4]
    let head = data[0]
    let tail: Number[] = []
    for (let i = 1; i < data.length; ++i) {
        tail.push(data[i])
    }

    class Point {
        x: number = 0.0
        y: number = 0.0
    }

    let p = new Point()
    let x = p.x
    let y = p.y

.. _R071:

|CB_R| The comma operator '``,``' is supported only in ``for`` loops
--------------------------------------------------------------------

|CB_RULE| ``arkts-no-comma-outside-loops``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: CommaOperator

|CB_ERROR|

|LANG| supports the comma operator '``,``' only in ``for`` loops. It is
useless otherwise as it makes the execution order harder to understand.

Note that this rule is applied to the "comma operator" only. Comma is allowed
in other cases where it is used to delimit variable declarations or parameters
of a function call.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    for (let i = 0, j = 0; i < 10; ++i, j += 2) {
        console.log(i)
        console.log(j)
    }

    let x = 0
    x = (++x, x++) // 1

|CB_OK|
~~~~~~~

.. code-block:: typescript

    for (let i = 0, j = 0; i < 10; ++i, j += 2) {
        console.log(i)
        console.log(j)
    }

    // Use explicit execution order instead of the comma operator:
    let x = 0
    ++x
    x = x++

.. _R074:

|CB_R| Destructuring variable declarations are partially supported
------------------------------------------------------------------

|CB_RULE| ``arkts-no-destruct-decls``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DestructuringDeclaration

|CB_ERROR|

|LANG| supports destructuring variable declarations for arrays and tuples.
Object destructuring and spread operator are not supported. This is a dynamic
feature relying on structural compatibility. In addition, names in destructuring
declarations must be equal to properties within destructured classes.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let [one, two] = [1, 2]; // semicolon is required here

    class Point {
        x: number = 0.0
        y: number = 0.0
    }

    function returnZeroPoint(): Point {
        return new Point()
    }

    let {x, y} = returnZeroPoint()

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let [one, two] = [1, 2]; // semicolon is required here

    class Point {
        x: number = 0.0
        y: number = 0.0
    }

    function returnZeroPoint(): Point {
        return new Point()
    }

    // Create an intermediate object, and work with it field by field
    // without name restrictions:
    let zp = returnZeroPoint()
    let x = zp.x
    let y = zp.y

.. _R079:

|CB_R| Type annotation in catch clause is not supported
-------------------------------------------------------

|CB_RULE| ``arkts-no-types-in-catch``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: CatchWithUnsupportedType
    :fix: Remove type annotation

|CB_ERROR|

In |TS|, catch clause variable type annotation must be ``any`` or ``unknown``
if specified. As |LANG| does not support these types, omit type annotations.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    try {
        // some code
    }
    catch (a: unknown) {
        // handle error
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    try {
        // some code
    }
    catch (a) {
        // handle error
    }

|CB_SEE|
~~~~~~~~

* :ref:`R087`

.. _R080:

|CB_R| ``for .. in`` is not supported
-------------------------------------

|CB_RULE| ``arkts-no-for-in``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ForInStatement

|CB_ERROR|

|LANG| does not support the iteration over object contents by the
``for .. in`` loop. Iteration over object properties at runtime is considered
redundant as object layout cannot be changed at runtime after being known
at compile time. For arrays, iterate with the regular ``for`` loop.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let a: number[] = [1.0, 2.0, 3.0]
    for (let i in a) {
        console.log(a[i])
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let a: number[] = [1.0, 2.0, 3.0]
    for (let i = 0; i < a.length; ++i) {
        console.log(a[i])
    }

.. _R083:

|CB_R| Mapped type expression is not supported
----------------------------------------------

|CB_RULE| ``arkts-no-mapped-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: MappedType

|CB_ERROR|

|LANG| does not support mapped types. Use other language idioms and regular
classes to achieve that same behavior.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    type OptionsFlags<Type> = {
        [Property in keyof Type]: boolean
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        n: number = 0
        s: string = ""
    }

    class CFlags {
        n: boolean = false
        s: boolean = false
    }

.. _R084:

|CB_R| ``with`` statement is not supported
------------------------------------------

|CB_RULE| ``arkts-no-with``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: WithStatement

|CB_ERROR|

|LANG| does not support the ``with`` statement. Use other language idioms
(including fully qualified names of functions) to achieve that same behavior.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    with (Math) { // Compile-time error, but JavaScript code still emitted
        let r: number = 42
        console.log("Area: ", PI * r * r)
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let r: number = 42
    console.log("Area: ", Math.PI * r * r)

.. _R087:

|CB_R| ``throw`` statements cannot accept values of arbitrary types
-------------------------------------------------------------------

|CB_RULE| ``arkts-limited-throw``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ThrowStatement
    :fix: Wrap in 'Error'

|CB_ERROR|

|LANG| supports throwing only objects of the class ``Error`` or any
derived class. Throwing an arbitrary type (i.e., a ``number`` or ``string``)
is prohibited.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    throw 4
    throw ""
    throw new Error()

|CB_OK|
~~~~~~~

.. code-block:: typescript

    throw new Error()

.. _R090:

|CB_R| Function return type inference is limited
------------------------------------------------

|CB_RULE| ``arkts-no-implicit-return-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: LimitedReturnTypeInference
    :fix: Annotate return type

|CB_ERROR|

|LANG| supports type inference for function return types, but this functionality
is currently restricted. In particular, a compile-time error occurs if the
expression in the ``return`` statement is a call to a function or method whose
return value type is omitted. In case of any such error, specify the return type
explicitly.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Compile-time error with noImplicitAny
    function f(x: number) {
        if (x <= 0) {
            return x
        }
        return g(x)
    }

    // Compile-time error with noImplicitAny
    function g(x: number) {
        return f(x - 1)
    }

    function doOperation(x: number, y: number) {
        return x + y
    }

    console.log(f(10))
    console.log(doOperation(2, 3))

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Explicit return type is required:
    function f(x: number) : number {
        if (x <= 0) {
            return x
        }
        return g(x)
    }

    // Return type may be omitted, it is inferred from f's explicit type:
    function g(x: number) {
        return f(x - 1)
    }

    // In this case, return type will be inferred
    function doOperation(x: number, y: number) {
        return x + y
    }

    console.log(f(10))
    console.log(doOperation(2, 3))

.. _R091:

|CB_R| Destructuring parameter declarations are partially supported
-------------------------------------------------------------------

|CB_RULE| ``arkts-no-destruct-params``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DestructuringParameter

|CB_ERROR|

|LANG| supports unpacking arrays and tuples passed as function parameters.
Unpacking properties from objects is not supported. |LANG| requires parameters
to be passed directly to the function, and local names to be assigned manually.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function drawPoint([x, y] = [0, 0]) {
        console.log(x)
        console.log(y)
    }

    drawPoint([1, 2])

    function drawText({ text = "", location: [x, y] = [0, 0], bold = false }) {
        console.log(text)
        console.log(x)
        console.log(y)
        console.log(bold)
    }

    drawText({ text: "Hello, world!", location: [100, 50], bold: true })

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function drawPoint([x, y] = [0, 0]) {
        console.log(x)
        console.log(y)
    }

    drawPoint([1, 2])

    function drawText(text: String, location: number[], bold: boolean) {
        let x = location[0]
        let y = location[1]
        console.log(text)
        console.log(x)
        console.log(y)
        console.log(bold)
    }

    function main() {
        drawText("Hello, world!", [100, 50], true)
    }

.. _R092:

|CB_R| Nested functions are not supported
-----------------------------------------

|CB_RULE| ``arkts-no-nested-funcs``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: LocalFunction

|CB_ERROR|

|LANG| does not support nested functions. Use lambdas instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function addNum(a: number, b: number): void {

        // nested function:
        function logToConsole(message: String): void {
            console.log(message)
        }

        let result = a + b

        // Invoking the nested function:
        logToConsole("result is " + result)
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function addNum(a: number, b: number): void {
        // Use lambda instead of a nested function:
        let logToConsole: (message: string) => void = (message: string): void => {
            console.log(message)
        }

        let result = a + b

        logToConsole("result is " + result)
    }

.. _R093:

|CB_R| Using ``this`` inside stand-alone functions is not supported
-------------------------------------------------------------------

|CB_RULE| ``arkts-no-standalone-this``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: FunctionContainsThis

|CB_ERROR|

|LANG| does not support the usage of ``this`` inside stand-alone functions and
static methods. Use ``this`` in instance methods only.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function foo(i: number) {
        this.count = i // Compile-time error only with noImplicitThis
    }

    class A {
        count: number = 1
        m = foo
    }

    let a = new A()
    console.log(a.count) // prints "1"
    a.m(2)
    console.log(a.count) // prints "2"


|CB_OK|
~~~~~~~

.. code-block:: typescript

    class A {
        count: number = 1
        m(i: number): void {
            this.count = i
        }
    }

    function main(): void {
        let a = new A()
        console.log(a.count)  // prints "1"
        a.m(2)
        console.log(a.count)  // prints "2"
    }

|CB_SEE|
~~~~~~~~

* :ref:`R140`

.. _R094:

|CB_R| Generator functions are not supported
--------------------------------------------

|CB_RULE| ``arkts-no-generators``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: GeneratorFunction, YieldExpression

|CB_ERROR|

Currently, |LANG| does not support generator functions.
Use the ``async`` / ``await`` mechanism for multitasking.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function* counter(start: number, end: number) {
        for (let i = start; i <= end; i++) {
            yield i
        }
    }

    for (let num of counter(1, 5)) {
        console.log(num)
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    async function complexNumberProcessing(n : number) : Promise<number> {
        // Some complex logic for processing the number here
        return n
    }

    async function foo() {
        for (let i = 1; i <= 5; i++) {
            console.log(await complexNumberProcessing(i))
        }
    }

    foo()

.. _R096:

|CB_R| Type guarding is supported with ``instanceof`` and ``as``
----------------------------------------------------------------

|CB_RULE| ``arkts-no-is``
~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: IsOperator

|CB_ERROR|

|LANG| does not support the ``is`` operator. Replace ``is`` for ``instanceof``
at all times. Use the ``as`` operator to cast the fields of an object to an
appropriate type before use.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Foo {
        foo: number = 0
        common: string = ""
    }

    class Bar {
        bar: number = 0
        common: string = ""
    }

    function isFoo(arg: any): arg is Foo {
        return arg.foo !== undefined
    }

    function doStuff(arg: Foo | Bar) {
        if (isFoo(arg)) {
            console.log(arg.foo)    // OK
            console.log(arg.bar)    // Compile-time error
        } else {
            console.log(arg.foo)    // Compile-time error
            console.log(arg.bar)    // OK
        }
    }

    doStuff({ foo: 123, common: '123' })
    doStuff({ bar: 123, common: '123' })

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Foo {
        foo: number = 0
        common: string = ""
    }

    class Bar {
        bar: number = 0
        common: string = ""
    }

    function isFoo(arg: Object): boolean {
        return arg instanceof Foo
    }

    function doStuff(arg: Object): void {
        if (isFoo(arg)) {
            let fooArg = arg as Foo
            console.log(fooArg.foo)     // OK
            console.log(arg.bar)        // Compile-time error
        } else {
            let barArg = arg as Bar
            console.log(arg.foo)        // Compile-time error
            console.log(barArg.bar)     // OK
        }
    }

    function main(): void {
        doStuff(new Foo())
        doStuff(new Bar())
    }

.. _R099:

|CB_R| Only arrays or classes derived from arrays can be spread into the rest parameter or array literals
---------------------------------------------------------------------------------------------------------

|CB_RULE| ``arkts-no-spread``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SpreadOperator

|CB_ERROR|

The only supported scenario for the spread operator is to spread an array or
class derived from array into the rest parameter or array literal.
Otherwise, manually "unpack" data from arrays and objects, where necessary.
All typed arrays from the standard library (e.g., ``Int32Array``)
are also supported.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function foo(x : number, y : number, z : number) {
        console.log(x, y, z)
    }

    let args : [number, number, number] = [0, 1, 2]
    foo(...args)

    let list1 = [1, 2]
    let list2 = [...list1, 3, 4]

    let point2d = {x: 1, y: 2}
    let point3d = {...point2d, z: 3}

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function sum_numbers(...numbers: number[]): number {
        let res = 0
        for (let n of numbers)
            res += n
        return res
    }
    console.log(sum_numbers(1, 2, 3))

    function log_numbers(x : number, y : number, z : number) {
        console.log(x, y, z)
    }
    let numbers: number[] = [1, 2, 3]
    log_numbers(numbers[0], numbers[1], numbers[2])

    let list1 : number[] = [1, 2]
    let list2 : number[] = [list1[0], list1[1], 3, 4]

    class Point2D {
        x: number = 0; y: number = 0
    }

    class Point3D {
        x: number = 0; y: number = 0; z: number = 0
        constructor(p2d: Point2D, z: number) {
            this.x = p2d.x
            this.y = p2d.y
            this.z = z
        }
    }

    let p3d = new Point3D({x: 1, y: 2} as Point2D, 3)
    console.log(p3d.x, p3d.y, p3d.z)

    class DerivedFromArray extends Uint16Array {};

    let arr1 = [1, 2, 3];
    let arr2 = new Uint16Array([4, 5, 6]);
    let arr3 = new DerivedFromArray([7, 8, 9])
    let arr4 = [...arr1, 10, ...arr2, 11, ...arr3]

.. _R102:

|CB_R| Interface cannot extend interfaces with the same method
--------------------------------------------------------------

|CB_RULE| ``arkts-no-extend-same-prop``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InterfaceExtendDifProps

|CB_ERROR|

In |TS|, an interface that extends two other interfaces with the same method
must declare that method, and have a combined type as a result. It is not
allowed in |LANG| because |LANG| allows no interface to contain two methods
with signatures that are not distinguishable (e.g., two methods cannot have
the same parameter lists but different return types).

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    interface Mover {
        getStatus(): { speed: number }
    }
    interface Shaker {
        getStatus(): { frequency: number }
    }

    interface MoverShaker extends Mover, Shaker {
        getStatus(): {
            speed: number
            frequency: number
        }
    }

    class C implements MoverShaker {
        private speed: number = 0
        private frequency: number = 0

        getStatus() {
            return { speed: this.speed, frequency: this.frequency }
        }
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class MoveStatus {
        public speed : number
        constructor() {
            this.speed = 0
        }
    }
    interface Mover {
        getMoveStatus(): MoveStatus
    }

    class ShakeStatus {
        public frequency : number
        constructor() {
            this.frequency = 0
        }
    }
    interface Shaker {
        getShakeStatus(): ShakeStatus
    }

    class MoveAndShakeStatus {
        public speed : number
        public frequency : number
        constructor() {
            this.speed = 0
            this.frequency = 0
        }
    }

    class C implements Mover, Shaker {
        private move_status : MoveStatus
        private shake_status : ShakeStatus

        constructor() {
            this.move_status = new MoveStatus()
            this.shake_status = new ShakeStatus()
        }

        public getMoveStatus() : MoveStatus {
            return this.move_status
        }

        public getShakeStatus() : ShakeStatus {
            return this.shake_status
        }

        public getStatus(): MoveAndShakeStatus {
            return {
                speed: this.move_status.speed,
                frequency: this.shake_status.frequency
            }
        }
    }

.. _R103:

|CB_R| Declaration merging is not supported
-------------------------------------------

|CB_RULE| ``arkts-no-decl-merging``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InterfaceMerging

|CB_ERROR|

|LANG| does not support merging declarations. Keep all definitions of classes
and interfaces compact in the codebase.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    interface Document {
        createElement(tagName: any): Element
    }

    interface Document {
        createElement(tagName: string): HTMLElement
    }

    interface Document {
        createElement(tagName: number): HTMLDivElement
        createElement(tagName: boolean): HTMLSpanElement
        createElement(tagName: string, value: number): HTMLCanvasElement
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface Document {
        createElement(tagName: number): HTMLDivElement
        createElement(tagName: boolean): HTMLSpanElement
        createElement(tagName: string, value: number): HTMLCanvasElement
        createElement(tagName: string): HTMLElement
        createElement(tagName: Object): Element
    }

.. _R104:

|CB_R| Interfaces cannot extend classes
---------------------------------------

|CB_RULE| ``arkts-extends-only-class``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InterfaceExtendsClass

|CB_ERROR|

|LANG| does not support interfaces that extend classes. Interfaces can only
extend interfaces.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Control {
        state: number = 0
    }

    interface SelectableControl extends Control {
        select(): void
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface Control {
        state: number
    }

    interface SelectableControl extends Control {
        select(): void
    }

.. _R106:

|CB_R| Constructor function type is not supported
-------------------------------------------------

|CB_RULE| ``arkts-no-ctor-signatures-funcs``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ConstructorFuncs

|CB_ERROR|

|LANG| does not support the usage of the constructor function type.
Use lambdas instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Person {
        constructor(
            name: string,
            age: number
        ) {}
    }
    type PersonCtor = new (name: string, age: number) => Person

    function createPerson(Ctor: PersonCtor, name: string, age: number): Person
    {
        return new Ctor(name, age)
    }

    const person = createPerson(Person, 'John', 30)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        constructor(
            name: string,
            age: number
        ) {}
    }
    type PersonCtor = (n: string, a: number) => Person

    function createPerson(Ctor: PersonCtor, n: string, a: number): Person {
        return Ctor(n, a)
    }

    let Impersonizer: PersonCtor = (n: string, a: number): Person => {
        return new Person(n, a)
    }

    const person = createPerson(Impersonizer, "John", 30)

.. _R111:

|CB_R| Enumeration members can be initialized only with compile time expressions of the same type
-------------------------------------------------------------------------------------------------

|CB_RULE| ``arkts-no-enum-mixed-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: EnumMemberNonConstInit

|CB_ERROR|

|LANG| does not support initializing members of enumerations with expressions
that are evaluated during program runtime. All explicitly set initializers
must be of the same type.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    enum E1 {
        A = 0xa,
        B = 0xb,
        C = Math.random(),
        D = 0xd,
        E // 0xe inferred
    }

    enum E2 {
        A = 0xa,
        B = "0xb",
        C = 0xc,
        D = "0xd"
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    enum E1 {
        A = 0xa,
        B = 0xb,
        C = 0xc,
        D = 0xd,
        E // 0xe inferred
    }

    enum E2 {
        A = "0xa",
        B = "0xb",
        C = "0xc",
        D = "0xd"
    }

.. _R113:

|CB_R| ``enum`` declaration merging is not supported
----------------------------------------------------

|CB_RULE| ``arkts-no-enum-merging``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: EnumMerging

|CB_ERROR|

|LANG| does not support merging declarations for ``enum``. Keep the
declaration of each ``enum`` compact in the codebase.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    enum Color {
        RED,
        GREEN
    }
    enum Color {
        YELLOW = 2
    }
    enum Color {
        BLACK = 3,
        BLUE
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    enum Color {
        RED,
        GREEN,
        YELLOW,
        BLACK,
        BLUE
    }

.. _R114:

|CB_R| Namespaces cannot be used as objects
-------------------------------------------

|CB_RULE| ``arkts-no-ns-as-obj``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: NamespaceAsObject

|CB_ERROR|

|LANG| does not support the usage of namespaces as objects.
Classes or modules can be interpreted as placeholders of namespaces.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    namespace MyNamespace {
        export let x: number
    }

    let m = MyNamespace
    m.x = 2

|CB_OK|
~~~~~~~

.. code-block:: typescript

    namespace MyNamespace {
        export let x: number
    }

    MyNamespace.x = 2

.. _R116:

|CB_R| Non-declaration statements in namespaces are not supported
-----------------------------------------------------------------

|CB_RULE| ``arkts-no-ns-statements``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: NonDeclarationInNamespace

|CB_ERROR|

|LANG| does not support statements in namespaces. Use function to execute
a statement.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    namespace A {
        export let x: number
        x = 1
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    namespace A {
        export let x: number

        export function init() {
          x = 1
        }
    }

    // Initialization function must be called to execute statements:
    A.init()

.. _R121:

|CB_R| ``require`` and ``import`` assignment are not supported
--------------------------------------------------------------

|CB_RULE| ``arkts-no-require``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ImportAssignment

|CB_ERROR|

|LANG| does not support importing via ``require`` and ``import`` assignments.
Use regular ``import`` instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    import m = require("mod")

|CB_OK|
~~~~~~~

.. code-block:: typescript

    import * as m from "mod"

|CB_SEE|
~~~~~~~~

* :ref:`R126`

.. _R126:

|CB_R| ``export = ...`` assignment is not supported
---------------------------------------------------

|CB_RULE| ``arkts-no-export-assignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ExportAssignment

|CB_ERROR|

|LANG| does not support ``export = ...`` syntax.
Use regular ``export`` / ``import`` instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // module1
    export = Point

    class Point {
        constructor(x: number, y: number) {}
        static origin = new Point(0, 0)
    }

    // module2
    import Pt = require("module1")

    let p = Pt.origin

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // module1
    export class Point {
        constructor(x: number, y: number) {}
        static origin = new Point(0, 0)
    }

    // module2
    import * as Pt from "module1"

    let p = Pt.origin

|CB_SEE|
~~~~~~~~

* :ref:`R121`

.. _R128:

|CB_R| Ambient module declaration is not supported
--------------------------------------------------

|CB_RULE| ``arkts-no-ambient-decls``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ShorthandAmbientModuleDecl

|CB_ERROR|

|LANG| does not support ambient module declaration because it has its
own mechanism to interoperate with |JS|.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    declare module "someModule" {
        export function normalize(s : string) : string;
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Import what you need from the original module
    import { normalize } from "someModule"

|CB_SEE|
~~~~~~~~

* :ref:`R129`

.. _R129:

|CB_R| Wildcards in module names are not supported
--------------------------------------------------

|CB_RULE| ``arkts-no-module-wildcards``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: WildcardsInModuleName

|CB_ERROR|

|LANG| does not support wildcards in module names because import is not a
runtime but a compile-time feature in the language.
Use ordinary export syntax instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Declaration:
    declare module "*!text" {
        const content: string
        export default content
    }

    // Consuming code:
    import fileContent from "some.txt!text"

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Declaration:
    declare namespace N {
        function foo(x: number): number
    }

    // Consuming code:
    import * as m from "module"
    console.log("N.foo called: ", N.foo(42))

|CB_SEE|
~~~~~~~~

* :ref:`R128`
* :ref:`R130`

.. _R130:

|CB_R| Universal module definitions (UMD) are not supported
-----------------------------------------------------------

|CB_RULE| ``arkts-no-umd``
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: UMDModuleDefinition

|CB_ERROR|

|LANG| does not support universal module definitions (UMD) because the language
has no concept of *script* (as opposed to *module*). Besides, import is not
a runtime but a compile-time feature in |LANG|. Use ordinary syntax for
``export`` and ``import`` instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // math-lib.d.ts
    export const isPrime(x: number): boolean
    export as namespace mathLib

    // in script
    mathLib.isPrime(2)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // math-lib.d.ts
    namespace mathLib {
        export isPrime(x: number): boolean
    }

    // in program
    import { mathLib } from "math-lib"
    mathLib.isPrime(2)

|CB_SEE|
~~~~~~~~

* :ref:`R129`

.. _R132:

|CB_R| ``new.target`` is not supported
--------------------------------------

|CB_RULE| ``arkts-no-new-target``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: NewTarget

|CB_ERROR|

|LANG| does not support ``new.target`` because the language has no concept
of runtime prototype inheritance.
This feature is considered not applicable to static typing.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class CustomError extends Error {
        constructor(message?: string) {
            // 'Error' breaks prototype chain here:
            super(message)

            // Restore prototype chain:
            Object.setPrototypeOf(this, new.target.prototype)
        }
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class CustomError extends Error {
        constructor(message?: string) {
            // Call parent's constructor, inheritance chain is static and
            // cannot be modified at runtime
            super(message)
            console.log(this instanceof Error) // true
        }
    }
    let ce = new CustomError()

|CB_SEE|
~~~~~~~~

* :ref:`R136`

.. _R134:

|CB_R| Definite assignment assertions are not supported
-------------------------------------------------------

|CB_RULE| ``arkts-no-definite-assignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DefiniteAssignment

|CB_WARNING|

|LANG| does not support definite assignment assertions ``let v!: T`` because
they are considered an excessive compiler hint.
Use declaration with initialization instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let x!: number // Hint: x will be initialized before usage

    initialize()

    function initialize() {
        x = 10
    }

    console.log("x = " + x)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function initialize() : number {
        return 10
    }

    let x: number = initialize()

    console.log("x = " + x)

.. _R136:

|CB_R| Prototype assignment is not supported
--------------------------------------------

|CB_RULE| ``arkts-no-prototype-assignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: Prototype

|CB_ERROR|

Prototype assignment is not supported because |LANG| has no concept of
runtime prototype inheritance. This feature is considered not applicable
to static typing. Use the classes and / or interfaces mechanism instead
to statically *combine* methods to data.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    var C = function(p: number) {
        this.p = p // Compile-time error only with noImplicitThis
    }

    C.prototype = {
        m() {
            console.log(this.p)
        }
    }

    C.prototype.q = function(r: number) {
        return this.p == r
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        p: number = 0
        m() {
            console.log(this.p)
        }
        q(r: number) {
            return this.p == r
        }
    }

|CB_SEE|
~~~~~~~~

* :ref:`R132`

.. _R137:

|CB_R| ``globalThis`` is not supported
--------------------------------------

|CB_RULE| ``arkts-no-globalthis``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: GlobalThis

|CB_WARNING|

|LANG| does not support both global scope and ``globalThis`` because untyped
objects with dynamically changed layouts are not supported.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // in a global file:
    var abc = 100

    // Refers to 'abc' from above.
    let x = globalThis.abc

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // file1
    export let abc : number = 100

    // file2
    import * as M from "file1"

    let x = M.abc

|CB_SEE|
~~~~~~~~

* :ref:`R139`
* :ref:`R144`

.. _R138:

|CB_R| Some utility types are not supported
-------------------------------------------

|CB_RULE| ``arkts-no-utility-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: UtilityType

|CB_ERROR|

Currently, |LANG| does not support utility types from |TS| extensions to the
standard library, except ``Partial``, ``Required``, ``Readonly``, and
``Record``.

For type ``Record<K, V>``, an indexing expression *rec[index]* is of type
``V | undefined``.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    type Person = {
        name: string
        age: number
        location: string
    }

    type QuantumPerson = Omit<Person, "location">

    let persons : Record<string, Person> = {
        "Alice": {
            name: "Alice",
            age: 32,
            location: "Shanghai"
        },
        "Bob": {
            name: "Bob",
            age: 48,
            location: "New York"
        }
    }
    console.log(persons["Bob"].age)
    console.log(persons["Rob"].age) // Runtime exception

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        name: string = ""
        age: number = 0
        location: string = ""
    }

    class QuantumPerson {
        name: string = ""
        age: number = 0
    }

    type OptionalPerson = Person | undefined
    let persons : Record<string, OptionalPerson> = {
    // Or:
    // let persons : Record<string, Person | undefined> = {
        "Alice": {
            name: "Alice",
            age: 32,
            location: "Shanghai"
        },
        "Bob": {
            name: "Bob",
            age: 48,
            location: "New York"
        }
    }
    console.log(persons["Bob"]!.age)
    if (persons["Rob"]) { // Explicit value check, no runtime exception
        console.log(persons["Rob"].age)
    }

.. _R139:

|CB_R| Declaring properties on functions is not supported
---------------------------------------------------------

|CB_RULE| ``arkts-no-func-props``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: PropertyDeclOnFunction

|CB_ERROR|

|LANG| does not support declaring properties on functions because objects
with dynamically changing layouts are not supported. Function objects follow
this rule, and their layout cannot be changed at runtime.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class MyImage {
        // ...
    }

    function readImage(
        path: string, callback: (err: any, image: MyImage) => void
    )
    {
        // ...
    }

    function readFileSync(path : string) : number[] {
        return []
    }

    function decodeImageSync(contents : number[]) {
        // ...
    }

    readImage.sync = (path: string) => {
        const contents = readFileSync(path)
        return decodeImageSync(contents)
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class MyImage {
        // ...
    }

    async function readImage(
        path: string, callback: (err: Error, image: MyImage) => void
    ) : Promise<MyImage>
    {
        // In real world, the implementation is more complex,
        // involving real network / DB logic, etc.
        return await new MyImage()
    }

    function readImageSync(path: string) : MyImage {
        return new MyImage()
    }

|CB_SEE|
~~~~~~~~

* :ref:`R137`

.. _R140:

|CB_R| ``Function.bind`` is not supported
-----------------------------------------

|CB_RULE| ``arkts-no-func-bind``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: FunctionBind

|CB_WARNING|

|LANG| does not allow using standard library function ``Function.bind``.
The standard library requires this API to explicitly set ``this`` parameter
for the called function.
The semantics of ``this`` in |LANG| is restricted to the conventional OOP
style, and the usage of ``this`` in stand-alone functions is prohibited.
This function is thus excessive.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    const person = {
        firstName: "aa",

        fullName: function(): string {
            return this.firstName
        }
    }

    const person1 = {
        firstName: "Mary"
    }

    // This will log "Mary":
    const boundFullName = person.fullName.bind(person1)
    console.log(boundFullName())

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        firstName : string

        constructor(firstName : string) {
            this.firstName = firstName
        }
        fullName() : string {
            return this.firstName
        }
    }

    let person = new Person("")
    let person1 = new Person("Mary")

    // This will log "Mary":
    console.log(person1.fullName())

|CB_SEE|
~~~~~~~~

* :ref:`R093`

.. _R142:

|CB_R| ``as const`` assertions are not supported
------------------------------------------------

|CB_RULE| ``arkts-no-as-const``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ConstAssertion

|CB_ERROR|

|LANG| does not support ``as const`` assertions as literal types are not
supported altogether, unlike in standard |TS| where ``as const`` annotates
literals with corresponding literal types.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Type 'hello':
    let x = "hello" as const

    // Type 'readonly [10, 20]':
    let y = [10, 20] as const

    // Type '{ readonly text: "hello" }':
    let z = { text: "hello" } as const

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Type 'string':
    let x : string = "hello"

    // Type 'number[]':
    let y : number[] = [10, 20]

    class Label {
        text : string = ""
    }

    // Type 'Label':
    let z : Label = {
        text: "hello"
    }

.. _R143:

|CB_R| Import assertions are not supported
------------------------------------------

|CB_RULE| ``arkts-no-import-assertions``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ImportAssertion

|CB_ERROR|

|LANG| does not support import assertions because import is not a runtime but
a compile-time feature in the language, and asserting correctness of imported
APIs at runtime makes no sense in a statically typed language. Use ordinary
``import`` syntax instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    import { obj } from "something.json" assert { type: "json" }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Correctness of importing T will be checked at compile-time:
    import { something } from "module"

|CB_SEE|
~~~~~~~~

* :ref:`R129`
* :ref:`R130`

.. _R144:

|CB_R| Usage of standard library is restricted
----------------------------------------------

|CB_RULE| ``arkts-limited-stdlib``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: LimitedStdlibApi

|CB_ERROR|

|LANG| does not allow using some APIs from the |TS|/|JS| standard libraries.
Most of the restricted APIs are related to handling objects dynamically, which
is not compatible with static typing. The usage of the following APIs is
prohibited:

Properties and functions of the global object: ``eval``;

``Object``: ``__proto__``, ``__defineGetter__``, ``__defineSetter__``,
``__lookupGetter__``, ``__lookupSetter__``, ``create``,
``defineProperties``, ``defineProperty``, ``freeze``,
``getOwnPropertyDescriptor``, ``getOwnPropertyDescriptors``,
``getOwnPropertySymbols``, ``getPrototypeOf``,
``hasOwnProperty``, ``is``, ``isExtensible``, ``isFrozen``,
``isPrototypeOf``, ``isSealed``, ``preventExtensions``,
``propertyIsEnumerable``, ``seal``, ``setPrototypeOf``;

``Reflect``: ``apply``, ``construct``, ``defineProperty``, ``deleteProperty``,
``getOwnPropertyDescriptor``, ``getPrototypeOf``,
``isExtensible``, ``preventExtensions``,
``setPrototypeOf``;

``Proxy``: ``handler.apply()``, ``handler.construct()``,
``handler.defineProperty()``, ``handler.deleteProperty()``, ``handler.get()``,
``handler.getOwnPropertyDescriptor()``, ``handler.getPrototypeOf()``,
``handler.has()``, ``handler.isExtensible()``, ``handler.ownKeys()``,
``handler.preventExtensions()``, ``handler.set()``,
``handler.setPrototypeOf()``.

The following APIs are partially supported:

``Object.assign(target: Record<string, Object | null | undefined>``,
``...source: Object[]): Record<string, Object | null | undefined>``.

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R002`
* :ref:`R029`
* :ref:`R060`
* :ref:`R066`
* :ref:`R137`

.. _R145:

|CB_R| Strict type checking is enforced
---------------------------------------

|CB_RULE| ``arkts-strict-typing``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: StrictDiagnostic

|CB_ERROR|

Type checker is not optional in |LANG|. The code must be typed explicitly and
correctly to be compiled and to run.
When porting from the standard |TS|, turn on the following flags:

-  ``noImplicitReturns``,
-  ``strictFunctionTypes``,
-  ``strictNullChecks``, and
-  ``strictPropertyInitialization``.


|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        n: number // Compile-time error only with strictPropertyInitialization
        s: string // Compile-time error only with strictPropertyInitialization
    }

    // Compile-time error only with noImplicitReturns
    function foo(s: string): string {
        if (s != "") {
            console.log(s)
            return s
        } else {
            console.log(s)
        }
    }

    let n: number = null // Compile-time error only with strictNullChecks

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        n: number = 0
        s: string = ""
    }

    function foo(s: string): string {
        console.log(s)
        return s
    }

    let n1: number | null = null
    let n2: number = 0

|CB_SEE|
~~~~~~~~

* :ref:`R008`
* :ref:`R146`

.. _R146:

|CB_R| Switching off type checks with in-place comments is not allowed
----------------------------------------------------------------------

|CB_RULE| ``arkts-strict-typing-required``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ErrorSuppression

|CB_ERROR|

Type checker in |LANG| is not optional, the code must be typed explicitly and
correctly to be compiled and to run. 'Suppressing' type checker in-place
with special comments is not allowed. In particular, ``@ts-ignore`` and
``@ts-nocheck`` annotations are not supported.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // @ts-nocheck
    // ...
    // Some code with switched off type checker
    // ...

    let s1: string = null // No error, type checker suppressed

    // @ts-ignore
    let s2: string = null // No error, type checker suppressed

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let s1: string | null = null // No error, properly types
    let s2: string = null // Compile-time error

|CB_SEE|
~~~~~~~~

* :ref:`R008`
* :ref:`R145`

.. _R147:

|CB_R| No dependencies on |TS| code are currently allowed
---------------------------------------------------------

|CB_RULE| ``arkts-no-ts-deps``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: NoTypeScriptDeps

|CB_ERROR|

Currently, the codebase implemented in the standard |TS| language must not
depend on |LANG| through importing the |LANG| codebase. Imports in the reverse
direction are supported.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // app.sts
    export class C {
        // ...
    }

    // lib.ts
    import { C } from "app"


|CB_OK|
~~~~~~~

.. code-block:: typescript

    // lib1.sts
    export class C {
        // ...
    }

    // lib2.sts
    import { C } from "lib1"

.. _R148:

|CB_R| No decorators except ArkUI decorators are currently allowed
------------------------------------------------------------------

|CB_RULE| ``arkts-no-decorators-except-arkui``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: UnsupportedDecorators

|CB_WARNING|

Currently, |LANG| allows ArkUI decorators only.
Any other decorator causes a compile-time error.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function classDecorator(x: any, y: any): void {
        //
    }

    @classDecorator
    class BugReport {
    }


|CB_OK|
~~~~~~~

.. code-block:: typescript

    function classDecorator(x: any, y: any): void {
        //
    }

    @classDecorator // compile-time error: unsupported decorator
    class BugReport {
    }

.. _R149:

|CB_R| Classes cannot be used as objects
----------------------------------------

|CB_RULE| ``arkts-no-classes-as-obj``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ClassAsObject

|CB_WARNING|

|LANG| does not support using classes as objects (assigning classes to
variables, etc.) because a ``class`` declaration introduces a new type,
not a value in the language.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        s: string = ""
        n: number = 0
    }

    let c = C

.. _R150:

|CB_R| ``import`` statements after other statements are not allowed
-------------------------------------------------------------------

|CB_RULE| ``arkts-no-misplaced-imports``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ImportAfterStatement

|CB_ERROR|

All ``import`` statements in |LANG| must precede any other statements in
the program.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        s: string = ""
        n: number = 0
    }

    import foo from "module1"

|CB_OK|
~~~~~~~

.. code-block:: typescript

    import foo from "module1"

    class C {
        s: string = ""
        n: number = 0
    }

.. _R151:

|CB_R| Usage of ``ESObject`` type is restricted
-----------------------------------------------

|CB_RULE| ``arkts-limited-esobj``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: EsObjectType

|CB_WARNING|

|LANG| does not allow using ``ESObject`` type in some cases. Most limitations
are put in place in order to prevent the spread of dynamic objects in a static
codebase. The usage of ``ESObject`` as type specifier is only permitted in the
local variable declaration scenario.
The initialization of ``ESObject`` type variables is also limited. Such
variables can only be initialized with values that originate from interop, i.e.,
other ``ESObject`` typed variables, ``any``, ``unknown``, anonymous type
variables, etc. Initializing an ``ESObject`` typed variable with a statically
typed value is prohibited. An ``ESObject`` typed variable can only be passed
to interop calls and assigned to other variables of type ``ESObject``.

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // lib.d.ts
    declare function foo(): any;
    declare function bar(a: any): number;

    // main.sts
    let e0: ESObject = foo(); // CTE - ``ESObject`` typed variable can only be local

    function f() {
        let e1 = foo(); // CTE - type of e1 is `any`
        let e2: ESObject = 1; // CTE - can't initialize ESObject with not dynamic values
        let e3: ESObject = {}; // CTE - can't initialize ESObject with not dynamic values
        let e4: ESObject = []; // CTE - can't initialize ESObject with not dynamic values
        let e5: ESObject = ""; // CTE - can't initialize ESObject with not dynamic values
        e5['prop'] // CTE - can't access dynamic properties of ESObject
        e5[1] // CTE - can't access dynamic properties of ESObject
        e5.prop // CTE - can't access dynamic properties of ESObject

        let e6: ESObject = foo(); // OK - explicitly annotated as ESObject
        let e7 = e6; // OK - initialize ESObject with ESObject
        bar(e7) // OK - ESObject is passed to interop call
    }

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R002`
* :ref:`R029`
* :ref:`R060`
* :ref:`R066`
* :ref:`R137`

.. _R152:

|CB_R| ``Function.apply``, ``Function.call`` are not supported
--------------------------------------------------------------

|CB_RULE| ``arkts-no-func-apply-call``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: FunctionApplyCall

|CB_ERROR|

|LANG| does not allow using standard library functions ``Function.apply``
and ``Function.call``. These APIs are needed in the standard library to
explicitly set ``this`` parameter for a called function.
The semantics of ``this`` in |LANG| is restricted to the conventional OOP
style, and the usage of ``this`` in stand-alone functions is prohibited.
These functions are thus excessive.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    const person = {
        firstName: "aa",

        fullName: function(): string {
            return this.firstName
        }
    }

    const person1 = {
        firstName: "Mary"
    }

    // This will log "Mary":
    console.log(person.fullName.apply(person1))

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        firstName : string

        constructor(firstName : string) {
            this.firstName = firstName
        }
        fullName() : string {
            return this.firstName
        }
    }

    let person = new Person("")
    let person1 = new Person("Mary")

    // This will log "Mary":
    console.log(person1.fullName())

|CB_SEE|
~~~~~~~~

* :ref:`R093`

.. _R153:

|CB_R| The inheritance for ``Sendable`` classes is limited
----------------------------------------------------------

|CB_RULE| ``arkts-sendable-class-inheritance``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableClassInheritance

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

``Sendable`` classes can inherit only from other ``Sendable`` classes in |LANG|,
and ``Non-Sendable`` classes are not allowed to inherit from ``Sendable``
classes.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {}

    @Sendable
    class B extends A {
        constructor() {
            super()
        }
    }

    @Sendable
    class C {}

    class D extends C {
        constructor() {
            super()
        }
    }

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    @Sendable
    class A {}

    @Sendable
    class B extends A {
        constructor() {
            super()
        }
    }

|CB_SEE|
~~~~~~~~

* :ref:`R154`
* :ref:`R155`
* :ref:`R156`
* :ref:`R157`

.. _R154:

|CB_R| Properties in ``Sendable`` classes and interfaces must have a ``Sendable data`` type
-------------------------------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-prop-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendablePropType

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

All properties of ``Sendable`` classes or interfaces must have
``Sendable data`` types in |LANG|.

``Sendable data`` is data of a type that belongs to one of the following
categories:

* Primitive types: ``boolean``, ``number``, ``string``, ``bigint``,
  ``null``, ``undefined``;
* ``Sendable`` class or interface;
* Type parameter of generic type ``Sendable``;
* ``Const enum`` type;
* Union type with elements that are ``Sendable`` data types.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {}

    interface I {}

    @Sendable
    class B {
        a: A = new A()  // Invalid, 'A' is not Sendable
        b: I = {}       // Invalid, 'I' is not Sendable
    }

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.sts
    @Sendable
    class A {}

    interface I extends ISendable {}

    enum E {
        A,
        B
    }

    // b.sts
    import { A, I, E } from "a"

    @Sendable
    class B {
        a: A = new A()
        b: I = {}
        c: number = 1
        d: string | null | undefined = undefined
        e: E = E.A
    }

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R155`
* :ref:`R156`
* :ref:`R157`

.. _R155:

|CB_R| Definite assignment assertion is not allowed in ``Sendable`` classes
---------------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-definite-assignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableDefiniteAssignment

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not allow using definite assignment assertions on properties of
``Sendable`` classes.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    @Sendable
    class A {
        a!: number;
    }

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    @Sendable
    class A {
        a: number = 1;
    }

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`
* :ref:`R156`
* :ref:`R157`

.. _R156:

|CB_R| Type arguments of generic ``Sendable`` type must be a ``Sendable data`` type
-----------------------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-generic-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableGenericTypes

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

Only ``Sendable data`` types are allowed as type arguments of generic
``Sendable`` types in |LANG|.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {}

    @Sendable
    class B<T> {
        a: T | undefined
    }

    let b = new B<A>()  // Invalid, 'A' is not Sendable

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.sts
    @Sendable
    export class A {
        a: number = 1;
    }

    @Sendable
    export class B<T> {}

    // b.sts
    import { A, B } from "a";

    @Sendable
    class C<T> {
        a: T | undefined = undefined
        b: B<T> = new B<T>();
        c: B<number | undefined> = new B<number | undefined>();
    }

    let c1 = new B<A>()
    let c2 = new B<string>()

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`
* :ref:`R155`
* :ref:`R157`

.. _R157:

|CB_R| Only imported variables can be captured by ``Sendable`` class
--------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-imported-variables``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableImportedVariables

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not support sharing closures at runtime. Therefore, ``Sendable``
classes are not allowed to capture local variable, or use a function or class
from the same module, as otherwise a closure is created. Only imported
variables, classes, and functions can be used inside a ``Sendable`` class body.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let foo: number = 1;

    function bar() {
        console.log("hello")
    }

    @Sendable
    class A {}

    @Sendable
    class B {
        a: A = new A();     // Invalid, 'A' is not imported
        b: number = foo;    // Invalid, 'foo' is not imported

        m(): number {
            bar();          // Invalid, 'bar' is not imported
            return foo;     // Invalid, 'foo' is not imported
        }
    }

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.sts
    export let foo: number = 1;

    export function bar() {
        console.log("hello")
    }

    @Sendable
    export class A {}

    // b.sts
    import {foo, bar, A} from "a"

    @Sendable
    class B {
        a: A = new A();
        b: number = foo;

        m(): number {
            bar();
            return foo;
        }
    }

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`
* :ref:`R155`
* :ref:`R156`

.. _R158:

|CB_R| Only ``@Sendable`` decorator can be used on ``Sendable`` class
---------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-class-decorator``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableClassDecorator

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

Only ``@Sendable`` decorator is allowed on a ``Sendable`` class in |LANG|.
In addition, decorators cannot be applied to fields, methods, accessors,
or constructor/method parameters of a ``Sendable`` class.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    import { DecoratorA, DecoratorB, DecoratorC, DecoratorD } from "decorator"

    @DecoratorA
    @Sendable
    class A {
        @DecoratorB
        a: number = 1

        @DecoratorC
        m(@DecoratorD p: number) {}
    }

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`

.. _R159:

|CB_R| Objects of ``Sendable`` type cannot be initialized by using object literal or array literal
--------------------------------------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-obj-init``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableObjectInitialization

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not support using object literals or array literals to initialize
objects of type ``Sendable``.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    import {collections} from "@arkts.collections.d.sts"

    let a: collections.Array<number> = [1, 2, 3]

    @Sendable
    class A {
        a: number = 1
    }

    let b: A = {a: 2}

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    import {collections} from "@arkts.collections.d.sts"

    let a: collections.Array<number> = new collections.Array<number>([1, 2, 3])

    @Sendable
    class A {
        a: number

        constructor(a: number) {
            this.a = a;
        }
    }

    let b: A = new A(2)

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`

.. _R160:

|CB_R| Computed property names are not allowed in ``Sendable`` classes
----------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-computed-prop-name``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableComputedPropName

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not allow declaring properties in ``Sendable`` classes by using
computed values.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    enum E {
        B = "b"
    }

    @Sendable
    class A {
        ['a']: number = 1
        [E.B]: number = 2
    }

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    @Sendable
    class A {
        a: number = 1
        b: number = 2
    }

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`

.. _R161:

|CB_R| Casting ``Non-sendable`` data to ``Sendable`` type is not allowed
------------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-as-expr``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableAsExpr

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not allow casting *non-sendable* data to ``Sendable`` type.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {}

    @Sendable
    class B {}

    let c: B = new A() as B

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    @Sendable
    class A {}

    class B {}

    let c: B = new A() as B

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`

.. _R162:

|CB_R| Importing a module for side-effects only is not supported in shared modules
----------------------------------------------------------------------------------

|CB_RULE| ``arkts-no-side-effects-imports``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SharedNoSideEffectImport

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not support importing a module for side-effects only in a shared
module.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    import 'module' // Error, importing a module for side-effects
    'use shared'

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    import { X, Y } from 'module'
    'use shared'

|CB_SEE|
~~~~~~~~

* :ref:`R163`
* :ref:`R164`

.. _R163:

|CB_R| Only ``Sendable`` entities can be exported in shared modules
-------------------------------------------------------------------

|CB_RULE| ``arkts-shared-module-exports``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SharedModuleExports

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

Only ``Sendable`` entities can be exported in shared modules in |LANG|.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.sts
    'use shared'

    export enum E { A, B } // Error, regular enum is not sendable
    export class C {} // Error, class C is not sendable
    export let v1: C; // Error, v1 has a non-sendable type

    type T1 = C;
    export { T1 }; // Error, type T1 is aliasing the non-sendable type
    let v2: T1;
    export { v2 }; // Error, v2 has a non-sendable type

    export { D } from 'b'; // Error, re-exporting non-sendable class
    export { v3 } from 'b'; // Error, re-exporting variable with non-sendable type

    // b.sts
    export class D {}
    export let v3: D;

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.sts
    'use shared'

    export const enum E { A, B }

    @Sendable
    export class C {}

    export let v1: C;

    type T1 = C;
    let v2: T1;
    export { T1, v2 };

    export { D, v3, v4 } from 'b';

    // b.sts
    @Sendable
    export class D {}
    export let v3: D;
    export let v4: number;

|CB_SEE|
~~~~~~~~

* :ref:`R162`
* :ref:`R164`

.. _R164:

|CB_R| ``export * from ...`` is not allowed in shared modules
-------------------------------------------------------------

|CB_RULE| ``arkts-shared-module-no-wildcard-export``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SharedModuleNoWildcardExport

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not allow using wildcard exports in shared modules. All exported
entities must be explicitly specified.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.sts
    @Sendable
    export class C {}
    export let a: number;

    // b.sts
    'use shared'
    export * from 'a'; // Error, wildcard export in a shared module

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.sts
    @Sendable
    export class C {}
    export let a: number;

    // b.sts
    'use shared'
    export { C, a } from 'a';

|CB_SEE|
~~~~~~~~

* :ref:`R162`
* :ref:`R163`

.. :comment-begin:
    The following IDs are RESERVED for special rules working only in
    the 'interop' mode (undocumented feature), and used internally
    in ArkTS linter. Therefore, they must NOT be used for regular
    cookbook recipes:

    R165, R166, R167, R168, R169, R170

    See https://gitee.com/openharmony/arkcompiler_ets_frontend/pulls/2397

.. :comment-end:

.. _R165:

|CB_R| Object literal properties can only contain name-value pairs
------------------------------------------------------------------

|CB_RULE| ``arkts-obj-literal-props``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ObjectLiteralProperty

|CB_ERROR|

In |LANG| an object literal is a comma-separated list of name-value pairs
enclosed in curly braces '``{``' and '``}``'. Each name-value pair
consists of an identifier and an expression.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        a: string = ""
        b: number = 0
    }
    const a = "Alice";
    const b = 42;

    let c: C = {
        a,
        b,
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        a: string = ""
        b: number = 0
    }
    const a = "Alice";
    const b = 42;

    let c: C = {
        a: a,
        b: b,
    }

.. _R183:

|CB_R| Optional methods are not supported
-----------------------------------------

|CB_RULE| ``arkts-no-optional-methods``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: OptionalMethod

|CB_ERROR|

|LANG| does not support optional methods, unlike |TS| where methods with '``?``'
are allowed to be not overriden in method overloading, or to be implemented
in interface implementation.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class X {
        throw?() {};
    }

    interface Y {
        throw?(): void;
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class X {
        throw() {};
    }

    interface Y {
        throw(): void;
    }
