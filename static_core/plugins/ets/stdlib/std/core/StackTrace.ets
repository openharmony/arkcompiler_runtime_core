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
 * Provides functionality to trace code execution for performance analysis.
 * Wraps the HiTrace mechanism for marking key code paths and measuring durations.
 */
final class Tracer {
    /**
     * Start a synchronous trace section.
     * The trace section must be finished by calling finishTrace on the same thread.
     * Do not use across coroutine suspension points.
     *
     * @param { String } name The name of the trace section
     */
    public static native startTrace(name: String): void

    /**
     * Finish the current synchronous trace section.
     */
    public static native finishTrace(): void

    /**
     * Start an asynchronous trace section.
     * Safe to use across coroutine suspension points.
     * Must be paired with finishAsyncTrace using the same name and taskId.
     *
     * @param { String } name The name of the trace section
     * @param { int } taskId The unique identifier for the asynchronous trace
     */
    public static native startAsyncTrace(name: String, taskId: int): void

    /**
     * Finish an asynchronous trace section.
     *
     * @param { String } name The name of the trace section
     * @param { int } taskId The unique identifier for the asynchronous trace
     */
    public static native finishAsyncTrace(name: String, taskId: int): void
}

/**
 * Output a debug-level log message via the native logging system.
 * The message is output in both debug and release builds.
 *
 * @param { String } msg The message to log
 */
native function hilogDebug(msg: String): void

/**
 * Output an info-level log message via the native logging system.
 * The message is output in both debug and release builds.
 *
 * @param { String } msg The message to log
 */
native function hilogInfo(msg: String): void