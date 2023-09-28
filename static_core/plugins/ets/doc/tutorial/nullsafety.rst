Null Safety
===========

|

All types in |LANG| are by default non-nullable, and the value of a type
cannot be null.
It is similar to the |TS| behavior in strict null checking mode
(``strictNullChecks``), but the |LANG| rules are stricter.

In the example below, all lines cause a compile-time error:

.. code-block:: typescript

    let x: number = null    // Compile-time error
    let y: string = null    // ditto
    let z: number[] = null  // ditto

A variable that can have a null value is defined with a union type ``T | null``.

.. code-block:: typescript

    let x: number | null = null
    x = 1    // ok
    x = null // ok
    if (x != null) { /* do something */ }

|

Non-Null Assertion Operator
-----------------------------

A postfix operator '``!``' can be used to assert that its operand is non-null.

If applied to a null value, the operator throws an error. Otherwise, the
type of the value is changed from ``T | null`` to ``T``:

.. code-block:: typescript

    let x: number | null = 1
    let y: number
    y = x + 1  // compile time error: cannot add to a nullable value
    y = x! + 1 // ok

|

Null-Coalescing Operator
------------------------

The null-coalescing binary operator '``??``' checks whether the evaluation
of the left-hand-side expression equals to null. If so, then the result of
the expression is the right-hand-side expression; otherwise, it is the
left-hand-side expression.

In other words, '``a ?? b``' equals the ternary operator '``a != null ? a : b``'.

In the following example, the method ``getNick`` returns a nickname if it is
set; otherwise, an empty string is returned:

.. code-block:: typescript

    class Person {
        // ...
        nick: string | null = null
        getNick(): string {
            return this.nick ?? "" 
        }
    }

|

Optional Chaining
-----------------

Optional chaining operator '``?.``' allows writing code where the evaluation
stops at an expression that partially evaluates to ``null`` or ``undefined``.

.. code-block:: typescript

    class Person {
        nick    : string | null = null
        spouse ?: Person

        setSpouse(spouse: Person) : void {
            this.spouse = spouse
        }

        getSpouseNick(): string | null | undefined {
            return this.spouse?.nick
        }

        constructor(nick: string) {
            this.nick = nick
            this.spouse = undefined
        }
    }

**Note**: the return type of ``getSpouseNick`` must be
``string | null | undefined``, as the method can return ``null`` or
``undefined``.

An optional chain can be of any length and contain any number of '``?.``'
operators.

In the following sample, the output is the person's spouse's nickname if that
person has a spouse, and the spouse has a nickname.

Otherwise, the output is ``undefined``:

.. code-block:: typescript

    class Person {
        nick    : string | null = null
        spouse ?: Person

        constructor(nick: string) {
            this.nick = nick
            this.spouse = undefined
        }
    }

    let p: Person = new Person("Alice")
    console.log(p.spouse?.nick) // print: undefined

|
|
