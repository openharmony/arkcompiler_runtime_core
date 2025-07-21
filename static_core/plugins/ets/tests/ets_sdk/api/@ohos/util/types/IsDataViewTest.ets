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
    const suite = new ArkTestsuite("CommonUtil types.isDataView() API tests")

    suite.addTest("Verify DataView is identified as DataView", testIsDataView001)
    suite.addTest("Verify Int8Array is not identified as DataView", testIsDataView002)
    suite.addTest("Verify Float64Array is not identified as DataView", testIsDataView003)
    suite.addTest("Verify Boolean is not identified as DataView", testIsDataView004)
    suite.addTest("Verify Boolean is not identified as DataView", testIsDataView005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0046
 * @tc.name: testIsDataView001
 * @tc.desc: Verify DataView is identified as DataView
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsDataView001() {
  let proc = new util.types();
  const ab = new ArrayBuffer(20);
  let result = proc.isDataView(new DataView(ab));
  assertTrue(result, "The DataView should be of the same type as the DataView");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0047
 * @tc.name: testIsDataView002
 * @tc.desc: Verify Int8Array is not identified as DataView
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsDataView002() {
  let proc = new util.types();
  let result = proc.isDataView(new Int8Array());
  assertTrue(result == false, "The Int8Array should not be of the same type as the DataView");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0048
 * @tc.name: testIsDataView003
 * @tc.desc: Verify Float64Array is not identified as DataView
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsDataView003() {
  let proc = new util.types();
  let result = proc.isDataView(new Float64Array());
  assertTrue(result == false, "The Float64Array should not be of the same type as the DataView");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0049
 * @tc.name: testIsDataView004
 * @tc.desc: Verify Boolean is not identified as DataView
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsDataView004() {
  let proc = new util.types();
  let result = proc.isDataView(true);
  assertTrue(result == false, "The primitive Boolean should not be of the same type as the DataView");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0050
 * @tc.name: testIsDataView005
 * @tc.desc: Verify Boolean is not identified as DataView
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsDataView005() {
  let proc = new util.types();
  let result = proc.isDataView(false);
  assertTrue(result == false, "The primitive Boolean should not be of the same type as the DataView");
}
