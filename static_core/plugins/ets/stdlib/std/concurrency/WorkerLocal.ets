/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

package std.concurrency;

export class WorkerLocal<T> {

    /**
    * Creates a WorkerLocal instance without initialization function.
    * Requires explicit value initialization via set() before calling get().
    * @constructor
    */
    constructor() {}
    
    /**
     * Creates a WorkerLocal instance with value initialization function.
     * Automatically initializes value on first get() call per worker.
     * @constructor
     * @param {() => T} init - Factory function for initial worker-local value
     */
    constructor(init: () => T) {
      this.init = init;
    }
  
    /**
     * Retrieves the worker-local value for current worker. Auto-initializes
     * using provided init function if value not exists.
     * 
     * @returns {T} Worker-specific stored value
     * @throws {Error} When value not initialized and no init function provided
     */
    get():T {
        const workerId = CoroutineExtras.getWorkerId();
        if (!this.storage.has(workerId)) {
            if (this.init) {
                this.storage.set(workerId, this.init!());
            } else {
                throw new Error("WorkerLocal value not initialized. Call set() first or provide an init function.");
            }
        }
        return this.storage.get(workerId)!;
    }

    /**
     * Updates the worker-local value for current worker.
     * 
     * @param {T} value - New value to store for current worker
     */ 
    set(value: T):void {
        const workerId = CoroutineExtras.getWorkerId();
        this.storage.set(workerId, value);
    }

    /**
     * Delete the worker-local value for current worker.
     */ 
    delete():void {
        const workerId = CoroutineExtras.getWorkerId();
        this.storage.delete(workerId);
    }

    private storage = new ConcurrentHashMap<WorkerId, T>();
    private init?: () => T;
}
