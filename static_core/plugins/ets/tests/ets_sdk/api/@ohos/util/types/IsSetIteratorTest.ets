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
    const suite = new ArkTestsuite("CommonUtil types.isSetIterator() API tests")

    suite.addTest("Verify Set.keys() is identified as SetIterator", testIsSetIterator001)
    suite.addTest("Verify Set.values() is identified as SetIterator", testIsSetIterator002)
    suite.addTest("Verify Set.entries() is identified as SetIterator", testIsSetIterator003)
    suite.addTest("Verify Set.$_iterator() is identified as SetIterator", testIsSetIterator004)
    suite.addTest("Verify Set is not identified as SetIterator", testIsSetIterator005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_SET_ITERATOR_001
 * @tc.name: testIsSetIterator001
 * @tc.desc: Verify Set.keys() is identified as SetIterator
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsSetIterator001() {
  let proc = new util.types();
  const set = new Set<int>();
  let result = proc.isSetIterator(set.keys());
  assertTrue(result, "The SetIterator should be of the same type as the SetIterator");
}

/**
* @tc.number: SUB_COMMONLIBRARY_UTIL_IS_SET_ITERATOR_002
* @tc.name: testIsSetIterator002
* @tc.desc: Verify Set.values() is identified as SetIterator
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 1
*/
function testIsSetIterator002() {
  let proc = new util.types();
  const set = new Set<int>();
  let result = proc.isSetIterator(set.values());
  assertTrue(result, "The SetIterator should be of the same type as the SetIterator");
}

/**
* @tc.number: SUB_COMMONLIBRARY_UTIL_IS_SET_ITERATOR_003
* @tc.name: testIsSetIterator003
* @tc.desc: Verify Set.entries() is identified as SetIterator
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 1
*/
function testIsSetIterator003() {
  let proc = new util.types();
  const set = new Set<int>();
  let result = proc.isSetIterator(set.entries());
  assertTrue(result, "The SetIterator should be of the same type as the SetIterator");
}

/**
* @tc.number: SUB_COMMONLIBRARY_UTIL_IS_SET_ITERATOR_004
* @tc.name: testIsSetIterator004
* @tc.desc: Verify Set.$_iterator() is identified as SetIterator
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 1
*/
function testIsSetIterator004() {
  let proc = new util.types();
  const set = new Set<int>();
  let result = proc.isSetIterator(set.$_iterator());
  assertTrue(result, "The SetIterator should be of the same type as the SetIterator");
}

/**
* @tc.number: SUB_COMMONLIBRARY_UTIL_IS_SET_ITERATOR_005
* @tc.name: testIsSetIterator005
* @tc.desc: Verify Set is not identified as SetIterator
* @tc.size: MediumTest
* @tc.type: Function
* @tc.level: Level 1
*/
function testIsSetIterator005() {
  let proc = new util.types();
  const set = new Set<int>();
  let result = proc.isSetIterator(set);
  assertTrue(result == false, "The SetIterator should be of the same type as the SetIterator");
}
