/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
 * Either ArrayBuffer 
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
    readonly buffer: ArrayBuffer

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
    public constructor(length: int, maxByteLength?: int)
    {
        super()
        if (length < 0) {
            throw new RangeError("length can't be less than 0")
        }
        if (maxByteLength != undefined) {
            if (maxByteLength! < 0) {
                throw new RangeError("maxByteLength can't be less than 0")
            }
            if (length > maxByteLength) {
                throw new RangeError("length can't be greater than maxByteLength")
            }
        }
        this.isResizable = (maxByteLength != undefined)
        this.data = ArrayBuffer.allocateNonMovable(this.isResizable ? maxByteLength! : length)
        this._byteLength = length
        this.dataAddress = ArrayBuffer.getAddress(this.data!)
    }

    /**
     * Creates ArrayBuffer with size equal to length parameter
     *
     * @param length size of ArrayBuffer
     */
    public constructor(length: number, maxByteLength?: number)
    {
        this(length as int, (maxByteLength !== undefined) ? maxByteLength as int : undefined)
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
        return ( obj instanceof DataView
            || obj instanceof Int8Array
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

    override sliceInternal(begin: int, end: int): Buffer {
        const byteLength = this.getByteLength()

        const startIndex = normalizeIndex(begin, byteLength)
        const endIndex = normalizeIndex(end, byteLength)

        let resultLength = endIndex - startIndex
        if (resultLength < 0) {
            resultLength = 0
        }

        let result = new ArrayBuffer(resultLength)

        if (resultLength == 0) return result
        result.setValues(this, startIndex)

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
        if (this.detached) {
            throw new TypeError("ArrayBuffer was detached")
        }
        if (end == undefined) return this.sliceInternal(begin, this.getByteLength()) as ArrayBuffer
        else return this.sliceInternal(begin, end) as ArrayBuffer
    }

    /**
     * Returns data at specified index.
     * No such method in JS library, required for TypedArrays
     *
     * @param i index
     *
     * @returns byte at index
     *
     * @throws {IndexOutOfBoundsError} if `pos` is out of bound
     */
    public final at(pos: int): byte {
        if (this.data !== undefined) {
            this.doBoundaryCheck(pos)
            // NOTE(dslynko, #24647) research performance of managed code compared to intrinsics
            return this.data![pos]
        }
        // Fallback to access through native pointer
        return this.atImpl(pos)
    }

    /**
     * Sets data at specified index.
     * No such method in JS library, required for TypedArrays
     *
     * @param b new value
     *
     * @param i index
     *
     * @throws {IndexOutOfBoundsError} if `pos` is out of bound
     */
    public final set(pos: int, val: byte): void {
        if (this.data !== undefined) {
            this.doBoundaryCheck(pos)
            // NOTE(dslynko, #24647) research performance of managed code compared to intrinsics
            this.data![pos] = val
        } else {
            // Fallback to access through native pointer
            this.setImpl(pos, val)
        }
    }

    public override getByteLength(): int {
        return this._byteLength
    }

    get byteLength(): number {
        return this._byteLength
    }

    get detached(): boolean {
        return this.dataAddress == 0
    }

    get resizable(): boolean {
        return this.isResizable
    }

    /// @brief Explicitly does boundary check, as `data` array can shrink down
    private final doBoundaryCheck(pos: int): void {
        if (pos < 0 || pos >= this._byteLength) {
            throw new IndexOutOfBoundsError("ArrayBuffer position " + pos + " is out of bounds")
        }
    }

    /**
     * @brief Copies into `this` array `this.byteLength` bytes starting from `begin` index of `other` data
     * Always valid for a non-detached ArrayBuffer
     */
    private final native setValues(other: ArrayBuffer, begin: int): void
    /// @brief Reads byte from specified position. Always valid for a non-detached ArrayBuffer
    private final native atImpl(pos: int): byte
    /// @brief Writes byte at specified position. Always valid for a non-detached ArrayBuffer
    private final native setImpl(pos: int, val: byte): void

    /**
     * Resizes the ArrayBuffer
     *
     * @param newLen new length
     */
    //NOTE: return undefined
    public resize(newLen : int): void {
        if (this.detached) {
            throw new TypeError("ArrayBuffer was detached")
        }
        if (!this.resizable) {
            throw new TypeError("ArrayBuffer is not resizable")
        }
        if (newLen < 0 || newLen > this.data!.length) {
            throw new RangeError("new length cannot be larger than maxByteLength")
        }
        this._byteLength = newLen
    }

    /// @brief Allocates non-movable array of fixed length
    private native static allocateNonMovable(length: int): FixedArray<byte>

    /// @brief Returns native address of non-movable array
    private native static getAddress(array: FixedArray<byte>): long

    /**
    * Creates a new ArrayBuffer from an array of bytes.
    *
    * This native method initializes an ArrayBuffer using the provided byte array as the underlying data.
    *
    * @param {byte[]} arr - An array of bytes to initialize the ArrayBuffer.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the data from the byte array.
    */
    public static from(arr: FixedArray<byte>): ArrayBuffer {
        const bf = new ArrayBuffer(arr.length)
        copyTo(arr, bf.data!, 0, 0, arr.length)
        return bf
    }

    /**
    * Creates a new ArrayBuffer from an array of numbers.
    *
    * The numbers in the provided array are converted to bytes, and then a new ArrayBuffer is constructed
    * using these bytes.
    *
    * @param {number[]} array - An array of numbers representing byte values.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the data from the array.
    */
    public static from(array: number[]): ArrayBuffer {
        const buf = new ArrayBuffer(array.length)
        for (let i = 0; i < array.length; ++i) {
            buf.set(i, array[i] as byte)
        }
        return buf
    }

    /**
    * Creates a new ArrayBuffer from a Uint8Array.
    *
    * This method extracts the underlying ArrayBuffer from the provided Uint8Array and converts it
    * to an ArrayBuffer using the native conversion for byte arrays.
    *
    * @param {Uint8Array} u8arr - A typed array of unsigned 8-bit integers.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the same binary data as the Uint8Array.
    */
    public static from(u8arr: Uint8Array): ArrayBuffer {
        const arrBuff = u8arr.buffer as ArrayBuffer
        if (arrBuff.detached) {
            return new ArrayBuffer(0)
        }
        const result = new ArrayBuffer(arrBuff.byteLength)
        result.setValues(arrBuff, 0)
        return result
    }

    /**
    * Creates a new ArrayBuffer from a string using the specified encoding.
    *
    * This native method encodes the given string using the provided encoding and creates an
    * ArrayBuffer with the resulting bytes. The resulting ArrayBuffer is expected to have the
    * specified length.
    *
    * @param {string} str - The string to be converted.
    * @param {string} encoding - The encoding to use for converting the string.
    * @param {int} length - The desired length of the resulting ArrayBuffer.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the encoded string data.
    */
    public static native from(str: string, encoding: string): ArrayBuffer;

    /**
    * Creates a new ArrayBuffer by copying a segment from an existing ArrayBuffer.
    *
    * This native method copies a range of bytes from the source ArrayBuffer, starting at the
    * specified byte offset and continuing for the specified length, into a new ArrayBuffer.
    *
    * @param {ArrayBuffer} buff - The source ArrayBuffer.
    * @param {int} byteOffset - The starting byte index in the source ArrayBuffer.
    * @param {int} length - The number of bytes to copy.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the specified slice of the source.
    */
    public static native from(buff: ArrayBuffer, byteOffset: int, length: int): ArrayBuffer;

    /**
    * Creates a new ArrayBuffer from a segment of an existing ArrayBuffer.
    *
    * This overload accepts numeric values for the byte offset and length parameters,
    * converts them to the native integer type, and then copies the specified range from the source ArrayBuffer.
    *
    * @param {ArrayBuffer} buff - The source ArrayBuffer.
    * @param {number} byteOffset - The starting byte index to copy from.
    * @param {number} length - The number of bytes to copy.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the specified segment.
    */
    public static from(buff: ArrayBuffer, byteOffset: number, length: number): ArrayBuffer {
        return ArrayBuffer.from(buff, byteOffset as int, length as int);
    }

    /**
    * Creates a new ArrayBuffer by slicing a segment from an existing ArrayBuffer.
    *
    * If the byte offset or length parameters are omitted, they default to 0 and the full length of the
    * source ArrayBuffer, respectively.
    *
    * @param {ArrayBuffer} buffer - The source ArrayBuffer.
    * @param {number} [byteOffset=0] - The starting byte index in the source ArrayBuffer.
    * @param {number} [length=buffer.byteLength] - The number of bytes to copy.
    * @returns {ArrayBuffer} A new ArrayBuffer containing the specified slice of the source.
    */
    public static from(buffer: ArrayBuffer, byteOffset?: number, length?: number): ArrayBuffer {
        const resolvedByteOffset = byteOffset ?? 0;
        const resolvedLength = length ?? buffer.byteLength;
        return ArrayBuffer.from(buffer, resolvedByteOffset as int, resolvedLength as int);
    }

    /**
    * Creates a new ArrayBuffer from an object, interpreting the second parameter as either a byte offset or an encoding.
    *
    * When the object is a string and the second parameter is also a string, the method treats the inputs as
    * parameters for string conversion (using the specified encoding and length). If the object is an ArrayBuffer,
    * it copies a segment starting from the provided byte offset for the specified length.
    *
    * @param {Object} obj - The source object, which should be either a string or an ArrayBuffer.
    * @param {number | string} byteOffsetOrEncoding - If `obj` is an ArrayBuffer, this is the starting byte offset;
    *                                                 if `obj` is a string, this is the encoding to use.
    * @param {number} length - The number of bytes to copy or the desired length of the encoded string data.
    * @returns {ArrayBuffer} A new ArrayBuffer created from the provided object.
    * @throws {Error} If the input object type is unsupported or the arguments are incorrect.
    */
    public static fromObject(obj: Object, byteOffsetOrEncoding: number | string, length: number): ArrayBuffer {
        if (obj instanceof string && byteOffsetOrEncoding instanceof string) {
            return ArrayBuffer.from(obj, byteOffsetOrEncoding as string);
        }
        if (!(obj instanceof ArrayBuffer)) {
            throw Error("Incorrect args");
        }
        return ArrayBuffer.from(obj as ArrayBuffer, byteOffsetOrEncoding as int, length as int);
    }


    // Fields' order is important due to alignment
    private data: FixedArray<byte> | undefined
    // Length in bytes
    private _byteLength: int
    // Pointer to underlying array, which is either non-movable array or native
    private dataAddress: long
    private isResizable: boolean


    /**
    * Returns the number of bytes required to encode a given string using the specified encoding.
    *
    * This native method calculates the byte length of the provided text when it is encoded with the
    * specified character encoding. Note that the byte length may differ from the number of characters
    * in the string.
    *
    * @param {string} text - The string to evaluate.
    * @param {string} encoding - The encoding used to convert the string to bytes.
    * @returns {int} The number of bytes required to encode the string.
    */
    public static native bytesLength(text: string, encoding: string): int;

    /**
    * Decodes a segment of an ArrayBuffer to a string using the specified encoding.
    *
    * This native method converts a portion of the ArrayBuffer, starting at the given index and ending
    * before the specified end index, into a string. The decoding is performed using the provided encoding.
    *
    * @param {ArrayBuffer} buffer - The ArrayBuffer containing the data to decode.
    * @param {string} encoding - The encoding to use for decoding the data.
    * @param {int} start - The starting byte index (inclusive) from which to begin decoding.
    * @param {int} end - The ending byte index (exclusive) at which to stop decoding.
    * @returns {string} The decoded string representation of the specified segment of the ArrayBuffer.
    */
    public static native stringify(buffer: ArrayBuffer, encoding: string, start: int, end: int): string;

    internal native atomicAddI8(index: int, byteOffset: int, value: byte): double;

    internal native atomicAndI8(index: int, byteOffset: int, value: byte): double;

    internal native atomicCompareExchangeI8(index: int, byteOffset: int, expectedValue: byte, replacementValue: byte): double;

    internal native atomicExchangeI8(index: int, byteOffset: int, value: byte): double;

    internal native atomicLoadI8(index: int, byteOffset: int): double;

    internal native atomicOrI8(index: int, byteOffset: int, value: byte): double;

    internal native atomicStoreI8(index: int, byteOffset: int, value: byte): double;

    internal native atomicSubI8(index: int, byteOffset: int, value: byte): double;

    internal native atomicXorI8(index: int, byteOffset: int, value: byte): double;

    internal native atomicAddI16(index: int, byteOffset: int, value: short): double;

    internal native atomicAndI16(index: int, byteOffset: int, value: short): double;

    internal native atomicCompareExchangeI16(index: int, byteOffset: int, expectedValue: short, replacementValue: short): double;

    internal native atomicExchangeI16(index: int, byteOffset: int, value: short): double;

    internal native atomicLoadI16(index: int, byteOffset: int): double;

    internal native atomicOrI16(index: int, byteOffset: int, value: short): double;

    internal native atomicStoreI16(index: int, byteOffset: int, value: short): double;

    internal native atomicSubI16(index: int, byteOffset: int, value: short): double;

    internal native atomicXorI16(index: int, byteOffset: int, value: short): double;

    internal native atomicAddI32(index: int, byteOffset: int, value: int): double;

    internal native atomicAndI32(index: int, byteOffset: int, value: int): double;

    internal native atomicCompareExchangeI32(index: int, byteOffset: int, expectedValue: int, replacementValue: int): double;

    internal native atomicExchangeI32(index: int, byteOffset: int, value: int): double;

    internal native atomicLoadI32(index: int, byteOffset: int): double;

    internal native atomicOrI32(index: int, byteOffset: int, value: int): double;

    internal native atomicStoreI32(index: int, byteOffset: int, value: int): double;

    internal native atomicSubI32(index: int, byteOffset: int, value: int): double;

    internal native atomicXorI32(index: int, byteOffset: int, value: int): double;

    internal native atomicAddI64(index: int, byteOffset: int, value: long): double;

    internal native atomicAndI64(index: int, byteOffset: int, value: long): double;

    internal native atomicCompareExchangeI64(index: int, byteOffset: int, expectedValue: long, replacementValue: long): double;

    internal native atomicExchangeI64(index: int, byteOffset: int, value: long): double;

    internal native atomicLoadI64(index: int, byteOffset: int): double;

    internal native atomicOrI64(index: int, byteOffset: int, value: long): double;

    internal native atomicStoreI64(index: int, byteOffset: int, value: long): double;

    internal native atomicSubI64(index: int, byteOffset: int, value: long): double;

    internal native atomicXorI64(index: int, byteOffset: int, value: long): double;

    internal native atomicAddU8(index: int, byteOffset: int, value: byte): double;

    internal native atomicAndU8(index: int, byteOffset: int, value: byte): double;

    internal native atomicCompareExchangeU8(index: int, byteOffset: int, expectedValue: byte, replacementValue: byte): double;

    internal native atomicExchangeU8(index: int, byteOffset: int, value: byte): double;

    internal native atomicLoadU8(index: int, byteOffset: int): double;

    internal native atomicOrU8(index: int, byteOffset: int, value: byte): double;

    internal native atomicStoreU8(index: int, byteOffset: int, value: byte): double;

    internal native atomicSubU8(index: int, byteOffset: int, value: byte): double;

    internal native atomicXorU8(index: int, byteOffset: int, value: byte): double;

    internal native atomicAddU16(index: int, byteOffset: int, value: short): double;

    internal native atomicAndU16(index: int, byteOffset: int, value: short): double;

    internal native atomicCompareExchangeU16(index: int, byteOffset: int, expectedValue: short, replacementValue: short): double;

    internal native atomicExchangeU16(index: int, byteOffset: int, value: short): double;

    internal native atomicLoadU16(index: int, byteOffset: int): double;

    internal native atomicOrU16(index: int, byteOffset: int, value: short): double;

    internal native atomicStoreU16(index: int, byteOffset: int, value: short): double;

    internal native atomicSubU16(index: int, byteOffset: int, value: short): double;

    internal native atomicXorU16(index: int, byteOffset: int, value: short): double;

    internal native atomicAddU32(index: int, byteOffset: int, value: int): double;

    internal native atomicAndU32(index: int, byteOffset: int, value: int): double;

    internal native atomicCompareExchangeU32(index: int, byteOffset: int, expectedValue: int, replacementValue: int): double;

    internal native atomicExchangeU32(index: int, byteOffset: int, value: int): double;

    internal native atomicLoadU32(index: int, byteOffset: int): double;

    internal native atomicOrU32(index: int, byteOffset: int, value: int): double;

    internal native atomicStoreU32(index: int, byteOffset: int, value: int): double;

    internal native atomicSubU32(index: int, byteOffset: int, value: int): double;

    internal native atomicXorU32(index: int, byteOffset: int, value: int): double;

    internal native atomicAddU64(index: int, byteOffset: int, value: long): double;

    internal native atomicAndU64(index: int, byteOffset: int, value: long): double;

    internal native atomicCompareExchangeU64(index: int, byteOffset: int, expectedValue: long, replacementValue: long): double;

    internal native atomicExchangeU64(index: int, byteOffset: int, value: long): double;

    internal native atomicLoadU64(index: int, byteOffset: int): double;

    internal native atomicOrU64(index: int, byteOffset: int, value: long): double;

    internal native atomicStoreU64(index: int, byteOffset: int, value: long): double;

    internal native atomicSubU64(index: int, byteOffset: int, value: long): double;

    internal native atomicXorU64(index: int, byteOffset: int, value: long): double;
}

export type ArrayBufferLike = ArrayBuffer
