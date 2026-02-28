/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let etsVm = globalThis.gtest.etsVm;
let testAll = etsVm.getFunction('Lesarray_static/ETSGLOBAL;', 'testAll');

export class ESUser {
    private name_: string;
    private age_: number;

    constructor(name: string, age: number) {
        this.name_ = name;
        this.age_ = age;
    }

    get name(): string {
        return this.name_;
    }
    get age(): number {
        return this.age_;
    }
}

export function testUserArrayAge(arr: Array<object>, idx: number, age: number): boolean {
    return arr[idx].age === age;
}

export function testUserArrayName(arr: Array<object>, idx: number, name: string): boolean {
    return arr[idx].name === name;
}

export function returnESUserArray(): Array<ESUser> {
    let arr = [];
    arr.push(new ESUser('user3', 11));
    arr.push(new ESUser('user4', 22));
    return arr;
}

export function returnFlatArray(): Array<Object> {
    let arr = [1, [2, 3, [4, 5]]];
    return arr;
}

export function esFuncArrayTest(arr: Array<number>, func: (arr: Array<number>) => boolean): boolean {
    return func(arr);
}

const arr = [];
export function returnSameArray(): Array<number> {
 	return arr;
}

export function createEmptyArray<T>(): Array<T> {
    return [];
}

export function createNumberArray(...values: number[]): Array<number> {
    return [...values];
}

export function createStringArray(...values: string[]): Array<string> {
    return [...values];
}

export function createObjectArray(...values: object[]): Array<object> {
    return [...values];
}

export let emptyArray: Array<string> = new Array<string>();
export let numberArray: Array<number> = new Array<number>(1, 2, 3, 4);

function main() {
    testAll();
}

main();
