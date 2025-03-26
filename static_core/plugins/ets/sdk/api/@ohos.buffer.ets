/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

package api;


function getArrayBufferFrom(value: buffer.Buffer | Uint8Array): ArrayBuffer {
    if (value instanceof Uint8Array) {
        return ArrayBuffer.from(value);
    }
    return value.buffer;
}

/**
 * Namespace containing Buffer implementation and related utilities
 * for handling binary data and various encoding operations.
 */
export namespace buffer {

    /**
     * Supported character encodings for Buffer operations
     */

    export type BufferEncoding = 'ascii'
        | 'utf8'
        | 'utf-8'
        | 'utf16le'
        | 'ucs2'
        | 'ucs-2'
        | 'base64'
        | 'base64url'
        | 'latin1'
        | 'binary'
        | 'hex';

    type TypedArray = Int8Array;

    const U32_MAX: long = 4294967295 as long;

    /**
     * Allocates a new Buffer using an array of bytes in the range 0 – 255.
     * Array entries outside that range will be truncated to fit into it.
     *
     * @param {number[]} array - An array of bytes in the range 0 – 255
     * @returns {Buffer} A new allocated Buffer containing the array data
     */
    export function from(array: number[]): Buffer {
        return new Buffer(ArrayBuffer.from(array))
    }

    /**
     * Creates a view of the ArrayBuffer without copying the underlying memory.
     *
     * @param {ArrayBuffer} arrayBuffer - The source ArrayBuffer to create a view from
     * @param {number} [byteOffset=0] - Index of first byte to expose
     * @param {number} [length=arrayBuffer.byteLength - byteOffset] - Number of bytes to expose
     * @returns {Buffer} A view of the ArrayBuffer
     * @throws {Error} If byteOffset or length are out of valid range
     */
    export function from(arrayBuffer: ArrayBuffer, byteOffset?: number, length?: number): Buffer {
        const resolvedByteOffset: number = byteOffset ?? 0;
        const resolvedLength: number = length ?? arrayBuffer.byteLength - resolvedByteOffset;
        return new Buffer(ArrayBuffer.from(arrayBuffer, resolvedByteOffset, resolvedLength));
    }

    /**
    * For the object whose value returned by valueof() function is strictly equal to object
    * or supports symbol To primitive object, a new buffer instance is created.
    *
    * @param { Object } object - object object An object supporting Symbol.toPrimitive or valueOf()
    * @param { number | string } offsetOrEncoding - offsetOrEncoding offsetOrEncoding A byte-offset or encoding
    * @param { number } length - length length A length
    * @returns { Buffer } Return a new allocated Buffer
    */
    function from(object: Object, offsetOrEncoding: number | string, length: number): Buffer {
        return new Buffer(ArrayBuffer.fromObject(object, offsetOrEncoding, length))
    }

    /**
     * Copies the passed buffer data onto a new Buffer instance.
     *
     * @param {Buffer | Uint8Array} buffer - An existing Buffer or Uint8Array from which to copy data
     * @returns {Buffer} A new Buffer containing a copy of the provided buffer's data
     */
    export function from(buff: Buffer | Uint8Array): Buffer {
        if (buff instanceof Buffer) {
            const arrBuff: ArrayBuffer = (buff as Buffer).buffer;
            return new Buffer(ArrayBuffer.from(arrBuff));
        }
        return new Buffer(ArrayBuffer.from(buff as Uint8Array));
    }

    /**
     * Creates a new Buffer containing the provided string encoded using the specified encoding.
     *
     * @param {String} string - The string to encode into the buffer
     * @param {BufferEncoding} [encoding='utf8'] - The character encoding to use
     * @returns {Buffer} A new Buffer containing the encoded string
     */
    export function from(string: String, encoding?: BufferEncoding): Buffer {
        const resolvedEncoding: string = (encoding ?? "utf8");
        const byteLength: int = ArrayBuffer.bytesLength(string, resolvedEncoding);
        return new Buffer(ArrayBuffer.from(string, resolvedEncoding, byteLength));
    }

