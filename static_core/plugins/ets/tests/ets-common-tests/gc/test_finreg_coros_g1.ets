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

let mainFinReg: FinalizationRegistry<undefined>|undefined = undefined;
let workerFinReg: FinalizationRegistry<undefined>|undefined = undefined;
let eaFinReg: FinalizationRegistry<undefined>|undefined = undefined;
let eaFinReg2: FinalizationRegistry<undefined>|undefined = undefined;

let eaWorker1: EAWorker = new EAWorker();
let eaWorker2: EAWorker = new EAWorker();

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
        eaFinReg2 = undefined;
        startGCWithCleanup(GC.Cause.FULL);
    }
}

function register(finreg: FinalizationRegistry<undefined>) {
    finreg.register(new Object(), undefined);
}

function doInWorker(func: () => void): Promise<void> {
    let params = new LaunchParams();
    params.workerGroupId = WorkerGroup.generateGroupId(WorkerDomain.GENERAL);
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    let promise = launch(params) {
        func();
    };
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
    return promise;
}

/**
 * The test creates FinRegs in different domains, triggers GC in the main coro.
 * GC in the main coro must finalize FinRegs in the main and worker domains.
 */
async function cleanupInMainTest(): Promise<void> {
    let cleanupInMainHappened = new AtomicFlag(false);
    let cleanupInWorkerHappened = new AtomicFlag(false);
    let cleanupInEAWorkerHappened = new AtomicFlag(false);
    let cleanupCount = 0;
    let cleanupFinishedResolve: (() => void) | undefined;
    let cleanupFinished = new Promise<void>((resolve) => {
        cleanupFinishedResolve = () => { resolve(undefined); };
    });
    let onCleanup = () => {
        cleanupCount++;
        if (cleanupCount == 2) {
            cleanupFinishedResolve!();
        }
    };
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
        onCleanup();
    });
    await doInWorker(() => {
        workerFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInWorkerHappened.set(true);
            onCleanup();
        });
    });
    await eaWorker1.run<void>((): void => {
        eaFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInEAWorkerHappened.set(true);
        });
    });
    register(mainFinReg!);
    register(workerFinReg!);
    register(eaFinReg!);
    startGCWithCleanup(GC.Cause.YOUNG);
    await cleanupFinished;
    arktest.assertEQ(cleanupInMainHappened.get(), true);
    arktest.assertEQ(cleanupInWorkerHappened.get(), true);
    arktest.assertEQ(cleanupInEAWorkerHappened.get(), false);
}

/**
 * The test creates FinRegs in different domains, triggers GC in a worker coro.
 * GC in the worker coro must finalize the FinReg in the worker domain.
 */
async function cleanupInWorkerTest(): Promise<void> {
    let cleanupInMainHappened = new AtomicFlag(false);
    let cleanupInWorkerHappened = new AtomicFlag(false);
    let cleanupInEAWorkerHappened = new AtomicFlag(false);
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
    });
    let cleanupFinishedResolve: (() => void) | undefined;
    let cleanupFinished = new Promise<void>((resolve) => {
        cleanupFinishedResolve = () => { resolve(undefined); };
    });
    await doInWorker(() => {
        workerFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInWorkerHappened.set(true);
            cleanupFinishedResolve!();
        });
    });
    await eaWorker1.run<void>((): void => {
        eaFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInEAWorkerHappened.set(true);
        });
    });
    register(mainFinReg!);
    register(workerFinReg!);
    register(eaFinReg!);
    await doInWorker(() => {
        startGCWithCleanup(GC.Cause.YOUNG);
    });
    await cleanupFinished;
    arktest.assertEQ(cleanupInMainHappened.get(), false);
    arktest.assertEQ(cleanupInWorkerHappened.get(), true);
    arktest.assertEQ(cleanupInEAWorkerHappened.get(), false);
}

/**
 * The test creates FinRegs in different domains, triggers GC in a EA coro.
 * GC in the EA coro must finalize the FinReg in the same EA worker.
 */
