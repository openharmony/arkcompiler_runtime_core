/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

package escompat;

/**
 * Represents JS API-compatible Array
 */
export final class Array {
    private data: Object[];

    /**
     * Creates a new empty instance of Array
     */
    public constructor() {
        this.data = new Object[0];
    }

    /**
     * Helper function copying array elements from given primitive `Object[]` array.
     */
    private initFromArray(d: Object[]): void {
        this.data = new Object[d.length];

        for (let k: int = 0; k < d.length; k++) {
            this.data[k] = d[k];
        }
    }

    /**
     * Creates a new instance of Array based on Object[]
     *
     * @param d Array initializer
     */
    public constructor(d: Object[]) {
        this.initFromArray(d);
    }

    /**
     * Takes an integer value and returns the item at that index,
     * allowing for positive and negative integers. Negative integers count back
     * from the last item in the array.
     *
     * @param index Zero-based index of the array element to be returned.
     * Negative index counts back from the end of the array — if `index` < 0, index + `array.length()` is accessed.
     *
     * @returns The element in the array matching the given index.
     * Returns null if `index` < `-length()` or `index` >= `length()`.
     */
    public at(index: int): Object | null {
        let len = this.data.length;
        let k: int;
        if (index >= 0) {
            k = index;
        } else {
            k = len + index;
        }

        if (k < 0 || k >= len) {
            return null;
        }

        return this.data[k];
    }

    /**
     * Returns the number of elements in the Array.
     *
     * @returns Element count in the `Array` instance.
     */
    public length(): int {
        return this.data.length;
    }

    /**
     * Creates a new `Object[]` from `Array` instance and given `Array` instance.
     *
     * @param other to concatenate into a new array.
     *
     * @returns Primitive array of `Object`s, constructed from `this` and given `other` instances of `Array` class.
     */
    private concatArray(other: Array): Object[] {
        assert other != null : "Can't concat with null Array";

        let res = new Object[this.data.length + other.data.length];
        let idx = 0;

        for (let i = 0; i < this.data.length; i++) {
            res[idx] = this.data[i];
            idx++;
        }
        for (let i = 0; i < other.data.length; i++) {
            res[idx] = other.data[i];
            idx++;
        }

        return res;
    }

    /**
     * Creates a new `Array` from this `Array` instance and given `Array` instance.
     *
     * @param other to concatenate into a new array.
     *
     * @returns New `Array` instance, constructed from `this` and given `other` instances of `Array` class.
     */
    public concat(other: Array): Array {
        if (other == null) {
            return this;
        }
        let res = this.concatArray(other);
        return new Array(res);
    }

    /**
     * Makes a shallow copy of the Array part to another location in the same Array and returns it without modifying its length.
     *
     * @param target index at which to copy the sequence
     *
     * @param start index at which to start copying elements from
     *
     * @param end index at which to end copying elements from
     *
     * @returns this array after transformation
     */
    public copyWithin(target: int, start: int, end: int): Array {
        if (target >= this.data.length || start >= this.data.length) {
            return this;
        }

        if (start < 0) {
            if (start < -this.data.length) {
                start = 0;
            } else {
                start = start + this.data.length;
            }
        }

        if (end < 0) {
            if (end < -this.data.length) {
                end = 0;
            } else {
                end = end + this.data.length;
            }
        }

        if (end >= this.data.length) {
            end = this.data.length;
        }

        if (target < 0) {
            if (target < -this.data.length) {
                target = 0;
            } else {
                target = target + this.data.length;
            }
        }

        if (target <= start) {
            while (start < end) {
                this.data[target++] = this.data[start++];
            }
        } else {
            let diff = target - start;
            while (target >= start) {
                if (target + diff < this.data.length && target < end) {
                    this.data[target + diff] = this.data[target];
                }
                --target;
            }
        }

        return this;
    }

    /**
     * Makes a shallow copy of the Array part to another location in the same Array and returns it without modifying its length.
     *
     * @param target index at which to copy the sequence
     *
     * @param start index at which to start copying elements from
     *
     * @returns this array after transformation
     */
    public copyWithin(target: int, start: int): Array {
        return this.copyWithin(target, start, this.data.length);
    }

    /**
     * Makes a shallow copy of the Array part to another location in the same Array and returns it without modifying its length.
     *
     * @param target index at which to copy the sequence
     *
     * @returns this array after transformation
     */
    public copyWithin(target: int): Array {
        return this.copyWithin(target, 0, this.data.length);
    }

    /**
     * Changes all elements in the Array to a static value, from a start index to an end index
     *
     * @param value to fill the array with
     *
     * @param start index at which to start filling
     *
     * @param end index at which to end filling, but not including
     *
     * @returns this array after transformation
     */
    public fill(value: Object, start: int, end: int): Array {
        if (start >= this.data.length) {
            return this;
        }

        if (start < 0) {
            if (start < -this.data.length) {
                start = 0;
            } else {
                start = start + this.data.length;
            }
        }

        if (end < 0) {
            if (end < -this.data.length) {
                end = 0;
            } else {
                end = end + this.data.length;
            }
        }

        if (end >= this.data.length) {
            end = this.data.length;
        }

        if (end < start) {
            return this;
        }

        for (let i = start; i < end; i++) {
            this.data[i] = value;
        }

        return this;
    }

    /**
     * Changes all elements in the Array to a static value
     *
     * @param value to fill the array with
     *
     * @returns this array after transformation
     */
    public fill(value: Object): Array {
        return this.fill(value, 0, this.data.length);
    }

