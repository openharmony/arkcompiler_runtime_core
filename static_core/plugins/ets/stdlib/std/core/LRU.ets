/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

package std.core;

/**
 * Concurrent LRU (Least Recently Used) cache with fixed capacity.
 * Thread-safe operations using a mutex.
 * @template K - key type
 * @template V - value type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
final class ConcurrentLRUCache<K, V> {
    private readonly capacity: int;
    private readonly map: Map<K, LRUNode<K, V>>;
    private head: LRUNode<K, V> | undefined;
    private tail: LRUNode<K, V> | undefined;
    private size: int = 0;
    private readonly mutex: Object;

    /**
     * Constructs a new LRUCache with the given capacity.
     * @param { int } - maximum number of entries to cache (must be positive)
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(capacity: int) {
        if (capacity <= 0) {
            throw new Error("LRUCache capacity must be positive");
        }
        this.capacity = capacity;
        this.map = new Map<K, LRUNode<K, V>>();
        this.head = undefined;
        this.tail = undefined;
        this.mutex = ConcurrencyHelpers.mutexCreate();
    }

    /**
     * Retrieves the value associated with the key, moving the entry to the most recently used position.
     * Returns undefined if the key is not present.
     * @param { K } - the key to look up
     * @returns { V | undefined } the cached value or undefined
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get(key: K): V | undefined {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            const node = this.map.get(key);
            if (node !== undefined) {
                this.moveToFront(node);
                return node.value;
            }
            return undefined;
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    /**
     * Checks if a key is present in the cache.
     * Does not affect the access order.
     * @param { K } - the key to check
     * @returns { true } if the key exists in the cache
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public has(key: K): boolean {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            return this.map.has(key);
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    /**
     * Inserts or updates a key-value pair, making it the most recently used.
     * If the cache is at capacity, the least recently used entry is evicted.
     * @param { K } - the key
     * @param { V } - the value
     * @returns { void }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public put(key: K, value: V): void {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            let node: LRUNode<K, V> | undefined = this.map.get(key);
            if (node !== undefined) {
                // update existing
                node.value = value;
                this.moveToFront(node);
            } else {
                // create new node
                node = new LRUNode<K, V>(key, value);
                this.map.set(key, node);
                this.addToFront(node);
                this.size++;
                // evict if over capacity
                if (this.size > this.capacity) {
                    this.evictLRU();
                }
            }
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    /**
     * Removes the entry for the given key, if present.
     * @param { K } - the key to remove
     * @returns { true } if an entry was removed, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public remove(key: K): boolean {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            const node = this.map.get(key);
            if (node === undefined) {
                return false;
            }
            this.removeNode(node);
            this.map.delete(key);
            this.size--;
            return true;
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    /**
     * Clears all entries from the cache.
     * @returns { void }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public clear(): void {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            this.map.clear();
            this.head = undefined;
            this.tail = undefined;
            this.size = 0;
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    /**
     * Returns the current number of entries in the cache.
     * @returns { int } The current number of entries in the cache.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getSize(): int {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            return this.size;
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    /**
     * Returns the cache capacity.
     * @returns { int } The capacity of the cache.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getCapacity(): int {
        return this.capacity;
    }

    /**
     * Atomically computes and inserts a value if the key is absent.
     * If the key already exists, returns the existing value without calling producer.
     * @param { K } - the key
     * @param { Function } - function that computes the value for the key
     * @returns { V } - the existing or newly computed value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public computeIfAbsent(key: K, producer: (key: K) => V): V {
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            let node: LRUNode<K, V> | undefined = this.map.get(key);
            if (node !== undefined) {
                this.moveToFront(node);
                return node.value;
            }
            const value = producer(key);
            node = new LRUNode<K, V>(key, value);
            this.map.set(key, node);
            this.addToFront(node);
            this.size++;
            if (this.size > this.capacity) {
                this.evictLRU();
            }
            return value;
        } finally {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    // -------- private helper methods ----------

    private moveToFront(node: LRUNode<K, V>): void {
        if (node === this.head) {
            return;
        }
        this.removeNode(node);
        this.addToFront(node);
    }

    private removeNode(node: LRUNode<K, V>): void {
        if (node.prev !== undefined) {
            node.prev!.next = node.next;
        } else {
            // node is head
            this.head = node.next;
        }
        if (node.next !== undefined) {
            node.next!.prev = node.prev;
        } else {
            // node is tail
            this.tail = node.prev;
        }
        node.prev = undefined;
        node.next = undefined;
    }

    private addToFront(node: LRUNode<K, V>): void {
        node.next = this.head;
        node.prev = undefined;
        if (this.head !== undefined) {
            this.head!.prev = node;
        }
        this.head = node;
        if (this.tail === undefined) {
            this.tail = node;
        }
    }

    private evictLRU(): void {
        if (this.tail === undefined) {
            return;
        }
        const lru = this.tail!;
        this.removeNode(lru);
        this.map.delete(lru.key);
        this.size--;
    }
}

    /**
 * Internal doubly linked list node.
 */
class LRUNode<K, V> {
    key: K;
    value: V;
    prev: LRUNode<K, V> | undefined;
    next: LRUNode<K, V> | undefined;

    constructor(key: K, value: V) {
        this.key = key;
        this.value = value;
    }
}
