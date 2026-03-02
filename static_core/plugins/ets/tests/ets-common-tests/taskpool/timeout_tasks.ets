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

function returnAbc(): string {
    return (() => 'a')() + (() => 'b')() + (() => 'c')();
}

function longFunc(duration: int): string {
    let t = Date.now();
    while ((Date.now() - t) < duration) {
        continue;
    }
    return 'success';
}

async function taskExecuteWithinTimeout(): Promise<void> {
    let task1 = new taskpool.Task(returnAbc);
    try {
        let res1 = await taskpool.execute(task1, { timeout: 100 });
        arktest.assertEQ(res1, 'abc');
    } catch (e) {
        arktest.assertEQ('taskpool:: task has been timeout', e.message);
    }

    let task2 = new taskpool.Task(longFunc, 300);
    arktest.expectError(() => {
        await taskpool.execute(task2, { timeout: 50 });
    }, new Error('taskpool:: task has been timeout'));

    let longTask1 = new taskpool.LongTask(longFunc, 50);
    try {
        let longTaskRes1 = await taskpool.execute(longTask1, { timeout: 100 });
        arktest.assertEQ(longTaskRes1, 'success');
    } catch (e) {
        arktest.assertEQ('taskpool:: task has been timeout', e.message);
    }

    let longTask2 = new taskpool.LongTask(longFunc, 300);
    arktest.expectError(() => {
        await taskpool.execute(longTask2, { timeout: 50 });
    }, new Error('taskpool:: task has been timeout'));
}

async function taskWithTimeoutCannotExecuteAgain(): Promise<void> {
    let task = new taskpool.Task(longFunc, 200);
    arktest.expectError(() => {
        await taskpool.execute(task, { timeout: 20 });
    }, new Error('taskpool:: task has been timeout'));

    arktest.expectError(() => {
        taskpool.execute(task, { timeout: 50 });
    }, new Error('taskpool:: timeout task cannot be executed again'));

    arktest.expectError(() => {
        taskpool.execute(task);
    }, new Error('taskpool:: timeout task cannot be executed again'));

    arktest.expectError(() => {
        taskpool.executeDelayed(100, task);
    }, new Error('taskpool:: timeout task cannot be executed again'));

    arktest.expectError(() => {
        taskpool.executePeriodically(100, task);
    }, new Error('taskpool:: The concurrent task has been executed and cannot be executed periodically'));

    arktest.expectError(() => {
        let asyncRunner = new taskpool.AsyncRunner(2);
        asyncRunner.execute(task);
    }, new Error('taskpool:: asyncRunner cannot execute executedTask'));

    arktest.expectError(() => {
        let seqRunner = new taskpool.SequenceRunner();
        seqRunner.execute(task);
    }, new Error('taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask'));

    arktest.expectError(() => {
        let group = new taskpool.TaskGroup();
        group.addTask(task);
    }, new Error('taskpool:: taskGroup cannot add seqRunnerTask or executedTask'));
}

async function timeoutTaskCannotHaveDependency(): Promise<void> {
    let task1 = new taskpool.Task(longFunc, 50);
    let task2 = new taskpool.Task(returnAbc);
    task1.addDependency(task2);
    arktest.expectError(() => {
        taskpool.execute(task1, { timeout: 200 });
    }, new Error('taskpool:: timeout task with dependency cannot execute'));

    let task3 = new taskpool.Task(longFunc, 50);
    let task4 = new taskpool.Task(returnAbc);
    task4.addDependency(task1);
    arktest.expectError(() => {
        taskpool.execute(task4, { timeout: 200 });
    }, new Error('taskpool:: timeout task with dependency cannot execute'));
}

async function timeoutTaskSendData() {
    let taskStarted = false;
    let onStartCalled = false;
    let receiveDataCount = 0;

    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData(0);
        let start = Date.now();
        while ((Date.now() - start) < 600) {
            continue;
        }
        arktest.expectError(() => {
            taskpool.Task.sendData(1);
        }, new Error('taskpool:: task has been timeout'));
    });

    task.onStartExecution(() => {
        onStartCalled = true;
    });

    task.onReceiveData((data: int) => {
        receiveDataCount++;
        if (data === 0) {
            taskStarted = true;
        }
    });

    let p = taskpool.execute(task, { timeout: 300 });
    arktest.expectError(() => {
        await p;
    }, new Error('taskpool:: task has been timeout'));

    if (onStartCalled) {
        arktest.assertTrue(taskStarted);
        arktest.assertEQ(receiveDataCount, 1);
    } else {
        arktest.assertFalse(taskStarted);
        arktest.assertEQ(receiveDataCount, 0);
    }
}

