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

package std.core;

import { launch } from "std/concurrency";
import { InteropSerializeHelper } from "std/interop";

type Task = () => void;
type ErrorHandler = (error: Error) => void;

/**
* @enum defines the message status
*/
enum MessageStatus {
    NOT_EXECUTED = 0,
    EXECUTED = 1,
    CANCELLED = 2
}

/**
 * @enum defines the worker priority
 */
export enum WorkerPriority {
    PRIORITY_IDLE = 0,
    PRIORITY_LOW = 1,
    PRIORITY_DEFAULT = 2,
    PRIORITY_HIGH = 3,
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
     * @param { int } eaworkerNum the current count of the worker
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     * @returns { int } the id of the worker
     */
    internal static elaunch(task: Task,  name: string, eaworkerNum: int, needInterop: boolean = false): int {
        let eTask = needInterop ? () => { ExclusiveLauncher.asyncCall(task); } : task;
        return ExclusiveLauncher.elaunch(eTask as Object, needInterop, name, eaworkerNum);
    }

    private static async asyncCall(task: Task) {
        task();
    }

    private static native elaunch(task: Object, needInterop: boolean, name: string, eaworkerNum: int): int;
};


/**
 * Helper class for ExclusiveLauncher that allows to use
 * the same exclusive worker for several tasks
 */
export class EAWorker {
    static {
        let mainWorkerObj = EAWorker.createMainWorker();
        let mainWorker = mainWorkerObj as EAWorker;
        mainWorker.isMain = true;
        EAWorker.mainWorker = mainWorker;
    }

    private worker: InternalWorker;
    private readonly name: string;
    // eaworkerNum is the current count of the worker, will be used for identifying the worker in workers map
    private readonly eaworkerNum: int;
    // TODO: using managed mutex instead of concurrent hash map
    internal messages: containers.ConcurrentHashMap<concurrency.Message, MessageStatus>;
    private loopStarted: AtomicFlag = new AtomicFlag(false);
    private readonly isMain: boolean;
    private isStarted: AtomicFlag = new AtomicFlag(false);
    private initialTask: Task | undefined = undefined;
    private static mainWorker: EAWorker | undefined = undefined;
    private priority: WorkerPriority = WorkerPriority.PRIORITY_DEFAULT;
    private static workers: containers.ConcurrentHashMap<int, EAWorker> = new containers.ConcurrentHashMap<int, EAWorker>();

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
        this.name = name;
        this.eaworkerNum = EAWorker.calculateEAWorkerCount();
        this.worker = needInterop ? new InteropWorker(this.name, this.eaworkerNum) : new StaticWorker(this.name, this.eaworkerNum);
        this.messages = this.worker.getMessages();
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
     * @param { ()=>void } task The task function to execute
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     */
    constructor(name: string, task: ()=>void, needInterop: boolean = false) {
        this.name = name;
        this.eaworkerNum = EAWorker.calculateEAWorkerCount();
        this.worker = needInterop ? new InteropWorker(this.name, this.eaworkerNum) : new StaticWorker(this.name, this.eaworkerNum);
        this.messages = this.worker.getMessages();
        this.initialTask = task;
    }

    /**
     * Get the worker identifier
     * @returns { int } The unique identifier of the worker
     */
    public getWorkerId(): int | undefined {
        if (this.isMain) {
            return MAIN_WORKER_ID;
        }
        if (!this.loopStarted.get()) {
            return undefined;
        }
        return this.worker.getWorkerId();
    }

    /**
     * Get the name of the worker
     * @returns { string } The name of the worker
     */
    public getName(): string {
        if (this.isMain) {
            return "main";
        }
        return this.name;
    }

    /**
     * Start the worker execution
     * Begins processing tasks in the worker thread
     */
    public start(): void {
        if (this.isMain) {
            throw new Error("Main worker cannot be started");
        }
        if (this.isStarted.get()) {
            throw new Error("Worker already started");
        }
        this.worker.start(this.initialTask);
        this.loopStarted.set(true);
        this.isStarted.set(true);
        EAWorker.workers.set(this.eaworkerNum, this);
    }

    /**
     * Check if the worker is currently alive and running
     * @returns { boolean } True if the worker is alive, false otherwise
     */
    public isAlive(): boolean {
        if (this.isMain) {
            return true;
        }
        return this.loopStarted.get();
    }

