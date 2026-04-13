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

import * as arkruntime from "arkruntime"

/**
 * Provides core functionality of ArkTS runtime.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
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
     */
    public static native getHashCodeByValue(o: Any): long;

    /**
     * Check if the current platform is little-endian.
     *
     * @returns { boolean } Return true if it's a little-endian platform, otherwise return false
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public native static isLittleEndianPlatform(): boolean;

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
     */
    static lambdaUnsafeCallStub(fn: Function, r: FixedArray<Any>): Any {
        return __lambdaUnsafeCallStub(fn, r)
    }

    // Internal stub to convert any to string
    private static native anyToString(a: Any): String;

    /**
    * Converts Any object to a string
    *
    * @param object to conversion to a string
    *
    * @returns result of the conversion
    */
    static toStringStub(a: Any): String {
        if (a instanceof Object) {
            return a.toString();
        }
        if (a === undefined) {
            return "undefined";
        }
        if (a === null) {
            return "null";
        }
        return Runtime.anyToString(a);
    }

}

class ErrorOptionsImpl implements ErrorOptions {

    private cause_?: Object

    override get cause(): Object | undefined {
        return this.cause_
    }

    set cause(cause: Object | undefined) {
        this.cause_ = cause
    }

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
 */
export function __narrowAny<T = (object | null | undefined)>(v: Any): T {
    return v as T
}
