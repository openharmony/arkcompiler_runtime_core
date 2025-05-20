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
    get size(): number;

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

final class MapEntry<K, V> {
    prev: MapEntry<K, V> = this
    next: MapEntry<K, V> = this

    key: K
    val: V

    constructor(key: K, val: V) {
        this.key = key
        this.val = val
    }

    toString(): string {
        return `{${this.key},${this.val}}`
    }
}

final class MapIterator<K, V, R> implements IterableIterator<R> {
    private node: MapEntry<K, V>
    private tail: MapEntry<K, V>

    private mapper: (e: MapEntry<K, V>) => R

    constructor(node: MapEntry<K, V>, tail: MapEntry<K, V>, mapper: (e: MapEntry<K, V>) => R) {
        this.node = node
        this.tail = tail
        this.mapper = mapper
    }

    override next(): IteratorResult<R> {
        const nextNode = this.node.next
        if (__runtimeIsSameReference(nextNode, this.tail)) {
            return new IteratorResult<R>()
        }
        this.node = nextNode
        return new IteratorResult<R>(this.mapper(this.node))
    }

    override $_iterator(): IterableIterator<R> {
        return this
    }
}

final class EmptyMapIterator<R> implements IterableIterator<R> {
    override next(): IteratorResult<R> {
        return new IteratorResult<R>()
    }

    override $_iterator(): IterableIterator<R> {
        return this
    }
}

type MapBucket<K, V> = Array<MapEntry<K, V>> | undefined

export class Map<K, V> implements ReadonlyMap<K, V> {
    private headEntry?: MapEntry<K, V>

    private buckets: MapBucket<K, V>[]
    private sizeVal: int = 0

    private static readonly ENTRY_KEY = 0
    private static readonly ENTRY_VAL = 1

    private static readonly MIN_BUCKETS_COUNT = 8

    private getBucketIndex(k: NullishType): int {
        if (__runtimeIsSameReference(k, null)) {
            return 0;
        }
        if (__runtimeIsSameReference(k, undefined)) {
            return 1;
        }

        const keyHash: int = (k! as object).$_hashCode() // #26217
        const t = keyHash >> 31

        return ((keyHash ^ t) - t) & (this.buckets.actualLength - 1)
    }

    private getInitialBucketsCount(entriesCount: int): int {
        const bucketsCount = Math.ceilPow2(entriesCount * 4 / 3 + 1)

        return (bucketsCount >= Map.MIN_BUCKETS_COUNT) ? bucketsCount : Map.MIN_BUCKETS_COUNT
    }

    /**
     * Constructs a Map from collection
     * @param entries initial collection
     */
    constructor(entries?: Iterable<[K, V]> | readonly ((readonly [K, V]) | null | undefined)[] | null) {
        if (entries == null) {
            this.buckets = new MapBucket<K, V>[Map.MIN_BUCKETS_COUNT]
        } else if (entries instanceof Iterable) {
            const entriesIter = (entries as Iterable<[K, V]>).$_iterator()
            const entriesBuf = new Array<[K, V]>()
            for (let iterRes = entriesIter.next();!iterRes.done;iterRes = entriesIter.next()) {
                const entry = iterRes.value
                if (entry != null) {
                    entriesBuf.push(entry)
                }
            }
            const bucketsCount = this.getInitialBucketsCount(entriesBuf.actualLength)
            this.buckets = new MapBucket<K, V>[bucketsCount]

            entriesBuf.forEach((e: [K, V]) => {
                this.set(e[0] as K, e[1] as V)
            })
        } else {
            const bucketsCount = this.getInitialBucketsCount(entries.actualLength)
            this.buckets = new MapBucket<K, V>[bucketsCount]
            for (let i = 0; i < entries.length; ++i) {
                if (entries[i] == null) {
                    continue
                }
                this.set(entries[i]![0] as K, entries[i]![1] as V)
            }
        }
    }

    /**
     * Constructs a Map from another Map
     * @param map another map
     */
    constructor(map: Map<K, V>) {
        const bucketsCount = this.getInitialBucketsCount(map.sizeVal)
        this.buckets = new MapBucket<K, V>[bucketsCount]

        map.forEach((v: V, k: K) => {
            this.set(k, v)
        })
    }

    /**
     * Constructs a Map from Array of entries
     * @param entries Array of entries
     */
    constructor(entries: Array<[K, V]>) {
        const bucketsCount = this.getInitialBucketsCount(entries.actualLength)
        this.buckets = new MapBucket<K, V>[bucketsCount]

        entries.forEach((entry: [K, V]) => {
            this.set(entry[0] as K, entry[1] as V)
        })
    }

    // NOTE(templin.konstantin): Temporary workaround due #20944 internal issue
    //  if we remove this constructor we got error on "readonly (readonly [K,V])[]"
    //  branch at constructor from union type
    constructor(values: FixedArray<[K, V]>) {
        const bucketsCount = this.getInitialBucketsCount(values.length)
        this.buckets = new MapBucket<K, V>[bucketsCount]
        for (let i = 0; i < values.length; ++i) {
            if (values[i] == null) {
                continue
            }
            this.set(values[i][0] as K, values[i][1] as V)
        }
    }

    override toString(): String {
        const strBuf = new StringBuilder()

        const entriesIter = this.entries()
        let entriesIterRes = entriesIter.next()
        while (!entriesIterRes.done) {
            const entry = entriesIterRes.value
            if (!__runtimeIsSameReference(entry, undefined) && !__runtimeIsSameReference(entry, null)) {
                strBuf.append(entry!)
            }

            entriesIterRes = entriesIter.next()
            if (!entriesIterRes.done) {
                strBuf.append(",")
            }
        }

        return strBuf.toString()
    }

