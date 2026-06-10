/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class SendDataSharedPayloadMarker {
    public x: int = 1;
}

async function delay(ms: int): Promise<void> {
    await new Promise<void>((resolve) => {
        setTimeout(() => {
            resolve(undefined);
        }, ms);
    });
}

function defaultWorkerPriorityIsHigh(): void {
    if (CoroutineExtras.isTaskpoolUsingLaunchMode()) {
        return;
    }

    let info = taskpool.getTaskPoolInfo();
    arktest.assertNE(info.threadInfos.length, 0);
    info.threadInfos.forEach((threadInfo: taskpool.ThreadInfo) => {
        arktest.assertEQ(threadInfo.priority, taskpool.Priority.HIGH);
    });
}

function scalingLongRunningTask(duration: int): string {
    let start = Date.now();
    while ((Date.now() - start) < duration) {
        continue;
    }
    return 'success';
}

async function waitUntilWorkersAtMost(expected: int, timeoutMs: int): Promise<void> {
    let start = Date.now();
    while ((Date.now() - start) < timeoutMs) {
        let workersNum = CoroutineExtras.getTaskPoolWorkersNum();
        arktest.assertTrue(workersNum <= expected);
        if (workersNum == expected) {
            return;
        }
        await delay(5);
    }
    arktest.assertEQ(CoroutineExtras.getTaskPoolWorkersNum(), expected);
}

async function executeScalingBatch(count: int): Promise<void> {
    let promises: Promise<Any>[] = [];
    for (let i: int = 0; i < count; i++) {
        let task = new taskpool.Task(scalingLongRunningTask, 20);
        promises.push(taskpool.execute(task));
    }

    await waitUntilWorkersAtMost(count, 60);
    await Promise.all(promises);
}

async function taskpoolScalingShouldNotOverExpand(): Promise<void> {
    if (CoroutineExtras.isTaskpoolUsingLaunchMode()) {
        return;
    }

    const limit: int = max(4, CoroutineExtras.getTaskPoolWorkersLimit());
    const currentWorkersNum: int = CoroutineExtras.getTaskPoolWorkersNum();
    if (currentWorkersNum >= limit) {
        return;
    }
    await executeScalingBatch(currentWorkersNum + 1);
}

async function cancelInSuccessCallback() {
    let syncSuccessCallbackCalled: boolean = false;
    let asyncSuccessCallbackCalled: boolean = false;
    let timeoutPromise = async (duration: int): Promise<void> => {
        await new Promise<void>((resolve) => {
            setTimeout(() => {
                resolve(undefined);
            }, duration);
        });
    }

    let task1 = new taskpool.Task(returnAbc);
    let task2 = new taskpool.Task(timeoutPromise, 100);
    task1.onExecutionSucceeded(() => {
        syncSuccessCallbackCalled = true;
        arktest.expectError(
            () => { taskpool.cancel(task1); },
            new Error('taskpool:: task has been executed')
        );
    });
    task2.onExecutionSucceeded(() => {
        asyncSuccessCallbackCalled = true;
        arktest.expectError(
            () => { taskpool.cancel(task2); },
            new Error('taskpool:: task has been executed')
        );
    });

    await taskpool.execute(task1);
    arktest.assertEQ(syncSuccessCallbackCalled, true);
    await taskpool.execute(task2);
    arktest.assertEQ(asyncSuccessCallbackCalled, true);
}

async function getcurrentTask() {
    let fun1 = async () => {
        let delay = async () => {
            await new Promise<void>((res) => { setTimeout(() => res(undefined), 10); });
        }
        await delay()
        // if we use currentExecutingTasks, t2 will hold currentExecutingTasks because t2's timeout is 20ms(t1 is 10ms).
        // so the sendData() -> getcurrentTask() -> currentExecutingTasks will get task t2;
        taskpool.Task.sendData(1)
    }
    let t1: taskpool.Task = new taskpool.Task(fun1)
    t1.onReceiveData((res: int) => {
        arktest.assertEQ(res, 1);
    })
    let p1 = taskpool.execute(t1);

    let fun2 = async () => {
        let delay = async () => {
            await new Promise<void>((res) => { setTimeout(() => res(undefined), 20); });
        }
        await delay()
    }

    // two t2 tasks can ensure that there is at least one t2'sworker assigned to t1' worker
    for (let i = 0; i < 2; ++i) {
        let t2: taskpool.Task = new taskpool.Task(fun2)
        taskpool.execute(t2);
    }

    try {
        await p1;
    } catch(e) {
        arktest.assertTrue(false, e.message)
    }
}

