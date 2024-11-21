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

.. _Features JS:

Features JS
###########

Function call
*************

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

    // 1.js
    function foo(a) {
        return a;
    }

.. code-block:: typescript
    :linenos:

    // 1.sts
    import { foo } from './1.js'

    // foo will be passed to ArkTS world by reference
    // 1 will be passed to JS world by copy
    foo(1)
    // the return value of foo(1) will be passed to ArkTS world by copy
    let s = foo(1)

    function test1(f: ESObject) {
      return f(1) as number
    }

    // runtime check for foo will delay from the moment it is passed in test1
    // to the moment it is called inside test1
    test1(foo);


Function hoisting
*****************

Description
^^^^^^^^^^^

Hoisting is a behavior in which a function or a variable can be used before declaration. 

.. code-block:: javascript
    :linenos:

    sqr0(5); /* hoisting */
    // function declaration
    function sqr0 (val) { return val * val; }

Interop rules
^^^^^^^^^^^^^


Function Expressions
********************

Description
^^^^^^^^^^^

A function expression is a way to store functions in variables.
    
.. code-block:: javascript
    :linenos:

    // store a function in the square variable
    let square = function(num) {
        return num * num;
    };

    console.log(square(5));  

    // Output: 25   

Interop rules
^^^^^^^^^^^^^

Arrow function
**************

Description
^^^^^^^^^^^

.. code-block:: javascript
    :linenos:

    // arrow function syntax
    let func = (arg1, arg2, ..., argN) => expression;


This creates a function func that accepts arguments arg1..argN, then evaluates the expression on the right side with their use and returns its result.          
       
In other words, it's the shorter version of:

.. code-block:: javascript
    :linenos:

    let func = function(arg1, arg2, ..., argN) {
        return expression;
    };

Interop rules
^^^^^^^^^^^^^

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

Spread operator
***************

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

Interop rules
^^^^^^^^^^^^^

Interop will parse and pass any count of parameters and types to any proxy. So no any issues and limitations here.

.. code-block:: typescript
    :linenos:

    //1.sts
    export function foo(x: int, y: int, z :int) {
        console.log(x + y + z);
    }

.. code-block:: javascript
    :linenos:

    //2.js
    import {foo} from `converted_sts_source`;
    let arr = [1, 2, 3];
    foo(...arr);


Array destructuring
*******************

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

Interop rules
^^^^^^^^^^^^^



Exceptions
**********

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

Interop rules
^^^^^^^^^^^^^

- JS and 2.0 exception objects are converted with common interop rules when cross the language boundary
- JS Error and escompat Error classes are mapped as reference proxy-classes
- If JS throws a value which is not an Error instance, the Error is boxed into JSError/RewrappedESObjectError 2.0 internal class

.. code-block:: javascript
    :linenos:

    // 1.js
    function foo(a) {
      throw new Error();
      return a;
    }

.. code-block:: typescript
    :linenos:

    // 1.sts
    import { foo } from './1.js'

    try {
        foo();
    } catch (e: Error) {
        e.message; // ok
    }

