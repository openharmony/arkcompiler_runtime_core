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
 * @file Defines the CreateFromJSONValue for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Creates a Boolean instance based on JSONValue
 *
 * @param { Boolean } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Boolean } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function createFromJSONValue(this: Boolean, json: JSONValue): Boolean {
    if (json instanceof JSONFalse) {
        return Boolean.FALSE
    } else if (json instanceof JSONTrue) {
        return Boolean.TRUE
    }
    throw new JSONTypeError('Cannot create Boolean from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Byte instance based on JSONValue
 *
 * @param { Byte } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Byte } - byte value decoded from JSON
 * @throws { JSONTypeError } if json does not encode a valid byte
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function createFromJSONValue(this: Byte, json: JSONValue): Byte {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        if (Double.isInteger(num) && Byte.MIN_VALUE <= num && num <= Byte.MAX_VALUE) {
            return num.toByte()
        }
    }
    throw new JSONTypeError('Cannot create Byte from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Char instance based on JSONValue
 *
 * @param { Char } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Char } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function createFromJSONValue(this: Char, json: JSONValue): Char {
    if (json instanceof JSONString) {
        let str = (json as JSONString).value
        if (str.getLength() == 1) {
            return Char.valueOf(str.charAt(0))
        }
    }
    throw new JSONTypeError('Cannot create Char from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Short instance based on JSONValue
 *
 * @param { Short } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Short } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function createFromJSONValue(this: Short, json: JSONValue): Short {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        if (Double.isInteger(num) && Short.MIN_VALUE <= num && num <= Short.MAX_VALUE) {
            return Short.valueOf(num.toShort())
        }
    }
    throw new JSONTypeError('Cannot create Short from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Int instance based on JSONValue
 *
 * @param { Int } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Int } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function createFromJSONValue(this: Int, json: JSONValue): Int {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        if (Double.isInteger(num) && Int.MIN_VALUE <= num && num <= Int.MAX_VALUE) {
            return Int.valueOf(Double.toInt(num))
        }
    }
    throw new JSONTypeError('Cannot create Int from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Long instance based on JSONValue
 *
 * @param { Long } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Long } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function createFromJSONValue(this: Long, json: JSONValue): Long {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        if (Double.isInteger(num) && Long.MIN_VALUE <= num && num <= Long.MAX_VALUE) {
            return Long.valueOf(num.toLong())
        }
    }
    throw new JSONTypeError('Cannot create Long from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Float instance based on JSONValue
 *
 * @param { Float } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Float } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function createFromJSONValue(this: Float, json: JSONValue): Float {
    if (json instanceof JSONNumber) {
        let val = (json as JSONNumber).value
        if (-Float.MAX_VALUE <= val && val <= Float.MAX_VALUE) {
            return Float.valueOf(((json as JSONNumber).value).toFloat())
        }
    }
    throw new JSONTypeError('Cannot create Double from JSON', {cause: json as Object} as ErrorOptions)
}

/**
/**
 * Creates a Double instance based on JSONValue
 *
 * @param { Double } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Double } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function createFromJSONValue(this: Double, json: JSONValue): Double {
    if (json instanceof JSONNumber) {
        return Double.valueOf((json as JSONNumber).value)
    }
    throw new JSONTypeError('Cannot create Double from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a String instance based on JSONValue
 *
 * @param { String } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { String } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid String
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function createFromJSONValue(this: String, json: JSONValue): String {
    if (json instanceof JSONString) {
        return (json as JSONString).value
    }
    throw new JSONTypeError('Cannot create String from JSON', {cause: json as Object} as ErrorOptions)
}
