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

Promise
*******

See :ref:`Async and concurrency features JS`

TODO: More std library entities
*******************************
