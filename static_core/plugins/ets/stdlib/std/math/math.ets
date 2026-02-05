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
 * @file Defines the math for ArkTS
 * @kit ArkTS
 */
package std.math;

import {PI, E, LN10, LN2, LOG2E, SQRT1_2, SQRT2} from "std/math/consts";

/**
 * Absolute value
 * @param { byte } v - Some byte value
 * @returns { byte } Absolute value of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function abs(v: byte): byte {
    return abs(v.toInt()).toByte()
}

/**
 * Absolute value
 * @param { short } v - Some short value
 * @returns { short } Absolute value of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function abs(v: short): short {
    return abs(v.toInt()).toShort()
}

/**
 * Absolute value
 * @param { int } v - Some int value
 * @returns { int } Absolute value of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function abs(v: int): int;

/**
 * Absolute value
 * @param { long } v - Some long value
 * @returns { long } Absolute value of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function abs(v: long): long;

/**
 * Absolute value
 * @param { float } v - Some float value
 * @returns { float } Absolute value of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function abs(v: float): float;

/**
 * Absolute value
 *
 * @param { double } v - Some double value
 * @returns { double } Absolute value of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * abs(doubleNaN) = nan
 * abs(doubleInf) = inf
 * abs(doubleNegInf) = inf
 */
export native function abs(v: double): double;

/**
 * Hyperbolic arccosine of an angle `v`
 * @param { float } v - angle in radians
 * @returns { float } Hyperbolic arccosine of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function acosh(v: float): float;

/**
 * Hyperbolic arccosine of an angle `v`
 * @param { double } v - angle in radians
 * @returns { double } Hyperbolic arccosine of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * acosh(doubleNaN) = -nan
 * acosh(doubleInf) = inf
 * acosh(doubleNegInf) = -nan
 */
export native function acosh(v: double): double;

/**
 * Arccosine of angle `v`
 * @param { float } v - angle in radians
 * @returns { float } Arccosine of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * acos(floatNaN) = -nan
 * acos(floatInf) = nan
 * acos(floatNegInf) = nan
 */
export native function acos(v: float): float;

/**
 * Arccosine of angle `v`
 * @param { double } v - angle in radians
 * @returns { double } Arccosine of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * acos(doubleNaN) = -nan
 * acos(doubleInf) = nan
 * acos(doubleNegInf) = nan
 */
export native function acos(v: double): double;

/**
 * Hyperbolic arcsine of angle `v`
 * @param { float } v - angle in radians
 * @returns { float } Hyperbolic arcsine of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function asinh(v: float): float;

/**
 * Hyperbolic arcsine of angle `v`
 * @param { double } v - angle in radians
 * @returns { double } Hyperbolic arcsine of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * asinh(doubleNaN) = -nan
 * asinh(doubleInf) = inf
 * asinh(doubleNegInf) = -inf
 */
export native function asinh(v: double): double;

/**
 * Arcsine of angle `v`
 * @param { float } v - angle in radians
 * @returns { float } Arcsine of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function asin(v: float): float;

/**
 * Arcsine of angle `v`
 * @param { double } v - angle in radians
 * @returns { double } Arcsine of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * asin(doubleInf) = nan
 * asin(doubleNegInf) = nan
 * asin(doubleNaN) = nan
 */
export native function asin(v: double): double;

/**
 * The atan2() method measures the counterclockwise angle θ, in radians, 
 * between the positive x-axis and the point (x, y).
 * @param { float } y - The y-coordinate of the point
 * @param { float } x - The x-coordinate of the point
 * @returns { float } The angle in radians (between -π and π, inclusive) between the positive x-axis
 *     and the ray from (0, 0) to the point (x, y).
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function atan2(y: float, x: float): float;

/**
 * The atan2() method measures the counterclockwise angle θ, in radians, 
 * between the positive x-axis and the point (x, y).
 * @param { double } y - The y-coordinate of the point
 * @param { double } x - The x-coordinate of the point
 * @returns { double } The angle in radians (between -π and π, inclusive) between the positive x-axis
 *     and the ray from (0, 0) to the point (x, y).
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * atan2(doubleInf, doubleInf)= 0.7853981633974483
 * atan2(doubleNegInf, doubleNegInf)= -0.7853981633974483
 * atan2(doubleNaN, doubleNaN)= -nan
 */
