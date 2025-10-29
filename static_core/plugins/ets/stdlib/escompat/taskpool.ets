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

package escompat;

import {launch, CoroutineId, WorkerId} from "std/concurrency"
import { InteropSerializeHelper } from "std/interop";

export namespace taskpool {
    const TASKPOOL_ERROR_NOT_IN_TASKPOOL: int = 10200022;
    const TASKPOOL_ERROR_CALLBACK_NOT_REGISTERED: int = 10200024;
    const TASKPOOL_ERROR_CIRCULAR_DEPENDENCY: int = 10200026;
    const TASKPOOL_ERROR_DEPENDENCY_NOT_EXIST: int = 10200027;
    const TASKPOOL_ERROR_DELAY_TIME_NEGATIVE: int = 10200028;
    const TASKPOOL_ERROR_LISTENER_REGISTRATION: int = 10200034;
    const TASKPOOL_ERROR_EXECUTED_TASK_ADD_DEPENDENCY: int = 10200070;
    const TASKPOOL_ERROR_EXECUTED_TASK_REMOVE_DEPENDENCY: int = 10200071;
    const TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE: int = 10200072;
    const TASKPOOL_ERROR_GROUP_TASK_ADD_DEPENDENCY: int = 10200073;
    const TASKPOOL_ERROR_GROUP_TASK_RELIED_ON: int = 10200074;
    const TASKPOOL_ERROR_GROUP_TASK_EXECUTE_OUTSIDE: int = 10200075;
    const TASKPOOL_ERROR_GROUP_TASK_EXECUTE_DELAYED: int = 10200076;
    const TASKPOOL_ERROR_GROUP_TASK_ADD_GROUP: int = 10200077;
    const TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_EXECUTED: int = 10200078;
    const TASKPOOL_ERROR_SEQ_RUNNER_OR_ASYNC_RUNNER_TASK_RELIED_ON: int = 10200079;
    const TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_OUTSIDE: int = 10200080;
    const TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_DELAYED: int = 10200081;
    const TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTED: int = 10200082;
    const TASKPOOL_ERROR_LONG_TASK_MULTIPLE_EXECUTIONS: int = 10200083;
    const TASKPOOL_ERROR_LONG_TASK_NOT_SUPPORTED: int = 10200084;
    const TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_BASIC: int = 10200085;
    const TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_PERIODIC: int = 10200086;
    const TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_LONG: int = 10200087;
    const TASKPOOL_ERROR_TASK_NOT_EXECUTED_TASK: int = 10200088;
    const TASKPOOL_ERROR_TASK_NOT_EXECUTED_GROUP: int = 10200089;
    const TASKPOOL_ERROR_DEPENDENT_TASK_EXECUTE_PERIODIC: int = 10200090;
    const TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED: int = 10200091;
    const TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_DELAYED: int = 10200092;
    const TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_AGAIN: int = 10200093;
    const TASKPOOL_ERROR_PERIODIC_TASK_NOT_SUPPORTED: int = 10200094;
    const TASKPOOL_ERROR_PERIODIC_TASK_SEQUENCE_RUNNER: int = 10200095;
    const TASKPOOL_ERROR_PERIOD_NEGATIVE: int = 10200096;
    const TASKPOOL_ERROR_NO_DEPENDENCY: int = 10200097;
    const TASKPOOL_ERROR_TASK_CANCELED: int = 10200098;
    const TASKPOOL_ERROR_GROUP_CANCELED: int = 10200099;
    const TASKPOOL_ERROR_SEQ_RUNNER_CANCELED: int = 10200100;
    const TASKPOOL_ERROR_NO_PARAMS: int = 10200101;
    const TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_GROUP: int = 10200102;
    const TASKPOOL_ERROR_GROUP_TASK_ADD_SEQ_RUNNER: int = 10200103;
    const TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_GROUP: int = 10200104;
    const TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_EXECUTED: int = 10200105;
    const TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_SQUENCE: int = 10200106;
    const TASKPOOL_ERROR_PERIODIC_TASK_ASYNC_RUNNER: int = 10200107;
    const TASKPOOL_ERROR_ASYNC_RUNNER_PARAM: int = 10200108;
    const TASKPOOL_ERROR_ASYNC_RUNNER_WAITING_QUEUE_DROPPED: int = 10200109;
    const TASKPOOL_ERROR_ASYNC_RUNNER_CANCELED: int = 10200110;
    const TASKPOOL_ERROR_ASYNC_RUNNER_TASK_EXECUTED: int = 10200111;
    const TASKPOOL_ERROR_ASYNC_RUNNER_TASK_EXECUTE_DELAYED: int = 10200112;

    class TaskPoolWorker extends EAWorker {
        constructor(supportInterop: boolean = false) {
            super("taskpool", supportInterop);
            this.start();
            this.run<void>((): void => {
                this.workerId = CoroutineExtras.getWorkerId();
            }).Await();
        }

        performTask(): boolean {
            let task: Task | undefined = getTask();
            if (task === undefined) {
                return false;
            }
            removeFromIdleWorkers(this);
            workerPendingTasksCount.set(this.workerId, workerPendingTasksCount.get(this.workerId)! + 1);
            this.currentPriority = InternalTask.of(task).priority;
            let job = this.run<NullishType>((): NullishType => {
                ConcurrencyHelpers.lockGuard(taskCountMutex, () => {
                    waitingTasksNum--;
                });
                let res = InternalTask.of(task!).execute();
                return res;
            });
            const taskRunner = (value: NullishType): NullishType => {
                let res = job.Await();
                return res;
            };
            let p = Promise.resolve<NullishType>(new Object()).then<NullishType, NullishType>(taskRunner, taskRunner);
            p.then((res: NullishType) => {
				InternalTask.of(task!).resolve(res);
            }).catch((e) => {
                InternalTask.of(task!).reject(e);
            }).finally(() => {
                this.currentPriority = undefined;
                ConcurrencyHelpers.lockGuard(workersMutex, () => {
                    workerPendingTasksCount.set(this.workerId, workerPendingTasksCount.get(this.workerId)! - 1);
                });
                this.notifyTaskFinished(task);
                setTimeout(() => {
                    InternalTask.of(task!).notifyDependencies();
                    this.notifyTaskFinished(task);
                })
            });
            return true;
        }

        private notifyTaskFinished(task: Task): void {
            // When the task is a sequenceRunner task or asyncRunner task.
            InternalTask.of(task).tryTriggerRunner();

            if (getTaskNum()) {
                triggerExecuteNextTask();
            } else {
                ConcurrencyHelpers.lockGuard(workersMutex, () => {
                    if (isIdleWorker(this) && this.idleTime == 0) {
                        idleWorkers.add(this);
                        this.idleTime = Chrono.nanoNow();
                    }
                })
            }
        }

        // For managerWorker to expand.
        triggerExpand(): void {
            let expandJob = this.run<NullishType>((): NullishType => {
                let targetNum: int = 0;
                ConcurrencyHelpers.lockGuard(taskCountMutex, () => {
                    if (totalExecCount) {
                        let averageDuration: number = totalExecTime / totalExecCount;
                        let result: number = (averageDuration * waitingTasksNum) / TASK_DURATION;
                        targetNum = (min(result, waitingTasksNum)).toInt();
                    } else {
                        let waitingNum: int = waitingTasksNum;
                        targetNum = (min(waitingNum, STEP_SIZE)).toInt();
                    }
                });
                if (targetNum == 0) {
                    return 0;
                }
                ConcurrencyHelpers.lockGuard(workersMutex, () => {
                    let idleWorkersNum: int = 0;
                    for (let w : TaskPoolWorker of workers) {
                        let currentWorkerTaskCount = workerPendingTasksCount.get(w.workerId);
                        if(!currentWorkerTaskCount) {
                            idleWorkersNum++;
                        }
                    }
                    if (workers.size < workersLimit && idleWorkersNum < targetNum) {
                        let step: int = (Math.ceil(min(workersLimit - workers.size,
                                                   min(workersLimit, targetNum) - idleWorkersNum))).toInt();
                        for (let i = 0; i < step; ++i) {
                            let w: TaskPoolWorker = new TaskPoolWorker(isTaskpoolSupportInterop);
                            workers.add(w);
                        }
                    }
                });
                return 1;
            });
        }

        // For managerWorker to shrink.
        private triggerShrink(): void {
            let shrinkJob = this.run<NullishType>((): NullishType => {
                this.timerId = setInterval(() => {
                    let runningWorkersNum: int = 0;
                    ConcurrencyHelpers.lockGuard(workersMutex, () => {
                        for (let w: TaskPoolWorker of workers) {
                            let currentWorkerTaskCount = workerPendingTasksCount.get(w.workerId);
                            if(currentWorkerTaskCount) {
                                runningWorkersNum++;
                            }
                        }
                    });
                    let targetNum: int = runningWorkersNum;
                    ConcurrencyHelpers.lockGuard(taskCountMutex, () => {
                        if (totalExecCount) {
                            let averageDuration: number = totalExecTime / totalExecCount;
                            let result: number = (averageDuration * waitingTasksNum) / TASK_DURATION;
                            targetNum += (min(waitingTasksNum, result)).toInt();
                        } else {
                            targetNum += min(2, waitingTasksNum);
                        }
                    });
                    ConcurrencyHelpers.lockGuard(workersMutex, () => {
                        if (workers.size > WORKERS_MINIMUM && workers.size > targetNum) {
                            targetNum = max(WORKERS_MINIMUM, targetNum);
                        } else {
                            return;
                        }
                        let step = min(SHRINK_STEP, workers.size - targetNum);
                        for (let i = 0; i < step && i < idleWorkers.size; ++i) {
                            let idleWorker = Array.from(idleWorkers)[i];
                            if (idleWorker.idleTime != 0 && (Chrono.nanoNow() - idleWorker.idleTime) > idleThreshold) {
                                idleWorker.join();
                                workers.delete(idleWorker);
                                idleWorkers.delete(idleWorker);
                            }
                        }
                    })
                }, triggerInterval);
            });
        }

