/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

package std.math;

import {PI, E, LN10, LN2, LOG2E, SQRT1_2, SQRT2} from "std/math/consts";

/**
 * Absolute value
 *
 * @param v Some double value
 *
 * @returns Absolute value of `v`
 *
 * @remark
 * Implemented as native function,  @see `abs()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L76).
 *
 * @example
 * ```
 * abs(doubleNaN) = nan
 * abs(doubleInf) = inf
 * abs(doubleNegInf) = inf
 */
export native function abs(v: double): double;

/**
 * Hyperbolic arccosine of an angle `v`
 *
 * @param v angle in radians
 *
 * @returns Hyperbolic arccosine of angle `v`
 *
 * @remark
 * Implemented as native function,  @see `acosh()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 *
 * @example
 * ```
 * acosh(doubleNaN) = -nan
 * acosh(doubleInf) = inf
 * acosh(doubleNegInf) = -nan
 */
export native function acosh(v: double): double;

/**
 * Arccosine of angle `v`
 *
 * @param v angle in radians
 *
 * @returns Arccosine of angle `v`
 *
 * @remark
 * Implemented as native function,  @see `acos()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 *
 * @example
 * ```
 * acos(doubleNaN) = -nan
 * acos(doubleInf) = nan
 * acos(doubleNegInf) = nan
 */
export native function acos(v: double): double;

/**
 * Hyperbolic arcsine of angle `v`
 *
 * @param v angle in radians
 *
 * @returns Hyperbolic arcsine of angle `v`
 *
 * @remark
 * Implemented as native function,  @see `asinh()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 *
 * @example
 * ```
 * asinh(doubleNaN) = -nan
 * asinh(doubleInf) = inf
 * asinh(doubleNegInf) = -inf
 */
export native function asinh(v: double): double;

/**
 * Arcsine of angle `v`
 *
 * @param v angle in radians
 *
 * @returns Arcsine of angle `v`
 */
export native function asin(v: double): double;
/**
 *
 * @returns The angle in radians (between -π and π, inclusive) between the positive x-axis and the ray from (0, 0) to the point (x, y).
 *
 * @remark
 * The atan2() method measures the counterclockwise angle θ, in radians, between the positive x-axis and the point (x, y).
 * Note that the arguments to this function pass the y-coordinate first and the x-coordinate second.
 */
export native function atan2(y: double, x: double): double;

/**
 * Hyperbolic arctangent of angle `v`
 *
 * @param v angle in radians
 *
 * @returns Hyperbolic arctangent of angle `v`
 *
 * @remark
 * Implemented as native function,  @see `atanh()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 *
 * @example
 * ```
 * atanh(doubleNaN) = -nan
 * atanh(doubleInf) = -nan
 * atanh(doubleNegInf) = -nan
 */
export native function atanh(v: double): double;

/**
 * Arctangent of angle `v`
 *
 * @param v angle in radians
 *
 * @returns Arctangent of angle `v`
 *
 * @remark
 * Implemented as native function,  @see `atan()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 *
 * @example
 * ```
 * atan(doubleNaN) = -nan
 * atan(doubleInf) = 1.5708
 * atan(doubleNegInf) = -1.5708
 */
export native function atan(v: double): double;

/**
 * The cbrt() function returns the cube root of a number.
 *
 * @param v arbitrary number
 *
 * @remark
 * 𝚌uberoot(𝚡) = ∛x = the unique y such that y³ = x.
*/
export native function cbrt(v: double): double;

/**
 * Smallest integer greater or equal to `v`
 *
 * @param v arbitrary number
 *
 * @returns Smallest integer greater or equal to v
 *
 * @remark
 * Implemented as native function,  @see `ceil()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 *
 * @example
 * ```
 * ceil(doubleNaN) = -nan
 * ceil(doubleInf) = inf
 * ceil(doubleNegInf) = -inf
 */
export native function ceil(v: double): double;

