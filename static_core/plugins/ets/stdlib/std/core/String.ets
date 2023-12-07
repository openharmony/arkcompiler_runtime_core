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

const MAX_CODE_POINT = 0x10FFFF
const MAX_CODE_UNIT = 0xFFFF

/**
 * Unicode string
 */
export class String extends Object implements Comparable<String>, JSONable<String> {

    // The constructors below are implemented via initobj instruction
    /**
     * Constructs an empty String
     */
    public constructor() {}

    /**
     * Constructs String from chars array initializer
     *
     * @param data initializer
     */
    public constructor(data: char[]) {}

    /**
     * Constructs String from another String
     *
     * @param otherStr initializer
     */
    public constructor(otherStr: String) {}

    /**
     * Checks equality of this string and another Object as String
     *
     * @param to another object to compare
     *
     * @returns true if strings are equal and false otherwise
     *
     * @remarks
     * Implemented as native function,  @see `equals()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L596).
     */
    public native override equals(to: NullishType): boolean;

    /**
     * Length of this string
     *
     * @returns length of this string
     *
     * @remarks
     * Implemented as native function,  @see `getLength()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L563).
     */
    public native getLength(): int;

    /**
     * Length of this string
     *
     * @returns length of this string
     */
    public get length(): number {
        return this.getLength() as number
    }

    /**
     * Getter for char at some index
     *
     * @param index index in char array inside String
     *
     * @returns char value at index
     *
     * @throws StringIndexOutOfBoundsException if index is negative or >= length
     *
     * @remarks
     * Implemented as native function,  @see `charAt()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L585).
     */
    public charAt(index: number): String {
        let c = new char[1];
        c[0] = this.charAt(index as int)
        return new String(c)
    }

    /**
     * Getter for char at some index
     *
     * @param index index in char array inside String
     *
     * @returns char value at index
     *
     * @throws StringIndexOutOfBoundsException if index is negative or >= length
     *
     * @remarks
     * Implemented as native function,  @see `charAt()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L585).
     */
    public native charAt(index: int): char;

    /**
     * Checks if this string is empty
     *
     * @returns true if empty and false otherwise
     * @remarks
     * Implemented as native function,  @see `length()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L574).
     */
    public native isEmpty(): boolean;

    /**
     * Gets the codepoint at the specified index in this string.
     * Is similar to charAt(index), but if the character at the
     * index is the beginning of a surrogate pair, the int
     * representation for this codepoint is returned. Otherwise,
     * the character at the index is returned.
     *
     * @param index index of the potential surrogate pair
     *
     * @throws StringIndexOutOfBoundsException if index is negative or >= length
     *
     * @returns the codepoint at the specified index
     */
    public codePointAt(index: number): Number | undefined {
        if (index < 0 || index >= this.getLength()) {
            return undefined
        }
        return this.codePointAt(index as int) as number
    }

    /**
     * Gets the codepoint at the specified index in this string.
     * Is similar to charAt(index), but if the character at the
     * index is the beginning of a surrogate pair, the int
     * representation for this codepoint is returned. Otherwise,
     * the character at the index is returned.
     *
     * @param index index of the potential surrogate pair
     *
     * @throws StringIndexOutOfBoundsException if index is negative or >= length
     *
     * @returns the codepoint at the specified index
     */
    public codePointAt(index: int): int {
        assert index >= 0 && index < this.getLength(): "Index is out of bound";
        let highValue: char = this.charAt(index);
        if (!Char.isHighSurrogate(highValue) || ++index == this.getLength()) {
            return highValue as int;
        }
        let lowValue: char = this.charAt(index);
        if (!Char.isLowSurrogate(lowValue)) {
            return highValue as int;
        }
        return Char.charsToCodePoint(highValue, lowValue);
    }

    /**
     * Returns the amount of full codepoints between begin and end
     * indexes. Characters outside the range are not counted, even if
     * the range ends in the middle of a surrogate pair.
     *
     * @param begin index to start from
     * @param end past the ending index
     *
     * @throws StringIndexOutOfBoundsException if begin is negative or >= length
     * @throws StringIndexOutOfBoundsException if end >= length
     * @throws AssertionError if begin > end
     *
     * @returns the amount of completed codepoints
     */
    public codePointCount(begin: number, end: number): number {
        return this.codePointCount(begin as int, end as int)
    }

    /**
     * Returns the amount of full codepoints between begin and end
     * indexes. Characters outside the range are not counted, even if
     * the range ends in the middle of a surrogate pair.
     *
     * @param begin index to start from
     * @param end past the ending index
     *
     * @throws StringIndexOutOfBoundsException if begin is negative or >= length
     * @throws StringIndexOutOfBoundsException if end >= length
     * @throws AssertionError if begin > end
     *
     * @returns the amount of completed codepoints
     */
    public codePointCount(begin: int, end: int): int {
        assert begin <= end: "Begin idx must be less then end idx";
        let counter: int = 0;
        for (let i: int = begin; i < end; i++) {
            ++counter;
            if (Char.isHighSurrogate(this.charAt(i)) &&
                (i + 1 < end) &&
                Char.isLowSurrogate(this.charAt(i + 1))) {
                ++i;
            }
        }
        return counter;
    }

    /**
     * Gets the full char sequence that is representing
     * this string.
     *
     * @returns char[] array
     */
    public getChars(): char[] {
        return this.getChars(0, this.getLength());
    }

    /**
     * Gets the char sequence that is representing the part of
     * this string between begin and end indexes. The range is
     * a half-interview [begin, end).
     *
     * @param begin index to start from
     * @param end past the ending index
     *
     * @throws StringIndexOutOfBoundsException if begin > end
     * @throws StringIndexOutOfBoundsException if begin is negative or >= length
     * @throws StringIndexOutOfBoundsException if end > length
     *
     * @returns char[] array
     *
     * @remark
     * Implemented as native function,  @see `getChars()` intrinsic [declaration](https://gitee.com/openharmony-sig/arkcompiler_runtime_core/blob/master/plugins/ets/runtime/ets_libbase_runtime.yaml#L231).
     */
    public native getChars(begin: int, end: int): char[];

