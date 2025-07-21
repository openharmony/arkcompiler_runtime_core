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

package escompat;

/**
 * Represents the NaN value according to IEEE-754 specification
 */
// initialized in _initializerBlock_.ets
export const NaN: number;

/**
 * Represents the +Infinity value according to IEEE-754 specification
 */
// initialized in _initializerBlock_.ets
export const Infinity: number;

/**
 * Parses integer number from `String` with specified radix
 *
 * This function is an equivalent to @link{<Double.parseInt(s, radix)>}
 *
 * @returns result of parsing
 */
export function parseInt(s: String, radix: int): number {
    return Double.parseInt(s, radix);
}

/**
 * Parses integer number from `String` with specified radix
 *
 * This function is an equivalent to @link{<Double.parseInt(s, radix)>}
 *
 * @returns result of parsing
 */
export function parseInt(s: String, radix: number): number {
    return Double.parseInt(s, radix);
}

/**
 * Parses integer number from `String` with specified radix
 *
 * This function is an equivalent to @link{<Double.parseInt(s, radix)>}
 *
 * @returns result of parsing
 */
export function parseInt(s: String): number {
    return Double.parseInt(s);
}

/**
 * Parses floating point number from `String`
 *
 * This function is an equivalent to @link{<Double.parseFloat(s)>}
 *
 * @returns result of parsing
 */
export function parseFloat(s: String): number {
    return Double.parseFloat(s);
}

/**
 * Checks if double is `NaN` (not a number)
 *
 * @param d the `double` to test
 *
 * @returns `true` if the argument is `NaN`, `false` otherwise.
 */
export function isNaN(d: number): boolean {
    return Double.isNaN(d);
}

/**
 * Checks if `Float` value is `NaN` (not a number)
 *
 * @param f the `Float` value to test
 *
 * @returns `true` if the argument is `NaN`, `false` otherwise.
 */
export function isNaN(f: Float): boolean {
    return f.isNaN();
}

/**
 * Checks if `float` value is `NaN` (not a number)
 *
 * @param f the `float` value to test
 *
 * @returns `true` if the argument is `NaN`, `false` otherwise.
 */
export function isNaN(f: float): boolean {
    return Float.isNaN(f);
}

/**
 * Checks if `byte` value is `NaN` (not a number)
 *
 * @param b the `byte` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(b: byte): boolean { return false; }

/**
 * Checks if `int` value is `NaN` (not a number)
 *
 * @param i the `int` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(i: int): boolean { return false; }

/**
 * Checks if `short` value is `NaN` (not a number)
 *
 * @param s the `short` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(s: short): boolean { return false; }

/**
 * Checks if `long` value is `NaN` (not a number)
 *
 * @param l the `long` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(l: long): boolean { return false; }

/**
 * Checks if `Byte` value is `NaN` (not a number)
 *
 * @param b the `Byte` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(b: Byte): boolean { return false; }

/**
 * Checks if `Int` value is `NaN` (not a number)
 *
 * @param i the `Int` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(i: Int): boolean { return false; }

/**
 * Checks if `Short` value is `NaN` (not a number)
 *
 * @param s the `Short` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(s: Short): boolean { return false; }

/**
 * Checks if `Long` value is `NaN` (not a number)
 *
 * @param l the `Long` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(l: Long): boolean { return false; }

/**
 * Checks if `Double` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param d the `Double` value to test
 *
 * @returns `true` if the argument is a floating point value, `false` otherwise.
 */
export function isFinite(d: Number): boolean {
    return d.isFinite();
}

/**
 * Checks if `double` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param d the `double` value to test
 *
 * @returns `true` if the argument is a floating point value, `false` otherwise.
 */
export function isFinite(d: number): boolean {
    return Double.isFinite(d);
}

/**
 * Checks if `Float` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param f the `Float` value to test
 *
 * @returns `true` if the argument is a floating point value, `false` otherwise.
 */
export function isFinite(f: Float): boolean {
    return f.isFinite();
}

/**
 * Checks if `float` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param f the `float` value to test
 *
 * @returns `true` if the argument is a floating point value, `false` otherwise.
 */
export function isFinite(f: float): boolean {
    return Float.isFinite(f);
}

