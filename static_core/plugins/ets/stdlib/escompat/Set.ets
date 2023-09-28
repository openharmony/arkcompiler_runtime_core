/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

package escompat;

/**
 * Set implementation via tree
 */
export final class Set<K extends Comparable<Object>> {
    private map: Map<K, Boolean>;

    /**
     * Constructs an empty TreeSet
     */
    constructor() {
        this.map = new Map<K, Boolean>();
    }

    /**
     * Puts a value into the Set
     *
     * @param v the value to put into the Set
     */
    add(v: K): void {
        this.map.set(v, true);
    }

    /**
     * Checks if a value is in the Set
     *
     * @param v the value to find in the Set
     *
     * @returns true if the value is in the Set
     */
    has(v: K): boolean {
        return this.map.has(v);
    }

    /**
     * Returns number of unique elements in the Set
     *
     * @returns number of unique elements in the Set
     */
    length(): int {
        return this.map.length();
    }

    /**
     * Removes a value from the Set
     *
     * @param v the value to remove
     */
    delete(v: K): void {
        this.map.delete(v);
    }

    /**
     * Deletes all elements from the Set
     */
    clear(): void {
        this.map.clear();
    }

    /**
     * Returns elements from the Set as an array
     *
     * @returns an array of Set values
     */
    values(): ValuesIterator<K> {
        return this.map.keys();
    }

    /**
     * Applies a function over all elements of the Set
     *
     * @param fn to apply
     */
    forEach(fn: (k: K) => void): void {
        let l: (v: Boolean, k: K) => void = (v: Boolean, k: K): void => { fn(k) }
        throw new Error("Set.forEach: not implemented")
        // this.map.forEach(l)
    }
}
