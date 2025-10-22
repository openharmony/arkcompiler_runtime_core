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

package std.core;

class FinalizationNode<T> {
    constructor (obj: WeakRef<Object>, args: T, token?: WeakRef<Object>) {
        this.obj = obj;
        this.token = token;
        this.args = args;
    }
    public obj: WeakRef<Object>;
    public token: WeakRef<Object> | undefined;
    public args: T;
    public prev: FinalizationNode<T> | undefined = undefined;
    public next: FinalizationNode<T> | undefined = undefined;
}

/**
  * FinalizationRegistry provides a way to execute the provided callback when a
  * registered object gets garbage collected.
  * FinalizationRegistry is coroutine-safe class.
  * - It is safe to pass an instance to another coroutines.
  * - It is safe to call methods from several coroutines.
  *
  * Notes:
  * - Runtime doesn't provide any guarantees that the registered object will be
  * garbage collected (even if its dead).
  * - Even the object is garbage collected runtime doesn't guarantee that
  * the callback will be called.
  * - Runtime ignores any exceptions thrown in the callback.
  * - User should avoid call await in the callback.
  * - Runtime calls the callback after the object is garbage collected at some point
  * during execution.
  * - The callback may be called in any coroutine.
  */
export final class FinalizationRegistry<T> {
    private callback: (value: T) => void;
    private static finalizationRegistryLock = new Mutex();
    private mutex = new Mutex();
    private workerDomain: int;

    // Map for unregistrable objects
    private buckets: FixedArray<FinalizationNode<T> | undefined> = [];
    private mapSize: int = 0;

    // List for non-unregistrable objects
    private nonUnregistrableList: FinalizationNode<T> | undefined = undefined;
    private listSize: int = 0;

    private static execCleanup(array: FixedArray<(WeakRef<FinalizationRegistry<T>> | undefined)>, workerDomain: int): void {
        FinalizationRegistry.finalizationRegistryLock.lock();
        try {
            for (let i = 0; i < array.length; i++) {
                if (array[i]?.deref() !== undefined) {
                    if (array[i]!.deref()!.workerDomain == workerDomain) {
                        try {
                            FinalizationRegistry.finalizationRegistryLock.unlock();
                            array[i]!.deref()!.cleanup();
                        } finally {
                            FinalizationRegistry.finalizationRegistryLock.lock();
                        }
                    }
                } else {
                    array[i] = undefined;
                }
            }
        } finally {
            FinalizationRegistry.finalizationRegistryLock.unlock();
            FinalizationRegistry.finishCleanup();
        }
    }

    /**
     * @returns size of the FinalizationRegistry
     */
    public getSize(): int {
        let count: int = 0;
        this.mutex.lockGuard(() => {
            count = this.mapSize + this.listSize;
        });
        return count;
    }

    private rehash(newSizeArg: int) {
        const oldMap = this.buckets;
        const newSize = newSizeArg;
        this.buckets = new (FinalizationNode<T> | undefined)[newSize];
        this.mapSize = 0;
        for (let oldMapIdx = 0; oldMapIdx < oldMap.length; oldMapIdx++) {
            let nextNode: (FinalizationNode<T> | undefined) = undefined;
            for (let currNode = oldMap[oldMapIdx]; currNode != undefined; currNode = nextNode) {
                nextNode = currNode!.next;
                if (currNode!.token!.deref() != undefined) {
                    const hashCode = this.getBucketIndex(Runtime.getHashCode(currNode!.token!.deref()!));
                    if (this.buckets[hashCode] == undefined) {
                        this.buckets[hashCode] = currNode;
                        currNode!.prev = undefined;
                        currNode!.next = undefined;
                    } else {
                        let head = this.buckets[hashCode]!;
                        head.prev = currNode;
                        currNode!.prev = undefined;
                        currNode!.next = head;
                        this.buckets[hashCode] = currNode;
                    }
                    this.mapSize++;
                } else {
                    // Token was deleted, so put node into non-unregistrable list
                    currNode!.prev = undefined;
                    currNode!.next = this.nonUnregistrableList;
                    this.nonUnregistrableList = currNode;
                    this.listSize++;
                }
            }
        }
    }

    private getBucketIndex(x: int): int {
        const t = x >> 31;
        return ((x ^ t) - t) & (this.buckets.length - 1);
    }

    private deleteSingleNodeFromMap(nodeToDelete: FinalizationNode<T>, hashCode: int) {
        let prev = nodeToDelete.prev;
        let next = nodeToDelete.next;

        if (prev == undefined && next == undefined) {
            this.buckets[hashCode] = undefined;
        } else if (prev == undefined) {
            this.buckets[hashCode] = next;
            next!.prev = undefined;
        } else if (next == undefined) {
            prev!.next = undefined;
        } else {
            prev!.next = next;
            next!.prev = prev;
        }

        this.mapSize--;
    }