export native function atan2(y: double, x: double): double;

/**
 * Hyperbolic arctangent of angle `v`
 * @param { float } v - angle in radians
 * @returns { float } Hyperbolic arctangent of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function atanh(v: float): float;

/**
 * Hyperbolic arctangent of angle `v`
 * @param { double } v - angle in radians
 * @returns { double } Hyperbolic arctangent of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * atanh(doubleNaN) = -nan
 * atanh(doubleInf) = -nan
 * atanh(doubleNegInf) = -nan
 */
export native function atanh(v: double): double;

/**
 * Arctangent of angle `v`
 * @param { float } v - angle in radians
 * @returns { float } Arctangent of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function atan(v: float): float;

/**
 * Arctangent of angle `v` with double
 * @param { double } v - angle in radians
 * @returns { double } Arctangent of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * atan(doubleNaN) = -nan
 * atan(doubleInf) = 1.5708
 * atan(doubleNegInf) = -1.5708
 */
export native function atan(v: double): double;

/**
 * The cbrt() function returns the cube root of a number.
 * @param { float } v - arbitrary number.
 * @returns { float } Returns the cube root of number.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function cbrt(v: float): float;

/**
 * The cbrt() function returns the cube root of a number.
 * @param { double } v - arbitrary number.
 * @returns { double } Returns the cube root of number.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * cbrt(doubleNaN) = nan
 * cbrt(doubleInf) = inf
 * cbrt(doubleNegInf) = -inf
 * cbrt(-1) = -1
 */
export native function cbrt(v: double): double;

/**
 * Smallest integer greater or equal to `v`
 * @param { double } v - arbitrary number
 * @returns { double } Smallest integer greater or equal to v
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * ceil(doubleNaN) = -nan
 * ceil(doubleInf) = inf
 * ceil(doubleNegInf) = -inf
 */
export native function ceil(v: double): double;

/**
 * Leading zero bits count in 64-bit representation of `v`
 * @param { long } v - 64-bit integer
 * @returns { int } Number of leading zero bits count in bit representation of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * clz64(0xFFFFFFFFFFFFFFFF) == 0
 * clz64(0x0000FFFFFFFFFFFF) == 16
 * clz64(0x0) == 64
 */
export native function clz64(v: long): int;

/**
 * Leading zero bits count in 32-bit representation of `v`
 * @param { int } v - 32-bit integer
 * @returns { int } Number of leading zero bits count in bit representation of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function clz64(v: int) {
    let extendedNum: long = v.toLong();
    let mask: long = (1 as long << 32) - 1;
    return clz64(extendedNum & mask);
}

/**
 * Leading zero bits count in 16-bit representation of `v`
 * @param { short } v - 16-bit integer
 * @returns { int } Number of leading zero bits count in bit representation of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function clz64(v: short) {
    let extendedNum: long = v.toLong();
    let mask: long = (1 as long << 16) - 1;
    return clz64(extendedNum & mask);
}

/**
 * Leading zero bits count in 8-bit representation of `v`
 * @param { byte } v - 8-bit integer
 * @returns { int } Number of leading zero bits count in bit representation of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function clz64(v: byte) {
    let extendedNum: long = v.toLong();
    let mask: long = (1 as long << 8) - 1;
    return clz64(extendedNum & mask);
}

/**
 * Leading zero bits count in 16-bit representation of `v`
 * @param { char } v - character value
 * @returns { int } Number of leading zero bits count in bit representation of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function clz64(v: char) {
    let extendedNum: long = v.toLong();
    let mask: long = (1 as long << 16) - 1;
    return clz64(extendedNum & mask);
}

/**
 * Leading zero bits count in 32-bit representation of `v`
 * @param { int } v - 32-bit integer
 * @returns { int } Number of leading zero bits count in bit representation of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * clz32(0xFFFFFFFF) == 0
 * clz32(0x0000FFFF) == 16
 * clz32(0x0) == 32
 */
