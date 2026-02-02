/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

const BYTE_LENGTH = 128

let abint8 = new ArrayBuffer(BYTE_LENGTH)
let int8array = new Int8Array(abint8)
let abint16 = new ArrayBuffer(BYTE_LENGTH)
let int16array = new Int16Array(abint16)
let abint32 = new ArrayBuffer(BYTE_LENGTH)
let int32array = new Int32Array(abint32)
let abuint8 = new ArrayBuffer(BYTE_LENGTH)
let uint8array = new Uint8Array(abuint8)
let abuint16 = new ArrayBuffer(BYTE_LENGTH)
let uint16array = new Uint16Array(abuint16)
let abuint32 = new ArrayBuffer(BYTE_LENGTH)
let uint32array = new Uint32Array(abuint32)

let typeArrayList = [
    int8array,
    int16array,
    int32array,
    uint8array,
    uint16array,
    uint32array
]
const expectedAtomicsRangeError = (e: Error): boolean => {
    return e instanceof RangeError &&
        e.code == 401 &&
        e.message == "Atomics:: Index out of bounds"
}

function testAdd() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        arktest.assertEQ(ta.byteOffset, 0)
        let arraySize = ta.length
        arktest.expectThrow(() => { Atomics.add(ta, -1, 1); }, expectedAtomicsRangeError)
        arktest.expectThrow(() => { Atomics.add(ta, ta.length, 1); }, expectedAtomicsRangeError)
    }
}

function testAnd() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        arktest.assertEQ(ta.byteOffset, 0)
        let arraySize = ta.length
        arktest.expectThrow(() => { Atomics.and(ta, -1, 1); }, expectedAtomicsRangeError)
        arktest.expectThrow(() => { Atomics.and(ta, ta.length, 1); }, expectedAtomicsRangeError)
    }
}

function testCompareExchange() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        arktest.assertEQ(ta.byteOffset, 0)
        let arraySize = ta.length
        arktest.expectThrow(() => { Atomics.compareExchange(ta, -1, 1, 1); }, expectedAtomicsRangeError)
        arktest.expectThrow(() => { Atomics.compareExchange(ta, ta.length, 1, 1); }, expectedAtomicsRangeError)
    }
}

function testExchange() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        arktest.assertEQ(ta.byteOffset, 0)
        let arraySize = ta.length
        arktest.expectThrow(() => { Atomics.exchange(ta, -1, 1); }, expectedAtomicsRangeError)
        arktest.expectThrow(() => { Atomics.exchange(ta, ta.length, 1); }, expectedAtomicsRangeError)
    }
}

function testLoad() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        arktest.assertEQ(ta.byteOffset, 0)
        let arraySize = ta.length
        arktest.expectThrow(() => { Atomics.load(ta, -1); }, expectedAtomicsRangeError)
        arktest.expectThrow(() => { Atomics.load(ta, ta.length); }, expectedAtomicsRangeError)
    }
}

function testOr() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        arktest.assertEQ(ta.byteOffset, 0)
        let arraySize = ta.length
        arktest.expectThrow(() => { Atomics.or(ta, -1, 1); }, expectedAtomicsRangeError)
        arktest.expectThrow(() => { Atomics.or(ta, ta.length, 1); }, expectedAtomicsRangeError)
    }
}

function testStore() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        arktest.assertEQ(ta.byteOffset, 0)
        let arraySize = ta.length
        arktest.expectThrow(() => { Atomics.store(ta, -1, 1); }, expectedAtomicsRangeError)
        arktest.expectThrow(() => { Atomics.store(ta, ta.length, 1); }, expectedAtomicsRangeError)
    }
}

function testSub() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        arktest.assertEQ(ta.byteOffset, 0)
        let arraySize = ta.length
        arktest.expectThrow(() => { Atomics.sub(ta, -1, 1); }, expectedAtomicsRangeError)
        arktest.expectThrow(() => { Atomics.sub(ta, ta.length, 1); }, expectedAtomicsRangeError)
    }
}

function testXor() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        arktest.assertEQ(ta.byteOffset, 0)
        let arraySize = ta.length
        arktest.expectThrow(() => { Atomics.xor(ta, -1, 1); }, expectedAtomicsRangeError)
        arktest.expectThrow(() => { Atomics.xor(ta, ta.length, 1); }, expectedAtomicsRangeError)
    }
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics index_range tests')
    suite.addTest('Function testAdd', testAdd)
    suite.addTest('Function testAnd', testAnd)
    suite.addTest('Function testCompareExchange', testCompareExchange)
    suite.addTest('Function testExchange', testExchange)
    suite.addTest('Function testLoad', testLoad)
    suite.addTest('Function testOr', testOr)
    suite.addTest('Function testStore', testStore)
    suite.addTest('Function testSub', testSub)
    suite.addTest('Function testXor', testXor)
    return suite.run()
}