async function cleanupInEAWorkerTest(): Promise<void> {
    let cleanupInMainHappened = new AtomicFlag(false);
    let cleanupInWorkerHappened = new AtomicFlag(false);
    let cleanupInEAWorkerHappened = new AtomicFlag(false);
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
    });
    await doInWorker(() => {
        workerFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInWorkerHappened.set(true);
        });
    });
    let cleanupFinishedResolve: (() => void) | undefined;
    let cleanupFinished = new Promise<void>((resolve) => {
        cleanupFinishedResolve = () => { resolve(undefined); };
    });
    await eaWorker1.run<void>((): void => {
        eaFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupInEAWorkerHappened.set(true);
            cleanupFinishedResolve!();
        });
    });
    register(mainFinReg!);
    register(workerFinReg!);
    register(eaFinReg!);
    await eaWorker1.run<void>((): void => {
        startGCWithCleanup(GC.Cause.YOUNG);
    });
    await cleanupFinished;
    arktest.assertEQ(cleanupInMainHappened.get(), false);
    arktest.assertEQ(cleanupInWorkerHappened.get(), false);
    arktest.assertEQ(cleanupInEAWorkerHappened.get(), true);
}

/**
 * The test creates FinRegs in different EA coros, triggers GC in the first EA coro.
 * GC in the first EA coro must finalize the FinReg in the same EA worker and
 * don't touch the FinReg in another EA coro.
 */
async function cleanupInSameEAWorkerTest(): Promise<void> {
    let cleanup1Happened = new AtomicFlag(false);
    let cleanup2Happened = new AtomicFlag(false);
    let cleanupFinishedResolve: (() => void) | undefined;
    let cleanupFinished = new Promise<void>((resolve) => {
        cleanupFinishedResolve = () => { resolve(undefined); };
    });
    await eaWorker1.run<void>((): void => {
        eaFinReg = new FinalizationRegistry<undefined>(() => {
            cleanup1Happened.set(true);
            cleanupFinishedResolve!();
        });
    });
    await eaWorker2.run<void>((): void => {
        eaFinReg2 = new FinalizationRegistry<undefined>(() => {
            cleanup2Happened.set(true);
        });
    });
    register(eaFinReg!);
    register(eaFinReg2!);
    await eaWorker1.run<void>((): void => {
        startGCWithCleanup(GC.Cause.YOUNG);
    });
    await eaWorker2.run<void>((): void => {
        Coroutine.Schedule();
    });
    await cleanupFinished;
    arktest.assertEQ(cleanup1Happened.get(), true);
    arktest.assertEQ(cleanup2Happened.get(), false);
}

async function surviveGCTest(): Promise<void> {
    let cleanupFinishedResolve: (() => void) | undefined;
    let cleanupFinished = new Promise<void>((resolve) => {
        cleanupFinishedResolve = () => { resolve(undefined); };
    });
    await eaWorker1.run<void>((): void => {
        eaFinReg = new FinalizationRegistry<undefined>(() => {
            cleanupFinishedResolve!();
        });
    });
    register(eaFinReg!);
    // This GC must enqueue FinRegNode and don't clean up it.
    startGCWithCleanup(GC.Cause.YOUNG);
    // This GC must not collect FinRegNode.
    startGCWithCleanup(GC.Cause.FULL);
    // This GC should cleanup FinRegNode
    await eaWorker1.run<void>((): void => {
        startGCWithCleanup(GC.Cause.FULL);
    });
    await cleanupFinished;
}

function main() {
    eaWorker1.start();
    eaWorker2.start();
    try {
        let testsuite = new FinRegTestSuite();
        testsuite.addAsyncTest('cleanupInMainTest', cleanupInMainTest);
        testsuite.addAsyncTest('cleanupInWorkerTest', cleanupInWorkerTest);
        testsuite.addAsyncTest('cleanupInEAWorkerTest', cleanupInEAWorkerTest);
        testsuite.addAsyncTest('cleanupInSameEAWorkerTest', cleanupInSameEAWorkerTest);
        testsuite.addAsyncTest('surviveGCTest', surviveGCTest);
        return testsuite.run();
    } finally {
        eaWorker1.quit();
        eaWorker2.quit();
    }
}
