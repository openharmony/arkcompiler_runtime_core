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
 * @file Defines the StringBuilder for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Performance-oriented class for string construction
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class StringBuilder {
    private static readonly INITIAL_BUF_SIZE: int = 16;

    private buf: FixedArray<(Object|undefined)> = new FixedArray<Object|undefined>(StringBuilder.INITIAL_BUF_SIZE); // array with pointers to strings or char[]
    private index: int = 0;                                             // index of the current free element in the buf
    private length: int = 0;                                            // length of the resulting string
    private compress: boolean = true;                                   // compress or not the resulting string.

    /**
     * Construct a new builder instance with the initial buffer of 16 chars
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor() {}

    /**
     * Construct a new builder instance with the provided array of chars
     *
     * @param { FixedArray<char> } fromChars array that will be used to initialize the builder
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(fromChars: FixedArray<char>) {
        let s: String = new String(fromChars);
        this.buf[this.index++] = s;
        this.length = s.getLength();
        if (!s.isCompressed()) {
            this.compress = false;
        }
    }

    /**
     * Construct a new builder instance with the provided array of chars
     *
     * @param { char[] } fromChars array that will be used to initialize the builder
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(fromChars: char[]) {
        let s: String = new String(fromChars);
        this.buf[this.index++] = s;
        this.length = s.getLength();
        if (!s.isCompressed()) {
            this.compress = false;
        }
    }

    /**
     * Constructs a new builder instance with provided string
     *
     * @param { String } s string that will be used to initialize the builder
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor (s: String) {
        this.buf[this.index++] = s;
        this.length = s.getLength();
        if (!s.isCompressed()) {
            this.compress = false;
        }
    }

    /**
     * Length of string accumulated so far
     *
     * @returns { int } length of string accumulated so far
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get stringLength(): int {
        return this.length;
    }

    /**
     * Concatenates two strings and return a result as a new string
     *
     * @param { String } lhs left string (prefix)
     * @param { String } rhs right string (suffix)
     * @returns { String } result of concatenation
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static concatStrings(lhs: String, rhs: String): String
    {
        return lhs.concat(rhs);
    }


    /**
     * Appends a string to the builder's internal buffer
     *
     * @param { String } s string to be appended
     * @returns { StringBuilder } builder with updated internal buffer
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native append(s: String): StringBuilder;
    private native append(s0: String, s1: String): StringBuilder;
    private native append(s0: String, s1: String, s2: String): StringBuilder;
    private native append(s0: String, s1: String, s2: String, s3: String): StringBuilder;

    /**
     * Appends a boolean as string to the builder's internal buffer
     *
     * @param { boolean } i value to be appended
     * @returns { StringBuilder } builder with updated internal buffer
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native append(i: boolean): StringBuilder;

    /**
     * Appends a byte as string to the builder's internal buffer
     *
     * @param { byte } i value to be appended
     * @returns { StringBuilder } builder with updated internal buffer
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native append(i: byte): StringBuilder;

    /**
     * Appends a char to the builder's internal buffer
     *
     * @param { char } i value to be appended
     * @returns { StringBuilder } builder with updated internal buffer
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native append(i: char): StringBuilder;

    /**
     * Appends a short as string to the builder's internal buffer
     *
     * @param { short } i value to be appended
     * @returns { StringBuilder } builder with updated internal buffer
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native append(i: short): StringBuilder;

    /**
     * Appends an int as string to the builder's internal buffer
     *
     * @param { int } i value to be appended
     * @returns { StringBuilder } builder with updated internal buffer
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native append(i: int): StringBuilder;

    /**
     * Appends a long as string to the builder's internal buffer
     *
     * @param { long } i value to be appended
     * @returns { StringBuilder } builder with updated internal buffer
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native append(i: long): StringBuilder;

    /**
     * Appends a float as string to the builder's internal buffer
     *
     * @param { float } i value to be appended
     * @returns { StringBuilder } builder with updated internal buffer
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native append(i: float): StringBuilder;

    /**
     * Appends a double as string to the builder's internal buffer
     *
     * @param { double } i value to be appended
     * @returns { StringBuilder } builder with updated internal buffer
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native append(i: double): StringBuilder;

    /**
     * Appends an object representation as string to the builder's internal buffer
     *
     * @param { object } o object that will be converted to a stringitg
     * @returns { StringBuilder } builder with updated internal buffer
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public append(o: Object): StringBuilder {
        this.append(o.toString());
        return this;
    }

    /**
     * Returns the string that was formed as a result of all append operations
     * @returns { String } - builder's current buffer as string
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native override toString(): String;

    /**
     * Converts the primitive to a string
     *
     * @param { boolean } i value to be converted
     * @returns { String } result of the conversion
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toString(i: boolean): String;

    /**
     * Converts the primitive to a string
     *
     * @param { byte } i value to be converted
     * @returns result of the conversion
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toString(i: byte): String;

    /**
     * Converts the primitive to a string
     *
     * @param { char } i value to be converted
     * @returns { String } result of the conversion
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toString(i: char): String;

    /**
     * Converts the primitive to a string
     *
     * @param { short } i value to be converted
     * @returns { String } result of the conversion
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toString(i: short): String;

    /**
     * Converts the primitive to a string
     *
     * @param { int } i value to be converted
     * @returns { String } result of the conversion
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toString(i: int): String;

    /**
     * Converts the primitive to a string
     *
     * @param { long } i value to be converted
     * @returns { String } result of the conversion
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native toString(i: long): String;

    /**
     * Converts the primitive to a string
     *
     * @param { float } f value to be converted
     * @returns { String } result of the conversion
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toString(f: float): String {
        return Float.toString(f);
    }

    /**
     * Converts the primitive to a string
     *
     * @param { double } d value to be converted
     * @returns { String } result of the conversion
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toString(d: double): String {
        return Double.toString(d);
    }

}

final class DoubleToStringCacheElement {
    string: string = "";
    // `lock` here and below is internal field used to provide atomicity
    // and to align `number` by 8 bytes
    lock: int;
    number: double;
}

final class FloatToStringCacheElement {
    string: string = "";
    lock: int;
    number: float;
}

final class LongToStringCacheElement {
    string: string = "";
    lock: int;
    number: long;
}
