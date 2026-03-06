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

class Student {
    public name: string
    public age: number
    public pet: Pet
    constructor(name: string, age: number, pet: Pet) {
        this.name = name;
        this.age = age;
        this.pet = pet;
    }
    getName(): string {
        return this.name;
    }
    getAge(): number {
        return this.age;
    }
}


class Pet {
    public name: string
    public species: string
    
    constructor(name: string, species: string) {
        this.name = name;
        this.species = species;
    }
    getName(): string {
        return this.name;
    }
    getSpecies(): string {
        return this.species;
    }
}

let pet = new Pet('Tom', 'Cat');
let student = new Student('Alice', 20, pet);