async function testTaskInfoDuration() {
    let event1 = new PromiseWithResolvers<void>();
    let event2 = new PromiseWithResolvers<void>();
    let task = new taskpool.Task(async () => {
        let startTime = Date.now();
        while (Date.now() - startTime < 100) {}
        event2.resolve(undefined);
        await event1.promise;
    });
    taskpool.execute(task);
    await event2.promise;
    let info = taskpool.getTaskPoolInfo();
    arktest.assertNE(info.taskInfos.length, 0);
    let found = false;
    for (let i = 0; i < info.taskInfos.length; i++) {
        if (info.taskInfos[i].taskId === task.taskId) {
            found = true;
            arktest.assertLT(0, info.taskInfos[i].duration as int);
            break;
        }
    }
    event1.resolve(undefined);
    arktest.assertEQ(found, true);
}

async function onExecutionFailedtestAsyncfun() {
    let catchSuccess : int = 0;
    let event = new PromiseWithResolvers<void>();
    let funca = async() : Promise<Any> => {
        throw new Error('Error');
    }
    let task = new taskpool.Task(funca);
    task.onExecutionFailed((e : Error) => {
       catchSuccess++;
       event.resolve(undefined);
    });
    taskpool.execute(task);
    await event.promise;
    arktest.assertEQ(catchSuccess, 1);
}

async function onExecutionFailedtestmultAsyncfun(){
    let catchSuccess : int = 0;
    let event1 = new PromiseWithResolvers<void>();
    let event2 = new PromiseWithResolvers<void>();
    let event3 = new PromiseWithResolvers<void>();
    let funca = async() : Promise<Any> => {
        throw new Error('Error');
    }
    let task1 = new taskpool.Task(funca);
    let task2 = new taskpool.Task(funca);
    let task3 = new taskpool.Task(funca);
    task3.addDependency(task2);
    task2.addDependency(task1);
    task1.onExecutionFailed((e : Error) => {
       catchSuccess++;
       event1.resolve(undefined);
    });
    task2.onExecutionFailed((e : Error) => {
       catchSuccess++;
       event2.resolve(undefined);
    });
    task3.onExecutionFailed((e : Error) => {
       catchSuccess++;
       event3.resolve(undefined);
    });
    taskpool.execute(task1);
    taskpool.execute(task2);
    taskpool.execute(task3);
    await event1.promise;
    await event2.promise;
    await event3.promise;
    arktest.assertEQ(catchSuccess,3);
}

async function onFailCallbackExceptionTest() {
    let callbackCalled: boolean = false;
    let errorCaught: boolean = false;
    let errorFunc = async() : Promise<void> => {
        throw new Error('Original Error');
    }
    let task = new taskpool.Task(errorFunc);
    task.onExecutionFailed((e: Error) => {
        callbackCalled = true;
        throw new Error('Callback Error');
    });

    try {
        await taskpool.execute(task);
        arktest.assertTrue(false, 'Original Error exception expected on callback');
    } catch (e) {
        if (e instanceof AssertionError) throw e
        arktest.assertEQ('Original Error', e.message);
    }

    arktest.assertEQ(callbackCalled, true);
}

async function multipleFailCallbackExceptionTest() {
    let callbacksCalled: int = 0;
    let errorsCaught: int = 0;
    let errorFunc = async(): Promise<void> => {
        throw new Error('Error');
    }

    let task1 = new taskpool.Task(errorFunc);
    let task2 = new taskpool.Task(errorFunc);

    task1.onExecutionFailed((e: Error) => {
        callbacksCalled++;
        throw new Error('Callback Error 1');
    });

    task2.onExecutionFailed((e: Error) => {
        callbacksCalled++;
        throw new Error('Callback Error 2');
    });

    try {
        await taskpool.execute(task1);
        arktest.assertTrue(false, 'Error exception expected on callback');
    } catch (e) {
        if (e instanceof AssertionError) throw e
        arktest.assertEQ('Error', e.message);
    }

    try {
        await taskpool.execute(task2);
        arktest.assertTrue(false, 'Error exception expected on callback');
    } catch (e) {
        if (e instanceof AssertionError) throw e
        arktest.assertEQ('Error', e.message);
    }

    arktest.assertEQ(callbacksCalled, 2);
}