/**
 * Leading zero bits count in 64-bit representation of `v`
 *
 * @param v 64-bit integer
 *
 * @returns Number of leading zero bits count in bit representation of `v`
 *
 * @example
 * ```
 * clz64(0xFFFFFFFFFFFFFFFF) == 0
 * clz64(0x0000FFFFFFFFFFFF) == 16
 * clz64(0x0) == 64
 */
export native function clz64(v: long): int;

export function clz64(v: int) {
    let extendedNum: long = v.toLong();
    let mask: long = (1 as long << 32) - 1;
    return clz64(extendedNum & mask);
}

export function clz64(v: short) {
    let extendedNum: long = v.toLong();
    let mask: long = (1 as long << 16) - 1;
    return clz64(extendedNum & mask);
}

export function clz64(v: byte) {
    let extendedNum: long = v.toLong();
    let mask: long = (1 as long << 8) - 1;
    return clz64(extendedNum & mask);
}

export function clz64(v: char) {
    let extendedNum: long = v.toLong();
    let mask: long = (1 as long << 16) - 1;
    return clz64(extendedNum & mask);
}

/**
 * Leading zero bits count in 32-bit representation of `v`
 *
 * @param v 32-bit integer
 *
 * @returns Number of leading zero bits count in bit representation of `v`
 *
 * @example
 * ```
 * clz32(0xFFFFFFFF) == 0
 * clz32(0x0000FFFF) == 16
 * clz32(0x0) == 32
 */


export native function clz32(v: int): int;

/**
 * Leading zero bits count in 32-bit representation of `v`
 *
 * @param v 64-bit double
 *
 * @returns Number of leading zero bits count in bit representation of `v`
 *
 * @example
 * ```
 * clz32(0xFFFFFFFF) == 0
 * clz32(0x0000FFFF) == 16
 * clz32(0x0) == 32
 */


export native function clz32Double(v: double): double;

/**
 * Hyperbolic cosine of an angle `v`
 *
 * @param v angle in radians
 *
 * @returns Hyperbolic cosine of angle `v`
 *
 * @remark
 * Implemented as native function,  @see `cosh()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 *
 * @example
 * ```
 * cosh(doubleNaN) = -nan
 * cosh(doubleInf) = inf
 * cosh(doubleNegInf) = inf
 */
export native function cosh(v: double): double;

/**
 * Cosine of `v`
 *
 * @param v Angle in radians
 *
 * @returns Cosine of angle in radians
 *
 * @remark
 * Implemented as native function,  @see `cos()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L43).
 *
 * @example
 * ```
 * cos(doubleNaN) = -nan
 * cos(doubleInf) = -nan
 * cos(doubleNegInf) = -nan
 */
export native function cos(v: double): double;

/**
 * 2 raised to power `v`
 *
 * @param v power value
 *
 * @returns the value of a 2 raised to a power of `v`
 */
export function power2(v: double): double {
    return power(2, v);
}

/**
 * (e raised to power `v`) - 1
 *
 * @param v power value
 *
 * @returns {@link E} raised to the power of a number v, subtracted by 1.
 *
 * @example
 * ```
 * expm1(doubleNaN) = -nan
 * expm1(doubleInf) = inf
 * expm1(doubleNegInf) = -1
 */
export native function expm1(v: double): double;

/**
 * e raised to power `v`
 *
 * @param v power value
 *
 * @returns {@link e} raised to the power of a number `v`.
 *
 * @example
 * ```
 * exp(doubleNaN) = -nan
 * exp(doubleInf) = inf
 * exp(doubleNegInf) = 0
 */
export native function exp(v: double): double;

/**
 * Largest integer less or equal to `v`
 *
 * @param v arbitrary number
 *
 * @returns Largest integer less or equal to `v`
 *
 * @example
 * ```
 * floor(doubleNaN) = -nan
 * floor(doubleInf) = inf
 * floor(doubleNegInf) = -inf
 *
 * @remark
 * Implemented as native function,  @see `floor()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L104).
 */
export native function floor(v: double): double;

/**
 * Square root of the sum of squares of `v` and `u`
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns The square root of the sum of squares of its arguments
 *
 * @remark
 * Implemented as native function,  @see `hypot()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L540).
 */
