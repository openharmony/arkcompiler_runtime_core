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

import {launch} from "std/concurrency"

export namespace taskpool {
    const TASKPOOL_ERROR_NOT_IN_TASKPOOL: number = 10200022;
    const TASKPOOL_ERROR_CALLBACK_NOT_REGISTERED: number = 10200024;
    const TASKPOOL_ERROR_CIRCULAR_DEPENDENCY: number = 10200026;
    const TASKPOOL_ERROR_DEPENDENCY_NOT_EXIST: number = 10200027;
    const TASKPOOL_ERROR_DELAY_TIME_NEGATIVE: number = 10200028;
    const TASKPOOL_ERROR_LISTENER_REGISTRATION: number = 10200034;
    const TASKPOOL_ERROR_EXECUTED_TASK_ADD_DEPENDENCY: number = 10200070;
    const TASKPOOL_ERROR_EXECUTED_TASK_REMOVE_DEPENDENCY: number = 10200071;
    const TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE: number = 10200072;
    const TASKPOOL_ERROR_GROUP_TASK_ADD_DEPENDENCY: number = 10200073;
    const TASKPOOL_ERROR_GROUP_TASK_RELIED_ON: number = 10200074;
    const TASKPOOL_ERROR_GROUP_TASK_EXECUTE_OUTSIDE: number = 10200075;
    const TASKPOOL_ERROR_GROUP_TASK_EXECUTE_DELAYED: number = 10200076;
    const TASKPOOL_ERROR_GROUP_TASK_ADD_GROUP: number = 10200077;
    const TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_EXECUTED: number = 10200078;
    const TASKPOOL_ERROR_SEQ_RUNNER_TASK_RELIED_ON: number = 10200079;
    const TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_OUTSIDE: number = 10200080;
    const TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_DELAYED: number = 10200081;
    const TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTED: number = 10200082;
    const TASKPOOL_ERROR_LONG_TASK_MULTIPLE_EXECUTIONS: number = 10200083;
    const TASKPOOL_ERROR_LONG_TASK_NOT_SUPPORTED: number = 10200084;
    const TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_BASIC: number = 10200085;
    const TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_PERIODIC: number = 10200086;
    const TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_LONG: number = 10200087;
    const TASKPOOL_ERROR_TASK_NOT_EXECUTED_TASK: number = 10200088;
    const TASKPOOL_ERROR_TASK_NOT_EXECUTED_GROUP: number = 10200089;
    const TASKPOOL_ERROR_DEPENDENT_TASK_EXECUTE_PERIODIC: number = 10200090;
    const TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED: number = 10200091;
    const TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_DELAYED: number = 10200092;
    const TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_AGAIN: number = 10200093;
    const TASKPOOL_ERROR_PERIODIC_TASK_NOT_SUPPORTED: number = 10200094;
    const TASKPOOL_ERROR_PERIODIC_TASK_SEQUENCE_RUNNER: number = 10200095;
    const TASKPOOL_ERROR_PERIOD_NEGATIVE: number = 10200096;
    const TASKPOOL_ERROR_NO_DEPENDENCY: number = 10200097;
    const TASKPOOL_ERROR_TASK_CANCELED: number = 10200098;
    const TASKPOOL_ERROR_GROUP_CANCELED: number = 10200099;
    const TASKPOOL_ERROR_SEQ_RUNNER_CANCELED: number = 10200100;
    const TASKPOOL_ERROR_NO_PARAMS: number = 10200101;
    const TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_GROUP: number = 10200102;
    const TASKPOOL_ERROR_GROUP_TASK_ADD_SEQ_RUNNER: number = 10200103;

    class TaskPoolWorker extends EAWorker {
        constructor(supportInterop: boolean = false) {
            super(supportInterop);
            this.run<void>((): void => {
                this.workerId = CoroutineExtras.getWorkerId();
            }).Await();
        }

        internal performTask(): boolean {
            let task: Task | undefined = getTask();
            if (task == undefined) {
                return false;
            }
            workerPendingTasksCount.set(this.workerId, workerPendingTasksCount.get(this.workerId)! + 1);
            this.currentPriority = task.priority;
            let job = this.run<NullishType>((): NullishType => {
                let res = task?.execute();
                return res;
            });
            const taskRunner = (value: NullishType): NullishType => {
                let res = job.Await();
                return res;
            }
            let p = Promise.resolve<NullishType>(new Object()).then<NullishType, NullishType>(taskRunner, taskRunner);
            p.then((res: NullishType) => {
				task?.resolve(res);
                this.notifyTaskFinished(task);
            }).catch((e) => {
                task?.reject(e);
                this.notifyTaskFinished(task);
            }).finally(() => {
                this.currentPriority = undefined;
                setTimeout(() => {
                    task?.notifyDependencies();
                    this.notifyTaskFinished(task);
                });
                ConcurrencyHelpers.lockGuard(workersMutex, () => {
                    workerPendingTasksCount.set(this.workerId, workerPendingTasksCount.get(this.workerId)! - 1);
                })
            })
            return true;
        }

