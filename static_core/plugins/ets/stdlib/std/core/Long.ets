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

package std.core;

/**
 * Represents boxed long value and related operations
 */
export final class Long extends Integral implements Comparable<Long>, JSONable<Long> {
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
     * Returns value of this instance as a primitive
     *
     * @returns value of this instance
     */
    public unboxed(): long {
        return this.value;
    }

    /**
     * Returns boxed representation of the primitive
     *
     * @param value value to box
     *
     * @returns boxed value
     */
    public static valueOf(value: long): Long {
        // TODO(ivan-tyulyandin): caching is possible
        return new Long(value);
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
        return Long.toFloat(this.value);
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
     * @returns result of the comparison
     */
    public override compareTo(other: Long): int {
        return (this.value - other.unboxed()).toInt();
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
     * @returns result of the conversion
     */
    public toString(radix: number): string {
        if (radix.toInt() < 2 || radix.toInt() > 36) {
            throw new RangeError('Radix must be between 2 and 36');
        }
        if (this.value == 0) {
            return "0";
        }
        const chars = '0123456789abcdefghijklmnopqrstuvwxyz';
        let val = this.value;
        let negative: boolean = (val < 0);
        let digitsNum = (log(abs(val)) / log(radix.toInt()) + 1).toInt();
        if (negative) {
            ++digitsNum;
        }
        let data : FixedArray<char> = new char[digitsNum];
        let curPlace = digitsNum - 1;
        while (val != 0) {
            let remainder = val % radix.toInt();
            if (negative) {
                remainder = -remainder;
            }
            data[curPlace] = chars.charAt(remainder.toInt());
            val /= radix.toInt();
            curPlace -= 1;
        }
        if (negative) {
            data[0] = c'-';
        }
        return new String(data);
    }

    /**
     * Returns a hash code (integer representation) for this instance
     *
     * @returns representation of this instance
     */
    public override $_hashCode(): int {
        return this.toInt();
    }

    /**
    * Checks for equality this instance with provided object, treated as a Long
    *
    * @param other object to be checked against
    *
    * @returns true if provided object and this instance have same value, false otherwise
    * Returns false if type of provided object is not the same as this type
    */
    public equals(other: NullishType): boolean {
        if (this === other) {
            return true
        }

        if (!(other instanceof Long)) {
            return false
        }

        return this.value == (other as Long).value
    }

    /**
     * Performs integral addition of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the addition
     *
     * @returns Result of the addition
     */
    public add(other: Long): Long {
        return Long.valueOf((this.value + other.toLong()) as long)
    }

    /**
     * Performs integral subtraction of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the subtraction
     *
     * @returns Result of the subtraction
     */
    public sub(other: Long): Long {
        return Long.valueOf((this.value - other.toLong()) as long)
    }

    /**
     * Performs integral multiplication of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the multiplication
     *
     * @returns Result of the multiplication
     */
    public mul(other: Long): Long {
        return Long.valueOf((this.value * other.toLong()) as long)
    }

    /**
     * Performs integral division of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the division
     *
     * @returns Result of the division
     */
    public div(other: Long): Long {
        return Long.valueOf((this.value / other.toLong()) as long)
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
     * Creates a Long instance based on JSONValue
     *
     * @param json: JSONValue - a JSON representation
     *
     * @throws JSONTypeError if json does not encode a valid long
     *
     * @returns Long - long value decoded from JSON
     */
    public createFromJSONValue(json: JSONValue): Long {
        if (json instanceof JSONNumber) {
            let num = (json as JSONNumber).value
            if (Double.isInteger(num) && Long.MIN_VALUE <= num && num <= Long.MAX_VALUE) {
                return Long.valueOf(Double.toLong(num))
            }
        }
        throw new JSONTypeError("Cannot create Long from JSON", new ErrorOptions(json as Object))
    }
}
