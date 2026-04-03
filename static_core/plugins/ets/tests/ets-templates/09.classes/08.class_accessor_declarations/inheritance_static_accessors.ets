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

{% for mod in modifiers %}

/*---
desc: Static accessors of a superclass are accessible via superclass name qualification.
assert: >-
    Static members of a superclass are not inherited. The subclass which wants
    to use static members of its superclass must use explicit qualification
    using the superclass name. That applies to static accessors.
params: {{mod}}
tags: [no-warmup]
---*/

class A {
  private static _val: int = 0;

  {{mod}} static get prop(): int {
    return A._val;
  }

  {{mod}} static set prop(v: int) {
    A._val = v;
  }
}

class B extends A {
  getProp(): int {
    return A.prop;
  }
  setProp(v: int): void {
    A.prop = v;
  }
}

function main(): void {
  let b: B = new B();
  arktest.assertEQ( b.getProp(), 0 );
  b.setProp(42);
  arktest.assertEQ( b.getProp(), 42 );
}

{% endfor %}
