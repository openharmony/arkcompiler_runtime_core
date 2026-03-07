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

const createAndPopulateMap = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'createAndPopulateMap');
const getMapSize = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'getMapSize');
const setToMap = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'setToMap');
const getFromMap = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'getFromMap');
const hasInMap = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'hasInMap');
const deleteFromMap = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'deleteFromMap');
const clearMap = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'clearMap');
const forEachMap = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'forEachMap');
const iterateMap = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'iterateMap');
const createMapWithDuplicates = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'createMapWithDuplicates');
const chainSet = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'chainSet');
const getNonExistentKey = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'getNonExistentKey');
const mapWithMultipleEntries = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'mapWithMultipleEntries');
const getMapToString = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'getMapToString');
const createEmptyMapForToString = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'createEmptyMapForToString');
const getFromMapWithDefault = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'getFromMapWithDefault');
const getExistingKeyWithDefault = etsVm.getFunction('Lstmap/ETSGLOBAL;', 'getExistingKeyWithDefault');

function isInstanceOfMap(val: st.Map<string, number>): boolean {
    return val instanceof Map;
}

function testInstanceOf(): void {
    let val1 = STValue.newSTMap();
    ASSERT_TRUE(!isInstanceOfMap(val1));
}

function testGetSize(): void {
    let val1 = STValue.newSTMap();
    ASSERT_TRUE(val1.size === 0);
    let val2 = createAndPopulateMap();
    ASSERT_TRUE(val2.size === 2);
}

function testSetAndGet(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    val1.set('c', 3);
    ASSERT_TRUE(val1.size === 3);
    ASSERT_TRUE(val1.get('a') === 1);
    ASSERT_TRUE(val1.get('b') === 2);
    ASSERT_TRUE(val1.get('c') === 3);
}

function testHas(): void {
    let val1 = STValue.newSTMap();
    val1.set('hello', 1);
    val1.set('world', 2);
    ASSERT_TRUE(val1.has('hello') === true);
    ASSERT_TRUE(val1.has('world') === true);
    ASSERT_TRUE(val1.has('nonexistent') === false);
}

function testDelete(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    val1.set('c', 3);
    let deleted = val1.delete('b');
    ASSERT_TRUE(deleted === true);
    ASSERT_TRUE(val1.size === 2);
    ASSERT_TRUE(val1.has('a') === true);
    ASSERT_TRUE(val1.has('b') === false);
    ASSERT_TRUE(val1.has('c') === true);

    let deleted2 = val1.delete('nonexistent');
    ASSERT_TRUE(deleted2 === false);
    ASSERT_TRUE(val1.size === 2);
}

function testClear(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    val1.set('c', 3);
    ASSERT_TRUE(val1.size === 3);
    val1.clear();
    ASSERT_TRUE(val1.size === 0);
    ASSERT_TRUE(val1.has('a') === false);
    ASSERT_TRUE(val1.has('b') === false);
    ASSERT_TRUE(val1.has('c') === false);
}

function testForEach(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    val1.set('c', 3);
    let count = 0;
    let entries: [string, number][] = [];
    val1.forEach((value: number, key: string, map: st.Map<string, number>) => {
        count++;
        entries.push([key, value]);
        ASSERT_TRUE(map === val1);
    });
    ASSERT_TRUE(count === 3);
    ASSERT_TRUE(entries.length === 3);
}

function testKeys(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    val1.set('c', 3);
    let keys: string[] = [];
    for (let key of val1.keys()) {
        keys.push(key);
    }
    ASSERT_TRUE(keys.length === 3);
    ASSERT_TRUE(keys.includes('a'));
    ASSERT_TRUE(keys.includes('b'));
    ASSERT_TRUE(keys.includes('c'));
}

function testValues(): void {
    let val1 = STValue.newSTMap();
    val1.set('x', 1);
    val1.set('y', 2);
    val1.set('z', 3);
    let values: number[] = [];
    for (let value of val1.values()) {
        values.push(value);
    }
    ASSERT_TRUE(values.length === 3);
    ASSERT_TRUE(values.includes(1));
    ASSERT_TRUE(values.includes(2));
    ASSERT_TRUE(values.includes(3));
}

function testEntries(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    let count = 0;
    for (let entry of val1.entries()) {
        count++;
        let key = entry[0];
        let value = entry[1];
        ASSERT_TRUE(key === 'a' || key === 'b');
        ASSERT_TRUE(value === 1 || value === 2);
    }
    ASSERT_TRUE(count === 2);
}

function testForOf(): void {
    let val1 = STValue.newSTMap();
    val1.set('key1', 1);
    val1.set('key2', 2);
    val1.set('key3', 3);
    let count = 0;
    for (let entry of val1) {
        count++;
        let key = entry[0];
        let value = entry[1];
    }
    ASSERT_TRUE(count === 3);
}

