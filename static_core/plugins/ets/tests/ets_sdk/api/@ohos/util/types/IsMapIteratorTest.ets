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
    const suite = new ArkTestsuite("CommonUtil types.isMapIterator() API tests")

    suite.addTest("Verify Map.keys() is identified as MapIterator", testIsMapIterator001)
    suite.addTest("Verify Map.values() is identified as MapIterator", testIsMapIterator002)
    suite.addTest("Verify Map.entries() is identified as MapIterator", testIsMapIterator003)
    suite.addTest("Verify Map.$_iterator() is identified as MapIterator", testIsMapIterator004)
    suite.addTest("Verify Map is not identified as MapIterator", testIsMapIterator005)
    suite.addTest("Verify Array is not identified as MapIterator", testIsMapIterator006)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0099
 * @tc.name: testIsMapIterator001
 * @tc.desc: Verify Map.keys() is identified as MapIterator
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsMapIterator001() {
  let proc = new util.types();
  const map = new Map<int, int>();
  let result = proc.isMapIterator(map.keys());
  assertTrue(result, "The MapIterator should be of the same type as the MapIterator");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0100
 * @tc.name: testIsMapIterator002
 * @tc.desc: Verify Map.values() is identified as MapIterator
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsMapIterator002() {
  let proc = new util.types();
  const map = new Map<int, int>();
  let result = proc.isMapIterator(map.values());
  assertTrue(result, "The MapIterator should be of the same type as the MapIterator");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0101
 * @tc.name: testIsMapIterator003
 * @tc.desc: Verify Map.entries() is identified as MapIterator
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsMapIterator003() {
  let proc = new util.types();
  const map = new Map<int, int>();
  let result = proc.isMapIterator(map.entries());
  assertTrue(result, "The MapIterator should be of the same type as the MapIterator");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0102
 * @tc.name: testIsMapIterator004
 * @tc.desc: Verify Map.$_iterator() is identified as MapIterator
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsMapIterator004() {
  let proc = new util.types();
  const map = new Map<int, int>();
  let result = proc.isMapIterator(map.$_iterator());
  assertTrue(result, "The MapIterator should be of the same type as the MapIterator");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0103
 * @tc.name: testIsMapIterator005
 * @tc.desc: Verify Map is not identified as MapIterator
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsMapIterator005() {
  let proc = new util.types();
  const map = new Map<int, int>();
  let result = proc.isMapIterator(map);
  assertTrue(result == false, "The Map should not be of the same type as the MapIterator");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_TYPES_0104
 * @tc.name: testIsMapIterator006
 * @tc.desc: Verify Array is not identified as MapIterator
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsMapIterator006() {
  let proc = new util.types();
  const array = new Array<int>();
  let result = proc.isMapIterator(array);
  assertTrue(result == false, "The Array should not be of the same type as the MapIterator");
}
