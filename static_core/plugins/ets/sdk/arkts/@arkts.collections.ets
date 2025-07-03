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


import { BusinessError } from "@ohos.base"

const OutOfBoundsErrorCodeId: number = 10200001;
const TypeErrorCodeId: number = 401;

export namespace collections {
    final class BitVectorIterator implements IterableIterator<number> {
        private index: int = 0;
        private parent: BitVector;

        constructor(parent: BitVector) {
            this.parent = parent;
        }

        override next(): IteratorResult<number> {
            if (this.parent.length === 0 || this.index >= this.parent.length) {
                return new IteratorResult<number>();
            }

            return new IteratorResult<number>(this.parent[this.index++]);
        }

        override $_iterator(): IterableIterator<number> {
            return this;
        }
    }

    final class BitIndex {
        // |0|1|...|0|0|1|1| ... |0|1|0|0|1| ... |0|1|
        //          ↑
        //         index
        // |-  element  -|-  element  -|-  element  -|
        //       ↑
        //    elementId
        // |---------------buffer--------------------|
        elementId: number;
        index: number;

        constructor(elementId: number, index: number) {
            this.elementId = elementId;
            this.index = index
        }
    }
    /**
     * An ordered collections of bit values, which are either 0 or 1.
     * If multiple threads access a BitVector instance concurrently,
     * and at least one of the threads modifies the array structurally,
     * it must be synchronized externally.
     *
     */
    export class BitVector implements Iterable<number> {
        static readonly BIT_SET_LENGTH = 32;    // ArkTS Specification, Release 1.1.0 <Integer Types and Operations>

        private buffer: Array<int>;
        private _length: int = 0;

        /**
        * A constructor used to create a BitVector object.
        *
        * @param { number } length - The length of BitVector object.
        */
        public constructor(length: number) {
            this.integerCheck("length", length);

            if (length === 0) {
                this.buffer = new Array<int>();
            } else {
                this._length = length as int;
                this.buffer = Array.create<int>(this.getUsedCapacity(), 0);
            }
        }

        /**
         * Gets the element number of the BitVector. This is a number one higher than the highest index in the bit vector.
         * It can be changed by resize().
         */
        public get length(): number {
            return this._length;
        }

        /**
         * Appends the bit element to the end of this bit vector.
         *
         * @param { number } element - Element to be appended to this bit vector (0 means 0, else means 1).
         * @returns { boolean } The boolean type, returns true if the addition is successful, and returns false if it fails.
         */
        public push(element: number): boolean {
            if (this.length >= Int.MAX_VALUE) {
                return false;
            }

            let index = this.computeElementIdAndBitId(this._length);
            this.checkAndIncrement(index.elementId);

            this.setBitUnsafe(this._length++, element);

            return true;
        }

        /**
         * Retrieves and removes the bit element to the end of this bit vector.
         *
         * @returns { number | undefined } The boolean type, if the bit push successfully, return true, else return false.
         */
        public pop(): number | undefined {
            if (this._length === 0) {
                return undefined;
            }

            return this.getBitUnsafe(--this._length);
        }

        /**
         * Returns the item at that index.
         *
         * @param { number } index - The zero-based index of the desired code unit.
         * @returns { number } The element in the bitVector matching the given index.
         * @throws { BusinessError } 10200001 - If the index is out of range.
         */
        public $_get(index: number): number {
            this.checkIndex(index);

            return this.getBitUnsafe(index);
        }

