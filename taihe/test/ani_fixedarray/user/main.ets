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
import * as hello from "hello";

loadLibrary("ani_fixedarray");

function testFixedArrayBoolean(
    test: hello.TestInterface, booleanArray: hello.FixedArray<boolean>) {
    test.setFixedArrayBoolean(booleanArray);
    let result = test.getFixedArrayBoolean();
    arktest.assertEQ(booleanArray.length, result.length);
    for (let i = 0; i < booleanArray.length; i++) {
        arktest.assertEQ(booleanArray[i], result[i]);
    }
}

function testFixedArrayNumber(
    test: hello.TestInterface, numberArray: hello.FixedArray<number>) {
    test.setFixedArrayNumber(numberArray);
    let result = test.getFixedArrayNumber();
    arktest.assertEQ(numberArray.length, result.length);
    for (let i = 0; i < numberArray.length; i++) {
        arktest.assertEQ(numberArray[i], result[i]);
    }
}

function testFixedArrayString(
    test: hello.TestInterface, stringArray: hello.FixedArray<string>) {
    test.setFixedArrayString(stringArray);
    let result = test.getFixedArrayString();
    arktest.assertEQ(stringArray.length, result.length);
    for (let i = 0; i < stringArray.length; i++) {
        arktest.assertEQ(stringArray[i], result[i]);
    }
}

function testFixedArrayData(
    test: hello.TestInterface, dataArray: hello.FixedArray<hello.Data>) {
    test.setFixedArrayData(dataArray);
    let result = test.getFixedArrayData();
    arktest.assertEQ(dataArray.length, result.length);
}

function testOptionalFixedArrayData(
    test: hello.TestInterface,
    optionalDataArray: hello.FixedArray<hello.Data>|undefined) {
    test.setOptionalFixedArrayData(optionalDataArray);
    let result = test.getOptionalFixedArrayData();
    if (optionalDataArray === undefined) {
        arktest.assertEQ(result, undefined);
    } else {
        arktest.assertEQ(optionalDataArray.length, result?.length);
    }
}

function main() {
    const suite = new arktest.ArkTestsuite("FixedArray Test");

    let test = new hello.TestInterface();

    suite.addTest("FixedArrayBoolean", () => {
        testFixedArrayBoolean(test, [true, false, true]);
    });
    suite.addTest("FixedArrayNumber", () => {
        testFixedArrayNumber(test, [1.1, 2.2, 3.3]);
    });
    suite.addTest("FixedArrayString", () => {
        testFixedArrayString(test, ["hello", "world", "test"]);
    });
    suite.addTest("FixedArrayData", () => {
        testFixedArrayData(test, [{}, {}, {}]);
    });
    suite.addTest("OptionalFixedArrayData", () => {
        testOptionalFixedArrayData(test, [{}, {}, {}]);
    });
    suite.addTest("OptionalFixedArrayData", () => {
        testOptionalFixedArrayData(test, undefined);
    });

    exit(suite.run());
}