/**
 * Checks if `byte` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param b the `byte` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(b: byte): boolean { return true; }

/**
 * Checks if `int` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param i the `int` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(i: int): boolean { return true; }

/**
 * Checks if `short` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param s the `short` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(s: short): boolean { return true; }

/**
 * Checks if `long` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param l the `long` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(l: long): boolean { return true; }

/**
 * Checks if `Byte` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param b the `Byte` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(b: Byte): boolean { return true; }

/**
 * Checks if `Int` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param i the `Int` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(i: Int): boolean { return true; }

/**
 * Checks if `Short` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param s the `Short` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(s: Short): boolean { return true; }

/**
 * Checks if `Long` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param l the `Long` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(l: Long): boolean { return true; }

/**
 * Checks if float is similar to an integer value
 *
 * @param v the float to test
 *
 * @returns boolean - true if the argument is similar to an integer value
 */
export function isInteger(v: float): boolean {
    return Float.isInteger(v);
}

/**
 * Checks if Float is similar to an integer value
 *
 * @param v the Float to test
 *
 * @returns boolean - true if the argument is similar to an integer value
 */
export function isInteger(v: Float): boolean {
    return v.isInteger();
}

/**
 * Checks if double is similar to an integer value
 *
 * @param v the double to test
 *
 * @returns boolean - true if the argument is similar to an integer value
 */
export function isInteger(v: number): boolean {
    return Double.isInteger(v);
}

/**
 * Checks if Double is similar to an integer value
 *
 * @param v the Double to test
 *
 * @returns boolean - true if the argument is similar to an integer value
 */
export function isInteger(v: Number): boolean {
    return v.isInteger();
}

/**
 * Checks if `byte` is similar to an integer value
 *
 * @param b the `byte` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(b: byte): boolean { return true; }

/**
 * Checks if `int` is similar to an integer value
 *
 * @param i the `int` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(i: int): boolean { return true; }

/**
 * Checks if `short` is similar to an integer value
 *
 * @param s the `short` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(s: short): boolean { return true; }

/**
 * Checks if `long` value is similar to an integer value
 *
 * @param l the `long` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(l: long): boolean { return true; }

/**
 * Checks if `Byte` is similar to an integer value
 *
 * @param b the `Byte` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(b: Byte): boolean { return true; }

/**
 * Checks if `Int` is similar to an integer value
 *
 * @param i the `Int` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(i: Int): boolean { return true; }

/**
 * Checks if `Short` is similar to an integer value
 *
 * @param s the `Short` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(s: Short): boolean { return true; }

/**
 * Checks if `Long` value is similar to an integer value
 *
 * @param l the `Long` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(l: Long): boolean { return true; }

/**
 * Checks if float is a safe integer value
 *
 * @param v: float - the float to test
 *
 * @returns boolean - true if the argument is integer and less than Float.MAX_SAFE_INTEGER
 */
export function isSafeInteger(v: float): boolean {
    return Float.isSafeInteger(v);
}

/**
 * Checks if Float is a safe integer value
 *
 * @param v: Float - the Float to test
 *
 * @returns boolean - true if the underlying float is a safe integer
 */
export function isSafeInteger(v: Float): boolean {
    return v.isSafeInteger();
}

/**
 * Checks if double is a safe integer value
 *
 * @param v the double to test
 *
 * @returns boolean - true if the argument is integer and less than Double.MAX_SAFE_INTEGER
 */
export function isSafeInteger(v: number): boolean {
    return Double.isSafeInteger(v);
}

/*
 * Checks if Double is a safe integer value
 *
 * @param v the Double to test
 *
 * @returns boolean - true if the underlying double is a safe integer
 */
export function isSafeInteger(v: Number): boolean {
    return v.isSafeInteger();
}

/**
 * Checks if int can be represented in float without lossing precision
 *
 * @param v: int - the int to test
 *
 * @returns boolean - true if the argument is less than Float.MAX_SAFE_INTEGER
 */
export function isSafeInteger(v: int): boolean {
    return v <= Float.MAX_SAFE_INTEGER as int;
}

/**
 * Checks if Int can be represented in float without lossing precision
 *
 * @param v: Int - the Int to test
 *
 * @returns boolean - true if the argument is less than Float.MAX_SAFE_INTEGER
 */
