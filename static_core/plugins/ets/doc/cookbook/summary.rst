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

.. _Recipes Summarized:

Recipes Summarized
==================

This chapter provides an informal summary of |TS| features that |LANG| either
can support with limitations, or cannot support. See :ref:`Recipes` for the
full list with more detailed code examples and workaround suggestions.

|

.. _Static Typing is Enforced:

Static Typing is Enforced
-------------------------

|LANG| was designed with the following goals in mind:

- |LANG| programs must be easy for a developer to read and understand because
  code is read more often than written;
- |LANG| must execute fast and consume as little power as possible because
  it is particularly critical on mobile devices which |LANG| targets.


One of the most important features of |LANG| that helps achieving both goals
is static typing. A statically typed program has all types known at compile
time. As a result, understanding what data structures are used in the code
is much easier.

That all types are known before a program actually runs results in the
compiler verifying code correctness, eliminating many runtime type checks,
and improving performance. To achieve this, the usage of type ``any`` is
prohibited in |LANG|.

|

Example
~~~~~~~

.. code-block:: typescript

    //
    // Not supported:
    //

    let res : any = some_api_function("hello", "world")

    // What is `res`? A numeric error code? Some string? An object?
    // How should we work with it?

    //
    // Supported:
    //

    class CallResult {
        public succeeded() : boolean { ... }
        public errorMessage() : string { ... }
    }

    let res : CallResult = some_api_function("hello", "world")
    if (!res.succeeded()) {
        console.log("Call failed: " + res.errorMessage())
    }

Rationale and Impact
~~~~~~~~~~~~~~~~~~~~

Our research and experiments let us conclude that ``any`` is not welcome already
in |TS|. According to our measurements, ``any`` is used in approximately 1% of
|TS| codebases. Moreover, today's code linters (e.g., ESLint) include a set
of rules that prohibit the usage of ``any``.

Prohibiting ``any`` provides a strong positive impact on the performance at the
cost of low-effort code refactoring.

|

.. _Changing Object Layout in Runtime Is Prohibited:

Changing Object Layout in Runtime Is Prohibited
-----------------------------------------------

To achieve maximum performance benefits, |LANG| requires the layout of objects
to not change during program execution. In other words, it is prohibited to do
the following:

- Add new properties or methods to objects;
- Delete the existing object properties or methods;
- Assign values of arbitrary types to object properties.


Note that many such operations are already prohibited by the |TS|
compiler. However, |TS| compiler still can be "tricked", e.g., by ``as any``
casts. |LANG| does not support such prohibited casts completely as shown in
the detailed example below.

Example
~~~~~~~

.. code-block:: typescript

    class Point {
        public x : number = 0
        public y : number = 0

        constructor(x : number, y : number) {
            this.x = x
            this.y = y
        }
    }

    /* It is impossible to delete a property 
       from the object. It is guaranteed that
       all Point objects have the property x:
    */
    let p1 = new Point(1.0, 1.0)
    delete p1.x           // Compile-time error in TypeScript and ArkTS
    delete (p1 as any).x  // OK in TypeScript, compile-time error in ArkTS

    /* Class Point does not define any property
       named `z`, and it is impossible to add
       it while the program runs.
    */
    let p2 = new Point(2.0, 2.0)
    p2.z = "Label";         // Compile-time error in TypeScript and ArkTS
    (p2 as any).z = "Label" // OK in TypeScript, compile-time error in ArkTS

    /* It is guaranteed that all Point objects
       have only properties x and y, it is
       impossible to generate some arbitrary
       identifier and use it as a new property:
    */
    let p3 = new Point(3.0, 3.0)
    let prop = Symbol();     // OK in TypeScript, compile-time error in ArkTS
    (p3 as any)[prop] = p3.x // OK in TypeScript, compile-time error in ArkTS
    p3[prop] = p3.x          // Compile-time error in TypeScript and ArkTS

    /* It is guaranteed that all Point objects
       have properties x and y of type number,
       so assigning a value of any other type
       is impossible:
    */
    let p4 = new Point(4.0, 4.0)
    p4.x = "Hello!";         // Compile-time error in TypeScript and ArkTS
    (p4 as any).x = "Hello!" // OK in TypeScript, compile-time error in ArkTS

    // Usage of Point objects which is compliant with the class definition:
    function distance(p1 : Point, p2 : Point) : number {
        return Math.sqrt(
          (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y)
        )
    }
    let p5 = new Point(5.0, 5.0)
    let p6 = new Point(6.0, 6.0)
    console.log("Distance between p5 and p6: " + distance(p5, p6))

