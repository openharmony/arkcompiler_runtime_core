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
 * @file Defines the ReflectInstanceField for ArkTS
 * @kit ArkTS
 */


package std.core;

/**
 * Core namespace for reflection operations, providing access to metadata such as classes, interfaces,
 * methods, and properties at runtime.
 *
 * @namespace reflect
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export namespace reflect {

/**
 * Represents an instance field of a class or interface.
 *
 * @extends Field
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class InstanceField extends Field {
    /**
     * Private constructor, direct calls are prohibited.
     * @throws { Error } Direct calls to the InstanceField constructor are prohibited.
     */
    private constructor() { throw new Error("InstanceField constructor called") }

    /**
     * Checks if the current instance field is equal to the given instance field.
     *
     * @param { InstanceField } other Another InstanceField object to compare with.
     * @returns { boolean } Returns true if the two instance fields are equal, otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public equals(other: InstanceField): boolean {
        return this.fieldPtr == other.fieldPtr
    }

    /**
     * Reads the value from the instance field.
     *
     * @param { Object } thisObj The target object as the `this` context.
     * @returns { Any } Returns the value read from the instance field.
     * @throws { TypeError } Throws when `thisObj` is incompatible with the field.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getValue(thisObj: Object): Any {
        return super.getValueInternal(thisObj)
    }

    /**
     * Writes a value to the instance field.
     *
     * @param { Object } thisObj The target object as the `this` context.
     * @param { Any } value The value to write.
     * @throws { TypeError } Throws when `thisObj` is incompatible with the field or the value type doesn't match.
     * @throws { TypeError } Throws when the field is readonly.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public setValue(thisObj: Object, value: Any): void {
        if (this.isReadonly()) {
            throw new TypeError("setValue to a readonly field")
        }
        super.setValueInternal(thisObj, value)
    }
}

} // namespace reflect