    private deleteSingleNodeFromList(nodeToDelete: FinalizationNode<T>) {
        let prev = nodeToDelete.prev;
        let next = nodeToDelete.next;
        if (prev == undefined && next == undefined) {
            this.nonUnregistrableList = undefined;
        } else if (next == undefined) {
            prev!.next = next;
        } else if (prev == undefined) {
            this.nonUnregistrableList = next;
            next!.prev = prev;
        } else {
            prev!.next = next;
            next!.prev = prev;
        }

        this.listSize--;
    }

    /**
     * Create FinalizationRegistry object with the specific callback function.
     * One can register an object and after reclamation of the object
     * the callback function will be called.
     *
     * @param callback_value callback function which will be called
     * after registered object reclamation.
     */
    public constructor(callback: (value: T) => void) {
        this.callback = callback;
        FinalizationRegistry.finalizationRegistryLock.lockGuard(() => {
            this.workerDomain = FinalizationRegistry.registerInstance(new WeakRef<Object>(this));
        });
        this.buckets = new (FinalizationNode<T> | undefined)[16];  // Init size for hash map is 16
    }

    /**
     * Registers an object with specific callback's argument and
     * optional token - an object used to remove the entry from the registry.
     * @see unregister
     * After the object is garbage collected, runtime will call the callback
     * with the specific argument.
     *
     * @param object a tracking object
     *
     * @param callbackArg callback argument for callback method
     *
     * @param token an object used to remove the entry from the registry
     */
    public register(object: Object, callbackArg: T, token?: Object): void {
        const refObject = new WeakRef<Object>(object);

        if (token != undefined) {
            // Add unregistrable node into map
            const refToken = new WeakRef<Object>(token!);
            const objectHash = Runtime.getHashCode(token!);
            this.mutex.lock();

            const hashCode = this.getBucketIndex(objectHash);
            let newNode = new FinalizationNode<T>(refObject, callbackArg, refToken);
            if (this.buckets[hashCode] != undefined) {
                let head = this.buckets[hashCode]!;
                head.prev = newNode;
                newNode.next = head;
            }
            this.buckets[hashCode] = newNode;
            this.mapSize++;

            // Rehash if elements count > 0.75 of map size
            if (this.mapSize * 4 > this.buckets.length * 3) {
                this.rehash(this.buckets.length * 4)
            }

            this.mutex.unlock();
        } else {
            // Add non-unregistrable node into list
            this.mutex.lock();
            let newNode = new FinalizationNode<T>(refObject, callbackArg);
            newNode.next = this.nonUnregistrableList;
            if (this.nonUnregistrableList !== undefined) {
                this.nonUnregistrableList!.prev = newNode;
            }
            this.nonUnregistrableList = newNode;
            this.listSize++;
            this.mutex.unlock();
        }
    }

    /**
     * Removes all entries with the specific token
     *
     * @param token object specified which entries should be removed
     */
    public unregister(token: Object): void {
        this.mutex.lockGuard(() => {
            const hashCode = this.getBucketIndex(Runtime.getHashCode(token));
            if (this.buckets[hashCode] != undefined) {
                let currNode = this.buckets[hashCode];
                while (currNode != undefined) {
                    const nextNode = currNode.next;
                    if (token == currNode.token!.deref()) {
                        this.deleteSingleNodeFromMap(currNode, hashCode);
                    }
                    currNode = nextNode;
                }
            }

        });
    }

    private callFinalizerIfNeeded(node: FinalizationNode<T>, hashCode?: int) {
        const object = node.obj;
        if (object.deref() == undefined) {
            const arg = node.args;
            if (hashCode != undefined) {
                this.deleteSingleNodeFromMap(node, hashCode);
            } else {
                this.deleteSingleNodeFromList(node);
            }
            try {
                this.mutex.unlock();
                this.callback(arg as T);
            } catch (e) {
                console.log("Error: " + e.toString());
                if (e instanceof Error) {
                    if (e.stack) {
                        console.log(e.stack);
                    }
                }
            } finally {
                this.mutex.lock();
            }
        }
    }

    private cleanup(): void {
        this.mutex.lock();
        try {
            // cleanup unregistrable objects
            for (let mapIdx = 0; mapIdx < this.buckets.length; mapIdx++) {
                let nextNode: (FinalizationNode<T> | undefined) = undefined;
                for (let currNode = this.buckets[mapIdx]; currNode != undefined; currNode = nextNode) {
                    nextNode = currNode!.next;
                    this.callFinalizerIfNeeded(currNode!, mapIdx);
                }
            }

            // cleanup non-unregistrable objects
            let nextNode: (FinalizationNode<T> | undefined) = undefined;
            for (let currNode = this.nonUnregistrableList; currNode != undefined; currNode = nextNode) {
                nextNode = currNode!.next;
                this.callFinalizerIfNeeded(currNode!);
            }
        } catch (e) {
            console.log(`Error: ` + e.toString());
            if (e instanceof Error) {
                if (e.stack) {
                    console.log(e.stack);
                }
            }
        } finally {
            this.mutex.unlock();
        }
    }

    private static native registerInstance(object: Object): int;
    private static native finishCleanup(): void;

}
