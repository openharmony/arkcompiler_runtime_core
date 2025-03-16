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

package std.core;

/**
 * FinalizableWeakRef - weak reference to object with finalizer.
 * This is an iternal class and is only used in napi and intrerop.
 * It's a expected that finalizer is a native function that doesn't need managed scope
 * and used to finalize some native data via finalizerPtr.
 * A weak reference to an object is a reference that does not prevent the object
 * from being reclaimed by the GC.
 * @remark It's important to avoid relying on any specific GC behaviors.
 *
 * @tparam T Object type for weak reference
 */
final class FinalizableWeakRef<T extends Object> {
    /**
     * Constructs weak reference object referring to a given target object
     *
     * @param target - target object for weak reference
     */
    public constructor(target: T) {
        this.referent = target;
    }

    // Reference to target object
    private referent: T | undefined;
    private prevRef: FinalizableWeakRef<T> | null = null;
    private nextRef: FinalizableWeakRef<T> | null = null;
    private finalizerPtr: long;
    private finalizerArgPtr: long;
}
