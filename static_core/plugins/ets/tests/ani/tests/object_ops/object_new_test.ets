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

class MobilePhone {
    constructor(model: String, weight: int) {
        this.model = model;
        this.weight = weight;
    }
    model: String;
    weight: int;
}

function checkModel(p: MobilePhone, model: String): boolean {
    return p.model == model;
}

function checkWeight(p: MobilePhone, weight: int): boolean {
    return p.weight == weight;
}

class A {
    constructor() {
    }

    boolean_method(a0: int, a1: int): boolean {
        if (a0 + a1 > 10) {
            return true;
        } else {
            return false;
        }
    }
}

class B {
    constructor() {
        throw new Error();
    }
};

class Animal {
    constructor(name: String, value: boolean) {
        this.name = name;
        this.value = value;
    }
    name: String;
    value: boolean;
}

function newAnimalObject() {
    return new Animal("Cat", true);
}

class Test {
    constructor(a: int, tag: String, animal: Animal) {
        this.a = a;
        this.tag = tag;
        this.animal = animal;
    }

    checkValue(): boolean {
        return this.a == 200 && this.tag == "Pure P60" && this.animal.name == "Cat" && this.animal.value == true;
    }

    setValue(a: int, tag: String, animal: Animal) {
        this.a = a;
        this.tag = tag;
        this.animal = animal;
    }

    newTestObject(a: int, tag: String, animal: Animal): Test {
        this.setValue(a, tag, animal);
        return this;
    }

    private a: int;
    private tag: String;
    private animal: Animal;
};

function getNull(): null {
    return null;
}

function getIntArray(): int[] {
    let a : int[] = [1, 2, 3, 4, 5]
    return a
}

function getByteArray(): byte[] {
    let a : byte[] = [6, 7, 8, 9]
    return a
}

class Mixture {
    newMixtureObject(v1: null, arr: int[], byteArr: byte[], i: int, f: float, d: double, b: boolean,
                  longVal: long, shortVal: short, str: String, animalObj: Animal): Mixture {
        this.v1 = v1;
        this.arr = arr;
        this.byteArr = byteArr;
        this.i = i;
        this.f = f;
        this.d = d;
        this.b = b;
        this.longVal = longVal;
        this.shortVal = shortVal;
        this.str = str;
        this.animalObj = animalObj;
        return this;
    }

    checkFun(): boolean {
        return true;
    }

    private v1: null = null;
    private arr: int[] = [1, 2, 3];
    private byteArr: byte[] = [10, 11, 12];
    private i: int = 0;
    private f: float = 1.0;
    private d: double = 20.0;
    private b: boolean = false;
    private longVal: long = 100;
    private shortVal: short = 5;
    private str: String = new String("");
    private animalObj: Animal = new Animal("Elephant", false);
};
