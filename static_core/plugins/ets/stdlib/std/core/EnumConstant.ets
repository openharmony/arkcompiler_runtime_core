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
 * @file Defines the EnumConstant for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * EnumConstant
 * 
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class EnumConstant {
    private name: string = ""
    private typ: Type | undefined
    private intVal: int
    private strVal: string = ""

    private constructor () {}

    /**
     * getType
     *
     * @returns { Type } get current object's type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getType(): Type {
        return this.typ!
    }

    /**
     * getName
     *
     * @returns { string } get current object's name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getName(): string {
        return this.name
    }

    /**
     * getValue
     *
     * @returns { int } get current object's value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getValue(): int {
        return this.intVal
    }

    /**
     * getStringValue
     *
     * @returns { string } get current object's stringvalue
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getStringValue(): string {
        return this.strVal
    }

    /**
     * hasValue
     *
     * @returns { boolean } get current object's hasValue
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public hasValue(): boolean {
        // TODO(shumilov-petr): not implemented
        return false
    }

    /**
     * hasStringValue
     *
     * @returns { boolean } get current object's hasStringValue
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public hasStringValue(): boolean {
        // TODO(shumilov-petr): not implemented
        return false
    }

    /**
     * create
     *
     * @param { string } name None
     * @param { Type } typ None
     * @param { int } v None
     * @returns { EnumConstant }
     * @throws { Error } - Input parameter error.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static create(name: string, typ: Type, v: int): EnumConstant {
        // TODO(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    /**
     * create
     *
     * @param { string } name None
     * @param { Type } typ None
     * @param { string } v None
     * @returns { EnumConstant }
     * @throws { Error } - Input parameter error.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static create(name: string, typ: Type, v: string): EnumConstant {
        // TODO(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    /**
     * toString
     *
     * @returns { string }
     * @throws { Error } - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toString(): string {
        // TODO(shumilov-petr): not implemented
        throw new Error("not implemented")
    }
}
