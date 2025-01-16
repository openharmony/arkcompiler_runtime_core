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

Runtime conversion rules
++++++++++++++++++++++++

.. _Conversion rules Primitive values:

Runtime conversion rules: Primitive values
******************************************

Primitive type values are copied between ArkTS runtime and JS runtime by value.
If the value is changed in one runtime it doesn't affect another one.

ArkTS to JS conversion
=======================

+-------------------------+-------------------+
| ArkTS runtime type      | JS runtime type   |
+=========================+===================+
| ``number/Number``       | ``number``        |
+-------------------------+-------------------+
| ``byte/Byte``           | ``number``        |
+-------------------------+-------------------+
| ``short/Short``         | ``number``        |
+-------------------------+-------------------+
| ``int/Int``             | ``number``        |
+-------------------------+-------------------+
| ``long/Long``           | ``number``        |
+-------------------------+-------------------+
| ``float/Float``         | ``number``        |
+-------------------------+-------------------+
| ``double/Double``       | ``number``        |
+-------------------------+-------------------+
| ``char/Char``           | ``string``        |
+-------------------------+-------------------+
| ``boolean/Boolean``     | ``boolean``       |
+-------------------------+-------------------+
| ``string/String``       | ``string``        |
+-------------------------+-------------------+
| ``bigint/BigInt``       | ``bigint``        |
+-------------------------+-------------------+
| ``enum``                | ``number/string`` |
+-------------------------+-------------------+
| ``literal type string`` | ``string``        |
+-------------------------+-------------------+
| ``undefined``           | ``undefined``     |
+-------------------------+-------------------+
| ``null``                | ``null``          |
+-------------------------+-------------------+

-  Numeric ArkTS types map to JS ``number``.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export let etsInt: int = 1;
  export let etsDouble: double = 2.1;

.. code-block:: javascript
  :linenos:

  // file1.js
  import { etsInt, etsDouble } from "file2";

  let jsInt = etsInt; // jsInt is 'number' and equals 1
  let jsDouble = etsDouble; // jsDouble is 'number' and equals 2.1

-  ArkTS ``boolean`` maps to JS ``boolean``.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export let etsBool: boolean = true;

.. code-block:: javascript
  :linenos:

  // file1.js
  import { etsBool } from "file2";

  let jsBool = etsBool; // jsBool is 'boolean' and equals true

-  ArkTS ``string`` maps to JS ``string``.

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  export let etsStr: string = "hello";

.. code-block:: javascript
  :linenos:

  // file1.js
  import { etsStr } from "file2";

  let jsStr = etsStr; // jsStr is 'string' and equals "hello"

-  ArkTS ``bigint`` maps to JS ``bigint``.

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  export let etsBigInt: bigint = 10n;

.. code-block:: javascript
  :linenos:

  // file1.js
  import { etsBigInt } from "file2";

  let jsBigInt = etsBigInt; // jsBigInt is 'bigint' and equals 10

-  ArkTS ``undefined`` maps to JS ``undefined``.

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  export let etsUndef: undefined = undefined;

.. code-block:: javascript
  :linenos:

  // file1.js
  import { etsUndef } from "file2";

  let jsUndef = etsUndef; // jsUndef is 'undefined' and equals undefined

-  ArkTS ``null`` maps to JS ``null``.

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  export let etsNull: null = null;

.. code-block:: javascript
  :linenos:

  // file1.js 
  import { etsNull } from "file2";

  let jsNull = etsNull; // jsNull is 'object' and equals null

-  Boxed types(e.g. Number, Char, etc) map to primitive JS types.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export let x: Number = 1; // x is 'object'

.. code-block:: javascript
  :linenos:

  // file1.js JS
  import { x } from "file2";

  let a = x; // x is 'number' and equals 1

