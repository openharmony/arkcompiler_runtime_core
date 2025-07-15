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
        this(AsyncLock.generateAnonymousLockName());
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
        let asyncLockInfo = new AsyncLockInfo(this.name, mode, callerCID);

        let wrapped = (l: AsyncLock) => { return l.wrapper<T, U>(callback, callerCID, mode, options); }
        if (this.canAcquireLock(mode, callerCID, asyncLockInfo)) {
            wrapped = (l: AsyncLock) => { return l.syncwrapper<T, U>(callback, asyncLockInfo, options); }
        } else {
            ConcurrencyHelpers.mutexLock(this.mutex);
            this.pending.add(asyncLockInfo);
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }

        return AsyncLock.launchPromise(wrapped, this);
    }

    private canAcquireLock<T>(mode: AsyncLockMode, callerCID: int, asyncLockInfo: AsyncLockInfo) : boolean {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            if (this.holderCID == 0 && this.mode == AsyncLockMode.UNLOCKED) {
                this.counter = 1;
                this.held.add(asyncLockInfo);
                this.holderCID = callerCID;
                this.mode = mode;
                return true;
            }
            if (mode == AsyncLockMode.SHARED && this.mode == AsyncLockMode.SHARED && this.holderCID == callerCID) {
                this.counter++;
                this.held.add(asyncLockInfo);
                this.holderCID = callerCID;
                this.mode = mode;
                return true;
            }
            return false;
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    private syncwrapper<T,U>(callback: AsyncLockCallback<T>, asyncLockInfo: AsyncLockInfo, options: AsyncLockOptions<U>) : Promise<T> {
        if (options.signal != null && options.signal!.aborted == true) {
            throw new Error("Aborted");
        }
        return Promise.resolve<T>(callback()).finally(() => {this.unlock(asyncLockInfo); });
    }

    private static launchPromise<T>(f: Function, ...args: FixedArray<NullishType>): Promise<T> {
        let resolver: (((v: T | PromiseLike<T>) => void) | undefined) = undefined;
        let rejecter: (((error: Error) => void) | undefined) = undefined;
        let p = new Promise<T>((res, rej) => {
            resolver = res
            rejecter = rej;
        })
        let cb = () => {
            try {
                let v = f.unsafeCall(...args) as T
                resolver!(v)
            } catch (e: Error) {
                rejecter!(e)
            }
        }
        launchSameWorker(cb)
        return p;
    }

    private wrapper<T, U>(callback: AsyncLockCallback<T>, callerCID: int, mode: AsyncLockMode, options: AsyncLockOptions<U>) : Promise<T> {
        let asyncLockInfo = new AsyncLockInfo(this.name, mode, callerCID);
        this.lock<U>(callerCID, mode, asyncLockInfo, options);
        return Promise.resolve<T>(callback()).finally(() => {this.unlock(asyncLockInfo); });
    }

    private lock<U>(callerCID: int, mode: AsyncLockMode, asyncLockInfo: AsyncLockInfo, options: AsyncLockOptions<U>): void {
        let isDone = false;
        let timeout = options.timeout;
        let isTimeoutExec: boolean = false;

        if (timeout) {
           setTimeout(() => {
               isTimeoutExec = true;
           }, timeout);
        }

        while (isDone == false) {
            if (options.signal != null && options.signal!.aborted == true) {
                throw new Error("Aborted");
            }

            if(isTimeoutExec == true) {
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
        try {
            return asyncLockManager.query(name);
        } catch (e: Error) {
            throw e;
        }
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

    private static generateAnonymousLockName() {
        return "__AnonymousLockName__" + Number.toString(AsyncLock.LastAnonymousId++);
    }

    public readonly name: string;
    private static LastAnonymousId: int = 0;
    private holderCID: int = 0;
    private mode: AsyncLockMode = AsyncLockMode.UNLOCKED;
    private counter: int = 0;
    private mutex: Object = ConcurrencyHelpers.mutexCreate();
    private held: Set<AsyncLockInfo> = new Set<AsyncLockInfo>();
    private pending: Set<AsyncLockInfo> = new Set<AsyncLockInfo>();
}