    /**
     * Returns the byte length of a string when encoded using `encoding`.
     * This is not the same as String.prototype.length, which does not account
     * for the encoding that is used to convert the string into bytes.
     *
     * @param {string | Buffer | TypedArray | DataView | ArrayBuffer} string - A value to calculate the length of
     * @param {BufferEncoding} [encoding='utf8'] - If `string` is a string, this is its encoding
     * @returns {number} The number of bytes contained within `string`
     * @throws {BusinessError} 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     */
    export function byteLength(
        doc: string | Buffer | TypedArray | DataView | ArrayBuffer,
        encoding?: BufferEncoding
    ): number {
        if (doc instanceof string) {
            let resolvedEncoding: string = encoding ?? "utf8";
            return ArrayBuffer.bytesLength(doc, resolvedEncoding);
        }
        // NOTE (templin.konstantin): Can't completely use smart casts due internal issue #21021
        //  now is used temporarily solution with reflection
        const objType = Type.of(doc);
        const parts: String[] = (objType as ClassType).getName().split('.');
        const klass = parts[parts.length - 1];
        switch (klass) {
            case "Buffer":
                return (doc as Buffer).length;
            case "Int8Array":
                return (doc as Int8Array).byteLength;
            case "DataView":
                return (doc as DataView).byteLength;
            case "ArrayBuffer":
                return (doc as ArrayBuffer).getByteLength();
        }
        throw new BusinessError("Invalid type", TypeErrorCodeId);
    }

    /**
     * Allocates a new Buffer for a fixed size bytes. If fill is undefined, the Buffer will be zero-filled.
     *
     * @param { number } size - The desired length of the new Buffer
     * @param { string | Buffer | number } [fill] - A value to pre-fill the new Buffer with
     * @param { BufferEncoding } [encoding] - If `fill` is a string, this is its encoding
     * @returns { Buffer } Return a new allocated Buffer
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     */
    export function alloc(
        size: number,
        fill?: string | Buffer | number,
        encoding?: BufferEncoding
    ): Buffer
    {
        if (size < 0) {
            throw new BusinessError('Parameter error: size must be a non-negative number.', TypeErrorCodeId);
        }
        const buffer = new Buffer(new ArrayBuffer(size));
        const resolvedEncoding = encoding ?? "utf-8";
        if (fill != undefined) {
            buffer.fill(fill, 0, size, resolvedEncoding);
        } else {
            buffer.fill(0, 0);
        }
        return buffer;
    }

    /**
     * Allocates a new Buffer for a fixed size bytes. The Buffer will not be initially filled.
     *
     * @param { number } size - The desired length of the new Buffer
     * @returns { Buffer } Return a new allocated Buffer
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     */
    export function allocUninitializedFromPool(size: number): Buffer {
        if (size < 0) {
            throw new BusinessError('Parameter error: size must be a non-negative number.', TypeErrorCodeId);
        }
        return new Buffer(new ArrayBuffer(size));
    }

    /**
     * Allocates a new un-pooled Buffer for a fixed size bytes. The Buffer will not be initially filled.
     *
     * @param { number } size - The desired length of the new Buffer
     * @returns { Buffer } Return a new allocated Buffer
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     */
    export function allocUninitialized(size: number): Buffer {
        if (size < 0) {
            throw new BusinessError('Parameter error: size must be a non-negative number.', TypeErrorCodeId);
        }
        return new Buffer(new ArrayBuffer(size));
    }

