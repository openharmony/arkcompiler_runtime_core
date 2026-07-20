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

import { CoroutineExtras } from 'std/debug/concurrency';

/*---
tags: [no-warmup]
---*/

let transientShrinkBlockersStarted: AtomicInt = new AtomicInt(0);
let transientShrinkBlockerRelease: AtomicFlag = new AtomicFlag(false);
let transientShrinkShortDone: AtomicFlag = new AtomicFlag(false);
let transientShrinkLongStarted: AtomicFlag = new AtomicFlag(false);
let transientShrinkLongDeferred: PromiseWithResolvers<int> = new PromiseWithResolvers<int>();

async function delay(ms: int): Promise<void> {
    await new Promise<void>((resolve) => {
        setTimeout(() => {
            resolve(undefined);
        }, ms);
    });
}

async function waitUntilWorkersNum(expected: int, timeoutMs: int): Promise<int> {
    let start = Date.now();
    while ((Date.now() - start) < timeoutMs) {
        let current = CoroutineExtras.getTaskPoolWorkersNum();
        if (current == expected) {
            return current;
        }
        await delay(20);
    }
    return CoroutineExtras.getTaskPoolWorkersNum();
}

async function waitUntilCondition(check: () => boolean, timeoutMs: int): Promise<boolean> {
    let start = Date.now();
    while ((Date.now() - start) < timeoutMs) {
        if (check()) {
            return true;
        }
        await delay(20);
    }
    return check();
}

function resetTransientShrinkRegressionState(): void {
    transientShrinkBlockersStarted = new AtomicInt(0);
    transientShrinkBlockerRelease.set(false);
    transientShrinkShortDone.set(false);
    transientShrinkLongStarted.set(false);
    transientShrinkLongDeferred = new PromiseWithResolvers<int>();
}

function warmupTask(): int {
    return 1;
}

function transientShrinkBlockerTask(): int {
    transientShrinkBlockersStarted.fetchAdd(1);
    while (!transientShrinkBlockerRelease.get()) {
        Coroutine.Schedule();
    }
    return 1;
}

function transientShrinkLongTask(): Promise<int> {
    transientShrinkLongDeferred = new PromiseWithResolvers<int>();
    transientShrinkLongStarted.set(true);
    return transientShrinkLongDeferred.promise;
}

function transientShrinkShortTask(): int {
    transientShrinkShortDone.set(true);
    return 1;
}

async function overlappingAsyncTransientShouldNotShrinkWorkerEarly(): Promise<void> {
    if (CoroutineExtras.isTaskpoolUsingLaunchMode()) {
        return;
    }

    const originalWorkersLimit: int = CoroutineExtras.getTaskPoolWorkersLimit();
    let blockerPromises: Promise<Any>[] = [];
    resetTransientShrinkRegressionState();

    try {
        arktest.assertEQ(await taskpool.execute(warmupTask), 1);

        let targetWorkersNum: int = CoroutineExtras.getTaskPoolWorkersNum();
        if (targetWorkersNum < 2) {
            CoroutineExtras.setTaskPoolWorkersLimit(2);
            CoroutineExtras.increaseTaskpoolWorkersToN(2);
            arktest.assertEQ(await waitUntilWorkersNum(2, 2000), 2);
            targetWorkersNum = 2;
        }
        CoroutineExtras.setTaskPoolWorkersLimit(targetWorkersNum);

        const blockersNeeded: int = targetWorkersNum - 1;
        for (let i: int = 0; i < blockersNeeded; i++) {
            let blocker = new taskpool.Task(transientShrinkBlockerTask);
            blockerPromises.push(taskpool.execute(blocker, taskpool.Priority.HIGH));
        }
        arktest.assertTrue(
            await waitUntilCondition((): boolean => {
                return transientShrinkBlockersStarted.load() >= blockersNeeded;
            }, 2000),
            'blockers did not all start'
        );

        let longPromise: Promise<Any> = taskpool.execute(transientShrinkLongTask);
        arktest.assertTrue(
            await waitUntilCondition((): boolean => transientShrinkLongStarted.get(), 2000),
            'long transient did not start'
        );

        let shortPromise: Promise<Any> = taskpool.execute(transientShrinkShortTask);
        arktest.assertEQ((await shortPromise) as int, 1);
        arktest.assertTrue(transientShrinkShortDone.get());

        CoroutineExtras.setTaskPoolIdleThreshold(1);
        CoroutineExtras.setTaskPoolTriggerShrinkInterval(10);
        CoroutineExtras.retriggerTaskPoolShrink();

        let shrunkWorkersNum: int = await waitUntilWorkersNum(targetWorkersNum - 1, 1000);
        arktest.assertNE(
            shrunkWorkersNum,
            targetWorkersNum - 1,
            'worker shrank before the earlier async transient settled'
        );

        transientShrinkLongDeferred.resolve(42);
        arktest.assertEQ((await longPromise) as int, 42);
    } finally {
        transientShrinkLongDeferred.resolve(42);
        transientShrinkBlockerRelease.set(true);
        if (blockerPromises.length > 0) {
            await Promise.all(blockerPromises);
        }
        CoroutineExtras.setTaskPoolWorkersLimit(originalWorkersLimit);
        CoroutineExtras.setTaskPoolTriggerShrinkInterval(30000);
        CoroutineExtras.setTaskPoolIdleThreshold(60000);
        resetTransientShrinkRegressionState();
    }
}

async function main(): Promise<int> {
    let suite = new arktest.ArkTestsuite('taskpool.OverlappingAsyncTransientShrink');
    suite.addAsyncTest(
        'OverlappingAsyncTransientShouldNotShrinkWorkerEarly',
        overlappingAsyncTransientShouldNotShrinkWorkerEarly
    );
    let res = await suite.runAsync();
    CoroutineExtras.stopTaskpool();
    return res;
}