async function timeoutBoundaryValues(): Promise<void> {
    let task1 = new taskpool.Task(longFunc, 50);
    let res1 = await taskpool.execute(task1, { timeout: 0 });
    arktest.assertEQ(res1, 'success');

    let task2 = new taskpool.Task(longFunc, 50);
    let res2 = await taskpool.execute(task2, { timeout: Infinity });
    arktest.assertEQ(res2, 'success');

    let task3 = new taskpool.Task(longFunc, 50);
    let res3 = await taskpool.execute(task3, { timeout: -1 });
    arktest.assertEQ(res3, 'success');
}

async function timeoutWithPriority(): Promise<void> {
    // timeout + HIGH priority
    let task1 = new taskpool.Task(longFunc, 50);
    try {
        let res1 = await taskpool.execute(task1, { timeout: 100, priority: taskpool.Priority.HIGH });
        arktest.assertEQ(res1, 'success');
    } catch (e) {
        arktest.assertEQ('taskpool:: task has been timeout', e.message);
    }

    // timeout + LOW priority
    let task2 = new taskpool.Task(longFunc, 50);
    try {
        let res2 = await taskpool.execute(task2, { timeout: 100, priority: taskpool.Priority.LOW });
        arktest.assertEQ(res2, 'success');
    } catch (e) {
        arktest.assertEQ('taskpool:: task has been timeout', e.message);
    }

    // timeout + MEDIUM priority
    let task3 = new taskpool.Task(longFunc, 50);
    try {
        let res3 = await taskpool.execute(task3, { timeout: 100, priority: taskpool.Priority.MEDIUM });
        arktest.assertEQ(res3, 'success');
    } catch (e) {
        arktest.assertEQ('taskpool:: task has been timeout', e.message);
    }
    
    // empty configs
    let task4 = new taskpool.Task(longFunc, 50);
    let res4 = await taskpool.execute(task4, {});
    arktest.assertEQ(res4, 'success');
}

async function taskExecutedByOtherApiCannotSetTimeout(): Promise<void> {
    let task1 = new taskpool.Task(returnAbc);
    await taskpool.execute(task1);
    arktest.expectError(() => {
        taskpool.execute(task1, { timeout: 50 });
    }, new Error('taskpool:: task has been executed'));

    let task2 = new taskpool.Task(returnAbc);
    taskpool.executePeriodically(100, task2);
    arktest.expectError(() => {
        taskpool.execute(task2, { timeout: 50 });
    }, new Error('taskpool:: the periodicTask cannot execute again'));
    taskpool.cancel(task2);

    let task3 = new taskpool.Task(returnAbc);
    let runner = new taskpool.SequenceRunner();
    await runner.execute(task3);
    arktest.expectError(() => {
        taskpool.execute(task3, { timeout: 50 });
    }, new Error('taskpool:: seqRunnerTask cannot execute outside'));

    let task4 = new taskpool.Task(returnAbc);
    let asyncRunner = new taskpool.AsyncRunner(2);
    await asyncRunner.execute(task4);
    arktest.expectError(() => {
        taskpool.execute(task4, { timeout: 50 });
    }, new Error('taskpool:: asyncRunnerTask cannot execute outside'));

    let task5 = new taskpool.Task(returnAbc);
    let group = new taskpool.TaskGroup();
    group.addTask(task5);
    arktest.expectError(() => {
        taskpool.execute(task5, { timeout: 50 });
    }, new Error('taskpool:: groupTask cannot execute outside'));

    let task6 = new taskpool.Task(returnAbc);
    await taskpool.executeDelayed(100, task6);
    arktest.expectError(() => {
        taskpool.execute(task6, { timeout: 50 });
    }, new Error('taskpool:: task has been executed'));
    taskpool.cancel(task6);
}