    private getBucket(k: K): MapBucket<K, V> {
        return this.buckets[this.getBucketIndex(k)]
    }

    private getOrCreateBucket(k: K): Array<MapEntry<K, V>> {
        const idx = this.getBucketIndex(k)
        const bucket = this.buckets[idx]
        if (bucket === undefined) {
            const newBucket = new Array<MapEntry<K, V>>()
            this.buckets[idx] = newBucket
            return newBucket
        } else {
            return bucket!
        }
    }

    private rehash(): void {
        const newSize = this.buckets.length * 2
        this.buckets = new MapBucket<K, V>[newSize]

        const entriesIter = this.mappedIterator<MapEntry<K, V>>((e: MapEntry<K, V>): MapEntry<K, V> => e)
        iteratorForEach<MapEntry<K, V>>(entriesIter, (e: MapEntry<K, V>): void => {
            this.getOrCreateBucket(e.key).push(e)
        })
    }


    /**
     * Puts a pair of key and value into the Map
     *
     * @param key the key to put into the Map
     *
     * @param val the value to put into the Map
     */
    set(key: K, val: V): this {
        // insert into buckets
        const buck = this.getOrCreateBucket(key)
        const buckSize = buck.actualLength
        for (let i = 0; i < buckSize; i++) {
            const entry = buck.$_get_unsafe(i)
            if (__runtimeSameValueZero(entry.key, key)) {
                entry.val = val
                return this
            }
        }

        // not found, add to insertion-ordered list
        const newEntry = new MapEntry<K, V>(key, val)

        if (this.headEntry === undefined) {
            this.headEntry = new MapEntry<K, V>(key, val)
        }

        const headEntry = this.headEntry!
        newEntry.next = headEntry
        newEntry.prev = headEntry.prev
        headEntry.prev.next = newEntry
        headEntry.prev = newEntry

        buck.push(newEntry)

        this.sizeVal += 1

        // constant load factor (entries / buckets < 0.75)
        if (this.sizeVal * 4 > this.buckets.length * 3) {
            this.rehash();
        }

        return this;
    }

    /**
     * Checks if a key is in the Map
     *
     * @param key the key to find in the Map
     *
     * @returns true if the value is in the Map
     */
    override has(key: K): boolean {
        const buck = this.getBucket(key)
        if (buck !== undefined) {
            const buckSize = buck.actualLength
            for (let i = 0; i < buckSize; i++) {
                const entry = buck.$_get_unsafe(i)
                if (__runtimeSameValueZero(entry.key, key)) {
                    return true
                }
            }
        }

        return false;
    }

    /**
     * Returns number of Entries with unique keys in the Map
     *
     * @returns number of Entries with unique keys in the Map
     */
    override get size(): number {
        return this.sizeVal
    }

    /**
     * Removes an Entry with specified key from the Map
     *
     * @param key the key to remove
     */
    delete(key: K): boolean {
        if (!this.deleteImpl(key)) {
            return false
        }

        this.sizeVal--
        if (this.sizeVal == 0) {
            this.headEntry = undefined
        }
        return true
    }

    private deleteImpl(key: K): boolean {
        const buck = this.getBucket(key)
        if (buck === undefined) {
            return false
        }

        const buckSize = buck.actualLength
        for (let i = 0; i < buckSize; i++) {
            const entry = buck.$_get_unsafe(i)
            if (!__runtimeSameValueZero(entry.key, key)) {
                continue
            }

            entry.prev.next = entry.next
            entry.next.prev = entry.prev

            const popped = buck.pop()!
            if (i != buckSize - 1) {
                buck[i] = popped
            }
            return true
        }

        return false
    }

    /**
     * Deletes all elements from the Map
     */
    clear(): void {
        this.sizeVal = 0

        if (this.headEntry !== undefined) {
            this.headEntry = undefined
            this.buckets = new MapBucket<K, V>[Map.MIN_BUCKETS_COUNT]
        }
    }

    /**
     * Returns a value assosiated with key if present
     *
     * @param key the key to find in the Map
     *
     * @returns value if assosiated with key presents.
     */
    override get(key: K): V | undefined {
        const buck = this.getBucket(key)
        if (buck !== undefined) {
            const buckSize = buck.actualLength
            for (let i = 0; i < buckSize; i++) {
                const entry = buck.$_get_unsafe(i)
                if (__runtimeSameValueZero(entry.key, key)) {
                    return entry.val
                }
            }
        }

        return undefined;
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
        const val = this.get(key)
        if (val !== undefined) {
            return val
        }
        return def
    }

    internal mappedIterator<R>(fn: (e: MapEntry<K, V>) => R): IterableIterator<R> {
        const headEntry = this.headEntry
        if (headEntry !== undefined) {
            return new MapIterator<K, V, R>(headEntry, headEntry, fn)
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
        return this.mappedIterator<K>((e: MapEntry<K, V>): K => e.key)
    }

    /**
     * Returns elements from the Map as an values Iterator
     *
     * @returns iterator with map values
     */
    override values(): IterableIterator<V> {
        return this.mappedIterator<V>((e: MapEntry<K, V>): V => e.val )
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
        return this.mappedIterator<[K, V]>((e: MapEntry<K, V>): [K, V] => [e.key, e.val])
    }

    /**
     * Executes a provided function once per each key/value pair in the Map, in insertion order
     *
     * @param callbackfn to apply
     */
    override forEach(callbackfn: (v: V, k: K, map: Map<K, V>) => void): void {
        const entriesIter = this.mappedIterator<MapEntry<K, V>>((e: MapEntry<K, V>): MapEntry<K, V> => e)
        iteratorForEach<MapEntry<K, V>>(entriesIter, (e: MapEntry<K, V>): void => {
            callbackfn(e.val, e.key, this)
        })
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