function testSetReturnThis(): void {
    let val1 = STValue.newSTMap();
    let result = val1.set('a', 1);
    ASSERT_TRUE(result === val1);
    val1.set('b', 2).set('c', 3);
    ASSERT_TRUE(val1.size === 3);
}

function testGetMapSize(): void {
    let val1 = STValue.newSTMap();
    val1.set('x', 1);
    val1.set('y', 2);
    val1.set('z', 3);
    ASSERT_TRUE(getMapSize(val1) === 3);
}

function testSetToMap(): void {
    let val1 = STValue.newSTMap();
    let result = setToMap(val1, 'test', 42);
    ASSERT_TRUE(result.size === 1);
    ASSERT_TRUE(result.has('test') === true);
    ASSERT_TRUE(result.get('test') === 42);
}

function testHasInMap(): void {
    let val1 = STValue.newSTMap();
    val1.set('existing', 1);
    ASSERT_TRUE(hasInMap(val1, 'existing') === true);
    ASSERT_TRUE(hasInMap(val1, 'nonexistent') === false);
}

function testDeleteFromMap(): void {
    let val1 = STValue.newSTMap();
    val1.set('toremove', 1);
    let deleted = deleteFromMap(val1, 'toremove');
    ASSERT_TRUE(deleted === true);
    ASSERT_TRUE(val1.has('toremove') === false);
    ASSERT_TRUE(val1.size === 0);
}

function testClearMap(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    val1.set('c', 3);
    clearMap(val1);
    ASSERT_TRUE(val1.size === 0);
}

function testCreateAndPopulateMap(): void {
    let val1 = createAndPopulateMap();
    ASSERT_TRUE(val1.size === 2);
    ASSERT_TRUE(val1.has('apple') === true);
    ASSERT_TRUE(val1.has('banana') === true);
    ASSERT_TRUE(val1.get('apple') === 1);
    ASSERT_TRUE(val1.get('banana') === 2);
}

function testDuplicateKeyOverwrites(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    val1.set('a', 10);
    val1.set('b', 20);
    ASSERT_TRUE(val1.size === 2);
    ASSERT_TRUE(val1.get('a') === 10);
    ASSERT_TRUE(val1.get('b') === 20);
}

function testGetNonExistentKey(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    ASSERT_TRUE(val1.get('nonexistent') === undefined);
}

function testSizeIsProperty(): void {
    let val1 = STValue.newSTMap();
    ASSERT_TRUE(typeof val1.size === 'number');
    ASSERT_TRUE(val1.size === 0);
    val1.set('x', 1);
    ASSERT_TRUE(val1.size === 1);
}

function testInsertionOrder(): void {
    let val1 = STValue.newSTMap();
    val1.set('first', 1);
    val1.set('second', 2);
    val1.set('third', 3);
    let keys: string[] = [];
    for (let entry of val1.entries()) {
        keys.push(entry[0]);
    }
    ASSERT_TRUE(keys[0] === 'first');
    ASSERT_TRUE(keys[1] === 'second');
    ASSERT_TRUE(keys[2] === 'third');
}

function testChaining(): void {
    let val1 = STValue.newSTMap();
    let result = val1.set('a', 1).set('b', 2).set('c', 3);
    ASSERT_TRUE(result === val1);
    ASSERT_TRUE(val1.size === 3);
}

function testEmptyMap(): void {
    let val1 = STValue.newSTMap();
    ASSERT_TRUE(val1.size === 0);
    ASSERT_TRUE(val1.has('anything') === false);
    ASSERT_TRUE(val1.get('anything') === undefined);
    ASSERT_TRUE(val1.delete('nothing') === false);
    val1.clear();
    ASSERT_TRUE(val1.size === 0);
}

function testForEachThreeParams(): void {
    let val1 = STValue.newSTMap();
    val1.set('x', 1);
    val1.set('y', 2);
    let count = 0;
    val1.forEach((value: number, key: string, map: st.Map<string, number>) => {
        count++;
        ASSERT_TRUE(value === 1 || value === 2);
        ASSERT_TRUE(key === 'x' || key === 'y');
        ASSERT_TRUE(map === val1);
    });
    ASSERT_TRUE(count === 2);
}

function testEntriesKeyValuePairs(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    let entries: [string, number][] = [];
    for (let entry of val1.entries()) {
        entries.push([entry[0], entry[1]]);
    }
    ASSERT_TRUE(entries.length === 2);
    ASSERT_TRUE(entries.some(e => e[0] === 'a' && e[1] === 1));
    ASSERT_TRUE(entries.some(e => e[0] === 'b' && e[1] === 2));
}

function testMixedKeyTypes(): void {
    let val1 = STValue.newSTMap();
    val1.set(1, 'number');
    val1.set('1', 'string');
    val1.set(true, 'boolean');
    ASSERT_TRUE(val1.size === 3);
    ASSERT_TRUE(val1.get(1) === 'number');
    ASSERT_TRUE(val1.get('1') === 'string');
    ASSERT_TRUE(val1.get(true) === 'boolean');
}

