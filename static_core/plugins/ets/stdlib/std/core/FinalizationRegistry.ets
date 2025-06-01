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
    // Can be changed to local trimplet class when it will be implemented
    private obj_token_refs: (Object | undefined)[] = [];
    private callback_arguments: (T | undefined)[] = [];
    // Contains chains of used and free trimplet indexes, [0] serves as uselist head
    private idxarr: int[] = [];
    private freeidx: int = -1;
    private static finalizationRegistryLock = new Mutex();
    private mutex = new Mutex();
    private launchMode: int;

    private static readonly FIRST_SIZE = 32;

    private static execCleanup(array: (WeakRef<FinalizationRegistry<T>> | undefined)[], launchmode: int): void {
        FinalizationRegistry.finalizationRegistryLock.lock();
        try {
            for (let i = 0; i < array.length; i++) {
                if (array[i]?.deref() !== undefined) {
                    if (array[i]!.deref()!.launchMode == launchmode) {
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

    private init_arrays(size: int): void {
        assertLT(1, size);
        this.idxarr = new int[size];
        this.idxarr[0] = -1; // first elem reserved for useidx head
        this.enlarge_freeidx(1);

        this.obj_token_refs = new Object[2 * size];
        this.callback_arguments = new (T | undefined)[size];
    }

    private enlarge_arrays(newsize: int): void {
        let old_idxarr = this.idxarr;
        this.idxarr = new int[newsize];
        for (let i = 0; i < old_idxarr.length; ++i) {
            this.idxarr[i] = old_idxarr[i];
        }
        this.enlarge_freeidx(old_idxarr.length);

        let old_obj_token_refs = this.obj_token_refs;
        this.obj_token_refs = new Object[newsize * 2];
        for (let i = 0; i < old_obj_token_refs.length; ++i) {
            this.obj_token_refs[i] = old_obj_token_refs[i];
        }
        let old_callback_arguments = this.callback_arguments;
        this.callback_arguments = new (T | undefined)[newsize];
        for (let i = 0; i < old_callback_arguments.length; ++i) {
            this.callback_arguments[i] = old_callback_arguments[i];
        }
    }

    private enlarge_freeidx(oldsize: int): void {
        // push new idxarr elements to freelist
        let last_free = this.idxarr.length - 1;
        this.idxarr[last_free] = this.freeidx;
        for (let i = oldsize; i < last_free; i++) {
            this.idxarr[i] = i + 1;
        }
        this.freeidx = oldsize;
    }

    /**
     * @returns size of the FinalizationRegistry
     */
    public getSize(): int {
        let count: int;
        this.mutex.lockGuard(() => {
            for (let i = this.idxarr[0]; i != -1; i = this.idxarr[i]) {
                count++;
            }
        });
        return count;
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
        this.init_arrays(FinalizationRegistry.FIRST_SIZE);
        this.callback = callback;
        FinalizationRegistry.finalizationRegistryLock.lockGuard(() => {
            this.launchMode = FinalizationRegistry.registerInstance(new WeakRef<Object>(this));
        });
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
        let refObject = new WeakRef<Object>(object);
        let refToken?: WeakRef<Object> | undefined = undefined;
        if (token != undefined) {
            refToken = new WeakRef<Object>(token);
        }
        this.mutex.lockGuard(() => {
            let free_index = this.allocTrimplet();
            // NOTE: GC and thus cleanup function can be triggered here after JIT/AOT optimizations
            this.setTrimplet(free_index, refObject, callbackArg, refToken);
        });
    }

    /**
     * Removes all entries with the specific token
     *
     * @param token object specified which entries should be removed
     */
    public unregister(token: Object): void {
        this.mutex.lockGuard(() => {
            let prev = 0;
            for (let i = this.idxarr[prev]; i != -1; prev = i, i = this.idxarr[i]) {
                let refToken = this.getToken(i);
                if (refToken != undefined && refToken.deref() == token) {
                    this.freeNextTrimplet(prev);
                    i = prev; // iterator rollback
                }
            }
        });
    }

    private cleanup(): void {
        this.mutex.lock();
        try {
            let prev = 0;
            for (let i = this.idxarr[prev]; i != -1; prev = i, i = this.idxarr[i]) {
                let object = this.getObject(i);
                if (object != undefined && object.deref() == undefined) {
                    // Remove from trimplet first to prevent
                    // calling it again in separate thread
                    let arg: T | undefined = this.getCallbackArg(i);
                    this.freeNextTrimplet(prev);
                    i = prev; // iterator rollback
                    try {
                        this.mutex.unlock();
                        this.callback(arg as T);
                    } catch (e) {
                        if (e instanceof Error) {
                            console.log("Error: " + e.toString());
                            if (e.stack) {
                                console.log(e.stack);
                            }
                        }
                    } finally {
                        this.mutex.lock();
                    }
                }
            }
        } finally {
            this.mutex.unlock();
        }
    }

    private allocTrimplet(): int {
        if (this.freeidx == -1) {
            this.enlarge_arrays(this.idxarr.length * 2);
        }
        // pop elem from freelist
        let new_idx = this.freeidx;
        this.freeidx = this.idxarr[new_idx];

        // push elem to uselist
        this.idxarr[new_idx] = this.idxarr[0];
        this.idxarr[0] = new_idx;
        return new_idx;
    }

    private freeNextTrimplet(index: int): void {
        // remove next elem from uselist
        let freed_idx = this.idxarr[index];
        this.idxarr[index] = this.idxarr[freed_idx];

        // push freed elem to freelist
        this.idxarr[freed_idx] = this.freeidx;
        this.freeidx = freed_idx;

        // clean data
        this.setTrimplet(freed_idx, undefined, undefined, undefined);
    }

    private getObject(index: int): WeakRef<Object> | undefined {
        return this.obj_token_refs[2 * index] as (WeakRef<Object> | undefined);
    }

    private getCallbackArg(index: int): T | undefined {
        return this.callback_arguments[index];
    }

    private getToken(index: int): WeakRef<Object> | undefined {
        return this.obj_token_refs[2 * index + 1] as (WeakRef<Object> | undefined);
    }

    private setTrimplet(index: int, obj: WeakRef<Object> | undefined, cbArg: T | undefined, token: WeakRef<Object> | undefined): void {
        this.obj_token_refs[2 * index] = obj;
        this.obj_token_refs[2 * index + 1] = token;
        this.callback_arguments[index] = cbArg;
    }

    private static native registerInstance(object: Object): int;
    private static native finishCleanup(): void;

}
