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

class Event {
    constructor() {
        this.promise = new Promise<boolean>((resolve: (val: boolean) => void) => {
            this.resolveFn = resolve;
        });
    }

    public wait(): boolean {
        return this.promise.awaitSync();
    }

    public fire(val: boolean = true) {
        this.resolveFn!(val);
    }

    private promise: Promise<boolean>;
    private resolveFn: ((val: boolean) => void) | null = null;
}

const LARGE_INTERVAL_MS = 600000;

function createBlockingEAWorkers(count: int, gate: Event, workers: EAWorker[]): void {
    for (let i = 0; i < count; ++i) {
        let worker = new EAWorker('verify-eaworker-' + i, () => {
            gate.wait();
        });
        worker.start();
        workers.push(worker);
        arktest.assertNE(worker.getWorkerId(), undefined, 'Failed to start EAWorker #' + i);
    }
}

function verifyNextEAWorkerCreationFails(gate: Event, workers: EAWorker[], userEaworkerLimit: int): void {
    let worker = new EAWorker('verify-eaworker-over-limit', () => {
        gate.wait();
    });
    try {
        worker.start();
        workers.push(worker);
        arktest.assertTrue(false, 'Creating EAWorker #' + (userEaworkerLimit + 1) + ' should fail');
    } catch (e) {
        arktest.assertTrue(e instanceof CoroutinesLimitExceedError,
            'Creating EAWorker #' + (userEaworkerLimit + 1) + ' should throw CoroutinesLimitExceedError');
    }
}

function releaseEAWorkers(workers: EAWorker[], gate: Event): void {
    gate.fire();
    for (let worker of workers) {
        worker.quit();
    }
}

function warmTaskpool(): void {
    let result = taskpool.execute((): int => {
        return 0;
    }).awaitSync() as int;
    arktest.assertEQ(result, 0);
}

function expandTaskpoolToLimit(): int {
    CoroutineExtras.setTaskPoolTriggerShrinkInterval(LARGE_INTERVAL_MS);
    CoroutineExtras.setTaskPoolIdleThreshold(LARGE_INTERVAL_MS);
    const expected = CoroutineExtras.getTaskPoolWorkersLimit();
    CoroutineExtras.increaseTaskpoolWorkersToN(expected);
    const actual = CoroutineExtras.getTaskPoolWorkersNum();
    arktest.assertEQ(actual, expected,
        'Taskpool worker count does not match computed limit');
    return actual;
}

function verifyEAWorkerAndTaskpoolLimitsDoNotInterfere(): void {
    let gate = new Event();
    let workers: EAWorker[] = [];
    const userEaworkerLimit = CoroutineExtras.getExclusiveWorkersLimit() as int;
    const taskpoolLimit = CoroutineExtras.getTaskPoolWorkersLimit();

    try {
        createBlockingEAWorkers(userEaworkerLimit, gate, workers);
        arktest.assertEQ(workers.length, userEaworkerLimit,
            'User EAWorker creation should reach ' + userEaworkerLimit);
        verifyNextEAWorkerCreationFails(gate, workers, userEaworkerLimit);

        warmTaskpool();
        let expandedWorkers = expandTaskpoolToLimit();
        arktest.assertEQ(expandedWorkers, taskpoolLimit,
            'Taskpool should expand to the computed worker limit');

        releaseEAWorkers(workers, gate);
        workers = [];

        arktest.assertEQ(CoroutineExtras.getTaskPoolWorkersNum(), taskpoolLimit,
            'Taskpool should remain expanded after releasing user EAWorkers');

        gate = new Event();
        createBlockingEAWorkers(userEaworkerLimit, gate, workers);
        arktest.assertEQ(workers.length, userEaworkerLimit,
            'User EAWorker creation should still reach ' + userEaworkerLimit + ' after taskpool expansion');
        verifyNextEAWorkerCreationFails(gate, workers, userEaworkerLimit);
    } finally {
        if (workers.length > 0) {
            releaseEAWorkers(workers, gate);
        }
        CoroutineExtras.stopTaskpool();
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite('taskpool.EAWorkerLimitIsolation');
    suite.addTest('EAWorkerAndTaskpoolLimitsDoNotInterfere', verifyEAWorkerAndTaskpoolLimitsDoNotInterfere);
    return suite.run();
}