    /**
     * Set the priority of the worker
     * @param { WorkerPriority } workerpriority The new priority value for the worker
     */
    public setPriority(workerpriority: WorkerPriority) {
        if (this.isMain) {
            throw new Error("Main worker cannot set priority");
            return;
        }
        if (!this.loopStarted.get()) {
            throw new Error("Can not set worker priority when not started");
        }
        this.worker.run<void>(() => {
            EAWorker.setCurrentWorkerPriority(workerpriority);
        });
        this.priority = workerpriority;
    }

    /**
     * Get the current priority of the worker
     * @returns { WorkerPriority } The current priority value of the worker
     */
    public getPriority(): WorkerPriority {
        if (this.isMain) {
            return WorkerPriority.PRIORITY_DEFAULT;
        }
        return this.priority;
    }

    /**
     * Quit the worker immediately
     * Stops the worker without waiting for pending tasks to complete
     */
    public quit(): void {
        if (this.isMain) {
            throw new Error("Main worker cannot be stopped");
            return;
        }
        if (!this.loopStarted.get()) {
            return;
        }
        this.worker.quit();
        this.loopStarted.set(false);
        if (EAWorker.workers.get(this.eaworkerNum) != undefined) {
            EAWorker.workers.delete(this.eaworkerNum);
        }
    }

    /**
     * Quit the worker safely
     * Waits for pending tasks to complete before stopping the worker
     */
    public quitSafely(): void {
        if (this.isMain) {
            throw new Error("Main worker cannot be stopped");
            return;
        }
        if (!this.loopStarted.get()) {
            return;
        }
        this.worker.join();
        EAWorker.workers.delete(this.eaworkerNum);
        this.loopStarted.set(false);
    }

    /**
     * Set the uncaught exception handler for the worker
     * @param { (error: Error) => void } handler The exception handler function
     */
    public setUnCaughtExceptionHandler(handler: (error: Error) => void): void {
        if (this.isMain) {
            throw new Error("Main worker cannot have uncaught exception handler");
        }
        this.worker.setErrorHandler(handler);
    }

    /**
     * Get the current uncaught exception handler
     * @returns { (error: Error) => void } The current exception handler function
     */
    public getUnCaughtExceptionHandler(): ErrorHandler | undefined {
        if (this.isMain) {
            return undefined;
        }
        return this.worker.getErrorHandler();
    }

    /**
     * Get the main worker instance
     * @returns { EAWorker } The main worker instance
     */
    public static main(): EAWorker {
        return EAWorker.mainWorker as EAWorker;
    }

    /**
     * Get the current worker instance
     * @returns { EAWorker } The current worker instance
     */
    public static current(): EAWorker | undefined {
        let currentEAWorkerNum = EAWorker.getEAWorkerNumFromThread();
        let currentWorker = EAWorker.workers.get(currentEAWorkerNum);
        if (currentWorker != undefined && currentWorker.name == "taskpool") {
            throw new Error("Can not get current worker in taskpool");
        }
        return currentWorker;
    }

    internal enqueue(message: concurrency.Message) {
        if (this.isMain) {
            EAWorker.postToMain(message.getTarget().handler, message);
        }
        this.messages.set(message, MessageStatus.NOT_EXECUTED);
        if (this.loopStarted.get()) {
            if (message != undefined) {
                this.worker.run<void>(message.getTarget().handler, message);
            }
        }
    }

    internal contains(message: concurrency.Message): boolean {
        if (this.isMain) {
            return false;
        }
        let res = false;
        this.messages.forEach((value: MessageStatus, key: concurrency.Message) => {
            if (key.equals(message) && value != MessageStatus.CANCELLED) {
                res = true;
            }
        });
        return res;
    }

    internal getMessage(message: concurrency.Message): concurrency.Message | undefined {
        if (this.isMain) {
            return undefined;
        }
        let res: concurrency.Message | undefined = undefined;
        this.messages.forEach((value: MessageStatus, key: concurrency.Message) => {
            if (key.equals(message)) {
                res = key;
            }
        });
        return res;
    }

