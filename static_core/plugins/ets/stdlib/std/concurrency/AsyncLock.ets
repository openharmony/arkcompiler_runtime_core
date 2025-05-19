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

type AsyncLockCallback<T> = () => T | Promise<T>;

export enum AsyncLockMode {
    UNLOCKED = 0,
    SHARED = 1,
    EXCLUSIVE = 2,
}

export final class AsyncLock {

    constructor() {
        this("default_name");
    }

    internal constructor(name: string) {
        this.name = name;
        asyncLockManager.locks.set(name, this);
    }

    /**
    * Find or create an instance of AsyncLock using the specified name.
    *
    * @param { string } name - name of the lock to find or create.
    * @returns { AsyncLock } Returns an instance of AsyncLock.
    */
    public static request(name: string): AsyncLock {
        return asyncLockManager.request(name);
    }

    /**
     * Perform an operation with the acquired lock exclusively.
     * The method acquires the lock first, then calls the callback, and then releases the lock.
     * The callback is called asynchronously in the same thread where lockAsync was called.
     *
     * @param { AsyncLockCallback<T> } callback - function to call when the lock gets acquired.
     * @returns { Promise<T> } Promise that will be resolved after the callback gets executed.
     */
    public lockAsync<T>(callback: AsyncLockCallback<T>): Promise<T> {
        return this.lockAsync(callback, AsyncLockMode.EXCLUSIVE, new AsyncLockOptions());
    }

    /**
     * Perform an operation with the acquired lock.
     * The method acquires the lock first, then calls the callback, and then releases the lock.
     * The callback is called asynchronously in the same thread where lockAsync was called.
     *
     * @param { AsyncLockCallback<T> } callback - function to call when the lock gets acquired.
     * @param { AsyncLockMode } mode - mode of the lock operation.
     * @returns { Promise<T> } Promise that will be resolved after the callback gets executed or rejected.
     */
    public lockAsync<T>(callback: AsyncLockCallback<T>, mode: AsyncLockMode): Promise<T> {
        return this.lockAsync(callback, mode, new AsyncLockOptions());
    }

    /**
     * Perform an operation with the acquired lock.
     * The method acquires the lock first, then calls the callback, and then releases the lock.
     * The callback is called asynchronously in the same thread where lockAsync was called.
     * An optional timeout value can be provided in {@link AsyncLockOptions}. In this case, lockAsync will reject the
     * resulting promise with a BusinessError instance if the lock is not acquired before timeout exceeds.
     * The error message, in this case, will contain the held and waited locks information and possible deadlock
     * warnings.
     *
     * @param { AsyncLockCallback<T> } callback - function to call when the lock gets acquired.
     * @param { AsyncLockMode } mode - mode of the lock operation.
     * @param { AsyncLockOptions<U> } options - lock operation options.
     * @returns { Promise<T | U> } Promise that will be resolved after the callback gets executed or rejected in case
     *     timeout exceeded.
     */
    public lockAsync<T, U>(callback: AsyncLockCallback<T>, mode: AsyncLockMode, options: AsyncLockOptions<U>): Promise<T> {
        let callerCID = CoroutineExtras.getCoroutineId();
        return launch this.wrapper<T, U>(callback, callerCID, mode, options) as Promise<T>;
    }

    private wrapper<T, U>(callback: AsyncLockCallback<T>, callerCID: int, mode: AsyncLockMode, options: AsyncLockOptions<U>) : T | Promise<T>{
        let asyncLockInfo = new AsyncLockInfo(this.name, mode, callerCID);
        ConcurrencyHelpers.mutexLock(this.mutex);
        this.pending.add(asyncLockInfo);
        ConcurrencyHelpers.mutexUnlock(this.mutex);

        this.lock<U>(callerCID, mode, asyncLockInfo, options);
        try {
            return Promise.resolve<T>(callback());
        } finally {
            this.unlock(asyncLockInfo);
        }
    }

    private lock<U>(callerCID: int, mode: AsyncLockMode, asyncLockInfo: AsyncLockInfo, options: AsyncLockOptions<U>): void {
        let isDone = false;
        let timeout = options.timeout;
        let startTime = Chrono.milliNow();

        while (isDone == false) {

            if (timeout != 0 && timeout < (Chrono.milliNow() - startTime)) {
                throw new Error("AsyncLock timeout");
            }

            ConcurrencyHelpers.mutexLock(this.mutex);

            if (this.holderCID == 0) {
                this.holderCID = callerCID;
                this.mode = mode;
                this.counter = 1;
                this.pending.delete(asyncLockInfo);
                this.held.add(asyncLockInfo);
                isDone = true;
            } else if (mode == AsyncLockMode.SHARED && this.mode == AsyncLockMode.SHARED && this.holderCID == callerCID) {
                this.counter++;
                this.pending.delete(asyncLockInfo);
                this.held.add(asyncLockInfo);
                isDone = true;
            }

            ConcurrencyHelpers.mutexUnlock(this.mutex);

            if (isDone == false) {
                if (options.isAvailable) {
                    throw new Error("AsyncLock is not available");
                } else {
                    Coroutine.Schedule();
                }
            }
        }
    }

    private unlock(asyncLockInfo: AsyncLockInfo): void {
        ConcurrencyHelpers.mutexLock(this.mutex);
        this.held.delete(asyncLockInfo);
        this.counter--;
        if (this.counter == 0) {
            this.holderCID = 0;
            this.mode = AsyncLockMode.UNLOCKED;
        }
        ConcurrencyHelpers.mutexUnlock(this.mutex);
    }

    public static query(name: string): AsyncLockState {
        return asyncLockManager.query(name);
    }

    public static queryAll(): AsyncLockState[] {
        return asyncLockManager.queryAll();
    }

    public query(): AsyncLockState {
        ConcurrencyHelpers.mutexLock(this.mutex);
        let state = new AsyncLockState(this.held, this.pending);
        ConcurrencyHelpers.mutexUnlock(this.mutex);
        return state;
    }

    public readonly name: string;
    private holderCID: int = 0;
    private mode: AsyncLockMode = AsyncLockMode.UNLOCKED;
    private counter: int = 0;
    private mutex: Object = ConcurrencyHelpers.mutexCreate();
    private held: Set<AsyncLockInfo> = new Set<AsyncLockInfo>();
    private pending: Set<AsyncLockInfo> = new Set<AsyncLockInfo>();
}
