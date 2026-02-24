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

function testCompareExchange() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        arktest.assertEQ(ta.byteOffset, 0)
        let arraySize = ta.length

        for (let j = 1; j < arraySize; j++) {
            Atomics.store(ta, j, 7)
            let old = Atomics.compareExchange(ta, j, 7, 12)
            arktest.assertEQ(old, 7)

            for (let k = 0; k < arraySize; k++) {
                let re = (k === j) ? 12 : 0
                arktest.assertEQ(Atomics.load(ta, k), re)
            }

            Atomics.store(ta, j, 0)
        }
    }
}

function testUnsignedCompareExchangeOverflow() {
    const U8VAL: long = 150;
    Atomics.store(uint8array, 0, U8VAL);
    let storedVal = Atomics.compareExchange(uint8array, 0, U8VAL, U8VAL);
    arktest.assertEQ(storedVal, U8VAL);
    arktest.assertEQ(uint8array[0], U8VAL);

    const U16VAL: long = 32918;
    Atomics.store(uint16array, 0, U16VAL);
    storedVal = Atomics.compareExchange(uint16array, 0, U16VAL, U16VAL);
    arktest.assertEQ(storedVal, U16VAL);
    arktest.assertEQ(uint16array[0], U16VAL);

    const U32VAL: long = 2147483798;
    Atomics.store(uint32array, 0, U32VAL);
    storedVal = Atomics.compareExchange(uint32array, 0, U32VAL, U32VAL);
    arktest.assertEQ(storedVal, U32VAL);
    arktest.assertEQ(uint32array[0], U32VAL);
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics base_operation_compare_exchange tests')
    suite.addTest('Function testCompareExchange', testCompareExchange)
    suite.addTest('Function testUnsignedCompareExchangeOverflow', testUnsignedCompareExchangeOverflow)
    return suite.run()
}