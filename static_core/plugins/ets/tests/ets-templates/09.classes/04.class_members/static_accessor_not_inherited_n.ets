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
desc: >-
  Static accessors of a superclass are not inherited. Accessing a superclass
  static accessor via the subclass name causes a compile-time error.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0087: Property 'sProp' does not exist on type 'Child'"
---*/

class Base {
  private static _val: int = 30;
  static get sProp(): int { return Base._val; }
  static set sProp(v: int) { Base._val = v; }
}

class Child extends Base {}

function main(): void {
  let x = Child.sProp;
}
