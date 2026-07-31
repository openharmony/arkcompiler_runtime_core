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

function isRangeError(error: Error): boolean {
    return error instanceof RangeError
}

function testIterableConstructorBufferSize(): void {
    const values = new Set<number>()
    values.add(10)
    values.add(20)
    values.add(30)
    const array = new Int8Array(values)
    arktest.assertEQ(array.buffer.byteLength, 3)
}

function testFractionalNegativeLength(): void {
    const array = new Int8Array(-0.5)
    arktest.assertEQ(array.length, 0)
    arktest.expectThrow(() => {
        new Int8Array(-1.5)
    }, isRangeError)
}

function testInfiniteLengthConstructor(): void {
    arktest.expectThrow(() => {
        new Int8Array(Number.POSITIVE_INFINITY)
    }, isRangeError)
}

function testFloat64MaximumValues(): void {
    const array = new Float64Array(2)
    array.$_set(0, 1.7976931348623157e+308)
    array.$_set(1, -1.7976931348623157e+308)
    arktest.assertEQ(array.toString(), '1.7976931348623157e+308,-1.7976931348623157e+308')
}

function testLargeSetOffsets(): void {
    const int8Target = new Int8Array([10, 20, 30])
    const int8Source = new Int8Array([1, 2])
    arktest.expectThrow(() => int8Target.set(int8Source, Int.MAX_VALUE), isRangeError)
    arktest.assertEQ(int8Target.toString(), '10,20,30')

    const bigTarget = new BigUint64Array([1n, 2n])
    const bigSource = new BigUint64Array([9n])
    arktest.expectThrow(() => bigTarget.set(bigSource, Int.MAX_VALUE), isRangeError)
    arktest.assertEQ(bigTarget.toString(), '1,2')
}

function testMinimumWithIndex(): void {
    const source = BigUint64Array.of(10n, 20n, 30n)
    arktest.expectThrow(() => {
        source.with(Int.MIN_VALUE, 99n)
    }, isRangeError)
    arktest.assertEQ(source.toString(), '10,20,30')
}

function testToLocaleStringTypedOptions(): void {
    const array = new Int8Array([127])
    const options: Intl.NumberFormatOptions = { minimumFractionDigits: 2 }
    arktest.assertEQ(array.toLocaleString('en-US', options), '127.00')
}

function testWithInfinityValue(): void {
    const array = new Int8Array([5])
    arktest.assertEQ(array.with(0, Number.POSITIVE_INFINITY).at(0), 0)
}

function testItem4Float32ReduceError(): void {
    const array = new Float32Array(0)
    const callback = (accumulator: number, value: number): number => accumulator + value
    arktest.expectThrow(() => {
        array.reduce(callback)
    }, (error: Error): boolean => error instanceof TypeError)
}

function testItem5BigInt64FindLastResult(): void {
    const array = new BigInt64Array([10n, 20n])
    const result = array.findLast((_value: BigInt, index: int): boolean => index == 0)
    arktest.assertTrue(result == 10n)
}

function testItem8BigUint64FractionalNegativeLength(): void {
    const array = new BigUint64Array(-0.5)
    arktest.assertEQ(array.length, 0)
}

function testItem10BigUint64ToLocaleString(): void {
    const array = new BigUint64Array([18446744073709551614n, 18446744073709551615n, -1n, -2n])
    const options = { useGrouping: false } as BigIntToLocaleStringOptions
    arktest.assertEQ(
        array.toLocaleString('en-US', options),
        '18446744073709551614,18446744073709551615,18446744073709551615,18446744073709551614'
    )
}

function testItem12BigUint64Callbacks(): void {
    const maximum = 18446744073709551615n
    const array = new BigUint64Array([maximum, maximum])
    let callCount = 0

    arktest.assertTrue(array.every((element: BigInt): boolean => {
        callCount++
        return element == maximum
    }))
    arktest.assertEQ(callCount, 2)

    const filtered = array.filter((element: BigInt): boolean => element == maximum)
    arktest.assertEQ(filtered.length, 2)
    arktest.assertTrue(array.find((element: BigInt): boolean => element == maximum) == maximum)
    arktest.assertEQ(array.findIndex((element: BigInt): boolean => element > 0n), 0)
    arktest.assertEQ(array.findLastIndex((element: BigInt): boolean => element != maximum), -1)

    const wrapped = new BigUint64Array([-1n])
    arktest.assertTrue(wrapped[0] == maximum)
    let matchedCount = 0
    wrapped.forEach((element: BigInt): void => {
        if (element == maximum) {
            matchedCount++
        }
    })
    arktest.assertEQ(matchedCount, 1)
    arktest.assertTrue(array.some((element: BigInt): boolean => element == maximum))

    const reduced = array.reduce<BigInt>(
        (previous: BigInt, current: BigInt): BigInt => previous + current,
        0n
    )
    const reducedRight = array.reduceRight<BigInt>(
        (previous: BigInt, current: BigInt): BigInt => previous - current,
        0n
    )
    arktest.assertTrue(reduced == 36893488147419103230n)
    arktest.assertTrue(reducedRight == -36893488147419103230n)
}

function testItem13Uint8ClampedWith(): void {
    const array = new Uint8ClampedArray([10, 20, 30])
    arktest.assertEQ(array.with(0, 1.5).toString(), '2,20,30')
}

function testItem24Float64LargeSetOffset(): void {
    const destination = new Float64Array([1.0, 2.0])
    const source = new Float64Array([9.0])
    arktest.expectThrow(() => destination.set(source, Int.MAX_VALUE), isRangeError)
    arktest.assertEQ(destination.toString(), '1,2')
}

function testItem26Float64BufferSize(): void {
    const buffer = new ArrayBuffer(15)
    arktest.expectThrow(() => {
        new Float64Array(buffer, 0)
    }, isRangeError)
}

function main(): int {
    const suite = new arktest.ArkTestsuite('GitCode issue 15 typed array regressions')
    suite.addTest('iterable constructor buffer size', testIterableConstructorBufferSize)
    suite.addTest('fractional negative length', testFractionalNegativeLength)
    suite.addTest('Infinity length constructor', testInfiniteLengthConstructor)
    suite.addTest('Float64 maximum values', testFloat64MaximumValues)
    suite.addTest('large set offsets', testLargeSetOffsets)
    suite.addTest('minimum with index', testMinimumWithIndex)
    suite.addTest('toLocaleString typed options', testToLocaleStringTypedOptions)
    suite.addTest('with Infinity value', testWithInfinityValue)
    suite.addTest('item 4 Float32 reduce TypeError', testItem4Float32ReduceError)
    suite.addTest('item 5 BigInt64 findLast result', testItem5BigInt64FindLastResult)
    suite.addTest('item 8 BigUint64 fractional negative length', testItem8BigUint64FractionalNegativeLength)
    suite.addTest('item 10 BigUint64 toLocaleString', testItem10BigUint64ToLocaleString)
    suite.addTest('item 12 BigUint64 callbacks', testItem12BigUint64Callbacks)
    suite.addTest('item 13 Uint8Clamped with', testItem13Uint8ClampedWith)
    suite.addTest('item 24 Float64 large set offset', testItem24Float64LargeSetOffset)
    suite.addTest('item 26 Float64 buffer size', testItem26Float64BufferSize)
    return suite.run()
}