    /**
     * Compares buf1 to buf2
     *
     * @param { Buffer | Uint8Array } buf1 - A Buffer or Uint8Array instance.
     * @param { Buffer | Uint8Array } buf2 - A Buffer or Uint8Array instance.
     * @returns { number } 0 is returned if target is the same as buf
     *         1 is returned if target should come before buf when sorted.
     *        -1 is returned if target should come after buf when sorted.
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     */
    export function compare(buf1: Buffer | Uint8Array, buf2: Buffer | Uint8Array): number {
        const resolvedBuf1 = getArrayBufferFrom(buf1);
        const resolvedBuf2 = getArrayBufferFrom(buf2);
        const len1 = resolvedBuf1.byteLength;
        const len2 = resolvedBuf2.byteLength;
        const minLength = Math.min(len1, len2);
        for (let i = 0; i < minLength; i++) {
            if (resolvedBuf1.at(i) != resolvedBuf2.at(i)) {
                return resolvedBuf1.at(i) < resolvedBuf2.at(i) ? -1 : 1;
            }
        }
        return len1 == len2 ? 0 : (len1 < len2 ? -1 : 1);
    }

    /**
     * Returns a new `Buffer` which is the result of concatenating all the `Buffer`instances in the `list` together.
     *
     * @param { Buffer[] | Uint8Array[] } list - List of `Buffer` or Uint8Array instances to concatenate
     * @param { number } [totalLength] - Total length of the `Buffer` instances in `list` when concatenated
     * @returns { Buffer } Return a new allocated Buffer
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     * @throws { BusinessError } 10200001 - The value of "length" is out of range. It must be >= 0 and <= uint32 max. Received value is: [length]
     */
    export function concat(list: Buffer[] | Uint8Array[], totalLength?: number): Buffer {
        const resolvedList = new Array<ArrayBuffer>();
        for (const item of list) {
            resolvedList.push(getArrayBufferFrom(item));
        }
        const length = totalLength ?? resolvedList.reduce(
            (acc: number, item: ArrayBuffer): number => acc + item.byteLength, 0);
        if (length < 0 || length > U32_MAX) {
            throw new BusinessError(`The value of "length" is out of range.
It must be >= 0 and <= uint32 max. Received value is: ${length}`, OutOfBoundsErrorCodeId);
        }
        const listOfBytes = new Array<byte>();
        for (const item of resolvedList) {
            for (let i = 0; i < item.byteLength; i++) {
                listOfBytes.push(item.at(i));
            }
        }
        const buffer = new ArrayBuffer(length);
        let offset = 0;
        for (let i = 0; i < listOfBytes.length; i++) {
            buffer.set(offset, listOfBytes[i]);
            offset += 1;
        }
        return new Buffer(buffer);
    }

    /**
     * Returns true if obj is a Buffer, false otherwise
     *
     * @param { Object } obj - Objects to be judged
     * @returns { boolean } true or false
     */
    export function isBuffer(obj: Object): boolean {
        return obj instanceof Buffer;
    }

    /**
     * Returns true if encoding is the name of a supported character encoding, or false otherwise.
     *
     * @param { string } encoding - A character encoding name to check
     * @returns { boolean } true or false
     */
    export function isEncoding(encoding: string): boolean {
        switch (encoding) {
            case 'ascii':
            case 'utf8':
            case 'utf-8':
            case 'utf16le':
            case 'ucs2':
            case 'ucs-2':
            case 'base64':
            case 'base64url':
            case 'latin1':
            case 'binary':
            case 'hex':
                return true;
            default:
                return false;
        }
    }

    /**
     * Re-encodes the given Buffer or Uint8Array instance from one character encoding to another.
     *
     * @param { Buffer | Uint8Array } source - A Buffer or Uint8Array instance.
     * @param { string } fromEnc - The current encoding
     * @param { string } toEnc - To target encoding
     * @returns { Buffer } Returns a new Buffer instance
     * @throws { BusinessError } 401 - Parameter error. Possible causes:
     * 1. Mandatory parameters are left unspecified;
     * 2. Parameter verification failed.
     */
    export function transcode(source: Buffer | Uint8Array, fromEnc: string, toEnc: string): Buffer {
        const resolvedSource = getArrayBufferFrom(source);
        const sourceBytesLength = resolvedSource.byteLength as int;
        const str = ArrayBuffer.stringify(resolvedSource, fromEnc, 0, sourceBytesLength);
        const newBytesLength = ArrayBuffer.bytesLength(str, toEnc);
        const newBytes = ArrayBuffer.from(str, toEnc, newBytesLength);
        return new Buffer(newBytes);
    }

