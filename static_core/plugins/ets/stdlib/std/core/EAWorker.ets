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

type Task = () => void;

/**
 * This class is needed to run Task in exclusive mode:
 * elaunch method creates a new worker and this task will be scheduled on it.
 * All child coroutines will be scheduled on the same worker.
 * A JS runtime is also created if the needInterop parameter is set to true
 */
export class ExclusiveLauncher {
    public static elaunch(task: Task, needInterop: boolean = false): void {
        let eTask = needInterop ? () => { ExclusiveLauncher.asyncCall(task); } : task;
        ExclusiveLauncher.elaunch(eTask as Object, needInterop);
    }

    private static async asyncCall(task: Task) {
        task();
    }

    private static native elaunch(task: Object, needInterop: boolean): void;
};

/**
 * Helper class for ExclusiveLauncher that allows to use
 * the same exclusive worker for several tasks 
 */
export class EAWorker {
    private worker: InternalWorker;

    constructor(needInterop: boolean = false) {
        this.worker = needInterop ? new InteropWorker() : new StaticWorker();
    }

    /**
     * Run task in EAWorker
     * Note: it has no effect if EAWorker was stoped
     */
    public run<T>(task: () => T): Promise<T> {
        return this.worker.run(task);
    }

    /**
     * Stop EAWorker
     * This method closes run queue and detaches worker
     * Note: it's required to call stop for EAWorker 
     */
    public stop() {
        this.worker.stop();
    }
};

interface InternalWorker {
    run<T>(task: () => T): Promise<T>;

    stop();
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

    constructor() {
        this.initEvent = new Event();
        ExclusiveLauncher.elaunch(() => this.body(), true);
    }
    
    public run<T>(task: () => T): Promise<T> {
        let resolveFn: (val: T) => void;
        let p = new Promise<T>((resolve: (val: T) => void) => {
            resolveFn = resolve;
        });
        this.initEvent.wait();
        this.poster!.post((): void => {
            InteropWorker.asyncCall(() => { resolveFn(task()); });
        });
        return p;
    }

    public stop() {
        this.initEvent.wait();
        this.poster!.destroy();
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
    private tasks: LinkedBlockingQueue<Task>;
    private static closingTask: Task = (): void => {};

    constructor() {
        this.tasks = new LinkedBlockingQueue<Task>();
        ExclusiveLauncher.elaunch(() => this.body(), false);
    }

    public run<T>(task: () => T): Promise<T> {
        let resolveFn: (val: T) => void;
        let rejectFn: (error: NullishType) => void;
        let p = new Promise<T>((resolve: (val: T) => void, reject: (error: NullishType) => void) => {
            resolveFn = resolve;
            rejectFn = reject;
        });
        this.tasks.push((): void => {
            try {
                resolveFn(task());
            } catch(e){
                rejectFn(e);
            }
        });
        return p;
    }

    public stop() {
        this.tasks.push(StaticWorker.closingTask);
    }

    private body() {
        while (true) {
            let task = this.tasks.pop();
            if (task === StaticWorker.closingTask) {
                break;
            }
            launch task();
        }
    }
}