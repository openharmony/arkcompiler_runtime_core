/**
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
 * @file Defines the BaseEnum for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Base class for all enum types in ArkTS.
 *
 * @class BaseEnum
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class BaseEnum<T extends byte | short | int | long | float | double | string> {
    /**
     * constructor for BaseEnum
     *
     * @param { T } value The underlying value of the enum constant.
     * @param { string } name The name of the enum constant.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(value: T, name: string) {
        this.name = name
        this.value = value
    }

    /**
     * Returns the value of the enum constant.
     *
     * @returns { T } The value of the enum constant.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public valueOf(): T {
        return this.value
    }

    /**
     * Returns the name of the enum constant.
     *
     * @returns { string } The name of the enum constant.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getName(): string {
        return this.name
    }

    /**
     * Returns a string representation of the enum constant.
     *
     * @returns { string } A string representation of the enum constant.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toString(): string {
        return (this.value as Object).toString()
    }

    private value: T
    private name: string
}
