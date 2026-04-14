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
 * The containers.stackless module provides concurrent queue APIs for stackless coroutines.
 *
 * @namespace containers.stackless
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 */
export namespace containers.stackless {

    /**
    * Defines the concurrent queue contract for stackless coroutine containers.
     *
     * @interface ConcurrentQueue
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    export interface ConcurrentQueue<T> {
        /**
         * Inserts an element at the tail of the queue.
         * Waits asynchronously until space becomes available when the queue is full.
         *
         * @param { T } element The element to append to the ConcurrentQueue.
         * @returns { Promise<void> } A promise that resolves after the element is appended,
         *     and after capacity becomes available when the queue is full.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        push(element: T): Promise<void>;

        /**
         * Removes and returns the head element from the queue.
         * Waits asynchronously until an element becomes available when the queue is empty.
         *
         * @returns { Promise<T> } A promise that resolves with the removed head element,
         *     and after an element becomes available when the queue is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        pop(): Promise<Awaited<T>>;

        /**
         * Attempts to insert an element without waiting.
         * Returns false immediately when no space is currently available.
         *
         * @param { T } element The element to insert into the ConcurrentQueue.
         * @returns { boolean } True when the element is inserted successfully,
         *     else false when no space is currently available.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        add(element: T): boolean;

        /**
         * Attempts to remove and return the head element without waiting.
         * Returns undefined immediately when the queue is empty.
         *
         * @returns { T | undefined } The removed head element when the queue is not empty,
         *     else undefined when the queue is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        poll(): T | undefined;

        /**
         * Returns the current head element without removing it.
         * Returns undefined when the queue is empty.
         *
         * @returns { T | undefined } The current head element when the queue is not empty,
         *     else undefined when the queue is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        getFirst(): T | undefined;

        /**
         * Checks whether the queue currently contains no elements.
         * Returns false as soon as at least one element is present.
         *
         * @returns { boolean } True when the ConcurrentQueue contains no elements,
         *     else false when at least one element is present.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        isEmpty(): boolean;

        /**
         * Gets the number of elements currently stored in the queue.
         * Returns the current element count without modifying the queue.
         *
         * @returns { int } The number of elements currently stored in the ConcurrentQueue,
         *     else returns 0 when the queue is empty.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        get size(): int;

        /**
         * Gets the configured capacity of the queue.
         * Returns the maximum number of elements that the queue can ideally hold.
         *
         * @returns { int } The configured capacity of the ConcurrentQueue,
         *     else returns the implementation maximum when the queue is unbounded.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        get capacity(): int;

        /**
         * Gets the number of additional elements that can be accepted without waiting.
         * Returns zero when the queue has reached its current capacity.
         *
         * @returns { int } The remaining capacity that can be accepted without waiting,
         *     else returns 0 when the queue is already full.
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         */
        remainingCapacity(): int;
    }

}