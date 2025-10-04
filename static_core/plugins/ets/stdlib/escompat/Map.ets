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

export interface ReadonlyMap<K, V> extends Iterable<[K, V]> {
    /**
     * Returns number of Entries with unique keys in the Map
     *
     * @returns number of Entries with unique keys in the Map
     */
    get size(): int;

    /**
     * Returns a value assosiated with key if present
     *
     * @param k the key to find in the Map/class
     *
     * @returns value if assosiated with key presents.
     */
    get(key: K): V | undefined;

    /**
     * Checks if a key is in the Map
     *
     * @param k the key to find in the Map
     *
     * @returns true if the value is in the Map
     */
    has(key: K): boolean;

    /**
     * Executes a provided function once per each key/value pair in the Map, in insertion order
     *
     * @param callbackfn to apply
     */
    forEach(callbackfn: (value: V, key: K, map: ReadonlyMap<K, V>) => void): void;

    /**
     * Returns elements from the Map as an keys Iterator
     *
     * @returns ValueIterator with map keys
     */
    keys(): IterableIterator<K>;

    /**
     * Returns elements from the Map as an values Iterator
     *
     * @returns ValueIterator with map values
     */
    values(): IterableIterator<V>;

    /**
     * Returns elements from the Map as an array of Entries
     *
     * @returns an array of Entries
     */
    entries(): IterableIterator<[K, V]>;
}

const EMPTY_ENTRY: int = 0  // Not filled yet - can be used as the margin of empty tail.
const DELETED_ENTRY: int = -2
const BUCKET_END: int = -1

const CLEAR_FLAG_IDX: int = 0  // If table has been cleared, there are no numbers of deleted elements - check this flag.

function getKeyAt<K>(data: FixedArray<Any>, at: int): K {
    return data[at * 2 - 2] as K
}

function getValueAt<V>(data: FixedArray<Any>, at: int): V {
    return data[at * 2 - 1] as V
}

function isDeletedAt(buckets: FixedArray<int>, at: int): boolean {
    return buckets[at * 2 - 1] === DELETED_ENTRY
}

function isEmptyAt(buckets: FixedArray<int>, at: int): boolean {
    return buckets[at * 2 - 1] === EMPTY_ENTRY
}

function getNumOfDeleted(buckets: FixedArray<int>, at: int) { // Call only when clear or rehash occured.
    if (buckets[CLEAR_FLAG_IDX]) {
        return at
    }
    return at > 0 ? buckets[at * 2 - 1] : 0
}

final class MapEntry<K, V> {
    key: K
    val: V
    next: int = -1

    constructor() {}

    constructor(key: K, val: V, next: int) {
        this.key = key
        this.val = val
        this.next = next
    }

    toString(): string {
        return `{${this.key},${this.val}}`
    }
}

class SearchTuple {
    first: int
    second: int

    constructor(first: int, second: int) {
        this.first = first
        this.second = second
    }
}

export final class MapIterator<K, V, R> implements IterableIterator<R> {
    private data: FixedArray<Any> | undefined
    private buckets: FixedArray<int> | undefined
    private curIndex: int

    private mapper: (k: K, v: V) => R

    constructor(data: FixedArray<Any>, mapper: (k: K, v: V) => R) {
        this.data = data
        this.buckets = data[data.length - 2] as FixedArray<int>
        this.curIndex = 1
        this.mapper = mapper
    }

    override next(): IteratorResult<R> {
        let data = this.data
        let buckets = this.buckets
        while (data !== undefined && data[data.length - 1] !== undefined) {
            if (this.curIndex > 1) {
                this.curIndex -= getNumOfDeleted(buckets!, this.curIndex - 1)
            }            
            data = data[data.length - 1] as (FixedArray<Any> | undefined)
            if (data !== undefined) {
                buckets = data[data.length - 2] as FixedArray<int>
            }
        }
        this.data = data
        if (data === undefined) {
            return new IteratorResult<R>()
        }
        this.buckets = buckets
        
        for (; this.curIndex <= this.buckets!.length / 2; this.curIndex++) {
            if (isEmptyAt(this.buckets!, this.curIndex)) {
                break
            }
            if (!isDeletedAt(this.buckets!, this.curIndex)) {
                const res = new IteratorResult<R>(this.mapper(getKeyAt<K>(this.data!, this.curIndex), getValueAt<V>(this.data!, this.curIndex)))
                this.curIndex++
                return res
            }
        }
        this.data = undefined
        return new IteratorResult<R>()
    }

