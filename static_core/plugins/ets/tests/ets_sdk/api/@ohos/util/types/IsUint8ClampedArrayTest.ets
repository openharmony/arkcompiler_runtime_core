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
    const suite = new ArkTestsuite("CommonUtil types.isUint8ClampedArray() API tests")
    
    suite.addTest("Verify Uint8ClampedArray is identified correctly", testIsUint8ClampedArray001)
    suite.addTest("Verify Int8Array is not identified as Uint8ClampedArray", testIsUint8ClampedArray002)
    suite.addTest("Verify Int16Array is not identified as Uint8ClampedArray", testIsUint8ClampedArray003)
    suite.addTest("Verify Int32Array is not identified as Uint8ClampedArray", testIsUint8ClampedArray004)
    suite.addTest("Verify Uint32Array is not identified as Uint8ClampedArray", testIsUint8ClampedArray005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT8_CLAMPED_ARRAY_001
 * @tc.name: testIsUint8ClampedArray001
 * @tc.desc: Verify Uint8ClampedArray is identified correctly
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint8ClampedArray001() {
    let proc = new util.types();
    let result = proc.isUint8ClampedArray(new Uint8ClampedArray());
    assertTrue(result, "The Uint8ClampedArray should be of the same type as the Uint8ClampedArray");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT8_CLAMPED_ARRAY_002
 * @tc.name: testIsUint8ClampedArray002
 * @tc.desc: Verify Int8Array is not identified as Uint8ClampedArray
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint8ClampedArray002() {
    let proc = new util.types();
    let result = proc.isUint8ClampedArray(new Int8Array());
    assertTrue(result == false, "The Int8Array should be of the same type as the Int8Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT8_CLAMPED_ARRAY_003
 * @tc.name: testIsUint8ClampedArray003
 * @tc.desc: Verify Int16Array is not identified as Uint8ClampedArray
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint8ClampedArray003() {
    let proc = new util.types();
    let result = proc.isUint8ClampedArray(new Int16Array());
    assertTrue(result == false, "The Int16Array should be of the same type as the Int16Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT8_CLAMPED_ARRAY_004
 * @tc.name: testIsUint8ClampedArray004
 * @tc.desc: Verify Int32Array is not identified as Uint8ClampedArray
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint8ClampedArray004() {
    let proc = new util.types();
    let result = proc.isUint8ClampedArray(new Int32Array());
    assertTrue(result == false, "The Int32Array should be of the same type as the Int32Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT8_CLAMPED_ARRAY_005
 * @tc.name: testIsUint8ClampedArray005
 * @tc.desc: Verify Uint32Array is not identified as Uint8ClampedArray
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint8ClampedArray005() {
    let proc = new util.types();
    let result = proc.isUint8ClampedArray(new Uint32Array());
    assertTrue(result == false, "The Uint32Array should be of the same type as the Uint32Array");
}
