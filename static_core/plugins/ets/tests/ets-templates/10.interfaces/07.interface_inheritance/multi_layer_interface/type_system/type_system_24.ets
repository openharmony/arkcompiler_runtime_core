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
desc: Verify that the GermanShepherd instance is of type Dog.
---*/

interface Animal {
    name: string;
    age: number;
    makeSound(): void;
}

interface Dog extends Animal {
    breed: string;
    bark(): void;
}

interface Guard {
    guard(): void;
}

class GermanShepherd implements Dog, Guard {
    name: string = '';
    age: number = 0;
    breed: string = '';

    constructor(name: string, age: number, breed: string) {
        this.name = name;
        this.age = age;
        this.breed = breed;
    }

    makeSound(): void {
        console.log("Generic animal sound");
    }

    bark(): void {
        console.log("Woof! Woof!");
    }

    guard(): void {
        console.log("Guarding the house");
    }
}

function main(): void {
    const dog: Dog = new GermanShepherd("Rex", 3, "German Shepherd");
    assertEQ(dog instanceof GermanShepherd, true);
}