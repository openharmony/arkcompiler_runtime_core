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

Features JS
###########

Functions
*********

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

Exceptions
**********

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
=====================
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

Rest/Spread parameters
**********************

- Interop will parse and pass any count of parameters and types to any proxy. So no any issues and limitations here.

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

Getter/Setter
*************

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

TODO: More features
*******************
