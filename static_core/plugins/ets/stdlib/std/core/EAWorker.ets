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

package std.core;

import { InteropSerializeHelper } from "std/interop";

type Task = () => void;
type ErrorHandlerType = (error: Error) => void;

/**
 * @enum defines the worker priority
 */
export enum WorkerPriority {
    PRIORITY_IDLE = 0,
    PRIORITY_LOW = 1,
    PRIORITY_DEFAULT = 2,
    PRIORITY_HIGH = 3,
    PRIORITY_DEADLINE_REQUEST = 4,
    PRIORITY_USER_INTERACTION = 5,
}

const INVALID_WORKER_ID = -1;
const MAIN_WORKER_ID = 0;

/**
 * This class is needed to run Task in exclusive mode:
 * elaunch method creates a new worker and this task will be scheduled on it.
 * All child coroutines will be scheduled on the same worker.
 * A JS runtime is also created if the needInterop parameter is set to true
 */
class ExclusiveLauncher {
    /**
     * Run task in exclusive mode
     * Creates a new worker
     *
     * @param { Task } task will be scheduled on created worker
     * @param { string } name the name of the worker
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     * @returns { int } the id of the worker
     */
    static native elaunch(task: Object, joiningPromise: Promise<void>, name: string, needInterop: boolean): int;
};

/**
 * Helper class for ExclusiveLauncher that allows to use
 * the same exclusive worker for several tasks
 */
export class EAWorker {
    private worker: InternalWorker;
    private priority: WorkerPriority = WorkerPriority.PRIORITY_HIGH;
    private loopStarted = new AtomicFlag(false);
    private loopJoined = new AtomicFlag(false);
    private initialTask: Task = () => {};
    private static workers = new containers.ConcurrentHashMap<int, EAWorker>();
    private static activeUserWorkers = new AtomicInt(0);
    private isTaskpool = false;
    private userWorkerSlotReserved = new AtomicInt(0);

    /**
     * Create EAWorker instance
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     */
    constructor(needInterop: boolean = false) {
        this("EAWorker", needInterop);
    }

    /**
     * Create EAWorker instance with specified name
     * @param { string } name The name of the worker
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     */
    constructor(name: string, needInterop: boolean = false) {
        this.worker = needInterop ? new InteropWorker(name) : new StaticWorker(name);
    }

    /**
     * Create EAWorker instance with specified name and whether it is a taskpool worker
     * Internal usage only
     * @param { string } name The name of the worker
     * @param { boolean } supportInterop true if need to create JS runtime
     * @param { boolean } isTaskpool true if the worker is a taskpool worker
     */
    protected constructor(name: string, supportInterop: boolean, isTaskpool: boolean) {
        this.isTaskpool = isTaskpool;
        this.worker = supportInterop ? new InteropWorker(name) : new StaticWorker(name);
    }

    /**
     * Create EAWorker instance with specified task
     * @param { Task } task The task function to execute
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     */
    constructor(task: Task, needInterop: boolean = false) {
        this("EAWorker", task, needInterop);
    }

    /**
     * Create EAWorker instance with specified name and task
     * @param { string } name The name of the worker
     * @param { Task } task The task function to execute
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     */
    constructor(name: string, task: Task, needInterop: boolean = false) {
        this.worker = needInterop ? new InteropWorker(name) : new StaticWorker(name);
        this.initialTask = task;
    }

    // NOTE(panferovi): make this constructor private
    protected constructor(worker: MainWorker) {
        this.worker = worker;
        this.loopStarted.set(true);
        EAWorker.workers.set(this.worker.getWorkerId(), this);
    }

    /**
     * Create EAWorker instance with specified name and supportInterop and whether it is a taskpool worker
     * Internal usage only
     * @param { string } name The name of the worker
     * @param { Object } task The initial task to execute
     * @param { boolean } supportInterop true if need to create JS runtime
     * @param { boolean } isTaskpool true if the worker is a taskpool worker
     */
    protected constructor(task: Object, name: string, supportInterop: boolean = false, isTaskpool: boolean = false) {
        this.isTaskpool = isTaskpool;
        this.initialTask = task as Task;
        this.worker = supportInterop ? new InteropWorker(name) : new StaticWorker(name);
    }

    protected setInitialTask(task: Object): void {
        this.initialTask = task as Task;
    }

    /**
     * Get all registered EAWorker instances.
     *
     * @returns { containers.ConcurrentHashMap<int, EAWorker> }
     *   A concurrent hash map containing all EAWorker instances,
     *   keyed by their integer identifiers.
     */
    private static getWorkers(): containers.ConcurrentHashMap<int, EAWorker> {
        return EAWorker.workers;
    }

