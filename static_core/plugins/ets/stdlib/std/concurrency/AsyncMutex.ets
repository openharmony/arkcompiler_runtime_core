/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
* The concurrency.stackless module provides APIs for stackless coroutines.
*
* @namespace concurrency.stackless
* @syscap SystemCapability.Utils.Lang
* @since 24
*/
export namespace concurrency.stackless {

/**
 * Coroutine mutex, that ensures that only one coroutine can access the critical section.
 *
 * @class AsyncMutex
 * @syscap SystemCapability.Utils.Lang
 * @since 24
 */
export class AsyncMutex {
    constructor() {
        this.waitersList_ = new WaitersList();
        this.waiters_ = new AtomicInt(0);
    }

    /**
     * Acquires the mutex. Blocks if already locked by another coroutine.
     *
     * @returns { Promise<void> } A promise that resolves when the mutex is acquired
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public async lock(): Promise<void> {
        if (this.waiters_.fetchAdd(1) == 0) {
            return;
        }
        await this.waitersList_.suspend();
    }

    /**
     * Releases the mutex, allowing other coroutines to acquire it.
     *
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public unlock(): void {
        if (this.waiters_.load() == 0) {
            throw new IllegalLockStateError();
        }
        if (this.waiters_.fetchSub(1) == 1) {
            return;
        }
        this.waitersList_.resume();
    }

    private waitersList_: WaitersList;
    private waiters_: AtomicInt;
}

}