    override $_iterator(): IterableIterator<R> {
        return this
    }
}

export final class EmptyMapIterator<R> implements IterableIterator<R> {
    override next(): IteratorResult<R> {
        return new IteratorResult<R>()
    }

    override $_iterator(): IterableIterator<R> {
        return this
    }
}

export class Map<K, V> implements ReadonlyMap<K, V> {
    private data: FixedArray<Any>
    private buckets: FixedArray<int>
    private cap: int
    private numEntries: int = 0
    private sizeVal: int = 0

    private static readonly MIN_CAPACITY = 4
    private static readonly MIN_SHRINK_CAPACITY = 16
    private static readonly ENTRY_KEY = 0
    private static readonly ENTRY_VAL = 1

    private static computeCapacity(numEntries: int): int {
        const capacity = Math.ceilPow2(numEntries + numEntries / 2)
        return (capacity >= Map.MIN_CAPACITY) ? capacity : Map.MIN_CAPACITY
    }

    private static computeCapacityForShrink(capacity: int, size: int): int {
        if (size >= capacity / Map.MIN_CAPACITY) {
            return 0
        }

        // quarter filled, shrink
        const newCapacity = capacity / 2
        return (newCapacity >= Map.MIN_SHRINK_CAPACITY) ? newCapacity : 0 
    }

    private static computeCapacityForGrow(capacity: int, size: int, numEntries: int): int {
        const deleted = numEntries - size
        if (deleted >= capacity / 2) {
            // just clear out deleted elements
            return capacity
        }

        // grow
        return capacity * 2
    }

    private static getNextInBucketAt(buckets: FixedArray<int>, at: int): int {
        return buckets[at * 2 - 1]
    }

    private static getBucket(buckets: FixedArray<int>, hash: int): int {
        return buckets[hash * 2]
    }

    private static setKey<K>(data: FixedArray<Any>, at: int, v: K) {
        data[at * 2] = v
    }

    private static setValue<V>(data: FixedArray<Any>, at: int, v: V) {
        data[at * 2 + 1] = v
    }

    private static setNextInBucketAt(buckets: FixedArray<int>, at: int, next: int) {
        buckets[at * 2 - 1] = next
    }

    private static setBucket(buckets: FixedArray<int>, at: int, next: int) {
        buckets[at * 2] = next
    }

    private static deleteAt(buckets: FixedArray<int>, at: int) {
        // NOTE(nkholiavin): maybe need to set deleted keys, values to undefined
        Map.setNextInBucketAt(buckets, at, DELETED_ENTRY)
    }

    private getBucketIndex(k: Any): int {
        if (k === null) {
            return 0
        }
        if (k === undefined) {
            return 1
        }

        const keyHash: int = Runtime.getHashCodeByValue(k! as object).toInt() // #26217
        const t = keyHash >> 31

        return ((keyHash ^ t) - t) & (this.cap - 1)
    }

    /**
     * Constructs a Map from collection
     * @param entries initial collection
     */
    constructor(entries?: Iterable<[K, V]> | readonly ((readonly [K, V]) | null | undefined)[] | null) {
        if (entries == null) {
            this.data = new Any[Map.MIN_CAPACITY * 2 + 2]
            this.buckets = new int[Map.MIN_CAPACITY * 2]
            this.data[this.data.length - 2] = this.buckets
            this.cap = Map.MIN_CAPACITY
        } else if (entries instanceof Array) {
            const array = entries as Array<(readonly [K, V]) | null | undefined>
            const capacity = Map.computeCapacity(array.length.toInt())
            this.data = new Any[capacity * 2 + 2]
            this.buckets = new int[capacity * 2]
            this.data[this.data.length - 2] = this.buckets
            this.cap = capacity
            for (let i = 0; i < array.length; ++i) {
                let pair = array[i]
                if (pair == undefined || pair == null) {
                    continue
                }
                this.set(pair[Map.ENTRY_KEY] as K, pair[Map.ENTRY_VAL] as V)
            }
        } else {
            const entriesIter = (entries as Iterable<[K, V]>).$_iterator()
            const entriesBuf = new Array<[K, V]>()
            for (let iterRes = entriesIter.next(); !iterRes.done; iterRes = entriesIter.next()) {
                const entry = iterRes.value
                if (entry != null) {
                    entriesBuf.push(entry)
                }
            }
            const capacity = Map.computeCapacity(entriesBuf.length)
            this.data = new Any[capacity * 2 + 2]
            this.buckets = new int[capacity * 2]
            this.data[this.data.length - 2] = this.buckets
            this.cap = capacity

            entriesBuf.forEach((e: [K, V]) => {
                this.set(e[Map.ENTRY_KEY] as K, e[Map.ENTRY_VAL] as V)
            })
        }
    }