async function taskGroupExecuteWithinTimeout(): Promise<void> {
    let group1 = new taskpool.TaskGroup();
    let task1 = new taskpool.Task(longFunc, 50);
    let task2 = new taskpool.Task(returnAbc);
    group1.addTask(task1);
    group1.addTask(task2);
    try {
        let results1 = await taskpool.execute(group1, { timeout: 200 });
        arktest.assertEQ(results1[0], 'success');
        arktest.assertEQ(results1[1], 'abc');
    } catch (e) {
        arktest.assertEQ('taskpool:: task group has been timeout', e.message);
    }

    let group2 = new taskpool.TaskGroup();
    let task3 = new taskpool.Task(longFunc, 500);
    let task4 = new taskpool.Task(returnAbc);
    group2.addTask(task3);
    group2.addTask(task4);
    arktest.expectError(() => {
        await taskpool.execute(group2, { timeout: 50 });
    }, new Error('taskpool:: task group has been timeout'));

    let group3 = new taskpool.TaskGroup();
    let failFunc = (): string => {
        throw new Error('task failed');
    };
    let task5 = new taskpool.Task(failFunc);
    let task6 = new taskpool.Task(longFunc, 50);
    group3.addTask(task5);
    group3.addTask(task6);
    arktest.expectError(() => {
        await taskpool.execute(group3, { timeout: 200 });
    }, new Error('task failed'));

    let group4 = new taskpool.TaskGroup();
    let task7 = new taskpool.Task(longFunc, 200);
    let failFunc2 = async (): Promise<string> => {
        await new Promise<void>((resolve) => {
            setTimeout(() => {
                resolve(undefined);
            }, 200);
        });
        throw new Error('task failed');
    };
    let task8 = new taskpool.Task(failFunc2);
    group4.addTask(task7);
    group4.addTask(task8);
    arktest.expectError(() => {
        await taskpool.execute(group4, { timeout: 50 });
    }, new Error('taskpool:: task group has been timeout'));
}

async function taskGroupTimeoutBoundaryValues(): Promise<void> {
    let group1 = new taskpool.TaskGroup();
    let task1 = new taskpool.Task(longFunc, 50);
    group1.addTask(task1);
    let results1 = await taskpool.execute(group1, { timeout: 0 });
    arktest.assertEQ(results1[0], 'success');

    let group2 = new taskpool.TaskGroup();
    let task2 = new taskpool.Task(longFunc, 50);
    group2.addTask(task2);
    let results2 = await taskpool.execute(group2, { timeout: Infinity });
    arktest.assertEQ(results2[0], 'success');

    let group3 = new taskpool.TaskGroup();
    let task3 = new taskpool.Task(longFunc, 50);
    group3.addTask(task3);
    let results3 = await taskpool.execute(group3, { timeout: -1 });
    arktest.assertEQ(results3[0], 'success');
}

async function taskGroupWithPriority(): Promise<void> {
    // timeout + HIGH priority
    let group1 = new taskpool.TaskGroup();
    try {
        let task1 = new taskpool.Task(longFunc, 50);
        group1.addTask(task1);
        let results1 = await taskpool.execute(group1, { timeout: 100, priority: taskpool.Priority.HIGH });
        arktest.assertEQ(results1[0], 'success');
    } catch (e) {
        arktest.assertEQ('taskpool:: task group has been timeout', e.message);
    }

    // timeout + LOW priority
    let group2 = new taskpool.TaskGroup();
    try {
        let task2 = new taskpool.Task(longFunc, 50);
        group2.addTask(task2);
        let results2 = await taskpool.execute(group2, { timeout: 100, priority: taskpool.Priority.LOW });
        arktest.assertEQ(results2[0], 'success');
    } catch (e) {
        arktest.assertEQ('taskpool:: task group has been timeout', e.message);
    }

    // timeout + MEDIUM priority
    let group3 = new taskpool.TaskGroup();
    try {
        let task3 = new taskpool.Task(longFunc, 50);
        group3.addTask(task3);
        let results3 = await taskpool.execute(group3, { timeout: 100, priority: taskpool.Priority.MEDIUM });
        arktest.assertEQ(results3[0], 'success');
    } catch (e) {
        arktest.assertEQ('taskpool:: task group has been timeout', e.message);
    }

    // empty configs
    let group4 = new taskpool.TaskGroup();
    let task4 = new taskpool.Task(longFunc, 50);
    group4.addTask(task4);
    let results4 = await taskpool.execute(group4, {});
    arktest.assertEQ(results4[0], 'success');
}

async function timeoutTaskGroupExecutedCannotSetTimeout(): Promise<void> {
    let group = new taskpool.TaskGroup();
    let task = new taskpool.Task(returnAbc);
    group.addTask(task);
    await taskpool.execute(group);
    arktest.expectError(() => {
        taskpool.execute(group, { timeout: 50 });
    }, new Error('taskpool:: the taskGroup cannot be executed again, taskGroup cannot be set timeout'));
}

async function taskGroupWithTimeoutCannotExecuteWithoutTimeout(): Promise<void> {
    let group = new taskpool.TaskGroup();
    let task1 = new taskpool.Task(longFunc, 500);
    let task2 = new taskpool.Task(returnAbc);
    group.addTask(task1);
    group.addTask(task2);

    arktest.expectError(() => {
        await taskpool.execute(group, { timeout: 50 });
    }, new Error('taskpool:: task group has been timeout'));

    arktest.expectError(() => {
        taskpool.execute(group);
    }, new Error('taskpool:: the taskGroup cannot be executed again, taskGroup has already set timeout'));
}

