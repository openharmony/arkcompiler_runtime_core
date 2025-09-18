/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the ”="License");
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

function sumFunc(): int {
    return (() => 10)() + (() => 20)();
}

function sum(a: int, b: int): int {
    let t = Date.now();
    // mock time-consuming operation
    while ((Date.now() - t) < 500) {
        continue;
    }
    return a + b;
}

function delayWithYield(duration: int): void {
    let t = Date.now();
    while ((Date.now() - t) < duration) {}
    Coroutine.Schedule();
}

function launchImpl(func: () => NullishType): Promise<NullishType> {
    const job: Job<NullishType> = launch<NullishType, () => NullishType>(
        func
    );
    return new Promise<NullishType>((resolve, reject) => {
        try {
            let res = job.Await();
            resolve(res);
        } catch(e: Error) {
            reject(e);
        }
    });
}

async function testDelayFunc(): Promise<Any> {
    let task = new taskpool.LongTask(sumFunc);
    let res = await taskpool.executeDelayed(10, task);
    arktest.expectError(() => {
        taskpool.execute(task);
    }, new Error('taskpool:: The long task can only be executed once'));
    return res;
}

async function executeDelayedTest(): Promise<void> {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    try {
        let res = await launchImpl(testDelayFunc);
        arktest.assertEQ(res, 30);
    } catch(e: Error) {
        // NOTE(wangzhaoyong, #25035): del try catch when fix stackoverflow and invalid error message
        arktest.assertEQ(e instanceof StackOverflowError, true);
    }
    let task = new taskpool.LongTask(sum, 10, 80);
    await taskpool.execute(task);
    arktest.expectError(() => {
        taskpool.executeDelayed(10, task);
    }, new Error('taskpool:: Multiple executions of LongTask are not supported in the executeDelayed'));
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

async function executePeriodicallyTest(): Promise<void> {
    let task1 = new taskpool.LongTask(sumFunc);
    await taskpool.execute(task1);
    arktest.expectError(() => {
        taskpool.executePeriodically(100, task1);
    }, new Error('taskpool:: The concurrent task has been executed and cannot be executed periodically'));
}

async function executeTest() {
    let task = new taskpool.LongTask(sumFunc);
    let result = await taskpool.execute(task);
    arktest.assertEQ(result, 30);
}

async function doubleExecutionTest() {
    let task = new taskpool.LongTask(sumFunc);
    taskpool.execute(task);
    arktest.expectError(() => { taskpool.execute(task) },
                new Error('taskpool:: The long task can only be executed once'));
}

async function longTaskWithParamsTest() {
    let task = new taskpool.LongTask(sum, 100, 200);
    let res = await taskpool.execute(task);
    arktest.assertEQ(res, 300);
}

function main():int {
    let longTaskTestsuite = new arktest.ArkTestsuite('taskpool.LongTask');
    longTaskTestsuite.addAsyncTest('ExecuteDelayedTest', executeDelayedTest);
    longTaskTestsuite.addAsyncTest('ExecutePeriodicallyTest', executePeriodicallyTest);
    longTaskTestsuite.addAsyncTest('ExecuteTest', executeTest);
    longTaskTestsuite.addAsyncTest('DoubleExecutionTest', doubleExecutionTest);
    longTaskTestsuite.addAsyncTest('LongTaskWithParamsTest', longTaskWithParamsTest);
    let res = longTaskTestsuite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
