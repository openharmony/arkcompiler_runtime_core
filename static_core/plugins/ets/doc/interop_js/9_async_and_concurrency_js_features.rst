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

.. _Async and concurrency features JS:

Async and concurrency features JS
#################################

``async/await``
***************

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

Interop rules
^^^^^^^^^^^^^

``Promise`` Objects
*******************

Description
^^^^^^^^^^^

A ``Promise`` is a proxy for a value not necessarily known when the ``promise`` is created. It allows you to associate handlers with an asynchronous action's eventual success value or failure reason. This lets asynchronous methods return values like synchronous methods: instead of immediately returning the final value, the asynchronous method returns a ``promise`` to supply the value at some point in the future.

A ``Promise`` is in one of these states:

- pending: initial state, neither fulfilled nor rejected

- fulfilled: meaning that the operation was completed successfully

- rejected: meaning that the operation failed

Interop rules
^^^^^^^^^^^^^

``setTimeout()``
****************

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

``setInterval()``
*****************

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^


``Promise.prototype.finally()``
*******************************

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^


Asynchronous iteration ``for-await-of``
***************************************

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

Interop rules
^^^^^^^^^^^^^


Atomics
*******

Description
^^^^^^^^^^^

Unlike most global objects, ``Atomics`` is not a constructor. You cannot use it with the ``new`` operator or invoke the ``Atomics`` object as a function. 
All properties and methods of ``Atomics`` are static (just like the ``Math`` object).

The ``Atomics`` namespace object are used with ``SharedArrayBuffer`` and ``ArrayBuffer`` objects.
When memory is shared, multiple threads can read and write the same data in memory. Atomic operations make sure that predictable values are written and read, that operations are finished before the next operation starts and that operations are not interrupted.

    - ``wait()`` and ``notify()`` methods

    The ``wait()`` and ``notify()`` methods are modeled on Linux futexes ("fast user-space mutex") and provide ways for waiting until a certain condition becomes true and are typically used as blocking constructs.


Interop rules
^^^^^^^^^^^^^
