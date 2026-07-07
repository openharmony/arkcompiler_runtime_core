/**
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


async function outOfCoroTest() {
    const coroToCreate = 2000;
    let arr = new Int32Array(new ArrayBuffer(1024))

    let jobs: Job<void>[] = [];

    arktest.expectError(() => {
        let wait_func = async (arr: Int32Array) => { await Atomics.asyncWait(arr, 0, 0); }
        let mainWaitJob = launch() { wait_func(arr) }
        let awaitIt = async () => { mainWaitJob.Await() }

        for (let i = 1; i < coroToCreate; ++ i) {
            jobs.push(launch(awaitIt));
        }
    }, new CoroutinesLimitExceedError('Unable to create a new coroutine: reached the limit for the number of existing coroutines.'));

    let eaw1 = new EAWorker();
    eaw1.start();

    let eaw2 = new EAWorker();
    eaw2.start();
    await eaw2.run<void>(() => {});

    let awaitIt = (): void => { blocker.promise.awaitSync(); }

    try {
        for (let i = 0; i < coroToCreate; ++i) {
            jobs.push(launch(awaitIt));
        }
    } catch (e) {
        errorCaught = true;
        arktest.assertTrue(e instanceof CoroutinesLimitExceedError);
        arktest.assertEQ(
            e.toString(),
            'CoroutinesLimitExceedError: Unable to create a new coroutine: reached the limit for the number of existing coroutines.'
        );
    }
    arktest.assertTrue(errorCaught, 'Coroutine limit error was expected');

    blocker.resolve(undefined);

    Atomics.store(arr, 0, 1);
    Atomics.notify(arr, 0);

    for (let j of jobs) {
        j.Await();
    }

    eaw1.quit();
    eaw2.run<void>(()=>{});
    eaw2.quit();
}

async function main(): Promise<int> {
    let testsuite = new arktest.ArkTestsuite('EAWorker');
    testsuite.addAsyncTest('outOfCoroTest', outOfCoroTest)
    return await testsuite.runAsync();
}