    /**
     * Constructs a Map from another Map
     * @param map another map
     */
    constructor(map: Map<K, V>) {
        const capacity = Map.computeCapacity(map.sizeVal)
        this.data = new Any[capacity * 2 + 2]
        this.buckets = new int[capacity * 2]
        this.data[this.data.length - 2] = this.buckets
        this.cap = capacity

        map.forEach((v: V, k: K) => {
            this.set(k, v)
        })
    }

    /**
     * Constructs a Map from Array of entries
     * @param entries Array of entries
     */
    constructor(entries: Array<[K, V]>) {
        const capacity = Map.computeCapacity(entries.length)
        this.data = new Any[capacity * 2 + 2]
        this.buckets = new int[capacity * 2]
        this.data[this.data.length - 2] = this.buckets
        this.cap = capacity

        entries.forEach((entry: [K, V]) => {
            this.set(entry[Map.ENTRY_KEY] as K, entry[Map.ENTRY_VAL] as V)
        })
    }

    // NOTE(templin.konstantin): Temporary workaround due #20944 internal issue
    //  if we remove this constructor we got error on "readonly (readonly [K,V])[]"
    //  branch at constructor from union type
    constructor(values: FixedArray<[K, V]>) {
        const capacity = Map.computeCapacity(values.length)
        this.data = new Any[capacity * 2 + 2]
        this.buckets = new int[capacity * 2]
        this.data[this.data.length - 2] = this.buckets
        this.cap = capacity
        for (let i = 0; i < values.length; ++i) {
            let pair = values[i]
            if (pair == null) {
                continue
            }
            this.set(pair[Map.ENTRY_KEY] as K, pair[Map.ENTRY_VAL] as V)
        }
    }

    // NOTE: This API may be removed someday due #28030 internal issue
    constructor(initialCapacity: int) {
        this.data = new Any[initialCapacity * 2 + 2]
        this.buckets = new int[initialCapacity * 2]
        this.data[this.data.length - 2] = this.buckets
        this.cap = initialCapacity
    }

    override toString(): String {
        const strBuf = new StringBuilder()

        const entriesIter = this.entries()
        let entriesIterRes = entriesIter.next()
        while (!entriesIterRes.done) {
            const entry = entriesIterRes.value
            if (entry != null) {
                strBuf.append(entry!)
            }

            entriesIterRes = entriesIter.next()
            if (!entriesIterRes.done) {
                strBuf.append(",")
            }
        }

        return strBuf.toString()
    }

    private insertEntry(key: K, val: V, bucketIdx: int, bucket: int) {
        this.sizeVal++
        const idx = this.numEntries++
        Map.setKey(this.data, idx, key)
        Map.setValue(this.data, idx, val)
        this.buckets[idx * 2 + 1] = bucket
        this.buckets[bucketIdx * 2] = idx + 1
    }