export function isSafeInteger(v: Int): boolean {
    return v.unboxed() <= Float.MAX_SAFE_INTEGER as int;
}

/**
 * Checks if long can be represented in double without lossing precision
 *
 * @param v: long - the long to test
 *
 * @returns boolean - true if the argument is less than Double.MAX_SAFE_INTEGER
 */
export function isSafeInteger(v: long): boolean {
    return v <= Double.MAX_SAFE_INTEGER.toLong();
}

/**
 * Execute function on the next cycle.
 *
 * @param func: Function - the function to be executed on the next cycle.
 * @returns number - timer id
 */
export function setTimeout(func: Function): number {
    return setTimeout(func, 0);
}

/**
 * Execute function with parameters after the timer expires.
 *
 * @param func: Function - the function to be executed after the timer expires.
 * @param delayMs: number - timeout in milliseconds, decimals will be rounded down.
 * @param ...args: NullishType[] - parameters passed to func.
 * @returns number - timer id
 */
export function setTimeout(func: Function, delayMs: number, ...args: FixedArray<NullishType>): number {
    return startTimer((): void => { func.unsafeCall(...args) }, delayMs, false);
}

/**
 * Print error message on the next cycle.
 * 
 * @param msg: string - the message to be printed on the next cycle.
 * @returns number - timer id
 */
export function setTimeout(msg: string): number {
    return setTimeout(():void => {
        console.error(msg)
    }, 0);
}

/**
 * Print error message after the timer expires.
 *
 * @param msg: string - the message to be printed after the timer expires.
 * @param delayMs: number - timeout in milliseconds, decimals will be rounded down.
 * @returns number - timer id
 */
export function setTimeout(msg: string, delayMs: number): number {
    return setTimeout(():void => {
        console.error(msg)
    }, delayMs);
}

/**
 * Cancel the specified timer.
 *
 * @param timerId: number - the id of the timer returned from setTimeout.
 */
export function clearTimeout(timerId: number): void {
    stopTimer(timerId);
}

/**
 * Repeatedly do asynchronous call of the function.
 *
 * @param func: Function - the function to be executed.
 * @returns number - timer id
 */
export function setInterval(func: Function): number {
    return setInterval(func, 0);
}

/**
 * Repeatedly call the function with delayMs interval between calls. The first call will be after delayMs.
 *
 * @param func: Function - the function to be executed.
 * @param delayMs: number - timeout in milliseconds.
 * @param ...args: NullishType[] - parameters passed to func.
 * @returns number - timer id
 */
export function setInterval(func: Function, delayMs: number, ...args: FixedArray<NullishType>): number {
    return startTimer((): void => { func.unsafeCall(...args) }, delayMs, true);
}

/**
 * Repeatedly print the error message asynchronously.
 *
 * @param msg: string - the message to be printed.
 * @returns number - timer id
 */
export function setInterval(msg: string): number {
    return setInterval(():void => {
        console.error(msg)
    }, 0);
}

/**
 * Repeatedly print the error message with delayMs interval between calls. The first print will be after delayMs.
 *
 * @param msg: string - the message to be printed.
 * @param delayMs: number - timeout in milliseconds.
 * @returns number - timer id
 */
export function setInterval(msg: string, delayMs: number): number {
    return setInterval(():void => {
        console.error(msg)
    }, delayMs)
}

/**
 * Cancel the specified timer.
 *
 * @param timerId: number - the id of the timer returned from setInterval.
 */
export function clearInterval(timerId: number): void {
    stopTimer(timerId);
}

function startTimer(func: () => void, delayMs: number, repeat: boolean): number
{
    const realDelayMs = delayMs.toInt();
    if (CoroutineExtras.workerHasExternalScheduler()) {
        return startTimerImpl(func as Object, realDelayMs < 0 ? 0 : realDelayMs, repeat).toDouble();
    }
    return registerTimer(func, realDelayMs < 0 ? 0 : realDelayMs, repeat).toDouble();
}

function stopTimer(timerId: number): void
{
    if (CoroutineExtras.workerHasExternalScheduler()) {
        stopTimerImpl(timerId.toInt());
        return;
    }
    clearTimer(timerId.toInt());
}

native function startTimerImpl(func: Object, delayMs: int, repeat: boolean): int;
native function stopTimerImpl(timerId: int): void;
