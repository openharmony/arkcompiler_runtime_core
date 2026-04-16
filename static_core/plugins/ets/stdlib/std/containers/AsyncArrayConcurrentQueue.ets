/**
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

package std.containers;

/**
* The containers.stackless module provides concurrent containers for stackless coroutines.
*
* @namespace containers.stackless
* @syscap SystemCapability.Utils.Lang
*/
export namespace containers.stackless {

    class AsyncCondVarOSBasedMutex {
        constructor() {
            this.waitersList_ = new WaitersList();
            this.waiters_ = 0;
        }

        public async wait(mutex: Object): Promise<void> {
            ++this.waiters_;
            ConcurrencyHelpers.mutexUnlock(mutex);
            await this.waitersList_.suspend();
            ConcurrencyHelpers.mutexLock(mutex);
        }

        public notifyOne(mutex: Object): void {
            if (this.waiters_ != 0) {
                this.waitersList_.resume();
                this.waiters_--;
            }
        }

        public notifyAll(mutex: Object): void {
            while (this.waiters_ != 0) {
                this.waitersList_.resume();
                this.waiters_--;
            }
        }

        private waitersList_: WaitersList;
        private waiters_: int;
    }

    /**
    * ArrayConcurrentQueue is a bounded, thread-safe concurrent queue backed by a fixed-size array;
    * ArrayConcurrentQueue supports both asynchronous and non-blocking
    * insertion and removal operations.
    *
    * @class ArrayConcurrentQueue
    * @syscap SystemCapability.Utils.Lang
    */
    export class ArrayConcurrentQueue<T extends Object> implements ConcurrentQueue<T> {
        /**
        * Constructs a ArrayConcurrentQueue with provided capacity.
        *
        * @param { int } capacity the capacity of the ArrayConcurrentQueue
        * @syscap SystemCapability.Utils.Lang
        */
        constructor(capacity: int) {
            if (capacity <= 0) {
                // NOTE(ypigunova): #33368 Add typed error
                throw new RangeError("Invalid capacity for ArrayConcurrentQueue's constructor.");
            }
            this.capacitySize = capacity;
            this.array = Array.create<T|undefined>(this.capacitySize, undefined);
            this.mutex = ConcurrencyHelpers.mutexCreate();
            this.condVar = new AsyncCondVarOSBasedMutex();
        }

        /**
        * Constructs a ArrayConcurrentQueue with provided capaity from iterable.
        *
        * @param { int } capacity the capacity of the ArrayConcurrentQueue
        * @param { Iterable<T> } iterable the iterable instance to initialize ArrayConcurrentQueue
        * @syscap SystemCapability.Utils.Lang
        */
        constructor(capacity: int, iterable: Iterable<T>) {
            this(capacity)
            const entriesIter = iterable.$_iterator();
            for (let iterRes = entriesIter.next(); !iterRes.done; iterRes = entriesIter.next()) {
                const val = iterRes.value;
                if (val !== undefined) {
                    this.add(val);
                }
            }
        }

        /**
        * Adds an element to the queue, waiting asynchronously if full.
        *
        * @param { T } el Adds an element to the queue, waiting asynchronously if full.
        * @returns { Promise<void> } A Promise that resolves when the element has been successfully added to the queue.
        * @syscap SystemCapability.Utils.Lang
        */
        public async push(el: T): Promise<void> {
            ConcurrencyHelpers.mutexLock(this.mutex);
            await this.waitUntilQueueIsNotFull();
            this.array[this.curTailIdx] = el;
            this.increaseTailIdx();
            this.condVar.notifyOne(this.mutex);
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }

