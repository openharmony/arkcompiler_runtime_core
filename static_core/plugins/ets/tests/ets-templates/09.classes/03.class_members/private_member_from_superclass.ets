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

{% for cm in class_members %}

/*---
desc: class members
assert: Members of a class that are declared private are not inherited by superclasses of that class.
tags: [negative, compile-only]
---*/

class A {
  {{cm.member}}
}

final class B extends A {
  public fee(): int {
    return {{cm.call}}
  }
}

function main(): void {
    let x: B = new B();

    assertEQ( x.fee(), {{cm.result}} );
}

{% endfor %}
