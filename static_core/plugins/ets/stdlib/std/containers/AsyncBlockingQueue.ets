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

export namespace containers.stackless {

    /**
    * BlockingQueue interface
    *
    * @syscap SystemCapability.Utils.Lang
    * @since 24
    */
    export interface BlockingQueue<T> {
        /**
        * Inserts element at the end. If the queue is full, waiting asynchronously.
        *
        * @param { T } element the element pushed to the BlockingQueue
        * @returns { Promise<void> } A Promise that resolves when the element has been successfully added to the queue
        * @syscap SystemCapability.Utils.Lang
        * @since 24
        */
        push(element: T): Promise<void>;

        /**
        * Removes and returns the first element in the BlockingQueue. If the queue is empty, waiting asynchronously.
        *
        * @returns { Promise<T> } A Promise that resolves when the element has been successfully removed from the queue
        * @syscap SystemCapability.Utils.Lang
        * @since 24
        */
        pop(): Promise<T>;

        /**
        * The insert operation​. Returns true upon success and false if no space is currently available,​ non-blocking.
        *
        * @param { T } element the element inserted to the BlockingQueue
        * @returns { boolean } True upon success and false if no space is currently available
        * @syscap SystemCapability.Utils.Lang
        * @since 24
        */
        add(element: T): boolean;

        /**
        * The poll operation. Pop one element and return it if the queue is not empty.
        * Returns undefined when the queue is empty. Non-blocking.
        *
        * @returns { T | undefined } the deleted element or undefined when the queue is empty
        * @syscap SystemCapability.Utils.Lang
        * @since 24
        */
        poll(): T | undefined;

        /**
        * Returns the first element in a BlockingQueue.
        *
        * @returns { T | undefined } the first element or undefined when the queue is empty
        * @syscap SystemCapability.Utils.Lang
        * @since 24
        */
        getFirst(): T | undefined;

        /**
        * Checks if the BlockingQueue is empty.
        *
        * @returns { boolean } True if a BlockingQueue has no elements, otherwise false
        * @syscap SystemCapability.Utils.Lang
        * @since 24
        */
        isEmpty(): boolean;

        /**
        * Returns the size of a BlockingQueue.
        *
        * @returns { int } the size of the BlockingQueue
        * @syscap SystemCapability.Utils.Lang
        * @since 24
        */
        get size(): int;

        /**
        * Returns the capacity of a BlockingQueue.
        *
        * @returns { int } the capacity of the BlockingQueue
        * @syscap SystemCapability.Utils.Lang
        * @since 24
        */
        get capacity(): int;

        /**
        * Returns the number of additional elements that this queue can ideally accept without blocking.
        *
        * @returns { int } the remaining capacity of the BlockingQueue
        * @syscap SystemCapability.Utils.Lang
        * @since 24
        */
        remainingCapacity(): int;
    }
}
