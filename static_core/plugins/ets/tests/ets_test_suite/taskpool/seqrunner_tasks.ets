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

function main(): int {
    let seqRunnerSuite = new ArkTestsuite("taskpool.SequenceRunner");
    seqRunnerSuite.addTest("OrderTest", () =>
    {
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
        assertEQ(res2, "12");
        let res3 = await p3;
        assertEQ(res3, "123");
        let res1 = await p1;
        assertEQ(res1, "1");
        assertEQ(resultStr, "123");
        assertEQ(task1.isDone(), true);
        assertEQ(task2.isDone(), true);
        assertEQ(task3.isDone(), true);
        // Execute LongTask
        let longTask = new taskpool.LongTask(returnAbc);
        let res = await runner.execute(longTask);
        assertEQ(res, "abc");
    });
    seqRunnerSuite.addTest("ExecuteCommonTest", (): void throws => 
    {
        let runner = new taskpool.SequenceRunner();
        let task = new taskpool.Task(returnAbc);
        taskpool.execute(task);
        expectError(() => { runner.execute(task) },
                    new Error("taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask"));
    });
    seqRunnerSuite.addTest("ExecuteSeqRunnerTaskTest", (): void throws =>
    {
        let runner1 = new taskpool.SequenceRunner();
        let runner2 = new taskpool.SequenceRunner();
        let task = new taskpool.Task(returnAbc);
        runner1.execute(task);
        expectError(() => { runner2.execute(task) },
                    new Error("taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask"));
    });
    seqRunnerSuite.addTest("ExecuteGroupTaskTest", (): void throws => {
        let runner = new taskpool.SequenceRunner();
        let group = new taskpool.TaskGroup();
        let task = new taskpool.Task(returnAbc);
        group.addTask(task);
        expectError(() => { runner.execute(task) },
                    new Error("taskpool:: sequenceRunner cannot execute groupTask"));
    });
    seqRunnerSuite.addTest("CancelNonStartedTaskTest", (): void throws =>
    {
        let task = new taskpool.Task(returnAbc);
        let runner = new taskpool.SequenceRunner();
        expectError(() => {
            taskpool.cancel(task);
            runner.execute(task);
        }, new Error("taskpool:: task is not executed or has been executed"));
        // tasks with params
        let t1 = new taskpool.Task(sum, 10, 20);
        let res1 = await runner.execute(t1);
        assertEQ(res1, 30);
        let t2 = new taskpool.Task(max, 80, 30);
        let res2 = await runner.execute(t2);
        assertEQ(res2, 80);
    });
    seqRunnerSuite.addTest("CancelExecutingTaskTest", (): void throws =>
    {
        let task = new taskpool.Task(returnAbc);
        let runner = new taskpool.SequenceRunner();
        let p = runner.execute(task);
        try {
            taskpool.cancel(task);
        } catch(e: Error) {
            assertEQ(e.message, "taskpool:: sequenceRunner task has been executed");
            return;
        }
        expectError(() => { let res = await p; }, new Error("taskpool:: sequenceRunner task has been canceled"));
    });
    seqRunnerSuite.addTest("CancelExecutedTaskTest", (): void throws =>
    {
        let task = new taskpool.Task(returnAbc);
        let runner = new taskpool.SequenceRunner();
        expectError(() => {
            await runner.execute(task);
            taskpool.cancel(task);
        }, new Error("taskpool:: sequenceRunner task has been executed"));
        // tasks with params
        let t1 = new taskpool.Task(sum, 10, 20);
        let res1 = await runner.execute(t1);
        assertEQ(res1, 30);
        let t2 = new taskpool.Task(max, 80, 30);
        let res2 = await runner.execute(t2);
        assertEQ(res2, 80);
    });
    seqRunnerSuite.addTest("AddDependentTaskTest", (): void throws =>
    {
        let task1 = new taskpool.Task(returnAbc);
        let task2 = new taskpool.Task(returnAbc);
        task1.addDependency(task2);
        let runner = new taskpool.SequenceRunner();
        expectError(() => { runner.execute(task1) }, new Error("taskpool:: sequenceRunner dependent task not allowed."));
    });
    seqRunnerSuite.addTest("AddDependencyExecutedTaskTest", (): void throws =>
    {
        let task1 = new taskpool.Task(returnAbc);
        let task2 = new taskpool.Task(returnAbc);
        let runner = new taskpool.SequenceRunner();
        runner.execute(task2);
        expectError(() => { task1.addDependency(task2) }, new Error("taskpool:: seqRunnerTask or executedTask cannot be relied on"));
        expectError(() => { task2.addDependency(task1) }, new Error("taskpool:: seqRunnerTask or executedTask cannot addDependency"));
    });
    seqRunnerSuite.addTest("TaskWithDifferentParamsTest", (): void throws =>
    {
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
        assertEQ(res1, "testWithZeroParam");
        assertEQ(res2, 50);
        assertEQ(res3, "testWithThreeParams res: 30");
        assertEQ(res4, "TEST res: 6");
        assertEQ(res5, 30);
        assertEQ(task1.isDone(), true);
        assertEQ(task2.isDone(), true);
        assertEQ(task3.isDone(), true);
        assertEQ(task4.isDone(), true);
        assertEQ(longTask.isDone(), true);
    });
    return seqRunnerSuite.run();
}
