/*
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

import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: number = 10200001;

function createBusinessError(code: number, message: string) {
    let err = new BusinessError();
    err.code = code;
    err.name = 'BusinessError';
    err.message = message;
    return err;
}

export default class Deque<T> implements Iterable<T> {
    private readonly DEFAULT_CAPACITY = 8;
    internal capacity: int = this.DEFAULT_CAPACITY;
    internal buffer: Array<T>;
    internal front: int = 0;
    internal rear: int = 0;

    /**
     * property: The number of elements in the ArrayList.
     */
    get length(): number {
        return (this.rear - this.front + this.capacity) % this.capacity;
    }

    /**
     * Returns the first element of the deque without removing it.
     *
     * @returns {T | undefined} The first element of the deque if it exists, otherwise `undefined` if the deque is empty.
     */
    public getFirst(): T | undefined {
        if (this.isEmpty()) {
            return undefined;
        }

        return this.buffer[this.front];
    }

    /**
     * Returns the last element of the deque without removing it.
     *
     * @returns {T | undefined} The last element of the deque if it exists, otherwise `undefined` if the deque is empty.
     */
    public getLast(): T | undefined {
        if (this.isEmpty()) {
            return undefined;
        }

        return this.buffer[this.rear - 1];
    }

    /**
     * Removes and returns the first element from the deque.
     *
     * @returns {T | undefined} The first element of the deque if it exists, otherwise `undefined` if the deque is empty.
     */
    public popFirst(): T | undefined {
        if (this.isEmpty()) {
            return undefined;
        }

        let firstElement: T = this.buffer[this.front];
        this.front = (this.front + 1) % this.capacity;

        return firstElement;
    }

    /**
     * Removes and returns the last element from the deque.
     * 
     * @returns {T | undefined} The last element of the deque if it exists, otherwise `undefined` if the deque is empty.
     */
    public popLast(): T | undefined {
        if (this.isEmpty()) {
            return undefined;
        }

        this.rear = (this.rear - 1 + this.capacity) % this.capacity;
        let element: T = this.buffer[this.rear];

        return element;
    }

    /**
     * Creates a new instance of Deque.
     */
    public constructor() {
        this.buffer = new Array<T>(this.capacity);
    }

    /**
     * Checks if the deque contains a specific element.
     *
     * @param element - The element to check for.
     * @returns True if the element is found, false otherwise.
     */
    public has(element: T): boolean {
        let i: int = this.front;
        while (i != this.rear) {
            if (this.buffer[i] === element) {
                return true;
            }
            i = (i + 1) % this.capacity;
        }

        return false;
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param callbackfn - The function to execute on each element, taking the value, index, and Deque as arguments.
     *
     */
    public forEach(callbackfn: (value: T, index: number, deque: Deque<T>) => void): void {
        let i: int = this.front, k = 0;
        while (i != this.rear) {
            callbackfn(this.buffer[i], k++, this);
            i = (i + 1) % this.capacity;
        }
    }

    /**
     * Inserts an element at the end of the deque.
     *
     * @param element - The element to insert.
     */
    public insertEnd(element: T): void {
        if (this.isFull()) {
            this.increaseCapacity();
        }

        this.buffer[this.rear] = element;
        this.rear = (this.rear + 1) % this.capacity;
    }

    /**
     * Inserts an element at the front of the deque.
     *
     * @param element - The element to insert.
     */
    public insertFront(element: T): void {
        if (this.isFull()) {
            this.increaseCapacity();
        }

        this.front = (this.front - 1 + this.capacity) % this.capacity;
        this.buffer[this.front] = element;
    }

    /**
     * Returns an iterator for the deque.
     *
     * @returns {IterableIterator<T>} An iterator for the deque.
     */
    public override $_iterator(): IterableIterator<T> {
        return new DequeValuesIterator_T<T>(this);
    }

    /**
     * Checks if the deque is full.
     *
     * @returns {boolean} True if the deque is full, false otherwise.
     */
    private isFull(): boolean {
        return ((this.front - 1 + this.capacity) % this.capacity) == this.rear;
    }

    /**
     * Checks if the deque is empty.
     *
     * @returns {boolean} True if the deque is empty, false otherwise.
     */
    private isEmpty(): boolean {
        return this.front == this.rear;
    }

    /**
     * Increases the capacity of the deque.
     *
     * @throws {RangeError} If the current capacity has reached the maximum limit.
     */
    private increaseCapacity(): void {
        if (this.capacity == Int.MAX_VALUE) {
            throw new RangeError("The current capacity has reached the maximum limit.");
        }

        let newCap: int = min(this.capacity * 2, Int.MAX_VALUE);

        let newBuffer = new Array<T>(newCap);

        let i: int = this.front, k = 0;
        while (i != this.rear) {
            newBuffer[k++] = this.buffer[i];
            i = (i + 1) % this.capacity;
        }

        this.rear = this.length as int;
        this.front = 0;
        this.capacity = newCap;
        this.buffer = newBuffer;
    }

    /**
     * Gets the element at the specified index.
     *
     * @param index - The index of the element to retrieve.
     *
     * @returns The element at the specified index.
     * @throws { BusinessError } 10200001 - The value of "index" is out of range. It must be >= 0 && <= 2147483647.
     */
    public $_get(index: number): T {
        if (index >= this.length || index < 0) {
            throw createBusinessError(OutOfBoundsErrorCodeId, `The value of \"index\" is out of range. It must be >= 0 && <= ${Int.MAX_VALUE}. Received value is: ${index}`);
        }

        return this.buffer[this.getRealIndex(index)];
    }

    /**
     * Sets the element at the specified index.
     *
     * @param index - The index of the element to set.
     *
     * @param val - The value to set at the specified index.
     * @throws { BusinessError } 10200001 - The value of "index" is out of range. It must be >= 0 && <= 2147483647.
     */
    public $_set(index: number, val: T): void {
        if (index >= this.length || index < 0) {
            throw createBusinessError(OutOfBoundsErrorCodeId, `The value of \"index\" is out of range. It must be >= 0 && <= ${Int.MAX_VALUE}. Received value is: ${index}`);
        }

        this.buffer[this.getRealIndex(index)] = val;
    }

    private getRealIndex(index: number): number {
        return (this.front + index) % this.capacity;
    }
}

class DequeValuesIterator_T<T> implements IterableIterator<T> {
    private parent: Deque<T>
    private idx: int = 0;

    constructor(parent: Deque<T>) {
        this.parent = parent
        this.idx = parent.front;
    }

    override next(): IteratorResult<T> {
        if (this.parent.length == 0 || this.parent.rear == this.idx) {
            return new IteratorResult<T>()
        }

        let val = new IteratorResult<T>(this.parent.buffer[this.idx]);
        this.idx = (this.idx + 1) % this.parent.capacity;

        return val;
    }

    override $_iterator(): IterableIterator<T> {
        return this;
    }
}
