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

type UncaughtErrorListener = StdProcess.UncaughtErrorListener;

/**
 * @class Stores EventListener that invokes when unhandled exception occurs
 */
class ErrorHandler {
    private constructor() {}

    /**
     * Constructs handler by passing EventListener
     *
     * @param cb inner callback
     *
     */
    public constructor(cb: UncaughtErrorListener) {
        this.callback = cb;
    }

    /**
     * Does not return.
     * Calls a custom handler on uncaught error and then exits
     *
     * @param error Uncaught error
     */
    public handleUncaughtError(error: Object): void {
        if (error instanceof Error) {
            error = error as Error;
        } else if (error instanceof Exception) {
            error = error as Exception;
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
        } finally {
            let procManager = new StdProcess.ProcessManager();
            procManager.exit(1);
        }
    }

    /**
     * Resets inner EventListener
     *
     * @param cb new callback
     *
     */
    public reset(cb: UncaughtErrorListener) {
        ConcurrencyHelpers.lockGuard(this.mutex, () => {
            this.callback = cb;
        });
    }

    private mutex: Object = ConcurrencyHelpers.mutexCreate();
    private callback: UncaughtErrorListener = (error: Object) => {};
};
