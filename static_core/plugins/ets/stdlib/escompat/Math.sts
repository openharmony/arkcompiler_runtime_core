/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

package escompat;

import {PI, E, LN10, LN2, LOG2E, LOG10E, SQRT1_2, SQRT2} from "std/math/consts";

/**
 * The Math class contains static properties and methods for mathematical constants and functions.
 */
export class Math {
    // TODO(dmitrys99) Reuse constants from "std/math"
    /**
     * PI as double value
     */
    public static readonly PI: number = PI;

    /**
     * Euler number as double value
     */
    public static readonly E: number = E;

    /**
     * Natural logarithm of 10 as double value
     */
    public static readonly LN10: number = LN10;

    /**
     * Natural logarithm of 2 as double value
     */
    public static readonly LN2: number = LN2;

    /**
     * Logarithm base 2 of Euler number as double value
     */
    public static readonly LOG2E: number = LOG2E;

    /**
     * Logarithm base 10 of Euler number as double value
     */
    public static readonly LOG10E: number = LOG10E;

    /**
     * Square root of 1/2 as double value
     */
    public static readonly SQRT1_2: number = SQRT1_2;

    /**
     * Square root of 2 as double value
     */
    public static readonly SQRT2: number = SQRT2;

    /**
     * Absolute value
     *
     * @param v - Some number value
     *
     * @returns Absolute value of `v`
     *
     * @example
     * ```
     * Math.abs(doubleNaN) = NaN
     * Math.abs(doubleInf) = Inf
     * Math.abs(doubleNegInf) = Inf
     * ```
     */
    public static abs(x: number): number {
        return abs(x);
    }

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
    public static acos(x: number): number {
        return acos(x);
    }

    /**
     * Hyperbolic arccosine of an angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic arccosine of angle `v`
     *
     * @example
     * ```
     * Math.acosh(doubleNaN) = -NaN
     * Math.acosh(doubleInf) = Inf
     * Math.acosh(doubleNegInf) = -NaN
     * ```
     */
    public static acosh(x: number): number {
        return acosh(x);
    }

    /**
     * Arcsine of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Arcsine of angle `v`
     */
    public static asin(x: number): number {
        return asin(x);
    }

    /**
     * Hyperbolic arcsine of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic arcsine of angle `v`
     *
     * @example
     * ```
     * Math.asinh(doubleNaN) = -NaN
     * Math.asinh(doubleInf) = Inf
     * Math.asinh(doubleNegInf) = -Inf
     * ```
     */
    public static asinh(x: number): number {
        return asinh(x);
    }

    /**
     * Arctangent of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Arctangent of angle `v`
     *
     * @example
     * ```
     * Math.atan(doubleNaN) = -NaN
     * Math.atan(doubleInf) = 1.5708
     * Math.atan(doubleNegInf) = -1.5708
     * ```
     */
    public static atan(x: number): number {
        return atan(x);
    }

    /**
     * Method returns the angle in the plane (in radians) between the positive x-axis and the ray from (0, 0) to the point (x, y), for Math.atan2(y, x).
     *
     * @returns The angle in radians (between -π and π, inclusive) between the positive x-axis and the ray from (0, 0) to the point (x, y).
     *
     * @remark
     * The atan2() method measures the counterclockwise angle θ, in radians, between the positive x-axis and the point (x, y).
     * Note that the arguments to this function pass the y-coordinate first and the x-coordinate second.
     */
    public static atan2(y: number, x: number): number {
        return atan2(y, x);
    }

    /**
     * Hyperbolic arctangent of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic arctangent of angle `v`
     *
     * @example
     * ```
     * Math.atanh(doubleNaN) = -NaN
     * Math.atanh(doubleInf) = -NaN
     * Math.atanh(doubleNegInf) = -NaN
     * ```
     */
    public static atanh(x: number): number {
        return atanh(x);
    }

