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

import { CoroutineExtras, AtomicFlag } from 'std/debug/concurrency';

let gSet: containers.ConcurrentSet<number> | undefined = undefined;
let gValue: number = 0;
let gGo: AtomicFlag = new AtomicFlag(false);
let gReady1: AtomicFlag = new AtomicFlag(false);
let gReady2: AtomicFlag = new AtomicFlag(false);

function addWorker(id: int): void {
    if (id == 1) {
        gReady1.set(true);
    } else {
        gReady2.set(true);
    }
    while (!gGo.get()) {
        Coroutine.Schedule();
    }
    gSet!.add(gValue);
}

async function twoTaskBarrierAdd(set: containers.ConcurrentSet<number>, v: number): Promise<void> {
    gSet = set;
    gValue = v;
    gGo.set(false);
    gReady1.set(false);
    gReady2.set(false);

    const group: taskpool.TaskGroup = new taskpool.TaskGroup();
    group.addTask(new taskpool.Task(addWorker, 1));
    group.addTask(new taskpool.Task(addWorker, 2));

    const p = taskpool.execute(group);
    while (!(gReady1.get() && gReady2.get())) {
        Coroutine.Schedule();
    }
    gGo.set(true);
    await p;
}

async function concurrentSetSameValueIdempotentAdd(): Promise<void> {
    const rounds: int = 5;
    const v: number = 42;
    for (let i: int = 0; i < rounds; i++) {
        const set: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();
        await twoTaskBarrierAdd(set, v);
        arktest.assertEQ(set.size, 1);
        arktest.assertEQ(set.has(v), true);
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite('taskpool.concurrent.container.set');
    suite.addAsyncTest('ConcurrentSetSameValueIdempotentAdd', concurrentSetSameValueIdempotentAdd);
    const res = suite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
