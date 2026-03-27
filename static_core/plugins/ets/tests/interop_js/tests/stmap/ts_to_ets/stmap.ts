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

namespace st {
    interface Map<K, V> {
        get size(): number;

        set(key: K, value: V): this;

        get(key: K): V | undefined;

        get(key: K, def: V): V;

        has(key: K): boolean;

        delete(key: K): boolean;

        clear(): void;

        forEach(callbackfn: (value: V, key: K, map: Map<K, V>) => void): void;

        keys(): IterableIterator<K>;

        values(): IterableIterator<V>;

        entries(): IterableIterator<[K, V]>;
        
        toString(): string;

        [Symbol.iterator](): IterableIterator<[K, V]>;
    }
}

const etsVm = globalThis.gtest.etsVm;
let STValue = etsVm.STValue;
const testMapProto = etsVm.getFunction('Ltest_stmap/ETSGLOBAL;', 'testMapProto');

export function returnMap(): st.Map<string, number> {
    return STValue.newSTMap();
}

export function consumeMap(map: st.Map<string, number>): boolean {
    map.set('hello', 1);
    map.set('world', 2);
    return map.size === 2 && map.get('hello') === 1 && map.get('world') === 2;
}

export function checkDuplicateKeyOverwrites(map: st.Map<string, number>): boolean {
    let beforeSize = map.size;
    map.set('a', 1);
    map.set('b', 2);
    map.set('a', 10);
    map.set('b', 20);
    return map.size === beforeSize + 2 && map.get('a') === 10 && map.get('b') === 20;
}

export function checkSizeIsProperty(map: st.Map<string, number>): boolean {
    return typeof map.size === 'number';
}

export function checkInsertionOrder(map: st.Map<string, number>): boolean {
    let keys: string[] = [];
    for (let entry of map.entries()) {
        keys.push(entry[0]);
    }
    return keys[0] === 'first' && keys[1] === 'second' && keys[2] === 'third';
}

export function checkChaining(map: st.Map<string, number>): boolean {
    let result = map.set('x', 1).set('y', 2).set('z', 3);
    return result === map && map.size === 3;
}

export function checkGetReturnsUndefined(map: st.Map<string, number>): boolean {
    map.set('existing', 42);
    return map.get('existing') === 42 && map.get('nonexistent') === undefined;
}

export function checkDeleteReturnsBoolean(map: st.Map<string, number>): boolean {
    map.set('test', 1);
    let deleted = map.delete('test');
    let deletedNonExistent = map.delete('nonexistent');
    return typeof deleted === 'boolean' && deleted === true &&
           typeof deletedNonExistent === 'boolean' && deletedNonExistent === false;
}

export function checkHasMethod(map: st.Map<string, number>): boolean {
    map.set('a', 1);
    map.set('b', 2);
    return typeof map.has === 'function' && map.has('a') === true && map.has('c') === false;
}

export function checkClearMethod(map: st.Map<string, number>): void {
    map.set('a', 1);
    map.set('b', 2);
    map.set('c', 3);
    map.clear();
}

export function checkForEachMethod(map: st.Map<string, number>): boolean {
    map.set('a', 1);
    map.set('b', 2);
    let count = 0;
    map.forEach((value: number, key: string, m: st.Map<string, number>) => {
        count++;
    });
    return count === 2;
}

export function checkEntriesMethod(map: st.Map<string, number>): boolean {
    map.set('key1', 1);
    map.set('key2', 2);
    let entries: [string, number][] = [];
    for (let entry of map.entries()) {
        entries.push([entry[0], entry[1]]);
    }
    return entries.length === 2;
}

export function checkKeysMethod(map: st.Map<string, number>): boolean {
    map.set('a', 1);
    map.set('b', 2);
    let keys: string[] = [];
    for (let key of map.keys()) {
        keys.push(key);
    }
    return keys.length === 2 && keys.includes('a') && keys.includes('b');
}

export function checkValuesMethod(map: st.Map<string, number>): boolean {
    map.set('a', 1);
    map.set('b', 2);
    let values: number[] = [];
    for (let value of map.values()) {
        values.push(value);
    }
    return values.length === 2 && values.includes(1) && values.includes(2);
}

export function createEmptyMap(): Map<number, number> {
    return new Map<number, number>;
}

function main(): void {
    let res = false;
    try {
        testMapProto(new Map<number, number>());
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Value is not assignable');
        res = res && e.message.includes('Lstd/core/Map;');
    }
    ASSERT_TRUE(res);
}

main()
