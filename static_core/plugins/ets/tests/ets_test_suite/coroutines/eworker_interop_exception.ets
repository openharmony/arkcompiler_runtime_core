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

function testEworkerInteropException() {
    let exceptionWasCaught = false;
    try {
        let eaw : EAWorker = new EAWorker(true);
        eaw.join();
    } catch (e) {
        if (e instanceof RuntimeException) {
            exceptionWasCaught = true;
        }
    }
    arktest.assertTrue(exceptionWasCaught);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.eworker_interop_exception');
    testSuite.addTest('testEworkerInteropException', testEworkerInteropException);
    let res = testSuite.run();
    return res;
}