-  ``enum`` conversion depends on the type of enumeration. Numeric ``enum`` converts to ``number``. String ``enum`` converts to ``string``.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  // numeric enum
  enum Direction {
      Up = -1,
      Down = 1
  }

  let up: Direction = Direction.Up;
  let down: Direction = Direction.Down;

  // string enum
  enum Color {
      Green = 'green',
      Red = 'red'
  }

  let green: Color = Color.Green;
  let red: Color = Color.Red;

.. code-block:: javascript
  :linenos:

  // file1.js
  import { up, down, green, red } from "file2";

  let a = up; // a is 'number' and equals -1
  let b = down; // b is 'number' and equals 1

  let c = green; // c is 'string' and equals 'green'
  let d = red; // d is 'string' and equals 'red'

-  ``literal type string`` map to JS ``string``

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  export let etsLiteral: "literal" = "literal";
  etsLiteral = "not literal"; // compilation error

.. code-block:: javascript
  :linenos:

  // file1.js 
  import { etsLiteral } from "file2";

  let val = etsLiteral; // val is "literal" but it can be changed
  val = "not literal"; // ok

JS to ArkTS conversion
=======================

+-----------------+-----------------------+
| JS runtime type | ArkTS runtime type    |
+=================+=======================+
| ``null``        | ``null``              |
+-----------------+-----------------------+
| ``undefined``   | ``undefined``         |
+-----------------+-----------------------+
| ``boolean``     | ``boolean``           |
+-----------------+-----------------------+
| ``number``      | ``number``            |
+-----------------+-----------------------+
| ``bigint``      | ``bigint``            |
+-----------------+-----------------------+
| ``string``      | ``string``            |
+-----------------+-----------------------+
| ``symbol``      | ``ESObject``          |
+-----------------+-----------------------+

Value imported from JS to ArkTS, should be converted explicitly using ``as`` keyword.

- JS ``null`` maps to ArkTS ``null``.

.. code-block:: javascript
  :linenos:

  // file1.js
  export let a = null;

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  import { a } from "file1";

  const valNull = a as null; // valNull is 'null' and equals null

- JS ``undefined`` maps to ArkTS ``undefined``.

.. code-block:: javascript
  :linenos:

  // file1.js
  export let a = undefined;

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  import { a } from "file1";

  const valUnDef = a as undefined; // valUnDef is 'undefined' and equals undefined

- JS ``boolean`` maps to ArkTS ``boolean``.

.. code-block:: javascript
  :linenos:

  // file1.js
  export let a = true;

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  import { a } from "file1";

  const valBool = a as boolean; // valBool is 'boolean' and equals true

- JS ``number`` maps to ArkTS ``number``.

.. code-block:: javascript
  :linenos:

  // file1.js
  export let a = 1;

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  import { a } from "file1";

  const valNum = a as number; // valNum is 'number' and equals 1

- JS ``bigint`` maps to ArkTS ``bigint``.

.. code-block:: javascript
  :linenos:

  // file1.js
  export let a = 10n;

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  import { a } from "file1";

  const valBigInt = a as bigint; // valBigInt is 'bigint' and equals 10

- JS ``string`` maps to ArkTS ``string``.

.. code-block:: javascript
  :linenos:

  // file1.js
  export let a = "abc";

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  import { a } from "file1";

  const valStr = a as string; // valStr is 'string' and equals "abc"

- There is no such type as ``Symbol`` in ArkTS so it's proxing to ESObject.

.. code-block:: javascript
  :linenos:

  // file1.js
  export let jsSymbol = Symbol("id");

.. code-block:: typescript
  :linenos:

  // file2.ets  ArkTS
  import { jsSymbol } from "file1";
  let val = jsSymbol; // ok, val is ESObject

Limitations
===========

Object wrapper types
--------------------

- Object wrapper types for primitive values such as ``Null``, ``Undefined``, ``Boolean``, ``Number``, ``Bigint``, ``String``, and ``Symbol``
  can't be copied by default to ArkTS values. It need special way for creating on JS side via ``new`` keyword. Without it even in
  case of capital letter it will be primitive type according JS.

