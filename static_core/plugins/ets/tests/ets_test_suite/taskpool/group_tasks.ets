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

function returnAbc(): string {
    return (() => "a")() + (() => "b")() + (() => "c")();
}

function returnDef(): string {
    return (() => "de")() + (() => "f")();
}

function sum(a: int, b: int): int {
    return a + b;
}

function max(a: int, b: int): int {
    return (a > b) ? a : b;
}

async function promiseCase(duration: int): Promise<string> {
    let p: Promise<string> = new Promise<string>((resolve: (value: string) => void, reject: (reason: Error) => void) => {
        let t = Date.now();
        while ((Date.now() - t) < duration) {
            continue;
        }
        resolve("success");
    });
    return p;
}

function main(): int {
    let taskGroupTestsuite = new ArkTestsuite("taskpool.TaskGroup");
    taskGroupTestsuite.addTest("RunOneGroupTest", () =>
    {
        let g2 = new taskpool.TaskGroup("g2");
        assertEQ(g2.name, "g2");
        let t1 = new taskpool.Task(returnAbc);
        let t2 = new taskpool.Task(returnDef);
        // tasks with params
        let t3 = new taskpool.Task(sum, 10, 20);
        let t4 = new taskpool.Task(max, 80, 30);
        g2.addTask(t1);
        g2.addTask(t2);
        g2.addTask(t3);
        g2.addTask(t4);
        let res2 = await taskpool.execute(g2);
        assertEQ(res2[0], "abc");
        assertEQ(res2[1], "def");
        assertEQ(res2[2], 30);
        assertEQ(res2[3], 80);
        assertEQ(g2.name, "g2");
        // Run group without tasks
        let g0 = new taskpool.TaskGroup("g0");
        let res0 = await taskpool.execute(g0);
        assertEQ(res0.length, 0);
        assertEQ(g0.name, "g0");
    });
    taskGroupTestsuite.addTest("RunSeveralGroupsTest", () =>
    {
        let g1 = new taskpool.TaskGroup();
        let g2 = new taskpool.TaskGroup();
        let g3 = new taskpool.TaskGroup();
        // Group 1
        let t1 = new taskpool.Task(returnAbc);
        let t2 = new taskpool.Task(returnDef);
        g1.addTask(t1);
        g1.addTask(t2);
        g1.addTask(returnAbc);
        g1.addTask(sum, 10, 20);
        // Group 2
        g2.addTask(returnDef);
        g2.addTask(returnAbc);
        g2.addTask(max, 80, 30);
        // Execute group 1
        let p1 = taskpool.execute(g1);
        // Group 3
        g3.addTask(returnAbc);
        g3.addTask(new taskpool.Task(returnDef));
        g3.addTask(new taskpool.Task(sum, 100, 60));
        // Execute group 3 and group 2
        let p3 = taskpool.execute(g3);
        let p2 = taskpool.execute(g2);
        // await result for all groups
        let res1 = await p1;
        let res3 = await p3;
        let res2 = await p2;
        // Asserts for group 1
        assertEQ(res1[0], "abc");
        assertEQ(res1[1], "def");
        assertEQ(res1[2], "abc");
        assertEQ(res1[3], 30);
        assertEQ(t1.isDone(), true);
        assertEQ(t2.isDone(), true);
        // Asserts for group 2
        assertEQ(res2[0], "def");
        assertEQ(res2[1], "abc");
        assertEQ(res2[2], 80);
        // Asserts for group 3
        assertEQ(res3[0], "abc");
        assertEQ(res3[1], "def");
        assertEQ(res3[2], 160);
    });
    taskGroupTestsuite.addTest("CancelNonStartedGroupTest", (): void throws =>
    {
        let g = new taskpool.TaskGroup();
        g.addTask(returnAbc);
        g.addTask(returnDef);
        expectError(() => {
            taskpool.cancel(g);
            taskpool.execute(g);
        }, new Error("taskpool:: taskGroup is not executed or has been executed"));
    });
    taskGroupTestsuite.addTest("CancelExecutingGroupTest", (): void throws =>
    {
        let g = new taskpool.TaskGroup();
        g.addTask(returnAbc);
        g.addTask(returnDef);
        let p = taskpool.execute(g);
        try {
            taskpool.cancel(g);
        } catch(e: Error) {
            assertEQ(e.message, "taskpool:: taskGroup is not executed or has been executed");
            return;
        }
        expectError(() => { await p }, new Error("taskpool:: taskGroup has been canceled"));
    });
    taskGroupTestsuite.addTest("CancelExecutedGroupTest1", (): void throws =>
    {
        let g = new taskpool.TaskGroup();
        g.addTask(returnAbc);
        g.addTask(returnDef);
        expectError(() => {
            await taskpool.execute(g);
            // Group tasks finished, so 'cancel' should produce the Error
            taskpool.cancel(g);
        }, new Error("taskpool:: taskGroup is not executed or has been executed"));
    });
    taskGroupTestsuite.addTest("CancelExecutedGroupTest2", (): void throws =>
    {
        // Cancel the completed taskGroup in the then branch.
        let result: int[] = [1, 1, 1];
        let g = new taskpool.TaskGroup();
        let task = new taskpool.Task(sum, 15, 15);
        g.addTask(sum, 100, 60);
        g.addTask(max, 80, 30);
        g.addTask(task);
        taskpool.execute(g).then((res: object): void => {
            expectError(() => {
                taskpool.cancel(g);
            }, new Error("taskpool:: taskGroup is not executed or has been executed"));
            result = res as int[];
            assertEQ(result[0], 160);
            assertEQ(result[1], 80);
            assertEQ(result[2], 30);
        });
    });
    taskGroupTestsuite.addTest("AddExecutedTaskTest", (): void throws =>
    {
        let g = new taskpool.TaskGroup();
        let expectedError = new Error("taskpool:: taskGroup cannot add seqRunnerTask or executedTask");
        // Executed task
        let task = new taskpool.Task(returnAbc);
        taskpool.execute(task);
        expectError(() => { g.addTask(task) }, expectedError);
        // Executed task via SequenceRunner
        let runnerTask = new taskpool.Task(returnAbc);
        let runner = new taskpool.SequenceRunner(); 
        runner.execute(runnerTask);
        expectError(() => { g.addTask(runnerTask) }, expectedError);
    });
    taskGroupTestsuite.addTest("AddLongTaskTest", (): void throws =>
    {
        let g = new taskpool.TaskGroup();
        let t = new taskpool.LongTask(returnAbc);
        expectError(() => { g.addTask(t) },
                    new Error("taskpool:: The interface does not support the long task"));
    });
    taskGroupTestsuite.addTest("AddTaskFromGroupTest", (): void throws =>
    {
        let g1 = new taskpool.TaskGroup();
        let t1 = new taskpool.Task(returnAbc);
        g1.addTask(t1);
        let g2 = new taskpool.TaskGroup();
        let t2 = new taskpool.Task(returnDef);
        g2.addTask(t2);
        let expectedError = new Error("taskpool:: taskGroup cannot add groupTask");
        // Add the same task
        expectError(() => { g1.addTask(t1) }, expectedError);
        // Add task from another group
        expectError(() => { g1.addTask(t2) }, expectedError);
    });
    taskGroupTestsuite.addTest("AddDependentTaskTest", (): void throws =>
    {
        let task1 = new taskpool.Task(returnAbc);
        let task2 = new taskpool.Task(returnAbc);
        task1.addDependency(task2);
        let taskGroup = new taskpool.TaskGroup();
        expectError(() => { taskGroup.addTask(task1) }, new Error("taskpool:: dependent task not allowed."));
    });
    taskGroupTestsuite.addTest("AddDependencyForGroupTaskTest", (): void throws =>
    {
        let task1 = new taskpool.Task(returnAbc);
        let taskGroup = new taskpool.TaskGroup();
        taskGroup.addTask(task1);
        let task2 = new taskpool.Task(returnAbc);
        expectError(() => { task1.addDependency(task2) }, new Error("taskpool:: groupTask cannot addDependency"));
        expectError(() => { task2.addDependency(task1) }, new Error("taskpool:: groupTask cannot be relied on"));
    });
    taskGroupTestsuite.addTest("AddTaskWithDifferentParamsTest", (): void throws =>
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
        let task2 = new taskpool.Task(testWithOneParam, 15);
        let task3 = new taskpool.Task(testWithThreeParams, 10.0, "task3", 20.0);
        let task4 = new taskpool.Task(testWithFiveParams, 1.0, 2.0, 3.0, true, "TEST");
        let longTask = new taskpool.LongTask(test, 10.0, 20.0);
        let taskGroup = new taskpool.TaskGroup();
        taskGroup.addTask(task1);
        taskGroup.addTask(task2);
        taskGroup.addTask(task3);
        taskGroup.addTask(task4);
        expectError(() => { taskGroup.addTask(longTask) },
                    new Error("taskpool:: The interface does not support the long task"));
        let res = await taskpool.execute(taskGroup);
        assertEQ(res[0], "testWithZeroParam");
        assertEQ(res[1], 30);
        assertEQ(res[2], "testWithThreeParams res: 30");
        assertEQ(res[3], "TEST res: 6");
        assertEQ(task1.isDone(), true);
        assertEQ(task2.isDone(), true);
        assertEQ(task3.isDone(), true);
        assertEQ(task4.isDone(), true);
        assertEQ(longTask.isDone(), false);
    });
    return taskGroupTestsuite.run();
}
