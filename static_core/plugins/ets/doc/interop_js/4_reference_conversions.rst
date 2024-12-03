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

Conversion rules: Reference values
##################################

1. ArkTS to JS conversion
*************************

- For 2.0 classes interop builds proxy-classes and proxy-objects via js APIs
- JS proxy-class object lazily constructed for any class from 2.0 if necessary at the moment when JS will try to get acces to it.
- ArkTS 2.0 are wrapped in lightweight JS proxy-instances. Objects appear as sealed in JS.

.. code-block:: typescript
  :linenos:

  //1.sts
  class A {
    val : string = "hi";
  };

  export let a = new A();

.. code-block:: javascript
  :linenos:

  //2.js

  import {a} from 'converted_sts_source' // intermediate src with decl
  a.val = "222"; // ok

  a.newVal = 1; // runtime exception, objects are sealed
  a.val = 123; // runtime exception, field has another type

- Inheritance also will be constructed for proxy classes

.. code-block:: typescript
  :linenos:

  //1.sts
  class B {
    valb = "b";
  };

  class A extends B {
    vala = "a";
  };

  let a = new A();


.. code-block:: javascript
  :linenos:

  //2.js

  import {a} from 'converted_sts_source'
  // Classes A and B will be constructed on JS side with inheritance relationships.
  a.vala = "222"; // ok
  a.valb = "333"; // ok

Limitations
===========

- Layout of static objects can not be changed and it is root of limitations for proxy-objects

.. code-block:: typescript
  :linenos:

  //1.sts
  class A {
    val : string = "hi";
  };

  export let a = new A();

.. code-block:: javascript
  :linenos:

  //2.js

  import {a} from 'converted_sts_source' // intermediate src with decl
  a.newVal = 1; // runtime exception, objects are sealed
  a.val = 123; // runtime exception, field has another type

Solutions
---------

- All changes for static classes should be done by user on static side

.. code-block:: typescript
  :linenos:

  //1.sts
  class A {
    val : number = 2;
    newVal : number = 3;
  };

  export let a = new A();

.. code-block:: javascript
  :linenos:

  //2.js

  import {a} from 'converted_sts_source' // intermediate src with decl
  a.newVal = 1; // ok
  a.val = 123; // ok

2. JS to ArkTS conversion
*************************

- ESObject may be used to proxy some reference type value from JS
- Specail operators: ``new``, ``.``, ``[]``, ``()`` will work properly with JS object, if such operations availeble on JS side, otherwise it will generate runtime exception

.. code-block:: javascript
    :linenos:

    // 1.js

    class A {
      v = 123
    }

    let a = new A()


.. code-block:: typescript
    :linenos:

    // 2.sts
    import { a } from './1.js'

    let number = a.v as number  // ok
    a.v = 456; // ok, will modify original JS object
    a.newfield = "hi"; // ok, will modify original JS object and create new field
    let missedFiled = a.missedFiled as undefined; // ok
    let number2 = a["v"] as number; // ok, will return 456
    let number2 = a[1] as undefined; // ok


Limitations
===========

- All unsupported special operation will throw runtime exception. Or incorrect conversions.

.. code-block:: javascript
    :linenos:

    // 1.js

    class A {
      v = 123
    }

    let a = new A()


.. code-block:: typescript
    :linenos:

    // 2.sts
    import { a } from './1.js'

    a(); // RTE
    a as number; // RTE
    a.v as string; // RTE
    a.newVal as string; // RTE
    a[1] as int; // RTE
    a["v"] as string; // RTE


Solutions
---------

- If you need non standart conversion, you should use conversion for static types

.. code-block:: javascript
    :linenos:

    // 1.js

    class A {
      v = 123
    }

    let a = new A()


.. code-block:: typescript
    :linenos:

    // 2.sts
    import { a } from './1.js'

    let num = a.v as number; // ok
    let str = num.toString(); // ok, now we get static string from number
