/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

/**
 * @file Defines the UndefinableObjectArray for ArkTS
 * @kit ArkTS
 */

package std.containers

/**
 * Container namespace providing implementations of various container classes
 * @namespace containers
 * @syscap SystemCapability.Utils.Lang
 * @since 24
 */
export namespace containers {
    
    /**
     * Represents a type that can be either Object or undefined
     * @typedef { Object | undefined } UndefinableObject
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    type UndefinableObject = Object | undefined

    /**
     * A dynamic array class that can contain Object or undefined values
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    export class UndefinableObjectArray {
        /**
         * Default initial size of the array
         * @type { int }
         * @static
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        /* const */ static ArrayInitSize : int = 16

        private init(capacity: int, val: UndefinableObject): void {
            this.data = new FixedArray<UndefinableObject>(capacity)
            for (let i = 0; i < this.data.length; ++i) {
                this.data[i] = val
            }
            this.curSize = capacity
        }

        /**
         * Constructs new UndefinableObjectArray with respect to capacity and initial value
         *
         * @param { int } capacity - The initial capacity of the array
         * @param { UndefinableObject } val - The value used to initialize all array elements
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        constructor(capacity: int, val: UndefinableObject) {
            this.init(capacity, val)
        }

        /**
         * Constructs new UndefinableObjectArray with required capacity
         *
         * @param { int } capacity - The initial capacity of the array
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        constructor(capacity: int) {
            this.data = new FixedArray<UndefinableObject>(capacity)
            this.curSize = 0
        }

        /**
         * Constructs new empty UndefinableObjectArray
         *
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        constructor() {
            this(UndefinableObjectArray.ArrayInitSize)
        }

        /**
         * Increases capacity if passed argument is greater than current capacity
         *
         * @param { int } capacity - The new capacity
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public reserve(capacity: int): void {
            if (this.data.length < capacity) {
                let newData : FixedArray<UndefinableObject> = new FixedArray<UndefinableObject>(capacity)
                for (let i = 0; i < this.curSize; ++i) {
                newData[i] = this.data[i]
                }
                this.data = newData
            }
        }

        /**
         * Gets the underlying array
         *
         * @returns { Object[] } The underlying array containing all non-undefined elements
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public toArray(): Object[] {
            let newData = new (Object)[this.curSize]
            for (let i = 0; i < this.curSize; ++i) {
                newData[i] = this.data[i]!
            }
            return newData
        }

        private static getNewCapacity(currentCapacity: int): int {
            const fastGrowThreshold = 8192
            const multiplier = 2
            if (currentCapacity < fastGrowThreshold) {
                // Adding 4 to jump over 0
                return (currentCapacity + 4) * multiplier * 2
            } else {
                return currentCapacity * multiplier
            }
        }

        /**
         * Pushes a value to the end of the array
         *
         * @param { UndefinableObject } e - Value to push
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public pushBack(e: UndefinableObject): void {
            if (this.curSize == this.data.length) {
                let newData : FixedArray<UndefinableObject> = new FixedArray<UndefinableObject>(UndefinableObjectArray.getNewCapacity(this.data.length))
                for (let i = 0; i < this.curSize; ++i) {
                    newData[i] = this.data[i]
                }
                this.data = newData
            }
            this.data[this.curSize] = e
            ++this.curSize
        }

        /**
         * Pops a value from the end of the List
         *
         * @returns { UndefinableObject } Popped value
         * @throws { AssertionError } When the array is empty, attempting to pop an element will throw this exception
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public popBack(): UndefinableObject {
            if (this.curSize === 0) {
                throw new AssertionError("No data to popBack from UndefinableObjectArray!")
            }
            --this.curSize
            return this.data[this.curSize]
        }

        /**
         * Returns number of elements in the List
         *
         * @returns { int } The number of elements in the array
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public size(): int {
            return this.curSize
        }

        /**
         * Returns an element at the specified index
         *
         * @param { int } index - Element position
         * @returns { UndefinableObject } An element at the specified index
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public at(index: int): UndefinableObject {
            return this.data[index]
        }

        /**
         * Sets an element at the specified index
         *
         * @param { int } index - Element position
         * @param { UndefinableObject } e - New value
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public set(index: int, e: UndefinableObject): void {
            this.data[index] = e
        }

        private data: FixedArray<UndefinableObject> = []
        private curSize: int
    }
}