    /**
     * Gets the byte sequence that is representing the part of
     * this string between begin and end indexes. The range is
     * a half-interview [begin, end).
     *
     * @param begin index to start from
     * @param end past the ending index
     *
     * @throws StringIndexOutOfBoundsException if begin is negative or >= length
     * @throws StringIndexOutOfBoundsException if end >= length
     * @throws AssertionError if begin > end
     *
     * @returns char[] array
     */
    public getBytes(begin: int, end: int): byte[] {
        assert begin <= end: "Begin idx must be less then end idx";
        let resChars: char[] = this.getChars(begin, end);
        let resBytes: byte[] = new byte[resChars.length];
        for (let i: int = 0; i < resChars.length; i++) {
            resBytes[i] = resChars[i] as byte;
        }
        return resBytes;
    }

    /**
     * Compares the given StringBuilder to this String. The
     * result is true if the StringBuilder has the same content
     * as this String at this moment and false otherwise.
     *
     * @param sb StringBuilder to compare to
     *
     * @throws NullPointerException if sb param is null
     *
     * @returns true if StringBuilder has the same String
     */
    public contentEquals(sb: StringBuilder): boolean {
        return this.equals(sb.toString());
    }

    /**
     * Lexicographical comparison between this String and another one.
     * The result is less than 0 if this string sorts before the other,
     * 0 if they are equal, and greater than 0 otherwise.
     *
     * @param other String to compare with
     *
     * @throws NullPointerException if to param is null
     *
     * @returns the comparison result
     */
    public override compareTo(other: String): int {
        let to: String = other as String;
        let len: int = min(this.getLength(), to.getLength()) as int;
        for (let i: int = 0; i < len; i++) {
            let lhsCh: Char = new Char(this.charAt(i));
            let rhsCh: Char = new Char(to.charAt(i));
            let tmpRes: int = lhsCh.compareTo(rhsCh);
            if (tmpRes != 0) {
                return tmpRes;
            }
        }
        return this.getLength() - to.getLength();
    }

    /**
     * Comparison between this String and another one based on locale.
     * The result is -1 if this string sorts before the another string,
     * 0 if they are equal, and 1 otherwise.
     *
     * @param another String to compare with
     * @param locale String representing the BCP47 language tag
     *
     * @throws RangeError if the locale tag is invalid or not found
     * @throws NullPointerException if another or locale is null
     *
     * @returns the comparison result
     */
    public native localeCompare(another: String, locale: String | null): short;

    /**
     * Comparison between this String and another one based on default
     * host locale. The result is -1 if this string sorts before the
     * another string, 0 if they are equal, and 1 otherwise.
     *
     * @param another String to compare with
     *
     * @throws RangeError if the locale tag is invalid or not found
     * @throws NullPointerException if another param is null
     *
     * @returns the comparison result
     */
    public localeCompare(another: String): short {
        return this.localeCompare(another, null);
    }

    /**
     * Checks that the substring of this string that starts from
     * the specified index starts with the specified prefix.
     * Negative `fromIndex` is treated as 0. Result is always true
     * if `prefix` is empty.
     *
     * @param prefix prefix string
     * @param fromIndex index to start from
     *
     * @throws NullPointerException if prefix param is null
     *
     * @returns true if the substring begins with prefix
     */
    public startsWith(prefix: String, fromIndex?: Number): boolean {
        if (fromIndex == undefined) {
            return this.startsWith(prefix, 0)
        }
        return this.startsWith(prefix, fromIndex!.intValue())
    }

