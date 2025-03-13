/**
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

package std.containers;

/// This class provides api for sync primitives (see SyncPrimitives.ets)
final class ConcurrencyHelpers {
    // Mutex api
    public static native mutexCreate(): Object;
    public static native mutexLock(mutex: Object): void;
    public static native mutexUnlock(mutex: Object): void;

    public static lockGuard(mutex: Object, callback: () => void): void {
        ConcurrencyHelpers.mutexLock(mutex);
        try {
            callback();
        } finally {
            ConcurrencyHelpers.mutexUnlock(mutex);
        }
    }

    // Event api
    public static native eventCreate(): Object;
    public static native eventWait(event: Object): void;
    public static native eventFire(event: Object): void;

    // CondVar api
    public static native condVarCreate(): Object;
    public static native condVarWait(condVar: Object, mutex: Object): void;
    public static native condVarNotifyOne(condVar: Object, mutex: Object): void;
    public static native condVarNotifyAll(condVar: Object, mutex: Object): void;
}
