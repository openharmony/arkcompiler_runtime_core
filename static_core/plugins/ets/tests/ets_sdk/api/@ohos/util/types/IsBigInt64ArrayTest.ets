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
    const suite = new ArkTestsuite("CommonUtil types.isBigInt64Array() API tests")

    suite.addTest("Verify BigInt64Array is identified as BigInt64Array", testIsBigInt64Array001)
    suite.addTest("Verify Int32Array is not identified as BigInt64Array", testIsBigInt64Array002)
    suite.addTest("Verify Uint8Array is not identified as BigInt64Array", testIsBigInt64Array003)
    suite.addTest("Verify Float64Array is not identified as BigInt64Array", testIsBigInt64Array004)
    suite.addTest("Verify Int8Array is not identified as BigInt64Array", testIsBigInt64Array005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0025
 * @tc.name: testIsBigInt64Array001
 * @tc.desc: Verify BigInt64Array is identified as BigInt64Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsBigInt64Array001() {
  let proc = new util.types();
  let result = proc.isBigInt64Array(new BigInt64Array());
  assertTrue(result, "The BigInt64Array should be of the same type as the BigInt64Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0026
 * @tc.name: testIsBigInt64Array002
 * @tc.desc: Verify Int32Array is not identified as BigInt64Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsBigInt64Array002() {
  let proc = new util.types();
  let result = proc.isBigInt64Array(new Int32Array());
  assertTrue(result == false, "The Int32Array should not be of the same type as the BigInt64Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0027
 * @tc.name: testIsBigInt64Array003
 * @tc.desc: Verify Uint8Array is not identified as BigInt64Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsBigInt64Array003() {
  let proc = new util.types();
  let result = proc.isBigInt64Array(new Uint8Array());
  assertTrue(result == false, "The Uint8Array should not be of the same type as the BigInt64Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0028
 * @tc.name: testIsBigInt64Array004
 * @tc.desc: Verify Float64Array is not identified as BigInt64Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsBigInt64Array004() {
  let proc = new util.types();
  let result = proc.isBigInt64Array(new Float64Array());
  assertTrue(result == false, "The Float64Array should not be of the same type as the BigInt64Array");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0029
 * @tc.name: testIsBigInt64Array005
 * @tc.desc: Verify Int8Array is not identified as BigInt64Array
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsBigInt64Array005() {
  let proc = new util.types();
  let result = proc.isBigInt64Array(new Int8Array());
  assertTrue(result == false, "The Int8Array should not be of the same type as the BigInt64Array");
}
