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

import LightWeightMap from '@ohos.util.LightWeightMap';

interface ReadonlyLightWeightSet<T> extends Iterable<T> {
    /**
     * Checks if a value is in the LightWeightSet
     *
     * @param v the value to find in the LightWeightSet
     *
     * @returns true if the value is in the LightWeightSet
     */
    has(key: T): boolean;

    /**
     * Returns number of unique elements in the LightWeightSet
     *
     * @returns number of unique elements in the LightWeightSet
     */
    get length(): number;

    /**
     * Executes a provided function once per each value in the LightWeightSet object, in insertion order
     *
     * @param callbackfn to apply; key is always same as value
     */
    forEach(callbackFn: (value: T, key: T, set: ReadonlyLightWeightSet<T>) => void): void;

    /**
     * Returns elements from the LightWeightSet
     *
     * @returns an iterable of the values in the LightWeightSet
     */
    values(): IterableIterator<T>;

    /**
     * @returns an iterable of [v,v] pairs for every value `v` in the LightWeightSet.
     */
    entries(): IterableIterator<[T, T]>;
}

export class LightWeightSet<T> implements ReadonlyLightWeightSet<T> {
    private buckets: LightWeightMap<T, T>;

    private static readonly SHIFT_COUNT = 31;

    private iteratorForEach<V>(x: Iterator<V>, fn: (x: V) => void): void {
        while (true) {
            const v = x.next();
            if (v.done) {
                return;
            }
            fn(v.value as V);
        }
    }

    constructor() {
        this.buckets = new LightWeightMap<T, T>();
    }

    /**
     * Returns number of unique elements in the LightWeightSet
     *
     * @returns number of unique elements in the LightWeightSet
     */
    get length(): number {
        return this.buckets.length;
    }

    /**
     * Adds a value to the LightWeightSet
     *
     * @param value the value to add to the LightWeightSet
     *
     * @returns true if the value was added, false otherwise
     */
    add(obj: T): boolean {
        if (this.buckets.hasKey(obj)) {
            return false;
        }
        this.buckets.set(obj, obj);
        return true;
    }

    /**
     * Checks if a value is in the LightWeightSet
     *
     * @param v the value to find in the LightWeightSet
     *
     * @returns true if the value is in the LightWeightSet
     */
    has(key: T): boolean {
        return this.buckets.hasKey(key);
    }

    /**
     * Removes a value from the LightWeightSet
     *
     * @param key the value to remove from the LightWeightSet
     *
     * @returns the removed value if it was present, undefined otherwise
     */
    remove(key: T): T | undefined {
        return this.buckets.remove(key);
    }

    /**
     * Clears all entries in the LightWeightSet
     */
    clear(): void {
        this.buckets.clear();
    }

    /**
     * Returns elements from the LightWeightSet
     *
     * @returns an iterable of the values in the LightWeightSet
     */
    values(): IterableIterator<T> {
        return this.buckets.keys();
    }

    /**
     * Returns an iterator for the LightWeightSet
     *
     * @returns An iterator for the LightWeightSet
     */
    $_iterator(): IterableIterator<T> {
        return this.values();
    }

    /**
     * @returns an iterable of [v,v] pairs for every value `v` in the LightWeightSet.
     */
    entries(): IterableIterator<[T, T]> {
        return this.buckets.entries()
    }

    /**
     * Executes a provided function once per each value in the LightWeightSet object, in insertion order
     *
     * @param callbackfn to apply; key is always same as value
     */
    forEach(callbackFn: (value: T, key: T, set: LightWeightSet<T>) => void): void {
        this.buckets.forEach((value: T, key: T): void => {
            callbackFn(value, key, this);
        });
    }

    /**
     * Adds all values from another LightWeightSet to this LightWeightSet
     *
     * @param set the LightWeightSet to add values from
     *
     * @returns true if all values were added, false otherwise
     */
    addAll(set: LightWeightSet<T>): boolean {
        let result: boolean = false;
        set.forEach((value: T): void => {
            result |= this.add(value);
        });
        return result;
    }

    /**
     * Gets the index of the specified value in the LightWeightSet
     *
     * @param key the value to find the index of
     *
     * @returns the index of the value, or a negative position if the value is not found
     */
    getIndexOf(key: T): number {
        const index = this.buckets.getIndexOfKey(key);
        if (index >= 0) {
            return index;
        } else {
            const hashCode = this.hash(key);
            let low: int = 0;
            let high: int = this.buckets.length as int - 1;
            while (low <= high) {
                const mid = (low + high) >>> 1;
                const midHash = this.hash(this.buckets.getKeyAt(mid)!);
                if (midHash < hashCode) {
                    low = mid + 1;
                } else {
                    high = mid - 1;
                }
            }
            return -(low + 1);
        }
    }

    /**
     * Gets the value at the specified index in the LightWeightSet
     *
     * @param index the index to get the value from
     *
     * @returns the value at the specified index
     */
    getValueAt(index: number): T | undefined {
        return this.buckets.getKeyAt(index);
    }

    /**
     * Checks if all values from another LightWeightSet are present in this LightWeightSet
     *
     * @param set the LightWeightSet to check values from
     *
     * @returns true if all values are present, false otherwise
     */
    hasAll(set: LightWeightSet<T>): boolean {
        if (set.length > this.buckets.length) {
            return false;
        }
        for (let index: number = 0; index < set.length; index++) {
            const value = set.getValueAt(index)!;
            if (!this.has(value)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Increases the capacity of the LightWeightSet to the specified minimum capacity
     *
     * @param minimumCapacity the minimum capacity to increase to
     */
    increaseCapacityTo(minimumCapacity: number): void {
        this.buckets.increaseCapacityTo(minimumCapacity);
    }

    /**
     * Checks if the LightWeightSet is empty
     *
     * @returns true if the LightWeightSet is empty, false otherwise
     */
    isEmpty(): boolean {
        return this.buckets.isEmpty();
    }

    /**
     * Removes the value at the specified index from the LightWeightSet
     *
     * @param index the index to remove the value from
     *
     * @returns true if the value was removed, false otherwise
     */
    removeAt(index: number): boolean {
        return this.buckets.removeAt(index);
    }

    /**
     * Converts the LightWeightSet to an array
     *
     * @returns an array containing all values in the LightWeightSet
     */
    toArray(): Array<T> {
        const res: Array<T> = new Array<T>();
        this.iteratorForEach<T>(this.values(), (value: T): void => res.push(value));
        return res;
    }

    /**
     * Returns a string representation of the LightWeightSet
     *
     * @returns a string representation of the LightWeightSet
     */
    toString(): String {
        const res: Array<T> = this.toArray();
        return res.join(',');
    }

    private hash(key: T): int {
        if (key === null) {
            return 0;
        }
        if (key === undefined) {
            return 1;
        }

        const keyHash: int = (key! as object).$_hashCode(); // #26217
        const positiveMask: int = 1 << LightWeightSet.SHIFT_COUNT;
        if (keyHash >= 0) {
            return (keyHash | positiveMask)
        } else {
            return (keyHash & (~positiveMask))
        }
    }
}
