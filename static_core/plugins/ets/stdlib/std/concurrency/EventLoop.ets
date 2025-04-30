/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

type WorkerId = int;
type TimerId = int;
type Callback = () => void;

export function registerTimer(callback: Callback, delay: int, repeat: boolean): TimerId {
    let timer = EventLoop.createTimer(callback, delay, repeat);
    EventLoop.getOrCreate().registerTimer(timer);
    return timer.getId();
}

export function clearTimer(id: TimerId) {
    EventLoop.clearTimer(id);
}

class EventLoop {
    constructor(workerId: WorkerId) {
        this.workerId = workerId;
        this.initEvent = ConcurrencyHelpers.eventCreate();
        this.timerQueue = new Array<Timer>();
    }

    static createTimer(callback: Callback, delay: int, repeat: boolean): Timer {
        return EventLoop.timerTable.createTimer(callback, delay, repeat);
    }

    static clearTimer(id: TimerId) {
        return EventLoop.timerTable.clearTimer(id);
    }

    static getOrCreate(): EventLoop {
        return EventLoop.registry.getOrCreateEventLoop();
    }

    public init() {
        this.pollLoop();
    }

    public registerTimer(timer: Timer) {
        this.timerQueue.push(timer);
        ConcurrencyHelpers.eventFire(this.initEvent);
    }

    private async pollLoop() {
        ConcurrencyHelpers.eventWait(this.initEvent);
        this.pollTimers();
        EventLoop.registry.removeEventLoop(this.workerId);
    }

    private pollTimers() {
        while (this.timerQueue.length != 0) {
            this.updateTimers();
            this.invokeTimers();
            Coroutine.Schedule();
        }
    }

    private updateTimers() {
        EventLoop.timerTable.removeClearedTimers(this.timerQueue);
        let currentTime = Date.now();
        this.timerQueue.forEach((timer: Timer) => { timer.updateDelay(currentTime); });
    }

    private invokeTimers() {
        this.timerQueue.forEach((timer: Timer) => {
            if (!timer.isExpired()) {
                return;
            }
            timer.invoke();
            if (timer.needRepeat()) {
                timer.reset();
            } else {
                EventLoop.timerTable.clearTimer(timer.getId());
            }
        });
    }

    private workerId: WorkerId;
    private initEvent: Object;
    private timerQueue: Array<Timer>;
    private static registry = new Registry();
    private static timerTable = new TimerTable();
};

class Registry {
    constructor() {
        this.eventLoops = new Map<WorkerId, EventLoop>();
        this.registryLock = ConcurrencyHelpers.spinlockCreate();
    }

    public getOrCreateEventLoop(): EventLoop {
        let workerId = CoroutineExtras.getWorkerId();
        let eLoop: EventLoop | undefined;
        let needInit = false;
        ConcurrencyHelpers.spinlockGuard(this.registryLock, () => {
            eLoop = this.eventLoops.get(workerId);
            needInit = eLoop === undefined;
            if (needInit) {
                eLoop = new EventLoop(workerId);
                this.eventLoops.set(workerId, eLoop!);
            }
        });
        if (needInit) {
            eLoop!.init();
        }
        return eLoop!;
    }

    public removeEventLoop(workerId: WorkerId) {
        ConcurrencyHelpers.spinlockGuard(this.registryLock, () => {
            this.eventLoops.delete(workerId);
        });
    }

    private eventLoops: Map<WorkerId, EventLoop>;
    private registryLock: Object;
};

class Timer {
    constructor(callback: Callback, delay: int, repeat: boolean, id: TimerId) {
        this.callback = callback;
        this.delay = delay;
        this.repeat = repeat;
        this.id = id;
        this.startTime = Date.now();
        this.elapsedTime = 0;
    }

    public updateDelay(time: number) {
        this.elapsedTime += time - this.startTime;
        this.startTime = time;
    }

    public isExpired(): boolean {
        return this.delay <= this.elapsedTime;
    }

    public async invoke() {
        this.callback();
    }

    public getId(): TimerId {
        return this.id;
    }

    public needRepeat(): boolean {
        return this.repeat;
    }

    public reset() {
        this.startTime = Date.now();
        this.elapsedTime = 0;
    }

    private callback: Callback;
    private delay: int;
    private repeat: boolean;
    private id: TimerId;
    private startTime: number;
    private elapsedTime: number;
};

class TimerTable {
    constructor() {
        this.nextFreeId = 0;
        this.tableSpinlock = ConcurrencyHelpers.spinlockCreate();
        this.timers = new Map<TimerId, Timer>();
    }

    public createTimer(callback: Callback, delay: int, repeat: boolean): Timer {
        let timer: Timer;
        ConcurrencyHelpers.spinlockGuard(this.tableSpinlock, () => {
            let timerId = this.nextFreeId++;
            timer = new Timer(callback, delay, repeat, timerId);
            this.timers.set(timerId, timer);
        });
        return timer;
    }

    public clearTimer(id: TimerId) {
        ConcurrencyHelpers.spinlockGuard(this.tableSpinlock, () => {
            this.timers.delete(id);
        });
    }

    public removeClearedTimers(timerQueue: Array<Timer>) {
        ConcurrencyHelpers.spinlockGuard(this.tableSpinlock, () => {
            let queueLen = timerQueue.length;
            for (let i = 0; i < queueLen; ++i) {
                let timer = timerQueue.shift()!;
                if (this.timers.has(timer.getId())) {
                    timerQueue.push(timer);
                }
            }
        });
    }

    private nextFreeId: TimerId;
    private tableSpinlock: Object;
    private timers: Map<TimerId, Timer>;
};
