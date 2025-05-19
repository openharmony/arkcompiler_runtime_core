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
 * BlockingQueue interface
 */
export interface BlockingQueue<T> {
    /**
     * Inserts element at the end. If the queue is full, ​blocking​.
     *
     * @param element the element pushed to the BlockingQueue
     *
     * @returns void
     */
    push(element: T): void;

    /**
     * pop() is remove operation and returns T.
     * If the BlockingQueue is empty, ​blocking​.
     *
     * @returns T The first element.
     */
    pop(): T;

    /**
     * The insert operation​. Return true upon success and false if no space is currently available,​ no blocking.
     *
     * @param element the element inserted to the BlockingQueue
     *
     * @returns T True upon success and false if no space is currently available.
     */
    add(element: T): boolean;

    /**
     * The poll operation. Pop one element and return it if the queue is not empty.
     * Return null when the queue is empty. No blocking.
     *
     * @returns Return the deleted element or null.
     */
    poll(): T | null;

    /**
     * Returns the first element in a BlockingQueue.
     *
     * @returns T | null The first element or null.
     */
    getFirst(): T | null;

    /**
     * Checks if the BlockingQueue is empty.
     *
     * @returns boolean True if a BlockingQueue has no elements, otherwise false.
     */
    isEmpty(): boolean;

    /**
     * Returns the size of a BlockingQueue.
     *
     * @returns int The size of the BlockingQueue.
     */
    get size(): int;

    /**
     * Returns the capacity of a BlockingQueue.
     *
     * @returns int The capacity of the BlockingQueue.
     */
    get capacity(): int;

    /**
     * Returns the number of additional elements that this queue can ideally accept without blocking.
     *
     * @returns int The capacity of the BlockingQueue.
     */
    remainingCapacity(): int;
}
