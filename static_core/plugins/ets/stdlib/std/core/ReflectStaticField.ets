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
 * @file Defines the ReflectStaticField for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Provide a namespace for reflection related functionality.
 * @namespace reflect
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export namespace reflect {

/**
 * Represents static field of class
 * @extends Field
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class StaticField extends Field {
    private constructor() { throw new Error("StaticField constructor called") }

    /**
     * Determine whether the current Static Field object is equal to another object.
     *
     * @param { StaticField } other Another Static Field object to compare.
     * @returns { boolean } If two objects are equal, return true; otherwise, return false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public equals(other: StaticField): boolean {
        return this.fieldPtr == other.fieldPtr
    }

    /**
     * Reads value from an instance field.
     *
     * @returns { Any } field value.
     * @throws { OutofMemoryError } if there is no memory to allocate return value.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getValue(): Any {
        return super.getValue(undefined)
    }

    /**
     * Writes value into a static field.
     *
     * @param { Any } value value to write.
     * @throws { TypeError } if there is an incompatible value.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public setValue(value: Any): void {
        if (this.isReadonly()) {
            throw new TypeError("setValue to a readonly field")
        }
        super.setValue(undefined, value)
    }
}

} // namespace reflect
