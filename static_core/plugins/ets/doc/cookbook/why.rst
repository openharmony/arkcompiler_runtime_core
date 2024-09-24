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

.. _Why Migrate:

Why Migrate from |TS| to |LANG|
===============================

This chapter explains why it makes sense to migrate from the standard |TS| to
|LANG|. In general, there are two reasons for doing this:

- Program stability. Dynamically typed languages like |JS| are very good at
  allowing programs to write code fast. At the same time, these languages are
  notorious for unexpected runtime errors. For example, you can forget
  to check some value for ``undefined``---as a result, your program would crash
  causing inconvenience to users. Detecting such issues during the development
  would be more beneficial. |TS| helps greatly here: it allows you to annotate
  code with types, and have the compiler detect many errors prior to
  deploying and using the code.
  However, even |TS| has limitations, and sometimes permits annotating code
  with types 'loosely', thus leaving room for runtime errors. |LANG| tries
  to overcome this drawback by enabling static typing for even stricter
  type checking and less runtime errors.
- Program performance. To ensure program correctness, dynamically-typed
  languages have to check actual types of objects when the program actually
  runs. Back to our example, |JS| does not allow reading a property from
  ``undefined``. Yet the only way to check if a value is ``undefined`` is to
  perform a runtime check, which all |JS| engines do: if the value is not
  ``undefined``, then the property can be read; otherwise, an exception is
  thrown. Modern engines can optimize such checks greatly, but these checks
  cannot be eliminated completely, and cause code to slow down. Since the
  standard |TS| compiles to |JS|, any code written in |TS| has exactly the same
  issues as described above. |LANG| addresses this problem. Thanks to static
  typing, |LANG| compiles programs not to |JS| but to a special execution
  format called bytecode. It is faster to execute and easier to optimize even
  further.

The examples below are to explain how |LANG| can help you improve program
stability and performance.

|

.. _Explicit Initialization of Fields for Better Stability:

Explicit Initialization of Fields for Better Stability
------------------------------------------------------

|LANG| requires all fields to be initialized explicitly with some
values---either when the field is declared or in the ``constructor``.
It is similar to ``strictPropertyInitialization`` mode of the standard |TS|.

Consider the following |TS| code:

.. code-block:: typescript

    class Person {
        name: string // Automatically is set to undefined

        setName(n:string): void {
            this.name = n
        }

        getName(): string {
            // Return type "string" hides from the developers the fact
            // that name can be undefined. The most correct would be
            // to write the return type as "string | undefined". By doing so
            // we tell the users of our API about all possible return values.
            return this.name
        }
    }

    let buddy = new Person()
    // Let's assume that the developer forgets to call setName:
    // buddy.setName("John")
    console.log(buddy.getName().length); // runtime exception: name is undefined

It looks as follows in |LANG| due to the explicit initialization:

.. code-block:: typescript

    class Person {
        name: string = "" // The field always is defined

        setName(n:string): void {
            this.name = n
        }

        // The type is string in all cases, null and undefined are impossible.
        getName(): string {
            return this.name
        }
    }

    let buddy = new Person()
    // Let's assume that the developer forgets to call setName:
    // buddys.setName("John")
    console.log(buddy.getName().length); // 0, no runtime error

If ``name`` can be ``undefined``, then you also have to specify it explicitly:

.. code-block:: typescript

    class Person {
        name ?: string // The field may be undefined

        setName(n:string): void {
            this.name = n
        }

        // Compile-time error:
        // name can be "undefined", so we cannot say to those who use this API
        // that it returns only strings:
        getNameWrong(): string {
            return this.name
        }

        getName(): string | undefined { // Return type matches the type of name
            return this.name
        }
    }

    let buddy = new Person()
    // Let's assume that the developer forgets to call setName:
    // buddy.setName("John")

    // Compile-time(!) error: Compiler suspects that we
    // may possibly access something undefined and won't build the code:
    console.log(buddy.getName().length); // The code won't build and run

    console.log(buddy.getName()?.length); // Builds ok, no runtime error

This case demonstrates how code stability and correctness in |LANG| can be
improved by enforcing stricter type checking.

|

.. Null Safety for Better Performance:

Null Safety for Better Performance
----------------------------------

Consider the following code:

.. code-block:: typescript

    function notify(who: string, what: string) {
        console.log(`Dear ${who}, a message for you: ${what}`)
    }

    notify("Jack", "You look great today")

In most cases, the function ``notify`` takes two string variables as input
and produces a new string. However, some "special" values---e.g.,
``notify(null, undefined)``---can be passed to the function. Then a program
continues to run and produce output as expected
(``Dear undefined, a message for you: null``). It looks quite fine at first,
but note that the engine that runs the code must always check for such special
cases to ensure the correct behavior. In pseudocode, something like the
following happens:

.. code-block:: typescript

    function __internal_tostring(s: any): string {
        if (typeof s === "string")
            return s
        if (s === undefined)
            return "undefined"
        if (s === null)
            return "null"
        // ...
    }

Now, imagine the function ``notify`` as a part of a complex, heavy-loaded
system that sends real notifications instead of just writing to the log.
In this scenario, executing all the checks from our ``__internal_tostring``
function can turn into a performance problem.

However, if we can guarantee to the execution engine that the values passed
to the ``notify`` function are only '*real*' strings and never some '*special*'
values such as ``null`` or ``undefined``, then we are 100% sure that no corner
cases can occur during program execution, and checks like ``__internal_tostring``
become redundant. In this particular case, the mechanism can be called
'*null-safety*' as it guarantees that ``null`` is not a valid value of the
``string`` type. With this feature available, the code simply would not build:

.. code-block:: typescript

    function notify(who: string, what: string) {
        console.log(`Dear ${who}, a message for you: ${what}`)
    }

    notify("Jack", "You look great today")
    notify(null, undefined) // Compile-time error

In |TS|, such behavior can be turned on by a special compiler flag called
``strictNullChecks``. As a standard, |TS| is compiled to |JS|, which
does not have such feature, and '*strict null checks*' work at compile-time
only for better type checking. However, |LANG| considers *null-safety* a very
important feature from the perspectives of both stability and performance.
That's why *null-safety* is enabled in the language. As a result, situations
like that in the example above always cause compile-time errors. In exchange,
we provide our running engine with more information that guarantees possible
type values and helps optimizing the performance.

|

|

.. raw:: pdf

   PageBreak
