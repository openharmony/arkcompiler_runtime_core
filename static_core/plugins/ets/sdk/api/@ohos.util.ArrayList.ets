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

const TypeErrorCodeId: number = 401;
const OutOfBoundsErrorCodeId: number = 10200001;
const IncorrectBufferSizeId: number = 10200009;
const CapacityErrorCodeId: number = 10200062;
const INT_MAX_VALUE_OFFSET: number = 4294967296;

export class ArrayList<T> implements Iterable<T>, JsonReplacer {
    private readonly DEFAULT_CAPACITY = 10;
    private capacity: int = this.DEFAULT_CAPACITY;
    private buffer: Array<T>;

    public jsonReplacer(): Record<String, NullishType> {
        const buf = this.buffer;
        const len = buf.length;
        let arrayObj: Record<String, NullishType> = {};
        for (let i = 0; i < len; i++) {
            arrayObj[String(i)] = buf[i];
        }
        return arrayObj;
    }

    /**
     * property: The number of elements in the ArrayList.
     */
    get length(): number {
        return this.buffer.length;
    }

    /**
     * Creates a new instance of ArrayList.
     */
    public constructor() {
        this.buffer = new Array<T>(this.capacity);
        this.buffer.length = 0;
    }

    private constructor(cap: int) {
        this.capacity = cap;
        this.buffer = new Array<T>(this.capacity);
        this.buffer.length = 0;
    }

    /**
     * Add new elements to the ArrayList.
     *
     * @param element - the added element.
     *
     * @returns true if added successfully.
     */
    public add(element: T): boolean {
        if (this.length >= Int.MAX_VALUE) {
            throw new BusinessError(CapacityErrorCodeId, new Error(`The current capacity has reached the maximum limit.`));
        }

        if (this.length == this.capacity) {
            this.increaseCap((this.capacity * 1.5).toInt());
        }
        this.buffer.push(element);
        return true;
    }

    /**
     * Insert new elements at a specific position in the ArrayList.
     *
     * @param element - the added element.
     *
     * @param index - the index.
     */
    public insert(element: T, index: number): void {
        this.checkIndexType(index);
        this.checkIndex(index, this.length);

        if (this.length + 1 <= this.capacity) {
            this.buffer.push(element); // length++
            for (let i: int = (this.length - 1).toInt(); i > index; i--) {
                this.buffer[i] = this.buffer[i - 1];
            }
            this.buffer[index.toInt()] = element;
        } else {
            this.increaseCap((this.capacity * 1.5).toInt());

            let newBuffer: Array<T> = new Array<T>(this.capacity);
            newBuffer.length = 0;
            for (let i: int = 0; i < index; i++) {
                newBuffer.push(this.buffer[i]);
            }
            newBuffer.push(element);
            for (let i: int = index.toInt(); i < this.length; i++) {
                newBuffer.push(this.buffer[i]);
            }

            this.buffer = newBuffer;
        }
    }

    /**
     * Check whether the specified element is contained in the ArrayList.
     *
     * @param element - the specified element.
     *
     * @returns true if the specified element is included, otherwise, false.
     */
    public has(element: T): boolean {
        return this.getIndexOf(element) != -1;
    }

    /**
     * Get the index of the specified element when it first appeared.
     *
     * @param element - the specified element.
     *
     * @returns the index if the specified element is included, otherwise, -1.
     */
    public getIndexOf(element: T): number {
        return this.buffer.findIndex((item: T): boolean => {
            return item == element;
        });
    }

    /**
     * Get the index of the specified element when it last appeared.
     *
     * @param element - the specified element.
     *
     * @returns the index if the specified element is included, otherwise, -1.
     */
    public getLastIndexOf(element: T): number {
        return this.buffer.findLastIndex((item: T): boolean => {
            return item == element;
        });
    }

    /**
     * Find an element based on its index, and delete it when it is returned.
     *
     * @param index - the index.
     *
     * @returns the deleted element.
     */
    public removeByIndex(index: number): T {
        this.checkIndexType(index);
        this.checkEmptyContainer();
        this.checkIndex(index, this.length - 1);

        let delItem: T = this.buffer[index];

        for (let i: int = index.toInt() + 1; i < this.length; i++) {
            this.buffer[i - 1] = this.buffer[i];
        }
        this.buffer.length--

        return delItem;
    }

