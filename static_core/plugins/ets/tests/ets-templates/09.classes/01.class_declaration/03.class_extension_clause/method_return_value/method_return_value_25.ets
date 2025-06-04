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
desc: The return value of the method is void, and the modifier is public.
---*/

class Parent {
    private isExecuted = false;
    public getValue(): void {
        console.log("some operations");
        this.isExecuted = true;
    }

    public wasExecuted(): boolean {
        return this.isExecuted;
    }
}

class Child extends Parent {}

function main(): void {
    let parent = new Parent();
    parent.getValue();
    assertEQ(parent.wasExecuted(), true);

    let child = new Child();
    child.getValue();
    assertEQ(child.wasExecuted(), true);
}