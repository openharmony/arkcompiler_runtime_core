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

JS Std library
##############

Arrays
******

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

- In JS [] and Array are indistinguishable, so interop rules are the same for both of them
- When JS array is passed through interop to ArkTs, the proxy object is constructed in ArkTs and user can work with the array as if it was passed by reference. So any modification to the array will be reflected in JS

.. code-block:: javascript
  :linenos:

  //1.js
  let a new ;
  export let a = new Array<number>(1, 2, 3, 4, 5);
  export let b = [1, 2, 3, 4 ,5]

.. code-block:: typescript
  :linenos:

  //1.sts

  import {a, b} from '1.js'
  let val1 = a[0]; // ok
  let val2 = b[0]; // ok
  let val3 = a.lenght; // ok
  let val4 = b.lenght; // ok
  a.push(6); // ok, will affect original Array
  b.push(6); // ok, will affect original Array

ArrayBuffer
***********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

DataView
********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Date
****

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Map
***

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

WeakMap
*******

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Set
***

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

WeakSet
*******

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

AggregateError
**************

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

BigInt
******

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

BigInt64Array
*************

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

BigUint64Array
**************

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Boolean
*******

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

DataView
********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Date
****

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Error
*****

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

EvalError
*********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

FinalizationRegistry
********************

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Float32Array
************

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Float64Array
************

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^


Function
********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^


Int8Array
*********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^


Int16Array
**********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Int32Array
**********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Uint8Array
**********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Uint8ClampedArray
*****************

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Uint16Array
***********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Uint32Array
***********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

URIError
********

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Math
****

Description
^^^^^^^^^^^

Interop rules
^^^^^^^^^^^^^

Promise
*******

See :ref:`Async and concurrency features JS`


``JSON Data``
*************

Description
^^^^^^^^^^^

``JSON data`` consists of key/value pairs similar to ``JavaScript object`` properties. 

.. code-block:: javascript
    :linenos:

    // JSON data
    "name": "John"

Interop rules
^^^^^^^^^^^^^

``JSON Object``
***************

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

Interop rules
^^^^^^^^^^^^^

``JSON Array``
**************

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

Interop rules
^^^^^^^^^^^^^

Regular Expression Liteals
**************************

Description
^^^^^^^^^^^

Regular expressions are patterns used to match character combinations in strings. Regular expressions are also objects. These patterns are used with the ``exec()`` and ``test()`` methods of RegExp, and with the ``match()``, ``matchAll()``, ``replace()``, ``replaceAll()``, ``search()``, and ``split()`` methods of String.

Interop rules
^^^^^^^^^^^^^


Standart functions
******************

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
