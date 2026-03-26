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

class A {
    foo(c: C2): string { return "A.C2" }
    foo(c: C1): string { return "A.C1" }
}

class B extends A {
    override foo(c: C2): string { return "B.C2" }
}

function main() {
    let a: A = new B()
    arktest.assertEQ(a.foo(new C2()), 'B.C2', 'B.foo(C2) call expected');
    arktest.assertEQ(a.foo(new C1()), 'A.C1', 'A.foo(C1) call expected');
}