function dependencyStressAsyncSleep(time: number): Promise<void> {
    return new Promise<void>((resolve) => {
        setTimeout(() => {
            resolve(undefined);
        }, time.toInt());
    });
}

async function workerTimerBackedTaskCompletes(): Promise<void> {
    let timerTask = new taskpool.Task(async (): Promise<int> => {
        await dependencyStressAsyncSleep(1.0);
        return 7;
    });
    let result = await taskpool.execute(timerTask);
    arktest.assertEQ(result, 7);
}

async function pendingDependencyShouldNotBlockReadyTask(): Promise<void> {
    const rounds: int = 10;
    for (let round: int = 0; round < rounds; round++) {
        let blocker: taskpool.Task = new taskpool.Task(async () => {
            await dependencyStressAsyncSleep(200.0);
            return 0 as int;
        });
        let blockedTasks: taskpool.Task[] = [];
        let blockedPromises: Promise<Any>[] = [];

        for (let i: int = 0; i < 60; i++) {
            let blockedTask: taskpool.Task = new taskpool.Task((): int => i);
            blockedTask.addDependency(blocker);
            blockedTasks.push(blockedTask);
            blockedPromises.push(
                taskpool.execute(blockedTask, taskpool.Priority.HIGH).then((res: Any) => res).catch((err: Error) => err)
            );
        }

        await dependencyStressAsyncSleep(20.0);

        let readyTask: taskpool.Task = new taskpool.Task((): int => 1);
        let readyTaskSettled = await Promise.race<boolean>([
            taskpool.execute(readyTask, taskpool.Priority.HIGH).then((_res: Any) => true).catch((_err: Error) => false),
            dependencyStressAsyncSleep(300.0).then(() => false)
        ]);

        let blockerExecuted = await taskpool.execute(blocker, taskpool.Priority.HIGH)
            .then((_res: Any) => true)
            .catch((_err: Error) => false);
        arktest.assertEQ(blockerExecuted, true);
        await Promise.allSettled(blockedPromises);
        blockedTasks.forEach((task: taskpool.Task) => {
            try {
                taskpool.cancel(task);
            } catch (_e) {
            }
        });

        arktest.assertEQ(readyTaskSettled, true, 'ready task must not be starved by blocked dependency tasks');
    }
}

async function onEnqueuedNestedExecuteTest() {
    let innerResult: string = '';
    let callbackCalled: boolean = false;
    let failed: boolean = false;
    let event = new PromiseWithResolvers<void>();
    let outerTask = new taskpool.Task(returnAbc);

    outerTask.onEnqueued(() => {
        callbackCalled = true;
        let innerTask = new taskpool.Task(returnAbc);
        taskpool.execute(innerTask).then((res: Any) => {
            innerResult = res as string;
            event.resolve(undefined);
        }).catch((e: Error) => {
            failed = true;
            event.resolve(undefined);
        });
    });

    let outerResult = await taskpool.execute(outerTask);
    arktest.assertEQ(outerResult, 'abc');
    arktest.assertEQ(callbackCalled, true);
    await event.promise;
    arktest.assertEQ(failed, false);
    arktest.assertEQ(innerResult, 'abc');
}

async function onEnqueuedNestedRunnerExecuteTest() {
    let callbackCalled: boolean = false;
    let failed: boolean = false;
    let completedCount: int = 0;
    let sequenceResult: string = '';
    let asyncResult: string = '';
    let event = new PromiseWithResolvers<void>();
    let outerTask = new taskpool.Task(returnAbc);

    outerTask.onEnqueued(() => {
        callbackCalled = true;
        let sequenceRunner = new taskpool.SequenceRunner();
        let asyncRunner = new taskpool.AsyncRunner(1);
        let sequenceTask = new taskpool.Task(returnAbc);
        let asyncTask = new taskpool.Task(returnAbc);
        let markDone = () => {
            completedCount++;
            if (completedCount == 2) {
                event.resolve(undefined);
            }
        }

        sequenceRunner.execute(sequenceTask).then((res: Any) => {
            sequenceResult = res as string;
            markDone();
        }).catch((e: Error) => {
            failed = true;
            markDone();
        });

        asyncRunner.execute(asyncTask).then((res: Any) => {
            asyncResult = res as string;
            markDone();
        }).catch((e: Error) => {
            failed = true;
            markDone();
        });
    });

    let outerResult = await taskpool.execute(outerTask);
    arktest.assertEQ(outerResult, 'abc');
    arktest.assertEQ(callbackCalled, true);
    await event.promise;
    arktest.assertEQ(failed, false);
    arktest.assertEQ(sequenceResult, 'abc');
    arktest.assertEQ(asyncResult, 'abc');
}

