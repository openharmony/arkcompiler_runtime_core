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

import Deque from "@ohos.util.Deque";

export class Queue<T> implements Iterable<T> {
    private buffer: Deque<T>;

    /**
     * Gets the number of elements in the Queue.
     */
    get length(): number {
        return this.buffer.length;
    }

    /**
     * Retrieves the first element in the Queue without removing it.
     * @returns The first element in the Queue, or undefined if the Queue is empty.
     */
    public getFirst(): T | undefined {
        return this.buffer.getFirst();
    }

    /**
     * Removes and returns the first element in the Queue.
     * @returns The first element in the Queue, or undefined if the Queue is empty.
     */
    public pop(): T | undefined {
        return this.buffer.popFirst();
    }

    /**
     * Creates a new instance of Queue.
     */
    public constructor() {
        this.buffer = new Deque<T>();
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param callbackfn - The function to execute on each element, taking the value, index, and Queue as arguments.
     */
    public forEach(callbackfn: (value: T, index: number, queue: Queue<T>) => void): void {
        this.buffer.forEach(((val: T, idx: number) => callbackfn(val, idx, this)));
    }

    /**
     * Adds an element to the end of the Queue.
     * @param element - The element to add.
     * 
     * @returns Returns true if the element is added successfully; returns false otherwise.
     */
    public add(element: T): boolean {
        try {
            this.buffer.insertEnd(element);
        } catch (e) {
            return false;
        }
        return true;
    }

    /**
     * Returns an iterator over the elements in the Queue.
     * @returns An iterator over the elements in the Queue.
     */
    public override $_iterator(): IterableIterator<T> {
        return this.buffer.$_iterator();
    }
}
