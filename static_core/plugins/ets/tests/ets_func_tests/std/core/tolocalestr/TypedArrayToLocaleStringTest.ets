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

const locale = 'zh-CN'
const options: Intl.NumberFormatOptions = {
    style: 'currency',
    currency: 'USD',
    currencyDisplay: 'symbol',
}

function test1(): void {
    const arr = new Int8Array([1, -2, 3])
    const result = arr.toLocaleString(locale)
    arktest.assertTrue(result.includes(','), 'Int8Array should separate elements with commas')
    arktest.assertTrue(result.includes('-2'), 'Int8Array should include negative numbers')

    const currencyOptions = { style: 'currency', currency: 'EUR' } as Intl.NumberFormatOptions
    const resultCurrency = arr.toLocaleString(locale, currencyOptions)
    arktest.assertTrue(resultCurrency.includes('€'), 'Currency formatting should include Euro symbol')
    arktest.assertTrue(resultCurrency.includes(','), 'Currency-formatted Int8Array should separate elements with commas')

    const noGroupOptions = { useGrouping: false, minimumIntegerDigits: 2 } as Intl.NumberFormatOptions
    const resultNoGroup = arr.toLocaleString(locale, noGroupOptions)
    arktest.assertTrue(resultNoGroup.length > 0, 'Int8Array should produce non-empty output')
}

function test2(): void {
    const data = [1n, -2n, 3456789012345n]
    const arr = new BigInt64Array(data)
    const result = arr.toLocaleString(locale)
    arktest.assertTrue(result.includes(','), 'BigInt64Array should separate elements with commas')
    arktest.assertTrue(result.includes('-2'), 'BigInt64Array should display negative BigInt values')

    const options = { useGrouping: true, minimumIntegerDigits: 2 } as BigIntToLocaleStringOptions
    const formatted = arr.toLocaleString(locale, options)
    arktest.assertTrue(formatted.length > 0, 'BigInt64Array should format elements into a non-empty string')
    arktest.assertTrue(formatted.includes(','), 'BigInt64Array formatted output should include commas')

    let typedResult = arr.toLocaleString(locale, options)
    for (let i = 0; i < data.length; ++i) {
        arktest.assertTrue(typedResult.includes(data[i].toLocaleString(locale, options)), 'BigInt64Array and BigInt should produce identical formatted output')
    }
}

