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

ESObject
########

- ESObject type is a special type in STS. It is separate from Object type.
- ESObject type can be used in following ways without type check: ``operator new``, ``operator []``, ``operator ()``, ``operator .``, ``operator as``. This operation can be applyed to dynamic entities by this operators.
- All entities imported from ETS/TS/JS will be of type ESObject.

..
   Clarify:: They can't be used in any contexts where types are allowed, such as type annotations, extend clause, implements clause, etc.

- Values of ESObject type can be converted to and from Object type. However, implicitly converting an ESObject type to an Object type is forbidden.
- Values of ESObject may wrap dynamic entities(from ETS/TS/JS) or pure static types of ArkTS 2.0

Examples
********

.. code-block:: javascript
    :linenos:

    // test.js
    class A {
    v = 123
    }
    let a = new A()

    function foo(v) {}

.. code-block:: typescript
    :linenos:

    // app.sts
    import { a, foo } from './test.js'  // a is of ESObject type
    a.v  // ok
    a[index] // ok, no CTE, but RTE, no operation [] on JS side
    a() // ok, no CTE, but RTE, no operation () on JS side
    new a() // ok

    let n1: number = a.v  // CTE (compiler-time error)
    let n2: number = a.v as number // ok
    let n3: ESObject = a.v  // ok
    a.v + 1  // compiler-time error
    (a.v as number) + 1  // ok

    let s1: ESObject = 1;  // ok, implicit conversion
    let s2: ESObject = new StaticClass() // ok, implicit conversion
    let s3: ESObject = 1 as ESObject; // ok, explicit conversion
    let s4: ESObject = new StaticClass() as ESObject; // ok, explicit conversion

    foo(1); // ok, implicitly convert 1 to the ESObject type
    foo(new StaticClass()); // ok, implicitly convert static object to the ESObject type