    class BufferIteratorKeys implements IterableIterator<number> {
        private length: int
        private idx: int = 0
        constructor(parent: Buffer) {
            this.length = parent.length as int
        }
        public override $_iterator(): IterableIterator<number> {
            return this
        }
        override next(): IteratorResult<number> {
            if (this.idx < 0 || this.idx >= this.length) {
                return new IteratorResult<number>()
            }
            return new IteratorResult<number>(false, this.idx++ as number)
        }
    }

    class BufferIteratorValues implements IterableIterator<number> {
        private length: int
        private parent: Buffer
        private idx: int = 0
        constructor(parent: Buffer) {
            this.length = parent.length as int
            this.parent = parent
        }
        public override $_iterator(): IterableIterator<number> {
            return this
        }
        override next(): IteratorResult<number> {
            if (this.idx < 0 || this.idx >= this.length) {
                return new IteratorResult<number>()
            }
            return new IteratorResult<number>(false, this.parent.at(this.idx++))
        }
    }

    class BufferEntriesIterator implements IterableIterator<[number, number]> {
        private length: int
        private parent: Buffer
        private idx: int = 0
        constructor(parent: Buffer) {
            this.length = parent.length as int
            this.parent = parent
        }
        public override $_iterator(): IterableIterator<[number, number]> {
            return this
        }
        override next(): IteratorResult<[number, number]> {
            if (this.idx < 0 || this.idx >= this.length) {
                return new IteratorResult<[number, number]>()
            }
            return new IteratorResult<[number, number]>(false, [this.idx, this.parent.at(this.idx++)])
        }
    }

    /**
     * A class representing a fixed-length sequence of bytes.
     * Provides methods for reading and manipulating binary data with various encodings.
     */
    export class Buffer {
        /** The underlying ArrayBuffer storing the binary data */
        public buffer: ArrayBuffer;
        /** The offset into the buffer where this Buffer instance starts */
        private byteOffset: number;

        /**
         * Gets the length of the buffer in bytes
         * @returns {number} The number of bytes in the buffer
         */
        get length(): number { return this.buffer.getByteLength() }

        /**
         * Creates a new Buffer instance
         * @param {ArrayBuffer} buffer - The underlying ArrayBuffer to use
         * @param {number} [byteOffset=0] - The starting offset into the ArrayBuffer
         */
        public constructor(buffer: ArrayBuffer, byteOffset: number = 0)
        {
            this.buffer = buffer;
            this.byteOffset = 0;
        }

        /**
         * Returns a string decoded from the buffer's contents.
         *
         * @param {BufferEncoding} [encoding='utf8'] - Character encoding to use for decoding
         * @param {number} [start=0] - Where to start decoding
         * @param {number} [end=buffer.length] - Where to stop decoding
         * @returns {string} The decoded string
         */
        public toString(encoding?: BufferEncoding, start?: number, end?: number): string {
            let resolvedEncoding: string = encoding ?? "utf8";
            let resolvedStart: int = (start ?? 0) as int;
            let resolvedEnd: int = (end ?? buffer.byteLength(this.buffer)) as int;
            return ArrayBuffer.stringify(this.buffer, resolvedEncoding, resolvedStart, resolvedEnd);
        }

        /**
         * Returns the byte at the specified index
         *
         * @param {int} index - Index of the byte to return
         * @returns {byte} The byte at the specified position
         */
        public at(index: int): byte {
            return this.buffer.at(index);
        }

