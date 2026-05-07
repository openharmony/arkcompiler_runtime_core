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
    Coroutine.Schedule();
    StdProcess.off('unhandledPromiseRejection');
}

async function main(): Promise<int> {
    StdProcess.off('unhandledPromiseRejection');
    let testSuite = new arktest.ArkTestsuite('coroutines.unhandled_rejection_event');
    testSuite.addTest('testUnhandledRejectionObserverRegistration', testUnhandledRejectionObserverRegistration);
    let res = await testSuite.runAsync();
    return res;
}