    /**
     * Cancel a message by setting its status to CANCELLED
     * @param { Message } message The message to cancel
     * @returns { boolean } True if the message was cancelled, false otherwise
     */
    internal cancelMessage(message: concurrency.Message): boolean {
        if (this.isMain) {
            return false;
        }
        let msg = this.getMessage(message);
        if (msg != undefined) {
            this.messages.set(msg, MessageStatus.CANCELLED);
            return true;
        }
        return false;
    }

    /**
     * Get the status of a specific message
     * @param { Message } message The message to check
     * @returns { MessageStatus | undefined } The status of the message, or undefined if not found
     */
    internal getMessageStatus(message: concurrency.Message): MessageStatus | undefined {
        if (this.isMain) {
            return undefined;
        }
        let status: MessageStatus | undefined = undefined;
        this.messages.forEach((value: MessageStatus, key: concurrency.Message) => {
            if (key.equals(message)) {
                status = value;
            }
        });
        return status;
    }

    /**
     * Run task in EAWorker
     * Note: it has no effect if EAWorker was stoped
     *
     * @param { Function } task function to execute
     * @param { ...NullishType } args argumnents for task to execute with
     * @returns { Job<R> } Job that represents the task
     */
    public run<R> (task: Function, ...args: FixedArray<NullishType>): Job<R> {
        if (this.isMain) {
            return EAWorker.postToMain<R>(task, ...args);
        }
        if (!this.loopStarted.get()) {
            throw new Error("Can not run task when worker is not started");
        }
        return this.worker.run<R>(task, ...args);
    }

    public static postToMain<R>(
        coroFun: Function,
        ...args: FixedArray<NullishType>
    ) : Job<R> {
        return EAWorker.postTaskToMain<R>(coroFun, args as FixedArray<NullishType>, false);
    }

    public static postToMain<R>(
        coroFun: Function,
        launchParams: LaunchParams,
        ...args: FixedArray<NullishType>
    ) : Job<R> {
        return EAWorker.postTaskToMain<R>(coroFun, args as FixedArray<NullishType>, launchParams.abortFlag);
    }

    /**
     * Post task to main worker
     * Note: it has no effect if EAWorker was stoped
     */
    private native static postTaskToMain<R>(
      coroFun: Object,
      args: FixedArray<NullishType>,
      abortFlag: boolean
    ) : Job<R>;

    /**
     * Stop EAWorker
     * This method closes run queue and detaches worker
     * Note: it's required to call join for EAWorker
     */
    public join() {
        if (this.isMain) {
            throw new Error("Main worker cannot be stopped");
            return;
        }
        if (!this.loopStarted.get()) {
            return;
        }
        this.worker.join();
        EAWorker.workers.delete(this.eaworkerNum);
        this.loopStarted.set(false);
    }

    /**
     * Post task to EAWorker
      @param { Task } task The task to post
     */
    public postTask(task: Task):void {
        if (this.isMain) {
            EAWorker.postToMain<void>(task);
            return;
        }
        if (this.loopStarted.get()) {
            this.worker.run<void>(task);
        }
    }

    /**
     * Calculate the current EAWorker count
     * @returns { int } The current EAWorker count
     */
    private static native calculateEAWorkerCount(): int;

    /**
     * Get the current EAWorker num from thread
     * @returns { int } The current EAWorker num from thread
     */
    private static native getEAWorkerNumFromThread(): int;

    /**
     * Create the main worker
     * @returns { Object } The main worker
     */
    private static native createMainWorker(): Object;

    /**
     * Set the priority of the worker
     * @param { int } priority The priority value for the worker
     */
    private static native setCurrentWorkerPriority(priority: int): void;

};

interface InternalWorker {
    run<R>(task:Function, ...args:FixedArray<NullishType>): Job<R>;

    start(initialTask?: Task);

    quit();

    join();

    setErrorHandler(handler: ErrorHandler);

    getWorkerId(): int | undefined;

    getErrorHandler(): ErrorHandler | undefined;

    getMessages(): containers.ConcurrentHashMap<concurrency.Message, MessageStatus>;
}

class TaskPoster {
    constructor() {
        this.mutex = new Mutex();
        this.poster = TaskPoster.create();
    }

    public post(task: Task) {
        this.mutex.lock();
        if (this.poster != 0) {
            TaskPoster.post(this.poster, task as Object);
        }
        this.mutex.unlock();
    }

