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

import {util} from "@ohos.util";

function main(): int {
    const suite = new ArkTestsuite("CommonUtil types.isArrayBuffer() API tests")

    suite.addTest("Verify ArrayBuffer is identified as ArrayBuffer", testIsArrayBuffer001)
    suite.addTest("Verify Boolean is not identified as ArrayBuffer", testIsArrayBuffer002)
    suite.addTest("Verify Boolean is not identified as ArrayBuffer", testIsArrayBuffer003)
    suite.addTest("Verify Int8Array is not identified as ArrayBuffer", testIsArrayBuffer004)
    suite.addTest("Verify Int16Array is not identified as ArrayBuffer", testIsArrayBuffer005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0016
 * @tc.name: testIsArrayBuffer001
 * @tc.desc: Verify ArrayBuffer is identified as ArrayBuffer
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsArrayBuffer001() {
  let proc = new util.types();
  let result = proc.isArrayBuffer(new ArrayBuffer(0));
  assertTrue(result, "The ArrayBuffer should be of the same type as the ArrayBuffer");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0017
 * @tc.name: testIsArrayBuffer002
 * @tc.desc: Verify Boolean is not identified as ArrayBuffer
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsArrayBuffer002() {
  let proc = new util.types();
  let result = proc.isArrayBuffer(true);
  assertTrue(result == false, "The Boolean should not be of the same type as the ArrayBuffer");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0018
 * @tc.name: testIsArrayBuffer003
 * @tc.desc: Verify Boolean is not identified as ArrayBuffer
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsArrayBuffer003() {
  let proc = new util.types();
  let result = proc.isArrayBuffer(false);
  assertTrue(result == false, "The Boolean should not be of the same type as the ArrayBuffer");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0019
 * @tc.name: testIsArrayBuffer004
 * @tc.desc: Verify Int8Array is not identified as ArrayBuffer
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsArrayBuffer004() {
  let proc = new util.types();
  let result = proc.isArrayBuffer(new Int8Array());
  assertTrue(result == false, "The Int8Array should not be of the same type as the ArrayBuffer");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0020
 * @tc.name: testIsArrayBuffer005
 * @tc.desc: Verify Int16Array is not identified as ArrayBuffer
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsArrayBuffer005() {
  let proc = new util.types();
  let result = proc.isArrayBuffer(new Int16Array());
  assertTrue(result == false, "The Int16Array should not be of the same type as the ArrayBuffer");
}
