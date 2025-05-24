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
    /**
     * Constructs a default LinkedBlockingQueue.
     */
    constructor() {
        this.actualCapacity = Int.MAX_VALUE;
        this.pushMutex = ConcurrencyHelpers.mutexCreate();
        this.pushCondVar = ConcurrencyHelpers.condVarCreate();
        this.popMutex = ConcurrencyHelpers.mutexCreate();
        this.popCondVar = ConcurrencyHelpers.condVarCreate();
        this.sizeMutex = ConcurrencyHelpers.mutexCreate();
        this.mutex = ConcurrencyHelpers.mutexCreate();
    }

    /**
     * Constructs a LinkedBlockingQueue with the capacity.
     *
     * @param capacity the capacity of LinkedBlockingQueue
     */
    constructor(capacity: int) {
        if (capacity <= 0) {
            throw new RangeError("Invalid capacity for LinkedBlockingQueue's constructor.");
        }
        this.actualCapacity = capacity;
        this.pushMutex = ConcurrencyHelpers.mutexCreate();
        this.pushCondVar = ConcurrencyHelpers.condVarCreate();
        this.popMutex = ConcurrencyHelpers.mutexCreate();
        this.popCondVar = ConcurrencyHelpers.condVarCreate();
        this.sizeMutex = ConcurrencyHelpers.mutexCreate();
        this.mutex = ConcurrencyHelpers.mutexCreate();
    }

    /**
     * Constructs a LinkedBlockingQueue with the Iterable.
     *
     * @param iterable the Iterable of LinkedBlockingQueue
     */
    constructor(iterable: Iterable<T>) {
        this.actualCapacity = Int.MAX_VALUE;
        this.pushMutex = ConcurrencyHelpers.mutexCreate();
        this.pushCondVar = ConcurrencyHelpers.condVarCreate();
        this.popMutex = ConcurrencyHelpers.mutexCreate();
        this.popCondVar = ConcurrencyHelpers.condVarCreate();
        this.sizeMutex = ConcurrencyHelpers.mutexCreate();
        this.mutex = ConcurrencyHelpers.mutexCreate();
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
     * @param capacity the capacity of LinkedBlockingQueue
     * @param iterable the Iterable of LinkedBlockingQueue
     */
    constructor(capacity: int , iterable: Iterable<T>) {
        if (capacity <= 0) {
            throw new RangeError("Invalid capacity for LinkedBlockingQueue's constructor.");
        }
        this.actualCapacity = capacity;
        this.pushMutex = ConcurrencyHelpers.mutexCreate();
        this.pushCondVar = ConcurrencyHelpers.condVarCreate();
        this.popMutex = ConcurrencyHelpers.mutexCreate();
        this.popCondVar = ConcurrencyHelpers.condVarCreate();
        this.sizeMutex = ConcurrencyHelpers.mutexCreate();
        this.mutex = ConcurrencyHelpers.mutexCreate();
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
     * @param element the element pushed to the BlockingQueue
     *
     * @returns void
     */
    override push(element: T): void {
        ConcurrencyHelpers.mutexLock(this.pushMutex);
        this.waitFullQueue();
        this.pushListNode(element);
        if(this.atomicsGetSize() < this.actualCapacity) {
            ConcurrencyHelpers.condVarNotifyOne(this.pushCondVar, this.pushMutex);
        }
        ConcurrencyHelpers.mutexUnlock(this.pushMutex);
        this.resumePopQueue();
    }

    /**
     * pop() is remove operation and returns T
     * If the BlockingQueue is empty, blocking.
     *
     * @returns T The first element.
     */
    override pop(): T {
        ConcurrencyHelpers.mutexLock(this.popMutex);
        this.waitEmptyQueue();
        try {
            const val = this.head;
            let first = val!.element;
            this.popListNode();
            if(this.atomicsGetSize() > 0) {
                ConcurrencyHelpers.condVarNotifyOne(this.popCondVar, this.popMutex);
            }
            return first;
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.popMutex);
            this.resumePushQueue();
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
        if (this.atomicsGetSize() == this.actualCapacity) {
            return false;
        }
        ConcurrencyHelpers.mutexLock(this.pushMutex);
        try {
            if (this.atomicsGetSize() < this.actualCapacity) {
                this.pushListNode(element);
                if(this.atomicsGetSize() < this.actualCapacity) {
                    ConcurrencyHelpers.condVarNotifyOne(this.pushCondVar, this.pushMutex);
                }
                return true;
            }else {
                return false;
            }
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.pushMutex);
            this.resumePopQueue();
        }
    }

    /**
     * The poll operation. Pop one element and return it if the queue is not empty.
     * Return null when the queue is empty. No blocking.
     *
     * @returns Return the deleted element or null.
     */
    override poll(): T | null {
        if (this.atomicsGetSize() == 0) {
            return null;
        }
        ConcurrencyHelpers.mutexLock(this.popMutex);
        try {
            if (this.atomicsGetSize() > 0) {
                const val = this.head;
                let first = val!.element;
                this.popListNode();
                if(this.atomicsGetSize() > 0) {
                    ConcurrencyHelpers.condVarNotifyOne(this.popCondVar, this.popMutex);
                }
                return first;
            }else {
                return null;
            }
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.popMutex);
            this.resumePushQueue();
        }
    }

    /**
     * Returns the first element in a BlockingQueue.
     *
     * @returns return the first element or null.
     */
    override getFirst(): T | null {
        ConcurrencyHelpers.mutexLock(this.popMutex);
        try {
            if (this.atomicsGetSize() == 0) {
                return null;
            } else {
                return this.head!.element;
            }
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.popMutex);
        }
    }

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
        }
    }

    private waitEmptyQueue() {
        while (this.atomicsGetSize() == 0) {
            ConcurrencyHelpers.condVarWait(this.popCondVar, this.popMutex);
        }
    }

    private resumePopQueue() {
        if (this.atomicsGetSize() == 1) {
            ConcurrencyHelpers.mutexLock(this.popMutex);
            ConcurrencyHelpers.condVarNotifyOne(this.popCondVar, this.popMutex);
            ConcurrencyHelpers.mutexUnlock(this.popMutex);
        }
    }

    private resumePushQueue() {
        if (this.atomicsGetSize() == this.actualCapacity - 1) {
            ConcurrencyHelpers.mutexLock(this.pushMutex);
            ConcurrencyHelpers.condVarNotifyOne(this.pushCondVar, this.pushMutex);
            ConcurrencyHelpers.mutexUnlock(this.pushMutex);
        }
    }

    private atomicsAddSize() {
        ConcurrencyHelpers.mutexLock(this.sizeMutex);
        this.actualSize++;
        ConcurrencyHelpers.mutexUnlock(this.sizeMutex);
    }

    private atomicsSubSize() {
        ConcurrencyHelpers.mutexLock(this.sizeMutex);
        this.actualSize--;
        ConcurrencyHelpers.mutexUnlock(this.sizeMutex);
    }

    private atomicsGetSize(): int {
        ConcurrencyHelpers.mutexLock(this.sizeMutex);
        try {
            return this.actualSize;
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.sizeMutex);
        }
    }

    private pushListNode(element: T): void {
        let newNode: ListNode<T> = new ListNode<T>(element);
        if (this.atomicsGetSize() < 3) {
            ConcurrencyHelpers.mutexLock(this.mutex);
            if (this.head === undefined) {
                this.head = this.tail = newNode;
                ConcurrencyHelpers.mutexUnlock(this.mutex);
                this.atomicsAddSize();
                return;
            }
            this.tail!.next = newNode;
            newNode.prev = this.tail;
            this.tail = newNode;
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        } else {
            this.tail!.next = newNode;
            newNode.prev = this.tail;
            this.tail = newNode;
        }

        this.atomicsAddSize();
        return;
    }

    private popListNode(): void {
        if (this.head !== undefined) {
            if (this.atomicsGetSize() < 3) {
                ConcurrencyHelpers.mutexLock(this.mutex);
                this.head = this.head!.next;
                if (this.head !== undefined) {
                    this.head!.prev = undefined;
                }
                if (this.head === undefined) {
                    this.tail = undefined;
                }
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            } else {
                this.head = this.head!.next;
                this.head!.prev = undefined;
            }

            this.atomicsSubSize();
        }
        return;
    }

    private mutex: Object;
    private pushMutex: Object;                          // push/add mutex
    private popMutex: Object;                           // pop/remove mutex
    private pushCondVar: Object;                        // push/add CondVar
    private popCondVar: Object;                         // pop/remove CondVar
    private actualCapacity : int = 0;                   // the capacity of the LinkedBlockingQueue.
    private actualSize : int = 0;                       // the size of the LinkedBlockingQueue.
    private sizeMutex: Object;                          // size mutex
    private head: ListNode<T> | undefined = undefined;
    private tail: ListNode<T> | undefined = undefined;
}
