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
desc: Verify that a child interface uses the default implementation from one parent when no implementation is provided.
---*/

abstract class Parent1 {
    greet(): string {
        return "Hello from Parent1";
    }
}

interface Parent2 {
    greet(): string;
}

interface ChildWithDefaultParent extends Parent2 {}

class ChildWithDefaultParentClass extends Parent1 implements ChildWithDefaultParent {
}

function main(): void {
    const childWithDefaultParent = new ChildWithDefaultParentClass();
    assertEQ(childWithDefaultParent.greet(), "Hello from Parent1");
}