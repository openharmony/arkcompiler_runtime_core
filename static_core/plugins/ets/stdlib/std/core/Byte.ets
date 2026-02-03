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
 * @file Defines the Byte for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Represents boxed byte value and related operations
 *
 * @extends Integral
 * @implements Comparable<Byte>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class Byte extends Integral implements Comparable<Byte> {
    private value: byte;

    /**
     * Constructs a new Byte instance with initial value zero
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = 0;
    }

    /**
     * Constructs a new Byte instance with provided initial value
     *
     * @param { byte } value the initial value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: byte) {
        this.value = value;
    }

    /**
     * Returns boxed representation of the primitive
     *
     * @param { byte } value value to box
     * @returns { Byte }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static valueOf(value: byte): Byte {
        // TODO(ivan-tyulyandin): caching is possible
        return new Byte(value);
    }

    /**
     * Minimal value that this type can have as an integral
     *
     * @type {byte}
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static MIN_VALUE: byte = -128;

    /**
     * Maximal value that this type can have as an integral
     *
     * @type {byte}
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static MAX_VALUE: byte = 127;

    /**
     * Size of this type in bits
     *
     * @type {byte}
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static BIT_SIZE: byte = 8;

    /**
     * Size of this type in bytes
     *
     * @type {byte}
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static BYTE_SIZE: byte = 1;

    /**
     * Returns value of this instance
     *
     * @returns { byte }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toByte(): byte {
        return this.value;
    }

    /**
     * Returns value of this instance Supportive interface
     *
     * @returns { byte }
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
     * @returns { short }
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
     * @returns { short }
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
     * @returns { int }
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
     * @returns { int }
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
     * @returns { long }
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
     * @returns { long }
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
     * @returns { float }
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
     * @returns { float }
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
     * @returns { double }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toDouble(): double {
        return this.value.toDouble();
    }

    /**
     * Returns value of this instance Supportive interface
     *
     * @returns { double }
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
     * @returns { char }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toChar(): char {
        return this.value.toChar();
    }

    /**
     * Returns the primitive as short value
     *
     * @param { byte } value value to cast
     * @returns { short }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toShort(value: byte): short;

    /**
     * Returns the primitive as int value
     *
     * @param { byte } value value to cast
     * @returns { int }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toInt(value: byte): int;

    /**
     * Returns the primitive as long value
     *
     * @param { byte } value value to cast
     * @returns { long }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toLong(value: byte): long;

    /**
     * Returns the primitive as float value
     *
     * @param { byte } value value to cast
     * @returns { float }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toFloat(value: byte): float;

    /**
     * Returns the primitive as double value
     *
     * @param { byte } value value to cast
     * @returns { double }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toDouble(value: byte): double;

    /**
     * Returns the primitive as char value
     *
     * @param { byte } value value to cast
     * @returns { char }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toChar(value: byte): char;

    /**
     * Returns the primitive as byte value
     *
     * @param { byte } value value to cast
     * @returns { byte }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toByte(value: byte): byte {
        return value;
    }

    /**
     * Compares this instance to other Byte object The result is less than 0 if this instance lesser than provided
     * object 0 if they are equal and greater than 0 otherwise.
     *
     * @param { Byte } other Byte object to compare with
     * @returns { int }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override compareTo(other: Byte): int {
        if (this.value < other) {
            return -1;
        }
        if (this.value == other) {
            return 0;
        }
        return 1;
    }

    /**
     * Converts this object to a string
     *
     * @returns { String }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toString(): String {
        return StringBuilder.toString(this.value);
    }

    /**
     * Converts the primitive to a string
     *
     * @param { byte } v value to be converted
     * @returns { string }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toString(v: byte): string {
        return StringBuilder.toString(v);
    }

    /**
     * Converts this object to a string
     *
     * @param { number } radix None
     * @returns { string }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toString(radix: number): string {
        return (new Long(this.value)).toString(radix);
    }

    /**
     * Checks for equality this instance with provided object, treated as a Byte Returns false if type of provided
     * object is not the same as this type
     *
     * @param { Any } other object to be checked against
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */

    public equals(other: Any): boolean {
        if (this === other) {
            return true
        }

        if (!(other instanceof Byte)) {
            return false
        }

        return this.value == (other as Byte).value
    }

    /**
     * Converts this object to a locale-specific string representation
     *
     * @returns { String }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toLocaleString(): String {
        return this.toString();
    }

    /**
     * Performs integral addition of this instance with provided one, returns the result as new instance
     *
     * @param { Byte } other Right hand side of the addition
     * @returns { Byte }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public add(other: Byte): Byte {
        return (this.value + other.toByte()).toByte()
    }

    /**
     * Performs integral subtraction of this instance with provided one, returns the result as new instance
     *
     * @param { Byte } other Right hand side of the subtraction
     * @returns { Byte }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public sub(other: Byte): Byte {
        return (this.value - other.toByte()).toByte()
    }

    /**
     * Performs integral multiplication of this instance with provided one, returns the result as new instance
     *
     * @param { Byte } other Right hand side of the multiplication
     * @returns { Byte }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public mul(other: Byte): Byte {
        return (this.value * other.toByte()).toByte()
    }

    /**
     * Performs integral division of this instance with provided one, returns the result as new instance
     *
     * @param { Byte } other Right hand side of the division
     * @returns { Byte }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public div(other: Byte): Byte {
        return (this.value / other.toByte()).toByte()
    }

    /**
     * Checks if this instance value is less than value of provided instance
     *
     * @param { Byte } other Right hand side of the comparison
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isLessThan(other: Byte): boolean {
        return this.value < other.toByte();
    }

    /**
     * Checks if this instance value is less than or equal to value of provided instance
     *
     * @param { Byte } other Right hand side of the comparison
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isLessEqualThan(other: Byte): boolean {
        return this.value <= other.toByte();
    }

    /**
     * Checks if this instance value is greater than value of provided instance
     *
     * @param { Byte } other Right hand side of the comparison
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isGreaterThan(other: Byte): boolean {
        return this.value > other.toByte();
    }

    /**
     * Checks if this instance value is greater than or equal to value of provided instance
     *
     * @param { Byte } other Right hand side of the comparison
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isGreaterEqualThan(other: Byte): boolean {
        return this.value >= other.toByte();
    }

    /**
     * Parses a string to a byte value.
     *
     * @param { String } s The string to parse.
     * @param { int } r The radix of the string.
     * @returns { byte } The parsed byte value.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native parseInt(s: String, r: int): byte;
}