Limitations&Solutions
""""""""""""""""""""""

- If JS throws a value which is not an Error instance, the Error is boxed into JSError/RewrappedESObjectError 2.0 internal class

.. code-block:: javascript
    :linenos:

    // 1.js
    function foo(a) {
      throw 123;
      return a;
    }

.. code-block:: typescript
    :linenos:

    // 1.sts
    import { foo } from './1.js'

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

- Accesing to getter/setter will do on JS side, so here should not be any additinal side effects or limitations, just the same as fo functions.

.. code-block:: javascript
    :linenos:

    // 1.js
    class A {
      get val() { return 42};
      set val(val) { console.log(val)};
    }

    export let a = new A();

.. code-block:: typescript
    :linenos:

    // 1.sts
    import { a } from './1.js'

    a.val = 35; // ok

Objects
*******

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

Interop rules
^^^^^^^^^^^^^


JS Method 
*********

Description
^^^^^^^^^^^

A JavaScript method is a function defined within an object.

Interop rules
^^^^^^^^^^^^^


``this`` key word
*****************

Description
^^^^^^^^^^^

The ``this`` keyword refers to the context where a piece of code, such as a function's body, is supposed to run. Most typically, it is used in object methods, where this refers to the object that the method is attached to, thus allowing the same method to be reused on different objects.

The value of ``this`` in JavaScript depends on how a function is invoked (runtime binding), not how it is defined. When a regular function is invoked as a method of an object (``obj.method()``), ``this`` points to that object. When invoked as a standalone function (not attached to an object: ``func()``), ``this`` typically refers to the ``global object`` (in ``non-strict mode``) or ``undefined`` (in ``strict mode``). 


Interop rules
^^^^^^^^^^^^^


Constructor 
***********

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

Interop rules
^^^^^^^^^^^^^


Prototype
*********

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

Interop rules
^^^^^^^^^^^^^


Import module
*************

Description
^^^^^^^^^^^

.. code-block:: javascript
    :linenos:

    import defaultExport from "module-name";
    import * as name from "module-name";
    import * from 'module-name';
    import { namedExport } from 'module-name';
    import "module-name";

Interop rules
^^^^^^^^^^^^^


Dynamic import
**************

Description
^^^^^^^^^^^

The import declaration syntax (import something from "somewhere") is static and will always result in the imported module being evaluated at load time. 
Dynamic imports allow one to circumvent the syntactic rigidity of import declarations and load a module conditionally or on demand. 

.. code-block:: javascript
    :linenos:

    import(moduleName)
    import(moduleName, options)


Interop rules
^^^^^^^^^^^^^


Export
******

Description
^^^^^^^^^^^

.. code-block:: javascript
    :linenos:

    export { name1, name2, ..., nameN };
    export default <expression>;
    export * from ...;
    export default function (...) { ... };

Interop rules
^^^^^^^^^^^^^


Classes
*******

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

Interop rules
^^^^^^^^^^^^^


Iterations
**********

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

Interop rules
^^^^^^^^^^^^^


Relational operators
********************

Description
^^^^^^^^^^^

* ``<``, ``>``, ``<=``, ``>=`` operators

* ``in``

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

Interop rules
^^^^^^^^^^^^^

Closure
********

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

Interop rules
^^^^^^^^^^^^^

Object of primitive types
*************************

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


Interop rules
^^^^^^^^^^^^^


The ``typeof`` Operator
***********************

Description
^^^^^^^^^^^

The ``typeof`` operator returns a string indicating the type of the operand's value.

   +------------------------------------------------------------------------+-----------------+
   | Type                                                                   |  	Result        |
   +========================================================================+=================+
   | Undefined                                                              |"undefined"      |
   +------------------------------------------------------------------------+-----------------+
   | Null                                                                   | "object"        | 
   +------------------------------------------------------------------------+-----------------+
   |Boolean                                                                 | "boolean"       | 
   +------------------------------------------------------------------------+-----------------+
   |Boolean                                                                 | "boolean"       | 
   +------------------------------------------------------------------------+-----------------+
   |Number                                                                  | "number"        | 
   +------------------------------------------------------------------------+-----------------+
   |BigInt                                                                  | "bigint"        | 
   +------------------------------------------------------------------------+-----------------+
   |String                                                                  | "string"        | 
   +------------------------------------------------------------------------+-----------------+
   |Symbol                                                                  | "symbol"        | 
   +-------------------------------------------------------------------------------+----------+
   |Function (implements [[Call]] in ECMA-262 terms; classes are functions as well)|"function"| 
   +-------------------------------------------------------------------------------+----------+
   |Any other object                                                               | "object" | 
   +-------------------------------------------------------------------------------+----------+


Interop rules
^^^^^^^^^^^^^


Generators
**********

Description
^^^^^^^^^^^

Generators can return (``yield``) multiple values, one after another, on-demand. 

    - key word ``yeild``, ``function*``, ``yeild* generator``

    - methods: 

        - ``generator.next()`` : returns a value of yield
        - ``generator.return()``: returns a value and terminates the generator
        - ``generator.throw()``: throws an error and terminates the generator


Interop rules
^^^^^^^^^^^^^


``With`` statement
******************

Description
^^^^^^^^^^^


Use of the ``with`` statement is not recommended, as it may be the source of confusing bugs and compatibility issues, makes optimization impossible, and is forbidden in ``strict mode``. 
The recommended alternative is to assign the object whose properties you want to access to a temporary variable.

Interop rules
^^^^^^^^^^^^^


The ``debugger`` statement
**************************


Description
^^^^^^^^^^^

The ``debugger`` statement invokes any available debugging functionality, such as setting a breakpoint. 
If no debugging functionality is available, this statement has no effect. 

Interop rules
^^^^^^^^^^^^^


Proxies
*******

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

Interop rules
^^^^^^^^^^^^^


The Global Object
*****************

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

Interop rules
^^^^^^^^^^^^^

Reflect
*******

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

Interop rules
^^^^^^^^^^^^^