    /**
     * Returns the first element in the provided array that satisfies the provided testing function
     *
     * @param fn testing function
     *
     * @returns found element or null otherwise
     */
    public find(fn: (elem: Object) => boolean): Object | null {
        for (let i = 0; i < this.data.length; i++) {
            if (fn(this.data[i])) {
                return this.data[i];
            }
        }
        return null;
    }

    /**
     * Returns the first element in the provided array that satisfies the provided testing function
     *
     * @param fn testing function
     *
     * @param thisArg an Array to search
     *
     * @returns found element or null otherwise
     */
    public static find(fn: (elem: Object) => boolean, thisArg: Array): Object | null {
        for (let i = 0; i < thisArg.data.length; i++) {
            if (fn(thisArg.data[i])) {
                return thisArg.data[i];
            }
        }
        return null;
    }

    /**
     * Returns the index of the first element in an array that satisfies the provided testing function
     *
     * @param fn testing function
     *
     * @returns found element index or -1 otherwise
     */
    public findIndex(fn: (elem: Object) => boolean): int {
        for (let i = 0; i < this.data.length; i++) {
            if (fn(this.data[i])) {
                return i;
            }
        }
        return -1;
    }

    /**
     * Returns the index of the first element in an array that satisfies the provided testing function
     *
     * @param fn testing function
     *
     * @param thisArg an Array to search
     *
     * @returns found element index or -1 otherwise
     */
    public static findIndex(fn: (elem: Object) => boolean, thisArg: Array): int {
        for (let i = 0; i < thisArg.data.length; i++) {
            if (fn(thisArg.data[i])) {
                return i;
            }
        }
        return -1;
    }

    /**
     * Iterates the array in reverse order and returns the value of the first element
     * that satisfies the provided testing function
     *
     * @param fn testing function
     *
     * @returns found element or null otherwise
     */
    public findLast(fn: (elem: Object) => boolean): Object | null {
        for (let i = this.data.length - 1; i >= 0; i--) {
            if (fn(this.data[i])) {
                return this.data[i];
            }
        }
        return null;
    }

    /**
     * Iterates the array in reverse order and returns the value of the first element
     * that satisfies the provided testing function
     *
     * @param fn testing function
     *
     * @param thisArg an Array to search
     *
     * @returns found element or null otherwise
     */
    public static findLast(fn: (elem: Object) => boolean, thisArg: Array): Object | null {
        for (let i = thisArg.data.length - 1; i >= 0; i--) {
            if (fn(thisArg.data[i])) {
                return thisArg.data[i];
            }
        }
        return null;
    }

    /**
     * Creates a new Array from array-like or iterable
     *
     * @param arrLike array-like or an iterable object
     *
     * @param mapFn a function to apply over all elements of arrLike
     *
     * @returns a new instance of an Array
     */
    public static fromAsync(arrLike: Object[], mapFn: (a: Object, i: int) => Object): Array {
        let mappedArr: Object[] = new Object[arrLike.length]
        for (let i = 0; i < arrLike.length; ++i) {
            mappedArr[i] = mapFn(arrLike[i], i)
        }
        return new Array(arrLike)
    }

    /**
     * Creates a new Array from array-like or iterable
     *
     * @param arrLike array-like or an iterable object
     *
     * @param mapFn a function to apply over all elements of arrLike
     *
     * @returns a new instance of an Array
     */
    public static fromAsync(arrLike: Object[], mapFn: (a: Object) => Object): Array {
        let noIndexLambda: (a: Object, i: int) => Object = (a: Object, i: int): Object => {return mapFn(a)}
        return Array.fromAsync(arrLike, noIndexLambda)
    }

    /**
     * Creates a new Array from array-like or iterable
     *
     * @param arrLike array-like or an iterable object
     *
     * @returns new instance of an Array
     */
    public static fromAsync(arrLike: Object[]): Array {
        let idLambda: (a: Object, i: int) => Object = (a: Object, i: int): Object => {return a}
        return Array.fromAsync(arrLike, idLambda)
    }

    /**
     * Creates and returns a new string by concatenating all of the elements in an `Array`,
     * separated by a specified separator string.
     * If the array has only one item, then that item will be returned without using the separator.
     *
     * @param sep specifies a separator
     *
     * @returns A string with all array elements joined. If `length()` is 0, the empty string is returned.
     */
    public join(sep: String): String {
        let sb = new StringBuilder();
        for (let i: int = 0; i < this.data.length; i++) {
            if (i != 0) {
            sb.append(sep);
            }
            sb.append(this.data[i].toString());
        }

        return sb.toString();
    }

    /**
     * Distribute values to a Map with a testing function
     *
     * @param fn a function to test
     *
     * @returns a distibution based on fn
     */
    //  public groupToMap(fn: (o: Object, i: int) => string): Map<string, Array> {
    //     let res = new Map<string, Array>()
    //     for (let i = 0; i < this.data.length; ++i) {
    //         let cur = this.data[i]
    //         let k = fn(cur, i)
    //         if (!res.has(k)) {
    //             res.set(k, new Array())
    //         }
    //         res.get(k).push(cur)
    //     }
    //     return res
    //  }

    /**
     * Distribute values to a Map with a testing function
     *
     * @param fn a function to test
     *
     * @returns a distibution based on fn
     */
    //  public groupToMap(fn: (o: Object) => string): Map<string, Array> {
    //     let l: (o: Object, i: int) => string = (o: Object, i: int): string => { return fn(o) }
    //     return this.groupToMap(l)
    //  }

    /**
     * Creates and returns a new string by concatenating all of the elements in an `Array`,
     * separated by a comma. If the array has only one item, then that item will be returned without using the separator.
     *
     * @returns A string with all array elements joined. If `length()` is 0, the empty string is returned.
     */
    public join(): String {
        return this.join(",");
    }