        // For managerWorker to retrigger shrink.
        retriggerShrink(): void {
            clearInterval(this.timerId);
            this.triggerShrink();
        }

        // For managerWorker to stop and release.
        stopManagerWorker(): void {
            clearInterval(this.timerId);
            this.join();
        }

        workerId: int;
        currentPriority?: Priority;
        idleTime: long;
        private timerId: int;
    }

    /**
     * @enum defines the task priority
     * @see taskpool.Task
     */
    export enum Priority {
        /// the task priority is high
        HIGH = 0,
        /// the task priority is medium, default priority
        MEDIUM = 1,
        /// the task priority is low
        LOW = 2,
        /// the task priority is idle, only be executed if there are no other priorities
        IDLE = 3
    }

    /**
     * @enum defines the task state
     * @see taskpool.TaskInfo
     */
    export enum State {
        /// the task state is waiting
        WAITING = 1,

        /// the task state is running
        RUNNING = 2,

        /// the task state is canceled
        CANCELED = 3,

        /// the task state is delayed
        DELAYED = 4
    }

    /**
     * The type of callback to be registered
     */
    export type CallbackFunction = () => void;

    /**
     * The type of callback with error code to be registered
     */
    export type CallbackFunctionWithError = (e: Error) => void;

    class InternalTask {
        constructor(task: Task) {
            this.taskMutex = ConcurrencyHelpers.mutexCreate();
            this.condVar = ConcurrencyHelpers.condVarCreate();
            this.dependentTasks = new containers.ConcurrentSet<Task>();
            this.task = task;
        }

        static of(task: Task) {
            return (task.internalTask as InternalTask);
        }

