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
export final class Array<T> {
    internal data: T[]; // TODO: make private

    /**
     * Creates a new empty instance of Array
     */
    public constructor() {
        this.data = new T[0];
    }

    private static objEquals<Y>(a: Y | null, b: Y | null): boolean {
        if (a == null) {
            return b == null
        }
        if (b == null) {
            return false
        }
        // TODO: is a.equals(null) a valid code? in some language it is
        return a.equals(b as Y)
    }

    /**
     * Helper function copying array elements from given primitive `Object[]` array.
     */
    private initFromArray(d: T[]): void {
        this.data = new T[d.length];

        for (let k: int = 0; k < d.length; k++) {
            this.data[k] = d[k];
        }
    }

    /**
     * Creates a new instance of Array based on Object[]
     *
     * @param d Array initializer
     */
    public constructor(d: T[]) {
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
    public at(index: number): T | null {
        return this.at(index as int)
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
    public at(index: int): T | null {
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

    // TODO: remove this method as it is not from common subset, but allows to omit checks
    // it should be called []
    public idx(index: int): T {
        if (index < 0 || index >= this.length()) {
            throw new RangeError("Out of bounds")
        }
        return this.data[index]
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
    private concatArray(other: Array<T>): T[] {
        assert other != null : "Can't concat with null Array";

        let res = new T[this.data.length + other.data.length];
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
    public concat(other: Array<T>): Array<T> {
        if (other == null) {
            return this;
        }
        let res = this.concatArray(other);
        return new Array<T>(res);
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
    public copyWithin(target: number, start: number, end: number): Array<T> {
        return this.copyWithin(target as int, start as int, end as int);
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
    public copyWithin(target: int, start: int, end: int): Array<T> {
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
    public copyWithin(target: number, start: number): Array<T> {
        return this.copyWithin(target as int, start as int)
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
    public copyWithin(target: int, start: int): Array<T> {
        return this.copyWithin(target, start, this.data.length);
    }

    /**
     * Makes a shallow copy of the Array part to another location in the same Array and returns it without modifying its length.
     *
     * @param target index at which to copy the sequence
     *
     * @returns this array after transformation
     */
    public copyWithin(target: number): Array<T> {
        return this.copyWithin(target as int)
    }

    /**
     * Makes a shallow copy of the Array part to another location in the same Array and returns it without modifying its length.
     *
     * @param target index at which to copy the sequence
     *
     * @returns this array after transformation
     */
    public copyWithin(target: int): Array<T> {
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
    public fill(value: T, start: number, end: number): Array<T> {
        return this.fill(value, start as int, end as int)
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
    public fill(value: T, start: int, end: int): Array<T> {
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
    public fill(value: T): Array<T> {
        return this.fill(value, 0, this.data.length);
    }

    /**
     * Returns the first element in the provided array that satisfies the provided testing function
     *
     * @param fn testing function
     *
     * @returns found element or null otherwise
     */
    public find(fn: (elem: T) => boolean): T | null {
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
    public static find<T>(fn: (elem: T) => boolean, thisArg: Array<T>): T | null {
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
    public findIndex(fn: (elem: T) => boolean): int {
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
    public static findIndex<T>(fn: (elem: T) => boolean, thisArg: Array<T>): int {
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
    public findLast(fn: (elem: T) => boolean): T | null {
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
    public static findLast<T>(fn: (elem: T) => boolean, thisArg: Array<T>): T | null {
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
    public static fromAsync<T, U>(arrLike: T[], mapFn: (a: T, i: number) => U): Array<U> {
        let mappedArr: U[] = new U[arrLike.length]
        for (let i = 0; i < arrLike.length; ++i) {
            mappedArr[i] = mapFn(arrLike[i], i)
        }
        return new Array<U>(mappedArr)
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
    public static fromAsync<T, U>(arrLike: T[], mapFn: (a: T, i: int) => U): Array<U> {
        let mappedArr: U[] = new U[arrLike.length]
        for (let i = 0; i < arrLike.length; ++i) {
            mappedArr[i] = mapFn(arrLike[i], i)
        }
        return new Array<U>(mappedArr)
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
    public static fromAsync<T, U>(arrLike: T[], mapFn: (a: T) => U): Array<U> {
        let noIndexLambda: (a: T, i: int) => U = (a: T, i: int): U => { return mapFn(a) }
        return Array.fromAsync<T, U>(arrLike, noIndexLambda)
    }

    /**
     * Creates a new Array from array-like or iterable
     *
     * @param arrLike array-like or an iterable object
     *
     * @returns new instance of an Array
     */
    public static fromAsync<T>(arrLike: T[]): Array<T> {
        let idLambda: (a: T, i: int) => T = (a: T, i: int): T => { return a }
        return Array.fromAsync<T, T>(arrLike, idLambda)
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
    private mapArray<U>(fn: (v: T, k: number) => U): U[] {
        let res = new U[this.data.length];
        for (let i = 0; i < this.data.length; i++) {
            res[i] = fn(this.data[i], i);
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
    private mapArray<U>(fn: (v: T, k: int) => U): U[] {
        let res = new U[this.data.length];
        for (let i = 0; i < this.data.length; i++) {
            res[i] = fn(this.data[i], i);
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
    private mapArray<U>(fn: (v: T) => U): U[] {
        let res = new U[this.data.length];
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
    public map<U>(fn: (v: T, k: number) => U): Array<U> {
        let res = this.mapArray<U>(fn);
        return new Array<U>(res);
    }

    /**
     * Creates a new `Array` object and populates
     * it with the results of calling a provided function on every element in `this` instance of `Array` class.
     *
     * @param fn mapping function, applied to each element of an array.
     *
     * @returns `Array` instance, constructed from `this` and given function.
     */
    public map<U>(fn: (v: T, k: int) => U): Array<U> {
        let res = this.mapArray<U>(fn);
        return new Array<U>(res);
    }

    /**
     * Creates a new `Array` object and populates
     * it with the results of calling a provided function on every element in `this` instance of `Array` class.
     *
     * @param fn mapping function, applied to each element of an array.
     *
     * @returns `Array` instance, constructed from `this` and given function.
     */
    public map<U>(fn: (v: T) => U): Array<U> {
        let res = this.mapArray<U>(fn);
        return new Array<U>(res);
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
    public every(fn: (v: T, k: number) => boolean): boolean {
        for (let i = 0; i < this.data.length; i++) {
            if (!fn(this.data[i], i)) {
                return false
            }
        }
        return true;
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
    public every(fn: (v: T, k: int) => boolean): boolean {
        for (let i = 0; i < this.data.length; i++) {
            if (!fn(this.data[i], i)) {
                return false
            }
        }
        return true;
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
    public every(fn: (v: T) => boolean): boolean {
        for (let i = 0; i < this.data.length; i++) {
            if (!fn(this.data[i])) {
                return false
            }
        }
        return true;
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
    public some(fn: (v: T, k: number) => boolean): boolean {
        for (let i = 0; i < this.data.length; i++) {
            if (fn(this.data[i], i)) { return true }
        }
        return false
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
    public some(fn: (v: T, k: int) => boolean): boolean {
        for (let i = 0; i < this.data.length; i++) {
            if (fn(this.data[i], i)) { return true }
        }
        return false
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
    public some(fn: (v: T) => boolean): boolean {
        for (let i = 0; i < this.data.length; i++) {
            if (fn(this.data[i])) { return true }
        }
        return false
    }

    /**
     * Creates a new `Object[]` primitive array and populates
     * it with same elements ordered towards the direction opposite to that previously stated.
     *
     * @returns Primitive array of `Object`s, constructed from `this` in reverse order.
     */
    private reverseArray(): T[] {
        let res = new T[this.data.length];
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
    public static from<T>(arr: T[]): Array<T> {
        return new Array<T>(arr);
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
    public static from<T, U>(arr: T[], fn: (v: T, k: number) => U): Array<U> {
        let d = new Array<T>(arr);
        return d.map<U>(fn);
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
    public static from<T, U>(arr: T[], fn: (v: T, k: int) => U): Array<U> {
        let d = new Array<T>(arr);
        return d.map<U>(fn);
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
    public static from<T, U>(arr: T[], fn: (v: T) => U): Array<U> {
        let d = new Array<T>(arr);
        return d.map<U>(fn);
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
    public static from<U>(str: String, fn: (v: String, k: number) => U): Array<U> {
        let strs = Array.charStrings(str);
        let d: Array<String> = Array.from<String>(strs);
        return d.map<U>(fn);
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
    public static from<U>(str: String, fn: (v: String, k: int) => U): Array<U> {
        let strs = Array.charStrings(str);
        let d: Array<String> = Array.from<String>(strs);
        return d.map<U>(fn);
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
    public static from<U>(str: String, fn: (v: String) => U): Array<U> {
        let strs = Array.charStrings(str);
        let d = Array.from<String>(strs);
        return d.map<U>(fn);
    }

    /**
     * Creates a new `Array` instance from characters of `String`.
     *
     * @param str source string to be converted to array of character's `String`
     *
     * @returns `Array` intance constructed from characters of source `String`.
     */
    public static from(str: String): Array<String> {
        let strs = Array.charStrings(str);
        return Array.from<String>(strs);
    }

    /**
     * Constructs new `Object[]` primitive array filtered down to
     * just the elements from the given array that pass the test implemented by the provided function.
     *
     * @param fn test function, applied to each element of an array.
     *
     * @returns Primitive array of `Object`s, constructed from `this` and given filtering function.
     */
    private filterArray(fn: (v: T, k: int) => boolean): T[] {
        let resNum = 0;
        let indicator: boolean[] = new boolean[this.data.length];

        for (let i: int = 0; i < this.data.length; i++) {
            if (fn(this.data[i], i)) {
                indicator[i] = true;
                resNum++;
            }
        }

        let res = new T[resNum];
        let j: int = 0;
        for (let i: int = 0; i < this.data.length; i++) {
            if (indicator[i]) {
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
    public filterArray(fn: (v: T) => boolean): T[] {
        return this.filterArray((v: T, k: int): boolean => { return fn(v); });
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
    public filter(fn: (v: T, k: number) => boolean): Array<T> {
        let fn1: (v: T, k: int) => boolean = (v: T, k: int): boolean => {
            return fn(v, k)
        }
        return this.filter(fn1);
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
    public filter(fn: (v: T, k: int) => boolean): Array<T> {
        let res = this.filterArray(fn);
        return new Array<T>(res);
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
    public filter(fn: (v: T) => boolean): Array<T> {
        let res = this.filterArray(fn);
        return new Array<T>(res);
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
    private sliceArray(start: int, end: int): T[] {
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

        let res = new T[count];
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
    public slice(start: number, end: number): Array<T> {
        return this.slice(start as int, end as int)
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
    public slice(start: int, end: int): Array<T> {
        return new Array<T>(this.sliceArray(start, end));
    }

    /**
     * Creates a new `Array` object and populates it with elements of `this` instance of `Array` class
     * selected from `start` to `Int.MAX_VALUE`, which means 'to the end of an array'.
     *
     * @param start zero-based index at which to start extraction
     *
     * @returns `Array` instance, constructed from extracted elements of `this` instance.
     */
    public slice(start: number): Array<T> {
        return this.slice(start as number)
    }

    /**
     * Creates a new `Array` object and populates it with elements of `this` instance of `Array` class
     * selected from `start` to `Int.MAX_VALUE`, which means 'to the end of an array'.
     *
     * @param start zero-based index at which to start extraction
     *
     * @returns `Array` instance, constructed from extracted elements of `this` instance.
     */
    public slice(start: int): Array<T> {
        return new Array<T>(this.sliceArray(start, Int.MAX_VALUE as int));
    }

    /**
     * Creates a new `Array` object and populates it with elements of `this` instance of `Array` class
     *
     * @returns `Array` instance, constructed all elements of `this` instance.
     *
     * @note This method creates full copy of original `Array` instance.
     */
    public slice(): Array<T> {
        return new Array<T>(this.sliceArray(Int.MIN_VALUE as int, Int.MAX_VALUE as int));
    }

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
    private doSort(a: T[], lo0: int, hi0: int, comparator: (a: T, b: T) => int): void {
        let lo = lo0;
        let hi = hi0;

        if (lo >= hi) {
            return;
        } else if (lo == (hi - 1)) {
            if (comparator(a[lo], a[hi]) > 0) {
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
            while ((comparator(a[lo], pivot) <= 0) && (lo < hi)) {
                lo++;
            }

            while ((comparator(pivot, a[hi]) <= 0) && (lo < hi)) {
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
    private static defaultComparator<T>(a: T, b: T): int {
        let sa = a.toString();
        let sb = b.toString();
        return sa.compareTo(sb);
    }

    /**
     * Helper function preparing copy of `this` instance of `Array` class' data array.
     *
     * @returns Copy of an `Array`'s primitive array data.
     */
    private copyArray(): T[] {
        let len: int = this.data.length;
        let res = new T[len];
        for (let i = 0; i < len; i++) {
            res[i] = this.data[i];
        }
        return res;
    }

    // TODO: can't distinguish `=> number` and `=> int`
    /**
     * Reorders elements of `this` using comparator function.
     *
     * @param comparator function that defines the sort order.
     *
     * @note Mutating method
     *
     * TODO clarify UTF-16 or UTF-8
     */
    public sort(comparator: (a: T, b: T) => int): void {
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
        this.doSort(res, 0, res.length-1, Array.defaultComparator);
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
    public lastIndexOf(element: T, fromIndex: number): number {
        return this.lastIndexOf(element, fromIndex)
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
    public lastIndexOf(element: T, fromIndex: int): int {
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
    public lastIndexOf(element: T): int {
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
    private insertValues(beforeIndex: int, values: T[]): T[] {
        let len = this.data.length + values.length;
        let res = new T[len];
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
    private removeMany(index: int, count: int): T[] {
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

        let res = new T[this.data.length - (m - k + 1)];

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
        let a = other as Array<Object>;
        if (this.data.length != a.data.length) {
            return false;
        }

        for (let i = 0; i < this.data.length; i++) {
            if (!Array.objEquals<Object>(this.data[i], a.data[i])) {
                return false
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
            const d: Object | null = this.data[i]
            if (d == null) {
                str.append("null")
            } else {
                str.append((d as Object).toString())
            }
            if(i != this.data.length - 1) {
                str.append(c',')
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
    public shift(): T | null {
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
    public pop(): T | null {
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
    public push(val: T): int {
        let arr = new T[this.data.length + 1]
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
    public splice(start: number, delete: number): Array<T> {
        return this.splice(start as int, delete as int)
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
    public splice(start: int, delete: int): Array<T> {
        if(start + delete > this.data.length){
            delete = this.data.length - start
        }
        let len = this.data.length - delete
        let arr = new T[len]
        let ret = new Array<T>()
        let count: int = 0
        for( let i: int = 0; i < this.data.length; i++){
            if(i >= start && i < start + delete){
                ret.push(this.data[i])
                continue
            }
            arr[count] = this.data[i]
            count++
        }
        this.data = arr
        return ret
    }

    /**
     * Changes the contents of an array by removing or replacing existing elements
     * and/or adding new elements in place.
     *
     * @param start index
     *
     * @returns an Array with deleted elements from start to the last element of the current instance
     */
    public splice(start: number): Array<T> {
        return this.splice(start as int)
    }

    /**
     * Changes the contents of an array by removing or replacing existing elements
     * and/or adding new elements in place.
     *
     * @param start index
     *
     * @returns an Array with deleted elements from start to the last element of the current instance
     */
    public splice(start: int): Array<T> {
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
    public toSpliced(start: number, delete: number): Array<T> {
        return this.toSpliced(start as int, delete as int)
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
    public toSpliced(start: int, delete: int): Array<T> {
        return this.splice(start, delete)
    }

    /**
     * Copying version of the splice() method.
     *
     * @param start index
     *
     * @returns a new Array with some elements removed and/or replaced at a given index.
     */
    public toSpliced(start: number): Array<T> {
        return this.toSpliced(start as int)
    }

    /**
     * Copying version of the splice() method.
     *
     * @param start index
     *
     * @returns a new Array with some elements removed and/or replaced at a given index.
     */
    public toSpliced(start: int): Array<T> {
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
    public includes(val: T): boolean {
        return this.indexOf(val) != -1
    }

    /**
     * Returns the first index at which a given element
     * can be found in the array, or -1 if it is not present.
     *
     * @param val value to search
     *
     * @returns index of val, -1 otherwise
     */
    public indexOf(val: T): int {
        for(let i = 0; i < this.data.length; i++) {
            if (Array.objEquals<T>(val, this.data[i])) {
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
    public findLastIndex(fn: (element : T) => boolean): int {
        for(let i = this.data.length - 1; i >= 0; i--) {
            if (fn(this.data[i])) {
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
    public toSorted(): Array<T> {
        let arr = new Array<T>(this.data)
        arr.doSort(this.data, 0, this.data.length, Array.defaultComparator)
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
    public toSorted(comparator: (a: T, b: T) => int): Array<T> {
        let arr = new Array<T>(this.data)
        arr.doSort(this.data, 0, this.data.length, comparator)
        return arr
    }

    /**
     * Returns the underlying Objects array
     *
     * @returns the underlying Objects array
     */
    private getData(): T[] {
        return this.data
    }

    /**
     * Checks whether the passed value is an Array.
     *
     * @param arr
     *
     * @returns true is arr is a non-null array, false otherwise
     */
    public static isArray<T>(arr: T[]): boolean {
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
    public static isArray<T>(arr: Array<T>): boolean {
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
    public toReversed(): Array<T> {
        let arr = new T[this.data.length]
        let count = this.data.length - 1
        for(let i = 0; i < this.data.length; i++) {
            arr[count] = this.data[i]
            count--
        }
        return new Array<T>(arr)
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
    public reduce<U>(fn: (a: U, b: T) => U, initVal: U): U {
        let acc: U = initVal
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
    public reduce(fn: (a: T, b: T) => T): T {
        if (this.data.length == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.data[0]
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
    public reduceRight<U>(fn: (a: U, b: T) => U, initVal: U): U {
        let acc: U = initVal
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
    public reduceRight(fn: (a: T, b: T) => T): T {
        if (this.data.length == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.data[this.data.length - 1]
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
    public forEach(fn: (a: T) => void): void {
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
    //public static of<T>(...values: T[]): Array<T> {
    //    return new Array<T>(values)
    //}

    /**
     * Adds the specified elements to the beginning of an Array
     * and returns the new length of the Array.
     *
     * @param values data to be added
     *
     * @returns new length of the Array
     */
    /* // TODO(mredkin): uncomment when #12735 will be fixed
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
    public with(index: number, value: T): Array<T> {
        return this.with(index as int, value)
    }

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
    public with(index: int, value: T): Array<T> {
        let arr = new T[this.data.length]
        for(let i: int = 0; i < this.data.length; i++) {
            arr[i] = this.data[i]
        }
        arr[index] = value
        return new Array<T>(arr)
    }

    /**
     * Creates a new Array with all sub-array elements concatenated
     * into it recursively up to the specified depth.
     *
     * @param depth
     *
     * @returns a flattened Array with respect to depth
     */
    public flat(depth: number): Array<Object | null> {
        return this.flat(depth as int)
    }

    /**
     * Creates a new Array with all sub-array elements concatenated
     * into it recursively up to the specified depth.
     *
     * @param depth
     *
     * @returns a flattened Array with respect to depth
     */
    public flat(depth: int): Array<Object | null> {
        let ret = new Array<Object | null>()
        this.flatImpl(depth, ret)
        return ret
    }

    private flatImpl(depth: int, to: Array<Object | null>) {
        throw new Error("not implemented");
        //if (depth <= 0) {
        //    to.push(this as Object)
        //    return
        //}
        //for (let i = 0; i < this.data.length; i++) {
        //    let e = this.data[i];
        //    if (e instanceof Array) {
        //        let eArr: Array<Object> = e as Object as Array<Object>;
        //        eArr.flatImpl(depth - 1, to)
        //    } else {
        //        to.push(e)
        //    }
        //}
    }

    /**
     * Creates a new Array with all sub-array elements concatenated
     *
     * @returns a flattened Array
     */
    public flat(): Array<Object | null> {
        return this.flat(1)
    }

    /**
     * Applies flat and than map
     *
     * fn a function to apply
     *
     * @return new Array after map and than flat
     */
     public flatMap(fn: (v: T, k: number) => Object): Array<Object | null> {
        let mapped: Array<Object> = this.map<Object>(fn)
        return mapped.flat()
     }

    /**
     * Applies flat and than map
     *
     * fn a function to apply
     *
     * @return new Array after map and than flat
     */
     public flatMap(fn: (v: T, k: int) => Object): Array<Object | null> {
        let mapped: Array<Object> = this.map<Object>(fn)
        return mapped.flat()
     }

    /**
     * Applies flat and than map
     *
     * fn a function to apply
     *
     * @return new Array after map and than flat
     */
     public flatMap(fn: (v: T) => Object): Array<Object | null> {
        let mapped: Array<Object> = this.map<Object>(fn)
        return mapped.flat()
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
    public values(): ValuesIterator<T> {
        return new ValuesIterator<T>(this.data);
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