    /**
     * Creates a new `Object[]` primitive array and populates
     * it with the results of calling a provided function on every element in the calling array.
     *
     * @param fn mapping function, applied to each element of an array.
     *
     * @returns Primitive array of `Object`s, constructed from `this` and given function.
     */
    private mapArray(fn: (k: int, v: Object) => Object): Object[] {
        let res = new Object[this.data.length];
        for (let i = 0; i < this.data.length; i++) {
            res[i] = fn(i, this.data[i]);
        }
        return res;
    }

    /**
     * Creates a new `Object[]` primitive array and populates
     * it with the results of calling a provided function on every element in the calling array.
     *
     * @param fn mapping function, applied to each element of an array.
     *
     * @returns Primitive array of `Object`s, constructed from `this` and given function.
     */
    private mapArray(fn: (v: Object) => Object): Object[] {
        let res = new Object[this.data.length];
        for (let i = 0; i < this.data.length; i++) {
            res[i] = fn(this.data[i]);
        }
        return res;
    }

    /**
     * Creates a new `Array` object and populates
     * it with the results of calling a provided function on every element in `this` instance of `Array` class.
     *
     * @param fn mapping function, applied to each element of an array.
     *
     * @returns `Array` instance, constructed from `this` and given function.
     */
    public map(fn: (k: int, v: Object) => Object): Array {
        let res = this.mapArray(fn);
        return new Array(res);
    }

    /**
     * Creates a new `Array` object and populates
     * it with the results of calling a provided function on every element in `this` instance of `Array` class.
     *
     * @param fn mapping function, applied to each element of an array.
     *
     * @returns `Array` instance, constructed from `this` and given function.
     */
    public map(fn: (v: Object) => Object): Array {
        let res = this.mapArray(fn);
        return new Array(res);
    }

    /**
     * Tests whether all elements in the array pass the test
     * implemented by the provided function. It returns a Boolean value.
     *
     * @param fn function to execute for each element in the array.
     * It should return a `true` to indicate the element passes the test, and a `false` value otherwise.
     *
     * @returns `true` if `fn` returns a `true` value for every array element. Otherwise, `false`.
     */
    public every(fn: (k: int, v: Object) => Boolean): Boolean {
        let res: Boolean = true;
        for (let i = 0; i < this.data.length; i++) {
            res = res && fn(i, this.data[i]);
        }
        return res;
    }

    /**
     * Tests whether all elements in the array pass the test
     * implemented by the provided function. It returns a Boolean value.
     *
     * @param fn function to execute for each element in the array.
     * It should return a `true` to indicate the element passes the test, and a `false` value otherwise.
     *
     * @returns `true` if `fn` returns a `true` value for every array element. Otherwise, `false`.
     */
    public every(fn: (v: Object) => Boolean): Boolean {
        let res: Boolean = true;
        for (let i = 0; i < this.data.length; i++) {
            res = res && fn(this.data[i]);
        }
        return res;
    }

    /**
     * Tests whether at least one element in the array pass the test
     * implemented by the provided function. It returns a Boolean value.
     *
     * @param fn function to execute for each element in the array.
     * It should return a `true` to indicate the element passes the test, and a `false` value otherwise.
     *
     * @returns `true` if `fn` returns a `true` value for at least one array element. Otherwise, `false`.
     */
    public some(fn: (k: int, v: Object) => Boolean): Boolean {
        let res: Boolean = false;
        for (let i = 0; i < this.data.length; i++) {
            res = res || fn(i, this.data[i]);
        }
        return res;
    }

    /**
     * Tests whether at least one element in the array pass the test
     * implemented by the provided function. It returns a Boolean value.
     *
     * @param fn function to execute for each element in the array.
     * It should return a `true` to indicate the element passes the test, and a `false` value otherwise.
     *
     * @returns `true` if `fn` returns a `true` value for at least one array element. Otherwise, `false`.
     */
    public some(fn: (v: Object) => Boolean): Boolean {
        let res: Boolean = false;
        for (let i = 0; i < this.data.length; i++) {
            res = res || fn(this.data[i]);
        }
        return res;
    }

    /**
     * Creates a new `Object[]` primitive array and populates
     * it with same elements ordered towards the direction opposite to that previously stated.
     *
     * @returns Primitive array of `Object`s, constructed from `this` in reverse order.
     */
    private reverseArray(): Object[] {
        let res = new Object[this.data.length];
        for (let i: int = 0; i < this.data.length; i++) {
            res[i] = this.data[this.data.length-i-1];
        }
        return res;
    }

    /**
     * Modifies `this` instance of `Array` class and populates
     * it with same elements ordered towards the direction opposite to that previously stated.
     *
     * @note Mutating method
     */
    public reverse(): void {
        let res = this.reverseArray();
        this.data = res;
    }

    /**
     * Creates a new `Array` instance from `Object[]` primitive array.
     *
     * @param arr primitive 'Object' array to be converted to `Array` instance.
     *
     * @returns `Array` intance constructed from `Object[]` primitive array.
     */
    public static from(arr: Object[]): Array {
        return new Array(arr);
    }

    /**
     * Creates a new `Array` instance from `Object[]` primitive array.
     *
     * @param arr primitive 'Object' array, converted to `Array` instance.
     *
     * @param fn map function to call on every element of the array.
     * Every value to be added to the array is first passed through this function, and `fn`'s return value
     * is added to the array instead.
     *
     * @returns `Array` intance constructed from `Object[]` primitive array and given function.
     */
    public static from(arr: Object[], fn: (k: int, v: Object) => Object): Array {
        let d = new Array(arr);
        return d.map(fn);
    }

