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

function sleep(duration: int): Promise<void> {
    return new Promise<void>((resolve) => {
        setTimeout(() => {
            resolve(undefined);
        }, duration);
    });
}

async function cancelByTaskIdTest(){
    let event = new PromiseWithResolvers<void>();
    let event2 = new PromiseWithResolvers<void>();
    let taskFunc = async (duration: int): Promise<string> => {
        event.resolve(undefined);
        while (!taskpool.Task.isCanceled()) {
            await sleep(10);
        }
        return 'canceled';
    }

    let task = new taskpool.Task(taskFunc, 200);
    let p = taskpool.execute(task)
    await event.promise;
    taskpool.cancel(task.taskId);
    arktest.assertEQ(await p, 'canceled');
}

async function main(): Promise<int> {
    let commonTaskSuite = new arktest.ArkTestsuite('taskpool.CancelTest');
    commonTaskSuite.addAsyncTest('cancelByTaskIdTest', cancelByTaskIdTest);

    let res = await commonTaskSuite.runAsync();
    CoroutineExtras.stopTaskpool();
    return res;
}
