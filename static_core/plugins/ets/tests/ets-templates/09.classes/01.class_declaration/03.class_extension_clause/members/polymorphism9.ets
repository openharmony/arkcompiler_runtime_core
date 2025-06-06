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
      Testing polymorphism and access to member variables when rewriting methods.
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

class Wolf extends Animal {
    public packSize: number;
    constructor(name: string, packSize: number) {
        super(name);
        this.packSize = packSize;
    }
    public speak(): string {
        return "Howl!";
    }
}

function main(): void {
    const animal: Animal = new Wolf("Luna", 3);
    assertEQ((animal as Wolf).packSize, 3);
    assertEQ((animal as Wolf).name, "Luna");
}