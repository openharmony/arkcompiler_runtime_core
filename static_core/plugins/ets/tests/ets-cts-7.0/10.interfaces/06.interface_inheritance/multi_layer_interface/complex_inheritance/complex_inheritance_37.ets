/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Rewriting of constants in the test class.
assertion: A property can be implemented as a field in different combinations.
---*/

interface G {
    CONSTANT: string;
}

class A implements G {
    CONSTANT: string = "A";
}

class B extends A {
    CONSTANT: string = "B";
}

let g1: G = new A();
let g2: G = new B();

let a1: A = new A();
let a2: A = new B();

let b: B = new B();

function main() {
    arktest.assertEQ(g1.CONSTANT, "A")
    arktest.assertEQ(g2.CONSTANT, "B")
    arktest.assertEQ(a1.CONSTANT, "A")
    arktest.assertEQ(a2.CONSTANT, "B")
    arktest.assertEQ(b.CONSTANT, "B")
}