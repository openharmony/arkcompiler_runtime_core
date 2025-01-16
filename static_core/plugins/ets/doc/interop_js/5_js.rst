..
    Copyright (c) 2025 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

++++++++++
JavaScript
++++++++++

.. _Features JS:

Features JS
###########

.. _Features JS. Functions:

Functions
*********

Description
^^^^^^^^^^^

To define a function in JS you should use key world ``function``.

.. code-block:: javascript
    :linenos:

    // define the function
    function greet() {
        console.log("Hello World!");
    }

To call a previously declared function, you need to write function name followed by parentheses ``()``.

.. code-block:: javascript
    :linenos:

    // call the function
    greet();

When the greet() function is called, the program's control transfers to the function definition.
All the code inside the function is executed (Hello World! is printed).
Then the program control jumps to the next statement after the function call.

Interop rules
^^^^^^^^^^^^^

- When some JS function is passed through interop to ArkTS, the function object is wrapped to ESObject.

- When some parameters are passed in this function, their values will be converted into JS values.

- The function will be executed in the JS environment, and the return value will be passed through interop to ArkTS.

.. code-block:: javascript
    :linenos:

    // file1.js
    function foo(a) {
        return a;
    }

.. code-block:: typescript
    :linenos:

    // file2.ets
    import { foo } from './file1'

    // foo will be passed to ArkTS world by reference
    // 1 will be passed to JS world by copy
    foo(1)
    let s = foo(1) // `s` is ESObject

    function test1(f: ESObject) {
      return f(1) as number
    }

    // runtime check for foo will delay from the moment it is passed in test1
    // to the moment it is called inside test1
    test1(foo);

- ``Function Expressions`` and ``Arrow function`` will work exactly in the same way. When a variable that contaions ``Function Expressions`` or ``Arrow function`` is passed through interop to ArkTS it works the same way as any function call.

.. code-block:: javascript
    :linenos:

    // file1.js
    let square = function(num) {
        return num * num;
    };

    let mul2 = (num) => {
      return num*2;
    };

.. code-block:: typescript
    :linenos:

    // file2.ets
    import { square, mul2 } from './file1'

    let c = square(5) as Number; // `c` is 25
    let d = mul2(5) as Number; // `d` is 10

Default parameters
******************

Description
^^^^^^^^^^^

Default function parameters allow named parameters to be initialized with default values if no value or ``undefined`` is passed.

.. code-block:: javascript
    :linenos:

    function multiply(a, b = 1) {
        return a * b;
    }

    console.log(multiply(5, 2));
    // Expected output: 10

    console.log(multiply(5));
    // Expected output: 5

Interop rules
^^^^^^^^^^^^^

- Default parameters should be stored on in js-function, so in case of interop no need special actions.

.. code-block:: javascript
    :linenos:

    // file1.js
    function multiply(a, b = 1) {
        return a * b;
    }


.. code-block:: typescript
    :linenos:

    // file2.ets
    import { multiply } from './file1'

    let c = multiply(5) as Number; // `c` is 5
    let d = multiply(5, 2) as Number; // `d` is 10


Arguments
*********

Description
^^^^^^^^^^^

Since ES6, the ``arguments`` object is no longer the only way how to handle variable parameters count.
ES6 introduced a concept called ``rest parameters``.

Here's how the arguments worked:


.. code-block:: javascript
    :linenos:

    function doSomething() {
        arguments[0]; // "A"
        arguments[1]; // "B"
        arguments[2]; // "C"
        arguments.length; // 3
    }

    doSomething("A","B","C");


Arguments limitations: ``Arguments`` object is array-like, not a full-fledged array. That means that useful methods, which arrays have are not available. You cannot use methods such as ``arguments.sort()``, ``arguments.map()`` or ``arguments.filter()``.
The only property you have is ``length``.

Interop rules
^^^^^^^^^^^^^

- Arguments will be passed by the default way through napi, so in case of interop no need special actions.

.. code-block:: javascript
    :linenos:

    // file1.js
    function doSomething() {
        arguments[0]; // "A"
        arguments[1]; // "B"
        arguments[2]; // "C"
        arguments.length; // 3
    }