export native function clz32(v: int): int;

/**
 * Leading zero bits count in 32-bit representation of `v`
 * @param { double } v - 64-bit double
 * @returns { double } Number of leading zero bits count in bit representation of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function clz32Double(v: double): double;

/**
 * Hyperbolic cosine of an angle `v`
 * @param { float } v - angle in radians
 * @returns { float } Hyperbolic cosine of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function cosh(v: float): float;

/**
 * Hyperbolic cosine of an angle `v`
 * @param { double } v - angle in radians
 * @returns { double } Hyperbolic cosine of angle `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * cosh(doubleNaN) = -nan
 * cosh(doubleInf) = inf
 * cosh(doubleNegInf) = inf
 */
export native function cosh(v: double): double;

/**
 * Cosine of `v`
 * @param { double } v - Angle in radians
 * @returns { double } Cosine of angle in radians
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * cos(doubleNaN) = -nan
 * cos(doubleInf) = -nan
 * cos(doubleNegInf) = -nan
 */
export native function cos(v: double): double;

/**
 * 2 raised to power `v`
 * @param { double } v - power value.
 * @returns { double } the value of a 2 raised to a power of `v`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * power2(doubleInf) = inf
 * power2(doubleNegInf) = 0
 * power2(doubleNaN) = nan
 */
export function power2(v: double): double {
    return power(2, v);
}

/**
 * (e raised to power `v`) - 1
 * @param { float } v - power value
 * @returns {float} raised to the power of a number v, subtracted by 1.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function expm1(v: float): float;

/**
 * (e raised to power `v`) - 1
 * @param { double } v - power value
 * @returns {double} raised to the power of a number v, subtracted by 1.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * expm1(doubleNaN) = -nan
 * expm1(doubleInf) = inf
 * expm1(doubleNegInf) = -1
 */
export native function expm1(v: double): double;

/**
 * e raised to power `v`
 * @param { float } v - power value
 * @returns {float} raised to the power of a number `v`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function exp(v: float): float;

/**
 * e raised to power `v`
 * @param { double } v - power value
 * @returns {double} raised to the power of a number `v`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * exp(doubleNaN) = -nan
 * exp(doubleInf) = inf
 * exp(doubleNegInf) = 0
 */
export native function exp(v: double): double;

/**
 * Largest integer less or equal to `v`
 * @param { double } v - arbitrary number
 * @returns { double } Largest integer less or equal to `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * floor(doubleNaN) = -nan
 * floor(doubleInf) = inf
 * floor(doubleNegInf) = -inf
 */
export native function floor(v: double): double;

/**
 * Square root of the sum of squares of `v` and `u`
 * @param { float } u - arbitrary number
 * @param { float } v - arbitrary number
 * @returns { float } The square root of the sum of squares of its arguments
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function hypot(u: float, v: float): float;

/**
 * Square root of the sum of squares of `v` and `u`
 * @param { double } u - arbitrary number
 * @param { double } v - arbitrary number
 * @returns { double } The square root of the sum of squares of its arguments
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * hypot(doubleInf) = inf
 * hypot(doubleNegInf) = inf
 * hypot(doubleNaN) = nan
 */
export native function hypot(u: double, v: double): double;

/**
 * Base 10 logarithm of `v`
 * @param { float } v - arbitrary number
 * @returns { float } Returns base 10 logarithm of 'v'
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function log10(v: float): float;

/**
 * Base 10 logarithm of `v`
 * @param { double } v - arbitrary number
 * @returns { double } Returns base 10 logarithm of 'v'
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * log10(doubleNaN) = -nan
 * log10(doubleInf) = inf
 * log10(doubleNegInf) = -nan
 */
