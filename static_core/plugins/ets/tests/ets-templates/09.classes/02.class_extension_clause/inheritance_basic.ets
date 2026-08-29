/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: "TEST-UNIT-09-0005 GAP-09-0005 GRC-09-0005 CI-09-02-001-01: Subclass inherits parent class members"
tags: [compile-only]
---*/

class Animal {
    public name: string

    constructor(name: string) {
        this.name = name
    }

    makeSound(): string {
        return 'Some sound'
    }
}

class Dog extends Animal {
    public breed: string

    constructor(name: string, breed: string) {
        super(name)
        this.breed = breed
    }

    makeSound(): string {
        return 'Woof!'
    }
}

function main(): void {
    let dog = new Dog('Buddy', 'Golden Retriever')
    console.log(dog.name)  // Inherited from Animal
    console.log(dog.breed)
    console.log(dog.makeSound())
}