async function timeoutNotTriggerSuccessAndFailedCallbacks(): Promise<void> {
    let taskOnExecutionSucceededStr = '';
    let taskOnExecutionFailedStr = '';
    let taskOnExecutionSucceededCallback = () => {
        taskOnExecutionSucceededStr = 'success callback is done';
    };
    let taskOnExecutionFailedCallback = (e: Error) => {
        taskOnExecutionFailedStr = e.message + ', fail callback is done';
    };

    let task = new taskpool.Task(longFunc, 300);
    task.onExecutionSucceeded(taskOnExecutionSucceededCallback);
    task.onExecutionFailed(taskOnExecutionFailedCallback);

    arktest.expectError(() => {
        await taskpool.execute(task, { timeout: 100 });
    }, new Error('taskpool:: task has been timeout'));

    arktest.assertEQ(taskOnExecutionSucceededStr, '');
    arktest.assertEQ(taskOnExecutionFailedStr, '');

    let groupOnExecutionSucceededStr1 = '';
    let groupOnExecutionFailedStr1 = '';
    let groupOnExecutionSucceededStr2 = '';
    let groupOnExecutionFailedStr2 = '';

    let task1 = new taskpool.Task(longFunc, 300);
    task1.onExecutionSucceeded(() => {
        groupOnExecutionSucceededStr1 = 'task1 success callback is done';
    });
    task1.onExecutionFailed((e: Error) => {
        groupOnExecutionFailedStr1 = e.message + ', task1 fail callback is done';
    });

    let task2 = new taskpool.Task(returnAbc);
    task2.onExecutionSucceeded(() => {
        groupOnExecutionSucceededStr2 = 'task2 success callback is done';
    });
    task2.onExecutionFailed((e: Error) => {
        groupOnExecutionFailedStr2 = e.message + ', task2 fail callback is done';
    });

    let group = new taskpool.TaskGroup();
    group.addTask(task1);
    group.addTask(task2);

    arktest.expectError(() => {
        await taskpool.execute(group, { timeout: 100 });
    }, new Error('taskpool:: task group has been timeout'));

    arktest.assertEQ(groupOnExecutionSucceededStr1, '');
    arktest.assertEQ(groupOnExecutionFailedStr1, '');
    arktest.assertEQ(groupOnExecutionSucceededStr2, '');
    arktest.assertEQ(groupOnExecutionFailedStr2, '');
}

function main(): int {
    let timeoutTaskSuite = new arktest.ArkTestsuite('taskpool.Timeout');
    let timeoutTaskSuiteDisabled = new arktest.ArkTestsuite('taskpool.CommonTask.DISABLED');
    timeoutTaskSuite.addAsyncTest('TaskExecuteWithinTimeout', taskExecuteWithinTimeout);
    timeoutTaskSuite.addAsyncTest('TaskWithTimeoutCannotExecuteAgain', taskWithTimeoutCannotExecuteAgain);
    timeoutTaskSuite.addAsyncTest('TimeoutTaskCannotHaveDependency', timeoutTaskCannotHaveDependency);
    timeoutTaskSuiteDisabled.addAsyncTest('TimeoutTaskSendData', timeoutTaskSendData);
    timeoutTaskSuite.addAsyncTest('TimeoutBoundaryValues', timeoutBoundaryValues);
    timeoutTaskSuite.addAsyncTest('TimeoutWithPriority', timeoutWithPriority);
    timeoutTaskSuite.addAsyncTest('TaskExecutedByOtherApiCannotSetTimeout', taskExecutedByOtherApiCannotSetTimeout);
    timeoutTaskSuiteDisabled.addAsyncTest('TaskGroupExecuteWithinTimeout', taskGroupExecuteWithinTimeout);
    timeoutTaskSuite.addAsyncTest('TaskGroupTimeoutBoundaryValues', taskGroupTimeoutBoundaryValues);
    timeoutTaskSuite.addAsyncTest('TaskGroupWithPriority', taskGroupWithPriority);
    timeoutTaskSuiteDisabled.addAsyncTest('TimeoutTaskGroupExecutedCannotSetTimeout', timeoutTaskGroupExecutedCannotSetTimeout);
    timeoutTaskSuiteDisabled.addAsyncTest('TaskGroupWithTimeoutCannotExecuteWithoutTimeout', taskGroupWithTimeoutCannotExecuteWithoutTimeout);
    timeoutTaskSuiteDisabled.addAsyncTest('TimeoutNotTriggerSuccessAndFailedCallbacks', timeoutNotTriggerSuccessAndFailedCallbacks);

    let res = timeoutTaskSuite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