    /**
     * Checks that the substring of this string that starts from
     * the specified index starts with the specified prefix.
     *
     * @param prefix prefix string
     * @param fromIndex index to start from
     *
     * @throws NullPointerException if prefix param is null
     * @throws StringIndexOutOfBoundsException if fromIndex param is negative or >= length
     *
     * @returns true if the substring begins with prefix
     */
    public startsWith(prefix: String, fromIndex: int): boolean {
        let prefixLen = prefix.getLength()
        if (fromIndex < 0) {
            fromIndex = 0;
        }
        if (fromIndex + prefixLen > this.getLength() && !prefix.isEmpty()) {
            return false;
        }
        for (let i: int = 0; i < prefixLen; i++) {
            if (this.charAt(fromIndex + i) != prefix.charAt(i)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Checks that this string ends with the specified suffix.
     * Result is always true if `suffix` is empty.
     *
     * @param suffix suffix string
     *
     * @param endPosition at which suffix is expected to be found. Defaults to str.length.
     *
     * @throws NullPointerException if suffix param is null
     *
     * @returns true if this string ends with suffix
     */
    public endsWith(suffix: String, endPosition?: Number): boolean {
        let ep = this.getLength()
        if (endPosition != undefined) {
            ep = endPosition!.unboxed() as int
        }
        let suffixLen = suffix.getLength()
        let fromIndex: int = ep - suffixLen;
        if (fromIndex < 0 && !suffix.isEmpty()) {
            return false;
        }
        for (let i: int = 0; i < suffixLen; i++) {
            if (this.charAt(fromIndex + i) != suffix.charAt(i)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Computes the hashcode for this String.
     *
     * @returns hashcode value of this String
     */
    public override hashCode(): int {
        let hash: int = 0;
        for (let i: int = 0; i < this.getLength(); i++) {
            hash = 31 * hash + (this.charAt(i) as int);
        }
        return hash;
    }

    /**
     * Finds the first occurrence of a character in this String.
     *
     * @param ch to find
     *
     * @returns index of the character from the beginning of this string, or -1 if not found
     */
    public native indexOf(ch: char): int;

    /**
     * Finds the first occurrence of a character in this String at position >= fromIndex.
     * Negative fromIndex is equivalent to 0, and fromIndex >= length implies no match.
     *
     * @param ch to find
     * @param fromIndex to start searching from
     *
     * @returns index of the character from the beginning of this string, or -1 if not found
     */
    public native indexOf(ch: char, fromIndex: int): int;

    /**
     * Finds the first occurrence of another String in this String
     *
     * @param str to find
     *
     * @param fromIndex to start searching from
     *
     * @returns index of the str from the beginning of this string, or -1 if not found
     */
    public indexOf(str: String, fromIndex?: Number): number {
        if (fromIndex == undefined) {
            return this.indexOf(str, 0)
        }
        return this.indexOf(str, fromIndex!.intValue())
    }

    /**
     * Finds the first occurrence of another String in this String at position >= fromIndex.
     * Negative fromIndex is equivalent to fromIndex = 0, and fromIndex >= length implies no match.
     *
     * @param str to find
     * @param fromIndex to start searching from
     *
     * @returns index of the str from the beginning of this string, or -1 if not found
     */
    public native indexOf(str: String, fromIndex: int): int;

    /**
     * Finds the last occurrence of a character in this String.
     *
     * @param ch to find
     *
     * @returns index of the character from the beginning of this string, or -1 if not found
     */
    public lastIndexOf(ch: char): int {
        return this.lastIndexOf(ch, this.getLength());
    }

    /**
     * Finds the last occurrence of a character in this String at position <= fromIndex.
     * All values of fromIndex >= length are equivalent, and negative fromIndex implies no match.
     *
     * @param ch to find
     * @param fromIndex to start searching from
     *
     * @returns index of the character from the beginning of this string, or -1 if not found
     */
    public lastIndexOf(ch: char, fromIndex: int): int {
        if (fromIndex >= this.getLength()) {
            fromIndex = this.getLength() - 1;
        }
        for (let i: int = fromIndex; i >= 0; i--) {
            if (this.charAt(i) == ch) {
                return i;
            }
        }
        return -1;
    }

    /**
     * Finds the last occurrence of another String in this String.
     *
     * @param str to find
     *
     * @param fromIndex to start searching from
    *
     * @throws NullPointerException if str param is null
     *
     * @returns index of the str from the beginning of this string, or -1 if not found
     */
    public lastIndexOf(str: String, fromIndex?: Number): number {
        if (fromIndex == undefined) {
            return this.lastIndexOf(str, Int.MAX_VALUE as int)
        }
        return this.lastIndexOf(str, fromIndex!.intValue())
    }

    /**
     * Finds the last occurrence of another String in this String at position <= fromIndex.
     * All values of fromIndex >= length are equivalent, and negative fromIndex implies no match.
     *
     * @param str to find
     * @param fromIndex to start searching from
     *
     * @returns index of the str from the beginning of this string, or -1 if not found
     */
    public native lastIndexOf(str: String, fromIndex: int): int;

    /**
     * Selects a substring of this String, starting at a specified index
     * and ending at the end of this String.
     *
     * @param begin to start substring
     *
     * @returns new String which is a substring of this String
     */
    public substring(begin: int): String {
        return this.substring(begin, this.getLength());
    }

    /**
     * Selects a substring of this String, starting at a specified index
     * and ending at index before another specified index.
     *
     * @param begin to start substring
     * @param end to end before at
     *
     * @returns new String which is a substring of this String
     */
    public substring(begin: number, end?: Number): String {
        if (end == undefined) {
            return this.substring(begin as int)
        }
        return this.substring(begin as int, end!.intValue())
    }

    /**
     * Selects a substring of this String, starting at a specified index
     * and ending at index before another specified index.
     *
     * If 'begin' < 0, then 'begin' is assumed to be equal to zero.
     * If 'begin' > this.length, then 'begin' is assumed to be equal to the this.length.
     * If 'end' < 0, then 'end' is assumed to be equal to zero.
     * If 'end' > this.length, then 'end' is assumed to be equal to this.length.
     * If 'begin' > 'end', then these are swapped.
     * If 'begin' == 'end', then an empty string is returned.
     *
     * @param begin to start substring
     * @param end to end before at
     *
     * @returns new String which is a substring of this String
     */
    public native substring(begin: int, end: int): String;

    /**
     * Concatenation of this and array of strings.
     *
     * @param strings strings to concat with
     *
     * @throws NullPointerException if strings param is null
     *
     * @returns new String which is a concatenation of this + strings[0] + ... + strings[string.length - 1]
     */
    public concat(...strings: String[]): String {
        let res = new StringBuilder(this);
        for (let i = 0; i < strings.length; i++) {
            res = res.append(strings[i])
        }
        return res.toString();
    }

    /*
     * Creates a String using substitutions
     *
     * @param d a template
     *
     * @param substs strings to be substituted
     *
     * @returns string with respected data being substituted
     */
    // TODO(ivan-tyulyandin): uncomment when #12735 will be fixed
    // public static raw(d: String, substs: ...String): String {
    //     throw new Error("String.raw: not implemented")
    // }

    /**
     * Replaces all occurrences of the specified character
     * with another specified character. If no specified
     * character in this string is found then the original
     * string will be returned
     *
     * @param oldCh character which occurrences will be replaced
     * @param newCh character to replace
     *
     * @returns new String with replaced characters
     */
    public replaceChar(oldCh: char, newCh: char): String {
        let newChars: char[] = this.getChars();
        for (let i: int = 0; i < newChars.length; i++) {
            if (newChars[i] == oldCh) {
                newChars[i] = newCh;
            }
        }
        return new String(newChars);
    }

    /**
     * Checks if this String contains the specified string.
     * The search starts from specified index (negative fromIndex is equivalent to fromIndex = 0, and fromIndex >= length implies no match).
     *
     * @param str string to search
     * @param fromIndex index to start search from
     *
     * @throws NullPointerException if str param is null
     *
     * @returns true if this String contains str and false otherwise
     */
    public contains(str: String, fromIndex: number): boolean {
        return this.contains(str, fromIndex as int);
    }

    /**
     * Checks if this String contains the specified string.
     * The search starts from specified index.
     *
     * @param str string to search
     * @param fromIndex index to start search from
     *
     * @throws NullPointerException if str param is null
     * @throws StringIndexOutOfBoundsException if fromIndex param is negative or >= length
     *
     * @returns true if this String contains str and false otherwise
     */
    public contains(str: String, fromIndex: int): boolean {
        return this.indexOf(str, fromIndex) != -1;
    }

    /**
     * Splits this String by pattern and returns ordered array of substrings.
     * The order of the resulted array corresponds to the order of the
     * passage of this String from beginning to end. The pattern is
     * excluded from substrings. The array is limited by some specified value.
     *
     * @param pattern String to split by
     * @param limit max length of the returned array. If it's negative then there is no limit.
     *
     * @throws NullPointerException if pattern param is null
     *
     * @returns string array contains substrings from this String
     */
    public split(pattern: String, limit: number): String[] {
        return this.split(pattern, limit as int)
    }

    /**
     * Splits this String by pattern and returns ordered array of substrings.
     * The order of the resulted array corresponds to the order of the
     * passage of this String from beginning to end. The pattern is
     * excluded from substrings. The array is limited by some specified value.
     *
     * @param pattern String to split by
     * @param limit max length of the returned array. If it's negative then there is no limit.
     *
     * @throws NullPointerException if pattern param is null
     *
     * @returns string array contains substrings from this String
     */
    public split(pattern: String, limit: int): String[] {
        if (limit == 0) {
            return new String[0];
        }
        let size: int = limit < 0 || limit > this.getLength() ? this.getLength() : limit;
        let offset: int = pattern.getLength() == 0 ? 1 : 0;
        let tmpArr: String[] = new String[size];
        let left: int = 0;
        let counter: int;
        for (counter = 0; counter < size; counter++) {
            let right: int = this.indexOf(pattern, left);
            if (right == -1) {
                let nextSubstr: String = this.substring(left);
                tmpArr[counter] = nextSubstr;
                counter++;
                break;
            }
            right += offset;
            let nextSubstr: String = this.substring(left, right);
            tmpArr[counter] = nextSubstr;
            left = right + pattern.getLength();
        }
        let resArr: String[] = new String[counter];
        for (let i: int = 0; i < counter; i++) {
            resArr[i] = tmpArr[i];
        }
        return resArr;
    }

    public split(separator: RegExp, limit: number): String[] {
        return this.split(separator, limit as int)
    }

    public split(separator: RegExp, limit: int): String[] {
        throw new Error("Not implemented")
    }

    /**
     * Splits this String by pattern and returns ordered array of substrings.
     * The order of the resulted array corresponds to the order of the
     * passage of this String from beginning to end. The pattern is
     * excluded from substrings.
     *
     * @param pattern String to split by
     *
     * @throws NullPointerException if pattern param is null
     *
     * @returns string array contains substrings from this String
     */
    public split(pattern: String): String[] {
        return this.split(pattern, -1);
    }

    /**
     * Splits this String by pattern and returns ordered array of substrings.
     * The order of the resulted array corresponds to the order of the
     * passage of this String from beginning to end. The pattern is
     * excluded from substrings.
     *
     * @param pattern String to split by
     *
     * @throws NullPointerException if pattern param is null
     *
     * @returns string array contains substrings from this String
     */
    public split(pattern: RegExp): String[] {
        return this.split(pattern, -1);
    }

    /**
     * Concatenates the specified string array by inserting
     * the specified separator between all elements.
     *
     * @param strings string array
     * @param delim separator between all elements
     *
     * @throws NullPointerException if strings param is null
     * @throws NullPointerException if delim param is null
     *
     * @returns newly created string from string array and delimiter
     */
    public static join(strings: String[], delim: String): String {
        return String.join(strings, delim, "", "");
    }

    /**
     * Concatenates the specified string array by inserting
     * the specified prefix before each element, the specified
     * suffix after each element, and the specified separator
     * between all elements.
     *
     * @param strings string array
     * @param delim separator between all elements
     * @param prefix prefix before each element
     * @param suffix suffix after each element
     *
     * @throws NullPointerException if strings param is null
     * @throws NullPointerException if delim param is null
     * @throws NullPointerException if prefix param is null
     * @throws NullPointerException if suffix param is null
     *
     * @returns newly created string from string array, prefix, suffix and delimiter
     */
    public static join(strings: String[], delim: String, prefix: String, suffix: String): String {
        let resStr: String = "";
        for (let i: int = 0; i < strings.length; i++) {
            resStr += prefix + strings[i] + suffix;
            if (i != strings.length - 1) {
                resStr += delim;
            }
        }
        return resStr;
    }

    /**
     * Creates new string similar to this String but with
     * all characters in lower case.
     *
     * @returns new string with all characters in lower case
     */
    public toLowerCase(): String {
        let resChars: char[] = this.getChars();
        for (let i: int = 0; i < resChars.length; i++) {
            resChars[i] = Char.toLowerCase(resChars[i]);
        }
        return new String(resChars);
    }

    /**
     * Creates new string similar to this String but with
     * all characters in upper case.
     *
     * @returns new string with all characters in upper case
     */
    public toUpperCase(): String {
        let resChars: char[] = this.getChars();
        for (let i: int = 0; i < resChars.length; i++) {
            resChars[i] = Char.toUpperCase(resChars[i]);
        }
        return new String(resChars);
    }

    /**
     * Trims all whitespaces from the beginning and end of this String.
     *
     * @returns new trimmed string
     */
    public trim(): String {
        return this.trimLeft().trimRight();
    }

    /**
     * Trims all whitespaces from the beginning of this String.
     *
     * @returns new left trimmed string
     */
    public trimLeft(): String {
        let firstNotWhiteSpaceIdx: int = this.getLength();
        for (let i: int = 0; i < this.getLength(); i++) {
            if (!Char.isWhiteSpace(this.charAt(i))) {
                firstNotWhiteSpaceIdx = i;
                break;
            }
        }
        return this.substring(firstNotWhiteSpaceIdx, this.getLength());
    }

    /**
     * Trims all whitespaces from the end of this String.
     *
     * @returns new right trimmed string
     */
    public trimRight(): String {
        let lastNotWhiteSpaceIdx: int = -1;
        for (let i: int = this.getLength() - 1; i >= 0; i--) {
            if (!Char.isWhiteSpace(this.charAt(i))) {
                lastNotWhiteSpaceIdx = i;
                break;
            }
        }
        return this.substring(0, lastNotWhiteSpaceIdx + 1);
    }

    /**
     * Checks whether the specified char is in the specified
     * char array or not.
     *
     * @param ch character to search
     * @param data char array to search in
     *
     * @throws NullPointerException if data param is null
     *
     * @returns true if ch is in the data and false otherwise
     */
    private static isCharOneOf(ch: char, data: char[]): boolean {
        for (let i: int = 0; i < data.length; i++) {
            if (ch == data[i]) {
                return true;
            }
        }
        return false;
    }

    /**
     * Trims all specified characters from the beginning and
     * end of this String.
     *
     * @param remove that contains the characters to trim
     *
     * @throws NullPointerException if remove param is null
     *
     * @returns new trimmed string
     */
    public trim(remove: char[]): String {
        return this.trimLeft(remove).trimRight(remove);
    }

    /**
     * Trims all specified characters from the beginning this String.
     *
     * @param remove that contains the characters to trim
     *
     * @throws NullPointerException if remove param is null
     *
     * @returns new left trimmed string
     */
    public trimLeft(remove: char[]): String {
        let firstNotSpecCharIdx: int = 0;
        for (let i: int = 0; i < this.getLength(); i++) {
            if (!String.isCharOneOf(this.charAt(i), remove)) {
                firstNotSpecCharIdx = i;
                break;
            }
        }
        return new String(this.getChars(firstNotSpecCharIdx, this.getLength()));
    }

    /**
     * Trims all specified characters from the end of this String.
     *
     * @param remove that contains the characters to trim
     *
     * @throws NullPointerException if remove param is null
     *
     * @returns new right trimmed string
     */
    public trimRight(remove: char[]): String {
        let lastNotSpecCharIdx: int = 0;
        for (let i: int = this.getLength() - 1; i >= 0; i--) {
            if (!String.isCharOneOf(this.charAt(i), remove)) {
                lastNotSpecCharIdx = i;
                break;
            }
        }
        return new String(this.getChars(0, lastNotSpecCharIdx + 1));
    }

    /**
     * Creates a new string of a specified length in which
     * the beginning of this String is padded with a
     * specified character. `padStart` is an alias of this method,
     * except the parameter order.
     *
     * @param pad to repeat
     * @param count of characters in the resulting string
     *
     * @returns new string with padding at the beginning
     */
    public padLeft(pad: char, count: int): String {
        return this.padStart(count, pad)
    }

    /**
     * Creates a new string of a specified length in which
     * the end of this String is padded with a specified
     * character. `padEnd` is an alias of this method,
     * except the parameter order.
     *
     * @param pad to repeat
     * @param count of characters in the resulting string
     *
     * @returns new string with padding at the end
     */
    public padRight(pad: char, count: int): String {
        return this.padEnd(count, pad)
    }

    /**
     * Repeats this string count times, i.e.
     *    a = "A",
     *    a.repeat(2) == "AA"
     *
     * @param count number of repetitions of this String
     *
     * @throws ArgumentOutOfRangeException if count < 0
     *
     * @returns this string that is repeated count times
     */
    public repeat(count: number): String throws {
        return this.repeat(count as int)
    }

    /**
     * Repeats this string count times, i.e.
     *    a = "A",
     *    a.repeat(2) == "AA"
    *
    * @param count number of repetitions of this String
    *
    * @throws ArgumentOutOfRangeException if count < 0
    *
    * @returns this string that is repeated count times
    */
    public repeat(count: int): String throws {
        if (count == 0) {
            return new String();
        }
        if (count < 0) {
            throw new ArgumentOutOfRangeException("repeat: count is negative")
        }
        let length: int = this.getLength();
        let resChars: char[] = new char[length * count];
        for (let i: int = 0; i < length; i++) {
            resChars[i] = this.charAt(i);
        }
        for (let repeat = 1; repeat < count; repeat++) {
            copyTo(resChars, resChars, repeat * length, 0, length);
        }
        return new String(resChars);
    }

    /**
     * The `toString()` method returns the string representation of the given String
     * in the form of a copy of the original object.
     *
     * @returns a copy of the original String
     */
    public override toString(): String {
        return new String(this);
    }

    /**
     * The at() method takes an integer value and returns a new String consisting of the single UTF-16 code unit located
     * at the specified offset. This method allows for positive and negative integers. Negative integers count back from the last string character.
     *
     * @returns A String consisting of the single UTF-16 code unit located at the specified position.
     * Returns undefined if the given index can not be found.
     */
    public at(index: number): String throws {
        throw new Error("not implemented")
    }

    /**
     * The at() method takes an integer value and returns a new String consisting of the single UTF-16 code unit located
     * at the specified offset. This method allows for positive and negative integers. Negative integers count back from the last string character.
     *
     * @returns A String consisting of the single UTF-16 code unit located at the specified position.
     * Returns undefined if the given index can not be found.
     */
    public at(index: int): Char throws {
        let n = this.getLength();
        if(index < 0 && -index <= n) {
            index += n
            return this.charAt(index)
        } else if (index >= 0 && index < n) {
            return this.charAt(index)
        }
        throw new ArgumentOutOfRangeException("undefined");
    }

    private CreateHTMLString(tag: String, param: String): String{
        return "<" + tag + param + ">" + new String(this) + "</" + tag + ">"
    }

    /**
     * The anchor() method creates a string that embeds a string in an <a> element with a name (<a name="...">str</a>)
     *
     * @returns A string beginning with an <a name="name"> start tag (double quotes in name are replaced with &quot;),
     * then the text str, and then an </a> end tag.
     */
    public anchor(name: String): String {
        return this.CreateHTMLString("a", " name=\"" + name + "\"")
    }

    /*
     * The big() method creates a string that embeds a string in a <big> element (<big>str</big>), which causes a string to be displayed in a big font.
     */
    public big(): String{
        return this.CreateHTMLString("big", "")
    }


    /*
     * The small() method creates a string that embeds a string in a <small> element (<small>str</small>), which causes a string to be displayed in a big font.
     */
    public small(): String{
        return this.CreateHTMLString("small", "")
    }

    /*
     * The blink() method creates a string that embeds a string in a <blink> element (<blink>str</blink>), which causes a string to be displayed in a big font.
     */
    public blink(): String{
        return this.CreateHTMLString("blink", "")
    }

    /*
     * The bold() method creates a string that embeds a string in a <bold> element (<b>str</b>), which causes a string to be displayed in a big font.
     */
    public bold(): String{
        return this.CreateHTMLString("bold", "")
    }

    /*
     * The italics() method creates a string that embeds a string in a <i> element (<i>str</i>), which causes a string to be displayed in a big font.
     */
    public italics(): String{
        return this.CreateHTMLString("i", "")
    }

    /*
     * The strike() method creates a string that embeds a string in a <strike> element (<strike>str</strike>), which causes a string to be displayed in a big font.
     */
    public strike(): String{
        return this.CreateHTMLString("strike", "")
    }

    /*
     * The sub() method creates a string that embeds a string in a <sub> element (<sub>str</sub>), which causes a string to be displayed in a big font.
     */
    public sub(): String{
        return this.CreateHTMLString("sub", "")
    }

    /*
     * The sup() method creates a string that embeds a string in a <sup> element (<sup>str</sup>), which causes a string to be displayed in a big font.
     */
    public sup(): String{
        return this.CreateHTMLString("sup", "")
    }

    /*
     * The fixed() method creates a string that embeds a string in a <tt> element (<tt>str</tt>), which causes a string to be displayed in a big font.
     */
    public fixed(): String{
        return this.CreateHTMLString("tt", "")
    }

    /*
     * The fontcolor() method creates a string that embeds a string in a <font> element (<font color="...">str</font>), which causes a string to be displayed in the specified font color.
     */
    public fontcolor(color: String): String{
        return this.CreateHTMLString("font", " color=\"" + color + "\"")
    }

    /*
     * The fontsize() method creates a string that embeds a string in a <font> element (<font size="...">str</font>), which causes a string to be displayed in the specified font size.
     */
    public fontsize(size: number): String {
        return this.CreateHTMLString("font", " size=\"" + size + "\"")
    }

    /*
     * The fontsize() method creates a string that embeds a string in a <font> element (<font size="...">str</font>), which causes a string to be displayed in the specified font size.
     */
    public fontsize(size: int): String{
        return this.CreateHTMLString("font", " size=\"" + size + "\"")
    }

    /*
     * The link() method creates a string that embeds a string in an <a> element (<a href="...">str</a>), to be used as a hypertext link to another URL.
     */
    public link(link: String): String{
        return this.CreateHTMLString("a", " href=\"" + link + "\"")
    }

    /*
     * The charCodeAt() method returns an integer between 0 and 65535 representing the UTF-16 code unit at the given index.
     */
    public charCodeAt(index: number): number {
        return this.charAt(index as int);
    }

    /*
     * The charCodeAt() method returns an integer between 0 and 65535 representing the UTF-16 code unit at the given index.
     */
    public charCodeAt(index: int): char {
        return this.charAt(index) as char;
    }

    /**
     * The String.fromCharCode() static method returns a string created from the specified sequence of UTF-16 code units
     *
     * @param codes are numbers between 0 and 65535 (0xFFFF) representing a UTF-16 code unit
     *
     * @throws RangeError if codes[i] is less than 0, or is greater than 0xFFFF
     *
     * @returns string consisting of specified UTF-16 code units.
     *
     */
    public static fromCharCode(...codes: number[]): String {
        let res = new StringBuilder();
        for (const cu of codes) {
            if (cu < 0 || cu > MAX_CODE_UNIT) {
                throw new RangeError("Invalid code unit: " + new Number(cu).toString())
            }
            res.append(cu as char)
        }
        return res.toString()
    }

    /*
     * The valueOf() method returns the primitive value of a String object.
     */
    public valueOf(): String {
        return this;
    }

    /**
     * The includes() method performs a case-sensitive search to determine whether one string may
     * be found within another string, returning true or false as appropriate.
     *
     * @param searchString to be searched
     *
     * @param position within the string at which to begin searching for searchString
     *
     * @returns true if the search string is found anywhere within the given string,
     *  including when searchString is an empty string; otherwise, false
     */
    public includes(searchString: String, position?: Number): boolean {
        if (position == undefined) {
            return this.indexOf(searchString) == -1 ? false : true;
        }
        return this.indexOf(searchString, position!.intValue()) == -1 ? false : true;
    }

    /**
     * The padEnd() method pads the current string with a given string (repeated, if needed)
     * so that the resulting string reaches a given length.
     * The padding is applied from the end of the current string.
     *
     * @param maxLength of the resulting string once the current str has been padded
     *
     * @param fillString to pad the current str with
     *
     * @returns string with the padString applied at the end of the current str
     */
    public padEnd(maxLength: number, fillString?: String): String {
        const str = (fillString == undefined) ? "\u0020" : fillString!
        return this.padEnd(maxLength as int, str)
    }

    /*
     * The padEnd() method pads the current string with a given string (repeated, if needed)
     * so that the resulting string reaches a given length.
     * The padding is applied from the end of the current string.
     */
    public padEnd(end: int, ch: char): String {
        if (this.getLength() >= end) {
            return this;
        }
        let arr = new char[end]
        for (let i: int = 0; i < this.getLength(); i++) {
            arr[i] = this.charAt(i)
        }
        for(let i = this.getLength(); i < end; i++){
            arr[i] = ch
        }
        return new String(arr)
    }

    public padEnd(end: int): String {
        return this.padEnd(end, ' ')
    }

    public padEnd(end: int, str: String): String {
        if (this.getLength() >= end) {
            return this;
        }
        let arr = new char[end];
        let length = this.getLength();
        let strLength = str.getLength()
        for (let i: int = 0; i < length; i++) {
            arr[i] = this.charAt(i);
        }
        for (let i = 0; i < strLength && length + i < end; i++) {
            arr[length + i] = str.charAt(i);
        }
        for (let i = this.getLength() + strLength; i < end; i++) {
            arr[i] = arr[i - strLength];
        }
        return new String(arr);
    }

    /**
     * The padStart() method pads the current string with another string (multiple times, if needed)
     * until the resulting string reaches the given length.
     * The padding is applied from the start of the resulting string.
     *
     * @param maxLength of the resulting string once the current str has been padded
     *
     * @param fillString to pad the current str with
     *
     * @returns string with the padString applied at the end of the current str
     */
    public padStart(maxLength: number, fillString?: String): String {
        const str = (fillString == undefined) ? "\u0020" : fillString!
        return this.padStart(maxLength as int, str)
    }

    /*
     * The padStart() method pads the current string with another string (multiple times, if needed)
     * until the resulting string reaches the given length.
     * The padding is applied from the start of the current string.
     */
    public padStart(end: int, ch: char): String {
        let padLen = end - this.getLength();
        if (padLen <= 0) {
            return this;
        }
        let arr: char[] = new char[end]
        for(let i = 0; i < padLen; i++){
            arr[i] = ch
        }
        for (let i: int = 0; i < this.getLength(); i++) {
            arr[padLen + i] = this.charAt(i);
        }
        return new String(arr)
    }

    public padStart(end: int): String {
        return this.padStart(end, ' ')
    }

    public padStart(end: int, str: String): String {
        if (this.getLength() >= end) {
            return this;
        }
        let arr: char[] = new char[end];
        let padLen: int = end - this.getLength();
        for (let i: int = 0; i < this.getLength(); i++) {
            arr[padLen + i] = this.charAt(i);
        }
        let strLength = str.getLength()
        for (let i = 0; i < strLength && i < padLen; i++) {
            arr[i] = str.charAt(i);
        }
        for (let i = strLength; i < padLen; i++) {
            arr[i] = arr[i - strLength];
        }
        return new String(arr);
    }

    /**
     * The substr() method returns a portion of the string, starting at the specified
     * index and extending for a given number of characters afterwards.
     *
     * @param begin is index of the first character to include in the returned substring
     *
     * @param length is number of characters to extract
     *
     * @returns new string containing the specified part of the given string
     */
    public substr(begin: number, length?: Number): String {
        if (length == undefined) {
            return this.substr(begin as int);
        }
        return this.substr(begin as int, length!.intValue());
    }

    public substr(begin: number, length: number): String {
        return this.substr(begin as int, length as int);
    }

    /*
     * The substr() method returns a portion of the string, starting at the specified
     * index and extending for a given number of characters afterwards.
     */
    public substr(begin: int): String {
        return this.substring(begin);
    }

    public substr(begin: int, length: int): String {
        let end = begin + length
        if (end > this.getLength()) {
            end = this.getLength()
        }
        return this.substring(begin, end);
    }

    /*
     * The trimEnd() method removes whitespace from the end of a string and returns a new string,
     * without modifying the original string. trimRight() is an alias of this method.
     */
    public trimEnd(): String {
        return this.trimRight()
    }

    /*
     * The trimStart() method removes whitespace from the beginning of a string and returns a new string,
     * without modifying the original string. trimLeft() is an alias of this method.
     */
    public trimStart(): String {
        return this.trimLeft()
    }

    /*
     * The slice() method extracts a section of a string and returns it as a new string,
     * without modifying the original string.
     */
    public slice(begin?: Number, end?: Number): String {
        const b = (begin == undefined) ? 0 : begin!
        const e = (end == undefined) ? this.getLength() : end!
        return this.slice(b as int, e as int)
    }

    /*
     * The slice() method extracts a section of a string and returns it as a new string,
     * without modifying the original string.
     */
    public slice(begin: int): String {
        if(begin < 0) {
            begin += this.getLength()
        }
        return this.substring(begin)
    }

    public slice(begin: number, end: number): String {
        return this.slice(begin as int, end as int)
    }

    public slice(begin: int, end: int): String {
        if(begin < 0) {
            begin += this.getLength()
        }
        if(end < 0) {
            end += this.getLength()
        }
        return this.substring(begin, end)
    }

    public replace(w1: String, w2: String): String {
        let n = this.indexOf(w1)
        if (n == -1) {
            return this;
        }
        return this.substring(0, n) + w2 + this.substring(n + w1.getLength())
    }

    /*
     * Returns a new string with one, some, or all matches of a pattern replaced by a replacement.
     * The pattern can be a string or a RegExp, and the replacement can be a string or a function called for each match.
     * If pattern is a string, only the first occurrence will be replaced. The original string is left unchanged.
     */
    public replace(pattern: RegExp, replacement: String) : String {
        throw new Error("Not implemented");
    }

    public replaceAll(w1: String, w2: String): String {
        let n = this.indexOf(w1)
        let str = new String(this)
        while(n != -1) {
            str = str.substring(0, n) + w2 + str.substring(n + w1.getLength())
            n = str.indexOf(w1, n + w2.getLength())
        }
        return str
    }

    /*
     * The replaceAll() method returns a new string with all matches of a pattern replaced by a replacement.
     * The pattern can be a string or a RegExp, and the replacement can be a string or a function to be called for each match.
     * The original string is left unchanged.
     */
    public replaceAll(patten: RegExp, replacement: String) : String {
        throw new Error("Not implemented");
    }

    public native search(reg: String): int;

    /**
     * Executes a search for a match between a regular expression and this String object.
     *
     * @param regexp a regular expression object
     *
     * @returns the index of the first match between the regular expression and the given string,
     * or -1 if no match was found.
     */
    public search(regexp: RegExp) : int {
        throw new Error("Not implemented");
    }

    /*
     * The toLocaleLowerCase() method returns the calling string value converted to lower case,
     * according to any locale-specific case mappings.
     */
    public native toLocaleLowerCase(locale: String): String;

    public toLocaleLowerCase(): String {
        return this.toLocaleLowerCase("");
    }

    /*
     * The toLocaleUpperCase() method returns the calling string value converted to upper case,
     * according to any locale-specific case mappings.
     */
    public native toLocaleUpperCase(locale: String): String;

    public toLocaleUpperCase(): String {
        return this.toLocaleUpperCase("");
    }

    public match(regexp: String) : String[] {
        return this.match(new RegExp(regexp));
    }

    /**
     * Retrieves the result of matching a string against a regular expression
     *
     * @param regexp a regular expression object
     *
     * @returns
     * If the regexp.global is true, all results matching the complete regular expression will be returned,
     * but capturing groups are not included
     * Otherwise, only the first complete match and its related capturing groups are returned
     */
    public match(regexp: RegExp) : String[] {
        let isGlobal = regexp.global;
        if (!isGlobal) {
            return regexp.exec(this).result;
        }
        regexp.lastIndex = 0;
        let matches = new ArrayAsListString();
        while (true) {
            let result = regexp.exec(this);
            if (!result.isCorrect) {
                return matches.toArray();
            }
            else {
                let matchStr = result.get(0);
                matches.pushBack(matchStr);
                if (matchStr == "") {
                    regexp.lastIndex = RegExp.advanceStringIndex(this, regexp.lastIndex, regexp.unicode);
                }
            }
        }
    }

    /**
     * Returns an iterator of all results matching a string against a regular expression,
     * including capturing groups
     *
     * @param regexp a regular expression object
     */
    // public matchAll(reg: String): Iterator;


    public native normalizeNFC(): String;

    public native normalizeNFD(): String;

    public native normalizeNFKC(): String;

    public native normalizeNFKD(): String;

    /**
     * The normalize() method of String values returns the Unicode Normalization Form of this string
     *
     * @param form is "NFC" or "NFD" or "NFKC" or "NFKD"
     *
     * @throws RangeError if form is not "NFC" or "NFD" or "NFKC" or "NFKD"
     *
     * @returns the Unicode Normalization Form of the string
     */
    public normalize(form?: String): String {
        const f = (form == undefined) ? "NFC" : form!
        switch (f) {
        case "NFC":
            return this.normalizeNFC()
        case "NFD":
            return this.normalizeNFD()
        case "NFKC":
            return this.normalizeNFKC()
        case "NFKD":
            return this.normalizeNFKD()
        default:
            throw new RangeError("The normalization form should be one of NFC, NFD, NFKC, NFKD.")
        }
    }

    /*
     * The toWellFormed() method of String values returns a string where all lone surrogates of
     * this string are replaced with the Unicode replacement character U+FFFD.
     */
    public toWellFormed(): String {
        return this.normalize()
    }

    private static native codePointToChar(cp: int): char

    /**
     * The String.fromCodePoint() static method returns a string created by using the specified sequence of code points
     *
     * @param codePoints are integers between 0 and 0x10FFFF (inclusive) representing a Unicode code point
     *
     * @throws RangeError if codePoints[i] is less than 0, or is greater than 0x10FFFF
     *
     * @returns string created by using the specified sequence of code points
     */
    public static fromCodePoint(...codePoints: number[]): String {
        let res = new StringBuilder();
        for (const cp of codePoints) {
            if (cp < 0 || cp > MAX_CODE_POINT) {
                throw new RangeError("Invalid code point: " + new Number(cp).toString())
            }
            res.append(String.codePointToChar(cp as int))
        }
        return res.toString()
    }

    /*
     * The isWellFormed() method of String values returns a boolean indicating whether this string contains any lone surrogates.
     */
    public native isWellFormed(): boolean;

    /**
     * Creates a String instance based on JSONValue
     *
     * @param json: JSONValue - a JSON representation
     *
     * @throws JSONTypeError if json does not encode a valid String
     *
     * @returns String - string value decoded from JSON
     */
    static createFromJSONValue(json: JSONValue): String {
        if (json instanceof JSONString) {
            return (json as JSONString).value
        }
        throw new JSONTypeError("Cannot create String from JSON", json)
    }
}