export native function hypot(u: double, v: double): double;

/**
 * Base 10 logarithm of `v`
 *
 * @param v arbitrary number
 *
 * @returns The square root of the sum of squares of its arguments
 *
 * @example
 * ```
 * log10(doubleNaN) = -nan
 * log10(doubleInf) = inf
 * log10(doubleNegInf) = -nan
 */
export native function log10(v: double): double;

/**
 * Natural logarithm of (1 + `v`)
 *
 * @param v arbitrary number
 *
 * @returns The square root of the sum of squares of its arguments
 *
 * @example
 * ```
 * log1p(doubleNaN) = -nan
 * log1p(doubleInf) = inf
 * log1p(doubleNegInf) = -nan
 */
export function log1p(v: double): double {
    return log(1.0 + v);
}

/**
 * Base 2 logarithm of `v`
 *
 * @param v arbitrary number
 *
 * @returns The square root of the sum of squares of its arguments
 *
 * @example
 * ```
 * log2(doubleNaN) = -nan
 * log2(doubleInf) = inf
 * log2(doubleNegInf) = -nan
 */
export function log2(v: double): double {
    return log(v) / log(2.0);
}

/**
 * Natural logarithm of `v`
 *
 * @param v arbitrary number
 *
 * @returns The square root of the sum of squares of its arguments
 *
 * @example
 * ```
 * log(doubleNaN) = -nan
 * log(doubleInf) = inf
 * log(doubleNegInf) = -nan
 *
 * @remark
 * Implemented as native function,  @see `log()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 */
export native function log(v: double): double;

/**
 * Largest value of `u` and `v`
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Largest value of `u` and `v`
 */
