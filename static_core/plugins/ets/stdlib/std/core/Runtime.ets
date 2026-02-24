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
 * @file Defines the Runtime for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Provides core functionality of ArkTS runtime.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class Runtime {

    /**
     * Returns a hash code for the Object.
     *
     * @param { Object } o The object to calculate hash code for
     * @returns { int } The hash code of the object
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native getHashCode(o: Object): int;

    /**
     * Returns a hash code for the Object. The hash code algorithm  depends
     *     on the actual type of the input parameter. Special algorithm is used for
     *     Byte, Short, Int, Long, Char, String. In other cases the result is same
     *     as getHashCode.
     *
     * @param { Object } o The object to calculate hash code for
     * @returns { long } The hash code of the object
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native getHashCodeByValue(o: Object): long;

    /**
     * Check if the current platform is little-endian.
     *
     * @returns { boolean } Return true if it's a little-endian platform, otherwise return false
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native static isLittleEndianPlatform(): boolean;

    // Deprecated.
    /**
     * Get type information of the specified object. This method is deprecated.
     *
     * @param { Any } o The object to get type information for
     * @returns { Class|undefined } The Class type of the object, return undefined if cannot be determined
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    native static __getTypeInfo(o: Any): Class|undefined;

    // Internal stubs to create a ClassCastError during failed cast events
    /**
     * Internal stub to create a ClassCastError when type conversion fails (excluding undefined case).
     *
     * @param { Any } source The source object being converted
     * @param { Class | undefined } target The target type
     * @returns { ClassCastError } A ClassCastError instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static native failedTypeCastExclUndefinedStub(source: Any, target: Class | undefined): ClassCastError
    /**
     * Internal stub to create a ClassCastError when type conversion fails (including undefined case).
     *
     * @param { Any } source The source object being converted
     * @param { Class | undefined } target The target type
     * @returns { ClassCastError } A ClassCastError instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static native failedTypeCastInclUndefinedStub(source: Any, target: Class | undefined): ClassCastError

    // Internal stub to create a FixedArray of the specified type and length
    private native static allocSameTypeArray<T extends FixedArray<Any>>(type: Class, length: int): T;

    // Internal stub to create a FixedArray with elements prepended in front of the original array
    /**
     * Internal stub to create a new FixedArray with specified elements prepended to the front of the original array.
     *
     * @param { FixedArray<Any> } original The original FixedArray
     * @param { FixedArray<Any> } front The FixedArray to prepend
     * @returns { FixedArray<Any> } A new FixedArray containing the prepended elements
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static addFrontElementsStub(original: FixedArray<Any>, front: FixedArray<Any>): FixedArray<Any> {
        let res = Runtime.allocSameTypeArray<FixedArray<Any>>(Class.of(original), original.length + front.length)
        for (let i: int = 0; i < front.length; ++i) {
            res[i] = front[i]
        }
        for (let i: int = 0; i < original.length; ++i) {
            res[i + front.length] = original[i]
        }
        return res
    }

    // Internal stub to create a FixedArray with several elements removed from front of the original array
    /**
     * Internal stub to create a new FixedArray with specified number of elements removed from
     * the front of the original array.
     *
     * @param { FixedArray<Any> } original The original FixedArray
     * @param { int } count The number of elements to remove from the front
     * @returns { FixedArray<Any> } A new FixedArray with the specified elements removed
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static removeFrontElementsStub(original: FixedArray<Any>, count: int): FixedArray<Any> {
        let res = Runtime.allocSameTypeArray<FixedArray<Any>>(Class.of(original), original.length - count)
        for (let i: int = count; i < original.length; ++i) {
            res[i - count] = original[i]
        }
        return res
    }

    // Internal stub to pad rest arguments array to handle possible argument underapplication
    /**
     * Internal stub to pad rest arguments array to handle possible argument underapplication.
     *
     * @param { FixedArray<Any> } rest The FixedArray of rest arguments
     * @param { int } required The required minimum number of arguments
     * @returns { FixedArray<Any> } A FixedArray with length at least required
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static restArgumentsUnderapplicationStub(rest: FixedArray<Any>, required: int): FixedArray<Any> {
        if (rest.length >= required) {
            return rest
        }
        let newRest : FixedArray<Any> = new FixedArray<Any>(required)
        for (let i: int = 0; i < rest.length; ++i) {
            newRest[i] = rest[i]
        }
        for (let i: int = rest.length; i < required; ++i) {
            newRest[i] = undefined
        }
        return newRest
    }

    // Internal stub to handle a generic case of unsafeCall for std.core.Lambda... helper classes
    /**
     * Internal stub to handle the generic case of unsafeCall for std.core.Lambda... helper classes.
     *
     * @param { Function } fn The function to call
     * @param { FixedArray<Any> } r The FixedArray containing call arguments
     * @returns { Any } The result of the function call
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static lambdaUnsafeCallStub(fn: Function, r: FixedArray<Any>): Any {
        return __lambdaUnsafeCallStub(fn, r)
    }
}

/**
 * Defines ErrorOptionsImpl class
 *
 * @implements { ErrorOptions }
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
class ErrorOptionsImpl implements ErrorOptions {
    /**
     * Specifies cause of an Error
     *
     * @type { Object | undefined }
     * @default undefined
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private cause_?: Object

    /**
     * Getter for Error cause
     *
     * @returns { Object | undefined } - A cause of the Error
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    override get cause(): Object | undefined {
        return this.cause_
    }

    /**
     * Setter for Error cause
     *
     * @param { Object | undefined } cause - A cause of the Error
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    set cause(cause: Object | undefined) {
        this.cause_ = cause
    }

    /**
     * Constructor for ErrorOptionsImpl
     *
     * @param { Object } [cause] - A cause of the Error
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(cause?: Object) {
        this.cause_ = cause
    }
}

// Deprecated. Narrow the value to type T or to default (object | null | undefined).
/**
 * Narrow the type of the value to generic type T. This function is deprecated.
 *
 * @param { Any } v The value to narrow the type for
 * @returns { T } The value with type T
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function __narrowAny<T = (object | null | undefined)>(v: Any): T {
    return v as T
}
