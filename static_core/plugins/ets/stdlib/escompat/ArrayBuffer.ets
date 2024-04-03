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

/**
 * Either ArrayBuffer or SharedArrayBuffer
 */
// NOTE (kprokopenko): make abstract after abstract overloads get to work
abstract class Buffer {
    abstract getByteLength(): int
    abstract at(i: int): byte
    abstract set(i: int, b: byte)
    // NOTE (egor-porsev): should be slice(int, int) without an extra 'sliceInternal' method. This is a workaround for #13402
    abstract sliceInternal(begin: int, end: int): Buffer
}

export interface ArrayBufferView {
    /**
     * The ArrayBuffer instance referenced by the array.
     */
    readonly buffer: ArrayBufferLike

    /**
     * The length in bytes of the array.
     */
    readonly byteLength: number

    /**
     * The offset in bytes of the array.
     */
    readonly byteOffset: number
}

/**
 * JS ArrayBuffer API-compatible class
 */
export class ArrayBuffer extends Buffer
{
    /**
     * Creates ArrayBuffer with size equal to length parameter
     *
     * @param length size of ArrayBuffer
     */
    public constructor(length: int)
    {
        this.data = new byte[length]
        this._byteLength = length
    }

    /**
     * Creates ArrayBuffer with size equal to length parameter
     *
     * @param length size of ArrayBuffer
     */
    public constructor(length: number)
    {
        this(length as int)
    }

    /**
     * Checks if the passed Object is a View
     *
     * @param obj to check
     *
     * @returns true if obj is instance of typed array
     */
    public static isView(obj: Object): boolean
    {
        //TODO(ivan-tyulyandin): add DataView and unsigned TypedArray API when it will be implemented
        return ( obj instanceof Int8Array
            || obj instanceof Int16Array
            || obj instanceof Int32Array
            || obj instanceof BigInt64Array
            || obj instanceof Float32Array
            || obj instanceof Float64Array
            || obj instanceof Uint8Array
            || obj instanceof Uint8ClampedArray
            || obj instanceof Uint16Array
            || obj instanceof Uint32Array
            || obj instanceof BigUint64Array)
    }

    internal override sliceInternal(begin: int, end: int): Buffer {
        const byteLength = this.getByteLength()

        const startIndex = normalizeIndex(begin, byteLength)
        const endIndex = normalizeIndex(end, byteLength)

        let resultLength = endIndex - startIndex
        if (resultLength < 0) {
            resultLength = 0
        }

        let result = new ArrayBuffer(resultLength);

        if (resultLength == 0) return result
        for (let i = 0; i < resultLength; ++i) {
            result.set(i, this.data[startIndex + i])
        }

        return result
    }

    /**
     * Creates a new ArrayBuffer with copy of bytes in range [begin;end)
     *
     * @param begin an inclusive index to start copying with
     *
     * @param end a last exclusive index to stop copying
     *
     * @returns data taken from current ArrayBuffer with respect to begin and end parameters
     */
    public slice(begin: number, end?: number): ArrayBuffer {
        if (end == undefined) return this.slice(begin as int)
        else return this.slice(begin as int, end as int)
    }

    /**
     * Creates a new ArrayBuffer with copy of bytes in range [begin;end)
     *
     * @param begin an inclusive index to start copying with
     *
     * @param end a last exclusive index to stop copying
     *
     * @returns data taken from current ArrayBuffer with respect to begin and end parameters
     */
    public slice(begin: int, end?: int): ArrayBuffer {
        if (end == undefined) return this.sliceInternal(begin, this.getByteLength()) as ArrayBuffer
        else return this.sliceInternal(begin, end!) as ArrayBuffer
    }

    /**
     * Returns data at specified index.
     * No such method in JS library, required for TypedArrays
     *
     * @param i index
     *
     * @returns byte at index
     */
    internal override at(i: int): byte {
        return this.data[i]
    }

    /**
     * Sets data at specified index.
     * No such method in JS library, required for TypedArrays
     *
     * @param b new value
     *
     * @param i index
     */
    internal override set(i: int, b: byte) {
        this.data[i] = b
    }

    internal override getByteLength(): int {
        return this._byteLength;
    }

    get byteLength(): number {
        return this._byteLength
    }

    /**
     * Resizes the ArrayBuffer
     *
     * @param newLen new length
     */
    public resize(newLen : number): void {
        /* return */ this.resize(newLen as int)
    }

    /**
     * Resizes the ArrayBuffer
     *
     * @param newLen new length
     */
    //NOTE: return undefined
    public resize(newLen : int): void {
        let newData = new byte[newLen]
        let size = min(newLen, this.data.length)
        for (let i = 0; i < size; ++i) {
            newData[i] = this.data[i]
        }
        this.data = newData
        this._byteLength = newLen
    }

