/**
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

// Classes for toJSON testing
export class UserInfo {
    name: string = 'Alice';
    age: int = 30;
    active: boolean = true;
    score: double = 95.5;

    constructor() {}
    constructor(name: string, age: int) {
        this.name = name;
        this.age = age;
    }

    greet(): string {
        return 'Hello, ' + this.name;
    }

    setName(name: string): void {
        this.name = name;
    }

    setAge(age: int): void {
        this.age = age;
    }
}

export class Product {
    id: int = 1001;
    name: string = 'Laptop';
    price: double = 999.99;
    inStock: boolean = true;
}

export class EmptyClass {}

export class Address {
    city: string = 'Shanghai';
    zip: string = '200000';
}

export class Person {
    name: string = 'Bob';
    age: int = 25;
    address: Address = new Address();

    getName(): string {
        return this.name;
    }
}

export class CollectionHolder {
    tags: Array<string> = ['tag1', 'tag2', 'tag3'];
    numbers: Array<int> = [10, 20, 30];
    id: int = 42;
}

// Inheritance chain for toJSON type checking
export class BaseAnimal {
    type: string = 'animal';
}

export class DogBreed extends BaseAnimal {
    breed: string = 'Labrador';
}

export class PrimitiveFields {
    byteVal: byte = 0x0A;
    shortVal: short = 1000;
    intVal: int = 100000;
    longVal: long = 10000000000;
    floatVal: float = 3.14f;
    doubleVal: double = 2.718281828;
    charVal: char = c'X';
    booleanVal: boolean = true;
}

export class NullableFields {
    name: string | null = null;
    tag: string | null = 'default';
}

export enum Status {
    ACTIVE = 0,
    INACTIVE = 1,
    PENDING = 2,
}

export class EnumHolder {
    status: Status = Status.ACTIVE;
    name: string = 'holder';
}

export namespace Unwrap {
    export let magicBoolean: boolean = false;
    export let magicByte: byte = 24;
    export let magicChar: char = c'h';
    export let magicShort: short = 124;
    export let magicInt: int = 15;
    export let magicLong: long = 102400;
    export let magicFloat: float = 3.14f
    export let magicDouble: double = 3.1415926
    export let magicString1: string = 'Hello World';
    export let magicString2: string = 'Hello World!!!';

    /*
    * toBigIntTest
    */
    export let defaultNumber: int = 42;
    export let bigIntFromString: bigint = BigInt('999999999999999999');
    export let bigIntFromLiteral: bigint = 123456789012345678901234567890n;
    export let stringVariable: string = 'Hello, World!';
    export let zeroBigInt: bigint = 0n;
    export let negativeBigInt: bigint = -1234567890123456789n;

    // toJSON test variables
    export let userInfo: UserInfo = new UserInfo();
    export let customUserInfo: UserInfo = new UserInfo('Charlie', 28);
    export let productInfo: Product = new Product();
    export let personInfo: Person = new Person();
    export let emptyObj: EmptyClass = new EmptyClass();
    export let collectionHolder: CollectionHolder = new CollectionHolder();
    export let dogObj: DogBreed = new DogBreed();
    export let intArray: Array<int> = [1, 2, 3, 4, 5];
    export let strArray: Array<string> = ['hello', 'world'];
    export let emptyArray: Array<int> = [];
    export let nullValue: UserInfo | null = null;
    export let primitiveFields: PrimitiveFields = new PrimitiveFields();
    export let nullableFields: NullableFields = new NullableFields();
    export let enumHolder: EnumHolder = new EnumHolder();
}
