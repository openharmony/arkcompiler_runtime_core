/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Static setter of a superclass is not accessible via subclass name.
assert: >-
    Static members of a superclass are not inherited. The subclass which wants
    to use static members of its superclass must use explicit qualification
    using the superclass name.
tags: [negative, compile-only]
expected_out: "Semantic error ESE0087: Property 'prop' does not exist on type 'B'"
---*/

class A {
  private static _val: int = 0;

  static set prop(v: int) {
    A._val = v;
  }
}

class B extends A {
}

function main(): void {
  B.prop = 1;
}