        internal notifyTaskFinished(task: Task): void {
            task?.tryTriggerRunner();
            if (getTaskNum()) {
                notifyExecuteTask();
            }
        }

        internal workerId: int;
        internal currentPriority?: Priority;
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

    /**
     * @class Task provides an interface to create a task to execute in the taskpool
     */
    export class Task {
        /**
         * Create a Task instance
         *
         * @param name The name of task
         * @param func Concurrent function to execute in the taskpool
         */
        constructor(name: string, func: Function, ...args: FixedArray<NullishType>) {
            this.name = name;
            this.arguments = Array.from(args);
            this.taskFunction = (): NullishType => {
                return func.unsafeCall(...args);
            };
            this.totalDuration = 0;
            this.ioDuration = 0;
            this.cpuDuration = 0;
            this.taskMutex = ConcurrencyHelpers.mutexCreate();
            this.condVar = ConcurrencyHelpers.condVarCreate();
            this.dependentTasks = new Set<Task>();
            // Each task has unique identifier
            this.id = Task.generateId();
        }

        /**
         * Create a Task instance
         *
         * @param func Concurrent function to execute in the taskpool
         */
        constructor(func: Function, ...args: FixedArray<NullishType>) {
            this(func.name, func, ...args);
        }

        /**
         * Check current running task is canceled or not
         *
         * @returns true if current running task is canceled, false - otherwise
         */
        static isCanceled(): boolean {
            return isCancel();
        }

        /**
         * Register a callback and call it when the task is enqueued
         * 
         * @param callback Callback to be registered and executed when the task is enqueued
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onEnqueued(callback: CallbackFunction): void {
            this.throwIfCallbackCannotBeAdded();
            this.onEnqueueCallback = callback;
        }

        /**
         * Register a callback and call it before the task execution
         * 
         * @param callback Callback to be registered and executed before the task execution
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onStartExecution(callback: CallbackFunction): void {
            this.throwIfCallbackCannotBeAdded();
            this.onStartCallback = callback;
        }

        /**
         * Register a callback and call it when the task fails to execute
         * 
         * @param callback Callback to be registered and executed when the task fails to execute
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onExecutionFailed(callback: CallbackFunctionWithError): void {
            this.throwIfCallbackCannotBeAdded();
            this.onFailCallback = callback;
        }

        /**
         * Register a callback and call it when the task successfully executed
         * 
         * @param callback Callback to be registered and executed when the task successfully executed
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onExecutionSucceeded(callback: CallbackFunction): void {
            this.throwIfCallbackCannotBeAdded();
            this.onSuccessCallback = callback;
        }

        /**
         * Add dependencies on the task array for this task
         * @param tasks An array of dependent tasks
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
            if (this.isSubmitted || this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_ADD_DEPENDENCY, "taskpool:: seqRunnerTask or executedTask cannot addDependency");
            }
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_DEPENDENCY, "taskpool:: groupTask cannot addDependency");
            }
            for (const task: Task of tasks) {
                if (task.isSubmitted || task.seqId != 0) {
                    throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_RELIED_ON, "taskpool:: seqRunnerTask or executedTask cannot be relied on");
                }
                if (task.groupId != 0) {
                    throw new Error(TASKPOOL_ERROR_GROUP_TASK_RELIED_ON, "taskpool:: groupTask cannot be relied on");
                }
                if (this.id == task.id || Task.hasTaskDFS(this, task)) {
                    throw new Error(TASKPOOL_ERROR_CIRCULAR_DEPENDENCY, "taskpool:: There is a circular dependency");
                }
            }
            for (const task: Task of tasks) {
                if (task.dependentTasks.has(this)) {
                    continue;
                }
                task.dependentTasks.add(this);
                this.taskDependenciesCount++;
            }
            this.isDependent = (this.taskDependenciesCount != 0);
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
            if (!this.isDependent) {
                throw new Error(TASKPOOL_ERROR_NO_DEPENDENCY, "taskpool:: task has no dependency");
            } 
            if (this.isSubmitted) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_REMOVE_DEPENDENCY, "taskpool:: executedTask cannot removeDependency");
            }
            for (const task: Task of tasks) {
                if (task.dependentTasks.size == 0 && !task.isDependent) {
                    throw new Error(TASKPOOL_ERROR_NO_DEPENDENCY, "taskpool:: task has no dependency");
                }
                if (task.isSubmitted) {
                    throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_REMOVE_DEPENDENCY, "taskpool:: cannot removeDependency on a dependent and executed task");
                }
                if (!task.dependentTasks.has(this)) {
                    throw new Error(TASKPOOL_ERROR_DEPENDENCY_NOT_EXIST, "taskpool:: The dependency does not exist");
                }
            }
            for (let task: Task of tasks) {
                task.dependentTasks.delete(this);
                this.taskDependenciesCount--;
            }
            this.isDependent = (this.taskDependenciesCount != 0);
        }

        /**
         * @returns true if the task has been completed, false - otherwise
         */
        isDone(): boolean {
            return this.isSubmitted && !hasTask(this);
        }

