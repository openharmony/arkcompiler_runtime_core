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
desc: The return value of the method is short, and the modifier is public.
---*/

class Parent {
    public getValue(): short {
        return 100;
    }
}

class Child extends Parent {}

function main(): void {
    let parent = new Parent();
    let parentValue: short = parent.getValue();
    assertEQ(parentValue, 100);

    let child = new Child();
    let childValue: short = child.getValue();
    assertEQ(childValue, 100);
}