    /**
     * Creates a new `Array` instance from `Object[]` primitive array.
     *
     * @param arr primitive 'Object' array, converted to `Array` instance.
     *
     * @param fn map function to call on every element of the array.
     * Every value to be added to the array is first passed through this function, and `fn`'s return value
     * is added to the array instead.
     *
     * @returns `Array` intance constructed from `Object[]` primitive array and given function.
     */
    public static from(arr: Object[], fn: (v: Object) => Object): Array {
        let d = new Array(arr);
        return d.map(fn);
    }

    /**
     * Converts every single character of source string to `String` instance
     * and returns `String[]` primitive array for all characters of a source string.
     *
     * @param str source string to be converted to array of character's `String`
     *
     * @returns `String[]` primitive array constructed from characters of a given source string.
     */
    private static charStrings(str: String): String[] {
        let chars = str.getChars();
        let strs = new String[chars.length];
        for (let i = 0; i < chars.length; i++) {
            strs[i] = new String([chars[i]]);
        }
        return strs;
    }

    /**
     * Creates a new `Array` instance from characters of `String` and mapping function.
     *
     * @param str source string to be converted to array of character's `String`
     *
     * @param fn map function to call on every character of source string.
     *
     * @returns `Array` intance constructed from characters of source `String` and given function.
     */
    public static from(str: String, fn: (k: int, v: Object) => Object): Array {
        let strs = Array.charStrings(str);
        let d = Array.from(strs);
        return d.map(fn);
    }

    /**
     * Creates a new `Array` instance from characters of `String` and mapping function.
     *
     * @param str source string to be converted to array of character's `String`
     *
     * @param fn map function to call on every character of source string.
     *
     * @returns `Array` intance constructed from characters of source `String` and given function.
     */
    public static from(str: String, fn: (v: Object) => Object): Array {
        let strs = Array.charStrings(str);
        let d = Array.from(strs);
        return d.map(fn);
    }

    /**
     * Creates a new `Array` instance from characters of `String`.
     *
     * @param str source string to be converted to array of character's `String`
     *
     * @returns `Array` intance constructed from characters of source `String`.
     */
    public static from(str: String): Array {
        let strs = Array.charStrings(str);
        return Array.from(strs);
    }

    /**
     * Constructs new `Object[]` primitive array filtered down to
     * just the elements from the given array that pass the test implemented by the provided function.
     *
     * @param fn test function, applied to each element of an array.
     *
     * @returns Primitive array of `Object`s, constructed from `this` and given filtering function.
     */
    private filterArray(fn: (k: int, v: Object) => Boolean): Object[] {
        let resNum = 0;
        let indicator: boolean[] = new boolean[this.data.length];

        for (let i: int = 0; i < this.data.length; i++) {
            // TODO Skip of '== true' may fail compilation.
                if (fn(i, this.data[i]) == true) {
            indicator[i] = true;
            resNum++;
            }
        }

        let res = new Object[resNum];
        let j: int = 0;
        for (let i: int = 0; i < this.data.length; i++) {
            // TODO Skip of '== true' may fail compilation.
                if (indicator[i] == true) {
            res[j] = this.data[i];
            j++;
                }
        }

        return res;
    }

    /**
     * Constructs new `Object[]` primitive array filtered down to
     * just the elements from the given array that pass the test implemented by the provided function.
     *
     * @param fn test function, applied to each element of an array.
     *
     * @returns Primitive array of `Object`s, constructed from `this` and given filtering function.
     */
    public filterArray(fn: (v: Object) => Boolean): Object[] {
        // TODO Code duplication can be cleared by introducing of well-working lambdas.
        let resNum = 0;
        let indicator: boolean[] = new boolean[this.data.length];

        for (let i: int = 0; i < this.data.length; i++) {
            // TODO Skip of '== true' may fail compilation.
                if (fn(this.data[i]) == true) {
            indicator[i] = true;
            resNum++;
            }
        }

        let res = new Object[resNum];
        let j: int = 0;
        for (let i: int = 0; i < this.data.length; i++) {
            // TODO Skip of '== true' may fail compilation.
                if (indicator[i] == true) {
            res[j] = this.data[i];
            j++;
                }
        }

        return res;
    }

    /**
     * Constructs a new `Array` instance and populates it with
     * portion of a given array, filtered down to just the elements from the
     * given array that pass the test implemented by the provided function.
     *
     * @param fn test function, applied to each element of an array.
     *
     * @returns New `Array` instance constructed from `this` with elements filtered using test function `fn`.
     */
    public filter(fn: (k: int, v: Object) => Boolean): Array {
        let res = this.filterArray(fn);
        return new Array(res);
    }

    /**
     * Creates a new `Array` instance and populates it with
     * portion of a given array, filtered down to just the elements from the
     * given array that pass the test implemented by the provided function.
     *
     * @param fn test function, applied to each element of an array.
     *
     * @returns New `Array` instance constructed from `this` with elements filtered using test function `fn`.
     */
    public filter(fn: (v: Object) => Boolean): Array {
        let res = this.filterArray(fn);
        return new Array(res);
    }

    /**
     * Creates a new `Object[]` primitive array from portion of an array
     * selected from `start` to `end` (`end` not included) where `start` and `end` represent the index
     * of items in that array.
     *
     * @param start zero-based index at which to start extraction
     *
     * @param end zero-based index at which to end extraction. `sliceArray()` extracts up to but not including end.
     *
     * @returns Primitive array of `Object`s, constructed from `this` and containing extracted elements.
     */
    private sliceArray(start: int, end: int): Object[] {
        let len: int = this.data.length;
        let relativeStart = start;
        let k: int;
        let _final: int;

        if (relativeStart == (Int.MIN_VALUE as int)) {
            k = 0;
        } else if (relativeStart < 0) {
            k = max(len + (relativeStart), 0);
        } else {
            k = min(relativeStart, len);
        }

        let relativeEnd: int;
        if (end == (Int.MAX_VALUE)) {
            relativeEnd = len;
        } else {
            relativeEnd = end;
        }

        if (relativeEnd == (Int.MIN_VALUE as int)) {
            _final = 0;
        } else if (relativeEnd < 0) {
            _final = max(len + relativeEnd, 0);
        } else {
            _final = min(relativeEnd, len);
        }
        let count = max(_final - k, 0);

        let res = new Object[count];
        let n = 0;
        while (k < _final) {
            res[n] = this.data[k];
            k++;
            n++;
        }

        return res;
    }

