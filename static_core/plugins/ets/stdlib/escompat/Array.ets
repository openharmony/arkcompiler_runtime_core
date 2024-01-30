/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

class ArrayKeysIterator<T> implements IterableIterator<number> {
    private parent: Array<T>
    private idx: int = 0

    constructor(parent: Array<T>) {
        this.parent = parent
    }

    override next(): IteratorResult<number> {
        if (this.idx >= this.parent.actualLength) {
            return new IteratorResult<number>()
        }
        return new IteratorResult<number>(this.idx++ as number)
    }

    override iterator(): Iterator<number> {
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
        if (this.idx >= this.parent.actualLength) {
            return new IteratorResult<T>()
        }
        return new IteratorResult<T>(this.parent.$_get(this.idx++))
    }

    override iterator(): Iterator<T> {
        return this
    }
}

function normalizeIndex(idx: int, len: int): int {
    if (idx < -len) {
        return 0
    }
    if (idx < 0) {
        return len + idx
    }
    if (idx > len) {
        return len
    }
    return idx
}

class FromBuffer {}
const FROM_BUFFER = new FromBuffer()

function asIntOrDefault(n: Number | undefined, def: int): int {
    if (__runtimeIsSameReference(n, undefined)) {
        return def
    }
    return (n! as Object as Number) as number as int;
}

function stringOf(a: NullishType): String {
    if (__runtimeIsSameReference(a, null)) {
        return "null"
    }
    if (__runtimeIsSameReference(a, undefined)) {
        return "undefined"
    }
    return a!.toString()
}

function hashOf(a: NullishType): int {
    if (__runtimeEquals(a, null)) {
        return 0;
    }
    if (__runtimeEquals(a, undefined)) {
        return 1;
    }
    return a!.$_hashCode();
}

/**
 * Represents JS API-compatible Array
 */
export final class Array<T> implements /* ReadonlyArray<T>, */ Iterable<T> {
    private buffer: NullishType[]
    internal actualLength: int

    // NOTE(kprokopenko): override
    get length(): number {
        return this.actualLength as number
    }

    // NOTE(kprokopenko): rename
    set lengthRename(newLen: number): void {
        const len = newLen as int
        if (len < 0 || len > this.actualLength) {
            throw new RangeError("can't change length to bigger or negative")
        }
        this.actualLength = len
    }

    public /* override */ $_get(index: number): T {
        return this.$_get(index as int)
    }

    public $_set(i: number, val: T): void {
        this.$_set(i as int, val)
    }

    public $_get(idx: int): T {
        if (idx >= this.actualLength) {
            throw new RangeError("Out of bounds")
        }
        return this.buffer[idx] as T
    }

    private $_get_unsafe(idx: int): T {
        return this.buffer[idx] as T
    }

    public $_set(idx: int, val: T): void {
        if (idx >= this.actualLength) {
            throw new RangeError("Out of bounds")
        }
        this.buffer[idx] = val as Object
    }

    private $_set_unsafe(idx: int, val: T): void {
        this.buffer[idx] = val as NullishType
    }

    /**
     * Creates a new instance of Array
     */
    public constructor(arrayLen: number) {
        this.buffer = new NullishType[arrayLen as int];
        this.actualLength = arrayLen as int // TODO(kprokopenko): this is incorrect! null-safety
    }

    internal constructor(_tag: FromBuffer, buf: NullishType[]) {
        this.buffer = buf
        this.actualLength = buf.length
    }

    internal constructor() {
        this.buffer = new NullishType[4];
        this.actualLength = 0
    }

