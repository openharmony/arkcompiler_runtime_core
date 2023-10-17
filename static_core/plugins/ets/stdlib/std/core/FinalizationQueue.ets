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

package std.core;

export final class FinalizationQueue {

    private callback: (value: Object) => void;
    private trimplets: (Object | null)[];
    // Contains chains of used and free trimplet indexes, [0] serves as uselist head
    private idxarr: int[];
    private freeidx: int = -1;

    private static readonly FIRST_SIZE = 32;

    private init_arrays(size: int): void {
        assert size > 1;
        this.idxarr = new int[size];
        this.idxarr[0] = -1; // first elem reserved for useidx head
        this.enlarge_freeidx(1);

        this.trimplets = new Object[3 * size];
    }

    private enlarge_arrays(newsize: int): void {
        let old_idxarr = this.idxarr;
        this.idxarr = new int[newsize];
        for (let i = 0; i < old_idxarr.length; ++i) {
            this.idxarr[i] = old_idxarr[i];
        }
        this.enlarge_freeidx(old_idxarr.length);

        let old_trimplets = this.trimplets;
        this.trimplets = new Object[newsize * 3];
        for (let i = 0; i < old_trimplets.length; ++i) {
            this.trimplets[i] = old_trimplets[i];
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
     * Returns size of the FinalizationQueue
     *
     * @returns size of the FinalizationQueue
     */
    public getSize(): int {
        let count: int;
        for (let i = this.idxarr[0]; i != -1; i = this.idxarr[i]) {
            count++;
        }
        return count;
    }

    /**
     * Creates FinalizationQueue object with the specific callback function.
     * One can register an object to this queue and after reclamation of this object
     * the callback function will be called.
     *
     * @param callback_value callback function which will be called
     * after registered object reclamation.
     */
    public constructor(callback_value: (value: Object) => void) {
        this.init_arrays(FinalizationQueue.FIRST_SIZE);
        this.callback = callback_value;
        FinalizationQueue.registerInstance(this);
    }

    /**
     * Registers an object with specific callback arg.
     * After the object will be removed, the callback method
     * will be called with the specific argument.
     *
     * @param object a tracking object
     *
     * @param callbackArg callback argument for callback method
     */
    public register(object: Object, callbackArg: Object | null): void {
        this.register(object, callbackArg, null);
    }

    /**
     * Registers an object with specific callback argument and
     * with token object - an object which can be used to remove
     * this registred callback. @see unregister
     * After the object will be removed, the callback method
     * will be called with the specific argument.
     *
     * @param object a tracking object
     *
     * @param callbackArg callback argument for callback method
     *
     * @param token an object which can be used to remove
     * this registred callback
     */
    public register(object: Object, callbackArg: Object | null, token: Object | null): void {
        let refObject = new WeakRef<Object>(object);
        let refToken: WeakRef<Object> | null = null;
        if (token != null) {
            refToken = new WeakRef<Object>(token!);
        }
        let free_index = this.allocTrimplet();
        this.setTrimplet(free_index, refObject, callbackArg, refToken);
    }

    /**
     * Removes all registered callbacks with the specific token value
     *
     * @param token all registered callbacks with this token will be removed
     */
    public unregister(token: Object | null): void {
        let prev = 0;
        for (let i = this.idxarr[prev]; i != -1; prev = i, i = this.idxarr[i]) {
            if (this.getToken(i) != null) {
                let refToken = this.getToken(i);
                if (refToken.deref() == token) {
                    this.freeNextTrimplet(prev);
                    i = prev; // iterator rollback
                }
            }
        }
    }

    private cleanup(): void {
        let prev = 0;
        for (let i = this.idxarr[prev]; i != -1; prev = i, i = this.idxarr[i]) {
            if (this.getObject(i).deref() == null) {
                // Remove from trimplet first to prevent
                // calling it again in separate thread
                let arg: Object | null = this.getCallbackArg(i);
                this.freeNextTrimplet(prev);
                i = prev; // iterator rollback
                this.callback(arg!);
            }
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

    private freeNextTrimplet(index: int): int {
        // remove next elem from uselist
        let freed_idx = this.idxarr[index];
        this.idxarr[index] = this.idxarr[freed_idx];

        // push freed elem to freelist
        this.idxarr[freed_idx] = this.freeidx;
        this.freeidx = freed_idx;

        // clean data
        this.setTrimplet(freed_idx, null, null, null);
    }

    private getObject(index: int): WeakRef<Object> {
        return this.trimplets[3 * index] as WeakRef<Object>;
    }

    private getCallbackArg(index: int): Object | null {
        return this.trimplets[3 * index + 1];
    }

    private getToken(index: int): WeakRef<Object> {
        return this.trimplets[3 * index + 2] as WeakRef<Object>;
    }

    private setTrimplet(index: int, obj: WeakRef<Object> | null, cbArg: Object | null, token: WeakRef<Object> | null): void {
        this.trimplets[3 * index] = obj;
        this.trimplets[3 * index + 1] = cbArg;
        this.trimplets[3 * index + 2] = token;
    }

    private static native registerInstance(object: Object): void;

}
