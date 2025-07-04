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

<<<<<<< HEAD
/**
 * A node in the linked list.
 */
final class ListNode<T> {
    element: T;
    prev: ListNode<T> | undefined = undefined;
    next: ListNode<T> | undefined = undefined;

    constructor(element: T) {
        this.element = element;
    }
}

/**
 * BlockingQueue implementation
 */
export class LinkedBlockingQueue<T> implements BlockingQueue<T> {
=======
export namespace containers {
    
>>>>>>> OpenHarmony_feature_20250328
    /**
    * A node in the linked list.
    */
    final class ListNode<T> {
        element: T | undefined = undefined;
        next: ListNode<T> | undefined = undefined;

        constructor() { }

        constructor(element: T) {
            this.element = element;
        }
    }

    /**
    * BlockingQueue implementation
    */
    export class LinkedBlockingQueue<T> implements BlockingQueue<T> {
        /**
        * Constructs a default LinkedBlockingQueue.
        */
        constructor() {
            this.actualCapacity = Int.MAX_VALUE;
            this.tail = this.head = new ListNode<T>();
            this.pushMutex = ConcurrencyHelpers.mutexCreate();
            this.pushCondVar = ConcurrencyHelpers.condVarCreate();
            this.popMutex = ConcurrencyHelpers.mutexCreate();
            this.popCondVar = ConcurrencyHelpers.condVarCreate();
            this.sizeMutex = ConcurrencyHelpers.spinlockCreate();
        }

        /**
        * Constructs a LinkedBlockingQueue with the capacity.
        *
        * @param { int } capacity the capacity of LinkedBlockingQueue
        */
        constructor(capacity: int) {
            if (capacity <= 0) {
                throw new RangeError("Invalid capacity for LinkedBlockingQueue's constructor.");
            }
            this.actualCapacity = capacity;
            this.tail = this.head = new ListNode<T>();
            this.pushMutex = ConcurrencyHelpers.mutexCreate();
            this.pushCondVar = ConcurrencyHelpers.condVarCreate();
            this.popMutex = ConcurrencyHelpers.mutexCreate();
            this.popCondVar = ConcurrencyHelpers.condVarCreate();
            this.sizeMutex = ConcurrencyHelpers.spinlockCreate();
        }

        /**
        * Constructs a LinkedBlockingQueue with the Iterable.
        *
        * @param { Iterable<T> } iterable the Iterable of LinkedBlockingQueue
        */
        constructor(iterable: Iterable<T>) {
            this.actualCapacity = Int.MAX_VALUE;
            this.tail = this.head = new ListNode<T>();
            this.pushMutex = ConcurrencyHelpers.mutexCreate();
            this.pushCondVar = ConcurrencyHelpers.condVarCreate();
            this.popMutex = ConcurrencyHelpers.mutexCreate();
            this.popCondVar = ConcurrencyHelpers.condVarCreate();
            this.sizeMutex = ConcurrencyHelpers.spinlockCreate();
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

        /**
        * Constructs a LinkedBlockingQueue with the Iterable.
        *
        * @param { int } capacity the capacity of LinkedBlockingQueue
        * @param { Iterable<T> } iterable the Iterable of LinkedBlockingQueue
        */
        constructor(capacity: int, iterable: Iterable<T>) {
            if (capacity <= 0) {
                throw new RangeError("Invalid capacity for LinkedBlockingQueue's constructor.");
            }
            this.actualCapacity = capacity;
            this.tail = this.head = new ListNode<T>();
            this.pushMutex = ConcurrencyHelpers.mutexCreate();
            this.pushCondVar = ConcurrencyHelpers.condVarCreate();
            this.popMutex = ConcurrencyHelpers.mutexCreate();
            this.popCondVar = ConcurrencyHelpers.condVarCreate();
            this.sizeMutex = ConcurrencyHelpers.spinlockCreate();
            const entriesIter = (iterable as Iterable<T>).$_iterator();
            for (let iterRes = entriesIter.next(); !iterRes.done; iterRes = entriesIter.next()) {
                const val = iterRes.value;
                if (val !== undefined) {
                    this.add(val as T);
                }
            }
        }

        /**
        * Inserts element at the end. If the queue is full, blocking.
        *
        * @param { T } element the element pushed to the BlockingQueue
        */
        override push(element: T): void {
            let newNode = new ListNode<T>(element);
            ConcurrencyHelpers.mutexLock(this.pushMutex);
            this.waitFullQueue();
            this.pushListNode(newNode);
            let c = this.fetchAndAddSize();
            if (c + 1 < this.actualCapacity) {
                ConcurrencyHelpers.condVarNotifyOne(this.pushCondVar, this.pushMutex);
            }
            ConcurrencyHelpers.mutexUnlock(this.pushMutex);
            if (c == 0)
                this.resumePopQueue();
        }

        /**
        * Removes and returns the first element in the BlockingQueue. If the queue is empty, blocking.
        *
        * @returns { T } the fisrt element poped in the queue.
        */
        override pop(): T {
            ConcurrencyHelpers.mutexLock(this.popMutex);
            this.waitEmptyQueue();
            let c = -1;
            try {
                let firstNode = this.head.next;
                let first = firstNode!.element;
                this.popListNode();
                c = this.fetchAndSubSize();
                if (c - 1 > 0) {
                    ConcurrencyHelpers.condVarNotifyOne(this.popCondVar, this.popMutex);
                }
                return first as T;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.popMutex);
                if (c == this.capacity)
                    this.resumePushQueue();
            }
        }

