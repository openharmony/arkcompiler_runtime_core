/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
 * Performance-oriented class for string construction
 */
export final class StringBuilder {
    private value: char[];
    private count: int;

    /**
     * Construct a new builder instance with the initial buffer of 16 chars
     */
    constructor() {
        this.value = new char[16] ;
    }

    /**
     * Construct a new builder instance with the provided array of chars
     *
     * @param fromChars array that will be used to initialize the builder
     */
    constructor(fromChars: char[]) {
        let capacity = StringBuilder.newCapacity(16, fromChars.length);
        this.count = fromChars.length;
        this.value = new char[capacity];
        for (let i = 0; i < this.count; ++i) {
            this.value[i] = fromChars[i];
        }
    }

    /**
     * Constructs a new builder instance with provided string
     *
     * @param s string that will be used to initialize the builder
     */
    public constructor (s: String) {
        let capacity = StringBuilder.newCapacity(16, s.length());
        this.count = s.length();
        this.value = new char[capacity];
        for (let i = 0; i < this.count; ++i) {
            this.value[i] = s.charAt(i);
        }
    }

    /*
     * adjust capacity so that it is either doubled plus 2 of the current
     * capacity or set to the `size` if the former is not enough
     */
    private static newCapacity(capacity : int, size : int) : int {
        if (capacity < size) {
            capacity += (capacity + 2);
            capacity = capacity >= size ? capacity : size;
        }
        return capacity;
    }

    /*
     * ensure the capacity is enough to append the size amount of chars
     */
    private static ensureCapacity(sb: StringBuilder, size: int) : char[] {
        let capacity = sb.value.length
        if (size + sb.count <= capacity) {
            return sb.value;
        }
        capacity = StringBuilder.newCapacity(capacity, size + sb.count);
        let newvalue = new char[capacity];
        for (let i = 0; i < sb.count; ++i) {
            newvalue[i] = sb.value[i];
        }
        sb.value = newvalue;
        return sb.value;
    }

    /**
     * Concatenates two strings and return a result as a new string
     *
     * @param lhs left string (prefix)
     *
     * @param rhs right string (suffix)
     *
     * @returns result of concatenation
     */
    public static native concatStrings(lhs: String, rhs: String): String;

    /**
     * Appends an object representation as string to the builder's internal buffer
     *
     * @param o object that will be converted to a string
     *
     * @returns builder with updated internal buffer
     */
    public append(o: Object): StringBuilder {
        this.append(o.toString());
        return this;
    }

    /**
     * Appends a string to the builder's internal buffer
     *
     * @param s string to be appended
     *
     * @returns builder with updated internal buffer
     */
    public native append(s: String): StringBuilder;

    /**
     * Appends a boolean as string to the builder's internal buffer
     *
     * @param i value to be appended
     *
     * @returns builder with updated internal buffer
     */
    public native append(i: boolean): StringBuilder;

    /**
     * Appends a byte as string to the builder's internal buffer
     *
     * @param i value to be appended
     *
     * @returns builder with updated internal buffer
     */
    public append(i: byte): StringBuilder {
        this.append(StringBuilder.toString(i));
        return this;
    }

    /**
     * Appends a short as string to the builder's internal buffer
     *
     * @param i value to be appended
     *
     * @returns builder with updated internal buffer
     */
    public append(i: short): StringBuilder {
        this.append(StringBuilder.toString(i));
        return this;
    }

    /**
     * Appends a char to the builder's internal buffer
     *
     * @param i value to be appended
     *
     * @returns builder with updated internal buffer
     */
    public native append(i: char): StringBuilder;

    /**
     * Appends an int as string to the builder's internal buffer
     *
     * @param i value to be appended
     *
     * @returns builder with updated internal buffer
     */
    public native append(i: int): StringBuilder;

    /**
     * Appends a long as string to the builder's internal buffer
     *
     * @param i value to be appended
     *
     * @returns builder with updated internal buffer
     */
    public native append(i: long): StringBuilder;

    /**
     * Appends a float as string to the builder's internal buffer
     *
     * @param i value to be appended
     *
     * @returns builder with updated internal buffer
     */
    public append(i: float): StringBuilder {
        this.append(StringBuilder.toString(i));
        return this;
    }

    /**
     * Appends a double as string to the builder's internal buffer
     *
     * @param i value to be appended
     *
     * @returns builder with updated internal buffer
     */
    public append(i: double): StringBuilder {
        this.append(StringBuilder.toString(i));
        return this;
    }

    /**
     * Returns the string that was formed as a result of all append operations
     *
     * @see append
     *
     * @returns String - builder's current buffer as string
     */
    public override toString(): String {
        let r = new char[this.count];
        for (let i = 0; i < this.count; ++i) {
            r[i] = this.value[i];
        }
        return new String(r);
    }

    /**
     * Converts the primitive to a string
     *
     * @param i value to be converted
     *
     * @returns result of the conversion
     */
    public static native toString(i: boolean): String;

    /**
     * Converts the primitive to a string
     *
     * @param i value to be converted
     *
     * @returns result of the conversion
     */
    public static native toString(i: byte): String;

    /**
     * Converts the primitive to a string
     *
     * @param i value to be converted
     *
     * @returns result of the conversion
     */
    public static native toString(i: short): String;

    /**
     * Converts the primitive to a string
     *
     * @param i value to be converted
     *
     * @returns result of the conversion
     */
    public static native toString(i: char): String;

    /**
     * Converts the primitive to a string
     *
     * @param i value to be converted
     *
     * @returns result of the conversion
     */
    public static native toString(i: int): String;

    /**
     * Converts the primitive to a string
     *
     * @param i value to be converted
     *
     * @returns result of the conversion
     */
    public static native toString(i: long): String;

    /**
     * Converts the primitive to a string
     *
     * @param i value to be converted
     *
     * @returns result of the conversion
     */
    public static native toString(i: float): String;

    /**
     * Converts the primitive to a string
     *
     * @param i value to be converted
     *
     * @returns result of the conversion
     */
    public static native toString(i: double): String;

}
