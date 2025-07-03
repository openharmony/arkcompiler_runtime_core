/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

function returnAbc() {
    return (() => "a")() + (() => "b")() + (() => "c")();
}

function sum(a: int, b: int): int {
    return a + b;
}

function max(a: int, b: int): int {
    return (a > b) ? a : b;
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

function testDelayFunc(): NullishType {
    let task = new taskpool.Task(max, 80, 90);
    let res = await taskpool.executeDelayed(50.0, task);
    let runner = new taskpool.SequenceRunner();
    expectError(() => {
        runner.execute(task);
    }, new Error("taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask"));
    return res;
}

function executeDelayedTest(): void {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    try {
        let res = await launchImpl(testDelayFunc);
        assertEQ(res, 90);
    } catch(e: Error) {
        // NOTE(wangzhaoyong, #25035): del try catch when fix stackoverflow and invalid error message
        assertEQ(e instanceof StackOverflowError, true);
    }
    let runner = new taskpool.SequenceRunner();
    let task = new taskpool.Task(sum, 25, 35);
    await runner.execute(task);
    expectError(() => {
        taskpool.executeDelayed(100.0, task);
    }, new Error("taskpool:: seqRunnerTask cannot executeDelayed outside"));
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

function executePeriodicallyTest(): void {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    let runner = new taskpool.SequenceRunner();
    let task1 = new taskpool.Task(returnAbc);
    await runner.execute(task1);
    expectError(() => {
        taskpool.executePeriodically(100.0, task1);
    }, new Error("taskpool:: The concurrent task has been executed and cannot be executed periodically"));
    try {
        let res2 = await launchImpl(testPeriodFunc);
        assertEQ(res2, "success");
    } catch(e: Error) {
        // NOTE(wangzhaoyong, #25035): del try catch when fix stackoverflow and invalid error message
        assertEQ(e instanceof LinkerUnresolvedClassError, true);
    }
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

function testPeriodFunc(): NullishType {
    let task = new taskpool.Task(sum, 80, 90);
    taskpool.executePeriodically(200.0, task);
    let isTaskCanceled: AtomicFlag = new AtomicFlag(false);
    setTimeout(() => {
        taskpool.cancel(task);
        isTaskCanceled.set(true);
    }, 50);
    while (!isTaskCanceled.get()) {
        delayWithYield(50);
    }
    let runner = new taskpool.SequenceRunner();
    expectError(() => {
        runner.execute(task);
    }, new Error("taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask"));
    return "success";
}

function orderTest() {
    let resultStr: string = "";
    let func1 = (): string => {
        resultStr += "1";
        return resultStr;
    };
    let func2 = (): string => {
        resultStr += "2";
        return resultStr;
    }
    let func3 = (): string => {
        resultStr += "3";
        return resultStr;
    }
    let runner = new taskpool.SequenceRunner();
    let task1 = new taskpool.Task(func1);
    let task2 = new taskpool.Task(func2);
    let task3 = new taskpool.Task(func3);
    let p1 = runner.execute(task1);
    let p2 = runner.execute(task2);
    let p3 = runner.execute(task3);
    let res2 = await p2;
    arktest.assertEQ(res2, "12");
    let res3 = await p3;
    arktest.assertEQ(res3, "123");
    let res1 = await p1;
    arktest.assertEQ(res1, "1");
    arktest.assertEQ(resultStr, "123");
    arktest.assertEQ(task1.isDone(), true);
    arktest.assertEQ(task2.isDone(), true);
    arktest.assertEQ(task3.isDone(), true);
    // Execute LongTask
    let longTask = new taskpool.LongTask(returnAbc);
    let res = await runner.execute(longTask);
    arktest.assertEQ(res, "abc");
}

function executeCommonTest() {
    let runner = new taskpool.SequenceRunner();
    let task = new taskpool.Task(returnAbc);
    taskpool.execute(task);
    arktest.expectError(() => { runner.execute(task) },
                new Error("taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask"));
}

function executeSeqRunnerTaskTest() {
    let runner1 = new taskpool.SequenceRunner();
    let runner2 = new taskpool.SequenceRunner();
    let task = new taskpool.Task(returnAbc);
    runner1.execute(task);
    arktest.expectError(() => { runner2.execute(task) },
                new Error("taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask"));
}

function executeGroupTaskTest() {
    let runner = new taskpool.SequenceRunner();
    let group = new taskpool.TaskGroup();
    let task = new taskpool.Task(returnAbc);
    group.addTask(task);
    arktest.expectError(() => { runner.execute(task) },
                new Error("taskpool:: sequenceRunner cannot execute groupTask"));
}

function cancelNonStartedTaskTest() {
    let task = new taskpool.Task(returnAbc);
    let runner = new taskpool.SequenceRunner();
    arktest.expectError(() => {
        taskpool.cancel(task);
        runner.execute(task);
    }, new Error("taskpool:: task is not executed or has been executed"));
    // tasks with params
    let t1 = new taskpool.Task(sum, 10, 20);
    let res1 = await runner.execute(t1);
    arktest.assertEQ(res1, 30);
    let t2 = new taskpool.Task(max, 80, 30);
    let res2 = await runner.execute(t2);
    arktest.assertEQ(res2, 80);
}

function cancelExecutingTaskTest() {
    let task = new taskpool.Task(returnAbc);
    let runner = new taskpool.SequenceRunner();
    let p = runner.execute(task);
    try {
        taskpool.cancel(task);
    } catch(e: Error) {
        arktest.assertEQ(e.message, "taskpool:: sequenceRunner task has been executed");
        return;
    }
    arktest.expectError(() => { let res = await p; }, new Error("taskpool:: sequenceRunner task has been canceled"));
}

function cancelExecutedTaskTest() {
    let task = new taskpool.Task(returnAbc);
    let runner = new taskpool.SequenceRunner();
    arktest.expectError(() => {
        await runner.execute(task);
        taskpool.cancel(task);
    }, new Error("taskpool:: sequenceRunner task has been executed"));
    // tasks with params
    let t1 = new taskpool.Task(sum, 10, 20);
    let res1 = await runner.execute(t1);
    arktest.assertEQ(res1, 30);
    let t2 = new taskpool.Task(max, 80, 30);
    let res2 = await runner.execute(t2);
    arktest.assertEQ(res2, 80);
}

function addDependentTaskTest() {
    let task1 = new taskpool.Task(returnAbc);
    let task2 = new taskpool.Task(returnAbc);
    task1.addDependency(task2);
    let runner = new taskpool.SequenceRunner();
    arktest.expectError(() => { runner.execute(task1) }, new Error("taskpool:: sequenceRunner dependent task not allowed."));
}

function addDependencyExecutedTaskTest() {
    let task1 = new taskpool.Task(returnAbc);
    let task2 = new taskpool.Task(returnAbc);
    let runner = new taskpool.SequenceRunner();
    runner.execute(task2);
    arktest.expectError(() => { task1.addDependency(task2) }, new Error("taskpool:: seqRunnerTask or executedTask cannot be relied on"));
    arktest.expectError(() => { task2.addDependency(task1) }, new Error("taskpool:: seqRunnerTask or executedTask cannot addDependency"));
}

function taskWithDifferentParamsTest() {
    let testWithZeroParam = (): string => {
        return "testWithZeroParam";
    }
    let testWithOneParam = (num: int): int => {
        return num * 2;
    }
    let testWithThreeParams = (a: number, str: string, b: number): string => {
        let sum: number = a + b;
        return "testWithThreeParams res: " + sum;
    }
    let testWithFiveParams = (a: number, b: number, c: number, flag: boolean, str: string): string => {
        let res: number = 0;
        if (flag) {
            res = a + b + c;
        } else {
            res = a * b * c;
        }
        return str + " res: " + res;
    }
    let test = (a: number, b: number): number => {
        let t = Date.now();
        // mock time-consuming operation
        while ((Date.now() - t) < 500) {
            continue;
        }
        return a + b;
    }
    let task1 = new taskpool.Task(testWithZeroParam);
    let task2 = new taskpool.Task(testWithOneParam, 25);
    let task3 = new taskpool.Task(testWithThreeParams, 10.0, "task3", 20.0);
    let task4 = new taskpool.Task(testWithFiveParams, 1.0, 2.0, 3.0, true, "TEST");
    let longTask = new taskpool.LongTask(test, 10.0, 20.0);
    let runner = new taskpool.SequenceRunner();
    let res1 = await runner.execute(task1);
    let res2 = await runner.execute(task2);
    let res3 = await runner.execute(task3);
    let res4 = await runner.execute(task4);
    let res5 = await runner.execute(longTask);
    arktest.assertEQ(res1, "testWithZeroParam");
    arktest.assertEQ(res2, 50);
    arktest.assertEQ(res3, "testWithThreeParams res: 30");
    arktest.assertEQ(res4, "TEST res: 6");
    arktest.assertEQ(res5, 30);
    arktest.assertEQ(task1.isDone(), true);
    arktest.assertEQ(task2.isDone(), true);
    arktest.assertEQ(task3.isDone(), true);
    arktest.assertEQ(task4.isDone(), true);
    arktest.assertEQ(longTask.isDone(), true);
}

function main(): int {
    let seqRunnerSuite = new arktest.ArkTestsuite("taskpool.SequenceRunner");
    seqRunnerSuite.addTest("ExecuteDelayedTest", executeDelayedTest);
    seqRunnerSuite.addTest("ExecutePeriodicallyTest", executePeriodicallyTest);
    seqRunnerSuite.addTest("OrderTest", orderTest);
    seqRunnerSuite.addTest("ExecuteCommonTest", executeCommonTest);
    seqRunnerSuite.addTest("ExecuteSeqRunnerTaskTest", executeSeqRunnerTaskTest);
    seqRunnerSuite.addTest("ExecuteGroupTaskTest", executeGroupTaskTest);
    seqRunnerSuite.addTest("CancelNonStartedTaskTest", cancelNonStartedTaskTest);
    seqRunnerSuite.addTest("CancelExecutingTaskTest", cancelExecutingTaskTest);
    seqRunnerSuite.addTest("CancelExecutedTaskTest", cancelExecutedTaskTest);
    seqRunnerSuite.addTest("AddDependentTaskTest", addDependentTaskTest);
    seqRunnerSuite.addTest("AddDependencyExecutedTaskTest", addDependencyExecutedTaskTest);
    seqRunnerSuite.addTest("TaskWithDifferentParamsTest", taskWithDifferentParamsTest);
    let res = seqRunnerSuite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
