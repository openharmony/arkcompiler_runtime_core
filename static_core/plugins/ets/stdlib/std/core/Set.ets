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
 * @file Defines the Set for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Represents a read-only Set collection.
 * @extends Iterable<T>
 * @interface ReadonlySet<T>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export interface ReadonlySet<T> extends Iterable<T> {
    /**
     * Checks if a value is in the Set.
     *
     * @param { T } value - The value to check.
     * @returns { boolean } - Return true if the value exists, otherwise return false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    has(value: T): boolean;

    /**
     * Get the number of unique elements in the Set.
     *
     * @returns { int } - The number of unique elements in the Set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get size(): int;

    /**
     * Executes the provided callback function once for each element in the Set in insertion order.
     *
     * @param { function } callbackfn - The callback function to execute for each element.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    forEach(callbackfn: (value: T, value2: T, set: ReadonlySet<T>) => void): void;

    /**
     * Despite name, returns elements from the Set.
     *
     * @returns { IterableIterator<T> } - an iterable of the values in the set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    keys(): IterableIterator<T>;

    /**
     * Returns elements from the Set.
     *
     * @returns {IterableIterator<T> } - an iterable of the values in the set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    values(): IterableIterator<T>;

    /**
     * Returns a new iterator object that contains all elements in the Set as [value, value] arrays.
     *
     * @returns {IterableIterator<[T, T]> } - an iterable of [v,v] pairs for every value `v` in the set..
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    entries(): IterableIterator<[T, T]>;
}

/**
 * Iterator interface for Set.
 * @extends IterableIterator<T>
 * @interface SetIterator<T>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export interface SetIterator<T> extends IterableIterator<T> {}

/**
 * Internal implementation of Set iterator.
 * @implements SetIterator<T>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
final class SetIteratorImpl<T> implements SetIterator<T> {
    /**
     * The underlying iterator.
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private it: IterableIterator<T>
    /**
     * Creates a new SetIteratorImpl instance.
     *
     * @param { IterableIterator<T> } it - The underlying iterator to wrap.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(it: IterableIterator<T>) {
        super()
        this.it = it
    }

    /**
     * Returns the next element in the iteration.
     *
     * @returns { IteratorResult<T> } - The next element in the iteration.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override next(): IteratorResult<T> {
        return this.it.next()
    }

    /**
     * Returns the iterator itself.
     *
     * @returns { IterableIterator<T> } - The iterator itself.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override $_iterator(): IterableIterator<T> {
        return this.it
    }
}

/**
 * Internal final Map implementation used by Set for storing elements.
 * @extends Map<K, V>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
final class FinalMap<K, V> extends Map<K, V> {
    /**
     * Creates a new FinalMap instance with the specified initial capacity.
     *
     * @param { int } initialCapacity - The initial capacity of the map.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(initialCapacity: int) {
        super(initialCapacity)
    }

    /**
     * Creates a new FinalMap instance from another FinalMap.
     *
     * @param { FinalMap<K, V> } map - Another FinalMap instance used for initialization.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(map: FinalMap<K, V>) {
        super(map)
    }

    /**
     * Creates a new FinalMap instance from an iterable or an array of entries.
     *
     * @param { Iterable<[K, V]> | readonly ((readonly [K, V]) | null | undefined)[] | null } [entries] - 
     *    The iterable or array of entries used for initialization.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(entries?: Iterable<[K, V]> | readonly ((readonly [K, V]) | null | undefined)[] | null) {
        super(entries)
    }
}

/**
 * Set implementation.
 * @implements ReadonlySet<K>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class Set<K> implements ReadonlySet<K> {
    /**
     * The underlying map that stores the Set elements as key-value pairs.
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private readonly elements: FinalMap<K, K>

    /**
     * Creates a new Set instance with the specified number of buckets.
     * NOTE: This API may be removed someday due #28030 internal issue
     *
     * @param { int } bucketsCount - The number of buckets for the internal map.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(bucketsCount: int) {
        this.elements = new FinalMap<K, K>(bucketsCount);
    }

    /**
     * Creates a new Set instance from another Set.
     *
     * @param { Set<K> } set - Another Set instance used for initialization.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(set: Set<K>) {
        this.elements = new FinalMap<K, K>(set.elements)
    }

    // NOTE(templin.konstantin): Temporary workaround due #20944 internal issue
    //  if we remove this constructor we got error on "readonly K[]" branch
    //  at constructor from union type
     /**
     * Creates a new Set instance from an array.
     *
     * @param { K[] } values - The array used for initialization.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(values: K[]) {
        this.elements = new FinalMap<K, K>()
        for (let i = 0; i < values.length; ++i) {
            this.elements.set(values[i], values[i])
        }
    }

    /**
     * Creates a new Set instance from an iterable object or FixedArray.
     *
     * @param { Iterable<K> | FixedArray<K> | null } [elements] - The iterable object, FixedArray,
     *     or null used for initialization.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(elements?: Iterable<K> | FixedArray<K> | null) {
        if (elements == null) {
            this.elements = new FinalMap<K, K>();
        } else if (elements instanceof Iterable) {
            const it = (elements as Iterable<K>).$_iterator()
            const entriesIter = new MappingIterator<K, [K, K]>(it, (element: K): [K, K] => [element, element])
            this.elements = new FinalMap<K, K>(entriesIter)
        } else {
            const entries = new ([K, K] | null | undefined)[(elements as FixedArray<K>).length]
            for (let i = 0; i < elements.length; ++i) {
                entries[i] = [elements[i], elements[i]] as [K, K]
            }
            this.elements = new FinalMap<K, K>(entries)
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

    /**
     * Returns a string representation of the Set.
     *
     * @returns { String } - A string representation of the Set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override toString(): String {
        if(this.elements.size == 0) {
            return "[object Set]"
        }
        return "Set[" + this.toStringVals() + "]"
    }

    /**
     * Adds an element to the end of the Set object. Returns the Set object.
     *
     * @param { K } val - The value to add to the Set.
     * @returns { this } - Returns the Set instance itself.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    add(val: K): this {
        this.elements.set(val, val)
        return this
    }

    /**
     * Determines whether a value exists in the Set.
     *
     * @param { K } val - The value to check.
     * @returns { boolean } - Return true if the value exists, otherwise return false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override has(val: K): boolean {
        return this.elements.has(val)
    }

    /**
     * Get the number of unique elements in the Set.
     *
     * @returns { int } - The number of unique elements in the Set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override get size(): int {
        return this.elements.size;
    }

    /**
     * Removes a value from the Set
     *
     * @param { K } val - The value to remove.
     * @returns { boolean } - Return true if the element exists and was successfully removed, otherwise return false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    delete(val: K): boolean {
        return this.elements.delete(val)
    }

    /**
     * Removes all elements from the Set object.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    clear(): void {
        this.elements.clear()
    }

    /**
     * Despite name, returns elements from the Set.
     *
     * @returns {IterableIterator<K> } - an iterable of the values in the set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override keys(): IterableIterator<K> {
        return new SetIteratorImpl<K>(this.elements.keys())
    }

    /**
     * Returns elements from the Set.
     *
     * @returns {IterableIterator<K> } - an iterable of the values in the set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override values(): IterableIterator<K> {
        return new SetIteratorImpl<K>(this.elements.values())
    }

    /**
     * Returns the default iterator of the Set, which is the values() iterator.
     *
     * @returns {IterableIterator<K> } - The default iterator of the Set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override $_iterator(): IterableIterator<K> {
        return new SetIteratorImpl<K>(this.values())
    }

    /**
     * Returns a new iterator object that contains all elements in the Set as [value, value] arrays.
     *
     * @returns {IterableIterator<[K, K]> } - an iterable of [v,v] pairs for every value `v` in the set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override entries(): IterableIterator<[K, K]> {
        return new SetIteratorImpl<[K, K]>(this.elements.entries())
    }

    /**
     * Executes the provided callback function once for each element in the Set in insertion order.
     *
     * @param { function } callbackfn - The callback function to execute for each element.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override forEach(callbackfn: (k: K, v: K, set: Set<K>) => void): void {
        let it = this.keys();
        while (true) {
            const v = it.next()
            if (v.done) {
                return
            }
            callbackfn(v.value as K, v.value as K, this)
        }
    }
}

/**
 * Internal iterator that maps elements from an input iterator to output elements using a mapping function.
 * @implements IterableIterator<O>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
class MappingIterator<I, O> implements IterableIterator<O> {
    /**
     * The input iterator to read elements from.
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private readonly inputIter: Iterator<I>
    /**
     * The mapping function to transform input elements to output elements.
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private readonly mappingFn: (input: I) => O

    /**
     * Creates a new MappingIterator instance.
     *
     * @param { Iterator<I> } inputIter - The input iterator to read elements from.
     * @param { function } mappingFn - The mapping function to transform input elements to output elements.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(inputIter: Iterator<I>, mappingFn: (input: I) => O) {
        this.inputIter = inputIter
        this.mappingFn = mappingFn
    }

    /**
     * Returns the next mapped element in the iteration.
     *
     * @returns { IteratorResult<O> } - The next mapped element in the iteration.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
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
