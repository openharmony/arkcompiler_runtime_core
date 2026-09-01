/**
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

import { util } from "@ohos.util"

// Named functions for testing %o/%O with function name
function namedFunc(): void {}
function namedFunc1(size: int): void {}
function namedFunc2(a: int, b: string): boolean { return true }

class TestObject1 {
    x: int = 5
    y: int = 6
}

class TestObject2 {
    strField: string
    intField: int
    floatField: float
    boolField: boolean = false
    constructor(strF: string, intF: int, floatF: float) {
        this.strField = strF
        this.intField = intF
        this.floatField = floatF
    }
}

class TestObject3 {
    prop: string = ""
    reference: Object = 0
}

class TestObject4 {
    f1: string = ""
    k1: int = 8
    t: boolean = false
}

class Address {
    city: string = 'New York'
    country: string = 'USA'
}

class UtilPersontype {
    name: string = 'John'
    age: int = 20
    address: Address = {}
}

function main(): int {
    const suite = new arktest.ArkTestsuite("util.format API tests")

    suite.addTest("util.format %s %d %f", testUtilFormat)
    suite.addTest("util.format with less arguments than format specifiers", testFormatLessArgsThanInFormat)
    suite.addTest("util.format with incorrect argument flags", testFormatIncorrectArgs)
    suite.addTest("util.format with no matching flags and arguments", testFormatNoMatchArgs)
    suite.addTest("util.format with more args than format specifiers", testFormatMoreArgsThanInUtilFormat)
    suite.addTest("util.format %j JSON", testFormatJSON)
    suite.addTest("util.format %% escape percent", testFormatEscapePercent)
    suite.addTest("util.format with args and empty format string", testEmptyFormatWithArgs)
    suite.addTest("util.format with args and non-empty format string", testNonEmptyFormatWithArgs)
    suite.addTest("util.format without args", testFormatWithoutArgs)
    suite.addTest("util.format %o Object with showHidden", testFormatObject)
    suite.addTest("util.format %O Object", testFormatObject2)
    suite.addTest("util.format %o/%O Array object", testFormatArrayObect)
    suite.addTest("util.format %o/%O anonymous Function", testFunctionAsObject)
    suite.addTest("util.format %o/%O named Function", testNamedFunctionAsObject)
    suite.addTest("util.format %o/%O Array of named functions", testNamedFunctionArray)
    suite.addTest("util.format %o/%O Array of anonymous functions", testFunctionsArrayObject)
    suite.addTest("util.format %o/%O Object with circular reference", testFormatCircularObject)
    suite.addTest("util.format %o/%O Array with circular reference objects", testArrayOfCircularObjects)
    suite.addTest("util.format api v1 tests", testApiV1Tests)

    return suite.run()
}

function testUtilFormat() {
    const s: string = "computer"
    const i: int = 35
    const fp: float = 1.7320534f

    arktest.assertEQ(util.format("String: %s", s), "String: computer")
    arktest.assertEQ(util.format("%s", s), "computer")
    arktest.assertEQ(util.format("%s String", s), "computer String")
    arktest.assertEQ(util.format("%s ", s), "computer ")
    arktest.assertEQ(util.format(" %s", s), " computer")
    arktest.assertEQ(util.format("%%s", s), "%s computer")
    arktest.assertEQ(util.format("%%s%", s), "%s% computer")
    arktest.assertEQ(util.format("s%", s), "s% computer")
    arktest.assertEQ(util.format("%s%", s), "computer%")

    arktest.assertEQ(util.format("   Integer:   %d\n", i), "   Integer:   35\n")
    arktest.assertEQ(util.format("   Real:      %f\n", fp), "   Real:      1.7320534\n")

    arktest.assertEQ(util.format("String: %s int %d !!! %d %f %d", s, 1, 2, 3, 4), "String: computer int 1 !!! 2 3 4")
}

function testFormatLessArgsThanInFormat() {
    let res = util.format("example %d of invalid %f format string", 1)
    arktest.assertEQ(res, "example 1 of invalid %f format string")
}

function testFormatIncorrectArgs() {
    let res = util.format("example %X of invalid format string", 1)
    arktest.assertEQ(res, "example %X of invalid format string 1")
}

function testFormatNoMatchArgs() {
    // NOTE(ekaterinazaytseva): converting string to int -> NaN
    let res = util.format("example %s of invalid %d format string %d", "string", 1, "string instead int")
    arktest.assertEQ(res, "example string of invalid 1 format string NaN")

    let res1 = util.format("example %s of invalid %d format string %o", "string", 1, "string as object")
    arktest.assertEQ(res1, "example string of invalid 1 format string 'string as object'")
}

function testFormatMoreArgsThanInUtilFormat() {
    let res = util.format("Text: %s", "ddd", 44, "one more time text")
    arktest.assertEQ(res, "Text: ddd 44 one more time text")
}

function testFormatJSON() {
    // simple JSON
    arktest.assertEQ(util.format("JSON : %j", new TestObject1()), "JSON : {\"x\":5,\"y\":6}")

    // JSON with circular reference
    let jsonWithCircularRef: string = "{\"name\":\"Kris\", \"father\":{\"name\":\"Bill\",\"wife\":{\"name\":\"Karen\"}},\"mother\":{\"$ref\":\"#father.wife\"}}"
    let expected = "JSON : \"{\\\"name\\\":\\\"Kris\\\", \\\"father\\\":{\\\"name\\\":\\\"Bill\\\",\\\"wife\\\":{\\\"name\\\":\\\"Karen\\\"}},\\\"mother\\\":{\\\"$ref\\\":\\\"#father.wife\\\"}}\""
    let res = util.format("JSON : %j", jsonWithCircularRef)
    arktest.assertEQ(res, expected)
}

function testFormatObject() {
    let res = util.format("Object: %o", new TestObject2("k28", 14, 0.1f))
    let expected = "Object: { strField: 'k28',\nintField: 14,\nfloatField: 0.1,\nboolField: false }"
    arktest.assertEQ(res, expected)
}

function testFormatObject2() {
    let res = util.format("Object: %O", new TestObject2("k28", 14, 0.1f))
    let expected = "Object: { strField: 'k28',\nintField: 14,\nfloatField: 0.1,\nboolField: false }"
    arktest.assertEQ(res, expected)
}

function testFormatCircularObject() {
    const circularObject = new TestObject3()
    circularObject.prop = "Circular Reference Example"
    circularObject.reference = circularObject

    let expected = "{ prop: 'Circular Reference Example',\nreference: [Circular]\n }"

    let res = util.format("%o", circularObject)
    arktest.assertEQ(res, expected)

    res = util.format("%O", circularObject)
    arktest.assertEQ(res, expected)
}

function testArrayOfCircularObjects() {
    const circularObject = new TestObject3()
    circularObject.prop = "Circular Reference Example"
    circularObject.reference = circularObject

    let arr: TestObject3[] = [circularObject, circularObject]

    let expected = "[ { prop: 'Circular Reference Example',\n  reference: [Circular]\n   }, \n{ prop: 'Circular Reference Example',\n  reference: [Circular]\n   }, [length]: 2 ]"
    let expected1 = "[ { prop: 'Circular Reference Example',\n  reference: [Circular]\n   }, \n{ prop: 'Circular Reference Example',\n  reference: [Circular]\n   } ]"

    let res = util.format("%o", arr)
    arktest.assertEQ(res, expected)

    res = util.format("%O", arr)
    arktest.assertEQ(res, expected1)
}

function testEmptyFormatWithArgs() {
    let res = util.format("", 1, 2, 3, "test")
    arktest.assertEQ(res, "1 2 3 test")
}

function testNonEmptyFormatWithArgs() {
    let res = util.format("Some important text!", 1, 2, 3, "test")
    arktest.assertEQ(res, "Some important text! 1 2 3 test")
}

function testFormatWithoutArgs() {
    let res = util.format("%d %s  lalala %d %d")
    arktest.assertEQ(res, "%d %s  lalala %d %d")
}

function testFormatEscapePercent() {
    let res = util.format("%%  lalala %%%%")
    arktest.assertEQ(res, "%  lalala %%")
}

function testFormatArrayObect() {
    let arr = new Array<TestObject4>()
    arktest.assertEQ(util.format('Array: %o', arr), "Array: [ [length]: 0 ]")
    arktest.assertEQ(util.format('Array: %O', arr), "Array: [  ]")

    let arr1: TestObject4[] = [new TestObject4()]
    let expected = "Array: [ { f1: '',\n  k1: 8,\n  t: false }, [length]: 1 ]"
    let expected1 = "Array: [ { f1: '',\n  k1: 8,\n  t: false } ]"
    arktest.assertEQ(util.format('Array: %o', arr1), expected)
    arktest.assertEQ(util.format('Array: %O', arr1), expected1)
}

function testFunctionAsObject() {
    let func = (arg: string): string => { return "XX" + arg }

    let expected = "Function: { [Function: func]  [length]: 0,  [name] :'func',  [prototype]: func { [constructor]: [Circular] } }"
    let expected1 = "Function: { [Function: func] }"

    arktest.assertEQ(util.format('Function: %o', func), expected)
    arktest.assertEQ(util.format('Function: %O', func), expected1)
}

function testNamedFunctionAsObject() {
    // %o with named function (0 params)
    let res0 = util.format('NamedFunc: %o', namedFunc)
    arktest.assertTrue(res0.includes('namedFunc'), '%o namedFunc should include function name')
    arktest.assertTrue(res0.includes('[Function: namedFunc]'), '%o namedFunc should include [Function: namedFunc]')

    // %O with named function (0 params)
    let res0b = util.format('NamedFunc: %O', namedFunc)
    arktest.assertTrue(res0b.includes('namedFunc'), '%O namedFunc should include function name')

    // %o with named function (1 param)
    let res1 = util.format('NamedFunc1: %o', namedFunc1)
    arktest.assertTrue(res1.includes('namedFunc1'), '%o namedFunc1 should include function name')
    arktest.assertTrue(res1.includes('[Function: namedFunc1]'), '%o namedFunc1 should include [Function: namedFunc1]')

    // %o with named function (2 params)
    let res2 = util.format('NamedFunc2: %o', namedFunc2)
    arktest.assertTrue(res2.includes('namedFunc2'), '%o namedFunc2 should include function name')
    arktest.assertTrue(res2.includes('[Function: namedFunc2]'), '%o namedFunc2 should include [Function: namedFunc2]')

    // %s with named function - uses toString(), not same as %o
    let resStr = util.format('Func str: %s', namedFunc)
    arktest.assertTrue(!resStr.includes('[Function:'), '%s should not format as %o')
}

function testNamedFunctionArray() {
    let arr = new Array<() => void>(namedFunc)

    let res = util.format('Array of named: %o', arr)
    arktest.assertTrue(res.includes('namedFunc'), '%o array of named functions should include function name')
    arktest.assertTrue(res.includes('[Function: namedFunc]'), '%o array should include [Function: namedFunc]')

    let res1 = util.format('Array of named: %O', arr)
    arktest.assertTrue(res1.includes('namedFunc'), '%O array of named functions should include function name')
}

function testFunctionsArrayObject() {
    let lambda = (arg: string): string => { return "XX" + arg }

    let arr = new Array<(arg: string) => string>(lambda)

    let expected = "Array of functions: [ { [Function: lambda]  [length]: 0,  [name] :'lambda',  [prototype]: lambda { [constructor]: [Circular] } }, [length]: 1 ]"
    let expected1 = "Array of functions: [ [Function: lambda] ]"

    arktest.assertEQ(util.format('Array of functions: %o', arr), expected)
    arktest.assertEQ(util.format('Array of functions: %O', arr), expected1)
}

function testApiV1Tests() {
    let name = 'John'
    let age = 20
    let formattedString = util.format('My name is %s and I am %s years old', name, age)
    arktest.assertEQ(formattedString, "My name is John and I am 20 years old")

    let num = 10.5
    formattedString = util.format('The number is %d', num)
    arktest.assertEQ(formattedString, "The number is 10.5")

    num = 100.5
    formattedString = util.format('The number is %i', num)
    arktest.assertEQ(formattedString, "The number is 100")

    const pi = 3.141592653
    formattedString = util.format('The value of pi is %f', pi)
    arktest.assertEQ(formattedString, "The value of pi is 3.141592653")

    const person = new UtilPersontype()
    let expected1 = "Formatted object using %O: { name: 'John',\nage: 20,\naddress:\n{ city: 'New York',\n  country: 'USA' } }"
    let expected2 = "Formatted object using %o: { name: 'John',\nage: 20,\naddress:\n{ city: 'New York',\n  country: 'USA' } }"

    arktest.assertEQ(util.format('Formatted object using %%O: %O', person), expected1)
    arktest.assertEQ(util.format('Formatted object using %%o: %o', person), expected2)

    const percentage = 80
    let arg = 'homework'
    formattedString = util.format('John finished %d%% of the %s', percentage, arg)
    arktest.assertEQ(formattedString, "John finished 80% of the homework")
}
