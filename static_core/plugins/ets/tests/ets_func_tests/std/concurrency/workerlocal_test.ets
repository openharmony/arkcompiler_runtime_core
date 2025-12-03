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

 /*---
    tags: [no-warmup]
 ---*/

function testInit() {
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

async function testWithoutInit() {
    let wl = new WorkerLocal<int>();
    arktest.expectError(() => {
        wl.get();
    }, new Error('WorkerLocal value not initialized. Call set() first or provide an init function.'))
    wl.set(1);
    arktest.assertEQ(wl.get(), 1);
    wl.set(wl.get() + 1);
    arktest.assertEQ(wl.get(), 2);
    await taskpool.execute(() => {
        arktest.expectError(() => {
            wl.get();
        }, new Error('WorkerLocal value not initialized. Call set() first or provide an init function.'))
        wl.set(1);
        arktest.assertEQ(wl.get(), 1);
        wl.set(wl.get() + 1);
        arktest.assertEQ(wl.get(), 2);
        wl.delete()
        arktest.expectError(() => {
            wl.get();
        }, new Error('WorkerLocal value not initialized. Call set() first or provide an init function.'))
    })
    arktest.assertEQ(wl.get(), 2);
}

function main() {
    const testSuite = new arktest.ArkTestsuite("WorkerLocalTest");
    testSuite.addTest("testInit", testInit);
    testSuite.addAsyncTest("testWithoutInit", testWithoutInit);
    testSuite.run();
    CoroutineExtras.stopTaskpool();
}