        /**
         * Checks if the buffer includes the given value.
         *
         * @param {string | number | Buffer | Uint8Array} value - The value to search for
         * @param {number} [byteOffset=0] - The byte position to start searching from
         * @param {BufferEncoding} [encoding='utf8'] - Encoding to use if `value` is a string
         * @returns {boolean} `true` if the value is found, otherwise `false`
         */
        public includes(value: string | number | Buffer | Uint8Array, byteOffset?: number, encoding?: BufferEncoding): boolean {
            return this.indexOf(value, byteOffset ?? 0, encoding ?? "utf8") != -1;
        }

        /**
         * Returns the first index where `value` is found in the buffer.
         *
         * @param {string | number | Buffer | Uint8Array} value - The value to search for
         * @param {number} [byteOffset=0] - The byte position to start searching from.
         *   - If negative, it is counted from the end of the buffer (`len + byteOffset`).
         *   - If out of bounds (`>= buffer length`), returns `-1`.
         * @param {BufferEncoding} [encoding='utf8'] - Encoding to use if `value` is a string
         * @returns {number} The index of the first occurrence of `value`, or -1 if not found
         */
        public indexOf(value: string | number | Buffer | Uint8Array, byteOffset?: number, encoding?: BufferEncoding): number {
            const searchBuffer = this.normalizeValueToBuffer(value, encoding ?? "utf8");
            const len = this.length;
            const searchLen = searchBuffer.length;
            let startIndex = byteOffset ?? 0;

            if (startIndex < 0) {
                startIndex = Math.max(0, len + startIndex);
            } else if (startIndex >= len) {
                return -1;
            }
            for (let i = startIndex; i <= len - searchLen; i++) {
                if (this.compareSubarray(i, searchBuffer)) return i;
            }
            return -1;
        }

        /**
         * Returns the last index where `value` is found in the buffer.
         *
         * @param {string | number | Buffer | Uint8Array} value - The value to search for
         * @param {number} [byteOffset=buffer length] - The byte position to start searching from (backwards).
         *   - If negative, it is counted from the end of the buffer (`len + byteOffset`).
         *   - If out of bounds (`>= buffer length`), it is clamped to `buffer length - searchLen`.
         * @param {BufferEncoding} [encoding='utf8'] - Encoding to use if `value` is a string
         * @returns {number} The index of the last occurrence of `value`, or -1 if not found
         */
        public lastIndexOf(value: string | number | Buffer | Uint8Array, byteOffset?: number , encoding?: BufferEncoding): number {
            const searchBuffer = this.normalizeValueToBuffer(value, encoding ?? "utf8");
            const searchLen = searchBuffer.length;
            const len = this.length;

            if (searchLen == 0 || len < searchLen) {
                return -1;
            }
            let endIndex = byteOffset ?? len;
            if (endIndex < 0) {
                endIndex = Math.max(0, len + endIndex);
            } else if (endIndex >= len) {
                endIndex = len - searchLen;
            }
            for (let i = endIndex; i >= 0; i--) {
                if (this.compareSubarray(i, searchBuffer)) {
                    return i;
                }
            }
            return -1;
        }

        private normalizeValueToBuffer(value: string | number | Buffer | Uint8Array, encoding: BufferEncoding): Buffer {
            if (value instanceof string) {
                return buffer.from(value, encoding ?? "utf8");
            } else if (value instanceof Number) {
                const arr: number[] = [value];
                return buffer.from(arr);
            } else if (value instanceof Buffer) {
                return value;
            } else {
                return buffer.from(value);
            }
        }

        private compareSubarray(startIndex: number, searchBuffer: Buffer): boolean {
            if (startIndex + searchBuffer.length > this.length){
                return false;
            }
            for (let j: int = 0; j < searchBuffer.length; j++) {
                if (this.at(startIndex as int + j) != searchBuffer.at(j)) {
                    return false;
                }
            }
            return true;
        }