    /**
     * Deletes the first specified element found.
     *
     * @param element - the specified element.
     *
     * @returns true if the specified element is included, otherwise, false.
     */
    public remove(element: T): boolean {
        let index: number = this.getIndexOf(element);
        if (index != -1) {
            this.removeByIndex(index);
            return true;
        }
        return false;
    }

    /**
     * Deletes elements from a range, including the start value but not the end value.
     *
     * @param fromIndex - the start index.
     *
     * @param toIndex - the end index.
     */
    public removeByRange(fromIndex: number, toIndex: number): void {
        this.checkEmptyContainer();
        this.checkRange(fromIndex, toIndex);

        for (let i: int = fromIndex.toInt(), j: int = toIndex.toInt(); j < this.length; i++, j++) {
            this.buffer[i] = this.buffer[j];
        }

        this.buffer.length -= toIndex - fromIndex;
    }

    /**
     * Replaces all elements in the ArrayList using a callback function that takes the value and index, and itself.
     *
     * @param callbackfn - The callback function taking the value, index, and the ArrayList as arguments.
     *
     */
    public replaceAllElements(callbackfn: (value: T, index: number, arrList: ArrayList<T>) => T): void {
        for (let i: int = 0; i < this.length; i++) {
            this.buffer[i] = callbackfn(this.buffer[i], i.toDouble(), this);
        }
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param callbackfn - The function to execute on each element, taking the value, index, and ArrayList as arguments.
     *
     */
    public forEach(callbackfn: (value: T, index: number, arrList: ArrayList<T>) => void): void {
       for (let i: int = 0; i < this.length; i++) {
            callbackfn(this.buffer[i], i.toDouble(), this);
        }
    }

    /**
     * Sorts the elements of an ArrayList in place and returns the sorted ArrayList.
     *
     * @param comparator - Optional. A function that defines the sort order.
     */

    public sort(comparator?: (firstValue: T, secondValue: T) => number): void {
        this.buffer.sort(comparator);
    }

    /**
     * Returns a new ArrayList containing a portion of the original ArrayList.
     *
     * @param fromIndex - The starting index of the portion to be copied.
     *
     * @param toIndex - The ending index of the portion to be copied.
     *
     * @returns A new ArrayList containing the specified portion of the original ArrayList.
     */
    public subArrayList(fromIndex: number, toIndex: number): ArrayList<T> {
        this.checkEmptyContainer();
        this.checkRange(fromIndex, toIndex);

        let subArrLength = toIndex - fromIndex;
        let newBuffer: ArrayList<T> = new ArrayList<T>(subArrLength.toInt());
        for (let i: int = fromIndex.toInt(); i < toIndex; i++) {
            newBuffer.add(this.buffer[i]);
        }

        return newBuffer;
    }

    /**
     * Removes all elements from the ArrayList.
     */
    public clear(): void {
        this.capacity = this.DEFAULT_CAPACITY;
        this.buffer = new Array<T>(this.capacity);
        this.buffer.length = 0;
    }

    /**
     * Creates a shallow copy of the ArrayList.
     *
     * @returns A new ArrayList that is a shallow copy of the original ArrayList.
     */
    public clone(): ArrayList<T> {
        let newBuffer: ArrayList<T> = new ArrayList<T>(this.capacity);
        for (let i: int = 0; i < this.length; i++) {
            newBuffer.add(this.buffer[i]);
        }

        return newBuffer;
    }

    /**
     * Returns the capacity of the ArrayList.
     *
     * @returns The capacity of the ArrayList.
     */
    public getCapacity(): number {
        return this.capacity;
    }

    /**
     * Converts the ArrayList to a standard JavaScript Array.
     *
     * @returns A new Array containing all elements of the ArrayList.
     */
    public convertToArray(): Array<T> {
        return Array.from(this.buffer);
    }

    /**
     * Checks if the ArrayList is empty.
     *
     * @returns true if the ArrayList contains no elements, otherwise false.
     */
    public isEmpty(): boolean {
        return this.length == 0;
    }

    /**
     * Gets the element at the specified index.
     *
     * @param index - The index of the element to retrieve.
     *
     * @returns The element at the specified index.
     */
    public $_get(index: number): T {
        if (index > Int.MAX_VALUE) {
            throw new BusinessError(TypeErrorCodeId, new Error(`The type of \"index\" must be small integer.`));
        }
        this.checkEmptyContainer();
        this.checkIndex(index, this.length - 1);

        return this.buffer[index];
    }

    /**
     * Sets the element at the specified index.
     *
     * @param index - The index of the element to set.
     *
     * @param val - The value to set at the specified index.
     */
    public $_set(index: number, val: T): void {
        this.checkEmptyContainer();
        if (index < 0 || index > Int.MAX_VALUE) {
            return;
        }
        this.checkIndex(index, this.length - 1);

        this.buffer[index] = val;
    }

    /**
     * Increases the capacity of the ArrayList to at least the given new capacity.
     *
     * @param newCapacity - The minimum new capacity for the ArrayList.
     */
    public increaseCapacityTo(newCapacity: number): void {
        if (newCapacity > Int.MAX_VALUE) {
            throw new BusinessError(CapacityErrorCodeId, new Error(`The current capacity has reached the maximum limit.`));
        }

        if (newCapacity > this.capacity) {
            this.increaseCap(newCapacity.toInt());
        }
    }

    /**
     * Trims the capacity of the ArrayList to fit just the current size.
     */
    public trimToCurrentLength(): void {
        let newBuffer: Array<T> = new Array<T>(this.length);
        for (let i: int = 0; i < this.length; i++) {
            newBuffer[i] = this.buffer[i];
        }
        this.capacity = this.length.toInt();

        this.buffer = newBuffer;
    }

    /**
     * Returns an iterator over the elements in the ArrayList.
     *
     * @returns An iterator over the elements in the ArrayList.
     */
    public override $_iterator(): IterableIterator<T> {
        return this.buffer.values();
    }

    private increaseCap(cap: int): void {
        let newBuffer: Array<T> = new Array<T>(cap);
        newBuffer.length = 0;

        for (let i: int = 0; i < this.length; i++) {
            newBuffer.push(this.buffer[i]);
        }

        this.capacity = cap;
        this.buffer = newBuffer;
    }

    private checkEmptyContainer(): void {
        if (this.isEmpty()) {
            throw new BusinessError(OutOfBoundsErrorCodeId, new Error(`Container is empty`));
        }
    }

    private checkIndexType(index: number): void {
        if (index > Int.MAX_VALUE) {
            throw new BusinessError(TypeErrorCodeId, new Error(`The type of "index" must be small integer. Received value is: ${index}`));
        }
    }

    private checkIndex(index: number, length: number): void {
        if (index > length || index < 0) {
            this.createBusinessError('index', length, index);
        }
    }

    private checkRange(fromIndex: number, toIndex: number): void {
        if (fromIndex > Int.MAX_VALUE) {
            this.createBusinessError('fromIndex', min(this.length - 1, toIndex - 1), fromIndex - INT_MAX_VALUE_OFFSET);
        }
        if (fromIndex < 0 || fromIndex >= toIndex || fromIndex >= this.length) {
            this.createBusinessError('fromIndex', min(this.length - 1, toIndex - 1), fromIndex);
        }
        if (toIndex > Int.MAX_VALUE) {
            this.createBusinessError('toIndex', this.length, toIndex - INT_MAX_VALUE_OFFSET);
        }
        if (toIndex > this.length) {
            this.createBusinessError('toIndex', this.length, toIndex);
        }
    }

    private createBusinessError(index: string, upperLimit: number, currentValue: number): void {
        throw new BusinessError(OutOfBoundsErrorCodeId, new Error(`The value of \"${index}\" is out of range. It must be >= 0 && <= ${upperLimit}. Received value is: ${currentValue}`));
    }
}