    /**
     * Creates a new instance of Array based on Object[]
     *
     * @param d Array initializer
     */
    public constructor(first: T, ...d: T[]) {
        this.buffer = new NullishType[d.length + 1];
        this.actualLength = d.length + 1

        this.buffer[0] = first

        for (let k: int = 0; k < d.length; k++) {
            this.$_set_unsafe(k + 1, d[k]);
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
     * Returns undefined if `index` < `-length()` or `index` >= `length()`.
     */
    public /* override */ at(index: number): T | undefined {
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
     * Returns undefined if `index` < `-length()` or `index` >= `length()`.
     */
    public at(index: int): T | undefined {
        let len = this.actualLength;
        let k: int;
        if (index >= 0) {
            k = index;
        } else {
            k = len + index;
        }

        if (k < 0 || k >= len) {
            return undefined;
        }

        return this.$_get_unsafe(k);
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
            res[idx] = this.$_get_unsafe(i);
            idx++;
        }
        for (let i = 0; i < other.actualLength; i++) {
            res[idx] = other.$_get_unsafe(i);
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
    // public /* override */ concat(...items: (T | ConcatArray<T>)[]): Array<T> {
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
    public concat(...items: ConcatArray<T>[]): Array<T> {
        let totalAdd = this.actualLength
        for (let i = 0; i < items.length; i++) {
            totalAdd += items[i].lengthMakeProp() as int
        }

        const res = new Array<T>(totalAdd)

        for (let i = 0; i < this.actualLength; i++) {
            res.buffer[i] = this.buffer[i]
        }

        let insertTo = this.actualLength
        for (let i = 0; i < items.length; i++) {
            const arr = items[i]
            const len = arr.lengthMakeProp() as int
            for (let j = 0; j < len; j++) {
                res.buffer[insertTo++] = arr.$_get(j) as NullishType
            }
        }

        return res;
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
    public copyWithin(target: number, start: number, end?: Number): /* this */ Array<T> { // NOTE(kprokopenko) blocked by #15372
        /* return */ this.copyWithin(target as int, start as int, asIntOrDefault(end, this.actualLength));
        return this;
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
    public copyWithin(target: int, start: int, end: int): this {
        target = normalizeIndex(target, this.actualLength)
        start = normalizeIndex(start, this.actualLength)
        end = normalizeIndex(end, this.actualLength)

        if (end <= start) {
            return this;
        }

        if (target <= start) {
            while (start < end) {
                this.$_set_unsafe(target++, this.$_get_unsafe(start++));
            }
        } else {
            let len = end - start;
            if (target + len > this.actualLength) {
                len = this.actualLength - target
            }
            for (let i = 0; i < len; i++) {
                this.$_set_unsafe(target + len - 1 - i, this.$_get_unsafe(start + len - 1 - i))
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
    public copyWithin(target: int, start: int): this {
        /* return */ this.copyWithin(target, start, this.actualLength);
        return this;
    }

    /**
     * Makes a shallow copy of the Array part to another location in the same Array and returns it without modifying its length.
     *
     * @param target index at which to copy the sequence
     *
     * @returns this array after transformation
     */
    public copyWithin(target: int): this {
        /* return */ this.copyWithin(target, 0, this.actualLength);
        return this;
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
    public fill(value: T, start?: Number, end?: Number): /* this */ Array<T> { // NOTE(kprokopenko) blocked by #15372
        /* return */ this.fill(value, asIntOrDefault(start, 0), asIntOrDefault(end, this.actualLength));
        return this;
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
    public fill(value: T, start: int, end: int): this {
        start = normalizeIndex(start, this.actualLength);
        end = normalizeIndex(end, this.actualLength)

        for (let i = start; i < end; i++) {
            this.$_set_unsafe(i, value);
        }

        return this;
    }

    // NOTE(gogabr): should override from ReadonlyArray, but blocked by #15096
    public /* override */ find(predicate: (value: T, index: number, obj: Array<T>) => boolean): T | undefined {
        const res = this.findIndex(predicate)
        if (res == -1) {
            return undefined
        }
        return this.$_get(res as int)
    }

    public /* override */ find(predicate: (value: T, index: number) => boolean): T | undefined {
        const res = this.findIndex(predicate)
        if (res == -1) {
            return undefined
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
     * @returns found element or undefined otherwise
     */
    public find(fn: (elem: T) => boolean): T | undefined {
        for (let i = 0; i < this.actualLength; i++) {
            const val = this.$_get_unsafe(i);
            if (fn(val)) {
                return val;
            }
        }
        return undefined;
    }

    /**
     * Returns the index of the first element in an array that satisfies the provided testing function
     *
     * @param fn testing function
     *
     * @returns found element index or -1 otherwise
     */
    public /* override */ findIndex(fn: (value: T, index: number) => boolean): number {
        for (let i = 0; i < this.actualLength; i++) {
            if (fn(this.$_get_unsafe(i), i)) {
                return i;
            }
        }
        return -1;
    }

    // NOTE(gogabr): should override from ReadonlyArray, but blocked by #15096
    public /* override */ findIndex(predicate: (value: T, index: number, obj: Array<T>) => boolean): number {
        for (let i = 0; i < this.actualLength; i++) {
            if (predicate(this.$_get_unsafe(i), i, this)) {
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
    public /* override */ findIndex(fn: (value: T) => boolean): number {
        for (let i = 0; i < this.actualLength; i++) {
            if (fn(this.$_get_unsafe(i))) {
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
     * @returns found element or undefined otherwise
     */
    public findLast(fn: (elem: T) => boolean): T | undefined {
        for (let i = this.actualLength - 1; i >= 0; i--) {
            const val = this.$_get_unsafe(i);
            if (fn(val)) {
                return val;
            }
        }
        return undefined;
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
    public /* override */ join(sep?: String): string {
        const sepReal = __runtimeEquals(sep, undefined) ? "," : sep!
        let sb = new StringBuilder();
        for (let i: int = 0; i < this.actualLength; i++) {
            if (i != 0) {
                sb.append(sepReal);
            }
            sb.append(stringOf(this.$_get_unsafe(i) as NullishType));
        }

        return sb.toString();
    }

    /**
     * Creates a new `Array` object and populates
     * it with the results of calling a provided function on every element in `this` instance of `Array` class.
     *
     * @param fn mapping function, applied to each element of an array.
     *
     * @returns `Array` instance, constructed from `this` and given function.
     */
    // NOTE(gogabr): should override from ReadonlyArray, but blocked by #15096
    public /* override */ map<U>(fn: (value: T, index: number, arr: Array<T>) => U): Array<U> {
        let res = new NullishType[this.actualLength];
        for (let i = 0; i < this.actualLength; i++) {
            res[i] = fn(this.$_get_unsafe(i), i, this) as NullishType;
        }
        return new Array<U>(FROM_BUFFER, res);
    }

    /**
     * Creates a new `Array` object and populates
     * it with the results of calling a provided function on every element in `this` instance of `Array` class.
     *
     * @param fn mapping function, applied to each element of an array.
     *
     * @returns `Array` instance, constructed from `this` and given function.
     */
    public /* override */ map<U>(fn: (v: T, k: number) => U): Array<U> {
        let res = new NullishType[this.actualLength];
        for (let i = 0; i < this.actualLength; i++) {
            res[i] = fn(this.$_get_unsafe(i), i) as NullishType;
        }
        return new Array<U>(FROM_BUFFER, res);
    }

    /**
     * Creates a new `Array` object and populates
     * it with the results of calling a provided function on every element in `this` instance of `Array` class.
     *
     * @param fn mapping function, applied to each element of an array.
     *
     * @returns `Array` instance, constructed from `this` and given function.
     */
    public /* override */ map<U>(fn: (v: T) => U): Array<U> {
        let res = new NullishType[this.actualLength];
        for (let i = 0; i < this.actualLength; i++) {
            res[i] = fn(this.$_get_unsafe(i)) as NullishType;
        }
        return new Array<U>(FROM_BUFFER, res);
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
    // NOTE(gogabr): should override from ReadonlyArray, but blocked by #15096
    public /* override */ every(fn: (v: T, k: number, arr: Array<T>) => boolean): boolean {
        for (let i = 0; i < this.actualLength; i++) {
            if (!fn(this.$_get_unsafe(i), i, this)) {
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
    public /* override */ every(fn: (v: T, k: number) => boolean): boolean {
        for (let i = 0; i < this.actualLength; i++) {
            if (!fn(this.$_get_unsafe(i), i)) {
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
    // TODO: to add overloads with 'int' we need to enhance generics supports
    // public every(fn: (v: T, k: int) => boolean): boolean {
    //     for (let i = 0; i < this.actualLength; i++) {
    //         if (!fn(this.$_get_unsafe(i), i)) {
    //             return false
    //         }
    //     }
    //     return true;
    // }

    /**
     * Tests whether all elements in the array pass the test
     * implemented by the provided function. It returns a Boolean value.
     *
     * @param fn function to execute for each element in the array.
     * It should return a `true` to indicate the element passes the test, and a `false` value otherwise.
     *
     * @returns `true` if `fn` returns a `true` value for every array element. Otherwise, `false`.
     */
    public /* override */ every(fn: (v: T) => boolean): boolean {
        for (let i = 0; i < this.actualLength; i++) {
            if (!fn(this.$_get_unsafe(i))) {
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
    public /* override */ some(fn: (v: T, k: number) => boolean): boolean {
        for (let i = 0; i < this.actualLength; i++) {
            if (fn(this.$_get_unsafe(i), i)) { return true }
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
    // TODO: to add overloads with 'int' we need to enhance generics supports
    // public some(fn: (v: T, k: int) => boolean): boolean {
    //     for (let i = 0; i < this.actualLength; i++) {
    //         if (fn(this.$_get_unsafe(i), i)) { return true }
    //     }
    //     return false
    // }

    /**
     * Tests whether at least one element in the array pass the test
     * implemented by the provided function. It returns a Boolean value.
     *
     * @param fn function to execute for each element in the array.
     * It should return a `true` to indicate the element passes the test, and a `false` value otherwise.
     *
     * @returns `true` if `fn` returns a `true` value for at least one array element. Otherwise, `false`.
     */
    public /* override */ some(fn: (v: T) => boolean): boolean {
        for (let i = 0; i < this.actualLength; i++) {
            if (fn(this.$_get_unsafe(i))) { return true }
        }
        return false
    }

    /**
     * Modifies `this` instance of `Array` class and populates
     * it with same elements ordered towards the direction opposite to that previously stated.
     *
     * @note Mutating method
     */
    public reverse(): this {
        for (let i = 0; i < this.actualLength / 2; i++) {
            const tmp = this.$_get_unsafe(i)
            this.$_set_unsafe(i, this.$_get_unsafe(this.actualLength - 1 - i))
            this.$_set_unsafe(this.actualLength - 1 - i, tmp)
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
    public static from<T>(arr: ArrayLike<T> | Iterable<T>): Array<T> {
        return Array.from<T, T>(arr, (x: T, k: number): T => x)
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
    public static from<T, U>(arr: ArrayLike<T> | Iterable<T>, fn: (v: T, k: number) => U): Array<U> {
        if (arr instanceof ArrayLike) {
            const arrLike = arr as ArrayLike<T>
            const len = arrLike.lengthMakeProp()
            assert len >= 0
            const ret = new NullishType[len as int]
            for (let i = 0; i < len; i++) {
                ret[i] = fn(arrLike.$_get(i) as T, i) as NullishType;
            }
            return new Array<U>(FROM_BUFFER, ret)
        }
        const ret = new Array<U>();
        const idx = new int[1]
        idx[0] = 0
        iteratorForEach((arr as Iterable<T>).iterator(), (x: T): void => {
            ret.push(fn(x, idx[0] as number))
            idx[0] = idx[0] + 1
        })
        return ret;
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
    // NOTE(gogabr): should override from ReadonlyArray, but blocked by #15096
    public /* override */ filter(fn: (v: T, k: number, arr: Array<T>) => boolean): Array<T> {
        return this.filter((v: T, k: number): boolean => fn(v, k, this));
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
    public /* override */ filter(fn: (v: T, k: number) => boolean): Array<T> {
        const res = new Array<T>()

        for (let i: int = 0; i < this.actualLength; i++) {
            const val = this.$_get_unsafe(i)
            if (fn(val, i)) {
                res.push(val)
            }
        }
        return res;
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
    public /* override */ filter(fn: (v: T) => boolean): Array<T> {
        return this.filter((v: T, k: number): boolean => fn(v));
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
    public /* override */ slice(start?: Number, end?: Number): Array<T> {
        const len: int = this.actualLength;
        return this.slice(asIntOrDefault(start, 0), asIntOrDefault(end, len))
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
        const len: int = this.actualLength;
        const relStart = normalizeIndex(start, len)
        const relEnd = normalizeIndex(end, len)

        let count = relEnd - relStart;
        if (count < 0) {
            count = 0;
        }
        let res = new NullishType[count];
        for (let i = 0; i < count; i++) {
            res[i] = this.$_get_unsafe(relStart + i) as NullishType;
        }

        return new Array<T>(FROM_BUFFER, res);
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
    private static defaultComparator(a: NullishType, b: NullishType): number {
        let sa = stringOf(a);
        let sb = stringOf(b);
        return sa.compareTo(sb);
    }

    /**
     * Helper function preparing copy of `this` instance of `Array` class' data array.
     *
     * @returns Copy of an `Array`'s primitive array data.
     */
    private copyArray(): NullishType[] {
        let len: int = this.actualLength;
        let res = new NullishType[len];
        for (let i = 0; i < len; i++) {
            res[i] = this.$_get_unsafe(i) as NullishType;
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
     * NOTE clarify UTF-16 or UTF-8
     */
    public sort(comparator: (a: T, b: T) => number): this {
        sort(this.buffer, 0, this.actualLength, (l: NullishType, r: NullishType): boolean => {
            return comparator(l as T, r as T) < 0;
        });
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
    public sort(): this {
        sort(this.buffer, 0, this.actualLength, (l: NullishType, r: NullishType): boolean => {
            return Array.defaultComparator(l, r) < 0;
        });
        return this;
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
    public /* override */ lastIndexOf(element: T, fromIndex?: Number): number {
        if (this.actualLength == 0) {
            return -1;
        }
        let n = asIntOrDefault(fromIndex, this.actualLength - 1);
        let k: int;
        if (n >= 0) {
            k = min(this.actualLength - 1, n);
        } else {
            k = this.actualLength + n;
        }

        while (k >= 0) {
            if (__runtimeEquals(this.$_get_unsafe(k) as NullishType, element as NullishType)) {
                return k;
            }
            k--;
        }
        return -1;
    }

    /**
     * Returns a string representing the specified array and its elements.
     *
     * @returns string representation
     */
    public override toString(): string {
        return this.join(",");
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
    public /* override */ toLocaleString(): string {
        return this.toLocaleString(new Object(), new Object())
    }

    /**
     * Removes the first element from an array and returns that removed element.
     * This method changes the length of the array.
     *
     * @returns shifted element, i.e. that was at index zero
     */
    public shift(): T | undefined {
        if(this.actualLength == 0) {
            return undefined
        }
        let obj = this.$_get_unsafe(0)
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
    public pop(): T | undefined {
        if(this.actualLength == 0) {
            return undefined
        }
        let obj = this.$_get_unsafe(this.actualLength - 1)
        this.buffer[this.actualLength - 1] = null
        this.actualLength--
        return obj
    }

    // TODO(kprokopenko): remove when #14756 is fixed and rename below fucntion to push
    public push(val: T): number {
        this.ensureUnusedCapacity(1)
        this.buffer[this.actualLength] = val as NullishType
        this.actualLength += 1
        return this.actualLength
    }

    /**
     * Adds the specified elements to the end of an array and returns the new length of the array.
     *
     * @returns new length
     */
    public pushECMA(...val: T[]): number {
        this.ensureUnusedCapacity(val.length)
        for (let i = 0; i < val.length; i++) {
            this.buffer[this.actualLength + i] = val[i] as NullishType;
        }
        this.actualLength += val.length
        return this.actualLength
    }

    private ensureUnusedCapacity(cap: int): void {
        if (this.actualLength + cap > this.buffer.length) {
            const copy = new NullishType[this.buffer.length * 2 + cap]
            for (let i = 0; i < this.actualLength; i++) {
                copy[i] = this.buffer[i]
            }
            this.buffer = copy
        }
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
    public splice(start: number, delete: Number | undefined, ...items: T[]): Array<T> {
        return this.splice(start as int, asIntOrDefault(delete, this.actualLength), ...items)
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
    public splice(start: int, delete: int, ...items: T[]): Array<T> {
        start = normalizeIndex(start, this.actualLength)
        if (delete < 0) {
            delete = 0;
        }
        if (start > this.actualLength - delete) {
            delete = this.actualLength - start
        }
        // this: [left middle right], we must replace middle with `items`

        this.ensureUnusedCapacity(items.length - delete)
        const oldLen = this.actualLength
        this.actualLength = this.actualLength - delete + items.length

        let ret = new Array<T>(delete)
        let lastSet = start
        // left part remains unchanged
        // copy excluded part
        for (let i = 0; i < delete; i++) {
            ret.buffer[i] = this.buffer[start + i]
        }
        // move right part to the right of the buffer
        const rightLen = oldLen - start - delete
        if (items.length > delete) {
            for (let i = 0; i < rightLen; i++) {
                this.buffer[this.actualLength - 1 - i] = this.buffer[oldLen - 1 - i]
            }
        } else {
            for (let i = 0; i < rightLen; i++) {
                this.buffer[start + items.length + i] = this.buffer[start + delete + i]
            }
        }
        // insert middle part
        for (let i = 0; i < items.length; i++) {
            this.buffer[start + i] = items[i] as NullishType
        }
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
    public /* override */ includes(val: T, fromIndex?: Number): boolean {
        const found = this.indexOfInt(val, asIntOrDefault(fromIndex, 0));
        if (found != -1) {
            return true;
        }
        if (val instanceof Number && isNaN(val as Object as Number)) {
            const frm = normalizeIndex(asIntOrDefault(fromIndex, 0), this.actualLength)
            return this.findIndex((v: T, idx: number): boolean => {
                if (idx < frm) {
                    return false;
                }
                if (v instanceof Number) {
                    return isNaN(v as Object as Number)
                }
            }) != -1
        }
        return false;
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
    private indexOfInt(val: T, fromIndex: int): int {
        fromIndex = normalizeIndex(fromIndex, this.actualLength)
        for (let i = fromIndex; i < this.actualLength; i++) {
            if (__runtimeEquals(val as NullishType, this.$_get_unsafe(i) as NullishType)) {
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
     * @param fromIndex index to search from
     *
     * @returns index of val, -1 otherwise
     */
    public /* override */ indexOf(val: T, fromIndex: Number|undefined): number {
        return this.indexOfInt(val, asIntOrDefault(fromIndex, 0))
    }

    /**
     * Returns the first index at which a given element
     * can be found in the array, or -1 if it is not present.
     *
     * @param val value to search
     *
     * @returns index of val, -1 otherwise
     */
    public /* override */ indexOf(val: T): number {
        return this.indexOfInt(val, 0)
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
            if (fn(this.$_get_unsafe(i))) {
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
        let arr = new Array<T>(FROM_BUFFER, this.copyArray())
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
        let arr = new Array<T>(FROM_BUFFER, this.copyArray())
        arr.sort(comparator)
        return arr
    }

    /**
     * Checks whether the passed value is an Array.
     *
     * @param arr
     *
     * @returns true is arr is a non-nullish array, false otherwise
     */
    public static isArray(o: NullishType): boolean {
        if (o instanceof Array) {
            return true
        }
        /// TODO(kprokopenko) remove null checks
        if (__runtimeIsSameReference(o, null)) {
            return false
        }
        if (__runtimeIsSameReference(o, undefined)) {
            return false
        }
        return (Type.of(o!) instanceof ArrayType)
    }

    /**
     * Copying version of the reverse() method.
     * It returns a new array with the elements in reversed order.
     *
     * @returns reversed copy of the current Array
     */
    public toReversed(): Array<T> {
        let arr = new NullishType[this.actualLength]
        for(let i = 0; i < this.actualLength; i++) {
            arr[this.actualLength - 1 - i] = this.$_get_unsafe(i) as NullishType
        }
        return new Array<T>(FROM_BUFFER, arr)
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
    // NOTE(gogabr): should override from ReadonlyArray, but blocked by #15096
    public /* override */ reduce(fn: (a: T, b: T, index: number, arr: Array<T>) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc = this.$_get_unsafe(0)
        for(let i = 1; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get_unsafe(i), i as number, this)
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
    public /* override */ reduce(fn: (a: T, b: T, index: number) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get_unsafe(0)
        for(let i = 1; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get_unsafe(i), i as number)
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
    public /* override */ reduce(fn: (a: T, b: T) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get_unsafe(0)
        for(let i = 1; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get_unsafe(i))
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
    // NOTE(gogabr): should override from ReadonlyArray, but blocked by #15096
    public /* override */ reduce<U>(fn: (previousValue: U, currentValue: T, currentIndex: number, arr: Array<T>) => U, initialValue: U): U {
        let acc = initialValue
        for(let i = 0; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get_unsafe(i), i as number, this)
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
    public /* override */ reduce<U>(fn: (previousValue: U, currentValue: T, currentIndex: number) => U, initialValue: U): U {
        let acc = initialValue
        for(let i = 0; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get_unsafe(i), i as number)
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
    public /* override */ reduce<U>(fn: (previousValue: U, currentValue: T) => U, initialValue: U): U {
        let acc: U = initialValue
        for(let i = 0; i < this.actualLength; i++) {
            acc = fn(acc, this.$_get_unsafe(i))
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
    // NOTE(gogabr): should override from ReadonlyArray, but blocked by #15096
    public /* override */ reduceRight(fn: (a: T, b: T, index: number, arr: Array<T>) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get_unsafe(this.actualLength - 1)
        for(let i = this.actualLength - 2; i >= 0; i--) {
            acc = fn(acc, this.$_get_unsafe(i), i as number, this)
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
    public /* override */ reduceRight(fn: (a: T, b: T, index: number) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get_unsafe(this.actualLength - 1)
        for(let i = this.actualLength - 2; i >= 0; i--) {
            acc = fn(acc, this.$_get_unsafe(i), i as number)
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
    public /* override */ reduceRight(fn: (a: T, b: T) => T): T {
        if (this.actualLength == 0) {
            throw new TypeError("Reduce of empty array with no initial value")
        }
        let acc: T = this.$_get_unsafe(this.actualLength - 1)
        for(let i = this.actualLength - 2; i >= 0; i--) {
            acc = fn(acc, this.$_get_unsafe(i))
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
    // NOTE(gogabr): should override from ReadonlyArray, but blocked by #15096
    public /* override */ reduceRight<U>(fn: (a: U, b: T, index: number, arr: Array<T>) => U, initVal: U): U {
        let acc = initVal
        for(let i = this.actualLength - 1; i >= 0; i--) {
            acc = fn(acc, this.$_get_unsafe(i), i as number, this)
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
    public /* override */ reduceRight<U>(fn: (a: U, b: T, index: number) => U, initVal: U): U {
        let acc = initVal
        for(let i = this.actualLength - 1; i >= 0; i--) {
            acc = fn(acc, this.$_get_unsafe(i), i as number)
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
    public /* override */ reduceRight<U>(fn: (a: U, b: T) => U, initVal: U): U {
        let acc = initVal
        for(let i = this.actualLength - 1; i >= 0; i--) {
            acc = fn(acc, this.$_get_unsafe(i))
        }
        return acc
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param fn to apply for each element of the Array
     */
    // NOTE(gogabr): should override from ReadonlyArray, but blocked by #15096
    public /* override */ forEach(fn: (a: T, index: number, arr: Array<T>) => void): void {
        for(let i = 0; i < this.actualLength; i++) {
            fn(this.$_get_unsafe(i), i, this)
        }
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param fn to apply for each element of the Array
     */
    public /* override */ forEach(fn: (a: T, index: number) => void): void {
        for(let i = 0; i < this.actualLength; i++) {
            fn(this.$_get_unsafe(i), i as number)
        }
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param fn to apply for each element of the Array
     */
    public /* override */ forEach(fn: (a: T) => void): void {
        for(let i = 0; i < this.actualLength; i++) {
            fn(this.$_get_unsafe(i))
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
    public static of<T>(...values: T[]): Array<T> {
        const ret = new Array<T>()
        ret.ensureUnusedCapacity(values.length)
        for (let i = 0; i < values.length; i++) {
            ret.push(values[i])
        }
        return ret;
    }

    /**
     * Adds the specified elements to the beginning of an Array
     * and returns the new length of the Array.
     *
     * @param values data to be added
     *
     * @returns new length of the Array
     */
    public unshift(...values: T[]): number {
        let buffer = this.buffer
        if (this.buffer.length <= values.length + this.actualLength) {
            buffer = new NullishType[this.buffer.length * 2 + values.length]
        }
        for (let i = 0; i < this.actualLength; i++) {
            buffer[this.actualLength + values.length - i - 1] = this.buffer[this.actualLength - 1 - i];
        }
        for (let i = 0; i < values.length; i++) {
            buffer[i] = values[i] as NullishType;
        }
        this.buffer = buffer
        this.actualLength += values.length;
        return this.actualLength;
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
        if (index < 0) {
            index += this.actualLength
        }
        if (index >= this.actualLength) {
            throw new RangeError("Invalid index")
        }
        let arr = this.copyArray()
        arr[index] = value as NullishType
        return new Array<T>(FROM_BUFFER, arr)
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
        let ret = new Array<U>()
        this.flatImpl(depth as int, ret)
        return ret;
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
        return ret;
    }

    // TODO: to enable overloads with 'int' we need to enhance generics supports
    // /**
    //  * Creates a new Array with all sub-array elements concatenated
    //  * into it recursively up to the specified depth.
    //  *
    //  * @param depth
    //  *
    //  * @returns a flattened Array with respect to depth
    //  */
    // public flat(depth: int): Array<Object | null> {
    //     let ret = new Array<Object | null>()
    //     this.flatImpl(depth, ret)
    //     return ret
    // }

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
    // TODO(ivan-tyulyandin): TBD, flatMap may be not subset, see ReadonlyArray
    public flatMap<U>(fn: (v: T, k: number, arr: Array<T>) => U): Array<U> {
        let mapped: Array<U> = this.map<U>(fn)
        return mapped.flat<U>()
    }

    // TODO: to enable overloads with 'int' we need to enhance generics supports
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
        return new ArrayKeysIterator<T>(this)
    }

    /**
     * Returns an iterator over all values
     */
    public /* override */ values(): IterableIterator<T> {
        return new ArrayValuesIterator<T>(this)
    }

    /**
     * Returns an iterator over all values
     */
    public override iterator(): Iterator<T> {
        return this.values()
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