function test3() {
    let i8 = new Int8Array()
    let i16 = new Int16Array()
    let i32 = new Int32Array()
    let f32 = new Float32Array()
    let f64 = new Float64Array()
    let big = new BigInt64Array()

    const expected0 = ''
    arktest.assertEQ(i8.toLocaleString(), expected0)
    arktest.assertEQ(i16.toLocaleString(), expected0)
    arktest.assertEQ(i32.toLocaleString(), expected0)
    arktest.assertEQ(f32.toLocaleString(), expected0)
    arktest.assertEQ(f64.toLocaleString(), expected0)
    arktest.assertEQ(big.toLocaleString(), expected0)

    arktest.assertEQ(i8.toLocaleString(undefined), expected0)
    arktest.assertEQ(i16.toLocaleString(undefined), expected0)
    arktest.assertEQ(i32.toLocaleString(undefined), expected0)
    arktest.assertEQ(f32.toLocaleString(undefined), expected0)
    arktest.assertEQ(f64.toLocaleString(undefined), expected0)
    arktest.assertEQ(big.toLocaleString(undefined), expected0)

    arktest.assertEQ(i8.toLocaleString(undefined, undefined), expected0)
    arktest.assertEQ(i16.toLocaleString(undefined, undefined), expected0)
    arktest.assertEQ(i32.toLocaleString(undefined, undefined), expected0)
    arktest.assertEQ(f32.toLocaleString(undefined, undefined), expected0)
    arktest.assertEQ(f64.toLocaleString(undefined, undefined), expected0)
    arktest.assertEQ(big.toLocaleString(undefined, undefined), expected0)

    arktest.assertEQ(i8.toLocaleString(locale, options), expected0)
    arktest.assertEQ(i16.toLocaleString(locale, options), expected0)
    arktest.assertEQ(i32.toLocaleString(locale, options), expected0)
    arktest.assertEQ(f32.toLocaleString(locale, options), expected0)
    arktest.assertEQ(f64.toLocaleString(locale, options), expected0)
    arktest.assertEQ(big.toLocaleString(locale,
        { style: options.style,
          currency: options.currency,
          currencyDisplay: options.currencyDisplay } as BigIntToLocaleStringOptions),
        expected0)

    const n: FixedArray<int> = [-127, 127]
    i8 = new Int8Array(n)
    i16 = new Int16Array(n)
    i32 = new Int32Array(n)
    f32 = new Float32Array(n)
    f64 = new Float64Array(n)
    big = new BigInt64Array(n)

    const expected1 = '-127,127'
    arktest.assertEQ(i8.toLocaleString(), expected1)
    arktest.assertEQ(i16.toLocaleString(), expected1)
    arktest.assertEQ(i32.toLocaleString(), expected1)
    arktest.assertEQ(f32.toLocaleString(), expected1)
    arktest.assertEQ(f64.toLocaleString(), expected1)
    arktest.assertEQ(big.toLocaleString(), expected1)

    arktest.assertEQ(i8.toLocaleString(undefined), expected1)
    arktest.assertEQ(i16.toLocaleString(undefined), expected1)
    arktest.assertEQ(i32.toLocaleString(undefined), expected1)
    arktest.assertEQ(f32.toLocaleString(undefined), expected1)
    arktest.assertEQ(f64.toLocaleString(undefined), expected1)
    arktest.assertEQ(big.toLocaleString(undefined), expected1)

    arktest.assertEQ(i8.toLocaleString(undefined, undefined), expected1)
    arktest.assertEQ(i16.toLocaleString(undefined, undefined), expected1)
    arktest.assertEQ(i32.toLocaleString(undefined, undefined), expected1)
    arktest.assertEQ(f32.toLocaleString(undefined, undefined), expected1)
    arktest.assertEQ(f64.toLocaleString(undefined, undefined), expected1)
    arktest.assertEQ(big.toLocaleString(undefined, undefined), expected1)

    arktest.assertEQ(i8.toLocaleString(locale), expected1)
    arktest.assertEQ(i16.toLocaleString(locale), expected1)
    arktest.assertEQ(i32.toLocaleString(locale), expected1)
    arktest.assertEQ(f32.toLocaleString(locale), expected1)
    arktest.assertEQ(f64.toLocaleString(locale), expected1)
    arktest.assertEQ(big.toLocaleString(locale), expected1)

    const expected2 = '-US$127.00,US$127.00'
    arktest.assertEQ(i8.toLocaleString(locale, options), expected2)
    arktest.assertEQ(i16.toLocaleString(locale, options), expected2)
    arktest.assertEQ(i32.toLocaleString(locale, options), expected2)
    arktest.assertEQ(f32.toLocaleString(locale, options), expected2)
    arktest.assertEQ(f64.toLocaleString(locale, options), expected2)
    arktest.assertEQ(big.toLocaleString(locale,
        { style: options.style,
          currency: options.currency,
          currencyDisplay: options.currencyDisplay } as BigIntToLocaleStringOptions),
        expected2)
}

