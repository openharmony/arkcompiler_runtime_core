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
import { BusinessError } from "@ohos.base";

const success = 0
const fail = 1

function main(): int {
    const suite = new ArkTestsuite("LRUCache API Exception tests")

    suite.addTest("LRUCache Constructor Long", testLRUCacheConstructor_Long);
    suite.addTest("LRUCache Constructor Decimal", testLRUCacheConstructor_Decimal);
    suite.addTest("LRUCache Constructor Zero", testLRUCacheConstructor_Zero);
    suite.addTest("LRUCache Constructor Negative", testLRUCacheConstructor_Negative);
    suite.addTest("LRUCache update&getCapacity Long", testLRUCacheUpdateGetCapacity_Long);
    suite.addTest("LRUCache update&getCapacity Decimal", testLRUCacheUpdateGetCapacity_Decimal);
    suite.addTest("LRUCache update&getCapacity Zero", testLRUCacheUpdateGetCapacity_Zero);
    suite.addTest("LRUCache update&getCapacity Negative", testLRUCacheUpdateGetCapacity_Negative);
    suite.addTest("LRUCache Constructor NumberMin", testLRUCacheConstructor_NumberMin);
    suite.addTest("LRUCache update&getCapacity NumberMin", testLRUCacheUpdateGetCapacity_NumberMin);

    return suite.run()
}

function testLRUCacheUpdateGetCapacity_Long() {
    let that = new util.LRUCache<number, number>();
    try {
        that.updateCapacity(2147483648);
    } catch (err: BusinessError) {
        assertEQ(err.code, 401, "LRUCache should throw an exception when capacity is not int")
        return
    }
    assertTrue(false, "LRUCache should throw an exception when capacity is not int");
}

function testLRUCacheUpdateGetCapacity_Decimal() {
    let that = new util.LRUCache<number, number>();
    try {
        that.updateCapacity(2.5);
    } catch (err: BusinessError) {
        assertEQ(err.code, 401, "LRUCache should throw an exception when capacity is not int")
        return
    }
    assertTrue(false, "LRUCache should throw an exception when capacity is not int");
}

function testLRUCacheUpdateGetCapacity_Zero() {
    let that = new util.LRUCache<number, number>();
    try {
        that.updateCapacity(0);
    } catch (err: BusinessError) {
        assertEQ(err.code, 401, "LRUCache should throw an exception when capacity is not int")
        return
    }
    assertTrue(false, "LRUCache should throw an exception when capacity is not int");
}

function testLRUCacheUpdateGetCapacity_Negative() {
    let that = new util.LRUCache<number, number>();
    try {
        that.updateCapacity(-1);
    } catch (err: BusinessError) {
        assertEQ(err.code, 401, "LRUCache should throw an exception when capacity is negative")
        return
    }
    assertTrue(false, "LRUCache should throw an exception when capacity is negative");
}

function testLRUCacheConstructor_Long() {
    try {
        let that = new util.LRUCache<number, number>(2147483648);
    } catch (err: BusinessError) {
        assertEQ(err.code, 401, "LRUCache should throw an exception when capacity is not int")
        return
    }
    assertTrue(false, "LRUCache should throw an exception when capacity is not int");
}

function testLRUCacheConstructor_Decimal() {
    try {
        let that = new util.LRUCache<number, number>(0.1);
    } catch (err: BusinessError) {
        assertEQ(err.code, 401, "LRUCache should throw an exception when capacity is not int")
        return
    }
    assertTrue(false, "LRUCache should throw an exception when capacity is not int");
}

function testLRUCacheConstructor_Zero() {
    try {
        let that = new util.LRUCache<number, number>(0);
    } catch (err: BusinessError) {
        assertEQ(err.code, 401, "LRUCache should throw an exception when capacity is not int")
        return
    }
}

function testLRUCacheConstructor_Negative() {
    try {
        let that = new util.LRUCache<number, number>(-1);
    } catch (err: BusinessError) {
        assertEQ(err.code, 401, "LRUCache should throw an exception when capacity is negative")
        return
    }
    assertTrue(false, "LRUCache should throw an exception when capacity is negative");
}

function testLRUCacheConstructor_NumberMin() {
    try {
        let that = new util.LRUCache<number, number>(Double.MIN_VALUE);
    } catch (err: BusinessError) {
        assertEQ(err.code, 401, "LRUCache should throw an exception when capacity is not int")
        return
    }
    assertTrue(false, "LRUCache should throw an exception when capacity is not int");
}

function testLRUCacheUpdateGetCapacity_NumberMin() {
    let that = new util.LRUCache<number, number>();
    try {
        that.updateCapacity(Double.MIN_VALUE);
    } catch (err: BusinessError) {
        assertEQ(err.code, 401, "LRUCache should throw an exception when capacity is not int")
        return
    }
    assertTrue(false, "LRUCache should throw an exception when capacity is not int");
}
