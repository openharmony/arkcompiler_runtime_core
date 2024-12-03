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

ArkTS 2.0 Std library
#####################

STDLib Mimic-proxy
******************

- Interop runtime may create a mimic-class that inherits some class(standart library) or implements an interface and intercepts all the defined operations so everything is redirected to the original JS Object
- ArkTS 2.0 Mimic-class is lazily constructed for any 2.0 class or interface if necessary
- ArkTS 2.0 Mimic-instance wraps the JS original object so it can be used as the 2.0 type
- This feature will be implemented for part of JS Std lib classes, at least for: ``Array``, ``Map``, ``Set``, ``Error``

- In this example ``foo`` is pure static function without any abilities to get ``ESObject`` as parameter. That's why it should be repacked to a special mimic-proxy way.

.. code-block:: typescript
  :linenos:

  //1.sts
  export function foo(a: Array<string>) {
      a.push("aaa"); // ok
  }

  export let a = new A();

.. code-block:: javascript
  :linenos:

  //2.js

  import {foo} from 'converted_sts_source'; // ok
  let arr = ["hello", "str"];
  foo(arr); // ok, now arr is ["hello", "str", "aaa"]. But without 'Mimic-proxy' feature we will get RTE here. That's why we need it.

Limitations
===========

- ArkTS 2.0 Mimic-class will follow strict type checking for elements

.. What about type erasure here???

.. code-block:: typescript
  :linenos:

  //1.sts
  export function foo(a: Array<string>) {
      a.push("aaa"); // ok
      a.at(0); // RTE, type of element [0] is number
  }

  export let a = new A();

.. code-block:: javascript
  :linenos:

  //2.js

  import {foo} from 'converted_sts_source'; // ok
  let arr = [1, "hello"];
  foo(arr); // RTE

Solutions
---------

- Probably can be prepared way to explicit conversion to ESObject for such cases. Need clarify.

Arrays
******

- In ArkTs [] and Array are two different types:  [] is a builtin array and has different layout from an Array which is a stdlib class, so interop rules for these two are different.
- When ArkTs Array is passed through interop to JS, the proxy object is constructed in JS, and user works with JS Array as if the original ArkTs array was passed by reference, so modifying the JS array will affect the original ArkTs Array.

.. code-block:: typescript
  :linenos:

  //1.sts
  let a new ;
  export let a = new Array<number>(1, 2, 3, 4, 5));
  export let b = [1, 2, 3, 4 ,5]

.. code-block:: javascript
  :linenos:

  //2.js

  import {a, b} from 'converted_sts_source'
  let val1 = a[0]; // ok
  let val2 = b[0]; // ok
  let val3 = a.lenght; // ok
  let val4 = b.lenght; // ok
  a.push(6); // ok

Limitations
===========

- When ArkTs builtin array [] is passed through interop to JS, the elements passed to the JS Array, so modifying the JS array will not affect the original ArkTs builtin array [].

.. code-block:: typescript
  :linenos:

  //1.sts
  let a new ;
  export let b = [1, 2, 3, 4 ,5]

.. code-block:: javascript
  :linenos:

  //2.js

  import {a, b} from 'converted_sts_source'
  b.push(6); // ok, but will not affect original sts builtin array []

Solutions
---------

- In ArkTS 2.0 side user can use Array type instead of builtin array []

.. code-block:: typescript
  :linenos:

  //1.sts
  let a new ;
  export let b = new Array<number>(1, 2, 3, 4, 5);

.. code-block:: javascript
  :linenos:

  //2.js

  import {a, b} from 'converted_sts_source'
  b.push(6); // ok

Promise
*******

See :ref:`Async and concurrency features ArkTS 2.0`

TODO: More std library entities
*******************************