    /**
     * Get the worker identifier
     * @returns { int } The unique identifier of the worker
     */
    public getWorkerId(): int | undefined {
        let wId = this.worker.getWorkerId();
        return wId != INVALID_WORKER_ID ? wId : undefined;
    }

    /**
     * Get the name of the worker
     * @returns { string } The name of the worker
     */
    public getName(): string {
        return this.worker.getName();
    }

    /**
     * Start the worker execution
     * Begins processing tasks in the worker thread
     */
    public start(): void {
        if ((this.loopStarted.get() || this.loopJoined.get()) && !this.isMain()) {
            throw new Error('EAWorker:: Worker already started or quit');
        }
        this.reserveUserWorkerSlot();
        try {
            this.worker.start(() => {
                try {
                    EAWorker.getWorkers().set(CoroutineExtras.getWorkerId(), this);
                    let wrappedInitialTask = this.worker.wrapTask<void>(this.initialTask);
                    wrappedInitialTask();
                } catch (e) {
                    this.releaseUserWorkerSlot();
                    throw e;
                }
            });
        } catch (e) {
            this.releaseUserWorkerSlot();
            throw e;
        }
        if (this.worker.getWorkerId() != INVALID_WORKER_ID) {
            EAWorker.getWorkers().set(this.worker.getWorkerId(), this);
        }
        this.worker.setPriority(this.priority);
        this.loopStarted.set(true);
    }

    /**
     * Check if the worker is currently alive and running
     * @returns { boolean } True if the worker is alive, false otherwise
     */
    public isAlive(): boolean {
        return this.loopStarted.get() && !this.loopJoined.get();
    }

    /**
     * Set the priority of the worker
     * @param { WorkerPriority } priority The new priority value for the worker
     */
    public setPriority(priority: WorkerPriority): void {
        if (this.loopStarted.get()) {
            this.worker.setPriority(priority);
        }
        this.priority = priority;
    }

    /**
     * Get the current priority of the worker
     * @returns { WorkerPriority } The current priority value of the worker
     */
    public getPriority(): WorkerPriority {
        return this.priority;
    }

    /**
     * Quit the worker
     * Waits for pending tasks to complete before stopping the worker
     */
    public quit(): void {
        let p = this.join();
        p.awaitSync();
    }

    /**
     * Set the uncaught exception handler for the worker
     * @param { (error: Error) => void } handler The exception handler function
     */
    public setUncaughtExceptionHandler(handler: (error: Error) => void): void {
        this.worker.setErrorHandler(handler);
    }

    /**
     * Get the current uncaught exception handler
     * @returns { (error: Error) => void } The current exception handler function
     */
    public getUncaughtExceptionHandler(): ErrorHandlerType | undefined {
        return this.worker.getErrorHandler();
    }

    /**
     * Get the main worker instance
     * @returns { EAWorker } The main worker instance
     */
    public static main(): EAWorker {
        return mainEAWorker;
    }

    /**
     * Get the current worker instance
     * @returns { EAWorker } The current worker instance
     */
    public static current(): EAWorker | undefined {
        let currentWorker = EAWorker.getWorkers().get(CoroutineExtras.getWorkerId());
        if (currentWorker !== undefined && currentWorker.isTaskpool) {
            throw new Error('EAWorker:: Cannot get current worker in taskpool');
        }
        return currentWorker;
    }

    /**
     * Run task in EAWorker
     * Note: it has no effect if EAWorker was stoped
     *
     * @param { Function } task function to execute
     * @param { ...Any } args argumnents for task to execute with
     * @returns { Promise<R> } Promise that represents the task
     */
    public run<R> (task: Function, ...args: FixedArray<Any>): Promise<R> {
        if (!this.loopStarted.get()) {
            throw new Error('EAWorker:: Cannot run task when worker is not started');
        }
        return this.worker.run<R>(task, ...args);
    }

    public static postToMain<R>(coroFun: Function, launchParams: LaunchParams, ...args: FixedArray<Any>) : Promise<R> {
        return EAWorker.postTaskToMain<R>(coroFun, args as FixedArray<Any>, launchParams.abortFlag);
    }