.. code-block:: typescript
    :linenos:

    // file2.ets
    import { doSomething } from './file1'

    doSomething("A","B","C"); // ok

Rest parameters
***************

Description
^^^^^^^^^^^

Rest parameters means that you can put ``...`` before the last parameter in the function.

.. code-block:: javascript
    :linenos:

    function doSomething(first, second, ...rest) {
        console.log(first); // First argument passed to the function
        console.log(second); // Second argument passed to the function
        console.log(rest[0]); // Third argument passed to the function
        console.log(rest[1]); // Fourth argument passed to the function
        // Etc.
    }

You can access the first two named parameters as usual.
However, all the other arguments passed to the function starting with third are automatically collected to an array called as the last parameter (``rest`` here).

If you pass less than three parameters, ``rest`` will be just empty array.

Unlike ``arguments`` object, ``rest parameters`` give you a real array so that you can use all the array-specific methods.
Moreover, unlike ``arguments``, they do work in ``arrow functions``.

.. code-block:: javascript
    :linenos:

    let doSomething = (...rest) => {
        rest[0]; // Can access the first argument
    };

    let doSomething = () => {
        arguments[0]; // Arrow functions don't have arguments
    };

In addition to the advantages above, ``rest parameters`` are part of the function signature. That means that just from the function "header" you can immediately recognize that it uses ``rest parameters`` and therefore accepts variable number of arguments. With ``arguments`` object, there is no such hint.


Rest parameters limitations

1. You can use them max once in a function, multiple rest parameters are not allowed.

.. code-block:: javascript
    :linenos:

    // This is not valid, multiple rest parameters
    function doSomething(first, ...second, ...third) {}

2. You can use rest parameters only as a last parameter of a function:

.. code-block:: javascript
    :linenos:

    // This is not valid, rest parameters not last
    function doSomething(first, ...second, third) {}

Interop rules
^^^^^^^^^^^^^

- Arguments will be passed by the default way through napi, so in case of interop no need special actions.

.. code-block:: javascript
    :linenos:

    // file1.js
    function doSomething(first, second, ...rest) {
        first; // "A"
        second; // "B"
        rest[0]; // "C"
        rest[1]; // "D"
    }

.. code-block:: typescript
    :linenos:

    // file2.ets
    import { doSomething } from './file1'

    doSomething("A","B","C","D"); // ok

Spread operator(empty)
**********************

Description
^^^^^^^^^^^

Syntactically ``spread operator`` it is the same ``...``, but it works opposite to the ``rest parameters``.
Instead of collecting multiple values in one array, it lets you expand one existing array (or other iterable) into multiple values.

.. code-block:: javascript
    :linenos:

    let numbers = [1, 2, 3];

    // equivalent to
    // console.log(numbers[0], numbers[1], numbers[2])
    console.log(...numbers);

    // Output: 1 2 3


.. code-block:: javascript
    :linenos:

    // Spread operator inside arrays

    // to expand the elements of another array
    let fruits = ["Apple", "Banana", "Cherry"];

    // add fruits array to moreFruits1
    // without using the ... operator
    let moreFruits1 = ["Dragonfruit", fruits, "Elderberry"];

    // spread fruits array within moreFruits2 array
    let moreFruits2 = ["Dragonfruit", ...fruits, "Elderberry"];

    console.log(moreFruits1);
    console.log(moreFruits2);

    // Output: [ 'Dragonfruit', [ 'Apple', 'Banana', 'Cherry' ], 'Elderberry' ]
    //         [ 'Dragonfruit', 'Apple', 'Banana', 'Cherry', 'Elderberry' ]


You can also use the spread operator with object literals:

.. code-block:: javascript
    :linenos:

    // Spread operator with object
    let obj1 = { x : 1, y : 2 };
    let obj2 = { z : 3 };

    // use the spread operator to add
    // members of obj1 and obj2 to obj3
    let obj3 = {...obj1, ...obj2};

    // add obj1 and obj2 without spread operator
    let obj4 = {obj1, obj2};

    console.log("obj3 =", obj3);
    console.log("obj4 =", obj4);

    // Output:
    // obj3 = { x: 1, y: 2, z: 3 }
    // obj4 = { obj1: { x: 1, y: 2 }, obj2: { z: 3 } }