        /**
         * Send data to host side and trigger the registered callback
         * @param args arguments for the registered callback
         * @throws Error if the function is not called from a taskpool task
         * @throws Error if the callback is not registered
         */
        static sendData(...args: FixedArray<NullishType>): void {
            const item: Task | undefined = getCurrentTask();
            if (item === undefined) {
                throw new Error(TASKPOOL_ERROR_NOT_IN_TASKPOOL, "taskpool:: The function is not called in the TaskPool thread");
            }
            const task: Task = item;
            if (task.onReceiveCallback === undefined) {
                throw new Error(TASKPOOL_ERROR_CALLBACK_NOT_REGISTERED, "taskpool:: The callback is not registered on the host side");
            }
            ConcurrencyHelpers.mutexLock(task.dataMutex);
            task.argsArray.push(args);
            ConcurrencyHelpers.mutexUnlock(task.dataMutex);
        }

        /**
         * Register a callback for this task to receive and handle data from the taskpool task 
         */
        onReceiveData(callback?: Function): void {
            this.onReceiveCallback = callback;
        }

        /// Concurrent function to execute in taskpool
        taskFunction: () => NullishType;

        /// The concurrent function arguments
        arguments?: NullishType[];

        /// Task name
        name: string;

        /// Total duration of task execution
        totalDuration: number;

        /// IO duration of task execution
        ioDuration: number;

        /// CPU duration of task execution
        cpuDuration: number;

        /// ----- Internal implementation part -----

