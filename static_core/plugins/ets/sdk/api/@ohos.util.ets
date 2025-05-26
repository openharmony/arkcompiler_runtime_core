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

import { BusinessError } from "@ohos.base";

const TypeErrorCodeId: number = 401;
const percentRate = 100;

function createBusinessError(code: number, message: string) {
    let err = new BusinessError();
    err.code = code;
    err.name = 'BusinessError';
    err.message = message;
    return err;
}

export namespace util {

    export class LRUCache<K, V> {
        private cache: Map<K, V>;
        // Default current size
        private maxSize: number;
        // Default maximum size
        private putCount: long = 0;
        private createCount: long = 0;
        private evictionCount: long = 0;
        private hitCount: long = 0;
        private missCount: long = 0;

        get length(): number {
            return this.cache.size;
        }

        public constructor(capacity?: number) {
            if (capacity == undefined) {
                this.maxSize = 64;
            } else {
                if (this.checkCapacityIsIllegal(capacity)) {
                    throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of ${capacity} must be small integer`)
                }
                this.maxSize = capacity;
            }
            this.cache = new Map<K, V>();
        }

        private checkCapacityIsIllegal(capacity: number): boolean {
            return (capacity > Int.MAX_VALUE || capacity <= 0 || (capacity % 1) != 0);
        }

        private getLRUKey(): K {
            return this.cache.keys().next().value as K;
        }

        private getLRUValue(): V {
            return this.cache.values().next().value as V;
        }

        private reduceCapacity(newCapacity: number): void {
            while (this.cache.keys().next() && this.cache.size > newCapacity) {
                let lruKey = this.getLRUKey();
                this.afterRemoval(true, lruKey, this.getLRUValue(), undefined);
                this.cache.delete(lruKey);
                this.evictionCount++;
            }
        }

        protected afterRemoval(isEvict: boolean, key: K | undefined, value: V | undefined,
            newValue: V | undefined): void {
        }

        public updateCapacity(newCapacity: number): void {
            if (this.checkCapacityIsIllegal(newCapacity)) {
                throw createBusinessError(TypeErrorCodeId, `Parameter error. The type of ${newCapacity} must be small integer`)
            } else if (this.cache.size > newCapacity) {
                this.reduceCapacity(newCapacity);
            }
            this.maxSize = newCapacity;
        }

        public getCapacity(): number {
            return this.maxSize;
        }

        public clear(): void {
            this.cache.forEach((v, k) => {
                this.afterRemoval(false, k, v, undefined);
            })
            this.cache.clear();
        }

        public isEmpty(): boolean {
            return (this.cache.size == 0);
        }

        public put(key: K, value: V): V | undefined {
            let valueBefore: V | undefined = undefined;
            this.putCount++;
            if (this.cache.has(key)) {
                valueBefore = this.cache.get(key);
                this.cache.delete(key);
                this.afterRemoval(false, key, valueBefore, value);
            }
            this.cache.set(key, value);
            if (this.cache.size > this.maxSize) {
                this.reduceCapacity(this.maxSize);
            }
            return valueBefore;
        }

        override toString(): String {
            let peek: number = 0;
            let hitRate: number = 0;
            peek = this.hitCount + this.missCount;
            if (peek != 0) {
                // The value is 100 times larger
                hitRate = percentRate * this.hitCount / peek;
            }
            return 'LRUCache[ maxSize = ' + this.maxSize + ', hits = ' + this.hitCount +
                ', misses = ' + this.missCount + ', hitRate = ' + hitRate + '% ]';
        }

        public getCreateCount(): number {
            return this.createCount as number;
        }

        public getMissCount(): number {
            return this.missCount as number;
        }

        public getRemovalCount(): number {
            return this.evictionCount as number;
        }

        public getMatchCount(): number {
            return this.hitCount as number;
        }

        public getPutCount(): number {
            return this.putCount as number;
        }

        public $_iterator(): IterableIterator<[K, V]> {
            return this.cache.entries();
        }

        public entries(): IterableIterator<[K, V]> {
            return this.cache.entries();
        }

        public keys(): Array<K> {
            let arr = Array.from<K>(this.cache.keys());
            return arr;
        }

        public values(): Array<V> {
            let arr = Array.from<V>(this.cache.values());
            return arr;
        }

        public remove(key: K): V | undefined {
            if (this.cache.has(key)) {
                let former: V | undefined = this.cache.get(key);
                this.cache.delete(key);
                if (former != undefined) {
                    this.afterRemoval(false, key, former, undefined);
                    return former;
                }
            }
            return undefined;
        }

        public get(key: K): V | undefined {
            if (this.cache.has(key)) {
                return this.containsInner(key);
            }
            this.missCount++;
            let createValue: V | undefined = this.createDefault(key);
            if (createValue == undefined) {
                return undefined;
            } else {
                this.put(key, createValue);
                this.createCount++;
                return createValue;
            }

        }

        protected createDefault(key: K): V | undefined {
            return undefined;
        }

        private containsInner(key: K): V | undefined {
            this.hitCount++;
            let value: V | undefined = this.cache.get(key);
            this.cache.delete(key);
            this.cache.set(key, value as V);
            return value;
        }

        public contains(key: K): boolean {
            if (this.cache.has(key)) {
                this.containsInner(key);
                return true;
            }
            this.missCount++;
            return false;
        }
    }

    export enum Type {
        BASIC,
        MIME,
        BASIC_URL_SAFE,
        MIME_URL_SAFE,
    }

    export class Base64Helper {
        private static BASE64_CHARS: string = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        private static BASE64_URL_SAFE_CHARS: string = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

        private static readonly BYTE_SIZE = 8;
        private static readonly BASE64_GROUP_SIZE = 6;
        private static readonly BASE64_MASK = 0x3F;
        private static readonly LINE_BREAK_FREQUENCY = 76;

        constructor() { }

        /**
         * Encodes the given byte array into a Base64 string.
         * @param bytes The byte array to encode.
         * @param urlSafe If true, uses the URL-safe Base64 alphabet.
         * @param mime If true, inserts line breaks according to MIME specifications.
         * @returns The Base64 encoded string.
         */
        private static encodeBase64(bytes: Uint8Array, urlSafe: boolean, mime: boolean): string {
            let table = urlSafe ? Base64Helper.BASE64_URL_SAFE_CHARS : Base64Helper.BASE64_CHARS;
            let binaryString: string = "";
            for (let i = 0; i < bytes.length; i++) {
                binaryString += String.fromCharCode(bytes[i]);
            }
            let encoded: string = "";
            let buffer: int = 0;
            let bufferLength: int = 0;

            for (let i = 0; i < binaryString.length; i++) {
                buffer = (buffer << Base64Helper.BYTE_SIZE) | binaryString.charCodeAt(i);
                bufferLength += Base64Helper.BYTE_SIZE;
                while (bufferLength >= Base64Helper.BASE64_GROUP_SIZE) {
                    bufferLength -= Base64Helper.BASE64_GROUP_SIZE;
                    encoded += table[(buffer >> bufferLength) & Base64Helper.BASE64_MASK];
                }
            }

            if (bufferLength > 0) {
                encoded += table[(buffer << (Base64Helper.BASE64_GROUP_SIZE - bufferLength)) & Base64Helper.BASE64_MASK];
            }

            while (encoded.length % 4 !== 0 && !urlSafe) {
                encoded += "=";
            }

            if (mime) {
                encoded = encoded.replace(new RegExp(`(.{${Base64Helper.LINE_BREAK_FREQUENCY}})`, "g"), "$1\r\n");
                if (!encoded.endsWith("\r\n")) {
                    encoded += "\r\n";
                }
            }
            return encoded;
        }

        /**
         * Decodes the given Base64 string into a byte array.
         * @param base64 The Base64 string to decode.
         * @param urlSafe If true, uses the URL-safe Base64 alphabet.
         * @returns A Uint8Array containing the decoded bytes.
         */
        private static decodeBase64(base64: string, urlSafe: boolean): Uint8Array {
            let table = urlSafe ? Base64Helper.BASE64_URL_SAFE_CHARS : Base64Helper.BASE64_CHARS;
            let allowedPattern = urlSafe ? "[^A-Za-z0-9\\-_=]" : "[^A-Za-z0-9+/=]";
            let cleaned: string = base64.replace(new RegExp(allowedPattern, "g"), "");
            let buffer: int = 0;
            let bufferLength: int = 0;
            let bytes: Array<number> = new Array<number>();

            for (let i = 0; i < cleaned.length; i++) {
                let ch: string = cleaned[i];
                if (ch == "=") {
                    break;
                }
                let index: int = table.indexOf(ch) as int;
                if (index == -1) {
                    continue;
                }
                buffer = (buffer << Base64Helper.BASE64_GROUP_SIZE) | index;
                bufferLength += Base64Helper.BASE64_GROUP_SIZE;
                if (bufferLength >= Base64Helper.BYTE_SIZE) {
                    bufferLength -= Base64Helper.BYTE_SIZE;
                    bytes.push((buffer >> bufferLength) & 0xFF);
                }
            }
            return new Uint8Array(bytes);
        }

        /**
         * Synchronously encodes the input byte array into a Base64 encoded Uint8Array.
         * @param src The byte array to encode.
         * @param options The encoding options (Base64 type).
         * @returns A Uint8Array containing the Base64 encoded string.
         */
        encodeSync(src: Uint8Array, options: Type = Type.BASIC): Uint8Array {
            let encoded: string = Base64Helper.encodeBase64(src, options === Type.BASIC_URL_SAFE || options === Type.MIME_URL_SAFE, options === Type.MIME || options === Type.MIME_URL_SAFE);
            let result: Uint8Array = new Uint8Array(encoded.length);
            for (let i: int = 0; i < encoded.length; i++) {
                result[i] = encoded.charCodeAt(i);
            }
            return result;
        }

        /**
         * Synchronously encodes the input byte array into a Base64 encoded string.
         * @param src The byte array to encode.
         * @param options The encoding options (Base64 type).
         * @returns A string containing the Base64 encoded result.
         */
        encodeToStringSync(src: Uint8Array, options: Type = Type.BASIC): string {
            return Base64Helper.encodeBase64(src, options === Type.BASIC_URL_SAFE || options === Type.MIME_URL_SAFE, options === Type.MIME || options === Type.MIME_URL_SAFE);
        }

        /**
         * Synchronously decodes a Base64 encoded Uint8Array into a byte array.
         * @param src A Uint8Array containing the Base64 encoded string.
         * @param options The decoding options (Base64 type).
         * @returns A Uint8Array with the decoded bytes.
         */
        decodeSync(src: Uint8Array, options: Type = Type.BASIC): Uint8Array {
            let base64String: string;
            let charArray: Array<string> = new Array<string>();
            for (let i: int = 0; i < src.length; i++) {
                charArray.push(String.fromCharCode(src[i]));
            }
            base64String = charArray.join("");
            if (options === Type.MIME || options === Type.MIME_URL_SAFE) {
                base64String = base64String.replace(new RegExp("[\\r\\n]", "g"), "");
            }
            return Base64Helper.decodeBase64(base64String, options === Type.BASIC_URL_SAFE || options === Type.MIME_URL_SAFE);
        }

        /**
        * Synchronously decodes a Base64 encoded string into a byte array.
        * @param src The Base64 encoded string.
        * @param options The decoding options (Base64 type).
        * @returns A Uint8Array with the decoded bytes.
        */
        decodeSync(src: string, options: Type = Type.BASIC): Uint8Array {
            let newString: string = src;
            if (options === Type.MIME || options === Type.MIME_URL_SAFE) {
                newString = src.replace(new RegExp("[\\r\\n]", "g"), "");
            }
            return Base64Helper.decodeBase64(newString, options === Type.BASIC_URL_SAFE || options === Type.MIME_URL_SAFE);
        }

        /**
         * Asynchronously encodes the input byte array into a Base64 encoded Uint8Array.
         * @param src The byte array to encode.
         * @param options The encoding options (Base64 type).
         * @returns A Promise that resolves to a Uint8Array containing the Base64 encoded string.
         */
        encode(src: Uint8Array, options: Type = Type.BASIC): Promise<Uint8Array> {
            return Promise.resolve(this.encodeSync(src, options));
        }

        /**
         * Asynchronously decodes a Base64 encoded string into a byte array.
         * @param src The Base64 encoded string.
         * @param options The decoding options (Base64 type).
         * @returns A Promise that resolves to a Uint8Array with the decoded bytes.
         */
        decode(src: string, options: Type = Type.BASIC): Promise<Uint8Array> {
            return Promise.resolve(this.decodeSync(src, options));
        }

        /**
         * Asynchronously decodes a Base64 encoded Uint8Array into a byte array.
         * @param src A Uint8Array containing the Base64 encoded string.
         * @param options The decoding options (Base64 type).
         * @returns A Promise that resolves to a Uint8Array with the decoded bytes.
         */
        decode(src: Uint8Array, options: Type = Type.BASIC): Promise<Uint8Array> {
            return Promise.resolve(this.decodeSync(src, options));
        }

        /**
         * Asynchronously encodes the input byte array into a Base64 encoded string.
         * @param src The byte array to encode.
         * @param options The encoding options (Base64 type).
         * @returns A Promise that resolves to a string containing the Base64 encoded result.
         */
        encodeToString(src: Uint8Array, options: Type = Type.BASIC): Promise<string> {
            return Promise.resolve(this.encodeToStringSync(src, options));
        }
    }

    /**
    * The util.format() method returns a formatted string using the first argument as a printf-like
    * format string which can contain zero or more format specifiers.
    * Each specifier is replaced with the converted value from the corresponding argument.
    * Supported specifiers are:
    * %s: String will be used to convert all values except BigInt, Object and -0. BigInt values will be represented
    *     with an n and Objects that have no user defined toString function are inspected using util.inspect()
    *     with options { depth: 0, colors: false, compact: 3 }.
    * %d: Number will be used to convert all values except BigInt and Char.
    * %i: parseInt(value, 10) is used for all values except BigInt and Char.
    * %f: parseFloat(value) is used for all values expect Char.
    * %j: JSON. Replaced with the string '[Circular]' if the argument contains circular references.
    * %o: Object. A string representation of an object with generic JavaScript object formatting.
    *     Similar to util.inspect() with options { showHidden: true, showProxy: true }.
    *     This will show the full object including non-enumerable properties and proxies.
    * %O: Object. A string representation of an object with generic JavaScript object formatting.
    *     Similar to util.inspect() without options. This will show the full object not including non-enumerable
    *     properties and proxies.
    * %c: CSS. This specifier is ignored and will skip any CSS passed in.
    * %%: single percent sign ('%'). This does not consume an argument.
    *
    * @param s: <string> A printf-like format string.
    *
    * @param args: <Object[]> format params
    *
    * @returns Returns: <string> The formatted string
    */
    export function format(s: string, ...args: Object[]): string {
        let preI: int = 0
        let i: int = 0
        let argCounter = 0
        const flags: string = "cdfijoOs"
        let res = new StringBuilder
        let ss = ""
        if (args.length == 0) {
            return s.replaceAll("%%", "%")
        }

        while ((i = (ss = s.substring(preI, s.length)).search("%") as int) != -1 && (preI + i < s.length - 1) && (argCounter < args.length)) {
            res.append(s.substring(preI, preI + i))
            let flagIndex = preI + i + 1
            //NOTE(ivan-tyulyandin): has to be string due to internal issue 21459
            let flag: string = s[flagIndex]
            if (flags.includes(flag)) {
                res.append(UtilHelper.processArg(flag, args[argCounter]))
                ++argCounter
            } else if (flag == "%") {
                res.append("%")
            } else {
                res.append("%" + flag)
            }
            preI += i + 2
        }
        if (preI < s.length) {
            res.append(s.substring(preI, s.length))
        }
        let strArgsList = UtilHelper.joinLastArgs(argCounter, ...args)
        let resStr = res.toString()
        return resStr.length == 0 ? res + strArgsList.slice(1) : res + strArgsList
    }

    export function generateRandomUUID(entropyCache?: boolean): string {
        let s = UtilHelper.generateRandomUUID((entropyCache == undefined) ? false : entropyCache)
        return s
    }

    export interface ScopeComparable<T> {

        /**
        * The comparison function is used by the scope.
        *
        * @param { ScopeComparable } other - Other
        * @returns { boolean } Returns whether the current object is greater than or equal to the input object.
        */
        compareTo(other: T): boolean;
    }

    /**
    * A type used to denote ScopeComparable or number.
    *
    * @typedef { ScopeComparable }
    */
    export type ScopeType<T> = ScopeComparable<T>;

    export class ScopeHelper<T extends ScopeComparable<T>> {
        private readonly _lowerLimit: T
        private readonly _upperLimit: T

        /**
        * A constructor used to create a Scope instance with the lower and upper bounds specified.
        *
        * @param { ScopeType } lowerObj - A ScopeType value
        * @param { ScopeType } upperObj - A ScopeType value
        */
        constructor(lowerObj: T, upperObj: T) {
            if (lowerObj.compareTo(upperObj)) {
                throw new Error('lower limit must be less than upper limit');
            }
            this._lowerLimit = lowerObj
            this._upperLimit = upperObj
        }

        /**
        * Obtains a string representation of the current range.
        *
        * @returns { string } Returns a string representation of the current range object.
        */
        toString(): string {
            let strLower: string = this._lowerLimit.toString()
            let strUpper: string = this._upperLimit.toString()
            return '[' + strLower + ', ' + strUpper + ']'
        }

        /**
        * Returns the intersection of a given range and the current range.
        *
        * @param { ScopeHelper } range - A Scope range object
        * @returns { ScopeHelper } Returns the intersection of a given range and the current range.
        */
        intersect(range: ScopeHelper<T>): ScopeHelper<T> {
            let lowerLimit = this._lowerLimit.compareTo(range.getLower()) ? this._lowerLimit : range.getLower()
            let upperLimit = this._upperLimit.compareTo(range.getUpper()) ? range.getUpper() : this._upperLimit
            return new ScopeHelper<T>(lowerLimit, upperLimit)
        }

        /**
        * Returns the intersection of the current range and the range specified by the given lower and upper bounds.
        *
        * @param { ScopeType } lowerObj - A ScopeType value
        * @param { ScopeType } upperObj - A ScopeType value
        * @returns { ScopeHelper } Returns the intersection of the current range and the range specified by the given lower and upper bounds.
        */
        intersect(lowerObj: T, upperObj: T): ScopeHelper<T> {
            let lowerLimit = this._lowerLimit.compareTo(lowerObj) ? this._lowerLimit : lowerObj
            let upperLimit = this._upperLimit.compareTo(upperObj) ? upperObj : this._upperLimit
            return new ScopeHelper<T>(lowerLimit, upperLimit)
        }

        /**
        * Obtains the upper bound of the current range.
        *
        * @returns { ScopeType } Returns the upper bound of the current range.
        */
        getUpper(): T {
            return this._upperLimit
        }

        /**
        * Obtains the lower bound of the current range.
        *
        * @returns { ScopeType } Returns the lower bound of the current range.
        */
        getLower(): T {
            return this._lowerLimit
        }

        /**
        * Creates the smallest range that includes the current range and the given lower and upper bounds.
        *
        * @param { ScopeType } lowerObj - A ScopeType value
        * @param { ScopeType } upperObj - A ScopeType value
        * @returns { ScopeHelper } Returns the smallest range that includes the current range and the given lower and upper bounds.
        */
        expand(lowerObj: T, upperObj: T): ScopeHelper<T> {
            let lowerLimit = this._lowerLimit.compareTo(lowerObj) ? lowerObj : this._lowerLimit
            let upperLimit = this._upperLimit.compareTo(upperObj) ? this._upperLimit : upperObj
            return new ScopeHelper<T>(lowerLimit, upperLimit)
        }

        /**
        * Creates the smallest range that includes the current range and a given range.
        *
        * @param { ScopeHelper } range - A Scope range object
        * @returns { ScopeHelper } Returns the smallest range that includes the current range and a given range.
        */
        /**
        * Creates the smallest range that includes the current range and a given range.
        *
        * @param { ScopeHelper } range - A Scope range object
        * @returns { ScopeHelper } Returns the smallest range that includes the current range and a given range.
        */
        expand(range: ScopeHelper<T>): ScopeHelper<T> {
            let lowerLimit = this._lowerLimit.compareTo(range.getLower()) ? range.getLower() : this._lowerLimit
            let upperLimit = this._upperLimit.compareTo(range.getUpper()) ? this._upperLimit : range.getUpper()
            return new ScopeHelper<T>(lowerLimit, upperLimit)
        }

        /**
        * Creates the smallest range that includes the current range and a given value.
        *
        * @param { ScopeType } value - A ScopeType value
        * @returns { ScopeHelper } Returns the smallest range that includes the current range and a given value.
        */
        expand(value: T): ScopeHelper<T> {
            let lowerLimit = this._lowerLimit.compareTo(value) ? value : this._lowerLimit
            let upperLimit = this._upperLimit.compareTo(value) ? this._upperLimit : value
            return new ScopeHelper<T>(lowerLimit, upperLimit)
        }

        /**
        * Checks whether a given value is within the current range.
        *
        * @param { ScopeType } value - A ScopeType value
        * @returns { boolean } If the value is within the current range return true,otherwise return false.
        */
        contains(value: T): boolean {
            let lowerRes = value.compareTo(this._lowerLimit)
            let upperRes = this._upperLimit.compareTo(value)
            return lowerRes && upperRes
        }

        /**
        * Checks whether a given range is within the current range.
        *
        * @param { ScopeHelper } range - A Scope range
        * @returns { boolean } If the current range is within the given range return true,otherwise return false.
        */
        contains(range: ScopeHelper<T>): boolean {
            let lowerRes = range.getLower().compareTo(this._lowerLimit)
            let upperRes = this._upperLimit.compareTo(range.getUpper())
            return lowerRes && upperRes
        }

        /**
        * Clamps a given value to the current range.
        *
        * @param { ScopeType } value - A ScopeType value
        * @returns { ScopeType } Returns a ScopeType object that a given value is clamped to the current range.
        */
        clamp(value: T): T {
            if (!value.compareTo(this._lowerLimit)) {
                return this._lowerLimit
            } else if (value.compareTo(this._upperLimit)) {
                return this._upperLimit
            } else {
                return value
            }
        }
    }
    
    export interface TextDecoderOptions {
        fatal?: boolean;
        ignoreBOM?: boolean;
    }

    export interface DecodeToStringOptions {
        stream?: boolean;
    }
    export class TextDecoder {
        // Use static properties to manage Finalization Registry and avoid module level exports
        private static destroyRegistry = new FinalizationRegistry<long>(
            (nativePtr: long) => TextDecoder.nativeDestroy(nativePtr)
        );
        private static unregisterToken = new object();

        static { loadLibrary("ets_sdk_native") }

        private encoding_: string = "utf-8";
        private fatal_: boolean = false;
        ignoreBOM_: boolean = false;
        const CONVERTER_FLAGS_FATAL = 0x2;
        const CONVERTER_FLAGS_IGNORE_BOM = 0x4;
        private nativeDecoder_: long;

        constructor(encoding?: string, options?: TextDecoderOptions) {
            let flags = 0;
            if (encoding) {
                this.encoding_ = encoding;
            }
            if (options) {
                flags |= options.fatal ? this.CONVERTER_FLAGS_FATAL : 0;
                flags |= options.ignoreBOM ? this.CONVERTER_FLAGS_IGNORE_BOM : 0;
                this.fatal_ = options.fatal ?? false;
                this.ignoreBOM_ = options.ignoreBOM ?? false;
            }
            this.bindNativeDecoder(this.encoding_, flags);
            TextDecoder.destroyRegistry.register(this, this.nativeDecoder_, TextDecoder.unregisterToken);
        }

        static create(encoding?: string, options?: TextDecoderOptions): TextDecoder {
            return new TextDecoder(encoding, options);
        }

        public decodeToString(input: Uint8Array, options?: DecodeToStringOptions): string {
            if (input.length === 0) {
                return '';
            }
            return this.decode(input, options?.stream ?? false);
        }

        get encoding(): string {
            return this.encoding_;
        }

        get fatal(): boolean {
            return this.fatal_;
        }

        get ignoreBOM(): boolean {
            return this.ignoreBOM_;
        }

        private native bindNativeDecoder(encoding: string, flags: int): void;
        private native decode(input: Uint8Array, decodeWithStream: boolean): string;
        private static native nativeDestroy(ptr: long): void;
    }

    export interface EncodeIntoUint8ArrayInfo {
        read: int;
        written: int;
    }

    class EncodeIntoUint8ArrayInfoInner implements EncodeIntoUint8ArrayInfo {
        read: int;
        written: int;
        constructor(outRead: int, outWritten: int) {
            this.read = outRead;
            this.written = outWritten;
        }
    }

    export class TextEncoder {

        static { loadLibrary("ets_sdk_native") }

        private encoding_: string = "utf-8";

        constructor(encoding?: string)
        {
            if (!encoding) {
                this.encoding_ = "utf-8";
                return;
            }
            let tempString: string = encoding.toLowerCase();
            let flag: boolean = TextEncoder.checkEncodingFormat(tempString);
            if (!flag) {
                throw createBusinessError(TypeErrorCodeId, `Wrong encoding format, the current '${encoding}' format is not support.`)
            }
            this.encoding_ = encoding;
        }

        static create(encoding?: string): TextEncoder
        {
            return new TextEncoder(encoding);
        }

        encodeInto(input?: string): Uint8Array
        {
            if (!input) {
                return new Uint8Array(0);
            }
            return TextEncoder.doEncodeInto(input, this.encoding_);
        }

        encodeIntoUint8Array(input: string, dest: Uint8Array): EncodeIntoUint8ArrayInfo
        {
            return TextEncoder.doEncodeInfoUint8Array(input, this.encoding_, dest);
        }

        get encoding(): string {
            return this.encoding_;
        }

        private static checkEncodingFormat(encoding: string): boolean
        {
            const knownEncodings: string[] = [
                "utf-8", "utf-16be", "utf-16le", "gbk", "gb2312", "gb18030", "ibm866",
                "iso-8859-1", "iso-8859-2", "iso-8859-3", "iso-8859-4", "iso-8859-5", "iso-8859-6",
                "iso-8859-7", "iso-8859-8", "iso-8859-8-i", "iso-8859-10", "iso-8859-13",
                "iso-8859-14", "iso-8859-15", "koi8-r", "koi8-u", "macintosh",
                "windows-874", "windows-1250", "windows-1251", "windows-1252",
                "windows-1253", "windows-1254", "windows-1255", "windows-1256",
                "windows-1257", "windows-1258", "big5", "euc-jp", "iso-2022-jp",
                "shift_jis", "euc-kr", "x-mac-cyrillic"
            ];
            for (const enc of knownEncodings) {
                if (enc == encoding) {
                    return true;
                }
            }
            return false;
        }

        private static native doEncodeInto(
            input: string, inputEncoding: string): Uint8Array;

        private static native doEncodeInfoUint8Array(
            input: string, inputEncoding: string, destArray: Uint8Array): EncodeIntoUint8ArrayInfo;
    }

    export class UtilHelper {

        static {
            loadLibrary("ets_sdk_native")
        }

        static native generateRandomUUID(entropyCache: boolean): string

    internal static objectToString(obj: Object, full: boolean = false): string {
            return (new UtilHelper).stringifyObject(obj, '\n', 1, full)
        }

        private static isNullObject(obj: NullishType): boolean {
            return (obj == undefined) || (obj == null)
        }

        private static isFunction(obj: Object): boolean {
            return (typeof obj === 'function')
        }

        private static isObject(obj: Object): boolean {
            return (typeof obj === 'object')
        }

        private static isString(obj: Object): boolean {
            return (typeof obj === 'string')
        }

        private static isArray(obj: Object): boolean {
            return obj instanceof Array
        }

        private stringifySubObject(delimiter: string, subObj: NullishType, subObjName: string, depth: number, full: boolean = false): string {

            if (UtilHelper.isNullObject(subObj)) {
                return UtilHelper.nullObjectDescription(subObjName) + ',' + delimiter
            }

            let strB = new StringBuilder

            if (UtilHelper.isArray(subObj!)) {
                strB.append(subObjName + ':' + delimiter + UtilHelper.arrayToString(delimiter + '  ', subObj! as Array<Object>, depth, full))
            } else if (UtilHelper.isObject(subObj!)) {
                strB.append(this.subObjectDescription(subObj!, subObjName, this.isCircular(subObj!), delimiter, depth))
            } else if (UtilHelper.isFunction(subObj!)) {
                strB.append(UtilHelper.functionDescription(subObj!, delimiter, true, full))
            } else {
                strB.append(UtilHelper.simpleObjectDescription(subObj!, subObjName, true))
            }
            this.objects.add(subObj!)

            strB.append(',' + delimiter)
            return strB.toString()
        }

        private objectDescription(obj: Object, delimiter: string = '\n', depth: number = 1, full: boolean = false): string {

            let strB = new StringBuilder('{ ')
            let isComplexObject: boolean = false

            for (const objEntry of Object.entries(obj)) {
                if (!objEntry) {
                    continue
                }
                isComplexObject = true
                strB.append(this.stringifySubObject(delimiter, objEntry[1], objEntry[0], depth, full))
            }

            if (!isComplexObject) {
                return JSON.stringify(obj)
            }

            return strB.toString().slice(0, - delimiter.length - 1) + ' }'
        }

        private subObjectDescription(obj: Object, objName: string, isCircular: boolean, delimiter: string, depth: number): string {
            if (isCircular) {
                return objName + ': [Circular]' + delimiter
            }
            return objName + ':' + delimiter + this.stringifyObject(obj, delimiter + '  ', depth + 1, true)
        }

        private objects = new Set<Object>

        private isCircular(obj: NullishType): boolean {
            if (obj == null) {
                return false
            }

            return this.objects.has(obj)
        }

        private static nullObjectDescription(objName: string = ''): string {
            return ((objName != '') ? objName + ': ' : '') + null
        }

        private static simpleObjectDescription(obj: Object, objName: string = '', isSub: boolean = false): string {
            let quotesStr = UtilHelper.isString(obj) ? '\'' : ''
            return (isSub ? objName + ': ' : '') + quotesStr + obj + quotesStr
        }

        private stringifyObject(obj: NullishType, delimiter: string = '\n', depth: number = 1, full: boolean = false): string {

            if (UtilHelper.isNullObject(obj)) {
                return UtilHelper.nullObjectDescription()
            }

            this.objects.add(obj!)
            let strB = new StringBuilder
            if (UtilHelper.isArray(obj!)) {
                strB.append(UtilHelper.arrayToString(delimiter, obj! as Array<Object>, depth, full))
            } else if (UtilHelper.isFunction(obj!)) {
                strB.append(UtilHelper.functionDescription(obj!, delimiter, false, full))
                if (!full) {
                    strB = (new StringBuilder).append('{ ').append(strB.toString()).append(' }')
                }
            } else if (UtilHelper.isObject(obj!)) {
                strB.append(this.objectDescription(obj!, delimiter, depth, full))
            } else {
                strB.append(UtilHelper.simpleObjectDescription(obj!, ''))
            }
            return strB.toString()
        }

        private static arrayToString(delimiter: string, arr: Array<Object>, depth: number, full: boolean = false): string {
            return ('[ ' + UtilHelper.arrayToStringImpl(delimiter, arr, depth, full) + (full ? ('[length]: ' + arr.length) : '') + ' ]')
        }

        private static functionDescription(obj: Object, delimiter: string = '\n', isSub: boolean = false, full: boolean = false, inArray: boolean = false): string {
            let strB = new StringBuilder
            let functionName = UtilHelper.getName(obj)
            strB.append((isSub && functionName !== '') ? (functionName + ':' + delimiter) : '');

            let baseFunctionStr = '[Function: ' + functionName + ']'

            if (full) {
                let end = delimiter + (inArray || isSub) ? '  ' : '';
                let length = UtilHelper.getLength(obj)
                strB.append('{ ' + baseFunctionStr + end +
                    '[length]: ' + (length ? length : 0) + ',' + end +
                    '[name] :\'' + functionName + '\',' + end +
                    '[prototype]: ' + functionName + ' { [constructor]: [Circular] } }')
            } else {
                strB.append(baseFunctionStr)
            }

            return strB.toString()
        }

        private static arrayToStringImpl(delimiter: string, arr: Array<Object>, depth: number, full: boolean = false): string {
            if (!arr.length) {
                return ''
            }

            let arrayDelimiter: string = ', '

            for (let item of arr) {
                if (!UtilHelper.isNullObject(item) && (UtilHelper.isFunction(item) || UtilHelper.isObject(item) && depth <= 2)) {
                    arrayDelimiter += delimiter
                    break
                }
            }

            let strB = new StringBuilder
            for (let item of arr) {
                if (UtilHelper.isObject(item)) {
                    strB.append((new UtilHelper).stringifyObject(item, delimiter + '  ', depth + 1, full))
                } else if (UtilHelper.isFunction(item)) {
                    strB.append(UtilHelper.functionDescription(item, delimiter, false, full, true))
                } else {
                    strB.append(UtilHelper.simpleObjectDescription(item, full ? UtilHelper.getName(item) : ''))
                }
                strB.append(arrayDelimiter)
            }

            return strB.toString().slice(0, -arrayDelimiter.length)
        }

        private static getName(obj: Object): string {
            let subObj = UtilHelper.getValue("name", obj)
            return subObj ? subObj! as string : 'null'
        }

        private static getLength(obj: Object): int {
            let subObj = UtilHelper.getValue("length", obj)
            return subObj ? subObj! as int : 0
        }

        private static getValue(key: string, obj: Object): NullishType {
            if (!obj) {
                return null
            }
            for (const objEntry of Object.entries(obj)) {
                if (!objEntry) {
                    continue
                }
                if ((!objEntry[0]) && (objEntry[0] == key)) {
                    return objEntry[1]
                }
            }
            return null
        }

        /**
          * Remove html tag, css, js of html string to extract the contents
          * @param {String} html
          * @return {String} contents of the html
          */
        static extractContents(html: string): string {
            return html
                .replace(new RegExp("(\n|\r|\t)", "gm"), '') // remove linebreaks
                .replace(new RegExp("<(style|script|link|noscript).*?>.*?<\/(style|script|link|noscript)>", "g"), '') // remove css, js blocks
                .replace(new RegExp("<!--.*?-->", "g"), '') // remove comments
                .replace(new RegExp("<.*?>", "g"), '') // remove tags
                .replace(new RegExp("[\s\!\@\#\$\%\^\&\*\(\)\_\+\-\=\{\}\[\]\:\"\;\'\<\>\?\,\.\/\|\\\`\~]+", "g"), '') // remove Chars
                .trim()
        }

        static processArg(flag: string, arg: Object): string {
            // %c: CSS. This specifier is ignored and will skip any CSS passed in.
            if (flag == "c" && (arg instanceof String)) {
                return UtilHelper.extractContents(arg as string)
            } else if (flag == "s" && !(arg instanceof BigInt)) {
                if (arg instanceof BigInt || arg instanceof Char) {
                    throw new Error("invalid flag " + flag + " for arg " + arg as string)
                }
                return arg as string
            } else if (flag == "d") {
                if (arg instanceof BigInt || arg instanceof Char) {
                    throw new Error("invalid flag " + flag + " for arg " + arg as string)
                }
                return "" + new Number(arg.toString())
            } else if (flag == "i") {
                if (arg instanceof BigInt || arg instanceof Char) {
                    throw new Error("invalid flag " + flag + " for arg " + arg as string)
                }
                return "" + parseInt(arg as string, 10)
            }
            else if (flag == "f") {
                if (arg instanceof Char) {
                    throw new Error("invalid flag " + flag + " for arg " + arg as string)
                }
                return "" + parseFloat(arg as string)
            } else if (flag == "j") {
                let res = ""
                try {
                    res = JSON.stringify(arg)
                }
                catch (error) {
                    if (typeof arg === "object") {
                        return UtilHelper.objectToString(arg)
                    } else {
                        throw new Error("error parse json")
                    }
                }

                return res
            } else if (flag == "o") {
                // Object. A string representation of an object
                // Similar to util.inspect() with options { showHidden: true, showProxy: true }
                return UtilHelper.objectToString(arg, true)
            } else if (flag == "O") {
                // Object. A string representation of an object
                // Similar to util.inspect() without options
                return UtilHelper.objectToString(arg)
            }

            return "%" + flag
        }

        static joinLastArgs(index: int, ...args: Object[]): string {
            let sb = new StringBuilder
            for (let i = index; i < args.length; ++i) {
                sb.append(" ")
                sb.append(args[i])
            }
            let res = sb.toString()
            return res
        }

    }  // class UtilHelper
}

