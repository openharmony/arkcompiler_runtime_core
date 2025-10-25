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

let mainFinReg: FinalizationRegistry<undefined>|undefined = undefined;
let workerFinReg: FinalizationRegistry<undefined>|undefined = undefined;
let eaFinReg: FinalizationRegistry<undefined>|undefined = undefined;

let cleanupInMainHappened = new AtomicFlag(false);
let cleanupInWorkerHappened = new AtomicFlag(false);
let cleanupInEAWorkerHappened = new AtomicFlag(false);

let eaWorker: EAWorker = new EAWorker();
let cleanupFinished = Promise.resolve();

function startGCWithCleanup(cause: GC.Cause) {
    GC.startGC(cause, true);
    Coroutine.Schedule();
}

class FinRegTestSuite extends arktest.ArkTestsuite {
    constructor() {
        super('FinalizationRegistrySuite');
    }

    public before() {
        mainFinReg = undefined;
        workerFinReg = undefined;
        eaFinReg = undefined;
        startGCWithCleanup(GC.Cause.FULL);
        cleanupInMainHappened.set(false);
        cleanupInWorkerHappened.set(false);
        cleanupInEAWorkerHappened.set(false);
    }
}

function register(finreg: FinalizationRegistry<undefined>) {
    finreg.register(new Object(), undefined);
}

function doInWorker(func: () => void): Job<void> {
    let params = new LaunchParams();
    params.workerGroupId = WorkerGroup.generateGroupId(WorkerDomain.GENERAL);
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    let job = launch<void, ()=>void>(() => {
        func();
    }, params);
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
    return job;
}

function cleanupInMainTest() {
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
    });
    doInWorker(() => {
        workerFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInWorkerHappened.set(true);
        });
    }).Await();
    eaWorker.run<void>((): void => {
        eaFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInEAWorkerHappened.set(true);
        });
    }).Await();
    register(mainFinReg!);
    register(workerFinReg!);
    register(eaFinReg!);
    startGCWithCleanup(GC.Cause.YOUNG);
    arktest.assertEQ(cleanupInMainHappened.get(), true);
    arktest.assertEQ(cleanupInWorkerHappened.get(), true);
    arktest.assertEQ(cleanupInEAWorkerHappened.get(), false);
}

function cleanupInWorkerTest() {
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
    });
    cleanupFinished = new Promise<void>((resolve) => {
        doInWorker(() => {
            workerFinReg = new FinalizationRegistry<undefined>(() => {
                cleanupInWorkerHappened.set(true);
                resolve(undefined);
            });
        }).Await();
    });
    eaWorker.run<void>((): void => {
        eaFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInEAWorkerHappened.set(true);
        });
    }).Await();
    register(mainFinReg!);
    register(workerFinReg!);
    register(eaFinReg!);
    doInWorker(() => {
        startGCWithCleanup(GC.Cause.YOUNG);
    }).Await();
    await cleanupFinished;
    arktest.assertEQ(cleanupInMainHappened.get(), false);
    arktest.assertEQ(cleanupInWorkerHappened.get(), true);
    arktest.assertEQ(cleanupInEAWorkerHappened.get(), false);
}

function cleanupInEAWorkerTest() {
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
    });
    doInWorker(() => {
        workerFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInWorkerHappened.set(true);
        });
    }).Await();
    cleanupFinished = new Promise<void>((resolve) => {
        eaWorker.run<void>((): void => {
            eaFinReg = new FinalizationRegistry<undefined>(() => {
                cleanupInEAWorkerHappened.set(true);
                resolve(undefined);
            });
        }).Await();
    });
    register(mainFinReg!);
    register(workerFinReg!);
    register(eaFinReg!);
    eaWorker.run<void>((): void => {
        startGCWithCleanup(GC.Cause.YOUNG);
    }).Await();
    await cleanupFinished;
    arktest.assertEQ(cleanupInMainHappened.get(), false);
    arktest.assertEQ(cleanupInWorkerHappened.get(), false);
    arktest.assertEQ(cleanupInEAWorkerHappened.get(), true);
}

function main() {
    eaWorker.start();
    try {
        let testsuite = new FinRegTestSuite();
        testsuite.addTest('cleanupInMainTest', cleanupInMainTest);
        testsuite.addTest('cleanupInWorkerTest', cleanupInWorkerTest);
        testsuite.addTest('cleanupInEAWorkerTest', cleanupInEAWorkerTest);
        return testsuite.run();
    } finally {
        eaWorker.join();
    }
}
