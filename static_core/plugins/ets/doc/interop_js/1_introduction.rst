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

++++++++++++
Introduction
++++++++++++

This document describe specification of interop between ArkTS runtime and
Typescript/Javascript, including additional compile time type checking for interop between
ArkTS and Typescript/Javascript.

Managed interop is part of runtime and frontend of ArkTS. It communicate
with javascript runtime via special native bridge. Here are excuted all
nessesary conversions for passed arguments. So all communication are only in
runtime. Additionally here can be typecheck for some cases at compile time. It
is just additional stage for better user experience, but it is does not affect
runtime part.

Interop via 'import' and import rules
-------------------------------------
Our interop can be made via `import`, like what we have done in TS. However, we do have some rules for import directions.

- ArkTS files can import TS/JS files,

  with additional restrictions:
    1. ArkTS class cannot `extends` TS/JS class

    2. ArkTS class cannot `implements` TS interfaces

- TS/JS files cannot import ArkTS files

Basic principles
----------------

- In the ArkTS environment, when executing the ArkTS code logic and encountering an object that is incompatible with the annotated type, a runtime exception will occur.

- ArkTS objects layout cannot be dynamically modified.

- Type conversion (explicit or implicit) in ArkTS will cause runtime type verification, and will generate runtime exceptions if conversion is not possible in proper way.

Principles for compilation type check
-------------------------------------

- Support use TS types in ArkTS files as type annotations.

- Compile-time type checking should be checked according to the rules of the file type: ArkTS files should follow the type checking rules of ArkTS, TS files should follow the type checking rules of TS. The interop part will be checked based on :ref:`Principles for compilation type mapping rules`, after type mapping to the type of the file where it is located.

- When the ArkTS files are compiled into bytecode, all type annotations from TS in the ArkTS files will degenerate into ESObject.

- When the objects imported from TS/JS in the ArkTS file are finally compiled into bytecode, the types of these objects will be treated as ESObject.

.. _Principles for compilation type mapping rules:

Principles for compilation type mapping rules
---------------------------------------------

- Primitive types are mapped into primitive types(e.g number -> number).(see :ref:`Typecheck conversion rules. Primitive values`)

- Composed types (classes/interfaces/structs) are mapped into composed types. Moreover, nested types will be mapped into nested types.(e.g interface Y {x: X, s: string} -> interface Y {x: X, s: string}) (see :ref:`Typecheck conversion rules. Reference values`)

Principles for runtime objects mapping rules
--------------------------------------------

- Primitive types will be mapped into primitive types (except special cases, see :ref:`Conversion rules Primitive values`)

- ArkTS reference types will be mapped into some proxy objects in JS runtime

- JS runtime objects will be mapped into `ESObject` in ArkTS runtime


Terminology
-----------

Throughout the docs we may often refer "CTE" by "Compile Time Error" and "RTE" by "Runtime Time Error".
