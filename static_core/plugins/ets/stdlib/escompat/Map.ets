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

export interface ReadonlyMap<K, V> /* TODO(kprokopenko): extends Iterable */ {
    /**
     * Returns number of Entries with unique keys in the Map
     *
     * @returns number of Entries with unique keys in the Map
     */
    size(): number; // TODO(kprokopenko): must be a property

    /**
     * Returns a value assosiated with key if present
     *
     * @param k the key to find in the Map
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
     * Applies a function over all elements of the Map
     *
     * @param fn to apply
     */
    forEach(callbackfn: (value: V) => void): void;

    /**
     * Applies a function over all elements of the Map
     *
     * @param fn to apply
     */
    forEach(callbackfn: (value: V, key: K) => void): void;

    /**
     * Applies a function over all elements of the Map
     *
     * @param fn to apply
     */
    forEach(callbackfn: (value: V, key: K, map: ReadonlyMap<K, V>) => void): void;

    // TODO(kprokopenko): uncomment when internal issue #14416 is fixed

    /**
     * Returns elements from the Map as an keys Iterator
     *
     * @returns ValueIterator with map keys
     */
    //keys(): ValuesIterator<K>;

    /**
     * Returns elements from the Map as an values Iterator
     *
     * @returns ValueIterator with map values
     */
    //values(): ValuesIterator<V>;

    /**
     * Returns elements from the Map as an array of Entries
     *
     * @returns an array of Entries
     */
    //entries(): MapIterator<K, V>;
}

class MapEntry<K, V> extends Entry<K, V> {
    constructor(key: K, value: V) {
        super(key, value)
    }

    override equals(o: NullishType): boolean {
        if (o instanceof MapEntry) {
            return __runtimeEquals(this.key as NullishType, ((o as MapEntry<K, V>).key) as NullishType)
        }
        return __runtimeEquals(this.key as NullishType, o)
    }

    override $_hashCode(): int {
        return hashOf(this.key as NullishType)
    }
}

export final class Map<K, V> implements ReadonlyMap<K, V> {
    private s: Set<MapEntry<K, V>> = new Set<MapEntry<K, V>>()

    /**
     * Constructs an empty Map
     */
    constructor() {
    }

    /**
     * Puts a pair of key and value into the Map
     *
     * @param k the key to put into the Map
     *
     * @param v the value to put into the Map
     */
    set(k: K, v: V): void {
        this.s.add(new MapEntry<K, V>(k, v))
    }

    /**
     * Checks if a key is in the Map
     *
     * @param k the key to find in the Map
     *
     * @returns true if the value is in the Map
     */
    override has(k: K): boolean {
        return !__runtimeIsSameReference(this.s.get(k as NullishType), undefined)
    }

    /**
     * Returns number of Entries with unique keys in the Map
     *
     * @returns number of Entries with unique keys in the Map
     */
    override size(): number {
        return this.s.size()
    }

    /**
     * Removes an Entry with specified key from the Map
     *
     * @param k the key to remove
     */
    delete(k: K): boolean {
        return this.s.deleteImpl(k as Object)
    }

    /**
     * Deletes all elements from the Map
     */
    clear(): void {
        this.s.clear()
    }

    /**
     * Returns a value assosiated with key if present
     *
     * @param k the key to find in the Map
     *
     * @returns value if assosiated with key presents.
     */
    override get(k: K): V | undefined {
        const r = this.s.get(k as NullishType)
        if (!__runtimeIsSameReference(r, undefined)) {
            return r!.value
        }
        return undefined
    }

    /**
     * Returns a value assosiated with key if present, or a default value otherwise
     *
     * @param k the key to find in the Map
     *
     * @param def a value to return if key is not in the Map
     *
     * @returns value if key presents, def otherwise
     */
    get(k: K, def: V): V {
        const r = this.s.get(k as NullishType)
        if (!__runtimeIsSameReference(r, undefined)) {
            return r!.value
        }
        return def
    }

    /**
     * Returns elements from the Map as an keys Iterator
     *
     * @returns iterator with map keys
     */
    /* override */ keys(): IterableIterator<K> {
        return this.s.mappedIterator<K>((x: ListNode): K => { return (x.val as MapEntry<K, V>).key as K })
    }

    /**
     * Returns elements from the Map as an values Iterator
     *
     * @returns iterator with map values
     */
    /* override */ values(): IterableIterator<V> {
        return this.s.mappedIterator<V>((x: ListNode): V => { return (x.val as MapEntry<K, V>).value })
    }

    /**
     * Returns elements from the Map as an array of Entries
     *
     * @returns an array of Entries
     */
    /* override */ entries(): IterableIterator<Entry<K, V>> {
        return this.s.mappedIterator<Entry<K, V>>((x: ListNode): Object => { return x.val as MapEntry<K, V> })
    }

    /**
     * Applies a function over all elements of the Map
     *
     * @param fn to apply
     */
    forEach(fn: (v: V, k: K, map: Map<K, V>) => void): void {
        iteratorForEach<Entry<K, V>>(this.entries(), (x: Entry<K, V>): void => {
            fn(x.value, x.key, this)
        })
    }

    /**
     * Applies a function over all elements of the Map
     *
     * @param fn to apply
     */
    override forEach(fn: (v: V, k: K) => void): void {
        iteratorForEach<Entry<K, V>>(this.entries(), (x: Entry<K, V>): void => {
            fn(x.value, x.key)
        })
    }

    /**
     * Applies a function over all elements of the Map
     *
     * @param fn to apply
     */
    override forEach(fn: (v: V) => void): void {
        iteratorForEach<V>(this.values(), fn)
    }

    /**
     * Applies a function over all elements of the Map
     *
     * @param fn to apply
     */
    override forEach(fn: (value: V, key: K, map: ReadonlyMap<K, V>) => void): void {
        iteratorForEach<Entry<K, V>>(this.entries(), (x: Entry<K, V>): void => {
            fn(x.value, x.key, this)
        })
    }
}
