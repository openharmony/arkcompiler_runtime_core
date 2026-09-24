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

export let numVal = 42;
export let strVal = 'hello_world';
export let boolVal = true;
export let bigIntVal = 123456789n;
export let undefVal = undefined;
export let nullVal = null;

export let testObject = {
    propNum: 100,
    propStr: 'test_string',
    propBool: false,
    propArr: [1, 2, 3],
    propNested: { inner: 'nested_val' },
    propFunc: function(x: number): number {
        return x * 3;
    }
};

export let testArray = [10, 20, 30, 40, 50];

export class ThreadTestClass {
    id: number;
    name: string;
    constructor(id: number, name: string) {
        this.id = id;
        this.name = name;
    }
    getId(): number {
        return this.id;
    }
    getName(): string {
        return this.name;
    }
    greet(): string {
        return `Hello, ${this.name}!`;
    }
}

export function add(a: number, b: number): number {
    return a + b;
}
export function multiply(a: number, b: number): number {
    return a * b;
}
export function greet(name: string): string {
    return `Hello, ${name}!`;
}

export let iterableObj = { a: 1, b: 2, c: 3, d: 4 };

export function throwErr(msg: string): void {
    throw new Error(msg);
}
