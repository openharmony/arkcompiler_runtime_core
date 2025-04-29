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

package escompat;

export interface ReadonlySet<T> extends Iterable<T> {
    /**
     * Checks if a value is in the Set
     *
     * @param v the value to find in the Set
     *
     * @returns true if the value is in the Set
     */

    has(value: T): boolean;

    /**
     * Returns number of unique elements in the Set
     *
     * @returns number of unique elements in the Set
     */
    get size(): number;

    /**
     * Executes a provided function once per each value in the Set object, in insertion order
     *
     * @param callbackfn to apply; value2 is always same as value1
     */
    forEach(callbackfn: (value: T, value2: T, set: ReadonlySet<T>) => void): void;

    /**
     * Despite name, returns elements from the Set
     * @returns an iterable of the values in the set
     */
    keys(): IterableIterator<T>;

    /**
     * Returns elements from the Set
     *
     * @returns an iterable of the values in the set
     */
    values(): IterableIterator<T>;

    /**
     * @returns an iterable of [v,v] pairs for every value `v` in the set.
     */
    entries(): IterableIterator<[T, T]>;
}

/**
 * Set implementation
 */
export class Set<K> implements ReadonlySet<K> {
    private readonly elements: Map<K, K>

    /**
     * Constructs a Set from collection
     * @param elements initial collection
     */
    constructor(elements?: Iterable<K> | FixedArray<K> | null) {
        if (elements == null) {
            this.elements = new Map<K, K>();
        } else if (elements instanceof Iterable) {
            const it = (elements as Iterable<K>).$_iterator()
            const entriesIter = new MappingIterator<K, [K, K]>(it, (element: K): [K, K] => [element, element])
            this.elements = new Map<K, K>(entriesIter)
        } else {
            const entries = new ([K, K] | null | undefined)[elements.length]
            for (let i = 0; i < elements.length; ++i) {
                entries[i] = [elements[i], elements[i]] as [K, K]
            }
            this.elements = new Map<K, K>(entries)
        }
    }

    /**
     * Constructs a Set from another Set
     * @param set another Set
     */
    constructor(set: Set<K>) {
        this.elements = new Map<K, K>(set.elements)
    }

    // NOTE(templin.konstantin): Temporary workaround due #20944 internal issue
    //  if we remove this constructor we got error on "readonly K[]" branch
    //  at constructor from union type
    constructor(values: K[]) {
        this.elements = new Map<K, K>()
        for (let i = 0; i < values.length; ++i) {
            this.elements.set(values[i], values[i])
        }
    }

    private toStringVals(): String {
        const strBuf = new StringBuilder()

        const valsIter = this.values()
        let valsIterRes = valsIter.next()
        while (!valsIterRes.done) {
            strBuf.append(`${valsIterRes.value}`)

            valsIterRes = valsIter.next()
            if (!valsIterRes.done) {
                strBuf.append(",")
            }
        }

        return strBuf.toString()
    }

    override toString(): String {
        return "Set[" + this.toStringVals() + "]"
    }

    /**
     * Puts a value into the Set
     *
     * @param val the value to put into the Set
     * @returns this
     */
    add(val: K): this {
        this.elements.set(val, val)
        return this
    }

    /**
     * Checks if a value is in the Set
     *
     * @param val the value to find in the Set
     *
     * @returns true if the value is in the Set
     */
    override has(val: K): boolean {
        return this.elements.has(val)
    }

    /**
     * Returns number of unique elements in the Set
     *
     * @returns number of unique elements in the Set
     */
    override get size(): number {
        return this.elements.size;
    }

    /**
     * Removes a value from the Set
     *
     * @param val the value to remove
     */
    delete(val: K): boolean {
        return this.elements.delete(val)
    }

    /**
     * Deletes all elements from the Set
     */
    clear(): void {
        this.elements.clear()
    }

    /**
     * Despite name, returns elements from the Set
     * @returns an iterable of the values in the set
     */
    override keys(): IterableIterator<K> {
        return this.elements.keys()
    }

    /**
     * Returns elements from the Set
     *
     * @returns an iterable of the values in the set
     */
    override values(): IterableIterator<K> {
        return this.elements.keys()
    }

    override $_iterator(): IterableIterator<K> {
        return this.values();
    }

    /**
     * @returns an iterable of [v,v] pairs for every value `v` in the set.
     */
    override entries(): IterableIterator<[K, K]> {
        return this.elements.entries()
    }

    /**
     * Executes a provided function once per each value in the Set object, in insertion order
     *
     * @param callbackfn to apply
     */
    override forEach(callbackfn: (k: K, v: K, set: Set<K>) => void): void {
        iteratorForEach<K>(this.keys(), (x: K): void => callbackfn(x, x, this))
    }
}

class MappingIterator<I, O> implements IterableIterator<O> {
    private readonly inputIter: Iterator<I>
    private readonly mappingFn: (input: I) => O

    constructor(inputIter: Iterator<I>, mappingFn: (input: I) => O) {
        this.inputIter = inputIter
        this.mappingFn = mappingFn
    }

    override next(): IteratorResult<O> {
        const inputIterRes = this.inputIter.next()

        if (inputIterRes.done) {
            return new IteratorResult<O>()
        } else {
            const output = this.mappingFn(inputIterRes.value as I)
            return new IteratorResult<O>(output)
        }
    }
}