What happens though when you introduce a property with the ``spread operator`` which already exists in the object:

.. code-block:: javascript
    :linenos:

    // Property conflicts
    let firstObject = {a: 1};
    let secondObject = {a: 2};

    let mergedObject = {...firstObject, ...secondObject};
    // a: 2 is after a: 1 so it wins
    console.log(mergedObject); // { a: 2 }


.. code-block:: javascript
    :linenos:

    // Updating immutable objects
    let original = {
        someProperty: "oldValue",
        someOtherProperty: 42
    };

    let updated = {...original, someProperty: "newValue"};
    // updated is now { someProperty: "newValue", someOtherProperty: 42 }



.. code-block:: javascript
    :linenos:

    // Object destructuring
    let myObject = { a: 1, b: 2, c: 3, d: 4};
    let {b, d, ...remaining} = myObject;

    console.log(b); // 2
    console.log(d); // 4
    console.log(remaining); // { a: 1, c: 3 }


.. code-block:: javascript
    :linenos:

    // Spread operator with functions
    let myArray = [1, 2, 3];

    function doSomething(first, second, third) {}

    doSomething(...myArray);
    // Is equivalent to
    doSomething(myArray[0], myArray[1], myArray[2]);

It works with any iterable, not just arrays. For example, using the spread operator with string will disassemble it to the individual characters.

You can combine this with passing individual parameters. Unlike rest parameters, you can use multiple spread operators in the same function call and it does not need to be the last item.

.. code-block:: javascript
    :linenos:

    // All of this is possible
    doSomething(1, ...myArray);
    doSomething(1, ...myArray, 2);
    doSomething(...myArray, ...otherArray);
    doSomething(2, ...myArray, ...otherArray, 3, 7);

Destructing assignment  (empty)
*******************************

Destructuring Assignment is a syntax that allows you to extract data from arrays and objects.

.. code-block:: javascript
    :linenos:

    const user = {firstName: 'Adrian', lastName: 'Mejia'};

    function getFullName({ firstName, lastName }) {
    return `${firstName} ${lastName}`;
    }

    console.log(getFullName(user));
    // Adrian Mejia

Array destructuring  (empty)
****************************

Description
^^^^^^^^^^^

.. code-block:: javascript
    :linenos:


    let myArray = [1, 2, 3, 4, 5];
    let [a, b, c, ...d] = myArray;

    console.log(a); // 1
    console.log(b); // 2
    console.log(c); // 3
    console.log(d); // [4, 5]

Exceptions(empty)
*****************

Description
^^^^^^^^^^^

``throw``
``try catch`` statement

Error Handling:

- Native Error Types Used

    - RangeError

    - ReferenceError

    - SyntaxError

    - TypeError

    - URIError

    - AggregateError

    - EvalError

    - InternalError

Including:

- the cause property on Error objects, which can be used to record a causation chain in errors

Interop rules
^^^^^^^^^^^^^

- JS Error and escompat Error classes are mapped as reference proxy-classes
- If JS throws a value which is not an Error instance, the Error is boxed into JSError/RewrappedESObjectError 2.0 internal class

.. code-block:: javascript
    :linenos:

    // file1.js
    function foo(a) {
      throw new Error();
      return a;
    }

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { foo } from './file1'

    try {
        foo();
    } catch (e: Error) {
        e.message; // ok
    }

