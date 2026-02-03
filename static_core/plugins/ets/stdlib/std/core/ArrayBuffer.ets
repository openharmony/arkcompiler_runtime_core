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
 * @file Defines the ArrayBuffer for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Interface representing a view of an ArrayBuffer.
 *
 * @interface ArrayBufferView
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export interface ArrayBufferView {
    /**
     * The ArrayBuffer instance referenced by the array.
     *
     * @type { ArrayBuffer }
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    readonly buffer: ArrayBuffer

    /**
     * The length in bytes of the array.
     *
     * @type { int }
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    readonly byteLength: int

    /**
     * The offset in bytes of the array.
     *
     * @type { int }
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    readonly byteOffset: int
}

/**
 * JS ArrayBuffer API-compatible class.
 * Used to represent a generic, fixed-length raw binary data buffer.
 *
 * @class ArrayBuffer
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class ArrayBuffer
{
    /**
     * Returns the string representation of the ArrayBuffer.
     *
     * @returns { String } The string representation of the ArrayBuffer.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    toString(): String {
        return "[object ArrayBuffer]";
    }

    /**
     * Creates an ArrayBuffer with size equal to the length parameter.
     *
     * @param { int } length The size of the ArrayBuffer in bytes.
     * @param { int } [maxByteLength] Optional maximum size the ArrayBuffer can be resized to.
     * @throws { RangeError } Throws if length or maxByteLength is negative, or length > maxByteLength.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
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
        this.data = new FixedArray<byte>(this.isResizable ? maxByteLength! : length)
        this._byteLength = length
        this.dataAddress = 0 // not needed in managed code
    }

    /**
     * Creates an ArrayBuffer with size equal to the length parameter.
     *
     * @param { number } length The size of the ArrayBuffer in bytes.
     * @param { number } [maxByteLength] Optional maximum size the ArrayBuffer can be resized to.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(length: number, maxByteLength?: number)
    {
        this(length.toInt(), (maxByteLength !== undefined) ? maxByteLength.toInt() : undefined)
    }

    /**
     * Checks if the passed object is one of the ArrayBuffer views.
     *
     * @param { Object } obj The object to check.
     * @returns { boolean } Returns true if the object is a view; otherwise false.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
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

    /**
     * Internal implementation for slicing the ArrayBuffer.
     *
     * @param { int } begin Starting index.
     * @param { int } end Ending index.
     * @returns { ArrayBuffer } A new ArrayBuffer containing the sliced data.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    sliceInternal(begin: int, end: int): ArrayBuffer {
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
     * Creates a new ArrayBuffer with a copy of bytes in the range [begin, end).
     *
     * @param { int } begin An inclusive index to start copying from.
     * @param { int } [end] An exclusive index to stop copying.
     * @returns { ArrayBuffer } The new ArrayBuffer.
     * @throws { TypeError } Throws if the ArrayBuffer is detached.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public slice(begin: int, end?: int): ArrayBuffer {
        if (this.detached) {
            throw new TypeError("ArrayBuffer was detached")
        }
        if (end == undefined) return this.sliceInternal(begin, this.getByteLength()) as ArrayBuffer
        else return this.sliceInternal(begin, end) as ArrayBuffer
    }

    /**
     * Creates a new ArrayBuffer with a copy of bytes in the range [begin, end).
     *
     * @param { number } begin An inclusive index to start copying from.
     * @param { number } [end] An exclusive index to stop copying.
     * @returns { ArrayBuffer } The new ArrayBuffer.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public slice(begin: number, end?: number): ArrayBuffer {
        if (end == undefined) return this.slice(begin.toInt())
        else return this.slice(begin.toInt(), end.toInt())
    }

    /**
     * Returns the byte at the specified index.
     *
     * @param { int } pos The position in the buffer.
     * @returns { byte } The byte value.
     * @throws { IndexOutOfBoundsError } Throws if pos is out of bounds.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
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
     * Sets the byte value at the specified index.
     *
     * @param { int } pos The position in the buffer.
     * @param { byte } val The byte value to set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
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

    /**
     * Returns the length of the ArrayBuffer in bytes.
     *
     * @returns { int } The byte length.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getByteLength(): int {
        return this._byteLength
    }

    /**
     * Read-only property for the byte length of the ArrayBuffer.
     *
     * @returns { int } The byte length.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get byteLength(): int {
        return this._byteLength
    }

    /**
     * Returns true if the ArrayBuffer has been detached.
     *
     * @returns { boolean } True if the ArrayBuffer has been detached.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get detached(): boolean {
        return this.data != undefined ? false : this.dataAddress == 0
    }

    /**
     * Returns true if the ArrayBuffer can be resized.
     *
     * @returns { boolean } True if the ArrayBuffer can be resized.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get resizable(): boolean {
        return this.isResizable
    }

    /**
     * Performs boundary check due to `data` array possibly shrinking down
     *
     * @param pos index to be checked before accessing the underlying buffer
     */
    private final doBoundaryCheck(pos: int): void {
        if (pos < 0 || pos >= this._byteLength) {
            throw new IndexOutOfBoundsError("ArrayBuffer position " + pos + " is out of bounds")
        }
    }

    /**
     * Copies into `this` array `this.byteLength` bytes starting from `begin` index of `other` data.
     * Always valid for a non-detached ArrayBuffer.
     *
     * @param other ArrayBuffer with source data
     *
     * @param begin index inside `other` buffer
     */
    private final native setValues(other: ArrayBuffer, begin: int): void

    /**
     * Reads byte from specified position. Always valid for a non-detached ArrayBuffer
     *
     * @param pos index to read value from
     */
    private final native atImpl(pos: int): byte

    /**
     * Writes byte at specified position. Always valid for a non-detached ArrayBuffer
     *
     * @param pos index to write value into
     */
    private final native setImpl(pos: int, val: byte): void

    /**
     * Resizes the ArrayBuffer to the specified length.
     *
     * @param { int } newLen The new byte length.
     * @throws { TypeError, RangeError } Throws TypeError if the buffer is detached or not resizable,
     *     RangeError if the new length is invalid.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
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

    /**
     * Allocates non-movable array of fixed length
     */
    private native static allocateNonMovable(length: int): FixedArray<byte>

    /**
     * Returns native address of non-movable array
     *
     * @param array FixedArray to read native address from, must be allocated in non-movable memory
     */
    private native static getAddress(array: FixedArray<byte>): long

    /**
     * Creates a new ArrayBuffer from an array of bytes.
     *
     * @param { FixedArray<byte> } arr Source byte array.
     * @returns { ArrayBuffer } The new ArrayBuffer.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static from(arr: FixedArray<byte>): ArrayBuffer {
        const bf = new ArrayBuffer(arr.length)
        copyTo(arr, bf.data!, 0, 0, arr.length)
        return bf
    }

    /**
     * Creates a new ArrayBuffer from a Uint8Array.
     *
     * @param { Uint8Array } u8arr Source typed array.
     * @returns { ArrayBuffer } The new ArrayBuffer.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
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
     * Creates a new ArrayBuffer from an array of numbers.
     *
     * @param { double[] } array Source number array.
     * @returns { ArrayBuffer } The new ArrayBuffer.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static from(array: double[]): ArrayBuffer {
        const buf = new ArrayBuffer(array.length)
        for (let i = 0; i < array.length; ++i) {
            buf.set(i, array[i].toByte())
        }
        return buf
    }

    /**
     * Creates a new ArrayBuffer from a string with specific encoding.
     *
     * @param { string } str Source string.
     * @param { string } encoding String encoding (e.g., "utf8").
     * @returns { ArrayBuffer } The new ArrayBuffer.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native from(str: string, encoding: string): ArrayBuffer;

    /**
     * Creates a new ArrayBuffer from a segment of an existing ArrayBuffer.
     *
     * @param { ArrayBuffer } buff Source buffer.
     * @param { int } byteOffset Start offset in source.
     * @param { int } length Number of bytes to copy.
     * @returns { ArrayBuffer } The new ArrayBuffer.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native from(buff: ArrayBuffer, byteOffset: int, length: int): ArrayBuffer;

    /**
     * Creates a new ArrayBuffer from a segment of an existing ArrayBuffer with number parameters.
     *
     * @param { ArrayBuffer } buffer Source buffer.
     * @param { number } [byteOffset] Start offset.
     * @param { number } [length] Byte length.
     * @returns { ArrayBuffer } The new ArrayBuffer.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static from(buffer: ArrayBuffer, byteOffset?: number, length?: number): ArrayBuffer {
        const resolvedByteOffset = byteOffset ?? 0;
        const resolvedLength = length ?? buffer.byteLength.toDouble();
        return ArrayBuffer.from(buffer, resolvedByteOffset.toInt(), resolvedLength.toInt());
    }

    /**
     * Creates a new ArrayBuffer from an object.
     *
     * @param { Object } obj Source object (string or ArrayBuffer).
     * @param { int | string } byteOffsetOrEncoding Byte offset or encoding string.
     * @param { int } length Length to copy.
     * @returns { ArrayBuffer } The new ArrayBuffer.
     * @throws { Error } Throws if arguments are incorrect.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static fromObject(obj: Object, byteOffsetOrEncoding: int | string, length: int): ArrayBuffer {
        if (obj instanceof string) {
            const resolvedEncoding: string = byteOffsetOrEncoding instanceof string ? byteOffsetOrEncoding as string : "utf8";
            const resolvedbyteOffset: int = byteOffsetOrEncoding instanceof int ? byteOffsetOrEncoding as int : 0;
            return ArrayBuffer.from(ArrayBuffer.from(obj, resolvedEncoding), resolvedbyteOffset as int, length);
        }
        if (!(obj instanceof ArrayBuffer)) {
            throw Error("Incorrect args");
        }
        return ArrayBuffer.from(obj, byteOffsetOrEncoding as int, length);
    }


    // Fields' order is important due to alignment
    private data: FixedArray<byte> | undefined
    private weakRef: Object | undefined
    /**
     * Pointer to underlying array, which is either non-movable array or native.
     * Not needed in managed code.
     */
    private dataAddress: long
    // Length in bytes
    private _byteLength: int
    private isResizable: boolean


    /**
     * Returns the byte length of a string in a given encoding.
     *
     * @param { string } text Source string.
     * @param { string } encoding Encoding type.
     * @returns { int } Calculated byte length.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native bytesLength(text: string, encoding: string): int;

    /**
     * Converts a segment of an ArrayBuffer to a string.
     *
     * @param { ArrayBuffer } buffer Source buffer.
     * @param { string } encoding Encoding to use.
     * @param { int } start Start index.
     * @param { int } end End index.
     * @returns { string } Decoded string.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static native stringify(buffer: ArrayBuffer, encoding: string, start: int, end: int): string;

    private constructor createExternal(data: long, length: int) {
        // This constructor must be called only from boot
        const callerClass = Class.ofCaller()
        if (callerClass != undefined && callerClass.getLinker() != getBootRuntimeLinker()) {
            throw new RuntimeError("External ArrayBuffer can be instantiated only from boot context")
        }
        this.data = undefined
        this.weakRef = undefined
        this.dataAddress = data
        this._byteLength = length
        this.isResizable = false
    }

    private constructor createFinalizable(data: long, length: int, finalizer: long, finalizerData: long) {
        // This constructor must be called only from boot
        const callerClass = Class.ofCaller()
        if (callerClass != undefined && callerClass.getLinker() != getBootRuntimeLinker()) {
            throw new RuntimeError("External ArrayBuffer can be instantiated only from boot context")
        }
        this.data = undefined
        this.weakRef = ArrayBuffer.registerWeakRef(this, finalizer, finalizerData)
        this.dataAddress = data
        this._byteLength = length
        this.isResizable = false
    }

    /**
     * Detaches finalizer from `this` and returns the argument which was stored inside
     */
    private detach(): long {
        // This method must be called only from boot
        const callerClass = Class.ofCaller()
        if (callerClass != undefined && callerClass.getLinker() != getBootRuntimeLinker()) {
            throw new RuntimeError("ArrayBuffer can be detached only from boot context")
        }
        // Check that ArrayBuffer is external and was not detached yet
        if (this.data != undefined || this.detached) {
            return 0
        }
        this.dataAddress = 0
        this._byteLength = 0
        // Need to unregister finalizer to avoid double free
        let finalizerData: long = 0
        if (this.weakRef != undefined) {
            finalizerData = ArrayBuffer.unregisterWeakRef(this.weakRef!)
            if (finalizerData == 0) {
                throw new RuntimeError("Failed to unregister finalizer")
            }
        }
        this.weakRef = undefined
        return finalizerData
    }

    /**
     * Registers `FinalizableWeakRef` for the given ArrayBuffer instance
     */
    private static native registerWeakRef(self: ArrayBuffer, finalizer: long, finalizerData: long): Object

    /**
     * Unregisters `FinalizableWeakRef` instance and returns the finalizer argument which was stored inside
     */
    private static native unregisterWeakRef(weakRef: Object): long

    /**
     * Atomically adds a byte value to the current value at the specified position in the ArrayBuffer
     * and replaces it with the result. This operation is thread-safe and guarantees atomicity.
     * The method performs an atomic read-modify-write operation on an 8-bit signed integer (byte)
     * located at the calculated address: `byteOffset + index * sizeof(byte)`.
     *
     * @param { int } index The index of the element to which the byte value is added.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer to the beginning.
     * @param { byte } value The byte value to add to the current value at the target location. The addition is
     *     performed atomically.
     * @returns { long } The value at the target location before the atomic addition operation. 
     * @throws { RangeError } If the calculated byte offset is out of bounds for the ArrayBuffer.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAddI8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically performs a bitwise AND operation on an 8-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The value to AND with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAndI8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically compares and exchanges an 8-bit signed integer if the current value matches the expected value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } expectedValue The expected current value.
     * @param { byte } replacementValue The new value to set if the comparison succeeds.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicCompareExchangeI8(index: int, byteOffset: int, expectedValue: byte, replacementValue: byte): long;

    /**
     * Atomically exchanges an 8-bit signed integer with a new value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The new value to set.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicExchangeI8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically loads an 8-bit signed integer from the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @returns { long } The value at the target location.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicLoadI8(index: int, byteOffset: int): long;

    /**
     * Atomically performs a bitwise OR operation on an 8-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The value to OR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicOrI8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically stores an 8-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The value to store.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicStoreI8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically subtracts a byte value from the current value at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The value to subtract from the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicSubI8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically performs a bitwise XOR operation on an 8-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The value to XOR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicXorI8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically adds a short value to the current value at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to add to the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAddI16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically performs a bitwise AND operation on a 16-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to AND with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAndI16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically compares and exchanges a 16-bit signed integer if the current value matches the expected value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } expectedValue The expected current value.
     * @param { short } replacementValue The new value to set if the comparison succeeds.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicCompareExchangeI16(index: int, byteOffset: int, expectedValue: short, replacementValue: short): long;

    /**
     * Atomically exchanges a 16-bit signed integer with a new value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The new value to set.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicExchangeI16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically loads a 16-bit signed integer from the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @returns { long } The value at the target location.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicLoadI16(index: int, byteOffset: int): long;

    /**
     * Atomically performs a bitwise OR operation on a 16-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to OR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicOrI16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically stores a 16-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to store.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicStoreI16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically subtracts a short value from the current value at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to subtract from the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicSubI16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically performs a bitwise XOR operation on a 16-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to XOR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicXorI16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically adds an int value to the current value at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to add to the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAddI32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically performs a bitwise AND operation on a 32-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to AND with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAndI32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically compares and exchanges a 32-bit signed integer if the current value matches the expected value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } expectedValue The expected current value.
     * @param { int } replacementValue The new value to set if the comparison succeeds.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicCompareExchangeI32(index: int, byteOffset: int, expectedValue: int, replacementValue: int): long;

    /**
     * Atomically exchanges a 32-bit signed integer with a new value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The new value to set.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicExchangeI32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically loads a 32-bit signed integer from the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @returns { long } The value at the target location.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicLoadI32(index: int, byteOffset: int): long;

    /**
     * Atomically performs a bitwise OR operation on a 32-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to OR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicOrI32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically stores a 32-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to store.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicStoreI32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically subtracts an int value from the current value at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to subtract from the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicSubI32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically performs a bitwise XOR operation on a 32-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to XOR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicXorI32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically adds a long value to the current value at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to add to the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAddI64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically performs a bitwise AND operation on a 64-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to AND with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAndI64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically compares and exchanges a 64-bit signed integer if the current value matches the expected value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } expectedValue The expected current value.
     * @param { long } replacementValue The new value to set if the comparison succeeds.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicCompareExchangeI64(index: int, byteOffset: int, expectedValue: long, replacementValue: long): long;

    /**
     * Atomically exchanges a 64-bit signed integer with a new value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The new value to set.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicExchangeI64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically loads a 64-bit signed integer from the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @returns { long } The value at the target location.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicLoadI64(index: int, byteOffset: int): long;

    /**
     * Atomically performs a bitwise OR operation on a 64-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to OR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicOrI64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically stores a 64-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to store.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicStoreI64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically subtracts a long value from the current value at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to subtract from the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicSubI64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically performs a bitwise XOR operation on a 64-bit signed integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to XOR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicXorI64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically adds a byte value to the current value at the specified position (unsigned).
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The value to add to the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAddU8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically performs a bitwise AND operation on an 8-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The value to AND with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAndU8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically compares and exchanges an 8-bit unsigned integer if the current value matches the expected value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } expectedValue The expected current value.
     * @param { byte } replacementValue The new value to set if the comparison succeeds.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicCompareExchangeU8(index: int, byteOffset: int, expectedValue: byte, replacementValue: byte): long;

    /**
     * Atomically exchanges an 8-bit unsigned integer with a new value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The new value to set.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicExchangeU8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically loads an 8-bit unsigned integer from the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @returns { long } The value at the target location.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicLoadU8(index: int, byteOffset: int): long;

    /**
     * Atomically performs a bitwise OR operation on an 8-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The value to OR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicOrU8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically stores an 8-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The value to store.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicStoreU8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically subtracts a byte value from the current value at the specified position (unsigned).
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The value to subtract from the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicSubU8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically performs a bitwise XOR operation on an 8-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { byte } value The value to XOR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicXorU8(index: int, byteOffset: int, value: byte): long;

    /**
     * Atomically adds a short value to the current value at the specified position (unsigned).
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to add to the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAddU16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically performs a bitwise AND operation on a 16-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to AND with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAndU16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically compares and exchanges a 16-bit unsigned integer if the current value matches the expected value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } expectedValue The expected current value.
     * @param { short } replacementValue The new value to set if the comparison succeeds.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicCompareExchangeU16(index: int, byteOffset: int, expectedValue: short, replacementValue: short): long;

    /**
     * Atomically exchanges a 16-bit unsigned integer with a new value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The new value to set.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicExchangeU16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically loads a 16-bit unsigned integer from the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @returns { long } The value at the target location.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicLoadU16(index: int, byteOffset: int): long;

    /**
     * Atomically performs a bitwise OR operation on a 16-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to OR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicOrU16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically stores a 16-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to store.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicStoreU16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically subtracts a short value from the current value at the specified position (unsigned).
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to subtract from the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicSubU16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically performs a bitwise XOR operation on a 16-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { short } value The value to XOR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicXorU16(index: int, byteOffset: int, value: short): long;

    /**
     * Atomically adds an int value to the current value at the specified position (unsigned).
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to add to the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAddU32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically performs a bitwise AND operation on a 32-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to AND with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAndU32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically compares and exchanges a 32-bit unsigned integer if the current value matches the expected value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } expectedValue The expected current value.
     * @param { int } replacementValue The new value to set if the comparison succeeds.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicCompareExchangeU32(index: int, byteOffset: int, expectedValue: int, replacementValue: int): long;

    /**
     * Atomically exchanges a 32-bit unsigned integer with a new value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The new value to set.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicExchangeU32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically loads a 32-bit unsigned integer from the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @returns { long } The value at the target location.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicLoadU32(index: int, byteOffset: int): long;

    /**
     * Atomically performs a bitwise OR operation on a 32-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to OR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicOrU32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically stores a 32-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to store.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicStoreU32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically subtracts an int value from the current value at the specified position (unsigned).
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to subtract from the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicSubU32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically performs a bitwise XOR operation on a 32-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { int } value The value to XOR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicXorU32(index: int, byteOffset: int, value: int): long;

    /**
     * Atomically adds a long value to the current value at the specified position (unsigned).
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to add to the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAddU64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically performs a bitwise AND operation on a 64-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to AND with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicAndU64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically compares and exchanges a 64-bit unsigned integer if the current value matches the expected value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } expectedValue The expected current value.
     * @param { long } replacementValue The new value to set if the comparison succeeds.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicCompareExchangeU64(index: int, byteOffset: int, expectedValue: long, replacementValue: long): long;

    /**
     * Atomically exchanges a 64-bit unsigned integer with a new value.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The new value to set.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicExchangeU64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically loads a 64-bit unsigned integer from the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @returns { long } The value at the target location.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicLoadU64(index: int, byteOffset: int): long;

    /**
     * Atomically performs a bitwise OR operation on a 64-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to OR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicOrU64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically stores a 64-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to store.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicStoreU64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically subtracts a long value from the current value at the specified position (unsigned).
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to subtract from the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicSubU64(index: int, byteOffset: int, value: long): long;

    /**
     * Atomically performs a bitwise XOR operation on a 64-bit unsigned integer at the specified position.
     *
     * @param { int } index The index of the element.
     * @param { int } byteOffset The byte offset from the start of the ArrayBuffer.
     * @param { long } value The value to XOR with the current value.
     * @returns { long } The value before the operation.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native atomicXorU64(index: int, byteOffset: int, value: long): long;
}

/**
 * Define the alias ArrayBufferLike on ArrayBuffer
 *
 * @typedef { ArrayBuffer } ArrayBufferLike
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export type ArrayBufferLike = ArrayBuffer
