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

package std.concurrency;

import { launch } from "std/concurrency"

/// This class provides api for sync primitives (see SyncPrimitives.ets)
final class ConcurrencyHelpers {
    // Mutex api
    public static native mutexCreate(): Object;
    public static native mutexLock(mutex: Object): void;
    public static native mutexUnlock(mutex: Object): void;
    // Event api
    public static native eventCreate(): Object;
    public static native eventWait(event: Object): void;
    public static native eventFire(event: Object): void;
    // Spinlock api
    public static native spinlockCreate(): Object;
    public static spinlockGuard(spinlock: Object, callback: () => void) {
        ConcurrencyHelpers.spinlockGuard(spinlock, callback as Object);
    }

    private static native spinlockGuard(spinlock: Object, callback: Object): void;
}

class NativeAsyncWorkHelper {
    static queue(executeCb: long, completeCb: long, data: long): void {
        NativeAsyncWorkHelper.queueInternal(executeCb, completeCb, data);
    }
    static async queueInternal(executeCb: long, completeCb: long, data: long) {
        let p = launch<void, (nativeCb: long, data: long, needNativeScope: boolean) => void>(NativeAsyncWorkHelper.asyncWorkNativeInvokeInternal, executeCb, data, true);
        p.Await();
        NativeAsyncWorkHelper.asyncWorkNativeInvokeInternal(completeCb, data, false);
    }
    static native asyncWorkNativeInvoke(nativeCb: long, data: long, needNativeScope: boolean): void;
    static asyncWorkNativeInvokeInternal(nativeCb: long, data: long, needNativeScope: boolean) {
        NativeAsyncWorkHelper.asyncWorkNativeInvoke(nativeCb, data, needNativeScope);
    }
}