        /**
        * Removes and returns the first element in the ConcurrentQueue. If the queue is empty, waiting asynchronously.
        *
        * @returns { Promise<T> } A Promise that resolves when the element has been successfully removed from the queue.
        * @syscap SystemCapability.Utils.Lang
        */
        public async pop(): Promise<Awaited<T>> {
            ConcurrencyHelpers.mutexLock(this.mutex);
            await this.waitUntilQueueIsNotEmpty();
            try {
                const val = this.array[this.curHeadIdx];
                this.array[this.curHeadIdx] = undefined;
                this.increaseHeadIdx();
                this.condVar.notifyOne(this.mutex);
                return Promise.resolve(val!);
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * The insert operation. Returns true upon success and false if no space is currently available, non-blocking.
        *
        * @param { T } element the element inserted to the ConcurrentQueue
        * @returns { boolean } True upon success and false if no space is currently available.
        * @syscap SystemCapability.Utils.Lang
        */
        public override add(element: T): boolean {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                if (this.isQueueFull) {
                    return false;
                }
                this.array[this.curTailIdx] = element;
                this.increaseTailIdx();
                this.condVar.notifyOne(this.mutex);
                return true;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * The poll operation. Pop one element and return it if the queue is not empty.
        * Returns undefined when the queue is empty. Non-blocking.
        *
        * @returns { T | undefined } the deleted element or undefined when the queue is empty.
        * @syscap SystemCapability.Utils.Lang
        */
        public override poll(): T | undefined {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                if (this.isQueueEmpty) {
                    return undefined;
                }
                const val = this.array[this.curHeadIdx];
                this.array[this.curHeadIdx] = undefined;
                this.increaseHeadIdx();
                this.condVar.notifyOne(this.mutex);
                return val;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * Returns the first element in a ConcurrentQueue.
        *
        * @returns { T | undefined } the first element or undefined when the queue is empty.
        * @syscap SystemCapability.Utils.Lang
        */
        public override getFirst(): T | undefined {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                if (this.isQueueEmpty) {
                    return undefined;
                }
                const value = this.array[this.curHeadIdx];
                return value;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * Returns the end element in a ArrayConcurrentQueue.
        *
        * @returns { T | undefined } the end element or undefined when the queue is empty.
        * @syscap SystemCapability.Utils.Lang
        */
        public getEnd(): T | undefined {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                if (this.isQueueEmpty) {
                    return undefined;
                }
                let tailIdx = this.curTailIdx;
                if (this.curTailIdx == 0) {
                    tailIdx = this.capacitySize;
                }
                const value = this.array[tailIdx - 1];
                return value;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * Returns the size of an ArrayConcurrentQueue.
        *
        * @returns { int } size
        * @syscap SystemCapability.Utils.Lang
        */
        public override get size(): int {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                let size: int = 0;
                if (this.curTailIdx >= this.curHeadIdx && !this.isQueueFull) {
                    size = this.curTailIdx - this.curHeadIdx;
                } else {
                    size = this.curTailIdx - this.curHeadIdx + this.capacitySize;
                }
                return size;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * Checks if the ConcurrentQueue is empty.
        *
        * @returns { boolean } True if a ConcurrentQueue has no elements, otherwise false.
        * @syscap SystemCapability.Utils.Lang
        */
        public override isEmpty(): boolean {
            return this.size == 0;
        }

        /**
        * Returns the capacity of a ConcurrentQueue.
        *
        * @returns { int } the capacity of the ConcurrentQueue.
        * @syscap SystemCapability.Utils.Lang
        */
        public override get capacity(): int {
            return this.capacitySize;
        }

        /**
        * Returns the number of additional elements that this queue can ideally accept without blocking.
        *
        * @returns { int } the remaining capacity of the ConcurrentQueue.
        * @syscap SystemCapability.Utils.Lang
        */
        public override remainingCapacity(): int {
            return this.capacitySize - this.size;
        }

        private async waitUntilQueueIsNotEmpty() {
            while (this.isQueueEmpty) {
                // Currently, the queue is empty. Need to await.
                // This method is used for waiting until the ArrayConcurrentQueue
                // has at least one element.

                await this.condVar.wait(this.mutex);
            }
        }

        private async waitUntilQueueIsNotFull() {
            while (this.isQueueFull) {
                // Currently, the queue is full. Need to await.
                // This method is used for waiting until the ArrayConcurrentQueue
                // is not full.
                await this.condVar.wait(this.mutex);
            }
        }

        private increaseTailIdx(): void {
            this.curTailIdx++;
            this.curTailIdx = this.curTailIdx % this.capacitySize;
            if (this.isQueueEmpty) {
                this.isQueueEmpty = false;
            }
            if (this.curTailIdx == this.curHeadIdx) {
                this.isQueueFull = true;
            }
        }

        private increaseHeadIdx(): void {
            this.curHeadIdx++;
            this.curHeadIdx = this.curHeadIdx % this.capacitySize;
            if (this.isQueueFull) {
                this.isQueueFull = false;
            }
            if (this.curHeadIdx == this.curTailIdx) {
                this.isQueueEmpty = true;
            }
        }

        private array: Array<T|undefined>;                     // Array, where all ArrayConcurrentQueue elements will be located
        private curHeadIdx: int = 0;                           // Index of the first element in a ArrayConcurrentQueue
        private curTailIdx: int = 0;                           // Index of the last element in a ArrayConcurrentQueue
        private mutex: Object;                                 // push/pop Mutex
        private condVar: AsyncCondVarOSBasedMutex;             // push/pop CondVar
        private capacitySize: int = 0;
        private isQueueFull: boolean = false;
        private isQueueEmpty: boolean = true;
    }
}
