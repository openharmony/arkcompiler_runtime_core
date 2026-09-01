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
desc: >-
  TEST-UNIT-09-0028 GAP-09-0028 GRC-09-0028 CI-09-10-003-01: Child class
  object can be assigned to parent class variable
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

class Cat extends Animal {
    constructor(name: string) {
        super(name)
    }

    makeSound(): string {
        return 'Meow'
    }
}

function main(): void {
    let animal: Animal = new Cat('Whiskers')  // Subtype assignment
    console.log(animal.name)
    console.log(animal.makeSound())  // Should call Cat's makeSound
}
