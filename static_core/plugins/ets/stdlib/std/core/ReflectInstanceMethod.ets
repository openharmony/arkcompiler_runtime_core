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
 * @file Defines the ReflectInstanceMethod for ArkTS
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
 * Represents an instance method of a class or interface.
 *
 * @extends Method
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class InstanceMethod extends Method {
    /**
     * Private constructor, direct calls are prohibited.
     * @throws { Error } Throws when attempting to call this constructor.
     */
    private constructor() { throw new Error("InstanceMethod constructor called") }

    /**
     * Checks if the method is a final method.
     *
     * @returns { boolean } Returns true if the method is final, otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isFinal(): boolean {
        return (this.attributes & Attributes.FINAL) != 0
    }

    /**
     * Checks if the method is an async method.
     *
     * @returns { boolean } Returns true if the method is async, otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isAsync(): boolean {
        return (this.attributes & Attributes.ASYNC) != 0
    }

    /**
     * Checks if the method is an abstract method.
     *
     * @returns { boolean } Returns true if the method is abstract, otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isAbstract(): boolean {
        return (this.attributes & Attributes.ABSTRACT) != 0
    }

    /**
     * Checks if the method is a getter method.
     *
     * @returns { boolean } Returns true if the method is a getter, otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isGetter(): boolean {
        return (this.attributes & Attributes.GETTER) != 0
    }

    /**
     * Checks if the method is a setter method.
     *
     * @returns { boolean } Returns true if the method is a setter, otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isSetter(): boolean {
        return (this.attributes & Attributes.SETTER) != 0
    }

    /**
     * Compares whether two instance methods are equal.
     *
     * @param { InstanceMethod } other Another instance method object used for comparison.
     * @returns { boolean } Returns true if the two methods are equal, otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public equals(other: InstanceMethod): boolean {
        return this.methodPtr == other.methodPtr
    }

    /**
     * Invokes this instance method.
     *
     * @param { Object } thisObj The this object used when invoking the method.
     * @param { FixedArray<Any> } [ args ] args The argument array passed when invoking the method.
     * @returns { Any } The execution result of the method.
     * @throws { TypeError } Throws when `thisObj` is incompatible with the method.
     * @throws { TypeError } Throws when the non-virtual instance method cannot be 
     *     retrieved under the class of the current instance.
     * @throws { TypeError } Throws when The number or type of `args` does not match the method signature.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public invoke(thisObj: Object, args?: FixedArray<Any>): Any {
        return super.invoke(thisObj, args)
    }
}

} // namespace reflect
