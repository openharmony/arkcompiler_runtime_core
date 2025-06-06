/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

import {
    TReturnsAny,
    TReturnsStrLiteral,
    TReturnsIntLiteral,
    TReturnsBoolLiteral,
    TReturnsBigNLiteral,
    TReturnsString,
    TReturnsMap,
    TTestClass,
    TTestInterface,
    TestClass,

    returnIntegerAsAny,
    returnStringAsAny,
    returnBigIntegerAsAny,
    returnBooleanAsAny,
    returnUndefinedAsAny,
    returnNullAsAny,
    returnMapAsAny,
    returnSetAsAny,

    returnStrLiteral,
    returnIntLiteral,
    returnBoolLiteral,
    returnsBigNLiteral,

    returnMap,
    returnSet,
    returnTuple,
    isTTupleTS,

    returnStringSubsetByRef,
    returnMapSubsetByRef,
    returnStringSubsetByValue,
    returnIntSubsetByValue,
    returnLongIntSubsetByValue,
    returnBoolSubsetByValue,
    returnUndefinedSubsetByValue,
    returnNullSubsetByValue,

    returnUnion,
    returnClass,
    returnInterface
} from 'js_code';

const SAMPLE_STRING = 'This is a test string'
const TEST_LONG_INT = 9007199254740991
const TEST_INT = 100
const TEST_BIG_INT = 10000000n
const TEST_BOOLEAN = true

const TEST_LITERAL_STRING = 'Test'
const TEST_LITERAL_INTEGER = 1
const TEST_LITERAL_BOOLEAN  = false
const TEST_LITERAL_BIGN = 100n

type ETSTupleType = [ string, int, boolean ]

function TestReturnIntegerAsAny(): boolean {
    const TestFunc: TReturnsAny = returnIntegerAsAny
    const testVal = TestFunc() as number
    return testVal == TEST_INT
}

function TestReturnStringAsAny(): boolean {
    const TestFunc: TReturnsAny = returnStringAsAny
    const testVal = TestFunc()
    return testVal == SAMPLE_STRING
}

function TestReturnBigIntegerAsAny(): boolean {
    const TestFunc: TReturnsAny = returnBigIntegerAsAny
    const testVal = TestFunc()
    return testVal == TEST_BIG_INT
}

function TestReturnBooleanAsAny(): boolean {
    const TestFunc: TReturnsAny = returnBooleanAsAny
    const testVal = TestFunc() as boolean
    return testVal == TEST_BOOLEAN
}

// Note: Using NullishType is a workaround until #17745 is not resolved
function TestReturnNullAsAny(): boolean {
    const TestFunc: TReturnsAny = returnNullAsAny
    const testVal = TestFunc() as NullishType
    return testVal == null
}

function TestReturnUndefinedAsAny(): boolean {
    const TestFunc: TReturnsAny = returnUndefinedAsAny
    const testVal = TestFunc()
    return testVal == undefined
}

function TestReturnMapAsAny(): boolean {
    const TestFunc: TReturnsAny = returnMapAsAny
    const map = TestFunc()
    return map.has(SAMPLE_STRING.toUpperCase()) as boolean
}

function TestReturnSetAsAny(): boolean {
    const TestFunc: TReturnsAny = returnSetAsAny
    const set = TestFunc()
    return (
        set.size as int > 0
        && !(set.has(SAMPLE_STRING) as boolean) 
        && set.has(SAMPLE_STRING.toLowerCase()) as boolean
    )
}

function TestReturnStringAsLiteral(): boolean {
    const TestFunc: TReturnsStrLiteral = returnStrLiteral
    const testVal = TestFunc()
    return testVal == TEST_LITERAL_STRING
}

function TestReturnBigNAsLiteral(): boolean {
    const TestFunc: TReturnsBigNLiteral = returnsBigNLiteral
    const testVal = TestFunc()
    return testVal == TEST_LITERAL_BIGN
}

function TestReturnIntAsLiteral(): boolean {
    const TestFunc: TReturnsIntLiteral = returnIntLiteral
    const testVal = TestFunc() as int
    return testVal == TEST_LITERAL_INTEGER
}

