/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

class SparseArrayKeysIterator<T> implements IterableIterator<int> {
    private parent: SparseArray<T>
    private idx: int = 0
    private isDone: boolean = false

    constructor(parent: SparseArray<T>) {
        this.parent = parent
    }

    override next(): IteratorResult<int> {
        while (this.idx < this.parent.length) {
            if (this.parent.$_get(this.idx++) !== undefined) {
                return new IteratorResult<int>((this.idx - 1))
            }
        }
        this.isDone = true
        return new IteratorResult<int>()
    }

    override $_iterator(): IterableIterator<int> {
        return this
    }
}

class SparseArrayValuesIterator<T> implements IterableIterator<T> {
    private parent: SparseArray<T>
    private idx: int = 0
    private isDone: boolean = false

    constructor(parent: SparseArray<T>) {
        this.parent = parent
    }

    override next(): IteratorResult<T> {
        while (this.idx < this.parent.length) {
            const val: T | undefined = this.parent.$_get(this.idx++)
            if (val !== undefined) {
                return new IteratorResult<T>(val)
            }
        }
        this.isDone = true
        return new IteratorResult<T>()
    }

    override $_iterator(): IterableIterator<T> {
        return this
    }
}

class SparseArrayEntriesIterator<T> implements IterableIterator<[int, T]> {
    private parent: SparseArray<T>
    private idx: int = 0
    private isDone: boolean = false

    constructor(parent: SparseArray<T>) {
        this.parent = parent
    }

    override next(): IteratorResult<[int, T]> {
        while (this.idx < this.parent.length) {
            const i: int = this.idx++
            const val: T | undefined = this.parent.$_get(i)
            if (val !== undefined) {
                const vl: [int, T] = [i, val]
                return new IteratorResult<[int, T]>(vl)
            }
        }
        this.isDone = true
        return new IteratorResult<[int, T]>()
    }

    override $_iterator(): IterableIterator<[int, T]> {
        return this
    }
}

/**
 * Represents a SparseArray.
 *
 * @class SparseArray
 * @implements Iterable<T>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 */
export final class SparseArray<T> implements Iterable<T> {
    private buffer: Map<int, T>
    private maxLength: int

    /**
     * Get the length of the array.
     *
     * @returns { int } The length of the array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public get length(): int {
        return this.maxLength
    }

    /**
     * Set the length of the array.
     *
     * @param { int } newLen The new length of the array.
     * @returns { void }
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    set length(newLen: int) {
        const len = newLen
        if (len < 0) {
            throw new RangeError("can't change length to negative")
        }
        for (let i: int = len; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                this.buffer.delete(i)
            }
        }
        this.maxLength = len
    }

    /**
     * Creates a new empty instance of SparseArray.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public constructor() {
        this.buffer = new Map<int, T>()
        this.maxLength = 0
    }

    /**
     * Creates a new instance of SparseArray with the specified initial length.
     *
     * @param { int } length The initial length of the array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * 
     */
    public constructor(length: int) {
        this.buffer = new Map<int, T>()
        this.maxLength = length < 0 ? 0 : length
    }