        /**
         Compares buf with target and returns a number indicating whether buf comes before, after,
         * or is the same as target in sort order. Comparison is based on the actual sequence of bytes in each Buffer.
         *
         * @param { Buffer | Uint8Array } target - A Buffer or Uint8Array with which to compare buf
         * @param { number } [targetStart] - The offset within target at which to begin comparison
         * @param { number } [targetEnd] - The offset within target at which to end comparison (not inclusive)
         * @param { number } [sourceStart] - The offset within buf at which to begin comparison
         * @param { number } [sourceEnd] - The offset within buf at which to end comparison (not inclusive)
         * @returns { number } 0 is returned if target is the same as buf
         *         1 is returned if target should come before buf when sorted.
         *        -1 is returned if target should come after buf when sorted.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         * 1. Mandatory parameters are left unspecified;
         * 2. Parameter verification failed.
         * @throws { BusinessError } 10200001 - The value of "[targetStart/targetEnd/sourceStart/sourceEnd]" is out of range.
         *         It must be >= 0 and <= [right range]. Received value is: [targetStart/targetEnd/sourceStart/sourceEnd]
         */
        public compare(
            target: Buffer | Uint8Array,
            targetStart?: number,
            targetEnd?: number,
            sourceStart?: number,
            sourceEnd?: number): number
        {
            const resolvedSource: ArrayBuffer = this.buffer;
            const resolvedTarget: ArrayBuffer = getArrayBufferFrom(target);
            const targetSlice = resolvedTarget.slice(targetStart ?? 0, targetEnd ?? resolvedTarget.byteLength);
            const sourceSlice = resolvedSource.slice(sourceStart ?? 0, sourceEnd ?? resolvedSource.byteLength);
            return compare(new Buffer(sourceSlice), new Buffer(targetSlice));
        }

        /**
         * Copies data from a region of buf to a region in target, even if the target memory region overlaps with buf.
         * If sourceEnd is greater than the length of the target, the length of the target shall prevail, and the extra part will not be overwritten.
         *
         * @param { Buffer | Uint8Array } target - A Buffer or Uint8Array to copy into
         * @param { number } [targetStart] - The offset within target at which to begin writing
         * @param { number } [sourceStart] - The offset within buf from which to begin copying
         * @param { number } [sourceEnd] - The offset within buf at which to stop copying (not inclusive)
         * @returns { number } The number of bytes copied
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         * 1. Mandatory parameters are left unspecified;
         * 2. Parameter verification failed.
         * @throws { BusinessError } 10200001 - The value of "[targetStart/sourceStart/sourceEnd]" is out of range. It must be >= 0.
         *                                    Received value is: [targetStart/sourceStart/sourceEnd]
         */
        public copy(
            target: Buffer | Uint8Array,
            targetStart?: number,
            sourceStart?: number,
            sourceEnd?: number): number
        {
            const resolvedTarget = getArrayBufferFrom(target);
            const resolvedSource = this.buffer;
            const resolvedTargetStart = (targetStart ?? 0) as int;
            const resolvedSourceStart = (sourceStart ?? 0) as int;
            const resolvedSourceEnd = (sourceEnd ?? this.length) as int;

            if (resolvedTargetStart < 0) {
                throw new BusinessError(`The value of "targetStart" is out of range. It must be >= 0.
Received value is: ${resolvedTargetStart}`, OutOfBoundsErrorCodeId);
            }
            if (resolvedSourceStart < 0) {
                throw new BusinessError(`The value of "sourceStart" is out of range. It must be >= 0.
Received value is: ${resolvedSourceStart}`, OutOfBoundsErrorCodeId);
            }
            if (resolvedSourceEnd < 0) {
                throw new BusinessError(`The value of "sourceEnd" is out of range. It must be >= 0.
Received value is: ${resolvedSourceEnd}`, OutOfBoundsErrorCodeId);
            }

            const sourceSlice = resolvedSource.slice(resolvedSourceStart,
                                                     resolvedSourceEnd);
            for (let i = 0; i < sourceSlice.byteLength; i++) {
                resolvedTarget.set(resolvedSourceStart + i, sourceSlice.at(i));
            }
            return sourceSlice.byteLength;
        }

