/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

{% for mod in modifiers %}

/*---
desc: Overriding method of superclass.
assert: An instance method mC declared in or inherited by class C, overrides from C another method mA declared in class A, if all of the following are true. C is a subclass of A. C does not inherit mA. The signature of mC is a subsignature (link to signatures) of the signature of mA.
params: {{mod}}
---*/

class A {
  {{mod}} m(): int {
    return 1;
  }
}

final class C extends A {
  override m(): int {
    return 5;
  }
}

function main(): void {
  let instance: C = new C();

  assertTrue( instance.m() == 5 );
}

{% endfor %}
