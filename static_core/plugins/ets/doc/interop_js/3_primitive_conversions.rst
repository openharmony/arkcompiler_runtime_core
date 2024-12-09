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

Conversion rules: Primitive values
##################################

Primitive type values are copied between ArkTS VM and JS VM by value.
If the value is changed in one VM it doesn't affect another one.

1. ArkTS to JS conversion
*************************

+-------------------------+-------------------+
| ArkTS type              | JS type           |
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

-  Numeric ArkTS 2.0 types map to JS ``number``.

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let stsInt: int = 1;
  export let stsDouble: double = 2.1;

.. code-block:: javascript
  :linenos:

  // 2.js
  import { stsInt, stsDouble } from "converted_sts_source";

  let jsInt = stsInt; // jsInt is 'number' and equal 1
  let jsDouble = stsDouble; // jsDouble is 'number' and equal 2.1

-  ArkTS 2.0 ``boolean`` maps to JS ``boolean``.

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let stsBool: boolean = true;

.. code-block:: javascript
  :linenos:

  // 2.js
  import { stsBool } from "converted_sts_source";

  let jsBool = stsBool; // jsBool is 'boolean' and equal true

-  ArkTS 2.0 ``string`` maps to JS ``string``.

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let stsStr: string = "hello";

.. code-block:: javascript
  :linenos:

  // 2.js
  import { stsStr } from "converted_sts_source";

  let jsStr = stsStr; // jsStr is 'string' and equal "hello"

-  ArkTS 2.0 ``bigint`` maps to JS ``bigint``.

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let stsBigInt: bigint = 10n;

.. code-block:: javascript
  :linenos:

  // 2.js
  import { stsBigInt } from "converted_sts_source";

  let jsBigInt = stsBigInt; // jsBigInt is 'bigint' and equal 10

-  ArkTS 2.0 ``undefined`` maps to JS ``undefined``.

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let stsUndef: undefined = undefined;

.. code-block:: javascript
  :linenos:

  // 2.js
  import { stsUndef } from "converted_sts_source";

  let jsUndef = stsUndef; // jsUndef is 'undefined' and equal undefined

-  ArkTS 2.0 ``null`` maps to JS ``null``.

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let stsNull: null = null;

.. code-block:: javascript
  :linenos:

  // 2.js
  import { stsNull } from "converted_sts_source";

  let jsNull = stsNull; // jsNull is 'object' and equal null

-  Boxed types(e.g. Number, Char, etc) map to primitive JS types.

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let x: Number = 1; // x is 'object'

.. code-block:: javascript
  :linenos:

  // 2.js
  import { x } from "converted_sts_source";

  let a = x; // x is 'number'

-  ``enum`` conversion depends on the type of enumeration. Numeric ``enum`` converts to ``number``. String ``enum`` converts to ``string``.

.. code-block:: typescript
  :linenos:

  // 1.sts
  // numeric enum
  enum Direction {
      Up = -1,
      Down = 1
  }

  // string enum
  enum Color {
      Green = 'green',
      Red = 'red'
  }

.. code-block:: javascript
  :linenos:

  // 2.js
  import { Direction, Color } from "converted_sts_source";

  let a = Direction.Up; // a is 'number' and equal -1
  let b = Direction.Down; // b is 'number' and equal 1

  let c = Color.Green; // c is 'string' and equal 'green'
  let d = Color.Red; // d is 'string' and equal 'red'

-  ``literal type string`` map to JS ``string``

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let stsLiteral: "literal" = "literal";
  stsLiteral = "not literal"; // compilation error

.. code-block:: javascript
  :linenos:

  // 2.js
  import { stsLiteral } from "converted_sts_source";

  let val = stsLiteral; // val is "literal" but it can be changed
  val = "not literal"; // ok

2. JS to ArkTS conversion
*************************