    /**
     * Cube root of a number.
     *
     * @param x arbitrary number
     *
     * @remark
     * Math.𝚌brt(𝚡) = ∛x = the unique y such that y³ = x.
     */
    public static cbrt(x: number): number {
        if (isNaN(x) || x == Infinity || x == -Infinity) return x;
        return cbrt(x);
    }

    /**
     * Smallest integer greater or equal to `v`
     *
     * @param v arbitrary number
     *
     * @returns Smallest integer greater or equal to v
     *
     * @example
     * ```
     * Math.ceil(doubleNaN) = -nan
     * Math.ceil(doubleInf) = inf
     * Math.ceil(doubleNegInf) = -inf
     * ```
     */
    public static ceil(x: number): number {
        return ceil(x);
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
     * Math.clz32(0xFFFFFFFF) == 0
     * Math.clz32(0x0000FFFF) == 16
     * Math.clz32(0x0) == 32
     * ```
     */
    public static clz32(x: Int): Int {
        return new Int(clz32(x.intValue()));
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
     * Math.clz32(0xFFFFFFFF) == 0
     * Math.clz32(0x0000FFFF) == 16
     * Math.clz32(0x0) == 32
     * ```
     */
    public static clz32(value: number): number {
        return clz32Double(value);
    }

    /**
     * Cosine of `x`
     *
     * @param x Angle in radians
     *
     * @returns Cosine of angle in radians
     *
     * @example
     * ```
     * Math.cos(doubleNaN) = -NaN
     * Math.cos(doubleInf) = -NaN
     * Math.cos(doubleNegInf) = -NaN
     * ```
     */
    public static cos(x: number): number {
        return cos(x);
    }

    /**
     * Hyperbolic cosine of an angle `x`
     *
     * @param x angle in radians
     *
     * @returns Hyperbolic cosine of angle `x`
     *
     * @example
     * ```
     * Math.cosh(doubleNaN) = -NaN
     * Math.cosh(doubleInf) = Inf
     * Math.cosh(doubleNegInf) = Inf
     * ```
     */
    public static cosh(x: number): number {
        return cosh(x);
    }
    /**
     * e raised to power `x`
     *
     * @param x power value
     *
     * @returns {@link Math.E} raised to the power of a number `x`.
     *
     * @example
     * ```
     * Math.exp(doubleNaN) = -NaN
     * Math.exp(doubleInf) = Inf
     * Math.exp(doubleNegInf) = 0
     * ```
     */
    public static exp(x: number): number {
        return exp(x);
    }

    /**
     * (`e` raised to power `v`) - 1
     *
     * @param x power value
     *
     * @returns {@link Math.E} raised to the power of a number `x`, subtracted by `1`.
     *
     * @example
     * ```
     * Math.expm1(doubleNaN) = -NaN
     * Math.expm1(doubleInf) = Inf
     * Math.expm1(doubleNegInf) = -1
     * ```
     */
    public static expm1(x: number): number {
        return expm1(x);
    }

    /**
     * Largest integer less or equal to `v`
     *
     * @param v arbitrary number
     *
     * @returns Largest integer less or equal to `v`
     *
     * @example
     * ```
     * Math.floor(doubleNaN) = -NaN
     * Math.floor(doubleInf) = Inf
     * Math.floor(doubleNegInf) = -Inf
     * ```
     */
    public static floor(x: number): number {
        return floor(x);
    }

    /**
    * "fround" returns nearest 32-bit single fp representation of a number
    *  in a 64-bit form
    *
    *  Math.fround(1.337) == 1.337 // false, result would be 1.3370000123977661
    *  Math.fround(1.5) == 1.5 // true
    *  Math.fround(-5.05) == -5.05 //false, result would be -5.050000190734863
    *  Math.fround(Infinity) // Infinity
    *  Math.fround(NaN) // NaN
    */
    public static fround(x: number): number {
        return fround(x);
    }

    /**
     * Square root of the sum of squares of `v` and `u`
     *
     * @param u arbitrary number
     * @param v arbitrary number
     *
     * @returns The square root of the sum of squares of its arguments
     */
    public static hypot(...values: number[]): number {
        let max: number = 0;
        let hasNaN = false;

        for (const value of values) {
            if (value == Infinity || value == -Infinity) {
                return Infinity;
            }
            if (isNaN(value)) {
                hasNaN = true;
            }
            max = Math.max(max, Math.abs(value));
        }
        if (hasNaN) return NaN;
        if (max == 0) return 0;
        let sum: number = 0.0;
        for (const value of values) {
            let normalized = value / max;
            sum += normalized * normalized;
        }
        return max * Math.sqrt(sum);
    }

    /**
     *  Method returns the result of the C-like 32-bit manipulation of the two parameters
     *
     *  @returns (a * b) % 2 ** 32
     *
     *  Math.imul(Infinity, 1) = 0
     *  Math.imul(NaN, 1) = 0
     *  Math.imul(2.5, 2.5) = 4
     *  Math.imul(-5, 5.05) = 25
     *
     */
    public static imul(a: number, b: number): number {
        return imul(a, b);
    }


    /**
     * Natural logarithm of `v`
     *
     * @param v: double - arbitrary number
     *
     * @returns The natural logarithm (base e) of x. If x is ±0, returns -Infinity. If x < 0, returns NaN.
     *
     * @example
     * ```
     * Math.log(doubleNaN) = -nan
     * Math.log(doubleInf) = inf
     * Math.log(doubleNegInf) = -nan
     * ```
     */
    public static log(x: number): number {
        return log(x);
    }

    /**
     * Base 10 logarithm of `x`
     *
     * @param v arbitrary number
     *
     * @returns The base 10 logarithm of x. If x < 0, returns NaN.
     *
     * @example
     * ```
     * Math.log10(doubleNaN) = -NaN
     * Math.log10(doubleInf) = Inf
     * Math.log10(doubleNegInf) = -NaN
     * ```
     */
    public static log10(x: number): number {
        return log10(x);
    }

    /**
     * Natural logarithm of (1 + `x`)
     *
     * @param x arbitrary number
     *
     * @returns The natural logarithm (base e) of x + 1. If x is -1, returns -Infinity. If x < -1, returns NaN.
     *
     * @example
     * ```
     * Math.log1p(doubleNaN) = -NaN
     * Math.log1p(doubleInf) = Inf
     * Math.log1p(doubleNegInf) = -NaN
     * ```
     */
    public static log1p(x: number): number {
        return log1p(x);
    }

    /**
     * Base 2 logarithm of `x`
     *
     * @param x arbitrary number
     *
     * @returns The base 2 logarithm of x. If x < 0, returns NaN.
     *
     * @example
     * ```
     * Math.log2(doubleNaN) = -nan
     * Math.log2(doubleInf) = inf
     * Math.log2(doubleNegInf) = -nan
     * ```
     */
    public static log2(x: number): number {
        return log2(x);
    }

    /**
     * Largest value of `u` and `v`
     *
     * @param u arbitrary number (of type `Int`)
     * @param v arbitrary number (of type `Int`)
     *
     * @returns Largest value of `u` and `v`
     */
    public static max(...values: number[]): number {
        if (values.length == 0) {
            return -Infinity;
        }
        let max = values[0];
        for (let i = 1; i < values.length; i++) {
            if (isNaN(values[i])) return NaN;
            if (values[i] > max) {
                max = values[i];
            }
        }
        return max;
    }

    /**
     * Smallest value of `u` and `v`
     *
     * @param u arbitrary number (of type `Int`)
     * @param v arbitrary number (of type `Int`)
     *
     * @returns Smallest value of `u` and `v`
     */
    public static min(...values: number[]): number {
        if (values.length == 0) {
            return Infinity;
        }
        let min = values[0];
        for (let i = 1; i < values.length; i++) {
            if (isNaN(values[i])) return NaN;
            if (values[i] < min) {
                min = values[i];
            }
        }
        return min;
    }

    /**
     * `u` raised to power of `v`
     *
     * @param u: base value
     * @param v: power value
     *
     * @returns The value of a base `u` raised to a power `v`
     *
     * @example
     * ```
     * Math.pow(doubleNaN, doubleInf) = -NaN
     * Math.pow(doubleInf, doubleInf) = Inf
     * ```
     */
    public static pow(u: number, v: number): number {
        return power(u, v);
    }

    /**
     * Pseudo-random number in the range [0.0, 1.0)
     *
     * @returns A floating-point, pseudo-random number that's greater than or equal to 0 and less than 1,
     * with approximately uniform distribution over that range — which you can then scale to your desired range.
     * Initial seed to the random number generator algorithm can be given using {@link seedRandom()} function.
     *
     */
    public static random(): number {
        return random();
    }

    /**
     * `v` rounded to nearest integer
     *
     * @param v arbitrary value
     *
     * @returns `v` rounded to nearest integer
     *
     * @example
     * ```
     * Math.round(doubleNaN) = -NaN
     * Math.round(doubleInf) = Inf
     * Math.round(doubleNegInf) = -Inf
     * ```
     */
    public static round(x: number): number {
        return round(x);
    }

    /**
     * @param x arbitrary number
     *
     * @returns -1 if `x` is negative, 1 if `x` is positive, 0 if `x` is close to zero (epsilon is 1e-13)
     *
     */
    public static sign(x: number): number {
        if (isNaN(x)) {
            return x;
        }
        return sign(x) as double as number;
    }

    /**
     * Sine of `v`
     *
     * @param v angle in radians
     *
     * @returns Sine of angle in radians
     *
     * @example
     * ```
     * Math.sin(doubleNaN) = -nan
     * Math.sin(doubleInf) = -nan
     * Math.sin(doubleNegInf) = -nan
     * ```
     */
    public static sin(x: number): number {
        return sin(x);
    }

    /**
     * Hyperbolic sine of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic sine of angle `v`
     *
     * @example
     * ```
     * Math.sinh(doubleNaN) = -nan
     * Math.sinh(doubleInf) = inf
     * Math.sinh(doubleNegInf) = -inf
     * ```
     */
    public static sinh(x: number): number {
        return sinh(x);
    }

    /**
     * Square root of `x`
     *
     * @param x number greater or equal to zero.
     *
     * @returns The square root of `x`, a non-negative number.
     * If `x` is less than zero, returns `NaN`.
     *
     * @example
     * ```
     * Math.sqrt(doubleNaN) = -nan
     * Math.sqrt(doubleInf) = inf
     * Math.sqrt(doubleNegInf) = -nan
     * ```
     */
    public static sqrt(x: number): number {
        return sqrt(x);
    }

    /**
     * Tangent of angle `x`
     *
     * @param x angle in radians
     *
     * @returns Tangent of angle `x`
     *
     * @example
     * ```
     * Math.tan(doubleNaN) = -nan
     * Math.tan(doubleInf) = -nan
     * Math.tan(doubleNegInf) = -nan
     * ```
     */
    public static tan(x: number): number {
        return tan(x);
    }

    /**
     * Hyperbolic arctangent of angle `x`
     *
     * @param x angle in radians
     *
     * @returns Hyperbolic arctangent of angle `x`
     *
     * @example
     * ```
     * Math.atanh(doubleNaN) = -nan
     * Math.atanh(doubleInf) = -nan
     * Math.atanh(doubleNegInf) = -nan
     * ```
     */
    public static tanh(x: number): number {
        if (isNaN(x)) return NaN;
        return tanh(x);
    }

    /**
     * Integer part of `v`
     *
     * @param v number to be truncated.
     *
     * @returns The integer part of a number by removing any fractional digits.
     *
     * @notes
     * If arg is +Infinity or -Infinity, it is returned unmodified.
     * If arg is +0 or -0, it is returned unmodified.
     * If arg is NaN, NaN is returned
     *
     */
    public static trunc(x: number): number {
        return trunc(x);
    }
}