        /**
         * Creates and returns an iterator of [index, byte] pairs from the contents of buf.
         *
         * @returns { IterableIterator<[number, number]> }
         */
        public entries(): IterableIterator<[number, number]> {
            return new BufferEntriesIterator(this);
        }

        /**
         * Returns true if both buf and otherBuffer have exactly the same bytes, false otherwise
         *
         * @param { Uint8Array | Buffer } otherBuffer - A Buffer or Uint8Array with which to compare buf
         * @returns { boolean } true or false
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         * 1. Mandatory parameters are left unspecified;
         * 2. Parameter verification failed.
         */
        public equals(otherBuffer: Uint8Array | Buffer): boolean {
            return this.compare(otherBuffer) == 0;
        }

        /**
         * Fills buf with the specified value. If the offset and end are not given, the entire buf will be filled.
         *
         * @param { string | Buffer | Uint8Array | number } value - The value with which to fill buf
         * @param { number } [offset] - Number of bytes to skip before starting to fill buf
         * @param { number } [end] - Where to stop filling buf (not inclusive)
         * @param { BufferEncoding } [encoding] - The encoding for value if value is a string
         * @returns { Buffer } A reference to buf
         * @throws { BusinessError } 10200001 - The value of "[offset/end]" is out of range. It must be >= 0 and <= [right range]. Received value is: [offset/end]
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         * 1. Mandatory parameters are left unspecified;
         * 2. Parameter verification failed.
         */
        public fill(
            value: string | Buffer | Uint8Array | number,
            offset: number = 0,
            end?: number,
            encoding?: BufferEncoding
        ): Buffer {
            const resolvedEnd = end ?? this.length;

            if (offset < 0) {
                throw new BusinessError(`The value of "offset" is out of range. It must be >= 0.
Received value is: ${offset}`, OutOfBoundsErrorCodeId);
            }
            if (resolvedEnd < 0 || resolvedEnd > this.length) {
                throw new BusinessError(`The value of "end" is out of range. It must be >= 0 and <= ${this.length}.
Received value is: ${resolvedEnd}`, OutOfBoundsErrorCodeId);
            }

            let klass: string = "string";
            if (value instanceof string) {
                klass = "string";
            } else if (value instanceof Number) {
                klass = "number";
            } else {
                // NOTE (templin.konstantin): Can't completely use smart casts due internal issue #21021
                //  now is used temporarily solution with reflection
                const objType = Type.of(value as (Buffer | Uint8Array));
                const parts: String[] = (objType as ClassType).getName().split('.');
                klass = parts[parts.length - 1];
            }

            const offsetInt = offset as int;
            const resolvedEndInt = resolvedEnd as int;
            const resolvedEncoding: BufferEncoding = encoding ?? "utf-8";

            switch (klass) {
                case "string":
                    const bytesSize = ArrayBuffer.bytesLength(value as string, resolvedEncoding as string);
                    let buff = ArrayBuffer.from(value as string,
                                                resolvedEncoding as string,
                                                bytesSize);
                    for (let i: int = offsetInt; i < resolvedEndInt; i++) {
                        this.buffer.set(i, buff.at((i % buff.byteLength) as int) as byte);
                    }
                    return this;
                case "number":
                    let asNum = value as number;
                    for (let i: int = offsetInt; i < resolvedEndInt; i++) {
                        this.buffer.set(i, asNum as byte);
                    }
                    return this;
                case "Buffer":
                    let asBuf = value as Buffer;
                    for (let i: int = offsetInt; i < resolvedEndInt; i++) {
                        this.buffer.set(i, asBuf.at((i % asBuf.length) as int));
                    }
                    return this;
                case "Uint8Array":
                    let asU8a = value as Uint8Array;
                    for (let i: int = offsetInt; i < resolvedEndInt; i++) {
                        this.buffer.set(i, asU8a.at(i % asU8a.length)! as byte);
                    }
                    return this;
                default:
                    throw new BusinessError(`Parameter error: value must be a string, Buffer, Uint8Array, or number.
Received value is: ${value}`, TypeErrorCodeId);
            }
        }

