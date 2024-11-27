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

-  Numeric ArkTS types map to JS ``number``.

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let stsInt: int = 1;
  export let stsDouble: double = 2.1;

  // 2.js
  import { stsInt, stsDouble } from "converted_sts_source";

  let jsInt = stsInt; // jsInt is 1
  typeof jsInt; // 'number'

  let jsDouble = stsDouble; // jsDouble is 2.1
  typeof jsDouble; // 'double'

-  ``boolean``, ``string``, ``bigint``, ``undefined``, and ``null`` map to JS ``boolean``, ``string``, ``bigint``, ``undefined``, and ``null`` accordingly.

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let stsNum: int = 1;
  export let stsBool: boolean = true;
  export let stsStr: string = "hello";
  export let stsBigInt: bigint = 10n;
  export let stsUndef: undefined = undefined;
  export let stsNull:null = null;

  // 2.js
  import { stsBool, stsStr, stsBigInt, stsUndef, stsNull } from "converted_sts_source";

  let jsInt = stsNum; // jsInt is 1
  typeof jsInt; // 'number'

  let jsBool = stsBool; // jsBool is true
  typeof jsBool; // 'boolean'

  let jsStr = stsStr; // jsStr is "hello"
  typeof jsStr; // 'string'

  let jsBigInt = stsBigInt; // jsBigInt is 10
  typeof jsBigInt; // 'bigint'

  let jsUndef = stsUndef; // jsUndef is undefined
  typeof jsUndef; // 'undefined'

  let jsNull = stsNull; // jsNull is null
  typeof jsNull; // 'null'

-  Boxed types(e.g. Number, Char, etc) map to primitive JS types.

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let x: Number = 1;
  typeof x; // 'object'

  // 2.js
  import { x } from "converted_sts_source";

  typeof x; // 'number'

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

  // 2.js
  import { Direction, Color } from "converted_sts_source";

  let val = typeof Direction.Up; // val is -1
  typeof val; // 'number'
  val = typeof Direction.Down; // val is 1
  typeof val; // 'number'

  val = typeof Color.Green; // val is 'green'
  typeof val; // 'string'
  val = typeof Color.Red; // val is 'red'
  typeof val; // 'string'

-  ``literal type string`` map to JS ``string``

.. code-block:: typescript
  :linenos:

  // 1.sts
  export let stsLiteral: "literal" = "literal";
  stsLiteral = "not literal"; // compilation error

  // 2.js
  import { stsLiteral } from "converted_sts_source";

  let val = stsLiteral; // val is "literal" but it can be changed
  val = "not literal"; // ok

2. JS to ArkTS conversion
*************************

+-------------------------+---------------+
| JS type                 | ArkTS type    |
+=========================+===============+
| ``null/Null``           | ``null``      |
+-------------------------+---------------+
| ``undefined/Undefined`` | ``undefined`` |
+-------------------------+---------------+
| ``boolean/Boolean``     | ``boolean``   |
+-------------------------+---------------+
| ``number/Number``       | ``number``    |
+-------------------------+---------------+
| ``bigint/Bigint``       | ``bigint``    |
+-------------------------+---------------+
| ``string/String``       | ``string``    |
+-------------------------+---------------+
| ``symbol/Symbol``       | ``ESObject``  |
+-------------------------+---------------+

-  Value imported from JS to ArkTS, should be converted explicitly using ``as`` keyword.

.. code-block:: typescript
  :linenos:

  // 1.js
  export a = 1;
  export b = "abc";
  export c = true;
  export d = undefined;
  export e = null;
  export f = 10n;

  // 2.sts
  import { a, b, c, d, e, f } from "1.js";

  const valNum = a as number; // valNum is 1
  typeof valNum; // 'number'

  const valStr = b as string; // valStr is "abc"
  typeof valStr; // 'string'

  const valBool = c as boolean; // valBool is true
  typeof valBool; // 'boolean'

  const valUnDef = d as undefined; // valUnDef is undefined
  typeof valUnDef; // 'undefined'

  const valNull = e as null; // valNull is null
  typeof valNull; // 'null'

  const valBigInt = f as bigint; // valBigInt is 10
  typeof valBigInt; // 'bigint'

-  JS object wrapper classes become primitive types in ArkTS.

.. code-block:: javascript
  :linenos:

  // 1.js
  export let a = new Number(1);

.. code-block:: typescript
  :linenos:

  // 2.sts
  import { a } from "1.js";
  let val = a as number; // ok

-  TODO: symbol -> ESObject

Limitations
***********

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
