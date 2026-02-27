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

// Temporary regression tests for blocked-worker special expansion.
// Can be deleted after CI gate starts running common_tasks.ets.
async function delay(ms: int): Promise<void> {
    await new Promise<void>((res, rej) => {
        setTimeout(() => {
            res(undefined);
        }, ms);
    });
}

function returnOne(): int {
    return 1;
}

function blockedExpandInnerTask(): int {
    return 1;
}

function blockedExpandOuterTask(): int {
    // Block this taskpool worker thread synchronously while waiting for another taskpool task.
    // With only one worker this creates a deadlock until the blocked-worker monitor triggers special expansion.
    let job = new CompletableJob<int>();

    (taskpool.execute(blockedExpandInnerTask) as Promise<int>).then((val: Any): void => {
        job.finish(val as int);
    }).catch((e: Any): void => {
        const err = (e instanceof Error) ? (e as Error) : new Error('blockedExpandInnerTask failed');
        job.fail(err);
    })

    return job.Await();
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

async function initTaskpoolWithOneWorker(): Promise<void> {
    // Touch taskpool to ensure it is initialized.
    let warmupTask = new taskpool.Task(returnOne);
    let res = await taskpool.execute(warmupTask);
    arktest.assertEQ(res, 1);

    // If pool starts with more than 1 worker (non-interop), force shrink to 1 quickly.
    CoroutineExtras.setTaskPoolIdleThreshold(1);
    CoroutineExtras.setTaskPoolTriggerShrinkInterval(10);
    CoroutineExtras.retriggerTaskPoolShrink();

    let workersNum = await waitUntilWorkersNum(1, 2000);
    arktest.assertEQ(workersNum, 1);

    // Avoid impacting other tests/files.
    CoroutineExtras.setTaskPoolTriggerShrinkInterval(30000);
    CoroutineExtras.setTaskPoolIdleThreshold(60000);
    CoroutineExtras.retriggerTaskPoolShrink();
}

async function blockedExpandDefaultConfigTest(): Promise<void> {
    if (CoroutineExtras.isTaskpoolUsingLaunchMode()) {
        return;
    }

    await initTaskpoolWithOneWorker();

    CoroutineExtras.setTaskPoolBlockedWorkerThresholdDebug(3000);
    CoroutineExtras.setTaskPoolBlockedWorkerMonitorIntervalDebug(1000);

    let result: int = 0;
    CoroutineExtras.retriggerTaskPoolBlockedExpandMonitorDebug();
    let p = taskpool.execute(blockedExpandOuterTask) as Promise<int>;
    p.then((val: Any): void => {
        result = val as int;
    });

    // After 5s, expansion should have happened and the nested result should be ready.
    await delay(5000);
    arktest.assertEQ(result, 1);
    arktest.assertEQ(CoroutineExtras.getTaskPoolWorkersNum(), 2);

    await p;

    // Restore defaults.
    CoroutineExtras.setTaskPoolBlockedWorkerThresholdDebug(3000);
    CoroutineExtras.setTaskPoolBlockedWorkerMonitorIntervalDebug(1000);
}

async function blockedExpandCustomThresholdAndIntervalTest(): Promise<void> {
    if (CoroutineExtras.isTaskpoolUsingLaunchMode()) {
        return;
    }

    await initTaskpoolWithOneWorker();

    CoroutineExtras.setTaskPoolBlockedWorkerThresholdDebug(5000);
    CoroutineExtras.setTaskPoolBlockedWorkerMonitorIntervalDebug(3500);

    let result: int = 0;
    CoroutineExtras.retriggerTaskPoolBlockedExpandMonitorDebug();
    let p = taskpool.execute(blockedExpandOuterTask) as Promise<int>;
    p.then((val: Any): void => {
        result = val as int;
    });

    // At 5s: threshold reached but monitor tick (3.5s) hasn't triggered expansion yet (next tick at ~7s).
    await delay(5000);
    arktest.assertEQ(result, 0);
    arktest.assertEQ(CoroutineExtras.getTaskPoolWorkersNum(), 1);

    // At 8s: expansion must have been triggered and the nested result should be ready.
    await delay(3000);
    arktest.assertEQ(result, 1);
    arktest.assertEQ(CoroutineExtras.getTaskPoolWorkersNum(), 2);

    await p;

    // Restore defaults.
    CoroutineExtras.setTaskPoolBlockedWorkerThresholdDebug(3000);
    CoroutineExtras.setTaskPoolBlockedWorkerMonitorIntervalDebug(1000);
}

function main(): int {
    let suite = new arktest.ArkTestsuite('taskpool.BlockedExpand');
    suite.addAsyncTest('BlockedExpandDefaultConfigTest', blockedExpandDefaultConfigTest);
    suite.addAsyncTest('BlockedExpandCustomThresholdAndIntervalTest', blockedExpandCustomThresholdAndIntervalTest);
    let res = suite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
