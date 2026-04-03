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
 * The concurrency.stackless module provides synchronization APIs for stackless coroutines.
 *
 * @namespace concurrency.stackless
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24 static
 */
export namespace concurrency.stackless {

/**
 * Condition variable implementation for OS-based mutexes used by stackless containers.
 *
 * @class AsyncCondVarBasedMutex
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24 static
 */
export final class AsyncCondVarBasedMutex {
    constructor() {
        this.waitersList_ = new WaitersList();
        this.waiters_ = 0;
    }

    /**
     * Atomically releases the held OS mutex, suspends the current coroutine,
     * and then re-acquires the mutex after wakeup.
     *
     * @param { Object } mutex The OS mutex object that must be locked before calling wait().
     * @returns { Promise<void> } A promise that resolves after the coroutine is resumed,
     *     and after the mutex is re-acquired.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24 static
     *
     * pre-condition: mutex is locked.
     * post-condition: mutex is locked.
     */
    public async wait(mutex: Object): Promise<void> {
        ++this.waiters_;
        ConcurrencyHelpers.mutexUnlock(mutex);
        await this.waitersList_.suspend();
        ConcurrencyHelpers.mutexLock(mutex);
    }

    /**
     * Unblocks ONE suspended coroutine associated with this CondVar, if it exists.
     *
     * @returns { void } No return value.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24 static
     *
     * pre-condition: mutex is locked.
     * post-condition: mutex is locked.
     */
    public notifyOne(): void {
        if (this.waiters_ != 0) {
            this.waitersList_.resume();
            this.waiters_--;
        }
    }

    /**
     * Unblocks ALL suspended coroutine associated with this CondVar.
     *
     * @returns { void } No return value.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24 static
     *
     * pre-condition: mutex is locked.
     * post-condition: mutex is locked.
     */
    public notifyAll(): void {
        while (this.waiters_ != 0) {
            this.waitersList_.resume();
            this.waiters_--;
        }
    }

    private waitersList_: WaitersList;
    private waiters_: int;
}

}