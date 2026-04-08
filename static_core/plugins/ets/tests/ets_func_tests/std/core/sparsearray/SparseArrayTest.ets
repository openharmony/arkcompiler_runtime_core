/**
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

function main(): int {
    const suite = new arktest.ArkTestsuite('SparseArray')
    suite.addTest('empty constructor', testConstructorEmpty)
    suite.addTest('construcotr length', testConstructorLength)
    suite.addTest('constructor rest', testConstructorRest)
    suite.addTest('from ArrayLike', testFromArrayLike)
    suite.addTest('isArray', testIsArray)
    suite.addTest('SparseArray of', testSparseArrayOf)
    suite.addTest('Push/pop test', testPushPop)
    suite.addTest('Reverse SparseArray', testReverse)
    suite.addTest('Join SparseArray', testJoin)
    suite.addTest('Shift SparseArray', testShift)
    suite.addTest('Splice SparseArray', testSplice)
    suite.addTest('Splice SparseArray single arg', testSpliceSingleArg)
    suite.addTest('Sort SparseArray', testSortDefault)
    suite.addTest('Fill SparseArray', testFill)
    suite.addTest('Concat SparseArray', testConcat)
    suite.addTest('Slice SparseArray', testSlice)
    suite.addTest('Map SparseArray', testMap)
    suite.addTest('Filter SparseArray', testFilter)
    suite.addTest('Foreach SparseArray', testForEach)
    suite.addTest('Reduce SparseArray', testReduce)
    suite.addTest('Some SparseArray', testSome)
    suite.addTest('Find SparseArray', testFind)
    suite.addTest('FindIndex SparseArray', testFindIndex)
    suite.addTest('IndexOf SparseArray', testIndexOf)
    suite.addTest('Includes SparseArray', testIncludes)
    suite.addTest('Every SparseArray', testEvery)
    suite.addTest('Unshift SparseArray', testUnshift)
    suite.addTest('Keys SparseArray', testKeysValues)
    suite.addTest('Copywithin SparseArray', testCopyWithIn)
    suite.addTest('Flat SparseArray', testFlat)
    suite.addTest('Sort union', testSortUnion)
    suite.addTest('Sort union', testSortLong)
    suite.addTest('Set length SparseArray', testLengthSet)
    suite.addTest('Test Holes SparseArray', testHoles)
    suite.addTest('Test SparseArray toLocaleString', testToLocaleString)
    return suite.run()
}

function checkExpected<T>(actual: SparseArray<T>, expected: Array<T>) {
    arktest.assertEQ(actual.length, expected.length)
    for (let i = 0; i < expected.length; i++) {
        arktest.assertEQ(actual[i], expected[i])
    }
}

function testConstructorEmpty() {
    let x = new SparseArray<int>()
    arktest.assertEQ(x.length, 0)
}

function testConstructorLength() {
    let x = new SparseArray<int>(12)
    arktest.assertEQ(x.length, 12)
}

function testConstructorRest() {
    let x = new SparseArray<int>(1, 2, 3, 4)
    arktest.assertEQ(x.length, 4)
    checkExpected(x, [1, 2, 3, 4])
}

function testFromArrayLike() {
    let x = SparseArray.from<int>([1, 2, 3, 4])
    arktest.assertEQ(x.length, 4)
    checkExpected(x, [1, 2, 3, 4])
}

function testIsArray() {
    let x = new SparseArray<int>()
    arktest.assertTrue(SparseArray.isArray(x), 'it is SparseArray')
    arktest.assertFalse(SparseArray.isArray(new Object), 'it is not SparseArray')
    arktest.assertFalse(SparseArray.isArray(new Array<int>()), 'it is not SparseArray')
}

function testSparseArrayOf() {
    let x = SparseArray.of<int>(2)
    arktest.assertEQ(x.length, 1)
    arktest.assertEQ(x[0], 2)
}

function testPushPop() {
    let x = new SparseArray<int>()
    x.push(2)
    x.push(3, 4)
    arktest.assertEQ(x.length, 3)
    arktest.assertEQ(x.pop(), 4)
    arktest.assertEQ(x.at(1), 3)
    arktest.assertEQ(x.pop(), 3)
    arktest.assertEQ(x.at(0), 2)
    arktest.assertEQ(x.pop(), 2)
    arktest.assertEQ(x.at(0), undefined)
}

function testReverse() {
    let x = new SparseArray<int>(1, 2, 3, 4)
    x.reverse()
    checkExpected(x, [4, 3, 2, 1])
}

function testJoin() {
    let x = new SparseArray<int>()
    x[2] = 3
    x[4] = 5
    arktest.assertEQ(x.join(), ',,3,,5')
    arktest.assertEQ(x.toString(), ',,3,,5')

    let y = new SparseArray<Any>(3)
    y[2] = 2
    y[0] = y
    arktest.assertEQ(y.join('!'), '!!2')
}

function testShift() {
    let x = new SparseArray<double>(1, 2, 3, 4)
    x.shift()
    checkExpected(x, [2, 3, 4])
}

function testSplice() {
    let x = new SparseArray<double>(1, 2, 3, 4)
    checkExpected(x.toSpliced(1, 1, 7, 7), [1, 7, 7, 3, 4])
    let res = x.splice(1, 1, 7, 7)

    checkExpected(x, [1, 7, 7, 3, 4])
    checkExpected(res, [2])
}

function testSpliceSingleArg() {
    let x = new SparseArray<double>(1, 2, 3, 4)
    let res = x.splice(3)
    checkExpected(x, [1, 2, 3])
    checkExpected(res, [4])

    x = new SparseArray<double>(1, 2, 3, 4)
    res = x.splice(3, undefined)
    checkExpected(x, [1, 2, 3, 4])
    checkExpected(res, [])
}

function testSortDefault() {
    let x = new SparseArray<double>(2, 4, 3, 1)
    checkExpected(x.toSorted(), [1, 2, 3, 4])
    x.sort()
    checkExpected(x, [1, 2, 3, 4])
}

function testFill() {
    let x = new SparseArray<double>(5)
    x.fill(1)
    x.fill(2, 4, 5)
    checkExpected(x, [1, 1, 1, 1, 2])
}

function testConcat() {
    let x = new SparseArray<int>()
    let res = x.concat([1, 3, 5], [2, 4, 6])
    let expected = [1, 3, 5, 2, 4, 6]
    checkExpected(res, expected)
}

function testSlice() {
    let x = new SparseArray<int>(1, 2, 3, 4, 5)
    let res = x.slice()
    checkExpected(res, [1, 2, 3, 4, 5])
    checkExpected(x.slice(1, 2), [2])
    checkExpected(x.slice(1), [2, 3, 4, 5])
}

function testMap() {
    let x = new SparseArray<int>(1, 1, 1, 1, 1)
    let res = x.map<string>((v, i, arr) => { return i.toString() })
    checkExpected(res, ['0', '1', '2', '3', '4'])
}

function testFilter() {
    let x = new SparseArray<int>(1, 2, 3, 4, 5)
    let res = x.filter((v, i, arr) => { return v > 2 })
    
    checkExpected(res, [3, 4, 5])
}

function testForEach() {
    let x = new SparseArray<int>(12)
    x.push(1, 3, 5)
    let count = 0
    x.forEach((v, i, arr) => { count += 1 })
    arktest.assertEQ(count, 3)
}

function testReduce() {
    let x = new SparseArray<int>(12)
    x.push(1, 3, 5)
    let sum = 0
    let res = x.reduce((prev, cur) => { return prev + cur})
    arktest.assertEQ(res, 9)

    res = x.reduceRight((prev, cur) => { return prev + cur})
    arktest.assertEQ(res, 9)
}

function testSome() {
    let x = new SparseArray<int>(1)
    x.push(1, 3, 5)
    let res = x.some((v, i, arr) => { return v < 0})
    arktest.assertEQ(res, false)
    res = x.some((v, i, arr) => { return v > 0})
    arktest.assertEQ(res, true)
}

function testFind() {
    let x = new SparseArray<int>(1)
    x.push(1, 3, 5)
    let res = x.find((val) => { return val === 3 })
    arktest.assertEQ(res, 3)
    res = x.find((val) => { return val === 2 })
    arktest.assertEQ(res, undefined)

    res = x.findLast((val) => { return val === 3 })
    arktest.assertEQ(res, 3)
    res = x.findLast((val) => { return val === 2 })
    arktest.assertEQ(res, undefined)
}

function testFindIndex() {
    let x = new SparseArray<int>(1)
    x.push(1, 3, 5)
    let res = x.findIndex((val) => { return val === 3 })
    arktest.assertEQ(res, 2)
    res = x.findIndex((val) => { return val === 2 })
    arktest.assertEQ(res, -1)

    res = x.findLastIndex((val) => { return val === 3 })
    arktest.assertEQ(res, 2)
    res = x.findLastIndex((val) => { return val === 2 })
    arktest.assertEQ(res, -1)
}

function testIndexOf() {
    let x  = new SparseArray<int>(1)
    x.push(1, 3, 5)
    let res = x.indexOf(3)
    arktest.assertEQ(res, 2)
    res = x.indexOf(3, 3)
    arktest.assertEQ(res, -1)

    res = x.lastIndexOf(3)
    arktest.assertEQ(res, 2)
    res = x.lastIndexOf(3, 2)
    arktest.assertEQ(res, 2)
    arktest.assertEQ(x.lastIndexOf(2, undefined), -1)
}

function testIncludes() {
    let x  = new SparseArray<number>(1)
    x.push(1, 3, 5, NaN)
    arktest.assertTrue(x.includes(3), '3 is in array')
    arktest.assertTrue(x.includes(NaN), 'NaN is in array')
    arktest.assertFalse(x.includes(-1), '-1 is not in array')
}

function testEvery() {
    let x = new SparseArray<int>(1)
    x.push(1, 3, 5)
    let res = x.every((v, i, arr) => { return v < 0 })
    arktest.assertFalse(res, 'every should return false')
    res = x.every((v, i, arr) => { return v > 0 })
    arktest.assertTrue(res, 'every should return true')
}

function testUnshift() {
    let x = new SparseArray<int>(1)
    x.push(1, 3, 5)
    let res = x.unshift(7, 7, 7)
    arktest.assertEQ(res, 7)
    arktest.assertEQ(x[0], 7)
    arktest.assertEQ(x[1], 7)
    arktest.assertEQ(x[2], 7)
}

function testKeysValues() {
    let x = new SparseArray<int>(1)
    x.push(1, 3, 5)
    let actual = Array.from<int>(x.keys())

    // ignores holes
    let expectedKeys: Array<int> = [1, 2, 3]
    arktest.assertEQ(actual.length, 3)
    for (let i = 0; i < actual.length; i++) {
        arktest.assertEQ(actual[i], expectedKeys[i])
    }

    // ignores holes
    let actualVals = Array.from<int>(x.values())
    let expectedVals: Array<int> = [1, 3, 5]
    arktest.assertEQ(actual.length, 3)
    for (let i = 0; i < actual.length; i++) {
        arktest.assertEQ(actualVals[i], expectedVals[i])
    }

    // ignores holes
    let actualEntries = Array.from<[int, int]>(x.entries())
    let expectedEntries: Array<[int, int]> = [[1, 1], [2, 3], [3, 5]]
    for (let i = 0; i < actual.length; i++) {
        arktest.assertEQ(actualEntries[i][0], expectedEntries[i][0])
        arktest.assertEQ(actualEntries[i][1], expectedEntries[i][1])
    }
}

function testFlat() {
    let inner = new SparseArray<int>(44, 55, 66)
    let x = new SparseArray<int | SparseArray<int>>(1)
    x.push(1, 3, 5)
    x.push(inner)
    let expected: Array<int> = [1, 3, 5, 44, 55, 66]
    checkExpected(x.flat<int>(), expected)

    // ignores holes
    let expectedFlatMap: Array<string> = ['1', '3', '5', '44,55,66']
    checkExpected(x.flatMap<string>((val, i, arr) => { return val.toString() }), expectedFlatMap)
}

function testCopyWithIn() {
    let x = new SparseArray<string>('1', '2', 'b', 'c', 'd')
    x.copyWithin(1, 2)
    checkExpected(x, ['1', 'b', 'c', 'd', 'd'])
    x.copyWithin(0, 2, 3)
    checkExpected(x, ['c', 'b', 'c', 'd', 'd'])
}

function testSortUnion() {
    let x = new SparseArray<long | string>(6, 1, 3, '2', '3', '2', 5)
    checkExpected(x.toSorted(), [1, '2', '2', 3, '3', 5, 6])
    x.sort()
    checkExpected(x, [1, '2', '2', 3, '3', 5, 6])

    x.sort(() => { return 0 })
    checkExpected(x, [1, '2', '2', 3, '3', 5, 6])
}

function testSortLong() {
    let x = new SparseArray<long>(6, 1, 3, 5, Long.MAX_VALUE, Long.MIN_VALUE)
    checkExpected(x.toSorted(), [Long.MIN_VALUE, 1, 3, 5, 6, Long.MAX_VALUE])
    x.sort()
    checkExpected(x, [Long.MIN_VALUE, 1, 3, 5, 6, Long.MAX_VALUE])

    x.sort(() => { return 0 })
    checkExpected(x, [Long.MIN_VALUE, 1, 3, 5, 6, Long.MAX_VALUE])
}

function testLengthSet() {
    let x = new SparseArray<int>(1, 2, 3, 4)
    x.length = 2
    arktest.assertEQ(x.length, 2)
    arktest.assertEQ(x[3], undefined)
    arktest.assertEQ(x[4], undefined)
    x.length = 6
    arktest.assertEQ(x.length, 6)
    x.length = 0
    x.forEach(() => { arktest.assertTrue(false, 'Array must be empty') })
}

function testHoles() {
    let x = new SparseArray<int | undefined>()
    x[0] = 1
    x[2] = 3

    
    arktest.assertEQ(x.findLastIndex((x) => { return x === undefined }), -1)
    arktest.assertEQ(x.findIndex((x) => { return x === undefined }), -1)
    arktest.assertFalse(x.includes(undefined), 'there is no undefined')
    arktest.assertEQ(x.indexOf(undefined), -1)
    arktest.assertEQ(x.lastIndexOf(undefined), -1)
    
    x[5] = undefined
    arktest.assertEQ(x.findLastIndex((x) => { return x === undefined }), 5)
    arktest.assertEQ(x.findIndex((x) => { return x === undefined }), 5)
    arktest.assertTrue(x.includes(undefined), 'there is undefined')
    arktest.assertEQ(x.lastIndexOf(undefined), 5)
}

function testToLocaleString() {
    const locale = 'zh-CN'
    const numOptions: Intl.NumberFormatOptions = { style: 'currency', currency: 'EUR' }
    const nums = new SparseArray<double>(1, 2, 3)
    const resultNum = nums.toLocaleString(locale, numOptions)
    arktest.assertTrue(resultNum.includes('€'), 'Number array should include Euro symbol')
    arktest.assertTrue(resultNum.includes(','), 'Number array should separate elements with commas')
    arktest.assertTrue(resultNum.includes('1'), 'Number array should contains 1')
    arktest.assertTrue(resultNum.includes('2'), 'Number array should contains 2')
    arktest.assertTrue(resultNum.includes('3'), 'Number array should contains 3')

    const bigints = new SparseArray<bigint>(12345678901234567890n, 42n)
    const bigintOptions: BigIntToLocaleStringOptions = { useGrouping: false }
    const resultBigint = bigints.toLocaleString(locale, bigintOptions)
    arktest.assertEQ(resultBigint, '12345678901234567890,42');

    const dates = new SparseArray<Date>(new Date(2024, 1, 19, 10, 0, 0), new Date(2024, 1, 19, 15, 30, 0))
    const dateOptions: Intl.DateTimeFormatOptions = { hour: '2-digit', minute: '2-digit', second: '2-digit' }
    const resultDate = dates.toLocaleString(locale, dateOptions)
    arktest.assertEQ(resultDate, '10:00:00,15:30:00')
}
