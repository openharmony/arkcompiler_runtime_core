/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
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

let etsVm = globalThis.gtest.etsVm;
let testAll = etsVm.getFunction('Lesmap_static/ETSGLOBAL;', 'testAll');

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

export function testUserMapAge(map: Map<string, object>, age: number): boolean {
    for (let [key, value] of map) {
        if (value.age === age) {
            return true;
        }
    }
    return false;
}

export function testUserMapName(map: Map<string, object>, name: string): boolean {
    for (let [key, value] of map) {
        if (value.name === name) {
            return true;
        }
    }
    return false;
}

export function returnESUserMap(): Map<string, ESUser> {
    let map = new Map<string, ESUser>();
    map.set('user3', new ESUser('user3', 11));
    map.set('user4', new ESUser('user4', 22));
    return map;
}

export function esFuncMapTest(map: Map<string, number>, func: (map: Map<string, number>) => boolean): boolean {
    return func(map);
}

const map = new Map<string, number>();
export function returnSameMap(): Map<string, number> {
    return map;
}

export function createEmptyMap<K, V>(): Map<K, V> {
    return new Map<K, V>();
}

export function createStringNumberMap(...entries: [string, number][]): Map<string, number> {
    return new Map<string, number>(entries);
}

export function createNumberStringMap(...entries: [number, string][]): Map<number, string> {
    return new Map<number, string>(entries);
}

export let emptyMap: Map<string, number> = new Map<string, number>();
export let numberMap: Map<string, number> = new Map<string, number>([['key1', 1], ['key2', 2], ['key3', 3]]);

function main() {
    testAll();
}

main();
