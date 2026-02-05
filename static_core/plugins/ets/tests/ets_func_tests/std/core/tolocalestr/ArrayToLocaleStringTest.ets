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

class MyClass {
    private value: number = 0

    constructor(val: number) {
        this.value = val
    }

    public override toLocaleString(locales?: Intl.LocalesArgument, options?: object): string {
        return 'mir_' + this.value.toLocaleString(locales, options)
    }
}

function test1(): void {
    const numOptions: Intl.NumberFormatOptions = { style: 'currency', currency: 'EUR' }
    const nums = [1, 2, 3]
    const resultNum = nums.toLocaleString(locale, numOptions)
    arktest.assertTrue(resultNum.includes('€'), 'Number array should include Euro symbol')
    arktest.assertTrue(resultNum.includes(','), 'Number array should separate elements with commas')
    arktest.assertTrue(resultNum.includes('1'), 'Number array should contains 1')
    arktest.assertTrue(resultNum.includes('2'), 'Number array should contains 2')
    arktest.assertTrue(resultNum.includes('3'), 'Number array should contains 3')

    const bigints = [12345678901234567890n, 42n]
    const bigintOptions: BigIntToLocaleStringOptions = { useGrouping: false }
    const resultBigint = bigints.toLocaleString(locale, bigintOptions)
    arktest.assertEQ(resultBigint, '12345678901234567890,42');

    const dates = [new Date(2024, 1, 19, 10, 0, 0), new Date(2024, 1, 19, 15, 30, 0)]
    const dateOptions: Intl.DateTimeFormatOptions = { hour: '2-digit', minute: '2-digit', second: '2-digit' }
    const resultDate = dates.toLocaleString(locale, dateOptions)
    arktest.assertEQ(resultDate, '10:00:00,15:30:00')
}

function test2(): void {
    const numberOptions = { currency: 'USD', style: 'currency' } as Intl.NumberFormatOptions
    const bigintOptions = { style: 'percent' } as BigIntToLocaleStringOptions
    const dateOptions = { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric', hour: '2-digit',
        minute: '2-digit', second: '2-digit' } as Intl.DateTimeFormatOptions

    let arr = new Array<Byte | Short | Int | Long | Float | Double | BigInt | Date | MyClass | Array<Double> >
    const expected0 = ''
    arktest.assertEQ(arr.toLocaleString(), expected0)
    arktest.assertEQ(arr.toLocaleString(undefined), expected0)
    arktest.assertEQ(arr.toLocaleString(undefined, undefined), expected0)
    arktest.assertEQ(arr.toLocaleString(locale), expected0)
    arktest.assertEQ(arr.toLocaleString(locale, numberOptions), expected0)
    arktest.assertEQ(arr.toLocaleString(locale, bigintOptions), expected0)
    arktest.assertEQ(arr.toLocaleString(locale, dateOptions), expected0)

    arr.push(new Byte(127))
    arr.push(new Short(32765))
    arr.push(new Int(21474836))
    arr.push(new Long(922337203))
    arr.push(new Float(2030.40))
    arr.push(new Double(6070.80))
    arr.push(new BigInt(19223372036n))
    arr.push(new Date('2024-01-15T10:30:00'))
    arr.push(new MyClass(33))
    arr.push([5.1, 7.2])

    let expected1 = '127,32,765,21,474,836,922,337,203,2,030.4,6,070.8,19,223,372,036,1/15/2024, 10:30:00 AM,mir_33,5.1,7.2'
    let expected2 = 'US$127.00,US$32,765.00,US$21,474,836.00,US$922,337,203.00,US$2,030.40,US$6,070.80,19,223,372,036,2024/1/15 10:30:00,mir_US$33.00,US$5.10,US$7.20'
    let expected3 = '127,32,765,21,474,836,922,337,203,2,030.4,6,070.8,19,223,372,036,2024/1/15 10:30:00,mir_33,5.1,7.2'
    let expected4 = '127,32,765,21,474,836,922,337,203,2,030.4,6,070.8,1,922,337,203,600%,2024/1/15 10:30:00,mir_33,5.1,7.2'
    let expected5 = '127,32,765,21,474,836,922,337,203,2,030.4,6,070.8,19,223,372,036,2024年1月15日星期一 10:30:00,mir_33,5.1,7.2'
    let expected6 = '127,32,765,21,474,836,922,337,203,2,030.4,6,070.8,19,223,372,036,1/15/2024, 10:30:00 AM,mir_33,5.1,7.2'

    arktest.assertEQ(arr.toLocaleString(), expected1)
    arktest.assertEQ(arr.toLocaleString(locale, numberOptions), expected2)
    arktest.assertEQ(arr.toLocaleString(locale, undefined), expected3)
    arktest.assertEQ(arr.toLocaleString(locale, bigintOptions), expected4)
    arktest.assertEQ(arr.toLocaleString(locale, dateOptions), expected5)
    arktest.expectThrow(() => { arr.toLocaleString("") }, (e: Error) => e instanceof RangeError)
    arktest.assertEQ(arr.toLocaleString(undefined), expected6)
    arktest.assertEQ(arr.toLocaleString(undefined, undefined), expected6)
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array')
    suite.addTest('test1', test1)
    suite.addTest('test2', test2)
    return suite.run()
}
