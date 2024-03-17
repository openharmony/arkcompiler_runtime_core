/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
     * Applies a function over all elements of the Set
     *
     * @param fn to apply
     */
    forEach(fn: (value: T) => void): void;

    /**
     * Applies a function over all elements of the Set
     *
     * @param fn to apply; value2 is always same as value1
     */
    forEach(fn: (value: T, value2: T) => void): void;

    /**
     * Applies a function over all elements of the Set
     *
     * @param fn to apply; value2 is always same as value1
     */
    forEach(fn: (value: T, value2: T, set: ReadonlySet<T>) => void): void { throw new Error("15998") }

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

final class ListNode {
    prev: ListNode = this
    next: ListNode = this
    val: NullishType = null
}

final class SetIterator<R> implements IterableIterator<R> {
    private node: ListNode
    private tail: ListNode
    private mapper: (v: ListNode) => R

    constructor(node: ListNode, tail: ListNode, mapper: (v: ListNode) => R) {
        this.node = node
        this.tail = tail
        this.mapper = mapper
    }

    override next(): IteratorResult<R> {
        this.node = this.node.next
        if (__runtimeIsSameReference(this.node, this.tail)) {
            return new IteratorResult<R>()
        }
        const res = new IteratorResult<R>(this.mapper(this.node))
        return res
    }

    override iterator(): IterableIterator<R> {
        return this
    }
}

/**
 * Set implementation
 */
export final class Set<K> implements ReadonlySet<K> {
    private listRoot: ListNode = new ListNode()
    private buckets: Array<Array<ListNode>> = new Array<Array<ListNode>>()
    private sizeVal: int = 0

    internal static hashOf(a: NullishType): int {
        if (__runtimeEquals(a, null)) {
            return 0;
        }
        if (__runtimeEquals(a, undefined)) {
            return 1;
        }
        return a!.$_hashCode();
    }

    /**
     * Constructs a Set from collection
     * @param iter initial collection
     */
    constructor(iter?: ArrayLike<K> | Iterable<K> | null) {
        this.buckets.push(new Array<ListNode>())
        if (!__runtimeIsSameReference(iter, undefined) && !__runtimeIsSameReference(iter, null)) {
            // NOTE(kprokopenko): dealing with this overload is tricky
            // with banned `instanceof` generic, so it is delegated to array here
            const vals = Array.from<K>(iter!);
            for (let i = 1; i < vals.actualLength; i++) {
                this.buckets.push(new Array<ListNode>())
            }
            for (let i = 0; i < vals.actualLength; i++) {
                this.add(vals[i])
            }
        }
    }

    internal toStringVals(): String {
        return Array.from<K>(this.values()).toString()
    }

    override toString(): String {
        return "Set[" + this.toStringVals() + "]"
    }

    private getBucket(v: NullishType): Array<ListNode> {
        let idx = Set.hashOf(v) % this.buckets.actualLength;
        idx += this.buckets.actualLength;
        idx %= this.buckets.actualLength;
        return this.buckets[idx]
    }

    private rehash(): void {
        const iter = new SetIterator<ListNode>(this.listRoot, this.listRoot, (x: ListNode): ListNode => x)
        const oldSize = this.buckets.length
        for (let i = 0; i < oldSize; i++) {
            const a = this.buckets[i]
            a.length = 0
            this.buckets.push(new Array<ListNode>())
        }
        iteratorForEach<ListNode>(iter, (x: ListNode): void => {
            this.getBucket(x.val).push(x)
        })
    }

    /**
     * Puts a value into the Set
     *
     * @param v the value to put into the Set
     * @returns this
     */
    add(v: K): this {
        // insert into buckets
        const buck = this.getBucket(v as NullishType)
        const buckSize = buck.length
        for (let i = 0; i < buckSize; i++) {
            const node = buck.$_get(i)
            if (__runtimeSameValueZero(node.val, v)) {
                node.val = v
                return this
            }
        }

        // not found, add to insertion-ordered list
        const node = new ListNode()
        node.val = v
        const t = this.listRoot
        node.next = t
        node.prev = t.prev
        t.prev.next = node
        t.prev = node
        buck.push(node)

        this.sizeVal += 1

        if (this.sizeVal * 4 > this.buckets.length * 3) {
            this.rehash();
        }

        return this;
    }

