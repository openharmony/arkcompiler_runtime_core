/**
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
 * @file Defines the Global for ArkTS
 * @kit ArkTS
 */


package escompat;

/**
 * Defines NaN
 * Represents the NaN value according to IEEE-754 specification.
 *
 * @type { number }
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const NaN: number;

/**
 * Defines Infinity
 * Represents the +Infinity value according to IEEE-754 specification.
 *
 * @type { number }
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export const Infinity: number;

/**
 * Parses integer number from `String` with specified radix
 * This function is an equivalent to @link{<Double.parseInt(s, radix)>}
 *
 * @param { String } s - a string that needs to be parsed
 * @returns { number } - result of parsing
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function parseInt(s: String): number {
    return Double.parseInt(s);
}

/**
 * Parses integer number from `String` with specified radix
 * This function is an equivalent to @link{<Double.parseInt(s, radix)>}
 *
 * @param { String } s - a string that needs to be parsed
 * @param { int } radix - radix format
 * @returns { number } - result of parsing
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function parseInt(s: String, radix: int): number {
    return Double.parseInt(s, radix);
}

/**
 * Parses integer number from `String` with specified radix
 * This function is an equivalent to @link{<Double.parseInt(s, radix)>}
 *
 * @param { String } s - a string that needs to be parsed
 * @param { number } radix - radix format
 * @returns { number } - result of parsing
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function parseInt(s: String, radix: number): number {
    return Double.parseInt(s, radix);
}

/**
 * Parses floating point number from `String`
 * This function is an equivalent to @link{<Double.parseFloat(s)>}
 *
 * @param { String } s - a string that needs to be parsed
 * @returns { number } - result of parsing
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function parseFloat(s: String): number {
    return Double.parseFloat(s);
}

/**
 * Checks if `byte` value is `NaN` (not a number)
 *
 * @param { byte } b - the `byte` value to test
 * @returns { boolean } - unconditionally returns `false`. Integral types can not be `NaN`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isNaN(b: byte): boolean { return false; }

/**
 * Checks if `short` value is `NaN` (not a number)
 *
 * @param { short } s - the `short` value to test
 * @returns { boolean } - unconditionally returns `false`. Integral types can not be `NaN`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isNaN(s: short): boolean { return false; }

/**
 * Checks if `int` value is `NaN` (not a number)
 *
 * @param { int } i - the `int` value to test
 * @returns { boolean } - unconditionally returns `false`. Integral types can not be `NaN`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isNaN(i: int): boolean { return false; }

/**
 * Checks if `long` value is `NaN` (not a number)
 *
 * @param { long } l - the `long` value to test
 * @returns { boolean } - unconditionally returns `false`. Integral types can not be `NaN`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isNaN(l: long): boolean { return false; }

/**
 * Checks if `float` value is `NaN` (not a number)
 *
 * @param { float } f - the `float` value to test
 * @returns { boolean } - `true` if the argument is `NaN`, `false` otherwise.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isNaN(f: float): boolean {
    return Float.isNaN(f);
}

/**
 * Checks if number is `NaN` (not a number)
 *
 * @param { number } d - the `number` to test
 * @returns { boolean } - `true` if the argument is `NaN`, `false` otherwise.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isNaN(d: number): boolean {
    return d.isNaN();
}

/**
 * Checks if `byte` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param { byte } b - the `byte` value to test
 * @returns { boolean } - unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isFinite(b: byte): boolean { return true; }

/**
 * Checks if `short` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param { short } s - the `short` value to test
 * @returns { boolean } - unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isFinite(s: short): boolean { return true; }

/**
 * Checks if `int` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param { int } i - the `int` value to test
 * @returns { boolean } - unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isFinite(i: int): boolean { return true; }

/**
 * Checks if `long` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param { long } l - the `long` value to test
 * @returns { boolean } - unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isFinite(l: long): boolean { return true; }

/**
 * Checks if `Float` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param { float } f - the `float` value to test
 * @returns { boolean } - `true` if the argument is a floating point value, `false` otherwise.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isFinite(f: float): boolean {
    return f.isFinite();
}

/**
 * Checks if `double` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param { number } d - the `double` value to test
 * @returns { boolean } - `true` if the argument is a floating point value, `false` otherwise.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isFinite(d: number): boolean {
    return d.isFinite();
}

/**
 * Checks if `byte` is similar to an integer value
 *
 * @param { byte } b - the `byte` value to test
 * @returns { boolean } - unconditionally returns `true` as integral types are integer.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isInteger(b: byte): boolean { return true; }

/**
 * Checks if `short` is similar to an integer value
 *
 * @param { short } s - the `short` value to test
 * @returns { boolean } - unconditionally returns `true` as integral types are integer.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isInteger(s: short): boolean { return true; }

/**
 * Checks if `int` is similar to an integer value
 *
 * @param { int } i - the `int` value to test
 * @returns { boolean } - unconditionally returns `true` as integral types are integer.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isInteger(i: int): boolean { return true; }

/**
 * Checks if `long` value is similar to an integer value
 *
 * @param { long } l - the `long` value to test
 * @returns { boolean } - unconditionally returns `true` as integral types are integer.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isInteger(l: long): boolean { return true; }

/**
 * Checks if float is similar to an integer value
 *
 * @param { float } f - the `float` value to test
 * @returns { boolean } - true if the argument is similar to an integer value
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isInteger(v: float): boolean {
    return v.isInteger();
}

/**
 * Checks if double is similar to an integer value
 *
 * @param { number } v - the `double` value to test
 * @returns { boolean } - true if the argument is similar to an integer value
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isInteger(v: number): boolean {
    return v.isInteger();
}

/**
 * Checks if int can be represented in float without lossing precision
 *
 * @param { int } v - the `int` value to test
 * @returns { boolean } - true if the argument is less than Float.MAX_SAFE_INTEGER
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isSafeInteger(v: int): boolean {
    return v <= Float.MAX_SAFE_INTEGER as int;
}

/**
 * Checks if long can be represented in double without lossing precision
 *
 * @param { long } v - the `long` value to test
 * @returns { boolean } - true if the argument is less than Double.MAX_SAFE_INTEGER
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isSafeInteger(v: long): boolean {
    return v <= Double.MAX_SAFE_INTEGER.toLong();
}

/**
 * Checks if float is a safe integer value
 *
 * @param { float } v - the `float` value to test
 * @returns { boolean } - true if the argument is integer and less than Float.MAX_SAFE_INTEGER
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isSafeInteger(v: float): boolean {
    return v.isSafeInteger();
}

/**
 * Checks if double is a safe integer value
 *
 * @param { number } v - the `double` value to test
 * @returns { boolean } - true if the argument is integer and less than Double.MAX_SAFE_INTEGER
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function isSafeInteger(v: number): boolean {
    return v.isSafeInteger();
}

/**
 * Execute function on the next cycle.
 *
 * @param { Function } func - the function to be executed on the next cycle.
 * @returns { int } - timer id
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function setTimeout(func: Function): int {
    return setTimeout(func, 0);
}

/**
 * Execute function with parameters after the timer expires.
 *
 * @param { Function } func - the function to be executed after the timer expires.
 * @param { int | null | undefined } delayMs - timeout in milliseconds,
 *     if pass null or undefined will be treated as 0 milliseconds.
 * @param { FixedArray<Any> } args - parameters passed to func.
 * @returns { int } - timer id
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function setTimeout(func: Function, delayMs: int | null | undefined, ...args: FixedArray<Any>): int {
    if (delayMs) {
        return startTimer((): void => { func.unsafeCall(...args) }, delayMs, false);
    } else {
        return startTimer((): void => { func.unsafeCall(...args) }, 0, false);
    }
}

/**
 * Print error message after the timer expires.
 *
 * @param { string } msg - the message to be printed after the timer expires.
 * @param { int | null } [delayMs]- timeout in milliseconds,
 *     if pass empty or null or undefined will be treated as 0 milliseconds.
 * @returns { int } - timer id
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function setTimeout(msg: string, delayMs?: int | null): int {
    if (delayMs) {
        return setTimeout(():void => {
            console.error(msg)
        }, delayMs);
    } else {
        return setTimeout(():void => {
            console.error(msg)
        }, 0);
    }
}

/**
 * Cancel the specified timer.
 *
 * @param { int | null } [timerId] - the id of the timer returned from setTimeout,
 *     if pass empty or null or undefined will do nothing.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function clearTimeout(timerId?: int | null): void {
    if (timerId) {
        stopTimer(timerId);
    }
}

/**
 * Repeatedly call the function with delayMs interval between calls. The first call will be after delayMs.
 *
 * @param { Function } func - the function to be executed.
 * @param { int | null | undefined } delayMs - timeout in milliseconds,
 *     if pass null or undefined will be treated as 0 milliseconds.
 * @param { FixedArray<Any> } args - parameters passed to func.
 * @returns { int } - timer id
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function setInterval(func: Function, delayMs: int | null | undefined, ...args: FixedArray<Any>): int {
    if (delayMs) {
        return startTimer((): void => { func.unsafeCall(...args) }, delayMs, true);
    } else {
        return startTimer((): void => { func.unsafeCall(...args) }, 0, true);
    }
}

/**
 * Repeatedly print the error message with delayMs interval between calls. The first print will be after delayMs.
 *
 * @param { string } msg - the message to be printed.
 * @param { int | null } [delayMs] - timeout in milliseconds,
 *     if pass empty or null or undefined will be treated as 0 milliseconds.
 * @returns { int } - timer id
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function setInterval(msg: string, delayMs?: int | null): int {
    if (delayMs) {
        return setInterval(():void => {
            console.error(msg)
        }, delayMs);
    } else {
        return setInterval(():void => {
            console.error(msg)
        }, 0);
    }
}

/**
 * Cancel the specified timer.
 *
 * @param { int | null } [timerId] - the id of the timer returned from setInterval,
 *     if pass empty or null or undefined will do nothing.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function clearInterval(timerId?: int | null): void {
    if (timerId) {
        stopTimer(timerId);
    }
}

function startTimer(func: () => void, delayMs: int, repeat: boolean): int
{
    const realDelayMs = delayMs;
    if (CoroutineExtras.isExternalTimerEnabled()) {
        return startTimerImpl(func as Object, realDelayMs < 0 ? 0 : realDelayMs, repeat);
    }
    return registerTimer(func, realDelayMs < 0 ? 0 : realDelayMs, repeat);
}

function stopTimer(timerId: int): void
{
    if (CoroutineExtras.isExternalTimerEnabled()) {
        stopTimerImpl(timerId);
        return;
    }
    clearTimer(timerId);
}

native function startTimerImpl(func: Object, delayMs: int, repeat: boolean): int;
native function stopTimerImpl(timerId: int): void;

native function registerTimer(func: Object, delayMs: int, repeat: boolean): int;
native function clearTimer(timerId: int): void;
