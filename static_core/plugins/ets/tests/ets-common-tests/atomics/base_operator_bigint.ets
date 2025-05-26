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

const BYTE_LENGTH = 128

let ab = new ArrayBuffer(BYTE_LENGTH)
let bigint64array = new BigInt64Array(ab)

function testAddBig() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        let old = Atomics.add(ta, j, 12n)
        assertEQ(old, 0n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 12n : 0n
            assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testAndBig() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 3n)

        let old = Atomics.and(ta, j, 1n)
        assertEQ(old, 3n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 1n : 0n
            assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testCompareExchangeBig() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 7n)
        let old = Atomics.compareExchange(ta, j, 7n, 12n)
        assertEQ(old, 7n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 12n : 0n
            assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testExchangeBig() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        let old = Atomics.exchange(ta, j, 12n)
        assertEQ(old, 0n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 12n : 0n
            assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testLoadBig() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 12n)

        for (let k = 0; k < arraySize; k++) {
            let expected = (k === j) ? 12n : 0n
            assertEQ(Atomics.load(ta as BigInt64Array, k), expected)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testOrBig() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 2n)

        let old = Atomics.or(ta, j, 1n)
        assertEQ(old, 2n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 3n : 0n
            assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testStoreBig() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        let re = Atomics.store(ta, j, 12n)
        assertEQ(re, 12n)

        for (let k = 0; k < arraySize; k++) {
            let expected = (k === j) ? 12n : 0n
            assertEQ(Atomics.load(ta as BigInt64Array, k), expected)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testSubBig() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 48n)

        let old = Atomics.sub(ta, j, 12n)
        assertEQ(old, 48n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 36n : 0n
            assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testXorBig() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 5n)

        let old = Atomics.xor(ta, j, 1n)
        assertEQ(old, 5n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 4n : 0n
            assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function main() {
    const suite = new ArkTestsuite('Function Atomics baseOperator')
    suite.addTest('Function Add', testAddBig)
    suite.addTest('Function And', testAndBig)
    suite.addTest('Function testCompareExchange', testCompareExchangeBig)
    suite.addTest('Function testExchange', testExchangeBig)
    suite.addTest('Function testLoad', testLoadBig)
    suite.addTest('Function testOr', testOrBig)
    suite.addTest('Function testStore', testStoreBig)
    suite.addTest('Function testSub', testSubBig)
    suite.addTest('Function testXor', testXorBig)
    return suite.run()
}