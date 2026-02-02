/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
/**
 * @file Defines the ErrorHandler for ArkTS
 * @kit ArkTS
 */
package std.core;

type UncaughtErrorListener = StdProcess.UncaughtErrorListener;

/**
 * Error handler class for uncaught exceptions. It calls registered listeners when an unhandled exception occurs.
 * 
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class ProcessErrorHandler {
    private constructor() {}

    /**
     * Constructs a ProcessErrorHandler instance with the provided uncaught error listener.
     * 
     * @param { UncaughtErrorListener } cb - The callback function for handling uncaught errors.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(cb: UncaughtErrorListener) {
        this.callback = cb;
    }

    /**
     * Handles uncaught errors. It calls the custom error handling callback and exits the process 
     * if handling fails or the error type is invalid.
     * 
     * @param { Object } error - The uncaught error object that occurred.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public handleUncaughtError(error: Object): void {
        if (error instanceof Error) {
            error = error as Error;
        } else {
            console.error('Error type is not recognized:', error);
            let procManager = new StdProcess.ProcessManager();
            procManager.exit(1);
        }
        ConcurrencyHelpers.mutexLock(this.mutex);
        try {
            this.callback(error);
        } catch (e) {
            console.error('Error while invoking callback:', e);
            let procManager = new StdProcess.ProcessManager();
            procManager.exit(1);
        }
        ConcurrencyHelpers.mutexUnlock(this.mutex);
    }

    /**
     * Resets the internal error handling callback function.
     * @param { UncaughtErrorListener } cb - The new error handling callback function.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public reset(cb: UncaughtErrorListener) {
        ConcurrencyHelpers.lockGuard(this.mutex, () => {
            this.callback = cb;
        });
    }

    private mutex: Object = ConcurrencyHelpers.mutexCreate();
    private callback: UncaughtErrorListener = (error: Object) => {};
};
