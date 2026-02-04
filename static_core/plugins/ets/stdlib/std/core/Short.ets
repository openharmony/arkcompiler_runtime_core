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
 * @file Defines ArkTS Short class
 * @kit ArkTS
 */
package std.core;

/**
 * Represents boxed short value and related operations
 * @extends Integral
 * @implements Comparable<Short>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class Short extends Integral implements Comparable<Short> {
    private value: short;

    /**
     * Constructs a new Short instance with initial value zero
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = 0;
    }

    /**
     * Constructs a new Short instance with provided initial value
     * @param { short } value the initial value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: short) {
        this.value = value;
    }

    /**
     * Returns boxed representation of the primitive
     * @param { short } value value to box
     * @returns { short } boxed value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static valueOf(value: short): Short {
        // TODO(ivan-tyulyandin): caching is possible
        return new Short(value);
    }

    /**
     * Minimal value that this type can have as an shortegral
     * @type { short } MIN_VALUE
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static MIN_VALUE: short = -32768;

    /**
     * Maximal value that this type can have as an shortegral
     * @type { short } MAX_VALUE
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static MAX_VALUE: short = 32767;

    /**
     * Size of this type in bits
     * @type { byte } BIT_SIZE
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static BIT_SIZE: byte = 16;

    /**
     * Size of this type in bytes.
     * @type { byte } BYTE_SIZE
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static BYTE_SIZE: byte = 2;

    /**
     * Returns value of this instance
     *
     * @returns { byte } value as short
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toByte(): byte {
        return (this.value).toByte();
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns { byte } value as byte
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
     * @returns { short } value as short
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toShort(): short {
        return this.value;
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns { short } value as short
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
     * @returns { int } value as int
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toInt(): int {
        return Short.toInt(this.value);
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns { int } value as int
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
     * @returns { long } value as long
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toLong(): long {
        return this.value.toLong();
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns { long } value as long
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
     * @returns { float } value as float
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toFloat(): float {
        return this.value.toFloat();
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns { float } value as float
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
     * @returns { double } value as double
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toDouble(): double {
        return Short.toDouble(this.value);
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns { double } value as double
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override doubleValue(): double {
        return this.toDouble();
    }

    /**
     * Returns value of this instance
     *
     * @returns { char } value as char
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toChar(): char {
        return this.value.toChar();
    }

    /**
     * Returns the primitive as byte value
     *
     * @param { short } value value to cast
     * @returns { byte } casted value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toByte(value: short): byte;

    /**
     * Returns the primitive as int value
     *
     * @param { short } value value to cast
     * @returns { int } casted value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toInt(value: short): int;

    /**
     * Returns the primitive as long value
     *
     * @param { short } value value to cast
     * @returns { long } casted value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toLong(value: short): long;

    /**
     * Returns the primitive as float value
     *
     * @param { short } value value to cast
     * @returns { float }  casted value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toFloat(value: short): float;

    /**
     * Returns the primitive as double value
     *
     * @param { short } value value to cast
     * @returns { double } casted value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toDouble(value: short): double;

    /**
     * Returns the primitive as char value
     *
     * @param { short } value value to cast
     * @returns { char } casted value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toChar(value: short): char;

    /**
     * Returns the primitive as short value
     *
     * @param { short } value value to cast
     * @returns { short } casted value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toShort(value: short): short {
        return value;
    }

    /**
     * Compares this instance to other Short object
     *     The result is less than 0 if this instance lesser than provided object
     *     0 if they are equal, and greater than 0 otherwise.
     *
     * @param { short } other Short object to compare with
     * @returns { int } result of the comparison, -1 if this instance is less than provided object,
     *     0 if equal, 1 if greater
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override compareTo(other: Short): int {
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
     * @param { short } v value to be converted
     * @returns { String } result of the conversion
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toString(v: short): string {
        return StringBuilder.toString(v);
    }

    /**
     * Converts this object to a string
     * @returns { String } result of the conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toString(): String {
        return StringBuilder.toString(this.value);
    }

    /**
     * Converts this object to a string
     *
     * @param { number } radix the radix base for string representation
     * @returns { string } result of the conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toString(radix: number): string {
        return (new Long(this.value)).toString(radix);
    }

    /**
    * Checks for equality this instance with provided object, treated as a Short
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

        if (!(other instanceof Short)) {
            return false
        }

        return this.value == (other as Short).value
    }

     /**
     * Converts this object to a locale-specific string representation
     * @returns { String } result of the locale-specific conversion
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toLocaleString(): String {
        return new Intl.NumberFormat().format(this.value)
    }

    /**
     * Performs shortegral addition of this instance with provided one, returns the result as new instance
     *
     * @param { Short } other Right hand side of the addition
     * @returns { Short } Result of the addition
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public add(other: Short): Short {
        return Short.valueOf((this.value + other.toShort()).toShort())
    }

    /**
     * Performs shortegral subtraction of this instance with provided one, returns the result as new instance
     *
     * @param { Short } other Right hand side of the subtraction
     * @returns { Short } Result of the subtraction
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public sub(other: Short): Short {
        return Short.valueOf((this.value - other.toShort()).toShort())
    }

    /**
     * Performs shortegral multiplication of this instance with provided one, returns the result as new instance
     *
     * @param { Short } other Right hand side of the multiplication
     * @returns { Short } Result of the multiplication
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public mul(other: Short): Short {
        return Short.valueOf((this.value * other.toShort()).toShort())
    }

    /**
     * Performs shortegral division of this instance with provided one, returns the result as new instance
     *
     * @param { Short } other Right hand side of the division
     * @returns { Short } Result of the division
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public div(other: Short): Short {
        return Short.valueOf((this.value / other.toShort()).toShort())
    }

    /**
     * Checks if this instance value is less than value of provided instance
     *
     * @param { Short } other Right hand side of the comparison
     * @returns { boolean } true if this value is less than provided, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isLessThan(other: Short): boolean {
        return this.value < other.toShort();
    }

    /**
     * Checks if this instance value is less than or equal to value of provided instance
     *
     * @param { Short } other Right hand side of the comparison
     * @returns { boolean } true if this value is less than or equal to provided, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isLessEqualThan(other: Short): boolean {
        return this.value <= other.toShort();
    }

    /**
     * Checks if this instance value is greater than value of provided instance
     *
     * @param { Short } other Right hand side of the comparison
     * @returns { boolean } true if this value is greater than provided, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isGreaterThan(other: Short): boolean {
        return this.value > other.toShort();
    }

    /**
     * Checks if this instance value is greater than or equal to value of provided instance
     *
     * @param { Short } other Right hand side of the comparison
     * @returns { boolean } true if this value is greater than or equal to provided, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isGreaterEqualThan(other: Short): boolean {
        return this.value >= other.toShort();
    }

    /**
     * parseInt(String, int) parses from String an integer of specified radix
     *
     * @param { String } s the string to convert
     * @param { int } r the radix of conversion; should be [2, 36]; 0 assumed to be 10
     * @returns { Short } the result of parsing
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
    */
    public static native parseInt(s: String, r: int): short;
}