async function asyncRunnerExecuteInOnEnqueuedNoDeadlock(): Promise<void> {
    let completedCount: int = 0;
    let event = new PromiseWithResolvers<void>();
    let emptyFunc = () => {};

    let asyncRunner: taskpool.AsyncRunner = new taskpool.AsyncRunner('async', 3, 3);
    let mainTask: taskpool.Task = new taskpool.Task(emptyFunc);

    mainTask.onEnqueued(() => {
        let innerTask: taskpool.Task = new taskpool.Task(emptyFunc);
        asyncRunner.execute(innerTask).then(() => {
            completedCount++;
            if (completedCount == 2) {
                event.resolve(undefined);
            }
        });
    });

    asyncRunner.execute(mainTask).then(() => {
        completedCount++;
        if (completedCount == 2) {
            event.resolve(undefined);
        }
    });

    await event.promise;
    arktest.assertEQ(completedCount, 2);
}

async function seqRunnerExecuteInOnEnqueuedNoDeadlock(): Promise<void> {
    let completedCount: int = 0;
    let event = new PromiseWithResolvers<void>();
    let emptyFunc = () => {};

    let seqRunner: taskpool.SequenceRunner = new taskpool.SequenceRunner('seq');
    let mainTask: taskpool.Task = new taskpool.Task(emptyFunc);

    mainTask.onEnqueued(() => {
        let innerTask: taskpool.Task = new taskpool.Task(emptyFunc);
        seqRunner.execute(innerTask).then(() => {
            completedCount++;
            if (completedCount == 2) {
                event.resolve(undefined);
            }
        });
    });

    seqRunner.execute(mainTask).then(() => {
        completedCount++;
        if (completedCount == 2) {
            event.resolve(undefined);
        }
    });

    await event.promise;
    arktest.assertEQ(completedCount, 2);
}

async function sendDataWorkerLocalPayloadReadableAfterScope(): Promise<void> {
    let receivedFlag: int = -1;
    let callbackInvoked: boolean = false;
    let task = new taskpool.Task((): void => {
        ((): void => {
            let local: Record<string, Any> = { 'flag': 42 as Any } as Record<string, Any>;
            taskpool.Task.sendData(local);
        })();
    });

    task.onReceiveData((arg: Record<string, Any>): void => {
        receivedFlag = arg['flag'] as int;
        callbackInvoked = true;
    });

    await taskpool.execute(task);
    arktest.assertEQ(callbackInvoked, true);
    arktest.assertEQ(receivedFlag, 42);
}

async function sendDataRecordSharesReferenceWithHost(): Promise<void> {
    let shared: Record<string, Any> = { 'x': 1 as Any } as Record<string, Any>;
    let sameRef: boolean = false;
    let task = new taskpool.Task((arg: Record<string, Any>): void => {
        taskpool.Task.sendData(arg);
    }, shared);

    task.onReceiveData((arg: Record<string, Any>): void => {
        sameRef = arg === shared;
        arg['x'] = 99 as Any;
    });

    await taskpool.execute(task);
    arktest.assertEQ(sameRef, true);
    arktest.assertEQ(shared['x'] as int, 99);
}

async function sendDataArrayBufferSharesReferenceWithHost(): Promise<void> {
    let buf = new ArrayBuffer(4);
    let u = new Uint8Array(buf);
    u[0] = 7;
    let sameRef: boolean = false;
    let task = new taskpool.Task((ab: ArrayBuffer): void => {
        taskpool.Task.sendData(ab);
    }, buf);

    task.onReceiveData((ab: ArrayBuffer): void => {
        sameRef = ab === buf;
        new Uint8Array(ab)[0] = 99;
    });

    await taskpool.execute(task);
    arktest.assertEQ(sameRef, true);
    arktest.assertEQ(u[0].toInt(), 99);
}