Limitations & Solutions
"""""""""""""""""""""""

- If JS throws a value which is not an Error instance, the Error is boxed into JSError/RewrappedESObjectError 2.0 internal class

.. code-block:: javascript
    :linenos:

    // file1.js
    function foo(a) {
      throw 123;
      return a;
    }

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { foo } from './file1'

    try {
        foo();
    } catch (e: Error) {
        if (e instanceof RewrappedESObjectError) {
          let v = e.getValue() as number; // ok, obtain what's actually thrown
        }
    }


Getter/Setter
*************

Description
^^^^^^^^^^^

Getters and setters allow you to define Object Accessors (Computed Properties).

This example uses a lang property to get the value of the language property:

.. code-block:: javascript
    :linenos:

    // Create an object:
    const person = {
      firstName: "John",
      lastName: "Doe",
      language: "en",
      get lang() {
        return this.language;
      }
    };

    // Display data from the object using a getter:
    document.getElementById("demo").innerHTML = person.lang;

This example uses a lang property to set the value of the language property:

.. code-block:: javascript
    :linenos:

    const person = {
      firstName: "John",
      lastName: "Doe",
      language: "",
      set lang(lang) {
        this.language = lang;
      }
    };

    // Set an object property using a setter:
    person.lang = "en";

    // Display data from the object:
    document.getElementById("demo").innerHTML = person.language;


Interop rules
^^^^^^^^^^^^^

- Accesing to getter/setter will do on JS side, so here should not be any additional side effects or limitations, just the same as for functions.

.. code-block:: javascript
    :linenos:

    // file1.js
    class A {
      get val() { return 42};
      set val(val) { console.log(val)};
    }

    export let a = new A();

.. code-block:: typescript
    :linenos:

    // file2.ets
    import { a } from './file1'

    a.val = 35; // ok

Objects (empty)
***************

Description
^^^^^^^^^^^

JavaScript object is a variable that can store multiple data in key-value pairs.

Syntax to create object:

    .. code-block:: javascript
        :linenos:

        const objectName = {
            key1: value1,
            key2: value2,
            ...,
            keyN: valueN
        };

Access object property:

    * Using dot notation
    * Using braket notation

Modify Object Properties:

    .. code-block:: javascript
        :linenos:

        const person = {
            name: "Bobby",
            hobby: "Dancing",
        };

        // modify property
        person.hobby = "Singing";

        // display the object
        console.log(person);

        // Output: { name: 'Bobby', hobby: 'Singing' }

Add Object Properties:

    .. code-block:: javascript
        :linenos:

        const student = {
            name: "John",
            age: 20,
        };

        // add properties
        student.rollNo = 14;
        student.faculty = "Science";

        // display the object
        console.log(student);

        // Output: { name: 'John', age: 20, rollNo: 14, faculty: 'Science' }

Delete Object Properties:

    .. code-block:: javascript
        :linenos:

        const employee = {
            name: "Tony",
            position: "Officer",
            salary: 30000,
        };

        // delete object property
        delete employee.salary

        // display the object
        console.log(employee);

        // Output: { name: 'Tony', position: 'Officer' }

JS Method  (empty)
******************

Description
^^^^^^^^^^^

A JavaScript method is a function defined within an object.

``this`` key word (empty)
*************************

Description
^^^^^^^^^^^

The ``this`` keyword refers to the context where a piece of code, such as a function's body, is supposed to run. Most typically, it is used in object methods, where this refers to the object that the method is attached to, thus allowing the same method to be reused on different objects.

The value of ``this`` in JavaScript depends on how a function is invoked (runtime binding), not how it is defined. When a regular function is invoked as a method of an object (``obj.method()``), ``this`` points to that object. When invoked as a standalone function (not attached to an object: ``func()``), ``this`` typically refers to the ``global object`` (in ``non-strict mode``) or ``undefined`` (in ``strict mode``).

Constructor (empty)
*******************

Description
^^^^^^^^^^^

Objects are not fundamentally class-based.
Objects may be created in various ways including via a literal notation or via constructors which create objects and then execute code that initializes all or part of them by assigning initial values to their properties. Each constructor is a function that has a property named "prototype" that is used to implement prototype-based inheritance and shared properties.
Objects are created by using constructors in new expressions.

.. code-block:: javascript
    :linenos:

    // constructor function
    function Person () {
        this.name = "John",
        this.age = 23
    }

    // create an object
    const person = new Person();


Every object created by a constructor has an implicit reference (called the object's prototype) to the value of its constructor's "prototype" property.

Prototype (empty)
*****************

Description
^^^^^^^^^^^

The state and methods are carried by objects, while structure, behaviour, and state are all inherited.

.. code-block:: javascript
    :linenos:

    //prototyping
    let animal = {
        who: () => console.log("animal"),
        say: () => console.log("arr")
    };

    let dog = {
        __proto__: animal,
        say: () => console.log("woof")
    };

    let puppy = {
        __proto__: dog,
        say: () => console.log("wf")
    };

    dog.who(); /* animal */
    dog.say(); /* woof */

    puppy.who(); /* animal */
    puppy.say(); /* wf */

Every object has prototype, if the object does not have the required property, then the search is performed in the object's prototype. If it is not there either, then in the prototype of the prototype, etc.
The function call can be delegated to prototypes located "above".

Including:

- Object.hasOwn, a convenient alternative to Object.prototype.hasOwnProperty.

Import module (empty)
*********************

Description
^^^^^^^^^^^

.. code-block:: javascript
    :linenos:

    import defaultExport from "module-name";
    import * as name from "module-name";
    import * from 'module-name';
    import { namedExport } from 'module-name';
    import "module-name";

Dynamic import (empty)
**********************

Description
^^^^^^^^^^^

The import declaration syntax (import something from "somewhere") is static and will always result in the imported module being evaluated at load time.
Dynamic imports allow one to circumvent the syntactic rigidity of import declarations and load a module conditionally or on demand.

.. code-block:: javascript
    :linenos:

    import(moduleName)
    import(moduleName, options)

Export (empty)
**************

Description
^^^^^^^^^^^

.. code-block:: javascript
    :linenos:

    export { name1, name2, ..., nameN };
    export default <expression>;
    export * from ...;
    export default function (...) { ... };

.. _Features JS. Classes:

Classes (empty)
***************

Description
^^^^^^^^^^^

Constructor - special method ``constructor``, which is called when the class is initialized with new.

The body of a class is executed in strict mode even without the ``"use strict"`` directive.

A class element can be characterized by three aspects:

    * Kind: Getter, setter, method, or field

    * Location: Static or instance

    * Visibility: Public or private

Parental constructor inherited automatically if the descendant does not have its own method constructor. If the descendant has his own constructor, then to inherit the parent's constructor you need to use ``super()`` with arguments for parent.
The ``extends`` keyword is used in class declarations or class expressions to create a class as a child of another constructor (either a class or a function).
If there is a constructor present in the subclass, it needs to first call super() before using this. The ``super`` keyword can also be used to call corresponding methods of super class.

.. code-block:: javascript
    :linenos:

    class ChildClass extends ParentClass { /* … */ }

Including fetures:
- public and private instance fields
- public and private static fields
- private instance methods and accessors
- private static methods and accessors
- static blocks inside classes, to perform per-class evaluation initialization
- the #x in obj syntax, to test for presence of private fields on objects

Interop rules
^^^^^^^^^^^^^

- Proxing JS class with ESObject.

.. code-block:: javascript
    :linenos:

    // file1.js
    export class A {
      v = 123;
    }

.. code-block:: typescript
    :linenos:

    // file2.ets ArkTS
    import { A } from './file1'

    let val = new A(); // ok, val is ESObject

Iterations (empty)
******************

Description
^^^^^^^^^^^

An ``Iterator`` object is an object that conforms to the iterator protocol by providing a ``next()`` method that returns an iterator result object. All built-in iterators inherit from the Iterator class. The Iterator class provides a [Symbol.iterator]() method that returns the iterator object itself, making the iterator also iterable.
It also provides some helper methods for working with iterators.

* do ... while

* for

* for ... in

.. code-block:: javascript
    :linenos:

    // Syntax
    for (variable in object)
        statement


The loop will iterate over all enumerable properties of the object itself and those the object inherits from its prototype chain (properties of nearer prototypes take precedence over those of prototypes further away from the object in its prototype chain).

Like other looping statements, you can use control flow statements inside statement:

    - ``break`` stops statement execution and goes to the first statement after the loop
    - ``continue`` stops statement execution and goes to the next iteration of the loop


* for..of

* for await ... of (see in asynchronous section)

* while

Relational operators: ``<``, ``>``, ``<=``, ``>=``  (empty)
***********************************************************

* ``<``, ``>``, ``<=``, ``>=`` operators

Relational operators: ``in``  (empty)
*************************************

Relational operators: ``instanceof``  (empty)
*********************************************

* ``instanceof`` operator

Operator ``instanceof`` checks whether an object belongs to a certain class. In other words, object instanceof constructor checks if an object is present constructor.prototype in the prototype chain object.

.. code-block:: javascript
    :linenos:

    function Car(brand, model, year) {
        this.brand = brand;
        this.model = model;
        this.year = year;
    }

    const auto = new Car('Honda', 'Camry', 1998);

    console.log(auto instanceof Car);
    // Expected output: true

    console.log(auto instanceof Object);
    // Expected output: true

Closure (empty)
***************

Description
^^^^^^^^^^^

.. code-block:: javascript
    :linenos:

    // nested function example

    // outer function
    function greet(name) {

        // inner function
        function displayName() {
            console.log('Hi' + ' ' + name);
        }

        // calling inner function
        displayName();
    }

    // calling outer function
    greet('John'); // Hi John

Closure provides access to the outer scope of a function from inside the inner function, even after the outer function has closed.

Object of primitive types (empty)
*********************************

Description
^^^^^^^^^^^

- Null

- Undefined

- Boolean

- Number

- Bigint

- String

- Symbol

    Symbol is a unique and immutable data type that can be used as an identifier for object properties.


The ``typeof`` Operator (empty)
*******************************

Description
^^^^^^^^^^^

The ``typeof`` operator returns a string indicating the type of the operand's value.

   +-------------------------------------------------------------------------------+-----------+
   | Type                                                                          |  Result   |
   +===============================================================================+===========+
   | Undefined                                                                     |"undefined"|
   +-------------------------------------------------------------------------------+-----------+
   | Null                                                                          | "object"  |
   +-------------------------------------------------------------------------------+-----------+
   |Boolean                                                                        | "boolean" |
   +-------------------------------------------------------------------------------+-----------+
   |Boolean                                                                        | "boolean" |
   +-------------------------------------------------------------------------------+-----------+
   |Number                                                                         | "number"  |
   +-------------------------------------------------------------------------------+-----------+
   |BigInt                                                                         | "bigint"  |
   +-------------------------------------------------------------------------------+-----------+
   |String                                                                         | "string"  |
   +-------------------------------------------------------------------------------+-----------+
   |Symbol                                                                         | "symbol"  |
   +-------------------------------------------------------------------------------+-----------+
   |Function (implements [[Call]] in ECMA-262 terms; classes are functions as well)|"function" |
   +-------------------------------------------------------------------------------+-----------+
   |Any other object                                                               | "object"  |
   +-------------------------------------------------------------------------------+-----------+


Generators (empty)
******************

Description
^^^^^^^^^^^

Generators can return (``yield``) multiple values, one after another, on-demand.

    - key word ``yeild``, ``function*``, ``yeild* generator``

    - methods:

        - ``generator.next()`` : returns a value of yield
        - ``generator.return()``: returns a value and terminates the generator
        - ``generator.throw()``: throws an error and terminates the generator

``With`` statement (empty)
**************************

Description
^^^^^^^^^^^

Use of the ``with`` statement is not recommended, as it may be the source of confusing bugs and compatibility issues, makes optimization impossible, and is forbidden in ``strict mode``.
The recommended alternative is to assign the object whose properties you want to access to a temporary variable.


The ``debugger`` statement (empty)
**********************************


Description
^^^^^^^^^^^

The ``debugger`` statement invokes any available debugging functionality, such as setting a breakpoint.
If no debugging functionality is available, this statement has no effect.

Proxies (empty)
***************

Description
^^^^^^^^^^^

The ``Proxy`` object enables you to create a proxy for another object, which can intercept and redefine fundamental operations for that object.

You create a ``Proxy`` with two parameters:

- ``target``: the original object which you want to proxy
- ``handler``: an object that defines which operations will be intercepted and how to redefine intercepted operations.

.. code-block:: javascript
    :linenos:

    const target = {
        message1: "hello",
        message2: "everyone",
    };

    const handler2 = {
        get(target, prop, receiver) {
            return "world";
        },
    };

    const proxy2 = new Proxy(target, handler2);

    console.log(proxy2.message1); // world
    console.log(proxy2.message2); // world

The Global Object (empty)
*************************

Description
^^^^^^^^^^^

The ``global object`` in JavaScript is an object which represents the global scope.

The ``globalThis`` global property allows one to access the ``global object`` regardless of the current environment.

Value properties of the ``Global Object``:

    - globalThis

    - Infinity

    - NaN

    - underfined

Other properties of the ``Global Object``:

    - Atomics

    - JSON (see below)

    - Math

    - Reflect

Reflect (empty)
***************

Description
^^^^^^^^^^^

The ``Reflect`` object provides a collection of static functions which have the same names as the ``proxy`` handler methods.
The ``Reflect`` object has a number of methods that allow developers to access and modify the internal state of an object.

.. code-block:: javascript
    :linenos:

    const person = {
        name: 'John Doe'
    };

    Reflect.set(person, 'name', 'Jane Doe');

    console.log(person.name); // 'Jane Doe'


``Reflect.get()``, ``Reflect.set()``, ``Reflect.apply()``, ``Reflect.construct()`` methods

.. _JS Std library:

Symbol (empty)
**************

JS Std library
##############

Arrays
******

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

- In JS [] and Array are indistinguishable, so interop rules are the same for both of them
- When JS array is passed through interop to ArkTS, the proxy object is constructed in ArkTS and user can work with the array as if it was passed by reference. So any modification to the array will be reflected in JS

.. code-block:: javascript
  :linenos:

  //file1.js
  let a new ;
  export let a = new Array<number>(1, 2, 3, 4, 5);
  export let b = [1, 2, 3, 4 ,5]

.. code-block:: typescript
  :linenos:

  //file2.ets  ArkTS

  import {a, b} from 'file1'
  let val1 = a[0]; // ok
  let val2 = b[0]; // ok
  let val3 = a.lenght; // ok
  let val4 = b.lenght; // ok
  a.push(6); // ok, will affect original Array
  b.push(6); // ok, will affect original Array

Set (empty)
***********

Map (empty)
***********

ArrayBuffer (empty)
*******************

BigInt64Array (empty)
^^^^^^^^^^^^^^^^^^^^^

BigUint64Array (empty)
^^^^^^^^^^^^^^^^^^^^^^

Float32Array (empty)
^^^^^^^^^^^^^^^^^^^^

Float64Array (empty)
^^^^^^^^^^^^^^^^^^^^

Int8Array (empty)
^^^^^^^^^^^^^^^^^

Int16Array (empty)
^^^^^^^^^^^^^^^^^^

Int32Array (empty)
^^^^^^^^^^^^^^^^^^

Uint8Array (empty)
^^^^^^^^^^^^^^^^^^

Uint8ClampedArray (empty)
^^^^^^^^^^^^^^^^^^^^^^^^^

Uint16Array (empty)
^^^^^^^^^^^^^^^^^^^

Uint32Array (empty)
^^^^^^^^^^^^^^^^^^^

DataView (empty)
****************

Date (empty)
************

WeakMap (empty)
***************

WeakSet (empty)
***************

BigInt (empty)
**************

Boolean (empty)
***************

DataView (empty)
****************

Date (empty)
************

Error (empty)
*************

EvalError (empty)
^^^^^^^^^^^^^^^^^

AggregateError (empty)
^^^^^^^^^^^^^^^^^^^^^^

URIError (empty)
^^^^^^^^^^^^^^^^

RangeError (empty)
^^^^^^^^^^^^^^^^^^

ReferenceError (empty)
^^^^^^^^^^^^^^^^^^^^^^

SyntaxError (empty)
^^^^^^^^^^^^^^^^^^^

TypeError (empty)
^^^^^^^^^^^^^^^^^

InternalError (empty)
^^^^^^^^^^^^^^^^^^^^^

FinalizationRegistry (empty)
****************************

Function (empty)
****************

Math (empty)
************

See :ref:`Async and concurrency features JS`


JSON Data (empty)
*****************

Description
^^^^^^^^^^^

``JSON data`` consists of key/value pairs similar to ``JavaScript object`` properties.

.. code-block:: javascript
    :linenos:

    // JSON data
    "name": "John"

JSON Object (empty)
*******************

Description
^^^^^^^^^^^

Contain multiple key/value pairs:

.. code-block:: javascript
    :linenos:

    // JSON object
    { "name": "John", "age": 22 }

- ``JavaScript Objects``VS ``JSON``

- Converting ``JSON`` to ``JavaScript Object`` : using the built-in ``JSON.parse()`` function

- Converting ``JavaScript Object`` to ``JSON`` : ``JSON.stringify()`` function

JSON Array  (empty)
*******************

Description
^^^^^^^^^^^

Is written inside square brackets ``[ ]``:

.. code-block:: javascript
    :linenos:

    // JSON array
    [ "apple", "mango", "banana"]

    // JSON array containing objects
    [
        { "name": "John", "age": 22 },
        { "name": "Peter", "age": 20 }.
        { "name": "Mark", "age": 23 }
    ]

Regular Expression Liteals (empty)
**********************************

Description
^^^^^^^^^^^

Regular expressions are patterns used to match character combinations in strings. Regular expressions are also objects. These patterns are used with the ``exec()`` and ``test()`` methods of RegExp, and with the ``match()``, ``matchAll()``, ``replace()``, ``replaceAll()``, ``search()``, and ``split()`` methods of String.

Including:

- Regular expression match indices via the /d flag, which provides start and end indices for matched substrings.

Standart functions (empty)
**************************

- decodeURI
- decodeURIComponent
- encodeURI
- encodeURIComponent
- eval
- isFinite
- isNaN
- parseFloat
- parseInt

TODO: More std library entities
*******************************

.. _Async and concurrency features JS:

Async and concurrency features JS (empty)
#########################################

async/await (empty)
*******************

Description
^^^^^^^^^^^

``async`` keyword ensures that the function returns a ``promise``, and wraps non-promises in it.

``await`` keyword works only inside ``async`` functions. It wait until the promise settles and returns its result.

.. code-block:: javascript
    :linenos:

    async function f() {

        let promise = new Promise((resolve, reject) => {
            setTimeout(() => resolve("done!"), 1000)
        });

        let result = await promise; // wait until the promise resolves (*)

        alert(result); // "done!"
    }

    f();


Promise Objects (empty)
***********************

Description
^^^^^^^^^^^

A ``Promise`` is a proxy for a value not necessarily known when the ``promise`` is created. It allows you to associate handlers with an asynchronous action's eventual success value or failure reason. This lets asynchronous methods return values like synchronous methods: instead of immediately returning the final value, the asynchronous method returns a ``promise`` to supply the value at some point in the future.

A ``Promise`` is in one of these states:

- pending: initial state, neither fulfilled nor rejected

- fulfilled: meaning that the operation was completed successfully

- rejected: meaning that the operation failed

setTimeout()  (empty)
*********************

setInterval()  (empty)
**********************

Promise.prototype.finally()  (empty)
************************************

Asynchronous iteration for-await-of  (empty)
********************************************

Description
^^^^^^^^^^^

Allows you to call asynchronous functions that return a promise (or an array with a bunch of promises) in a loop:

.. code-block:: javascript
    :linenos:

    const promises = [
    new Promise(resolve => resolve(1)),
    new Promise(resolve => resolve(2)),
    new Promise(resolve => resolve(3))];

    async function testFunc() {
        for await (const obj of promises) {
            console.log(obj);
        }
    }

    testFunc(); // 1, 2, 3

Atomics (empty)
***************

Description
^^^^^^^^^^^

Unlike most global objects, ``Atomics`` is not a constructor. You cannot use it with the ``new`` operator or invoke the ``Atomics`` object as a function.
All properties and methods of ``Atomics`` are static (just like the ``Math`` object).

The ``Atomics`` namespace object are used with ``SharedArrayBuffer`` and ``ArrayBuffer`` objects.
When memory is shared, multiple threads can read and write the same data in memory. Atomic operations make sure that predictable values are written and read, that operations are finished before the next operation starts and that operations are not interrupted.

    - ``wait()`` and ``notify()`` methods

    The ``wait()`` and ``notify()`` methods are modeled on Linux futexes ("fast user-space mutex") and provide ways for waiting until a certain condition becomes true and are typically used as blocking constructs.

Await (empty)
*************

Description
^^^^^^^^^^^

- Including top-level await, allowing the keyword to be used at the top level of modules