Rationale and Impact
~~~~~~~~~~~~~~~~~~~~

An unpredictable change of an object layout contradicts both good readability
and code performance. Having class definition at one place, and modifying
actual object layout elsewhere is confusing and error-prone from the developer's
point of view. It opposes the idea of static typing (why adding or removing
additional properties if typing is to be as explicit as possible?), and requires
extra runtime support that causes undesired execution overhead.

According to our observations and experiments, this feature is already not
welcome in |TS|: it is used in a marginal number of real-world projects,
and state-of-the-art linters have rules that prohibit the usage.

We conclude that prohibiting runtime changes to object layouts provides a
strong positive impact on the performance at the cost of low-effort refactoring.

|

.. _Semantics of Operators Is Restricted:

Semantics of Operators Is Restricted
------------------------------------

To achieve better performance and encourage developers to write clearer code,
|LANG| restricts the semantics of some operators. An example is given below,
while the full list of restrictions is outlined in :ref:`Recipes`.

Example
~~~~~~~

.. code-block:: typescript

    // Unary `+` is defined only for numbers, but not for strings:
    console.log(+42) // OK
    console.log(+"42") // Compile-time error

Rationale and Impact
~~~~~~~~~~~~~~~~~~~~

Loading language operators with extra semantics overcomplicates the language
specification, makes the developers remember all possible corner cases with
appropriate handling rules, and in some cases causes some undesired runtime
overhead.

According to our observations and experiments, this feature is not popular
already in |TS|. It is used in less than 1% of real-world codebases, and such
cases are easy to refactor.

Restricting the operator semantics results in a clearer code that can
perform better at the cost of low-effort changes.

|

.. _Structural Typing Is Not Supported:

Structural Typing Is Not Supported (Yet)
----------------------------------------

Assume that two unrelated classes ``T`` and ``U`` have the same public API:

.. code-block:: typescript

    class T {
        public name : string = ""

        public greet() : void {
            console.log("Hello, " + this.name)
        }
    }

    class U {
        public name : string = ""

        public greet() : void {
            console.log("Greetings, " + this.name)
        }
    }

Can we assign a value of ``T`` to a variable of ``U``?

.. code-block:: typescript

    let u : U = new T() // Is this allowed?

Can we pass a value of ``T`` to a function that accepts a parameter of ``U``?

.. code-block:: typescript

    function greeter(u : U) {
        console.log("To " + u.name)
        u.greet()
    }

    let t : T = new T()
    greeter(t) // Is this allowed?

In other words, we are to take one of the following approaches:

- ``T`` and ``U`` are not related by inheritance or any common interface, but
  are "somewhat equivalent" since they have the same public API, and thus the
  answer to both questions above is "yes";
- ``T`` and ``U`` are not related by inheritance or any common interface, and
  must be considered totally different types at any time, and thus the answer
  to both questions above is "no".

The languages that take the first approach are said to support structural
typing. The languages that take the second approach do not support structural
typing. Currently, |TS| supports structural typing, and |LANG| does not.

It is debatable whether or not structural typing helps to produce a clearer
and more understandable code as both *pro* and *contra* arguments can be found.
Why not just support it then? The reason is that structural typing support is
a major feature that needs much consideration and care for the implementation
in the language specification, compiler, and runtime. More importantly, in the
case of |LANG| that enforces static typing (see above), runtime support for
structural typing implies performance overhead.

Since functionally correct and performant implementation requires taking so
many aspects into account, the structural typing support is postponed.
The |LANG| team is ready to reconsider based on real-world scenarios and
feedback. More cases and suggested workarounds can be found in :ref:`Recipes`.

|

.. raw:: pdf

   PageBreak