        /**
         * Check that the task can be executed as common task
         * @throws Error if the task can not be executed as common task
         * @see taskpool.execute
         */
        checkExecution(): void {
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_EXECUTE_OUTSIDE, "taskpool:: groupTask cannot execute outside");
            } else if (this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_OUTSIDE, "taskpool:: seqRunnerTask cannot execute outside");
            } else if (this.isSubmitted && (this.isDependent || this.dependentTasks.size != 0)) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE, "taskpool:: executedTask with dependency cannot execute again");
            }
            if (this.task instanceof LongTask) {
                if (this.isSubmitted) {
                    throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_LONG, "taskpool:: The long task can only be executed once");
                } else if (this.isPeriodicTask) {
                    throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_AGAIN, "taskpool:: the periodicTask cannot execute again");
                }
            }
        }

        /**
         * Check that the task can be executed as delayed task
         * @throws Error if the task can not be executed as delayed task
         * @see taskpool.executeDelayed
         */
        checkExecutionForDelayedTask(delayTime: int): void {
            if (delayTime < 0) {
                throw new Error(TASKPOOL_ERROR_DELAY_TIME_NEGATIVE, "taskpool:: The delayTime is less than zero");
            }
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_EXECUTE_DELAYED, "taskpool:: groupTask cannot executeDelayed outside");
            }
            if (this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_DELAYED, "taskpool:: seqRunnerTask cannot executeDelayed outside");
            }
            if (this.asyncId != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_TASK_EXECUTE_DELAYED, "taskpool:: asyncRunnerTask cannot executeDelayed outside");
            }
            if (this.isSubmitted && (this.isDependent || this.dependentTasks.size != 0)) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE, "taskpool:: executedTask with dependency cannot execute again");
            }
            if (this.isSubmitted && (this.task instanceof LongTask)) {
                throw new Error(TASKPOOL_ERROR_LONG_TASK_MULTIPLE_EXECUTIONS, "taskpool:: Multiple executions of LongTask are not supported in the executeDelayed");
            }
            if (this.isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_DELAYED, "taskpool:: the periodicTask cannot executeDelayed");
            }
        }

        /**
         * Check that the task can be executed as period task
         * @throws Error if the task can not be executed as period task
         * @see taskpool.executeDelayed
         */
        checkExecutionForPeriodTask(period: int): void {
            if (period < 0) {
                throw new Error(TASKPOOL_ERROR_PERIOD_NEGATIVE, "taskpool:: The period value is less than zero");
            }
            if (this.isSubmitted || this.isPeriodicTask || this.groupId != 0 || this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_PERIODIC, "taskpool:: The concurrent task has been executed and cannot be executed periodically");
            }
            if (this.isDependent || this.dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_EXECUTE_PERIODIC, "taskpool:: the task with dependency cannot executePeriodically");
            }
        }

        private waitForDependencies(): void {
            ConcurrencyHelpers.mutexLock(this.taskMutex);
            while (this.taskDependenciesCount > 0) {
                ConcurrencyHelpers.condVarWait(this.condVar, this.taskMutex);
            }
            // Only one instance of the task can be running
            if (this.isRunning) {
                ConcurrencyHelpers.condVarWait(this.condVar, this.taskMutex);
            }
            this.isRunning = true;
            ConcurrencyHelpers.mutexUnlock(this.taskMutex);
        }

        notifyDependencies(): void {
            for (const dependentTask: Task of this.dependentTasks) {
                let internalDepTask = InternalTask.of(dependentTask);
                ConcurrencyHelpers.lockGuard(internalDepTask.taskMutex, () => {
                    internalDepTask.taskDependenciesCount--;
                    ConcurrencyHelpers.condVarNotifyOne(internalDepTask.condVar, internalDepTask.taskMutex);
                });
            }
            ConcurrencyHelpers.mutexLock(this.taskMutex);
            this.isRunning = false;
            ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.taskMutex);
            ConcurrencyHelpers.mutexUnlock(this.taskMutex);
        }

        getDuration(): int {
            return ((Chrono.nanoNow() - this.startTime) / Chrono.NS_PER_MS).toInt();
        }

        execute(): NullishType {
            initWorkerPool();
            this.waitForDependencies();
            if (!taskStarted(this.task)) {
                this.notifyDependencies();
                this.throwCancelTaskError();
            }
            this.callUserCallback(this.onStartCallback);
            let result: NullishType = null;
            this.startTime = Chrono.nanoNow();
            const startCpuTime: long = Chrono.getCpuTime();
            try {
                result = this.task.taskFunction();
                this.callUserCallback(this.onSuccessCallback);
            } catch (e: Error) {
                try {
                    this.onFailCallback?.(e);
                } catch (callbackError) {}
                throw e;
            } finally {
                // We cannot reliably calculate ioDuration in 1.2 due to possible coroutine switches.
                // And we should set ioDuration = 0, totalDuration = cpuDuration here
                this.task.cpuDuration = ((Chrono.getCpuTime() - startCpuTime) / Chrono.NS_PER_MS).toInt();
                this.task.totalDuration = this.task.cpuDuration;
                this.task.ioDuration = 0;
                // Process the task finishing
                let taskFinishedhandle = () : void => {
                    const notCanceled: boolean = taskFinished(this.task);
                    if (isTaskPoolUseLaunch) {
                        this.notifyDependencies();
                    } else {
                        ConcurrencyHelpers.lockGuard(taskCountMutex, () => {
                            totalExecTime += this.getDuration();
                            totalExecCount++;
                        });
                    }
                    if (!notCanceled) {
                        this.throwCancelTaskError();
                    }
                };
                if (result instanceof Promise) {
                    result.then(() => {
                        taskFinishedhandle();
                    });
                } else {
                    taskFinishedhandle();
                }
            }
            return result;
        }

        /// @throws Error for the canceled task
        private throwCancelTaskError(): void {
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_CANCELED, "taskpool:: taskGroup has been canceled");
            }
            if (this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_CANCELED, "taskpool:: sequenceRunner task has been canceled");
            }
            if (this.asyncId != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_CANCELED, "taskpool:: asyncRunner task has been canceled");
            }
            throw new Error(TASKPOOL_ERROR_TASK_CANCELED, "taskpool:: task has been canceled");
        }

        private callUserCallback(callback: CallbackFunction | undefined): void {
            try {
                callback?.();
            } catch (e) {}
        }

        /// The task is added to taskpool for execution
        enqueue(priority?: Priority): void {
            priority = priority === undefined ? Priority.MEDIUM : priority;
            this.priority = priority;
            if (!isTaskPoolUseLaunch) {
                ConcurrencyHelpers.mutexLock(taskQueuesMutex);
                initTaskQueues();
                taskQueues?.get(priority)?.push(this.task);
                ConcurrencyHelpers.mutexUnlock(taskQueuesMutex);
            }

            this.callUserCallback(this.onEnqueueCallback);
        }

        /**
         * @see taskpool.execute
         * @see taskpool.SequenceRunner.execute
         * @see taskpool.AsyncRunner.execute
         */
        getOnReceivePromise(p: Promise<NullishType>): Promise<NullishType> {
            if (this.onReceiveCallback === undefined) {
                return p;
            }
            return p.then<NullishType>((value: NullishType): NullishType => {
                const argsArray: Array<FixedArray<NullishType>> = this.extractDataArgs();
                argsArray.forEach((args: FixedArray<NullishType>) => {
                    try {
                        this.onReceiveCallback!.unsafeCall(...args);
                    } catch(e: Error) {}
                });
                return value;
            });
        }

        /// @see taskpool.Task.getOnReceivePromise
        extractDataArgs(): Array<FixedArray<NullishType>> {
            ConcurrencyHelpers.mutexLock(this.dataMutex);
            const argsArray: Array<FixedArray<NullishType>> = this.argsArray;
            this.argsArray = new Array<FixedArray<NullishType>>();
            ConcurrencyHelpers.mutexUnlock(this.dataMutex);
            return argsArray;
        }

        /// trigger next runner task to execute
        tryTriggerRunner(): void {
            // handle sequenceRunner
            if (this.seqId != 0) {
                if (this.runner.tasks.length == 0) {
                    this.runner.isRunning = false;
                } else {
                    let nextTask = this.runner.tasks.shift()
                    InternalTask.of(nextTask!).enqueue(this.runner.priority);
                }
            }

            // handle asyncRunner
            if (this.asyncId != 0) {
                const asyncRunner = AsyncRunnerImpl.getAsyncRunnerById(this.asyncId);
                if (asyncRunner) {
                    AsyncRunnerImpl.of(asyncRunner).handleTaskCompletionFromTryTriggerRunner();
                }
            }
        }

        /// id of group which contains the task, 0 means the task is not part of any group
        groupId: int = 0;
        /// id of sequence runner which ran the task, 0 means task was not ran via any sequence runner
        seqId: int = 0;
        /// id of async runner which ran the task, 0 means task was not ran via any async runner
        asyncId: int = 0;
        /// the task has been submitted into taskpool
        isSubmitted: boolean = false;
        /// the task depends on another task
        isDependent: boolean = false;
        /// Count of tasks which should be executed before this task
        taskDependenciesCount: int = 0;
        /// Set of tasks which can be executed after this task
        dependentTasks: containers.ConcurrentSet<Task>;
        /// identifier of worker which starts to execute the task
        workerId: int = -1;
        /// an instance of the task is running
        private isRunning: boolean = false;
        /// start time of the task
        private startTime: long = 0;
        /// the task has been executed periodically
        isPeriodicTask: boolean = false;
        /// identifier of task which is executeDelayed or executePeriodically
        timerId: int = -1;
        /// id of the coroutine that executes task
        coroId: int = -1;

        // NOTE(ipetrov, #19949): Change to private when bug in frontend will be fixed
        taskMutex: Object;
        condVar: Object;

        resolve: (value: NullishType) => void;
        reject: (e: Error) => void;

        priority: Priority;

        private runner: SequenceRunner;
        private runnerAsync: AsyncRunner;

        private onReceiveCallback?: Function;
        private dataMutex: Object = ConcurrencyHelpers.mutexCreate();
        private argsArray: Array<FixedArray<NullishType>> = new Array<FixedArray<NullishType>>();

        onEnqueueCallback?: CallbackFunction;
        onStartCallback?: CallbackFunction;
        onFailCallback?: CallbackFunctionWithError;
        onSuccessCallback?: CallbackFunction;
        private task: Task;
    }

    /**
     * @class Task provides an interface to create a task to execute in the taskpool
     */
    export class Task {
        /**
         * Create a Task instance
         *
         * @param { string } name The name of task
         * @param { Function } func Concurrent function to execute in the taskpool
         * @param { ...NullishType } args arguments for function to execute with
         */
        constructor(name: string, func: Function, ...args: FixedArray<NullishType>) {
            this.name = name;
            this.arguments = Array.from(args);
            if (isSupportingInterop()) {
                let serializeHelper = new InteropSerializeHelper(args);
                this.taskFunction = (): NullishType => {
                    let deserializedArgs = serializeHelper.deserialize();
                    return func.unsafeCall(...deserializedArgs);
                };
            } else {
                this.taskFunction = (): NullishType => {
                    return func.unsafeCall(...args);
                };
            }
            this.totalDuration = 0;
            this.ioDuration = 0;
            this.cpuDuration = 0;
            // Each task has unique identifier
            this.taskId = Task.generateId();
            this.internalTask = new InternalTask(this);
        }

        /**
         * Create a Task instance
         *
         * @param { Function } func Concurrent function to execute in the taskpool
         * @param { ...NullishType } args arguments for function to execute with
         */
        constructor(func: Function, ...args: FixedArray<NullishType>) {
            this(func.name, func, ...args);
        }

        /**
         * Check current running task is canceled or not
         *
         * @returns { boolean } true if current running task is canceled, false - otherwise
         */
        static isCanceled(): boolean {
            return isCancel();
        }

        /**
         * Register a callback and call it when the task is enqueued
         *
         * @param { CallbackFunction } callback Callback to be registered and executed when the task is enqueued
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onEnqueued(callback: CallbackFunction): void {
            this.throwIfCallbackCannotBeAdded();
            InternalTask.of(this).onEnqueueCallback = callback;
        }

        /**
         * Register a callback and call it before the task execution
         *
         * @param { CallbackFunction } callback Callback to be registered and executed before the task execution
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onStartExecution(callback: CallbackFunction): void {
            this.throwIfCallbackCannotBeAdded();
            InternalTask.of(this).onStartCallback = callback;
        }

        /**
         * Register a callback and call it when the task fails to execute
         *
         * @param { CallbackFunctionWithError } callback Callback to be registered and executed when the task fails to execute
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onExecutionFailed(callback: CallbackFunctionWithError): void {
            this.throwIfCallbackCannotBeAdded();
            InternalTask.of(this).onFailCallback = callback;
        }

        /**
         * Register a callback and call it when the task successfully executed
         *
         * @param { CallbackFunction } callback Callback to be registered and executed when the task successfully executed
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onExecutionSucceeded(callback: CallbackFunction): void {
            this.throwIfCallbackCannotBeAdded();
            InternalTask.of(this).onSuccessCallback = callback;
        }

        /**
         * Add dependencies on the task array for this task
         * @param { Task[] } tasks An array of dependent tasks
         * @throws Error if
         *     - no parameters
         *     - this task or argument tasks executed (as common or via SequenceRunner)
         *     - task or argument tasks were added to any group
         *     - argument tasks add a circular dependency
         */
        addDependency(...tasks: Task[]): void {
            if (tasks.length == 0) {
                throw new Error(TASKPOOL_ERROR_NO_PARAMS, "taskpool:: addDependency has no params.");
            }
            if (InternalTask.of(this).isSubmitted || InternalTask.of(this).seqId != 0 || InternalTask.of(this).asyncId != 0) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_ADD_DEPENDENCY, "taskpool:: asyncRunnerTask or seqRunnerTask or executedTask cannot addDependency");
            }
            if (InternalTask.of(this).groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_DEPENDENCY, "taskpool:: groupTask cannot addDependency");
            }
            for (const task: Task of tasks) {
                if (InternalTask.of(task).isSubmitted || InternalTask.of(task).seqId != 0 || InternalTask.of(this).asyncId != 0) {
                    throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_OR_ASYNC_RUNNER_TASK_RELIED_ON, "taskpool:: asyncRunnerTask or seqRunnerTask or executedTask cannot be relied on");
                }
                if (InternalTask.of(task).groupId != 0) {
                    throw new Error(TASKPOOL_ERROR_GROUP_TASK_RELIED_ON, "taskpool:: groupTask cannot be relied on");
                }
                if (this.taskId == task.taskId || this.hasTaskDFS(task)) {
                    throw new Error(TASKPOOL_ERROR_CIRCULAR_DEPENDENCY, "taskpool:: There is a circular dependency");
                }
            }
            for (const task: Task of tasks) {
                if (InternalTask.of(task).dependentTasks.has(this)) {
                    continue;
                }
                InternalTask.of(task).dependentTasks.add(this);
                InternalTask.of(this).taskDependenciesCount++;
            }
            InternalTask.of(this).isDependent = (InternalTask.of(this).taskDependenciesCount != 0);
        }

        /**
         * Remove dependencies on the task array for this task
         * @param tasks An array of dependent tasks
         * @throws Error if
         *     - no parameters
         *     - this task or argument tasks executed
         *     - this task or argument tasks have not dependencies
         *     - this task does not depend on an argument task
         */
        removeDependency(...tasks: Task[]): void {
            if (tasks.length == 0) {
                throw new Error(TASKPOOL_ERROR_NO_PARAMS, "taskpool:: removeDependency has no params.");
            }
            if (!InternalTask.of(this).isDependent) {
                throw new Error(TASKPOOL_ERROR_NO_DEPENDENCY, "taskpool:: task has no dependency");
            }
            if (InternalTask.of(this).isSubmitted) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_REMOVE_DEPENDENCY, "taskpool:: executedTask cannot removeDependency");
            }
            for (const task: Task of tasks) {
                if (InternalTask.of(task).dependentTasks.size == 0 && !InternalTask.of(task).isDependent) {
                    throw new Error(TASKPOOL_ERROR_NO_DEPENDENCY, "taskpool:: task has no dependency");
                }
                if (InternalTask.of(task).isSubmitted) {
                    throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_REMOVE_DEPENDENCY, "taskpool:: cannot removeDependency on a dependent and executed task");
                }
                if (!InternalTask.of(task).dependentTasks.has(this)) {
                    throw new Error(TASKPOOL_ERROR_DEPENDENCY_NOT_EXIST, "taskpool:: The dependency does not exist");
                }
            }
            for (let task: Task of tasks) {
                InternalTask.of(task).dependentTasks.delete(this);
                InternalTask.of(this).taskDependenciesCount--;
            }
            InternalTask.of(this).isDependent = (InternalTask.of(this).taskDependenciesCount != 0);
        }

        /**
         * @returns true if the task has been completed, false - otherwise
         */
        isDone(): boolean {
            return InternalTask.of(this).isSubmitted && !hasTask(this);
        }

        /**
         * Send data to host side and trigger the registered callback
         * @param { ...NullishType } args arguments for the registered callback
         * @throws Error if the function is not called from a taskpool task
         * @throws Error if the callback is not registered
         */
        static sendData(...args: FixedArray<NullishType>): void {
            const item: Task | undefined = getCurrentTask();
            if (item === undefined) {
                throw new Error(TASKPOOL_ERROR_NOT_IN_TASKPOOL, "taskpool:: The function is not called in the TaskPool thread");
            }
            const task: Task = item;
            if (InternalTask.of(task).onReceiveCallback === undefined) {
                throw new Error(TASKPOOL_ERROR_CALLBACK_NOT_REGISTERED, "taskpool:: The callback is not registered on the host side");
            }
            ConcurrencyHelpers.mutexLock(InternalTask.of(task).dataMutex);
            InternalTask.of(task).argsArray.push(args);
            ConcurrencyHelpers.mutexUnlock(InternalTask.of(task).dataMutex);
        }

        /**
         * Register a callback for this task to receive and handle data from the taskpool task
         */
        onReceiveData(callback?: Function): void {
            InternalTask.of(this).onReceiveCallback = callback;
        }

        /// Concurrent function to execute in taskpool
        taskFunction: () => NullishType;

        /// The concurrent function arguments
        arguments?: NullishType[];

        /// Task name
        name: string;

        /// Task identity
        taskId: int;

        /// Total duration of task execution
        totalDuration: int;

        /// IO duration of task execution
        ioDuration: int;

        /// CPU duration of task execution
        cpuDuration: int;

        /**
         * @see taskpool.Task.addDependency
         */
        private hasTaskDFS(targetTask: Task): boolean {
            for (const dependentTask: Task of InternalTask.of(this).dependentTasks) {
                if (dependentTask.taskId == targetTask.taskId) {
                    return true;
                }
                if (dependentTask.hasTaskDFS(targetTask)) {
                    return true;
                }
            }
            return false;
        }

        private throwIfCallbackCannotBeAdded() {
            if (InternalTask.of(this).isSubmitted) {
                throw new Error(TASKPOOL_ERROR_LISTENER_REGISTRATION, "taskpool:: The executed task does not support the registration of listeners.");
            }
        }

        /**
         * Generate new unique identifier for a new task
         * @see Task.constructor
         */
        private static native generateId(): int;

        /// ----- Internal implementation part -----
        readonly internalTask: Any;
    }

    /**
     * @class LongTask provides an interface to create a long executing task.
     * The such task can be executed only once
     * @extends Task
     */
    export class LongTask extends Task {
        /**
         * Create a LongTask instance
         * @param { string } name The name of long task
         * @param { Function } func Concurrent function to execute in the taskpool
         */
        constructor(name: string, func: Function, ...args: FixedArray<NullishType>) {
            super(name, func, ...args);
        }

        /**
         * Create a LongTask instance
         * @param { Function } func Concurrent function to execute in the taskpool
         * @param { ...NullishType } args arguments for func to execute with
         */
        constructor(func: Function, ...args: FixedArray<NullishType>) {
            super("", func, ...args);
        }
    }

    class GroupState {
        constructor() {
            this.id = GroupState.generateGroupId();
            this.tasks = new Array<Task>();
        }

        static of(group: TaskGroup) {
            return group.state as GroupState;
        }

        /**
         * Generate unique identifier for the group
         * @see TaskGroup.constructor
         * @return unique group identifier for a new Taskgroup instance
         */
        private static native generateGroupId(): int;

        readonly id: int;
        tasks: Array<Task>;
        isTaskAlreadySubmitted: boolean = false;
    }

    /**
     * @class TaskGroup provides an interface to create a task group
     */
    export class TaskGroup {
        /**
         * Create a TaskGroup instance
         */
        constructor() {
            this("");
        }

        /**
         * Create a TaskGroup instance
         * @param { string } name The name of TaskGroup
         */
        constructor(name: string) {
            this.name = name;
            this.state = new GroupState();
        }

        /**
         * Add a concurrent function into task group
         *
         * @param { Function } func the concurrent function to add to the task group
         * @param { ...NullishType } args arguments for func to execute with
         */
        addTask(func: Function, ...args: FixedArray<NullishType>): void {
            this.addTask(new Task(func, ...args));
        }

        /**
         * Add a task into task group
         *
         * @param { Task } task the task to add to the task group
         * @throws Error if
         *     - task was already added to any group
         *     - task is LongTask
         *     - task was executed outside the group
         *     - task executed via SequenceRunner
         */
        addTask(task: Task): void {
            this.checkExecution(task);
            InternalTask.of(task).groupId = GroupState.of(this).id;
            GroupState.of(this).tasks.push(task);
        }

        private checkExecution(task: Task): void {
            if (InternalTask.of(task).groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_GROUP, "taskpool:: taskGroup cannot add groupTask");
            } else if (task instanceof LongTask) {
                throw new Error(TASKPOOL_ERROR_LONG_TASK_NOT_SUPPORTED, "taskpool:: The interface does not support the long task");
            } else if (InternalTask.of(task).isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_NOT_SUPPORTED, "taskpool:: The interface does not support the periodicTask");
            } else if (InternalTask.of(task).isSubmitted || InternalTask.of(task).seqId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_SEQ_RUNNER, "taskpool:: taskGroup cannot add seqRunnerTask or executedTask");
            } else if (InternalTask.of(task).isDependent || InternalTask.of(task).dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED, "taskpool:: dependent task not allowed.");
            }
        }

        /**
         * TaskGroup name
         */
        name: string;

        /// ----- Internal implementation part -----
        readonly state: Any;
    }

    class SeqRunnerImpl {
        constructor(id: int) {
            this.id = id;
            this.seqPromise = Promise.resolve<NullishType>(new Object());
        }
        readonly id: int;
        seqPromise: Promise<NullishType>;
    }

    /**
     * @class SequenceRunner provides a queue, in which all tasks are executed in sequence
     */
    export class SequenceRunner {
        /**
         * Create a SequenceRunner instance
         */
        constructor(priority?: Priority) {
            this("", priority);
        }

        constructor(name: string, priority?: Priority) {
            this.impl = SequenceRunner.getOrCreateSeqRunnerImpl(name);
            this.priority = priority === undefined ? Priority.MEDIUM : priority;
            this.tasks = new Array<Task>();
        }

        execute(task: Task): Promise<NullishType> {
            initWorkerPool();
            this.checkExecution(task);
            InternalTask.of(task).seqId = this.impl.id;
            InternalTask.of(task).runner = this;
            taskSubmitted(task);
            InternalTask.of(task).isSubmitted = true;
            if (isTaskPoolUseLaunch) {
                InternalTask.of(task).enqueue(this.priority);
                const taskRunner = (value: NullishType): NullishType => {
                    return InternalTask.of(task).execute();
                };
                this.impl.seqPromise = InternalTask.of(task).getOnReceivePromise(this.impl.seqPromise.then<NullishType>(taskRunner, (error) => {
                    taskRunner(undefined);
                    return new Promise<never>((resolve, reject) => {});
                } ));
                return this.impl.seqPromise;
            }

            const promise = new Promise<NullishType>((resolve, reject) => {
                InternalTask.of(task).resolve = resolve;
                InternalTask.of(task).reject = reject;
            });
            if (this.isRunning) {
                this.tasks.push(task);
            } else {
                this.isRunning = true;
                InternalTask.of(task).enqueue(this.priority);
            }
            notifyExecuteTask();
            return InternalTask.of(task).getOnReceivePromise(promise);
        }

        private static native generateSeqRunnerId(): int;

        private static getOrCreateSeqRunnerImpl(name: string): SeqRunnerImpl {
            if (name == "") {
                return new SeqRunnerImpl(SequenceRunner.generateSeqRunnerId());
            }
            ConcurrencyHelpers.mutexLock(SequenceRunner.mutex);
            let impl: SeqRunnerImpl | undefined = SequenceRunner.namedRunners.get(name);
            if (impl === undefined) {
                // No sequence runner with passed name, so create instance and set to named runners set
                impl = new SeqRunnerImpl(SequenceRunner.generateSeqRunnerId());
                SequenceRunner.namedRunners.set(name, impl);
            }
            ConcurrencyHelpers.mutexUnlock(SequenceRunner.mutex);
            return impl!;
        }

        private checkExecution(task: Task) {
            if (InternalTask.of(task).groupId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_GROUP, "taskpool:: sequenceRunner cannot execute groupTask");
            } else if (InternalTask.of(task).isSubmitted || InternalTask.of(task).seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_EXECUTED, "taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask");
            } else if (InternalTask.of(task).isDependent || InternalTask.of(task).dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED, "taskpool:: sequenceRunner dependent task not allowed.");
            } else if (InternalTask.of(task).isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_SEQUENCE_RUNNER, "taskpool:: sequenceRunner cannot execute the periodicTask");
            } else if (InternalTask.of(task).isSubmitted) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_BASIC, "taskpool:: task has been executed");
            }
        }

        private impl: SeqRunnerImpl;
        private priority: Priority;
        private isRunning: Boolean = false;
        private tasks: Array<Task>;

        static namedRunners: Map<string, SeqRunnerImpl> = new Map<string, SeqRunnerImpl>();
        static mutex: Object = ConcurrencyHelpers.mutexCreate();
    }

    class AsyncRunnerImpl {
        constructor(id: int, runningCapacity: int, waitingCapacity: int) {
            this.id = id;
            this.runningCapacity = runningCapacity;
            this.waitingCapacity = waitingCapacity;
            this.runningCount = 0;
            this.waitingQueue = new Array<Task>();
            this.implInnerMutex = ConcurrencyHelpers.mutexCreate();
        }

        static of(runner: AsyncRunner) {
            return (runner.impl as AsyncRunnerImpl);
        }

        static getAsyncRunnerById(id: int): AsyncRunner | undefined {
            ConcurrencyHelpers.mutexLock(AsyncRunnerImpl.implMutex);
            let r = AsyncRunnerImpl.asyncRunners.get(id);
            ConcurrencyHelpers.mutexUnlock(AsyncRunnerImpl.implMutex);
            return r;
        }

        /**decrement runningCount and poll tasks from the waiting queue
         * @see Internal.tryTriggerRunner()
         */
        handleTaskCompletionFromTryTriggerRunner(): void {
            let toEnqueue: Array<Task> = new Array<Task>();
            ConcurrencyHelpers.mutexLock(this.implInnerMutex);
            try {

                if (this.runningCount > 0) {
                    this.runningCount--;
                }
                // poll tasks from the waiting queue if available
                while (this.waitingQueue.length > 0 && this.runningCount < this.runningCapacity) {
                    let nextTask = this.waitingQueue.shift();
                    if (nextTask == undefined) break;
                    this.runningCount++;
                    toEnqueue.push(nextTask);
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.implInnerMutex);
            }

            for (let t of toEnqueue) {
                InternalTask.of(t).enqueue(InternalTask.of(t).priority);
            }
            if (toEnqueue.length > 0) {
                notifyExecuteTask();
            }
        }

        readonly id: int;
        readonly runningCapacity: int;
        readonly waitingCapacity: int;

        private runner: AsyncRunner;

        runningCount: int;
        waitingQueue: Array<Task>;
        implInnerMutex: Object;
        static implMutex: Object = ConcurrencyHelpers.mutexCreate();
        // Find the corresponding asyncRunner by task.asyncID
        static asyncRunners: Map<number, AsyncRunner> = new Map<number, AsyncRunner>();
    }

    /**
     * @class The AsyncRunner class provides an interface to create an Asyncrunner.
     */
    export class AsyncRunner {
        /**
         * Create a AsyncRunner instance.
         * @param runningCapacity The maximum task execution capacity.
         * @param waitingCapacity The waiting task capacity, 0 is default, means no limit on waiting task capacity.
         */
        constructor(runningCapacity: int, waitingCapacity?: int) {
            this(AsyncRunner.ANONYMOUS_NAME, runningCapacity, waitingCapacity);
        }

        /**
         * Create or get a AsyncRunner instance by name.
         * @param name AsyncRunner's name, if name is the same, will return the same AsyncRunner.
         * @param runningCapacity The maximum task execution capacity.
         * @param waitingCapacity The waiting task capacity, 0 is default, means no limit on waiting task capacity.
         */
        constructor(name: string | undefined, runningCapacity: int, waitingCapacity?: int) {
            const rcap: int = runningCapacity;
            if (runningCapacity == undefined || runningCapacity <= 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_PARAM, "taskpool:: runningCapacity must be a positive number");
            }
            const wcap: int = waitingCapacity == undefined ? 0 : waitingCapacity;
            if (waitingCapacity != undefined && wcap < 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_PARAM, "taskpool:: watingCapacity must be a non-negative number");
            }

            this.impl = AsyncRunner.getOrCreateAsyncRunnerImpl(name, rcap, wcap);
            this.priority = Priority.MEDIUM;

            ConcurrencyHelpers.lockGuard(AsyncRunner.mutex, () => {
                AsyncRunnerImpl.asyncRunners.set(this.impl.id, this);
            });
        }

        /**
         * Execute a concurrent function.
         * @param task The task to be executed
         * @param priority Task execution priority, MEDIUM is default.
         * @returns 
         */
        execute(task: Task, priority?: Priority): Promise<Any> {
            initWorkerPool();
            this.checkExecution(task);
            
            InternalTask.of(task).runnerAsync = this;
            InternalTask.of(task).asyncId = this.impl.id; 
            taskSubmitted(task);
            InternalTask.of(task).isSubmitted = true;

            const promise = new Promise<Any>((resolve, reject) => {
                InternalTask.of(task).resolve = resolve;
                InternalTask.of(task).reject = reject;
            });

            ConcurrencyHelpers.mutexLock(this.impl.implInnerMutex);
            try {
                if (this.impl.runningCount < this.impl.runningCapacity) {
                    this.impl.runningCount = this.impl.runningCount + 1;
                    InternalTask.of(task).enqueue(priority == undefined ? this.priority : priority);
                    notifyExecuteTask();
                } else {
                    // If waitingCapacity == 0, treat it's waitingQueue's length as unlimited. Task is added to tail.
                    if (this.impl.waitingCapacity == 0) {
                        this.impl.waitingQueue.push(task);
                    } else {
                        if (this.impl.waitingCapacity > 0 && this.impl.waitingQueue.length >= this.impl.waitingCapacity) {
                            // the waiting queue exceeds capacity, discard the head task.
                            const head = this.impl.waitingQueue.shift();
                            if (head != undefined) {
                                if (InternalTask.of(head).reject) {
                                    InternalTask.of(head).reject(new Error(TASKPOOL_ERROR_ASYNC_RUNNER_WAITING_QUEUE_DROPPED, "taskpool:: waiting queue dropped head"));
                                }
                                InternalTask.of(head).isSubmitted = false;
                            }
                        }
                        this.impl.waitingQueue.push(task);
                    }
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.impl.implInnerMutex);
            }

            return InternalTask.of(task).getOnReceivePromise(promise);
        }

        private static native generateAsyncRunnerId(): int;
 
        private static getOrCreateAsyncRunnerImpl(name: string | undefined, runningCapacity: int, waitingCapacity: int): AsyncRunnerImpl {
            if (name == AsyncRunner.ANONYMOUS_NAME) {
                return new AsyncRunnerImpl(AsyncRunner.generateAsyncRunnerId(), runningCapacity, waitingCapacity);
            }
            ConcurrencyHelpers.mutexLock(AsyncRunner.mutex);
            let impl: AsyncRunnerImpl | undefined = AsyncRunner.namedRunners.get(name);
            if (impl === undefined) {
                // No asyncRunner with passed name, so create instance and set to named runners map.
                impl = new AsyncRunnerImpl(AsyncRunner.generateAsyncRunnerId(), runningCapacity, waitingCapacity);
                AsyncRunner.namedRunners.set(name, impl);
            }
            ConcurrencyHelpers.mutexUnlock(AsyncRunner.mutex);
            return impl!;
        }

        private checkExecution(task: Task) {
            if (InternalTask.of(task).groupId != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_GROUP, "taskpool:: asyncRunner cannot execute groupTask");
            } else if (InternalTask.of(task).isSubmitted || InternalTask.of(task).asyncId != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_EXECUTED, "taskpool:: asyncRunner cannot execute executedTask");
            } else if (InternalTask.of(task).isDependent || InternalTask.of(task).dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED, "taskpool:: asyncRunner dependent task not allowed.");
            } else if (InternalTask.of(task).isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_ASYNC_RUNNER, "taskpool:: asyncRunner cannot execute the periodicTask");
            } else if (InternalTask.of(task).isSubmitted) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_BASIC, "taskpool:: task has been executed");
            } else if (InternalTask.of(task).seqId != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_SQUENCE, "taskpool:: asyncRunner cannot execute seqenceRunnerTask");
            }
        }


        private impl: AsyncRunnerImpl;
        private priority: Priority;
        private static readonly ANONYMOUS_NAME: string | undefined = undefined;

        // Reuse AsyncRunner instance by a specified name
        private static namedRunners: Map<string | undefined, AsyncRunnerImpl> = new Map<string | undefined, AsyncRunnerImpl>();
        private static mutex: Object = ConcurrencyHelpers.mutexCreate();
    }

    /**
     * @class represents internal information about task in taskpool
     */
    export class TaskInfo {
        /// Unique identifier of task
        taskId: int = 0;

        /// Task state in taskpool
        state: State = State.WAITING;

        /// Duration of task exeuction
        duration?: int;

        /// Task name
        name: string = "";
    }

    /**
     * @class represents internal information about worker thread with taskpool tasks
     */
    export class ThreadInfo {
        /// Worker thread id
        tid: int = -1;

        /// Running task identifiers list on current worker thread
        taskIds?: int[];

        /// Thread priority
        priority?: Priority;
    }

    /**
     * @class represents internal information about taskpool
     */
    export class TaskPoolInfo {
        /// Array of threads information with taskpool tasks
        threadInfos: ThreadInfo[] = new ThreadInfo[0];

        /// Array of taskpool tasks information
        taskInfos: TaskInfo[] = new TaskInfo[0];
    }

    /**
     * Create task and execute
     *
     * @param { Function } func a concurrent function to create task from
     * @param { ...NullishType } args arguments for func to execute with
     * @returns { Promise<NullishType> } Promise for result of executed task
     */
    export function execute(func: Function, ...args: FixedArray<NullishType>): Promise<NullishType> {
        initWorkerPool();
        if (isTaskPoolUseLaunch) {
            return launchPromise(func, ...args);
        }
        let task = new Task(func.name, func, ...args);
        return execute(task);
    }

    /**
     * Execute a concurrent task
     *
     * @param task The task to execute
     * @param { Priority } [priority] priority for the task
     * @returns { Promise<NullishType> } Promise for result of executed task
     */
    export function execute(task: Task, priority?: Priority): Promise<NullishType> {
        initWorkerPool();
        InternalTask.of(task).checkExecution();
        taskSubmitted(task);
        InternalTask.of(task).isSubmitted = true;
        InternalTask.of(task).enqueue(priority);

        if (isTaskPoolUseLaunch) {
            return InternalTask.of(task).getOnReceivePromise(launchPromise(InternalTask.of(task).execute));
        }

        const promise = new Promise<NullishType>((resolve, reject) => {
            InternalTask.of(task).resolve = resolve;
            InternalTask.of(task).reject = reject;
        });
        notifyExecuteTask();
        return InternalTask.of(task).getOnReceivePromise(promise);
    }

    /**
     * Execute a concurrent task group
     *
     * @param { TaskGroup } group The task group for execution
     * @param { Priority } [priority] priority for tasks in the group
     * @returns { Promise<Array<NullishType>> } Promise for array of results of executed tasks from the group
     */
    export function execute(group: TaskGroup, priority?: Priority): Promise<Array<NullishType>> {
        initWorkerPool();
        const tasksCount = GroupState.of(group).tasks.length.toInt();
        if (tasksCount == 0) {
            return Promise.resolve<Array<NullishType>>(new Array<NullishType>());
        }
        taskGroupSubmitted(GroupState.of(group).id, tasksCount);
        GroupState.of(group).isTaskAlreadySubmitted = true;
        const promises: Array<Promise<NullishType>> = new Array<Promise<NullishType>>();
        GroupState.of(group).tasks.forEach((task: Task) => {
            taskSubmitted(task);
            InternalTask.of(task).isSubmitted = true;
            InternalTask.of(task).enqueue(priority);
            if (isTaskPoolUseLaunch) {
                promises.push(InternalTask.of(task).getOnReceivePromise(launchPromise(InternalTask.of(task).execute)));
            } else {
                const promise = new Promise<NullishType>((resolve, reject) => {
                    InternalTask.of(task).resolve = resolve;
                    InternalTask.of(task).reject = reject;
                });
                notifyExecuteTask();
                promises.push(InternalTask.of(task).getOnReceivePromise(promise));
            }
        });
        return Promise.all<NullishType>(promises);
    }

    /**
     * Delayed execution of a concurrent task
     *
     * @param { int } delayTime timeout in ms to execute task after
     * @param { Task } task The task to execute
     * @param { Priority } [priority] priority for task
     * @returns { Promise<NullishType> } Promise for result of executed task
     */
    export function executeDelayed(delayTime: int, task: Task, priority?: Priority): Promise<NullishType> {
        initWorkerPool();
        InternalTask.of(task).checkExecutionForDelayedTask(delayTime);
        InternalTask.of(task).isSubmitted = true;
        ConcurrencyHelpers.lockGuard(mutex, () => {
            pendingDelayed.add(task);
        });
        return new Promise<NullishType>((resolve, reject) => {
            InternalTask.of(task).timerId = setTimeout(() => {
                ConcurrencyHelpers.lockGuard(mutex, () => {
                    pendingDelayed.delete(task);
                });
                if (InternalTask.of(task).timerId == -1 || isCanceled(task)) {
                    ConcurrencyHelpers.lockGuard(mutex, () => {
                        pendingDelayedCanceled.delete(task);
                    });
                    reject(new Error("taskpool:: task has been canceled"));
                }
                taskSubmitted(task);
                InternalTask.of(task).enqueue(priority);
                if (isTaskPoolUseLaunch) {
                    launchImpl(task, priority).then((res: NullishType) => {
                        resolve(res);
                    }).catch((e) => {
                        reject(e);
                    })
                } else {
                    const p = new Promise<NullishType>((resolve, reject) => {
                        InternalTask.of(task).resolve = resolve;
                        InternalTask.of(task).reject = reject;
                    })
                    notifyExecuteTask();
                    InternalTask.of(task).getOnReceivePromise(p).then((res: NullishType) => {
                        resolve(res);
                    }).catch((e) => {
                        reject(e);
                    })
                }
            }, delayTime);
        });
    }

    /**
     * Delayed execution of a concurrent task
     *
     * @param { int } period period in ms to execute task
     * @param { Task } task The task to execute
     * @param { Priority } [priority] priority for task
     */
    export function executePeriodically(period: int, task: Task, priority?: Priority): void {
        initWorkerPool();
        InternalTask.of(task).checkExecutionForPeriodTask(period);
        InternalTask.of(task).isSubmitted = true;
        InternalTask.of(task).isPeriodicTask = true;
        InternalTask.of(task).timerId = setInterval(() => {
            if (InternalTask.of(task).timerId == -1) {
                return;
            }
            taskSubmitted(task);
            InternalTask.of(task).enqueue(priority);
            if (isTaskPoolUseLaunch) {
                launchImpl(task, priority);
            } else {
                const p = new Promise<NullishType>((resolve, reject) => {
                    InternalTask.of(task).resolve = resolve;
                    InternalTask.of(task).reject = reject;
                })
                notifyExecuteTask();
            }
        }, period);
    }

    /**
     * Cancel a concurrent task
     *
     * @param task The task for canceling
     * @throws Error if the task does not exist when it is canceled
     * @see Task.isCancel
     */
    export function cancel(task: Task): void {
        ConcurrencyHelpers.lockGuard(mutex, () => {
            let timerId = -1;
            if (InternalTask.of(task).timerId !== -1) {
                timerId = InternalTask.of(task).timerId;
                if (InternalTask.of(task).isPeriodicTask) {
                    clearInterval(InternalTask.of(task).timerId);
                }
                InternalTask.of(task).timerId = -1;
            }
            if (pendingDelayed.has(task)) {
                pendingDelayedCanceled.add(task);
                return;
            }
            if (waitingTasks.has(task) || runningTasks.has(task)) {
                tasksToBeCanceled.add(task);
                return;
            }
            if (timerId != -1) {
                return;
            }
            if (InternalTask.of(task).seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTED, "taskpool:: sequenceRunner task has been executed");
            } else if (InternalTask.of(task).asyncId != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_TASK_EXECUTED, "taskpool:: asyncRunner task has been executed")
            } else {
                throw new Error(TASKPOOL_ERROR_TASK_NOT_EXECUTED_TASK, "taskpool:: task is not executed or has been executed");
            }
        });
        tryCancelDependentTasks(task);
    }

    /**
     * Cancel a concurrent task group
     *
     * @param group The group for canceling
     * @throws Error if the task group does not exist when it is canceled
     */
    export function cancel(group: TaskGroup): void {
        ConcurrencyHelpers.lockGuard(mutex, () => {
            if (waitingGroupTasks.has(GroupState.of(group).id) || runningGroupTasks.has(GroupState.of(group).id)) {
                groupsToBeCanceled.add(GroupState.of(group).id);
                return;
            }
            throw new Error(TASKPOOL_ERROR_TASK_NOT_EXECUTED_GROUP, "taskpool:: taskGroup is not executed or has been executed");
        });
    }

    /**
     * Terminate a long task.
     *
     * @param longTask The long task for terminating
     * @note This method is needed for compatibilty with OHOS API. For coroutines in static ArkTS it is not required
     */
    export function terminateTask(longTask: LongTask): void {}

    /**
     * @returns { TaskPoolInfo } taskpool internal information about tasks and threads
     */
    export function getTaskPoolInfo(): TaskPoolInfo {
        initWorkerPool();
        const taskInfos: Array<TaskInfo> = new Array<TaskInfo>();
        const threadInfos: Array<ThreadInfo> = new Array<ThreadInfo>();
        ConcurrencyHelpers.mutexLock(mutex);
        // Collect running and canceled tasks info
        runningTasks.forEach((count: int, task: Task) => {
            const taskInfo: TaskInfo = new TaskInfo();
            taskInfo.taskId = task.taskId;
            taskInfo.name = task.name;
            taskInfo.state =  (isCanceled(task)) ? State.CANCELED : State.RUNNING;
            taskInfo.duration = InternalTask.of(task).getDuration();
            taskInfos.push(taskInfo);
        });
        // Collect waiting and canceled tasks info
        waitingTasks.forEach((count: int, task: Task) => {
            // One task can be submitted to taskpool several time, but only one instance can be running,
            // so if task is running, it was added with running status
            if (runningTasks.has(task)) {
                return;
            }
            const taskInfo: TaskInfo = new TaskInfo();
            taskInfo.taskId = task.taskId;
            taskInfo.name = task.name;
            taskInfo.state =  (isCanceled(task)) ? State.CANCELED : State.WAITING;
            taskInfos.push(taskInfo);
        });
        // Collect delay and canceled tasks info
        pendingDelayed.forEach((task: Task) => {
            const taskInfo: TaskInfo = new TaskInfo();
            taskInfo.taskId = task.taskId;
            taskInfo.name = task.name;
            taskInfo.state =  (isCanceled(task)) ? State.CANCELED : State.DELAYED;
            taskInfos.push(taskInfo);
        });
        // Collect worker threads info
        if (!isTaskPoolUseLaunch) {
            workers.forEach(worker=>{
                const threadInfo: ThreadInfo = new ThreadInfo();
                threadInfo.tid = worker.workerId;
                threadInfo.priority = worker!.currentPriority;
                let tasks = workerToTasks.get(worker.workerId);
                if (tasks != undefined) {
                    let tasksCount : int = tasks.size.toInt();
                    const taskIds: int[] = new int[tasksCount];
                    let currentIndex: int = 0;
                    tasks.forEach((task: Task) => {
                        taskIds[currentIndex] = task.taskId;
                        currentIndex++;
                    });
                    threadInfo.taskIds = taskIds;
                }
                threadInfos.push(threadInfo);
            })
        } else {
            workerToTasks.forEach((tasks: Set<Task>, workerId: int) => {
                const threadInfo: ThreadInfo = new ThreadInfo();
                threadInfo.tid = workerId;
                const tasksCount: int = tasks.size.toInt();
                if (tasksCount == 0) {
                    threadInfos.push(threadInfo);
                    return;
                }
                const taskIds: int[] = new int[tasksCount];
                let currentIndex: int = 0;
                tasks.forEach((task: Task) => {
                    taskIds[currentIndex] = task.taskId;
                    currentIndex++;
                    // priority of running tasks can be regard as the priority of current thread's priority
                    // current only when task has been executed the worker can execute other task
                    if (runningTasks.has(task) && threadInfo.priority === undefined) {
                        threadInfo.priority = InternalTask.of(task).priority;
                    }
                });
                threadInfo.taskIds = taskIds;
                threadInfos.push(threadInfo);
            });
        }
        ConcurrencyHelpers.mutexUnlock(mutex);
        const Info: TaskPoolInfo = new TaskPoolInfo();
        const taskInfosRes: TaskInfo[] = new TaskInfo[taskInfos.length.toInt()];
        const threadInfosRes: ThreadInfo[] = new ThreadInfo[threadInfos.length.toInt()];
        taskInfos.forEach((taskInfo: TaskInfo, index: number) => {
            taskInfosRes[index.toInt()] = taskInfo;
        });
        threadInfos.forEach((threadInfo: ThreadInfo, index: number) => {
            threadInfosRes[index.toInt()] = threadInfo;
        });
        Info.taskInfos = taskInfosRes;
        Info.threadInfos = threadInfosRes;
        return Info;
    }

    /// ----- Internal implementation part -----

    /**
     * @brief Submit task to the taskpool on the execute method
     * @see taskpool.execute
     * @see taskpool.SequenceRunner.execute
     * @see taskpool.AsyncRunner.execute
     * @param task submitting task
     */
    function taskSubmitted(task: Task): void {
        ConcurrencyHelpers.mutexLock(mutex);
        const count: int = waitingTasks.get(task, /* default */ 0);
        waitingTasks.set(task, count + 1);
        ConcurrencyHelpers.mutexUnlock(mutex);
    }

    /**
     * @brief Submit group of tasks to the taskpool on the execute method
     * @see taskpool.execute
     * @param groupId identifier of the submitted group
     * @param tasksCount count of tasks in the passed group
     */
    function taskGroupSubmitted(groupId: int, tasksCount: int): void {
        ConcurrencyHelpers.mutexLock(mutex);
        const count: int = waitingGroupTasks.get(groupId, /* default */ 0);
        waitingGroupTasks.set(groupId, count + tasksCount);
        ConcurrencyHelpers.mutexUnlock(mutex);
    }

    /**
     * @brief Notify the taskpool that the task is started on a coroutine
     * @param task starting task
     * @returns true if task is not cancled by cancel method, false - otherwise
     *
     * @see taskpool.Task.execute
     * @see taskpool.cancel
     */
    function taskStarted(task: Task): boolean {
        let isTaskCancel: boolean = false;
        ConcurrencyHelpers.lockGuard(mutex, () => {
            const cancelByTask: boolean = popFromMap(/* extracting task from waiting map */ task,
                                                            /* pop from */ waitingTasks,
                                                            /* the map can contain the instance of the task */ runningTasks,
                                                            /* set of tasks marked as canceled */ tasksToBeCanceled);
            let cancelByGroup: boolean = false;
            if (InternalTask.of(task).groupId != 0) {
                // If the task is group task then decrement count of waiting task for the group
                cancelByGroup = popFromMap(InternalTask.of(task).groupId, waitingGroupTasks, runningGroupTasks, groupsToBeCanceled);
            }
            if (cancelByGroup || cancelByTask) {
                isTaskCancel = true;
                return;
            }
            pushToRunning(task);
        });
        return !isTaskCancel;
    }

    function popFromMap<T>(item: T, currentTasksMap: Map<T, int>, possibleTasksMap: Map<T, int>, cancelSet: Set<T>): boolean {
        const instancesOfTheTask: int = currentTasksMap.get(item)! - 1;
        if (instancesOfTheTask == 0) {
            // This is last instance in the map
            currentTasksMap.delete(item);
        } else {
            currentTasksMap.set(item, instancesOfTheTask);
        }
        if (cancelSet.has(item)) {
            if (instancesOfTheTask == 0 && !possibleTasksMap.has(item)) {
                // No more instances in the taskpool, so delete from cancelSet
                cancelSet.delete(item);
            }
            return true; // was marked as canceled
        }
        return false;
    }

    function pushToRunning(task: Task): void {
        const runningInstancesOfTheTask: int = runningTasks.get(task, /* default */ 0);
        runningTasks.set(task, runningInstancesOfTheTask + 1);
        if (InternalTask.of(task).groupId != 0) {
            const runningTasksOfTheGroup: int = runningGroupTasks.get(InternalTask.of(task).groupId, /* default */ 0);
            runningGroupTasks.set(InternalTask.of(task).groupId, runningTasksOfTheGroup + 1);
        }
        // NOTE(ipetrov, #20208): CoroutineExtras is debug functionality, maybe need to create a separate intrinsic
        const currentCoroId: int = CoroutineExtras.getCoroutineId()
        InternalTask.of(task).coroId = currentCoroId;
        executingTasks.set(currentCoroId, task);
        const currentWorkerId: int = CoroutineExtras.getWorkerId();
        InternalTask.of(task).workerId = currentWorkerId;
        currentExecutingTasks.set(currentWorkerId, task);
        const workerTasks: Set<Task> = workerToTasks.get(currentWorkerId, /* default */ new Set<Task>());
        workerTasks.add(task);
        workerToTasks.set(currentWorkerId, workerTasks);
    }

    /**
     * @brief Notify the taskpool that the task is finished on a coroutine
     * @param taskId identifier of the finished task
     * @param groupId group identifier of the passed task (0 means task is not group task)
     * @returns true if task is not cancled by cancel method, false - otherwise
     *
     * @see taskpool.Task.execute
     * @see taskpool.cancel
     */
    function taskFinished(task: Task): boolean {
        let isTaskCanceled: boolean = false;
        ConcurrencyHelpers.lockGuard(mutex, () => {
            executingTasks.delete(InternalTask.of(task).coroId);
            workerToTasks.get(InternalTask.of(task).workerId)!.delete(task);
            const cancelByTask: boolean = popFromMap(task, runningTasks, waitingTasks, tasksToBeCanceled);
            let cancelByGroup: boolean = false;
            if (InternalTask.of(task).groupId != 0) {
                cancelByGroup = popFromMap(InternalTask.of(task).groupId, runningGroupTasks, waitingGroupTasks, groupsToBeCanceled);
            }
            if (cancelByGroup || cancelByTask) {
                isTaskCanceled = true;
            }
        });
        return !isTaskCanceled;
    }

    function getCurrentTaskUnsafe(): Task | undefined {
        // NOTE(ipetrov, #20208): CoroutineExtras is debug functionality, maybe need to create a separate intrinsic
        if (isTaskPoolUseLaunch) {
            return executingTasks.get(CoroutineExtras.getCoroutineId());
        }
        return currentExecutingTasks.get(CoroutineExtras.getWorkerId());
    }

    function getCurrentTask(): Task | undefined {
        ConcurrencyHelpers.mutexLock(mutex);
        const task: Task | undefined = getCurrentTaskUnsafe();
        ConcurrencyHelpers.mutexUnlock(mutex);
        return task;
    }

    function isCancel(): boolean {
        ConcurrencyHelpers.mutexLock(mutex);
        const item: Task | undefined = getCurrentTaskUnsafe();
        if (item === undefined) {
            ConcurrencyHelpers.mutexUnlock(mutex);
            // No task on current coroutine
            return false;
        }
        const result: boolean = isCanceled(item!);
        ConcurrencyHelpers.mutexUnlock(mutex);
        return result;
    }

    function isCanceled(task: Task): boolean {
        return pendingDelayedCanceled.has(task) || tasksToBeCanceled.has(task) || groupsToBeCanceled.has(InternalTask.of(task).groupId);
    }

    function tryCancelDependentTasks(task: Task): void {
        for(const dependentTask: Task of InternalTask.of(task).dependentTasks) {
            try {
                cancel(dependentTask);
            } catch (e) {
                // The error can be ignored.
            }
        }
    }

    function hasTask(task: Task): boolean {
        ConcurrencyHelpers.mutexLock(mutex);
        const result: boolean = waitingTasks.has(task) || runningTasks.has(task);
        ConcurrencyHelpers.mutexUnlock(mutex);
        return result;
    }

    /// Stop all taskpool workers for debug.
    function stopAllWorkers(): void {
        if (isTaskPoolUseLaunch) {
            return;
        }
        managerWorker?.stopManagerWorker();
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            for (let w : TaskPoolWorker of workers) {
                w.join();
            }
            workers.clear();
            idleWorkers.clear();
        });
    }

    /**
     * @brief Set taskpool trigger shrink interval in taskpool test suite for debug.
     * @param interval The trigger shrink interval value.
     */
    function setTaskPoolTriggerShrinkInterval(interval: int): void {
        triggerInterval = interval;
    }

    /**
     * @brief Set taskpool idle threshold in taskpool test suite for debug.
     * @param threshold The idle threshold value.
     */
    function setTaskPoolIdleThreshold(threshold: int): void {
        idleThreshold = threshold;
    }

    /**
     * @brief Get current taskpool workers num for debug.
     * @returns TaskPool workers num.
     */
    function getTaskPoolWorkersNum(): int {
        let num = 0;
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            num = workers.size;
        });
        return num;
    }

    /**
     * @brief Set taskpool workers limit in taskpool test suite for debug.
     * @param limit TaskPool workers limit.
     */
    function setTaskPoolWorkersLimit(limit: int): void {
        if (limit <= 0) {
            return;
        }
        workersLimit = limit;
    }

    /**
     * @brief Retrigger taskpool shrink in taskpool test suite for debug.
     */
    function retriggerTaskPoolShrink(): void {
        managerWorker?.retriggerShrink();
    }

    /**
     * @brief Increase workers for debug.
     * @param num target workers num.
     */
    function increaseWorkersToN(num: int): void {
        if (isTaskPoolUseLaunch) {
            return;
        }
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            let workersNum = workers.size;
            if (workersNum > num) {
                return;
            }
            for (let i = workersNum; i < num; ++i) {
                let w = new TaskPoolWorker();
                workers.add(w);
            }
        })
    }

    /**
     * @brief Initialize taskpool worker pool.
     * @see taskpool.execute
     * @see taskpool.Task.constructor
     * @see taskpool.TaskGroup.constructor
     * @see taskpool.SequenceRunner.constructor
     * @see taskpool.AsyncRunner.constructor
     */
    function initWorkerPool(): void {
        if (isWorkersInitialized.get() || isTaskPoolUseLaunch) {
                return;
        }

        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            if (isWorkersInitialized.get() || isTaskPoolUseLaunch) {
                return;
            }

            if (managerWorker === undefined) {
                // Create managerWorker, and start to trigger shrink.
                managerWorker = new TaskPoolWorker(isTaskpoolSupportInterop);
                managerWorker.retriggerShrink();
            }

            // Init taskpool worker pool.
            for (let i = 0; i < initialWorkersNumber; ++i) {
                let w = new TaskPoolWorker(isTaskpoolSupportInterop);
                workers.add(w);
            }
            isWorkersInitialized.set(true);
        })
    }

    /**
     * @brief Try to trigger expand.
     * @see taskpool.notifyExecuteTask
     */
    function tryTriggerExpand(): void {
        managerWorker?.triggerExpand();
    }

    /**
     * @brief Notify idle workers to perform tasks.
     * @see taskpool.execute
     * @see taskpool.executeDelayed
     * @see taskpool.executePeriodically
     * @see taskpool.SequenceRunner.execute
     */
    function notifyExecuteTask(): void {
        ConcurrencyHelpers.lockGuard(taskCountMutex, () => {
            waitingTasksNum++;
        });
        triggerExecuteNextTask();
        tryTriggerExpand();
    }

    /**
     * @brief Trigger execute task.
     * @see taskpool.notifyExecuteTask
     * @see taskpool.TaskPoolWorker.notifyTaskFinished
     */
    function triggerExecuteNextTask(): void {
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            let w = chooseWorkerForTask();
            if (!w.performTask() && getTaskNum() > 0) {
                // Current getTask may get an undefined task when higher priority task hit the continueCount
                // So here we try to performTask again to avoid task cannot be executed
                w.performTask();
            } else if (isIdleWorker(w) && w.idleTime == 0) {
                idleWorkers.add(w);
                w.idleTime = Chrono.nanoNow();
            }
        })
    }

    /**
     * @brief Check the taskpool worker is idle or not.
     * @see taskpool.TaskPoolWorker.notifyTaskFinished
     * @see taskpool.triggerExecuteNextTask
     */
    function isIdleWorker(w: TaskPoolWorker): boolean {
        let currentWorkerTaskCount = workerPendingTasksCount.get(w.workerId);
        if(currentWorkerTaskCount) {
            return false;
        }
        return true;
    }

    /**
     * @brief Update taskpool worker state.
     * @see taskpool.TaskPoolWorker.performTask
     */
    function removeFromIdleWorkers(worker: TaskPoolWorker): void {
        for (let w: TaskPoolWorker of idleWorkers) {
            if (worker == w) {
                idleWorkers.delete(w);
                w.idleTime = 0;
                return;
            }
        }
    }

    /**
     * @brief Select an optimal worker for task execution based on current pending tasks count.
     * @see taskpool.notifyExecuteTask
     */
    function chooseWorkerForTask(): TaskPoolWorker {
        let minTaskCount = Int.MAX_VALUE;
        let selectedWorker = Array.from(workers)[0];
        for (let w : TaskPoolWorker of workers) {
            let currentWorkerTaskCount = workerPendingTasksCount.get(w.workerId);
            if(!currentWorkerTaskCount) {
                workerPendingTasksCount.set(w.workerId, 0);
                return w;
            }
            if(currentWorkerTaskCount < minTaskCount) {
                minTaskCount = currentWorkerTaskCount;
                selectedWorker = w;
            }
        }
        return selectedWorker;
    }

    /**
     * @brief Initialize taskpool taskQueues.
     * @see taskpool.Task.enqueue
     */
    function initTaskQueues(): void {
        if (isTaskQueuesInitialized) {
            return;
        }
        taskQueues = new Map<Priority, Array<Task>>();
        highTaskQueue = new Array<Task>();
        mediumTaskQueue = new Array<Task>();
        lowTaskQueue = new Array<Task>();
        idleTaskQueue = new Array<Task>();
        taskQueues.set(Priority.HIGH, highTaskQueue);
        taskQueues.set(Priority.MEDIUM, mediumTaskQueue);
        taskQueues.set(Priority.LOW, lowTaskQueue);
        taskQueues.set(Priority.IDLE, idleTaskQueue);
        isTaskQueuesInitialized = true;
    }

    /**
     * @brief The worker gets an executable task.
     * @returns Task if task has no dependency, undefined - otherwise.
     * @see taskpool.TaskPoolWorker.performTask
     */
    function getTask(): Task | undefined {
        let task: Task | undefined = undefined;
        ConcurrencyHelpers.lockGuard(taskQueuesMutex, () => {
            if ((highTaskQueue?.length != 0) && (highTaskExecuteCount < continuousExecutionCount)) {
                highTaskExecuteCount++;
                task = getTaskImpl(highTaskQueue);
                return;
            }
            highTaskExecuteCount = 0;
            if ((mediumTaskQueue?.length != 0) && (mediumTaskExecuteCount < continuousExecutionCount)) {
                mediumTaskExecuteCount++;
                task = getTaskImpl(mediumTaskQueue);
                return;
            }
            mediumTaskExecuteCount = 0;
            if (lowTaskQueue?.length != 0) {
                task = getTaskImpl(lowTaskQueue);
                return;
            }
            if (idleTaskQueue?.length != 0 && highTaskQueue?.length == 0
                && mediumTaskQueue?.length == 0 && lowTaskQueue?.length == 0) {
                task = getTaskImpl(idleTaskQueue);
                return;
            }
        })
        return task;
    }

    function getTaskImpl(tasksQueue: Array<Task>): Task | undefined {
        let task: Task | undefined = undefined;
        let len = tasksQueue.length.toInt();
        for (let i = 0; i < len; ++i) {
            task = tasksQueue.shift();
            if (InternalTask.of(task!).taskDependenciesCount == 0) {
                break;
            } else {
                let t: Task = task as Task;
                tasksQueue.push(t);
                task = undefined;
            }
        }
        return task;
    }

    /**
     * @brief Get the number of tasks to be executed .
     * @returns tasks num.
     * @see taskpool.TaskPoolWorker.notifyTaskFinished
     */
    function getTaskNum(): int {
        let tasksNum: int = 0;
        ConcurrencyHelpers.lockGuard(taskQueuesMutex, () => {
            for (const taskQueue of taskQueues.values()) {
                tasksNum += taskQueue.length;
            }
        })
        return tasksNum;
    }

    /**
     * @brief Submit delayed task or period task to the taskpool on the execute method.
     * @see taskpool.executeDelayed
     * @see taskpool.Task.executePeriodTask
     * @param task submitting task
     * @param priority task execution priority
     */
    function launchImpl(task: Task, priority?: Priority): Promise<NullishType> {
        const job: Job<NullishType> = launch<NullishType, () => NullishType>(
            InternalTask.of(task).execute
        );
        let promise = new Promise<NullishType>((resolve, reject) => {
            try {
                let res = job.Await();
                resolve(res);
            } catch(e: Error) {
                reject(e);
            }
        });
        return InternalTask.of(task).getOnReceivePromise(promise);
    }

    native function isUsingLaunch(): boolean;
    native function isSupportingInterop(): boolean;
    native function getTaskPoolWorkersLimit(): int;

    // managed mutex for taskpool containers below
    const mutex = ConcurrencyHelpers.mutexCreate();
    // value is count of tasks
    const waitingTasks: Map<Task, int> = new Map<Task, int>();
    const runningTasks: Map<Task, int> = new Map<Task, int>();
    const pendingDelayed: Set<Task> = new Set<Task>();
    const pendingDelayedCanceled: Set<Task> = new Set<Task>();
    const tasksToBeCanceled: Set<Task> = new Set<Task>();
    // key is group id, value is count of associated tasks with this group
    const waitingGroupTasks: Map<int, int> = new Map<int, int>();
    const runningGroupTasks: Map<int, int> = new Map<int, int>();
    const groupsToBeCanceled: Set<int> = new Set<int>();
    // value is executing task on the associated coroutine
    const executingTasks: Map<CoroutineId, Task> = new Map<CoroutineId, Task>();
    // value is current executing task on the associated worker
    const currentExecutingTasks: Map<WorkerId, Task> = new Map<WorkerId, Task>();
    // value is set of executing tasks on this worker
    const workerToTasks: Map<WorkerId, Set<Task>> = new Map<WorkerId, Set<Task>>();
    // Count of pending tasks per worker
    const workerPendingTasksCount: Map<WorkerId, Int> = new Map<WorkerId, Int>();

    const workersMutex = ConcurrencyHelpers.mutexCreate();
    let workers: Set<TaskPoolWorker> = new Set<TaskPoolWorker>();
    let idleWorkers: Set<TaskPoolWorker> = new Set<TaskPoolWorker>();
    let isWorkersInitialized: AtomicFlag = new AtomicFlag(false);

    const taskQueuesMutex = ConcurrencyHelpers.mutexCreate();
    let isTaskQueuesInitialized: boolean = false;
    let taskQueues: Map<Priority, Array<Task>>;
    let highTaskQueue: Array<Task>;
    let mediumTaskQueue: Array<Task>;
    let lowTaskQueue: Array<Task>;
    let idleTaskQueue: Array<Task>;
    let highTaskExecuteCount: int = 0;
    let mediumTaskExecuteCount: int = 0;

    const isTaskPoolUseLaunch: boolean = isUsingLaunch();
    const isTaskpoolSupportInterop: boolean = isSupportingInterop();

    const initialWorkersNumber: int = 2;
    const continuousExecutionCount: int = 5;

    const taskCountMutex = ConcurrencyHelpers.mutexCreate();
    let waitingTasksNum: int = 0;
    let totalExecCount: int = 0;
    let totalExecTime: int = 0;
    let managerWorker: TaskPoolWorker | undefined = undefined;
    let triggerInterval: int = 30000; // 30s is the default shrink interval.
    let idleThreshold: int = 60000; // If the worker is idle for more than 60s, it will be released.
    let workersLimit: int = max(4, getTaskPoolWorkersLimit()); // 4 is the minimum of workers limit.
    const STEP_SIZE: int = 2;
    const SHRINK_STEP: int = 4; // 4: try to release 4 threads every time.
    const TASK_DURATION: int = 100; // 100: 100ms.
    const WORKERS_MINIMUM: int = 1; // 1: minimum thread num when idle.
}

function launchPromise(f: Function, ...args: FixedArray<NullishType>): Promise<NullishType> {
    let resolver: (((v: NullishType | PromiseLike<NullishType>) => void) | undefined) = undefined;
    let rejecter: (((error: Error) => void) | undefined) = undefined;
    let p = new Promise<NullishType>((res, rej) => {
        resolver = res
        rejecter = rej;
    })
    let cb = () => {
        try {
            let v = f.unsafeCall(...args)
            resolver!(v)
        } catch (e: Error) {
            rejecter!(e)
        }
    }
    launch<void, () => void>(cb)
    return p;
}
