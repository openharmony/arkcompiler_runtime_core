/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import { BusinessError } from "@ohos.base";

export type Rounding = number
export type Modulo = number
export type DecimalValue = Decimal | number | string;

const RANGE_ERROR_CODE = 10200001;
const TYPE_ERROR_CODE = 401;
const PRECISION_LIMIT_EXCEEDED_ERROR_CODE = 10200060;
const CRYPTO_UNAVAILABLE_ERROR_CODE = 10200061;

const EXP_LIMIT: number = 9e15;
const MAX_DIGITS: number = 1e9;
const NUMERALS: string = '0123456789abcdef';

const LN10 = ('2.302585092994045684017991454684364207601101488628772976033327900967572609677352480235997205089598298341'
    + '9677840422862486334095254650828067566662873690987816894829072083255546808437998948262331985283935053089653777326'
    + '2884616336622228769821988674654366747440424327436515504893431493939147961940440022210510171417480036880840126470'
    + '8068556774321622835522011480466371565912137345074785694768346361679210180644507064800027750268491674655058685693'
    + '5673420670581136429224554405758925724208241314695689016758940256776311356919292033376587141660230105703089634572'
    + '0754403708474699401682692828084811842893148485249486448719278096762712757753970276686059524967166741834857044225'
    + '0719796500471495105049221477656763693866297697952211071826454973477266242570942932258279850258550978526538320760'
    + '6726317164309505995087807523710333101197857547331541421808427543863591778117054309827482385045648019095610299291'
    + '8243182375253577097505395651876975103749708886921802051893395072385392051446341972652872869651108625714921988499'
    + '78748873771345686209167058');

const DEFAULTS_PRECISION = 20;
const DEFAULTS_ROUNDING = 4;
const DEFAULTS_MODULO = 1;
const DEFAULTS_TOEXPNEG = -7;
const DEFAULTS_TOEXPPOS = 21;

let external: boolean = true;
let inexact: boolean = false;
const tag: string = '[object Decimal]';

const isBinary: RegExp = new RegExp("^0b([01]+(\\.[01]*)?|\\.[01]+)(p[+-]?\\d+)?$", "i");
const isHex: RegExp = new RegExp("^0x([0-9a-f]+(\\.[0-9a-f]*)?|\\.[0-9a-f]+)(p[+-]?\\d+)?$", "i");
const isOctal: RegExp = new RegExp("^0o([0-7]+(\\.[0-7]*)?|\\.[0-7]+)(p[+-]?\\d+)?$", "i");
const isDecimal: RegExp = new RegExp("^(\\d+(\\.\\d*)?|\\.\\d+)(e[+-]?\\d+)?$", "i");

const BASE: number = 1e7;
const LOG_BASE: number = 7;
const MAX_SAFE_INTEGER: number = 9007199254740991;

const LN10_PRECISION = LN10.length - 1;

export interface DecimalConfig {
    precision?: number;
    rounding?: number;
    toExpNeg?: number;
    toExpPos?: number;
    minE?: number;
    maxE?: number;
    crypto?: boolean;
    modulo?: number;
    defaults?: boolean;
}

export class Decimal {
    internal digits: Array<number> | null = new Array<number>();
    internal exponent: number;
    internal sign: number;
    internal toStringTag: string = '[object Decimal]';

    public static maxE: number = EXP_LIMIT;
    public static minE: number = -EXP_LIMIT;
    public static precision: number = DEFAULTS_PRECISION;
    public static rounding: number = DEFAULTS_ROUNDING;
    public static toExpNeg: number = DEFAULTS_TOEXPNEG;
    public static toExpPos: number = DEFAULTS_TOEXPPOS;
    public static modulo: number = DEFAULTS_MODULO;
    public static crypto: boolean = false;

    // defined in d.ets, See "@arkts.math.Decimal.d.ets" for details.
    public static readonly ROUND_UP: number = 0;
    public static readonly ROUND_DOWN: number = 1;
    public static readonly ROUND_CEILING: number = 2;
    public static readonly ROUND_FLOOR: number = 3;
    public static readonly ROUND_HALF_UP: number = 4;
    public static readonly ROUND_HALF_DOWN: number = 5;
    public static readonly ROUND_HALF_EVEN: number = 6;
    public static readonly ROUND_HALF_CEILING: number = 7;
    public static readonly ROUND_HALF_FLOOR: number = 8;
    public static readonly EUCLIDEAN: number = 9;

    get d(): Array<number> | null {
        return this.digits;
    }

    get e(): number {
        return this.exponent;
    }

    get s(): number {
        return this.sign;
    }

    /**
     * Return a new Decimal whose value is the absolute value of this Decimal.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     */
    constructor(n: DecimalValue) {
        if (Utils.isDecimalInstance(n)) {
            this.initializeByDecimal(n as Decimal);
        } else if (typeof n === 'number') {
            this.initializeByNumber(n as number);
        } else if (typeof n === 'string') {
            this.initializeByString(n as string);
        } else {
            throw Utils.createBusinessError(TYPE_ERROR_CODE,
                `The type of "index" must be String. Received value is: ${n}`);
        }
    }

