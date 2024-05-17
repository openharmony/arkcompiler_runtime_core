/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
 * WeakRef - weak reference to object.
 * A weak reference to an object is a reference that does not prevent the object
 * from being reclaimed by the GC.
 * @remark It's important to avoid relying on any specific GC behaviors.
 *
 * @tparam T Object type for weak reference
 */
export final class WeakRef<T extends Object> {
    // Reference to target object
    private referent: T | undefined;

    /**
     * Constructs weak reference object referring to a given target object
     *
     * @param target - target object for weak reference
     */
    public constructor(target: T) {
        this.referent = target;
    }

    /**
     * Returns underlying parameter
     *
     * @returns instance's target object, or undefined if the target object has been collected
     */
    public deref(): T | undefined {
        return this.referent;
    }
}