    public destroy() {
        this.mutex.lock();
        let poster = this.poster;
        this.poster = 0;
        this.mutex.unlock();
        TaskPoster.destroy(poster);
    }

    private static native create(): long;
    private static native destroy(poster: long): void;
    private static native post(poster: long, task: Object): void;

    private mutex: Mutex;
    private poster: long;
}

class InteropWorker implements InternalWorker {
    private poster?: TaskPoster;
    private initEvent: Event;
    private readonly name: string;
    private workerId: int = INVALID_WORKER_ID;
    // eaworkerNum is the current count of the worker, will be used for identifying the worker in workers map
    private readonly eaworkerNum: int;
    private errorHandler: ErrorHandler | undefined = undefined;
    private errHandlerMutex: QueueSpinlock = new QueueSpinlock();
    private messages: containers.ConcurrentHashMap<concurrency.Message, MessageStatus>;

    constructor(name: string, eaworkerNum: int) {
        this.name = name;
        this.eaworkerNum = eaworkerNum;
        this.initEvent = new Event();
        this.messages = new containers.ConcurrentHashMap<concurrency.Message, MessageStatus>();
    }

    getMessages(): containers.ConcurrentHashMap<concurrency.Message, MessageStatus> {
        return this.messages;
    }

    start(initialTask?: Task) {
        this.workerId = ExclusiveLauncher.elaunch(() => this.body(), this.name, this.eaworkerNum, true);
        if (initialTask !== undefined) {
            this.run<void>(initialTask);
        }
        if (this.messages.size > 0) {
            this.messages.forEach((value: MessageStatus, key: concurrency.Message) => {
                if (value != MessageStatus.CANCELLED) {
                    this.run<void>(key.getTarget().handler, key);
                }
            });
        }
    }

    getWorkerId(): int | undefined {
        return this?.workerId;
    }

    public run<R>(task:Function, ...args:FixedArray<NullishType>):Job<R> {
        let job = new CompletableJob<R>();
        this.initEvent.wait();
        let serializeHelper = new InteropSerializeHelper(args);
        let wrapperTask = ():R => {
            let deserializedArgs = serializeHelper.deserialize();
            return task.unsafeCall(...deserializedArgs) as R;
        }

        // for noraml run all task should be executed
        let shouldExecute = true;

        let currentMessage: concurrency.Message | undefined = undefined;
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof concurrency.Message) {
                currentMessage = args[i] as concurrency.Message;
                break;
            }
        }
        
        // handle message cancellation
        if (currentMessage !== undefined) {
            if (this.messages.get(currentMessage) === MessageStatus.CANCELLED) {
                shouldExecute = false;
                this.messages.delete(currentMessage);
            }
        }

        let postTask = (): void => {
            InteropWorker.asyncCall(() => {
                try {
                    if (currentMessage !== undefined) {
                        if (this.messages.get(currentMessage) != MessageStatus.CANCELLED) {
                            job.finish(wrapperTask());
                            this.messages.delete(currentMessage);
                        }
                    } else {
                        job.finish(wrapperTask());
                    }
                } catch (e) {
                    if (this.errorHandler != undefined) {
                        let errHandler = this.errorHandler as ErrorHandler;
                        // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                        errHandler(e as Error);
                    }
                    // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                    job.fail(e as Error);
                }
            });
        }

        if (shouldExecute) {
            this.poster!.post(postTask);
        }
        return job;
    }

    public join() {
        this.initEvent.wait();
        this.poster!.destroy();
    }

    public quit() {
        this.messages.forEach((value: MessageStatus, key: concurrency.Message) => {
            this.messages.set(key, MessageStatus.CANCELLED);
        });
        this.join();
    }

    setErrorHandler(handler: ErrorHandler) {
        this.errHandlerMutex.guard(() => {
            this.errorHandler = handler;
        });
    }

    getErrorHandler(): ErrorHandler | undefined {
        let errHandler: ErrorHandler | undefined = undefined;
        this.errHandlerMutex.guard(() => {
            errHandler = this.errorHandler;
        });
        return errHandler;
    }

    private body() {
        this.poster = new TaskPoster();
        this.initEvent.fire();
    }

    private static async asyncCall(task: Task) {
        task();
    }
}

