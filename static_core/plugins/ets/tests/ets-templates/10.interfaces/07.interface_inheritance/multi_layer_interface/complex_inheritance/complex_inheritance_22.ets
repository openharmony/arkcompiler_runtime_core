/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

function main(): void {
    const child = new Child();
    assertEQ(child.MESSAGE, "Hello from Child Instance");
    assertEQ(Child.MESSAGE, "Hello from Child");
    assertEQ(GrandParent.MESSAGE, "Hello from GrandParent");
}