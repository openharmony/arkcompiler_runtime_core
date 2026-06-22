/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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


function testIsSafeIntegerInt(): void {
    // int: Float.MAX_SAFE_INTEGER = 16777215
    arktest.assertEQ(isSafeInteger(0), true)
    arktest.assertEQ(isSafeInteger(16777215), true)
    arktest.assertEQ(isSafeInteger(16777216), false)
    arktest.assertEQ(isSafeInteger(-16777215), true)
    arktest.assertEQ(isSafeInteger(-16777216), false)
}

function testIsSafeIntegerLong(): void {
    // long: Double.MAX_SAFE_INTEGER = 9007199254740991, MIN = -9007199254740991
    arktest.assertEQ(isSafeInteger(0), true)
    arktest.assertEQ(isSafeInteger(9007199254740991), true)
    arktest.assertEQ(isSafeInteger(9007199254740992), false)    
    arktest.assertEQ(isSafeInteger(-9007199254740991), true)
    arktest.assertEQ(isSafeInteger(-9007199254740992), false)
}

function main(): int {
    const suite = new arktest.ArkTestsuite("isSafeInteger tests")
    suite.addTest('isSafeInteger for int', testIsSafeIntegerInt);
    suite.addTest('isSafeInteger for long', testIsSafeIntegerLong);
    return suite.run()
}