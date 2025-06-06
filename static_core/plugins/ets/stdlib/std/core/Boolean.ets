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
 * Represents boxed boolean value and related operations
 */
export final class Boolean extends Object implements Comparable<Boolean> {
    private value: boolean;

    /**
     * Constructs a new Boolean with provided value
     *
     * @param value value to construct class instance with
     */
    public constructor(value: boolean) {
        this.value = value
    }

    /**
     * Constructs a new Boolean with provided byte value
     *
     * @param value value to construct class instance with
     */
    public constructor(value: byte) {
        this.value = value != 0;
    }

    /**
     * Constructs a new Boolean with provided short value
     *
     * @param value value to construct class instance with
     */
    public constructor(value: short) {
        this.value = value != 0;
    }

    /**
     * Constructs a new Boolean with provided int value
     *
     * @param value value to construct class instance with
     */
    public constructor(value: int) {
        this.value = value != 0;
    }

    /**
     * Constructs a new Boolean with provided long value
     *
     * @param value value to construct class instance with
     */
    public constructor(value: long) {
        this.value = value != 0;
    }

    /**
     * Constructs a new Boolean with provided bigint value
     *
     * @param value value to construct class instance with
     */
    public constructor(value: bigint) {
        this.value = value != new BigInt(0);
    }

    /**
     * Constructs a new Boolean with provided char value
     *
     * @param value value to construct class instance with
     */
    public constructor(value: char) {
        this.value = value != c'\u0000';
    }

    /**
     * Constructs a new Boolean with provided float value
     *
     * @param value value to construct class instance with
     */
    public constructor(value: float) {
        this.value = (value != 0.0 && !isNaN(value));
    }

    /**
     * Constructs a new Boolean with provided number value
     *
     * @param value value to construct class instance with
     */
    public constructor(value: number) {
        this.value = (value != 0 && !isNaN(value));
    }


    /**
     * Constructs a new Boolean with provided string value
     *
     * @param value value to construct class instance with
     */
    public constructor(value: string) {
        this.value = value != "";
    }

    /**
     * Constructs a new Boolean with provided Object value
     *
     * @param value value to construct class instance with
     */
    public constructor(value: NullishType = undefined) {
        if (value == null || value == undefined) {
            this.value = false;
        } else if (value instanceof Boolean) {
            this.value = value.unboxed();
        } else if (value instanceof Number) {
            this.value = (value.unboxed() != 0 && !isNaN(value.unboxed()));
        } else if (value instanceof Char) {
            this.value = value.unboxed() != c'\u0000';
        } else if (value instanceof Float) {
            this.value = (value.unboxed() != 0.0 && !isNaN(value.unboxed()));
        } else if (value instanceof Long) {
            this.value = value.unboxed() != 0;
        } else if (value instanceof Int) {
            this.value = value.unboxed() != 0;
        } else if (value instanceof Byte) {
            this.value = value.unboxed() != 0;
        } else if (value instanceof Short) {
            this.value = value.unboxed() != 0;
        } else if (value instanceof String) {
            this.value = value != "";
        } else if (value instanceof Object) {
            this.value = true;
        } else {
            this.value = false;
        }
    }

    /**
     * Creates a new instance of a Boolean based on the specified value
     *
     * @param value The value to be converted to a Boolean (optional).
     *
     * @returns A new Boolean instance representing the specified value.
     */
    static $_invoke(): boolean {
        return Boolean.FALSE;
    }

    /**
     * Creates a new instance of a Boolean based on the specified value
     *
     * @param value The value to be converted to a Boolean (optional).
     *
     * @returns A new Boolean instance representing the specified value.
     */
    static $_invoke<T>(value: T): boolean {
        return new Boolean(value).valueOf();
    }

    /**
     * Returns the value of this instance as a primitive
     *
     * @returns primitive boolean value
     */
    public unboxed(): boolean {
        return this.value;
    }

    /**
     * Static instance that represents true boolean value
     */
    public static readonly TRUE = new Boolean(true);

    /**
     * Static instance that represents false boolean value
     */
    public static readonly FALSE = new Boolean(false);

    /**
     * Static method that converts primitive boolean to boxed version
     *
     * @param b value to be converted
     *
     * @returns boxed value that represents provided primitive value
     */
    public static valueOf(b: boolean): Boolean {
        if (b) {
            return Boolean.TRUE;
        };
        return Boolean.FALSE;
    }

    /**
     * Returns the value of this instance as primitive
     *
     * @returns primitive boolean value
     */
    public valueOf(): boolean {
        return this.value
    }

    /**
    * Checks for equality this instance with provided object, treated as a Boolean
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

        if (!(other instanceof Boolean)) {
            return false
        }

        return this.value == (other as Boolean).value
    }

    /**
     * Returns a hash code (integer representation) for this instance
     *
     * @returns representation of this instance
     */
    public override $_hashCode(): int {
        if (this.value) {
            return 1;
        }
        return 0;
    }

    /**
     * Converts this object to a string
     *
     * @returns "True" if this instance is true, "False" otherwise
     */
    public override toString(): String {
        if (this.value) {
            return "true";
        }
        return "false";
    }

    /**
     * Compares this instance to other Boolean object
     * The result is less than 0 if this instance lesser than provided object
     * 0 if they are equal
     * and greater than 0 otherwise.
     *
     * @param other Boolean object to compare with
     *
     * @returns result of the comparison
     */
    public override compareTo(other: Boolean): int {
        return this.$_hashCode() - other.$_hashCode();
    }

    /**
     * Checks if this instance is true
     *
     * @returns true if this instance is true, false otherwise
     */
    public isTrue(): boolean {
        return this.value == true;
    }

    /**
     * Checks if this instance is false
     *
     * @returns true if this instance is false, false otherwise
     */
    public isFalse(): boolean {
        return this.value == false;
    }

    /**
     * Inverts this instance value
     *
     * @returns true if value was false and vice versa
     */
    public negate(): Boolean {
        return Boolean.valueOf(!this.value)
    }

    /**
     * Does logical `and` on this instance and provided instance
     *
     * @param other provided instance
     *
     * @returns value as a result of logical `and`
     */
    public and(other: Boolean): Boolean {
        return Boolean.valueOf(this.value && other.unboxed())
    }

    /**
     * Does logical `or` on this instance and provided instance
     *
     * @param other provided instance
     *
     * @returns value as a result of logical `or`
     */
    public or(other: Boolean): Boolean {
        return Boolean.valueOf(this.value || other.unboxed())
    }

    /**
     * Does `xor` on this instance and provided instance
     *
     * @param other provided instance
     *
     * @returns value as a result of `xor`
     */
    public xor(other: Boolean): Boolean {
        return Boolean.valueOf(this.value ^ other.unboxed())
    }

    /**
     * Creates a Boolean instance based on JSONValue
     *
     * @param json: JSONValue - a JSON representation
     *
     * @throws JSONTypeError if json does not encode a valid boolean literal
     *
     * @returns Boolean - true if JSON encodes true literal, false if JSON encodes false literal
     */
    public createFromJSONValue(json: JSONValue): Boolean {
        if (json instanceof JSONFalse) {
            return Boolean.FALSE
        } else if (json instanceof JSONTrue) {
            return Boolean.TRUE
        }
        throw new JSONTypeError("Cannot create Boolean from JSON", new ErrorOptions(json as Object))
    }
}
