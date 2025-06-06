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

import TreeMap from '@ohos.util.TreeMap';

export interface ReadonlyTreeSet<T> extends Iterable<T> {
    /**
     * Return the number of elements in the TreeSet
     *
     * @returns the number of elements in the TreeSet
     */
    get length(): number;

    /**
     * Check whether the given value is in the TreeSet
     *
     * @param value: the value to find in the TreeSet
     *
     * @returns true if the value is in the TreeSet
     */
    has(value: T): boolean;

    /**
     * Execute a provided function once per each value in the TreeSet
     *
     * @param callbackfn: the function to apply, the key is always same as the value
     */
    forEach(callbackfn: (value: T, key: T, set: ReadonlyTreeSet<T>) => void): void;

    /**
     * Return the elements in the TreeSet as an Iterator of values
     *
     * @returns an Iterator of values
     */
    values(): IterableIterator<T>;

    /**
     * Return the elements in the TreeSet as an Iterator of [value, value]s
     *
     * @returns an Iterator of [value, value]s
     */
    entries(): IterableIterator<[T, T]>;
}

export class TreeSet<T> implements ReadonlyTreeSet<T> {
    private treeMap: TreeMap<T, T>;

    /**
     * Construct a TreeSet
     *
     * @param comparator: the comparator of the TreeSet
     */
    constructor(comparator?: (firstValue: T, secondValue: T) => number) {
        this.treeMap = new TreeMap<T, T>(comparator);
    }

    /**
     * Return the number of elements in the TreeSet
     *
     * @returns the number of elements in the TreeSet
     */
    override get length(): number {
        return this.treeMap.length;
    }

    /**
     * Check whether the given value is in the TreeSet
     *
     * @param value: the value to find in the TreeSet
     *
     * @returns true if the value is in the TreeSet
     */
    override has(value: T): boolean {
        return this.treeMap.hasKey(value);
    }

    /**
     * Execute a provided function once per each value in the TreeSet
     *
     * @param callbackfn: the function to apply, the key is always same as the value
     */
    forEach(callbackfn: (value: T, key: T, set: TreeSet<T>) => void): void {
        this.treeMap.forEach((value: T, key: T) => {
            callbackfn(value, key, this);
        })
    }

    /**
     * Return the elements in the TreeSet as an Iterator of values
     *
     * @returns an Iterator of values
     */
    values(): IterableIterator<T> {
        return this.treeMap.keys();
    }

    /**
     * Return the elements in the TreeSet as an Iterator of [value, value]s
     *
     * @returns an Iterator of [value, value]s
     */
    entries(): IterableIterator<[T, T]> {
        return this.treeMap.entries();
    }

    /**
     * Add a new element into the TreeSet
     *
     * @param value: the value to be added into the TreeSet
     *
     * @returns true if the element is successfully added into the TreeSet
     */
    add(value: T): boolean {
        if (this.has(value)) {
            return false;
        } else {
            this.treeMap.set(value, value)
            return true;
        }
    }

    /**
     * Delete all elements of the TreeSet
     */
    clear(): void {
        this.treeMap.clear();
    }

    /**
     * Return the value of the first element in the TreeSet
     *
     * @returns the value of the first element if exists
     */
    getFirstValue(): T | undefined {
        return this.treeMap.getFirstKey();
    }

    /**
     * Return the higher value of the given key's element in the TreeSet
     *
     * @param key: the key of the element which will be get the higher value
     *
     * @returns the higher value of the given key's element if exists or undefined
     */
    getHigherValue(key: T): T | undefined {
        return this.treeMap.getHigherKey(key);
    }

    /**
     * Return the value of the last element in the TreeSet
     *
     * @returns the value of the last element if exists
     */
    getLastValue(): T | undefined {
        return this.treeMap.getLastKey();
    }

    /**
     * Return the lower value of the given key's element in the TreeSet
     *
     * @param key:the key of the element which will be get the lower value
     *
     * @returns the lower value of the given key's element if exists or undefined
     */
    getLowerValue(key: T): T | undefined {
        return this.treeMap.getLowerKey(key);
    }

    /**
     * Check if the TreeSet is empty
     *
     * @returns true if the TreeSet is empty
     */
    isEmpty(): boolean {
        return this.treeMap.isEmpty();
    }

    /**
     * Remove the first element in the TreeSet
     *
     * @returns the value of the first element in the TreeSet if exists
     */
    popFirst(): T | undefined {
        if (this.isEmpty()) {
            return undefined;
        } else {
            const res = this.getFirstValue()!;
            this.remove(res);
            return res;
        }
    }

    /**
     * Remove the last element in the TreeSet
     *
     * @returns the value of the last element in the TreeSet if exists
     */
    popLast(): T | undefined {
        if (this.isEmpty()) {
            return undefined;
        } else {
            const res = this.getLastValue()!;
            this.remove(res);
            return res;
        }
    }

    /**
     * Remove an element from the TreeSet
     *
     * @param value: the value of the element which will be remove from the TreeSet
     *
     * @returns true if the element is successfully removed from the TreeSet
     */
    remove(value: T): boolean {
        if (this.has(value)) {
            this.treeMap.remove(value);
            return true;
        } else {
            return false;
        }
    }

    /**
     * Returns an iterator for the TreeSet
     *
     * @returns An iterator for the TreeSet
     */
    override $_iterator(): IterableIterator<T> {
        return this.treeMap.keys();
    }
}
