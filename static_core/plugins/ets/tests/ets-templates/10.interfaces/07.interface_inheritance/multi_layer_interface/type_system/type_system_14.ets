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
desc: Verify the correctness of the type system through the polymorphism of interfaces and classes.
---*/

interface Animal {
    sound(): string;
}

class Dog implements Animal {
    sound() {
        return "Woof!";
    }
}

class Cat implements Animal {
    sound() {
        return "Meow!";
    }
}

function main(): void {
    const animals: Animal[] = [new Dog(), new Cat()];
    assertEQ(animals[0].sound(), "Woof!");
    assertEQ(animals[1].sound(), "Meow!");
}