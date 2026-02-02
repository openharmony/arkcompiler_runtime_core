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
 * @file Defines the Double for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Represents boxed double value and related operations
 * 
 * @extends Floating
 * @implements Comparable<Double>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class Double extends Floating implements Comparable<Double> {
    private value: double;

    /**
     * Constructs a new Double instance with initial value zero
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = 0.0;
    }

    /**
     * Constructs a new Double instance with provided initial value
     *
     * @param { double } value the initial value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: double) {
        this.value = value;
    }

    /**
     * Constructs a new Double instance from BigInt
     *
     * @param { BigInt } value 
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: BigInt) {
        this.value = value.doubleValue()
    }

    /**
     * Constructs a new Double instance from string
     *
     * @param { String } value string that may contain a number
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: String) {
        this.value = Double.numberFromString(value);
    }

    /**
     * Creates a new instance of a Double
     *
     * @returns { Number }A new Double instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static $_invoke(): Number {
        return new Double();
    }

    /**
     * Creates a new instance of a Double
     *
     * @param { String | Number | BigInt | undefined | null } value The value to be converted to a number.
     * Can be a string, number, or BigInt (optional).
     * @returns { Number } A new Double instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static $_invoke(value: String | Number | BigInt | undefined | null): Number {
        if (value instanceof String) {
            return new Double(value as String);
        } else if (value instanceof Number) {
            return new Double(value as Number);
        } else if (value instanceof BigInt) {
            return new Double(value as BigInt);
        }
        if (value === null) {
            return new Double();
        }
        return new Double(NaN);
    }

    /**
     * Returns boxed representation of the primitive
     *
     * @param { double } value value to box
     * @returns { Double } return current double value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static valueOf(value: double): Double {
        // TODO(ivan-tyulyandin): caching is possible
        return new Double(value);
    }

    /**
     * Returns boxed representation of the primitive
     *
     * @returns { number } boxed value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public valueOf(): number {
        return this.value as number;
    }

    /**
     * Minimal value that this type can have as a double
     * the workarond for libc's double literal parsing bug
     *
     * @type { double }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly MIN_VALUE: double = 4.9e-300 / 1.e+24;

    /**
     * Maximal value that this type can have as a double
     *
     * @type { double }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly MAX_VALUE: double = 1.7976931348623157e+308;

    /**
     * Maximal integer value that can be used as a double without loss of precision
     *
     * @type { double }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly MAX_SAFE_INTEGER: double = 9007199254740991;

    /**
     * Minimal integer value that can be used as a double without loss of precision
     *
     * @type { double }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly MIN_SAFE_INTEGER: double = -9007199254740991;

    /**
     * Size of this type in bits
     * 
     * @type { byte }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly BIT_SIZE: byte = 64;

    /**
     * Size of this type in bytes
     *
     * @type { byte }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly BYTE_SIZE: byte = 8;

    /**
     * Represents the NaN value according to IEEE-754 specification
     *
     * @type { double }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly NaN: double = 0.0 / 0.0;

    /**
     * Represents the +Infinity value according to IEEE-754 specification
     *
     * @type { double }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly POSITIVE_INFINITY: double = 1.0 / 0.0;

    /**
     * Represents the -Infinity value according to IEEE-754 specification
     *
     * @type { double }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly NEGATIVE_INFINITY: double = -1.0 / 0.0;

    /**
     * Number of significant precision bits in this floating type
     * 
     * @type { byte }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly PRECISION: byte = 53;

    /**
     * Minimal possible difference between two double values.
     * For double (IEEE-754 binary64) it is 2^(-52) and its bit representation is 0x3cb0000000000000.
     * 
     * @type { double }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly DELTA: double = Double.bitCastFromLong(0x3cb0000000000000);

    /**
     * Minimal possible difference between two double values
     *
     * @type { double }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly EPSILON: double = Double.DELTA;


    /**
     * Returns value of this instance
     *
     * @returns { byte } return the current value convert byte value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toByte(): byte {
        return this.value.toByte();
    }

    /**
     * Returns value of this instance. Supportive interface
     *
     * @returns { byte } return the current value convert byte value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override byteValue(): byte {
        return this.toByte();
    }

    /**
     * Returns value of this instance
     *
     * @returns { short } return the current value convert short value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toShort(): short {
        return this.value.toShort();
    }

    /**
     * Returns value of this instance Supportive interface
     *
     * @returns { short } return the current value convert short value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override shortValue(): short {
        return this.toShort();
    }

    /**
     * Returns value of this instance
     *
     * @returns { int } return the current value convert int value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toInt(): int {
        return this.value.toInt();
    }

    /**
     * Returns value of this instance Supportive interface
     *
     * @returns { int } return the current value convert int value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override intValue(): int {
        return this.toInt();
    }

    /**
     * Returns value of this instance
     *
     * @returns { long } return the current value convert long value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toLong(): long {
        return this.value.toLong();
    }

    /**
     * Returns value of this instance Supportive interface
     *
     * @returns { long } return the current value convert long value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override longValue(): long {
        return this.toLong();
    }

    /**
     * Returns value of this instance
     *
     * @returns { float } return the current value convert float value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toFloat(): float {
        return this.value.toFloat();
    }

    /**
     * Returns value of this instance Supportive interface
     *
     * @returns { float } return the current value convert float value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override floatValue(): float {
        return this.toFloat();
    }

    /**
     * Returns value of this instance
     *
     * @returns { double } return the current value convert double value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toDouble(): double {
        return this.value;
    }

    /**
     * Returns value of this instance Supportive interface
     *
     * @returns { double } return the current value convert double value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override doubleValue(): double {
        return this.toDouble();
    }

    /**
     * Returns the primitive as double value
     *
     * @param { double } value value to cast
     * @returns { double }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toDouble(value: double): double {
        return value;
    }

    /**
     * Returns the primitive as byte value
     *
     * @param { double } value value to cast
     * @returns { byte }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toByte(value: double): byte;

    /**
     * Returns the primitive as short value
     *
     * @param { double } value value to cast
     * @returns { short }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toShort(value: double): short;

    /**
     * Returns the primitive as int value
     *
     * @param { double } value value to cast
     * @returns { int }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toInt(value: double): int;

    /**
     * Returns the primitive as long value
     *
     * @param { double } value value to cast
     * @returns { long }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toLong(value: double): long;

    /**
     * Returns the primitive as float value
     *
     * @param { double } value value to cast
     * @returns { float }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toFloat(value: double): float;

    /**
     * Compares this instance to other Double object
     * The result is less than 0 if this instance lesser than provided object
     * 0 if they are equal
     * and greater than 0 otherwise.
     *
     * @param { Double } other Double object to compare with
     * @returns { int } if the cur value > the other reutrn 0,otherwise return -1
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override compareTo(other: Double): int {
        if ((this.isNaN() && other.isNaN()) || (Math.abs(this.value - other) < Double.EPSILON)) {
            return 0;
        }
        if (this.isNaN() || this.value > other) {
            return 1;
        }
        return -1;
    }

    /**
     * toString(d: double, r: int): String -- returns a string representation of d by radix r
     * 
     * @param { double } d 
     * @param { int } r 
     * @returns { String } return the string value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toString(d: double, r: int): String;

    /**
     * toString
     *
     * @param { double } d  return the string value
     * @returns { String }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toString(d: double): String {
        return Double.toString(d, 10);
    }

    /**
     * toString
     *
     * @param { int } r 
     * @returns { String } return the string value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toString(r: int): String {
        return Double.toString(this.value, r);
    }

    /**
     * toString
     *
     * @param { Number } d 
     * @returns { String } return the string value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toString(d: Number): String {
        return Double.toString(this.value, d.toInt());
    }

    /**
     * Converts this object to a string
     *
     * @returns { String } return the string value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toString(): String {
        return Double.toString(this.value, 10);
    }

    /**
     * Without an argument method returns just toString value
     *
     * @returns { String } result of the conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toLocaleString(): String {
        return new Intl.NumberFormat().format(this.value)
    }

    /**
     * toLocaleString
     *
     * @param { string | Intl.Locale | (string | Intl.Locale)[] } [locales] set intl locales
     * @param { Intl.NumberFormatOptions } [options] set intl param options
     * @returns { string } result of the conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toLocaleString(locales?: string | Intl.Locale | (string | Intl.Locale)[], options?: Intl.NumberFormatOptions): string {
        const formatter = new Intl.NumberFormat(Intl.intlLocalesToLanguageTags(locales), options)
        return formatter.format(this.value)
    }

    /**
     * compare(double, double) checks if two doubles are differs no more than by Double.DELTA
     *
     * @param { double } lhs left-hand side double for comparison
     * @param { double } rhs right-hand side double for comparison
     * @returns { boolean } true if lhs and rhs are equal with respect to Double.DELTA
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static compare(lhs: double, rhs: double): boolean {
        return (abs(lhs - rhs) <= Double.DELTA)
    }

    /**
     * Checks for equality this instance with provided object, 
     * treated as a Double Returns false if type of provided object is not the same as this type
     *
     * @param { Any } other object to be checked against
     * @returns { boolean } true if provided object and this instance have same value, false otherwise
     *     Returns false if type of provided object is not the same as this type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public equals(other: Any): boolean {
        if (this === other) {
            return true
        }

        if (!(other instanceof Double)) {
            return false
        }

        return this.value == (other as Double).value
    }

    /**
     * isNaN(double) checks if double is NaN (not a number)
     *
     * @param { double } v the double to test
     * @returns { boolean } true if the argument is NaN
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isNaN(v: double): boolean {
        // IEEE-754 feature
        return v != v;
    }

    /**
     * isNaN() checks if the underlying double is NaN (not a number)
     *
     * @returns { boolean } true if the underlying double is NaN
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isNaN(): boolean {
        return Double.isNaN(this.value);
    }

    /**
     * isFinite(double) checks if double is a floating point value (not a NaN or infinity)
     *
     * @param { double } v the double to test
     * @returns { boolean } true if the argument is a floating point value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isFinite(v: double): boolean {
        return !(Double.isNaN(v) || (v == Double.POSITIVE_INFINITY) || (v == Double.NEGATIVE_INFINITY));
    }

    /**
     * isFinite() checks if the underlying double is a floating point value (not a NaN or infinity)
     *
     * @returns { boolean } true if the underlying double is a floating point value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isFinite(): boolean {
        return Double.isFinite(this.value);
    }

    /**
     * Checks if double is similar to an integer value
     *
     * @param { double } v the double to test
     * @returns { boolean } true if the argument is similar to an integer value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isInteger(v: double): boolean {
        // In the language % works as C fmod that differs with IEEE-754 % definition
        return (abs(v % (1.0 as double)) == 0.0);
    }

    /**
     * Checks if the underlying double is similar to an integer value
     *
     * @returns { boolean } true if the underlying double is similar to an integer value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isInteger(): boolean {
        return Double.isInteger(this.value);
    }

    /**
     * Checks if double is a safe integer value
     *
     * @param { double } v the double to test
     * @returns { boolean } true if the argument is integer ans less than MAX_SAFE_INTEGER
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isSafeInteger(v: double): boolean {
        return Double.isInteger(v) && (abs(v) <= Double.MAX_SAFE_INTEGER);
    }

    /**
     * Checks if double is a safe integer value
     *
     * @returns { boolean } true if the underlying double is a safe integer
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isSafeInteger(): boolean {
        return Double.isSafeInteger(this.value);
    }


    /**
     * Performs floating point addition of this instance with provided one, returns the result as new instance
     *
     * @param { Double } other Right hand side of the addition
     * @returns { Double } Result of the addition
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public add(other: Double): Double {
        return Double.valueOf((this.value + other.toDouble()) as double)
    }

    /**
     * Performs floating point subtraction of this instance with provided one, returns the result as new instance
     *
     * @param { Double } other Right hand side of the subtraction
     * @returns { Double } Result of the subtraction
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public sub(other: Double): Double {
        return Double.valueOf((this.value - other.toDouble()) as double)
    }

    /**
     * Performs floating point multiplication of this instance with provided one, returns the result as new instance
     *
     * @param { Double } other Right hand side of the multiplication
     * @returns { Double } Result of the multiplication
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public mul(other: Double): Double {
        return Double.valueOf((this.value * other.toDouble()) as double)
    }

    /**
     * Performs floating point division of this instance with provided one, returns the result as new instance
     *
     * @param { Double } other Right hand side of the division
     * @returns { Double } Result of the division
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public div(other: Double): Double {
        return Double.valueOf((this.value / other.toDouble()) as double)
    }

    /**
     * Checks if this instance value is less than value of provided instance
     *
     * @param { Double } other Right hand side of the comparison
     * @returns { boolean } true if this value is less than provided, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isLessThan(other: Double): boolean {
        return this.value < other.toDouble();
    }

    /**
     * Checks if this instance value is less than or equal to value of provided instance
     *
     * @param { Double } other Right hand side of the comparison
     * @returns { boolean } true if this value is less than or equal to provided, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isLessEqualThan(other: Double): boolean {
        return this.value <= other.toDouble();
    }

    /**
     * Checks if this instance value is greater than value of provided instance
     *
     * @param { Double } other Right hand side of the comparison
     * @returns { boolean } true if this value is greater than provided, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isGreaterThan(other: Double): boolean {
        return this.value > other.toDouble();
    }

    /**
     * Checks if this instance value is greater than or equal to value of provided instance
     *
     * @param { Double } other Right hand side of the comparison
     * @returns { boolean } true if this value is greater than or equal to provided, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isGreaterEqualThan(other: Double): boolean {
        return this.value >= other.toDouble();
    }

    /**
     * parseFloat(String) converts std.core.String to double
     * If arg is '+Infinity', 'Infinity' or '-Infinity', return value is `inf` or `-inf` respectively.
     * If arg is '+0' or '-0', return value is 0 or -0.
     * If arg has leading zeroes, it's ignored: '0001.5' -> 1.5, '-0001.5' -> -1.5
     * If arg starts from '.', leading zero is implied: '.5' -> 0.5, '-.5' -> -0.5
     * If arg successfully parsed, trailing non-digits ignored: '-.6ffg' -> -0.6
     * If arg can not be parsed into a number, NaN is returned
     *
     * @param { String } s the string to convert
     * @returns { double } the result of conversion
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native parseFloat(s: String): double;

    /**
     * parseInt(String, int) parses from String an integer of specified radix
     * If args ('10', 1) -> thrown ArgumentOutOfRangeError, ('10', 37) -> thrown ArgumentOutOfRangeError
     * If args ('10', 2) -> 2
     * If args ('10', 10) -> 10, ('10', 0) -> 10
     * If args ('ff', 16) -> 255
     * etc.
     *
     * @param { String } s the string to convert
     * @param { int } r the radix of conversion; should be [2, 36]; 0 assumed to be 10
     * @returns { double } the result of parsing
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native parseIntCore(s: String, r: int): double;

    /**
     * parseInt(String) parses from String an integer of radix 10
     * 
     * @returns the result of parsing
     * @param { String } s the string to convert
     * @returns { double } the result of parsing
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static parseInt(s: String): double {
        return Double.parseIntCore(s, 0);
    }
    
    /**
     * parseInt(String, int) parses from String an integer of specified radix
     * If args ('10', 1) -> thrown ArgumentOutOfRangeError, ('10', 37) -> thrown ArgumentOutOfRangeError
     * If args ('10', 2) -> 2
     * If args ('10', 10) -> 10, ('10', 0) -> 10
     * If args ('ff', 16) -> 255
     * etc.
     * 
     * @param { String } s the string to convert
     * @param { int } r the radix of conversion; should be [2, 36]; 0 assumed to be 10
     * @returns { double } the result of parsing
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static parseInt(s: String, r: int): double {
        if ((r < 2 || r > 36) && r != 0.0) {
            return NaN;
        } else {
            return Double.parseIntCore(s, r);
        }
    }

    /**
     * parseInt(String, double) parses from String an integer of specified radix
     * If args ('10', 1) -> thrown ArgumentOutOfRangeError, ('10', 37) -> thrown ArgumentOutOfRangeError
     * If args ('10', 2) -> 2
     * If args ('10', 10) -> 10, ('10', 0) -> 10
     * If args ('ff', 16) -> 255
     * etc.
     *
     * @param { String } s the string to convert
     * @param { double } r the radix of conversion; should be [2, 36]; 0 assumed to be 10
     * @returns { double } the result of parsing
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static parseInt(s: String, r: double): double {
        if (!Double.isFinite(r)) {
            r = 0.0;
        }
        if ((r < 2.0 || r > 36.0) && r != 0.0) {
            return NaN;
        } else {
            return Double.parseIntCore(s.trim(), Double.toInt(r));
        }
    }

    /**
     * toExponential(double) returns std.core.String representing the underlying double in exponential notation
     * If d = new Double(0.25); d.toExponential(2) -> '2.50e-1'
     * If d = new Double(0.25); d.toExponential(2.5) -> '2.50e-1'
     * If d = new Double(0.25); d.toExponential(1) -> '2.5e-1'
     * If d = new Double(12345.01); d.toExponential(10) -> '1.2345010000e+4'
     * If d = new Double(NaN); d.toExponential(10) -> 'NaN';
     * If d = new Double(Double.POSITIVE_INFINITY); d.toExponential(10) -> 'Infinity';
     *     '-Infinity' for negative
     * @returns { String } the result of conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private native toExponentialImpl(d: double): String;
    public native toExponentialWithNoDigit(): String;

    /**
     * toExponential() returns std.core.String representing the underlying double in exponential notation
     *
     * @returns { String } the result of conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toExponential(): String {
        return this.toExponentialWithNoDigit();
    }

    /**
     * toExponential() returns std.core.String representing the underlying double in exponential notation
     *
     * @param { double } [ d ] 
     * @returns { String } the result of conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toExponential(d?: double): String {
        if (d == undefined) {
            return this.toExponentialWithNoDigit()
        }
        return this.toExponentialImpl(d)
    }

    /**
     * toPrecision(double) returns std.core.String representing the underlying double on the specified precision
     * If d = new Double(0.25); d.toPrecision(4) -> '0.2500'
     * If d = new Double(1.01); d.toPrecision(4.7) -> '1.010'
     * If d = new Double(0.25); d.toPrecision(0) -> thrown ArgumentOutOfRangeError
     * If d = new Double(12345.123455); d.toPrecision(10) -> '12345.12346'
     * 
     * @param { double } d precision (rounded to nearest integer); must be in [1, 100]
     * @returns { String } the result of conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native toPrecision(d: double): String;

    /**
     * toPrecision() returns std.core.String representing the underlying double in exponential notation
     *     basically, if toPrecision called with no argument it's just toString according to spec
     * 
     * @returns { String } the result of conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toPrecision(): String {
        return this.toString();
    }

    /**
     * toFixed(double) returns std.core.String representing the underlying double using fixed-point notation
     * If d = new Double(0.1); d.toFixed(0) -> '0'
     * If d = new Double(0.7); d.toFixed(0) -> '1'
     * If d = new Double(0.12345); d.toFixed(1) -> '0.1'
     * If d = new Double(0.12345); d.toFixed(3) -> '0.123'
     * If d = new Double(Double.POSITIVE_INFINITY); d.toFixed(3) -> 'Infinity'
     * If d = new Double(Double.NaN); d.toFixed(3) -> 'NaN'
     * If d = new Double(0.25); d.toFixed(200) -> thrown ArgumentOutOfRangeError
     * 
     * @param { double } d fixed size (integer part); must be in [0, 100]
     * @returns { String } the result of conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native toFixedImpl(d: double): String;

    /**
     * toFixed(double) returns std.core.String representing the underlying double using fixed-point notation
     *
     * @returns { String }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toFixed(): String {
        return this.toFixed(0);
    }

    /**
     * toFixed(double) returns std.core.String representing the underlying double using fixed-point notation
     *
     * @param { double } [ d ] 
     * @returns { String }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toFixed(d?: double): String {
        if (d == undefined) {
            return this.toFixedImpl(0)
        }
        return this.toFixedImpl(d)
    }

    /**
     * Converts bit representation to corresponding IEEE-754 floating point representation
     * 
     * @param { long } bits bits to convert
     * @returns { double } converted value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native bitCastFromLong(bits: long): double

    /**
     * Converts IEEE-754 floating point representation to corresponding bit representation
     * 
     * @param { double } val value to convert
     * @returns { long } bit representation
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native bitCastToLong(val: double): long

    /**
     * numberFromString(String) converts std.core.String to double
     *
     * @param s the string to convert
     *
     * @returns the result of conversion
     *
     * @note
     * If arg is '+Infinity', 'Infinity' or '-Infinity', return value is `inf` or `-inf` respectively.
     * If arg is '+0' or '-0', return value is 0 or -0.
     * If arg has leading zeroes, it's ignored: '0001.5' -> 1.5, '-0001.5' -> -1.5
     * If arg starts from '.', leading zero is implied: '.5' -> 0.5, '-.5' -> -0.5
     * If arg successfully parsed, trailing non-digits cause return value is NaN: '-.6ffg' -> -NaN
     * If arg can not be parsed into a number, NaN is returned
     *
     *
     * ECMA reference: https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number-constructor-number-value
     */
    private static native numberFromString(s: String): double;
}

/**
 * `Number' values represent floating-point numbers like 6 or -3.14
 * 
 * @typedef { Double } Number
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export type Number = Double;

/**
 Number is a synonym for type number
 *
 * @typedef { double } number
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export type number = double;