async function sendDataSequentialCallbacksPreserveSendOrder(): Promise<void> {
    let seq: int[] = [];
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData(10);
        taskpool.Task.sendData(20);
        taskpool.Task.sendData(30);
    });

    task.onReceiveData((v: int): void => {
        seq.push(v);
    });

    await taskpool.execute(task);
    arktest.assertEQ(seq.length, 3);
    arktest.assertEQ(seq[0], 10);
    arktest.assertEQ(seq[1], 20);
    arktest.assertEQ(seq[2], 30);
}

async function sendDataPromisePayloadPassThrough(): Promise<void> {
    let callbackDone = new PromiseWithResolvers<void>();
    let received: int = -1;
    let task = new taskpool.Task((): void => {
        let deferred = new PromiseWithResolvers<int>();
        taskpool.Task.sendData(deferred.promise);
        deferred.resolve(2);
    });

    task.onReceiveData((promise: Promise<int>): void => {
        promise.then((value: int) => {
            received = value;
            callbackDone.resolve(undefined);
        });
    });

    await taskpool.execute(task);
    await callbackDone.promise;
    arktest.assertEQ(received, 2);
}

async function sendDataFunctionPayloadPassThrough(): Promise<void> {
    let received: string = '';
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData((prefix: string): string => prefix + 'hi');
    });

    task.onReceiveData((fn: (prefix: string) => string): void => {
        received = fn('say ');
    });

    await taskpool.execute(task);
    arktest.assertEQ(received, 'say hi');
}

async function sendDataCustomClassInstanceSharesReferenceWithHost(): Promise<void> {
    let marker = new SendDataSharedPayloadMarker();
    let sameRef: boolean = false;
    let task = new taskpool.Task((arg: SendDataSharedPayloadMarker): void => {
        taskpool.Task.sendData(arg);
    }, marker);

    task.onReceiveData((arg: SendDataSharedPayloadMarker): void => {
        sameRef = arg === marker;
        arg.x = 7;
    });

    await taskpool.execute(task);
    arktest.assertEQ(sameRef, true);
    arktest.assertEQ(marker.x, 7);
}

async function sendDataScalarMultiArgsUnchanged(): Promise<void> {
    let gotA: int = 0;
    let gotB: string = '';
    let gotC: boolean = false;
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData(42, 'hi', true);
    });

    task.onReceiveData((a: int, b: string, c: boolean): void => {
        gotA = a;
        gotB = b;
        gotC = c;
    });

    await taskpool.execute(task);
    arktest.assertEQ(gotA, 42);
    arktest.assertEQ(gotB, 'hi');
    arktest.assertEQ(gotC, true);
}

async function sendDataEmptyInvocationStillDispatchesCallback(): Promise<void> {
    let hits: int = 0;
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData();
    });

    task.onReceiveData((): void => {
        hits++;
    });

    await taskpool.execute(task);
    arktest.assertEQ(hits, 1);
}

async function sendDataPromisePayloadExecuteSucceedsWithNoopCallback(): Promise<void> {
    let task = new taskpool.Task((): void => {
        let deferred = new PromiseWithResolvers<bigint>();
        taskpool.Task.sendData(deferred.promise);
        deferred.resolve(2n);
    });

    task.onReceiveData((p: Promise<bigint>): void => {});

    await taskpool.execute(task);
}

async function sendDataFunctionPayloadExecuteSucceedsWithNoopCallback(): Promise<void> {
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData((x: int): int => x + 1);
    });

    task.onReceiveData((fn: (x: int) => int): void => {});

    await taskpool.execute(task);
}

async function sendDataNullPayloadPassThrough(): Promise<void> {
    let gotNull: boolean = false;
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData(null);
    });

    task.onReceiveData((arg: Any): void => {
        gotNull = (arg === null);
    });

    await taskpool.execute(task);
    arktest.assertEQ(gotNull, true);
}

async function sendDataArraySharesReferenceWithHost(): Promise<void> {
    let arr: int[] = [1, 2, 3];
    let sameRef: boolean = false;
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData(arr);
    });

    task.onReceiveData((a: int[]): void => {
        sameRef = a === arr;
        a[0] = 99;
    });

    await taskpool.execute(task);
    arktest.assertEQ(sameRef, true);
    arktest.assertEQ(arr[0], 99);
}