    public static postToMain<R>(coroFun: Function, ...args: FixedArray<Any>) : Promise<R> {
        return EAWorker.postTaskToMain<R>(coroFun, args as FixedArray<Any>, false);
    }
    /**
     * Post task to main worker
     * Note: it has no effect if EAWorker was stoped
     */
    private static postTaskToMain<R>(coroFun: Function, args: FixedArray<Any>, abort: boolean) : Promise<R> {
        let wrappedTask = (): R => {
            return coroFun.unsafeCall(...args) as R;
        }
        let lParams: LaunchParams = {
            workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.EXACT_ID, [MAIN_WORKER_ID]),
            abortFlag: abort
        };
        return launch(lParams, wrappedTask)
    }

    /**
     * Stop EAWorker
     * This method closes run queue and detaches worker
     * Note: it's required to call join for EAWorker
     * @returns { Promise<void> } Promise that resolves when the worker has joined
     */
    public join(): Promise<void> {
        if (!this.loopStarted.get()) {
             throw new Error('EAWorker:: Cannot stop worker before start');
        }
        let finalTask = () => {
            EAWorker.getWorkers().delete(this.worker.getWorkerId());
            this.releaseUserWorkerSlot();
        };
        let joinPromise = this.worker.join(finalTask);
        this.loopStarted.set(false);
        this.loopJoined.set(true);
        return joinPromise;
    }

    /**
     * Post task to EAWorker
     * @param { Task } task The task to post
     */
    public postTask(task: Task):void {
        if (this.isMain()) {
            EAWorker.postToMain<void>(task);
            return;
        }
        if (!this.loopStarted.get()) {
            throw new Error('EAWorker:: Cannot post task when worker is not started');
        }
        this.worker.run<void>(task);
    }

    private isMain() {
        return this === EAWorker.main();
    }

    private reserveUserWorkerSlot(): void {
        if (this.isTaskpool || this.isMain()) {
            return;
        }
        if (this.userWorkerSlotReserved.compareAndSwap(0, 1) != 0) {
            throw new Error('EAWorker:: Worker already started or quit');
        }
        const limit = CoroutineExtras.getExclusiveWorkersLimit();
        let reserved = false;
        while (!reserved) {
            const current = EAWorker.activeUserWorkers.load();
            if (current >= limit) {
                this.userWorkerSlotReserved.store(0);
                throw new CoroutinesLimitExceedError("The limit of Exclusive Workers has been reached");
            }
            if (EAWorker.activeUserWorkers.compareAndSwap(current, current + 1) == current) {
                reserved = true;
            }
        }
    }

    private releaseUserWorkerSlot(): void {
        if (this.userWorkerSlotReserved.exchange(0) == 1) {
            EAWorker.activeUserWorkers.fetchSub(1);
        }
    }

    private static handleInteropEnvError() {
        let currentWorker = EAWorker.current();
        currentWorker?.handleInteropEnvErrorInternal();
    }

    private handleInteropEnvErrorInternal() {
        this.loopStarted.set(false);
        this.loopJoined.set(true);
        EAWorker.getWorkers().delete(this.worker.getWorkerId());
        this.releaseUserWorkerSlot();
        this.worker.handleCreateInteropEnvError();
        EAWorker.postToMain<void>(() => {
            throw new Error('EAWorker:: Cannot create EAWorker support interop without JsEnv');
        });
    }
};

export abstract class InternalWorker {
    private readonly name: string;
    protected workerId: int;
    private errorHandler: ErrorHandlerType | undefined = undefined;
    private joiningPromise = new Promise<void>((res) => {});

    constructor(name: string, workerId: int) {
        this.name = name;
        this.workerId = workerId;
    }

    run<R>(task: Function, ...args: FixedArray<Any>): Promise<R> {
        return this.runImpl<R>((): R => { return task.unsafeCall(...args) as R; });
    }

    abstract start(initialTask: Task): void;

    protected startImpl(initialTask: Task, needInterop: boolean): void {
        let wrappedTask = () => {
            this.workerId = CoroutineExtras.getWorkerId();
            initialTask();
        }
        this.workerId = ExclusiveLauncher.elaunch(wrappedTask, this.joiningPromise, this.getName(), needInterop);
    }

    join(finalTask: Task): Promise<void> {
        this.runImpl<void>(finalTask);
        InternalWorker.join(this.workerId);
        return this.joiningPromise;
    }

    abstract handleCreateInteropEnvError(): void;

    getName(): string {
        return this.name;
    }

    getWorkerId(): int {
        return this.workerId;
    }

    setPriority(priority: WorkerPriority): void {
        this.runImpl<void>(() => { InternalWorker.setCurrentWorkerPriority(priority); });
    }

    setErrorHandler(handler: ErrorHandlerType): void {
        this.errorHandler = handler;
    }

    getErrorHandler(): ErrorHandlerType | undefined {
        return this.errorHandler;
    }

