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

function doInWorker(func: () => void): Job<void> {
    let params = new LaunchParams();
    params.workerGroupId = WorkerGroup.generateGroupId(WorkerDomain.GENERAL);
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    let job = launch(params) {
        func();
    };
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
    return job;
}

/**
 * The test creates FinRegs in different domains, triggers GC in the main coro.
 * GC in the main coro must finalize FinRegs in the main and worker domains.
 */
function cleanupCMCGCInMainTest() {
    let cleanupInMainHappened = new AtomicFlag(false);
    let cleanupInWorkerHappened = new AtomicFlag(false);
    let mainWorkerId = CoroutineExtras.getWorkerId();
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
        arktest.assertEQ(mainWorkerId, CoroutineExtras.getWorkerId());
    });
    let cleanupFinished = new Promise<void>((resolve, reject) => {
        doInWorker(() => {
            workerFinReg = new FinalizationRegistry<undefined>(() => {
                cleanupInWorkerHappened.set(true);
                let currentId = CoroutineExtras.getWorkerId();
                if (currentId == mainWorkerId) {
                    resolve(undefined);
                } else {
                    reject(new Error("Expected worker id " + mainWorkerId + ", but got " + currentId));
                }
            });
        }).Await();
    });
    register(mainFinReg!);
    register(workerFinReg!);
    GC.waitForFinishGC(GC.startGC(GC.Cause.THRESHOLD));
    waitForCompletion(async () => {
        await cleanupFinished;
    });
    arktest.assertEQ(cleanupInMainHappened.get(), true);
    arktest.assertEQ(cleanupInWorkerHappened.get(), true);
}

/**
 * The test creates FinRegs in different domains, triggers GC in a worker coro.
 * GC in the worker coro must finalize the FinReg in the worker domain.
 */
function cleanupCMCGCInWorkerTest() {
    let cleanupInMainHappened = new AtomicFlag(false);
    let cleanupInWorkerHappened = new AtomicFlag(false);
    let mainWorkerId = CoroutineExtras.getWorkerId();
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
        arktest.assertEQ(mainWorkerId, CoroutineExtras.getWorkerId());
    });
    let cleanupFinished = new Promise<void>((resolve, reject) => {
        doInWorker(() => {
            let workerId = CoroutineExtras.getWorkerId();
            workerFinReg = new FinalizationRegistry<undefined>(() => {
                let currentId = CoroutineExtras.getWorkerId();
                cleanupInWorkerHappened.set(true);
                if (currentId == workerId || currentId == mainWorkerId) {
                    resolve(undefined);
                } else {
                    reject(new Error("Expected worker id " + workerId  + ", but got " + currentId));
                }
            });
        }).Await();
    });
    register(mainFinReg!);
    register(workerFinReg!);
    GC.waitForFinishGC(GC.startGC(GC.Cause.THRESHOLD));
    waitForCompletion(async () => {
        await cleanupFinished;
    })
    arktest.assertEQ(cleanupInMainHappened.get(), true);
    arktest.assertEQ(cleanupInWorkerHappened.get(), true);
}

/**
 * The test creates FinRegs in different domains, triggers GC in a EA coro.
 * GC in the EA coro must finalize the FinReg in the same EA worker.
 */
function cleanupCMCGCInEAWorkerTest() {
    let cleanupInMainHappened = new AtomicFlag(false);
    let cleanupInWorkerHappened = new AtomicFlag(false);
    let cleanupInEAWorkerHappened = new AtomicFlag(false);
    let mainWorkerId = CoroutineExtras.getWorkerId();
    mainFinReg = new FinalizationRegistry<undefined>(() => {
        cleanupInMainHappened.set(true);
        arktest.assertEQ(mainWorkerId, CoroutineExtras.getWorkerId());
    });
    let workerCleanupFinished = new Promise<void>((resolve, reject) => {
        doInWorker(() => {
            workerFinReg = new FinalizationRegistry<undefined>(() => {
                cleanupInWorkerHappened.set(true);
                let currentId = CoroutineExtras.getWorkerId();
                if (currentId == mainWorkerId) {
                    resolve(undefined);
                } else {
                    reject(new Error("Expected worker id " + mainWorkerId + ", but got " + currentId));
                }
            });
        }).Await();
    });
    let eaWorkerCleanupFinished = new Promise<void>((resolve, reject) => {
        eaWorker1.run<void>((): void => {
            let workerId = CoroutineExtras.getWorkerId();
            eaFinReg = new FinalizationRegistry<undefined>(() => {
                cleanupInEAWorkerHappened.set(true);
                let currentId = CoroutineExtras.getWorkerId();
                if (currentId == workerId) {
                    resolve(undefined);
                } else {
                    reject(new Error("Expected worker id " + workerId + ", but got " + currentId));
                }
            });
        }).Await();
    });
    register(mainFinReg!);
    register(workerFinReg!);
    register(eaFinReg!);
    GC.waitForFinishGC(GC.startGC(GC.Cause.THRESHOLD));
    eaWorker1.run<void>((): void => {
        Coroutine.Schedule();
    }).Await();
    waitForCompletion(async () => {
        await workerCleanupFinished;
    });
    waitForCompletion(async () => {
        await eaWorkerCleanupFinished;
    })
    arktest.assertEQ(cleanupInMainHappened.get(), true);
    arktest.assertEQ(cleanupInWorkerHappened.get(), true);
    arktest.assertEQ(cleanupInEAWorkerHappened.get(), true);
}

