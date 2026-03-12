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
    interface Array<T> {

        get length(): number;

        set length(newLength: number);

        [index: number]: T;

        sort(comparator?: (a: T, b: T) => number): Array<T>;

        shift(): T | undefined;

        unshift(...val: T[]): number;

        pop(): T | undefined;

        push(...val: T[]): number;

        splice(start: number): Array<T>;

        splice(start: number, deleteCount: number, ...val: T[]): Array<T>;

        keys(): IterableIterator<number>;

        values(): IterableIterator<T>;

        entries(): IterableIterator<[number, T]>;

        [Symbol.iterator](): IterableIterator<T>;

        filter(predicate: (value: T, index: number, array: Array<T>) => boolean): Array<T>;

        flat(depth?: number): Array<T>;

        concat(...val: Array<T>[]): Array<T>;

        at(key: number): T;

        copyWithin(target: number, start?: number, end?: number): Array<T>;

        fill(value: T, start?: number, end?: number): Array<T>;

        find(predicate: (value: T, index: number, array: Array<T>) => boolean): T | undefined;

        findIndex(predicate: (value: T, index: number, array: Array<T>) => boolean): number;

        findLast(predicate: (value: T, index: number, array: Array<T>) => boolean): T | undefined;

        findLastIndex(predicate: (value: T, index: number, array: Array<T>) => boolean): number;

        every(predicate: (value: T, index: number, array: Array<T>) => boolean): boolean;

        some(predicate: (value: T, index: number, array: Array<T>) => boolean): boolean;

        reduce(callback: (previousVal: T, curVal: T, idx: number, arr: Array<T>) => T, initVal?: T): T;

        reduceRight(callback: (previousVal: T, curVal: T, idx: number, arr: Array<T>) => T, initVal?: T): T;

        forEach(callbackfn: (value: T, index: number, array: Array<T>) => void): void;

        slice(start: number, end?: number): Array<T>;

        lastIndexOf(searchElement: T, fromIndex?: number): number;

        join(sep?: string): string;

        toString(): string;

        toLocaleString(): string;

        toSpliced(start?: number, deleteCount?: number, ...items: T[]): Array<T>;

        includes(val: T, fromIndex?: number): boolean;

        indexOf(val: T, fromIndex?: number): number;

        toSorted(comparator?: (a: T, b: T) => number): Array<T>;

        reverse(): Array<T>;

        toReversed(): Array<T>;

        with(index: number, val: T): Array<T>;

        map(callbackfn: (value: T, index: number, array: Array<T>) => T): Array<T>;
    }
}

const etsVm = globalThis.gtest.etsVm;
let STValue = etsVm.STValue;

const getConstArray = etsVm.getFunction('Lstarray/ETSGLOBAL;', 'getConstArray');
const customLengthArray = etsVm.getFunction('Lstarray/ETSGLOBAL;', 'customLengthArray');
const testArrayValByIndex = etsVm.getFunction('Lstarray/ETSGLOBAL;', 'testArrayValByIndex');
const setArrayValByIndex = etsVm.getFunction('Lstarray/ETSGLOBAL;', 'setArrayValByIndex');
const User = etsVm.getClass('Lstarray/User;');
const testUserArrayName = etsVm.getFunction('Lstarray/ETSGLOBAL;', 'testUserArrayName');
const testUserArrayAge = etsVm.getFunction('Lstarray/ETSGLOBAL;', 'testUserArrayAge');
const returnUserArray = etsVm.getFunction('Lstarray/ETSGLOBAL;', 'returnUserArray');
const esFuncArrayTest = etsVm.getFunction('Lstarray/ETSGLOBAL;', 'esFuncArrayTest');

function isInstatnceofArray(val: st.Array<string>): boolean {
    return val instanceof Array;
}

function testInstatnceof(): void {
    let val1 = customLengthArray(0);
    ASSERT_TRUE(!isInstatnceofArray(val1));
    let val2 = STValue.newSTArray();
    ASSERT_TRUE(!isInstatnceofArray(val2));
}

function testSameObject(): void {
    let val1 = getConstArray();
    let val2 = getConstArray();
    ASSERT_TRUE(val1 === val2);
    let val3 = STValue.newSTArray();
    let val4 = STValue.newSTArray();
    ASSERT_TRUE(val3 !== val4);
}

