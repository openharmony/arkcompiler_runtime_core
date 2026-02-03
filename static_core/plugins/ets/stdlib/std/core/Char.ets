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
 * @file Defines the Char for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Define the alias UTF_16_CodePoint for int.
 *
 * @typedef { int } UTF_16_CodePoint
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export type UTF_16_CodePoint = int;

/**
 * Represents boxed char value and related operations
 *
 * @extends Object
 * @implements Comparable<Char>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class Char extends Object implements Comparable<Char> {
    private value: char;

    /**
     * constructor() creates a default Char object.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = c'\u0000';
    }

    /**
     * constructor(char) creates a Char object from a specified primitive char.
     *
     * @param { char } value a primitive char to create Char from.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: char) {
        this.value = value;
    }

    /**
     * valueOf(char) creates a Char object from a primitive char. This method is preferred over
     * {@link <constructor(char)>} since this method can use a cached Char object, i.e. this method might
     * be more efficient.
     *
     * @param { char } value a primitive char to create Char from.
     * @returns { Char }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static valueOf(value: char): Char {
        // TODO(ivan-tyulyandin): caching is possible
        return new Char(value);
    }

    /**
     * equals(Object) compares two Chars by their underlying primitive chars. Returns false if the argument
     * is not an instance of Char.
     *
     * @param { Any } other a reference to object to be compared with.
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public equals(other: Any): boolean {
        if (this === other) {
            return true
        }

        if (!(other instanceof Char)) {
            return false
        }

        return this.value == (other as Char).value
    }

    /**
     * toString() converts Char to String object that contains a single element with the underlying char.
     *
     * @returns { String }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toString(): String {
        return Char.toString(this.value)
    }

    /**
     * The `toLocaleString()` method returns the string representation of the given Char in the form of a copy
     * of the original object.
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
     * compare(Char, Char) compares two Chars by their underlying chars.
     *
     * @param { Char } lhs the first Char to compare.
     * @param { Char } rhs the second Char to compare.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static compare(lhs: Char, rhs: Char): boolean {
        return (lhs == rhs);
    }

    /**
     * Compares this instance to other Char object The result is less than 0 if this instance lesser than provided
     * object 0 if they are equal and greater than 0 otherwise.
     *
     * @param { Char } other Char object to compare with
     * @returns { int }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override compareTo(other: Char): int {
        if (this.value < other) {
            return -1;
        }
        if (this.value == other) {
            return 0;
        }
        return 1;
    }

    /**
     * Compares this instance to other Char object The result is less than 0 if this instance lesser than provided
     * object 0 if they are equal and greater than 0 otherwise.
     *
     * @type { int }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly CHAR_BIT_SIZE = 16;

    /**
     * Compares this instance to other Char object The result is less than 0 if this instance lesser than provided
     * object 0 if they are equal and greater than 0 otherwise.
     *
     * @type { int }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly MAX_CODE_POINT = 0x10FFFF

    /**
     * Compares this instance to other Char object The result is less than 0 if this instance lesser than provided
     * object 0 if they are equal and greater than 0 otherwise.
     *
     * @type { char }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly HIGH_SURROGATE_MIN: char = c'\uD800';

    /**
     * Compares this instance to other Char object The result is less than 0 if this instance lesser than provided
     * object 0 if they are equal and greater than 0 otherwise.
     *
     * @type { char }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly HIGH_SURROGATE_MAX: char = c'\uDBFF';

    /**
     * Compares this instance to other Char object The result is less than 0 if this instance lesser than provided
     * object 0 if they are equal and greater than 0 otherwise.
     *
     * @type { char }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly LOW_SURROGATE_MIN: char = c'\uDC00';

    /**
     * Compares this instance to other Char object The result is less than 0 if this instance lesser than provided
     * object 0 if they are equal and greater than 0 otherwise.
     *
     * @type { char }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly LOW_SURROGATE_MAX: char = c'\uDFFF';
    private static readonly BMP_BORDER: UTF_16_CodePoint = 0x10000;

    /**
     * `MIN_VALUE` is a smallest value of type `char`
     *
     * @type { char }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly MIN_VALUE: char = c'\u0000';

    /**
     * `MAX_VALUE` is a largest value of type `char`
     *
     * @type { char }
     * @readonly
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly MAX_VALUE: char = c'\uFFFF';

    /**
     * Returns value of this instance
     *
     * @returns { byte }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toByte(): byte {
        return this.value.toByte();
    }

    /**
     * Returns value of this instance
     *
     * @returns { short }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toShort(): short {
        return this.value.toShort();
    }

    /**
     * Returns value of this instance
     *
     * @returns { int }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toInt(): int {
        return this.value.toInt();
    }

    /**
     * Returns value of this instance
     *
     * @returns { long }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toLong(): long {
        return this.value.toLong();
    }

    /**
     * Returns value of this instance
     *
     * @returns { float }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toFloat(): float {
        return this.value.toFloat();
    }

    /**
     * Returns value of this instance
     *
     * @returns { double }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toDouble(): double {
        return this.value.toDouble();
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
        return this.value;
    }

    /**
     * Returns the primitive as byte value
     *
     * @param { char } value value to cast
     * @returns { byte }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toByte(value: char): byte;

    /**
     * Returns the primitive as short value
     *
     * @param { char } value value to cast
     * @returns { short }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toShort(value: char): short;

    /**
     * Returns the primitive as int value
     *
     * @param { char } value value to cast
     * @returns { int }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toInt(value: char): int;

    /**
     * Returns the primitive as long value
     *
     * @param { char } value value to cast
     * @returns { long }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toLong(value: char): long;

    /**
     * Returns the primitive as float value
     *
     * @param { char } value value to cast
     * @returns { float }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toFloat(value: char): float;

    /**
     * Returns the primitive as float value
     *
     * @param { char } value value to cast
     * @returns { double }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toDouble(value: char): double;

    /**
     * Returns the primitive as string value
     *
     * @param { char } value value to cast
     * @returns { string }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toString(value: char): string {
        return String.fromCharCode(value.toInt())
    }

    /**
     * Returns the primitive as char value
     *
     * @param { char } value value to cast
     * @returns { char }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toChar(value: char): char {
        return value;
    };

    /**
     * isInBasicMultilingualPlane(char) checks if the char is in Basic Multilingual Plane.
     * See UTF-16 for more details.
     *
     * @param { char } value the char to be checked.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isInBasicMultilingualPlane(value: char): boolean {
        if (value.toInt() < Char.BMP_BORDER) {
            return (value.compareTo(Char.HIGH_SURROGATE_MIN) < 0) || (value.compareTo(Char.LOW_SURROGATE_MAX) > 0);
        }
        return false
    }

    /**
     * isInBasicMultilingualPlane(UTF_16_CodePoint) checks if the code point is in Basic Multilingual Plane.
     * See UTF-16 for more details.
     *
     * @param { UTF_16_CodePoint } value the code point to be checked.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isInBasicMultilingualPlane(value: UTF_16_CodePoint): boolean {
        if (value < Char.BMP_BORDER) {
            return Char.isInBasicMultilingualPlane(UTF_16_CodePoint.toChar(value));
        }
        return false;
    }

    /**
     * isInBasicMultilingualPlane() checks if the underlying char is in Basic Multilingual Plane.
     * See UTF-16 for more details.
     *
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isInBasicMultilingualPlane(): boolean {
        return Char.isInBasicMultilingualPlane(this.value);
    }

    /**
     * isValidCodePoint() checks if the code point is correctly encoded.
     * See UTF-16 for more details.
     *
     * @param { UTF_16_CodePoint } codePoint 
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isValidCodePoint(codePoint: UTF_16_CodePoint): boolean {
        return (codePoint >= 0 && codePoint <= Char.MAX_CODE_POINT &&
            !(codePoint >= (Char.HIGH_SURROGATE_MIN.toInt()) && codePoint <= (Char.LOW_SURROGATE_MAX.toInt())));
    }

    /**
     * codeUnitsToEncode(UTF_16_CodePoint) counts a number of code units to encode the UTF-16 code point.
     * See UTF-16 for more details.
     *
     * @param { UTF_16_CodePoint } value UTF-16 code point to be examinated.
     * @returns { int }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static codeUnitsToEncode(value: UTF_16_CodePoint): int {
        if (Char.isInBasicMultilingualPlane(value)) {
            return 1;
        }
        return 2;
    }

    /**
     * isHighSurrogate(char) checks if the char is a high surrogate.
     *
     * @param { char } value the char to be checked.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isHighSurrogate(value: char): boolean {
        return (Char.HIGH_SURROGATE_MIN.compareTo(value) <= 0)
            && (Char.HIGH_SURROGATE_MAX.compareTo(value) >= 0);
    }

    /**
     * isLowSurrogate(char) checks if the char is a low surrogate.
     *
     * @param { char } value the char to be checked.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isLowSurrogate(value: char): boolean {
        return (Char.LOW_SURROGATE_MIN.compareTo(value) <= 0)
            && (Char.LOW_SURROGATE_MAX.compareTo(value) >= 0);
    }

    /**
     * getHighSurrogate(UTF_16_CodePoint) splits code point as a two code units and return the first one.
     * The result can be malformed und thus has to be checked with {@link <isHighSurrogate(char)>}.
     *
     * @param { UTF_16_CodePoint } value an encoded code point.
     * @returns { char }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static getHighSurrogate(value: UTF_16_CodePoint): char {
        return (((value - 0x10000) >>> 10) + Char.HIGH_SURROGATE_MIN.toInt()).toChar()
    }

    /**
     * getLowSurrogate(UTF_16_CodePoint) splits code point as a two code units and return the second one.
     * The result can be malformed und thus has to be checked with {@link <isLowSurrogate(char)>}.
     *
     * @param { UTF_16_CodePoint } value an encoded code point.
     * @returns { char }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static getLowSurrogate(value: UTF_16_CodePoint): char {
        return (((value - 0x10000) & 0x3FF) + Char.LOW_SURROGATE_MIN.toInt()).toChar();
    }

    /**
     * isPartOfSurrogatePair(char) checks whether the char is low or high surrogate.
     *
     * @param { char } value the char to be tested.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isPartOfSurrogatePair(value: char): boolean {
        return Char.isHighSurrogate(value) || Char.isLowSurrogate(value);
    }

    /**
     * isPartOfSurrogatePair() checks whether the underlying char is low or high surrogate.
     *
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isPartOfSurrogatePair(): boolean {
        return Char.isPartOfSurrogatePair(this.value);
    }

    /**
     * charsToCodePoint(char, char) combines to chars to code point
     *
     * @param { char } highValue None
     * @param { char } lowValue None
     * @returns { UTF_16_CodePoint }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static charsToCodePoint(highValue: char, lowValue: char): UTF_16_CodePoint {
        return ((highValue.toInt() - Char.HIGH_SURROGATE_MIN.toInt()) * 0x400) + (lowValue.toInt() - Char.LOW_SURROGATE_MIN.toInt()) + 0x10000;
    }

    /**
     * isBinDigit() checks whether the char represents a binary digit.
     *
     * @param { char } value a char to check.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isBinDigit(value: char): boolean {
        let diff: int = value.toInt() - Char.toInt(c'0');
        return (0 == diff) || (diff == 1);
    }

    /**
     * isBinDigit() checks whether the underlying char represents a binary digit.
     *
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isBinDigit(): boolean {
        return Char.isBinDigit(this.value);
    }

    /**
     * isDecDigit() checks whether the char represents a decimal digit.
     *
     * @param { char } value a char to check.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isDecDigit(value: char): boolean {
        let diff: int = value.toInt() - Char.toInt(c'0');
        return (0 <= diff) && (diff <= 9);
    }

    /**
     * isDecDigit() checks whether the underlying char represents a decimal digit.
     *
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isDecDigit(): boolean {
        return Char.isDecDigit(this.value);
    }

    /**
     * isBinDigit() checks whether the char represents a hexadecimal digit.
     *
     * @param { char } value a char to check.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isHexDigit(value: char): boolean {
        let isDigit = (0 <= value.compareTo(c'0')) && (value.compareTo(c'9') <= 0)
        let isChar = ((0 <= value.compareTo(c'a')) && (value.compareTo(c'f') <= 0))
                  || ((0 <= value.compareTo(c'A')) && (value.compareTo(c'F') <= 0))
        return isDigit || isChar
    }

    /**
     * isHexDigit() checks whether the underlying char represents a hexadecimal digit.
     *
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isHexDigit(): boolean {
        return Char.isHexDigit(this.value);
    }

    /**
     * isLetter(char) checks whether the char is a letter.
     *
     * @param { char } value a char to be tested.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isLetter(value: char): boolean {
        return (0 <= value.compareTo(c'A')) && (value.compareTo(c'Z') <= 0)
            || (0 <= value.compareTo(c'a')) && (value.compareTo(c'z') <= 0);
    }

    /**
     * isLetter() checks whether the underlying char is a letter.
     *
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isLetter(): boolean {
        return Char.isLetter(this.value);
    }

    /**
     * isUpperCase(char) checks whether the char is an upper case letter.
     *
     * @param { char } value a char to be tested.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native isUpperCase(value: char): boolean;

    /**
     * isUpperCase() checks whether the underlying char is an upper case letter.
     *
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isUpperCase(): boolean {
        return Char.isUpperCase(this.value);
    }

    /**
     * isLowerCase(char) checks whether the char is a lower case letter.
     *
     * @param { char } value a char to be tested.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native isLowerCase(value: char): boolean

    /**
     * isLowerCase() checks whether the underlying char is a lower case letter.
     *
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isLowerCase(): boolean {
        return Char.isLowerCase(this.value);
    }

    /**
     * toUpperCase(char) converts the char to upper case if it is in lower case, otherwise the char returned itself
     *
     * @param { char } value a char to transform to upper case.
     * @returns { char }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toUpperCase(value: char): char

    /**
     * toUpperCase() converts the underlying char to upper case if it is in lower case, otherwise the char unchanged
     *
     * @returns { Char }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toUpperCase(): Char {
        return Char.toUpperCase(this.value);
    }

    /**
     * toLowerCase(char) converts the char to lower case if it is in upper case, otherwise the char returned itself
     *
     * @param { char } value a char to transform to upper case.
     * @returns { char }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toLowerCase(value: char): char

    /**
     * toLowerCase() converts the underlying char to a lower case if it is in upper case, otherwise the char unchanged.
     *
     * @returns { Char }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toLowerCase(): Char {
        return Char.toLowerCase(this.value);
    }

    /**
     * isWhiteSpace(char) checks whether the char is a whitespace char.
     *
     * @param { char } value a char to be tested.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native isWhiteSpace(value: char): boolean;

    /**
     * isWhiteSpace() checks whether the underlying char is a whitespace char.
     *
     * @returns { boolean }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
     public isWhiteSpace(): boolean {
        return Char.isWhiteSpace(this.value);
    }
}
