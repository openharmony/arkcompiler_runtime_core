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

function asyncSleep(time: int): Promise<Object> {
    return new Promise<Object>((resolve, _reject) => {
        setTimeout(() => {
            resolve({});
        }, time);
    });
}

function testExceptionCaughtByAwait(): void {
    let workerInstance = new EAWorker('worker1');
    workerInstance.start();
    let caught = false;
    try {
        workerInstance.run<void>(() => {
            throw new Error('throw error in run task');
        }).Await();
    } catch (e) {
        caught = true;
    }
    workerInstance.quit();
    arktest.assertTrue(caught, 'run task exception should be caught by Await');
}

async function testStdProcessUncaughtErrorFromInitTask(): Promise<void> {
    let caught = new AtomicFlag(false);
    let uncaughtListener: StdProcess.UncaughtErrorListener = (error: Object): void => {
        if (String(error) == 'Error: throw error in init task') {
            caught.set(true);
        }
    };

    let workerInstance = new EAWorker('worker2', (): void => {
        StdProcess.on('uncaughtError', uncaughtListener);
        throw new Error('throw error in init task');
    });
    workerInstance.start();
    await asyncSleep(500);
    arktest.assertTrue(caught.get(), 'StdProcess should catch uncaught error from EAWorker init task when handler is registered');
    workerInstance.quit();
    StdProcess.off('uncaughtError');
}

async function testStdProcessUncaughtErrorFromExceptionHandler(): Promise<void> {
    let caughtErrors: string[] = [];
    let uncaughtListener: StdProcess.UncaughtErrorListener = (error: Object): void => {
        caughtErrors.push(String(error));
    };

    StdProcess.on('uncaughtError', uncaughtListener);
    let workerInstance = new EAWorker('worker3', (): void => {
        throw new Error('throw error in eaworker');
    });
    workerInstance.setUncaughtExceptionHandler((_e) => {
        throw new Error('throw error in setUncaughtExceptionHandler');
    });
    workerInstance.start();
    await asyncSleep(100);
    arktest.assertTrue(caughtErrors.indexOf('Error: throw error in setUncaughtExceptionHandler') != -1, 'StdProcess should catch uncaught error from exception handler');
    workerInstance.quit();
    StdProcess.off('uncaughtError');
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.eworker_stdprocess_uncaught_error');
    testSuite.addTest('testEAWorkerRunExceptionIsCaughtByAwait', testExceptionCaughtByAwait);
    testSuite.addAsyncTest('testStdProcessUncaughtErrorFromInitTask', testStdProcessUncaughtErrorFromInitTask);
    testSuite.addAsyncTest('testStdProcessUncaughtErrorFromExceptionHandler', testStdProcessUncaughtErrorFromExceptionHandler);
    let res = testSuite.run();
    return res;
}