    /**
     * Creates a new `Array` object and populates it with elements of `this` instance of `Array` class
     * selected from `start` to `end` (`end` not included) where `start` and `end` represent the index of items in that array.
     *
     * @param start zero-based index at which to start extraction
     *
     * @param end zero-based index at which to end extraction. `slice()` extracts up to but not including end.
     *
     * @returns `Array` instance, constructed from extracted elements of `this` instance.
     */
    public slice(start: int, end: int): Array {
        return new Array(this.sliceArray(start, end));
    }

    /**
     * Creates a new `Array` object and populates it with elements of `this` instance of `Array` class
     * selected from `start` to `Int.MAX_VALUE`, which means 'to the end of an array'.
     *
     * @param start zero-based index at which to start extraction
     *
     * @returns `Array` instance, constructed from extracted elements of `this` instance.
     */
    public slice(start: int): Array {
        return new Array(this.sliceArray(start, Int.MAX_VALUE as int));
    }

    /**
     * Creates a new `Array` object and populates it with elements of `this` instance of `Array` class
     *
     * @returns `Array` instance, constructed all elements of `this` instance.
     *
     * @note This method creates full copy of original `Array` instance.
     */
    public slice(): Array {
        return new Array(this.sliceArray(Int.MIN_VALUE as int, Int.MAX_VALUE as int));
    }

    // TODO Move to math.ets?
    /**
     * Named value for _less_ value of `sign()` function.
     */
    private readonly COMPARE_LE = -1;

    /**
     * Named value for _equal_ value of `sign()` function.
     */
    private readonly COMPARE_EQ =  0;

    /**
     * Named value for _greater_ value of `sign()` function.
     */
    private readonly COMPARE_GT =  1;

    /**
     * Helper function for sorting given primitive `Object[]` array using comparator.
     * Primitive array is sorted using "Quick sort" algorithm.
     *
     * @param a: Object[] - primitive array of `Object[]` to be sorted
     *
     * @param lo0: int - left index of an subarray to be sorted.
     *
     * @param hi0: int - right index of an subarray to be sorted.
     *
     * @param comparator: (a: Object, b: Object): int - comparison function. Returns one of values -1, 0, 1
     * (_less_, _equal_, _greater_ respectively).
     *
     * @note Modifies array `a`.
     */
    private doSort(a: Object[], lo0: int, hi0: int, comparator: (a: Object, b: Object) => int): void {
        let lo = lo0;
        let hi = hi0;

        if (lo >= hi) {
            return;
        } else if (lo == (hi - 1)) {
            if (comparator(a[lo], a[hi]) == this.COMPARE_GT) {
            let t = a[lo];
            a[lo] = a[hi];
            a[hi] = t;
            }
            return;
        }
        let m = (lo + hi) / 2;
        let pivot = a[m];
        a[m] = a[hi];
        a[hi] = pivot;

        while (lo < hi) {

            while ((comparator(a[lo], pivot) != this.COMPARE_GT) && (lo < hi)) {
            lo++;
            }

            while ((comparator(pivot, a[hi]) != this.COMPARE_GT) && (lo < hi)) {
            hi--;
            }

            if (lo < hi) {
            let t = a[lo];
            a[lo] = a[hi];
            a[hi] = t;

            }
        }

        a[hi0] = a[hi];
        a[hi] = pivot;

        this.doSort(a, lo0, lo - 1, comparator);
        this.doSort(a, hi + 1, hi0, comparator);
    }

    /**
     * Default comparison function for sort algorithm.
     * Objects are compared as string. Both objects are convereted to string
     * using `toString()` method and compared using `compareTo() method of `String` class.
     *
     * @param a: Object - Object to be compared
     *
     * @param b: Object - Object to be compared
     *
     * @returns  Returns one of values -1, 0, 1 (_less_, _equal_, _greater_ respectively).
     */
    private defaultComparator(a: Object, b: Object): int {
        let sa = a.toString();
        let sb = b.toString();
        return sign(sa.compareTo(sb));
    }

    /**
     * Helper function preparing copy of `this` instance of `Array` class' data array.
     *
     * @returns Copy of an `Array`'s primitive array data.
     */
    private copyArray(): Object[] {
        let len: int = this.data.length;
        let res = new Object[len];
        for (let i = 0; i < len; i++) {
            res[i] = this.data[i];
        }
        return res;
    }

    /**
     * Reorders elements of `this` using comparator function.
     *
     * @param comparator function that defines the sort order.
     *
     * @note Mutating method
     *
     * TODO clarify UTF-16 or UTF-8
     */
    public sort(comparator: (a: Object, b: Object) => int): void {
        let res = this.copyArray();
        this.doSort(res, 0, res.length-1, comparator);
        this.data = res;
    }

    /**
     * Reorders elements of `this` using a default comparator.
     * Elements sorted in ascending order built upon converting the elements into strings, then comparing
     * their sequences of UTF-16 code units values.
     *
     * @note Mutating method
     *
     * TODO clarify UTF-16 or UTF-8
     */
    public sort(): void {
        let res = this.copyArray();
        this.doSort(res, 0, res.length-1, this.defaultComparator);
        this.data = res;
    }

