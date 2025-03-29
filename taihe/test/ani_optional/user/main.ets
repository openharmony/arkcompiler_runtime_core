/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import * as OptTest from "../author_generated/opt";

loadLibrary("ani_optional");

function showOptionalInt() {
    OptTest.showOptionalInt(100 as int)
}

function showOptionalInt_1() {
    OptTest.showOptionalInt(undefined)
}

function makeOptionalInt() {
    let res = OptTest.makeOptionalInt(true)
    console.log(res)
    assertTrue(res != undefined);
}

function makeOptionalInt_1() {
    let res = OptTest.makeOptionalInt(false)
    console.log(res)
    assertTrue(res == undefined);
}

function makeOptionalArray() {
    let res = OptTest.makeOptionalArray(true, 5, 3)
    console.log(res)
    assertTrue(res != undefined);
}

function makeOptionalArray_1() {
    let res = OptTest.makeOptionalArray(false, 5, 3)
    console.log(res)
    assertTrue(res == undefined);
}

function sendReturnResult() {
    let result: OptTest.ReturnResult = {results: ["ab", "cd", "ef"]};
    let res = OptTest.sendReturnResult(result)
    console.log(res)
    assertTrue(res != undefined);
}

function sendReturnResult_1() {
    let result: OptTest.ReturnResult = {results: undefined};
    let res = OptTest.sendReturnResult(result)
    console.log(res)
    assertTrue(res == undefined);
}

function main() {
    console.log("run main ... OptTest ...")

    const suite = new ArkTestsuite("OptTest")
    suite.addTest("showOptionalInt", showOptionalInt)
    suite.addTest("showOptionalInt_1", showOptionalInt_1)
    suite.addTest("makeOptionalInt", makeOptionalInt)
    suite.addTest("makeOptionalInt_1", makeOptionalInt_1)
    suite.addTest("makeOptionalArray", makeOptionalArray)
    suite.addTest("makeOptionalArray_1", makeOptionalArray_1)
    suite.addTest("sendReturnResult", sendReturnResult)
    suite.addTest("sendReturnResult_1", sendReturnResult_1)
    suite.run()
}