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

class ArrayKeysIterator implements IterableIterator<number> {
    // NOTE(kprokopenko): must contain array to update keys on update, but blocked by frontend
    private parent: int //Array<T>
    private idx: int = 0

    constructor(parent: int) {
        this.parent = parent
    }

    override next(): IteratorResult<number> {
        if (this.idx >= this.parent) {
            return new IteratorResult<number>()
        }
        return new IteratorResult<number>(false, this.idx++ as number)
    }

    override iterator(): IterableIterator<number> {
        return this
    }
}

class ArrayValuesIterator<T> implements IterableIterator<T> {
    private parent: Array<T>
    private idx: int = 0

    constructor(parent: Array<T>) {
        this.parent = parent
    }

    override next(): IteratorResult<T> {
        if (this.idx >= this.parent.length) {
            return new IteratorResult<T>()
        }
        return new IteratorResult<T>(false, this.parent.at(this.idx++)!)
    }

    override iterator(): IterableIterator<T> {
        return this
    }
}

/**
 * Represents JS API-compatible Array
 */
export final class Array<T> implements ReadonlyArray<T> {
    private buffer: NullishType[]
    private actualLength: int

    /* override */ get length(): number {
        return this.actualLength as number
    }

    /**
     * Creates a new instance of Array
     */
    public constructor() {
        this.buffer = new NullishType[4]
        this.actualLength = 0
    }

    $_get(i: number): T {
        const idx = i as int
        if (idx < 0 || idx >= this.actualLength) {
            throw new RangeError("Out of bounds")
        }
        return this.buffer[idx] as T
    }

    $_set(i: number, val: T): void {
        let idx = i as int
        if (idx < 0 || idx >= this.actualLength) {
            throw new RangeError("Out of bounds")
        }
        this.buffer[idx] = val
    }

    /**
     * Creates a new instance of Array
     */
    public constructor(arrayLen: number) {
        this.buffer = new NullishType[arrayLen as int];
        this.actualLength = arrayLen as int // NOTE(kprokopenko): this is incorrect! null-safety
    }

    private static objEquals<Y>(a: Y | null, b: Y | null): boolean {
        if (a == null) {
            return b == null
        }
        if (b == null) {
            return false
        }
        // NOTE: is a.equals(null) a valid code? in some language it is
        return a.equals(b as Y)
    }