function testGetSetLength(): void {
    let val1 = customLengthArray(10);
    ASSERT_TRUE(val1.length === 10);
    val1.length = 3;
    ASSERT_TRUE(val1.length === 3);
    let val2 = customLengthArray(0);
    try {
        val2.length = 10;
    } catch (e: Error) {
        ASSERT_TRUE(e.message === "can't change length to bigger or negative");
    }
    let val3 = STValue.newSTArray();
    ASSERT_TRUE(val3.length === 0);
}

function testGetterSetterByIndex(): void {
    let val1 = customLengthArray(10);
    ASSERT_TRUE(val1[0] === undefined);
    ASSERT_TRUE(val1[9] === undefined);
    try {
        ASSERT_TRUE(val1[10] === undefined);
    } catch(e: Error) {
        ASSERT_TRUE(e.message === 'Out of bounds');
    }
    let testStr = 'test';
    val1[0] = testStr;
    ASSERT_TRUE(val1[0] === testStr);
    ASSERT_TRUE(testArrayValByIndex(val1, 0, testStr));
    setArrayValByIndex(val1, 1, testStr);
    ASSERT_TRUE(val1[1] === testStr);
    let val2 = STValue.newSTArray();
    try {
        ASSERT_TRUE(val2[0] === undefined);
    } catch(e: Error) {
        ASSERT_TRUE(e.message === 'Out of bounds');
    }
}

function testShiftUnshift(): void {
    let val1 = STValue.newSTArray();
    val1.unshift('1');
    val1.unshift('4', '3', '2');
    ASSERT_TRUE(testArrayValByIndex(val1, 0, '4'));
    ASSERT_TRUE(testArrayValByIndex(val1, 1, '3'));
    ASSERT_TRUE(testArrayValByIndex(val1, 2, '2'));
    ASSERT_TRUE(testArrayValByIndex(val1, 3, '1'));
    ASSERT_TRUE(val1.shift() === '4');
    ASSERT_TRUE(val1.shift() === '3');
    ASSERT_TRUE(val1.shift() === '2');
    ASSERT_TRUE(val1.shift() === '1');
    ASSERT_TRUE(val1.shift() === undefined);
}

function testPushPop(): void {
    let val1 = STValue.newSTArray();
    val1.push('1');
    val1.push('2', '3', '4');
    ASSERT_TRUE(testArrayValByIndex(val1, 0, '1'));
    ASSERT_TRUE(testArrayValByIndex(val1, 1, '2'));
    ASSERT_TRUE(testArrayValByIndex(val1, 2, '3'));
    ASSERT_TRUE(testArrayValByIndex(val1, 3, '4'));
    ASSERT_TRUE(val1.pop() === '4');
    ASSERT_TRUE(val1.pop() === '3');
    ASSERT_TRUE(val1.pop() === '2');
    ASSERT_TRUE(val1.pop() === '1');
    ASSERT_TRUE(val1.pop() === undefined);
}

function testToString(): void {
    let val1 = STValue.newSTArray();
    ASSERT_TRUE(val1.toString() === '');
    ASSERT_TRUE(val1.toLocaleString() === '');
    val1.push('1', '2', '3', '4');
    ASSERT_TRUE(val1.toString() === '1,2,3,4');
    ASSERT_TRUE(val1.toLocaleString() === '1,2,3,4');
    let val2 = customLengthArray(3);
    ASSERT_TRUE(val2.toString() === ',,');
    ASSERT_TRUE(val2.toLocaleString() === ',,');
}

function testSplice(): void {
    let val1 = STValue.newSTArray();
    val1.push('1', '2', '3');
    val1.splice(1);
    ASSERT_TRUE(val1.toString() === '1');
    let val2 = STValue.newSTArray();
    val2.push('1', '2', '3');
    val2.splice(1, 1, 'a', 'b', 'c');
    ASSERT_TRUE(val2.toString() === '1,a,b,c,3');
}

function testToSpliced(): void {
    let val1 = STValue.newSTArray();
    val1.push('1', '2', '3');
    let val2 = val1.toSpliced(1);
    ASSERT_TRUE(val1.toString() === '1,2,3');
    ASSERT_TRUE(val2.toString() === '1');
    let val3 = STValue.newSTArray();
    val3.push('1', '2', '3');
    let val4 = val3.toSpliced(1, 1, 'a', 'b', 'c');
    ASSERT_TRUE(val3.toString() === '1,2,3');
    ASSERT_TRUE(val4.toString() === '1,a,b,c,3');
}


