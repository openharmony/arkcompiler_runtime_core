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

import {
    arrObj, User, es,
    InsertArray, GetArrLength, SetArrLength,
    ARRNUM1, ARRNUM2, ARRNUM3,
    testArr1, testArr2, 
    IsEqual,
} from 'special_function_ts';

const INSERT_NUMBER3: boolean = false;

function testMimicArrAdd() {
    arrObj.push(INSERT_NUMBER3);
    InsertArray(true);
    return arrObj.length === GetArrLength();
}

function testMimicArrLength() {
    SetArrLength(10);
    return arrObj.length === 10;
}

function testGetArray() {
    let numArr: es.Array<number> = testArr1;
    return numArr[0] === ARRNUM1 as number &&
        numArr[1] === ARRNUM2 as number &&
        numArr[2] === ARRNUM3 as number;
}

function testSetArray() {
    let numArr2: es.Array<number> = testArr2;
    numArr2[0] = ARRNUM1 as number;
    numArr2[1] = ARRNUM2 as number;
    numArr2[2] = ARRNUM3 as number;
    return IsEqual();
}

function testCustomizeType() {
    const user = new User("UserA", 128);
    user.userName = "UserB";
    user.userIdx = 256;
    return user.userName as string === user.GetUserName() as string && 
        user.userIdx as number == user.GetUserIdx() as number;
}
