/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

package std.debug.concurrency;

export final class CoroutineExtras {
    public static POLICY_ANY = 0;
    public static POLICY_NON_MAIN = 1;

    // returns coroutine worker id, which is the same for coros which are running within the same worker thread
    public static native getWorkerId(): int;
    // sets default coroutine affinity
    public static native setSchedulingPolicy(policy: int): void;
    // returns coroutine id
    public static native getCoroutineId(): int;
    // returns true if current worker is main, false otherwise
    public static native isMainWorker(): boolean;
    /**
     * This method changes the size of the worker pool
     * @param scaler is a parameter that determines the number of workers
     * that will be created or removed (in this case scaler is less than zero)
     */
    public static native scaleWorkersPool(scaler: int): void;
}
