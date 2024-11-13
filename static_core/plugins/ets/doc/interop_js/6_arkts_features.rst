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

Features ArkTS 2.0
##################

Functions
*********

- When some ArkTS function is passed through interop to JS, the proxy function is constructed in JS and user can work with the function as if it was passed by reference.
- When some parameters are passed in this function, their values will be converted into ArkTS values.
- The function will be executed in the ArkTS environment, and the return value will be passed through interop to JS. Further, some runtime type checks are applied to ArkTS functions.

.. code-block:: typescript
  :linenos:

  //1.sts
  export function foo(val: int) : void;


  export let a = new A();

.. code-block:: javascript
  :linenos:

  //2.js
  import {foo} from `converted_sts_source`;
  foo(1); // ok

Limitations
===========
- On JS side function call should match the signature, or will be RTE

.. code-block:: typescript
  :linenos:

  //1.sts
  export function foo(val: int) : void;


  export let a = new A();

.. code-block:: javascript
  :linenos:

  //2.js
  import {foo} from `converted_sts_source`;
  foo("1"); // RTE
  foo(); // RTE
  foo(1, 2); // RTE

Solutions
---------
- Arguments can be coverted on JS side

.. code-block:: typescript
  :linenos:

  //1.sts
  export function foo(val: int) : void;


  export let a = new A();

.. code-block:: javascript
  :linenos:

  //2.js
  import {foo} from `converted_sts_source`;
  foo(Number("1")); // ok

- Static function declaration can be fixed on ArkTS 2.0 side

.. code-block:: typescript
  :linenos:

  //1.sts
  export function foo(val: int1, val: int2) : void;
  export function fooZ() : void {
    return foo(0, 0); // default values
  };

  export let a = new A();

.. code-block:: javascript
  :linenos:

  //2.js
  import {foo, fooZ} from `converted_sts_source`;
  fooZ(); // ok
  foo(1, 2); // ok

Exceptions
**********

- JS and 2.0 exception objects are converted with common interop rules when cross the language boundary
- JS Error and escompat Error classes are mapped as reference proxy-classes


.. code-block:: typescript
  :linenos:

  //1.sts
    function foo(a) {
      throw new Error();
      return a;
    }

.. code-block:: javascript
  :linenos:

  //2.js
  import {foo} from `converted_sts_source`;
    try {
        foo();
    } catch (e: Error) {
        e.message; // ok
    }

Overloading
***********

- Overloading in ArkTS 2.0 has more features than ArkTS 1.0. It will be very hard to achieve the semantic of 2.0 overload resolution in interop.

Limitations
===========

- Developer can not use static overloaded fucntios in dynamic source code

.. code-block:: typescript
  :linenos:

  //1.sts
  export function foo(val: int) : void;
  export function foo(string: val): void


  export let a = new A();

.. code-block:: javascript
  :linenos:

  //2.js

  import {foo} from `converted_sts_source`; // runtime exception for ambiguous import

Solutions
---------

- User can change names of static funtions and do not overloading for them

.. code-block:: typescript
  :linenos:

  //1.sts
  export function fooInt(val: int) : void;
  export function fooStr(string: val): void

.. code-block:: javascript
  :linenos:

  //2.js

  import {fooInt, fooStr} from `converted_sts_source`; // ok
  fooInt(1); // ok
  fooStr("hi"); // ok

Rest/Spread parameters
**********************

- ArkTS 2.0 can pass any count of parameters and types to any ESObject. So no any issues and limitations here.

.. code-block:: javascript
    :linenos:

    // 1.js
    function foo(x, y, z) {
      console.log(x + y + z);
    }

.. code-block:: typescript
    :linenos:

    // 1.sts
    import { foo } from './1.js'

    let arr = [1, 2, 3];
    foo(...arr);

Getter/Setter
*************

- Accesing to getter/setter will do on JS side, so here should not be any additinal side effects or limitations, just the same as fo functions.

.. code-block:: typescript
  :linenos:

  //1.sts
  class A {
      get val() { return 42};
      set val(v : number) { console.log(v)};
  }

  export let a = new A();

.. code-block:: javascript
  :linenos:

  //2.js

  import {a} from `converted_sts_source`; // ok
  a.val = 23 ; // ok

TODO: More features
*******************