    /**
     * Return a new Decimal whose value is the absolute value of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public abs(): Decimal {
        let x = new Decimal(this);
        if (x.sign < 0) {
            x.sign = 1;
        }
        return Utils.finaliseExternal(x);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a whole number in the
     * direction of negative Infinity.
     *
     * @returns { Decimal } the Decimal type
     */
    public floor(): Decimal {
        return Utils.finalise(new Decimal(this), this.exponent + 1, Decimal.ROUND_FLOOR);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a whole number in the
     * direction of positive Infinity.
     *
     * @returns { Decimal } the Decimal type
     */
    public ceil(): Decimal {
        return Utils.finalise(new Decimal(this), this.exponent + 1, Decimal.ROUND_CEILING);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal truncated to a whole number.
     *
     * @returns { Decimal } the Decimal type
     */
    public trunc(): Decimal {
        return Utils.finalise(new Decimal(this), this.exponent + 1, Decimal.ROUND_DOWN);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal clamped to the range
     * delineated by `min` and `max`.
     *
     * @param { DecimalValue } min {number | string | Decimal}
     * @param { DecimalValue } max {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200001 - The value of `min` is out of range.
     */
    public clamp(min: DecimalValue, max: DecimalValue): Decimal {
        let minDecimal = new Decimal(min);
        let maxDecimal = new Decimal(max);
        if (!minDecimal.s || !maxDecimal.s) {
            return new Decimal(NaN);
        }
        if (minDecimal.greaterThan(maxDecimal)) {
            throw Utils.createBusinessError(RANGE_ERROR_CODE,
                `The value of min is out of range. It must be <= ${maxDecimal}. Received value is: ${minDecimal}`);
        }
        let k = this.comparedTo(minDecimal);
        return k < 0 ? minDecimal : this.comparedTo(maxDecimal) > 0 ? maxDecimal : new Decimal(this);
    }

    /**
     * Return true if the value of this Decimal is a finite number, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isFinite(): boolean {
        return !!this.digits;
    }

    /**
     * Return true if the value of this Decimal is an integer, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isInteger(): boolean {
        return !!this.digits && Math.floor(this.exponent / LOG_BASE) > this.digits!.length - 2;
    }

    /**
     * Return true if the value of this Decimal is NaN, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isNaN(): boolean {
        return !this.sign;
    }

    /**
     * Return true if the value of this Decimal is negative, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isNegative(): boolean {
        return this.sign < 0;
    }

    /**
     * Return true if the value of this Decimal is positive, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isPositive(): boolean {
        return this.sign > 0;
    }

    /**
     * Return true if the value of this Decimal is 0 or -0, otherwise return false.
     *
     * @returns { boolean } the boolean type
     */
    public isZero (): boolean {
        return !!this.digits && this.digits![0] === 0;
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal negated, i.e. as if multiplied by -1.
     *
     * @returns { Decimal } the Decimal type
     */
    public negate(): Decimal {
        let x = new Decimal(this);
        x.sign = -x.sign;
        return Utils.finaliseExternal(x);
    }

    /**
     * Return a string representing the value of this Decimal in base 2.
     *
     * @returns { string } the string type
     */
    public toBinary(): string {
        return this.toStringBinary(2);
    }

    /**
     * Return a string representing the value of this Decimal in base 2, round to `significantDigits`
     * significant digits.
     *
     * @param { number } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits` is out of range.
     */
    public toBinary(significantDigits: number): string {
        return this.toStringBinary(2, significantDigits);
    }

    /**
     * Return a string representing the value of this Decimal in base 2, round to `significantDigits`
     * significant digits using rounding mode `rounding`.
     *
     * @param { number } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits | rounding` is out of range.
     */
    public toBinary(significantDigits: number, rounding: Rounding): string {
        return this.toStringBinary(2, significantDigits, rounding);
    }

    /**
     * Return a string representing the value of this Decimal in base 8.
     *
     * @returns { string } the string type
     */
    public toOctal(): string {
        return this.toStringBinary(8);
    }

    /**
     * Return a string representing the value of this Decimal in base 8, round to `significantDigits` significant.
     *
     * @param { number } significantDigits {number | string | Decimal}
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits` is out of range.
     */
    public toOctal(significantDigits: number): string {
        return this.toStringBinary(8, significantDigits);
    }

    /**
     * Return a string representing the value of this Decimal in base 8, round to `significantDigits` significant
     * digits using rounding mode `rounding`.
     *
     * @param { number } significantDigits {number | string | Decimal}
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits | rounding` is out of range.
     */
    public toOctal(significantDigits: number, rounding: Rounding): string {
        return this.toStringBinary(8, significantDigits, rounding);
    }

    /**
     * Return a string representing the value of this Decimal in base 16
     *
     * @returns { string } the string type
     */
    public toHexadecimal(): string {
        return this.toStringBinary(16);
    }

    /**
     * Return a string representing the value of this Decimal in base 16, round to `significantDigits` significant.
     *
     * @param { number } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits` is out of range.
     */
    public toHexadecimal(significantDigits: number): string {
        return this.toStringBinary(16, significantDigits);
    }

    /**
     * Return a string representing the value of this Decimal in base 16, round to `significantDigits` significant
     * digits using rounding mode `rounding`.
     *
     * @param { number } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits | rounding` is out of range.
     */
    public toHexadecimal(significantDigits: number, rounding: Rounding): string {
        return this.toStringBinary(16, significantDigits, rounding);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public toDecimalPlaces(): Decimal {
        return new Decimal(this);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a maximum of `decimalPlaces`
     * decimal places.
     *
     * @param { number } decimalPlaces Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @returns { Decimal } the Decimal type
     */
    public toDecimalPlaces(decimalPlaces: number): Decimal {
        return this.toDecimalPlaces(decimalPlaces, Decimal.rounding);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a maximum of `decimalPlaces`
     * decimal places using rounding mode `rounding`.
     *
     * @param { number } decimalPlaces Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { Decimal } the Decimal type
     */
    public toDecimalPlaces(decimalPlaces: number, rounding: Rounding): Decimal {
        let decimal = this.toDecimalPlaces();
        Utils.checkInt32(decimalPlaces, 0, MAX_DIGITS);
        Utils.checkInt32(rounding, 0, 8);
        return Utils.finalise(decimal, decimalPlaces + decimal.e + 1, rounding);
    }

    /**
     * Return a string representing the value of this Decimal in exponential notation.
     *
     * @returns { string } the string type
     */
    public toExponential(): string {
        let str = this.finiteToString(true);
        return this.isNegative() && !this.isZero() ? '-' + str : str;
    }

    /**
     * Return a string representing the value of this Decimal in exponential notation rounded to
     * `decimalPlaces` fixed decimal places.
     *
     * @param { number } decimalPlaces Decimal places. Integer, 0 to MAX_DIGITS inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `decimalPlaces` is out of range.
     */
    public toExponential(decimalPlaces: number): string {
        return this.toExponential(decimalPlaces, Decimal.rounding);
    }

    /**
     * Return a string representing the value of this Decimal in exponential notation rounded to
     * `decimalPlaces` fixed decimal places using rounding mode `rounding`.
     *
     * @param { number } decimalPlaces Decimal places. Integer, 0 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `decimalPlaces | rounding` is out of range.
     */
    public toExponential(decimalPlaces: number, rounding: Rounding): string {
        Utils.checkInt32(decimalPlaces, 0, MAX_DIGITS);
        Utils.checkInt32(rounding, 0, 8);
        let x = Utils.finalise(new Decimal(this), decimalPlaces + 1, rounding);
        let str = x.finiteToString(true, decimalPlaces + 1);
        return x.isNegative() && !x.isZero() ? '-' + str : str;
    }

    /**
     * Return a string representing the value of this Decimal in normal (fixed-point).
     *
     * @returns { string } the string type
     */
    public toFixed(): string {
        let str = this.finiteToString(false);
        return this.isNegative() && !this.isZero() ? '-' + str : str;
    }

    /**
     * Return a string representing the value of this Decimal in normal (fixed-point) notation to
     * `decimalPlaces` fixed decimal places.
     *
     * @param { number } decimalPlaces Decimal places. Integer, 0 to MAX_DIGITS inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `decimalPlaces` is out of range.
     */
    public toFixed(decimalPlaces: number): string {
        return this.toFixed(decimalPlaces, Decimal.rounding);
    }

    /**
     * Return a string representing the value of this Decimal in normal (fixed-point) notation to
     * `decimalPlaces` fixed decimal places and rounded using rounding mode `rounding`.
     *
     * @param { number } decimalPlaces Decimal places. Integer, 0 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `decimalPlaces | rounding` is out of range.
     */
    public toFixed(decimalPlaces: number, rounding: Rounding): string {
        Utils.checkInt32(decimalPlaces, 0, MAX_DIGITS);
        Utils.checkInt32(rounding, 0, 8);
        let y = Utils.finalise(new Decimal(this), decimalPlaces + this.e + 1, rounding);
        let str = y.finiteToString(false, decimalPlaces + y.e + 1);
        return this.isNegative() && !this.isZero() ? '-' + str : str;
    }

    /**
     * Returns a new Decimal whose value is the nearest multiple of `n`.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public toNearest(n: DecimalValue): Decimal {
        return this.toNearest(n, Decimal.rounding);
    }

    /**
     * Returns a new Decimal whose value is the nearest multiple of `n` in the direction of rounding
     * mode `rounding`, to the value of this Decimal.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200001 - The value of `rounding` is out of range.
     */
    public toNearest(n: DecimalValue, rounding: Rounding): Decimal {
        let x = new Decimal(this);
        let y = new Decimal(n);
        Utils.checkInt32(rounding, 0, 8);

        // If x is not finite, return x if y is not NaN, else NaN.
        if (!x.d) {
            return y.s ? x : y;
        }

        // If y is not finite, return Infinity with the sign of x if y is Infinity, else NaN.
        if (!y.d) {
            if (y.s) {
                y.sign = x.s;
            }
            return y;
        }

        // If y is not zero, calculate the nearest multiple of y to x.
        if (y.d![0]) {
            external = false;
            x = Utils.divide(x, y, 0, rounding, true).mul(y);
            external = true;
            Utils.finalise(x, undefined, undefined);
        } else {
            // If y is zero, return zero with the sign of x.
            y.sign = x.s;
            x = y;
        }

        return x;
    }

    /**
     * Return a string representing the value of this Decimal.
     *
     * @returns { string } the string type
     */
    public toPrecision(): string {
        let str = this.finiteToString(this.e <= Decimal.toExpNeg || this.e >= Decimal.toExpPos);
        return this.isNegative() && !this.isZero() ? '-' + str : str;
    }

    /**
     * Return a string representing the value of this Decimal rounded to `significantDigits` significant digits.
     *
     * @param { number } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits` is out of range.
     */
    public toPrecision(significantDigits: number): string {
        return this.toPrecision(significantDigits, Decimal.rounding);
    }

    /**
     * Return a string representing the value of this Decimal rounded to `significantDigits` significant digits
     * using rounding mode `rounding`.
     *
     * @param { number } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { string } the string type
     * @throws { BusinessError } 10200001 - The value of `significantDigits | rounding` is out of range.
     */
    public toPrecision(significantDigits: number, rounding: Rounding): string {
        Utils.checkInt32(significantDigits, 1, MAX_DIGITS);
        Utils.checkInt32(rounding, 0, 8);
        let x = Utils.finalise(new Decimal(this), significantDigits, rounding);
        let str = x.finiteToString(significantDigits <= x.e || x.e <= Decimal.toExpNeg, significantDigits);
        return x.isNegative() && !x.isZero() ? '-' + str : str;
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal.
     *
     * @returns { Decimal } the Decimal type
     */
    public toSignificantDigits(): Decimal {
        return Utils.finalise(new Decimal(this), Decimal.precision, Decimal.rounding);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a maximum of `significantDigits`
     * significant digits.
     *
     * @param { number } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200001 - The value of `significantDigits` is out of range.
     */
    public toSignificantDigits(significantDigits: number): Decimal {
        return this.toSignificantDigits(significantDigits, Decimal.rounding);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal rounded to a maximum of `significantDigits`
     * significant digits using rounding mode `rounding`.
     *
     * @param { number } significantDigits Significant digits. Integer, 1 to MAX_DIGITS inclusive.
     * @param { Rounding } rounding Rounding mode. Integer, 0 to 8 inclusive.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200001 - The value of `significantDigits | rounding` is out of range.
     */
    public toSignificantDigits(significantDigits: number, rounding: Rounding): Decimal {
        Utils.checkInt32(significantDigits, 1, MAX_DIGITS);
        Utils.checkInt32(rounding, 0, 8);
        return Utils.finalise(new Decimal(this), significantDigits, rounding);
    }

    /**
     * Return the value of this Decimal converted to a number primitive. Zero keeps its sign.
     *
     * @returns { number } the number type
     */
    public toNumber(): number {
        return Utils.toNumber(this.valueOf());
    }

    /**
     * Return a string representing the value of this Decimal.
     * Return exponential notation if this Decimal has a positive exponent equal to or greater than
     * `toExpPos`, or a negative exponent equal to or less than `toExpNeg`.
     *
     * @returns { string } the string type
     */
    public toString(): string {
        let str = this.finiteToString(this.exponent <= Decimal.toExpNeg || this.exponent >= Decimal.toExpPos);
        return this.isNegative() && !this.isZero() ? '-' + str : str;
    }

    /**
     * Return the number of decimal places of the value of this Decimal.
     *
     * @returns { number } the number type
     */
    public decimalPlaces(): number {
        let digits = this.digits;
        let result: number = NaN;
        if (digits) {
            let len = digits!.length - 1;
            result = (len - Math.floor(this.exponent / LOG_BASE)) * LOG_BASE;
            let value = digits[len];
            if (value) {
                for (; value % 10 == 0; value /= 10) {
                    result--;
                }
            }
            if (result < 0) {
                result = 0;
            }
        }
        return result;
    }

    /**
     * Return a string representing the value of this Decimal.
     * Unlike `toString`, negative zero will include the minus sign.
     *
     * @returns { string } the string type
     */
    public valueOf(): string {
        let str = this.finiteToString(this.exponent <= Decimal.toExpNeg || this.exponent >= Decimal.toExpPos);
        return this.isNegative() ? '-' + str : str;
    }

    /**
     * Return a new Decimal whose value is the absolute value of `n`.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static abs(n: DecimalValue): Decimal {
        return new Decimal(n).abs();
    }

    /**
     * Return a new Decimal whose value is `n` round to an integer using `ROUND_FLOOR`.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static floor(n: DecimalValue): Decimal {
        return new Decimal(n).floor();
    }

    /**
     * Return a new Decimal whose value is `n` rounded to an integer using `ROUND_CEIL`.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static ceil(n: DecimalValue): Decimal {
        return new Decimal(n).ceil();
    }

    /**
     * Return a new Decimal whose value is `n` truncated to an integer.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static trunc(n: DecimalValue): Decimal {
        return new Decimal(n).trunc();
    }

    /**
     * Return a new Decimal whose value is `n` clamped to the range delineated by `min` and `max`.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @param { DecimalValue } min {number | string | Decimal}
     * @param { DecimalValue } max {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static clamp(n: DecimalValue, min: DecimalValue, max: DecimalValue): Decimal {
        return new Decimal(n).clamp(min, max);
    }

    /**
     * Return a new Decimal whose value is the sum of `x` and `y`, rounded to `precision` significant
     * digits using rounding mode `rounding`.
     *
     * @param { DecimalValue } x {number | string | Decimal}
     * @param { DecimalValue } y {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static add(x: DecimalValue, y: DecimalValue): Decimal {
        return new Decimal(x).add(y);
    }

    /**
     * Return a new Decimal whose value is the sum of the arguments, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * Only the result is rounded, not the intermediate calculations.
     *
     * @param { DecimalValue[] } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static sum(...values: DecimalValue[]): Decimal {
        let result = new Decimal(values[0]);
        external = false;
        for (let i = 1; i < values.length; i++) {
            if (result.isNaN()) {
                break;
            }
            result = result.add(values[i]);
        }
        external = true;
        return Utils.finalise(result, Decimal.precision, Decimal.rounding);
    }

    /**
     * Return a new Decimal whose value is `x` minus `y`, rounded to `precision` significant digits
     * using rounding mode `rounding`.
     *
     * @param { DecimalValue } x {number | string | Decimal}
     * @param { DecimalValue } y {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static sub(x: DecimalValue, y: DecimalValue): Decimal {
        return new Decimal(x).sub(y);
    }

    /**
     * Return a new Decimal whose value is `x` multiplied by `y`, rounded to `precision` significant
     * digits using rounding mode `rounding`.
     *
     * @param { DecimalValue } x {number | string | Decimal}
     * @param { DecimalValue } y {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static mul(x: DecimalValue, y: DecimalValue): Decimal {
        return new Decimal(x).mul(y);
    }

    /**
     * Return a new Decimal whose value is the square root of the sum of the squares of the arguments,
     * rounded to `precision` significant digits using rounding mode `rounding`.
     *
     * @param { DecimalValue[] } n {number | string | Decimal} Decimal
     * @returns { Decimal } the Decimal type
     */
    static hypot(...n: DecimalValue[]): Decimal {
        let t = new Decimal(0);
        external = false;
        for (let i = 0; i < n.length;) {
            let v = new Decimal(n[i++]);
            if (!v.d) {
                if (v.s) {
                    external = true;
                    return new Decimal(Infinity);
                }
                t = v;
            } else if (t.d) {
                t = t.add(v.mul(v));
            }
        }
        external = true;
        return t.sqrt();
    }

    /**
     * Return a new Decimal whose value is the maximum of the arguments.
     *
     * @param { DecimalValue[] } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static max(...values: DecimalValue[]): Decimal {
        let result = new Decimal(values[0]);
        for (let i = 1; i < values.length; i++) {
            let other = new Decimal(values[i]);
            if (other.isNaN()) {
                result = other;
                break;
            } else if (result.lessThan(other)) {
                result = other;
            }
        }
        return result;
    }

    /**
     * Return a new Decimal whose value is the minimum of the arguments.
     *
     * @param { DecimalValue[] } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static min(...values: DecimalValue[]): Decimal {
        let result = new Decimal(values[0]);
        for (let i = 1; i < values.length; i++) {
            let other = new Decimal(values[i]);
            if (other.isNaN()) {
                result = other;
                break;
            } else if (result.greaterThan(other)) {
                result = other;
            }
        }
        return result;
    }

    /**
     * Returns a new Decimal with a random value equal to or greater than 0 and less than 1.
     *
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200061 - Crypto unavailable
     */
    static random(): Decimal {
        return Utils.random();
    }

    /**
     * Returns a new Decimal with a random value equal to or greater than 0 and less than 1, and with
     * `significantDigits` significant digits (or less if trailing zeros are produced).
     *
     * @param { number } significantDigits Significant digits. Integer, 0 to MAX_DIGITS inclusive.
     * @returns { Decimal } the Decimal type
     * @throws { BusinessError } 10200061 - Crypto unavailable
     */
    static random(significantDigits: number): Decimal {
        return Utils.random(significantDigits);
    }

    /**
     * Return the sign of the passed value to the method.
     *   1    if x > 0,
     *  -1    if x < 0,
     *   0    if x is 0,
     *  -0    if x is -0,
     *   NaN  otherwise
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static sign(n: DecimalValue): number {
        let x = new Decimal(n);
        return x.d ? (x.d![0] ? x.s : 0 * x.s) : x.s || NaN;
    }

    /**
     * Return a new Decimal whose value is `n` rounded to an integer using rounding mode `rounding`.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    static round(n: DecimalValue): Decimal {
        let x = new Decimal(n)
        return Utils.finalise(x, x.e + 1, Decimal.rounding);
    }

    /**
     * Configures the 'global' settings for this particular Decimal constructor.
     *
     * @param { DecimalConfig } config object with one or more of the following properties,
     *   precision  {number}
     *   rounding   {number}
     *   toExpNeg   {number}
     *   toExpPos   {number}
     *   maxE       {number}
     *   minE       {number}
     *   modulo     {number}
     *   crypto     {boolean|number}
     *   defaults   {true}
     * @returns { void }
     * @throws { BusinessError } 10200001 - The value of `DecimalConfig.properties` is out of range.
     * @throws { BusinessError } 10200061 - Crypto unavailable
     */
    static set(config: DecimalConfig): void {
        let useDefaults = (config.defaults != undefined && config.defaults === true);
        Decimal.precision = Utils.checkRange(config.precision, 1, MAX_DIGITS, useDefaults, DEFAULTS_PRECISION);
        Decimal.rounding = Utils.checkRange(config.rounding, 0, 8, useDefaults, DEFAULTS_ROUNDING);
        Decimal.toExpNeg = Utils.checkRange(config.toExpNeg, -EXP_LIMIT, 0, useDefaults, DEFAULTS_TOEXPNEG);
        Decimal.toExpPos = Utils.checkRange(config.toExpPos, 0, EXP_LIMIT, useDefaults, DEFAULTS_TOEXPPOS);
        Decimal.maxE = Utils.checkRange(config.maxE, 0, EXP_LIMIT, useDefaults, EXP_LIMIT);
        Decimal.minE = Utils.checkRange(config.minE, -EXP_LIMIT, 0, useDefaults, -EXP_LIMIT);
        Decimal.modulo = Utils.checkRange(config.modulo, 0, 9, useDefaults, DEFAULTS_MODULO);

        if (useDefaults) {
            Decimal.crypto = false;
        }
        if (config.crypto != undefined) {
            if (config.crypto!) {
                throw Utils.createBusinessError(CRYPTO_UNAVAILABLE_ERROR_CODE, `Crypto unavailable`);
            } else {
                Decimal.crypto = false;
            }
        }
    }

    /**
     * Return a new Decimal whose value is the square root of this Decimal, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @returns { Decimal } the Decimal type
     */
    public sqrt(): Decimal {
        let x = this;
        let d = x.d;
        let s: number = x.s;

        if (s !== 1 || !d || !d![0]) {
            return new Decimal(!s || s < 0 && (!d || d![0]) ? NaN : d ? x : Infinity);
        }

        external = false;
        s = Math.sqrt(+(x.toNumber()));
        let n: string = '';
        let e: number = x.e;
        let r: Decimal;
        if (s == 0 || s == Infinity) {
            n = Utils.digitsToString(d);
            if ((n.length + e) % 2 == 0) {
                n += '0';
            }
            s = Math.sqrt(Utils.toNumber(n));
            e = Math.floor((e + 1) / 2) - ((e < 0 || e % 2) as number);
            if (s == Infinity) {
                n = '5e' + e;
            } else {
                n = Utils.toExponential(s);
                n = n.slice(0, n.indexOf('e') + 1) + e;
            }
            r = new Decimal(n);
        } else {
            r = new Decimal(s);
        }

        let sd = (e = Decimal.precision) + 3;
        let rep: number = 0;
        let m: boolean = false;
        while (true) {
            let t = r;
            r = t.add(Utils.divide(x, t, sd + 2, 1)).mul(0.5);
            if (Utils.digitsToString(t.d!).slice(0, sd) === (n = Utils.digitsToString(r.d!)).slice(0, sd)) {
                n = n.slice(sd - 3, sd + 1);
                if (n == '9999' || !rep && n == '4999') {
                    if (!rep) {
                        Utils.finalise(t, e + 1, 0);
                        if (t.mul(t).equals(x)) {
                            r = t;
                            break;
                        }
                    }
                    sd += 4;
                    rep = 1;
                } else {
                    if (Utils.toNumber(n) > 0 || Utils.toNumber(n.slice(1)) > 0
                        && (n.charAt(0) == c'5')) {
                        Utils.finalise(r, e + 1, 1);
                        m = !r.mul(r).equals(x);
                    }
                    break;
                }
            }
        }
        external = true;
        return Utils.finalise(r, e, Decimal.rounding, m);
    }

    /**
     * Return true if the value of this Decimal is equal to the value of `n`, otherwise return false.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { boolean } the boolean type
     */
    public equals(n: DecimalValue): boolean {
        return this.comparedTo(n) === 0;
    }

    /**
     * Return true if the value of this Decimal is greater than the value of `n`, otherwise return false.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { boolean } the boolean type
     */
    public greaterThan(n: DecimalValue): boolean {
        return this.comparedTo(n) > 0;
    }

    /**
     * Return true if the value of this Decimal is greater than or equal to the value of `n`,
     * otherwise return false.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { boolean } the boolean type
     */
    public greaterThanOrEqualTo(n: DecimalValue): boolean {
        let k = this.comparedTo(n);
        return k == 1 || k === 0;
    }

    /**
     * Return true if the value of this Decimal is less than `n`, otherwise return false.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { boolean } the boolean type
     */
    public lessThan(n: DecimalValue): boolean {
        return this.comparedTo(n) < 0;
    }

    /**
     * Return true if the value of this Decimal is less than or equal to `n`, otherwise return false.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { boolean } the boolean type
     */
    public lessThanOrEqualTo(n: DecimalValue): boolean {
        return this.comparedTo(n) <= 0;
    }

    /**
     * Return
     *   1    if the value of this Decimal is greater than the value of `n`,
     *  -1    if the value of this Decimal is less than the value of `n`,
     *   0    if they have the same value,
     *   NaN  if the value of either Decimal is NaN.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { number } the number type
     */
    public comparedTo(n: DecimalValue): number {
        let y = new Decimal(n);
        let xd = this.d;
        let yd = y.d;
        let xs = this.s;
        let ys = y.s;

        if (!xd || !yd) {
            return (!xs || !ys) ? NaN : (xs !== ys) ? xs : (xd === yd) ? 0 : (!xd ^ xs < 0) ? 1 : -1;
        }

        if (!xd![0] || !yd![0]) {
            return xd![0] ? xs : yd![0] ? -ys : 0;
        }

        if (xs !== ys) {
            return xs;
        }

        if (this.e !== y.e) {
            return this.e > y.e ^ xs < 0 ? 1 : -1;
        }

        let xdL = xd.length;
        let ydL = yd.length;

        for (let i = 0, j = xdL < ydL ? xdL : ydL; i < j; ++i) {
            if (xd![i] !== yd![i]) {
                return xd![i] > yd![i] ^ xs < 0 ? 1 : -1;
            }
        }
        return xdL === ydL ? 0 : xdL > ydL ^ xs < 0 ? 1 : -1;
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal divided by `n`, rounded to
     * `precision` significant digits using rounding mode `rounding`.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public div(n: DecimalValue): Decimal {
        return Utils.divide(this, new Decimal(n));
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal raised to the power `n`, rounded
     * to `precision` significant digits using rounding mode `rounding`.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public pow(n: DecimalValue): Decimal {
        let y = new Decimal(n);
        let yn = +y.toNumber();
        if (!this.d || !y.d || !this.d![0] || !y.d![0]) {
            return new Decimal(Math.pow(+this.toNumber(), yn));
        }

        let x = new Decimal(this);
        if (x.equals(1)) {
            return x;
        }

        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        if (y.equals(1)) {
            return Utils.finalise(x, pr, rm);
        }
        let e = Math.floor(y.e / LOG_BASE);
        let k: number;
        if (e >= y.d!.length - 1 && (k = yn < 0 ? -yn : yn) <= MAX_SAFE_INTEGER) {
            let r = this.intPow(x, k, pr);
            return y.s < 0 ? new Decimal(1).div(r) : Utils.finalise(r, pr, rm);
        }

        let s: number = x.s;
        if (s < 0) {
            if (e < y.d!.length - 1) {
                return new Decimal(NaN);
            }
            if ((y.d![e] & 1) == 0) {
                s = 1;
            }
            if (x.e == 0 && x.d![0] == 1 && x.d!.length == 1) {
                x.sign = s;
                return x;
            }
        }

        k = Math.pow(+x.toNumber(), yn);
        e = k == 0 || !isFinite(k)
          ? Math.floor(yn * (Math.log(Utils.toNumber('0.' + Utils.digitsToString(x.d!))) / Math.LN10 + x.e + 1))
          : new Decimal(k + '').e;

        if (e > Decimal.maxE + 1 || e < Decimal.minE - 1) {
            return new Decimal(e > 0 ? s / 0 : 0);
        }

        external = false;
        Decimal.rounding = x.sign = 1;
        k = Math.min(12, (e + '').length);
        let r = this.naturalExponential(y.mul(this.naturalLogarithm(x, pr + k)), pr);
        if (r.d) {
            r = Utils.finalise(r, pr + 5, 1);
            if (Utils.checkRoundingDigits(r.d!, pr, rm)) {
                e = pr + 10;
                r = Utils.finalise(this.naturalExponential(y.mul(this.naturalLogarithm(x, e + k)), e), e + 5, 1);
                if (Utils.toNumber(Utils.digitsToString(r.d!).slice(pr + 1, pr + 15)) + 1 == 1e14) {
                    r = Utils.finalise(r, pr + 1, 0);
                }
            }
        }
        r.sign = s;
        external = true;
        Decimal.rounding = rm;
        return Utils.finalise(r, pr, rm);
    }

    /**
     * Return a new Decimal whose value is the natural exponential of the value of this Decimal,
     * i.e. the base e raised to the power the value of this Decimal, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @returns { Decimal } the Decimal type
     */
    public exp(): Decimal {
        return this.naturalExponential(this);
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal plus `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public add(n: DecimalValue): Decimal {
        let y = new Decimal(n);
        if (!this.d || !y.d) {
            if (!this.s || !y.s) {
                y = new Decimal(NaN);
            } else if (!this.d) {
                y = new Decimal(y.d || this.s === y.s ? this : NaN);
            }
            return y;
        }

        if (this.s != y.s) {
            y.sign = -y.s;
            return this.sub(y);
        }
        let xd = this.d;
        let yd = y.d;
        let pr = Decimal.precision;
        let rm = Decimal.rounding;

        if (!xd![0] || !yd![0]) {
            if (!yd![0]) {
                y = new Decimal(this);
            }
            return external ? Utils.finalise(y, pr, rm) : y;
        }
        let k = Math.floor(this.e / LOG_BASE);
        let e = Math.floor(y.e / LOG_BASE);
        xd = xd!.slice();
        let i = k - e;
        let len: number;
        let d: Array<number>;
        if (i) {
            if (i < 0) {
                d = xd;
                i = -i;
                len = yd!.length;
            } else {
                d = yd!;
                e = k;
                len = xd.length;
            }
            k = Math.ceil(pr / LOG_BASE);
            len = k > len ? k + 1 : len + 1;
            if (i > len) {
                i = len;
                d.length = 1;
            }
            d.reverse();
            for (; i--;) {
                d.push(0);
            }
            d.reverse();
        }
        len = xd.length;
        i = yd!.length;
        if (len - i < 0) {
            i = len;
            d = yd!;
            yd = xd;
            xd = d;
        }
        let carry: number = 0;
        for (carry = 0; i;) {
            --i;
            xd[i] = xd[i] + yd![i] + carry;
            carry = (xd[i] / BASE) | 0;
            xd[i] = xd[i] % BASE;
        }

        if (carry) {
            xd.unshift(carry);
            ++e;
        }

        for (len = xd.length; xd[--len] == 0;) {
            xd.pop();
        }
        y.digits = xd;
        y.exponent = this.getBase10Exponent(xd, e);
        return external ? Utils.finalise(y, pr, rm) : y;
    }

    /**
     * Return a new Decimal whose value is the value of this Decimal minus `n`, rounded to `precision`
     * significant digits using rounding mode `rounding`.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public sub(n: DecimalValue): Decimal {
        let y: Decimal = new Decimal(n);
        let x = this;
        if (!x.d || !y.d) {
            if (!x.s || !y.s) {
                y = new Decimal(NaN);
            } else if (x.d) {
                y.sign = -y.s;
            } else {
                y = new Decimal(y.d || x.s !== y.s ? x : NaN);
            }
            return y;
        }

        if (x.s != y.s) {
            y.sign = -y.s;
            return x.add(y);
        }

        let xd = x.d;
        let yd = y.d;
        let pr = Decimal.precision;
        let rm = Decimal.rounding;
        if (!xd![0] || !yd![0]) {
            if (yd![0]) {
                y.sign = -y.s;
            } else if (xd![0]) {
                y = new Decimal(x);
            } else {
                return new Decimal(rm === 3 ? -0 : 0);
            }
            return external ? Utils.finalise(y, pr, rm) : y;
        }

        // Calculate base 1e7 exponents.
        let yExp = Math.floor(y.e / LOG_BASE);
        let xExp = Math.floor(x.e / LOG_BASE);
        xd = xd!.slice();
        let expDiff = xExp - yExp;
        let len: number;
        let i: number;
        let xLessThanY: boolean = false;
        if (expDiff) {
            xLessThanY = expDiff < 0;
            let d: Array<number>;
            if (xLessThanY) {
                d = xd!;
                expDiff = -expDiff;
                len = yd!.length;
            } else {
                d = yd!;
                yExp = xExp;
                len = xd!.length;
            }
            let i = Math.max(Math.ceil(pr / LOG_BASE), len) + 2;
            if (expDiff > i) {
                expDiff = i;
                d.length = 1;
            }
            d.reverse();
            for (i = expDiff; i--;) {
                d.push(0);
            }
            d.reverse();
        } else {
            let i = xd!.length;
            len = yd!.length;
            xLessThanY = i < len;
            if (xLessThanY) {
                len = i;
            }
            for (i = 0; i < len; i++) {
                if (xd![i] != yd![i]) {
                    xLessThanY = xd![i] < yd![i];
                    break;
                }
            }
            expDiff = 0;
        }

        if (xLessThanY) {
            let d = xd;
            xd = yd;
            yd = d;
            y.sign = -y.s;
        }
        len = xd!.length;
        for (i = yd!.length - len; i > 0; --i) {
            xd!.push(0);
            len++
        }
        let j: number;
        for (i = yd!.length; i > expDiff;) {
            if (xd![--i] < yd![i]) {
                for (j = i; j && xd![--j] === 0;) {
                    xd![j] = BASE - 1;
                }
                --xd![j];
                xd![i] = xd![i] + BASE;
            }
            xd![i] -= yd![i];
        }

        for (; xd![--len] === 0;) {
            xd!.pop();
        }

        for (; xd![0] === 0; xd!.shift()) {
            --yExp;
        }

        if (!xd![0]) {
            return new Decimal(rm === 3 ? -0 : 0);
        }
        y.digits = xd;
        y.exponent = this.getBase10Exponent(xd!, yExp);
        return external ? Utils.finalise(y, pr, rm) : y;
    }

    /**
     * Return a new Decimal whose value is this Decimal times `n`, rounded to `precision` significant
     * digits using rounding mode `rounding`.
     *
     * @param { DecimalValue } n {number | string | Decimal}
     * @returns { Decimal } the Decimal type
     */
    public mul(n: DecimalValue): Decimal {
        let xd = this.digits;
        let y = new Decimal(n);
        let yd = y.d;
        y.sign *= this.sign;
        if (!xd || !(xd![0]) || !yd || !(yd![0])) {
            return new Decimal((!y.s || xd && !(xd![0]) && !yd || yd && !(yd![0]) && !xd)
                ? NaN
                : (!xd || !yd) ? y.s / 0 : y.s * 0);
        }
        let e = Math.floor(this.exponent / LOG_BASE) + Math.floor(y.e / LOG_BASE);
        let xdL = xd!.length;
        let ydL = yd!.length;
        let r: Array<number> | null;
        let rL: number;
        if (xdL < ydL) {
          r = xd;
          xd = yd;
          yd = r;
          rL = xdL;
          xdL = ydL;
          ydL = rL;
        }
        r = new Array<number>();
        rL = xdL + ydL;
        let i: number;
        for (i = rL; i--;) {
            r.push(0);
        }

        let carry: number = 0;
        let k: number;
        let t: number;
        for (i = ydL; --i >= 0;) {
          carry = 0;
          for (k = xdL + i; k > i;) {
            t = r[k] + yd![i] * xd![k - i - 1] + carry;
            r[k--] = t % BASE | 0;
            carry = t / BASE | 0;
          }
          r[k] = (r[k] + carry) % BASE | 0;
        }

        for (; !r[--rL];) {
            r.pop();
        }
        if (carry) {
            ++e;
        } else {
            r.shift();
        }
        y.digits = r;
        y.exponent = this.getBase10Exponent(r, e);
        return external ? Utils.finalise(y, Decimal.precision, Decimal.rounding) : y;
    }

    private initializeByDecimal(v: Decimal) {
        this.sign = v.s;
        if (external) {
            if (!v.d || v.e > Decimal.maxE) {
                this.exponent = NaN;
                this.digits = null;
            } else if (v.e < Decimal.minE) {
                this.exponent = 0;
                this.digits = Utils.updateDigits(0);
            } else {
                this.exponent = v.e;
                this.digits = v.d!.slice();
            }
        } else {
            this.exponent = v.e;
            this.digits = v.d ? v.d!.slice() : v.d;
        }
    }

    private initializeByNumber(v: number) {
        if (v == 0) {
            this.sign = 1;
            this.exponent = 0;
            this.digits = Utils.updateDigits(0);
            return;
        }
        let tv = v;
        if (v < 0) {
            tv = -v;
            this.sign = -1;
        } else {
            this.sign = 1;
        }

        if (tv === ~~tv && tv < 1e7) {
            let e: number;
            let i: number;
            for (e = 0, i = tv; i >= 10; i /= 10) {
                e++;
            }
            if (external) {
                if (e > Decimal.maxE) {
                  this.exponent = NaN;
                  this.digits = null;
                } else if (e < Decimal.minE) {
                    this.exponent = 0;
                    this.digits = Utils.updateDigits(0);
                } else {
                    this.exponent = e;
                    this.digits = Utils.updateDigits(tv);
                }
            } else {
                this.exponent = e;
                this.digits = Utils.updateDigits(tv);
            }
            return;
        } else if (tv * 0 !== 0) {
            if (!tv) {
                this.sign = NaN;
            }
            this.exponent = NaN;
            this.digits = null;
            return;
        }
        this.parseDecimal(Utils.toString(tv));
    }

    private initializeByString(v: string) {
        let i: number;
        let str: string = v;
        if ((i = v.charCodeAt(0)) === 45) {
            str = v.slice(1);
            this.sign = -1;
        } else {
            if (i === 43) {
                str = v.slice(1);
            }
            this.sign = 1;
        }
        if (isDecimal.test(str)) {
            this.parseDecimal(str);
        } else {
            this.parseOther(str);
        }
    }

    private parseDecimal(str: String): void {
        let e: number;
        let i: number;
        let len: number;
        if ((e = str.indexOf('.')) > -1) {
            str = str.replace('.', '');
        }

        if ((i = str.search(new RegExp("e", "i"))) > 0) {
            if (e < 0) {
                e = i;
            }
            e += Utils.toNumber(str.slice(i + 1));
            str = str.substring(0, i);
        } else if (e < 0) {
            e = str.length;
        }

        for (i = 0; str.charCodeAt(i) === 48; i++);

        for (len = str.length; str.charCodeAt(len - 1) === 48; --len);
        str = str.slice(i, len);
        if (str) {
            len -= i;
            this.exponent = e = e - i - 1;
            this.digits = new Array<number>();
            i = (e + 1) % LOG_BASE;
            if (e < 0) {
                i += LOG_BASE;
            }
            if (i < len) {
                if (i) {
                    this.digits!.push(Utils.toNumber(str.slice(0, i)));
                }
                for (len -= LOG_BASE; i < len;) {
                    this.digits!.push(Utils.toNumber(str.slice(i, i += LOG_BASE)));
                }
                str = str.slice(i);
                i = LOG_BASE - str.length;
            } else {
                i -= len;
            }

            for (; i--;) {
                str += '0';
            }
            this.digits!.push(Utils.toNumber(str));
            if (external) {
                if (this.exponent > Decimal.maxE) {
                    this.digits = null;
                    this.exponent = NaN;
                } else if (this.exponent < Decimal.minE) {
                    this.exponent = 0;
                    this.digits = Utils.updateDigits(0);
                }
            }
        } else {
            this.exponent = 0;
            this.digits = Utils.updateDigits(0);
        }
        return;
    }

    private parseOther(str: string): void {
        if (str.indexOf('_') > -1) {
            str = str.replace(new RegExp("(\\d)_(?=\\d)", "g"), '$1');
            if (isDecimal.test(str)) {
                this.parseDecimal(str);
                return;
            }
        } else if (str === 'Infinity' || str === 'NaN') {
            if (!Utils.toNumber(str)) {
                this.sign = NaN;
            }
            this.exponent = NaN;
            this.digits = null;
            return;
        }

        let base: number;
        if (isHex.test(str))  {
            base = 16;
            str = str.toLowerCase();
        } else if (isBinary.test(str))  {
            base = 2;
        } else if (isOctal.test(str))  {
            base = 8;
        }  else {
            throw Utils.createBusinessError(TYPE_ERROR_CODE,
                `The type of "test(str)" must be Hex/Binary/Octal. Received value is: ${str}`);
        }

        let i = str.search(new RegExp("p", "i"));
        let p: number = 0;
        if (i > 0) {
            p = Utils.toNumber(str.slice(i + 1));
            str = str.substring(2, i);
        } else {
            str = str.slice(2);
        }

        i = str.indexOf('.');
        let isFloat: boolean = i >= 0;
        let len: number = 0;
        let divisor: Decimal = new Decimal(0);
        if (isFloat) {
            str = str.replace('.', '');
            len = str.length;
            i = len - i;
            divisor = this.intPow(new Decimal(base), i, i * 2);
        }
        let xd = this.convertBase(str, base, BASE);
        let xe = xd.length - 1;
        for (i = xe; i >= 0 && xd[i] === 0; --i) {
            xd.pop();
        }
        if (i < 0) {
            this.sign = this.sign < 0 ? -1 : 1;
            this.exponent = 0;
            this.digits = Utils.updateDigits(0);
            return;
        }
        this.exponent = this.getBase10Exponent(xd, xe);
        this.digits = xd;
        external = false;
        if (isFloat) {
            let x = Utils.divide(this, divisor, len * 4);
            this.initializeByDecimal(x);
        }

        if (p) {
            let x = this.mul(Math.abs(p) < 54 ? Math.pow(2, p) : Utils.pow(2, p));
            this.initializeByDecimal(x)
        }
        external = true;
        return;
    }

    private intPow(x: Decimal, n: number, pr: number): Decimal {
        let r = new Decimal(1);
        let k = Math.ceil(pr / LOG_BASE + 4);
        external = false;
        let isTruncated: boolean = false;
        while (true) {
            if (n % 2) {
                r = r.mul(x);
                if (this.truncate(r.d!, k)) {
                    isTruncated = true;
                }
            }

            n = Math.floor(n / 2);
            if (n === 0) {
                n = r.d!.length - 1;
                if (isTruncated && r.d![n] === 0) {
                    ++r.d![n];
                }
                break;
            }
            x = x.mul(x);
            this.truncate(x.d!, k);
        }
        external = true;
        return r;
    }

    private getBase10Exponent(digits: Array<number>, e: number): number {
        let w = digits[0];
        for (e *= LOG_BASE; w >= 10; w /= 10) {
            e++;
        }
        return e;
    }

    private truncate(arr: Array<number>, len: number): boolean {
        if (arr.length > len) {
            arr.length = len;
            return true;
        }
        return false;
    }

    private convertBase(str: string, baseIn: number, baseOut: number): Array<number> {
        let arrL: number;
        let arr: Array<number> = Utils.updateDigits(0);
        for (let i = 0; i < str.length;) {
            for (arrL = arr.length; arrL--;) {
               arr[arrL] = arr[arrL] * baseIn;
            }
            arr[0] = arr[0] + NUMERALS.indexOf(str.charAt(i++));
            for (let j = 0; j < arr.length; j++) {
               if (arr[j] > baseOut - 1) {
                    if ((j + 1) == arr.length) {
                        arr.push(0);
                    }
                    arr[j + 1] = arr[j + 1] + (arr[j] / baseOut | 0);
                    arr[j] = arr[j] % baseOut;
                }
            }
        }
        return arr.reverse();
    }

    internal finiteToString(isExp: boolean, sd?: number): string {
        if (!this.isFinite()) {
            return this.nonFiniteToString();
        }
        let k: number;
        let e = this.exponent;
        let str: string = Utils.digitsToString(this.d!);
        let len: number = str.length;
        if (isExp) {
          if (sd && (k = sd - len) > 0) {
            str = str.charAt(0) + '.' + str.slice(1) + Utils.getZeroString(k);
          } else if (len > 1) {
            str = str.charAt(0) + '.' + str.slice(1);
          }
          str = str + (this.e < 0 ? 'e' : 'e+') + this.e;
        } else if (e < 0) {
          str = '0.' + Utils.getZeroString(-e - 1) + str;
          if (sd && (k = sd - len) > 0) str += Utils.getZeroString(k);
        } else if (e >= len) {
          str += Utils.getZeroString(e + 1 - len);
          if (sd && (k = sd - e - 1) > 0) str = str + '.' + Utils.getZeroString(k);
        } else {
          if ((k = e + 1) < len) str = str.slice(0, k) + '.' + str.slice(k);
          if (sd && (k = sd - len) > 0) {
            if (e + 1 === len) str += '.';
            str += Utils.getZeroString(k);
          }
        }
        return str;
    }

    private nonFiniteToString(): String {
        return String(this.sign * this.sign / 0);
    }

    private naturalExponential(x: Decimal, sd?: number): Decimal {
        let rm = Decimal.rounding;
        let pr = Decimal.precision;
        if (!x.d || !x.d![0] || x.e > 17) {
            return new Decimal(x.d
                ? !x.d![0] ? 1 as number : x.s < 0 ? 0 as number : Infinity
                : x.s ? x.s < 0 ? 0 as number : x : NaN);
        }

        let wpr: number;
        if (sd == undefined) {
            external = false;
            wpr = pr;
        } else {
            wpr = sd;
        }
        let t = new Decimal(0.03125);
        let k: number = 0;
        while (x.e > -2) {
            x = x.mul(t);
            k += 5;
        }

        // Use 2 * log10(2^k) + 5 (empirically derived) to estimate the increase in precision
        // necessary to ensure the first 4 rounding digits are correct.
        let guard = Math.log(Math.pow(2, k)) / Math.LN10 * 2 + 5 | 0;
        wpr += guard;
        let denominator = new Decimal(1);
        let pow = new Decimal(1);
        let sum = new Decimal(1);
        Decimal.precision = wpr;
        let i = 0;
        while (true) {
            pow = Utils.finalise(pow.mul(x), wpr, 1);
            denominator = denominator.mul(++i);
            t = sum.add(Utils.divide(pow, denominator, wpr, 1));
            if (Utils.digitsToString(t.d!).slice(0, wpr) === Utils.digitsToString(sum.d!).slice(0, wpr)) {
                let j = k;
                while (j--) {
                    sum = Utils.finalise(sum.mul(sum), wpr, 1);
                }

                if (sd == undefined) {
                    let rep: number = 0;
                    if (rep < 3 && Utils.checkRoundingDigits(sum.d!, wpr - guard, rm, rep)) {
                        Decimal.precision = wpr += 10;
                        denominator = pow = t = new Decimal(1);
                        i = 0;
                        rep++;
                    } else {
                        return Utils.finalise(sum, Decimal.precision = pr, rm, external = true);
                    }
                } else {
                    Decimal.precision = pr;
                    return sum;
                }
            }
            sum = t;
        }
    }

    private naturalLogarithm(y: Decimal, sd?: number): Decimal {
        let x = y;
        let xd = x.d;
        let rm = Decimal.rounding;
        let n: number = 1;
        if (x.s < 0 || !xd || !xd![0] || !x.e && xd![0] == 1 && xd!.length == 1) {
            return new Decimal(xd && !xd![0] ? -Infinity : x.s != 1 ? NaN : xd ? 0 as number : x);
        }
        let wpr: number;
        let pr = Decimal.precision;
        if (sd == null) {
            external = false;
            wpr = pr;
        } else {
            wpr = sd;
        }
        let guard: number = 10;
        Decimal.precision = wpr += guard;
        let c = Utils.digitsToString(xd!);
        let ch = new Char(c.charAt(0));
        let c0 = Utils.toNumber(ch.toString());
        let e: number;
        if (Math.abs(e = x.e) < 1.5e15) {
            let n: number = 1;
            while (c0 < 7 && c0 != 1 || c0 == 1 && (Utils.toNumber((new Char(c.charAt(1))).toString())) > 3) {
                x = x.mul(y);
                c = Utils.digitsToString(x.d!);
                c0 = Utils.toNumber((new Char(c.charAt(0))).toString());
                n++;
            }
            e = x.e;
            if (c0 > 1) {
                x = new Decimal('0.' + c);
                e++;
            } else {
                x = new Decimal(c0 + '.' + c.slice(1));
            }
        } else {
            let t = this.getLn10(wpr + 2, pr).mul(e + '');
            x = this.naturalLogarithm(new Decimal(c0 + '.' + c.slice(1)), wpr - guard).add(t);
            Decimal.precision = pr;
            return sd == null ? Utils.finalise(x, pr, rm, external = true) : x;
        }
        let x1 = x;
        let numerator: Decimal;
        let sum = numerator = x = Utils.divide(x.sub(1), x.add(1), wpr, 1);
        let x2 = Utils.finalise(x.mul(x), wpr, 1);
        let denominator: number = 3;
        while (true) {
            numerator = Utils.finalise(numerator.mul(x2), wpr, 1);
            let t = sum.add(Utils.divide(numerator, new Decimal(denominator), wpr, 1));
            if (Utils.digitsToString(t.d!).slice(0, wpr) === Utils.digitsToString(sum.d!).slice(0, wpr)) {
                sum = sum.mul(2);
                if (e !== 0) {
                    sum = sum.add(this.getLn10(wpr + 2, pr).mul(e + ''));
                }
                sum = Utils.divide(sum, new Decimal(n), wpr, 1);
                let rep: number = 0;
                if (sd == null) {
                    if (Utils.checkRoundingDigits(sum.d!, wpr - guard, rm, rep)) {
                        Decimal.precision = wpr += guard;
                        t = numerator = x = Utils.divide(x1.sub(1), x1.add(1), wpr, 1);
                        x2 = Utils.finalise(x.mul(x), wpr, 1);
                        denominator = rep = 1;
                    } else {
                        return Utils.finalise(sum, Decimal.precision = pr, rm, external = true);
                    }
                } else {
                    Decimal.precision = pr;
                    return sum;
                }
          }
          sum = t;
          denominator += 2;
        }
    }

    private getLn10(sd: number, pr?: number): Decimal {
        if (sd > LN10_PRECISION) {
            external = true;
            if (pr != undefined) {
                Decimal.precision = pr;
            }
            throw Utils.createBusinessError(PRECISION_LIMIT_EXCEEDED_ERROR_CODE,
                `Precision limit exceeded, "sd" must be <= LN10_PRECISION`);
        }
        return Utils.finalise(new Decimal(LN10), sd, 1, true);
    }

    private getDigitsValidIndex(digits: Array<number>): number {
        let len: number = 0;
        for (len = digits.length; !digits[len - 1]; --len);
        return len;
    }

    private digitsToString(digits: Array<number>, start: number, orgStr: string, length?: number): string {
        let len: number;
        let str: string = orgStr;
        if (length != undefined) {
            len = length;
        } else {
            len = this.getDigitsValidIndex(digits);
        }

        for (let i = start; i < len; i++) {
            str += NUMERALS.charAt(digits[i]);
        }
        return str;
    }

    private completeStringWithZero(isExp: boolean, len: number, baseOut: number,
        orgStr: string, base: number, exp: number): string {
        let str = orgStr;
        if (isExp) {
            if (len > 1) {
                if (baseOut == 16 || baseOut == 8) {
                    let i = baseOut == 16 ? 4 : 3;
                    for (let j = len - 1; j % i; j++) {
                        str += '0';
                    }
                    let xd = this.convertBase(str, base, baseOut);
                    str = this.digitsToString(xd, 1, '1.');
                } else {
                    str = str.charAt(0) + '.' + str.slice(1);
                }
            }
            str =  str + (exp < 0 ? 'p' : 'p+') + exp;
        } else if (exp < 0) {
            while (++exp != 0) {
                str = '0' + str;
            }
            str = '0.' + str;
        } else {
            if (++exp > len) {
                for (exp -= len; exp != 0; exp--) {
                    str += '0';
                }
            } else if (exp < len) {
                str = str.slice(0, exp) + '.' + str.slice(exp);
            }
        }
        return str;
    }

    private toStringBinary(baseOut: number, sd?: number, rm?: number) {
        let isExp = (sd != undefined);
        if (isExp) {
            Utils.checkInt32(sd!, 1, MAX_DIGITS);
            if (rm == undefined) {
                rm = Decimal.rounding;
            } else {
                Utils.checkInt32(rm!, 0, 8);
            }
        } else {
            sd = Decimal.precision;
            rm = Decimal.rounding;
        }

        if (!this.isFinite()) {
            let str = this.nonFiniteToString();
            return this.sign < 0 ? '-' + str : str;
        } else {
            let str = this.finiteToString(false);
            let i = str.indexOf('.');
            let base: number = baseOut;
            if (isExp) {
                base = 2;
                if (baseOut == 16) {
                    sd = sd! * 4 - 3;
                } else if (baseOut == 8) {
                    sd = sd! * 3 - 2;
                }
            }

            let x: Decimal = new Decimal(this);
            let y: Decimal = new Decimal(1);
            if (i >= 0) {
                str = str.replace('.', '');
                y.exponent = str.length - i;
                y.digits = this.convertBase(y.finiteToString(false), 10, base);
                y.exponent = y.d!.length;
            }
            let xd = this.convertBase(str, 10, base);
            let e = xd.length;
            let len = xd.length;
            for (; len >= 1 && xd[--len] == 0;) {
                xd.pop();
            }

            let roundUp: boolean = false;
            if (xd.length == 0 || !xd[0]) {
                str = isExp ? '0p+0' : '0';
            } else {
                if (i < 0) {
                    e--;
                } else {
                    x.digits = xd;
                    x.exponent = e;
                    x = Utils.divide(x, y, sd, rm, false, base);
                    xd = x.d!;
                    e = x.e;
                    roundUp = inexact;
                }
          
                i = sd! < xd.length ? xd[sd!] : -1;
                let k = base / 2;
                roundUp = roundUp || (sd! + 1 < xd.length ? xd[sd! + 1] != undefined : false);
                roundUp = rm != undefined && rm < 4
                    ? roundUp && (rm === 0 || rm === (x.s < 0 ? 3 : 2))
                    : i > k || i === k && (rm === 4 || roundUp || rm === 6 && (xd[sd! - 1] & 1) > 0 ||
                        rm === (x.s < 0 ? 8 : 7));

                // ArkTs1.2's Array can't change length to bigger or negative.
                if (sd! <= xd.length) {
                    xd.length = sd!;
                } else {
                    xd.extendTo(sd!, 0);
                }     

                if (roundUp) {
                    sd!--;
                    xd[sd!] += 1
                    for (; xd[sd!] > (base - 1);) {
                        xd[sd!] = 0;
                        if (!sd) {
                            ++e;
                            xd.unshift(1);
                        }
                        sd!--;
                        if (sd! < 0) {
                            break;
                        }
                        xd[sd!] += 1
                    }
                }

                len = this.getDigitsValidIndex(xd);
                str = this.digitsToString(xd, 0, '', len);
                str = this.completeStringWithZero(isExp, len, baseOut, str, base, e);
            }
            str = (baseOut == 16 ? '0x' : baseOut == 2 ? '0b' : baseOut == 8 ? '0o' : '') + str;
            return x.s < 0 ? '-' + str : str;
        }
    }
}

class Utils {
    public static isDecimalInstance(obj: Object): boolean {
        return obj instanceof Decimal
            || (obj.hasOwnProperty('toStringTag') && (obj as Decimal).toStringTag === tag) || false;
    }

    public static getZeroString(k: number): string {
        let zs: string = '';
        for (; k--;) {
            zs += '0';
        }
        return zs;
    }

    public static digitsToString(d: Array<number>): string {
        let indexOfLastWord: number = d.length - 1;
        let str: string = '';
        let w: number = d[0];
        if (indexOfLastWord > 0) {
            str += w;
            let i: number;
            let k: number;
            let ws: string;
            for (i = 1; i < indexOfLastWord; i++) {
                ws = d[i] + '';
                k = LOG_BASE - ws.length;
                if (k) {
                    str += Utils.getZeroString(k);
                }
                str += ws;
            }
            w = d[i];
            ws = w + '';
            k = LOG_BASE - ws.length;
            if (k) {
                str += Utils.getZeroString(k);
            }
        } else if (w === 0) {
            return '0';
        }
        for (; w % 10 === 0;) {
            w /= 10;
        }
        return str + w;
    }

    public static checkInt32(i: number, min: number, max: number): void {
        if (i !== ~~i || i < min || i > max) {
            throw Utils.createBusinessError(RANGE_ERROR_CODE,
                `The value of "${i}" is out of range. It must be >= ${min} && <= ${max} . Received value is: ${i}`);
        }
    }

    public static multiplyInteger(x: Array<number>, k: number, base: number): Array<number> {
        let temp: number = 0;
        let carry: number = 0;
        let i: number = x.length;
        for (x = x.slice(); i--;) {
            temp = x[i] * k + carry;
            x[i] = temp % base | 0;
            carry = temp / base | 0;
        }
        if (carry) {
            x.unshift(carry);
        }
        return x;
    }

    public static compare(a: Array<number>, b: Array<number>, aL: number, bL: number): number {
        let r: number = 0;
        if (aL != bL) {
            r = aL > bL ? 1 : -1;
        } else {
          for (let i = r = 0; i < aL; i++) {
            if (a[i] != b[i]) {
                r = a[i] > b[i] ? 1 : -1;
                break;
            }
          }
        }
        return r;
    }

    public static subtract(a: Array<number>, b: Array<number>, aL: number, base: number): void {
        let i: number = 0;
        for (; aL--;) {
            a[aL] = a[aL] - i;
            i = a[aL] < b[aL] ? 1 : 0;
            a[aL] = i * base + a[aL] - b[aL];
        }
        for (; !a[0] && a.length > 1;) {
            a.shift();
        }
    }

    public static pow(x: number, y: number): Decimal {
        return new Decimal(x).pow(y);
    }

    /**
     * Check 5 rounding digits if `repeating` is undefined, 4 otherwise.
     * `repeating == undefined` if caller is `log` or `pow`,
     * `repeating != undefined` if caller is `naturalLogarithm` or `naturalExponential`.
     */
    public static checkRoundingDigits(d: Array<number>, i: number, rm: number, repeating?: number): boolean {
        let k: number;
        // Get the length of the first word of the array d.
        for (k = d[0]; k >= 10; k /= 10) {
            --i;
        }
        let di: number;
        // Is the rounding digit in the first word of d?
        if (--i < 0) {
            i += LOG_BASE;
            di = 0;
        } else {
            di = Math.ceil((i + 1) / LOG_BASE);
            i %= LOG_BASE;
        }
        // i is the index (0 - 6) of the rounding digit.
        // E.g. if within the word 3487563 the first rounding digit is 5,
        // then i = 4, k = 1000, rd = 3487563 % 1000 = 563
        k = Math.pow(10, LOG_BASE - i);
        let rd = d[di] % k | 0;
        let r: boolean = false;
        if (repeating == undefined) {
            if (i < 3) {
                if (i == 0) {
                    rd = rd / 100 | 0;
                } else if (i == 1) {
                    rd = rd / 10 | 0;
                }
                // Is the rounding digits are 0, 99999, 49999 or 50000
                r = rm < 4 && rd == 99999 || rm > 3 && rd == 49999 || rd == 50000 || rd == 0;
            } else {
                r = (rm < 4 && rd + 1 == k || rm > 3 && rd + 1 == k / 2) &&
                    (d[di + 1] / k / 100 | 0) == Math.pow(10, i - 2) - 1 ||
                    (rd == k / 2 || rd == 0) && (d[di + 1] / k / 100 | 0) == 0;
            }
        } else {
            if (i < 4) {
                if (i == 0) {
                    rd = rd / 1000 | 0;
                } else if (i == 1) {
                    rd = rd / 100 | 0;
                } else if (i == 2) {
                    rd = rd / 10 | 0;
                }
                r = (repeating > 0 || rm < 4) && rd == 9999 || !repeating && rm > 3 && rd == 4999;
            } else {
                r = ((repeating > 0 || rm < 4) && rd + 1 == k ||
                    (!repeating && rm > 3) && rd + 1 == k / 2) &&
                    (d[di + 1] / k / 1000 | 0) == Math.pow(10, i - 3) - 1;
            }
        }
        return r;
    }

    public static updateDigits(v?: number): Array<number> {
        let res = new Array<number>();
        if (v != undefined) {
            res.push(v!);
        }
        return res;
    }

    public static toNumber(s: string): number {
        return +Number.parseFloat(s);
    }

    public static toExponential(n: number): String {
        return new Double(n).toExponential();
    }

    public static toString(n: number): String {
        return new String(n);
    }

    public static checkRange(v: number | undefined, min: number, max: number, useDef: boolean, def: number): number {
        if (v == undefined && useDef) {
            return def;
        }
        if (v != undefined) {
            if (Math.floor(v) === v && v >= min && v <= max) {
                return v!;
            } else {
                throw Utils.createBusinessError(RANGE_ERROR_CODE,
                    `The value of "${v!}" is out of range. It must be >= ${min} && <= ${max}. Received value is: ${v!}`
                );
            }
        }
        return def;
    }

    /**
     * Round `x` to `sd` significant digits using rounding mode `rm`.
     * Check for over/under-flow.
     */
    public static finalise(x: Decimal, sd: number | undefined, rm: number | undefined,
        isTruncated: boolean = false): Decimal {
        // Don't round if sd is undefined.
        if (sd != undefined) {
            let xd = x.d;
            // Infinity/NaN.
            if (!xd) {
                return x;
            }

            // digits: the number of digits of w.
            let digits: number;
            let k: number;
            for (digits = 1, k = xd![0]; k >= 10; k /= 10) {
                digits++;
            }
            let signDigits: number = sd!;
            // i: what would be the index of rd within w if all the numbers were 7 digits long (i.e. if
            // they had leading zeros)
            let i = sd - digits;
            // j: if > 0, the actual index of rd within w (if < 0, rd is a leading zero).
            let j: number;
            // w: the word of xd containing rd, a base 1e7 number.
            let w: number;
            // xdi: the index of w within xd.
            let xdi: number = 0;
            // rd: the rounding digit, i.e. the digit after the digit that may be rounded up.
            let rd: number;
            if (i < 0) {
                i += LOG_BASE;
                j = signDigits;
                w = xd[xdi];
                // Get the rounding digit at index j of w.
                rd = w / Math.pow(10, digits - j - 1) % 10 | 0;
            } else {
                xdi = Math.ceil((i + 1) / LOG_BASE);
                k = xd.length;
                if (xdi >= k) {
                    if (isTruncated) {
                        for (; k++ <= xdi;) {
                            xd!.push(0);
                        }
                        w = rd = 0;
                        digits = 1;
                        i %= LOG_BASE;
                        j = i - LOG_BASE + 1;
                    } else {
                        return Utils.finaliseExternal(x);
                    }
                } else {
                    w = k = xd[xdi];
                    for (digits = 1; k >= 10; k /= 10) {
                        digits++;
                    }

                    i %= LOG_BASE;
                    j = i - LOG_BASE + digits;
                    rd = j < 0 ? 0 : w / Math.pow(10, digits - j - 1) % 10 | 0;
                }
            }

            // Are there any non-zero digits after the rounding digit?
            isTruncated = isTruncated || signDigits < 0
                || ((xdi + 1) < xd!.length && xd![xdi + 1] !== undefined)
                || (j < 0 ? w : w % Math.pow(10, digits - j - 1)) > 0;

            let roundUp = (rm != undefined && rm! < 4)
                ? (rd || isTruncated) && (rm == 0 || rm == (x.s < 0 ? 3 : 2))
                : rd > 5 || rd == 5 && (rm == 4 || isTruncated || rm == 6 &&
                ((i > 0 ? j > 0 ? w / Math.pow(10, digits - j) : 0 : xdi - 1 >= 0 ? xd![xdi - 1] : NaN) % 10) & 1 ||
                rm == (x.s < 0 ? 8 : 7));

            if (signDigits < 1 || !(xd![0])) {
                xd!.length = 0;
                if (roundUp) {
                    signDigits -= x.e + 1;
                    xd!.push(Math.pow(10, (LOG_BASE - signDigits % LOG_BASE) % LOG_BASE));
                    x.exponent = -signDigits || 0;
                } else {
                    xd!.push(x.exponent = 0);
                }
                return x;
            }

            if (i == 0) {
                xd!.length = xdi;
                k = 1;
                xdi--;
            } else {
               xd!.length = xdi + 1;
               k = Math.pow(10, LOG_BASE - i);
               xd![xdi] = j > 0 ? (w / Math.pow(10, digits - j) % Math.pow(10, j) | 0) * k : 0;
            }

            if (roundUp) {
                while (true) {
                    if (xdi == 0) {
                        for (i = 1, j = xd![0]; j >= 10; j /= 10) {
                            i++;
                        }
                        xd![0] = xd![0] + k;
                        j = xd![0];
                        for (k = 1; j >= 10; j /= 10) {
                            k++;
                        }
                        if (i != k) {
                            x.exponent++;
                            if (xd![0] == BASE) {
                                xd![0] = 1;
                            }
                        }
                        break;
                    } else {
                        xd![xdi] = xd![xdi] + k;
                        if (xd![xdi] != BASE) {
                            break;
                        }
                        xd![xdi--] = 0;
                        k = 1;
                    }
                }
            }
            for (i = xd.length; xd![--i] === 0;) {
                xd!.pop();
            }
        }
        return Utils.finaliseExternal(x);
    }

    public static finaliseExternal(x: Decimal): Decimal {
        if (external) {
            if (x.e > Decimal.maxE) {
                x.digits = null;
                x.exponent = NaN;
            } else if (x.e < Decimal.minE) {
                x.exponent = 0;
                x.digits = Utils.updateDigits(0);
            }
        }
        return x;
    }

    /**
     * Return a new Decimal whose value is `x` divided by `y`, rounded to
     * `precision` significant digits using rounding mode `rounding`.
     *
     * @param { Decimal } x dividend
     * @param { Decimal } y divisor
     * @param { number } pr rounding precision
     * @param { number } rm rounding mode
     * @param { boolean } dp the flag of decimal places
     * @param { number } base division in the specified base, {2 | 8 | 10 | 16}
     * @returns { Decimal } the Decimal type
     */
    public static divide(x: Decimal, y: Decimal, pr?: number, rm?: number, dp?: boolean, base?: number): Decimal {
        let sign = x.s == y.s ? 1 : -1;
        let xd = x.d;
        let yd = y.d;
        if (!xd || !xd![0] || !yd || !yd![0]) {
            return new Decimal(
                !x.s || !y.s || (xd ? yd && xd![0] == yd![0] : !yd) ? NaN :
                xd && xd![0] == 0 || !yd ? sign * 0 : sign / 0);
        }
        let logBase: number;
        let e: number;
        if (base) {
            logBase = 1;
            e = x.e - y.e;
        } else {
            base = BASE;
            logBase = LOG_BASE;
            e = Math.floor(x.e / logBase) - Math.floor(y.e / logBase);
        }
        let yL = yd.length;
        let xL = xd.length;
        let q = new Decimal(sign);
        let qd = new Array<number>();
        q.digits = qd;
        let i: number;
        for (i = 0; i < yL && yd![i] == (xd![i] || 0); i++);

        if (yd![i] > (xd![i] || 0)) {
            e--;
        }
        let sd: number;
        if (pr == undefined) {
            sd = pr = Decimal.precision;
            rm = Decimal.rounding;
        } else if (dp) {
            sd = pr + (x.exponent - y.e) + 1;
        } else {
            sd = pr;
        }
        let more: boolean;
        let k: number;
        if (sd < 0) {
            qd.push(1);
            more = true;
        } else {
            sd = sd / logBase + 2 | 0;
            i = 0;
            if (yL == 1) {
                k = 0;
                let ydv = yd![0];
                sd++;
                let t: number;
                for (; (i < xL || k) && sd--; i++) {
                    t = k * base + (i < xd!.length ? (xd![i] || 0) : 0);
                    qd.push(t / ydv | 0);
                    k = t % ydv | 0;
                }
                more = (k || i) < xL;
            } else {
                k = base / (yd![0] + 1) | 0;
                if (k > 1) {
                    yd = Utils.multiplyInteger(yd, k, base);
                    xd = Utils.multiplyInteger(xd, k, base);
                    yL = yd!.length;
                    xL = xd!.length;
                }
                let xi = yL;
                let rem: Array<number> | undefined = xd!.slice(0, yL);
                let remL = rem.length;
                for (; remL < yL;) {
                    if (remL == rem.length) {
                        rem.push(0);
                        remL++;
                    } else {
                        rem[remL++] = 0;
                    }
                }
                let yz = yd!.slice();
                yz.unshift(0);
                let yd0 = yd![0];
                if (yd![1] >= base / 2) {
                    ++yd0;
                }
                do {
                    k = 0;
                    let cmp = Utils.compare(yd, rem!, yL, remL);
                    if (cmp < 0) {
                        let rem0 = rem![0];
                        if (yL != remL) {
                            rem0 = rem0 * base + (rem![1] || 0);
                        }
                        k = rem0 / yd0 | 0;
                        let prod: Array<number>;
                        if (k > 1) {
                            if (k >= base) {
                                k = base - 1;
                            }
                            prod = Utils.multiplyInteger(yd, k, base);
                            let prodL = prod.length;
                            remL = rem!.length;
                            cmp = Utils.compare(prod, rem!, prodL, remL);
                            if (cmp == 1) {
                                k--;
                                Utils.subtract(prod, yL < prodL ? yz : yd, prodL, base);
                            }
                        } else {
                            if (k == 0) {
                                cmp = k = 1;
                            }
                            prod = yd!.slice();
                        }
                        let prodL = prod.length;
                        if (prodL < remL) {
                            prod.unshift(0);
                        }
                        Utils.subtract(rem!, prod, remL, base);
                        if (cmp == -1) {
                            remL = rem!.length;
                            cmp = Utils.compare(yd, rem!, yL, remL);
                            if (cmp < 1) {
                                k++;
                                Utils.subtract(rem!, yL < remL ? yz : yd, remL, base);
                            }
                        }
                        remL = rem!.length;
                    } else if (cmp === 0) {
                        k++;
                        rem = Utils.updateDigits(0);
                    }
                    qd.push(k);
                    i++;
                    if (cmp && rem![0]) {
                        rem!.push(xi < xL ? (xd![xi] || 0) : 0);
                        remL++;
                    } else {
                        rem = xi < xd!.length ? Utils.updateDigits(xd![xi]) : undefined;
                        remL = 1;
                    }
                } while ((xi++ < xL || rem !== undefined) && sd--);
                more = rem !== undefined;
            }
            if (!qd![0]) {
                qd!.shift();
            }
        }
        if (logBase == 1) {
            q.exponent = e;
            inexact = more;
        } else {
            for (i = 1, k = qd![0]; k >= 10; k /= 10) {
                i++;
            }
            q.exponent = i + e * logBase - 1;
            let sd: number = dp ? pr + q.e + 1 : pr;
            Utils.finalise(q, sd, rm, more);
        }
        return q;
    }

    public static random(significantDigits?: number): Decimal {
        let result = new Decimal(1);
        if (significantDigits == undefined) {
            significantDigits = Decimal.precision;
        } else {
            Utils.checkInt32(significantDigits, 1, MAX_DIGITS);
        }

        // calculate the exponent based on 1e7.
        let exp = Math.ceil(significantDigits / LOG_BASE);
        let i: number = 0;
        let resultDigits: Array<number> = new Array<number>();
        if (!Decimal.crypto) {
            for (; i < exp;) {
                resultDigits.push(Math.random() * 1e7 | 0);
                i++;
            }
        } else {
            throw Utils.createBusinessError(CRYPTO_UNAVAILABLE_ERROR_CODE, `Crypto unavailable`);
        }

        let digitsValue = resultDigits[--i];
        significantDigits = significantDigits! % LOG_BASE;

        // Convert trailing digits to zeros according to significant digits.
        if (digitsValue && significantDigits) {
            let value = Math.pow(10, LOG_BASE - significantDigits);
            if (i < resultDigits.length) {
                resultDigits[i] = (digitsValue / value | 0) * value;
            } else {
                resultDigits.push((digitsValue / value | 0) * value);
            } 
        }

        // Remove trailing words which are zero.
        for (; i >= 0 && resultDigits[i] === 0; i--) {
            resultDigits.pop();
        }

        // Zero?
        let resultExponent: number = 0;
        if (i < 0) {
            resultExponent = 0;
            resultDigits = new Array<number>();
            resultDigits.push(0);
        } else {
            resultExponent = -1;
            // Remove leading words which are zero and adjust exponent accordingly.
            for (; resultDigits[0] === 0; resultExponent -= LOG_BASE) {
                resultDigits.shift();
            }

            // Count the digits of the first word of result digits to determine leading zeros.
            let exponent: number = 0;
            let firstVal: number = 0;
            for (exponent = 1, firstVal = resultDigits[0]; firstVal >= 10; firstVal /= 10) {
                exponent++;
            }

            // Adjust the exponent for leading zeros of the first word of result digits.
            if (exponent < LOG_BASE) {
                resultExponent -= LOG_BASE - exponent;
            }
        }
        result.exponent = resultExponent;
        result.digits = resultDigits;

        return result;
    }

    public static createBusinessError(code: number, message: string) {
        return new BusinessError(code, new Error(message));
    }
}
