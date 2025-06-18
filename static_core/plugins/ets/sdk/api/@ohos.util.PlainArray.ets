/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

import LightWeightMap from '@ohos.util.LightWeightMap';
import { BusinessError } from "@ohos.base";

const TypeErrorCodeId: number = 401;
const OutOfBoundsErrorCodeId: number = 10200001;

/**
 * The PlainArray class is a collection of key-value pairs.
 */
export class PlainArray<T> implements Iterable<[Number, T]> {
    private buckets: LightWeightMap<number, T>;

    /**
     * Returns number of Entries with unique keys in the PlainArray
     *
     * @returns number of Entries with unique keys in the PlainArray
     */
    public get length(): number {
        return this.buckets.length;
    }

    /**
     * Add a set of data to PlainArray.
     * @param key the key of the element to add
     */
    public add(key: number, value: T): void {
        this.checkKey(key);
        this.buckets.set(key, value);
    }

    /**
     * Clear all elements in PlainArray
     */
    public clear(): void {
        this.buckets.clear();
    }

    /**
     * Clone an PlainArray
     * @returns a new PlainArray object
     */
    public clone(): PlainArray<T> {
        const clone: PlainArray<T> = new PlainArray<T>()
        this.buckets.forEach((value: T, key: number) => {
            clone.add(key, value);
        })

        return clone;
    }

    /**
     * Creates a new instance of PlainArray
     */
    public constructor() {
        this.buckets = new LightWeightMap<number, T>();
    }

    /**
     * Callback function is used to iterate over the elements on the instance object and their corresponding subscripts.
     * @param callbackFn to apply
     * @param callbackFn.value the value of the element
     * @param callbackFn.key the key of the element
     * @param callbackFn.PlainArray the instance of this object
     */
    public forEach(callbackFn: (value: T, key: number, PlainArray: PlainArray<T>) => void): void {
        this.buckets.forEach((value: T, key: number) => {
            callbackFn(value, key, this);
        });
    }

    /**
     * Gets the value of the key.
     * @param key the key of the element to get
     * @returns the value of the key
     */
    public get(key: number): T | undefined {
        this.checkKey(key);
        return this.buckets.get(key);
    }

    /**
     * Gets the index of the key
     * @param key the key of the element to get the index of
     *
     * @returns the index of the key, or -1 if the key is not found
     */
    public getIndexOfKey(key: number): number {
        this.checkKey(key);

        return this.buckets.getIndexOfKey(key);
    }

    /**
     * Gets the first occurrence of the index for the value element
     * @param value the value of the element to get the index of
     *
     * @returns the index of the value, or -1 if the value is not found
     */
    public getIndexOfValue(value: T): number {
        return this.buckets.getIndexOfValue(value);
    }

    /**
     * Finds the key of the index.
     * @param index the index of the element to get the key of
     *
     * @returns the key of the index
     * 
     * @throws BusinessError if the index is out of range.
     */
    public getKeyAt(index: number): number | undefined {
        this.checkIndexType(index);
        if (index < 0 || index >= this.length) {
            return undefined;
        }

        return this.buckets.getKeyAt(index)!;
    }

    /**
     * Finds the value of the index.
     * @param index the index of the element to get the value of
     *
     * @returns the value of the index, or -1 if the index is not found
     */
    public getValueAt(index: number): T {
        this.checkIndexType(index);
        this.checkEmptyContainer();
        this.checkIndex(index, this.length - 1);

        return this.buckets.getValueAt(index)!;
    }

    /**
     * Determines whether the PlainArray contains the key.
     * @param key the key to check
     *
     * @returns true if the key is in the PlainArray, false otherwise
     */
    public has(key: number): boolean {
        this.checkKey(key);
        return this.buckets.hasKey(key);
    }

    /**
     * Determines whether the PlainArray is empty.
     *
     * @returns true if the PlainArray is empty, false otherwise
     */
    public isEmpty(): boolean {
        return this.buckets.isEmpty();
    }

    /**
     * Remove the key - value pair for the key.
     * @param key the key of the element to remove
     *
     * @returns the value of the key, or undefined if the key is not found
     */
    public remove(key: number): T | undefined {
        this.checkKey(key);
        return this.buckets.remove(key);
    }

    /**
     * Remove the key-value pair for the index.
     * @param index the index of the element to remove
     *
     * @returns the value of the index, or undefined if the index is not found
     */
    public removeAt(index: number): T | undefined {
        this.checkIndexType(index);
        if (index < 0 || index >= this.length) {
            return undefined;
        }

        const value: T | undefined = this.getValueAt(index);
        if (value !== undefined) {
            this.buckets.removeAt(index);
        }
        return value;
    }

    /**
     * Delete a range of elements.
     * @param index the index of the element to remove
     * @param size the number of elements to remove
     *
     * @returns the number of elements removed
     */
    public removeRangeFrom(index: number, size: number): number {
        this.checkIndexType(index);
        this.checkEmptyContainer();
        if (size > Int.MAX_VALUE) {
            throw new BusinessError(TypeErrorCodeId,
                new Error(`The type of \"size\" must be small integer. Received value is: ${size}`));
        }
        this.checkIndex(index, this.length - 1);
        if (size < 1) {
            throw new BusinessError(OutOfBoundsErrorCodeId, new Error(`The value of \"size\" is out of range. It must be > 0. Received value is: ${size}`));
        }

        const elemNum = this.length - index;
        let removedSize: number = 0;
        while(removedSize < min(size, elemNum)) {
            this.buckets.removeAt(index);
            removedSize++;
        }
        return removedSize;
    }

    /**
     * Replace the container specified in the index corresponding keys on the key-value pair.
     * @param index the index of the element to replace
     * @param value the value of the element to replace
     * 
     * @throws BusinessError if the index is out of range or the container is empty.
     */
    public setValueAt(index: number, value: T): void {
        this.checkIndexType(index);
        this.checkEmptyContainer();
        this.checkIndex(index, this.length - 1);

        this.buckets.setValueAt(index, value);
    }

    /**
     * Gets a string containing all the keys and values in the PlainArray.
     *
     * @returns a string containing all the keys and values in the PlainArray
     */
    public toString(): string {
        return this.buckets.toString();
    }

    /*
     * Returns an iterator object containing a key-value pair
     */
    override $_iterator(): IterableIterator<[Number, T]> {
        return this.buckets.$_iterator();
    }

    private checkIndexType(index: number): void {
        if (index > Int.MAX_VALUE) {
            throw new BusinessError(TypeErrorCodeId, new Error(`The type of \"index\" must be small integer. Received value is: ${index}`));
        }
    }

    private checkEmptyContainer(): void {
        if (this.isEmpty()) {
            throw new BusinessError(OutOfBoundsErrorCodeId, new Error(`Container is empty`));
        }
    }

    private checkIndex(index: number, length: number): void {
        if (index < 0 || index > length) {
            throw new BusinessError(OutOfBoundsErrorCodeId, new Error(`The value of \"index\" is out of range. It must be >= 0 && <= ${length}. Received value is: ${index}`));
        }
    }

    private checkKey(key: number): void {
        if (key > Int.MAX_VALUE) {
            throw new BusinessError(TypeErrorCodeId, new Error(`The type of \"key\" must be small integer. Received value is: ${key}`));
        }
    }
}