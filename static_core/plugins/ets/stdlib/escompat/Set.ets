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

export interface ReadonlySet<T> /* TODO(kprokopenko): extends Iterable */ {
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
    size(): number; // TODO(kprokopenko): must be a property

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
    forEach(fn: (value: T, value2: T, set: ReadonlySet<T>) => void): void;

    // TODO(kprokopenko): uncomment when internal issue #14416 is fixed

    /**
     * Despite name, returns elements from the Set
     * @returns an iterable of the values in the set
     */
    //keys(): ValuesIterator<T>;

    /**
     * Returns elements from the Set
     *
     * @returns an iterable of the values in the set
     */
    //values(): ValuesIterator<T>;

    /**
     * @returns an iterable of [v,v] pairs for every value `v` in the set.
     */
    //entries(): MapIterator<T, T>;
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
        if (__runtimeIsSameReference(this.node, this.tail)) {
            return new IteratorResult<R>()
        }
        const res = new IteratorResult<R>(this.mapper(this.node) as R)
        this.node = this.node.next
        return res
    }

    override iterator(): Iterator<R> {
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

    /**
     * Constructs an empty Set
     */
    constructor() {
        this.buckets.push(new Array<ListNode>())
    }

    /**
     * Constructs a Set with same elements
     * @param iterable collection of elements to add to a set
     */
    constructor(iterable: Iterable<K>) {
        this()
        iteratorForEach<K>(iterable.iterator(), (x: K): void => {
            this.add(x)
        })
    }

    private getBucket(v: NullishType): Array<ListNode> {
        return this.buckets.at(hashOf(v) % this.buckets.length)!
    }

    internal static __asGeneric<T>(x: Object): (x: T) => void {
        return x as (x: T) => void
    }

    private rehash(): void {
        const iter = new SetIterator<ListNode>(this.listRoot.next, this.listRoot, (x: ListNode): ListNode => x)
        const oldSize = this.buckets.length
        for (let i = 0; i < oldSize; i++) {
            const a = this.buckets.at(i)!
            // TODO(kprokopenko): find a better solution
            while (a.length != 0) {
                a.pop()
            }
            this.buckets.push(new Array<ListNode>())
        }
        const fn: (v: Object) => void = (v: Object): void => {
            const x = v as ListNode
            this.getBucket(x.val!).push(x)
        };
        // internal issue #13655
        iteratorForEach<ListNode>(iter, Set.__asGeneric<ListNode>(fn as Object))
    }

    /**
     * Puts a value into the Set
     *
     * @param v the value to put into the Set
     * @returns this
     */
    add(v: K): Set<K> {
        // insert into buckets
        const buck = this.getBucket(v as NullishType)
        const buckSize = buck.length
        for (let i = 0; i < buckSize; i++) {
            const node = buck.$_get(i)
            if (node.val!.equals(v as Object)) {
                node.val = v as Object
                return this
            }
        }

        // not found, add to insertion-ordered list
        const node = new ListNode()
        node.val = v as Object
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
            if (node.val!.equals(v)) {
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
        return !__runtimeIsSameReference(this.get(v as NullishType) as NullishType, undefined);
    }

    /**
     * Returns number of unique elements in the Set
     *
     * @returns number of unique elements in the Set
     */
    override size(): number {
        return this.sizeVal;
    }

    /**
     * Removes a value from the Set
     *
     * @param v the value to remove
     */
    delete(v: K): boolean {
        return this.deleteImpl(v as Object)
    }

    internal deleteImpl(v: Object): boolean {
        const buck = this.getBucket(v)
        const buckSize = buck.length
        for (let i = 0; i < buckSize; i++) {
            const node = buck.$_get(i)
            if (node.val!.equals(v)) {
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
        return new SetIterator<R>(this.listRoot.next, this.listRoot, fn)
    }

    /**
     * Despite name, returns elements from the Set
     * @returns an iterable of the values in the set
     */
    /* override */ keys(): IterableIterator<K> {
        return this.values()
    }

    /**
     * Returns elements from the Set
     *
     * @returns an iterable of the values in the set
     */
    /* override */ values(): IterableIterator<K> {
        return this.mappedIterator<K>((x: ListNode): K => {
            return x.val! as K
        })
    }

    /**
     * @returns an iterable of [v,v] pairs for every value `v` in the set.
     */
    /* override */ entries(): IterableIterator<Entry<K, K>> {
        return this.mappedIterator<Entry<K, K>>((x: ListNode): NullishType => {
            const v = x.val as K;
            return new Entry<K, K>(v, v)
        }) as IterableIterator<Entry<K, K>>
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
    // fn should take Set instead of ReadonlySet, but blocked by #15096
    override forEach(fn: (k: K, v: K, set: ReadonlySet<K>) => void): void {
        iteratorForEach<K>(this.keys(), (x: K): void => fn(x, x, this))
    }
}
