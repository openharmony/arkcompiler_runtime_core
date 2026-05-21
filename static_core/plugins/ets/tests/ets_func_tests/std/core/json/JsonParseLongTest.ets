/*
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

class LongHolder {
    public value: long = 0
    public constructor() {}
}

const longHolderClass = Class.from<LongHolder>()
const longClass = Class.from<long>()

function testParseLongSafeInteger(): void {
    let result = JSON.parse<LongHolder>('{"value":12345}', longHolderClass)
    arktest.assertEQ(result!.value, 12345)

    let longResult = JSON.parse<long>('12345', longClass)
    arktest.assertEQ(longResult, 12345)
}

function testParseLongMaxSafeInteger(): void {
    // 2^53-1 = 9007199254740991, boundary of safe integer
    let result = JSON.parse<LongHolder>('{"value":9007199254740991}', longHolderClass)
    arktest.assertEQ(result!.value, 9007199254740991)

    let longResult = JSON.parse<long>('9007199254740991', longClass)
    arktest.assertEQ(longResult, 9007199254740991)
}

function testParseLongBeyondSafeInteger(): void {
    // 9007199254740993 is beyond Number.MAX_SAFE_INTEGER.
    // As double it becomes 9007199254740992, so num.toLong() returns wrong value.
    let result = JSON.parse<LongHolder>('{"value":9007199254740993}', longHolderClass)
    arktest.assertEQ(result!.value, 9007199254740993)

    let longResult = JSON.parse<long>('9007199254740993', longClass)
    arktest.assertEQ(longResult, 9007199254740993)
}

function testParseLongMaxValue(): void {
    let result = JSON.parse<LongHolder>('{"value":9223372036854775807}', longHolderClass)
    arktest.assertEQ(result!.value, 9223372036854775807)

    let longResult = JSON.parse<long>('9223372036854775807', longClass)
    arktest.assertEQ(longResult, 9223372036854775807)
}

function testParseLongMinValue(): void {
    let result = JSON.parse<LongHolder>('{"value":-9223372036854775808}', longHolderClass)
    arktest.assertEQ(result!.value, -9223372036854775808)

    let longResult = JSON.parse<long>('-9223372036854775808', longClass)
    arktest.assertEQ(longResult, -9223372036854775808)
}

function testParseLongNegativeBeyondSafeInteger(): void {
    let result = JSON.parse<LongHolder>('{"value":-9007199254740993}', longHolderClass)
    arktest.assertEQ(result!.value, -9007199254740993)

    let longResult = JSON.parse<long>('-9007199254740993', longClass)
    arktest.assertEQ(longResult, -9007199254740993)
}

function testParseLongRejectsFloat(): void {
    arktest.expectThrow(() => {
        JSON.parse<LongHolder>('{"value":3.14}', longHolderClass)
    }, (e: Error): boolean => {
        return e instanceof JSONTypeError
    })

    arktest.expectThrow(() => {
        JSON.parse<long>('3.14', longClass)
    }, (e: Error): boolean => {
        return e instanceof JSONTypeError
    })
}

function testParseLongScientificNotation(): void {
    // 1e10 = 10000000000, scientific notation within Long range
    let result = JSON.parse<LongHolder>('{"value":1e10}', longHolderClass)
    arktest.assertEQ(result!.value, 10000000000)

    let longResult = JSON.parse<long>('1e10', longClass)
    arktest.assertEQ(longResult, 10000000000)
}

function testParseLongRejectsOverflowScientific(): void {
    // 1e20 = 100000000000000000000, exceeds Long.MAX_VALUE (9223372036854775807)
    arktest.expectThrow(() => {
        JSON.parse<LongHolder>('{"value":1e20}', longHolderClass)
    }, (e: Error): boolean => {
        return e instanceof JSONTypeError
    })

    arktest.expectThrow(() => {
        JSON.parse<long>('1e20', longClass)
    }, (e: Error): boolean => {
        return e instanceof JSONTypeError
    })
}

function testParseLongRejectsOverflowByOne(): void {
    // 9223372036854775808 = Long.MAX_VALUE + 1, just beyond Long range
    arktest.expectThrow(() => {
        JSON.parse<LongHolder>('{"value":9223372036854775808}', longHolderClass)
    }, (e: Error): boolean => {
        return e instanceof JSONTypeError
    })

    arktest.expectThrow(() => {
        JSON.parse<long>('9223372036854775808', longClass)
    }, (e: Error): boolean => {
        return e instanceof JSONTypeError
    })
}

function testParseLongRejectsUnderflowByOne(): void {
    // -9223372036854775809 = Long.MIN_VALUE - 1, just below Long range
    arktest.expectThrow(() => {
        JSON.parse<LongHolder>('{"value":-9223372036854775809}', longHolderClass)
    }, (e: Error): boolean => {
        return e instanceof JSONTypeError
    })

    arktest.expectThrow(() => {
        JSON.parse<long>('-9223372036854775809', longClass)
    }, (e: Error): boolean => {
        return e instanceof JSONTypeError
    })
}

function testParseLongZero(): void {
    let result = JSON.parse<LongHolder>('{"value":0}', longHolderClass)
    arktest.assertEQ(result!.value, 0)

    let longResult = JSON.parse<long>('0', longClass)
    arktest.assertEQ(longResult, 0)
}


function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.parse Long precision tests')

    suite.addTest('parse long safe integer', testParseLongSafeInteger)
    suite.addTest('parse long max safe integer', testParseLongMaxSafeInteger)
    suite.addTest('parse long beyond safe integer', testParseLongBeyondSafeInteger)
    suite.addTest('parse long MAX_VALUE', testParseLongMaxValue)
    suite.addTest('parse long MIN_VALUE', testParseLongMinValue)
    suite.addTest('parse long negative beyond safe integer', testParseLongNegativeBeyondSafeInteger)
    suite.addTest('parse long rejects float', testParseLongRejectsFloat)
    suite.addTest('parse long scientific notation', testParseLongScientificNotation)
    suite.addTest('parse long rejects overflow scientific', testParseLongRejectsOverflowScientific)
    suite.addTest('parse long rejects overflow by one', testParseLongRejectsOverflowByOne)
    suite.addTest('parse long rejects underflow by one', testParseLongRejectsUnderflowByOne)
    suite.addTest('parse long zero', testParseLongZero)

    return suite.run()
}