        /**
         * Sets the value of item at that index.
         *
         * @param { number } index - The zero-based index of the desired code unit.
         * @param { number } value - The value to set at the given index.
         * @throws { BusinessError } 10200001 - If the index is out of range.
         */
        public $_set(index: number, value: number): void {
            this.checkIndex(index);

            this.setBitUnsafe(index, value);
        }
        /**
         * Check if bit vector contains a particular bit element.
         *
         * @param { number } element - Element to be contained (0 means 0, else means 1).
         * @param { number } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { number } toIndex - The end of the index, containing the value at that index.
         * @returns { boolean } The boolean type, if bit vector contains the specified element, return true,
                                 else return false.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public has(element: number, fromIndex: number, toIndex: number): boolean {
            this.checkRange(fromIndex, toIndex);

            let from = this.computeElementIdAndBitId(fromIndex);
            let to =  this.computeElementIdAndBitId(toIndex);

            if (from.elementId === to.elementId) {
                return this.checkBit(this.buffer[from.elementId], from.index, to.index - 1, element);
            }

            if (this.checkBit(this.buffer[from.elementId], from.index, BitVector.BIT_SET_LENGTH - 1, element)) {
                return true;
            }

            from.elementId++;
            while (from.elementId < to.elementId) {
                if (this.checkBit(this.buffer[from.elementId], 0, BitVector.BIT_SET_LENGTH - 1, element)) {
                    return true;
                }
                from.elementId++;
            }

            if (to.index > 0) {
                return this.checkBit(this.buffer[to.elementId], 0, to.index - 1, element);
            }

            return false;
        }

        /**
         * Returns an iterator that iterates over bit vector.
         *
         * @returns { IterableIterator<number> } A new iterable iterator object.
         */
        public override $_iterator(): IterableIterator<number> {
            return new BitVectorIterator(this);
        }

        /**
         * Returns an iterable of values in the bit vector
         *
         * @returns { IterableIterator<number> }  A new iterable iterator object.
         * @throws { BusinessError } 10200011 - The values method cannot be bound.
         * @throws { BusinessError } 10200201 - Concurrent modification error.
         * @syscap SystemCapability.Utils.Lang
         * @crossplatform
         * @atomicservice
         * @since 20
         * @arkts 1.2
         */
        public values(): IterableIterator<number> {
            return this.$_iterator();
        }

        /**
         * Flips the bit value by index in a bit vector.(Flip 0 to 1, flip 1 to 0)
         *
         * @param { number } index - The index in the bit vector.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of index is out of range.
         */
        public flipBitByIndex(index: number): void {
            this.integerCheck("index", index);
            this.checkIndex(index);

            let bitIndex = this.computeElementIdAndBitId(index);
            this.buffer[bitIndex.elementId] = this.flipBits(this.buffer[bitIndex.elementId], bitIndex.index, bitIndex.index);
        }

        /**
         * Flips a range of bit values in a bit vector.(Flip 0 to 1, flip 1 to 0).
         *
         * @param { number } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { number } toIndex - The end of the index, excluding the value at that index.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public flipBitsByRange(fromIndex: number, toIndex: number): void {
            this.checkRange(fromIndex, toIndex);

            let from = this.computeElementIdAndBitId(fromIndex);
            let to =  this.computeElementIdAndBitId(toIndex);

            if (from.elementId === to.elementId) {
                this.buffer[from.elementId] = this.flipBits(this.buffer[from.elementId], from.index, to.index - 1);
                return;
            }

            this.buffer[from.elementId] = this.flipBits(this.buffer[from.elementId], from.index, BitVector.BIT_SET_LENGTH - 1);

            ++from.elementId;
            while (from.elementId < to.elementId) {
                this.buffer[from.elementId] = this.flipBits(this.buffer[from.elementId], 0, BitVector.BIT_SET_LENGTH - 1);
                ++from.elementId;
            }

            if (to.index > 0) {
                this.buffer[to.elementId] = this.flipBits(this.buffer[to.elementId], 0, to.index - 1);
            }
        }

        /**
         * Returns the bit values in a range of indices in a bit vector.
         *
         * @param { number } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { number } toIndex - The end of the index, excluding the value at that index.
         * @returns { BitVector } The BitVector type, returns the bit values in a range of indices in a bit vector.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public getBitsByRange(fromIndex: number, toIndex: number): BitVector {
            this.checkRange(fromIndex, toIndex);

            let from = this.computeElementIdAndBitId(fromIndex);
            let to =  this.computeElementIdAndBitId(toIndex);

            let newBitVector = new BitVector(toIndex - fromIndex);

            if (from.elementId === to.elementId) {
                newBitVector.buffer[0] = this.getBits(this.buffer[from.elementId], from.index, to.index - 1);
                return newBitVector;
            }

            for (let bit of this) {
                newBitVector.push(bit);
            }

            return newBitVector;
        }

        /**
         * Sets a range of bits in a bit vector to a particular element.
         *
         * @param { number } element - Element to be set (0 means 0, else means 1).
         * @param { number } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { number } toIndex - The end of the index, excluding the value at that index.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public setBitsByRange(element: number, fromIndex: number, toIndex: number): void {
            this.checkRange(fromIndex, toIndex);

            let from = this.computeElementIdAndBitId(fromIndex);
            let to =  this.computeElementIdAndBitId(toIndex);

            if (from.elementId === to.elementId) {
                this.buffer[from.elementId] = this.setBits(this.buffer[from.elementId], from.index, to.index - 1, element);
                return;
            }

            this.buffer[from.elementId] = this.setBits(this.buffer[from.elementId], from.index, BitVector.BIT_SET_LENGTH - 1, element);

            ++from.elementId;
            while (from.elementId < to.elementId) {
                this.buffer[from.elementId] = this.setBits(this.buffer[from.elementId], 0, BitVector.BIT_SET_LENGTH - 1, element);
                ++from.elementId;
            }

            if (to.index != 0) {
                this.buffer[from.elementId] = this.setBits(this.buffer[to.elementId], 0, to.index - 1, element);
            }
        }

        /**
         * Sets all of bits in a bit vector to a particular element.
         *
         * @param { number } element - Element to be set (0 means 0, else means 1).
         */
        public setAllBits(element: number): void {
            if (element === 0) {
                for (let i = 0; i < this.getUsedCapacity(); i++) {
                    this.buffer[i] = 0;
                }
            } else {
                for (let i = 0; i < this.getUsedCapacity(); i++) {
                    this.buffer[i] = ~0x0;
                }
            }
        }