async function sendDataTypedArraySharesReferenceWithHost(): Promise<void> {
    let buf = new ArrayBuffer(4);
    let u8 = new Uint8Array(buf);
    u8[0] = 7;
    let sameRef: boolean = false;
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData(u8);
    });

    task.onReceiveData((ta: Uint8Array): void => {
        sameRef = ta === u8;
        ta[0] = 99;
    });

    await taskpool.execute(task);
    arktest.assertEQ(sameRef, true);
    arktest.assertEQ(u8[0].toInt(), 99);
}

async function sendDataDataViewSharesReferenceWithHost(): Promise<void> {
    let buf = new ArrayBuffer(8);
    let dv = new DataView(buf);
    dv.setInt32(0, 7);
    let sameRef: boolean = false;
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData(dv);
    });

    task.onReceiveData((view: DataView): void => {
        sameRef = view === dv;
        view.setInt32(0, 99);
    });

    await taskpool.execute(task);
    arktest.assertEQ(sameRef, true);
    arktest.assertEQ(dv.getInt32(0), 99);
}

async function sendDataBigIntPayloadPassThrough(): Promise<void> {
    let expected: bigint = 12345678901234567890n;
    let received: bigint = 0n;
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData(expected);
    });

    task.onReceiveData((v: bigint): void => {
        received = v;
    });

    await taskpool.execute(task);
    arktest.assertEQ(received, expected);
}

async function sendDataExecuteSettlesOnlyAfterReceiveCallbacks(): Promise<void> {
    let callbacksDone: int = 0;
    let executeSettledEarly: boolean = false;
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData(10);
        taskpool.Task.sendData(20);
    });

    task.onReceiveData((): void => {
        callbacksDone++;
    });

    let executePromise = taskpool.execute(task);
    executePromise.then(() => {
        if (callbacksDone < 2) {
            executeSettledEarly = true;
        }
    });
    await executePromise;
    arktest.assertEQ(callbacksDone, 2);
    arktest.assertEQ(executeSettledEarly, false);
}

async function sequenceRunnerSendDataDeliveredOnHost(): Promise<void> {
    let runner = new taskpool.SequenceRunner();
    let received: int = 0;
    let task = new taskpool.Task((): void => {
        taskpool.Task.sendData(42);
    });

    task.onReceiveData((v: int): void => {
        received = v;
    });

    await runner.execute(task);
    arktest.assertEQ(received, 42);
}

function identityInt(value: int): int {
    return value;
}

// The async stack integration touches task submission paths. Keep this as a
// regression test to ensure existing taskpool execution semantics are unchanged.
async function asyncStackSubmissionKeepsTaskpoolSemantics(): Promise<void> {
    let executeTask = new taskpool.Task(identityInt, 1);
    let executeResult = await taskpool.execute(executeTask);
    arktest.assertEQ(executeResult, 1);

    let timeoutTask = new taskpool.Task(identityInt, 2);
    let timeoutResult = await taskpool.execute(timeoutTask, { timeout: 50 });
    arktest.assertEQ(timeoutResult, 2);

    let delayedTask = new taskpool.Task(identityInt, 3);
    let delayedResult = await taskpool.executeDelayed(10, delayedTask);
    arktest.assertEQ(delayedResult, 3);

    let group = new taskpool.TaskGroup();
    group.addTask(new taskpool.Task(identityInt, 4));
    group.addTask(new taskpool.Task(identityInt, 5));
    let groupResult = await taskpool.execute(group);
    arktest.assertEQ(groupResult[0], 4);
    arktest.assertEQ(groupResult[1], 5);

    let sequenceRunner = new taskpool.SequenceRunner();
    let sequenceTask = new taskpool.Task(identityInt, 6);
    let sequenceResult = await sequenceRunner.execute(sequenceTask);
    arktest.assertEQ(sequenceResult, 6);

    let asyncRunner = new taskpool.AsyncRunner(1, 2);
    let asyncRunnerBlocker = new taskpool.Task(async (): Promise<int> => {
        await dependencyStressAsyncSleep(20.0);
        return 7;
    });
    let asyncRunnerQueued = new taskpool.Task(identityInt, 8);
    let asyncRunnerBlockerPromise = asyncRunner.execute(asyncRunnerBlocker);
    let asyncRunnerQueuedPromise = asyncRunner.execute(asyncRunnerQueued);
    arktest.assertEQ(await asyncRunnerBlockerPromise, 7);
    arktest.assertEQ(await asyncRunnerQueuedPromise, 8);

    let periodicEvent = new PromiseWithResolvers<void>();
    let periodicCount: int = 0;
    let periodicTask = new taskpool.Task((): int => {
        periodicCount++;
        periodicEvent.resolve(undefined);
        return periodicCount;
    });
    taskpool.executePeriodically(10, periodicTask);
    await periodicEvent.promise;
    taskpool.cancel(periodicTask);
    arktest.assertLE(1, periodicCount);
}