function testKeysValuesEntries(): void {
    let val1 = STValue.newSTArray();
    val1.push('1', '2', '3');
    let index = 0;
    for (let key of val1.keys()) {
        ASSERT_TRUE(key === index++);
    }
    index = 0;
    for (let val of val1.values()) {
        ASSERT_TRUE(val === val1[index++]);
    }
    index = 0;
    for (let entry of val1.entries()) {
        ASSERT_TRUE(entry[0] === index);
        ASSERT_TRUE(entry[1] === val1[index++]);
    }
}

function testForOf(): void {
    let val1 = STValue.newSTArray();
    val1.push('1', '2', '3');
    let index = 0;
    for (let val of val1) {
        ASSERT_TRUE(val === val1[index++]);
    }
}

function testReferenceArray(): void {
    let val1 = STValue.newSTArray();
    val1.push(new User('user1', 10));
    val1.push(new User('user2', 20));
    ASSERT_TRUE(testUserArrayName(val1, 0, 'user1'));
    ASSERT_TRUE(testUserArrayName(val1, 1, 'user2'));
    ASSERT_TRUE(testUserArrayAge(val1, 0, 10));
    ASSERT_TRUE(testUserArrayAge(val1, 1, 20));
    let val2 = returnUserArray();
    ASSERT_TRUE(val2[0].name === 'user3');
    ASSERT_TRUE(val2[0].age === 11);
    ASSERT_TRUE(val2[1].name === 'user4');
    ASSERT_TRUE(val2[1].age === 22);
}

function testFilter(): void {
    let val1 = STValue.newSTArray();
    val1.push(1, 3, 5, 2, 4, 6);
    let val2 = val1.filter((val) => {return val >= 3;});
    ASSERT_TRUE(val1.toString() === '1,3,5,2,4,6');
    ASSERT_TRUE(val2.toString() === '3,5,4,6');
}

function testFlat(): void {
    let val1 = STValue.newSTArray();
    val1.push(1);
    let val2 = STValue.newSTArray();
    val2.push(2, 3);
    let val3 = STValue.newSTArray();
    val3.push(4, 5);
    val2.push(val3);
    val1.push(val2);
    let val4 = val1.flat();
    ASSERT_TRUE(val4[1] === 2);
    ASSERT_TRUE(val4[2] === 3);
    ASSERT_TRUE(val4[3] === val3);
    let val5 = val1.flat(2);
    ASSERT_TRUE(val5[3] === 4);
    ASSERT_TRUE(val5[4] === 5);
}

function testConcat(): void {
    let val1 = STValue.newSTArray();
    val1.push(1);
    let val2 = STValue.newSTArray();
    val2.push(2, 3);
    let val3 = STValue.newSTArray();
    val3.push(4, 5);
    let val4 = val1.concat(val2, val3);
    ASSERT_TRUE(val1.toString() === '1');
    ASSERT_TRUE(val2.toString() === '2,3');
    ASSERT_TRUE(val3.toString() === '4,5');
    ASSERT_TRUE(val4.toString() === '1,2,3,4,5');
}

function testAt(): void {
    let val1 = STValue.newSTArray();
    val1.push(1);
    ASSERT_TRUE(val1.at(0) === 1);
    try {
        val1.at(1);
    } catch(e: Error) {
        ASSERT_TRUE(e.message === 'Invalid index');
    }
    try {
        val1.at(-1);
    } catch(e: Error) {
        ASSERT_TRUE(e.message === 'Invalid index');
    }
}

function getOrderArray(length: number): st.Array<number> {
    let val1 = STValue.newSTArray();
    for (let i = 0; i < length; i++) {
        val1.push(i + 1);
    }
    return val1;
}

function testCopyWithin(): void {
    let val1 = getOrderArray(5);
    val1.copyWithin(-2);
    ASSERT_TRUE(val1.toString() === '1,2,3,1,2');
    let val2 = getOrderArray(5);
    val2.copyWithin(0, 3);
    ASSERT_TRUE(val2.toString() === '4,5,3,4,5');
    let val3 = getOrderArray(5);
    val3.copyWithin(0, 3, 4);
    ASSERT_TRUE(val3.toString() === '4,2,3,4,5');
    let val4 = getOrderArray(5);
    val4.copyWithin(-2, -3, -1);
    ASSERT_TRUE(val4.toString() === '1,2,3,3,4');
}

function testFill(): void {
    let val1 = getOrderArray(5);
    val1.fill(6);
    ASSERT_TRUE(val1.toString() === '6,6,6,6,6');
    let val2 = getOrderArray(5);
    val2.fill(6, 2);
    ASSERT_TRUE(val2.toString() === '1,2,6,6,6');
    let val3 = getOrderArray(5);
    val3.fill(6, 1, 4);
    ASSERT_TRUE(val3.toString() === '1,6,6,6,5');
}

