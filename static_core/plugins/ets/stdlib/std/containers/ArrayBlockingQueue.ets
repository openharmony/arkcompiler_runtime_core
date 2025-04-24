/**
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

package std.containers;

/**
 * ArrayBlockingQueue implementation
 */
export class ArrayBlockingQueue<T> implements BlockingQueue<T> {

    constructor(capacity: int) {
        if (capacity <= 0) {
            throw new RangeError("Invalid capacity for ArrayBlockingQueue's constructor.");
        }
        this.capacitySize = capacity;
        this.array = new Array<T>(this.capacitySize);
        this.mutex = ConcurrencyHelpers.mutexCreate();
        this.condVar = ConcurrencyHelpers.condVarCreate();
    }

    constructor(capacity: int, iterable: Iterable<T>) {
        if (capacity <= 0) {
            throw new RangeError("Invalid capacity for ArrayBlockingQueue's constructor.");
        }
        this.capacitySize = capacity;
        this.array = new Array<T>(this.capacitySize);
        this.mutex = ConcurrencyHelpers.mutexCreate();
        this.condVar = ConcurrencyHelpers.condVarCreate();
        if (iterable) {
            const entriesIter = iterable.$_iterator();
            for (let iterRes = entriesIter.next(); !iterRes.done; iterRes = entriesIter.next()) {
                const val = iterRes.value;
                if (val !== undefined) {
                    this.add(val as T);
                }
            }
        }
    }

    // Inserts element at the end.
    override push(el: T): void {
        ConcurrencyHelpers.mutexLock(this.mutex);
        this.waitUntilQueueIsNotFull();
        this.array[this.curTailIdx] = el;
        this.increaseTailIdx();
        ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.mutex);
        ConcurrencyHelpers.mutexUnlock(this.mutex);
    }

    // Removes and returns the first element in a ArrayBlockingQueue.
    // If the ArrayBlockingQueue is empty then wait method is called.
    override pop(): T {
        ConcurrencyHelpers.mutexLock(this.mutex);
        this.waitUntilQueueIsNotEmpty();
        try {
            const val = this.array[this.curHeadIdx];
            this.increaseHeadIdx();
            return val;
        } finally {
            ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.mutex);
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    /**
     * The insert operation. Return true upon success and false if no space is currently available, no blocking.
     *
     * @param element the element inserted to the BlockingQueue
     *
     * @returns T True upon success and false if no space is currently available.
     */
    override add(element: T): boolean {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            if (this.isQueueFull) {
                return false;
            }
            this.array[this.curTailIdx] = element;
            this.increaseTailIdx();
            return true;
        } finally {
            ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.mutex);
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    /**
     * The poll operation. Pop one element and return it if the queue is not empty.
     * Return null when the queue is empty. No blocking.
     *
     * @returns Return the deleted element or null.
     */
    override poll(): T | null {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            if (this.isQueueEmpty) {
                return null;
            }
            const val = this.array[this.curHeadIdx];
            this.increaseHeadIdx();
            return val;
        } finally {
            ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.mutex);
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    /**
     * Returns the first element in a BlockingQueue.
     *
     * @returns T | null The first element or null.
     */
    override getFirst(): T | null {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            if (this.isQueueEmpty) {
                return null;
            }
            const value = this.array[this.curHeadIdx];
            return value;
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    /**
     * Returns the end element in a ArrayBlockingQueue.
     *
     * @returns return the end element or null.
     */
    public getEnd(): T | null {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            if (this.isQueueEmpty) {
                return null;
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

    // Returns size of a ArrayBlockingQueue.
    override get size(): int {
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
     * Checks if the BlockingQueue is empty.
     *
     * @returns boolean True if a BlockingQueue has no elements, otherwise false.
     */
    override isEmpty(): boolean {
        return this.size == 0;
    }

    /**
     * Returns the capacity of a BlockingQueue.
     *
     * @returns int The capacity of the BlockingQueue.
     */
    override get capacity(): int {
        return this.capacitySize;
    }

    /**
     * Returns the number of additional elements that this queue can ideally accept without blocking.
     *
     * @returns int The capacity of the BlockingQueue.
     */
    override remainingCapacity(): int {
        return this.capacitySize - this.size;
    }


    private waitUntilQueueIsNotEmpty() {
        while (this.isQueueEmpty) {
            // Currently, the queue is empty. Need to await.
            // This method is used for waiting until the ArrayBlockingQueue
            // has at least one element.
            ConcurrencyHelpers.condVarWait(this.condVar, this.mutex);
        }
    }

    private waitUntilQueueIsNotFull() {
        while (this.isQueueFull) {
            // Currently, the queue is full. Need to await.
            // This method is used for waiting until the ArrayBlockingQueue
            // is not full.
            ConcurrencyHelpers.condVarWait(this.condVar, this.mutex);
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

    private array: Array<T>;                        // Array, where all ArrayBlockingQueue elements will be located
    private curHeadIdx: int = 0;                    // Index of the first element in a ArrayBlockingQueue
    private curTailIdx: int = 0;                    // Index of the last element in a ArrayBlockingQueue
    private mutex: Object;                          // push/pop Mutex
    private condVar: Object;                        // push/pop CondVar
    private capacitySize: int = 0;
    private isQueueFull: boolean = false;
    private isQueueEmpty: boolean = true;
}