/**
 * The test creates FinRegs in different EA coros, triggers GC in the first EA coro.
 * GC in the first EA coro must finalize the FinReg in the same EA worker and
 * don't touch the FinReg in another EA coro.
 */
function cleanupInSameEAWorkerCMCGCTest() {
    let cleanup1Happened = new AtomicFlag(false);
    let cleanup2Happened = new AtomicFlag(false);
    let cleanup1Finished = new Promise<void>((resolve, reject) => {
        eaWorker1.run<void>((): void => {
            let workerId = CoroutineExtras.getWorkerId();
            eaFinReg = new FinalizationRegistry<undefined>(() => {
                cleanup1Happened.set(true);
                let currentId = CoroutineExtras.getWorkerId();
                if (currentId == workerId) {
                    resolve(undefined);
                } else {
                    reject(new Error("Expected worker id " + workerId + ", but got " + currentId));
                }
            });
        }).Await();
    });
    let cleanup2Finished = new Promise<void>((resolve, reject) => {
        eaWorker2.run<void>((): void => {
            let workerId = CoroutineExtras.getWorkerId();
            eaFinReg2 = new FinalizationRegistry<undefined>(() => {
                cleanup2Happened.set(true);
                let currentId = CoroutineExtras.getWorkerId();
                if (currentId == workerId) {
                    resolve(undefined);
                } else {
                    reject(new Error("Expected worker id " + workerId + ", but got " + currentId));
                }
            });
        }).Await();
    });
    register(eaFinReg!);
    register(eaFinReg2!);
    GC.waitForFinishGC(GC.startGC(GC.Cause.THRESHOLD));
    eaWorker1.run<void>((): void => {
        Coroutine.Schedule();
    }).Await();
    eaWorker2.run<void>((): void => {
        Coroutine.Schedule();
    }).Await();
    waitForCompletion(async () => {
        await cleanup1Finished;
    })
    waitForCompletion(async () => {
        await cleanup2Finished;
    })
    arktest.assertEQ(cleanup1Happened.get(), true);
    arktest.assertEQ(cleanup2Happened.get(), true);
}

function surviveGCTest() {
    let cleanupFinished = new Promise<void>((resolve) => {
        eaWorker1.run<void>((): void => {
            eaFinReg = new FinalizationRegistry<undefined>(() => {
                resolve(undefined);
            });
        }).Await();
    });
    register(eaFinReg!);
    // This GC must enqueue FinRegNode and don't clean up it.
    startGCWithCleanup(GC.Cause.YOUNG);
    // This GC must not collect FinRegNode.
    startGCWithCleanup(GC.Cause.FULL);
    // This GC should cleanup FinRegNode
    eaWorker1.run<void>((): void => {
        startGCWithCleanup(GC.Cause.FULL);
    }).Await();
    waitForCompletion(async () => {
        await cleanupFinished;
    })
}

function main() {
    eaWorker1.start();
    eaWorker2.start();
    try {
        let testsuite = new FinRegTestSuite();
        testsuite.addTest('cleanupCMCGCInMainTest', cleanupCMCGCInMainTest);
        testsuite.addTest('cleanupCMCGCInWorkerTest', cleanupCMCGCInWorkerTest);
        testsuite.addTest('cleanupCMCGCInEAWorkerTest', cleanupCMCGCInEAWorkerTest);
        testsuite.addTest('cleanupInSameEAWorkerCMCGCTest', cleanupInSameEAWorkerCMCGCTest);
        testsuite.addTest('surviveGCTest', surviveGCTest);
        return testsuite.run();
    } finally {
        eaWorker1.join();
        eaWorker2.join();
    }
}
