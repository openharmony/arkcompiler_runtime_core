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
desc: Testing access to member variables in the case of polymorphism.
      Test member variable access when combining interfaces and polymorphism.
---*/

class Animal {
    name: string = '';
    constructor(name: string) {
        this.name = name;
    }
    public speak(): string {
        return "Animal speak";
    }
}

interface Motion {
    swim(): string;
}

class Dolphin extends Animal implements Motion {
    public swim(): string {
        return "Dolphin is swimming";
    }
    public speak(): string {
        return "Click!";
    }
    constructor(name: string) {
        super(name);
    }
}

function main(): void {
    const animal: Animal = new Dolphin("Dolly");
    assertEQ((animal as Dolphin).swim(), "Dolphin is swimming");
}