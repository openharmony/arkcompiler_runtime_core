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

function testUnhandledRejectionObserverRegistration(): void {
    StdProcess.on('unhandledPromiseRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Promise, 'The object should be of type Promise');
        arktest.assertEQ(reason.message, 'get unhandled promise rejection');
    });

    StdProcess.on('unhandledPromiseRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Promise, 'The object should be of type Promise');
        arktest.assertEQ(reason.message, 'get unhandled promise rejection');
    });

    let observer1 = (reason: Error, obj: Object): void => {
        arktest.assertTrue(false, 'Promise handler that have been removed using off will no longer be triggered');
    };

    let observer2 = (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Promise, 'The object should be of type Promise');
        arktest.assertEQ(reason.message, 'get unhandled promise rejection');
    };

    StdProcess.on('unhandledPromiseRejection', observer1);
    StdProcess.on('unhandledPromiseRejection', observer2);
    StdProcess.off('unhandledPromiseRejection', observer1);

    let p1 = Promise.reject(new Error('get unhandled promise rejection'));

    let p2 = Promise.reject(new Error('handled promise rejection'));
    p2.catch(error => {

    });
}

function Failing(): string {
    throw Error('get unhandled job rejection');
}

function catchFailing(): string {
    throw Error('catch launch rejection');
}

function testAllUnhandledJobRejection(): void {
    StdProcess.on('unhandledJobRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Job, 'The object should be of type Job');
        arktest.assertEQ(reason.message, 'get unhandled job rejection');
    });

    StdProcess.on('unhandledJobRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Job, 'The object should be of type Job');
        arktest.assertEQ(reason.message, 'get unhandled job rejection');
    });

    let observer1 = (reason: Error, obj: Object): void => {
        arktest.assertTrue(false, 'Job handler that have been removed using off will no longer be triggered');
    };

    let observer2 = (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Job, 'The object should be of type Job');
        arktest.assertEQ(reason.message, 'get unhandled job rejection');
    };

    StdProcess.on('unhandledJobRejection', observer1);
    StdProcess.on('unhandledJobRejection', observer2);
    StdProcess.off('unhandledJobRejection', observer1);

    let g = launch<String, () => string>(Failing);

    try {
        launch<String, () => string>(catchFailing).Await();
    } catch(e) {

    }

    let job1 = new CompletableJob<int>();
    job1.fail(new Error('get unhandled job rejection'));

    let job2 = new CompletableJob<int>();
    job2.fail(new Error('catch job rejection'));
    try {
        job2.Await();
    } catch (e) {

    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.unhandled_rejection_event');
    testSuite.addTest('testUnhandledRejectionObserverRegistration', testUnhandledRejectionObserverRegistration);
    testSuite.addTest('testAllUnhandledJobRejection', testAllUnhandledJobRejection);
    let res = testSuite.run();
    return res;
}
