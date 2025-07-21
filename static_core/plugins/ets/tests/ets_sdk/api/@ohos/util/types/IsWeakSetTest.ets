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
    const suite = new ArkTestsuite("CommonUtil types.isWeakSet() API tests")

    suite.addTest("Verify WeakSet is identified as WeakSet", testIsWeakSet001)
    suite.addTest("Verify WeakMap is not identified as WeakSet", testIsWeakSet002)
    suite.addTest("Verify String is not identified as WeakSet", testIsWeakSet003)
    suite.addTest("Verify Set is not identified as WeakSet", testIsWeakSet004)
    suite.addTest("Verify Map is not identified as WeakSet", testIsWeakSet005)

    return suite.run()
}

/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_WEAK_SET_001
 * @tc.name: testIsWeakSet001
 * @tc.desc: Verify WeakSet is identified as WeakSet
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsWeakSet001() {
    let proc = new util.types();
    let result = proc.isWeakSet(new WeakSet<int>());
    assertTrue(result, "The Number WeakSet be of the same type as the WeakSet");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_WEAK_SET_002
 * @tc.name: testIsWeakSet002
 * @tc.desc: Verify WeakMap is not identified as WeakSet
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsWeakSet002() {
    let proc = new util.types();
    let result = proc.isWeakSet(new WeakMap<int, int>());
    assertTrue(result == false, "The WeakMap should be of the same type as the WeakMap");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_WEAK_SET_003
 * @tc.name: testIsWeakSet003
 * @tc.desc: Verify String is not identified as WeakSet
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsWeakSet003() {
    let proc = new util.types();
    let result = proc.isWeakSet(new String());
    assertTrue(result == false, "The String should be of the same type as the String");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_WEAK_SET_004
 * @tc.name: testIsWeakSet004
 * @tc.desc: Verify Set is not identified as WeakSet
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsWeakSet004() {
    let proc = new util.types();
    let result = proc.isWeakSet(new Set<int>());
    assertTrue(result == false, "The Set should be of the same type as the Set");
}
/**
 * @tc.number: SUB_COMMONLIBRARY_UTIL_IS_WEAK_SET_005
 * @tc.name: testIsWeakSet005
 * @tc.desc: Verify Map is not identified as WeakSet
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 1
 */
function testIsWeakSet005() {
    let proc = new util.types();
    let result = proc.isWeakSet(new Map<int, int>());
    assertTrue(result == false, "The Map should be of the same type as the Map");
}
