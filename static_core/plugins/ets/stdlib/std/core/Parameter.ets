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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * @file Defines the Parameter for ArkTS
 * @kit ArkTS
 */


package std.core;

/**
 * Represents a parameter of a function type.
 *
 * @extends Object
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class TypeAPIParameter extends Object {
    private paramType: Type | undefined // not undefined if already created
    private name: string = ""
    private attributes: int

    private constructor () {}

    /**
     * Gets the type of the parameter.
     *
     * @returns { Type } The type object of the parameter.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getType(): Type {
        return this.paramType!
    }

    /**
     * Gets the name of the parameter.
     *
     * @returns { string } The name string of the parameter.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getName(): string {
        return this.name
    }

    /**
     * Gets the attributes of the parameter.
     *
     * @returns { int } The attribute value of the parameter, which is a bit mask.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getAttributes(): int {
        return this.attributes
    }

    /**
     * Determines whether this parameter is a rest parameter.
     *
     * @returns { boolean } Returns true if it is a rest parameter, otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isRest(): boolean {
        return (this.attributes & Attributes.REST) != 0
    }

    /**
     * Determines whether this parameter is an optional parameter.
     *
     * @returns { boolean } Returns true if it is an optional parameter, otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isOptional(): boolean {
        return (this.attributes & Attributes.OPTIONAL) != 0
    }

    /**
     * Returns the string representation of the parameter in the format 'name: type'.
     *
     * @returns { string } The string representation of the parameter.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toString(): string {
        return this.getName() + ": " + this.getType().toString()
    }

    /**
     * Compares whether the current parameter object is equal to another object.
     *
     * @param { Any } oth Another object used for comparison.
     * @returns { boolean } Returns true if the objects are equal, otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public equals(oth: Any): boolean {
        return oth instanceof TypeAPIParameter &&
                this.paramType!.equals((oth as TypeAPIParameter).paramType!) &&
                this.name == (oth as TypeAPIParameter).name &&
                this.attributes == (oth as TypeAPIParameter).attributes
    }
}
