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


async function out_of_coro_test() {
    const coro_to_create = 7001;
    let blocker = new PromiseWithResolvers<void>();
    let jobs = new Array<Promise<void>>();
    let errorCaught = false;

    let eaw1 = new EAWorker();
    eaw1.start();

    let eaw2 = new EAWorker();
    eaw2.start();
    await eaw2.run<void>(() => {});

    let awaitIt = (): void => { blocker.promise.awaitSync(); }

    try {
        for (let i = 0; i < coro_to_create; ++i) {
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

    for (let j of jobs) {
        await j;
    }

    let join1 = eaw1.join();
    let join2 = eaw2.join();
    await join1;
    await join2;
}

async function main(): Promise<int> {
    let testsuite = new arktest.ArkTestsuite("EAWorker");
    testsuite.addAsyncTest("out_of_coro_test", out_of_coro_test)
    return await testsuite.runAsync();
}