+---------------+---------------+
| JS type       | ArkTS type    |
+===============+===============+
| ``null``      | ``null``      |
+---------------+---------------+
| ``undefined`` | ``undefined`` |
+---------------+---------------+
| ``boolean``   | ``boolean``   |
+---------------+---------------+
| ``number``    | ``number``    |
+---------------+---------------+
| ``bigint``    | ``bigint``    |
+---------------+---------------+
| ``string``    | ``string``    |
+---------------+---------------+
| ``symbol``    | ``ESObject``  |
+---------------+---------------+

Value imported from JS to ArkTS 2.0, should be converted explicitly using ``as`` keyword.

- JS ``null`` maps to ArkTS 2.0 ``null``.

.. code-block:: javascript
  :linenos:

  // 1.js
  export let a = null;

.. code-block:: typescript
  :linenos:

  // 2.sts
  import { a } from "1.js";

  const valNull = a as null; // valNull is 'null' and equal null

- JS ``undefined`` maps to ArkTS 2.0 ``undefined``.

.. code-block:: javascript
  :linenos:

  // 1.js
  export let a = undefined;

.. code-block:: typescript
  :linenos:

  // 2.sts
  import { a } from "1.js";

  const valUnDef = a as undefined; // valUnDef is 'undefined' and equal undefined

- JS ``boolean`` maps to ArkTS 2.0 ``boolean``.

.. code-block:: javascript
  :linenos:

  // 1.js
  export let a = true;

.. code-block:: typescript
  :linenos:

  // 2.sts
  import { a } from "1.js";

  const valBool = a as boolean; // valBool is 'boolean' and equal true

- JS ``number`` maps to ArkTS 2.0 ``number``.

.. code-block:: javascript
  :linenos:

  // 1.js
  export let a = 1;

.. code-block:: typescript
  :linenos:

  // 2.sts
  import { a } from "1.js";

  const valNum = a as number; // valNum is 'number' and equal 1

- JS ``bigint`` maps to ArkTS 2.0 ``bigint``.

.. code-block:: javascript
  :linenos:

  // 1.js
  export let a = 10n;

.. code-block:: typescript
  :linenos:

  // 2.sts
  import { a } from "1.js";

  const valBigInt = a as bigint; // valBigInt is 'bigint' and equal 10

- JS ``string`` maps to ArkTS 2.0 ``string``.

.. code-block:: javascript
  :linenos:

  // 1.js
  export let a = "abc";

.. code-block:: typescript
  :linenos:

  // 2.sts
  import { a } from "1.js";

  const valStr = a as string; // valStr is 'string' and equal "abc"

- There is no such type as ``Symbol`` in ArkTS 2.0 so it's proxing to ESObject.

.. code-block:: javascript
  :linenos:

  // 1.js
  export let jsSymbol = Symbol("id");

.. code-block:: typescript
  :linenos:

  // 2.sts
  import { jsSymbol } from "1.js";
  let val = jsSymbol; // ok, val is ESObject

Limitations
***********

Object wrapper types
====================

- Object wrapper types for primitive values such as ``Null``, ``Undefined``, ``Bolean``, ``Number``, ``Bigint``, ``String``, and ``Symbol``
  can't be copied by default to ArkTS 2.0 values

.. code-block:: typescript
  :linenos:

  // 1.js
  let a = Number(123);

  // 2.sts
  import { a } from "1.js";
  let b = a as int; // RTE

Solutions
---------

- Use ``valueOf`` to get primitive values from wrapper objects and copy them to ArkTS 2.0

.. code-block:: typescript
  :linenos:

  // 1.js
  let a = Number(123);

  // 2.sts
  import { a } from "1.js";
  let b = a.valueOf() as int; // ok

Copy semantic
=============

-  Primitive type value is copied from JS VM to ArkTS VM by value so there is no connection with JS VM after compilation and no side effects.
   E.g. if Prototype is changed in JS VM it won't be changed in ArkTS VM.

.. code-block:: typescript
  :linenos:

  // 1.js
  Number.Prototype.toString = () => {
      return "hello";
  }
  export let a = Number(123);

  // 2.sts
  import { a } from "1.js";
  a.toString(); // "123", STS semantics, not JS

