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
    TReturnsStrArray,
    TReturnsNumArray,
    TReturnsRecord,
    isTTupleTS,

    returnTuple,
    returnStrArray,
    returnNumArray,
    returnRecord,
    returnInterface
} from 'js_code';

const TEST_STRING: string = 'This is a test string'
const TEST_INT: int = 100
const TEST_BOOLEAN: boolean = true
const EXP_STR_ARRAY: FixedArray<string> = ["One", "Two", "Three"]
const EXP_NUM_ARRAY: FixedArray<int> = [1, 2, 3]

type ETSTupleType = [ string, int, boolean ]

function TestReturnTuple(): boolean {
    const actTuple: ETSTupleType = returnTuple() as ETSTupleType

    return (actTuple instanceof ETSTupleType 
        && isTTupleTS(actTuple) as boolean
        && actTuple[0] as string == TEST_STRING
        && actTuple[1] as int == TEST_INT
        && actTuple[2] as boolean == TEST_BOOLEAN
    )
}

function TestReturnStrArray(): boolean {
    const actArray = returnStrArray() as TReturnsStrArray
    return (
        actArray[0] == EXP_STR_ARRAY[0]
        && actArray[1] == EXP_STR_ARRAY[1]
        && actArray[2] == EXP_STR_ARRAY[2]
    )
}

function TestReturnNumArray(): boolean {
    const actArray = returnNumArray() as TReturnsNumArray
    return (
        actArray[0] as int == EXP_NUM_ARRAY[0]
        && actArray[1] as int == EXP_NUM_ARRAY[1]
        && actArray[2] as int == EXP_NUM_ARRAY[2]
    )
}

function TestReturnRecord(): boolean {
    const actRecord = returnRecord() as TReturnsRecord
    return (
        actRecord.one as int == 1
        && actRecord.two as int == 2
        && actRecord.three as int == 3
    )
}

interface Shape {
    width: int
}
   
interface TwoDimensioned extends Shape {
    length: int
}
   
interface ThreeDimensioned extends Shape {
    length: int
    height: int
}

function TestReturnInterface(): boolean {
    const testInterface = returnInterface
    const testTwoVal = testInterface(false) as TwoDimensioned
    const testThreeVal = testInterface(true) as ThreeDimensioned

    return (testTwoVal.width as int == 100 && testTwoVal.length as int == 40 &&
            testThreeVal.width as int == 100 && testThreeVal.length as int == 50 && testThreeVal.height as int == 25)
}