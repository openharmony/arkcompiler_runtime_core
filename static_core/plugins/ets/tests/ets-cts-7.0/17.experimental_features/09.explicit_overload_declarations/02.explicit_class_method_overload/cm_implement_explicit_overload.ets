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

{% for c in cases %}
/*---
desc: If a class implements some interfaces with an explicit overload of the same name,
      then a new explicit overload must include all overloaded methods
---*/

class A1 {}
class A2 {}
class A3 {}
class A4 {}

interface I1 {
    f1(a: A1): string { return "1" }
    f2(a: A2): string { return "2" }
    overload foo { f1, f2 }
}

interface I2 {
    f3(a: A3): string { return "3" }
    f4(a: A4): string { return "4" }
    overload foo { f3, f4 }
}

class C implements I1, I2 {
    {{c.expl_over}}
}

function main() {
    let c = new C();
    let a1 = new A1();
    let a2 = new A2();
    let a3 = new A3();
    let a4 = new A4();

    arktest.assertEQ(c.foo(a1), "1", "f1 call expected");
    arktest.assertEQ(c.foo(a2), "2", "f2 call expected");
    arktest.assertEQ(c.foo(a3), "3", "f3 call expected");
    arktest.assertEQ(c.foo(a4), "4", "f4 call expected");
}

{% endfor %}