function testSameTypeKeyValue(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 'value_a');
    val1.set('b', 'value_b');
    ASSERT_TRUE(val1.size === 2);
    ASSERT_TRUE(val1.get('a') === 'value_a');
    ASSERT_TRUE(val1.get('b') === 'value_b');
}

function testForEachFunction(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    val1.set('c', 3);
    ASSERT_TRUE(forEachMap(val1) === 3);
    ASSERT_TRUE(iterateMap(val1) === 3);
}

function testCreateMapWithDuplicates(): void {
    let val1 = createMapWithDuplicates();
    ASSERT_TRUE(val1.size === 3);
    ASSERT_TRUE(val1.get('a') === 10);
    ASSERT_TRUE(val1.get('b') === 20);
    ASSERT_TRUE(val1.get('c') === 3);
}

function testChainSetFunction(): void {
    let val1 = chainSet();
    ASSERT_TRUE(val1.size === 3);
    ASSERT_TRUE(val1.get('x') === 1);
    ASSERT_TRUE(val1.get('y') === 2);
    ASSERT_TRUE(val1.get('z') === 3);
}

function testGetNonExistentKeyFunction(): void {
    let val1 = STValue.newSTMap();
    val1.set('existing', 42);
    ASSERT_TRUE(getNonExistentKey(val1) === undefined);
}

function testMapWithMultipleEntries(): void {
    let val1 = mapWithMultipleEntries();
    ASSERT_TRUE(val1.size === 5);
    ASSERT_TRUE(val1.get('key0') === 0);
    ASSERT_TRUE(val1.get('key1') === 1);
    ASSERT_TRUE(val1.get('key2') === 2);
    ASSERT_TRUE(val1.get('key3') === 3);
    ASSERT_TRUE(val1.get('key4') === 4);
}

function testToStringEmptyMap(): void {
    let val1 = STValue.newSTMap();
    let str = val1.toString();
    ASSERT_TRUE(typeof str === 'string');
}

function testToStringPopulatedMap(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    val1.set('b', 2);
    val1.set('c', 3);
    let str = val1.toString();
    ASSERT_TRUE(typeof str === 'string');
}

function testToStringFromETS(): void {
    let val1 = createAndPopulateMap();
    let str = getMapToString(val1);
    ASSERT_TRUE(typeof str === 'string');
}

function testGetWithDefaultKeyExists(): void {
    let val1 = STValue.newSTMap();
    val1.set('existing', 42);
    let result = val1.get('existing', 99);
    ASSERT_TRUE(result === 42);
}

function testGetWithDefaultKeyNotExists(): void {
    let val1 = STValue.newSTMap();
    val1.set('existing', 42);
    let result = val1.get('nonexistent', 99);
    ASSERT_TRUE(result === 99);
}

function testGetWithDefaultEmptyMap(): void {
    let val1 = STValue.newSTMap();
    let result = val1.get('anything', 100);
    ASSERT_TRUE(result === 100);
}

function testGetWithDefaultZeroDefault(): void {
    let val1 = STValue.newSTMap();
    val1.set('a', 1);
    let result1 = val1.get('a', 0);
    ASSERT_TRUE(result1 === 1);
    let result2 = val1.get('nonexistent', 0);
    ASSERT_TRUE(result2 === 0);
}

function testGetWithDefaultFromETS(): void {
    let val1 = createAndPopulateMap();
    let existing = getExistingKeyWithDefault(val1, 'apple', 99);
    ASSERT_TRUE(existing === 1);
    let nonExistent = getFromMapWithDefault(val1, 'nonexistent', 88);
    ASSERT_TRUE(nonExistent === 88);
}

testInstanceOf();
testGetSize();
testSetAndGet();
testHas();
testDelete();
testClear();
testForEach();
testKeys();
testValues();
testEntries();
testForOf();
testSetReturnThis();
testGetMapSize();
testSetToMap();
testHasInMap();
testDeleteFromMap();
testClearMap();
testCreateAndPopulateMap();
testDuplicateKeyOverwrites();
testGetNonExistentKey();
testSizeIsProperty();
testInsertionOrder();
testChaining();
testEmptyMap();
testForEachThreeParams();
testEntriesKeyValuePairs();
testMixedKeyTypes();
testSameTypeKeyValue();
testForEachFunction();
testCreateMapWithDuplicates();
testChainSetFunction();
testGetNonExistentKeyFunction();
testMapWithMultipleEntries();
testToStringEmptyMap();
testToStringPopulatedMap();
testToStringFromETS();
testGetWithDefaultKeyExists();
testGetWithDefaultKeyNotExists();
testGetWithDefaultEmptyMap();
testGetWithDefaultZeroDefault();
testGetWithDefaultFromETS();
