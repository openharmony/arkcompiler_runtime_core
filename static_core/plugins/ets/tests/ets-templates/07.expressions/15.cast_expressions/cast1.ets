/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    A compile-time error occurs if the casting conversion (see Casting Conversions)
    cannot convert the operand’s compile-time type to the target type
    specified by the cast operator
tags: [compile-only, negative]
---*/

class A {}
class B {}

function main(): void {
  let b: B = new B()

  let a = b as A
  assertTrue( a instanceof A )
}