    internal get(v: NullishType): K | undefined {
        const buck = this.getBucket(v)
        const buckSize = buck.length
        for (let i = 0; i < buckSize; i++) {
            const node = buck.$_get(i)
            if (__runtimeSameValueZero(node.val, v)) {
                return node.val as K
            }
        }
        return undefined;
    }

    /**
     * Checks if a value is in the Set
     *
     * @param v the value to find in the Set
     *
     * @returns true if the value is in the Set
     */
    override has(v: K): boolean {
        const buck = this.getBucket(v)
        const buckSize = buck.length
        for (let i = 0; i < buckSize; i++) {
            const node = buck.$_get(i)
            if (__runtimeSameValueZero(node.val, v)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Returns number of unique elements in the Set
     *
     * @returns number of unique elements in the Set
     */
    get size(): number {
        return this.sizeVal;
    }

    /**
     * Removes a value from the Set
     *
     * @param v the value to remove
     */
    delete(v: K): boolean {
        return this.deleteImpl(v)
    }

    internal deleteImpl(v: NullishType): boolean {
        const buck = this.getBucket(v)
        const buckSize = buck.length
        for (let i = 0; i < buckSize; i++) {
            const node = buck.$_get(i)
            if (__runtimeSameValueZero(node.val, v)) {
                node.prev.next = node.next
                node.next.prev = node.prev
                buck.fill(buck.pop()!, i, i + 1)
                this.sizeVal -= 1
                return true
            }
        }
        return false
    }

    /**
     * Deletes all elements from the Set
     */
    clear(): void {
        this.sizeVal = 0
        this.listRoot.next = this.listRoot
        this.listRoot.prev = this.listRoot
        this.buckets = new Array<Array<ListNode>>()
        this.buckets.push(new Array<ListNode>())
    }

    internal mappedIterator<R>(fn: (n: ListNode) => R): IterableIterator<R> {
        return new SetIterator<R>(this.listRoot, this.listRoot, fn)
    }

    /**
     * Despite name, returns elements from the Set
     * @returns an iterable of the values in the set
     */
    override keys(): IterableIterator<K> {
        return this.values()
    }

    /**
     * Returns elements from the Set
     *
     * @returns an iterable of the values in the set
     */
    override values(): IterableIterator<K> {
        return this.mappedIterator<K>((x: ListNode): K => {
            return x.val as K
        })
    }

    override iterator(): IterableIterator<K> {
        return this.values();
    }

    /**
     * @returns an iterable of [v,v] pairs for every value `v` in the set.
     */
    override entries(): IterableIterator<[K, K]> {
        return this.mappedIterator<Object>((x: ListNode): Object => {
            const v = x.val as K;
            const res: [K, K] = [v, v]
            return res
        }) as IterableIterator<[K, K]>
    }

    /**
     * Applies a function over all elements of the Set
     *
     * @param fn to apply
     */
    override forEach(fn: (k: K) => void): void {
        iteratorForEach<K>(this.keys(), fn)
    }

    /**
     * Applies a function over all elements of the Set
     *
     * @param fn to apply
     */
    override forEach(fn: (k: K, v: K) => void): void {
        iteratorForEach<K>(this.keys(), (x: K): void => fn(x, x))
    }

    /**
     * Applies a function over all elements of the Set
     *
     * @param fn to apply
     */
    override forEach(fn: (k: K, v: K, set: Set<K>) => void): void {
        iteratorForEach<K>(this.keys(), (x: K): void => fn(x, x, this))
    }
}
