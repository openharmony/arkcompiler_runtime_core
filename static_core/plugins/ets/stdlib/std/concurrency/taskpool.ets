/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

package std.concurrency;

import { PromiseWithResolvers, WorkerPriority } from "std/core";
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
    const TASKPOOL_ERROR_PERIODIC_TASK_ADD_DEPENDENCY: int = 10200113;
    const TASKPOOL_ERROR_SEQ_RUNNER_CHANGE_PRIORITY: int = 10200114;
    const TASKPOOL_ERROR_TASK_EXECUTE_TIMEOUT: int = 10200115;
    const TASKPOOL_ERROR_TIMEOUT_TASK_CANNOT_EXECUTED: int = 10200116;
    const TASKPOOL_ERROR_ASYNC_RUNNER_TASK_EXECUTE_OUTSIDE: int = 10200117;

    function materializeSendDataArgs(args: FixedArray<Any>): Array<Any> {
        const payload = new Array<Any>();
        for (let i = 0; i < args.length; i++) {
            payload.push(args[i]);
        }
        return payload;
    }

    class ReceiveListenerBinding {
        constructor(callback: Function, hostWorkerId: int) {
            this.callback = callback;
            this.hostWorkerId = hostWorkerId;
        }

        readonly callback: Function;
        readonly hostWorkerId: int;
    }

    class ReceiveEvent {
        constructor(payload: Array<Any>, listener: ReceiveListenerBinding) {
            this.payload = payload;
            this.listener = listener;
        }

        readonly payload: Array<Any>;
        readonly listener: ReceiveListenerBinding;
    }

    class ReceiveDispatcher {
        constructor(onProgress: () => void) {
            this.onProgress = onProgress;
        }

        enqueue(event: ReceiveEvent): boolean {
            const shouldSchedule = this.pendingCount.fetchAdd(1) == 0;
            try {
                ConcurrencyHelpers.lockGuard(this.queueMutex, () => {
                    this.queue.push(event);
                });
            } catch (e) {
                this.complete();
                throw e;
            }
            return shouldSchedule;
        }

        poll(): ReceiveEvent | undefined {
            let event: ReceiveEvent | undefined = undefined;
            ConcurrencyHelpers.lockGuard(this.queueMutex, () => {
                event = this.queue.shift();
            });
            return event;
        }

        prepend(event: ReceiveEvent): void {
            ConcurrencyHelpers.lockGuard(this.queueMutex, () => {
                this.queue.unshift(event);
            });
        }

        complete(): void {
            const previous = this.pendingCount.fetchSub(1);
            if (previous <= 0) {
                this.pendingCount.store(0);
            }
            this.onProgress();
        }

        hasPendingAcceptedEvents(): boolean {
            return this.pendingCount.load() != 0;
        }

        private readonly onProgress: () => void;
        private readonly queueMutex: Object = ConcurrencyHelpers.mutexCreate();
        private readonly queue: Array<ReceiveEvent> = new Array<ReceiveEvent>();
        private readonly pendingCount: AtomicInt = new AtomicInt(0);
    }

    const EXECUTION_RECORD_RESULT_READY: int = 1 << 0;
    const EXECUTION_RECORD_RESULT_IS_ERROR: int = 1 << 1;
    const EXECUTION_RECORD_FINALIZED: int = 1 << 2;
    const EXECUTION_RECORD_DISPATCH_CLOSED: int = 1 << 3;
    const EXECUTION_RECORD_NON_ERROR_MASK: int =
        EXECUTION_RECORD_RESULT_READY | EXECUTION_RECORD_FINALIZED | EXECUTION_RECORD_DISPATCH_CLOSED;

    class ExecutionRecord {
        constructor(submissionId: int, onFinalized?: () => void) {
            this.submissionId = submissionId;
            this.resultPromise = new PromiseWithResolvers<Any>();
            this.promise = this.resultPromise.promise;
            this.receiveDispatcher = new ReceiveDispatcher(() => this.tryFinalize());
            this.onFinalized = onFinalized;
        }

        resolvePromise(value: Any): void {
            this.resultPromise.resolve(value);
        }

        rejectPromise(error: Error): void {
            this.resultPromise.reject(error);
        }

        markTaskResult(value: Any): void {
            this.resultSlot.store(value);
            while (true) {
                const currentState = this.stateFlags.load();
                const nextState = (currentState & EXECUTION_RECORD_NON_ERROR_MASK) | EXECUTION_RECORD_RESULT_READY;
                if (this.stateFlags.compareAndSwap(currentState, nextState) == currentState) {
                    break;
                }
            }
            this.tryFinalize();
        }

        markTaskError(error: Error): void {
            this.resultSlot.store(error);
            while (true) {
                const currentState = this.stateFlags.load();
                const nextState = currentState | EXECUTION_RECORD_RESULT_READY | EXECUTION_RECORD_RESULT_IS_ERROR;
                if (this.stateFlags.compareAndSwap(currentState, nextState) == currentState) {
                    break;
                }
            }
            this.tryFinalize();
        }

        markTimedOut(): void {
            this.closeDispatchToHost();
        }

        markCanceled(): void {
            this.closeDispatchToHost();
        }

        canDispatchToHost(): boolean {
            return (this.stateFlags.load() & EXECUTION_RECORD_DISPATCH_CLOSED) == 0;
        }

        tryFinalize(): void {
            while (true) {
                const currentState = this.stateFlags.load();
                if ((currentState & EXECUTION_RECORD_FINALIZED) != 0 ||
                    (currentState & EXECUTION_RECORD_RESULT_READY) == 0 ||
                    this.receiveDispatcher.hasPendingAcceptedEvents()) {
                    return;
                }
                const nextState = currentState | EXECUTION_RECORD_FINALIZED;
                if (this.stateFlags.compareAndSwap(currentState, nextState) != currentState) {
                    continue;
                }
                const result = this.resultSlot.load();
                if ((currentState & EXECUTION_RECORD_RESULT_IS_ERROR) != 0) {
                    this.resultPromise.reject(result as Error);
                } else {
                    this.resultPromise.resolve(result);
                }
                try {
                    this.onFinalized?.();
                } catch (e) {
                    // Keep finalize path non-throwing.
                    console.error(`taskpool:: execution record finalize hook failed: ${e}`);
                }
                return;
            }
        }

        readonly submissionId: int;
        readonly promise: Promise<Any>;
        readonly receiveDispatcher: ReceiveDispatcher;

        private readonly resultPromise: PromiseWithResolvers<Any>;
        private readonly stateFlags: AtomicInt = new AtomicInt(0);
        private readonly resultSlot: AtomicReference<Any> = new AtomicReference<Any>(undefined);
        private readonly onFinalized?: () => void;

        private closeDispatchToHost(): void {
            this.stateFlags.fetchOr(EXECUTION_RECORD_DISPATCH_CLOSED);
        }
    }

    class QueuedExecution {
        constructor(task: Task, executionRecord: ExecutionRecord, priority: Priority) {
            this.task = task;
            this.executionRecord = executionRecord;
            this.priority = priority;
        }

        readonly task: Task;
        readonly executionRecord: ExecutionRecord;
        readonly priority: Priority;
    }

    function tryScheduleOnReceiveDrain(executionRecord: ExecutionRecord, hostWorkerId: int): void {
        const launchParams: LaunchParams = {
            abortFlag: false,
            workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.EXACT_ID, [hostWorkerId])
        };
        launch(launchParams) {
            drainOnReceiveEvents(executionRecord);
        };
    }

    function drainOnReceiveEvents(executionRecord: ExecutionRecord): void {
        const currentWorkerId = CoroutineExtras.getWorkerId();
        while (true) {
            const event = executionRecord.receiveDispatcher.poll();
            if (event === undefined) {
                if (executionRecord.receiveDispatcher.hasPendingAcceptedEvents()) {
                    Coroutine.Schedule();
                    continue;
                }
                executionRecord.tryFinalize();
                return;
            }
            if (event.listener.hostWorkerId != currentWorkerId) {
                executionRecord.receiveDispatcher.prepend(event);
                tryScheduleOnReceiveDrain(executionRecord, event.listener.hostWorkerId);
                return;
            }
            try {
                if (!executionRecord.canDispatchToHost()) {
                    continue;
                }
                event.listener.callback.unsafeCall(...event.payload);
            } catch (e) {
                console.error(`taskpool:: onReceiveData callback failed: ${e}`);
            } finally {
                executionRecord.receiveDispatcher.complete();
            }
        }
    }

    class GlobalQueueWorker extends EAWorker {
        public static closingTask: Task = new Task("closingTask", (): void => {});

        constructor (supportInterop: boolean) {
            super((() => this.workerBody()) as Object, "taskpool", supportInterop, true);
            initGlobalTaskQueues();
            this.needInterop = supportInterop;
            this.start();
            this.workerId = this.getWorkerId()!.toInt();
        }

        constructor () {
            super("managerWorker", false, true);
            this.start();
        }

        private workerBody() {
            while (this.isWorkerActive()) {
                if (this.tryAdvanceRetireRequest()) {
                    continue;
                }
                let queuedExecution = getTaskFromGlobalQueue(this); // Now handles blocking internally to avoid CPU busy waiting
                if (queuedExecution === undefined) {
                    continue;
                }
                const task = queuedExecution.task;
                removeFromIdleWorkers(this);

                const internalTask = InternalTask.of(task);
                const taskPriority = internalTask?.priority;
                if (taskPriority != undefined && this.currentPriority !== taskPriority) {
                    this.updateCurrentWorkerPriority(taskPriority);
                    this.currentPriority = taskPriority;
                }
                this.currentTasksCount.fetchAdd(1);
                ConcurrencyHelpers.lockGuard(this.currentTasksMutex, () => {
                    const curCount: int | undefined = this.currentTasks.get(task);
                    this.currentTasks.set(task, curCount == undefined ? 1 : curCount + 1);
                });
                let count = getNewTaskTotalCount();
                this.idleTime = 0;

                // Mark synchronous task body execution start.
                // Use an atomic millisecond tick to avoid extra locking in the managerWorker monitor.
                this.executingTaskBodyStartTime.store((Chrono.nanoNow() / Chrono.NS_PER_MS).toInt());

                try {
                    internalTask?.execute(this, queuedExecution.executionRecord);
                } catch (e) {
                    queuedExecution.executionRecord.markTaskError(e as Error);
                    internalTask!.finishExecutionRecord(queuedExecution.executionRecord);
                } finally {
                    // Clear synchronous task body execution mark.
                    this.executingTaskBodyStartTime.store(0);
                    this.currentTasksCount.fetchSub(1);
                    // this is to mock ArkTS-dyn behavior
                    EAWorker.postToMain<void>(() => {
                        setTimeout(() => {
                            internalTask!.notifyDependencies();
                        }, 1);
                    });
                    this.tryAdvanceRetireRequest();
                }
            }
        }

        closeWorker(): void {
            this.setWorkerActive(false);
            ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
                // notiy all to make sure the right one can be closed
                // other workers' idle time will update
                ConcurrencyHelpers.condVarNotifyAll(globalTaskCondVar, globalTaskQueuesMutex);
            });
            this.join();
        }

        private updateCurrentWorkerPriority(taskPriority: Priority): void {
            setCurrentTaskpoolWorkerPriority(GlobalQueueWorker.getWorkerPriority(taskPriority).valueOf());
        }

        public isWorkerActive(): boolean {
            let res: boolean = true;
            ConcurrencyHelpers.lockGuard(this.isAliveMutex, () => {
                res = this.isActive;
            });
            return res;
        }
        
        public setWorkerActive(active: boolean): void {
            ConcurrencyHelpers.lockGuard(this.isAliveMutex, () => {
                this.isActive = active;
            });
        }
        public notifyNewTask(): void {
            ConcurrencyHelpers.lockGuard(this.isAliveMutex, () => {
                ConcurrencyHelpers.condVarNotifyOne(this.newTaskCondVar, this.isAliveMutex);
            });
        }

        private tryRequestRetireForShrink(): boolean {
            if (this.retireRequested.get() || this.retireReady.get()) {
                return false;
            }
            if (!removeFromIdleWorkersLocked(this)) {
                return false;
            }
            this.retireRequested.set(true);
            return true;
        }

        public hasRetireRequested(): boolean {
            return this.retireRequested.get();
        }

        public isRetireReady(): boolean {
            return this.retireReady.get();
        }

        public tryAdvanceRetireRequest(): boolean {
            if (!this.retireRequested.get() || this.retireReady.get()) {
                return false;
            }
            if (this.hasVisibleGlobalTask() || this.hasLocalPendingWork()) {
                this.retireRequested.set(false);
                return false;
            }

            let canRetire: boolean = false;
            ConcurrencyHelpers.lockGuard(workersMutex, () => {
                let readyWorkersNum: int = 0;
                for (let w: GlobalQueueWorker of workers) {
                    if (w.isRetireReady()) {
                        readyWorkersNum++;
                    }
                }
                canRetire = workers.size - readyWorkersNum > WORKERS_MINIMUM;
                if (canRetire) {
                    this.retireReady.set(true);
                    this.retireRequested.set(false);
                } else {
                    this.retireRequested.set(false);
                }
            });
            if (!canRetire) {
                return false;
            }

            this.setWorkerActive(false);
            requestRetireCleanup();
            return true;
        }

        private hasLocalPendingWork(): boolean {
            let currentTasksSize: int = 0;
            ConcurrencyHelpers.lockGuard(this.currentTasksMutex, () => {
                currentTasksSize = this.currentTasks.size.toInt();
            });
            if (currentTasksSize != 0 || this.executingTaskBodyStartTime.load() != 0) {
                return true;
            }
            return currentWorkerHasPendingLocalJobs();
        }

        private hasVisibleGlobalTask(): boolean {
            return waitingTasksNum.load() != 0 || getNewTaskTotalCount() != 0;
        }

        // For managerWorker to expand.
        triggerExpand(): void {
            if (!this.tryMarkExpandJobPending()) {
                return;
            }
            let expandJob = this.run<Any>((): Any => {
                try {
                    // Take the queue snapshot before reading execution stats to avoid nested mutexes.
                    let queueSnap: int = 0;
                    ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
                        queueSnap = getNewTaskTotalCount();
                    });
                    let targetNum: int = 0;
                    ConcurrencyHelpers.lockGuard(taskCountMutex, () => {
                        // Use a queue snapshot for the expansion target. waitingTasksNum is updated across
                        // enqueue/dequeue races and can briefly include tasks that are already running.
                        if (totalExecCount.load() == 0) {
                            targetNum = min(queueSnap, STEP_SIZE);
                        } else {
                            const averageDuration: number = totalExecTime.load() / totalExecCount.load();
                            const result: number = Math.ceil((averageDuration * queueSnap) / TASK_DURATION);
                            targetNum = min(queueSnap, result.toInt());
                        }
                    });
                    if (targetNum == 0) {
                        return 0;
                    }
                    ConcurrencyHelpers.lockGuard(workersMutex, () => {
                        const workerCount: int = workers.size.toInt();
                        let runnableIdleWorkersNum: int = 0;
                        let runningWorkersNum: int = 0;
                        for (let w : GlobalQueueWorker of workers) {
                            if (w.hasRetireRequested() || w.isRetireReady()) {
                                continue;
                            }
                            if (w.currentTasksCount.load() == 0) {
                                runnableIdleWorkersNum++;
                            } else {
                                runningWorkersNum++;
                            }
                        }
                        const idleWorkersNum: int = runnableIdleWorkersNum;
                        
                        if (workerCount < workersLimit && idleWorkersNum < targetNum) {
                            // Cap by total-worker shortage. The target total is running workers plus the
                            // runnable backlog target, so stale waiting counters cannot over-expand a batch.
                            const targetTotalWorkers: int = min(workersLimit, targetNum + runningWorkersNum);
                            const desiredTotal: int = targetTotalWorkers - workerCount;
                            const available: int = workersLimit - workerCount;
                            const step: int = min(desiredTotal, available);
                            if (step > 0) {
                                for (let i = 0; i < step; ++i) {
                                    let w: GlobalQueueWorker = new GlobalQueueWorker(isTaskpoolSupportInterop);
                                    if (this.isAlive()) {
                                        workers.add(w);
                                    } else {
                                        w.closeWorker();
                                    }
                                }
                            }
                        }
                    });
                    // Align with native DispatchAndTryExpandInner: re-check expansion while tasks remain.
                    if (!isTaskPoolUseLaunch) {
                        let poolSize: int = 0;
                        ConcurrencyHelpers.lockGuard(workersMutex, () => {
                            poolSize = workers.size.toInt();
                        });
                        if (poolSize < workersLimit) {
                            let qRem: int = 0;
                            ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
                                qRem = getNewTaskTotalCount();
                            });
                            if (qRem > 0) {
                                this.scheduleExpandRetryIfNeeded();
                            }
                        }
                    }
                    return 1;
                } finally {
                    this.finishExpandJob();
                }
            });
        }

        private tryMarkExpandJobPending(): boolean {
            let shouldRun: boolean = false;
            ConcurrencyHelpers.lockGuard(this.expandJobMutex, () => {
                if (!this.expandJobPending) {
                    this.expandJobPending = true;
                    shouldRun = true;
                }
            });
            return shouldRun;
        }

        private finishExpandJob(): void {
            ConcurrencyHelpers.lockGuard(this.expandJobMutex, () => {
                this.expandJobPending = false;
            });
        }

        /// Debounced short retry so async expand does not miss one worker when races clear the queue snapshot.
        private scheduleExpandRetryIfNeeded(): void {
            ConcurrencyHelpers.lockGuard(this.expandJobMutex, () => {
                if (this.expandRetryTimerId != -1) {
                    return;
                }
                this.expandRetryTimerId = setTimeout((): void => {
                    ConcurrencyHelpers.lockGuard(this.expandJobMutex, () => {
                        this.expandRetryTimerId = -1;
                    });
                    this.triggerExpand();
                }, TRIGGER_EXPAND_RETRY_MS);
            });
        }

        // For managerWorker to shrink.
        private triggerShrink(): void {
            let shrinkJob = this.run<void>((): void => {
                this.timerId = setInterval(() => {
                    let runningWorkersNum: int = 0;
                    let retireRequests: int = 0;
                    ConcurrencyHelpers.lockGuard(workersMutex, () => {
                        this.cleanupReadyWorkersLocked();
                        for (let w: GlobalQueueWorker of workers) {
                            let currentTasksSize: int = 0;
                            ConcurrencyHelpers.lockGuard(w.currentTasksMutex, () => {
                                currentTasksSize = w.currentTasks.size.toInt();
                            });
                            if (currentTasksSize !== 0) {
                                runningWorkersNum++;
                            }
                        }
                    });
                    let targetNum: int = runningWorkersNum;
                    ConcurrencyHelpers.lockGuard(taskCountMutex, () => {
                        if (totalExecCount.load() != 0) {
                            let averageDuration: number = totalExecTime.load() / totalExecCount.load();
                            let result: number = (averageDuration * waitingTasksNum.load()) / TASK_DURATION;
                            targetNum += (min(waitingTasksNum.load(), result)).toInt();
                        } else {
                            targetNum += min(2, waitingTasksNum.load());
                        }
                    });
                    ConcurrencyHelpers.lockGuard(workersMutex, () => {
                        if (workers.size > WORKERS_MINIMUM && workers.size > targetNum) {
                            targetNum = max(WORKERS_MINIMUM, targetNum);
                        } else {
                            return;
                        }
                        let step = min(SHRINK_STEP, workers.size - targetNum);
                        let count: int = 0;
                        for (let worker: GlobalQueueWorker of idleWorkers) {
                            if (count >= step) {
                                break;
                            }
                            if (worker.idleTime == 0) {
                                continue;
                            }
                            if (Chrono.nanoNow() - worker.idleTime < idleThreshold) {
                                continue;
                            }
                            if (worker.tryRequestRetireForShrink()) {
                                count++;
                            }
                        }
                        retireRequests = count;
                    });
                    if (retireRequests > 0) {
                        notifyNewTaskAvailable();
                    }
                }, triggerInterval);
            });
        }

        // For managerWorker to expand when some workers are blocked for a long time.
        triggerBlockedExpandMonitor(): void {
            this.run<void>((): void => {
                this.blockedExpandTimerId = setInterval(() => {
                    const nowMsTick: int = (Chrono.nanoNow() / Chrono.NS_PER_MS).toInt();
                    const newlyBlockedWorkerIds: Array<int> = new Array<int>();

                    ConcurrencyHelpers.lockGuard(workersMutex, () => {
                        for (let w: GlobalQueueWorker of workers) {
                            let isBlocked: boolean = false;
                            const startMsTick: int = w.executingTaskBodyStartTime.load();
                            if (startMsTick != 0) {
                                const elapsedMs: int = nowMsTick - startMsTick;
                                // elapsedMs is computed on a 32-bit tick and is safe for small thresholds.
                                isBlocked = elapsedMs >= blockedWorkerThresholdMs;
                            }

                            if (isBlocked) {
                                if (!this.blockedExpandedWorkerIds.has(w.workerId)) {
                                    newlyBlockedWorkerIds.push(w.workerId);
                                }
                            } else {
                                if (this.blockedExpandedWorkerIds.has(w.workerId)) {
                                    this.blockedExpandedWorkerIds.delete(w.workerId);
                                }
                            }
                        }

                        if (newlyBlockedWorkerIds.length == 0) {
                            return;
                        }

                        // Degrade strategy: expand as many as possible within the limit.
                        const available: int = workersLimit - workers.size;
                        const expandCount: int = min(available, newlyBlockedWorkerIds.length.toInt());
                        if (expandCount <= 0) {
                            return;
                        }

                        for (let i: int = 0; i < expandCount; ++i) {
                            const blockedId: int = newlyBlockedWorkerIds[i];
                            let w: GlobalQueueWorker = new GlobalQueueWorker(isTaskpoolSupportInterop);
                            if (this.isAlive()) {
                                workers.add(w);
                                // Record only workers that actually triggered a special expansion in this tick.
                                this.blockedExpandedWorkerIds.add(blockedId);
                            } else {
                                w.closeWorker();
                            }
                        }
                    });
                }, blockedWorkerMonitorIntervalMs);
            });
        }

        // For managerWorker to retrigger shrink.
        retriggerShrink(): void {
            clearInterval(this.timerId);
            this.triggerShrink();
        }

        // For managerWorker to (re)start blocked-worker expansion monitor.
        retriggerBlockedExpandMonitor(): void {
            if (isTaskPoolUseLaunch) {
                return;
            }
            if (this.blockedExpandTimerId != -1) {
                clearInterval(this.blockedExpandTimerId);
                this.blockedExpandTimerId = -1;
            }
            this.blockedExpandedWorkerIds.clear();
            this.triggerBlockedExpandMonitor();
        }

        public cleanupReadyWorkers(): void {
            ConcurrencyHelpers.lockGuard(workersMutex, () => {
                this.cleanupReadyWorkersLocked();
            });
        }

        private cleanupReadyWorkersLocked(): void {
            let readyWorkers: Array<GlobalQueueWorker> = new Array<GlobalQueueWorker>();
            for (let worker: GlobalQueueWorker of workers) {
                if (worker.isRetireReady()) {
                    readyWorkers.push(worker);
                }
            }

            for (let worker: GlobalQueueWorker of readyWorkers) {
                removeFromIdleWorkersLocked(worker);
                workers.delete(worker);
                this.blockedExpandedWorkerIds.delete(worker.workerId);
                worker.retireReady.set(false);
                worker.idleTime = 0;
                worker.closeWorker();
            }
        }

        // For managerWorker to stop and release.
        stopManagerWorker(): void {
            clearInterval(this.timerId);
            if (this.blockedExpandTimerId != -1) {
                clearInterval(this.blockedExpandTimerId);
                this.blockedExpandTimerId = -1;
            }
            ConcurrencyHelpers.lockGuard(this.expandJobMutex, () => {
                if (this.expandRetryTimerId != -1) {
                    clearTimeout(this.expandRetryTimerId);
                    this.expandRetryTimerId = -1;
                }
            });
            this.finishExpandJob();
            this.join();
        }

        public isAliveMutex: Object = ConcurrencyHelpers.mutexCreate();
        public newTaskCondVar: Object = ConcurrencyHelpers.condVarCreate();

        private needInterop: boolean = false;
        public isActive: boolean = true;
        public workerId: int = -1;
        public currentPriority?: Priority = Priority.HIGH;
        public currentTasks: Map<Task, int> = new Map<Task, int>();
        public currentTasksMutex: Object = ConcurrencyHelpers.mutexCreate();
        public currentTasksCount: AtomicInt = new AtomicInt(0);
        public idleTime: long = 0;
        private retireRequested: AtomicFlag = new AtomicFlag(false);
        private retireReady: AtomicFlag = new AtomicFlag(false);

        // Start time of the current synchronous execution in this worker thread.
        // Stored as an atomic millisecond tick (lower 32-bit of ms-since-boot); 0 means not executing.
        private executingTaskBodyStartTime: AtomicInt = new AtomicInt(0);

        static workerCount: int = 0;
        private timerId: int;

        // managerWorker only: blocked-worker expansion monitor timer.
        private blockedExpandTimerId: int = -1;
        // managerWorker only: one-shot expand re-check (see native TRIGGER_EXPAND_INTERVAL).
        private expandRetryTimerId: int = -1;
        // managerWorker only: coalesce concurrent enqueue-triggered expansion jobs and guard expandRetryTimerId.
        private expandJobPending: boolean = false;
        private expandJobMutex: Object = ConcurrencyHelpers.mutexCreate();
        // managerWorker only: worker ids that already triggered a special expansion.
        private blockedExpandedWorkerIds: Set<int> = new Set<int>();
        private static getWorkerPriority(taskPriority: Priority): WorkerPriority {
            switch (taskPriority) {
                case Priority.USER_INTERACTION:
                    return WorkerPriority.PRIORITY_USER_INTERACTION;
                case Priority.DEADLINE_REQUEST:
                    return WorkerPriority.PRIORITY_DEADLINE_REQUEST;
                case Priority.HIGH:
                    return WorkerPriority.PRIORITY_HIGH;
                case Priority.MEDIUM:
                    return WorkerPriority.PRIORITY_DEFAULT;
                case Priority.LOW:
                    return WorkerPriority.PRIORITY_LOW;
                case Priority.IDLE:
                    return WorkerPriority.PRIORITY_IDLE;
            }
            return WorkerPriority.PRIORITY_DEFAULT;
        }
    }

    /**
     * @enum defines the task priority
     * @see taskpool.Task
     */
    export enum Priority {
        /// the task priority is user interaction
        USER_INTERACTION = -2,
        /// the task priority is deadline request
        DEADLINE_REQUEST = -1,
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

    enum InternalState {
        /// the task is not found in the global task pool
        NOT_FOUND = 0,

        /// the task state is waiting
        WAITING = 1,

        /// the task state is running
        RUNNING = 2,

        /// the task state is canceled
        CANCELED = 3,

        /// the task state is delayed
        DELAYED = 4,

        /// the task state is finished
        FINISHED = 5,

        /// the task is ending
        ENDING = 6,

        /// the task has timed out
        TIMEOUT = 7,
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
     * @typedef Configs
     * Provides configs that can be set for the task to execute.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    export interface Configs {
        /**
         * The priority of the task. The default value is taskpool.Priority.MEDIUM.
         *
         * @type { ?Priority }
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        priority?: Priority;

        /**
         * The timeout for the task in ms. The default value is Infinity.
         *
         * @type { ?number }
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        timeout?: number;
    }

    class InternalTask {
        constructor(task: Task) {
            this.taskMutex = ConcurrencyHelpers.mutexCreate();
            this.condVar = ConcurrencyHelpers.condVarCreate();
            this.dependentTasks = new containers.ConcurrentSet<Task>();
            this.task = task;
            this.creatorWorkerId = CoroutineExtras.getWorkerId();
        }

        static of(task: Task): InternalTask {
            return (task.internalTask as InternalTask);
        }

        /**
         * @returns true if the task has been completed, false - otherwise
         */
        isDoneInner(): boolean {
            return this.state.load() == InternalState.FINISHED.valueOf();
        }

        /**
         * Common execution check for all task execution types
         * @throws Error if the task violates common execution
         */
        private checkCommonExecution(): void {
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_EXECUTE_OUTSIDE, "taskpool:: groupTask cannot execute outside");
            } else if (this.seqId.load() != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_OUTSIDE, "taskpool:: seqRunnerTask cannot execute outside");
            } else if (this.isSubmitted && (this.isDependent || this.dependentTasks.size != 0)) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE, "taskpool:: executedTask with dependency cannot execute again");
            } else if (this.isSubmitted && this.task instanceof LongTask) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_LONG, "taskpool:: The long task can only be executed once");
            } else if (this.isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_AGAIN, "taskpool:: the periodicTask cannot execute again");
            }
        }

        /**
         * Common execution check including asyncRunner check
         * @throws Error if the task violates common execution
         */
        private checkCommonExecutionWithAsync(): void {
            this.checkCommonExecution();
            if (this.asyncId.load() != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_TASK_EXECUTE_OUTSIDE, "taskpool:: asyncRunnerTask cannot execute outside");
            }
        }

        /**
         * Check that the task can be executed as common task
         * @throws Error if the task can not be executed as common task
         * @see taskpool.execute
         */
        checkExecution(): void {
            this.checkCommonExecution();
            if (this.hasTimeout && this.state.load() != InternalState.NOT_FOUND.valueOf()) {
                throw new Error(TASKPOOL_ERROR_TIMEOUT_TASK_CANNOT_EXECUTED, "taskpool:: timeout task cannot be executed again");
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
            if (this.seqId.load() != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_DELAYED, "taskpool:: seqRunnerTask cannot executeDelayed outside");
            }
            if (this.asyncId.load() != 0) {
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
            if (this.hasTimeout && this.state.load() != InternalState.NOT_FOUND.valueOf()) {
                throw new Error(TASKPOOL_ERROR_TIMEOUT_TASK_CANNOT_EXECUTED, "taskpool:: timeout task cannot be executed again");
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
            if (this.isSubmitted || this.isPeriodicTask || this.groupId != 0 || this.seqId.load() != 0 || this.hasTimeout) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_PERIODIC, "taskpool:: The concurrent task has been executed and cannot be executed periodically");
            }
            if (this.isDependent || this.dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_EXECUTE_PERIODIC, "taskpool:: the task with dependency cannot executePeriodically");
            }
        }

        /**
         * Check that the task can be executed with timeout
         * @throws Error if the task can not be executed with timeout
         * @see taskpool.execute
         */
        checkExecutionForTimeout(): void {
            this.checkCommonExecutionWithAsync();
            if (this.isDependent || this.dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE, "taskpool:: timeout task with dependency cannot execute");
            }
            if (this.hasTimeout && this.state.load() !== InternalState.NOT_FOUND.valueOf()) {
                throw new Error(TASKPOOL_ERROR_TIMEOUT_TASK_CANNOT_EXECUTED, "taskpool:: timeout task cannot be executed again");
            }
            if (this.isSubmitted) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_BASIC, "taskpool:: task has been executed");
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
                let shouldActivatePendingTask = false;
                ConcurrencyHelpers.lockGuard(internalDepTask.taskMutex, () => {
                    if (internalDepTask.taskDependenciesCount > 0) {
                        internalDepTask.taskDependenciesCount--;
                    }
                    shouldActivatePendingTask = (internalDepTask.taskDependenciesCount == 0);
                    ConcurrencyHelpers.condVarNotifyOne(internalDepTask.condVar, internalDepTask.taskMutex);
                });
                if (shouldActivatePendingTask) {
                    if (!tryActivatePendingDependencyTask(dependentTask)) {
                        ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
                            ConcurrencyHelpers.condVarNotifyOne(globalTaskCondVar, globalTaskQueuesMutex);
                        });
                    }
                }
            }
            ConcurrencyHelpers.mutexLock(this.taskMutex);
            this.isRunning = false;
            ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.taskMutex);
            ConcurrencyHelpers.mutexUnlock(this.taskMutex);
        }

        getDuration(): int {
            return ((Chrono.nanoNow() - this.startTime) / Chrono.NS_PER_MS).toInt();
        }

        getTaskInfoDuration(): int {
            return this.startTime == InternalTask.NOT_STARTED_TIME ? 0 : this.getDuration();
        }

        captureAsyncStack(): void {
            this.asyncStackId = collectAsyncStack();
        }

        execute(currentWorker : GlobalQueueWorker | undefined = undefined, executionRecordArg?: ExecutionRecord): Any {
            initWorkerPoolSync();
            removePendingDependencyTask(this.task);
            this.waitForDependencies();
            if (this.state.compareAndSwap(State.WAITING, State.RUNNING) == State.CANCELED) {
                this.notifyDependencies();
                const count = allTasks.get(this.task) ?? 0;
                if (count <= 1) {
                    allTasks.delete(this.task);
                    taskSet.delete(this.task);
                } else {
                    allTasks.set(this.task, count - 1);
                }
                this.tryTriggerAsyncRunnerAndGroup();
                this.throwCancelTaskError();
            }
            // Task timed out during waiting - skip user function execution (promise already rejected)
            if (this.state.load() == InternalState.TIMEOUT.valueOf()) {
                throw new Error(TASKPOOL_ERROR_TASK_EXECUTE_TIMEOUT, "taskpool:: task has been timeout");
            }
            let previousAsyncStackId: long = 0;
            let shouldRestoreAsyncStackId: boolean = false;
            if (this.asyncStackId != 0) {
                previousAsyncStackId = getSubmitterStackId();
                if (previousAsyncStackId != this.asyncStackId) {
                    setSubmitterStackId(this.asyncStackId);
                    shouldRestoreAsyncStackId = true;
                }
            }
            this.callUserCallback(this.onStartCallback);
            let executionSucceeded: boolean = false;
            let taskResultPromise: Promise<Any>;
            this.startTime = Chrono.nanoNow();
            const startCpuTime: long = Chrono.getCpuTime();
            const executionRecord = executionRecordArg ?? this.currentExecutionRecord;
            if (executionRecord === undefined) {
                throw new Error("taskpool:: missing execution record");
            }
            const previousTaskId = getTaskId();
            const previousSubmissionId = getSubmissionId();
            try {
                setTaskId(this.task.taskId);
                setSubmissionId(executionRecord.submissionId);
                this.registerRunningExecutionRecord(executionRecord);
                const result = this.task.taskFunction();
                if (result instanceof Promise) {
                    taskResultPromise = result.then((value: Any) => {
                        executionSucceeded = true;
                        return value;
                    }).catch((e) => {
                        this.handleTaskFailure(e as Error);
                        throw e;
                    });
                } else {
                    executionSucceeded = true;
                    taskResultPromise = Promise.resolve(result);
                }
            } catch (e) {
                this.handleTaskFailure(e as Error);
                taskResultPromise = Promise.reject(e as Error);
            }

            // We cannot reliably calculate ioDuration in 1.2 due to possible coroutine switches.
            // And we should set ioDuration = 0, totalDuration = cpuDuration here
            this.task.cpuDuration = ((Chrono.getCpuTime() - startCpuTime) / Chrono.NS_PER_MS).toInt();
            this.task.totalDuration = this.task.cpuDuration;
            this.task.ioDuration = 0;

            let taskFinishedhandle = (): void => {
                if (isTaskPoolUseLaunch) {
                    this.notifyDependencies();
                } else {
                    totalExecTime.fetchAdd(this.getDuration());
                    totalExecCount.fetchAdd(1);
                }
                const count = allTasks.get(this.task) ?? 0;
                if (count <= 1) {
                    allTasks.delete(this.task);
                    taskSet.delete(this.task);
                } else {
                    allTasks.set(this.task, count - 1);
                }
                if (currentWorker !== undefined) {
                    ConcurrencyHelpers.lockGuard(currentWorker!.currentTasksMutex, () => {
                        const curCount: int | undefined = currentWorker!.currentTasks.get(this.task);
                        if (curCount == undefined || curCount <= 1) {
                            currentWorker!.currentTasks.delete(this.task);
                        } else {
                            currentWorker!.currentTasks.set(this.task, curCount - 1);
                        }
                        currentWorker!.currentPriority = undefined;
                    });
                }
                if (this.isTimeout.load()) {
                    this.state.store(InternalState.FINISHED.valueOf());
                    this.tryTriggerAsyncRunnerAndGroup();
                    return;
                }
                this.state.store(InternalState.FINISHED.valueOf());
                try {
                    if (executionSucceeded) {
                        if (this.groupId != 0 && this.onSuccessCallback !== undefined) {
                            let group: TaskGroup | undefined = allGroups.get(this.groupId);
                            if (group !== undefined) {
                                let callback: CallbackFunction = this.onSuccessCallback as CallbackFunction;
                                GroupState.of(group).successCallbacks.add(callback);
                            }
                        } else {
                            this.callUserCallback(this.onSuccessCallback);
                        }
                    }
                } catch (e) {
                    try {
                        this.onFailCallback?.(e as Error);
                    } catch (callbackError) {}
                    throw e;
                } finally {
                    this.tryTriggerAsyncRunnerAndGroup();
                }
            };

            return taskResultPromise.finally(() => {
                try {
                    this.unregisterRunningExecutionRecord(executionRecord);
                    setSubmissionId(previousSubmissionId);
                    setTaskId(previousTaskId);
                    taskFinishedhandle();
                    this.finishExecutionRecord(executionRecord);
                } finally {
                    if (shouldRestoreAsyncStackId) {
                        setSubmitterStackId(previousAsyncStackId);
                    }
                }
            }).then((value: Any) => {
                executionRecord.markTaskResult(value);
                return executionRecord.promise;
            }).catch((e) => {
                executionRecord.markTaskError(e as Error);
                return executionRecord.promise;
            });
        }

        checkCancellation(): void {
            if (this.state.load() == State.CANCELED.valueOf()) {
                this.throwCancelTaskError();
            }
        }

        /// @throws Error for the canceled task
        private throwCancelTaskError(): void {
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_CANCELED, "taskpool:: taskGroup has been canceled");
            }
            if (this.seqId.load() != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_CANCELED, "taskpool:: sequenceRunner task has been canceled");
            }
            if (this.asyncId.load() != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_CANCELED, "taskpool:: asyncRunner task has been canceled");
            }
            throw new Error(TASKPOOL_ERROR_TASK_CANCELED, "taskpool:: task has been canceled");
        }

        private callUserCallback(callback: CallbackFunction | undefined): void {
            try {
                callback?.();
            } catch (e) {}
        }

        private handleTaskFailure(error: Error): void {
            if (this.groupId != 0) {
                let group: TaskGroup | undefined = allGroups.get(this.groupId);
                if (group !== undefined) {
                    GroupState.of(group).hasFailedTask.store(1);
                }
            }
            try {
                // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                this.onFailCallback?.(error);
            } catch (callbackError) {}
        }

        /// The task is added to taskpool for execution
        enqueue(priority?: Priority, executionRecordArg?: ExecutionRecord): void {
            priority = priority === undefined ? Priority.MEDIUM : priority;
            this.priority = priority;
            while(true) {
                let curState = this.state.load();
                if (curState == State.CANCELED) {
                    break;
                }
                if (this.state.compareAndSwap(curState, InternalState.WAITING) == curState) {
                    break;
                }
            }
            if (!isTaskPoolUseLaunch) {
                const executionRecord = executionRecordArg ?? this.currentExecutionRecord;
                if (executionRecord === undefined) {
                    throw new Error("taskpool:: missing execution record");
                }
                initGlobalTaskQueues();
                while (!globalTaskQueue?.get(normalizeTaskQueuePriority(priority))?.add(
                    new QueuedExecution(this.task, executionRecord, priority)
                )) {
                    Coroutine.Schedule();
                }
                let count = allTasks.get(this.task) ?? 0;
                allTasks.set(this.task, count + 1);
                waitingTasksNum.fetchAdd(1);
                tryTriggerExpand();
                // Notify waiting workers using condition variable
                ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
                    ConcurrencyHelpers.condVarNotifyOne(globalTaskCondVar, globalTaskQueuesMutex);
                });
            } 
            this.callUserCallback(this.onEnqueueCallback);
        }

        createExecutionRecord(): ExecutionRecord {
            const executionRecord = new ExecutionRecord(
                this.nextSubmissionId.fetchAdd(1),
                () => this.tryTriggerSequenceRunner()
            );
            ConcurrencyHelpers.lockGuard(this.taskMutex, () => {
                if (this.currentExecutionRecord === undefined) {
                    this.currentExecutionRecord = executionRecord;
                } else {
                    this.pendingExecutionRecords.push(executionRecord);
                }
            });
            return executionRecord;
        }

        /// Caller must not hold seqMutex, implInnerMutex, or other outer locks that may nest with taskMutex.
        finishExecutionRecord(executionRecord: ExecutionRecord): void {
            ConcurrencyHelpers.lockGuard(this.taskMutex, () => {
                if (this.currentExecutionRecord === executionRecord) {
                    this.currentExecutionRecord = undefined;
                    if (this.pendingExecutionRecords.length > 0) {
                        this.currentExecutionRecord = this.pendingExecutionRecords.shift();
                    }
                    return;
                }
                const idx = this.pendingExecutionRecords.indexOf(executionRecord);
                if (idx >= 0) {
                    this.pendingExecutionRecords.splice(idx, 1);
                }
            });
        }

        registerRunningExecutionRecord(executionRecord: ExecutionRecord): void {
            ConcurrencyHelpers.lockGuard(this.taskMutex, () => {
                this.executionRecordsBySubmissionId.set(executionRecord.submissionId, executionRecord);
            });
        }

        unregisterRunningExecutionRecord(executionRecord: ExecutionRecord): void {
            ConcurrencyHelpers.lockGuard(this.taskMutex, () => {
                const registeredRecord = this.executionRecordsBySubmissionId.get(executionRecord.submissionId);
                if (registeredRecord === executionRecord) {
                    this.executionRecordsBySubmissionId.delete(executionRecord.submissionId);
                }
            });
        }

        getExecutionRecordBySubmissionId(submissionId: int): ExecutionRecord | undefined {
            let executionRecord: ExecutionRecord | undefined = undefined;
            ConcurrencyHelpers.lockGuard(this.taskMutex, () => {
                executionRecord = this.executionRecordsBySubmissionId.get(submissionId);
            });
            return executionRecord;
        }

        getOnReceiveListenerBinding(): ReceiveListenerBinding | undefined {
            return this.onReceiveListener.load();
        }

        /**
         * @see taskpool.execute
         * @see taskpool.SequenceRunner.execute
         * @see taskpool.AsyncRunner.execute
         */


        tryTriggerSequenceRunner(): void {
            if (this.seqId.load() == 0) {
                return;
            }
            let nextTask: Task | undefined = undefined;
            const canceledExecutionRecords: Array<ExecutionRecord> = new Array<ExecutionRecord>();
            const canceledInternalTasks: Array<InternalTask> = new Array<InternalTask>();
            ConcurrencyHelpers.lockGuard(this.runnerImpl.seqMutex, () => {
                if (this.runnerImpl.tasks.length == 0) {
                    this.runnerImpl.isRunning = false;
                } else {
                    nextTask = this.runnerImpl.tasks.shift();
                    while (nextTask !== undefined && InternalTask.of(nextTask!).state.load() == (State.CANCELED.valueOf())) {
                        const nextInternalTask = InternalTask.of(nextTask!);
                        const currentExecutionRecord = nextInternalTask.currentExecutionRecord;
                        if (currentExecutionRecord !== undefined) {
                            currentExecutionRecord.markCanceled();
                            currentExecutionRecord.rejectPromise(new Error(
                                TASKPOOL_ERROR_SEQ_RUNNER_CANCELED,
                                "taskpool:: sequenceRunner task has been canceled"
                            ));
                            canceledExecutionRecords.push(currentExecutionRecord);
                            canceledInternalTasks.push(nextInternalTask);
                        }
                        nextTask = this.runnerImpl.tasks.shift();
                    }
                    if (nextTask === undefined) {
                        this.runnerImpl.isRunning = false;
                    }
                }
            });
            for (let i = 0; i < canceledExecutionRecords.length; i++) {
                canceledInternalTasks[i].finishExecutionRecord(canceledExecutionRecords[i]);
            }
            if (nextTask !== undefined) {
                InternalTask.of(nextTask!).enqueue(this.runnerImpl.priority);
            }
        }

        tryTriggerAsyncRunnerAndGroup(): void {
            // handle asyncRunner
            if (this.asyncId.load() != 0) {
                const asyncRunner = AsyncRunnerImpl.getAsyncRunnerById(this.asyncId.load());
                if (asyncRunner) {
                    asyncRunner.handleTaskCompletionFromTryTriggerRunner();
                }
            }

            if (this.groupId != 0) {
                let group: TaskGroup | undefined = allGroups.get(this.groupId);
                if (group === undefined) {
                    return;
                }
                GroupState.of(group).taskCount.fetchSub(1);
                if (GroupState.of(group).state_.load() != InternalState.RUNNING.valueOf()) {
                    GroupState.of(group).state_.store(InternalState.RUNNING.valueOf());
                }
                if (GroupState.of(group).taskCount.load() == 0) {
                    if (GroupState.of(group).hasFailedTask.load() == 0) {
                        GroupState.of(group).successCallbacks.forEach((callback: CallbackFunction) => {
                            try {
                                callback();
                            } catch (e) {}
                        });
                    }
                    GroupState.of(group).state_.store(InternalState.FINISHED.valueOf());
                    allGroups.delete(this.groupId);
                }
            }
        }
        clearPendingExecutionRecordsUnsafe(): void {
            while (this.pendingExecutionRecords.length > 0) {
                this.pendingExecutionRecords.shift();
            }
        }

        /**
         * Try to transition task state to TIMEOUT
         * @returns true if state transition succeeded, false otherwise
         */
        tryTransitionToTimeout(): boolean {
            return this.state.compareAndSwap(InternalState.NOT_FOUND, InternalState.TIMEOUT) == InternalState.NOT_FOUND ||
                   this.state.compareAndSwap(InternalState.WAITING, InternalState.TIMEOUT) == InternalState.WAITING ||
                   this.state.compareAndSwap(InternalState.RUNNING, InternalState.TIMEOUT) == InternalState.RUNNING;
        }

        /// id of group which contains the task, 0 means the task is not part of any group
        groupId: int = 0;
        /// id of sequence runner which ran the task, 0 means task was not ran via any sequence runner
        seqId: AtomicInt = new AtomicInt(0);
        /// id of async runner which ran the task, 0 means task was not ran via any async runner
        asyncId: AtomicInt = new AtomicInt(0);
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
        /// identifier of worker which created the task
        creatorWorkerId: int = -1;
        /// an instance of the task is running
        isRunning: boolean = false;
        /// start time of the task
        private startTime: long = 0;
        /// the task has been executed periodically
        isPeriodicTask: boolean = false;
        /// identifier of task which is executeDelayed or executePeriodically
        timerId: int = -1;
        /// id of the coroutine that executes task
        coroId: int = -1;
        /// id of the async stack collected on task submission
        asyncStackId: long = 0;
        /// whether the task has a timeout set
        hasTimeout: boolean = false;
        /// timeout duration in milliseconds
        timeoutMs: number = 0;
        /// an instance of task has been timeout
        isTimeout: AtomicBoolean = new AtomicBoolean(false);

        // NOTE(ipetrov, #19949): Change to private when bug in frontend will be fixed
        taskMutex: Object;
        condVar: Object;

        currentExecutionRecord: ExecutionRecord | undefined = undefined;
        pendingExecutionRecords: Array<ExecutionRecord> = new Array<ExecutionRecord>();
        executionRecordsBySubmissionId: Map<int, ExecutionRecord> = new Map<int, ExecutionRecord>();

        priority: Priority;
        state: AtomicInt = new AtomicInt(InternalState.NOT_FOUND);

        runnerImpl: SeqRunnerImpl;
        runnerAsyncImpl: AsyncRunnerImpl;

        readonly onReceiveListener: AtomicReference<ReceiveListenerBinding | undefined> =
            new AtomicReference<ReceiveListenerBinding | undefined>(undefined);
        nextSubmissionId: AtomicInt = new AtomicInt(1);

        onEnqueueCallback?: CallbackFunction;
        onStartCallback?: CallbackFunction;
        onFailCallback?: CallbackFunctionWithError;
        onSuccessCallback?: CallbackFunction;
        private task: Task;
        private static readonly NOT_STARTED_TIME: long = 0;
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
         * @param { ...Any } args arguments for function to execute with
         */
        constructor(name: string, func: Function, ...args: FixedArray<Any>) {
            this.name = name;
            this.arguments = Array.from(args);
            if (isTaskpoolSupportInterop) {
                let serializeHelper = new InteropSerializeHelper(args);
                this.taskFunction = (): Any => {
                    let deserializedArgs = serializeHelper.deserialize();
                    return func.unsafeCall(...deserializedArgs);
                };
            } else {
                this.taskFunction = (): Any => {
                    return func.unsafeCall(...args);
                };
            }
            this.totalDuration = 0;
            this.ioDuration = 0;
            this.cpuDuration = 0;
            // Each task has unique identifier
            this.taskId = Task.generateId();
            this.internalTask = new InternalTask(this);
            taskSet.add(this);
        }

        /**
         * Create a Task instance
         *
         * @param { Function } func Concurrent function to execute in the taskpool
         * @param { ...Any } args arguments for function to execute with
         */
        constructor(func: Function, ...args: FixedArray<Any>) {
            this(func.name, func, ...args);
        }

        /**
         * Check current running task is canceled or not
         *
         * @returns { boolean } true if current running task is canceled, false - otherwise
         */
        static isCanceled(): boolean {
            let result: boolean = false;
            let currentTask = getCurrentTask();
            if (currentTask !== undefined) {
                result = InternalTask.of(currentTask!).state.load() == (State.CANCELED.valueOf());
            }
            return result;
        }

        /**
         * Check whether the current task has been canceled
         *
         * @throws Error if the current task has been canceled
         */
        static checkCancellation(): void {
            let task = getCurrentTask();
            if (task !== undefined) {
                InternalTask.of(task!).checkCancellation();
            }
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
            if (InternalTask.of(this).isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_ADD_DEPENDENCY, "taskpool:: the periodic task cannot have a dependency");
            }
            if (InternalTask.of(this).isSubmitted || InternalTask.of(this).seqId.load() != 0 || InternalTask.of(this).asyncId.load() != 0) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_ADD_DEPENDENCY, "taskpool:: asyncRunnerTask or seqRunnerTask or executedTask cannot addDependency");
            }
            if (InternalTask.of(this).groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_DEPENDENCY, "taskpool:: groupTask cannot addDependency");
            }
            for (const task: Task of tasks) {
                if (InternalTask.of(task).isPeriodicTask) {
                    throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_ADD_DEPENDENCY, "taskpool:: the periodic task cannot have a dependency");
                }
                if (InternalTask.of(task).isSubmitted || InternalTask.of(task).seqId.load() != 0 || InternalTask.of(task).asyncId.load() != 0) {
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
            return InternalTask.of(this).isDoneInner();
        }

        /**
         * Send data to host side and trigger the registered callback
         * @param { ...Any } args arguments for the registered callback
         * @throws Error if the function is not called from a taskpool task
         * @throws Error if the callback is not registered
         */
        static sendData(...args: FixedArray<Any>): void {
            const item: Task | undefined = getCurrentTask();
            if (item === undefined) {
                throw new Error(TASKPOOL_ERROR_NOT_IN_TASKPOOL, "taskpool:: The function is not called in the TaskPool thread");
            }
            const task: Task = item;
            const internalTask = InternalTask.of(task);
            const binding = internalTask.getOnReceiveListenerBinding();
            if (binding === undefined) {
                throw new Error(TASKPOOL_ERROR_CALLBACK_NOT_REGISTERED, "taskpool:: The callback is not registered on the host side");
            }
            const submissionId = getSubmissionId();
            if (submissionId <= 0) {
                throw new Error(TASKPOOL_ERROR_NOT_IN_TASKPOOL, "taskpool:: The function is not called in the TaskPool thread");
            }
            const executionRecord = internalTask.getExecutionRecordBySubmissionId(submissionId);
            if (executionRecord === undefined) {
                throw new Error(TASKPOOL_ERROR_NOT_IN_TASKPOOL, "taskpool:: The function is not called in the TaskPool thread");
            }
            if (!executionRecord.canDispatchToHost()) {
                return;
            }
            const payload = materializeSendDataArgs(args);
            const event = new ReceiveEvent(payload, binding);
            if (executionRecord.receiveDispatcher.enqueue(event)) {
                tryScheduleOnReceiveDrain(executionRecord, binding.hostWorkerId);
            }
        }

        /**
         * Register a callback for this task to receive and handle data from the taskpool task
         */
        onReceiveData(callback?: Function): void {
            const internalTask = InternalTask.of(this);
            if (callback === undefined) {
                internalTask.onReceiveListener.store(undefined);
                return;
            }
            if (callback === null) {
                throw new TypeError("the type of onReceiveData's parameter must be function.");
            }
            if (!(callback instanceof Function)) {
                throw new TypeError("the type of onReceiveData's parameter must be function.");
            }
            internalTask.onReceiveListener.store(
                new ReceiveListenerBinding(callback, CoroutineExtras.getWorkerId())
            );
        }

        /// Concurrent function to execute in taskpool
        taskFunction: () => Any;

        /// The concurrent function arguments
        arguments?: Any[];

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
        constructor(name: string, func: Function, ...args: FixedArray<Any>) {
            super(name, func, ...args);
        }

        /**
         * Create a LongTask instance
         * @param { Function } func Concurrent function to execute in the taskpool
         * @param { ...Any } args arguments for func to execute with
         */
        constructor(func: Function, ...args: FixedArray<Any>) {
            super("", func, ...args);
        }
    }

    class GroupState {
        constructor() {
            this.id = GroupState.generateGroupId();
            this.tasks = new Array<Task>();
        }

        static of(group: TaskGroup): GroupState {
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
        state_: AtomicInt = new AtomicInt(InternalState.NOT_FOUND);
        taskCount: AtomicInt = new AtomicInt(0);
        hasFailedTask: AtomicInt = new AtomicInt(0);
        hasTimeout: boolean = false;
        successCallbacks: containers.ConcurrentSet<CallbackFunction> = new containers.ConcurrentSet<CallbackFunction>();
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
         * @param { ...Any } args arguments for func to execute with
         */
        addTask(func: Function, ...args: FixedArray<Any>): void {
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
            GroupState.of(this).taskCount.fetchAdd(1);
        }

        private checkExecution(task: Task): void {
            if (InternalTask.of(task).groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_GROUP, "taskpool:: taskGroup cannot add groupTask");
            } else if (task instanceof LongTask) {
                throw new Error(TASKPOOL_ERROR_LONG_TASK_NOT_SUPPORTED, "taskpool:: The interface does not support the long task");
            } else if (InternalTask.of(task).isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_NOT_SUPPORTED, "taskpool:: The interface does not support the periodicTask");
            } else if (InternalTask.of(task).isSubmitted || InternalTask.of(task).seqId.load() != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_SEQ_RUNNER, "taskpool:: taskGroup cannot add seqRunnerTask or executedTask");
            } else if (InternalTask.of(task).isDependent || InternalTask.of(task).dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED, "taskpool:: dependent task not allowed.");
            } else if (InternalTask.of(task).hasTimeout && InternalTask.of(task).state.load() != InternalState.NOT_FOUND.valueOf()) {
                throw new Error(TASKPOOL_ERROR_TIMEOUT_TASK_CANNOT_EXECUTED, "taskpool:: timeout task cannot be executed again");
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
            this.seqPromise = Promise.resolve<Any>(new Object());
            this.tasks = new Array<Task>();
            this.seqMutex = ConcurrencyHelpers.mutexCreate();
            this.isRunning = false;
        }

        priority: Priority;
        isRunning: boolean;
        tasks: Array<Task>;
        seqMutex: Object;

        readonly id: int;
        seqPromise: Promise<Any>;
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
            let newPriority = priority === undefined ? Priority.MEDIUM : priority;
            if (name.isEmpty() || this.impl.priority === undefined) {
                this.impl.priority = newPriority;
            } else if (this.impl.priority !== newPriority) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_CHANGE_PRIORITY, "seqRunner:: The priority can not be changed.");
            }
        }

        execute(task: Task): Promise<Any> {
            initWorkerPoolSync();
            this.checkExecution(task);

            const internalTask = InternalTask.of(task);
            internalTask.captureAsyncStack();
            internalTask.seqId.store(this.impl.id);
            internalTask.runnerImpl = this.impl;
            taskSubmitted(task);
            internalTask.isSubmitted = true;

            const executionRecord = internalTask.createExecutionRecord();
            if (isTaskPoolUseLaunch) {
                internalTask.enqueue(this.impl.priority, executionRecord);
                const taskRunner = (value: Any): Any => {
                    return launchQueuedExecutionRecord(internalTask, executionRecord);
                };
                this.impl.seqPromise = this.impl.seqPromise.then<Any>(taskRunner, (error) => {
                    taskRunner(undefined);
                    return new Promise<never>((resolve, reject) => {});
                } );
                return executionRecord.promise;
            }

            ConcurrencyHelpers.lockGuard(this.impl.seqMutex, () => {
                if (this.impl.isRunning) {
                    while(true) {
                        let curState = InternalTask.of(task).state.load();
                        if (curState == State.CANCELED) {
                            break;
                        }
                        if (InternalTask.of(task).state.compareAndSwap(curState, InternalState.WAITING) == curState) {
                            break;
                        }
                    }
                    this.impl.tasks.push(task);
                } else {
                    this.impl.isRunning = true;
                    internalTask.enqueue(this.impl.priority, executionRecord);
                }
            });
            return executionRecord.promise;
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
            } else if (InternalTask.of(task).isSubmitted || InternalTask.of(task).seqId.load() != 0 || InternalTask.of(task).hasTimeout) {
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

        static getAsyncRunnerById(id: int): AsyncRunnerImpl | undefined {
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
            const canceledExecutionRecords: Array<ExecutionRecord> = new Array<ExecutionRecord>();
            const canceledInternalTasks: Array<InternalTask> = new Array<InternalTask>();
            ConcurrencyHelpers.mutexLock(this.implInnerMutex);
            try {
                if (this.runningCount > 0) {
                    this.runningCount--;
                }
                // poll tasks from the waiting queue if available
                while (this.waitingQueue.length > 0 && this.runningCount < this.runningCapacity) {
                    let nextTask = this.waitingQueue.shift();
                    if (nextTask == undefined) break;
                    if (InternalTask.of(nextTask).state.load() == State.CANCELED.valueOf()) {
                        const canceledInternalTask = InternalTask.of(nextTask!);
                        const canceledRecord = canceledInternalTask.currentExecutionRecord;
                        if (canceledRecord !== undefined) {
                            canceledRecord.markCanceled();
                            canceledRecord.rejectPromise(
                                new Error(TASKPOOL_ERROR_ASYNC_RUNNER_CANCELED, "taskpool:: asyncRunner task has been canceled")
                            );
                            canceledExecutionRecords.push(canceledRecord);
                            canceledInternalTasks.push(canceledInternalTask);
                        }
                        continue;
                    }
                    this.runningCount++;
                    toEnqueue.push(nextTask);
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.implInnerMutex);
            }

            for (let i = 0; i < canceledExecutionRecords.length; i++) {
                canceledInternalTasks[i].finishExecutionRecord(canceledExecutionRecords[i]);
            }

            for (let t of toEnqueue) {
                const taskInternalTask = InternalTask.of(t);
                taskInternalTask.enqueue(taskInternalTask.priority);
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
        // Find the corresponding asyncRunnerImpl by task.asyncID
        static asyncRunners: Map<number, AsyncRunnerImpl> = new Map<number, AsyncRunnerImpl>();
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
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_PARAM, "taskpool:: waitingCapacity must be a non-negative number");
            }

            this.impl = AsyncRunner.getOrCreateAsyncRunnerImpl(name, rcap, wcap);
            this.priority = Priority.MEDIUM;

            ConcurrencyHelpers.lockGuard(AsyncRunner.mutex, () => {
                AsyncRunnerImpl.asyncRunners.set(this.impl.id, this.impl);
            });
        }

        /**
         * Execute a concurrent function.
         * @param task The task to be executed
         * @param priority Task execution priority, MEDIUM is default.
         * @returns
         */
        execute(task: Task, priority?: Priority): Promise<Any> {
            initWorkerPoolSync();
            this.checkExecution(task);

            const internalTask = InternalTask.of(task);
            internalTask.captureAsyncStack();
            internalTask.runnerAsyncImpl = this.impl;
            internalTask.asyncId.store(this.impl.id);
            taskSubmitted(task);
            internalTask.isSubmitted = true;

            const executionRecord = internalTask.createExecutionRecord();

            let shouldEnqueue = false;
            let enqueuePriority: Priority = priority == undefined ? this.priority : priority;
            let droppedHeadExecutionRecord: ExecutionRecord | undefined = undefined;
            let droppedHeadInternalTask: InternalTask | undefined = undefined;
            ConcurrencyHelpers.mutexLock(this.impl.implInnerMutex);
            try {
                if (this.impl.runningCount < this.impl.runningCapacity) {
                    this.impl.runningCount = this.impl.runningCount + 1;
                    shouldEnqueue = true;
                } else {
                    // If waitingCapacity == 0, treat it's waitingQueue's length as unlimited. Task is added to tail.
                    while(true) {
                        let curState = InternalTask.of(task).state.load();
                        if (curState == State.CANCELED) {
                            break;
                        }
                        if (InternalTask.of(task).state.compareAndSwap(curState, InternalState.WAITING) == curState) {
                            break;
                        }
                    }
                    if (this.impl.waitingCapacity == 0) {
                        this.impl.waitingQueue.push(task);
                    } else {
                        if (this.impl.waitingCapacity > 0 && this.impl.waitingQueue.length >= this.impl.waitingCapacity) {
                            // the waiting queue exceeds capacity, discard the head task.
                            const head = this.impl.waitingQueue.shift();
                            if (head != undefined) {
                                const headInternalTask = InternalTask.of(head!);
                                const currentExecutionRecord = headInternalTask.currentExecutionRecord;
                                if (currentExecutionRecord !== undefined) {
                                    currentExecutionRecord.markCanceled();
                                    currentExecutionRecord.rejectPromise(new Error(
                                        TASKPOOL_ERROR_ASYNC_RUNNER_WAITING_QUEUE_DROPPED,
                                        "taskpool:: waiting queue dropped head"
                                    ));
                                    droppedHeadExecutionRecord = currentExecutionRecord;
                                    droppedHeadInternalTask = headInternalTask;
                                }
                                headInternalTask.isSubmitted = false;
                            }
                        }
                        this.impl.waitingQueue.push(task);
                    }
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.impl.implInnerMutex);
            }

            if (droppedHeadInternalTask !== undefined && droppedHeadExecutionRecord !== undefined) {
                droppedHeadInternalTask.finishExecutionRecord(droppedHeadExecutionRecord);
            }

            // Enqueue outside of implInnerMutex to avoid ABBA deadlock with globalTaskQueuesMutex.
            if (shouldEnqueue) {
                internalTask.enqueue(enqueuePriority, executionRecord);
            }

            return executionRecord.promise;
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
            } else if (InternalTask.of(task).isSubmitted || InternalTask.of(task).asyncId.load() != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_EXECUTED, "taskpool:: asyncRunner cannot execute executedTask");
            } else if (InternalTask.of(task).isDependent || InternalTask.of(task).dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED, "taskpool:: asyncRunner dependent task not allowed.");
            } else if (InternalTask.of(task).isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_ASYNC_RUNNER, "taskpool:: asyncRunner cannot execute the periodicTask");
            } else if (InternalTask.of(task).isSubmitted) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_BASIC, "taskpool:: task has been executed");
            } else if (InternalTask.of(task).seqId.load() != 0) {
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
        threadInfos: ThreadInfo[] = new ThreadInfo[0](new ThreadInfo());

        /// Array of taskpool tasks information
        taskInfos: TaskInfo[] = new TaskInfo[0](new TaskInfo());
    }

    /**
     * Create task and execute
     *
     * @param { Function } func a concurrent function to create task from
     * @param { ...Any } args arguments for func to execute with
     * @returns { Promise<Any> } Promise for result of executed task
     */
    export function execute(func: Function, ...args: FixedArray<Any>): Promise<Any> {
        let task = new Task(func.name, func, ...args);
        return execute(task);
    }

    /**
     * Execute a concurrent task with default priority
     *
     * @param task The task to execute
     * @returns { Promise<Any> } Promise for result of executed task
     */
    export function execute(task: Task): Promise<Any>{
        return executeImpl(task);
    }

    /**
     * Execute a concurrent task with specified priority
     *
     * @param task The task to execute
     * @param { Priority } priority for the task
     * @returns { Promise<Any> } Promise for result of executed task
     */
    export function execute(task: Task, priority: Priority): Promise<Any>{
        return executeImpl(task, priority);
    }

    /**
     * Execute a concurrent task with Configs.
     *
     * @param { Task } task - Task to be executed.
     * @param { Configs } configs - Configs of the task.
     * @returns { Promise<Any> } - Promise for result of executed task
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    export function execute(task: Task, configs: Configs): Promise<Any>{
        let timeout: number = configs.timeout ?? Infinity;
        if (timeout >= 1 && timeout !== Infinity) {
            return executeWithTimeout(task, configs);
        }
        return executeImpl(task, configs.priority);
    }

    function executeImpl(task: Task, priority?: Priority): Promise<Any> {
        initWorkerPoolSync();
        const internalTask = InternalTask.of(task);
        internalTask.checkExecution();

        internalTask.captureAsyncStack();
        internalTask.isSubmitted = true;

        if (isTaskPoolUseLaunch) {
            return launchTaskExecution(task).promise;
        }

        const executionRecord = internalTask.createExecutionRecord();
        internalTask.enqueue(priority, executionRecord);
        return executionRecord.promise;
    }

    function executeWithTimeout(task: Task, configs: Configs): Promise<Any> {
        initWorkerPoolSync();
        const internalTask = InternalTask.of(task);
        internalTask.checkExecutionForTimeout();

        internalTask.captureAsyncStack();
        internalTask.isSubmitted = true;
        internalTask.hasTimeout = true;
        internalTask.timeoutMs = configs.timeout ?? Infinity;

        let executionRecord: ExecutionRecord;
        if (isTaskPoolUseLaunch) {
            executionRecord = launchTaskExecution(task);
        } else {
            executionRecord = internalTask.createExecutionRecord();
        }

        // Create timeout promise
        let timeoutTimerId: int = -1;
        const timeoutPromise = new Promise<Any>((resolve, reject) => {
            timeoutTimerId = setTimeout(() => {
                if (internalTask.tryTransitionToTimeout() || (internalTask.state.load() == InternalState.CANCELED.valueOf() && internalTask.isRunning)) {
                    internalTask.isTimeout.store(true);
                    executionRecord.markTimedOut();
                    reject(new Error(TASKPOOL_ERROR_TASK_EXECUTE_TIMEOUT, "taskpool:: task has been timeout"));
                }
            }, internalTask.timeoutMs.toInt());
        });

        // Return race result
        const racePromise = Promise.race<Any>([executionRecord.promise, timeoutPromise]);
        if (!isTaskPoolUseLaunch) {
            internalTask.enqueue(configs.priority, executionRecord);
        }
        return racePromise.finally(() => {
            if (timeoutTimerId !== -1) {
                clearTimeout(timeoutTimerId);
                timeoutTimerId = -1;
            }
        });
    }

    /**
     * Execute a concurrent task group
     *
     * @param { TaskGroup } group The task group for execution
     * @param { Priority } [priority] priority for tasks in the group
     * @returns { Promise<Array<Any>> } Promise for array of results of executed tasks from the group
     */
    export function execute(group: TaskGroup, priority?: Priority): Promise<Array<Any>> {
        return executeImplGroup(group, priority);
    }

    /**
     * Execute a concurrent task group with specified configs
     *
     * @param { TaskGroup } group - Task group to be executed.
     * @param { Configs } configs - Configs of each task in the TaskGroup.
     * @returns { Promise<Array<Any>> } Promise for array of results of executed tasks from the group
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    export function execute(group: TaskGroup, configs: Configs): Promise<Array<Any>> {
        let timeout: number = configs.timeout ?? Infinity;
        if (timeout >= 1 && timeout !== Infinity) {
            return executeGroupWithTimeout(group, configs);
        }
        return executeImplGroup(group, configs.priority);
    }

    /**
     * Submit all tasks in a group and return their promises
     * @param group The task group
     * @param priority Optional priority for task execution
     * @returns Array of promises for each task
     */
    function submitGroupTasks(group: TaskGroup, priority?: Priority): Array<Promise<Any>> {
        const promises: Array<Promise<Any>> = new Array<Promise<Any>>();
        GroupState.of(group).tasks.forEach((task: Task) => {
            const internalTask = InternalTask.of(task);
            internalTask.captureAsyncStack();
            internalTask.isSubmitted = true;
            if (isTaskPoolUseLaunch) {
                promises.push(launchTaskExecution(task).promise);
            } else {
                const executionRecord = internalTask.createExecutionRecord();
                internalTask.enqueue(priority, executionRecord);
                promises.push(executionRecord.promise);
            }
        });
        return promises;
    }

    function executeImplGroup(group: TaskGroup, priority?: Priority): Promise<Array<Any>> {
        initWorkerPoolSync();
        const tasksCount = GroupState.of(group).tasks.length.toInt();
        if (tasksCount == 0) {
            return Promise.resolve<Array<Any>>(new Array<Any>());
        }
        // Check if taskGroup has already been configured with timeout
        if (GroupState.of(group).hasTimeout) {
            throw new Error(TASKPOOL_ERROR_TIMEOUT_TASK_CANNOT_EXECUTED, "taskpool:: the taskGroup cannot be executed again, taskGroup has already set timeout");
        }
        if (GroupState.of(group).isTaskAlreadySubmitted) {
            GroupState.of(group).taskCount.store(tasksCount);
            GroupState.of(group).hasFailedTask.store(0);
            GroupState.of(group).successCallbacks.clear();
        }
        GroupState.of(group).isTaskAlreadySubmitted = true;
        allGroups.set(GroupState.of(group).id, group);
        GroupState.of(group).state_.store(InternalState.WAITING.valueOf());
        const promises = submitGroupTasks(group, priority);
        return Promise.all<Any>(promises);
    }

    function executeGroupWithTimeout(group: TaskGroup, configs: Configs): Promise<Array<Any>> {
        initWorkerPoolSync();
        const tasksCount = GroupState.of(group).tasks.length.toInt();
        if (tasksCount == 0) {
            return Promise.resolve<Array<Any>>(new Array<Any>());
        }
        // Check if taskGroup has already been executed
        if (GroupState.of(group).state_.compareAndSwap(InternalState.NOT_FOUND, InternalState.WAITING) != InternalState.NOT_FOUND) {
            throw new Error(TASKPOOL_ERROR_TASK_NOT_EXECUTED_GROUP, "taskpool:: the taskGroup cannot be executed again, taskGroup cannot be set timeout");
        }
        GroupState.of(group).isTaskAlreadySubmitted = true;
        GroupState.of(group).hasTimeout = true;
        allGroups.set(GroupState.of(group).id, group);
        const promises = submitGroupTasks(group, configs.priority);

        let timeoutTimerId: int = -1;
        const timeoutPromise = new Promise<Array<Any>>((resolve, reject) => {
            timeoutTimerId = setTimeout(() => {
                if (timeoutTimerId === -1 || GroupState.of(group).state_.load() === InternalState.FINISHED.valueOf()) {
                    return;
                }
                if (GroupState.of(group).state_.load() === InternalState.CANCELED.valueOf()) {
                    let hasRunningTask = GroupState.of(group).tasks.some((task: Task) => InternalTask.of(task).isRunning);
                    if (hasRunningTask) {
                        reject(new Error(TASKPOOL_ERROR_TASK_EXECUTE_TIMEOUT, "taskpool:: task group has been timeout"));
                    }
                    return;
                }
                GroupState.of(group).state_.store(InternalState.TIMEOUT.valueOf());
                GroupState.of(group).tasks.forEach((task: Task) => {
                    InternalTask.of(task).state.store(InternalState.TIMEOUT.valueOf());
                });
                reject(new Error(TASKPOOL_ERROR_TASK_EXECUTE_TIMEOUT, "taskpool:: task group has been timeout"));
            }, configs.timeout!.toInt());
        });
        const allPromise = Promise.all<Any>(promises);

        return new Promise<Array<Any>>((resolve, reject) => {
            Promise.race<Array<Any>>([allPromise, timeoutPromise]).then((result: Array<Any>) => {
                if (timeoutTimerId != -1) {
                    clearTimeout(timeoutTimerId);
                    timeoutTimerId = -1;
                }
                resolve(result);
            }).catch((error: Error) => {
                if (timeoutTimerId != -1) {
                    clearTimeout(timeoutTimerId);
                    timeoutTimerId = -1;
                }
                reject(error);
            });
        });
    }

    /**
     * Delayed execution of a concurrent task with default priority
     *
     * @param { int } delayTime timeout in ms to execute task after
     * @param { Task } task The task to execute
     * @returns { Promise<Any> } Promise for result of executed task
     */
    export function executeDelayed(delayTime: int, task: Task): Promise<Any> {
        return executeDelayedImpl(delayTime, task);
    }

    /**
     * Delayed execution of a concurrent task with specified priority
     *
     * @param { int } delayTime timeout in ms to execute task after
     * @param { Task } task The task to execute
     * @param { Priority } priority specified priority for task
     * @returns { Promise<Any> } Promise for result of executed task
     */
    export function executeDelayed(delayTime: int, task: Task, priority: Priority): Promise<Any> {
        return executeDelayedImpl(delayTime, task, priority);
    }

    function executeDelayedImpl(delayTime: int, task: Task, priority?: Priority): Promise<Any> {
        initWorkerPoolSync();
        const internalTask = InternalTask.of(task);
        internalTask.checkExecutionForDelayedTask(delayTime);
        internalTask.captureAsyncStack();
        internalTask.isSubmitted = true;
        ConcurrencyHelpers.lockGuard(mutex, () => {
            pendingDelayed.add(task);
        });
        while(true) {
            let curState = InternalTask.of(task).state.load();
            if (curState == State.CANCELED) {
                break;
            }
            if (InternalTask.of(task).state.compareAndSwap(curState, InternalState.DELAYED) == curState) {
                break;
            }
        }
        return new Promise<Any>((resolve, reject) => {
            InternalTask.of(task).timerId = setTimeout(() => {
                ConcurrencyHelpers.lockGuard(mutex, () => {
                    pendingDelayed.delete(task);
                });
                if (InternalTask.of(task).timerId == -1 || isCanceled(task)) {
                    ConcurrencyHelpers.lockGuard(mutex, () => {
                        pendingDelayedCanceled.delete(task);
                    });
                    reject(new Error("taskpool:: task has been canceled"));
                    return;
                }
                if (isTaskPoolUseLaunch) {
                    launchImpl(task, priority).then((res: Any) => {
                        resolve(res);
                    }).catch((e) => {
                        reject(e);
                    })
                } else {
                    const executionRecord = InternalTask.of(task).createExecutionRecord();
                    const p = executionRecord.promise;
                    InternalTask.of(task).enqueue(priority, executionRecord);
                    p.then((res: Any) => {
                        resolve(res);
                    }).catch((e) => {
                        reject(e);
                    })
                }
            }, delayTime);
        });
    }

    /**
     * Execute a task periodically with default priority
     *
     * @param { int } period period in ms to execute task
     * @param { Task } task The task to execute
     */
    export function executePeriodically(period: int, task: Task): void {
        return executePeriodicallyImpl(period, task);
    }

    /**
     * Execute a task periodically with specified priority
     *
     * @param { int } period period in ms to execute task
     * @param { Task } task The task to execute
     * @param { Priority } priority priority for task
     */
    export function executePeriodically(period: int, task: Task, priority: Priority): void {
        return executePeriodicallyImpl(period, task, priority);
    }

    function executePeriodicallyImpl(period: int, task: Task, priority?: Priority): void {
        initWorkerPoolSync();
        const internalTask = InternalTask.of(task);
        internalTask.checkExecutionForPeriodTask(period);
        internalTask.captureAsyncStack();
        internalTask.isSubmitted = true;
        internalTask.isPeriodicTask = true;
        internalTask.timerId = setInterval(() => {
            if (internalTask.timerId == -1) {
                return;
            }
            taskSubmitted(task);
            if (isTaskPoolUseLaunch) {
                launchImpl(task, priority);
            } else {
                const executionRecord = InternalTask.of(task).createExecutionRecord();
                InternalTask.of(task).enqueue(priority, executionRecord);
            }
        }, period);
    }

    /**
     * Cancel a concurrent task
     *
     * @param taskId The id of task for canceling
     * @throws Error if the task does not exist when it is not executed or has been canceled
     */
    export function cancel(taskId: int): void {
        let task: Task | undefined = undefined;
        for (let t of allTasks.keys()) {
            if (t.taskId == taskId) {
                task = t;
                break;
            }
        }
        if (task === undefined) {
            let err = new Error(TASKPOOL_ERROR_TASK_NOT_EXECUTED_TASK, "taskpool:: task is not executed or has been executed");
            throw err;
        }
        cancel(task!);
    }

    /**
     * Cancel a concurrent task
     *
     * @param task The task for canceling
     * @throws Error if the task does not exist when it is canceled
     * @see Task.isCancel
     */
    export function cancel(task: Task): void {
        tryCancelDependentTasks(task);
        // 1. task has been canceled skip it
        // 2. task is waiting or running, cancel it
        let timerId = -1;
        if (InternalTask.of(task).timerId !== -1) {
            timerId = InternalTask.of(task).timerId;
            if (InternalTask.of(task).isPeriodicTask) {
                clearInterval(InternalTask.of(task).timerId);
            }
            InternalTask.of(task).timerId = -1;
        }
        if (timerId != -1) {
            // canceled task set state to canceled, avoid duplicate execution
            InternalTask.of(task).state.store(State.CANCELED.valueOf());
            return;
        }
        while(true) {
            let curState = InternalTask.of(task).state.load();
            if (curState == InternalState.CANCELED) {
                return;
            }
            if (curState == InternalState.NOT_FOUND) {
                throw new Error(TASKPOOL_ERROR_TASK_NOT_EXECUTED_TASK, "taskpool:: task is not executed or has been executed");
            }
            if (curState == InternalState.FINISHED) {
                if (InternalTask.of(task).seqId.load() != 0) {
                    throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTED, "taskpool:: sequenceRunner task has been executed");
                }
                if (InternalTask.of(task).asyncId.load() != 0) {
                    throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_TASK_EXECUTED, "taskpool:: asyncRunner task has been executed")
                }
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_BASIC, "taskpool:: task has been executed");
            }
            if (InternalTask.of(task).state.compareAndSwap(curState, InternalState.CANCELED) == curState) {
                cleanupCanceledPendingDependencyTask(task);
                return;
            }
        }
    }

    /**
     * Cancel a concurrent task group
     *
     * @param group The group for canceling
     * @throws Error if the task group does not exist when it is canceled
     */
    export function cancel(group: TaskGroup): void {
        let groupState = GroupState.of(group).state_.load();
        if (groupState != (InternalState.RUNNING.valueOf()) && groupState != (InternalState.WAITING.valueOf())) {
            throw new Error(TASKPOOL_ERROR_TASK_NOT_EXECUTED_GROUP, "taskpool:: taskGroup is not executed or has been executed");
        }
        GroupState.of(group).state_.store(State.CANCELED.valueOf());
        GroupState.of(group).tasks.forEach((task: Task) => {
            InternalTask.of(task).state.store(State.CANCELED.valueOf());
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
        initWorkerPoolSync();
        const taskInfos: Array<TaskInfo> = new Array<TaskInfo>();
        const threadInfos: Array<ThreadInfo> = new Array<ThreadInfo>();
        ConcurrencyHelpers.mutexLock(mutex);
        // collect all tasks waiting and running
        for (let task of allTasks.keys()) {
            if (InternalTask.of(task).state.load() == InternalState.FINISHED.valueOf()) {
                // should not enter this branch, task will be removed from allTasks map after finished
                continue;
            }
            const taskInfo: TaskInfo = new TaskInfo();
            taskInfo.taskId = task.taskId;
            taskInfo.name = task.name;
            let state = InternalTask.of(task).state.load();
            if (state == 1) {
                taskInfo.state = State.WAITING;
            } else if (state == 2) {
                taskInfo.state = State.RUNNING;
            } else if (state == 3) {
                taskInfo.state = State.CANCELED;
            } else if (state == 4) {
                taskInfo.state = State.DELAYED;
            }
            taskInfo.duration = InternalTask.of(task).getTaskInfoDuration();
            taskInfos.push(taskInfo);
        }
        // Collect delay and canceled tasks info
        pendingDelayed.forEach((task: Task) => {
            const taskInfo: TaskInfo = new TaskInfo();
            taskInfo.taskId = task.taskId;
            taskInfo.name = task.name;
            let state = InternalTask.of(task).state.load();
            if (state == State.WAITING.valueOf()) {
                taskInfo.state = State.WAITING;
            } else if (state == State.RUNNING.valueOf()) {
                taskInfo.state = State.RUNNING;
            } else if (state == State.CANCELED.valueOf()) {
                taskInfo.state = State.CANCELED;
            } else if (state == State.DELAYED.valueOf()) {
                taskInfo.state = State.DELAYED;
            }
            taskInfo.duration = InternalTask.of(task).getTaskInfoDuration();
            taskInfos.push(taskInfo);
        });
        // Collect worker threads info
        if (!isTaskPoolUseLaunch) {
            workers.forEach(worker=>{
                const threadInfo: ThreadInfo = new ThreadInfo();
                threadInfo.tid = worker.workerId;
                threadInfo.priority = worker!.currentPriority;
                ConcurrencyHelpers.lockGuard(worker.currentTasksMutex, () => {
                    const tasksCount = worker.currentTasks.size.toInt();
                    if (tasksCount == 0) {
                        return;
                    }
                    const taskIds: int[] = new int[tasksCount](0);
                    let currentIndex: int = 0;
                    worker.currentTasks.forEach((_count: int, task: Task) => {
                        taskIds[currentIndex] = task.taskId;
                        currentIndex++;
                    });
                    threadInfo.taskIds = taskIds;
                });
                threadInfos.push(threadInfo);
            })
        }
        ConcurrencyHelpers.mutexUnlock(mutex);
        const Info: TaskPoolInfo = new TaskPoolInfo();
        const taskInfosRes: TaskInfo[] = new TaskInfo[taskInfos.length](new TaskInfo());
        const threadInfosRes: ThreadInfo[] = new ThreadInfo[threadInfos.length](new ThreadInfo());
        taskInfos.forEach((taskInfo: TaskInfo, index: int) => {
            taskInfosRes[index.toInt()] = taskInfo;
        });
        threadInfos.forEach((threadInfo: ThreadInfo, index: int) => {
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
        const count: int | undefined = waitingTasks.get(task);
        waitingTasks.set(task, count == undefined ? 1 : count + 1);
    }

    function popFromMap<T extends Object>(item: T, currentTasksMap: containers.ConcurrentHashMap<T, int>, possibleTasksMap: containers.ConcurrentHashMap<T, int>, cancelSet: containers.ConcurrentSet<T>): boolean {
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

    /**
     * Get task by taskId and taskName(if have).
     * @returns The task
     */
    export function getTask(taskId: int, taskName?: string): Task | undefined {
        let task: Task | undefined = undefined;
        let currentWorkerId = CoroutineExtras.getWorkerId();
        for(const t: Task of taskSet.keys()) {
            if (t.taskId == taskId) {
                task = t;
                break;
            }
        }

        if (task !== undefined) {
            let internalTask = InternalTask.of(task!);
            if (internalTask.creatorWorkerId != currentWorkerId) {
                return undefined;
            }
        }

        if (taskName !== undefined && task !== undefined && task!.name != taskName) {
            return undefined;
        }
        return task;
    }

    function getCurrentTask(): Task | undefined {
        let task: Task | undefined = undefined;
        let taskId = getTaskId();
        let workerId = CoroutineExtras.getWorkerId();
        let currentWorker:Array<GlobalQueueWorker> = [];
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            currentWorker = filter<GlobalQueueWorker>(workers, (worker: GlobalQueueWorker) => worker.workerId == workerId);
        });
        if (currentWorker.length == 0) {
            return task;
        }
        ConcurrencyHelpers.lockGuard(currentWorker[0].currentTasksMutex, () => {
            for (let t: Task of currentWorker[0].currentTasks.keys()) {
                if (t.taskId == taskId) {
                    task = t;
                    break;
                }
            }
        });
        return task;
    }

    function filter<T>(s: Set<T>, cb: (t: T, idx: int, arr: Array<T>) => boolean): Array<T> {
        return Array.from(s).filter(cb);
    }

    function isCanceled(task: Task): boolean {
        return InternalTask.of(task).state.load() == InternalState.CANCELED.valueOf();
    }

    function getTaskCanceledError(internalTask: InternalTask): Error {
        if (internalTask.groupId != 0) {
            return new Error(TASKPOOL_ERROR_GROUP_CANCELED, "taskpool:: taskGroup has been canceled");
        }
        if (internalTask.seqId.load() != 0) {
            return new Error(TASKPOOL_ERROR_SEQ_RUNNER_CANCELED, "taskpool:: sequenceRunner task has been canceled");
        }
        if (internalTask.asyncId.load() != 0) {
            return new Error(TASKPOOL_ERROR_ASYNC_RUNNER_CANCELED, "taskpool:: asyncRunner task has been canceled");
        }
        return new Error(TASKPOOL_ERROR_TASK_CANCELED, "taskpool:: task has been canceled");
    }

    function finishCanceledPendingDependencyTask(task: Task): void {
        waitingTasksNum.fetchSub(1);
        ConcurrencyHelpers.lockGuard(mutex, () => {
            const count = allTasks.get(task) ?? 0;
            if (count <= 1) {
                allTasks.delete(task);
            } else {
                allTasks.set(task, count - 1);
            }
        });

        let internalTask = InternalTask.of(task);
        let cancelError = getTaskCanceledError(internalTask);
        ConcurrencyHelpers.lockGuard(internalTask.taskMutex, () => {
            internalTask.currentExecutionRecord?.markCanceled();
            internalTask.currentExecutionRecord?.rejectPromise(cancelError);
            internalTask.currentExecutionRecord = undefined;
            while (internalTask.pendingExecutionRecords.length > 0) {
                let executionRecord = internalTask.pendingExecutionRecords.shift();
                executionRecord?.markCanceled();
                executionRecord?.rejectPromise(cancelError);
            }
        });
    }

    function takePendingDependencyQueuedExecution(task: Task): QueuedExecution | undefined {
        let queuedExecution: QueuedExecution | undefined = undefined;
        ConcurrencyHelpers.lockGuard(pendingDependencyTasksMutex, () => {
            queuedExecution = pendingDependencyTasks.get(task);
            if (queuedExecution != undefined) {
                pendingDependencyTasks.delete(task);
            }
        });
        return queuedExecution;
    }

    function cleanupCanceledPendingDependencyTask(task: Task): boolean {
        let queuedExecution = takePendingDependencyQueuedExecution(task);
        if (queuedExecution == undefined) {
            return false;
        }
        finishCanceledPendingDependencyTask(task);
        return true;
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
            workers.forEach((worker: GlobalQueueWorker) => {
                worker.closeWorker();
            });
            workers.clear();
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
     * @brief Set blocked worker threshold in ms.
     * If a worker stays inside a synchronous task execution longer than this threshold,
     * managerWorker will attempt a special expansion.
     * @param thresholdMs The blocked threshold value in milliseconds.
     */
    function setTaskPoolBlockedWorkerThreshold(thresholdMs: int): void {
        if (thresholdMs <= 0) {
            return;
        }
        blockedWorkerThresholdMs = thresholdMs;
    }

    /**
     * @brief Set blocked worker monitor interval in ms.
     * managerWorker checks blocked workers periodically with this interval.
     * @param intervalMs The monitor interval value in milliseconds.
     */
    function setTaskPoolBlockedWorkerMonitorInterval(intervalMs: int): void {
        if (intervalMs <= 0) {
            return;
        }
        blockedWorkerMonitorIntervalMs = intervalMs;
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
     * @brief Get current taskpool workers limit for debug.
     * @returns Current TaskPool workers limit.
     */
    function getTaskPoolWorkersLimit(): int {
        return workersLimit;
    }

    /**
     * @brief Retrigger taskpool shrink in taskpool test suite for debug.
     */
    function retriggerTaskPoolShrink(): void {
        managerWorker?.retriggerShrink();
    }

    /**
     * @brief (Re)start blocked-worker expansion monitor in taskpool test suite for debug.
     *
     * NOTE: blocked-worker monitor interval is captured when the monitor is started.
     * Call this after setting blocked-worker threshold/interval if taskpool is already initialized.
     */
    function retriggerTaskPoolBlockedExpandMonitor(): void {
        managerWorker?.retriggerBlockedExpandMonitor();
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
                let w = new GlobalQueueWorker(isTaskpoolSupportInterop);
                workers.add(w);
            }
        })
    }

    /**
     * @brief Initialize taskpool worker pool in manager worker avoid blocking main thread for long time
     * @see taskpool.execute
     * @see taskpool.Task.constructor
     * @see taskpool.TaskGroup.constructor
     * @see taskpool.SequenceRunner.constructor
     * @see taskpool.AsyncRunner.constructor
     */
    function initWorkerPool(): void {
        if (isWorkersInitialized.get() || isTaskPoolUseLaunch || isInitTaskLaunched.get()) {
                return;
        }

        launch() {
            ConcurrencyHelpers.lockGuard(workersMutex, () => {
                if (isWorkersInitialized.get() || isTaskPoolUseLaunch) {
                    return;
                }

                if (managerWorker === undefined) {
                    // Create managerWorker, and start to trigger shrink.
                    managerWorker = new GlobalQueueWorker();
                    managerWorker.retriggerShrink();
                    // Start blocked-worker expansion monitor separately from shrink logic.
                    managerWorker.retriggerBlockedExpandMonitor();
                }

                // Init taskpool worker pool.
                for (let i = 0; i < initialWorkersNumber; ++i) {
                    let w = new GlobalQueueWorker(isTaskpoolSupportInterop);
                    workers.add(w);
                }
                for(let i = 0; i < waitingTasksNum.load(); i++) {
                    ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
                        ConcurrencyHelpers.condVarNotifyOne(globalTaskCondVar, globalTaskQueuesMutex);
                    });
                }
                isWorkersInitialized.set(true);
            })
        };
        isInitTaskLaunched.set(true);

    }

    /**
     * @brief Initialize taskpool worker sequential mode for avoid missing task
     */
    function initWorkerPoolSync(): void {
        if (isWorkersInitialized.get() || isTaskPoolUseLaunch) {
                return;
        }
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            if (isWorkersInitialized.get() || isTaskPoolUseLaunch) {
                return;
            }

            if (managerWorker === undefined) {
                // Create managerWorker, and start to trigger shrink.
                managerWorker = new GlobalQueueWorker();
                managerWorker!.retriggerShrink();
                // Start blocked-worker expansion monitor separately from shrink logic.
                managerWorker!.retriggerBlockedExpandMonitor();
            }

            for (let i = 0; i < initialWorkersNumber; ++i) {
                let w = new GlobalQueueWorker(isTaskpoolSupportInterop);
                workers.add(w);
            }
            isWorkersInitialized.set(true);
        });
    }

    /**
     * @brief Try to trigger expand.
     * @see taskpool.notifyExecuteTask
     */
    function tryTriggerExpand(): void {
        if (workers.size >= workersLimit) {
            return;
        }
        managerWorker?.triggerExpand();
    }

    /**
     * @brief Update taskpool worker state.
     * @see taskpool.TaskPoolWorker.performTask
     */
    function removeFromIdleWorkers(worker: GlobalQueueWorker): void {
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            removeFromIdleWorkersLocked(worker);
        });
    }

    function removeFromIdleWorkersLocked(worker: GlobalQueueWorker): boolean {
        for (let w: GlobalQueueWorker of idleWorkers) {
            if (worker == w) {
                idleWorkers.delete(w);
                w.idleTime = 0;
                return true;
            }
        }
        return false;
    }

    function addToIdleWorkers(worker: GlobalQueueWorker): void {
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            addToIdleWorkersLocked(worker);
        });
    }

    function addToIdleWorkersLocked(worker: GlobalQueueWorker): void {
        for (let w: GlobalQueueWorker of idleWorkers) {
            if (worker == w) {
                return;
            }
        }
        worker.idleTime = Chrono.nanoNow();
        idleWorkers.add(worker);
    }

    function initGlobalTaskQueues(): void {
        if (isGlobalTaskQueuesInitialized.get()) {
            return;
        }
        globalTaskQueue = new Map<Priority, containers.stackless.LinkedConcurrentQueue<QueuedExecution>>();
        globalTaskQueue.set(Priority.HIGH, new containers.stackless.LinkedConcurrentQueue<QueuedExecution>());
        globalTaskQueue.set(Priority.MEDIUM, new containers.stackless.LinkedConcurrentQueue<QueuedExecution>());
        globalTaskQueue.set(Priority.LOW, new containers.stackless.LinkedConcurrentQueue<QueuedExecution>());
        globalTaskQueue.set(Priority.IDLE, new containers.stackless.LinkedConcurrentQueue<QueuedExecution>());

        isGlobalTaskQueuesInitialized.set(true);
    }

    // Temporarily reuse the HIGH queue for USER_INTERACTION and DEADLINE_REQUEST; dedicated queues will be added later.
    function normalizeTaskQueuePriority(priority: Priority): Priority {
        if (priority == Priority.USER_INTERACTION || priority == Priority.DEADLINE_REQUEST) {
            return Priority.HIGH;
        }
        return priority;
    }

    /**
     * @brief Get task from new task queues with priority logic
     * Uses condition variable to wait when no tasks are available to avoid busy waiting
     * @returns Task if task has no dependency, undefined - otherwise.
     */
    function getTaskFromGlobalQueue(worker: GlobalQueueWorker): QueuedExecution | undefined {
        // First try non-blocking priority selection
        // here get task with attempts is used to avoid missing tasks
        let queuedExecution = getTaskWithAttempts();
        if (queuedExecution != undefined) {
            waitingTasksNum.fetchSub(1);
            return queuedExecution;
        }
        
        // No tasks available, wait on condition variable to avoid CPU busy waiting
        // Only wait if worker is active
        if (!worker.isWorkerActive()) {
            return undefined;
        }
        ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
            // Double-check after acquiring lock to avoid race condition
            queuedExecution = getTaskWithAttempts();
        });
        if (queuedExecution != undefined) {
            waitingTasksNum.fetchSub(1);
            return queuedExecution;
        }
        if (!worker.isWorkerActive()) {
            return undefined;
        }
        if (worker.hasRetireRequested()) {
            worker.tryAdvanceRetireRequest();
            return undefined;
        }
        addToIdleWorkers(worker);
        ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
            // Re-check after publishing idle state to avoid missing a wake-up.
            queuedExecution = getTaskWithAttempts();
            if (queuedExecution === undefined && worker.isWorkerActive() && !worker.hasRetireRequested()) {
                Coroutine.Schedule();
                ConcurrencyHelpers.condVarWait(globalTaskCondVar, globalTaskQueuesMutex);
            }
        });
        // After waking up, try again (task might still be undefined due to spurious wakeup)
        if (queuedExecution != undefined) {
            waitingTasksNum.fetchSub(1);
            return queuedExecution;
        }
        worker.tryAdvanceRetireRequest();
        return undefined;
    }

    function getTaskWithAttempts(): QueuedExecution | undefined {
        let attempts = continuousExecutionCount;
        while (attempts > 0) {
            let queuedExecution = selectTaskByPriorityNonBlocking();
            if (queuedExecution != undefined) {
                return queuedExecution;
            }
            attempts--;
        }
        return undefined;
    } 
    /**
    * @brief Non-blocking priority-based task selection using LinkedConcurrentQueue.poll()
     * Uses poll() instead of pop() to avoid blocking
     */
    function selectTaskByPriorityNonBlocking(): QueuedExecution | undefined {
        let queuedExecution: QueuedExecution | undefined = undefined;
        
        // HIGH priority tasks (with continuous execution limit)
        if (getGlobalTaskQueueSize(Priority.HIGH) > 0) {
            if (globalHighTaskExecuteCount.load() < continuousExecutionCount) {
                globalHighTaskExecuteCount.fetchAdd(1);
                queuedExecution = getGlobalTaskImplNonBlocking(Priority.HIGH);
            } else {
                globalHighTaskExecuteCount.store(0);
            }
        }
        
        // MEDIUM priority tasks (with continuous execution limit)
        if (getGlobalTaskQueueSize(Priority.MEDIUM) > 0 && queuedExecution === undefined) {
            if (globalMediumTaskExecuteCount.load() < continuousExecutionCount) {
                globalMediumTaskExecuteCount.fetchAdd(1);
                queuedExecution = getGlobalTaskImplNonBlocking(Priority.MEDIUM);
            } else {
                globalMediumTaskExecuteCount.store(0);
            }
        }
        
        // LOW priority tasks
        if (getGlobalTaskQueueSize(Priority.LOW) > 0 && queuedExecution === undefined) {
            queuedExecution = getGlobalTaskImplNonBlocking(Priority.LOW);
        }
        
        // IDLE priority tasks (non-blocking check)
        if (getGlobalTaskQueueSize(Priority.IDLE) > 0 && queuedExecution === undefined) {
            queuedExecution = getGlobalTaskImplNonBlocking(Priority.IDLE);
        }
        
        return queuedExecution;
    }


    /**
    * @brief Non-blocking implementation helper using poll() instead of pop()
     * @param priority The priority queue to get task from
     * @returns Task if found and executable, undefined otherwise
     */
    function moveToPendingDependencyQueue(queuedExecution: QueuedExecution): boolean {
        let moved = false;
        const task = queuedExecution.task;
        let internalTask = InternalTask.of(task);
        ConcurrencyHelpers.lockGuard(internalTask.taskMutex, () => {
            if (internalTask.taskDependenciesCount == 0) {
                return;
            }
            ConcurrencyHelpers.lockGuard(pendingDependencyTasksMutex, () => {
                pendingDependencyTasks.set(task, queuedExecution);
            });
            moved = true;
        });
        return moved;
    }

    function removePendingDependencyTask(task: Task): void {
        ConcurrencyHelpers.lockGuard(pendingDependencyTasksMutex, () => {
            pendingDependencyTasks.delete(task);
        });
    }

    function tryActivatePendingDependencyTask(task: Task): boolean {
        let isReady = false;
        let internalTask = InternalTask.of(task);
        ConcurrencyHelpers.lockGuard(internalTask.taskMutex, () => {
            isReady = (internalTask.taskDependenciesCount == 0);
        });
        if (!isReady) {
            return false;
        }

        let queuedExecution = takePendingDependencyQueuedExecution(task);
        if (queuedExecution == undefined) {
            return false;
        }
        if (internalTask.state.load() == InternalState.CANCELED.valueOf()) {
            finishCanceledPendingDependencyTask(task);
            return false;
        }

        while (!globalTaskQueue.get(normalizeTaskQueuePriority(queuedExecution.priority))?.add(queuedExecution)) {
            Coroutine.Schedule();
        }
        ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
            ConcurrencyHelpers.condVarNotifyOne(globalTaskCondVar, globalTaskQueuesMutex);
        });
        return true;
    }

    function getGlobalTaskImplNonBlocking(priority: Priority): QueuedExecution | undefined {
        let queue = globalTaskQueue.get(normalizeTaskQueuePriority(priority));
        if (queue == undefined || queue.size == 0) {
            return undefined;
        }
        
        let queuedExecution = queue.poll(); // Non-blocking poll instead of blocking pop
        if (queuedExecution == undefined) {
            return undefined;
        }
        const task = queuedExecution.task;
        // Check if task has dependencies
        if (InternalTask.of(task!).taskDependenciesCount == 0) {
            return queuedExecution;
        } else {
            if (!moveToPendingDependencyQueue(queuedExecution)) {
                return queuedExecution;
            }
        }
        return undefined;
    }

    function requestRetireCleanup(): void {
        managerWorker?.run<void>(() => {
            managerWorker!.cleanupReadyWorkers();
        });
    }

    /**
     * @brief Get total count of tasks across all new task queues
     * @returns Total number of tasks waiting
     */
    function getNewTaskTotalCount(): int {
        let total = 0;
        total += getGlobalTaskQueueSize(Priority.HIGH);
        total += getGlobalTaskQueueSize(Priority.MEDIUM);
        total += getGlobalTaskQueueSize(Priority.LOW);
        total += getGlobalTaskQueueSize(Priority.IDLE);
        return total;
    }

    /**
     * @brief Get size of specific priority queue
     * @param priority The priority queue to check
     * @returns Number of tasks in the queue
     */
    function getGlobalTaskQueueSize(priority: Priority): int {
        let queue = globalTaskQueue.get(normalizeTaskQueuePriority(priority));
        return queue?.size ?? 0;
    }

    /**
     * @brief Notify waiting workers that new tasks are available
     */
    function notifyNewTaskAvailable(): void {
        ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
            ConcurrencyHelpers.condVarNotifyAll(globalTaskCondVar, globalTaskQueuesMutex);
        });
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
    function launchQueuedExecutionRecord(internalTask: InternalTask, executionRecord: ExecutionRecord): Promise<Any> {
        launch(() => {
            try {
                internalTask.execute(undefined, executionRecord);
            } catch (e) {
                executionRecord.markTaskError(e as Error);
                internalTask.finishExecutionRecord(executionRecord);
            }
        });
        return executionRecord.promise;
    }

    function launchTaskExecution(task: Task): ExecutionRecord {
        const internalTask = InternalTask.of(task);
        const executionRecord = internalTask.createExecutionRecord();
        launchQueuedExecutionRecord(internalTask, executionRecord);
        return executionRecord;
    }

    function launchImpl(task: Task, priority?: Priority): Promise<Any> {
        return launchTaskExecution(task).promise;
    }

    native function isUsingLaunch(): boolean;
    native function isSupportingInterop(): boolean;
    native function getDefaultTaskPoolWorkersLimit(): int;
    native function currentWorkerHasPendingLocalJobs(): boolean;
    native function getTaskId(): int;
    native function setTaskId(taskid: int): void;
    native function getSubmissionId(): int;
    native function setSubmissionId(submissionId: int): void;
    native function setCurrentTaskpoolWorkerPriority(priority: int): void;
    native function collectAsyncStack(): long;
    native function getSubmitterStackId(): long;
    native function setSubmitterStackId(stackId: long): void;

    // managed mutex for taskpool containers below
    const mutex: Object = ConcurrencyHelpers.mutexCreate();
    // value is count of tasks
    const waitingTasks: containers.ConcurrentHashMap<Task, int> = new containers.ConcurrentHashMap<Task, int>();
    const runningTasks: containers.ConcurrentHashMap<Task, int> = new containers.ConcurrentHashMap<Task, int>();
    const pendingDelayed: containers.ConcurrentSet<Task> = new containers.ConcurrentSet<Task>();
    const pendingDelayedCanceled: containers.ConcurrentSet<Task> = new containers.ConcurrentSet<Task>();
    const tasksToBeCanceled: containers.ConcurrentSet<Task> = new containers.ConcurrentSet<Task>();

    const workersMutex: Object = ConcurrencyHelpers.mutexCreate();
    let workers: Set<GlobalQueueWorker> = new Set<GlobalQueueWorker>();
    let idleWorkers: containers.ConcurrentSet<GlobalQueueWorker> = new containers.ConcurrentSet<GlobalQueueWorker>();
    let isWorkersInitialized: AtomicFlag = new AtomicFlag(false);
    let isInitTaskLaunched: AtomicFlag = new AtomicFlag(false);

    const taskQueuesMutex: Object = ConcurrencyHelpers.mutexCreate();
    let isGlobalTaskQueuesInitialized: AtomicFlag = new AtomicFlag(false);
    let taskQueues: Map<Priority, Array<Task>>;
    let globalTaskQueue: Map<Priority, containers.stackless.LinkedConcurrentQueue<QueuedExecution>>;
    let pendingDependencyTasks: Map<Task, QueuedExecution> = new Map<Task, QueuedExecution>();

    // New task queue management variables
    const globalTaskQueuesMutex: Object = ConcurrencyHelpers.mutexCreate();
    const globalTaskCondVar: Object = ConcurrencyHelpers.condVarCreate();
    const globalTaskCounterMutex: Object = ConcurrencyHelpers.mutexCreate();
    const pendingDependencyTasksMutex: Object = ConcurrencyHelpers.mutexCreate();
    let globalHighTaskExecuteCount: AtomicInt = new AtomicInt(0);
    let globalMediumTaskExecuteCount: AtomicInt = new AtomicInt(0);

    // all tasks in the taskpool
    let allTasks: containers.ConcurrentHashMap<Task, int> = new containers.ConcurrentHashMap<Task, int>();
    let taskSet: containers.ConcurrentSet<Task> = new containers.ConcurrentSet<Task>();
    // manage the taskGroup
    let allGroups: containers.ConcurrentHashMap<int, TaskGroup> = new containers.ConcurrentHashMap<int, TaskGroup>();

    const isTaskPoolUseLaunch: boolean = isUsingLaunch();
    const isTaskpoolSupportInterop: boolean = isSupportingInterop();

    // If taskpool supports interop, reduce initialWorkersNumber to minimize VM creation
    const initialWorkersNumber: int = isTaskpoolSupportInterop ? 1 : 2;
    const continuousExecutionCount: int = 5;

    const taskCountMutex: Object = ConcurrencyHelpers.mutexCreate();
    let waitingTasksNum: AtomicInt = new AtomicInt(0);
    let totalExecCount: AtomicInt = new AtomicInt(0);
    let totalExecTime: AtomicInt = new AtomicInt(0);
    let managerWorker: GlobalQueueWorker | undefined = undefined;
    let triggerInterval: int = 30000; // 30s is the default shrink interval.
    let idleThreshold: int = 60000; // If the worker is idle for more than 60s, it will be released.

    // Special expansion when a worker is blocked inside synchronous task execution.
    // Threshold (ms) after which a worker is considered blocked.
    let blockedWorkerThresholdMs: int = 3000;
    // Monitor interval (ms) for managerWorker to check blocked workers.
    let blockedWorkerMonitorIntervalMs: int = 1000;

    let workersLimit: int = getDefaultTaskPoolWorkersLimit();
    const STEP_SIZE: int = 2;
    const SHRINK_STEP: int = 4; // 4: try to release 4 threads every time.
    const TASK_DURATION: int = 100; // 100: 100ms.
    const TRIGGER_EXPAND_RETRY_MS: int = 10; // Same order as native TRIGGER_EXPAND_INTERVAL.
    const WORKERS_MINIMUM: int = 1; // 1: minimum thread num when idle.
}

