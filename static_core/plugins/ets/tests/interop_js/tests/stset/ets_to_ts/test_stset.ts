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
    interface Set<T> {

        get size(): number;

        add(value: T): this;

        has(value: T): boolean;

        delete(value: T): boolean;

        clear(): void;

        forEach(callbackfn: (value: T, value2: T, set: Set<T>) => void): void;

        keys(): IterableIterator<T>;

        values(): IterableIterator<T>;

        entries(): IterableIterator<[T, T]>;

        toString(): string;
        
        [Symbol.iterator](): IterableIterator<T>;

        [index: number]: T | undefined;
    }
}

const etsVm = globalThis.gtest.etsVm;
let STValue = etsVm.STValue;

const createAndPopulateSet = etsVm.getFunction('Lstset/ETSGLOBAL;', 'createAndPopulateSet');
const getSetSize = etsVm.getFunction('Lstset/ETSGLOBAL;', 'getSetSize');
const addToSet = etsVm.getFunction('Lstset/ETSGLOBAL;', 'addToSet');
const hasInSet = etsVm.getFunction('Lstset/ETSGLOBAL;', 'hasInSet');
const deleteFromSet = etsVm.getFunction('Lstset/ETSGLOBAL;', 'deleteFromSet');
const clearSet = etsVm.getFunction('Lstset/ETSGLOBAL;', 'clearSet');
const getSetToString = etsVm.getFunction('Lstset/ETSGLOBAL;', 'getSetToString');
const createEmptySetForToString = etsVm.getFunction('Lstset/ETSGLOBAL;', 'createEmptySetForToString');

function isInstanceOfSet(val: st.Set<string>): boolean {
    return val instanceof Set;
}

function testInstanceOf(): void {
    let val1 = STValue.newSTSet();
    ASSERT_TRUE(!isInstanceOfSet(val1));
}

function testGetSize(): void {
    let val1 = STValue.newSTSet();
    ASSERT_TRUE(val1.size === 0);
    let val2 = createAndPopulateSet();
    ASSERT_TRUE(val2.size === 2);
}

function testAdd(): void {
    let val1 = STValue.newSTSet();
    val1.add('a');
    val1.add('b');
    val1.add('c');
    ASSERT_TRUE(val1.size === 3);
    ASSERT_TRUE(val1.has('a') === true);
    ASSERT_TRUE(val1.has('b') === true);
    ASSERT_TRUE(val1.has('c') === true);
}

function testHas(): void {
    let val1 = STValue.newSTSet();
    val1.add('hello');
    val1.add('world');
    ASSERT_TRUE(val1.has('hello') === true);
    ASSERT_TRUE(val1.has('world') === true);
    ASSERT_TRUE(val1.has('nonexistent') === false);
}

function testDelete(): void {
    let val1 = STValue.newSTSet();
    val1.add('a');
    val1.add('b');
    val1.add('c');
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
    let val1 = STValue.newSTSet();
    val1.add('a');
    val1.add('b');
    val1.add('c');
    ASSERT_TRUE(val1.size === 3);
    val1.clear();
    ASSERT_TRUE(val1.size === 0);
    ASSERT_TRUE(val1.has('a') === false);
    ASSERT_TRUE(val1.has('b') === false);
    ASSERT_TRUE(val1.has('c') === false);
}

function testForEach(): void {
    let val1 = STValue.newSTSet();
    val1.add('a');
    val1.add('b');
    val1.add('c');
    let count = 0;
    let values: string[] = [];
    val1.forEach((value: string, value2: string, set: st.Set<string>) => {
        count++;
        values.push(value);
        ASSERT_TRUE(value === value2);
        ASSERT_TRUE(set === val1);
    });
    ASSERT_TRUE(count === 3);
    ASSERT_TRUE(values.length === 3);
    ASSERT_TRUE(values.includes('a'));
    ASSERT_TRUE(values.includes('b'));
    ASSERT_TRUE(values.includes('c'));
}

function testKeys(): void {
    let val1 = STValue.newSTSet();
    val1.add('a');
    val1.add('b');
    val1.add('c');
    let values: string[] = [];
    for (let key of val1.keys()) {
        values.push(key);
    }
    ASSERT_TRUE(values.length === 3);
    ASSERT_TRUE(values.includes('a'));
    ASSERT_TRUE(values.includes('b'));
    ASSERT_TRUE(values.includes('c'));
}

function testValues(): void {
    let val1 = STValue.newSTSet();
    val1.add('x');
    val1.add('y');
    val1.add('z');
    let values: string[] = [];
    for (let value of val1.values()) {
        values.push(value);
    }
    ASSERT_TRUE(values.length === 3);
    ASSERT_TRUE(values.includes('x'));
    ASSERT_TRUE(values.includes('y'));
    ASSERT_TRUE(values.includes('z'));
}

function testEntries(): void {
    let val1 = STValue.newSTSet();
    val1.add('a');
    val1.add('b');
    let count = 0;
    for (let entry of val1.entries()) {
        count++;
        ASSERT_TRUE(entry[0] === entry[1]);
        ASSERT_TRUE(entry[0] === 'a' || entry[0] === 'b');
    }
    ASSERT_TRUE(count === 2);
}

function testForOf(): void {
    let val1 = STValue.newSTSet();
    val1.add('1');
    val1.add('2');
    val1.add('3');
    let values: string[] = [];
    for (let value of val1) {
        values.push(value);
    }
    ASSERT_TRUE(values.length === 3);
    ASSERT_TRUE(values.includes('1'));
    ASSERT_TRUE(values.includes('2'));
    ASSERT_TRUE(values.includes('3'));
}