        /**
        * The insert operation. Returns true upon success and false if no space is currently available, no blocking.
        *
        * @param { T } element the element inserted to the BlockingQueue
        *
        * @returns { boolean } True upon success and false if no space is currently available.
        */
        override add(element: T): boolean {
            if (this.atomicsGetSize() == this.actualCapacity) {
                return false;
            }
            let newNode = new ListNode<T>(element);
            let c: int = -1;
            ConcurrencyHelpers.mutexLock(this.pushMutex);
            try {
                if (this.atomicsGetSize() < this.actualCapacity) {
                    this.pushListNode(newNode);
                    c = this.fetchAndAddSize();
                    if (c + 1 < this.actualCapacity) {
                        ConcurrencyHelpers.condVarNotifyOne(this.pushCondVar, this.pushMutex);
                    }
                    return true;
                } else {
                    return false;
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.pushMutex);
                if (c == 0)
                    this.resumePopQueue();
            }
        }

        /**
        * The poll operation. Pop one element and return it if the queue is not empty.
        * Returns undefined when the queue is empty. No blocking.
        *
        * @returns { T | undefined } the deleted element or undefined when the queue is empty.
        */
        override poll(): T | undefined {
            if (this.atomicsGetSize() == 0) {
                return undefined;
            }
            let c: int = -1;
            ConcurrencyHelpers.mutexLock(this.popMutex);
            try {
                if (this.atomicsGetSize() > 0) {
                    let firstNode = this.head.next;
                    let first = firstNode!.element;
                    this.popListNode();
                    c = this.fetchAndSubSize();
                    if (c - 1 > 0) {
                        ConcurrencyHelpers.condVarNotifyOne(this.popCondVar, this.popMutex);
                    }
                    return first as T;
                } else {
                    return undefined;
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.popMutex);
                if (c == this.capacity)
                    this.resumePushQueue();
            }
        }

<<<<<<< HEAD
    /**
     * Returns the end element in a LinkedBlockingQueue.
     *
     * @returns return the end element or null.
     */
    public getEnd(): T | null {
        ConcurrencyHelpers.mutexLock(this.pushMutex);
        try {
            if (this.atomicsGetSize() == 0) {
                return null;
            } else {
                return this.tail!.element;
            }
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.pushMutex);
        }
    }

    /**
     * Checks if the BlockingQueue is empty.
     *
     * @returns boolean True if a BlockingQueue has no elements, otherwise false.
     */
    override isEmpty(): boolean {
        return this.atomicsGetSize() == 0;
    }

    /**
     * Returns the size of a BlockingQueue.
     *
     * @returns int The size of the BlockingQueue.
     */
    override get size(): int {
        return this.atomicsGetSize();
    }

    /**
     * Returns the capacity of a BlockingQueue.
     *
     * @returns int The capacity of the BlockingQueue.
     */
    override get capacity(): int {
        return this.actualCapacity;
    }

    /**
     * Returns the number of additional elements that this queue can ideally accept without blocking.
     *
     * @returns int The capacity of the BlockingQueue.
     */
    override remainingCapacity(): int {
        return (this.actualCapacity - this.atomicsGetSize()) as int;
    }