    private rehash(newCapacity: int) {
        let curDeletedElements: int = 0

        const newData: FixedArray<Any> = new Any[newCapacity * 2 + 2]
        const newBuckets: FixedArray<int> = new int[newCapacity * 2]
        newData[newData.length - 2] = newBuckets

        const oldNumEntries = this.numEntries
        const oldData = this.data
        const oldBuckets = this.buckets

        this.sizeVal = 0
        let sizeVal: int = 0

        for (let i: int = 0; i < oldNumEntries; i++) {
            if (oldBuckets[i * 2 + 1] === DELETED_ENTRY) {
                oldBuckets[i * 2 + 1] = ++curDeletedElements
                continue
            }
            oldBuckets[i * 2 + 1] = curDeletedElements

            const key = oldData[i * 2]
            const val = oldData[i * 2 + 1]
            let bucketIdx = 0
            if (key !== null) {
                if (key === undefined) {
                    bucketIdx = 1
                } else {
                    const keyHash: int = Runtime.getHashCodeByValue(key as object).toInt()
                    const t = keyHash >> 31
                    bucketIdx = ((keyHash ^ t) - t) & (newCapacity - 1)
                }
            }

            const bucket = newBuckets[bucketIdx * 2]
            const idx = sizeVal++
            Map.setKey(newData, idx, key)
            Map.setValue(newData, idx, val)
            newBuckets[idx * 2 + 1] = bucket > 0 ? bucket : BUCKET_END
            newBuckets[bucketIdx * 2] = idx + 1
        }
        oldBuckets[CLEAR_FLAG_IDX] = 0  // means false

        this.numEntries = this.sizeVal = sizeVal
        this.data[this.data.length - 1] = newData
        this.data = newData
        this.buckets = newBuckets
        this.cap = newCapacity
    }

    private searchBucket(bucket: int, key: K): SearchTuple {
        let prevEntryIdx: int = 0
        let entryIdx: int = bucket
        while (entryIdx > 0) {
            if (Runtime.sameValueZero(getKeyAt<K>(this.data, entryIdx), key)) {
                return new SearchTuple(entryIdx, prevEntryIdx)
            }
            prevEntryIdx = entryIdx
            entryIdx = Map.getNextInBucketAt(this.buckets, entryIdx)
        }
        return new SearchTuple(entryIdx, prevEntryIdx)
    }

    private searchBucketIndex(bucket: int, key: K): int {
        let entryIdx: int = bucket
        while (entryIdx > 0) {
            if (Runtime.sameValueZero(getKeyAt<K>(this.data, entryIdx), key)) {
                break
            }
            entryIdx = Map.getNextInBucketAt(this.buckets, entryIdx)
        }
        return entryIdx
    }

    /**
     * Puts a pair of key and value into the Map
     *
     * @param key the key to put into the Map
     *
     * @param val the value to put into the Map
     */
    set(key: K, val: V): this {
        // rehash
        if (this.numEntries * 4 >= this.cap * 3) {
            this.rehash(Map.computeCapacityForGrow(this.cap, this.sizeVal, this.numEntries))
        }

        // check bucket
        const capacity = this.cap
        let bucketIdx = 0
        if (key !== null) {
            if (key === undefined) {
                bucketIdx = 1
            } else {
                const keyHash: int = Runtime.getHashCodeByValue(key as object).toInt()
                const t = keyHash >> 31
                bucketIdx = ((keyHash ^ t) - t) & (capacity - 1)
            }
        }
        const buckets = this.buckets
        const data = this.data
        let bucket = buckets[bucketIdx * 2]
        if (bucket > 0) {
            let foundIdx: int = bucket - 1
            do {
                if (Runtime.sameValueZero(data[foundIdx * 2], key)) {
                    // found
                    Map.setValue(data, foundIdx, val)
                    return this
                }
                foundIdx = buckets[foundIdx * 2 + 1] - 1
            } while (foundIdx >= 0);
        } else {
            bucket = BUCKET_END
        }

        // insert
        const idx = this.numEntries++
        Map.setKey(data, idx, key)
        Map.setValue(data, idx, val)
        buckets[idx * 2 + 1] = bucket
        buckets[bucketIdx * 2] = idx + 1
        this.sizeVal++
        return this
    }

    /**
     * Checks if a key is in the Map
     *
     * @param key the key to find in the Map
     *
     * @returns true if the value is in the Map
     */
    override has(key: K): boolean {
        const bucketIdx = this.getBucketIndex(key)
        const bucket = Map.getBucket(this.buckets, bucketIdx)
        return this.searchBucketIndex(bucket, key) > 0
    }

    /**
     * Returns number of Entries with unique keys in the Map
     *
     * @returns number of Entries with unique keys in the Map
     */
    override get size(): int {
        return this.sizeVal
    }

    private shrink() {
        const newCapacity = Map.computeCapacityForShrink(this.cap, this.sizeVal)
        if (newCapacity !== 0) {
            this.rehash(newCapacity)
        }
    }