.. code-block:: typescript
  :linenos:

  // file1.js
  let a = new Number(123); // typeof a is "object"
  let b = Number(123); // typeof a is "number"

  // file2.ets ArkTS
  import { a, b } from "file1";
  let aa = a as number; // RTE, as a is a reference from JS runtime
  let bb = b as number; // ok, as b is a primitive number from JS runtime

Solutions
^^^^^^^^^

- Use ``valueOf`` to get primitive values from wrapper objects and copy them to ArkTS

.. code-block:: typescript
  :linenos:

  // file1.js
  let a = new Number(123); // typeof a == "object"
  let b = Number(123); // typeof a == "number"

  // file2.ets ArkTS
  import { a, b } from "file1";
  let aa = a.valueOf() as number; // ok
  let bb = b as number; // ok

Copy semantic
=============

-  Primitive type value is copied from JS runtime to ArkTS runtime by value so there is no connection with JS runtime after compilation and no side effects.
   E.g. if Prototype is changed in JS runtime it won't be changed in ArkTS runtime.

.. code-block:: typescript
  :linenos:

  // file1.js
  Number.Prototype.toString = () => {
      return "hello";
  }
  export let a = Number(123);

  // file2.ets ArkTS
  import { a } from "file1";
  a.toString(); // "123", ArkTS semantics, not JS

- JS object of primitive types with capital letter(Number, Boolean, String) also will be copied and just ignore all additional fields

.. code-block:: javascript
  :linenos:

  // file1.js
  let a = new Number(3);
  a.newfield = "hello" // will be ignored in ArkTS

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  import { a } from "file1";
  let num = a as number; // num is just static number with val 3

Solutions
=========

- Instead of importing primitive types, global contex can be imported instead of them and manipulation can be done through global context

.. code-block:: javascript
  :linenos:

  // file1.js
  let a = new Number(3);
  a.newfield = "hello" // will be ignored in ArkTS

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  import * as global from "file1";
  global.a = 42; // Will change original value on JS side too

- Also original source can be changed and value can be moved into a class

.. code-block:: javascript
  :linenos:

  // file1.js
  class A {
    val = 3;
  }
  export let a = new A();

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  import { a } from "file1";
  a.val = 42; // Will change original value on JS side too

Wide limitation
---------------

-  ``long`` of value lower :math:`-2^{53}` and higher :math:`2^{53}-1`  when converted to JS number will have precision loss. Use ``bigint`` for such numbers.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export let a: long = Math.pow(2, 53) + 10;

  // file1.js
  import { a } from "file2"; // this import will result in precision loss

-  Integer ``number`` values when converted to ArkTS may have precision loss if a value out of range of ArkTS type

   - ``byte`` range is :math:`-2^7` to :math:`2^7-1`
   - ``short`` range is :math:`-2^{15}` to :math:`2^{15}-1`
   - ``int`` range is :math:`-2^{31}` to :math:`2^{31}-1`
   - ``long`` range is :math:`-2^{53}` to :math:`2^{53}-1`

.. code-block:: typescript
  :linenos:

  // file1.js
  export x = Math.pow(2, 15) + 10;

  // file2.ets ArkTS
  import { x } from "file1";

  const valShort = x as short; // convertion will lead to truncation
  const valInt = x as int;  // safe, no truncation

-  Floating-point ``number`` values when converted to ArkTS ``float`` may have precision loss since it is 32-bit number and JS ``number`` is 64-bit number.

   - ``float`` is the set of all IEEE 754 32-bit floating-point numbers
   - ``double`` is the set of all IEEE 754 32-bit floating-point numbers

Solutions
^^^^^^^^^

- Use more wide types on ArkTS side. For exampe use ``bigint`` instead of ``long``

.. code-block:: javascript
  :linenos:

  // file1.ets ArkTS
  export let a: bigint = 12314; // any big val

.. code-block:: typescript
  :linenos:

  // file2.js
  import { a } from "file1";
  let num = a; // ok, bigint no precision loss

