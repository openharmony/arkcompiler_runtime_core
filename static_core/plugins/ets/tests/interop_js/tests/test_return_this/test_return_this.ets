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
    TestClass,
    testFuncReturnStr,
    getThis,
    getThisAsFunction,
    getThisAsAsyncFunction
} from 'js_code';

const TEST_NUM_ARRAY: FixedArray<number> = [1, 2, 3, 4, 5]
const TEST_STRING: string = 'Test'
const TEST_NUMBER: number = 100
const TEST_NULL: null = null
const TEST_OBJ: Object = true
const TEST_TUPLE: [string, number] = ['Test', 1]
const TEST_MAP: Map<string,string> = new Map<string,string>()
TEST_MAP.set('TEST', 'test')

type TestTupleType = [string, number] 

function returnThis<T>(arg: T): T {
    return getThis.call(arg) as T
}

function testReturnThisInClass(): boolean {
    let testClass = new TestClass()
    let testVal = testClass.getThis()
    return testClass === testVal
}

function testReturnThisAsNumber(): boolean {
    return getThis.call(TEST_NUMBER) == TEST_NUMBER
}

function testReturnThisAsString(): boolean {
    return getThis.call(TEST_STRING) == TEST_STRING
}

function testReturnThisAsNull(): boolean {
    return getThis.call(TEST_NULL) as NullishType == TEST_NULL
}

function testReturnThisAsMap(): boolean {
    return getThis.call(TEST_MAP) == TEST_MAP
}

function testReturnThisAsTuple(): boolean {
    return returnThis<TestTupleType>(TEST_TUPLE) == TEST_TUPLE
}

function testReturnThisAsArray(): boolean {
    return returnThis<FixedArray<number>>(TEST_NUM_ARRAY) == TEST_NUM_ARRAY
}

function testReturnThisAsObj(): boolean {
    return returnThis<Object>(TEST_OBJ) == TEST_OBJ
}

function testReturnThisAsObjClass(): boolean {
    const testClass = new TestClass()
    const testVal = getThis.call(testClass)
    return testVal === testClass
}

function testReturnThisAsFunc(): boolean {
    const testVal = getThisAsFunction.call(testFuncReturnStr)
    return testVal() == TEST_STRING
}

// Note: Test is commented until #18468 is resolved
/*
function testReturnThisAsAsyncFunc(): boolean {
    console.log("Start TestReturnThisAsAsyncFunc");
    let p = getThisAsAsyncFunction.call(testFuncReturnStr());
    let testVal = "";
    console.log("P = " + p);
    p.then<string | null>((value: string): string | null => {
        testVal = value;
        return null;
    });
    p.catch<string | null>((err: NullishType): string | null => {
        return null;
    });
    return testVal == TEST_STRING;
}
*/