    private waitFullQueue() {
        while (this.atomicsGetSize() == this.actualCapacity) {
            ConcurrencyHelpers.condVarWait(this.pushCondVar, this.pushMutex);
=======
        /**
        * Returns the first element in a BlockingQueue.
        *
        * @returns { T | undefined } the first element or undefined when the queue is empty.
        */
        override getFirst(): T | undefined {
            ConcurrencyHelpers.mutexLock(this.popMutex);
            try {
                return this.head.next!.element;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.popMutex);
            }
>>>>>>> OpenHarmony_feature_20250328
        }

        /**
        * Returns the end element in a LinkedBlockingQueue.
        *
        * @returns { T | undefined } the end element or undefined when the queue is empty.
        */
        public getEnd(): T | undefined {
            ConcurrencyHelpers.mutexLock(this.pushMutex);
            try {
                ConcurrencyHelpers.mutexLock(this.popMutex);
                let element = this.tail.element;
                ConcurrencyHelpers.mutexUnlock(this.popMutex);
                return element;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.pushMutex);
            }
        }

        /**
        * Checks if the BlockingQueue is empty.
        *
        * @returns { boolean } True if a BlockingQueue has no elements, otherwise false.
        */
        override isEmpty(): boolean {
            return this.atomicsGetSize() == 0;
        }

        /**
        * Returns the size of a BlockingQueue.
        *
        * @returns { int } the size of the BlockingQueue.
        */
        override get size(): int {
            return this.atomicsGetSize();
        }

        /**
        * Returns the capacity of a BlockingQueue.
        *
        * @returns { int } the capacity of the BlockingQueue.
        */
        override get capacity(): int {
            return this.actualCapacity;
        }

        /**
        * Returns the number of additional elements that this queue can ideally accept without blocking.
        *
        * @returns { int } the remainingcapacity of the BlockingQueue.
        */
        override remainingCapacity(): int {
            return this.actualCapacity - this.atomicsGetSize();
        }

        private waitFullQueue() {
            while (this.atomicsGetSize() == this.actualCapacity) {
                ConcurrencyHelpers.condVarWait(this.pushCondVar, this.pushMutex);
            }
        }

        private waitEmptyQueue() {
            while (this.atomicsGetSize() == 0) {
                ConcurrencyHelpers.condVarWait(this.popCondVar, this.popMutex);
            }
        }

        private resumePopQueue() {
            ConcurrencyHelpers.mutexLock(this.popMutex);
            ConcurrencyHelpers.condVarNotifyOne(this.popCondVar, this.popMutex);
            ConcurrencyHelpers.mutexUnlock(this.popMutex);
        }

        private resumePushQueue() {
            ConcurrencyHelpers.mutexLock(this.pushMutex);
            ConcurrencyHelpers.condVarNotifyOne(this.pushCondVar, this.pushMutex);
            ConcurrencyHelpers.mutexUnlock(this.pushMutex);
        }

        private fetchAndAddSize(): int {
            let res = 0;
            ConcurrencyHelpers.spinlockGuard(this.sizeMutex, () => {
                res = this.actualSize;
                this.actualSize++;
            });
            return res;
        }

        private fetchAndSubSize(): int {
            let res = 0;
            ConcurrencyHelpers.spinlockGuard(this.sizeMutex, () => {
                res = this.actualSize;
                this.actualSize--;
            });
            return res;
        }

        private atomicsGetSize(): int {
            let res = 0;
            ConcurrencyHelpers.spinlockGuard(this.sizeMutex, () => {
                res = this.actualSize;
            });
            return res;
        }

        private pushListNode(newNode: ListNode<T>): void {
            this.tail.next = newNode;
            this.tail = newNode;
            return;
        }

        private popListNode(): void {
            let firstNode = this.head.next;
            let h = this.head;
            h.next = h;
            this.head = firstNode!;
            firstNode!.element = undefined;
            return;
        }

        private pushMutex: Object;                          // push/add mutex
        private popMutex: Object;                           // pop/remove mutex
        private pushCondVar: Object;                        // push/add CondVar, notify when NotFull
        private popCondVar: Object;                         // pop/remove CondVar, notify when NotEmpty
        private actualCapacity: int = 0;                    // the capacity of the LinkedBlockingQueue.
        private actualSize: int = 0;                        // the size of the LinkedBlockingQueue.
        private sizeMutex: Object;                          // size mutex
        private head: ListNode<T>;
        private tail: ListNode<T>;
    }
}