export native function log10(v: double): double;

/**
 * Natural logarithm of (1 + `v`)
 * @param { float } v - A numeric expression.
 * @returns { float } Returns the natural logarithm of '1 + v'.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function log1p(v: float): float {
    return log(1.0f + v);
}

/**
 * Natural logarithm of (1 + `v`)
 * @param { double } v - A numeric expression.
 * @returns { double } Returns the natural logarithm of '1 + v'.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * log1p(doubleNaN) = nan
 * log1p(doubleInf) = inf
 * log1p(doubleNegInf) = nan
 */
export function log1p(v: double): double {
    return log((1.0).toDouble() + v);
}

/**
 * Base 2 logarithm of `v`
 * @param { float } v - arbitrary number
 * @returns { float } Returns base 2 logarithm of 'v'
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function log2(v: float): float {
    return log(v) / log(2.0f);
}

/**
 * Base 2 logarithm of `v`
 * @param { double } v - arbitrary number
 * @returns { double } Returns base 2 logarithm of 'v'
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * log2(doubleNaN) = -nan
 * log2(doubleInf) = inf
 * log2(doubleNegInf) = -nan
 */
export function log2(v: double): double {
    return log(v) / log((2.0).toDouble());
}

/**
 * Natural logarithm of `v`
 * @param { float } v - arbitrary number
 * @returns { float } Returns the natural logarithm of 'v'
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function log(v: float): float;

/**
 * Natural logarithm of `v`
 * @param { double } v - arbitrary number
 * @returns { double } Returns the natural logarithm of 'v'
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * log(doubleNaN) = -nan
 * log(doubleInf) = inf
 * log(doubleNegInf) = -nan
 */
export native function log(v: double): double;

/**
 * Largest value of `u` and `v`
 * @param { byte } v - arbitrary number
 * @param { byte } u - arbitrary number
 * @returns { byte } Largest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function max(v: byte, u: byte): byte {
    if (v > u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Largest value of `u` and `v`
 * @param { char } v - arbitrary number
 * @param { char } u - arbitrary number
 * @returns { char } Largest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function max(v: char, u: char): char {
    if (v.compareTo(u) > 0) {
        return v;
    } else {
        return u;
    }
}

/**
 * Largest value of `u` and `v`
 * @param { short } v - arbitrary number
 * @param { short } u - arbitrary number
 * @returns { short } Largest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function max(v: short, u: short): short {
    if (v > u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Largest value of `u` and `v`
 * @param { int } v - arbitrary number
 * @param { int } u - arbitrary number
 * @returns { int } Largest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function max(v: int, u: int): int {
    if (v > u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Largest value of `u` and `v`
 * @param { long } v - arbitrary number
 * @param { long } u - arbitrary number
 * @returns { long } Largest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function max(v: long, u: long): long {
    if (v > u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Largest value of `u` and `v`
 * @param { float } v - arbitrary number
 * @param { float } u - arbitrary number
 * @returns { float } Largest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function max(v: float, u: float): float {
    if (v > u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Largest value of `u` and `v`
 * @param { double } v - arbitrary number
 * @param { double } u - arbitrary number
 * @returns { double } Largest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function max(v: double, u: double): double {
    if (v > u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Smallest value of `u` and `v`
 * @param { byte } v - arbitrary number
 * @param { byte } u - arbitrary number
 * @returns { byte } Smallest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function min(v: byte, u: byte): byte {
    if (v < u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Smallest value of `u` and `v`
 * @param { char } v - arbitrary number
 * @param { char } u - arbitrary number
 * @returns { char } Smallest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function min(v: char, u: char): char {
    if (v.compareTo(u) < 0) {
        return v;
    } else {
        return u;
    }
}

/**
 * Smallest value of `u` and `v`
 * @param { short } v - arbitrary number
 * @param { short } u - arbitrary number
 * @returns { short } Smallest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function min(v: short, u: short): short {
    if (v < u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Smallest value of `u` and `v`
 * @param { int } v - arbitrary number
 * @param { int } u - arbitrary number
 * @returns { int } Smallest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function min(v: int, u: int): int {
    if (v < u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Smallest value of `u` and `v`
 * @param { long } v - arbitrary number
 * @param { long } u - arbitrary number
 * @returns { long } Smallest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function min(v: long, u: long): long {
    if (v < u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Smallest value of `u` and `v`
 * @param { float } v - arbitrary number
 * @param { float } u - arbitrary number
 * @returns { float } Smallest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function min(v: float, u: float): float {
    if (v < u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Smallest value of `u` and `v`
 * @param { double } v - arbitrary number
 * @param { double } u - arbitrary number
 * @returns { double } Smallest value of `u` and `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function min(v: double, u: double): double {
    if (v < u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Remainder of `v / u`, reunded towards zero
 * @param { double } u - dividend, which is the number to be divided.
 * @param { double } v - divisor, which is the number by which the dividend is divided.
 * @returns { double } The remainder of the division of `v` by `u`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * mod(doubleInf, doubleInf) = nan
 * mod(doubleNegInf, doubleNegInf) = nan
 * mod(doubleNaN, doubleNaN) = nan
 */