class StaticWorker implements InternalWorker {
    private tasks: containers.LinkedBlockingQueue<Task>;
    private static closingTask: Task = (): void => {};
    private loopStarted: boolean = false;
    private readonly name: string;
    private workerId: int = INVALID_WORKER_ID;
    // eaworkerNum is the current count of the worker, will be used for identifying the worker in workers map
    private readonly eaworkerNum: int;
    private errHandlerMutex: QueueSpinlock = new QueueSpinlock();
    private errorHandler: ErrorHandler | undefined = undefined;
    // TODO: using maaged mutex instead of concurrent hash map
    private messages: containers.ConcurrentHashMap<concurrency.Message, MessageStatus>;
    // TODO: using managed mutex instead of concurrent hash map
    private taskToMessageMap: containers.ConcurrentHashMap<Task, concurrency.Message>;

    constructor(name: string, eaworkerNum: int) {
        this.name = name;
        this.eaworkerNum = eaworkerNum;
        this.tasks = new containers.LinkedBlockingQueue<Task>();
        this.messages = new containers.ConcurrentHashMap<concurrency.Message, MessageStatus>();
        this.taskToMessageMap = new containers.ConcurrentHashMap<Task, concurrency.Message>();
    }

    start(initialTask?: Task) {
        this.loopStarted = true;
        this.workerId = ExclusiveLauncher.elaunch(() => this.body(), this.name, this.eaworkerNum, false);
        if (initialTask !== undefined) {
            this.run<void>(initialTask);
        }
        if (this.messages.size > 0) {
            this.messages.forEach((value: MessageStatus, key: concurrency.Message) => {
                if (value != MessageStatus.CANCELLED) {
                    this.run<void>(key.getTarget().handler, key);
                }
            });
        }
    }

    getMessages(): containers.ConcurrentHashMap<concurrency.Message, MessageStatus> {
        return this.messages;
    }

    getWorkerId(): int | undefined {
        return this?.workerId;
    }

    public run<T>(task: Function, ...args:FixedArray<NullishType>): Job<T> {
        let job = new CompletableJob<T>();
        let wrapperTask = ():T => {
            return task.unsafeCall(...args) as T;
        }

        let associatedMessage: concurrency.Message | undefined = undefined;
        for (let i = 0; i < args.length; i++) {
            if (args[i] instanceof concurrency.Message) {
                associatedMessage = args[i] as concurrency.Message;
                break;
            }
        }

        let taskFunction = (): void => {
            try {
                if (associatedMessage !== undefined) {
                    if (this.messages.get(associatedMessage) != MessageStatus.CANCELLED) {
                        job.finish(wrapperTask());
                    }
                    this.messages.delete(associatedMessage);
                } else {
                    job.finish(wrapperTask());
                }
            } catch(e){
                // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                job.fail(e as Error);
                if (this.errorHandler != undefined) {
                    let errHandler = this.errorHandler as ErrorHandler;
                    // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                    errHandler(e as Error);
                }
            }
        }

        if (associatedMessage !== undefined) {
            this.taskToMessageMap.set(taskFunction, associatedMessage);
        }

        this.tasks.push(taskFunction);
        return job;
    }

    public join() {
        this.tasks.push(StaticWorker.closingTask);
    }

    public quit() {
        // clear task queue and exit
        while (this.tasks.size > 0) {
            this.tasks.poll();
        }
        this.join();
    }

    setErrorHandler(handler: ErrorHandler) {
        this.errHandlerMutex.guard(() => {
            this.errorHandler = handler;
        });
    }

    getErrorHandler(): ErrorHandler | undefined {
        let errHandler: ErrorHandler | undefined = undefined;
        this.errHandlerMutex.guard(() => {
            errHandler = this.errorHandler;
        });
        return errHandler;
    }

    private body() {
        while (true) {
            let task = this.tasks.pop();
            if (task === StaticWorker.closingTask) {
                break;
            }
            let shouldExecute = true;
            let associatedMessage = this.taskToMessageMap.get(task);
            if (associatedMessage !== undefined) {
                let messageStatus = this.messages.get(associatedMessage);
                if (messageStatus === MessageStatus.CANCELLED) {
                    shouldExecute = false;
                }
                this.taskToMessageMap.delete(task);
            }

            if (shouldExecute) {
                task(); 
            }
        }
    }
}
