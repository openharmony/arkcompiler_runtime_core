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
 * Condition variable, that allows coroutines to wait for a specific condition
 * to become true and be notified by other coroutines when the condition changes.
 *
 * @class AsyncCondVar
 * @syscap SystemCapability.Utils.Lang
 * @since 24
 */
export class AsyncCondVar {
    constructor() {
        this.waitersList_ = new WaitersList();
        this.waiters_ = 0;
    }

    /**
     * The function atomically releases the held mutex,
     * suspends the current coroutine,
     * and then re-acquires the mutex upon wakeup.
     * This method is thread-safe in relation to other methods of the CondVar.
     *
     * @param { AsyncMutex } mutex - The mutex that must be locked when calling,
     * it will be unlocked before waiting and re-locked after wakeup
     * @returns { Promise<void> } A promise that resolves after wakeup and re-acquisition of the mutex
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     *
     * pre-condition: mutex is locked.
     * post-condition: mutex is locked.
     */
    public async wait(mutex: AsyncMutex): Promise<void> {
        ++this.waiters_;
        mutex.unlock();
        await this.waitersList_.suspend();
        await mutex.lock();
    }

    /**
     * Unblocks ONE suspended coroutine associated with this CondVar, if it exists.
     * This method is thread-safe in relation to other methods of the CondVar.
     *
     * @param { AsyncMutex } mutex - The mutex associated with the condition,
     * must be locked when calling to ensure correct synchronization
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     *
     * pre-condition: mutex is locked.
     * post-condition: mutex is locked.
     */
    public notifyOne(mutex: AsyncMutex): void {
        if (this.waiters_ != 0) {
            this.waitersList_.resume();
            this.waiters_--;
        }
    }

    /**
     * Unblocks ALL suspended coroutine associated with this CondVar.
     * This method is thread-safe in relation to other methods of the CondVar.
     *
     * @param { AsyncMutex } mutex - The mutex associated with the condition,
     * must be locked when calling to ensure correct synchronization
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     *
     * pre-condition: mutex is locked.
     * post-condition: mutex is locked.
     */
    public notifyAll(mutex: AsyncMutex): void {
        while (this.waiters_ != 0) {
            this.waitersList_.resume();
            this.waiters_--;
        }
    }

    private waitersList_: WaitersList;
    private waiters_: int;
}

}
