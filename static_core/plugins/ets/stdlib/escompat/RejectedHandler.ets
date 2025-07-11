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

type RejectedObjectListener = StdProcess.EventListener;

/**
 * @class Stores EventListener that handles rejected promises and jobs
 */
class RejectedHandler {
    private constructor() {}

    /**
     * Constructs handler by passing EventListener
     *
     * @param cb inner callback
     *
     */
    public constructor(cb: RejectedObjectListener) {
        this.callback = cb;
    }

    /**
     * Resets inner EventListener
     *
     * @param cb new callback
     *
     */
    public reset(cb: RejectedObjectListener) {
        this.callback = cb;
    }

    /**
     * Applies handler to a provided object
     * It is used to apply handler to a result of rejected promise or job
     *
     * @param obj object on which the callback will be applied
     *
     */
    public apply(obj: Object): void {
        this.callback(obj);
    }

    private callback: RejectedObjectListener = (obj: Object) => {};
};