function testFind(): void {
    let val1 = getOrderArray(5);
    let result1 = val1.find((val) => { return val > 3;});
    ASSERT_TRUE(result1 === 4);
    let result2 = val1.find((val) => { return val > 5;});
    ASSERT_TRUE(result2 === undefined);
}

function testFindLast(): void {
    let val1 = getOrderArray(5);
    let result1 = val1.findLast((val) => { return val < 3;});
    ASSERT_TRUE(result1 === 2);
    let result2 = val1.findLast((val) => { return val < -1;});
    ASSERT_TRUE(result2 === undefined);
}

function testFindIndex(): void {
    let val1 = getOrderArray(5);
    let result1 = val1.findIndex((val) => { return val > 3;});
    ASSERT_TRUE(result1 === 3);
    let result2 = val1.findIndex((val) => { return val > 5;});
    ASSERT_TRUE(result2 === -1);
}

function testFindLastIndex(): void {
    let val1 = getOrderArray(5);
    let result1 = val1.findLastIndex((val) => { return val < 3;});
    ASSERT_TRUE(result1 === 1);
    let result2 = val1.findLastIndex((val) => { return val < -1;});
    ASSERT_TRUE(result2 === -1);
}

function testEvery(): void {
    let val1 = getOrderArray(5);
    let result1 = val1.every((val) => { return val < 3;});
    ASSERT_TRUE(result1 === false);
    let result2 = val1.every((val) => { return val < 6;});
    ASSERT_TRUE(result2);
}

function testSome(): void {
    let val1 = getOrderArray(5);
    let result1 = val1.some((val) => { return val < 3;});
    ASSERT_TRUE(result1);
    let result2 = val1.some((val) => { return val > 6;});
    ASSERT_TRUE(result2 === false);
}

function testReduce(): void {
    let val1 = getOrderArray(5);
    let result1 = val1.reduce((v1, v2) => { return v1 + v2;});
    ASSERT_TRUE(result1 === 15);
    let result2 = val1.reduce((v1, v2) => { return v1 + v2;}, 10);
    ASSERT_TRUE(result2 === 25);
}

function testRduceRight(): void {
    let val1 = getOrderArray(5);
    let result1 = val1.reduceRight((v1, v2) => { return v1 + v2;});
    ASSERT_TRUE(result1 === 15);
    let result2 = val1.reduceRight((v1, v2) => { return v1 + v2;}, 10);
    ASSERT_TRUE(result2 === 25);
}

function testForEach(): void {
    let val1 = getOrderArray(5);
    let total = 0;
    let index = 0;
    val1.forEach((val, idx, arr) => {
        total += val;
        ASSERT_TRUE(idx === index++);
        ASSERT_TRUE(arr[idx] === val);
    });
    ASSERT_TRUE(total === 15);
}

function testSlice(): void {
    let val1 = getOrderArray(5);
    let val2 = val1.slice(2);
    ASSERT_TRUE(val2.toString() === '3,4,5');
    let val3 = val1.slice(2, 4);
    ASSERT_TRUE(val3.toString() === '3,4');
    let val4 = val1.slice(1, 5);
    ASSERT_TRUE(val4.toString() === '2,3,4,5');
    let val5 = val1.slice(-2);
    ASSERT_TRUE(val5.toString() === '4,5');
    let val6 = val1.slice(2, -1);
    ASSERT_TRUE(val6.toString() === '3,4');
}

function testIndexOf(): void {
    let val1 = getOrderArray(5);
    val1.push(3);
    let index1 = val1.indexOf(3);
    ASSERT_TRUE(index1 === 2);
    let index2 = val1.indexOf(3, 3);
    ASSERT_TRUE(index2 === 5);
    let index3 = val1.indexOf(6);
    ASSERT_TRUE(index3 === -1);
}

function testLastIndexOf(): void {
    let val1 = getOrderArray(5);
    val1.push(3);
    let index1 = val1.lastIndexOf(3);
    ASSERT_TRUE(index1 === 5);
    let index2 = val1.lastIndexOf(3, 4);
    ASSERT_TRUE(index2 === 2);
    let index3 = val1.lastIndexOf(6);
    ASSERT_TRUE(index3 === -1);
}

function testIncludes(): void {
    let val1 = getOrderArray(5);
    let isIncludes1 = val1.includes(1);
    ASSERT_TRUE(isIncludes1);
    let isIncludes2 = val1.includes(1, 1);
    ASSERT_TRUE(isIncludes2 === false);
}

