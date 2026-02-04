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
 * @file Defines the Boolean for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Represents boxed boolean value and related operations
 *
 * @extends Object
 * @implements Comparable<Boolean>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class Boolean extends Object implements Comparable<Boolean> {
    private value: boolean;

    /**
     * Constructs a new Boolean with provided value
     *
     * @param { boolean } value value to construct class instance with
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: boolean) {
        this.value = value
    }

    /**
     * Constructs a new Boolean with provided byte value
     *
     * @param { byte } value value to construct class instance with
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: byte) {
        this.value = value != 0;
    }

    /**
     * Constructs a new Boolean with provided char value
     *
     * @param { char } value value to construct class instance with
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: char) {
        this.value = value != c'\u0000';
    }

    /**
     * Constructs a new Boolean with provided short value
     *
     * @param { short } value value to construct class instance with
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: short) {
        this.value = value != 0;
    }

    /**
     * Constructs a new Boolean with provided int value
     *
     * @param { int } value value to construct class instance with
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: int) {
        this.value = value != 0;
    }

    /**
     * Constructs a new Boolean with provided long value
     *
     * @param { long } value value to construct class instance with
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: long) {
        this.value = value != 0;
    }

    /**
     * Constructs a new Boolean with provided bigint value
     *
     * @param { bigint } value value to construct class instance with
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: bigint) {
        this.value = value != new BigInt(0);
    }

    /**
     * Constructs a new Boolean with provided float value
     *
     * @param { float } value value to construct class instance with
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: float) {
        this.value = (value != 0.0 && !isNaN(value));
    }

    /**
     * Constructs a new Boolean with provided number value
     *
     * @param { number } value value to construct class instance with
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: number) {
        this.value = (value != 0 && !isNaN(value));
    }

    /**
     * Constructs a new Boolean with provided string value
     *
     * @param { string } value value to construct class instance with
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: string) {
        this.value = value != '';
    }

    /**
     * Constructs a new Boolean with provided Object value
     *
     * @param { Any } value value to construct class instance with
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: Any = undefined) {
        if (value == null || value == undefined) {
            this.value = false;
        } else if (value instanceof Boolean) {
            this.value = value;
        } else if (value instanceof Number) {
            this.value = (value != 0 && !isNaN(value));
        } else if (value instanceof Char) {
            this.value = value != c'\u0000';
        } else if (value instanceof Float) {
            this.value = (value != 0.0 && !isNaN(value));
        } else if (value instanceof Long) {
            this.value = value != 0;
        } else if (value instanceof Int) {
            this.value = value != 0;
        } else if (value instanceof Byte) {
            this.value = value != 0;
        } else if (value instanceof Short) {
            this.value = value != 0;
        } else if (value instanceof String) {
            this.value = value != '';
        } else if (value instanceof Object) {
            this.value = true;
        } else {
            this.value = false;
        }
    }

    /**
     * Creates a new instance of a Boolean based on the specified value
     *
     * @returns { boolean } A new Boolean instance representing the specified value.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static $_invoke(): boolean {
        return Boolean.FALSE;
    }

    /**
     * Creates a new instance of a Boolean based on the specified value
     *
     * @param { T } value The value that can be represented in Boolean.
     * @returns { boolean } A new Boolean instance representing the specified value.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static $_invoke<T>(value: T): boolean {
        return new Boolean(value).valueOf();
    }

    /**
     * Static instance that represents true boolean value
     *
     * @type { Boolean }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly TRUE = new Boolean(true);

    /**
     * Static instance that represents false boolean value
     *
     * @type { Boolean }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly FALSE = new Boolean(false);

    /**
     * Returns the primitive as boolean value
     *
     * @param { boolean } value value to cast
     * @returns { boolean } Returns the input boolean value unchanged.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toBoolean(value: boolean): boolean {
        return value;
    }

    /**
     * Returns value of this instance
     *
     * @returns { boolean } The instance value converted to boolean.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toBoolean(): boolean {
        return this.value;
    }

    /**
     * Static method that converts primitive boolean to boxed version
     *
     * @param { boolean } b value to be converted
     * @returns { Boolean } The boxed Boolean object representing the input value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
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
     * @returns { boolean } The primitive boolean value of this instance
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public valueOf(): boolean {
        return this.value
    }

    /**
     * Checks for equality this instance with provided object, treated as a Boolean Returns false if type of provided
     * object is not the same as this type
     *
     * @param { Any } other object to be checked against
     * @returns { boolean } True if the objects are equal, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public equals(other: Any): boolean {
        if (this === other) {
            return true
        }

        if (!(other instanceof Boolean)) {
            return false
        }

        return this.value == (other as Boolean).value
    }

    /**
     * Converts the primitive to a string
     *
     * @param { boolean } v value to be converted
     * @returns { string } The string representation of the boolean value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toString(v: boolean): string {
        if (v) {
            return 'true';
        }
        return 'false';
    }

    /**
     * Converts this object to a string
     *
     * @returns { String } The string representation of this Boolean object
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toString(): String {
        if (this.value) {
            return 'true';
        }
        return 'false';
    }

    /**
     * Compares this instance to other Boolean object The result is less than 0 if this instance lesser than provided
     * object 0 if they are equal and greater than 0 otherwise.
     *
     * @param { Boolean } other Boolean object to compare with
     * @returns { int } Negative if this < other,0 if equal, positive if this > other
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override compareTo(other: Boolean): int {
        let thisInt = this.value ? 1 : 0
        let otherInt = other.value ? 1 : 0
        return thisInt - otherInt;
    }

    /**
     * Checks if this instance is true
     *
     * @returns { boolean } True if the instance value is true, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isTrue(): boolean {
        return this.value == true;
    }

    /**
     * Checks if this instance is false
     *
     * @returns { boolean } True if the instance value is false, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isFalse(): boolean {
        return this.value == false;
    }

    /**
     * Inverts this instance value
     *
     * @returns { Boolean } The logical negation of this Boolean instance
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public negate(): Boolean {
        return Boolean.valueOf(!this.value)
    }

    /**
     * Does logical `and` on this instance and provided instance
     *
     * @param { Boolean } other provided instance
     * @returns { Boolean } The logical AND result of both Boolean instances
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public and(other: Boolean): Boolean {
        return Boolean.valueOf(this.value && other)
    }

    /**
     * Does logical `or` on this instance and provided instance
     *
     * @param { Boolean } other provided instance
     * @returns { Boolean } The logical OR result of both Boolean instances
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public or(other: Boolean): Boolean {
        return Boolean.valueOf(this.value || other)
    }

    /**
     * Does `xor` on this instance and provided instance
     *
     * @param { Boolean } other provided instance
     * @returns { Boolean } The logical XOR result of both Boolean instances
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public xor(other: Boolean): Boolean {
        return Boolean.valueOf(this.value ^ other)
    }
}