    public wrapTask<R>(task: () => R): () => R {
        let wrappedTask = (): R => {
            try {
                return task();
            } catch (e) {
                if (this.getErrorHandler() != undefined) {
                    // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                    this.getErrorHandler()!(e as Error);
                }
                throw e;
            }
        };
        return wrappedTask;
    }

    protected runImpl<R>(task: () => R): Promise<R> {
        let wGroup: LaunchParams = {workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.EXACT_ID, [this.workerId])};
        return launch(wGroup, this.wrapTask<R>(task))
    }

    /**
     * Set the priority of the worker
     * @param { int } priority The priority value for the worker
     */
    private static native setCurrentWorkerPriority(priority: int): void;

    private static native join(workerId: int): void;
}

class InteropWorker extends InternalWorker {
    private interopInitialized: AtomicFlag = new AtomicFlag(false);
    private pendingTasks: containers.stackless.ConcurrentQueue<PendingTaskHandler> =
        new containers.stackless.LinkedConcurrentQueue<PendingTaskHandler>();

    constructor(name: string) {
        super(name, INVALID_WORKER_ID);
    }

    start(initialTask: Task) {
        let wrappedTask = () => {
            try {
                initialTask();
            } finally {
                this.interopInitialized.set(true);
                this.processPendingTasks();
            }
        }
        super.startImpl(wrappedTask, true);
    }

    private processPendingTasks() { 
        while (this.pendingTasks.size > 0) {
            let deferredTask = this.pendingTasks.poll();
            if (deferredTask !== undefined) {
                deferredTask.runTask();
            }
        }
    }

    public override run<R>(task: Function, ...args: FixedArray<Any>): Promise<R> {
        if (this.interopInitialized.get()) {
            return this.runInternal<R>(task, args);
        }

        return new Promise<R>((resolve: (value: R) => void, reject: (error: Error) => void) => {
            let pendingTask: Task = () => {
                try {
                    let serializeHelper = new InteropSerializeHelper(args);
                    let res = task.unsafeCall(...serializeHelper.deserialize()) as R;
                    resolve(res);
                } catch (e) {
                    reject(e as Error);
                }
            }
            let failJob: ErrorHandlerType = (err: Error) => {
                reject(err);
            }
            let deferredTask = new PendingTaskHandler(failJob, pendingTask);
            this.pendingTasks.add(deferredTask);
        });
    }

    private runInternal<R>(task: Function, args: FixedArray<Any>): Promise<R> {
        let serializeHelper = new InteropSerializeHelper(args);
        let wrappedTask = (): R => {
            return task.unsafeCall(...serializeHelper.deserialize()) as R;
        }
        return super.runImpl<R>(wrappedTask);
    }

    handleCreateInteropEnvError() {
        let err = new Error('EAWorker:: Cannot create EAWorker support interop without JsEnv');
        while (this.pendingTasks.size > 0) {
            let deferredTask = this.pendingTasks.poll();
            if (deferredTask !== undefined) {
                deferredTask.failJob(err);
            }
        }
    }
}


class PendingTaskHandler {
    private failJobHandler: ErrorHandlerType;
    private task: Task;

    constructor (failJobHandler: ErrorHandlerType, task: Task) {
        this.failJobHandler = failJobHandler;
        this.task = task;
    }

    public failJob(err: Error) {
        this.failJobHandler(err);
    }

    public runTask() {
        this.task();
    }
}

class StaticWorker extends InternalWorker {
    constructor(name: string) {
        super(name, INVALID_WORKER_ID);
    }

    start(initialTask: Task) {
        super.startImpl(initialTask, false);
    }

    handleCreateInteropEnvError() {
        throw new Error("EAWorker:: Static worker does not support interop");
    }
}

class MainEAWorker extends EAWorker {
    constructor() {
        super(new MainWorker());
    }
}

export class MainWorker extends InternalWorker {
    constructor() {
        super("main", MAIN_WORKER_ID);
    }

    start(initialTask: Task): void {
        throw new Error('EAWorker:: Main worker cannot be started');
    }

    join(finalTask: Task): Promise<void> {
        throw new Error('EAWorker:: Main worker cannot be stopped');
    }

    handleCreateInteropEnvError(): void {
        throw new Error("EAWorker:: Main worker do not need to handle create interop env error");
    }

    override setPriority(priority: WorkerPriority): void {
        throw new Error('EAWorker:: Main worker cannot set priority');
    }

    override setErrorHandler(handler: ErrorHandlerType): void {
        throw new Error('EAWorker:: Main worker cannot have uncaught exception handler');
    }

    override getErrorHandler(): ErrorHandlerType | undefined {
        return undefined;
    }
}

const mainEAWorker: EAWorker;
