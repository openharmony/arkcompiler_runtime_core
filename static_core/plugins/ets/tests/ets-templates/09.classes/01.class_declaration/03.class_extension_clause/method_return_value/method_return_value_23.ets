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
desc: The return value of the method is [], and the modifier is public.
---*/

interface Person {
    name: string;
    age: number;
    hobbies: string[];
}

class Parent {
    public createGenericObject(): Person {
        return {
            name: "John",
            age: 30,
            hobbies: ["reading", "swimming"]
        };
    }
}

class Child extends Parent {}

function main(): void {
    let parent = new Parent();
    let obj1: Person = parent.createGenericObject();
    assertEQ(obj1.name, "John");
    assertEQ(obj1.age, 30);
    assertEQ(obj1.hobbies[0], "reading");
    assertEQ(obj1.hobbies[1], "swimming");

    let child = new Child();
    let obj2: Person = child.createGenericObject();
    assertEQ(obj2.name, "John");
    assertEQ(obj2.age, 30);
    assertEQ(obj2.hobbies[0], "reading");
    assertEQ(obj2.hobbies[1], "swimming");
}