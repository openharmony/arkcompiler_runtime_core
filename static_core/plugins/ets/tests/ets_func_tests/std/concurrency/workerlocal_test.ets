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

function test_init() {
    let str = 'workerLocal';
    let wl = new WorkerLocal<string>(() => str);
    arktest.assertEQ(wl.get(), str);
    wl.set('main');
    arktest.assertEQ(wl.get(), 'main');
    await taskpool.execute(() => {
        arktest.assertEQ(wl.get(), str);
        wl.set('subWorker');
        arktest.assertEQ(wl.get(), 'subWorker');
        wl.delete();
        arktest.assertEQ(wl.get(), str);
    })
    arktest.assertEQ(wl.get(), 'main');
}

function test_without_init() {
    let wl = new WorkerLocal<int>();
    try {
        wl.get();
        throw new Error('Call get() before set() without \'init\' should throw an error');
    } catch(err: Error) {
        arktest.assertEQ(err.message, 'WorkerLocal value not initialized. Call set() first or provide an init function.');
    }
    wl.set(1);
    arktest.assertEQ(wl.get(), 1);
    wl.set(wl.get() + 1);
    arktest.assertEQ(wl.get(), 2);
    await taskpool.execute(() => {
        try {
            wl.get();
            throw new Error('Call get() before set() without \'init\' should throw an error');
        } catch(err: Error) {
            arktest.assertEQ(err.message, 'WorkerLocal value not initialized. Call set() first or provide an init function.');
        }
        wl.set(1);
        arktest.assertEQ(wl.get(), 1);
        wl.set(wl.get() + 1);
        arktest.assertEQ(wl.get(), 2);
        wl.delete()
        try {
            wl.get();
            throw new Error('Call get() after delete() without \'init\' should throw an error');
        } catch(err: Error) {
            arktest.assertEQ(err.message, 'WorkerLocal value not initialized. Call set() first or provide an init function.');
        }
    })
    arktest.assertEQ(wl.get(), 2);
}

function main() {
    const testSuite = new arktest.ArkTestsuite("WorkerLocalTest");
    testSuite.addTest("test_init", test_init);
    testSuite.addTest("test_without_init", test_without_init);
    testSuite.run();
    CoroutineExtras.stopTaskpool();
}