- JS object of primitive types with capital letter(Number, Boolean, String) also will be copied and just ignore all additional fields

.. code-block:: javascript
  :linenos:

  // 1.js
  let a = new Number(3);
  a.newfield = "hello" // will be ignored in ArkTS 2.0

.. code-block:: typescript
  :linenos:

  // 2.sts
  import { a } from "1.js";
  let num = a as number; // num is just static number with val 3

Solutions
---------

- Instead of importing primitive types, global contex can be imported instead of them and manipulation can be done through global context

.. code-block:: javascript
  :linenos:

  // 1.js
  let a = new Number(3);
  a.newfield = "hello" // will be ignored in ArkTS 2.0

.. code-block:: typescript
  :linenos:

  // 2.sts
  import * as global from "1.js";
  global.a = 42; // Will change original value on JS side too

- Also original source can be changed and value can be moved to class

.. code-block:: javascript
  :linenos:

  // 1.js
  class A {
    val;
  }
  let a = new A();
  a.val = 3;

.. code-block:: typescript
  :linenos:

  // 2.sts
  import { a } from "1.js";
  a.val = 42; // Will change original value on JS side too

Wide limitation
===============

-  ``long`` of value lower :math:`-2^{53}` and higher :math:`2^{53}-1`  when converted to JS number will have precision loss. Use ``bigint`` for such numbers.

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let a: long = Math.pow(2, 53) + 10;

  // 2.js
  import { a } from "converted_sts_source"; // this import will result in precision loss

-  Integer ``number`` values when converted to ArkTS may have precision loss if a value out of range of ArkTS type

   - ``byte`` range is :math:`-2^7` to :math:`2^7-1`
   - ``short`` range is :math:`-2^{15}` to :math:`2^{15}-1`
   - ``int`` range is :math:`-2^{31}` to :math:`2^{31}-1`
   - ``long`` range is :math:`-2^{63}` to :math:`2^{63}-1`

.. code-block:: typescript
  :linenos:

  // 1.js
  export x = Math.pow(2, 15) + 10;

  // 2.sts
  import { x } from "1.js";

  const valShort = x as short; // convertion will lead to truncation
  const valInt = x as int;  // safe, no truncation

-  Floating-point ``number`` values when converted to ArkTS ``float`` may have precision loss since it is 32-bit number and JS ``number`` is 64-bit number.

   - ``float`` is the set of all IEEE 754 32-bit floating-point numbers
   - ``double`` is the set of all IEEE 754 32-bit floating-point numbers

Solutions
---------
- Use more wide types on ArkTS 2.0 side. For exampe use ``bigint`` instead of ``long``

.. code-block:: javascript
  :linenos:

  // 1.sts
  export let a: bigint = 12314; // any big val

.. code-block:: typescript
  :linenos:

  // 2.js
  import { a } from "converted_sts_source";
  let num = a; // ok, bigint no precision loss

- Using ``number`` instead of ``float``

.. code-block:: javascript
  :linenos:

  // 1.js
  let a = 456.52; // any big double value which  wider than 32 bit

.. code-block:: typescript
  :linenos:

  // 2.sts
  import a from "1.js";
  let num = 42 as number; // ok, will be correct
  let num = 42 as float; // not ok, can be precision loss, use ``number`` type instead of it

ESObject operators usage
========================

-  When using ``as`` keyword if ArkTS type doesn't map to JS type then an exception will be thrown.

.. code-block:: typescript
  :linenos:

  // 1.js
  export x = 1;

  // 2.sts
  import { x } from "1.js";
  const valStr = x as string; // runtime exception

-  When primitive type values are copied to ArkTS they become ESObject that contains primitive value.
   In that case operators ``[]``, ``()``, ``.``, and ``new`` have no meaning and their usage will throw an exception.

.. code-block:: typescript
  :linenos:

  // 1.js
  export x = 1;

  // 2.sts
  import { x } from "1.js";
  const val = new x(); // runtime exception
