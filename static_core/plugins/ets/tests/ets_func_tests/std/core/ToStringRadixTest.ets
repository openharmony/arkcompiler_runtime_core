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

function main() {
    let suite = new arktest.ArkTestsuite('toString(radix) tests')
    
    suite.addTest('Char.toString(radix) tests', Char_toString_tests)
    suite.addTest('Byte.toString(radix) tests', Byte_toString_tests)
    suite.addTest('Short.toString(radix) tests', Short_toString_tests)
    suite.addTest('Int.toString(radix) tests', Int_toString_tests)
    suite.addTest('Long.toString(radix) tests', Long_toString_tests)
    
    suite.run()
}

interface CharTestCase {
    value: char
    radix: int
    expected: string
}

function Char_toString_tests() {
    const testCases: CharTestCase[] = [
        { value: c'\n', radix: 2, expected: '1010' },
        { value: c'd', radix: 8, expected: '144' },
        { value: c'A', radix: 10, expected: '65' },
        { value: c'ÿ', radix: 16, expected: 'ff' },
        { value: c'#', radix: 36, expected: 'z' },
    ]
    
    for (let tc: CharTestCase of testCases) {
        const result = tc.value.toString(tc.radix)
        arktest.assertEQ(result, tc.expected,
            `Char(${tc.value}).toString(${tc.radix}) should be '${tc.expected}', got '${result}'`)
    }
}

interface ByteTestCase {
    value: byte
    radix: int
    expected: string
}

function Byte_toString_tests() {
    const testCases: ByteTestCase[] = [
        { value: 64, radix: 16, expected: '40' },
        { value: 127, radix: 10, expected: '127' },
        { value: 42, radix: 2, expected: '101010' },
        { value: 100, radix: 8, expected: '144' },
        { value: 35, radix: 36, expected: 'z' },
        { value: 0, radix: 10, expected: '0' },
        { value: 1, radix: 36, expected: '1' },
        { value: -128, radix: 10, expected: '-128' },
        { value: -35, radix: 36, expected: '-z' },
        { value: -1, radix: 10, expected: '-1' }
    ]
    
    for (let tc: ByteTestCase of testCases) {
        const result = tc.value.toString(tc.radix)
        arktest.assertEQ(result, tc.expected,
            `Byte(${tc.value}).toString(${tc.radix}) should be '${tc.expected}', got '${result}'`)
    }
}

interface ShortTestCase {
    value: short
    radix: int
    expected: string
}

function Short_toString_tests() {
    const testCases: ShortTestCase[] = [
        { value: 32767, radix: 16, expected: '7fff' },
        { value: 1024, radix: 2, expected: '10000000000' },
        { value: 4096, radix: 8, expected: '10000' },
        { value: 1295, radix: 36, expected: 'zz' },
        { value: 0, radix: 10, expected: '0' },
        { value: 35, radix: 36, expected: 'z' },
        { value: -32768, radix: 10, expected: '-32768' },
        { value: -1295, radix: 36, expected: '-zz' },
        { value: -1, radix: 10, expected: '-1' }
    ]
    
    for (let tc of testCases) {
        const result = tc.value.toString(tc.radix)
        arktest.assertEQ(result, tc.expected,
            `Short(${tc.value}).toString(${tc.radix}) should be '${tc.expected}', got '${result}'`)
    }
}

interface IntTestCase {
    value: int
    radix: int
    expected: string
}

function Int_toString_tests() {
    const testCases: IntTestCase[] = [
        { value: 255, radix: 2, expected: '11111111' },
        { value: 511, radix: 8, expected: '777' },
        { value: 65535, radix: 16, expected: 'ffff' },
        { value: 46655, radix: 36, expected: 'zzz' },
        { value: 0, radix: 10, expected: '0' },
        { value: 35, radix: 36, expected: 'z' },
        { value: 1295, radix: 36, expected: 'zz' },
        { value: -46655, radix: 36, expected: '-zzz' },
        { value: -1295, radix: 36, expected: '-zz' },
        { value: -1, radix: 10, expected: '-1' },
        { value: 1000, radix: 10, expected: '1000' },
    ]
    
    for (let tc of testCases) {
        const result = new Int(tc.value).toString(tc.radix)
        arktest.assertEQ(result, tc.expected,
            `Int(${tc.value}).toString(${tc.radix}) should be '${tc.expected}', got '${result}'`)
    }
}

interface LongTestCase {
    value: long
    radix: int
    expected: string
}

function Long_toString_tests() {
    const testCases: LongTestCase[] = [
        { value: 1023, radix: 2, expected: '1111111111' },
        { value: 2097151, radix: 8, expected: '7777777' },
        { value: 4294967295, radix: 16, expected: 'ffffffff' },
        { value: 1679615, radix: 36, expected: 'zzzz' },
        { value: 0, radix: 10, expected: '0' },
        { value: 35, radix: 36, expected: 'z' },
        { value: 1295, radix: 36, expected: 'zz' },
        { value: 46655, radix: 36, expected: 'zzz' },
        { value: -1679615, radix: 36, expected: '-zzzz' },
        { value: -46655, radix: 36, expected: '-zzz' },
        { value: -1295, radix: 36, expected: '-zz' },
        { value: -1, radix: 10, expected: '-1' },
    ]
    
    for (let tc of testCases) {
        const result = new Long(tc.value).toString(tc.radix)
        arktest.assertEQ(result, tc.expected,
            `Long(${tc.value}).toString(${tc.radix}) should be '${tc.expected}', got '${result}'`)
    }
}
