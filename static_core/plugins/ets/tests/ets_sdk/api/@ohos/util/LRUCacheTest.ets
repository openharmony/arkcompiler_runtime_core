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

    suite.addTest("LRUCache Constructor No Parameter", testLRUCacheConstructorNoParameter);
    suite.addTest("LRUCache Constructor Int", testLRUCacheConstructorInt);
    suite.addTest("LRUCache Constructor x.0", testLRUCacheConstructorDotZero);
    suite.addTest("LRUCache update&getCapacity Int", testLRUCacheUpdateGetCapacityInt);
    suite.addTest("LRUCache update&getCapacity x.0", testLRUCacheUpdateGetCapacityDotZero);
    suite.addTest("LRUCache Get Int", testLRUCacheGetInt);
    suite.addTest("LRUCache Get Str&Undefined", testLRUCacheGetStrAndUndefined);
    suite.addTest("LRUCache Get CapacityChenged", testLRUCacheGetCapacityChenged);
    suite.addTest("LRUCache Put Once", testLRUCachePutOnce);
    suite.addTest("LRUCache Put Twice", testLRUCachePutTwice);
    suite.addTest("LRUCache Put&Get", testLRUCachePutAndGet);
    suite.addTest("LRUCache Put CapacityChenged", testLRUCachePutCapacityChenged);
    suite.addTest("LRUCache RemoveOnce", testLRUCacheRemoveOnce);
    suite.addTest("LRUCache RemoveTwice", testLRUCacheRemoveTwice);
    suite.addTest("LRUCache RemoveUndefined", testLRUCacheRemoveUndefined);
    suite.addTest("LRUCache Remove&Get", testLRUCacheRemoveAndGet);
    suite.addTest("LRUCache Clear One", testLRUCacheClearOne);
    suite.addTest("LRUCache Clear Multiple", testLRUCacheClearMulti);
    suite.addTest("LRUCache Contains True", testLRUCacheContainsTrue);
    suite.addTest("LRUCache Contains False", testLRUCacheContainsFalse);
    suite.addTest("LRUCache Contains Mixed", testLRUCacheContainsMixed);
    suite.addTest("LRUCache ToString Put", testLRUCacheToStringPut);
    suite.addTest("LRUCache ToString Hit Once", testLRUCacheToStringHitOnce);
    suite.addTest("LRUCache ToString HitAll", testLRUCacheToStringHitAll);
    suite.addTest("LRUCache ToString Hit&Miss", testLRUCacheToStringHitAndMiss);
    suite.addTest("LRUCache ToString Mixed", testLRUCacheToStringMixed);

    return suite.run()
}

function testLRUCacheUpdateGetCapacityInt() {
    let that = new util.LRUCache<number, number>();
    that.updateCapacity(10);
    let res1: number = that.getCapacity();
    assertEQ(res1, 10);
    that.updateCapacity(2147483646);
    let res2: number = that.getCapacity();
    assertEQ(res2, 2147483646);
}

function testLRUCacheUpdateGetCapacityDotZero() {
    let that = new util.LRUCache<number, number>(20);
    that.updateCapacity(10.0);
    let res: number = that.getCapacity();
    assertEQ(res, 10);
}

function testLRUCacheGetInt() {
    let that = new util.LRUCache<number, number>();
    that.put(1,2);
    let res = that.get(1);
    assertEQ(res, 2);
}

function testLRUCacheGetStrAndUndefined() {
    let that = new util.LRUCache<number|string, number|string>(100);
    that.put(1,2)
    that.put(2,10)
    that.put('abcd',15)
    let res1 = that.get(20)
    let res2 = that.get('abcd')
    let res3 = that.get(2)
    assertEQ(res1, undefined);
    assertEQ(res2, 15);
    assertEQ(res3, 10);
}

function testLRUCacheGetCapacityChenged() {
    let that = new util.LRUCache<number|string, number|string>(100);
    that.put('1111','bcjdshc')
    that.put(20,'cdjcaxb')
    that.updateCapacity(2)
    that.put('abcd',15)
    let res1 = that.get('1111')
    let res2 = that.get('abcd')
    assertEQ(res1, undefined);
    assertEQ(res2, 15);
}

function testLRUCachePutOnce() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('1111', 'bcjdshc');
    let res1 = that.get('1111');
    assertEQ(res1, 'bcjdshc');
}

function testLRUCachePutTwice() {
    let that = new util.LRUCache<string|number, string|number>();
    let temp = that.put('1111', 'bcjdshc');
    let temp2 = that.put('1111', 22);
    assertEQ(temp, undefined);
    assertEQ(temp2, 'bcjdshc');
}

function testLRUCachePutAndGet() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('1111','bcjdshc');
    that.put(1,12);
    that.put(2,5);
    let temp = that.put(1,10);
    let temp2 = that.put(2,22);
    let temp3 = that.put(2,30);
    let res1 = that.get(1);
    let res2 = that.get(2);
    assertEQ(temp, 12);
    assertEQ(temp2, 5);
    assertEQ(temp3, 22);
    assertEQ(res1, 10);
    assertEQ(res2, 30);
}