function TestReturnBoolAsLiteral(): boolean {
    const TestFunc: TReturnsBoolLiteral = returnBoolLiteral
    const testVal = TestFunc() as boolean
    return testVal == TEST_LITERAL_BOOLEAN
}

function TestReturnMap(): boolean {
    const map = returnMap()
    return map.has(SAMPLE_STRING.toUpperCase()) as boolean
}

function TestReturnSet(): boolean {
    const set = returnSet()
    return (
        set.size as int > 0
        && !(set.has(SAMPLE_STRING) as boolean) 
        && set.has(SAMPLE_STRING.toLowerCase()) as boolean
    )
}

function TestReturnTuple(): boolean {
    const expectedTuple: ETSTupleType = returnTuple() as ETSTupleType
    return (expectedTuple instanceof ETSTupleType && isTTupleTS(expectedTuple) as boolean)
}

function TestReturnStringSubsetByRef(): boolean {
    const TestFunc: TReturnsString = returnStringSubsetByRef
    const testVal = TestFunc() as string
    return (
        testVal == SAMPLE_STRING
        && typeof testVal == 'string'
    )
}

function TestReturnMapSubsetByRef(): boolean {
    const TestFunc: TReturnsMap = returnMapSubsetByRef
    const initialMap = TestFunc()
    const changedMap = TestFunc()

    const initialInitialMapSize: number = initialMap.size
    const initialChangedMapSize: number = changedMap.size
    changedMap.set('2', 'Test')
    const finalInitialMapSize: number = initialMap.size 
    const finalChangedMapSize: number = changedMap.size
    
    return (
        initialInitialMapSize == 1 
        && initialChangedMapSize == 1
        && initialMap.has('2') as boolean
        && finalInitialMapSize == 2
        && finalChangedMapSize == 2
    )
}

function TestReturnStringSubsetByValue(): boolean {
    const testVal  = returnStringSubsetByValue() as string
    return (
        testVal == SAMPLE_STRING
        && typeof testVal == 'string'
    )
}

function TestReturnIntSubsetByValue(): boolean {
    const TestFunc = returnIntSubsetByValue
    const testVal = TestFunc() as int
    return (
        testVal == TEST_INT
        && typeof testVal == 'number'
    )
}

function TestReturnLongIntSubsetByValue(): boolean {
    const TestFunc = returnLongIntSubsetByValue
    const testVal = TestFunc() as long
    return (
        testVal == TEST_LONG_INT
        && typeof testVal == 'number'
    )
}

function TestReturnBoolSubsetByValue(): boolean {
    const TestFunc = returnBoolSubsetByValue
    const testVal = TestFunc() as boolean
    return (
        testVal == TEST_BOOLEAN
        && typeof testVal == 'boolean'
    )
}

// Note: Using NullishType is a workaround until #17745 is not resolved
function TestReturnNullSubsetByValue(): boolean {
    const TestFunc = returnNullSubsetByValue
    const testVal = TestFunc() as NullishType
    return (
        testVal == null
        && typeof testVal == 'object'
    )
}

function TestReturnUndefinedSubsetByValue(): boolean {
    const TestFunc = returnUndefinedSubsetByValue
    const testVal = TestFunc()
    return testVal == undefined
}

function TestReturnUnion(): boolean {
    const TestFunc = returnUnion
    const testValStr = TestFunc(true) as string
    const testValNum = TestFunc(false) as number

    return (
        typeof testValStr == 'string' && testValStr as string == '1000'
        && typeof testValNum == 'number' && testValNum as number == 1000)

}

function TestReturnClass(): boolean {
    const testFunc: TTestClass = returnClass
    const testClassVal = testFunc()
    return (
        testClassVal.testString as string == 'Test'
        && testClassVal.testNumber as number == 1
        && testClassVal.testFunction() as number == 1
        && (TestClass.isTestClass(testClassVal) as boolean) == true
    )
}

function TestReturnInterface(): boolean {
    const TestFunc: TTestInterface = returnInterface
    const testVal = TestFunc()
    return (
        testVal.testString as string == 'Test'
        && testVal.testNumber as number == 100
    )
}
