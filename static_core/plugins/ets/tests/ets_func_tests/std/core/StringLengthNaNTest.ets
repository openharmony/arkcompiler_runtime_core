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

function main(): int {
    const suite = new arktest.ArkTestsuite("Tests ");
    suite.addTest("testCase001", testCase001);
    suite.addTest("testCase002", testCase002);
    suite.addTest("testCase003", testCase003);
    suite.addTest("testCase004", testCase004);
    return suite.run();
}

function testCase001() {
    arktest.assertEQ('abc'.substring(0, NaN), '', 'start: 0, length: NaN');
}

function testCase002() {
    arktest.assertEQ('abc'.substring(1, NaN), 'a', 'start: 1, length: NaN');
}

function testCase003() {
    arktest.assertEQ('abc'.substring(2, NaN), 'ab', 'start: 2, length: NaN');
}

function testCase004() {
    arktest.assertEQ('abc'.substring(3, NaN), 'abc', 'start: 3, length: NaN');
}
