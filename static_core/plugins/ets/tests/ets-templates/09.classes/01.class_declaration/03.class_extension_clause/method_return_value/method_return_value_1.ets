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
desc: The method returns a value of number and the modifier is protected to verify that the rewrite works correctly.
---*/

class Parent {
    protected getValue(): number {
        return 100;
    }

    public getParentValue(): number {
        return this.getValue();
    }
}

class Child extends Parent {

    protected getValue(): number {
        return 200;
    }

    public getChildValue(): number {
        return this.getValue();
    }
}

function main(): void {
    let parent = new Parent();
    let parentValue: number = parent.getParentValue();
    assertEQ(parentValue, 100);

    let child = new Child();
    let childValue: number = child.getChildValue();
    assertEQ(childValue, 200);
}