    /**
     * Removes an Entry with specified key from the Map
     *
     * @param key the key to remove
     */
    delete(key: K): boolean {
        const bucketIdx = this.getBucketIndex(key)
        const bucket = Map.getBucket(this.buckets, bucketIdx)
        const found = this.searchBucket(bucket, key)
        const foundIdx = found.first
        if (foundIdx <= 0) {
            return false
        }

        const prevIdx = found.second
        if (prevIdx > 0) {
            Map.setNextInBucketAt(this.buckets, prevIdx, Map.getNextInBucketAt(this.buckets, foundIdx))
        } else {
            Map.setBucket(this.buckets, bucketIdx, Map.getNextInBucketAt(this.buckets, foundIdx))
        }
        Map.deleteAt(this.buckets, foundIdx)
        this.sizeVal--
        this.shrink()
        return true
    }

    /**
     * Deletes all elements from the Map
     */
    clear(): void {
        if (this.sizeVal === 0 && this.cap === Map.MIN_CAPACITY) {
            return
        }

        const oldData = this.data
        this.data = new Any[Map.MIN_CAPACITY * 2 + 2]
        oldData[oldData.length - 1] = this.data
        this.buckets[CLEAR_FLAG_IDX] = 1  // means true
        this.buckets = new int[Map.MIN_CAPACITY * 2]
        this.data[this.data.length - 2] = this.buckets
        this.cap = Map.MIN_CAPACITY
        this.sizeVal = 0
        this.numEntries = 0
    }

    /**
     * Returns a value assosiated with key if present
     *
     * @param key the key to find in the Map
     *
     * @returns value if assosiated with key presents.
     */
    override get(key: K): V | undefined {
        const bucketIdx = this.getBucketIndex(key)
        const bucket = Map.getBucket(this.buckets, bucketIdx)
        const foundIdx = this.searchBucketIndex(bucket, key)
        if (foundIdx > 0) {
            return getValueAt<V>(this.data, foundIdx)
        }
        return undefined
    }

    /**
     * Returns a value assosiated with key if present, or a default value otherwise
     *
     * @param key the key to find in the Map
     *
     * @param def a value to return if key is not in the Map
     *
     * @returns value if key presents, def otherwise
     */
    get(key: K, def: V): V {
        return this.get(key) ?? def
    }

    private mappedIterator<R>(fn: (k: K, v: V) => R): IterableIterator<R> {
        if (this.sizeVal > 0) {
            return new MapIterator<K, V, R>(this.data, fn)
        } else {
            return new EmptyMapIterator<R>()
        }
    }

    /**
     * Returns elements from the Map as an keys Iterator
     *
     * @returns iterator with map keys
     */
    override keys(): IterableIterator<K> {
        return this.mappedIterator<K>((k: K, v: V): K => k)
    }

    /**
     * Returns elements from the Map as an values Iterator
     *
     * @returns iterator with map values
     */
    override values(): IterableIterator<V> {
        return this.mappedIterator<V>((k: K, v: V): V => v)
    }

    override $_iterator(): IterableIterator<[K, V]> {
        return this.entries()
    }

    /**
     * Returns elements from the Map as an array of Entries
     *
     * @returns an array of Entries
     */
    override entries(): IterableIterator<[K, V]> {
        return this.mappedIterator<[K, V]>((k: K, v: V): [K, V] => [k, v])
    }

    /**
     * Executes a provided function once per each key/value pair in the Map, in insertion order
     *
     * @param callbackfn to apply
     */
    override forEach(callbackfn: (v: V, k: K, map: Map<K, V>) => void): void {
        let curData = this.data

        for (let curAt = 1; curAt <= this.numEntries; curAt++) {
            if (isDeletedAt(this.buckets, curAt))
                continue
                
            callbackfn(getValueAt<V>(this.data, curAt), getKeyAt<K>(this.data, curAt), this)

            while (curData[curData.length - 1] !== undefined) {
                let curBuckets = curData[curData.length - 2] as FixedArray<int>
                curAt -= getNumOfDeleted(curBuckets!, curAt)
                curData = curData[curData.length - 1] as FixedArray<Any>
            }
        }
    }
}

export class Record<K extends Numeric | string | BaseEnum<Int> | BaseEnum<Long> | BaseEnum<string>, V> extends Map<K, V> {
    $_get(k : K) : V | undefined {
        return super.get(k)
    }

    $_set(k: K, v: V) : void {
        super.set(k, v)
    }
}
