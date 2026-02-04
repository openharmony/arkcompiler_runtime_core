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
 * @file Defines the ReflectConstructor for ArkTS
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
 * Represents a class constructor.
 *
 * @extends Method
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class Constructor extends Method {
    /**
     * Private constructor, direct instantiation of the Constructor class is prohibited.
     * @throws { Error } Throws when attempting to directly instantiate the Constructor class.
     */
    private constructor() { throw new Error("Constructor constructor called") }

    /**
     * Compares whether the current constructor object is equal to another constructor object.
     *
     * @param { Constructor } other Another constructor object to compare with.
     * @returns { boolean } Returns true if the two constructor objects are equal, otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public equals(other: Constructor): boolean {
        return this.methodPtr == other.methodPtr
    }

    /**
     * Creates a new instance of its belonging class using this constructor.
     *
     * @param { FixedArray<Any> } [args] args The argument list for the constructor.
     * @returns { Any } The newly created class instance.
     * @throws { OutOfMemoryError } Throws when not enough memory to create the object.
     * @throws { TypeError } Throws when `thisObj` is incompatible with the method.
     * @throws { TypeError } Throws when the non-virtual instance method cannot be 
     *     retrieved under the class of the current instance.
     * @throws { TypeError } Throws when The number or type of `args` does not match the method signature.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public createInstance(args?: FixedArray<Any>): Any {
        return this.createInstanceInternal(args);
    }

    private native createInstanceInternal(args?: FixedArray<Any>): Any
}

} // namespace reflect
