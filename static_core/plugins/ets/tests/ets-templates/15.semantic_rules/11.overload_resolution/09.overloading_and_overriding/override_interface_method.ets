/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/*---
desc: As a compile-time polymorphism, overload resolution selects the method 
      to call from a class type or an interface type known at compile time.
---*/

class C1 {}
class C2 extends C1 {}

interface I {
    foo(c: C2): string { return "I.C2" }
    foo(c: C1): string { return "I.C1" }
}

class A implements I {
    override foo(c: C2): string { return "A.C2" }
}

function main() {
    let a: I = new A()
    arktest.assertEQ(a.foo(new C2()), 'A.C2', 'A.foo(C2) call expected');
    arktest.assertEQ(a.foo(new C1()), 'I.C1', 'I.foo(C1) call expected');
}
