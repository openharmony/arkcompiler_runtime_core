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

import {util} from "@ohos.util"

const success = 0
const fail = 1

function main(): int {
    const suite = new ArkTestsuite("LRUCache API tests")

    suite.addTest("LRUCache Length Zero", testLRUCacheLengthZero);
    suite.addTest("LRUCache Length One", testLRUCacheLengthOne);
    suite.addTest("LRUCache Length Multiple", testLRUCacheLengthMulti);
    suite.addTest("LRUCache Length Cover", testLRUCacheLengthCover);
    suite.addTest("LRUCache Length OutofCapacity", testLRUCacheLengthCapacity);
    suite.addTest("LRUCache IsEmpty False", testLRUCacheIsEmptyFalse);
    suite.addTest("LRUCache IsEmpty True", testLRUCacheIsEmptyTrue);
    suite.addTest("LRUCache Keys Once", testLRUCacheKeysOnce);
    suite.addTest("LRUCache Keys Multiple", testLRUCacheKeysMulti);
    suite.addTest("LRUCache Keys Get&Contains", testLRUCacheKeysGetAndContains);
    suite.addTest("LRUCache Keys CapacityChenged", testLRUCacheKeysCapacityChenged);
    suite.addTest("LRUCache Values Once", testLRUCacheValuesOnce);
    suite.addTest("LRUCache Values Multiple", testLRUCacheValuesMulti);
    suite.addTest("LRUCache Values Get&Contains", testLRUCacheValuesGetAndContains);
    suite.addTest("LRUCache Values CapacityChenged", testLRUCacheValuesCapacityChenged);
    suite.addTest("LRUCache Entries&Iterator", testLRUCacheEntriesIterator);

    return suite.run()
}

function testLRUCacheKeysOnce() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('string','abc');
    let result = that.keys();
    assertEQ(result[0], 'string');
}

function testLRUCacheKeysMulti() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2)
    that.put(3,10)
    that.put(5,15)
    that.put('abc',20);
    let result = that.keys();
    assertEQ(result[0], 1);
    assertEQ(result[1], 3);
    assertEQ(result[2], 5);
    assertEQ(result[3], 'abc');
}

function testLRUCacheKeysGetAndContains() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    that.put(3,10);
    that.put(5,15);
    that.put('abc',20);
    that.get(3);
    that.contains(1);
    let result = that.keys();
    assertEQ(result[0], 5);
    assertEQ(result[1], 'abc');
    assertEQ(result[2], 3);
    assertEQ(result[3], 1);
}

function testLRUCacheKeysCapacityChenged() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2)
    that.put(3,10)
    that.put(5,15)
    that.put('abc',20)
    that.get(3)
    that.remove(5)
    that.contains(3)
    that.put(1,11)
    that.contains('abc')
    let result = that.keys();
    assertEQ(result[0], 3);
    assertEQ(result[1], 1);
    assertEQ(result[2], 'abc');
}

function testLRUCacheValuesOnce() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('string','abc');
    let result = that.values();
    assertEQ(result[0], 'abc');
}

function testLRUCacheValuesMulti() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2)
    that.put(3,10)
    that.put(5,15)
    that.put('abc',20);
    let result = that.values();
    assertEQ(result[0], 2);
    assertEQ(result[1], 10);
    assertEQ(result[2], 15);
    assertEQ(result[3], 20);
}

function testLRUCacheValuesGetAndContains() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    that.put(3,10);
    that.put(5,15);
    that.put('abc',20);
    that.get(3);
    that.contains(1);
    let result = that.values();
    assertEQ(result[0], 15);
    assertEQ(result[1], 20);
    assertEQ(result[2], 10);
    assertEQ(result[3], 2);
}

function testLRUCacheValuesCapacityChenged() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2)
    that.put(3,10)
    that.put(5,15)
    that.put('abc',20)
    that.get(3)
    that.remove(5)
    that.contains(3)
    that.put(1,11)
    that.contains('abc')
    let result = that.values();
    assertEQ(result[0], 10);
    assertEQ(result[1], 11);
    assertEQ(result[2], 20);
}

function testLRUCacheEntriesIterator() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    that.put(3,10);
    that.put(5,15);
    that.put('abc',20);
    that.get(3);
    that.remove(5);
    that.contains(3);
    that.get(1);
    that.contains('abc');
    let i = 0;
    let j = 0;
    let arr = new Array<Object|undefined|null>();
    let arr2 = new Array<Object|undefined|null>();
    for (let entry of that.entries()) {
        arr.push(entry[0]);
        i++;
        arr.push(entry[1]);
        i++;
    }
    for (let entry of that) {
        arr2.push(entry[0]);
        j++;
        arr2.push(entry[1]);
        j++;
    }
    assertEQ(arr[1], 10);
    assertEQ(arr[2], 1);
    assertEQ(arr[3], 2);
    assertEQ(arr[4], 'abc');
    assertEQ(arr[5], 20);
    assertEQ(i, j);
    for (let k=0; k<i; k++) {
        assertEQ(arr[k], arr2[k]);
    }
}

function testLRUCacheLengthZero() {
    let that = new util.LRUCache<string|number, string|number>();
    let result = that.length;
    assertEQ(result, 0);
}

function testLRUCacheLengthOne() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    let result = that.length;
    assertEQ(result, 1);
}

function testLRUCacheLengthMulti() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    that.put('1','2');
    that.put('a',0xa);
    let result = that.length;
    assertEQ(result, 3);
}

function testLRUCacheLengthCover() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2)
    that.put(2,10)
    that.put('abcd',15)
    that.put('abcd',20);
    let result = that.length;
    assertEQ(result, 3);
}

function testLRUCacheLengthCapacity() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    that.put(2,10);
    that.put('abcd',15);
    that.put('abcd',20);
    that.updateCapacity(3);
    that.put(5,33);
    that.put(25,35);
    let result = that.length;
    assertEQ(result, 3);
}

function testLRUCacheIsEmptyFalse() {
    let that = new util.LRUCache<number, number>();
    that.put(1, 2);
    that.put(2, 4);
    that.put(3, 8);
    let result = that.isEmpty();
    assertEQ(result, false);
}

function testLRUCacheIsEmptyTrue() {
    let that = new util.LRUCache<number, number>();
    that.put(10, 20);
    let result1 = that.isEmpty();
    that.clear();
    let result2 = that.isEmpty();
    assertEQ(result1, false);
    assertEQ(result2, true);
}