function test4() {
    let u8clamped = new Uint8ClampedArray()
    let u8 = new Uint8Array()
    let u16 = new Uint16Array()
    let u32 = new Uint32Array()
    let ubig = new BigUint64Array()

    const expected0 = ''
    arktest.assertEQ(u8clamped.toLocaleString(), expected0)
    arktest.assertEQ(u8.toLocaleString(), expected0)
    arktest.assertEQ(u16.toLocaleString(), expected0)
    arktest.assertEQ(u32.toLocaleString(), expected0)
    arktest.assertEQ(ubig.toLocaleString(), expected0)

    arktest.assertEQ(u8clamped.toLocaleString(undefined), expected0)
    arktest.assertEQ(u8.toLocaleString(undefined), expected0)
    arktest.assertEQ(u16.toLocaleString(undefined), expected0)
    arktest.assertEQ(u32.toLocaleString(undefined), expected0)
    arktest.assertEQ(ubig.toLocaleString(undefined), expected0)

    arktest.assertEQ(u8clamped.toLocaleString(undefined, undefined), expected0)
    arktest.assertEQ(u8.toLocaleString(undefined, undefined), expected0)
    arktest.assertEQ(u16.toLocaleString(undefined, undefined), expected0)
    arktest.assertEQ(u32.toLocaleString(undefined, undefined), expected0)
    arktest.assertEQ(ubig.toLocaleString(undefined, undefined), expected0)

    arktest.assertEQ(u8clamped.toLocaleString(locale, options), expected0)
    arktest.assertEQ(u8.toLocaleString(locale, options), expected0)
    arktest.assertEQ(u16.toLocaleString(locale, options), expected0)
    arktest.assertEQ(u32.toLocaleString(locale, options), expected0)
    arktest.assertEQ(ubig.toLocaleString(locale,
        { style: options.style,
          currency: options.currency,
          currencyDisplay: options.currencyDisplay } as BigIntToLocaleStringOptions),
        expected0)

    const m: FixedArray<int> = [1, 127]
    u8clamped = new Uint8ClampedArray(m)
    u8 = new Uint8Array(m)
    u16 = new Uint16Array(m)
    u32 = new Uint32Array(m)
    ubig = new BigUint64Array(m)

    const expected1 = '1,127'
    arktest.assertEQ(u8clamped.toLocaleString(), expected1)
    arktest.assertEQ(u8.toLocaleString(), expected1)
    arktest.assertEQ(u16.toLocaleString(), expected1)
    arktest.assertEQ(u32.toLocaleString(), expected1)
    arktest.assertEQ(ubig.toLocaleString(), expected1)

    arktest.assertEQ(u8clamped.toLocaleString(undefined), expected1)
    arktest.assertEQ(u8.toLocaleString(undefined), expected1)
    arktest.assertEQ(u16.toLocaleString(undefined), expected1)
    arktest.assertEQ(u32.toLocaleString(undefined), expected1)
    arktest.assertEQ(ubig.toLocaleString(undefined), expected1)

    arktest.assertEQ(u8clamped.toLocaleString(undefined, undefined), expected1)
    arktest.assertEQ(u8.toLocaleString(undefined, undefined), expected1)
    arktest.assertEQ(u16.toLocaleString(undefined, undefined), expected1)
    arktest.assertEQ(u32.toLocaleString(undefined, undefined), expected1)
    arktest.assertEQ(ubig.toLocaleString(undefined, undefined), expected1)

    arktest.assertEQ(u8clamped.toLocaleString(locale), expected1)
    arktest.assertEQ(u8.toLocaleString(locale), expected1)
    arktest.assertEQ(u16.toLocaleString(locale), expected1)
    arktest.assertEQ(u32.toLocaleString(locale), expected1)
    arktest.assertEQ(ubig.toLocaleString(locale), expected1)

    const expected2 = 'US$1.00,US$127.00'
    arktest.assertEQ(u8clamped.toLocaleString (locale, options), expected2)
    arktest.assertEQ(u8.toLocaleString(locale, options), expected2)
    arktest.assertEQ(u16.toLocaleString(locale, options), expected2)
    arktest.assertEQ(u32.toLocaleString(locale, options), expected2)
    arktest.assertEQ(ubig.toLocaleString(locale,
        { style: options.style,
          currency: options.currency,
          currencyDisplay: options.currencyDisplay } as BigIntToLocaleStringOptions),
        expected2)
}



function main(): int {
    const suite = new arktest.ArkTestsuite('TypedArray')
    suite.addTest('test1', test1)
    suite.addTest('test2', test2)
    suite.addTest('test3', test3)
    suite.addTest('test4', test4)
    return suite.run()
}