function testAddReturnThis(): void {
    let val1 = STValue.newSTSet();
    let result = val1.add('a');
    ASSERT_TRUE(result === val1);
    val1.add('b').add('c');
    ASSERT_TRUE(val1.size === 3);
}

function testGetSetSize(): void {
    let val1 = STValue.newSTSet();
    val1.add('x');
    val1.add('y');
    val1.add('z');
    ASSERT_TRUE(getSetSize(val1) === 3);
}

function testAddToSet(): void {
    let val1 = STValue.newSTSet();
    let result = addToSet(val1, 'test');
    ASSERT_TRUE(result.size === 1);
    ASSERT_TRUE(result.has('test') === true);
}

function testHasInSet(): void {
    let val1 = STValue.newSTSet();
    val1.add('existing');
    ASSERT_TRUE(hasInSet(val1, 'existing') === true);
    ASSERT_TRUE(hasInSet(val1, 'nonexistent') === false);
}

function testDeleteFromSet(): void {
    let val1 = STValue.newSTSet();
    val1.add('toremove');
    let deleted = deleteFromSet(val1, 'toremove');
    ASSERT_TRUE(deleted === true);
    ASSERT_TRUE(val1.has('toremove') === false);
    ASSERT_TRUE(val1.size === 0);
}

function testClearSet(): void {
    let val1 = STValue.newSTSet();
    val1.add('a');
    val1.add('b');
    val1.add('c');
    clearSet(val1);
    ASSERT_TRUE(val1.size === 0);
}

function testCreateAndPopulateSet(): void {
    let val1 = createAndPopulateSet();
    ASSERT_TRUE(val1.size === 2);
    ASSERT_TRUE(val1.has('hello') === true);
    ASSERT_TRUE(val1.has('world') === true);
}

function testUniqueness(): void {
    let val1 = STValue.newSTSet();
    val1.add('a');
    val1.add('b');
    val1.add('a');
    val1.add('b');
    ASSERT_TRUE(val1.size === 2);
    ASSERT_TRUE(val1.has('a') === true);
    ASSERT_TRUE(val1.has('b') === true);
}

function testSizeIsProperty(): void {
    let val1 = STValue.newSTSet();
    ASSERT_TRUE(typeof val1.size === 'number');
    ASSERT_TRUE(val1.size === 0);
    val1.add('x');
    ASSERT_TRUE(val1.size === 1);
}

function testInsertionOrder(): void {
    let val1 = STValue.newSTSet();
    val1.add('first');
    val1.add('second');
    val1.add('third');
    let values: string[] = [];
    for (let value of val1) {
        values.push(value);
    }
    ASSERT_TRUE(values[0] === 'first');
    ASSERT_TRUE(values[1] === 'second');
    ASSERT_TRUE(values[2] === 'third');
}

function testMixedTypes(): void {
    let val1 = STValue.newSTSet();
    val1.add(1);
    val1.add('1');
    val1.add(true);
    val1.add(null);
    ASSERT_TRUE(val1.size === 4);
    ASSERT_TRUE(val1.has(1) === true);
    ASSERT_TRUE(val1.has('1') === true);
    ASSERT_TRUE(val1.has(true) === true);
    ASSERT_TRUE(val1.has(null) === true);
}

function testChaining(): void {
    let val1 = STValue.newSTSet();
    let result = val1.add('a').add('b').add('c');
    ASSERT_TRUE(result === val1);
    ASSERT_TRUE(val1.size === 3);
}

function testEmptySet(): void {
    let val1 = STValue.newSTSet();
    ASSERT_TRUE(val1.size === 0);
    ASSERT_TRUE(val1.has('anything') === false);
    ASSERT_TRUE(val1.delete('nothing') === false);
    val1.clear();
    ASSERT_TRUE(val1.size === 0);
}

function testForEachThreeParams(): void {
    let val1 = STValue.newSTSet();
    val1.add('x');
    val1.add('y');
    let count = 0;
    val1.forEach((value: string, value2: string, set: st.Set<string>) => {
        count++;
        ASSERT_TRUE(value === value2);
        ASSERT_TRUE(set === val1);
    });
    ASSERT_TRUE(count === 2);
}

function testNoIndexAccess(): void {
    let val1 = STValue.newSTSet();
    val1.add('a');
    val1.add('b');
    ASSERT_TRUE(val1[0] === undefined);
    ASSERT_TRUE(val1[1] === undefined);
}

function testNoPushPop(): void {
    let val1 = STValue.newSTSet();
    val1.add('a');
    val1.add('b');
    ASSERT_TRUE(val1.size === 2);
    val1.delete('a');
    ASSERT_TRUE(val1.size === 1);
}

function testToStringEmptySet(): void {
    let val1 = STValue.newSTSet();
    let str = val1.toString();
    ASSERT_TRUE(typeof str === 'string');
}

function testToStringPopulatedSet(): void {
    let val1 = STValue.newSTSet();
    val1.add('a');
    val1.add('b');
    val1.add('c');
    let str = val1.toString();
    ASSERT_TRUE(typeof str === 'string');
}

function testToStringFromETS(): void {
    let val1 = createAndPopulateSet();
    let str = getSetToString(val1);
    ASSERT_TRUE(typeof str === 'string');
}

testInstanceOf();
testGetSize();
testAdd();
testHas();
testDelete();
testClear();
testForEach();
testKeys();
testValues();
testEntries();
testForOf();
testAddReturnThis();
testGetSetSize();
testAddToSet();
testHasInSet();
testDeleteFromSet();
testClearSet();
testCreateAndPopulateSet();
testUniqueness();
testSizeIsProperty();
testInsertionOrder();
testMixedTypes();
testChaining();
testEmptySet();
testForEachThreeParams();
testNoIndexAccess();
testNoPushPop();
testToStringEmptySet();
testToStringPopulatedSet();
testToStringFromETS();
