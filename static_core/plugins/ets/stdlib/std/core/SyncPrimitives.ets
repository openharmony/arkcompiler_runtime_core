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

final class WaitersList {
    public constructor() {
        this.head = 0;
        this.tail = 0;
    }

    private head: long;
    private tail: long;
}

/// Coroutine mutex. This allows to get exclusive access to the critical section.
final class Mutex {
    public constructor() {
        this.waitersList = new WaitersList();
        this.waiters = 0;
    }

    public lockGuard(callback: () => void): void {
        this.lock();
        try {
            callback();
        } finally {
            this.unlock();
        }
    }


    /**
     * Method gives exclusive access to critical section is case of successful mutex lock.
     * Otherwise, blocks current coroutine until other coroutine will unlock the mutex.
     */
    public lock(): void {
        Mutex.lock(this);
    }

    /**
     * Calling the method means exiting the critical section.
     * If there are blocked coroutines, unlocks one of them.
     */
    public unlock(): void {
        Mutex.unlock(this);
    }

    private static native lock(mutex: Object): void;
    private static native unlock(mutex: Object): void;

    private waitersList: WaitersList;
    private waiters: int;
}

/// Coroutine one-shot event. This allows to block current coroutine until event is fired
final class Event {
    public constructor() {
        this.waitersList = new WaitersList();
        this.state = 0;
    }

    /**
     * Blocks current coroutine until another coroutine will fire the same event.
     * It can be used concurrently with other wait/fire.
     * It has no effect in case fire has already been caused, but guarantees happens-before.
     */
    public wait(): void {
        Event.wait(this);
    }

    /**
     * Unblocks all coroutines that are waiting the same event.
     * Can be used concurrently with other wait/fire but multiply calls have no effect.
     */
    public fire(): void {
        Event.fire(this);
    }

    private static native wait(event: Object): void;
    private static native fire(event: Object): void;

    private waitersList: WaitersList;
    private state: int;
}

/// Coroutine conditional variable
final class CondVar {
    public constructor() {
        this.waitersList = new WaitersList();
        this.waiters = 0;
    }

    /**
     * precondition: mutex is locked
     * 1. Unlocks mutex
     * 2. Blocks current coroutine
     * 3. Locks mutex
     * This method is thread-safe in relation to other methods of the CondVar
     */
    public wait(mutex: Mutex) {
        CondVar.wait(this, mutex);
    }

    /**
     * precondition: mutex is locked
     * Unblocks ONE suspended coroutine associated with this CondVar, if it exists.
     * This method is thread-safe in relation to other methods of the CondVar
     */
    public notifyOne(mutex: Mutex) {
        CondVar.notifyOne(this, mutex);
    }

    /**
     * precondition: mutex is locked
     * Unblocks ALL suspended coroutine associated with this CondVar.
     * This method is thread-safe in relation to other methods of the CondVar
     */
    public notifyAll(mutex: Mutex) {
        CondVar.notifyAll(this, mutex);
    }

    private static native wait(condVar: Object, mutex: Object): void;
    private static native notifyOne(condVar: Object, mutex: Object): void;
    private static native notifyAll(condVar: Object, mutex: Object): void;

    private waitersList: WaitersList;
    private waiters: int;
}

/// Queue spinlock allows to get exclusive access to the critical section without context switching
final class QueueSpinlock {
    public constructor() {
        this.tail = 0;
    }

    /**
     * The method provides exclusive access to the critical section if the lock is successfully acquired.
     * Otherwise, blocks current coroutine without context switching until other coroutine will release lock.
     */
    public guard(callback: () => void) {
        QueueSpinlock.guard(this, callback as Object);
    }

    private static native guard(spinlock: Object, callback: Object): void;

    private tail: long;
}