export function max(v: double, u: double): double {
    if (v > u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Largest value of `u` and `v`
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Largest value of `u` and `v`
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
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Largest value of `u` and `v`
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
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Largest value of `u` and `v`
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
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Largest value of `u` and `v`
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
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Largest value of `u` and `v`
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
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Largest value of `u` and `v`
 */
export function max(v: char, u: char): char {
    if (v > u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Smallest value of `u` and `v`
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Smallest value of `u` and `v`
 */
export function min(v: double, u: double): double {
    if (v < u) {
        return v;
    } else {
        return u;
    }
}

/**
 * Smallest value of `u` and `v`
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Smallest value of `u` and `v`
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
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Smallest value of `u` and `v`
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
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Smallest value of `u` and `v`
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
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Smallest value of `u` and `v`
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
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Smallest value of `u` and `v`
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
 *
 * @param u arbitrary number
 * @param v arbitrary number
 *
 * @returns Smallest value of `u` and `v`
 */
export function min(v: char, u: char): char {
    if (v < u) {
        return v;
    } else {
        return u;
    }
}

// Remainder of `v / u`, reunded towards zero
export native function mod(u: double, v: double): double;

/**
 * `u` raised to power of `v`
 *
 * @param u base value
 * @param v power value
 *
 * @returns The value of a base `u` raised to a power `v`
 *
 * @remark
 * Implemented as native function,  @see `power()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L54).
 *
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
 * @param u base value
 * @param v power value
 *
 * @returns The remainder of `v / u`, reunded to nearest
 *
 * @remark
 * Implemented as native function,  @see `rem()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 */
export native function rem(v: double, u: double): double;

/**
 * `v` rounded to nearest integer
 *
 * @param v power value
 *
 * @returns `v` rounded to nearest integer
 *
 * @remark
 * Implemented as native function,  @see `rem()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 *
 * @example
 * ```
 * round(doubleNaN) = -nan
 * round(doubleInf) = inf
 * round(doubleNegInf) = -inf
 */
export native function round(v: double): double;

// `v` * 2 ** `n`
export function scalbn(v: double, n: int): double {
    return v * power(2, n);
}

/**
 * @param v arbitrary number
 *
 * @returns true if arg is negative, false otherwise
 *
 * @remark
 * Implemented as native function,  @see `signbit()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 *
 * @example
 * ```
 * signbit(doubleNaN) = true
 * signbit(doubleInf) = false
 * signbit(doubleNegInf) = true
 */
export native function signbit(v: double): boolean;

/**
 * @param v arbitrary number
 * @returns sign of `v`
 * @example
 * sign(0) = 0
 * sign(-Infinity) = -1
 * sign(NaN) ==== NaN
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

// Hyperbolic sine of `v`
export native function sinh(v: double): double;

/**
 * Sine of `v`
 *
 * @param v Angle in radians
 *
 * @returns Sine of angle in radians
 *
 * @remark
 * Implemented as native function,  @see `sin()` intrinsic [declration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L32).
 *
 * @example
 * ```
 * sin(doubleNaN) = -nan
 * sin(doubleInf) = -nan
 * sin(doubleNegInf) = -nan
 */
export native function sin(v: double): double;

/**
 * Square root of `v`
 *
 * @param v Number greater or equal to zero.
 *
 * @returns The square root of `v`, a nonnegative number.
 * If x is less than zero, returns NaN.
 *
 * @remark
 * Implemented as native function,  @see `sqrt()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L65).
 *
 * @example
 * ```
 * sqrt(doubleNaN) = -nan
 * sqrt(doubleInf) = inf
 * sqrt(doubleNegInf) = -nan
 */
export native function sqrt(v: double): double;

// Hyperbolic tangent of `v`
export native function tanh(v: double): double;

// Tangent of `v`
export native function tan(v: double): double;

/**
 * Integer part of `v`
 *
 * @param v Number to be truncated.
 *
 * @returns The integer part of a number by removing any fractional digits.
 *
 * @note
 * If arg is +Infinity or -Infinity, it is returned unmodified.
 * If arg is +0 or -0, it is returned unmodified.
 * If arg is NaN, NaN is returned
 *
 * @remark
 * Implemented as native function,  @see `trunc()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L115).
 */
export native function trunc(v: double): double;

export function seedRandom(value: long): void {}

/**
 * Pseudo-random number in the range [0.0, 1.0)
 *
 * @returns A floating-point, pseudo-random number that's greater than or equal to 0 and less than 1,
 * with approximately uniform distribution over that range — which you can then scale to your desired range.
 * Initial seed to the random number generator algorithm can be given using {@link seedRandom()} function.
 *
 * @remark
 * Implemented as native function,  @see `random()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#???).
 */
export native function random(): double;

export function log10(v: int): int throws {
    if (v < 1) {
        throw new ArgumentOutOfRangeException("Integer argument for log10 should be positive");
    }
    return (v >= 1000000000) ? 9 : (v >= 100000000) ? 8 :
        (v >= 10000000) ? 7 : (v >= 1000000) ? 6 :
        (v >= 100000) ? 5 : (v >= 10000) ? 4 :
        (v >= 1000) ? 3 : (v >= 100) ? 2 : (v >= 10) ? 1 : 0;
}

/**
 * "imul" method returns the result of the C-like 32-bit manipulation of the two parameters
 *  a cast to i32
 *  b cast to i32
 *  return (a * b) % 2 ** 32
 *
 *  imul(Infinity, 1) // 0
 *  imul(NaN, 1) // 0
 *  imul(2.5, 2.5) // 4
 *  imul(-5, 5.05) // 25
 *
 *  ECMA reference: https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-math.imul
 *
 * @remark
 * Implemented as native function,  @see `imul()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L327).
 */
export native function imul(a: double, b: double): int;

/**
 * "fround" returns nearest 32-bit single fp representation of a number
 *  in a 64-bit form
 *
 *  Math.fround(1.337) == 1.337 // false, result would be 1.3370000123977661
 *  Math.fround(1.5) == 1.5 // true
 *  Math.fround(-5.05) == -5.05 //false, result would be -5.050000190734863
 *  Math.fround(Infinity) // Infinity
 *  Math.fround(NaN) // NaN
 *
 * ECMA reference: https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-math.fround
  *
 * @remark
 * Implemented as native function,  @see `fround()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L338).
 */
export native function fround(doubleVal: double): double;
