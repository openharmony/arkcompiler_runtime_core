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

package std.core;

// initialized in _initializerBlock_.ets
/** Lookup table used to convert integer numbers to strings with radix */
const digitChars: char[];

/**
 * Represents boxed long value and related operations
 */
export final class Long extends Integral implements Comparable<Long> {
    private value: long;

    /**
     * Constructs a new Long instance with initial value zero
     */
    public constructor() {
        this.value = 0;
    }

    /**
     * Constructs a new Long instance with provided initial value
     *
     * @param value the initial value
     */
    public constructor(value: long) {
        this.value = value;
    }

    /**
     * Minimal value that this type can have as a long
     */
    public static MIN_VALUE: long = -9223372036854775808;

    /**
     * Maximal value that this type can have as a long
     */
    public static MAX_VALUE: long = 9223372036854775807;

    /**
     * Size of this type in bits
     */
    public static BIT_SIZE: byte = 64;

    /**
     * Size of this type in bytes
     */
    public static BYTE_SIZE: byte = 8;

    /**
     * Returns value of this instance
     *
     * @returns value as long
     */
    public override toByte(): byte {
        return this.value.toByte();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as short
     */
    public override toShort(): short {
        return this.value.toShort();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as int
     */
    public override toInt(): int {
        return this.value.toInt();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as long
     */
    public override toLong(): long {
        return this.value;
    }

    /**
     * Returns value of this instance
     *
     * @returns value as long
     */
    public override toFloat(): float {
        return this.value.toFloat();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as double
     */
    public override toDouble(): double {
        return this.value.toDouble();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as char
     */
    public toChar(): char {
        return this.value.toChar();
    }

    /**
     * Returns the primitive as byte value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toByte(value: long): byte;

    /**
     * Returns the primitive as short value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toShort(value: long): short;

    /**
     * Returns the primitive as int value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toInt(value: long): int;

    /**
     * Returns the primitive as float value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toFloat(value: long): float;

    /**
     * Returns the primitive as double value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toDouble(value: long): double;

    /**
     * Returns the primitive as char value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toChar(value: long): char;

    /**
     * Returns the primitive as long value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static toLong(value: long): long {
        return value;
    }

    /**
     * Compares this instance to other Long object
     * The result is less than 0 if this instance lesser than provided object
     * 0 if they are equal
     * and greater than 0 otherwise.
     *
     * @param other Long object to compare with
     *
     * @returns result of the comparison, -1 if this instance is less than provided object, 0 if equal, 1 if greater
     */
    public override compareTo(other: Long): int {
        if (this.value < other) {
            return -1;
        }
        if (this.value == other) {
            return 0;
        }
        return 1;
    }

    /**
     * Converts the primitive to a string
     *
     * @param v value to be converted
     *
     * @returns result of the conversion
     */
    public static toString(v: long): string {
        return StringBuilder.toString(v);
    }

    /**
     * Converts this object to a string
     *
     * @returns result of the conversion
     */
    public override toString(): String {
        return StringBuilder.toString(this.value);
    }

    /**
     * Converts this object to a string
     *
     * @param radix the radix base for string representation
     *
     * @returns result of the conversion
     */
    public toString(radix: int): string {
        if (radix < 2 || radix > 36) {
            throw new RangeError('Radix must be between 2 and 36')
        }
        if (this.value == 0) {
            return '0'
        }

        // NOTE (igorlegalov): workaround for #26305, replace with 0 when the circular dependency bug is fixed
        const zero: int = (0.0).toInt()
        const negative: boolean = this.value < 0

        let digitsNum: int = negative ? 1 : zero
        for (let val: long = abs(this.value); val > 0; val /= radix) {
            digitsNum++
        }

        const data: FixedArray<char> = new FixedArray<char>(digitsNum)
        let i: int = digitsNum - 1
        for (let val: long = abs(this.value); val > 0; val /= radix) {
            let remainder: long = val % radix
            data[i--] = digitChars[remainder.toInt()]
        }
        if (negative) {
            data[0] = c'-'
        }
        return new String(data)
    }

    /**
    * Checks for equality this instance with provided object, treated as a Long
    *
    * @param other object to be checked against
    *
    * @returns true if provided object and this instance have same value, false otherwise
    * Returns false if type of provided object is not the same as this type
    */
    public equals(other: Any): boolean {
        if (this === other) {
            return true
        }

        if (!(other instanceof Long)) {
            return false
        }

        return this.value == (other as Long).value
    }

    /**
     * Converts this object to a locale-specific string representation
     *
     * @param { Intl.LocalesArgument } [ locales ] A string with a BCP 47 language tag, or an
     *     array of such strings
     * @param { Intl.NumberFormatOptions } [ options ] An object with some or all of the properties of the
     *     Intl.NumberFormat options
     * @returns { String } result of the locale-specific conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public toLocaleString(locales?: Intl.LocalesArgument, options?: Intl.NumberFormatOptions): String {
        const formatter = new Intl.NumberFormat(Intl.intlLocalesToLanguageTags(locales), options)
        return formatter.format(this.value)
    }

    /**
     * Converts this object to a locale-specific string representation
     *
     * @param { Intl.LocalesArgument } [ locales ] A string with a BCP 47 language tag, or an
     *     array of such strings
     * @param { object } [ options ] An object with some or all of the properties of the
     *     Intl.NumberFormat options
     * @returns { String } result of the locale-specific conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public override toLocaleString(locales?: Intl.LocalesArgument, options?: object): String {
        if (options instanceof Intl.NumberFormatOptions) {
            return this.toLocaleString(locales, options)
        }
        return this.toLocaleString(locales, undefined)
    }

    /**
     * Performs integral addition of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the addition
     *
     * @returns Result of the addition
     */
    public add(other: Long): Long {
        return new Long((this.value + other.toLong()) as long)
    }

    /**
     * Performs integral subtraction of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the subtraction
     *
     * @returns Result of the subtraction
     */
    public sub(other: Long): Long {
        return new Long((this.value - other.toLong()) as long)
    }

    /**
     * Performs integral multiplication of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the multiplication
     *
     * @returns Result of the multiplication
     */
    public mul(other: Long): Long {
        return new Long((this.value * other.toLong()) as long)
    }

    /**
     * Performs integral division of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the division
     *
     * @returns Result of the division
     */
    public div(other: Long): Long {
        return new Long((this.value / other.toLong()) as long)
    }

    /**
     * Checks if this instance value is less than value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is less than provided, false otherwise
     */
    public isLessThan(other: Long): boolean {
        return this.value < other.toLong();
    }

    /**
     * Checks if this instance value is less than or equal to value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is less than or equal to provided, false otherwise
     */
    public isLessEqualThan(other: Long): boolean {
        return this.value <= other.toLong();
    }

    /**
     * Checks if this instance value is greater than value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is greater than provided, false otherwise
     */
    public isGreaterThan(other: Long): boolean {
        return this.value > other.toLong();
    }

    /**
     * Checks if this instance value is greater than or equal to value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is greater than or equal to provided, false otherwise
     */
    public isGreaterEqualThan(other: Long): boolean {
        return this.value >= other.toLong();
    }

    /**
     * parseInt(String, int) parses from String an integer of specified radix
     *
     * @param s the string to convert
     * @param r the radix of conversion; should be [2, 36]; 0 assumed to be 10
     *
     * @returns the result of parsing
    */
    public static native parseInt(s: String, r: int): long;
}
