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
    const suite = new ArkTestsuite("CommonUtil types.isUint32Array() API tests")
    
    suite.addTest("Verify Uint32Array is identified as Uint32Array", testIsUint32Array001)
    suite.addTest("Verify Uint8Array is not identified as Uint32Array", testIsUint32Array002)
    suite.addTest("Verify Uint16Array is not identified as Uint32Array", testIsUint32Array003)
    suite.addTest("Verify Int8Array is not identified as Uint32Array", testIsUint32Array004)
    suite.addTest("Verify Uint8ClampedArray is not identified as Uint32Array", testIsUint32Array005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT32_ARRAY_001
 * @tc.name: testIsUint32Array001
 * @tc.desc: Verify Uint32Array is identified as Uint32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint32Array001() {
    let proc = new util.types();
    let result = proc.isUint32Array(new Uint32Array());
    assertTrue(result, "The Uint32Array should be of the same type as the Uint32Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT32_ARRAY_002
 * @tc.name: testIsUint32Array002
 * @tc.desc: Verify Uint8Array is not identified as Uint32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint32Array002() {
    let proc = new util.types();
    let result = proc.isUint32Array(new Uint8Array());
    assertTrue(result == false, "The Uint8Array should be of the same type as the Uint8Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT32_ARRAY_003
 * @tc.name: testIsUint32Array003
 * @tc.desc: Verify Uint16Array is not identified as Uint32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint32Array003() {
    let proc = new util.types();
    let result = proc.isUint32Array(new Uint16Array());
    assertTrue(result == false, "The Uint16Array should be of the same type as the Uint16Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT32_ARRAY_004
 * @tc.name: testIsUint32Array004
 * @tc.desc: Verify Int8Array is not identified as Uint32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint32Array004() {
    let proc = new util.types();
    let result = proc.isUint32Array(new Int8Array());
    assertTrue(result == false, "The Int8Array should be of the same type as the Int8Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_UINT32_ARRAY_005
 * @tc.name: testIsUint32Array005
 * @tc.desc: Verify Uint8ClampedArray is not identified as Uint32Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsUint32Array005() {
    let proc = new util.types();
    let result = proc.isUint32Array(new Uint8ClampedArray());
    assertTrue(result == false, "The Uint8ClampedArray should be of the same type as the Uint8ClampedArray");
}