    /**
     * Returns the last index at which a given element can be found in the array,
     * or -1 if it is not present. The array is searched backwards, starting at fromIndex.
     *
     * @param element element to locate in the array.
     * @param fromIndex zero-based index at which to start searching backwards.
     * Negative index counts back from the end of the array — if `fromIndex` < 0, `fromIndex` + `length()` is used.
     * If `fromIndex` < `-length()`, the array is not searched and -1 is returned.
     * If `fromIndex` >= `length()` then `array.length - 1` is used, causing the entire array to be searched.
     *
     * @returns The last index of the element in the array; -1 if not found.
     */
    public lastIndexOf(element: Object, fromIndex: int): int {
        if (this.data.length == 0) {
            return -1;
        }
        let n = fromIndex;
        let k: int;
        if (n >= 0) {
            k = min(this.data.length - 1, n);
        } else {
            k = this.data.length + n;
        }

        while (k >= 0) {
            if (this.data[k].equals(element)) {
            return k;
            }
            k--;
        }
        return -1;
    }

    /**
     * Returns the last index at which a given element can be found in the array,
     * or -1 if it is not present.
     *
     * @param element to find in the array.
     *
     * @returns The last index of the element in the array; -1 if not found.
     */
    public lastIndexOf(element: Object): int {
        return this.lastIndexOf(element, this.data.length - 1);
    }

    /**
     * Inserts values into Array
     *
     * @param beforeIndex
     *
     * @param values
     *
     * @returns a shallow copy of the underlying array and inserted data
     */
    private insertValues(beforeIndex: int, values: Object[]): Object[] {
        let len = this.data.length + values.length;
        let res = new Object[len];
        let n = beforeIndex;
        let k: int;

        if (n >= 0) {
            k = min(this.data.length - 1, n);
        } else {
            k = this.data.length + n;
        }

        if (k < 0) {
            k = 0;
        }

        let idx = 0;

        for (let i = 0; i < k; i++) {
            res[idx] = this.data[i];
            idx++;
        }
        for (let i = 0; i < values.length; i++) {
            res[idx] = values[i];
            idx++;
        }

        for (let i = k; i < this.data.length; i++) {
            res[idx] = this.data[i];
            idx++
        }

        return res;
    }

    /**
     * Removes values from Array
     *
     * @param index
     *
     * @param count
     *
     * @returns a shallow copy of the underlying array without removed data
     */
    private removeMany(index: int, count: int): Object[] {
        assert count > 0;

        let n = index;
        let k: int;

        if (n >= 0) {
            k = min(this.data.length - 1, n);
        } else {
            k = this.data.length + n;
        }

        if (k < 0) {
            k = 0;
        }

        let m = min(k + count-1, this.data.length-1);

        let idx = 0;

        let res = new Object[this.data.length - (m - k + 1)];

        for (let i = 0; i < k; i++) {
            res[idx] = this.data[i];
            idx++;
        }

        for (let i = m + 1; i < this.data.length; i++) {
            res[idx] = this.data[i];
            idx++
        }

        return res;
    }

    /**
     * Checks if two `Array` instances have the same content.
     *
     * @param other `Array` instance to be compared with.
     *
     * @returns `true` if `other` is `Array` instance, both `this` and `other` have
     * same `length()` and all elements are `equals()`, `false` otherwise.
     */
    public override equals(other: Object|null): boolean {
        if (!(other instanceof Array)) {
            return false;
        }
        let a = other as Array;
        if (this.data.length != a.data.length) {
            return false;
        }

        for (let i = 0; i < this.data.length; i++) {
            // Special cases for nullability
            if ((this.data[i] == null && a.data[i] != null) || (this.data[i] != null && a.data[i] == null)) {
            return false;
            }
            if (this.data[i] == null && a.data[i] == null) {
            continue;
            }

            if (!this.data[i].equals(a.data[i])) {
            return false;
            }
        }

        return true;
    }

    /**
     * Returns a string representing the specified array and its elements.
     *
     * @returns string representation
     */
    public override toString(): String {
        let str = new StringBuilder()
        for (let i = 0; i < this.data.length; i++) {
            str.append(this.data[i])
            if(i != this.data.length - 1) {
                str.append(",")
            }
        }
        return str.toString()
    }

    /**
     * Returns a locale string representing the specified array and its elements.
     *
     * @param locales
     *
     * @param options
     *
     * @returns string representation
     */
    public toLocaleString(locales: Object, options: Object): string {
        throw new Error("Array.toLocaleString: not implemented")
    }

    /**
     * Returns a locale string representing the specified array and its elements.
     *
     * @param options
     *
     * @returns string representation
     */
    public toLocaleString(locales: Object): string {
        return this.toLocaleString(new Object(), new Object())
    }

    /**
     * Returns a locale string representing the specified array and its elements.
     *
     * @returns string representation
     */
    public toLocaleString(): string {
        return this.toLocaleString(new Object(), new Object())
    }

    /**
     * Removes the first element from an array and returns that removed element.
     * This method changes the length of the array.
     *
     * @returns shifted element, i.e. that was at index zero
     */
    public shift(): Object | null {
        if(this.data.length == 0) {
            return null
        }
        let obj = this.data[0]
        this.data = this.sliceArray(1, this.data.length)
        return obj
    }

    /**
     * Removes the last element from an array and returns that element.
     * This method changes the length of the array.
     *
     * @returns removed element
     */
    public pop(): Object | null {
        if(this.data.length == 0) {
            return null
        }
        let obj = this.data[this.data.length - 1]
        this.data = this.sliceArray(0, this.data.length - 1)
        return obj
    }