function testLRUCachePutCapacityChenged() {
    let that = new util.LRUCache<string|number|number, string|number>();
    that.put('1111','bcjdshc');
    that.put(1,12);
    that.put(2,5);
    that.updateCapacity(2);
    that.put(2,'adasfdad');
    that.put(3,'a');
    let res1 = that.get(1);
    assertEQ(res1, undefined);
}

function testLRUCacheClearOne() {
    let that = new util.LRUCache<number, number>();
    that.put(1,2);
    let result1 = that.length;
    that.clear();
    let result2 = that.length;
    assertEQ(result1, 1);
    assertEQ(result2, 0);
}

function testLRUCacheClearMulti() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2)
    that.put(2,10)
    that.put(5,33);
    that.put('abcd','string');
    that.updateCapacity(3);
    let result1 = that.length;
    that.clear();
    let result2 = that.length;
    assertEQ(result1, 3);
    assertEQ(result2, 0);
}

function testLRUCacheContainsTrue() {
    let that = new util.LRUCache<number, number>();
    that.put(1,2);
    let result1 = that.contains(1);
    assertEQ(result1, true);
}

function testLRUCacheContainsFalse() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('abc','abc');
    let result1 = that.contains(1);
    assertEQ(result1, false);
}

function testLRUCacheContainsMixed() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2)
    that.put('string','string')
    let result1 = that.contains(1)
    let result2 = that.contains('string')
    let result3 = that.contains(0)
    assertEQ(result1, true);
    assertEQ(result2, true);
    assertEQ(result3, false);
}

function testLRUCacheRemoveOnce() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    let res1 = that.remove(1);
    assertEQ(res1, 2);
}

function testLRUCacheRemoveTwice() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    that.put('abcd',20);
    let res1 = that.remove(1);
    let res2 = that.remove('abcd');
    assertEQ(res1, 2);
    assertEQ(res2, 20);
}

function testLRUCacheRemoveUndefined() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    let res1 = that.remove(1);
    let res2 = that.remove(5);
    assertEQ(res1, 2);
    assertEQ(res2, undefined);
}

function testLRUCacheRemoveAndGet() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2);
    that.put(3,10);
    that.put(5,15);
    let res1 = that.remove(3);
    let res2 = that.get(3);
    let res3 = that.remove(3);
    assertEQ(res1, 10);
    assertEQ(res2, undefined);
    assertEQ(res3, undefined);
}

function testLRUCacheToStringPut() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('1111','bcjdshc')
    that.put(1,2)
    let result = that.toString();
    assertEQ(result, 'LRUCache[ maxSize = 64, hits = 0, misses = 0, hitRate = 0% ]');
}

function testLRUCacheToStringHitAll() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put('1111','bcjdshc')
    that.put(1,2)
    that.get(1);
    that.get('1111');
    let result = that.toString();
    assertEQ(result, 'LRUCache[ maxSize = 64, hits = 2, misses = 0, hitRate = 100% ]');
}

function testLRUCacheToStringHitOnce() {
    let that = new util.LRUCache<string|number, string|number>(100);
    that.put(1,2)
    that.put(3,10)
    that.put(5,15)
    that.put('abc',20)
    that.get(3)
    let result = that.toString();
    assertEQ(result, 'LRUCache[ maxSize = 100, hits = 1, misses = 0, hitRate = 100% ]');
}

function testLRUCacheToStringHitAndMiss() {
    let that = new util.LRUCache<string|number, string|number>();
    that.put(1,2)
    that.put(3,10)
    that.put(5,15)
    that.put('abc',20)
    that.get(3)
    that.get(2);
    let result = that.toString();
    assertEQ(result, 'LRUCache[ maxSize = 64, hits = 1, misses = 1, hitRate = 50% ]');
}

function testLRUCacheToStringMixed() {
    let that = new util.LRUCache<string|number, string|number>(123456);
    that.put(1,2);
    that.put(3,10);
    that.put(5,15);
    that.put('abc',20);
    that.get(3);
    that.get(2);
    that.get(1);
    that.contains(5);
    let result = that.toString();
    assertEQ(result, 'LRUCache[ maxSize = 123456, hits = 3, misses = 1, hitRate = 75% ]');
}

function testLRUCacheConstructorNoParameter() {
    let that = new util.LRUCache<number, number>();
    let res: number = that.getCapacity();
    assertEQ(res, 64);
}

function testLRUCacheConstructorInt() {
    let that = new util.LRUCache<number, number>(2147483646);
    let res: number = that.getCapacity();
    assertEQ(res, 2147483646);
}

function testLRUCacheConstructorDotZero() {
    let that = new util.LRUCache<number, number>(21.0);
    let res: number = that.getCapacity();
    assertEQ(res, 21);
}
