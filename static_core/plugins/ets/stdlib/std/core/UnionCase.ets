/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * @file Defines the UnionCase for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Represents a case in a union type, including a name, type, and value.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class UnionCase {
    private name: string = ""
    private typ: Type | undefined // not undefined if already created
    private val: Object = new Object()

    private constructor () {}

    /**
     * Check if the current UnionCase instance represents a type.
     * @returns { boolean } Returns true if it represents a type; otherwise, returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isType(): boolean {
        return false
    }

    /**
     * Get the type of the UnionCase instance.
     * @returns { Type } Return the type of the UnionCase instance.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getType(): Type {
        return this.typ!
    }

    /**
     * Convert the UnionCase instance to its string representation.
     * @returns { string } Returns the string representation of the UnionCase instance.
     * @throws { Error } The toString method is not implemented.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toString(): string {
        // TODO(shumilov-petr): not implemented
        throw new Error("not implemented")
    }
}