- Using ``number`` instead of ``float``

.. code-block:: javascript
  :linenos:

  // file1.js
  let a = 456.52; // any big double value which is wider than 32 bit

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  import { a } from "file1";
  let x = a as number; // ok, will be correct
  let y = b as float; // may lose precision, use ``number`` type instead of float

.. _Conversion rules Reference values:

Runtime conversion rules: Reference values
******************************************

Reference values after conversion are connected to original objects and changing them in one runtime will change them in another runtime.

ArkTS to JS conversion
=======================

- For ArkTS classes interop builds proxy-classes and proxy-objects via JS native APIs.
- JS proxy-class object lazily constructed for any class from ArkTS if necessary at the moment when JS will try to get acces to it.
- ArkTS objects are wrapped in lightweight JS proxy-instances. Objects appear as sealed in JS.

+-------------------------+
| ArkTS reference types   |
+=========================+
| ``object``              |
+-------------------------+
| ``class``               |
+-------------------------+
| ``interface``           |
+-------------------------+
| ``function``            |
+-------------------------+
| ``tuple``               |
+-------------------------+
| ``union``               |
+-------------------------+
| ``Std library objects`` |
+-------------------------+

- Proxing ArkTS object.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  class A {
    val : string = "hi";
  };

  export let a = new A();

.. code-block:: javascript
  :linenos:

  // file1.js
  import { a } from 'file2'
  a.val = "222"; // ok

  Reflect.set(a, "newVal", "hello"); // runtime exception, objects are sealed
  Reflect.set(a, "val", 123);  // runtime exception, field has another type

- Inheritance also will be constructed for proxy classes

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  class B {
    valb = "b";
  };

  class A extends B {
    vala = "a";
  };

  let a = new A();

.. code-block:: javascript
  :linenos:

  // file1.js
  import { a } from 'file2'
  // Classes A and B will be constructed on JS side with inheritance relationships.
  a.vala = "222"; // ok
  a.valb = "333"; // ok

- About proxing ArkTS ``union`` see :ref:`Features ArkTS Union`
- About proxing ArkTS ``tuple`` see :ref:`Features ArkTS Tuple`
- About proxing ArkTS ``class`` see :ref:`Features ArkTS Classes`
- About proxing ArkTS ``interface`` see :ref:`Features ArkTS Interfaces`
- About proxing ArkTS ``function`` see :ref:`Features ArkTS Functions`
- About proxing ``Std library objects`` see :ref:`ArkTS Std library` and :ref:`Async and concurrency features ArkTS`.

Limitations
-----------

- Layout of ArkTS objects can not be changed and it is root of limitations for proxy-objects

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  class A {
    val : string = "hi";
  };

  export let a = new A();

.. code-block:: javascript
  :linenos:

  // file1.js

  import {a} from 'file2'
  a.newVal = 1; // runtime exception, objects are sealed
  a.val = 123; // runtime exception, field has another type
  a.val = "123";  // ok

Solutions
^^^^^^^^^

- All changes for static classes should be done by user on static side

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  class A {
    val : number|string = 2;
    newVal : number = 3;
  };

  export let a = new A();

.. code-block:: javascript
  :linenos:

  // file1.js

  import {a} from 'file2'
  a.newVal = 1; // ok
  a.val = 123; // ok
  a.val = "123"; // ok

2. JS to ArkTS conversion
============================

In JS everything that is not a primitive value is an object. We will call it a reference value and it follows the reference conversion rules described in this chapter.
We can group all reference values into the following categories.

+----------------------------------+
| JS reference types               |
+==================================+
| ``Object``                       |
+----------------------------------+
| ``Class``                        |
+----------------------------------+
| ``Function``                     |
+----------------------------------+
| ``Collection``                   |
|                                  |
| ``(Array, Set, Map, etc)``       |
+----------------------------------+
| ``Other standard builtins``      |
|                                  |
| ``(Date, RegExp, Promise, etc)`` |
+----------------------------------+