    /**
     * Adds the specified elements to the end of an array and returns the new length of the array.
     *
     * @returns new length
     */
    public push(val: Object): int {
        let arr = new Object[this.data.length + 1]
        for(let i = 0; i < this.data.length; i++) {
            arr[i] = this.data[i]
        }
        arr[this.data.length] = val
        this.data = arr
        return this.data.length
    }

    /**
     * Changes the contents of an array by removing or replacing existing elements
     * and/or adding new elements in place.
     *
     * @param start index
     *
     * @param delete number of items after start index
     *
     * @returns an Array with deleted elements
     */
    public splice(start: int, delete: int): Array {
        if(start + delete > this.data.length){
            delete = this.data.length - start
        }
        let len = this.data.length - delete
        let arr = new Object[len]
        let count: int = 0
            for(let i: int = 0; i < this.data.length; i++){
                if(i >= start && i < start + delete){
                    continue
                }
                arr[count] = this.data[i]
                count++
            }
        return new Array(arr)
    }

    /**
     * Changes the contents of an array by removing or replacing existing elements
     * and/or adding new elements in place.
     *
     * @param start index
     *
     * @returns an Array with deleted elements from start to the last element of the current instance
     */
    public splice(start: int): Array {
        return this.splice(start, this.data.length - start)
    }

    /**
     * Copying version of the splice() method.
     *
     * @param start index
     *
     * @param delete number of items after start index
     *
     * @returns a new Array with some elements removed and/or replaced at a given index.
     */
    public toSpliced(start: int, delete: int): Array {
        return this.splice(start, delete)
    }

    /**
     * Copying version of the splice() method.
     *
     * @param start index
     *
     * @returns a new Array with some elements removed and/or replaced at a given index.
     */
    public toSpliced(start: int): Array {
        return this.splice(start)
    }

    /**
     * Checks whether an Array includes a certain value among its entries,
     * returning true or false as appropriate.
     *
     * @param val value to search
     *
     * @returns true if val is in Array
     */
    public includes(val: Object): boolean {
        for(let i = 0; i < this.data.length; i++) {
            if(val == this.data[i]){
                return true
            }
        }
        return false
    }

    /**
     * Returns the first index at which a given element
     * can be found in the array, or -1 if it is not present.
     *
     * @param val value to search
     *
     * @returns index of val, -1 otherwise
     */
    public indexOf(val: Object): int {
        for(let i = 0; i < this.data.length; i++) {
            if(val == this.data[i]){
                return i
            }
        }
        return -1
    }

    /**
     * Iterates the array in reverse order and returns the index of
     * the first element that satisfies the provided testing function.
     * If no elements satisfy the testing function, -1 is returned.
     *
     * @param fn testing function
     *
     * @returns index of first element satisfying to fn, -1 if no such element
     */
    public findLastIndex(fn: (element : Object) => boolean): int {
        for(let i = this.data.length - 1; i >= 0; i--) {
            if(fn(this.data[i])){
                return i
            }
        }
        return -1
    }

    /**
     * Copying version of the sort() method.
     * It returns a new array with the elements sorted in ascending order.
     *
     * @returns sorted copy of hte current instance using default comparator
     */
    public toSorted(): Array {
        let arr = new Array(this.data)
        arr.doSort(this.data, 0, this.data.length, this.defaultComparator)
        return arr
    }

    /**
     * Copying version of the sort() method.
     * It returns a new array with the elements sorted in ascending order.
     *
     * @param comparator function to compare to elements of the Array
     *
     * @returns sorted copy of the current instance comparator
     */
    public toSorted(comparator: (a: Object, b: Object) => int): Array {
        let arr = new Array(this.data)
        arr.doSort(this.data, 0, this.data.length, comparator)
        return arr
    }

    /**
     * Returns the underlying Objects array
     *
     * @returns the underlying Objects array
     */
    private getData(): Object[] {
        return this.data
    }

    /**
     * Checks whether the passed value is an Array.
     *
     * @param arr
     *
     * @returns true is arr is a non-null array, false otherwise
     */
    public static isArray(arr: Object[]): boolean {
        if(arr == null){
            return false
        }
        return true
    }

    /**
     * Checks whether the passed value is an Array.
     *
     * @param arr
     *
     * @returns true is arr is a non-null and non-empty array, false otherwise
     */
    public static isArray(arr: Array): boolean {
        if(arr.getData() == null){
            return false
        }
        return true
    }

    /**
     * Copying version of the reverse() method.
     * It returns a new array with the elements in reversed order.
     *
     * @returns reversed copy of the current Array
     */
    public toReversed(): Array {
        let arr = new Object[this.data.length]
        let count = this.data.length - 1
        for(let i = 0; i < this.data.length; i++) {
            arr[count] = this.data[i]
            count--
        }
        return new Array(arr)
    }

    /**
     * Executes a user-supplied "reducer" callback function on each element of the array,
     * in order, passing in the return value from the calculation on the preceding element.
     * The final result of running the reducer across all elements of the array is a single value.
     * Order is from left-to-right.
     *
     * @param fn reduce function
     *
     * @param initVal start value
     *
     * @returns a result after applying fn over all elements of the Array
     */
    public reduce(fn: (a: Object, b: Object) => Object, initVal: Object): Object {
        let acc: Object = initVal
        for(let i = 0; i < this.data.length; i++) {
            acc = fn(acc, this.data[i])
        }
        return acc
    }