    public static native from(array: Object): ArrayBuffer;

    private data: byte[]
    /** Length in bytes */
    private _byteLength: int
}

/**
 * Internal non-movable memory of a SharedArrayBuffer
 */
class SharedMemory {
    private data: byte[]
    private waiterPtr: long

    private constructor() {}

    internal native static create(byteLength: int): SharedMemory;

    internal at(i: int): byte {
        return this.data[i]
    }

    internal set(i: int, value: byte): void {
        this.data[i] = value
    }

    internal getByteLength(): int {
        return this.data.length
    }

    internal native atomicAddI8(index: int, value: byte): byte;

    internal native atomicAndI8(index: int, value: byte): byte;

    internal native atomicCompareExchangeI8(index: int, expectedValue: byte, replacementValue: byte): byte;

    internal native atomicExchangeI8(index: int, value: byte): byte;

    internal native atomicLoadI8(index: int): byte;

    internal native atomicOrI8(index: int, value: byte): byte;

    internal native atomicStoreI8(index: int, value: byte): byte;

    internal native atomicSubI8(index: int, value: byte): byte;

    internal native atomicXorI8(index: int, value: byte): byte;

    internal native atomicWaitI32(byteOffset: int, value: int): int;

    internal native atomicWaitI64(byteOffset: int, value: long): int;

    internal native atomicTimedWaitI32(byteOffset: int, value: int, timeout: long): int;

    internal native atomicTimedWaitI64(byteOffset: int, value: long, timeout: long): int;

    internal native atomicNotify(byteOffset: int): int;

    internal native atomicBoundedNotify(byteOffset: int, count: int): int;
}

export class SharedArrayBuffer extends Buffer {
    private sharedMemory: SharedMemory

    public constructor(byteLength: int) {
        super()
        this.sharedMemory = SharedMemory.create(byteLength)
    }

    internal getSharedMemory(): SharedMemory {
        return this.sharedMemory
    }

    internal override getByteLength(): int {
        return this.sharedMemory.getByteLength()
    }

    /**
     * Returns data at specified index.
     * No such method in JS library, required for TypedArrays
     *
     * @param i index
     *
     * @returns byte at index
     */
    internal override at(i: int): byte {
        return this.sharedMemory.at(i)
    }

    /**
     * Sets data at specified index.
     * No such method in JS library, required for TypedArrays
     *
     * @param b new value
     *
     * @param i index
     */
    internal override set(i: int, b: byte) {
        this.sharedMemory.set(i, b)
    }

    internal override sliceInternal(begin: int, end: int): Buffer {
        // TODO(egor-porsev): code duplication
        let byteLength = this.getByteLength()
        if (begin < -byteLength) {
            begin = 0
        }
        if (begin < 0) {
            begin = byteLength + begin
        }
        if (begin >= byteLength) {
            begin = byteLength
        }

        if (end < -byteLength) {
            end = 0
        }
        if (end < 0) {
            end = byteLength + end
        }
        if (end >= byteLength) {
            end = byteLength
        }

        if (end <= begin) {
            return new SharedArrayBuffer(0)
        }

        let len = end - begin
        if (len < 0) {
            len = 0
        }
        let res = new SharedArrayBuffer(len);
        for (let i = 0; i < len; ++i) {
            res.set(i, this.at(begin + i))
        }
        return res
    }

    /**
     * Creates a new ArrayBuffer with copy of bytes in range [begin;end)
     *
     * @param begin an inclusive index to start copying with
     *
     * @param end a last exclusive index to stop copying
     *
     * @returns data taken from current ArrayBuffer with respect to begin and end parameters
     */
    public slice(begin: int, end: int): SharedArrayBuffer {
        return this.sliceInternal(begin, end) as SharedArrayBuffer
    }

    public grow(newLength: int) {
        // grow() cannot shrink the array and cannot expand it beyond 'maxByteLength'.
        // Source: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/SharedArrayBuffer/grow
        // This is because the underlying SharedMemory object is non-movable and cannot be reallocated.
        //
        // Possible implementation: when SharedArrayBuffer(curByteLength, { maxByteLength }) is constructed
        // preallocate a SharedMemory object of size 'maxByteLength' but the user should observe the length to be 'curByteLength'.
        // In other words, SharedMemory may be implemented as (pointer to data, current length, max capacity).
        throw new Error("not implemented");
    }
}

export type ArrayBufferLike = ArrayBuffer | SharedArrayBuffer