async function cancelPeriodicTaskStopsExecution(): Promise<void> {
    let cancelPeriodicCount = new AtomicInt(0);
    let firstExecution = new PromiseWithResolvers<void>();
    let countAfterCancel: int = 0;
    let canceled = new AtomicBoolean(false);
    let task = new taskpool.Task(() => {
        cancelPeriodicCount.fetchAdd(1);
        if (cancelPeriodicCount.load() == 1) {
            firstExecution.resolve(undefined);
        }
    });

    task.onExecutionSucceeded(() => {
        if (canceled.load()) {
            // Allow at most 1 more execution after cancel.
            arktest.assertLE(cancelPeriodicCount.load(), countAfterCancel + 1);
        }
    });
    taskpool.executePeriodically(0, task, taskpool.Priority.MEDIUM);

    await firstExecution.promise;
    taskpool.cancel(task);
    countAfterCancel = cancelPeriodicCount.load();
    canceled.store(true);
    arktest.assertLE(cancelPeriodicCount.load(), countAfterCancel + 1);
}

async function verifyCancelPeriodicTaskAfterExecution(cancelByTaskId: boolean): Promise<void> {
    let executions = new AtomicInt(0);
    let firstExecution = new PromiseWithResolvers<void>();
    let countAfterCancel: int = 0;
    let canceled = new AtomicBoolean(false);
    let task = new taskpool.LongTask(() => {
        executions.fetchAdd(1);
        if (executions.load() == 1) {
            firstExecution.resolve(undefined);
        }
    });

    task.onExecutionSucceeded(() => {
        if (canceled.load()) {
            // Allow at most 1 more execution after cancel.
            arktest.assertLE(executions.load(), countAfterCancel + 1);
        }
    });

    taskpool.executePeriodically(100, task);

    await firstExecution.promise;
    if (cancelByTaskId) {
        taskpool.cancel(task.taskId);
    } else {
        taskpool.cancel(task);
    }
    countAfterCancel = executions.load();
    canceled.store(true);
    arktest.assertLE(executions.load(), countAfterCancel + 1);
}

async function cancelPeriodicTaskByTaskAfterExecution(): Promise<void> {
    await verifyCancelPeriodicTaskAfterExecution(false);
}

async function cancelPeriodicTaskByIdAfterExecution(): Promise<void> {
    await verifyCancelPeriodicTaskAfterExecution(true);
}