        /**
         * Check that the task can be executed as common task
         * @throws Error if the task can not be executed as common task 
         * @see taskpool.execute
         */
        internal checkExecution(): void {
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_EXECUTE_OUTSIDE, "taskpool:: groupTask cannot execute outside");
            } else if (this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_OUTSIDE, "taskpool:: seqRunnerTask cannot execute outside");
            } else if (this.isSubmitted && (this.isDependent || this.dependentTasks.size != 0)) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE, "taskpool:: executedTask with dependency cannot execute again");
            }
        }

        /**
         * Check that the task can be executed as delayed task
         * @throws Error if the task can not be executed as delayed task
         * @see taskpool.executeDelayed
         */
        internal checkExecutionForDelayedTask(delayTime: number): void {
            if (delayTime < 0) {
                throw new Error(TASKPOOL_ERROR_DELAY_TIME_NEGATIVE, "taskpool:: The delayTime is less than zero");
            }
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_EXECUTE_DELAYED, "taskpool:: groupTask cannot executeDelayed outside");
            }
            if (this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_DELAYED, "taskpool:: seqRunnerTask cannot executeDelayed outside");
            }
            if (this.isSubmitted && (this.isDependent || this.dependentTasks.size != 0)) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE, "taskpool:: executedTask with dependency cannot execute again");
            }
            if (this.isSubmitted && (this instanceof LongTask)) {
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
        internal checkExecutionForPeriodTask(period: number): void {
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

        internal notifyDependencies(): void {
            for (const dependentTask: Task of this.dependentTasks) {
                ConcurrencyHelpers.lockGuard(dependentTask.taskMutex, () => {
                    dependentTask.taskDependenciesCount--;
                    ConcurrencyHelpers.condVarNotifyOne(dependentTask.condVar, dependentTask.taskMutex);
                });
            }
            ConcurrencyHelpers.mutexLock(this.taskMutex);
            this.isRunning = false;
            ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.taskMutex);
            ConcurrencyHelpers.mutexUnlock(this.taskMutex);
        }

        internal getDuration(): number {
            return (Chrono.nanoNow() - this.startTime) / Chrono.NS_PER_MS;
        }

        internal execute(): NullishType {
            initWorkerPool();
            this.waitForDependencies();
            if (!taskStarted(this)) {
                this.notifyDependencies();
                this.throwCancelTaskError();
            }
            this.callUserCallback(this.onStartCallback);
            let result: NullishType = null;
            this.startTime = Chrono.nanoNow();
            const startCpuTime: long = Chrono.getCpuTime();
            try {
                result = this.taskFunction();
                this.callUserCallback(this.onSuccessCallback);
            } catch (e: Error) {
                try {
                    this.onFailCallback?.(e);
                } catch (callbackError) {}
                throw e;
            } finally {
                // We cannot reliably calculate ioDuration in 1.2 due to possible coroutine switches.
                // And we should set ioDuration = 0, totalDuration = cpuDuration here
                this.cpuDuration = (Chrono.getCpuTime() - startCpuTime) / Chrono.NS_PER_MS;
                this.totalDuration = this.cpuDuration;
                this.ioDuration = 0;
                // Process the task finishing
                const notCanceled: boolean = taskFinished(this);
                if(isTaskPoolUseLaunch) {
                    this.notifyDependencies();
                }
                if (!notCanceled) {
                    this.throwCancelTaskError();
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
            throw new Error(TASKPOOL_ERROR_TASK_CANCELED, "taskpool:: task has been canceled");
        }

        private throwIfCallbackCannotBeAdded() {
            if (this.isSubmitted) {
                throw new Error(TASKPOOL_ERROR_LISTENER_REGISTRATION, "taskpool:: The executed task does not support the registration of listeners.");
            }
        }

        private callUserCallback(callback: CallbackFunction | undefined): void {
            try {
                callback?.();
            } catch (e) {}
        }

        /// The task is added to taskpool for execution
        internal enqueue(priority?: Priority): void {
            priority = priority == undefined ? Priority.MEDIUM : priority;
            this.priority = priority;
            if (!isTaskPoolUseLaunch) {
                ConcurrencyHelpers.mutexLock(taskQueuesMutex);
                initTaskQueues();
                taskQueues?.get(priority)?.push(this);
                ConcurrencyHelpers.mutexUnlock(taskQueuesMutex);
            }

            this.callUserCallback(this.onEnqueueCallback);
        }

        /**
         * @see taskpool.execute
         * @see taskpool.SequenceRunner.execute
         */
        internal getOnReceivePromise(p: Promise<NullishType>): Promise<NullishType> {
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

        /**
         * Generate new unique identifier for a new task
         * @see Task.constructor
         */
        private static native generateId(): long;

        /**
         * @see taskpool.Task.addDependency
         */
        internal static hasTaskDFS(startTask: Task, targetTask: Task): boolean {
            for (const dependentTask: Task of startTask.dependentTasks) {
                if (dependentTask.id == targetTask.id) {
                    return true;
                }
                if (Task.hasTaskDFS(dependentTask, targetTask)) {
                    return true;
                }
            }
            return false;
        }

        /// @see taskpool.Task.getOnReceivePromise
        internal extractDataArgs(): Array<FixedArray<NullishType>> {
            ConcurrencyHelpers.mutexLock(this.dataMutex);
            const argsArray: Array<FixedArray<NullishType>> = this.argsArray;
            this.argsArray = new Array<FixedArray<NullishType>>();
            ConcurrencyHelpers.mutexUnlock(this.dataMutex);
            return argsArray;
        }

        /// trigger next runner task to execute
        internal tryTriggerRunner(): void {
            if (this.seqId != 0) {
                let nextTask: Task | undefined = undefined;
                if (this.runner.tasks.length == 0) {
                    this.runner.isRunning = false;
                } else {
                    nextTask = this.runner.tasks.shift()
                }
                nextTask?.enqueue(this.runner.priority);
            }
        }

        /// Unique identifier of the task
        internal readonly id: long;
        /// id of group which contains the task, 0 means the task is not part of any group
        internal groupId: long = 0;
        /// id of sequence runner which ran the task, 0 means task was not ran via any sequence runner
        internal seqId: long = 0;
        /// the task has been submitted into taskpool
        internal isSubmitted: boolean = false;
        /// the task depends on another task
        internal isDependent: boolean = false;
        /// Count of tasks which should be executed before this task
        internal taskDependenciesCount: int = 0;
        /// Set of tasks which can be executed after this task
        internal dependentTasks: Set<Task>;
        /// identifier of worker which starts to execute the task
        internal workerId: int = -1;
        /// an instance of the task is running
        private isRunning: boolean = false;
        /// start time of the task
        private startTime: long = 0;
        /// the task has been executed periodically
        internal isPeriodicTask: boolean = false;
        /// identifier of task which is executeDelayed or executePeriodically
        internal timerId: number = -1.0;

        // NOTE(ipetrov, #19949): Change to private when bug in frontend will be fixed
        internal taskMutex: Object;
        internal condVar: Object;

        internal resolve: (value: NullishType) => void;
        internal reject: (e: Error) => void;

        internal priority: Priority;

        private runner: SequenceRunner;

        private onReceiveCallback?: Function;
        private dataMutex: Object = ConcurrencyHelpers.mutexCreate();
        private argsArray: Array<FixedArray<NullishType>> = new Array<FixedArray<NullishType>>();

        private onEnqueueCallback?: CallbackFunction;
        private onStartCallback?: CallbackFunction;
        private onFailCallback?: CallbackFunctionWithError;
        private onSuccessCallback?: CallbackFunction;
    }

    /**
     * @class LongTask provides an interface to create a long executing task.
     * The such task can be executed only once
     * @extends Task
     */
    export class LongTask extends Task {
        /**
         * Create a LongTask instance
         * @param name The name of long task
         * @param func Concurrent function to execute in the taskpool
         */
        constructor(name: string, func: Function, ...args: FixedArray<NullishType>) {
            super(name, func, ...args);
        }

        /**
         * Create a LongTask instance
         * @param func Concurrent function to execute in the taskpool
         */
        constructor(func: Function, ...args: FixedArray<NullishType>) {
            super("", func, ...args);
        }

        internal override checkExecution(): void {
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_EXECUTE_OUTSIDE, "taskpool:: groupTask cannot execute outside");
            } else if (this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_OUTSIDE, "taskpool:: seqRunnerTask cannot execute outside");
            } else if (this.isSubmitted && (this.isDependent || this.dependentTasks.size != 0)) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE, "taskpool:: executedTask with dependency cannot execute again");
            } else if (this.isSubmitted) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_LONG, "taskpool:: The long task can only be executed once");
            } else if (this.isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_AGAIN, "taskpool:: the periodicTask cannot execute again");
            }
        }
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
         * @param name The name of TaskGroup
         */
        constructor(name: string) {
            this.name = name;
            this.id = TaskGroup.generateGroupId();
            this.tasks = new Array<Task>();
        }

        /**
         * Add a concurrent function into task group
         *
         * @param func the concurrent function to add to the task group
         */
        addTask(func: Function, ...args: FixedArray<NullishType>): void {
            this.addTask(new Task(func, ...args));
        }

        /**
         * Add a task into task group
         *
         * @param task the task to add to the task group
         * @throws Error if
         *     - task was already added to any group
         *     - task is LongTask
         *     - task was executed outside the group
         *     - task executed via SequenceRunner
         */
        addTask(task: Task): void {
            this.checkExecution(task);
            task.groupId = this.id;
            this.tasks.push(task);
        }

        private checkExecution(task: Task): void {
            if (task.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_GROUP, "taskpool:: taskGroup cannot add groupTask");
            } else if (task instanceof LongTask) {
                throw new Error(TASKPOOL_ERROR_LONG_TASK_NOT_SUPPORTED, "taskpool:: The interface does not support the long task");
            } else if (task.isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_NOT_SUPPORTED, "taskpool:: The interface does not support the periodicTask");
            } else if (task.isSubmitted || task.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_SEQ_RUNNER, "taskpool:: taskGroup cannot add seqRunnerTask or executedTask");
            } else if (task.isDependent || task.dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED, "taskpool:: dependent task not allowed.");
            }
        }

        /**
         * Generate unique identifier for the group
         * @see TaskGroup.constructor
         * @return unique group identifier for a new Taskgroup instance 
         */
        private static native generateGroupId(): long;

        /**
         * TaskGroup name
         */
        name: string;

        internal readonly id: long;
        internal tasks: Array<Task>;
        internal isTaskAlreadySubmitted: boolean = false;
    }

    class SeqRunnerImpl {
        constructor(id: long) {
            this.id = id;
            this.seqPromise = Promise.resolve<NullishType>(new Object());
        }
        readonly id: long;
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
            this.priority = priority == undefined ? Priority.MEDIUM : priority;
            this.tasks = new Array<Task>();
        }

        execute(task: Task): Promise<NullishType> {
            initWorkerPool();
            this.checkExecution(task);
            task.seqId = this.impl.id;
            task.runner = this;
            taskSubmitted(task);
            task.isSubmitted = true;
            if (isTaskPoolUseLaunch) {
                task.enqueue(this.priority);
                const taskRunner = (value: NullishType): NullishType => {
                    return task.execute();
                };
                this.impl.seqPromise = task.getOnReceivePromise(this.impl.seqPromise.then<NullishType>(taskRunner, (error) => {
                    taskRunner(undefined);
                    return new Promise<never>((resolve, reject) => {});
                } ));
                return this.impl.seqPromise;
            }

            const promise = new Promise<NullishType>((resolve, reject) => {
                task.resolve = resolve;
                task.reject = reject;
            });
            if (this.isRunning) {
                this.tasks.push(task);
            } else {
                this.isRunning = true;
                task.enqueue(this.priority);
            }
            notifyExecuteTask();
            return task.getOnReceivePromise(promise);
        }

        private static native generateSeqRunnerId(): long;

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
            if (task.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_GROUP, "taskpool:: sequenceRunner cannot execute groupTask");
            } else if (task.isSubmitted || task.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_EXECUTED, "taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask");
            } else if (task.isDependent || task.dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED, "taskpool:: sequenceRunner dependent task not allowed.");
            } else if (task.isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_SEQUENCE_RUNNER, "taskpool:: sequenceRunner cannot execute the periodicTask");
            } else if (task.isSubmitted) {
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

    /**
     * @class represents internal information about task in taskpool
     */
    export class TaskInfo {
        /// Unique identifier of task
        taskId: number = 0;

        /// Task state in taskpool
        state: State = State.WAITING;

        /// Duration of task exeuction
        duration?: number;

        /// Task name
        name: string = "";
    }

    /**
     * @class represents internal information about worker thread with taskpool tasks
     */
    export class ThreadInfo {
        /// Worker thread id
        tid: number = -1.0;

        /// Running task identifiers list on current worker thread
        taskIds?: number[];

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

    type CoroutineId = int;
    type WorkerId = int;

    // NOTE(ipetrov, #16281): Temporary solution, make as namespace when namespaces will be supported

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
     * @param task The task for executing
     * @returns Promise for result of executed task
     */
    export function execute(task: Task, priority?: Priority): Promise<NullishType> {
        initWorkerPool();
        task.checkExecution();
        taskSubmitted(task);
        task.isSubmitted = true;
        task.enqueue(priority);

        if (isTaskPoolUseLaunch) {
            return task.getOnReceivePromise(launchPromise(task.execute));
        }

        const promise = new Promise<NullishType>((resolve, reject) => {
            task.resolve = resolve;
            task.reject = reject;
        });
        notifyExecuteTask();
        return task.getOnReceivePromise(promise);
    }

    /**
     * Execute a concurrent task group
     *
     * @param group The task group for execution
     * @returns Promise for array of results of executed tasks from the group
     */
    export function execute(group: TaskGroup, priority?: Priority): Promise<Array<NullishType>> {
        initWorkerPool();
        const tasksCount = group.tasks.length.toInt();
        if (tasksCount == 0) {
            return Promise.resolve<Array<NullishType>>(new Array<NullishType>());
        }
        taskGroupSubmitted(group.id, tasksCount);
        group.isTaskAlreadySubmitted = true;
        const promises: Array<Promise<NullishType>> = new Array<Promise<NullishType>>();
        group.tasks.forEach((task: Task) => {
            taskSubmitted(task);
            task.isSubmitted = true;
            task.enqueue(priority);
            if (isTaskPoolUseLaunch) {
                promises.push(task.getOnReceivePromise(launchPromise(task.execute)));
            } else {
                const promise = new Promise<NullishType>((resolve, reject) => {
                    task.resolve = resolve;
                    task.reject = reject;
                });
                notifyExecuteTask();
                promises.push(task.getOnReceivePromise(promise));
            }
        });
        return Promise.all<NullishType>(promises);
    }

    export function executeDelayed(delayTime: number, task: Task, priority?: Priority): Promise<NullishType> {
        initWorkerPool();
        task.checkExecutionForDelayedTask(delayTime);
        task.isSubmitted = true;
        ConcurrencyHelpers.lockGuard(mutex, () => {
            pendingDelayed.add(task);
        });
        return new Promise<NullishType>((resolve, reject) => {
            task.timerId = setTimeout(() => {
                ConcurrencyHelpers.lockGuard(mutex, () => {
                    pendingDelayed.delete(task);
                });
                if (task.timerId == -1.0 || isCanceled(task)) {
                    ConcurrencyHelpers.lockGuard(mutex, () => {
                        pendingDelayedCanceled.delete(task);
                    });
                    reject(new Error("taskpool:: task has been canceled"));
                }
                taskSubmitted(task);
                task.enqueue(priority);
                if (isTaskPoolUseLaunch) {
                    launchImpl(task, priority).then((res: NullishType) => {
                        resolve(res);
                    }).catch((e) => {
                        reject(e);
                    })
                } else {
                    const p = new Promise<NullishType>((resolve, reject) => {
                        task.resolve = resolve;
                        task.reject = reject;
                    })
                    notifyExecuteTask();
                    task.getOnReceivePromise(p).then((res: NullishType) => {
                        resolve(res);
                    }).catch((e) => {
                        reject(e);
                    })
                }
            }, delayTime);
        });
    }

    export function executePeriodically(period: number, task: Task, priority?: Priority): void {
        initWorkerPool();
        task.checkExecutionForPeriodTask(period);
        task.isSubmitted = true;
        task.isPeriodicTask = true;
        task.timerId = setInterval(() => {
            if (task.timerId == -1.0) {
                return;
            }
            taskSubmitted(task);
            task.enqueue(priority);
            if (isTaskPoolUseLaunch) {
                launchImpl(task, priority);
            } else {
                const p = new Promise<NullishType>((resolve, reject) => {
                    task.resolve = resolve;
                    task.reject = reject;
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
            let timerId = -1.0;
            if (task.timerId !== -1.0) {
                timerId = task.timerId;
                if (task.isPeriodicTask) {
                    clearInterval(task.timerId);
                }
                task.timerId = -1.0;
            }
            if (pendingDelayed.has(task)) {
                pendingDelayedCanceled.add(task);
                return;
            }
            if (waitingTasks.has(task) || runningTasks.has(task)) {
                tasksToBeCanceled.add(task);
                return;
            }
            if (timerId != -1.0) {
                return;
            }
            if (task.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTED, "taskpool:: sequenceRunner task has been executed");
            } else {
                throw new Error(TASKPOOL_ERROR_TASK_NOT_EXECUTED_TASK, "taskpool:: task is not executed or has been executed");
            }
        });
    }

    /**
     * Cancel a concurrent task group
     *
     * @param group The group for canceling
     * @throws Error if the task group does not exist when it is canceled
     */
    export function cancel(group: TaskGroup): void {
        ConcurrencyHelpers.lockGuard(mutex, () => {
            if (waitingGroupTasks.has(group.id) || runningGroupTasks.has(group.id)) {
                groupsToBeCanceled.add(group.id);
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
     * @returns taskpool internal information about tasks and threads
     */
    export function getTaskPoolInfo(): TaskPoolInfo {
        initWorkerPool();
        const taskInfos: Array<TaskInfo> = new Array<TaskInfo>();
        const threadInfos: Array<ThreadInfo> = new Array<ThreadInfo>();
        ConcurrencyHelpers.mutexLock(mutex);
        // Collect running and canceled tasks info
        runningTasks.forEach((count: int, task: Task) => {
            const taskInfo: TaskInfo = new TaskInfo();
            taskInfo.taskId = task.id;
            taskInfo.name = task.name;
            taskInfo.state =  (isCanceled(task)) ? State.CANCELED : State.RUNNING;
            taskInfo.duration = task.getDuration();
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
            taskInfo.taskId = task.id;
            taskInfo.name = task.name;
            taskInfo.state =  (isCanceled(task)) ? State.CANCELED : State.WAITING;
            taskInfos.push(taskInfo);
        });
        // Collect delay and canceled tasks info
        pendingDelayed.forEach((task: Task) => {
            const taskInfo: TaskInfo = new TaskInfo();
            taskInfo.taskId = task.id;
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
                    const taskIds: number[] = new number[tasksCount];
                    let currentIndex: int = 0;
                    tasks.forEach((task: Task) => {
                        taskIds[currentIndex] = task.id;
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
                const taskIds: number[] = new number[tasksCount];
                let currentIndex: int = 0;
                tasks.forEach((task: Task) => {
                    taskIds[currentIndex] = task.id;
                    currentIndex++;
                    // priority of running tasks can be regard as the priority of current thread's priority
                    // current only when task has been executed the worker can execute other task
                    if (runningTasks.has(task) && threadInfo.priority == undefined) {
                        threadInfo.priority = task.priority;
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
    function taskGroupSubmitted(groupId: long, tasksCount: int): void {
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
            if (task.groupId != 0) {
                // If the task is group task then decrement count of waiting task for the group
                cancelByGroup = popFromMap(task.groupId, waitingGroupTasks, runningGroupTasks, groupsToBeCanceled);
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
        if (task.groupId != 0) {
            const runningTasksOfTheGroup: int = runningGroupTasks.get(task.groupId, /* default */ 0);
            runningGroupTasks.set(task.groupId, runningTasksOfTheGroup + 1);
        }
        // NOTE(ipetrov, #20208): CoroutineExtras is debug functionality, maybe need to create a separate intrinsic
        executingTasks.set(CoroutineExtras.getCoroutineId(), task);
        const currentWorkerId: int = CoroutineExtras.getWorkerId();
        task.workerId = currentWorkerId;
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
            // NOTE(ipetrov, #20208): CoroutineExtras is debug functionality, maybe need to create a separate intrinsic
            executingTasks.delete(CoroutineExtras.getCoroutineId());
            workerToTasks.get(task.workerId)!.delete(task);
            const cancelByTask: boolean = popFromMap(task, runningTasks, waitingTasks, tasksToBeCanceled);
            let cancelByGroup: boolean = false;
            if (task.groupId != 0) {
                cancelByGroup = popFromMap(task.groupId, runningGroupTasks, waitingGroupTasks, groupsToBeCanceled);
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
        return pendingDelayedCanceled.has(task) || tasksToBeCanceled.has(task) || groupsToBeCanceled.has(task.groupId);
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
        for (let w : TaskPoolWorker of workers) {
            w.join();
        }
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            isWorkersInitialized.set(false);
            workers.clear();
        })
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
     */
    function initWorkerPool(): void {
        if (isWorkersInitialized.get() || isTaskPoolUseLaunch) {
                return;
        }
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            if (isWorkersInitialized.get() || isTaskPoolUseLaunch) {
                return;
            }
            for (let i = 0; i < initialWorkersNumber; ++i) {
                let w = new TaskPoolWorker(isTaskpoolSupportInterop);
                workers.add(w);
            }
            isWorkersInitialized.set(true);
        })
    }

    /**
     * @brief Notify idle workers to perform tasks.
     * @see taskpool.execute
     * @see taskpool.executeDelayed
     * @see taskpool.executePeriodically
     * @see taskpool.SequenceRunner.execute
     */
    function notifyExecuteTask(): void {
        initWorkerPool();
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            let w = chooseWorkerForTask();
            if (!w.performTask() && getTaskNum() > 0) {
                // Current getTask may get an undefined task when higher priority task hit the continueCount
                // So here we try to performTask again to avoid task cannot be executed
                w.performTask();
            }
        })
    }

    /**
     * @brief Select an optimal worker for task execution based on current pending tasks count.
     * @see taskpool.notifyExecuteTask
     */
    function chooseWorkerForTask(): TaskPoolWorker {
        let minTaskCount  = Int.MAX_VALUE;
        let selectedWorker = Array.from(workers)[0];
        for (let w : TaskPoolWorker of workers) {
            let currentWorkerTaskCount = workerPendingTasksCount.get(w.workerId);
            if(!currentWorkerTaskCount) {
                workerPendingTasksCount.set(w.workerId, 0);
                return w;
            }
            if(currentWorkerTaskCount < minTaskCount ) {
                minTaskCount  = currentWorkerTaskCount;
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
            if (task?.taskDependenciesCount == 0) {
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
            task.execute
        );
        let promise = new Promise<NullishType>((resolve, reject) => {
            try {
                let res = job.Await();
                resolve(res);
            } catch(e: Error) {
                reject(e);
            }
        });
        return task.getOnReceivePromise(promise);
    }

    native function isUsingLaunch(): boolean;
    native function isSupportingInterop(): boolean;

    // managed mutex for taskpool containers below
    const mutex = ConcurrencyHelpers.mutexCreate();
    // value is count of tasks
    const waitingTasks: Map<Task, int> = new Map<Task, int>();
    const runningTasks: Map<Task, int> = new Map<Task, int>();
    const pendingDelayed: Set<Task> = new Set<Task>();
    const pendingDelayedCanceled: Set<Task> = new Set<Task>();
    const tasksToBeCanceled: Set<Task> = new Set<Task>();
    // key is group id, value is count of associated tasks with this group
    const waitingGroupTasks: Map<long, int> = new Map<long, int>();
    const runningGroupTasks: Map<long, int> = new Map<long, int>();
    const groupsToBeCanceled: Set<long> = new Set<long>();
    // value is executing task on the associated coroutine
    const executingTasks: Map<CoroutineId, Task> = new Map<CoroutineId, Task>();
    // value is current executing task on the associated worker
    const currentExecutingTasks: Map<WorkerId, Task> = new Map<WorkerId, Task>();
    // value is set of executing tasks on this worker
    const workerToTasks: Map<WorkerId, Set<Task>> = new Map<WorkerId, Set<Task>>();
    // Count of pending tasks per worker
    const workerPendingTasksCount: Map<WorkerId, Int> = new Map<WorkerId, Int>();

    const workersMutex = ConcurrencyHelpers.mutexCreate();
    const workers: Set<TaskPoolWorker> = new Set<TaskPoolWorker>();
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
