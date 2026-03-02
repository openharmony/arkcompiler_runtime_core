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


async function delay(ms: int) {
    await new Promise<void>((res, rej) => {
        setTimeout(() => {
            res(undefined);
        }, ms);
    });
}

async function taskForcurrentTaskTest(value: number): Promise<string> {
    await delay(10);
    taskpool.Task.sendData(value);
    return value.toString();
}

async function currentTaskTest(): Promise<void> {
    const expectedValue: number = 1.0;

    let count: AtomicInt = new AtomicInt(0);
    let errEnqueueCount: AtomicInt = new AtomicInt(0);
    let errStartCount: AtomicInt = new AtomicInt(0);
    let errSuccessCount: AtomicInt = new AtomicInt(0);

    let enqueueId1: AtomicInt = new AtomicInt(0);
    let startExecute: AtomicInt = new AtomicInt(0);
    let executeSuccess: AtomicInt = new AtomicInt(0);
    let onReceive: AtomicInt = new AtomicInt(0);

    let task1: taskpool.Task = new taskpool.Task(taskForcurrentTaskTest, expectedValue);

    let promises: Promise<Any>[] = [];

    for (let i: int = 0; i < 10; i++) {
        // The same task can register listeners multiple times before it is executed.
        // After it is enqueued/executed, registering listeners again should throw.
        // (But previously registered listeners must still run for repeated executions.)
        try {
            task1.onEnqueued(() => {
                enqueueId1.fetchAdd(1);
            });
        } catch (e) {
            errEnqueueCount.fetchAdd(1);
        }

        try {
            task1.onStartExecution(() => {
                startExecute.fetchAdd(1);
            });
        } catch (e) {
            errStartCount.fetchAdd(1);
        }

        try {
            task1.onExecutionSucceeded(() => {
                executeSuccess.fetchAdd(1);
            });
        } catch (e) {
            errSuccessCount.fetchAdd(1);
        }

        // The same task can set receive callback multiple times.
        // Expected behavior: overwrites the old callback, no throw.
        task1.onReceiveData((res: number) => {
            arktest.assertEQ(res, expectedValue);
            onReceive.fetchAdd(1);
        });

        let p = taskpool.executeDelayed(2, task1, taskpool.Priority.HIGH)
            .then((res: Any): void => {
                arktest.assertEQ(res as string, expectedValue.toString());
                count.fetchAdd(1);
            })
        promises.push(p);
    }

    await Promise.all(promises);

    arktest.assertEQ(count.load(), 10);
    arktest.assertEQ(enqueueId1.load(), 10);
    arktest.assertEQ(startExecute.load(), 10);
    arktest.assertEQ(executeSuccess.load(), 10);
    arktest.assertEQ(onReceive.load(), 10);

    // After the first enqueue, further listener registrations should fail.
    arktest.assertEQ(errEnqueueCount.load(), 9);
    arktest.assertEQ(errStartCount.load(), 9);
    arktest.assertEQ(errSuccessCount.load(), 9);
}

function main(): int {
    let suite = new arktest.ArkTestsuite('taskpool.CurrentTaskTest');
    suite.addAsyncTest('CurrentTaskTest', currentTaskTest);
    let res = suite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