async function main(): Promise<int> {
    let commonTaskSuite = new arktest.ArkTestsuite('taskpool.CommonTask');
    let commonTaskSuiteDisabled = new arktest.ArkTestsuite('taskpool.CommonTask.DISABLED');
    commonTaskSuite.addTest('DefaultWorkerPriorityIsHigh', defaultWorkerPriorityIsHigh);
    commonTaskSuite.addAsyncTest('cancelInSuccessCallback', cancelInSuccessCallback);
    commonTaskSuite.addAsyncTest('getcurrentTask', getcurrentTask);
    commonTaskSuite.addAsyncTest('testTaskInfoDuration', testTaskInfoDuration);
    commonTaskSuite.addAsyncTest('onExecutionFailedtestAsyncfun', onExecutionFailedtestAsyncfun);
    commonTaskSuite.addAsyncTest('onExecutionFailedtestmultAsyncfun', onExecutionFailedtestmultAsyncfun);
    commonTaskSuite.addAsyncTest('onFailCallbackExceptionTest', onFailCallbackExceptionTest);
    commonTaskSuite.addAsyncTest('multipleFailCallbackExceptionTest', multipleFailCallbackExceptionTest);
    commonTaskSuite.addAsyncTest('workerTimerBackedTaskCompletes', workerTimerBackedTaskCompletes);
    // #33877: Test is incorrect and flaky fails
    commonTaskSuiteDisabled.addAsyncTest(
        'pendingDependencyShouldNotBlockReadyTask', pendingDependencyShouldNotBlockReadyTask);
    commonTaskSuite.addAsyncTest('onEnqueuedNestedExecuteTest', onEnqueuedNestedExecuteTest);
    commonTaskSuite.addAsyncTest('onEnqueuedNestedRunnerExecuteTest', onEnqueuedNestedRunnerExecuteTest);
    commonTaskSuite.addAsyncTest('asyncRunnerExecuteInOnEnqueuedNoDeadlock', asyncRunnerExecuteInOnEnqueuedNoDeadlock);
    commonTaskSuite.addAsyncTest('seqRunnerExecuteInOnEnqueuedNoDeadlock', seqRunnerExecuteInOnEnqueuedNoDeadlock);
    commonTaskSuite.addAsyncTest(
        'sendDataWorkerLocalPayloadReadableAfterScope', sendDataWorkerLocalPayloadReadableAfterScope);
    commonTaskSuite.addAsyncTest('sendDataRecordSharesReferenceWithHost', sendDataRecordSharesReferenceWithHost);
    commonTaskSuite.addAsyncTest(
        'sendDataArrayBufferSharesReferenceWithHost', sendDataArrayBufferSharesReferenceWithHost);
    commonTaskSuite.addAsyncTest(
        'sendDataSequentialCallbacksPreserveSendOrder', sendDataSequentialCallbacksPreserveSendOrder);
    commonTaskSuite.addAsyncTest('sendDataPromisePayloadPassThrough', sendDataPromisePayloadPassThrough);
    commonTaskSuite.addAsyncTest('sendDataFunctionPayloadPassThrough', sendDataFunctionPayloadPassThrough);
    commonTaskSuite.addAsyncTest(
        'sendDataCustomClassInstanceSharesReferenceWithHost', sendDataCustomClassInstanceSharesReferenceWithHost);
    commonTaskSuite.addAsyncTest('sendDataScalarMultiArgsUnchanged', sendDataScalarMultiArgsUnchanged);
    commonTaskSuite.addAsyncTest(
        'sendDataEmptyInvocationStillDispatchesCallback', sendDataEmptyInvocationStillDispatchesCallback);
    commonTaskSuite.addAsyncTest(
        'sendDataPromisePayloadExecuteSucceedsWithNoopCallback',
        sendDataPromisePayloadExecuteSucceedsWithNoopCallback);
    commonTaskSuite.addAsyncTest(
        'sendDataFunctionPayloadExecuteSucceedsWithNoopCallback',
        sendDataFunctionPayloadExecuteSucceedsWithNoopCallback);
    commonTaskSuite.addAsyncTest('sendDataNullPayloadPassThrough', sendDataNullPayloadPassThrough);
    commonTaskSuite.addAsyncTest('sendDataArraySharesReferenceWithHost', sendDataArraySharesReferenceWithHost);
    commonTaskSuite.addAsyncTest(
        'sendDataTypedArraySharesReferenceWithHost', sendDataTypedArraySharesReferenceWithHost);
    commonTaskSuite.addAsyncTest('sendDataDataViewSharesReferenceWithHost', sendDataDataViewSharesReferenceWithHost);
    commonTaskSuite.addAsyncTest('sendDataBigIntPayloadPassThrough', sendDataBigIntPayloadPassThrough);
    commonTaskSuite.addAsyncTest(
        'sendDataExecuteSettlesOnlyAfterReceiveCallbacks', sendDataExecuteSettlesOnlyAfterReceiveCallbacks);
    commonTaskSuite.addAsyncTest('sequenceRunnerSendDataDeliveredOnHost', sequenceRunnerSendDataDeliveredOnHost);
    commonTaskSuite.addAsyncTest('taskpoolScalingShouldNotOverExpand', taskpoolScalingShouldNotOverExpand);
    commonTaskSuite.addAsyncTest(
        'asyncStackSubmissionKeepsTaskpoolSemantics', asyncStackSubmissionKeepsTaskpoolSemantics);
    commonTaskSuite.addAsyncTest('cancelPeriodicTaskStopsExecution', cancelPeriodicTaskStopsExecution);
    commonTaskSuite.addAsyncTest('cancelPeriodicTaskByTaskAfterExecution', cancelPeriodicTaskByTaskAfterExecution);
    commonTaskSuite.addAsyncTest('cancelPeriodicTaskByIdAfterExecution', cancelPeriodicTaskByIdAfterExecution);

    let res = await commonTaskSuite.runAsync();
    
    CoroutineExtras.stopTaskpool();
    return res;
}