        /**
         * Resize the bitVector's length.
         *
         * @param { number } size - The new size for bitVector. If count is greater than the current size of bitVector,
         * the additional bit elements are set to 0.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                            1.Mandatory parameters are left unspecified.
         *                            2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of size is out of range.
         */
        public resize(size: number): void {
            this.integerCheck("size", size);

            if (size < 0) {
                throw new BusinessError(OutOfBoundsErrorCodeId, new Error(`The value of \"length\" is out of range. It must be >= 0. Received value is: ${size}`));
            }

            let sizeInt: int = size as int;
            if (this._length >= sizeInt) {
                this._length = sizeInt;
                return;
            }

            let after = this.computeElementIdAndBitId(sizeInt);
            let before = this.computeElementIdAndBitId(this.length);

            this.checkAndIncrement(before.elementId);
            this.buffer[before.elementId] = this.setBits(this.buffer[before.elementId], before.index, BitVector.BIT_SET_LENGTH - 1, 0);

            if (before.elementId != after.elementId) {
                ++before.elementId;
                while (before.elementId <= after.elementId) {
                    this.buffer.push(0);
                    ++before.elementId;
                }
            }
            this._length = sizeInt;
        }

        /**
         * Counts the number of times a certain bit element occurs within a range of bits in a bit vector.
         *
         * @param { number } element - Element to be counted (0 means 0, else means 1).
         * @param { number } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { number } toIndex - The end of the index, excluding the value at that index.
         * @returns { number } The number type, return the number of times a certain bit element
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public getBitCountByRange(element: number, fromIndex: number, toIndex: number): number {
            this.checkRange(fromIndex, toIndex);

            let from = this.computeElementIdAndBitId(fromIndex);
            let to =  this.computeElementIdAndBitId(toIndex);

            if (from.elementId === to.elementId) {
                let count = this.countBit1InRange(this.buffer[from.elementId], from.index, to.index - 1);
                return element === 0 ? (toIndex - fromIndex - count) : count;
            }

            let count = this.countBit1InRange(this.buffer[from.elementId], from.index, BitVector.BIT_SET_LENGTH - 1);

            ++from.elementId;
            while (from.elementId < to.elementId) {
                count += this.countBit1InRange(this.buffer[from.elementId], 0, BitVector.BIT_SET_LENGTH - 1);
                ++from.elementId;
            }

            if (to.index != 0) {
                count += this.countBit1InRange(this.buffer[to.elementId], 0, to.index - 1);
            }

            return element === 0 ? (toIndex - fromIndex - count) : count;
        }

        private checkAndIncrement(index: number): void {
            if (this.buffer.length === index) {
                this.buffer.push(0);
            }
        }

        private checkRange(fromIndex: number, toIndex: number): void {
            this.integerCheck("fromIndex", fromIndex);
            this.integerCheck("toIndex", toIndex);

            if (toIndex < 0 || toIndex > this._length) {
                throw new BusinessError(OutOfBoundsErrorCodeId, new Error(`The value of \"toIndex\" is out of range. It must be >= 0 && <= ${this._length}. Received value is: ${toIndex}`));
            }

            if (fromIndex < 0 || fromIndex >= toIndex) {
                throw new BusinessError(OutOfBoundsErrorCodeId, new Error(`The value of \"fromIndex\" is out of range. It must be >= 0 && <= ${toIndex - 1}. Received value is: ${fromIndex}`));
            }
        }

        private checkIndex(index: number): void {
            if (index < 0 || index >= this._length) {
                throw new BusinessError(OutOfBoundsErrorCodeId, new Error(`The value of \"index\" is out of range. It must be >= 0 && <= ${this._length - 1}. Received value is: ${index}`));
            }
        }

        private integerCheck(name: string, value: number): void {
            if (!Double.isInteger(value)) {
                throw new BusinessError(TypeErrorCodeId, new Error(`The type of \"${name}\" must be integer. Received value is: ${value}`));
            }
        }

        private getUsedCapacity(): number {
            return Math.ceil(this._length as number / BitVector.BIT_SET_LENGTH);
        }

        private computeElementIdAndBitId(index: number): BitIndex {
            let elementId = Math.floor(index / BitVector.BIT_SET_LENGTH);
            let bitId = index % BitVector.BIT_SET_LENGTH;
            return new BitIndex(elementId, bitId);
        }

        private computeIndex(elementId: number, bitId: number): number {
            return elementId * BitVector.BIT_SET_LENGTH + bitId;
        }

        private getBitUnsafe(index: number): number {
            let bitIndex = this.computeElementIdAndBitId(index);
            let value = (this.buffer[bitIndex.elementId] >> bitIndex.index) & 1;

            return value;
        }

        private setBitUnsafe(index: number, value: number): void {
            let bitIndex = this.computeElementIdAndBitId(index);
            if (value === 0) {
                let mask = ~(1 << bitIndex.index);
                this.buffer[bitIndex.elementId] = (this.buffer[bitIndex.elementId] & mask);
            } else {
                this.buffer[bitIndex.elementId] = (this.buffer[bitIndex.elementId] | (1 << bitIndex.index));
            }
        }

        private checkBit(element: int, fromIndex: number, toIndex: number, target_bit: number): boolean {
            let length = toIndex - fromIndex + 1;
            let mask: int = this.getRangeMask(fromIndex, length);

            if (target_bit !== 0) {
                return (element & mask) != 0;
            } else {
                return (element & mask) != mask;
            }
        }

        private flipBits(num: int, fromIndex: number, toIndex: number): int {
            let length = toIndex - fromIndex + 1;
            let mask = this.getRangeMask(fromIndex, length);

            return num ^ mask;
        }

        private getRangeMask(fromIndex: number, length: number): int {
            let mask: int = 0x0;

            if (length == BitVector.BIT_SET_LENGTH) {
                mask = ~mask;
            } else {
                mask = ((1 << length) - 1) << fromIndex;
            }

            return mask;
        }

        private getBits(num: int, fromIndex: number, toIndex: number): int {
            let length = toIndex - fromIndex + 1;
            let mask: int = this.getRangeMask(fromIndex, length);
            return (num & mask) >> fromIndex;
        }

        private setBits(num: int, fromIndex: number, toIndex: number, element: number): int {
            let length = toIndex - fromIndex + 1;
            let mask: int = this.getRangeMask(fromIndex, length);
            if (element != 0) {
                return num | mask;
            } else {
                return num & ~mask;
            }
        }

        private countBit1InRange(num: int, fromIndex: number, toIndex: number): number {
            let length = toIndex - fromIndex + 1;
            let mask: int = this.getRangeMask(fromIndex, length);

            let masked_value = num & mask;

            let count = 0;
            while (masked_value != 0) {
                masked_value = masked_value & (masked_value - 1);
                count++;
            }

            return count;
        }
    }
}