    /**
     * Creates a new instance of SparseArray with the given elements.
     *
     * @param { T[] } values The rest of the elements to initialize the array with.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public constructor(...values: T[]) {
        this.buffer = new Map<int, T>()
        this.maxLength = values.length
        for (let i: int = 0; i < values.length; i++) {
            this.buffer.set(i, values[i])
        }
    }

    /**
     * Creates a new `SparseArray` instance from an `ArrayLike` object.
     *
     * @param { ArrayLike<T> } arr An array-like object to convert to an array.
     * @returns { SparseArray<T> } A new SparseArray instance containing the elements from the array-like object.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static from<T>(arr: ArrayLike<T>): SparseArray<T> {
        const ret: SparseArray<T> = new SparseArray<T>()
        for (let i: int = 0; i < arr.length; i++) {
            ret.buffer.set(i, arr.$_get(i))
            if (i + 1 > ret.maxLength) {
                ret.maxLength = i + 1
            }
        }
        return ret
    }

    /**
     * Creates a new `SparseArray` instance from an iterable or array-like object.
     *
     * @param { ArrayLike<T> | Iterable<T> } iterable An iterable or array-like object to convert to an array.
     * @returns { SparseArray<T> } A new SparseArray instance containing the elements from the iterable.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static from<T>(iterable: ArrayLike<T> | Iterable<T>): SparseArray<T> {
        const ret: SparseArray<T> = new SparseArray<T>()
        let idx: int = 0
        iteratorForEach<T>(iterable.$_iterator(), (x: T): void => {
            ret.buffer.set(idx++, x)
        })
        if (idx > ret.maxLength) {
            ret.maxLength = idx
        }
        return ret
    }

    /**
     * Creates a new `SparseArray` instance from a `ArrayLike<T> | Iterable<T>` and applies a mapping function to each element.
     *
     * @param { ArrayLike<T> | Iterable<T> } iterable The source primitive array.
     * @param { function } mapfn A mapping function to call on every element of the array.
     * @returns { SparseArray<U> } A new SparseArray instance with the mapped values.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static from<T, U>(iterable: ArrayLike<T> | Iterable<T>, mapfn: (v: T, k: int) => U): SparseArray<U> {
        const ret: SparseArray<U> = new SparseArray<U>()
        let idx: int = 0
        iteratorForEach<T>(iterable.$_iterator(), (x: T): void => {
            ret.buffer.set(idx, mapfn(x, idx))
            idx += 1
        })
        if (idx > ret.maxLength) {
            ret.maxLength = idx
        }
        return ret
    }

    /**
     * Checks whether the passed value is an SparseArray.
     *
     * @param { Any } o The value to be checked.
     * @returns { boolean } True if the value is an SparseArray; otherwise, false.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static isArray(o: Any): boolean {
        return o instanceof SparseArray
    }

    /**
     * Creates a new SparseArray instance from a variable number of arguments, regardless of number or type of the arguments.
     *
     * @param { T[] } values Elements of which to create the array.
     * @returns { SparseArray<T> } A new SparseArray instance.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static of<T>(...values: T[]): SparseArray<T> {
        return new SparseArray<T>(...values)
    }

    /**
     * Adds the specified element to the end of an array and returns the new length of the array.
     *
     * @param { T } val The element to add to the end of the array.
     * @returns { int } The new length of the array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public push(val: T): int {
        this.buffer.set(this.maxLength, val)
        this.maxLength += 1
        return this.maxLength
    }

    /**
     * Adds the specified elements to the end of an array and returns the new length of the array.
     *
     * @param { T[] } val The elements to add to the end of the array.
     * @returns { int } The new length of the array upon which the method was called.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public push(...val: T[]): int {
        const length: int = val.length
        for (let i = 0; i < length; i++) {
            this.buffer.set(this.maxLength + i, val[i])
        }
        this.maxLength += length
        return this.maxLength
    }

    /**
     * Removes the last element from an array and returns that element. This method changes the length of the array.
     *
     * @returns { T | undefined } The removed element from the array; undefined if the array is empty.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public pop(): T | undefined {
        if (this.maxLength == 0) {
            return undefined
        }
        this.maxLength -= 1
        const val: T | undefined = this.buffer.get(this.maxLength)
        this.buffer.delete(this.maxLength)
        return val
    }

    /**
     * Removes the first element from an array and returns that removed element. This method changes the length of the array.
     *
     * @returns { T | undefined } The removed element from the array; undefined if the array is empty.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public shift(): T | undefined {
        if (this.maxLength == 0) {
            return undefined
        }
        const val: T | undefined = this.buffer.get(0)
        this.buffer.delete(0)
        for (let i: int = 1; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                this.buffer.set(i - 1, v as T)
            }
        }
        this.maxLength -= 1
        return val
    }

    /**
     * Changes the contents of an array by removing existing elements in place from the start index to the end.
     *
     * @param { int } start The index at which to start changing the array.
     * @returns { SparseArray<T> } An array containing the deleted elements.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public splice(start: int): SparseArray<T> {
        return this.splice(start, this.maxLength)
    }

    /**
     * Changes the contents of an array by removing or replacing existing elements and/or adding new elements in place.
     *
     * @param { int } start The index at which to start changing the array.
     * @param { int | undefined } delete The number of items to remove after the start index.
     * @param { T[] } items The elements to add to the array, beginning from start.
     * @returns { SparseArray<T> } An array containing the deleted elements.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public splice(start: int, delete: int | undefined, ...items: T[]): SparseArray<T> {
        if (delete === undefined) {
            delete = 0
        }
        start = normalizeIndex(start, this.maxLength)
        if (delete < 0) {
            delete = 0
        }
        if (start > this.maxLength - delete) {
            delete = this.maxLength - start
        }

        const ret: SparseArray<T> = new SparseArray<T>()
        for (let i: int = 0; i < delete; i++) {
            const idx: int = start + i
            if (idx < this.maxLength) {
                ret.buffer.set(i, this.buffer.get(idx) as T)
            }
        }
        ret.maxLength = delete

        const newBuffer: Map<int, T> = new Map<int, T>()
        const itemsLength: int = items.length
        const newLen: int = this.maxLength - delete + itemsLength

        for (let i: int = 0; i < start; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                newBuffer.set(i, v as T)
            }
        }

        for (let i: int = 0; i < itemsLength; i++) {
            newBuffer.set(start + i, items[i])
        }

        for (let i: int = start + delete; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                newBuffer.set(i - delete + itemsLength, v as T)
            }
        }

        this.buffer = newBuffer
        this.maxLength = newLen

        return ret
    }

    /**
     * Reverses an array in place. The first array element becomes the last, and the last array element becomes the
     * first.
     *
     * @returns { this } The reversed array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public reverse(): this {
        const newBuffer: Map<int, T> = new Map<int, T>()
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                newBuffer.set(this.maxLength - 1 - i, v as T)
            }
        }
        this.buffer = newBuffer
        return this
    }

    private static defaultComparator(a: Any, b: Any): int {
        if (a instanceof Number && b instanceof Number) {
            const x: Number = a
            const y: Number = b
            if (Number.isInteger(x) && Number.isInteger(y) &&
                x <= Int.MAX_VALUE / 128 && x >= Int.MIN_VALUE / 128 &&
                y <= Int.MAX_VALUE / 128 && y >= Int.MIN_VALUE / 128) {
                let z: int = x.toInt()
                let w: int = y.toInt()
                return z - w
            }
        } else if (a instanceof String && b instanceof String) {
            return a.compareTo(b)
        }
        let sa: String = new String(a)
        let sb: String = new String(b)
        return sa.compareTo(sb)
    }

    /**
     * Sorts the elements of an array in place and returns the reference to the same array, now sorted.
     *
     * @param { function } [comparator] Optional. A function that defines the sort order. If omitted, the array is sorted
     *     according to each character's Unicode code point value, according to the string conversion of each element.
     * @returns { this } The sorted array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public sort(comparator?: (a: T, b: T) => int): this {
        if (this.maxLength <= 1) {
            return this
        }

        const values: Array<T> = new Array<T>()
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                values.push(v as T)
            }
        }

        if (comparator == undefined) {
            values.sort((a: T, b: T): int => SparseArray.defaultComparator(a, b))
        } else {
            values.sort(comparator)
        }

        const newBuffer: Map<int, T> = new Map<int, T>()
        let idx: int = 0
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                newBuffer.set(i, values[idx++])
            }
        }
        this.buffer = newBuffer

        return this
    }

    /**
     * Makes a shallow copy of the Array part to another location in the same Array and returns it without modifying its
     * length.
     *
     * @param { int } target Zero-based index at which to copy the sequence to.
     * @param { int } start Zero-based index at which to start copying elements from.
     * @param { int } [end] Zero-based index at which to end copying elements from.
     * @returns { this } The modified array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public copyWithin(target: int, start: int = 0, end: int = this.maxLength): this {
        target = normalizeIndex(target, this.maxLength)
        start = normalizeIndex(start, this.maxLength)
        end = normalizeIndex(end, this.maxLength)

        if (end <= start) {
            return this
        }

        const newBuffer: Map<int, T> = new Map<int, T>()
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                newBuffer.set(i, v as T)
            }
        }

        if (target <= start) {
            let t: int = target
            for (let s: int = start; s < end; s++) {
                if (this.buffer.has(s)) {
                    const v: T | undefined = this.buffer.get(s)
                    newBuffer.set(t++, v as T)
                } else {
                    newBuffer.delete(t++)
                }
            }
        } else {
            let len: int = end - start
            if (target + len > this.maxLength) {
                len = this.maxLength - target
            }
            for (let i: int = 0; i < len; i++) {
                if (this.buffer.has(start + len - 1 - i)) {
                    const v: T | undefined = this.buffer.get(start + len - 1 - i)
                    newBuffer.set(target + len - 1 - i, v as T)
                } else {
                    newBuffer.delete(target + len - 1 - i)
                }
            }
        }

        this.buffer = newBuffer
        return this
    }

    /**
     * Changes all elements in the Array to a static value, from a start index to an end index.
     *
     * @param { T } value The value to fill the array with.
     * @param { int } [start] The index at which to start filling (optional).
     * @param { int } [end] The index at which to end filling (optional, not included).
     * @returns { this } The modified array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public fill(value: T, start: int = 0, end: int = this.maxLength): this {
        start = normalizeIndex(start, this.maxLength)
        end = normalizeIndex(end, this.maxLength)

        for (let i: int = start; i < end; i++) {
            this.buffer.set(i, value)
        }
        return this
    }

    /**
     * Creates a new `SparseArray` by merging this `SparseArray` instance with given arrays and/or values.
     *
     * @param { FixedArray<ConcatArray<T>> } items Arrays and/or values to concatenate into a new array.
     * @returns { SparseArray<T> } A new Array instance.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public concat(...items: FixedArray<ConcatArray<T>>): SparseArray<T> {
        const ret: SparseArray<T> = new SparseArray<T>()
        ret.maxLength = this.maxLength

        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                ret.buffer.set(i, v as T)
            }
        }

        let offset: int = this.maxLength
        for (let j: int = 0; j < items.length; j++) {
            const arr: ConcatArray<T> = items[j]
            for (let i: int = 0; i < arr.length; i++) {
                const v: T = arr.$_get(i)
                ret.buffer.set(offset + i, v)
            }
            offset += arr.length
            ret.maxLength = offset
        }

        return ret
    }

    /**
     * Returns a copy of a section of an array.
     *
     * @param { int } start The beginning index of the specified portion of the array.
     * @returns { SparseArray<T> } A new SparseArray object containing the extracted elements.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public slice(start: int = 0, end: int = this.maxLength): SparseArray<T> {
        const len: int = this.maxLength
        const relStart: int = normalizeIndex(start, len)
        const relEnd: int = normalizeIndex(end, len)

        let count: int = relEnd - relStart
        if (count < 0) {
            count = 0
        }

        const ret: SparseArray<T> = new SparseArray<T>()
        ret.maxLength = count
        for (let i: int = 0; i < count; i++) {
            if (this.buffer.has(relStart + i)) {
                const v: T | undefined = this.buffer.get(relStart + i)
                ret.buffer.set(i, v as T)
            }
        }

        return ret
    }

    private isPrimitive(element: T): boolean {
        return (element instanceof String ||
                element instanceof Int ||
                element instanceof Double ||
                element instanceof Float ||
                element instanceof Long ||
                element instanceof Short ||
                element instanceof Boolean ||
                element instanceof Char ||
                element instanceof Byte ||
                element == null);
    }

    /**
     * Creates and returns a new string by concatenating all of the elements in an `SparseArray`, separated by a specified
     * separator string.
     *
     * @param { String } [sep] A string to separate each pair of adjacent elements of the array. If omitted, the array
     *     elements are separated with a comma.
     * @returns { string } A string with all array elements joined.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public join(sep?: String): string {
        if (this.maxLength == 0) {
            return ""
        }
        const sepStr: String = sep === undefined ? "," : sep!
        let objArray: boolean = false;
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i) && !this.isPrimitive(this.buffer.get(i) as T)) {
                objArray = true;
                break;
            }
        }

        if (objArray) {
            const visited: Set<SparseArray<T>> = new Set<SparseArray<T>>()
            const result: StringBuilder = new StringBuilder()
            this.joinObject(this, visited, result, sep)
            return result.toString()
        }
        return this.joinInternal(sepStr)
    }

    private joinInternal(sep: String): string {
        const sb: StringBuilder = new StringBuilder()
        let first: boolean = true

        for (let i: int = 0; i < this.maxLength; i++) {
            let strVal = ''
            if (!first) {
                sb.append(sep)
            }
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                strVal = new String(v)  
            }
            sb.append(strVal)
            first = false
        }

        return sb.toString()
    }

    private joinObject(arr: SparseArray<T>, visited: Set<SparseArray<T>>, result: StringBuilder, sep?: String): void {
        let length = arr.length
        if (length == 0 || visited.has(arr)) {
            return
        }
        visited.add(arr)
        const sepReal = sep === undefined ? ',' : sep!
        if (arr.buffer.has(0)) {
            const first_el = arr[0]
            if (first_el instanceof SparseArray) {
                this.joinObject(first_el as SparseArray<T>, visited, result)
            } else {
                result.append(new String(first_el))
            }
        }
        for (let i: int = 1; i < length; i++) {
            result.append(sepReal)
            if (!arr.buffer.has(i)) {
                continue
            }
            const tmp = arr[i]
            if (tmp instanceof SparseArray) {
                this.joinObject(tmp as SparseArray<T>, visited, result)
            } else {
                result.append(new String(tmp))
            }
        }
        visited.delete(arr)
    }

    /**
     * Returns a string representing the specified array and its elements.
     *

     * @returns { string } A string representing the array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public toString(): string {
        return this.join(",")
    }

    /**
     * Returns a string representing the elements of the array. The elements are converted to Strings using their
     * toLocaleString methods.
     *
     * @param { Intl.LocalesArgument } [ locales ] A string with a BCP 47 language tag, or an
     *     array of such strings.
     * @param { object } [ options ] An object with configuration properties.
     * @returns { string } A string representing the elements of the array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public toLocaleString(locales: Intl.LocalesArgument, options: Object): string {
        const len = this.maxLength;
        if (len > 0) {
            const sb = new StringBuilder()
            let x = this.$_get(0);
            if (x instanceof object) {
                sb.append(x.toLocaleString(locales, options))
            }
            for (let i: int = 1; i < len; ++i) {
                sb.append(",")
                let x = this.$_get(i);
                if (x instanceof object) {
                    sb.append(x.toLocaleString(locales, options))
                }
            }
            return sb.toString()
        }
        return ""
    }

    /**
     * Performs the specified action for each element in an array.
     *
     * @param { function } callbackfn A function that accepts up to three arguments. forEach calls the callbackfn
     *     function one time for each element in the array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public forEach(callbackfn: (value: T, index: int, array: SparseArray<T>) => void): void {
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                callbackfn(v as T, i, this)
            }
        }
    }

    /**
     * Creates a new array populated with the results of calling a provided function on every element in the calling
     * array.
     *
     * @param { function } callbackfn Function that is called for every element of the array. Each time callbackfn
     *     executes, the returned value is added to the new array.
     * @returns { SparseArray<U> } A new array with each element being the result of the callback function.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public map<U>(callbackfn: (value: T, index: int, array: SparseArray<T>) => U): SparseArray<U> {
        const ret: SparseArray<U> = new SparseArray<U>()
        ret.maxLength = this.maxLength

        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                ret.buffer.set(i, callbackfn(v as T, i, this))
            }
        }

        return ret
    }

    /**
     * Returns the elements of an array that meet the condition specified in a callback function.
     *
     * @param { function } predicate A function that accepts up to three arguments. The filter method calls the
     *     predicate function one time for each element in the array.
     * @returns { SparseArray<T> } A new array with the elements that pass the test.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public filter(predicate: (value: T, index: int, array: SparseArray<T>) => boolean): SparseArray<T> {
        const ret: SparseArray<T> = new SparseArray<T>()
        let idx: int = 0

        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                if (predicate(v as T, i, this)) {
                    ret.buffer.set(idx++, v as T)
                }
            }
        }

        ret.maxLength = idx
        return ret
    }

    /**
     * Calls the specified callback function for all the elements in an array. The return value of the callback function
     * is the accumulated result, and is provided as an argument in the next call to the callback function.
     *
     * @param { function } callbackfn A function that accepts up to four arguments. The reduce method calls the
     *     callbackfn function one time for each element in the array.
     * @returns { T } The accumulated result.
     * @throws { TypeError } Throws a TypeError if the array is empty and no initial value is provided.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public reduce(callbackfn: (previousValue: T, currentValue: T, index: int, array: SparseArray<T>) => T): T {
        let first: boolean = true
        let acc: T | undefined = undefined

        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                if (first) {
                    acc = v
                    first = false
                } else {
                    acc = callbackfn(acc as T, v as T, i, this)
                }
            }
        }

        if (first) {
            throw new TypeError("Reduce of empty array with no initial value")
        }

        return acc as T
    }

    /**
     * Calls the specified callback function for all the elements in an array, in descending order. The return value of
     * the callback function is the accumulated result, and is provided as an argument in the next call to the callback
     * function.

     * @param { function } callbackfn A function that accepts up to four arguments. The reduceRight method calls the
     *     callbackfn function one time for each element in the array.
     * @param { T } initialValue The initial value of the accumulator.
     * @returns { T } The accumulated result.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public reduceRight(callbackfn: (previousValue: T, currentValue: T, index: int, array: SparseArray<T>) => T): T {
        let first: boolean = true
        let acc: T | undefined = undefined

        for (let i: int = this.maxLength - 1; i >= 0; i--) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                if (first) {
                    acc = v
                    first = false
                } else {
                    acc = callbackfn(acc as T, v as T, i, this)
                }
            }
        }

        if (first) {
            throw new TypeError("Reduce of empty array with no initial value")
        }

        return acc as T
    }

    /**
     * Determines whether the specified callback function returns true for any element of an array.
     *
     * @param { function } predicate A function that accepts up to three arguments. The some method calls the predicate
     *     function for each element in the array until the predicate returns a value which is
     *     coercible to the Boolean value true, or until the end of the array.
     * @returns { boolean } True if the predicate returns true for at least one element in the array; otherwise, false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public some(predicate: (value: T, index: int, array: SparseArray<T>) => boolean): boolean {
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i) && predicate(this.buffer.get(i) as T, i, this)) {
                return true
            }
        }
        return false
    }

    /**
     * Creates a new Array with all sub-array elements concatenated into it recursively up to the specified depth.
     *
     * @param { int } depth The depth level specifying how deep a nested array structure should be flattened.
     * @returns { SparseArray<U> } A new array with the sub-array elements concatenated into it.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public flat<U = T>(depth: int): SparseArray<U> {
        const ret: SparseArray<U> = new SparseArray<U>()
        let idx: int = 0

        for (let i: int = 0; i < this.maxLength; i++) {
            const element: T | undefined = this.buffer.get(i)
            if (element !== undefined) {
                if (depth > 0 && element instanceof SparseArray) {
                    const subArray: SparseArray<U> = (element as SparseArray<T>).flat<U>(depth - 1)
                    for (let j: int = 0; j < subArray.maxLength; j++) {
                        if (subArray.buffer.has(j)) {
                            const v: U | undefined = subArray.buffer.get(j)
                            ret.buffer.set(idx++, v as U)
                        }
                    }
                } else {
                    ret.buffer.set(idx++, element as U)
                }
            }
        }

        ret.maxLength = idx
        return ret
    }

    /**
     * Creates a new SparseArray with all sub-array elements concatenated into it recursively with a default depth of 1.
     *
     * @returns { SparseArray<U> } A new array with the sub-array elements concatenated into it.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public flat<U = T>(): SparseArray<U> {
        return this.flat<U>(1)
    }

    /**
     * Calls a defined callback function on each element of an array. Then, flattens the result into a new array.
     * This is identical to a map() followed by a flat() with depth 1.
     *
     * @param { function } fn A function that produces an element of the new SparseArray.
     * @returns { SparseArray<U> } A new array with each element being the result of the callback function and flattened.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public flatMap<U>(fn: (v: T, k: int, arr: SparseArray<T>) => U): SparseArray<U> {
        const ret: SparseArray<U> = new SparseArray<U>()
        let idx: int = 0

        for (let i: int = 0; i < this.maxLength; ++i) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                ret.buffer.set(idx++, fn(v as T, i, this))
            }
        }

        ret.maxLength = idx
        return ret
    }

    /**
     * Returns a new SparseArray Iterator object that contains the keys for each index in the array.
     *
     * @returns { IterableIterator<int> } A new SparseArray Iterator object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public keys(): IterableIterator<int> {
        return new SparseArrayKeysIterator<T>(this)
    }

    /**
     * Returns a new SparseArray Iterator object that contains the values for each index in the array.
     *
     * @returns { IterableIterator<T> } A new SparseArray Iterator object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public values(): IterableIterator<T> {
        return new SparseArrayValuesIterator<T>(this)
    }

    /**
     * Returns a new SparseArray Iterator object that contains the key/value pairs for each index in the array.
     *
     * @returns {IterableIterator<[int, T]> } A new SparseArray Iterator object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public entries(): IterableIterator<[int, T]> {
        return new SparseArrayEntriesIterator<T>(this)
    }

    /**
     * Takes an integer value and returns the item at that index, allowing for positive and negative integers.
     * Negative integers count back from the last item in the array.
     *

     * @param { int } index Zero-based index of the array element to be returned.
     * @returns { T | undefined} The element at the given index.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public at(index: int): T | undefined {
        if (this.maxLength == 0) {
            return undefined
        }
        let k: int = 0
        if (index >= 0) {
            k = index
        } else {
            k = this.maxLength + index
        }

        if (k < 0 || k >= this.maxLength) {
            return undefined
        }

        return this.buffer.get(k)
    }

    /**
     * Returns the value of the first element in the array where predicate is true, and undefined otherwise.
     *
     * @param { function } predicate A function to execute on each value in the array.
     * @returns { T | undefined } The value of the first element that satisfies the provided testing function;
     *     otherwise, undefined.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public find(predicate: (value: T, index: int, array: SparseArray<T>) => boolean): T | undefined {
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                if (predicate(v as T, i, this)) {
                    return v
                }
            }
        }
        return undefined
    }

    /**
     * Returns the index of the first element in the array where predicate is true, and -1 otherwise.
     *
     * @param { function } predicate A function to execute on each value in the array.
     * @returns { int } The index of the first element that satisfies the provided testing function; otherwise, -1.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public findIndex(predicate: (value: T, index: int, array: SparseArray<T>) => boolean): int {
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i) && predicate(this.buffer.get(i) as T, i, this)) {
                return i
            }
        }
        return -1
    }

    /**
     * Iterates the array in reverse order and returns the value of the first element that satisfies the provided
     * testing function.
     *
     * @param { function } predicate A function to execute on each value in the array.
     * @returns { T | undefined } The value of the element if found; otherwise, undefined.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public findLast(predicate: (elem: T, index: int, array: SparseArray<T>) => boolean): T | undefined {
        for (let i: int = this.maxLength - 1; i >= 0; i--) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                if (predicate(v as T, i, this)) {
                    return v
                }
            }
        }
        return undefined
    }

    /**
     * Iterates the array in reverse order and returns the index of the first element that satisfies the provided
     * testing function. If no elements satisfy the testing function, -1 is returned.
     *
     * @param { function } predicate A function to execute on each value in the array.
     * @returns { int } The index of the first element that satisfies the provided testing function; otherwise, -1.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public findLastIndex(predicate: (element: T, index: int, array: SparseArray<T>) => boolean): int {
        for (let i: int = this.maxLength - 1; i >= 0; i--) {
            if (this.buffer.has(i) && predicate(this.buffer.get(i) as T, i, this)) {
                return i
            }
        }
        return -1
    }

    private searchDouble(val: double, fi: int, len: int): boolean {
        if (isNaN(val)) {
            for (let i = fi; i < len; i++) {
                const tmp = this.$_get(i)
                if (tmp instanceof Double) {
                    if (isNaN(tmp)) {
                        return true
                    }
                }
            }
        } else {
            for (let i = fi; i < len; i++) {
                const tmp = this.$_get(i)
                if (tmp instanceof Double) {
                    if (val == tmp) {
                        return true
                    }
                }
            }
        }
        return false
    }

    private searchFloat(val: Float, fi: int, len: int): boolean {
        if (isNaN(val)) {
            for (let i = fi; i < len; i++) {
                const tmp = this.$_get(i)
                if (tmp instanceof Float) {
                    if (isNaN(tmp)) {
                        return true
                    }
                }
            }
        } else {
            for (let i = fi; i < len; i++) {
                const tmp = this.$_get(i)
                if (tmp instanceof Float) {
                    if (val == tmp) {
                        return true
                    }
                }
            }
        }
        return false
    }

    /**
     * Determines whether an array includes a certain value among its entries, returning true or false as appropriate.
     *
     * @param { T } val The value to search for.
     * @param { int } [fromIndex] The position in this array at which to begin searching for value.
     * @returns { boolean } True if the value is found, false otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public includes(val: T, fromIndex?: int): boolean {
        const fi: int = fromIndex === undefined ? 0 : asIntOrDefault(fromIndex, 0)
        const start: int = normalizeIndex(fi, this.maxLength)
        if (val instanceof Float) {
            return this.searchFloat(val, start, this.maxLength)
        } else if (val instanceof Double) {
            return this.searchDouble(val, start, this.maxLength)
        }
        for (let i: int = start; i < this.maxLength; i++) {
            if (this.buffer.has(i) && this.buffer.get(i) === val) {
                return true
            }
        }
        return false
    }

    /**
     * Returns the first index at which a given element can be found in the array, or -1 if it is not present.
     *
     * @param { T } val The element to locate in the array.
     * @param { int } [fromIndex] The index to start the search at.
     * @returns { int } The first index of the element in the array; -1 if not found.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public indexOf(val: T, fromIndex: int = 0): int {
        const start: int = normalizeIndex(fromIndex, this.maxLength)

        for (let i: int = start; i < this.maxLength; i++) {
            if (this.buffer.has(i) && this.buffer.get(i) === val) {
                return i
            }
        }
        return -1
    }

    /**
     * Returns the last index at which a given element can be found in the array, or -1 if it is not present. The array
     * is searched backwards.
     *

     * @param { T } searchElement The element to locate in the array.
     * @returns { int } The last index of the element in the array; -1 if not found.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public lastIndexOf(searchElement: T): int {
        return this.lastIndexOf(searchElement, this.maxLength - 1)
    }

    /**
     * Returns the last index at which a given element can be found in the array, or -1 if it is not present. The array
     * is searched backwards, starting at fromIndex.
     *

     * @param { T } searchElement The element to locate in the array.
     * @param { int } [fromIndex] The index at which to start searching backwards.
     * @returns { int } The last index of the element in the array; -1 if not found.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public lastIndexOf(searchElement: T, fromIndex: int = 0): int {
        if (this.maxLength == 0) {
            return -1
        }
        let start: int = fromIndex
        if (start < 0) {
            start = this.maxLength + start
        }
        if (start < 0) {
            return -1
        }
        if (start >= this.maxLength) {
            start = this.maxLength - 1
        }

        for (let i: int = start; i >= 0; i--) {
            if (this.buffer.has(i) && this.buffer.get(i) === searchElement) {
                return i
            }
        }
        return -1
    }

    /**
     * Get the element at the specified index.
     *
     * @param { int } idx The index of the element to get.
     * @returns { T } The element at the specified index.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public $_get(index: int): T | undefined {
        if (index < 0 || index >= this.maxLength) {
            return undefined
        }
        return this.buffer.get(index)
    }

    /**
     * Set the element at the specified index.
     *
     * @param { int } idx The index of the element to set.
     * @param { T } val The value to set at the specified index.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public $_set(index: int, value: T): void {
        if (index < 0) {
            return
        }
        if (index >= this.maxLength) {
            this.maxLength = index + 1
        }
        this.buffer.set(index, value)
    }

    /**
     * Adds the specified elements to the beginning of an SparseArray and returns the new length of the Array.
     *
     * @param { T[] } values The elements to add to the front of the array.
     * @returns { int } The new length of the array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public unshift(...values: T[]): int {
        const length: int = values.length
        const newBuffer: Map<int, T> = new Map<int, T>()

        for (let i: int = 0; i < length; i++) {
            newBuffer.set(i, values[i])
        }

        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                newBuffer.set(i + length, v as T)
            }
        }

        this.buffer = newBuffer
        this.maxLength += length
        return this.maxLength
    }

    /**
     * Determines whether all the members of an array satisfy the specified test.
     *
     * @param { function } predicate A function that accepts up to three arguments. The every method calls the predicate
     *     function for each element in the array until the predicate returns a value which is
     *     coercible to the Boolean value false, or until the end of the array.
     * @returns { boolean } True if the predicate returns true for all elements in the array; otherwise, false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public every(predicate: (value: T, index: int, array: SparseArray<T>) => boolean): boolean {
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i) && !predicate(this.buffer.get(i) as T, i, this)) {
                return false
            }
        }
        return true
    }

    /**
     * Returns an iterator over all values
     *
     * @returns { IterableIterator<T> } An iterator over all values.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public $_iterator(): IterableIterator<T> {
        return this.values()
    }

    /**
     * Returns a new sparse array with the elements in reversed order.
     *
     * @returns { SparseArray<T> } A new reversed sparse array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public toReversed(): SparseArray<T> {
        const ret: SparseArray<T> = new SparseArray<T>()
        ret.maxLength = this.maxLength

        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                ret.buffer.set(this.maxLength - 1 - i, v as T)
            }
        }

        return ret
    }

    /**
     * Returns a new array with the elements sorted in ascending order.
     *
     * @returns { SparseArray<T> } A new sorted array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public toSorted(): SparseArray<T> {
        const ret: SparseArray<T> = new SparseArray<T>()
        ret.maxLength = this.maxLength

        const values: Array<T> = new Array<T>()
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                values.push(v as T)
            }
        }

        values.sort((a: T, b: T): int => SparseArray.defaultComparator(a, b))

        let idx: int = 0
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                ret.buffer.set(i, values[idx++])
            }
        }

        return ret
    }

    /**
     * Returns a new array with the elements sorted using the provided comparator function.
     *
     * @param { function } comparator A function that defines the sort order.
     * @returns { SparseArray<T> } A new sorted array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public toSorted(comparator: (a: T, b: T) => int): SparseArray<T> {
        const ret: SparseArray<T> = new SparseArray<T>()
        ret.maxLength = this.maxLength

        const values: Array<T> = new Array<T>()
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                values.push(v as T)
            }
        }

        values.sort(comparator)

        let idx: int = 0
        for (let i: int = 0; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                ret.buffer.set(i, values[idx++])
            }
        }

        return ret
    }

    /**
     * Returns a new array with some elements removed and/or replaced at a given index.
     *

     * @param { int } start The zero-based index at which to start changing the array.
     * @param { int } delete The number of elements to remove.
     * @param { FixedArray<T> } items The elements to add to the array.
     * @returns { SparsedArray<T> } A new array with the changes applied.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public toSpliced(start: int, delete: int, ...items: FixedArray<T>): SparseArray<T> {
        start = normalizeIndex(start, this.maxLength)
        if (delete < 0) {
            delete = 0
        } else if (delete > this.maxLength) {
            delete = this.maxLength
        }
        if (start > this.maxLength - delete) {
            delete = this.maxLength - start
        }

        const ret: SparseArray<T> = new SparseArray<T>()
        ret.maxLength = this.maxLength - delete + items.length

        let offset: int = 0
        for (let i: int = 0; i < start; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                ret.buffer.set(i, v as T)
            }
        }

        for (let i: int = 0; i < items.length; i++) {
            ret.buffer.set(start + i, items[i])
        }

        for (let i: int = start + delete; i < this.maxLength; i++) {
            if (this.buffer.has(i)) {
                const v: T | undefined = this.buffer.get(i)
                ret.buffer.set(i - delete + items.length, v as T)
            }
        }

        return ret
    }
}
