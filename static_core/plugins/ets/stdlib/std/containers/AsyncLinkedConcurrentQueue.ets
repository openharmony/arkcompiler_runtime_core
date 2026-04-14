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
 * @FaAndStageModel
 */
export namespace containers.stackless {

    final class ListNode<T> {
        element: T | undefined = undefined;
        next: ListNode<T> | undefined = undefined;

        /**
        * Constructs an empty list node.
        *
        * @syscap SystemCapability.Utils.Lang
        */
        public constructor() { }

        /**
        * Constructs a list node with provided element.
        *
        * @param { T } element the element stored in the node
        * @syscap SystemCapability.Utils.Lang
        */
        public constructor(element: T) {
            this.element = element;
        }
    }

    /**
     * Stores elements in a thread-safe linked queue for stackless coroutines.
     * Supports both asynchronous blocking operations and immediate non-blocking operations.
     *
    * @class LinkedConcurrentQueue
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    export final class LinkedConcurrentQueue<T extends Object> implements ConcurrentQueue<T> {

        /**
         * Creates a LinkedConcurrentQueue with the default capacity.
         * Uses the implementation maximum capacity for an unbounded queue.
         *
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public constructor() {
            this.actualCapacity = Int.MAX_VALUE;
            this.initializeQueueState();
        }

        /**
         * Creates a LinkedConcurrentQueue with the provided capacity.
         * Throws a RangeError when the provided capacity is not positive.
         *
         * @param { int } capacity The maximum number of elements accepted by the queue.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public constructor(capacity: int) {
            if (capacity <= 0) {
                // #33368: Add a typed Error for AsyncLock and other Error
                throw new RangeError("LinkedConcurrentQueue:: Invalid capacity");
            }
            this.actualCapacity = capacity;
            this.initializeQueueState();
        }

        /**
         * Creates a LinkedConcurrentQueue from an iterable with the default capacity.
         * Appends each defined element from the iterable in iteration order.
         *
         * @param { Iterable<T> } iterable The iterable source used to initialize the queue.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public constructor(iterable: Iterable<T>) {
            this.actualCapacity = Int.MAX_VALUE;
            this.initializeQueueState();
            this.appendIterable(iterable);
        }

        /**
         * Creates a LinkedConcurrentQueue with the provided capacity from an iterable.
         * Appends each defined element from the iterable in iteration order.
         *
         * @param { int } capacity The maximum number of elements accepted by the queue.
         * @param { Iterable<T> } iterable The iterable source used to initialize the queue.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public constructor(capacity: int, iterable: Iterable<T>) {
            this(capacity);
            this.appendIterable(iterable as Iterable<T>);
        }

        /**
         * Inserts an element at the tail of the queue.
         * Waits asynchronously until space becomes available when the queue is full.
         *
         * @param { T } element The element to append to the LinkedConcurrentQueue.
         * @returns { Promise<void> } A promise that resolves after the element is appended,
         *     and after capacity becomes available when the queue is full.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public async push(element: T): Promise<void> {
            const newNode = new ListNode<T>(element);
            let previousSize: int = -1;
            ConcurrencyHelpers.mutexLock(this.pushMutex);
            try {
                await this.waitFullQueue();
                previousSize = this.completeEnqueueLocked(newNode);
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.pushMutex);
            }
            this.resumeConsumersIfQueueWasEmpty(previousSize);
        }

        /**
         * Removes and returns the head element from the queue.
         * Waits asynchronously until an element becomes available when the queue is empty.
         *
         * @returns { Promise<T> } A promise that resolves with the removed head element,
         *     and after an element becomes available when the queue is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public async pop(): Promise<Awaited<T>> {
            let previousSize: int = -1;
            let first: T | undefined = undefined;
            ConcurrencyHelpers.mutexLock(this.popMutex);
            try {
                await this.waitEmptyQueue();
                first = this.dequeueElementLocked();
                previousSize = this.completeDequeueLocked();
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.popMutex);
            }
            this.resumeProducersIfQueueWasFull(previousSize);
            return Promise.resolve(first!);
        }

        /**
         * Attempts to insert an element without waiting.
         * Returns false immediately when no space is currently available.
         *
         * @param { T } element The element to insert into the LinkedConcurrentQueue.
         * @returns { boolean } True when the element is inserted successfully,
         *     else false when no space is currently available.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public add(element: T): boolean {
            if (this.atomicsGetSize() == this.actualCapacity) {
                return false;
            }
            const newNode = new ListNode<T>(element);
            let previousSize: int = -1;
            ConcurrencyHelpers.mutexLock(this.pushMutex);
            try {
                if (this.atomicsGetSize() < this.actualCapacity) {
                    previousSize = this.completeEnqueueLocked(newNode);
                    return true;
                } else {
                    return false;
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.pushMutex);
                this.resumeConsumersIfQueueWasEmpty(previousSize);
            }
        }

        /**
         * Attempts to remove and return the head element without waiting.
         * Returns undefined immediately when the queue is empty.
         *
         * @returns { T | undefined } The removed head element when the queue is not empty,
         *     else undefined when the queue is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public poll(): T | undefined {
            if (this.atomicsGetSize() == 0) {
                return undefined;
            }
            let previousSize: int = -1;
            ConcurrencyHelpers.mutexLock(this.popMutex);
            try {
                if (this.atomicsGetSize() > 0) {
                    const first = this.dequeueElementLocked();
                    previousSize = this.completeDequeueLocked();
                    return first as T;
                } else {
                    return undefined;
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.popMutex);
                this.resumeProducersIfQueueWasFull(previousSize);
            }
        }

        /**
         * Returns the current head element without removing it.
         * Returns undefined when the queue is empty.
         *
         * @returns { T | undefined } The current head element when the queue is not empty,
         *     else undefined when the queue is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public getFirst(): T | undefined {
            ConcurrencyHelpers.mutexLock(this.pushMutex);
            ConcurrencyHelpers.mutexLock(this.popMutex);
            try {
                let element = this.head.next?.element;
                return element;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.popMutex);
                ConcurrencyHelpers.mutexUnlock(this.pushMutex);
            }
        }

        /**
         * Returns the current tail element without removing it.
         * Returns undefined when the queue is empty.
         *
         * @returns { T | undefined } The current tail element when the queue is not empty,
         *     else undefined when the queue is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public getEnd(): T | undefined {
            ConcurrencyHelpers.mutexLock(this.pushMutex);
            ConcurrencyHelpers.mutexLock(this.popMutex);
            try {
                let element = this.tail.element;
                return element;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.popMutex);
                ConcurrencyHelpers.mutexUnlock(this.pushMutex);
            }
        }

        /**
         * Checks whether the queue currently contains no elements.
         * Returns false as soon as at least one element is present.
         *
         * @returns { boolean } True when the LinkedConcurrentQueue contains no elements,
         *     else false when at least one element is present.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public isEmpty(): boolean {
            return this.atomicsGetSize() == 0;
        }

        /**
         * Gets the number of elements currently stored in the queue.
         * Returns the current element count without modifying the queue.
         *
         * @returns { int } The number of elements currently stored in the LinkedConcurrentQueue,
         *     else returns 0 when the queue is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public get size(): int {
            return this.atomicsGetSize();
        }

        /**
         * Gets the configured capacity of the queue.
         * Returns the maximum number of elements that the queue can ideally hold.
         *
         * @returns { int } The configured capacity of the LinkedConcurrentQueue,
         *     else returns the implementation maximum when the queue is unbounded.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public get capacity(): int {
            return this.actualCapacity;
        }

        /**
         * Gets the number of additional elements that can be accepted without waiting.
         * Returns zero when the queue has reached its current capacity.
         *
         * @returns { int } The remaining capacity that can be accepted without waiting,
         *     else returns 0 when the queue is already full.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        public remainingCapacity(): int {
            return this.actualCapacity - this.atomicsGetSize();
        }

        /**
         * Waits asynchronously until the queue is no longer full.
         * Resumes when at least one additional element can be inserted.
         *
         * @returns { Promise<void> } A promise that resolves after free space becomes available,
         *     and after the waiting coroutine is resumed.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private async waitFullQueue(): Promise<void> {
            while (this.atomicsGetSize() == this.actualCapacity) {
                await this.pushCondVar.wait(this.pushMutex);
            }
        }

        /**
         * Waits asynchronously until the queue is no longer empty.
         * Resumes when at least one element becomes available to remove.
         *
         * @returns { Promise<void> } A promise that resolves after an element becomes available,
         *     and after the waiting coroutine is resumed.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private async waitEmptyQueue(): Promise<void> {
            while (this.atomicsGetSize() == 0) {
                await this.popCondVar.wait(this.popMutex);
            }
        }

        /**
         * Wakes one coroutine waiting to remove an element from the queue.
         * Performs the notification while holding the pop-side mutex.
         *
         * @returns { void } No return value.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private resumePopQueue(): void {
            ConcurrencyHelpers.mutexLock(this.popMutex);
            this.popCondVar.notifyOne();
            ConcurrencyHelpers.mutexUnlock(this.popMutex);
        }

        /**
         * Wakes one coroutine waiting to insert an element into the queue.
         * Performs the notification while holding the push-side mutex.
         *
         * @returns { void } No return value.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private resumePushQueue(): void {
            ConcurrencyHelpers.mutexLock(this.pushMutex);
            this.pushCondVar.notifyOne();
            ConcurrencyHelpers.mutexUnlock(this.pushMutex);
        }

        /**
         * Atomically increments the queue size and returns the previous value.
         * Uses the internal atomic counter shared by producers and consumers.
         *
         * @returns { int } The previous queue size before the increment is applied,
         *     and before the new element count becomes visible.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private fetchAndAddSize(): int {
            return this.actualSize.fetchAdd(1);
        }

        /**
         * Atomically decrements the queue size and returns the previous value.
         * Uses the internal atomic counter shared by producers and consumers.
         *
         * @returns { int } The previous queue size before the decrement is applied,
         *     and before the new element count becomes visible.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private fetchAndSubSize(): int {
            return this.actualSize.fetchSub(1);
        }

        /**
         * Reads the current queue size atomically.
         * Returns the latest visible element count without modifying the queue.
         *
         * @returns { int } The current queue size observed from the atomic counter,
         *     else returns 0 when the queue is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private atomicsGetSize(): int {
            return this.actualSize.load();
        }

        /**
         * Initializes the queue sentinel state used by all constructors.
         *
         * @returns { void } No return value.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private initializeQueueState(): void {
            const sentinel = new ListNode<T>();
            this.head = sentinel;
            this.tail = sentinel;
        }

        /**
         * Appends all defined values from an iterable in iteration order.
         * Centralizes constructor iteration so the undefined filtering rule is documented once.
         *
         * @param { Iterable<T> } iterable The iterable source used to initialize the queue.
         * @returns { void } No return value.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private appendIterable(iterable: Iterable<T>): void {
            const entriesIter = iterable.$_iterator();
            for (let iterRes = entriesIter.next(); !iterRes.done; iterRes = entriesIter.next()) {
                const val = iterRes.value;
                if (val !== undefined) {
                    // The undefined guard above proves the iterated value is present for the non-null assertion.
                    if (!this.add(val!)) {
                        throw new RangeError("LinkedConcurrentQueue:: iterable exceeds capacity");
                    }
                }
            }
        }

        /**
         * Appends a node and updates the queue size.
         * The caller must hold the push-side mutex while invoking this helper.
         *
         * @param { ListNode<T> } newNode The node appended to the linked queue.
         * @returns { int } The queue size observed before the node was appended.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private completeEnqueueLocked(newNode: ListNode<T>): int {
            this.pushListNode(newNode);
            const previousSize = this.fetchAndAddSize();
            this.notifyPendingProducersLocked(previousSize);
            return previousSize;
        }

        /**
         * Removes and returns the first visible element while preserving invariant checks.
         * The caller must hold the pop-side mutex while invoking this helper.
         *
         * @returns { T } The removed head element.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private dequeueElementLocked(): T {
            const firstNode = this.requireFirstNodeLocked();
            const first = this.requireNodeElement(firstNode);
            this.popListNode(firstNode);
            return first;
        }

        /**
         * Completes the dequeue state transition after the head element was removed.
         * The caller must hold the pop-side mutex while invoking this helper.
         *
         * @returns { int } The queue size observed before the dequeue completed.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private completeDequeueLocked(): int {
            const previousSize = this.fetchAndSubSize();
            this.notifyPendingConsumersLocked(previousSize);
            return previousSize;
        }

        /**
         * Returns the first linked node that still carries a queue element.
         * Throws when the queue invariants are broken while the queue is expected to be non-empty.
         *
         * @returns { ListNode<T> } The first linked node after the sentinel head.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private requireFirstNodeLocked(): ListNode<T> {
            const firstNode = this.head.next;
            if (firstNode === undefined) {
                throw new Error("LinkedConcurrentQueue:: missing first node");
            }
            return firstNode;
        }

        /**
         * Reads an element from a node and throws when an invariant violation is detected.
         * This keeps internal corruption visible instead of relying on unchecked non-null assertions.
         *
         * @param { ListNode<T> } node The node that should carry an element.
         * @returns { T } The element stored in the node.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private requireNodeElement(node: ListNode<T>): T {
            const element = node.element;
            if (element === undefined) {
                throw new Error("LinkedConcurrentQueue:: missing first element");
            }
            return element;
        }

        /**
         * Wakes another producer already waiting on the not-full condition.
         * The caller must hold the push-side mutex while invoking this helper.
         *
         * @param { int } previousSize The queue size before the append.
         * @returns { void } No return value.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private notifyPendingProducersLocked(previousSize: int): void {
            if (previousSize + 1 < this.actualCapacity) {
                this.pushCondVar.notifyOne();
            }
        }

        /**
         * Wakes another consumer already waiting on the not-empty condition.
         * The caller must hold the pop-side mutex while invoking this helper.
         *
         * @param { int } previousSize The queue size before the dequeue.
         * @returns { void } No return value.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private notifyPendingConsumersLocked(previousSize: int): void {
            if (previousSize - 1 > 0) {
                this.popCondVar.notifyOne();
            }
        }

        /**
         * Wakes one waiting consumer after a transition from empty to non-empty.
         * This notification is performed after releasing the push-side mutex.
         *
         * @param { int } previousSize The queue size before the append.
         * @returns { void } No return value.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private resumeConsumersIfQueueWasEmpty(previousSize: int): void {
            if (previousSize == 0) {
                this.resumePopQueue();
            }
        }

        /**
         * Wakes one waiting producer after a transition from full to non-full.
         * This notification is performed after releasing the pop-side mutex.
         *
         * @param { int } previousSize The queue size before the dequeue.
         * @returns { void } No return value.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private resumeProducersIfQueueWasFull(previousSize: int): void {
            if (previousSize == this.actualCapacity) {
                this.resumePushQueue();
            }
        }

        /**
         * Appends a node to the tail of the linked queue.
         * Updates the tail pointer to reference the appended node.
         *
         * @param { ListNode<T> } newNode The node to append at the tail of the queue.
         * @returns { void } No return value.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private pushListNode(newNode: ListNode<T>): void {
            this.tail.next = newNode;
            this.tail = newNode;
            return;
        }

        /**
         * Removes the first node from the linked queue.
         * Advances the sentinel head node to the next linked node.
         *
         * @returns { void } No return value.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        private popListNode(firstNode: ListNode<T>): void {
            let h = this.head;
            h.next = h;
            this.head = firstNode;
            firstNode.element = undefined;
            return;
        }

        private pushMutex: Object = ConcurrencyHelpers.mutexCreate();
        private popMutex: Object = ConcurrencyHelpers.mutexCreate();
        private pushCondVar: concurrency.stackless.AsyncCondVarBasedMutex = new concurrency.stackless.AsyncCondVarBasedMutex();
        private popCondVar: concurrency.stackless.AsyncCondVarBasedMutex = new concurrency.stackless.AsyncCondVarBasedMutex();
        private actualCapacity: int = Int.MAX_VALUE;
        private actualSize: AtomicInt = new AtomicInt(0);
        private head: ListNode<T>;
        private tail: ListNode<T>;
    }
}