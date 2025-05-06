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

import { CoroutineExtras, AtomicFlag } from "std/debug/concurrency";

function returnAbc(): string {
    return (() => "a")() + (() => "b")() + (() => "c")();
}

function returnFailAbc(): string {
    throw new Error("Concurrent function failed");
}

function sum(a: int, b: int): int {
    return a + b;
}

function max(a: int, b: int): int {
    return (a > b) ? a : b;
}

function longFunc(duration: int): string {
    let t = Date.now();
    while ((Date.now() - t) < duration) {
        continue;
    }
    return "success";
}

function testCancelFunc(duration: int): boolean {
    let t = Date.now();
    while((Date.now() - t) < duration) {
        continue;
    }
    if (taskpool.Task.isCanceled()) {
        return false;
    }
    return true;
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
    let task1 = new taskpool.Task(returnFailAbc);
    let res: string = "";
    let isTask1Finished: AtomicFlag = new AtomicFlag(false);
    taskpool.executeDelayed(100, task1).catch((e: NullishType) => {
        res = (e as Error).message;
        isTask1Finished.set(true);
    })
    while (!isTask1Finished.get()) {
        delayWithYield(50);
    }
    return res;
}

function taskExecuteDelayedTest(): void {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    try {
        let res1 = await launchImpl(testDelayFunc);
        assertEQ(res1, "Concurrent function failed");
        let task2 = new taskpool.Task(longFunc, 50);
        expectError(() => {
            await taskpool.executeDelayed(-100.0, task2);
        }, new Error("taskpool:: The delayTime is less than zero"));
    } catch(e: Error) {
        // NOTE(wangzhaoyong, #25035): del try catch when fix stackoverflow and invalid error message
        assertEQ(e instanceof NullPointerError, true);
    }
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

let verifyCount = 0;
function verifySendDataRes(a:int):void{
    assertEQ(a, 10)
    verifyCount++;
}

function verifySendDataResWithRestParameter(...args:FixedArray<NullishType>) {
    let arg1 = args[0] as Array<int>;
    let arg2 = args[1] as Array<string>;
    assertEQ(arg1[0], 1);
    assertEQ(arg1[1], 2);
    assertEQ(arg2[0], "hello");
    assertEQ(arg2[1], "world");
}

function testOnReceiveDataRestParameter():void {
    let task = new taskpool.Task(() => {
        let arg = new Array<int>(1, 2);
        taskpool.Task.sendData(arg, new Array<string>("hello", "world"));
    })
    task.onReceiveData(verifySendDataResWithRestParameter)
    await taskpool.execute(task);
}

function taskOnReceiveData():void {
    clearCount();
    let task = new taskpool.Task(() => {
        taskpool.Task.sendData(10);
        taskpool.Task.sendData(10);
        taskpool.Task.sendData(10);
        taskpool.Task.sendData(10);
    });
    task.onReceiveData(verifySendDataRes);
    await taskpool.execute(task);
    assertEQ(verifyCount, 4);
}

function clearCount():void {
    verifyCount = 0;
}

function taskSendData():void {
    clearCount();
    let task = new taskpool.Task(() => {
        taskpool.Task.sendData(10);
        taskpool.Task.sendData(undefined);
        taskpool.Task.sendData(null);
        taskpool.Task.sendData("string");
        taskpool.Task.sendData(1.0);
    });

    task.onReceiveData((): void => {
        verifyCount++
    });
    await taskpool.execute(task);
    assertEQ(verifyCount, 5);
}

function taskExecutePeriodicallyTest(): void {
    let task1 = new taskpool.Task(returnAbc);
    expectError(() => {
        taskpool.executePeriodically(-100.0, task1);
    }, new Error("taskpool:: The period value is less than zero"));
    let res = await taskpool.execute(task1);
    assertEQ(res, "abc");
    expectError(() => {
        taskpool.executePeriodically(100.0, task1);
    }, new Error("taskpool:: The concurrent task has been executed and cannot be executed periodically"));
}

function taskWithDependencyExecutePeriodicallyTest(): void {
    let task1 = new taskpool.Task(sum, 5, 25);
    let task2 = new taskpool.Task(max, 36, 12);
    task1.addDependency(task2);
    expectError(() => {
        taskpool.executePeriodically(10.0, task1);
    }, new Error("taskpool:: the task with dependency cannot executePeriodically"));
    expectError(() => {
        taskpool.executePeriodically(100.0, task2);
    }, new Error("taskpool:: the task with dependency cannot executePeriodically"));
}

function testTaskpoolInfo(){
    // Check non empty threadInfo without tasks
    let emptyThreadInfo = taskpool.getTaskPoolInfo();
    assertNE(emptyThreadInfo.threadInfos.length, 0);
    let flagForTask: AtomicFlag = new AtomicFlag(false);
    let isRunTask: AtomicFlag = new AtomicFlag(false);
    let funcForTask = (): string => {
        isRunTask.set(true);
        while (flagForTask.get() != true) {}
        return "result"
    };
    let task = new taskpool.Task("task_name", funcForTask);
    let info1 = taskpool.getTaskPoolInfo();
    // Obtain the worker pool info, and the behavior is the same as 1.0 taskpool
    assertNE(info1.threadInfos.length, 0);
    let p = taskpool.execute(task, taskpool.Priority.HIGH);
    // wait for task started
    while (isRunTask.get() != true) {}
    let info2 = taskpool.getTaskPoolInfo();
    // Notify to continue task
    flagForTask.set(true);
    // Check current priority of the worker
    assertEQ(info2.threadInfos[0].priority, taskpool.Priority.HIGH);
    assertEQ(info2.taskInfos.length, 1);
    assertNE(info2.threadInfos.length, 0);
    let isNonEmptyWorker: boolean = false;
    for (let threadInfo of info2.threadInfos) {
        assertLT(0, threadInfo.tid);
        isNonEmptyWorker = isNonEmptyWorker || (threadInfo.taskIds != undefined);
    }
    assertEQ(isNonEmptyWorker, true);
    // NOTE(ipetrov, #20349): uncomment when frontend fix enum comparison
    assertEQ(info2.taskInfos[0].state as int, taskpool.State.RUNNING as int);
    assertEQ(info2.taskInfos[0].name, "task_name");
    assertEQ(await p, "result");
}

function main(): int {
    let commonTaskSuite = new ArkTestsuite("taskpool.CommonTask");
    let commonTaskSuiteDisabled = new ArkTestsuite("taskpool.CommonTask.DISABLED");
    commonTaskSuite.addTest("GetTaskpoolInfoTest", testTaskpoolInfo);
    commonTaskSuite.addTest("OneTaskExecutionTest", () =>
    {
        let Sum = (): int => {
            return 10 + 20;
        }
        // Task with lambda
        let lambdaTask = new taskpool.Task(Sum);
        let result = await taskpool.execute(lambdaTask);
        assertEQ(result, 30);
        assertEQ(lambdaTask.isDone(), true);
        // Task with function
        let funcTask = new taskpool.Task(returnAbc);
        let res = await taskpool.execute(funcTask);
        assertEQ(res, "abc");
        assertEQ(funcTask.isDone(), true);
        // Disallow execute one task several times
        expectError(() => {
            taskpool.execute(funcTask);
        }, new Error("taskpool:: task has been executed"));
        // Tasks with params
        let task1 = new taskpool.Task(sum, 10, 20);
        let res1 = await taskpool.execute(task1);
        assertEQ(res1, 30);
        assertEQ(task1.isDone(), true);
        let task2 = new taskpool.Task(max, 80, 60);
        let res2 = await taskpool.execute(task2);
        assertEQ(res2, 80);
        assertEQ(task2.isDone(), true);
    });
    commonTaskSuite.addTest("CallbackTest", (): void throws =>
    {
        let onEnqueuedStr = "";
        let onEnqueuedExpectedStr = "enqueue callback is done";
        let onEnqueuedCallback = () => { onEnqueuedStr = onEnqueuedExpectedStr; };
        let onStartExecutionStr = "";
        let onStartExecutionExpepctedStr = "start callback is done";
        let onStartExecutionCallback = () => { onStartExecutionStr = onStartExecutionExpepctedStr; };
        let onExecutionSucceededStr = "";
        let onExecutionSucceededExpectedStr = "success callback is done";
        let onExecutionSucceededCallback = () => { onExecutionSucceededStr = onExecutionSucceededExpectedStr; };
        let onExecutionFailedStr = "";
        let onExecutionFailedExpectedStr = "Concurrent function failed, fail callback is done";
        let onExecutionFailedCallback = (e: Error) => { onExecutionFailedStr = e.message + ", fail callback is done"; };
        // Successed task
        let task1 = new taskpool.Task(returnAbc);
        task1.onEnqueued(onEnqueuedCallback);
        task1.onStartExecution(onStartExecutionCallback);
        task1.onExecutionSucceeded(onExecutionSucceededCallback);
        task1.onExecutionFailed(onExecutionFailedCallback);
        let res1 = await taskpool.execute(task1);
        assertEQ(res1, "abc");
        assertEQ(onEnqueuedStr, onEnqueuedExpectedStr);
        assertEQ(onStartExecutionStr, onStartExecutionExpepctedStr);
        assertEQ(onExecutionSucceededStr, onExecutionSucceededExpectedStr);
        assertEQ(onExecutionFailedStr, "");
        onEnqueuedStr = "";
        onStartExecutionStr = "";
        onExecutionSucceededStr = "";
        // Successed task with params
        let task2 = new taskpool.Task(sum, 10, 20);
        task2.onEnqueued(onEnqueuedCallback);
        task2.onStartExecution(onStartExecutionCallback);
        task2.onExecutionSucceeded(onExecutionSucceededCallback);
        task2.onExecutionFailed(onExecutionFailedCallback);
        let res2 = await taskpool.execute(task2);
        assertEQ(res2, 30);
        assertEQ(onEnqueuedStr, onEnqueuedExpectedStr);
        assertEQ(onStartExecutionStr, onStartExecutionExpepctedStr);
        assertEQ(onExecutionSucceededStr, onExecutionSucceededExpectedStr);
        assertEQ(onExecutionFailedStr, "");
        // Failed task
        onEnqueuedStr = "";
        onStartExecutionStr = "";
        onExecutionSucceededStr = "";
        let failedTask = new taskpool.Task(returnFailAbc);
        failedTask.onEnqueued(onEnqueuedCallback);
        failedTask.onStartExecution(onStartExecutionCallback);
        failedTask.onExecutionSucceeded(onExecutionSucceededCallback);
        failedTask.onExecutionFailed(onExecutionFailedCallback);
        expectError(() => { await taskpool.execute(failedTask) },
                    new Error("Concurrent function failed"));
        assertEQ(onEnqueuedStr, onEnqueuedExpectedStr);
        assertEQ(onStartExecutionStr, onStartExecutionExpepctedStr);
        assertEQ(onExecutionSucceededStr, "");
        assertEQ(onExecutionFailedStr, onExecutionFailedExpectedStr);
        // Add callbacks for executed task
        let executedTask = new taskpool.Task(returnAbc);
        let expectedCallbackError = new Error("taskpool:: The executed task does not support the registration of listeners.");
        taskpool.execute(executedTask);
        expectError(() => { executedTask.onEnqueued(() => {}); }, expectedCallbackError);
        expectError(() => { executedTask.onStartExecution(() => {}); }, expectedCallbackError);
        expectError(() => { executedTask.onExecutionSucceeded(() => {}); }, expectedCallbackError);
        expectError(() => { executedTask.onExecutionFailed((e: Error) => {}); }, expectedCallbackError);
    });
    commonTaskSuite.addTest("IsCancelTest", (): void throws =>
    {
        assertEQ(taskpool.Task.isCanceled(), false);
        let t = new taskpool.Task(returnAbc);
        let res = await taskpool.execute(t);
        assertEQ(res, "abc");
        assertEQ(taskpool.Task.isCanceled(), false);
        // Check canceling of exeuting task
        let isCanceled: AtomicFlag = new AtomicFlag(false);
        let isRunning: AtomicFlag = new AtomicFlag(false);
        let waitForCancel: AtomicFlag = new AtomicFlag(true);
        let funcForTask = (): boolean => {
            isRunning.set(true);
            while(waitForCancel.get()) {}
            // Set isCanceled value from taskpool
            isCanceled.set(taskpool.Task.isCanceled());
            isRunning.set(false);
            return true;
        };
        let task = new taskpool.Task(funcForTask);
        let p = taskpool.execute(task);
        while (!isRunning.get()) {}
        assertEQ(task.isDone(), false);
        taskpool.cancel(task);
        waitForCancel.set(false);
        while (isRunning.get()) {}
        // Check isCanceled from taskpool
        assertEQ(isCanceled.get(), true);
        expectError(() => { await p }, new Error("taskpool:: task has been canceled"));
    });
    commonTaskSuite.addTest("CancelNonExecutingTaskTest", (): void throws =>
    {
        let expectedError = new Error("taskpool:: task is not executed or has been executed");
        expectError(() => {
            let t = new taskpool.Task(returnAbc);
            taskpool.cancel(t);
            taskpool.execute(t);
        }, expectedError);
        expectError(() => {
            let t = new taskpool.Task(returnAbc);
            await taskpool.execute(t);
            taskpool.cancel(t);
        }, expectedError);
        // tasks with params
        expectError(() => {
            let t = new taskpool.Task(sum, 10, 20);
            taskpool.cancel(t);
            taskpool.execute(t);
        }, expectedError);
        expectError(() => {
            let t = new taskpool.Task(sum, 10, 20);
            await taskpool.execute(t);
            taskpool.cancel(t);
        }, expectedError);
    });
    commonTaskSuite.addTest("CancelExecutingTaskTest", (): void throws =>
    {
        let task1 = new taskpool.Task(returnAbc);
        let p1 = taskpool.execute(task1);
        try {
            taskpool.cancel(task1);
        } catch(e: Error) {
            assertEQ(e.message, "taskpool:: task is not executed or has been executed");
            return;
        }
        expectError(() => { await p1 }, new Error("taskpool:: task has been canceled"));
        // task finished, disallow new execution
        expectError(() => {
            await taskpool.execute(task1);
        }, new Error("taskpool:: task has been executed"));
        let task2 = new taskpool.Task(testCancelFunc, 300);
        let val: int = 0;
        let isExecuted: AtomicFlag = new AtomicFlag(false);
        taskpool.execute(task2).then((value: boolean): void => {
            val = 1;
            isExecuted.set(true);
        }).catch((e: object): void => {
            val = 2;
            isExecuted.set(true);
        });
        await promiseCase(50);
        try {
            taskpool.cancel(task2);
        } catch(e: Error) {
            assertEQ(e.message, "taskpool:: task is not executed or has been executed");
            return;
        }
        while (isExecuted.get() != true) {
            delayWithYield(10);
        }
        assertEQ(task2.isDone(), true);
        // The canceled task result returns through the catch branch, not the then branch.
        assertEQ(val, 2);
    });
    commonTaskSuite.addTest("CheckIsDoneAndCancelTaskTest", (): void throws =>
    {
        let isCanceled: AtomicFlag = new AtomicFlag(false);
        let isRunning: AtomicFlag = new AtomicFlag(false);
        let waitCancel: AtomicFlag = new AtomicFlag(true);
        let funcForTask = (duration: int): boolean => {
            isRunning.set(true);
            while(waitCancel.get()) {}
            // mock time-consuming operation
            longFunc(duration);
            if (taskpool.Task.isCanceled()) {
                isCanceled.set(true);
                isRunning.set(false);
                return false;
            }
            // mock time-consuming operation
            longFunc(duration);
            isRunning.set(false);
            return true;
        };
        let task1 = new taskpool.Task(funcForTask, 50);
        let p1 = taskpool.execute(task1);
        while (!isRunning.get()) {}
        // Cancel the executing task will not throw an error.
        expectNoThrow(() => {
            if (!task1.isDone()) {
                taskpool.cancel(task1);
            } else {
                throw new Error("The task isDone status is wrong.");
            }
        });
        waitCancel.set(false);
        while (isRunning.get()) {}
        assertEQ(isCanceled.get(), true);
        expectError(() => { await p1 }, new Error("taskpool:: task has been canceled"));
        let task2 = new taskpool.Task(testCancelFunc, 100);
        expectError(() => {
            await taskpool.execute(task2);
            // If isDone is true, cancel the task will throw an error.
            if (task2.isDone()) {
                taskpool.cancel(task2);
            }
        }, new Error("taskpool:: task is not executed or has been executed"));
    });
    commonTaskSuite.addTest("DependentTasksTest", (): void throws =>
    {
        let resultStr: string = "";
        let func1 = (): string => {
            resultStr += "1";
            return resultStr;
        };
        let func2 = (a: string): string => {
            resultStr += a;
            return resultStr;
        }
        let func3 = (): string => {
            resultStr += "3";
            return resultStr;
        }
        let task1 = new taskpool.Task(func1);
        let task2 = new taskpool.Task(func2, "2");
        let task3 = new taskpool.Task(func3);
        task1.addDependency(task2);
        task2.addDependency(task3);
        let p1 = taskpool.execute(task1);
        let p2 = taskpool.execute(task2);
        let p3 = taskpool.execute(task3);
        let res2 = await p2;
        assertEQ(res2, "32");
        let res3 = await p3;
        assertEQ(res3, "3");
        let res1 = await p1;
        assertEQ(res1, "321");
        assertEQ(resultStr, "321");
        // Empty dependencies
        expectError(() => {
            let task = new taskpool.Task(returnAbc);
            task.addDependency();
        }, new Error("addDependency has no params."));
        expectError(() => {
            let task = new taskpool.Task(returnAbc);
            task.removeDependency();
        }, new Error("removeDependency has no params."));
    });
    commonTaskSuite.addTest("DependentTasksWithParamsTest", (): void throws =>
    {
        let func1 = (str: string): string => {
            return str;
        };
        let func2 = (num: int): int => {
            let t = Date.now();
            while ((Date.now() - t) < 100) {}
            return num;
        }
        let func3 = (duration: int): string => {
            let t = Date.now();
            while ((Date.now() - t) < duration) {}
            return "Task3";
        }
        let task1 = new taskpool.Task(func1, "1");
        let task2 = new taskpool.Task(func2, 222);
        let task3 = new taskpool.Task(func3, 500);
        task1.addDependency(task2);
        task2.addDependency(task3);
        task1.removeDependency(task2);
        task2.removeDependency(task3);
        let p1 = taskpool.execute(task1);
        let p2 = taskpool.execute(task2);
        let p3 = taskpool.execute(task3);
        let res1 = await p1;
        assertEQ(res1, "1");
        let res2 = await p2;
        assertEQ(res2, 222);
        let res3 = await p3;
        assertEQ(res3, "Task3");
        assertEQ(task1.isDone(), true);
        assertEQ(task2.isDone(), true);
        assertEQ(task3.isDone(), true);
        let task4 = new taskpool.Task(func3, 100);
        let task5 = new taskpool.Task(func3, 200);
        let task6 = new taskpool.Task(func3, 100);
        let task7 = new taskpool.Task(func3, 200);
        // one task depends on multiple tasks
        task4.addDependency(task5, task6, task7);
        let p4 = taskpool.execute(task4);
        taskpool.execute(task5);
        taskpool.execute(task6);
        taskpool.execute(task7);
        await p4;
        assertEQ(task5.isDone(), true);
        assertEQ(task6.isDone(), true);
        assertEQ(task7.isDone(), true);
    });
    commonTaskSuite.addTest("CircularDependencyTest", (): void throws =>
    {
        let task1 = new taskpool.Task(returnAbc);
        let task2 = new taskpool.Task(returnAbc);
        let task3 = new taskpool.Task(returnAbc);
        let task4 = new taskpool.Task(returnAbc);
        let task5 = new taskpool.Task(returnAbc);
        let task6 = new taskpool.Task(returnAbc);
        let expectedError = new Error("There is a circular dependency");
        expectNoThrow(() => {
            task1.addDependency(task2);
            task2.addDependency(task3);
            task3.addDependency(task4);
            task4.addDependency(task5);
            task5.addDependency(task6);
        });
        expectError(() => {
            // circular dependency
            task6.addDependency(task1);
        }, expectedError);
        expectError(() => { task1.addDependency(task1) }, expectedError);
        expectError(() => { task1.removeDependency(task3) }, new Error("The dependency does not exist, "));
    });
    commonTaskSuite.addTest("DependencyExecutedTaskTest", (): void throws =>
    {
        let task1 = new taskpool.Task(returnAbc);
        let task2 = new taskpool.Task(sum, 10, 20);
        expectError(() => { task1.removeDependency(task2) }, new Error("taskpool:: task has no dependency"));
        let res1 = await taskpool.execute(task2);
        assertEQ(res1, 30);
        expectError(() => { task1.addDependency(task2) }, new Error("taskpool:: seqRunnerTask or executedTask cannot be relied on"));
        expectError(() => { task2.addDependency(task1) }, new Error("taskpool:: seqRunnerTask or executedTask cannot addDependency"));
        let task3 = new taskpool.Task(returnAbc);
        let task4 = new taskpool.Task(max, 30, 10);
        let expectedAgainExecuteError = new Error("taskpool:: executedTask with dependency cannot execute again");
        task1.addDependency(task3);
        let res2 = await taskpool.execute(task3);
        assertEQ(res2, "abc");
        expectError(() => { task1.removeDependency(task3); }, new Error("taskpool:: cannot removeDependency on a dependent and executed task"));
        expectError(() => { task1.removeDependency(task4); }, new Error("taskpool:: task has no dependency"));
        let res3 = await taskpool.execute(task1);
        assertEQ(res3, "abc");
        expectError(() => { await taskpool.execute(task1) }, expectedAgainExecuteError);
        expectError(() => { await taskpool.execute(task3) }, expectedAgainExecuteError);
        // Try to remove dependency after execute
        let task5 = new taskpool.Task(returnAbc);
        let task6 = new taskpool.Task(returnAbc);
        task5.addDependency(task6);
        taskpool.execute(task5);
        expectError(() => { task5.removeDependency(task6); }, new Error("taskpool:: executedTask cannot removeDependency"));
        taskpool.execute(task6);
    });
    commonTaskSuite.addTest("SendReceiveDataTest", () =>
    {
        let res = 0;
        let recvCallback = (): string => {
            res += 10;
            return "";
        };
        let sendDataFunc = (): string => {
            taskpool.Task.sendData();
            return "abc";
        };
        let t1 = new taskpool.Task(sendDataFunc);
        t1.onReceiveData(recvCallback);
        let t2 = new taskpool.Task(sendDataFunc);
        t2.onReceiveData(recvCallback);
        let t3 = new taskpool.Task(sendDataFunc);
        t3.onReceiveData(recvCallback);
        let p1 = taskpool.execute(t1);
        let p2 = taskpool.execute(t2);
        let res1 = await p1;
        let res2 = await p2;
        assertEQ(res1, "abc");
        assertEQ(res2, "abc");
        assertEQ(res, 20);
        let r = await taskpool.execute(t3);
        assertEQ(r, "abc");
        assertEQ(res, 30);
    });
    commonTaskSuite.addTest("SendDataAndCancelTest", () =>
    {
        // Cancel the task after the sendData is executed on the taskpool task.
        let value = 0;
        let recvCallback = (): string => {
            value += 10;
            return "";
        };
        let isRunning: AtomicFlag = new AtomicFlag(false);
        let sendDataFunc = (): string => {
            isRunning.set(true);
            taskpool.Task.sendData();
            let t = Date.now();
            while ((Date.now() - t) < 200) {
                continue;
            }
            if (taskpool.Task.isCanceled()) {
                return "def";
            }
            t = Date.now();
            while ((Date.now() - t) < 200) {
                continue;
            }
            taskpool.Task.sendData();
            return "abc";
        };
        let task = new taskpool.Task(sendDataFunc);
        task.onReceiveData(recvCallback);
        let p = taskpool.execute(task);
        while (!isRunning.get()) {
            delayWithYield(10);
        }
        try {
            taskpool.cancel(task);
        } catch(e: Error) {
            assertEQ(e.message, "taskpool:: task is not executed or has been executed");
            return;
        }
        expectError(() => { await p }, new Error("taskpool:: task has been canceled"));
        // When the task is canceled, recvCallback will no longer be executed.
        assertEQ(value, 0);
    });
    commonTaskSuite.addTest("TasksWithDifferentParamsTest", () =>
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
            res = flag ? (a + b + c) : (a * b * c);
            return str + " res: " + res;
        }
        let testWith16Params = (a: number, b: number, c: number, d: number, e: number, f: number, g: number, h: number,
                                i: number, j: number, k: number, l: number, m: number, n: number, o: number, p: number): number => {
            return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
        }
        let task1 = new taskpool.Task(testWithZeroParam);
        let task2 = new taskpool.Task(testWithOneParam, 15);
        let task3 = new taskpool.Task(testWithThreeParams, 10.0, "task3", 20.0);
        let task4 = new taskpool.Task(testWithFiveParams, 1.0, 2.0, 3.0, true, "TEST");
        let task5 = new taskpool.Task(testWith16Params, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                                        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
        let res1 = await taskpool.execute(task1);
        let res2 = await taskpool.execute(task2);
        let res3 = await taskpool.execute(task3);
        let res4 = await taskpool.execute(task4);
        let res5 = await taskpool.execute(task5);
        assertEQ(res1, "testWithZeroParam");
        assertEQ(res2, 30);
        assertEQ(res3, "testWithThreeParams res: 30");
        assertEQ(res4, "TEST res: 6");
        assertEQ(res5, 16.0);
        assertEQ(task1.isDone(), true);
        assertEQ(task2.isDone(), true);
        assertEQ(task3.isDone(), true);
        assertEQ(task4.isDone(), true);
        assertEQ(task5.isDone(), true);
    });
    commonTaskSuite.addTest("TaskPoolInTaskPoolTest", () =>
    {
        // execute the taskpool task in the taskpool task
        let res: int = 0;
        let innerFunc = (a: int): int => {
            let t = Date.now();
            res += a;
            while ((Date.now() - t) < 50) {
                continue;
            }
            return a + 10;
        };
        let outerFunc = (): string => {
            let innerTask = new taskpool.Task(innerFunc, 10);
            let innerRes = await taskpool.execute(innerTask);
            assertEQ(innerRes, 20);
            res += 20;
            return "";
        };
        let task = new taskpool.Task(outerFunc);
        await taskpool.execute(task);
        assertEQ(res, 30);
        assertEQ(task.isDone(), true);
    });
    commonTaskSuite.addTest("TasksExecutionTest", () =>
    {
        // execute a large number of taskpool tasks
        let num: int = 0;
        let count: int = 120;
        let taskArray: Array<taskpool.Task> = new Array<taskpool.Task>();
        for (let i: int = 0; i < count; ++i) {
            let task: taskpool.Task = new taskpool.Task(sum, 10, 20);
            taskArray.push(task);
        }
        let allExecuted: AtomicFlag = new AtomicFlag(false);
        for (let i: int = 0; i < count; ++i) {
            taskpool.execute(taskArray[i]).then((value: object): void => {
                num++;
                if (num == count) {
                    allExecuted.set(true);
                }
            })
        }
        while (allExecuted.get() != true) {
            delayWithYield(5);
        }
        assertEQ(taskArray[0].isDone(), true);
        assertEQ(taskArray[50].isDone(), true);
        assertEQ(taskArray[100].isDone(), true);
        assertEQ(num, 120);
    });
    commonTaskSuite.addTest("TaskDurationTest", () =>
    {
        let task1 = new taskpool.Task(longFunc, 200);
        assertEQ(task1.ioDuration, 0);
        assertEQ(task1.cpuDuration, 0);
        assertEQ(task1.totalDuration, 0);
        let res1 = await taskpool.execute(task1);
        assertEQ(res1, "success");
        assertNE(task1.cpuDuration, 0);
        assertNE(task1.totalDuration, 0);
        let task2 = new taskpool.Task(longFunc, 1000);
        assertEQ(task2.ioDuration, 0);
        assertEQ(task2.cpuDuration, 0);
        assertEQ(task2.totalDuration, 0);
        let res2 = await taskpool.execute(task2);
        assertEQ(res2, "success");
        assertNE(task2.cpuDuration, 0);
        assertNE(task2.totalDuration, 0);
    });
    commonTaskSuite.addTest("TaskExecuteDelayedTest", taskExecuteDelayedTest);
    commonTaskSuite.addTest("TaskExecutePeriodicallyTest", taskExecutePeriodicallyTest);
    commonTaskSuite.addTest("TaskWithDependencyExecutePeriodicallyTest", taskWithDependencyExecutePeriodicallyTest);
    commonTaskSuite.addTest("TaskOnReceiveDataTest", taskOnReceiveData);
    commonTaskSuite.addTest("TaskSendDataTest", taskSendData);
    commonTaskSuite.addTest("testOnReceiveDataRestParameter", testOnReceiveDataRestParameter);
    let res = commonTaskSuite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
