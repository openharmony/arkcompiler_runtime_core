/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
 * @file Defines the UndefinableStringArray for ArkTS
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
     * Represents a type that can be either String or undefined
     * @typedef { String | undefined } UndefinableString
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    type UndefinableString = String | undefined

    /**
     * A dynamic array class that can contain String or undefined elements
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    export class UndefinableStringArray {
        /**
         * Default initial capacity of the array
         * @type { int }
         * @static
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        /* const */ static ArrayInitSize : int = 16

        private init(capacity: int, val: UndefinableString): void {
            this.data = new UndefinableString[capacity]
            for (let i = 0; i < this.data.length; ++i) {
                this.data[i] = val
            }
            this.curSize = capacity
        }

        /**
         * Constructs new UndefinableStringArray with respect to capacity and initial value
         * @param { int } capacity - The initial capacity of the array
         * @param { UndefinableString } val - The initial value used to fill the array
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        constructor(capacity: int, val: UndefinableString) {
            this.init(capacity, val)
        }

        /**
         * Constructs new UndefinableStringArray with required capacity
         * @param { int } capacity - The initial capacity of the array
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        constructor(capacity: int) {
            this.data = new UndefinableString[capacity]
            this.curSize = 0
        }

        /**
         * Constructs new empty UndefinableStringArray
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        constructor() {
            this(UndefinableStringArray.ArrayInitSize)
        }

        /**
         * Increases capacity if passed argument is greater than current capacity
         * @param { int } capacity - The new capacity requirement
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public reserve(capacity: int): void {
            if (this.data.length < capacity) {
                let newData : FixedArray<UndefinableString> = new UndefinableString[capacity]
                for (let i = 0; i < this.curSize; ++i) {
                newData[i] = this.data[i]
                }
                this.data = newData
            }
        }

        /**
         * Gets the underlying array
         * @returns { String[] } The underlying array containing all non-undefined elements
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public toArray(): String[] {
            let newData = new String[this.curSize]
            for (let i = 0; i < this.curSize; ++i) {
                if (this.data[i] == null) {
                    continue;
                }
                newData[i] = this.data[i]!
            }
            return newData
        }

        /**
         * Converts UndefinableStringArray to a FixedArray of String
         * @returns { FixedArray<String> } A new FixedArray containing all elements
         * @throws { TypeError } If an element is undefined, a type error may be thrown when assigning to FixedArray
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public toFixedArray(): FixedArray<String> {
            let newData: FixedArray<String> = new String[this.curSize]
            for (let i = 0; i < this.curSize; ++i) {
                newData[i] = this.data[i]!
            }
            return newData
        }

        /**
         * Converts UndefinableStringArray to a standard (String | undefined) array
         * @returns { (String | undefined)[] } A new (String | undefined) array containing all elements
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public toUndefinableArray(): (String | undefined)[] {
            let newData = new (String | undefined)[this.curSize]
            for (let i = 0; i < this.curSize; ++i) {
                newData[i] = this.data[i]
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
         * @param { UndefinableString } e - Value to push
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public pushBack(e: UndefinableString): void {
            if (this.curSize == this.data.length) {
                let newData : FixedArray<UndefinableString> = new UndefinableString[UndefinableStringArray.getNewCapacity(this.data.length)]
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
         * @returns { UndefinableString } Popped value
         * @throws { AssertionError } When the array is empty
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public popBack(): UndefinableString {
            if (this.curSize === 0) {
                throw new AssertionError("No data to popBack from UndefinableStringArray!")
            }
            --this.curSize
            return this.data[this.curSize]
        }

        /**
         * Returns number of elements in the List
         * @returns { int } The number of elements in the array
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public size(): int {
            return this.curSize
        }

        /**
         * Returns an element at the specified index
         * @param { int } index - Element position
         * @returns { UndefinableString } An element at the specified index
         * @throws { RangeError } When index is out of bounds
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public at(index: int): UndefinableString {
            return this.data[index]
        }

        /**
         * Sets an element at the specified index
         * @param { int } index - Element position
         * @param { UndefinableString } e - New value
         * @throws { RangeError } When index is out of bounds
         * @syscap SystemCapability.Utils.Lang
         * @since 24
         */
        public set(index: int, e: UndefinableString): void {
            this.data[index] = e
        }

        private data: FixedArray<UndefinableString> = []
        private curSize: int
    }
}