    /**
     * Creates a new instance of Array based on Object[]
     *
     * @param d Array initializer
     */
    public constructor(d: T[]) {
        this.buffer = new NullishType[d.length];
        this.actualLength = d.length

        for (let k: int = 0; k < d.length; k++) {
            this.$_set(k, d[k]);
        }
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
    public override at(index: number): T | null {
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
        let len = this.actualLength;
        let k: int;
        if (index >= 0) {
            k = index;
        } else {
            k = len + index;
        }

        if (k < 0 || k >= len) {
            return null;
        }

        return this.$_get(k);
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

        let res = new T[this.actualLength + other.actualLength];
        let idx = 0;

        for (let i = 0; i < this.actualLength; i++) {
            res[idx] = this.$_get(i);
            idx++;
        }
        for (let i = 0; i < other.actualLength; i++) {
            res[idx] = other.$_get(i);
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
    // public override concat(...items: (T | ConcatArray<T>)[]): Array<T> {
    //     throw new Error("not implemented")
        // See for-body

        // let res = new Array<T>(this.data)
        // let len = items.length as int
        // for (let i = 0; i < len; ++i) {
        //     if (items[i] instanceof ConcatArray<T>) { // SyntaxError: Unexpected token ')'.
        //         res.data = res.concatArray((items[i] as ConcatArray<T>).data)
        //     } else if (items[i] instanceof T) {
        //         res.push(items[i] as T)
        //     }
        // }
        // return res
    // }

    // Typing and previous overload have to cover it
    // public override concat(...items: ConcatArray<T>[]): T[] {
    //     throw new Error("not implemented")
    // }

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
        if (target >= this.actualLength || start >= this.actualLength) {
            return this;
        }

        if (start < 0) {
            if (start < -this.actualLength) {
                start = 0;
            } else {
                start = start + this.actualLength;
            }
        }

        if (end < 0) {
            if (end < -this.actualLength) {
                end = 0;
            } else {
                end = end + this.actualLength;
            }
        }

        if (end >= this.actualLength) {
            end = this.actualLength;
        }

        if (target < 0) {
            if (target < -this.actualLength) {
                target = 0;
            } else {
                target = target + this.actualLength;
            }
        }

        if (target <= start) {
            while (start < end) {
                this.$_set(target++, this.$_get(start++));
            }
        } else {
            let diff = target - start;
            while (target >= start) {
                if (target + diff < this.actualLength && target < end) {
                    this.$_set(target + diff, this.$_get(target));
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
        return this.copyWithin(target, start, this.actualLength);
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
        return this.copyWithin(target, 0, this.actualLength);
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
        if (start >= this.actualLength) {
            return this;
        }

        if (start < 0) {
            if (start < -this.actualLength) {
                start = 0;
            } else {
                start = start + this.actualLength;
            }
        }

        if (end < 0) {
            if (end < -this.actualLength) {
                end = 0;
            } else {
                end = end + this.actualLength;
            }
        }

        if (end >= this.actualLength) {
            end = this.actualLength;
        }

        if (end < start) {
            return this;
        }

        for (let i = start; i < end; i++) {
            this.$_set(i, value);
        }

        return this;
    }

    /**
     * Changes all elements in the Array to a static value, from a start index to an end index
     *
     * @param value to fill the array with
     *
     * @param start index at which to start filling
     *
     * @returns this array after transformation
     */
    public fill(value: T, start: number): Array<T> {
        return this.fill(value, start as int, 0)
    }

    /**
     * Changes all elements in the Array to a static value
     *
     * @param value to fill the array with
     *
     * @returns this array after transformation
     */
    public fill(value: T): Array<T> {
        return this.fill(value, 0, this.actualLength);
    }

    // NOTE(ivan-tyulyandin): blocked by undefined implementation
    public override find(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): T | null {
        const res = this.findIndex(predicate)
        if (res == -1) {
            return null
        }
        return this.$_get(res as int)
    }
    public override find(predicate: (value: T, index: number) => boolean): T | null {
        const res = this.findIndex(predicate)
        if (res == -1) {
            return null
        }
        return this.$_get(res as int)
    }
    // find<S extends T>(predicate: (value: T, index: number, obj: readonly T[]) => value is S, thisArg?: any): S | undefined;
    // Typing and previous overload have to cover it

    /**
     * Returns the first element in the provided array that satisfies the provided testing function
     *
     * @param fn testing function
     *
     * @returns found element or null otherwise
     */
    public find(fn: (elem: T) => boolean): T | null {
        for (let i = 0; i < this.actualLength; i++) {
            if (fn(this.$_get(i))) {
                return this.$_get(i);
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
    public override findIndex(fn: (value: T, index: number) => boolean): number {
        for (let i = 0; i < this.actualLength; i++) {
            if (fn(this.$_get(i), i)) {
                return i;
            }
        }
        return -1;
    }

    public override findIndex(predicate: (value: T, index: number, obj: ReadonlyArray<T>) => boolean): number {
        for (let i = 0; i < this.actualLength; i++) {
            if (predicate(this.$_get(i), i, this)) {
                return i;
            }
        }
        return -1;
    }

    public findIndex(predicate: (value: T, index: number, obj: Array<T>) => boolean): number {
        for (let i = 0; i < this.actualLength; i++) {
            if (predicate(this.$_get(i), i, this)) {
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
     * @returns found element index or -1 otherwise
     */
    public override findIndex(fn: (value: T) => boolean): number {
        for (let i = 0; i < this.actualLength; i++) {
            if (fn(this.$_get(i))) {
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
        for (let i = this.actualLength - 1; i >= 0; i--) {
            if (fn(this.$_get(i))) {
                return this.$_get(i);
            }
        }
        return null;
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
    public override join(sep: string): string {
        let sb = new StringBuilder();
        for (let i: int = 0; i < this.actualLength; i++) {
            if (i != 0) {
            sb.append(sep);
            }
            sb.append(this.$_get(i).toString());
        }

        return sb.toString();
    }

    /**
     * Creates and returns a new string by concatenating all of the elements in an `Array`,
     * separated by a comma. If the array has only one item, then that item will be returned without using the separator.
     *
     * @returns A string with all array elements joined. If `length()` is 0, the empty string is returned.
     */
    public override join(): string {
        return this.join(",");
    }

    /**
     * Creates a new `Array` object and populates
     * it with the results of calling a provided function on every element in `this` instance of `Array` class.
     *
     * @param fn mapping function, applied to each element of an array.
     *
     * @returns `Array` instance, constructed from `this` and given function.
     */
    public override map<U>(fn: (value: T, index: number, arr: ReadonlyArray<T>) => U): Array<U> {
        let res = new U[this.actualLength];
        for (let i = 0; i < this.actualLength; i++) {
            res[i] = fn(this.$_get(i), i, this);
        }
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
    public map<U>(fn: (value: T, index: number, arr: Array<T>) => U): Array<U> {
        let res = new U[this.actualLength];
        for (let i = 0; i < this.actualLength; i++) {
            res[i] = fn(this.$_get(i), i, this);
        }
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
    public override map<U>(fn: (v: T, k: number) => U): Array<U> {
        let res = new U[this.actualLength];
        for (let i = 0; i < this.actualLength; i++) {
            res[i] = fn(this.$_get(i), i);
        }
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
    public override map<U>(fn: (v: T) => U): Array<U> {
        let res = new U[this.actualLength];
        for (let i = 0; i < this.actualLength; i++) {
            res[i] = fn(this.$_get(i));
        }
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
    public override every(fn: (v: T, k: number, arr: ReadonlyArray<T>) => boolean): boolean {
        for (let i = 0; i < this.actualLength; i++) {
            if (!fn(this.$_get(i), i, this)) {
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
    public every(fn: (v: T, k: number, arr: Array<T>) => boolean): boolean {
        for (let i = 0; i < this.actualLength; i++) {
            if (!fn(this.$_get(i), i, this)) {
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
    public override every(fn: (v: T, k: number) => boolean): boolean {
        for (let i = 0; i < this.actualLength; i++) {
            if (!fn(this.$_get(i), i)) {
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
        for (let i = 0; i < this.actualLength; i++) {
            if (!fn(this.$_get(i), i)) {
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
    public override every(fn: (v: T) => boolean): boolean {
        for (let i = 0; i < this.actualLength; i++) {
            if (!fn(this.$_get(i))) {
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
    public override some(fn: (v: T, k: number) => boolean): boolean {
        for (let i = 0; i < this.actualLength; i++) {
            if (fn(this.$_get(i), i)) { return true }
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
        for (let i = 0; i < this.actualLength; i++) {
            if (fn(this.$_get(i), i)) { return true }
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
    public override some(fn: (v: T) => boolean): boolean {
        for (let i = 0; i < this.actualLength; i++) {
            if (fn(this.$_get(i))) { return true }
        }
        return false
    }

    /**
     * Modifies `this` instance of `Array` class and populates
     * it with same elements ordered towards the direction opposite to that previously stated.
     *
     * @note Mutating method
     */
    public reverse(): Array<T> {
        for (let i = 0; i < this.actualLength / 2; i++) {
            const tmp = this.$_get(i)
            this.$_set(i, this.$_get(this.actualLength - 1 - i))
            this.$_set(this.actualLength - 1 - i, tmp)
        }
        return this;
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
    public static from<T, U>(arr: T[], fn: (v: T, k: number, arr: Array<T>) => U): Array<U> {
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
    public static from<T, U>(arr: T[], fn: (v: T) => U): Array<U> {
        let d = new Array<T>(arr);
        return d.map<U>(fn);
    }

    /**
     * Converts every single character of source string to `string` instance
     * and returns `string[]` primitive array for all characters of a source string.
     *
     * @param str source string to be converted to array of character's `string`
     *
     * @returns `string[]` primitive array constructed from characters of a given source string.
     */
    private static charStrings(str: string): string[] {
        let chars = str.getChars();
        let strs = new string[chars.length];
        for (let i = 0; i < chars.length; i++) {
            strs[i] = new string([chars[i]]);
        }
        return strs;
    }

    /**
     * Creates a new `Array` instance from characters of `string` and mapping function.
     *
     * @param str source string to be converted to array of character's `string`
     *
     * @param fn map function to call on every character of source string.
     *
     * @returns `Array` intance constructed from characters of source `string` and given function.
     */
    public static from<U>(str: string, fn: (v: string, k: number, arr: Array<string>) => U): Array<U> {
        let strs = Array.charStrings(str);
        let d: Array<string> = Array.from<string>(strs);
        return d.map<U>(fn);
    }

    /**
     * Creates a new `Array` instance from characters of `string` and mapping function.
     *
     * @param str source string to be converted to array of character's `string`
     *
     * @param fn map function to call on every character of source string.
     *
     * @returns `Array` intance constructed from characters of source `string` and given function.
     */
    public static from<U>(str: string, fn: (v: string, k: number) => U): Array<U> {
        let strs = Array.charStrings(str);
        let d: Array<string> = Array.from<string>(strs);
        return d.map<U>(fn);
    }

    /**
     * Creates a new `Array` instance from characters of `string` and mapping function.
     *
     * @param str source string to be converted to array of character's `string`
     *
     * @param fn map function to call on every character of source string.
     *
     * @returns `Array` intance constructed from characters of source `string` and given function.
     */
    public static from<U>(str: string, fn: (v: string) => U): Array<U> {
        let strs = Array.charStrings(str);
        let d = Array.from<string>(strs);
        return d.map<U>(fn);
    }

    /**
     * Creates a new `Array` instance from characters of `string`.
     *
     * @param str source string to be converted to array of character's `string`
     *
     * @returns `Array` intance constructed from characters of source `string`.
     */
    public static from(str: string): string[] {
        return Array.charStrings(str);
    }

    /**
     * Constructs new `Object[]` primitive array filtered down to
     * just the elements from the given array that pass the test implemented by the provided function.
     *
     * @param fn test function, applied to each element of an array.
     *
     * @returns Primitive array of `Object`s, constructed from `this` and given filtering function.
     */
    private filterArray(fn: (v: T, k: int) => boolean): Array<T> {
        let resNum = 0;
        let indicator: boolean[] = new boolean[this.actualLength];

        for (let i: int = 0; i < this.actualLength; i++) {
            if (fn(this.$_get(i), i)) {
                indicator[i] = true;
                resNum++;
            }
        }

        let res = new T[resNum];
        let j: int = 0;
        for (let i: int = 0; i < this.actualLength; i++) {
            if (indicator[i]) {
                res[j] = this.$_get(i);
                j++;
            }
        }

        return new Array<T>(res);
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
    public override filter(fn: (v: T, k: number, arr: ReadonlyArray<T>) => boolean): Array<T> {
        return this.filterArray((v: T, k: int): boolean => fn(v, k, this));
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
    public filter(fn: (v: T, k: number, arr: Array<T>) => boolean): Array<T> {
        return this.filterArray((v: T, k: int): boolean => fn(v, k, this));
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
    public override filter(fn: (v: T, k: number) => boolean): Array<T> {
        return this.filterArray((v: T, k: int): boolean => fn(v, k));
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
        return this.filterArray(fn);
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
    public override filter(fn: (v: T) => boolean): Array<T> {
        return this.filterArray((v: T, k: int): boolean => fn(v));
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
    public override slice(start: number, end: number): Array<T> {
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
        let len: int = this.actualLength;
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
        if (end == (Int.MAX_VALUE as int)) {
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
            res[n] = this.$_get(k);
            k++;
            n++;
        }

        return new Array<T>(res);
    }

    /**
     * Creates a new `Array` object and populates it with elements of `this` instance of `Array` class
     * selected from `start` to `Int.MAX_VALUE`, which means 'to the end of an array'.
     *
     * @param start zero-based index at which to start extraction
     *
     * @returns `Array` instance, constructed from extracted elements of `this` instance.
     */
    public override slice(start: number): Array<T> {
        return this.slice(start as int)
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
        return this.slice(start, Int.MAX_VALUE as int);
    }

    /**
     * Creates a new `Array` object and populates it with elements of `this` instance of `Array` class
     *
     * @returns `Array` instance, constructed all elements of `this` instance.
     *
     * @note This method creates full copy of original `Array` instance.
     */
    public override slice(): Array<T> {
        return this.slice(Int.MIN_VALUE as int, Int.MAX_VALUE as int);
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
    private doSort(a: T[], lo0: int, hi0: int, comparator: (a: T, b: T) => number): void {
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
     * using `toString()` method and compared using `compareTo() method of `string` class.
     *
     * @param a: Object - Object to be compared
     *
     * @param b: Object - Object to be compared
     *
     * @returns  Returns one of values -1, 0, 1 (_less_, _equal_, _greater_ respectively).
     */
    private static defaultComparator<T>(a: T, b: T): number {
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
        let len: int = this.actualLength;
        let res = new T[len];
        for (let i = 0; i < len; i++) {
            res[i] = this.$_get(i);
        }
        return res;
    }

    // NOTE: can't distinguish `=> number` and `=> int`
    /**
     * Reorders elements of `this` using comparator function.
     *
     * @param comparator function that defines the sort order.
     *
     * @note Mutating method
     *
     * NOTE clarify UTF-16 or UTF-8
     */
    public sort(comparator: (a: T, b: T) => number): Array<T> {
        let res = this.copyArray();
        this.doSort(res, 0, res.length-1, comparator);
        this.buffer = res;
        this.actualLength = this.buffer.length
        return this;
    }

    /**
     * Reorders elements of `this` using a default comparator.
     * Elements sorted in ascending order built upon converting the elements into strings, then comparing
     * their sequences of UTF-16 code units values.
     *
     * @note Mutating method
     *
     * NOTE clarify UTF-16 or UTF-8
     */
    public sort(): Array<T> {
        return this.sort(Array.defaultComparator)
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
    public override lastIndexOf(element: T, fromIndex: number): number {
        if (this.actualLength == 0) {
            return -1;
        }
        let n = fromIndex as int;
        let k: int;
        if (n >= 0) {
            k = min(this.actualLength - 1, n);
        } else {
            k = this.actualLength + n;
        }

        while (k >= 0) {
            if (this.$_get(k).equals(element)) {
                return k;
            }
            k--;
        }
        return -1;
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
    // NOTE(ivan-tyulyandin): investigate, an overloading issue
    // public lastIndexOf(element: T, fromIndex: int): int {
    //     if (this.actualLength == 0) {
    //         return -1;
    //     }
    //     let n = fromIndex;
    //     let k: int;
    //     if (n >= 0) {
    //         k = min(this.actualLength - 1, n);
    //     } else {
    //         k = this.actualLength + n;
    //     }

    //     while (k >= 0) {
    //         if (this.$_get(k).equals(element)) {
    //             return k;
    //         }
    //         k--;
    //     }
    //     return -1;
    // }

    /**
     * Returns the last index at which a given element can be found in the array,
     * or -1 if it is not present.
     *
     * @param element to find in the array.
     *
     * @returns The last index of the element in the array; -1 if not found.
     */
    public override lastIndexOf(element: T): number {
        return this.lastIndexOf(element, this.actualLength - 1);
    }

    /**
     * Returns the last index at which a given element can be found in the array,
     * or -1 if it is not present.
     *
     * @param element to find in the array.
     *
     * @returns The last index of the element in the array; -1 if not found.
     */
    // No overload by return type
    // public lastIndexOf(element: T): int {
    //     return this.lastIndexOf(element, this.actualLength - 1);
    // }

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
        let len = this.actualLength + values.length;
        let res = new T[len];
        let n = beforeIndex;
        let k: int;

        if (n >= 0) {
            k = min(this.actualLength - 1, n);
        } else {
            k = this.actualLength + n;
        }

        if (k < 0) {
            k = 0;
        }

        let idx = 0;

        for (let i = 0; i < k; i++) {
            res[idx] = this.$_get(i);
            idx++;
        }
        for (let i = 0; i < values.length; i++) {
            res[idx] = values[i];
            idx++;
        }

        for (let i = k; i < this.actualLength; i++) {
            res[idx] = this.$_get(i);
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
            k = min(this.actualLength - 1, n);
        } else {
            k = this.actualLength + n;
        }

        if (k < 0) {
            k = 0;
        }

        let m = min(k + count-1, this.actualLength-1);

        let idx = 0;

        let res = new T[this.actualLength - (m - k + 1)];

        for (let i = 0; i < k; i++) {
            res[idx] = this.$_get(i);
            idx++;
        }

        for (let i = m + 1; i < this.actualLength; i++) {
            res[idx] = this.$_get(i);
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
    public override equals(other: NullishType): boolean {
        if (!(other instanceof Array)) {
            return false;
        }
        let a = other as Array<Object>;
        if (this.actualLength != a.length) {
            return false;
        }

        for (let i = 0; i < this.actualLength; i++) {
            if (!Array.objEquals<Object>(this.$_get(i), a.$_get(i))) {
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
    public override toString(): string {
        let str = new StringBuilder()
        for (let i = 0; i < this.actualLength; i++) {
            const d: NullishType = this.$_get(i)
            if (d == null) {
                str.append("null")
            } else {
                str.append((d as Object).toString())
            }
            if(i != this.actualLength - 1) {
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
    public override toLocaleString(): string {
        return this.toLocaleString(new Object(), new Object())
    }

    /**
     * Removes the first element from an array and returns that removed element.
     * This method changes the length of the array.
     *
     * @returns shifted element, i.e. that was at index zero
     */
    public shift(): T | null {
        if(this.actualLength == 0) {
            return null
        }
        let obj = this.$_get(0)
        const other = this.slice(1, this.actualLength)
        this.buffer = other.buffer
        this.actualLength = other.actualLength
        return obj
    }

    /**
     * Removes the last element from an array and returns that element.
     * This method changes the length of the array.
     *
     * @returns removed element
     */
    public pop(): T | null {
        if(this.actualLength == 0) {
            return null
        }
        let obj = this.$_get(this.actualLength - 1)
        this.buffer[this.actualLength - 1] = null
        this.actualLength--
        return obj
    }

    /**
     * Adds the specified elements to the end of an array and returns the new length of the array.
     *
     * @returns new length
     */
    public push(val: T): number {
        if (this.actualLength == this.buffer.length) {
            const copy = new NullishType[this.buffer.length * 2 + 1]
            for (let i = 0; i < this.actualLength; i++) {
                copy[i] = this.buffer[i]
            }
            this.buffer = copy
        }
        this.actualLength += 1
        this.$_set(this.actualLength - 1, val)
        return this.actualLength
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
        if(start + delete > this.actualLength){
            delete = this.actualLength - start
        }
        let len = this.actualLength - delete
        let ret = new Array<T>(delete)
        let lastSet = start
        for(let i: int = start; i < this.actualLength; i++) {
            if (i < start + delete) {
                ret.$_set(i - start, this.$_get(i))
            } else {
                this.$_set(lastSet++, this.$_get(i))
            }
        }
        this.actualLength = lastSet
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
        return this.splice(start, this.actualLength - start)
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
     * @param fromIndex start index
     *
     * @returns true if val is in Array
     */
    public override includes(val: T, fromIndex: number): boolean {
        return this.indexOf(val, fromIndex) != -1
    }

    /**
     * Checks whether an Array includes a certain value among its entries,
     * returning true or false as appropriate.
     *
     * @param val value to search
     *
     * @returns true if val is in Array
     */
    public override includes(val: T): boolean {
        return this.indexOf(val) != -1
    }

    /**
     * Returns the first index at which a given element
     * can be found in the array, or -1 if it is not present.
     *
     * @param val value to search
     *
     * @param fromIndex index to search from
     *
     * @returns index of val, -1 otherwise
     */
    public override indexOf(val: T, fromIndex: number): number {
        for (let i = fromIndex as int; i < this.actualLength; i++) {
            if (Array.objEquals<T>(val, this.$_get(i))) {
                return i
            }
        }
        return -1
    }

    /**
     * Returns the first index at which a given element
     * can be found in the array, or -1 if it is not present.
     *
     * @param val value to search
     *
     * @returns index of val, -1 otherwise
     */
    public override indexOf(val: T): number {
        return this.indexOf(val, 0)
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
        for(let i = this.actualLength - 1; i >= 0; i--) {
            if (fn(this.$_get(i))) {
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
        let arr = new Array<T>(this.copyArray())
        arr.sort()
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
    public toSorted(comparator: (a: T, b: T) => number): Array<T> {
        let arr = new Array<T>(this.copyArray())
        arr.sort(comparator)
        return arr
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
        return true
    }

    /**
     * Copying version of the reverse() method.
     * It returns a new array with the elements in reversed order.
     *
     * @returns reversed copy of the current Array
     */
    public toReversed(): Array<T> {
        let arr = new T[this.actualLength]
        let count = this.actualLength - 1
        for(let i = 0; i < this.actualLength; i++) {
            arr[count] = this.$_get(i)
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
     * @returns a result after applying fn over all elements of the Array
     */
    public override reduce(fn: (a: T, b: T, index: number, arr: ReadonlyArray<T>) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get(0)
        for(let i = 1; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get(i), i as number, this)
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
    public reduce(fn: (a: T, b: T, index: number, arr: Array<T>) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get(0)
        for(let i = 1; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get(i), i as number, this)
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
    public override reduce(fn: (a: T, b: T, index: number) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get(0)
        for(let i = 1; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get(i), i)
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
    public override reduce(fn: (a: T, b: T) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get(0)
        for(let i = 1; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get(i))
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
     * @param initialValue initial value
     *
     * @returns a result after applying fn over all elements of the Array
     */
    public reduce(fn: (previousValue: T, currentValue: T, currentIndex: number, arr: Array<T>) => T, initialValue: T): T {
        let acc: T = initialValue
        for(let i = 0; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get(i), i as number, this)
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
     * @param initialValue initial value
     *
     * @returns a result after applying fn over all elements of the Array
     */
    public override reduce(fn: (previousValue: T, currentValue: T, currentIndex: number, arr: ReadonlyArray<T>) => T, initialValue: T): T {
        let acc: T = initialValue
        for(let i = 0; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get(i), i as number, this)
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
     * @param initialValue initial value
     *
     * @returns a result after applying fn over all elements of the Array
     */
    public override reduce(fn: (previousValue: T, currentValue: T, currentIndex: number) => T, initialValue: T): T {
        let acc: T = initialValue
        for(let i = 0; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get(i), i)
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
     * @param initialValue initial value
     *
     * @returns a result after applying fn over all elements of the Array
     */
    public override reduce(fn: (previousValue: T, currentValue: T) => T, initialValue: T): T {
        let acc: T = initialValue
        for(let i = 0; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get(i))
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
    public override reduceRight(fn: (a: T, b: T, index: number, arr: ReadonlyArray<T>) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get(this.actualLength - 1)
        for(let i = this.actualLength - 2; i >= 0; i--) {
            acc = fn(acc, this.$_get(i), i as number, this)
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
    public reduceRight(fn: (a: T, b: T, index: number, arr: Array<T>) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get(this.actualLength - 1)
        for(let i = this.actualLength - 2; i >= 0; i--) {
            acc = fn(acc, this.$_get(i), i as number, this)
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
    public override reduceRight(fn: (a: T, b: T, index: number) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get(this.actualLength - 1)
        for(let i = this.actualLength - 2; i >= 0; i--) {
            acc = fn(acc, this.$_get(i), i)
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
    public override reduceRight(fn: (a: T, b: T) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get(this.actualLength - 1)
        for(let i = this.actualLength - 2; i >= 0; i--) {
            acc = fn(acc, this.$_get(i))
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
    public override reduceRight(fn: (a: T, b: T, index: number, arr: ReadonlyArray<T>) => T, initVal: T): T {
        let acc: T = initVal
        for(let i = this.actualLength - 1; i >= 0; i--) {
            acc = fn(acc, this.$_get(i), i as number, this)
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
    public reduceRight(fn: (a: T, b: T, index: number, arr: Array<T>) => T, initVal: T): T {
        let acc: T = initVal
        for(let i = this.actualLength - 1; i >= 0; i--) {
            acc = fn(acc, this.$_get(i), i as number, this)
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
    public override reduceRight(fn: (a: T, b: T, index: number) => T, initVal: T): T {
        let acc: T = initVal
        for(let i = this.actualLength - 1; i >= 0; i--) {
            acc = fn(acc, this.$_get(i), i)
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
    public override reduceRight(fn: (a: T, b: T) => T, initVal: T): T {
        let acc: T = initVal
        for(let i = this.actualLength - 1; i >= 0; i--) {
            acc = fn(acc, this.$_get(i))
        }
        return acc
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param fn to apply for each element of the Array
     */
    public override forEach(fn: (a: T, index: number, arr: ReadonlyArray<T>) => void): void {
        for(let i = 0; i < this.actualLength; i++) {
            fn(this.$_get(i), i, this)
        }
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param fn to apply for each element of the Array
     */
    public forEach(fn: (a: T, index: number, arr: Array<T>) => void): void {
        for(let i = 0; i < this.actualLength; i++) {
            fn(this.$_get(i), i, this)
        }
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param fn to apply for each element of the Array
     */
    public override forEach(fn: (a: T, index: number) => void): void {
        for(let i = 0; i < this.actualLength; i++) {
            fn(this.$_get(i), i)
        }
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param fn to apply for each element of the Array
     */
    public override forEach(fn: (a: T) => void): void {
        for(let i = 0; i < this.actualLength; i++) {
            fn(this.$_get(i))
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
    public unshift(...values: T[]): number {
        let valLen = values.length as int
        let resLen = valLen + this.actualLength
        let arr = new NullishType[resLen];
        for (let i: int = 0; i < valLen; ++i) {
            arr[i] = values[i]
        }
        let curPlace = 0
        for (let i: int = valLen; i < resLen; ++i) {
            arr[i] = this.buffer[curPlace];
            ++curPlace
        }
        this.buffer = arr;
        this.actualLength = resLen
        return this.actualLength
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
        let arr = new T[this.actualLength]
        for(let i: int = 0; i < this.actualLength; i++) {
            arr[i] = this.$_get(i)
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
    public flat<U>(depth: number): Array<U> {
        return this.flat<U>(depth as int)
    }

    /**
     * Creates a new Array with all sub-array elements concatenated
     * into it recursively up to the specified depth.
     *
     * @param depth
     *
     * @returns a flattened Array with respect to depth
     */
    public flat<U>(depth: int): Array<U> {
        let ret = new Array<U>()
        this.flatImpl<U>(depth, ret)
        return ret
    }

    private flatImpl<U>(depth: int, to: Array<U>) {
        throw new Error("not implemented");
        //if (depth <= 0) {
        //    to.push(this as Object)
        //    return
        //}
        //for (let i = 0; i < this.actualLength; i++) {
        //    let e = this.$_get(i);
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
    public flat<U>(): Array<U> {
        return this.flat<U>(1)
    }

    /**
     * Applies flat and than map
     *
     * fn a function to apply
     *
     * @return new Array after map and than flat
     */
    // NOTE(ivan-tyulyandin): TBD, flatMap may be not subset, see ReadonlyArray
    public flatMap<U>(fn: (v: T, k: number, arr: Array<T>) => U): Array<U> {
        let mapped: Array<U> = this.map<U>(fn)
        return mapped.flat<U>()
    }

    /**
     * Applies flat and than map
     *
     * fn a function to apply
     *
     * @return new Array after map and than flat
     */
     // NOTE(ivan-tyulyandin): TBD, flatMap may be not subset, see ReadonlyArray
    public flatMap<U>(fn: (v: T, k: number) => U): Array<U> {
        let mapped: Array<U> = this.map<U>(fn)
        return mapped.flat<U>()
    }

    /**
     * Applies flat and than map
     *
     * fn a function to apply
     *
     * @return new Array after map and than flat
     */
    // NOTE(ivan-tyulyandin): TBD, flatMap may be not subset, see ReadonlyArray
    public flatMap<U>(fn: (v: T) => U): Array<U> {
        let mapped: Array<U> = this.map<U>(fn)
        return mapped.flat<U>()
    }

    /**
     * Returns an iterator over all indices
     */
    public /* override */ keys(): IterableIterator<Number> { // NOTE(kprokopenko): must be number, blocked by frontend
        return new ArrayKeysIterator(this.actualLength)
    }

    /**
     * Returns an iterator over all values
     */
    public override values(): IterableIterator<T> {
        return new ArrayValuesIterator<T>(this)
    }

    /**
     * Returns an array iterator
     */
    // NOTE(ivan-tyulyandin): blocked by tuples implementation
    // public entries(): Iterator<[Int, T]> {
    //     let ret: Entry<Int, Object> = new Entry<Int, Object>[this.actualLength];
    //     for (let i: int = 0; i < this.actualLength; i++) {
    //         ret[i] = new Entry<Int, Object>(i, this.$_get(i));
    //     }
    //     return new MapIterator<Int, Object>(ret);
    // }
}
