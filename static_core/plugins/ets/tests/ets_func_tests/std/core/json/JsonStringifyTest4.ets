/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.stringify primitives, objects, and wrappers')
    suite.addTest('byte toString matches', byteToString)
    suite.addTest('short toString matches', shortToString)
    suite.addTest('int toString matches', intToString)
    suite.addTest('BigInt toString matches', bigIntToString)
    return suite.run()
}

function byteToString(): void {
    const bytes: FixedArray<byte> = [
        -128, -127, -126, -125, -124, -123, -122, -121, -120, 50, 55, 60, 65, 70, 75, 80, -80, -75, -70, -65, -60, -55,
        -50, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    ]
    for (const value of bytes) {
        arktest.assertEQ(JSON.stringify(value), value.toString())
    }
}

function shortToString(): void {
    const shorts: FixedArray<short> = [
        -32768, -32767, -32766, -32765, -32764, -32763, -32762, -32761, -32760, 10000, 12000, 14000, 16000, 18000,
        20000, -20000, -18000, -16000, -14000, -12000, -10000, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
        7, 8, 9,
    ]
    for (const value of shorts) {
        arktest.assertEQ(JSON.stringify(value), value.toString())
    }
}

function intToString(): void {
    const ints: FixedArray<int> = [
        -2147483648, -2147483647, -2147483646, -2147483645, -2147483644, 1000000, 500000000, 1000000000, 1500000000,
        2000000000, -2000000000, -1500000000, -1000000000, -500000000, -1000000, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 2147483647, 2147483646, 2147483645, 2147483644, 2147483643,
    ]
    for (const value of ints) {
        arktest.assertEQ(JSON.stringify(value), value.toString())
    }
}

function bigIntToString(): void {
    const bigInts: FixedArray<BigInt> = [
        -9223372036854775808n, -9223372036854775807n, -9223372036854775806n, -9223372036854775805n,
        -9223372036854775804n, 1000000000000n, 10000000000000n, 100000000000000n, 1000000000000000n, 10000000000000000n,
        100000000000000000n, -100000000000000000n, -10000000000000000n, -1000000000000000n, -100000000000000n,
        -10000000000000n, -1000000000000n, - 9n, -8n, -7n, -6n, -5n, -4n, -3n, -2n, -1n, 0n, 1n, 2n, 3n, 4n, 5n, 6n, 7n,
        8n, 9n, 9223372036854775807n, 9223372036854775806n, 9223372036854775805n, 9223372036854775804n,
        9223372036854775803n,
    ]
    for (const value of bigInts) {
        arktest.assertEQ(JSON.stringify(value), value.toString())
    }
}
