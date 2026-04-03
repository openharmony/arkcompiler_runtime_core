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
desc: Static fields of a superclass are accessible via superclass name qualification.
assert: >-
    Static members of a superclass are not inherited. The subclass which wants
    to use static members of its superclass must use explicit qualification
    using the superclass name. That applies to static fields.
params: {{mod}}
---*/

class A {
  {{mod}} static f: int = 1
  {{mod}} static readonly rf: int = 2
}

class B extends A {
  getF(): int {
    return A.f;
  }
  getRF(): int {
    return A.rf;
  }
}

function main(): void {
  let b: B = new B();
  arktest.assertEQ( b.getF(), 1 );
  arktest.assertEQ( b.getRF(), 2 );
}

{% endfor %}
