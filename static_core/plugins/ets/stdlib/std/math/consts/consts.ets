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
 * @file Defines mathematical constants used in ArkTS.
 * @kit ArkTS
 */

package std.math.consts;

/**
 * Double-precision constant representing the value of π (pi).
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const PI: double = 3.141592653589793;

/**
 * Double-precision constant representing the base of natural logarithms e.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const E: double = 2.718281828459045;

/**
 * Double-precision constant representing the natural logarithm of 10.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const LN10: double = 2.302585092994046;

/**
 * Double-precision constant representing the natural logarithm of 2.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const LN2: double = 0.6931471805599453;

/**
 * Double-precision constant representing the base-2 logarithm of e.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const LOG2E: double = 1.4426950408889634;

/**
 * Double-precision constant representing the base-10 logarithm of e.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const LOG10E: double = 0.4342944819032518;

/**
 * Double-precision constant representing the square root of 1/2.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const SQRT1_2: double = 0.7071067811865476;

/**
 * Double-precision constant representing the square root of 2.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const SQRT2: double = 1.4142135623730951;

// export const floatEpsion: float = 1.19209e-07;
// export const floatMinimum: float = 1.17549e-38;
// export const floatMaximum: float = 3.40282e+38;
// export const floatNaN: float = 0.0 / 0.0;
// export const floatInf: float = 1.0 / 0.0;
// export const floatNegInf: float = -1.0 / 0.0;

/**
 * Double-precision constant representing the smallest positive number representable by a double, i.e., 
 *   the smallest difference between two double values.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const doubleEpsilon: double = 2.22045e-16;

/**
 * Double-precision constant representing the smallest normalized value representable by a double.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const doubleMinimum: double = 2.2250738585072014E-308;

/**
 * Double-precision constant representing the largest finite value representable by a double.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const doubleMaximum: double = 1.7976931348623157E+308;

/**
 * Double-precision constant representing a Not-a-Number (NaN) value as defined by IEEE 754.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const doubleNaN: double = 0.0 / 0.0;

/**
 * Double-precision constant representing positive infinity (+Infinity) as defined by IEEE 754.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const doubleInf: double = 1.0 / 0.0;

/**
 * Double-precision constant representing negative infinity (-Infinity) as defined by IEEE 754.
 * @type {double}
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const doubleNegInf: double = -1.0 / 0.0;
