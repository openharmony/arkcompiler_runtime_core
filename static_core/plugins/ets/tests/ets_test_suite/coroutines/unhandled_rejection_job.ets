/**
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
const COUNTER_MODULO = 61;
const rejectionReason = 'get unhandled job rejection';
let count = 0;

async function testUnhandledJobRejection1(): Promise<void> {

     StdProcess.on('unhandledJobRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Job, 'The object should be of type Job');
        arktest.assertEQ(reason.message, rejectionReason);
        arktest.assertTrue(count >= COUNTER_MODULO);
    });

    let job1 = new CompletableJob<int>();
    job1.fail(new Error('get unhandled job rejection'));
  
    for (; count <= COUNTER_MODULO + 1  ; ++count) {
        setTimeout( () => {
            Coroutine.Schedule();
        });
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.unhandled_job_rejection');
    testSuite.addAsyncTest('testUnhandledJobRejection123', testUnhandledJobRejection1);
    let res = testSuite.run();

    return res;
}
