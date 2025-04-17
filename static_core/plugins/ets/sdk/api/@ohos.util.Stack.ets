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

export class Stack<T> implements Iterable<T> {
    private buffer: Array<T>;

    /**
     * Gets the number of elements in the Stack.
     */
    get length(): number {
        return this.buffer.length;
    }

    /**
     * Retrieves the top element in the Stack without removing it.
     * @returns The top element in the Stack, or undefined if the Stack is empty.
     */
    public peek(): T | undefined {
        return this.buffer.length == 0 ? undefined : this.buffer[this.length - 1];
    }

    /**
     * Finds the index of the specified element in the Stack.
     * @param element - The element to locate in the Stack.
     * @returns The index of the element if found, otherwise -1.
     */
    public locate(element: T): number {
        return this.buffer.findIndex((val) => val == element);
    }

    /**
     * Removes and returns the first element in the Stack.
     * @returns The first element in the Stack, or undefined if the Stack is empty.
     */
    public pop(): T | undefined {
        return this.buffer.pop();
    }

    /**
     * Creates a new instance of Stack.
     */
    public constructor() {
        this.buffer = new Array<T>();
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param callbackfn - The function to execute on each element, taking the value, index, and Stack as arguments.
     */
    public forEach(callbackfn: (value: T, index: number, stack: Stack<T>) => void): void {
        this.buffer.forEach(((val: T, idx: number) => callbackfn(val, idx, this)));
    }

    /**
     * Adds an element to the front of the Stack.
     * @param element - The element to add.
     */
    public push(element: T): T {
        this.buffer.push(element);
        return element;
    }

    /**
     * Checks if the stack is empty.
     *
     * @returns {boolean} `true` if the stack is empty, otherwise `false`.
     */
    public isEmpty(): boolean {
        return this.length == 0;
    }

    /**
     * Returns an iterator over the elements in the Stack.
     * @returns An iterator over the elements in the Stack.
     */
    public override $_iterator(): IterableIterator<T> {
        return this.buffer.$_iterator();
    }
}
