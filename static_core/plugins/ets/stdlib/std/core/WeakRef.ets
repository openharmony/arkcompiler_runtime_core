/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
 * @file Defines the WeakRef for ArkTS
 * @kit ArkTS
 */

package std.core;

class BaseWeakRef<T> {
    // Reference to target object
    private referent: T | undefined;

    /**
     * Constructs a weak reference instance pointing to the given target object.
     *
     * @param { T } target - The target object for the weak reference.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(target: T) {
        this.referent = target;
    }

    /**
     * Gets the target object pointed to by the weak reference.
     *
     * @returns { T | undefined } Returns the target object, or undefined if the object has been garbage collected.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public final deref(): T | undefined {
        return this.referent;
    }
}

/**
 * WeakRef - Weak reference to an object.
 * A weak reference does not prevent its referenced object from being reclaimed by the garbage collector.
 * @extends BaseWeakRef<T>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class WeakRef<T extends Object> extends BaseWeakRef<T> {
    /**
     * Constructs a weak reference instance pointing to the given target object.
     * @param { T } target - The target object for the weak reference.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(target: T) {
        super(target);
    }
}
