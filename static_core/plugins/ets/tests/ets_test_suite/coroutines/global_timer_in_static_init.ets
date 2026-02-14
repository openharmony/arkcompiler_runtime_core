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

// check setTimeout in global context
let timeoutCheckPassed = false;
arktest.expectThrow(() => {
    setTimeout(() => {}, 1);
}, (e: Error) => {
    return e instanceof InvalidCoroutineOperationError;
});
timeoutCheckPassed = true;

// check setInterval in global context
let intervalCheckPassed = false;
arktest.expectThrow(() => {
    setInterval(() => {}, 1);
}, (e: Error) => {
    return e instanceof InvalidCoroutineOperationError;
});
intervalCheckPassed = true;

function testGlobalTimerInStaticInit(): void {
    arktest.assertTrue(timeoutCheckPassed);
    arktest.assertTrue(intervalCheckPassed);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.global_timer_in_static_init');
    testSuite.addTest('testGlobalTimerInStaticInit', testGlobalTimerInStaticInit);
    return testSuite.run();
}