        /**
         * Creates and returns an iterator of buf keys (indices).
         *
         * @returns { IterableIterator<number> }
         */
        public keys(): IterableIterator<number> {
            return new BufferIteratorKeys(this);
        }

        /**
         * Creates and returns an iterator for buf values (bytes).
         *
         * @returns { IterableIterator<number> }
         */
        public values(): IterableIterator<number> {
            return new BufferIteratorValues(this);
        }

        /**
         * Returns a new Buffer that references the same memory as the original, but offset and cropped by the start and end indices.
         *
         * @param { number } [start] - Where the new Buffer will start
         * @param { number } [end] - Where the new Buffer will end (not inclusive)
         * @returns { Buffer } Returns a new Buffer that references the same memory as the original
         */
        public subarray(start?: number, end?: number): Buffer {
            const resolvedStart = start ?? 0 as number;
            const resolvedEnd = end ?? this.length;
            return new Buffer(this.buffer.slice(resolvedStart, resolvedEnd));
        }

        /**
         * Interprets buf as an array of unsigned 16-bit integers and swaps the byte order in-place.
         *
         * @returns { Buffer } A reference to buf
         * @throws { BusinessError } 10200009 - The buffer size must be a multiple of 16-bits
         */
        public swap16(): Buffer {
            if (this.length % 2 != 0) {
                throw new BusinessError('The buffer size must be a multiple of 16-bits', IncorrectBufferSizeId);
            }
            for (let i = 0; i < this.length; i += 2) {
                const a = this.buffer.at(i);
                this.buffer.set(i, this.buffer.at(i + 1));
                this.buffer.set(i + 1, a);
            }
            return this;
        }

        /**
         * Interprets buf as an array of unsigned 32-bit integers and swaps the byte order in-place.
         *
         * @returns { Buffer } A reference to buf
         * @throws { BusinessError } 10200009 - The buffer size must be a multiple of 32-bits
         */
        public swap32(): Buffer {
            if (this.length % 4 != 0) {
                throw new BusinessError('The buffer size must be a multiple of 32-bits', IncorrectBufferSizeId);
            }
            for (let i = 0; i < this.length; i += 4) {
                const a = this.buffer.at(i);
                const b = this.buffer.at(i + 1);
                this.buffer.set(i, this.buffer.at(i + 3));
                this.buffer.set(i + 1, this.buffer.at(i + 2));
                this.buffer.set(i + 2, b);
                this.buffer.set(i + 3, a);
            }
            return this;
        }

        /**
         * Interprets buf as an array of unsigned 64-bit integers and swaps the byte order in-place.
         *
         * @returns { Buffer } A reference to buf
         * @throws { BusinessError } 10200009 - The buffer size must be a multiple of 64-bits
         */
        public swap64(): Buffer {
            if (this.length % 8 != 0) {
                throw new BusinessError('The buffer size must be a multiple of 64-bits', IncorrectBufferSizeId);
            }
            for (let i = 0; i < this.length; i += 8) {
                const a = this.buffer.at(i);
                const b = this.buffer.at(i + 1);
                const c = this.buffer.at(i + 2);
                const d = this.buffer.at(i + 3);
                this.buffer.set(i, this.buffer.at(i + 7));
                this.buffer.set(i + 1, this.buffer.at(i + 6));
                this.buffer.set(i + 2, this.buffer.at(i + 5));
                this.buffer.set(i + 3, this.buffer.at(i + 4));
                this.buffer.set(i + 4, d);
                this.buffer.set(i + 5, c);
                this.buffer.set(i + 6, b);
                this.buffer.set(i + 7, a);
            }
            return this;
        }
    }
}
