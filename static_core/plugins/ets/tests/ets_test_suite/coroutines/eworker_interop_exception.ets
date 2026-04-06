/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

function testEworkerInteropError() {
    let errorWasCaught = false;
    try {
        let eaw : EAWorker = new EAWorker(true);
        eaw.start();
        eaw.join();
    } catch (e) {
        if (e instanceof RuntimeError) {
            errorWasCaught = true;
        }
    }
    arktest.assertTrue(errorWasCaught);
}

async function main(): Promise<int> {
    let testSuite = new arktest.ArkTestsuite('coroutines.eworker_interop_error');
    testSuite.addTest('testEworkerInteropError', testEworkerInteropError);
    let res = await testSuite.runAsync();
    return res;
}