function testJoin(): void {
    let val1 = getOrderArray(5);
    let result1 = val1.join();
    ASSERT_TRUE(result1 === '1,2,3,4,5');
    let result2 = val1.join('$');
    ASSERT_TRUE(result2 === '1$2$3$4$5');
}

function testReverse(): void {
    let val1 = getOrderArray(5);
    let val2 = val1.reverse();
    ASSERT_TRUE(val2 === val1);
    ASSERT_TRUE(val2.toString() === '5,4,3,2,1');
}

function testToReversed(): void {
    let val1 = getOrderArray(5);
    let val2 = val1.toReversed();
    ASSERT_TRUE(val2 !== val1);
    ASSERT_TRUE(val1.toString() === '1,2,3,4,5');
    ASSERT_TRUE(val2.toString() === '5,4,3,2,1');
}

function testWith(): void {
    let val1 = getOrderArray(5);
    let val2 = val1.with(2, 6);
    ASSERT_TRUE(val2 !== val1);
    ASSERT_TRUE(val1.toString() === '1,2,3,4,5');
    ASSERT_TRUE(val2.toString() === '1,2,6,4,5');
}

function testMap(): void {
    let val1 = getOrderArray(5);
    let val2 = val1.map((val) => {return val;});
    ASSERT_TRUE(val2 !== val1);
    ASSERT_TRUE(val1.toString() === '1,2,3,4,5');
    ASSERT_TRUE(val2.toString() === '1,2,3,4,5');
}

function consumeSTArray(val: st.Array<string>): void {
    ASSERT_TRUE(val.toString() === '1,2,3,4,5');
}

function testEsValueOnSTArray(): void {
    esFuncArrayTest(consumeSTArray, getOrderArray(5));
}

function testIsSTArray(): void {
    let arr = STValue.newSTArray();
    ASSERT_TRUE(STValue.isSTArray(arr));

    let constArr = getConstArray();
    ASSERT_TRUE(STValue.isSTArray(constArr));

    let customArr = customLengthArray(5);
    ASSERT_TRUE(STValue.isSTArray(customArr));

    let num = STValue.wrapNumber(42);
    ASSERT_TRUE(!STValue.isSTArray(num));

    let str = STValue.wrapString('test');
    ASSERT_TRUE(!STValue.isSTArray(str));

    let nul = STValue.getNull();
    ASSERT_TRUE(!STValue.isSTArray(nul));

    let und = STValue.getUndefined();
    ASSERT_TRUE(!STValue.isSTArray(und));
}

function testIsSTSet(): void {
    let set = STValue.newSTSet();
    ASSERT_TRUE(STValue.isSTSet(set));

    let num = STValue.wrapNumber(42);
    ASSERT_TRUE(!STValue.isSTSet(num));

    let str = STValue.wrapString('test');
    ASSERT_TRUE(!STValue.isSTSet(str));

    let nul = STValue.getNull();
    ASSERT_TRUE(!STValue.isSTSet(nul));

    let und = STValue.getUndefined();
    ASSERT_TRUE(!STValue.isSTSet(und));
}

function testIsSTMap(): void {
    let map = STValue.newSTMap();
    ASSERT_TRUE(STValue.isSTMap(map));

    let num = STValue.wrapNumber(42);
    ASSERT_TRUE(!STValue.isSTMap(num));

    let str = STValue.wrapString('test');
    ASSERT_TRUE(!STValue.isSTMap(str));

    let nul = STValue.getNull();
    ASSERT_TRUE(!STValue.isSTMap(nul));

    let und = STValue.getUndefined();
    ASSERT_TRUE(!STValue.isSTMap(und));
}

testSameObject();
testInstatnceof();
testGetSetLength();
testGetterSetterByIndex();
testShiftUnshift();
testPushPop();
testToString();
testSplice();
testToSpliced();
testKeysValuesEntries();
testForOf();
testReferenceArray();
testFilter();
testFlat();
testConcat();
testAt();
testCopyWithin();
testFill();
testFind();
testFindLast();
testFindIndex();
testFindLastIndex();
testEvery();
testSome();
testReduce();
testRduceRight();
testForEach();
testSlice();
testIndexOf();
testLastIndexOf();
testIncludes();
testJoin();
testReverse();
testToReversed();
testWith();
testMap();
testEsValueOnSTArray();
testIsSTArray();
testIsSTSet();
testIsSTMap();