export native function mod(u: double, v: double): double;

/**
 * `u` raised to power of `v`
 * @param { double } v - base value
 * @param { double } u - power value
 * @returns { double } The value of a base `u` raised to a power `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * power(doubleNaN, doubleInf) = -nan
 * power(doubleInf, doubleInf) = inf
 *
 */
export native function power(v: double, u: double): double;

/**
 * Remainder of `v / u`, reunded to nearest
 *
 * @param { double } v - base value
 * @param { double } u - power value
 * @returns { double } The remainder of `v / u`, reunded to nearest
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function rem(v: double, u: double): double;

/**
 * `v` rounded to nearest integer
 * @param { double } v - power value
 * @returns { double } rounded to nearest integer
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * round(doubleNaN) = -nan
 * round(doubleInf) = inf
 * round(doubleNegInf) = -inf
 */
export native function round(v: double): double;

/**
 * Multiplies `v` by 2 raised to the power of `n` (`v * 2 ** n`).
 * @param { double } v - The base value to be scaled.
 * @param { int } n - The exponent value, indicating the power of 2 to scale `v` by.
 * @returns { double } The result of `v * 2 ** n`, computed as a floating-point number.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * scalbn(doubleNaN, 5) = nan
 * scalbn(doubleInf, 10) = inf
 * scalbn(doubleNegInf, -5) = -inf
 */
export function scalbn(v: double, n: int): double {
    return v * power(2, n);
}

/**
 * Returns true if the given double-precision floating-point number is negative.
 * @param { double } v - arbitrary number
 * @returns { boolean } true if arg is negative, false otherwise
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * signbit(doubleNaN) = true
 * signbit(doubleInf) = false
 * signbit(doubleNegInf) = true
 */
export native function signbit(v: double): boolean;

/**
 * Returns an integer indicating the sign of the given double-precision floating-point.
 * @param { double } v - arbitrary number
 * @returns { int } sign of `v`
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * sign(0) = 0
 * sign(doubleInf) = 1
 * sign(doubleNegInf) = -1
 * sign(doubleNaN) = nan
 */
export function sign(v: double): int {
    if(v < 0){
        return -1;
    }
    if(v > 0){
        return 1;
    }
    if(abs(v) < 1e-13){
        return 0;
    }
    throw new Error("NaN passed to sign");
}

/**
 * Hyperbolic sine of a number
 * @param { float } v - A numeric expression that contains an angle measured in radians.
 * @returns { float } Returns the hyperbolic sine of a number.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function sinh(v: float): float;

/**
 * Hyperbolic sine of a number
 * @param { double } v - A numeric expression that contains an angle measured in radians.
 * @returns { double } Returns the hyperbolic sine of a number.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * sinh(doubleInf) = inf
 * sinh(doubleNegInf) = -inf
 * sinh(doubleNaN) = nan
 */
