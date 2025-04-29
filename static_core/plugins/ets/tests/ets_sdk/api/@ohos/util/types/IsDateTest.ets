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
    const suite = new ArkTestsuite("CommonUtil types.isDate() API tests")

    suite.addTest("Verify Date is identified as Date", testIsDate001)
    suite.addTest("Verify Int8Array is not identified as Date", testIsDate002)
    suite.addTest("Verify Boolean is not identified as Date", testIsDate003)
    suite.addTest("Verify Int16Array is not identified as Date", testIsDate004)
    suite.addTest("Verify Float64Array is not identified as Date", testIsDate005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0051
 * @tc.name: testIsDate001
 * @tc.desc: Verify Date is identified as Date
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsDate001() {
  let proc = new util.types();
  let result = proc.isDate(new Date());
  assertTrue(result, "The Date should be of the same type as the Date");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0052
 * @tc.name: testIsDate002
 * @tc.desc: Verify Int8Array is not identified as Date
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsDate002() {
  let proc = new util.types();
  let result = proc.isDate(new Int8Array());
  assertTrue(result == false, "The Int8Array should not be of the same type as the Date");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0053
 * @tc.name: testIsDate003
 * @tc.desc: Verify Boolean is not identified as Date
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsDate003() {
  let proc = new util.types();
  let result = proc.isDate(true);
  assertTrue(result == false, "The primitive Boolean should not be of the same type as the Date");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0054
 * @tc.name: testIsDate004
 * @tc.desc: Verify Int16Array is not identified as Date
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsDate004() {
  let proc = new util.types();
  let result = proc.isDate(new Int16Array());
  assertTrue(result == false, "The Int16Array should not be of the same type as the Date");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0055
 * @tc.name: testIsDate005
 * @tc.desc: Verify Float64Array is not identified as Date
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsDate005() {
  let proc = new util.types();
  let result = proc.isDate(new Float64Array());
  assertTrue(result == false, "The Float64Array should not be of the same type as the Date");
}