    /**
     * Executes a user-supplied "reducer" callback function on each element of the array,
     * in order, passing in the return value from the calculation on the preceding element.
     * The final result of running the reducer across all elements of the array is a single value.
     * Order is from left-to-right.
     *
     * @param fn reduce function
     *
     * @returns a result after applying fn over all elements of the Array
     */
    public reduce(fn: (a: Object, b: Object) => Object): Object {
        let acc: Object = this.data[0]
        for(let i = 1; i < this.data.length; i++) {
            acc = fn(acc, this.data[i])
        }
        return acc
    }

    /**
     * Executes a user-supplied "reducer" callback function on each element of the array,
     * in order, passing in the return value from the calculation on the preceding element.
     * The final result of running the reducer across all elements of the array is a single value.
     * Order is from right-to-left.
     *
     * @param fn reduce function
     *
     * @param initVal start value
     *
     * @returns a result after applying fn over all elements of the Array
     */
    public reduceRight(fn: (a: Object, b: Object) => Object, initVal: Object): Object {
        let acc: Object = initVal
        for(let i = this.data.length - 1; i >= 0; i--) {
            acc = fn(acc, this.data[i])
        }
        return acc
    }

    /**
     * Executes a user-supplied "reducer" callback function on each element of the array,
     * in order, passing in the return value from the calculation on the preceding element.
     * The final result of running the reducer across all elements of the array is a single value.
     * Order is from right-to-left.
     *
     * @param fn reduce function
     *
     * @returns a result after applying fn over all elements of the Array
     */
    public reduceRight(fn: (a: Object, b: Object) => Object): Object {
        let acc: Object = this.data[this.data.length - 1]
        for(let i = this.data.length - 2; i >= 0; i--) {
            acc = fn(acc, this.data[i])
        }
        return acc
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param fn to apply for each element of the Array
     */
    public forEach(fn: (a: Object) => void): void {
        for(let i = 0; i < this.data.length; i++) {
            fn(this.data[i])
        }
    }

    /**
     * Creates a new Array instance from a variable number of arguments,
     * regardless of number or type of the arguments.
     *
     * @param values an initilizer
     *
     * @returns a newly created Array
     */
    /** TODO(mredkin): uncomment when #12735 will be fixed
     * public of(...values: Object[]): Array {
     *     return new Array(values)
     * }
     */

    /**
     * Adds the specified elements to the beginning of an Array
     * and returns the new length of the Array.
     *
     * @param values data to be added
     *
     * @returns new length of the Array
     */
    /** TODO(mredkin): uncomment when #12735 will be fixed
     * public unshift(...values: Object[]): int {
     *     let arr = new Object[values.length + this.data.length]
     *     for(let i: int = 0; i < values.length; i++) {
     *         arr[i] = values[i]
     *     }
     *     for(let i: int = values.length; i < values.length + this.data.length; i++) {
     *         arr[i] = this.data[i - values.length]
     *     }
     *     return values.length + this.data.length
     * }
     */

    /**
     * Copying version of using the bracket notation to change the value of a given index.
     * It returns a new Array with the element at the given index replaced with the given value.
     *
     * @param index to replace
     *
     * @param value new value
     *
     * @returns a new Array with the element at the given index replaced with the given value
     */
    public with(index: int, value: Object): Array {
        let arr = new Object[this.data.length]
        for(let i: int = 0; i < this.data.length; i++) {
            arr[i] = this.data[i]
        }
        arr[index] = value
        return new Array(arr)
    }

    /**
     * Creates a new Array with all sub-array elements concatenated
     * into it recursively up to the specified depth.
     *
     * @param depth
     *
     * @returns a flattened Array with respect to depth
     */
    public flat(depth: int): Array {
        if((this.data.length > 0 && this.data[0] instanceof Object) || this.data.length == 0 || depth < 1) {
            return new Array(this.data)
        }

        let arr = new Array()
        for(let i: int = 0; i < this.data.length; i++){
            arr.concatArray(this.data[i] as Array)
        }
        return arr.flat(depth--)
    }

    /**
     * Creates a new Array with all sub-array elements concatenated
     *
     * @returns a flattened Array
     */
    public flat(): Array {
        if(this.data.length == 0) {
            return new Array(this.data)
        }
        let arr = new Array(this.data)
        while(!(arr.at(0) instanceof Object)){
            arr = arr.flat(1)
        }
        return arr
    }

    /**
     * Applies flat and than map
     *
     * fn a function to apply
     *
     * @return new Array after map and than flat
     */
     public flatMap(fn: (v: Object, k: int) => Object): Array {
        let lambda: (k: int, v: Object) => Object = (k: int, v: Object): Object => { return fn(v, k) }
        return new Array(this.mapArray(lambda)).flat()
     }

    /**
     * Applies flat and than map
     *
     * fn a function to apply
     *
     * @return new Array after map and than flat
     */
     public flatMap(fn: (v: Object) => Object): Array {
        let lambda: (v: Object, k: int) => Object = (v: Object, k: int): Object => { return fn(v) }
        return this.flatMap(lambda)
     }

    /**
     * Returns an iterator over all indices
     */
    public keys(): ValuesIterator<Int> {
        let ret: Int[] = new Int[this.data.length];
        for (let i: int = 0; i < this.data.length; i++) {
            ret[i] = i;
        }
        return new ValuesIterator<Int>(ret);
    }

    /**
     * Returns an iterator over all values
     */
    public values(): ValuesIterator<Object> {
        return new ValuesIterator<Object>(this.data);
    }

    /**
     * Returns an array iterator
     */
    // public entries(): MapIterator<K, V> {
    //     let ret: Entry<Int, Object> = new Entry<Int, Object>[this.data.length];
    //     for (let i: int = 0; i < this.data.length; i++) {
    //         ret[i] = new Entry<Int, Object>(i, this.data[i]);
    //     }
    //     return new MapIterator<Int, Object>(ret);
    // }
}