export native function sinh(v: double): double;

/**
 * Sine of `v`
 * @param { double } v - Angle in radians
 * @returns { double } Sine of angle in radians
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * sin(doubleNaN) = -nan
 * sin(doubleInf) = -nan
 * sin(doubleNegInf) = -nan
 */
export native function sin(v: double): double;

/**
 * Square root of `v`
 * @param { double } v - Number greater or equal to zero.
 * @returns { double } The square root of `v`, a nonnegative number.
 *     If x is less than zero, returns NaN.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * sqrt(doubleNaN) = nan
 * sqrt(doubleInf) = inf
 * sqrt(doubleNegInf) = nan
 * sqrt(-1.0) = nan
 */
export native function sqrt(v: double): double;

/**
 * Hyperbolic tangent of a number.
 * @param { float } v - A numeric expression that contains an angle measured in radians.
 * @returns { float } Returns the hyperbolic tangent of a number.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function tanh(v: float): float;

/**
 * Hyperbolic tangent of a number.
 * @param { double } v - A numeric expression that contains an angle measured in radians.
 * @returns { double } Returns the hyperbolic tangent of a number.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * tanh(doubleInf) = 1.0
 * tanh(doubleNegInf) = -1.0
 * tanh(doubleNaN) = nan
 */
export native function tanh(v: double): double;

/**
 * Tangent of a number
 * @param { float } v - A numeric expression that contains an angle measured in radians.
 * @returns { float } Returns the tangent of a number.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function tan(v: float): float;

/**
 * Tangent of a number
 * @param { double } v - A numeric expression that contains an angle measured in radians.
 * @returns { double } Returns the tangent of a number.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * tan(doubleInf) = nan
 * tan(doubleNegInf) = nan
 * tan(doubleNaN) = nan
 */
export native function tan(v: double): double;

/**
 * Integer part of `v`
 * @param { double } v - Number to be truncated.
 * @returns { double } The integer part of a number by removing any fractional digits.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function trunc(v: double): double;

/**
 * Sets the seed for the pseudo-random number generator
 * @param { long } value - The seed value for the random number generator
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function seedRandom(value: long): void {}

/**
 * Pseudo-random number in the range [0.0, 1.0)
 * @returns { double } A floating-point, pseudo-random number that's greater than or equal to 0 and less than 1,
 *     with approximately uniform distribution over that range — which you can then scale to your desired range.
 *     Initial seed to the random number generator algorithm can be given using {@link seedRandom()} function.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function random(): double;

/**
 * C-like 32-bit manipulation of the two numbers
 *  a cast to i32
 *  b cast to i32
 *  return (a * b) % 2 ** 32
 * @param { double } a - First number
 * @param { double } b - Second number
 * @returns { int } Returns the result of 32-bit multiplication of two numbers.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 * imul(doubleInf, 1) = 0
 * imul(doubleNegInf, 1) = 0
 * imul(doubleNaN, 1) = 0
 * imul(2.5, 2.5) = 4
 * imul(-5, 5.05) = 25
 */
export native function imul(a: double, b: double): int;

/**
 * "fround" returns nearest 32-bit single fp representation of a number
 *  in a 64-bit form
 * @param { double } doubleVal - The double value to convert to 32-bit float representation
 * @returns { double } The nearest 32-bit single precision float representation
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 * @example
 * ```
 *  Math.fround(1.337) == 1.337 // false, result would be 1.3370000123977661
 *  Math.fround(1.5) == 1.5 // true
 *  Math.fround(-5.05) == -5.05 //false, result would be -5.050000190734863
 *  Math.fround(Infinity) // Infinity
 *  Math.fround(NaN) // NaN
 */
export native function fround(doubleVal: double): double;
