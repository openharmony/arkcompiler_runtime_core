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
        })
        this.job = new CompletableJob<boolean>();
        this.promise.then((result: boolean) => {
            this.job.finish(result);
        }).catch((e: Error) => {
            this.job.fail(e);
        });
    }

    public wait(): boolean {
        return this.job.Await();
    }

    public fire(val: boolean = true) {
        this.resolveFn!(val);
    }

    private promise: Promise<boolean>;
    private resolveFn: ((val: boolean) => void) | null = null;
    private job: CompletableJob<boolean>;
}

function sleep(duration: int): Promise<void> {
    return new Promise<void>((resolve) => {
        setTimeout(() => {
            resolve(undefined);
        }, duration);
    });
}

async function cancelByTaskIdTest(){
    let event = new Event();
    let event2 = new Event();
    let taskFunc = async (duration: int): Promise<string> => {
        event.fire();
        while (!taskpool.Task.isCanceled()) {
            await sleep(10);
        }
        return 'canceled';
    }

    let task = new taskpool.Task(taskFunc, 200);
    let p = taskpool.execute(task)
    event.wait();
    taskpool.cancel(task.taskId);
    arktest.assertEQ(await p, 'canceled');
}

function main(): int {
    let commonTaskSuite = new arktest.ArkTestsuite('taskpool.CancelTest');
    commonTaskSuite.addAsyncTest('cancelByTaskIdTest', cancelByTaskIdTest);

    let res = commonTaskSuite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
