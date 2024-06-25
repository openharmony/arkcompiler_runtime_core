..
    Copyright (c) 2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

Features
============

In this section there are some examples for each feature.
Prepared pieces of code will give ETS-Warnings if
compilation happens. Warning messages are added to examples as comments.

After badly performing code there is a rewritten version
of ArkTS code with special note on each fixed line.


Feature #1: Show implicit Boxing and Unboxing conversions
---------------------------------------------------------------------------------

|CB_RULE| ``ets-implicit-boxing-unboxing``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rationales
~~~~~~~~~~~~~~

|LANG| supports Boxing and Unboxing conversions, which perform badly. Use
other constructions or types to avoid extra Boxing/Unboxing conversions and boost
performance from 44% to 97.5%.

|LANG|
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let k: Int = 5;

|LANG| efficient
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let k: int = 5;



Feature #2: Boost Equality Statements
--------------------------------------------

|CB_RULE| ``ets-boost-equality-statement``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rationales
~~~~~~~~~~~~~~

|LANG| supports reference binary operators. If left part of statement
is nullable type and right part is reference type, change sides of expression -
it will work about 12% faster then in another order. Result of expression for both cases is the same.

|LANG| badly performing
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let x: Int = new Int(5);

    if (x == null) { // ETS Warning: Boost Equality Statement. Change sides of binary expression.
        console.println("slow way");
    }

    let k: boolean = x == null; // ETS Warning: Boost Equality Statement. Change sides of binary expression.

|LANG| more efficient
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let x: Int = new Int(5);

    if (null == x) { // Fixed ETS-Warning
        console.println("performance way");
    }

    let k: boolean = null == x; // Fixed version - sides of binary expression changed.


or

|LANG| badly performing
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    function main() {
        let i: Char = 15 as char; // ETS Warning: Implicit Boxing to Char in Variable Declaration.
        let i1: Float = 5.0; // ETS Warning: Implicit Boxing to Float in Variable Declaration.
        let i4: Long = 5 as long; // ETS Warning: Implicit Boxing to Long in Variable Declaration.
    }

|LANG| more efficient version 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    function main() {
        let i: char = 15 as char; // Fixed version
        let i1: float = 5.0; // Fixed version
        let i4: long = 5 as long; // Fixed version
    }

or

|LANG| more efficient version 2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    function main() {
        let i: Char = 15 as Char; // Fixed version: explicit Boxing
        let i1: Float = 5.0 as Float; // Fixed version: explicit Boxing
        let i4: Long = 5 as Long; // Fixed version: explicit Boxing
    }


Feature #3: Prohibit Top-Level statements
----------------------------------------------------

|CB_RULE| ``ets-phohibit-top-level-statements``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rationales
~~~~~~~~~~~~~~

|LANG| supports Top-Level statements. However, they can lead to slower
startup times for application because of executing before the application's entry point.
Encapsulate code within methods or classes.

|LANG| bad startup
~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    console.println("Hello, World"); // ETS Warning: Prohibit top-level statements - call expression

    let x: int = 5; // ETS Warning: Prohibit top-level statements - assignment expression.

    if (x == 6) { // ETS Warning: Prohibit top-level statements.
        console.println("Oh no, 5 is equal to 6!");
    }

|LANG| better startup
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // Fixing example: wrap call, assignment expressions and ``if`` statement in new function
    function greeting(): void {
        console.println("Hello, World");

        let x: int = 5;

        if (x == 6) {
            console.println("Oh no, 5 is equal to 6!");
        }
    }

Feature #4: Use Coroutines instead of Async-functions
-----------------------------------------------------------

|CB_RULE| ``ets-remove-async``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rationales
~~~~~~~~~~~~~~

|LANG| supports Async-functions and Coroutines. Async-function
type is only supported for the backward |TS| compatibility.
System ArkTS suggests to use Coroutines instead of Async-functions.

|LANG| bad way
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let num_of_cycles = 30000

    async function foo(): Promise <int> {
        return 1;
    }

    function bench_body(): void {
        for (let i = 0; i < num_of_cycles; i++) {
            let promise = foo(); // ETS Warning: Replace asynchronous function with coroutine.
        }
    }

    function main(): void {
        bench_body();
    }

|LANG| better way
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let num_of_cycles = 30000

    async function foo(): Promise <int> {
        return 1;
    }

    function bench_body(): void {
        for (let i = 0; i < num_of_cycles; i++) {
            let promise = launch foo(); // Changed to coroutine way - begin
            let i = await promise; // Changed to coroutine way - end
        }
    }

    function main(): void {
        bench_body();
    }

Feature #5: Suggest final modifier for classes and methods
----------------------------------------------------------

|CB_RULE| ``ets-suggest-final``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rationales
~~~~~~~~~~~~~~

|LANG| classes and methods have 'open' modifier as default. This requires runtime resolution.
Making class or method ``final`` allows more efficient calls - up to 67% better performance.

|LANG| badly performing
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {
        foo(): String {
            return "foo";
        };
    }

    class K extends A { // ETS Warning: Suggest 'final' modifier for class
        foo_to_suggest(): void {}; // ETS Warning: Suggest 'final' modifier for method.
        override foo(): String { // ETS Warning: Suggest 'final' modifier for method.
            return "overridden_foo";
        }
    }

|LANG| more efficient
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {
        foo(): String {
            return "foo";
        };
    }

    final class K extends A {
        final foo_to_suggest(): void {};
        final override foo(): String {
            return "overridden_foo";
        }
    }

Feature #6: Using function call instead of lambda
----------------------------------------------------

|CB_RULE| ``ets-remove-lambda``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rationales
~~~~~~~~~~~~~~

|LANG| supports lambda calls. However, using function call is 4 time faster
then lambda. System ArkTS suggests to use function calls.

|LANG| badly performing
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let foo: (i: int) => int
    foo = (i: int): int => {return i + 1} // ETS Warning: Replace the lambda function with a regular function.

|LANG| more efficient
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // Fixed version: function call
    function foo(i: int) : int {
        return i + 1
    }
