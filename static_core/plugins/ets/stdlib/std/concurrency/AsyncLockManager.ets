
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

package std.concurrency;

/**
 * Information about all lock operations on the AsyncLock instance.
 */
class AsyncLockManager {

    internal constructor() {}

    /**
    * Find or create an instance of AsyncLock using the specified name.
    *
    * @param { string } name - name of the lock to find or create.
    * @returns { AsyncLock } Returns an instance of AsyncLock.
    */
    public request(name: string): AsyncLock {
        ConcurrencyHelpers.mutexLock(this.mutex);
        let lock = this.locks.get(name);
        if (lock == undefined) {
            lock = new AsyncLock(name);
        }
        ConcurrencyHelpers.mutexUnlock(this.mutex);
        return lock;
    }

    /**
    * Query information about the specified lock.
    *
    * @param { string } name - name of the lock.
    * @returns { AsyncLockState } Returns an instance of AsyncLockState.
    */
    public query(name: string): AsyncLockState {
        try{
            ConcurrencyHelpers.mutexLock(this.mutex);
            let lock = this.locks.get(name);
            if (lock == undefined) {
                throw new Error("AsyncLock " + name + " not found");
            }
            let state = lock.query();
            return state;
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    /**
    * Query information about all locks.
    *
    * @returns { AsyncLockState[] } Returns an array of AsyncLockState.
    */
    public queryAll(): AsyncLockState[] {
        ConcurrencyHelpers.mutexLock(this.mutex);
        let result = new AsyncLockState[this.locks.size as int];  // Statement "as int" to be removed after fixing issue 19773.
        let idx = 0;
        this.locks.forEach((val: AsyncLock, key: string): void => {
            result[idx++] = val.query();
        });
        ConcurrencyHelpers.mutexUnlock(this.mutex);
        return result;
    }

    internal locks: Map<string, AsyncLock> = new Map<string, AsyncLock>();
    private mutex: Object = ConcurrencyHelpers.mutexCreate();
}

const asyncLockManager = new AsyncLockManager();

/**
 * Information about all lock operations on the AsyncLock instance.
 */
export class AsyncLockState {

    constructor() {
        this.held = [];
        this.pending = [];
    }

    constructor(held: Set<AsyncLockInfo>, pending: Set<AsyncLockInfo>) {
        this.held = this.toArray(held);
        this.pending = this.toArray(pending);
    }

    private toArray(set: Set<AsyncLockInfo>) : AsyncLockInfo[] {
        let array = new AsyncLockInfo[set.size as int]; // Statement "as int" to be removed after fixing issue 19773.
        let idx = 0;
        set.forEach((val: AsyncLockInfo): void => {
            array[idx++] = val;
        });
        return array;
    }

    /**
    * Held locks information.
    */
    public held: AsyncLockInfo[];

    /**
    * Pending locks information.
    */
    public pending: AsyncLockInfo[];
}

/**
 * Information about a lock.
 */
export class AsyncLockInfo {

    constructor() {
        this.name = "";
        this.mode = AsyncLockMode.EXCLUSIVE;
        this.contextId = -1;
    }

    constructor(name: string, mode: AsyncLockMode, contextId: number) {
        this.name = name;
        this.mode = mode;
        this.contextId = contextId;
    }

    /**
    * Name of the lock.
    */
    name: string;

    /**
    * Lock operation mode.
    */
    mode: AsyncLockMode;

    /**
    * lockAsync caller's execution context identifier.
    */
    contextId: number;
}

/**
 * Lock operation's options.
 */
export class AsyncLockOptions<T = undefined> {
    constructor() {
        this.isAvailable = false;
        this.signal = null;
        this.timeout = 0;
    }

    constructor(isAvailable: boolean, signal: AbortSignal<T> | null, timeout: number) {
        this.isAvailable = isAvailable;
        this.signal = signal;
        this.timeout = timeout;
    }

    /**
     * If the value is true and lockAsync cannot acquire the lock immediately, the operation is canceled.
     */
    isAvailable: boolean;

    /**
     * The object used to abort the async operation. If signal.aborted is true, the callback will not be called.
     */
    signal: AbortSignal<T> | null;

    /**
     * Lock operation timeout in milliseconds. If it is greater than zero, lockAsync will reject the resulting promise
     * when the timeout is exceeded.
     */
    timeout: number;
}

/**
 * Object used to abort an async operation.
 * An instance of this class must be accessed in the same thread where the instance is created.
 * Access to fields of this class from another thread is undefined behaviour.
 */
export class AbortSignal<T> {
    /**
     * Set to true to abort an operation.
     */
    public aborted: boolean;

    /**
     * Reason for the abort. This value will be used to reject the promise returned from lockAsync.
     */
    public reason: T
}
