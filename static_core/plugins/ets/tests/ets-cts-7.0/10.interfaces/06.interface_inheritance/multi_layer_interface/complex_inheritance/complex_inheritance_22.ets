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
desc: Interface combined with multi-level inheritance of static constants.
---*/

interface Greetable {
    readonly MESSAGE: string;
}

class GrandParent implements Greetable {
    static readonly MESSAGE = "Hello from GrandParent";
    readonly MESSAGE = "Hello from GrandParent Instance";
}

class Parent extends GrandParent {}

class Child extends Parent {
    static readonly MESSAGE = "Hello from Child";
    readonly MESSAGE = "Hello from Child Instance";
}

let i1: Greetable = new GrandParent();
let i2: Greetable = new Parent();
let i3: Greetable = new Child();

let g1: GrandParent = new GrandParent();
let g2: GrandParent = new Parent();
let g3: GrandParent = new Child();

let p1: Parent = new Parent();
let p2: Child = new Child();

let c1: Child = new Child();

function main() {
    arktest.assertEQ(i1.MESSAGE, "Hello from GrandParent Instance")
    arktest.assertEQ(i2.MESSAGE, "Hello from GrandParent Instance")
    arktest.assertEQ(i3.MESSAGE, "Hello from Child Instance")

    arktest.assertEQ(g1.MESSAGE, "Hello from GrandParent Instance")
    arktest.assertEQ(g2.MESSAGE, "Hello from GrandParent Instance")
    arktest.assertEQ(g3.MESSAGE, "Hello from Child Instance")

    arktest.assertEQ(p1.MESSAGE, "Hello from GrandParent Instance")
    arktest.assertEQ(p2.MESSAGE, "Hello from Child Instance")

    arktest.assertEQ(c1.MESSAGE, "Hello from Child Instance")

    arktest.assertEQ(GrandParent.MESSAGE, "Hello from GrandParent")
    arktest.assertEQ(Child.MESSAGE, "Hello from Child")
}