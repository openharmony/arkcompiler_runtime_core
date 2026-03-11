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
let testAll = etsVm.getFunction('Lesset_static/ETSGLOBAL;', 'testAll');

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

export function testUserSetAge(set: Set<object>, age: number): boolean {
    for (let item of set) {
        if (item.age === age) {
            return true;
        }
    }
    return false;
}

export function testUserSetName(set: Set<object>, name: string): boolean {
    for (let item of set) {
        if (item.name === name) {
            return true;
        }
    }
    return false;
}

export function returnESUserSet(): Set<ESUser> {
    let set = new Set<ESUser>();
    set.add(new ESUser('user3', 11));
    set.add(new ESUser('user4', 22));
    return set;
}

export function esFuncSetTest(set: Set<number>, func: (set: Set<number>) => boolean): boolean {
    return func(set);
}

const set = new Set<number>();
export function returnSameSet(): Set<number> {
    return set;
}

export function createEmptySet<T>(): Set<T> {
    return new Set<T>();
}

export function createNumberSet(...values: number[]): Set<number> {
    return new Set<number>(values);
}

export function createStringSet(...values: string[]): Set<string> {
    return new Set<string>(values);
}

export function createObjectSet(...values: object[]): Set<object> {
    return new Set<object>(values);
}

export let emptySet: Set<string> = new Set<string>();
export let numberSet: Set<number> = new Set<number>([1, 2, 3, 4]);

function main() {
    testAll();
}

main();
