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
    type TypedArrayOrigin = Int8Array | Uint8Array | Uint8ClampedArray | Int16Array | Uint16Array | Int32Array | Uint32Array | Float32Array | Float64Array

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
     * @throws {Error} If the input type is not supported
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
        throw new Error("Invalid type");
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
    }

    export interface BlobOptions {
        /** The Blob content-type. The intent is for type to convey the MIME media type of the data, however no validation of the type format is performed. */
        type: string
        /** One of either 'transparent' or 'native'. When set to 'native', line endings in string source parts will be converted to the platform native line-ending as specified by require('node:os').EOL.*/
        endings: 'transparent'|'native'
    };

    type SrcType = string[] | ArrayBuffer[] | TypedArray[] | DataView[] | Blob[]
    type SrcItemType = string | ArrayBuffer | TypedArray | DataView | Blob
    type ArrSrcType = Array<string> | Array<ArrayBuffer> | Array<TypedArray> | Array<DataView> | Array<Blob>

    function typeKey(sources: SrcType): string {
        if (sources instanceof string[]) {
            return "string"
        } else if (sources instanceof ArrayBuffer[]) {
            return "ArrayBuffer"
        } else if (sources instanceof TypedArray[]) {
            return "TypedArray"
        } else if (sources instanceof DataView[]) {
            return "DataView"
        } else if (sources instanceof Blob[]) {
            return "Blob"
        }
        return ""
    }

    function toString(src: SrcItemType): string {
        if (src instanceof string) {
            return src
        } else if (src instanceof ArrayBuffer) {
            return buffer.from(src as ArrayBuffer).toString()
        } else if (src instanceof TypedArray) {
            return (src as TypedArray).join('')
        } else if (src instanceof DataView) {
            return buffer.from((src as DataView).buffer as ArrayBuffer).toString()
        }
        return ""
    }

    /**
    * A Blob encapsulates immutable, raw data that can be safely shared across multiple worker threads.
    */
    export class Blob {
        /**
         * Creates a new Blob object containing a concatenation of the given sources.
         * <ArrayBuffer>, <TypedArray>, <DataView>, and <Buffer> sources are copied into the 'Blob' and can therefore be safely modified after the 'Blob' is created.
         * String sources are encoded as UTF-8 byte sequences and copied into the Blob. Unmatched surrogate pairs within each string part will be replaced by Unicode U+FFFD replacement characters.
         * @param {string[] | ArrayBuffer[] | TypedArray[] | DataView[] | Blob[]} [sources] - An array of string, <ArrayBuffer>, <TypedArray>, <DataView>, or <Blob> objects, or any mix of such objects, that will be stored within the Blob.
         * @param {BlobOptions} [options] - options
         */
        constructor(sources: string[] | ArrayBuffer[] | TypedArray[] | DataView[] | Blob[], options?: BlobOptions) {
            this.typeKey = typeKey(sources)
            this.createArray(sources)
            if (options != undefined) {
                this.type = options!.type ? options!.type! : ''
            } else {
                this.type = ''
            }
            this.calcSize()
        }

        private constructor(sources: ArrSrcType, typeKey: string, type?: string) {
            this.arr = sources
            this.typeKey = typeKey
            this.calcSize()
            if (type != undefined) {
                this.type = type
            } else {
                this.type = ''
            }
        }

        private constructor(blob: Blob) {
            this.arr = blob.arr
            this.size = blob.size
            this.type = blob.type
            this.typeKey = blob.typeKey
        }

        private calcSize(): number {
            this.size = 0
            switch (this.typeKey) {
                case "string": {
                    for (const item of this.arr as Array<string>) {
                        this.size += item.length
                    }
                } break
                case "ArrayBuffer": {
                    for (const item of this.arr as Array<ArrayBuffer>) {
                        this.size += item.getByteLength()
                    }
                } break
                case "TypedArray": {
                    for (const item of this.arr as Array<TypedArray>) {
                        this.size += item.byteLength
                    }
                } break
                case "DataView": {
                    for (const item of this.arr as Array<DataView>) {
                        this.size += item.byteLength
                    }
                } break
                case "Blob": {
                    for (const item of this.arr as Array<Blob>) {
                        this.size += item.calcSize()
                    }
                } break
            }
            return this.size
        }

        private createArray(sources: SrcType) {
            switch (this.typeKey) {
                case "string": {
                    this.arr = Array.from(sources as string[])
                } break
                case "ArrayBuffer": {
                    this.arr = Array.from(sources as ArrayBuffer[])
                } break
                case "TypedArray": {
                    this.arr = Array.from(sources as TypedArray[])
                } break
                case "DataView": {
                    this.arr = Array.from(sources as DataView[])
                } break
                case "Blob": {
                    this.arr = Array.from(sources as Blob[])
                } break
            }
        }

        /**
         * Creates and returns a new Blob containing a subset of this Blob objects data. The original Blob is not altered.
         * 
         * @param {number} [start=0] - The starting index.
         * @param {number} [end=buffer.length] -  The ending index.
         * @param {string} [type] - The content-type for the new Blob
         * @returns {Blob}
         */
        slice(start?: number, end?: number, type?: string): Blob {
            switch (this.typeKey) {
                case "string": {
                    let arr: Array<string> = (this.arr as Array<string>).slice(start, end)
                    return new Blob(arr, this.typeKey, type)
                }
                case "ArrayBuffer": {
                    let arr: Array<ArrayBuffer> = (this.arr as Array<ArrayBuffer>).slice(start, end)
                    return new Blob(arr, this.typeKey, type)
                }
                case "TypedArray": {
                    let arr: Array<TypedArray> = (this.arr as Array<TypedArray>).slice(start, end)
                    return new Blob(arr, this.typeKey, type)
                }
                case "DataView": {
                    let arr: Array<DataView> = (this.arr as Array<DataView>).slice(start, end)
                    return new Blob(arr, this.typeKey, type)
                }
                case "Blob": {
                    let arr: Array<Blob> = (this.arr as Array<Blob>).slice(start, end)
                    return new Blob(arr, this.typeKey, type)
                }
            }
            return new Blob(this)
        }

        /**
         * Returns a promise that fulfills with the contents of the Blob decoded as a UTF-8 string.
         * 
         * @returns {Promise}
         */
        text(): Promise<string> {
            return Promise.resolve(this.getString())
        }

        /**
         * Returns a promise that fulfills with an <ArrayBuffer> containing a copy of the Blob data.
         * 
         * @returns {Promise}
         */
        arrayBuffer(): Promise<ArrayBuffer> {
            return Promise.resolve(this.toArrayBuffer())
        }

        size: number 

        /**
         * The content-type of the Blob.
         */
        type: string

        private arr: ArrSrcType
        private typeKey: string = 'string'

        private toArrayBuffer(): ArrayBuffer {
            const str = this.getString()
            return ArrayBuffer.from(str, "utf-8", ArrayBuffer.bytesLength(str, "utf-8"))
        }

        private getString(): string {
            let str = ''
            let b = new StringBuilder()
            switch (this.typeKey) {
                case "string": {
                    (this.arr as Array<string>).forEach((value: string) => { b.append(toString(value as SrcItemType)) })
                } break
                case "ArrayBuffer": {
                    (this.arr as Array<ArrayBuffer>).forEach((value: ArrayBuffer) => { b.append(toString(value)) })
                } break
                case "TypedArray": {
                    (this.arr as Array<TypedArray>).forEach((value: TypedArray) => { b.append(toString(value)) })
                } break
                case "DataView": {
                    (this.arr as Array<DataView>).forEach((value: DataView) => { b.append(toString(value)) })
                } break
                case "Blob": {
                    (this.arr as Array<Blob>).forEach((value: Blob) => { b.append(value.getString()) })
                } break
            }
            return b.toString()
        }
    }
}