ESObject is used to proxy any reference values from JS.

- Proxing JS object with ESObject.

.. code-block:: javascript
    :linenos:

    // file1.js
    export class A {
      v = 123;
    }

    export let a = new A(); // ``a`` is JS object

.. code-block:: typescript
    :linenos:

    // file2.ets ArkTS
    import { a } from 'file1'

    let b = a; // ok, ``b`` is ESObject
    let c = a.v; // ok, ``c`` is ESObject
    let d = a.v as number; // ok, ``d`` is number

- Special operators: ``new``, ``.``, ``[]``, ``()`` will work properly with ESObject, if such operations available on JS side, otherwise it will generate runtime exception

.. code-block:: javascript
    :linenos:

    // file1.js
    export class A {
      v = 123;
    }

    export let a = new A()

.. code-block:: typescript
    :linenos:

    // file2.ets ArkTS
    import { a } from 'file1'

    let number1: number = a.v as number  // ok
    a.v = 456; // ok, will modify original JS object
    a.newfield = "hi"; // ok, will modify original JS object and create new field
    let missedFiled = a.missedFiled as undefined; // ok
    let number2 = a["v"] as number; // ok, will return 456
    let number2 = a[1] as undefined; // ok

- Prototype of JS object can be modified on ArkTS side and it will be applied to all instances of Class

.. code-block:: javascript
    :linenos:

    // file1.js
    export class A {
      v = 123;
      testFunction() {
        return true;
      }
    }

.. code-block:: typescript
    :linenos:

    // file2.ets ArkTS
    import { A } from './file1'

    let a = new A(); // ESObject will be returned but it will be unused
    let a1 = a.testFunction() as boolean; // ``a1`` is true

    A.prototype.testFunction = function() {
      return false;
    }

    let a2 = a.testFunction() as boolean; // ``a2`` is false

- About proxing JS ``function`` see :ref:`Features JS. Functions`
- About proxing JS ``class`` see :ref:`Features JS. Classes`
- About proxing collections and other standard builtins see :ref:`JS Std library` and :ref:`Async and concurrency features JS`.

Limitations
===========

Unsupported operations
----------------------

- All unsupported special operations will throw runtime exception. Or incorrect conversions.

.. code-block:: javascript
    :linenos:

    // file1.js
    class A {
      v = 123
    }

    export let a = new A()


.. code-block:: typescript
    :linenos:

    // file2.ets ArkTS
    import { a } from 'file1'

    a(); // RTE
    a as number; // RTE
    a.v as string; // RTE
    a.newVal as string; // RTE
    a[1] as int; // RTE
    a["v"] as string; // RTE


Solutions
^^^^^^^^^

- If you need non standard conversion, you should use conversion for static types

.. code-block:: javascript
    :linenos:

    // file1.js

    class A {
      v = 123
    }

    export let a = new A()


.. code-block:: typescript
    :linenos:

    // file2.ets ArkTS
    import { a } from 'file1'

    let num = a.v as number; // ok
    let str = num.toString(); // ok, now we get static string from number

Cast to static object
---------------------

- ESObject which contains reference type values from JS runtime can't be cast to ArkTS object.

.. code-block:: javascript
    :linenos:

    // file1.js
    export class A {
      v = 123;
    }

    export let a = new A()

.. code-block:: typescript
    :linenos:

    // file2.ets ArkTS
    import { a } from 'file1'

    let b = a as Object; // RTE. a is ESObject with reference values from JS runtime.

Solutions
^^^^^^^^^

- Only primitive types can be cast to ArkTS objects.

.. code-block:: javascript
    :linenos:

    // file1.js
    export class A {
      v = 123;
    }

    export let a = new A()

.. code-block:: typescript
    :linenos:

    // file2.ets ArkTS
    import { a } from './file1'

    let b: number = a.v as number; // ok
    let c: Object = a.v as Object; // ok
