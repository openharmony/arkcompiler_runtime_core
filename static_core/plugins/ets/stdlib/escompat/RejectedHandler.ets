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

package escompat;

type RejectedObjectListener = StdProcess.RejectedObjectListener;

/**
 * @class Stores EventListener that handles rejected promises and jobs
 */
class RejectedHandler {
    /**
     * Constructs a new RejectedHandler with an empty listener set.
     */
    constructor() {
        this.handlers = new containers.ConcurrentSet<RejectedObjectListener>();
    }

    /**
     * Registers a new rejection handler.
     *
     * @param cb new callback
     *
     */
    public register(cb: RejectedObjectListener): void {
        this.handlers.add(cb);
    }

    /**
     * Unregisters a previously registered rejection handler.
     *
     * @param cb The callback to be removed from the handler set.
     */
    public unregister(cb: RejectedObjectListener): void {
        this.handlers.delete(cb);
    }

    /**
     * Removes all registered rejection handlers.
     */
    public reset(): void {
        this.handlers.clear();
    }

    /**
     * Invokes all registered handlers with the provided rejection reason and object.
     * Typically used when a promise or task is rejected and listeners should be notified.
     *
     * @param reason rejection result of an object
     * @param obj the object itself
     *
     */
    public apply(reason: Error, obj: Object): void {
        for (const handler of this.handlers) {
            handler(reason, obj);
        }
    }

    private handlers: containers.ConcurrentSet<RejectedObjectListener>;
}
