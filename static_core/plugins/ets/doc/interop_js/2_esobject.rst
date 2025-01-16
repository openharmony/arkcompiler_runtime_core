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

.. _ESObject:

++++++++++++++++++++
ESObject in ArkTS
++++++++++++++++++++

- ESObject type is a special type in ArkTS. It is separate from Object type.
- ESObject type can be used in following ways without compliation type check: ``operator new``, ``operator []``, ``operator ()``, ``operator .``, ``operator as``. Moreover, the types of results of ``operator new``, ``operator []``, ``operator ()``, ``operator .`` are still ESObject and they will be called on JS-side.
- In bytecode of ArkTS, all entities imported from TS/JS will be of type ESObject.
- Values of ESObject type can be converted to and from Object type. However, implicitly converting an object of ESObject type to any other type is forbidden.
- Values of ESObject type may wrap entities from TS/JS, they also may wrap normal objects of types in ArkTS.
- ESObject can be used as type parameters in generics.
- It is not allowed to define an object literal of type ESObject.

Examples
********

.. code-block:: javascript
    :linenos:

    // test.js
    class A {
      v = 123
    }
    export let a = new A()

    export function foo(v) {}

.. code-block:: typescript
    :linenos:

    // app.ets
    import { a, foo } from './test'  // a is of ESObject type
    a.v  // ok
    a[index] // ok, will be called operator [] on JS-side
    a() // ok, no CTE, but RTE, no operation () on JS side
    new a() // no CTE, but RTE, no operation new on JS side

    let n1: number = a.v  // CTE
    let n2: number = a.v as number // ok
    let n3: ESObject = a.v  // ok
    a.v + 1  // CTE
    (a.v as number) + 1  // ok

    let s1: ESObject = 1;  // ok, implicit conversion
    let s2: ESObject = new StaticClass() // ok, implicit conversion
    let s3: ESObject = 1 as ESObject; // ok, explicit conversion
    let s4: ESObject = new StaticClass() as ESObject; // ok, explicit conversion

    foo(1); // ok, implicitly convert 1 to the ESObject type
    foo(new StaticClass()); // ok, implicitly convert static object to the ESObject type

    let arr = new Array<string, ESObject>();  // ok, ESObject is used as a type parameter

    let point: ESObject = {x: 0, y: 1};  // CTE, cannot create object literal of type ESObject

    foo({x: 0, y: 1});  // CTE, cannot create object literal of type ESObject

-  When using ``as`` keyword if ArkTS type doesn't map to JS type then a runtime exception will be thrown.

.. code-block:: typescript
  :linenos:

  // file1.js
  export x = 1;

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  import { x } from "file1";
  const valStr = x as string; // RTE, cannot cast number to string

-  When primitive type values are copied to ArkTS they become ESObject that contains primitive value.
   In that case operators ``[]``, ``()``, ``.``, and ``new`` have no meaning and their usage will throw an exception.

.. code-block:: typescript
  :linenos:

  // file1.js
  export x = 1;

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  import { x } from "file1";
  const val = new x(); // RTE, no operation new on JS side
