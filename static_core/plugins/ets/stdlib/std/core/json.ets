/**
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

package std.core

const BigIntType: Class;

export type JsonRecordType = boolean | bigint | String | undefined | null | Double | Long | Record<string, JsonRecordType> | Array<JsonRecordType>

export interface JsonReplacer {
    jsonReplacer(): Record<String, Any>
}

export @interface JSONRename {
    newName: string
}

export @interface JSONStringifyIgnore {
}

export @interface JSONParseIgnore {
}

/**
 * Marks a zero-argument instance 'get' method to be called by JSON.stringifyWithGetters().
 * The method name is used as the JSON key and the return value as the corresponding value.
 * 
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 */
export @interface JSONStringifyGetter {
}


class JSONAPI {
    public native static getJSONStringifyIgnore(field: reflect.InstanceField): boolean

    public native static getJSONParseIgnore(field: reflect.InstanceField): boolean

    public native static getJSONRename(field: reflect.InstanceField): string | undefined

    public native static getJSONStringifyGetter(method: reflect.InstanceMethod): boolean
}

class FieldMetadata {
    readonly field: reflect.InstanceField
    readonly ignore: boolean
    readonly rename: string | undefined
    readonly name: string

    constructor(field: reflect.InstanceField, ignore: boolean, rename: string | undefined) {
        this.field = field
        this.ignore = ignore
        this.rename = rename
        this.name = field.getName()
    }

    getValue(obj: object): Any {
        return this.field.getValue(obj)
    }

    getName(): string {
        return this.rename ?? this.name
    }
}

class CachedObjectLayoutEntry {
    readonly name: string
    readonly metadata: FieldMetadata

    constructor(name: string, metadata: FieldMetadata) {
        this.name = name
        this.metadata = metadata
    }
}

function validateWritableFieldName(cls: Class, metadata: FieldMetadata, writtenNames: Set<string>): string {
    const writableFieldName = metadata.rename!
    if (writtenNames.has(writableFieldName)) {
        throw new Error(`Cannot double rename ${writableFieldName} in keys of ${cls.getName()}`)
    }
    return writableFieldName
}

class ClassMetadata {
    readonly cls: Class
    readonly metadata: Array<FieldMetadata>
    readonly nameMap: Map<string, FieldMetadata>
    readonly hasRenames: boolean
    readonly objectLayout: Array<CachedObjectLayoutEntry>
    readonly fastObjectLayout: boolean
    readonly tupleEntries: Array<FieldMetadata>

    private constructor(cls: Class, metadata: Array<FieldMetadata>, nameMap: Map<string, FieldMetadata>, hasRenames: boolean,
        objectLayout: Array<CachedObjectLayoutEntry>, fastObjectLayout: boolean, tupleEntries: Array<FieldMetadata>) {
        this.cls = cls
        this.metadata = metadata
        this.nameMap = nameMap
        this.hasRenames = hasRenames
        this.objectLayout = objectLayout
        this.fastObjectLayout = fastObjectLayout
        this.tupleEntries = tupleEntries
    }

    static createFastObjectLayout(cls: Class, metadata: Array<FieldMetadata>, nameMap: Map<string, FieldMetadata>,
        objectLayout: Array<CachedObjectLayoutEntry>, tupleEntries: Array<FieldMetadata>): ClassMetadata {
        return new ClassMetadata(cls, metadata, nameMap, false, objectLayout, true, tupleEntries)
    }

    static createObjectLayout(cls: Class, metadata: Array<FieldMetadata>, nameMap: Map<string, FieldMetadata>,
        hasRenames: boolean, tupleEntries: Array<FieldMetadata>): ClassMetadata {
        const objectLayout = new Array<CachedObjectLayoutEntry>()
        const nameToIndex = new Map<string, int>()
        const writtenNames = new Set<string>()
        const metadataLen = metadata.length
        for (let i = 0; i < metadataLen; i++) {
            const meta = metadata[i]
            if (meta.ignore) {
                continue
            }
            if (meta.rename !== undefined && nameMap.has(meta.rename!)) {
                throw new Error(`Double usage of \`${meta.rename}\` in ${cls.getName()}.` +
                    ' The name also is used by JSONRename in base class')
            }
            const writableFieldName = meta.rename !== undefined
                ? validateWritableFieldName(cls, meta, writtenNames)
                : meta.name
            const entry = new CachedObjectLayoutEntry(writableFieldName, meta)
            const existingIndex = nameToIndex.get(meta.name)
            if (meta.rename !== undefined || existingIndex === undefined) {
                nameToIndex.set(writableFieldName, objectLayout.length)
                writtenNames.add(writableFieldName)
                objectLayout.push(entry)
            } else {
                objectLayout[existingIndex] = entry
            }
        }
        return new ClassMetadata(cls, metadata, nameMap, hasRenames, objectLayout, false, tupleEntries)
    }
}

class FieldMetadataCache {
    private static readonly CACHE_CAPACITY: int = 200
    private static readonly cache: ConcurrentLRUCache<Class, ClassMetadata> = new ConcurrentLRUCache<Class, ClassMetadata>(FieldMetadataCache.CACHE_CAPACITY)

    private static getClassMetadata(cls: Class): ClassMetadata {
        return FieldMetadataCache.cache.computeIfAbsent(cls, (c: Class): ClassMetadata => {
            const fields = reflect.getInstanceFieldsRecursive(c)
            const metadata = new Array<FieldMetadata>(fields.length, (i) => {
                const field = fields[i]
                const ignore = JSONAPI.getJSONStringifyIgnore(field)
                const rename = JSONAPI.getJSONRename(field)
                return new FieldMetadata(field, ignore, rename)
            })
            const nameMap = new Map<string, FieldMetadata>()
            const objectLayout = new Array<CachedObjectLayoutEntry>()
            const tupleEntries = new Array<FieldMetadata>()
            let hasRenames = false
            let hasSpecialSerialization = false
            const metadataLen = metadata.length

            for (let i = 0; i < metadataLen; i++) {
                const meta = metadata[i]
                nameMap.set(meta.name, meta)
                if (!meta.ignore) {
                    tupleEntries.push(meta)
                }
                if (meta.ignore || meta.rename !== undefined) {
                    hasSpecialSerialization = true
                }
                if (!meta.ignore && meta.rename !== undefined) {
                    hasRenames = true
                }
                if (!hasSpecialSerialization) {
                    objectLayout.push(new CachedObjectLayoutEntry(meta.name, meta))
                }
            }

            if (!hasSpecialSerialization) {
                return ClassMetadata.createFastObjectLayout(c, metadata, nameMap, objectLayout, tupleEntries)
            }
            return ClassMetadata.createObjectLayout(c, metadata, nameMap, hasRenames, tupleEntries)
        })
    }

    static getMetadataByName(cls: Class, name: string): FieldMetadata | undefined {
        return FieldMetadataCache.getClassMetadata(cls).nameMap.get(name)
    }

    static hasMetadataByName(cls: Class, name: string): boolean {
        return FieldMetadataCache.getClassMetadata(cls).nameMap.has(name)
    }

    static getObjectLayout(cls: Class): Array<CachedObjectLayoutEntry> {
        return FieldMetadataCache.getClassMetadata(cls).objectLayout
    }

    static hasFastObjectLayout(cls: Class): boolean {
        return FieldMetadataCache.getClassMetadata(cls).fastObjectLayout
    }

    static getTupleEntries(cls: Class): Array<FieldMetadata> {
        return FieldMetadataCache.getClassMetadata(cls).tupleEntries
    }

}

// JsonSerializable provides a interface for external classes to override toJSON
export interface JsonSerializable {
    toJSON(): string
}

export class JSON {

    /**
     * Converts byte to JSON format
     *
     * @param d: byte - byte to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of byte
     */
    public static stringify(d: byte): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts char to JSON format
     *
     * @param d: char - char to be converted to a JSON as a String
     *
     * @returns String - JSON representation of char
     */
    public static stringify(d: char): String {
        let sb = new StringBuilder('"')
        sb.append(d)
        sb.append('"')
        return sb.toString()
    }

    /**
     * Converts short to JSON format
     *
     * @param d: short - short to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of short
     */
    public static stringify(d: short): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts int to JSON format
     *
     * @param d: int - int to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of int
     */
    public static stringify(d: int): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts long to JSON format
     *
     * @param d: long - long to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of long
     */
    public static stringify(d: long): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts float to JSON format
     *
     * @param d: float - float to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of float
     */
    public static stringify(d: float): String {
        if (Float.isFinite(d)) {
            return StringBuilder.toString(d)
        } else {
            return 'null'
        }
    }

    /**
     * Converts double to JSON format
     *
     * @param d: double - double to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of double
     */
    public static stringify(d: double): String {
        if (Double.isFinite(d)) {
            return StringBuilder.toString(d)
        } else {
            return 'null'
        }
    }

    /**
     * Converts bigint to JSON format
     *
     * @param d: bigint - bigint to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of bigint
     */
    public static stringify(d: bigint): String {
        return d.toString()
    }

    /**
     * Converts boolean to JSON format
     *
     * @param d: boolean - boolean to be converted to a JSON as a Boolean literal
     *
     * @returns String - JSON representation of boolean
     */
    public static stringify(d: boolean): String {
        if (d) {
            return 'true'
        }
        return 'false'
    }

    private static readonly JSON_SPACE_INDENT_LIMIT = 10

    /**
     * Converts String to JSON format
     *
     * @param d: String - byte to be converted to a JSON as a String
     *
     * @returns String - JSON representation of byte
     */
    public static stringify(d: String): String {
        const len = d.getLength()

        if (len == 0) {
            return '""'
        }

        let needsEscaping = false
        let hasSurrogates = false
        let finalSize = len + 2

        for (let i = 0; i < len; i++) {
            const code = d.charCodeAt(i)
            if (code < 128) {
                if (code == c'"' || code == c'\\' || code == c'\b' || code == c'\f' ||
                    code == c'\n' || code == c'\r' || code == c'\t') {
                    needsEscaping = true
                    finalSize += 1
                } else if (code < 0x20 || code == 0x7F) {
                    needsEscaping = true
                    finalSize += 5
                }
            } else if (code >= 0xD800 && code <= 0xDFFF) {
                if (code >= 0xD800 && code <= 0xDBFF) {
                    if (i + 1 >= len || d.charCodeAt(i + 1) < 0xDC00 || d.charCodeAt(i + 1) > 0xDFFF) {
                        hasSurrogates = true
                        needsEscaping = true
                        finalSize += 5  // \uXXXX
                    } else {
                        hasSurrogates = true
                        i++
                    }
                } else {
                    hasSurrogates = true
                    needsEscaping = true
                    finalSize += 5
                }
            }
        }

        if (!needsEscaping && !hasSurrogates) {
            return '"' + d + '"'
        }

        let chars = new ValueArray<char>(finalSize)
        let charIndex = 0
        chars[charIndex++] = c'"'

        let begin = 0
        for (let i = 0; i < len; ) {
            const ch = d.charAt(i)
            const code = ch.toInt()

            if (begin < i) {
                for (let j = begin; j < i; j++) {
                    chars[charIndex++] = d.charAt(j)
                }
            }

            if (code < 128) {
                switch (ch) {
                    case c'"':
                        chars[charIndex++] = c'\\'
                        chars[charIndex++] = c'"'
                        begin = i + 1
                        break
                    case c'\\':
                        chars[charIndex++] = c'\\'
                        chars[charIndex++] = c'\\'
                        begin = i + 1
                        break
                    case c'\b':
                        chars[charIndex++] = c'\\'
                        chars[charIndex++] = c'b'
                        begin = i + 1
                        break
                    case c'\f':
                        chars[charIndex++] = c'\\'
                        chars[charIndex++] = c'f'
                        begin = i + 1
                        break
                    case c'\n':
                        chars[charIndex++] = c'\\'
                        chars[charIndex++] = c'n'
                        begin = i + 1
                        break
                    case c'\r':
                        chars[charIndex++] = c'\\'
                        chars[charIndex++] = c'r'
                        begin = i + 1
                        break
                    case c'\t':
                        chars[charIndex++] = c'\\'
                        chars[charIndex++] = c't'
                        begin = i + 1
                        break
                    default:
                        if (code < 0x20 || code == 0x7F) {
                            const hex = code.toString(16).padStart(4, '0')
                            chars[charIndex++] = c'\\'
                            chars[charIndex++] = c'u'
                            chars[charIndex++] = hex.charAt(0)
                            chars[charIndex++] = hex.charAt(1)
                            chars[charIndex++] = hex.charAt(2)
                            chars[charIndex++] = hex.charAt(3)
                            begin = i + 1
                        } else {
                            begin = i
                        }
                        break
                }
                i++
            }
            // Check for surrogate characters
            else if (code >= 0xD800 && code <= 0xDFFF) {
                if (Char.isHighSurrogate(ch)) {
                    if (i + 1 < len && Char.isLowSurrogate(d.charAt(i + 1))) {
                        chars[charIndex++] = ch
                        chars[charIndex++] = d.charAt(i + 1)
                        i += 2
                    } else {
                        const hex = code.toString(16).padStart(4, '0')
                        chars[charIndex++] = c'\\'
                        chars[charIndex++] = c'u'
                        chars[charIndex++] = hex.charAt(0)
                        chars[charIndex++] = hex.charAt(1)
                        chars[charIndex++] = hex.charAt(2)
                        chars[charIndex++] = hex.charAt(3)
                        i++
                    }
                } else if (Char.isLowSurrogate(ch)) {
                    const hex = code.toString(16).padStart(4, '0')
                    chars[charIndex++] = c'\\'
                    chars[charIndex++] = c'u'
                    chars[charIndex++] = hex.charAt(0)
                    chars[charIndex++] = hex.charAt(1)
                    chars[charIndex++] = hex.charAt(2)
                    chars[charIndex++] = hex.charAt(3)
                    i++
                } else {
                    i++
                }
                begin = i
            }
            else {
                i++
            }
        }

        if (begin < len) {
            for (let j = begin; j < len; j++) {
                chars[charIndex++] = d.charAt(j)
            }
        }

        chars[charIndex++] = c'"'
        return new String(chars)
    }

    //--------------------------
    //          arrays
    //--------------------------

    /**
     * Converts bytes array to JSON format
     *
     * @param d: byte[] - bytes array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of bytes array
     */
    public static stringify(d: FixedArray<byte>): String {
        let s = new StringBuilder('[')
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(d[i])
            s.append(',')
        }
        if (d.length > 0) {
            s.append(d[last_elem])
        }
        s.append(']')
        return s.toString()
    }

    public static stringify(d: ValueArray<byte>): String {
        return JSON.stringify(arkruntime.stub.toFixedArray(d))
    }

    /**
     * Converts chars array to JSON format
     *
     * @param d: FixedArray<char> - chars array  to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of chars array
     */
    public static stringify(d: FixedArray<char>): String {
        let s = new StringBuilder('[')
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append('"')
            s.append(d[i])
            s.append('",')
        }
        if (d.length > 0) {
            s.append('"')
            s.append(d[last_elem])
            s.append('"')
        }
        s.append(']')
        return s.toString()
    }

    public static stringify(d: ValueArray<char>): String {
        return JSON.stringify(arkruntime.stub.toFixedArray(d))
    }

    /**
     * Converts shorts array to JSON format
     *
     * @param d: FixedArray<short> - shorts array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of shorts array
     */
    public static stringify(d: FixedArray<short>): String {
        let s = new StringBuilder('[')
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(d[i])
            s.append(',')
        }
        if (d.length > 0) {
            s.append(d[last_elem])
        }
        s.append(']')
        return s.toString()
    }

    public static stringify(d: ValueArray<short>): String {
        return JSON.stringify(arkruntime.stub.toFixedArray(d))
    }

    /**
     * Converts ints array to JSON format
     *
     * @param d: FixedArray<int> - ints array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of ints array
     */
    public static stringify(d: FixedArray<int>): String {
        let s = new StringBuilder('[')
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(d[i])
            s.append(',')
        }
        if (d.length > 0) {
            s.append(d[last_elem])
        }
        s.append(']')
        return s.toString()
    }

    public static stringify(d: ValueArray<int>): String {
        return JSON.stringify(arkruntime.stub.toFixedArray(d))
    }

    /**
     * Converts longs array to JSON format
     *
     * @param d: FixedArray<long> - longs array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of longs array
     */
    public static stringify(d: FixedArray<long>): String {
        let s = new StringBuilder('[')
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(d[i])
            s.append(',')
        }
        if (d.length > 0) {
            s.append(d[last_elem])
        }
        s.append(']')
        return s.toString()
    }

    public static stringify(d: ValueArray<long>): String {
        return JSON.stringify(arkruntime.stub.toFixedArray(d))
    }

    /**
     * Converts array of bytes to JSON format
     *
     * @param d: FixedArray<byte> - array of byte to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of array of bytes
     */
    public static stringify(d: FixedArray<float>): String {
        let s = new StringBuilder('[')
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(JSON.stringify(d[i]))
            s.append(',')
        }
        if (d.length > 0) {
            s.append(JSON.stringify(d[last_elem]))
        }
        s.append(']')
        return s.toString()
    }

    public static stringify(d: ValueArray<float>): String {
        return JSON.stringify(arkruntime.stub.toFixedArray(d))
    }

    /**
     * Converts doubles array to JSON format
     *
     * @param d: FixedArray<double> - doubles array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of doubles array
     */
    public static stringify(d: FixedArray<double>): String {
        let s = new StringBuilder('[')
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(JSON.stringify(d[i]))
            s.append(',')
        }
        if (d.length > 0) {
            s.append(JSON.stringify(d[last_elem]))
        }
        s.append(']')
        return s.toString()
    }

    public static stringify(d: ValueArray<double>): String {
        return JSON.stringify(arkruntime.stub.toFixedArray(d))
    }

    /**
     * Converts booleans array to JSON format
     *
     * @param d: FixedArray<boolean> - booleans array to be converted to a JSON as an Array of Boolean literals
     *
     * @returns String - JSON representation of booleans array
     */
    public static stringify(d: FixedArray<boolean>): String {
        let s = new StringBuilder('[')
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            if (d[i]) {
                s.append('true,')
            } else {
                s.append('false,')
            }
        }
        if (d.length > 0) {
            s.append(d[last_elem])
        }
        s.append(']')
        return s.toString()
    }

    public static stringify(d: ValueArray<boolean>): String {
        return JSON.stringify(arkruntime.stub.toFixedArray(d))
    }

    public static stringify(d: Array<Number>): String {
        let s = new StringBuilder('[')
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(JSON.stringify(d[i]));
            s.append(',');
        }
        if (d.length > 0) {
            s.append(JSON.stringify(d[last_elem]));
        }
        s.append(']');
        return s.toString()
    }

    public static stringify(d: ArrayLike<Number>): String {
        let s = new StringBuilder('{')
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append('"'.concat(JSON.stringify(i), '":'));
            s.append(JSON.stringify(d[i]));
            s.append(',');
        }
        if (d.length > 0) {
            s.append('"'.concat(JSON.stringify(last_elem), '":'));
            s.append(JSON.stringify(d[last_elem]));
        }
        s.append('}');
        return s.toString()
    }

    public static stringify(obj: JsonReplacer): String {
        const record = obj.jsonReplacer();
        return JSON.stringify(record);
    }

    private static native stringifyJSValue(d: JSValue): String;

    /**
     * Converts an object to a JavaScript Object Notation (JSON) string.
     *
     * @param obj: Object An object to be converted.
     *
     * @returns String - JSON representation of Object
     */
    public static stringify(obj: Any): String {
        if (obj === undefined) {
            return ''
        }
        if (obj instanceof Function) {
            return ''
        }
        if (obj instanceof JSValue) {
            return JSON.stringifyJSValue(obj)
        }
        return new JSONWriter().writeFast(obj)
    }

    private static native stringifyFast(obj: Any): string

    /**
     * Converts an object to a JavaScript Object Notation (JSON) string,
     * invoking any instance methods annotated with @JSONStringifyGetter and
     * including their return values in the output. Each annotated method's name
     * is used as the JSON key, and its return value is used as the corresponding value.
     *
     * @param { Any } obj - An object to be converted.
     *
     * @returns String - JSON representation of Object, including annotated getter values
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static stringifyWithGetters(obj: Any): String {
        const replacerOpt: ((key: string, value: Any) => Any) | undefined = undefined
        return new JSONWriter(replacerOpt, undefined, true).write(obj)
    }

    /**
     * Converts an object to a JavaScript Object Notation (JSON) string,
     * invoking any instance methods annotated with @JSONStringifyGetter and
     * including their return values in the output. Each annotated method's name
     * is used as the JSON key, and its return value is used as the corresponding value.
     *
     * @param { Any } obj - An object to be converted.
     *
     * @param { ((key: string, value: Any) => Any) | undefined | null } replacer - A function that transforms the results.
     *
     * @param { int | string } [space] - A string or number that's used to insert white space
     * (including indentation, line break characters, etc.) into the output JSON string for readability purposes.
     *
     * @returns String - JSON representation of Object, including annotated getter values
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static stringifyWithGetters(
                obj: Any,
                replacer: ((key: string, value: Any) => Any) | undefined | null,
                space?: int | string): string {
        if (obj === undefined) {
            return ''
        }
        const replacerOpt: ((key: string, value: Any) => Any) | undefined = (replacer === null) ? undefined : replacer
        let spaceOpt: String | undefined
        if (space instanceof Int) {
            spaceOpt = JSON.spaceFromNumber(space)
        } else {
            spaceOpt = space
        }
        return new JSONWriter(replacerOpt, spaceOpt, true).write(obj)
    }

    /**
     * Converts an object to a JavaScript Object Notation (JSON) string,
     * invoking any instance methods annotated with @JSONStringifyGetter and
     * including their return values in the output. Each annotated method's name
     * is used as the JSON key, and its return value is used as the corresponding value.
     *
     * @param { Any } obj - An object to be converted.
     *
     * @param { Array<double | string> | Array<string> | Array<double> } replacer - An array with elements indicating names of the properties in the object
     * that should be included in the resulting JSON string.
     * Annotated getter methods whose names appear in this array are also included.
     *
     * @param { int | string } [space] - A string or number that's used to insert white space
     * (including indentation, line break characters, etc.) into the output JSON string for readability purposes.
     *
     * @returns String - JSON representation of Object, including annotated getter values
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static stringifyWithGetters(
                obj: Any,
                replacer: Array<double | string> | Array<string> | Array<double>,
                space?: int | string): string {
        if (obj === undefined) {
            return ''
        }
        const filter: FixedArray<string> = new FixedArray<string>(replacer.length)
        let index: int = 0
        for (let element of replacer) {
            filter[index++] = `${element}`
        }
        let spaceOpt: String | undefined
        if (space instanceof Int) {
            spaceOpt = JSON.spaceFromNumber(space)
        } else {
            spaceOpt = space
        }
        return new JSONWriter(filter, spaceOpt, true).write(obj)
    }

    /**
     * Converts an object to a JavaScript Object Notation (JSON) string.
     *
     * @param obj: Object An object to be converted.
     *
     * @param replacer A function that transforms the results.
     *
     * @param space A string or number that's used to insert white space
     * (including indentation, line break characters, etc.) into the output JSON string for readability purposes.
     *
     * @returns String - JSON representation of Object
     */
    public static stringify(
                obj: Any,
                replacer: ((key: string, value: Any) => Any) | undefined | null,
                space?: string | int): string {
        const replacerOpt: ((key: string, value: Any) => Any) | undefined = (replacer === null) ? undefined : replacer

        if (space instanceof String || space === undefined) {
            return new JSONWriter(replacerOpt, space).write(obj)
        } else {
            return new JSONWriter(replacerOpt, JSON.spaceFromNumber(space as int)).write(obj)
        }
    }

    /**
     * Converts an object to a JavaScript Object Notation (JSON) string.
     *
     * @param obj: Object An object to be converted.
     *
     * @param replacer An array with elements indicating names of the properties in the object
     * that should be included in the resulting JSON string
     *
     * @param space A string or number that's used to insert white space
     * (including indentation, line break characters, etc.) into the output JSON string for readability purposes.
     *
     * @returns String - JSON representation of Object
     */
    public static stringify(obj: Any, replacer: FixedArray<double | string>, space?: int | string): string {
        const filter : FixedArray<string> = new FixedArray<string>(replacer.length)
        for (let i = 0; i < replacer.length; i++) {
            filter[i] = `${replacer[i]}`
        }

        if (space instanceof String || space === undefined) {
            return new JSONWriter(filter, space).write(obj)
        } else {
            return new JSONWriter(filter, JSON.spaceFromNumber(space! as int)).write(obj)
        }
    }

    public static stringify(obj: Any, replacer: Array<double | string> | Array<string> | Array<double>,
        space?: int | string): string {

        const filter : FixedArray<string> = new FixedArray<string>(replacer.length)
        let index: int = 0
        for (let element of replacer) {
            filter[index++] = `${element}`
        }

        if (space instanceof String || space === undefined) {
            return new JSONWriter(filter, space).write(obj)
        } else {
            return new JSONWriter(filter, JSON.spaceFromNumber(space! as int)).write(obj)
        }
    }

    private static spaceFromNumber(space: int): String {
        let spacesCount = 0
        if (space > 0)  {
            spacesCount = (space > JSON.JSON_SPACE_INDENT_LIMIT) ? JSON.JSON_SPACE_INDENT_LIMIT : space
        }

        return ' '.repeat(spacesCount)
    }

    /**
      * Converts a JsonElementSerializable to a JSON string.
     * @param elem - The JsonElementDeserializable to stringify
     * @returns The JSON string representation
     */
    public static stringifyJsonElement(elem: jsonx.JsonElementSerializable): string {
        return JSON.stringifyJsonElement(elem.toJSON())
    }

    /**
      * Converts a JsonElementSerializable to a JSON string with optional formatting.
     * @param elem - The JsonElementDeserializable to stringify
     * @param replacer - Array of keys to include (currently not implemented)
     * @param space - String or number of spaces for indentation
     * @returns The JSON string representation
     */
    public static stringifyJsonElement(elem: jsonx.JsonElementSerializable, replacer?: (double | string)[],
        space?: int | string): string {

        return JSON.stringifyJsonElement(elem.toJSON(), replacer, space)
    }

    /**
      * Converts a JsonElement to a JSON string.
     * @param elem - The JsonElement to stringify
     * @returns The JSON string representation
     */
    public static stringifyJsonElement(elem: jsonx.JsonElement): string {
        return stringifyElementRecursively(elem, undefined, undefined, 0)
    }

    /**
      * Converts a JsonElement to a JSON string with optional formatting.
     * @param elem - The JsonElement to stringify
     * @param replacer - Array of keys to include (currently not implemented)
     * @param space - String or number of spaces for indentation
     * @returns The JSON string representation
     */
    public static stringifyJsonElement(elem: jsonx.JsonElement, replacer?: (double | string)[],
        space?: int | string): string {

        return stringifyElementRecursively(elem, replacer, space, 0)
    }

    // TODO(kirill-mitkin): Map<Long, Long> blocked by internal issue
    private static checkType(type: Class, used: Map<Class, TypeColor>): void {
        const tc = used.get(type)
        if (tc !== undefined) {
            if (tc.color == TypeColor.VISITED) {
                return
            }
            throw new Error(`Incorrect type: ${type.getName()}`)
        }
        if (type === BigIntType ||
                type === TypeColor.STRING_CLASS ||
                reflect.internals.Types.isPrimitiveType(type) ||
                type === TypeColor.NULL_CLASS) {
            return
        }
        if (type.isFixedArray()) {
            JSON.checkType(type.getFixedArrayComponentType()!, used)
        } else if (type.isUnion()) {
            // NOTE(lyupaanastasia) parse union #27136
            throw new Error('UnionType is not supported yet')
        } else if (type.isSubtypeOf(TypeColor.OBJECT_CLASS)) {
            const newTc = new TypeColor(type, TypeColor.AT_STACK)
            used.set(type, newTc)
            if (!type.isEnum()) {
                if (!JSON.classHasDefaultCtor(type)) {
                    throw new Error(`Incorrect type: '${type.getName()}' doesn't provide default public constructor`)
                }
                const fields: Array<reflect.InstanceField> = reflect.getInstanceFieldsRecursive(type)
                const len = fields.length
                for (let i = 0; i < len; ++i) {
                    const field = fields[i]
                    if (JSONAPI.getJSONParseIgnore(field)) {
                        continue
                    }
                    try {
                        JSON.checkType(field.getType(), used)
                    } catch (e) {
                        throw new Error(`Incorrect field: '${type.getName()}.${field.getName()}'. ${e.message}`)
                    }
                }
            }
            newTc.color = TypeColor.VISITED
        } else {
            throw new Error(`Incorrect type: ${type.getName()}`)
        }
    }

    private static classHasDefaultCtor(cls: Class): boolean {
        for (const ctor of cls.getConstructors()) {
            if (ctor.getParametersNum() == 0) {
                return true
            }
        }
        return false
    }

    public static parse<T = Any>(text: String, type: Type): T | null | undefined {
        const cls = type.getClass()
        if (cls !== undefined) {
            return JSON.parse<T>(text, undefined, cls)
        }
        throw new Error(`Unable to resolve type ${type} runtime class`)
    }

    public static parse<T = Any>(text: string, reviver: ((key: string, value: Any) => Any) | undefined, type: Type,
        options?: jsonx.ParseOptions): T | null | undefined {

        const cls = type.getClass()
        if (cls !== undefined) {
            return JSON.parse<T>(text, reviver, cls, options)
        }
        throw new Error(`Unable to resolve type ${type} runtime class`)
    }

    public static parse<T = Any>(json: string, type: Class): T | null | undefined {
        return JSON.parse<T>(json, undefined, type)
    }

    public static parse<T = Any>(json: string, reviver: ((key: string, value: Any) => Any) | undefined, type: Class,
        options?: jsonx.ParseOptions): T | null | undefined {

        JSON.checkType(type, new Map<Class, TypeColor>())

        let bigIntOpt: int = options ? options.bigIntMode : 0
        let jsonValue = UnifiedJsonParser.parseToJSONValue(json)
        const parsingResult = new JSONValueParser(reviver, bigIntOpt).parse(jsonValue, type) as T

        if (reviver !== undefined) {
            return reviver('', parsingResult) as T
        } else {
            return parsingResult
        }
    }

    /**
     * Parses a JSON string and populates the fields of an existing instance.
     * Unlike parse<T>(json, type), this does NOT require a default constructor.
     * Fields whose keys are absent from the JSON retain their current values
     * on the instance (merge semantics).
     *
     * @param { string } json - JSON string to parse
     * @param { T } instance - Existing instance whose fields will be populated
     * @returns The same instance after population
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static parseUpdate<T>(json: string, instance: T): T {
        return JSON.parseUpdate<T>(json, undefined, instance)
    }

    /**
     * Parses a JSON string and populates the fields of an existing instance,
     * with an optional reviver and parse options.
     * Unlike parse<T>(json, type), this does NOT require a default constructor.
     * Fields whose keys are absent from the JSON retain their current values
     * on the instance (merge semantics).
     *
     * @param { string } json - JSON string to parse
     * @param { ((key: string, value: Any) => Any) | undefined } reviver - Optional function to transform each parsed value, or undefined
     * @param { T } instance - Existing instance whose fields will be populated
     * @param { jsonx.ParseOptions } [options] - Optional ParseOptions (e.g. bigIntMode)
     * @returns The same instance after population
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static parseUpdate<T>(
        json: string,
        reviver: ((key: string, value: Any) => Any) | undefined,
        instance: T,
        options?: jsonx.ParseOptions
    ): T {
        let bigIntOpt: int = options ? options.bigIntMode : 0
        const jsonValue = UnifiedJsonParser.parseToJSONValue(json)
        const result = new JSONValueParser(reviver, bigIntOpt).parseInto(jsonValue, instance as object) as T

        if (reviver !== undefined) {
            return reviver('', result) as T
        }
        return result
    }

    /**
      * Parses a JSON string and returns a JsonElement.
      * @param {string} text - The JSON string to parse
      * @param {jsonx.ParseOptions} [options] - BigInt parsing options
      * @returns {JsonElement} The parsed JSON element
      * @throws {JsonParseError} If the JSON string is invalid
      * @static
      * @syscap SystemCapability.Utils.Lang
      * @FaAndStageModel
      */
    public static parseJsonElement(text: string, options?: jsonx.ParseOptions): jsonx.JsonElement {
        return new UnifiedJsonParser(text, options).parse()
    }

    /**
      * Parses a JSON string with a reviver function and returns a JsonElement.
      * @param {string} text - The JSON string to parse
      * @param {(key: string, value: JsonElement) => JsonElement} reviver - Function to transform values
      * @param {jsonx.ParseOptions} [options] - BigInt parsing options
      * @returns {JsonElement} The parsed JSON element
      * @throws {JsonParseError} If the JSON string is invalid
      * @static
      * @syscap SystemCapability.Utils.Lang
      * @FaAndStageModel
      */
    public static parseJsonElement(text: string, reviver: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement,
        options?: jsonx.ParseOptions): jsonx.JsonElement {

        return new UnifiedJsonParser(text, reviver, options).parse()
    }

    /**
      * Parses a JSON object string and returns a Record.
      * @param {string} text - The JSON string to parse
      * @param {jsonx.ParseOptions} [options] - BigInt parsing options
      * @returns {Record<string, JsonRecordType>} The parsed Record object
      * @throws {JsonParseError} If the JSON string is invalid
      * @static
      * @syscap SystemCapability.Utils.Lang
      * @FaAndStageModel
      */
    public static parseJsonRecord(text: string, options?: jsonx.ParseOptions): Record<string, JsonRecordType> {
        return new UnifiedJsonParser(text, options).parseRecord()
    }

}

class JSONWriter {
    private static readonly STD_CORE_INTEROP_JSVALUE_CLASS = Class.from<JSValue>()
    // Length of the bytecode getter prefix "%%get-" (GETTER_BEGIN in ets_type.h)
    private static readonly GETTER_PREFIX_LEN: int = 6
    private static readonly maxSpaceLength = 10
    private static readonly getterMetadataCache = new Map<Class, Array<[string, reflect.InstanceMethod]>>()

    private static truncateSpace(space: String | undefined): String | undefined {
        if (space !== undefined && space.length > JSONWriter.maxSpaceLength) {
            return space.substring(0, JSONWriter.maxSpaceLength)
        }
        return space
    }

    private readonly replacer: ((key: String, value: Any) => Any) | undefined
    private readonly fieldsFilter: FixedArray<string>
    private readonly space: String | undefined

    private readonly useReplacer: boolean
    private readonly useFieldsFilter: boolean
    private readonly useIndent: boolean
    private readonly useAnnotatedGetters: boolean
    private readonly useSimpleFastPath: boolean

    private static readonly PADDING_CACHE_SIZE: int = 20

    private indentLevel: int = 0
    private paddingCache: FixedArray<String> | undefined
    private writtenNamesBuf: Set<string> = new Set<string>()
    private fieldsFilterSet: Set<string> | undefined
    private filterNamePos: Map<string, int> | undefined

    private readonly path = new Set<Any>()
    private buffer = new StringBuilder()

    private getPaddingCache(): FixedArray<String> {
        if (this.paddingCache === undefined) {
            const cache = new FixedArray<String>(JSONWriter.PADDING_CACHE_SIZE)
            for (let i = 0; i < JSONWriter.PADDING_CACHE_SIZE; i++) {
                cache[i] = this.space!.repeat(i)
            }
            this.paddingCache = cache
        }
        return this.paddingCache!
    }

    public constructor(replacer?: (key: String, value: Any) => Any, space?: String, useAnnotatedGetters: boolean = false) {
        this.replacer = replacer
        this.fieldsFilter = []
        this.space = JSONWriter.truncateSpace(space)

        this.useReplacer = this.replacer != undefined
        this.useFieldsFilter = false
        this.useIndent = !(this.space == undefined || this.space == '')
        this.useAnnotatedGetters = useAnnotatedGetters
        this.useSimpleFastPath = !this.useReplacer && !this.useIndent && !this.useAnnotatedGetters
        this.fieldsFilterSet = new Set<string>()
        this.filterNamePos = undefined
    }

    public constructor(filter: FixedArray<string>, space?: String, useAnnotatedGetters: boolean = false) {
        this.replacer = undefined
        this.fieldsFilter = filter
        this.space = JSONWriter.truncateSpace(space)

        this.useReplacer = false
        this.useFieldsFilter = true
        this.useIndent = !(this.space == undefined || this.space == '')
        this.useAnnotatedGetters = useAnnotatedGetters
        this.useSimpleFastPath = false

        const filterSet = new Set<string>()
        const namePos = new Map<string, int>()
        for (let idx = 0; idx < filter.length; idx++) {
            filterSet.add(filter[idx])
            namePos.set(filter[idx], idx)
        }
        this.fieldsFilterSet = filterSet
        this.filterNamePos = namePos
    }

    public write(obj: Any): String {
        if (this.useReplacer) {
            this.writeObject(this.replacer!('', obj))
        } else {
            this.writeObject(obj)
        }

        return this.buffer.toString()
    }

    public writeFast(obj: Any): String {
        this.writeObjectFast(obj)
        return this.buffer.toString()
    }

    private writeObjectFast(obj: Any): void {
        if (obj === null) {
            this.buffer.append('null')
        } else if (obj === undefined) {
            this.buffer.append('')
        } else if (obj instanceof String) {
            this.writeStringDirect(obj)
        } else if (obj instanceof Tuple) {
            this.writeTupleValue(obj)
        } else if (obj instanceof Record) {
            this.writeStdCoreRecord(obj as Record<Numeric | string | BaseEnum<Int> | BaseEnum<Long> |
                BaseEnum<string> | BaseEnum<Float> | BaseEnum<Double> | BaseEnum<Byte> | BaseEnum<Short>, Any>)
        } else if (obj instanceof JsonReplacer) {
            const record = obj.jsonReplacer()
            this.writeStdCoreRecord(record)
        } else if (obj instanceof ESValue) {
            this.writeInteropESValue(obj)
        } else if (obj instanceof jsonx.JsonElementSerializable) {
            this.buffer.append(JSON.stringifyJsonElement(obj))
        } else if (obj instanceof JsonSerializable) {
            this.buffer.append('"'.concat(obj.toJSON(), '"'))
        } else if (obj instanceof ArrayBuffer || obj instanceof DataView) {
            this.buffer.append(JSONObject.EMPTY)
        } else if (obj instanceof Function) {
            this.buffer.append('')
        } else if (this.writeValueTypeWrapper(obj)) {
            // nothing to do - write completed successfully
        } else if (obj instanceof Date) {
            this.writeObject(obj.toJSON())
        } else if (obj instanceof Array) {
            this.writeStdCoreArrayFast(obj)
        } else if (obj instanceof Int8Array || obj instanceof Int16Array || obj instanceof Int32Array
                || obj instanceof Uint8Array || obj instanceof BigInt64Array || obj instanceof Uint16Array
                || obj instanceof Uint32Array || obj instanceof Float32Array || obj instanceof Float64Array
                || obj instanceof BigUint64Array || obj instanceof Uint8ClampedArray) {
            this.writeTypedArray(obj)
        } else if (obj instanceof Set || obj instanceof Promise || obj instanceof Map || obj instanceof RegExp) {
            this.buffer.append(JSONObject.EMPTY)
        } else if (obj instanceof JSValue) {
            this.buffer.append(JSON.stringify(obj))
        } else if (obj instanceof Object) {
            const objCls = Class.of(obj)
            if (objCls.isFixedArray()) {
                this.writeFixedArrayObjectFast(obj as FixedArray<Any>)
            } else if (objCls.isEnum()) {
                this.writeEnumValue(obj, objCls)
            } else {
                if (FieldMetadataCache.hasFastObjectLayout(objCls)) {
                    this.writeClassValueFast(obj, FieldMetadataCache.getObjectLayout(objCls))
                } else {
                    this.writeObject(obj)
                }
            }
        } else {
            this.writeObject(obj)
        }
    }

    private writeStringDirect(str: String): void {
        let len = str.getLength()
        let needsEscape = false
        for (let i = 0; i < len; i++) {
            let code = str.charCodeAt(i)
            if (code < 32 || code == 34 || code == 92 || code == 127 ||
                (code >= 0xD800 && code <= 0xDFFF)) {
                needsEscape = true
                break
            }
        }

        if (!needsEscape) {
            this.buffer.append('"')
            this.buffer.append(str)
            this.buffer.append('"')
            return
        }

        this.buffer.append('"')

        let begin = 0
        let i = 0

        while (i < len) {
            const ch = str.charAt(i)
            const code = ch.toInt()

            if (code < 128) {
                switch (ch) {
                    case c'"':
                        // Copy characters before escape
                        if (begin < i) {
                            this.buffer.append(str.substring(begin, i))
                        }
                        this.buffer.append('\\"')
                        begin = i + 1
                        break
                    case c'\\':
                        if (begin < i) {
                            this.buffer.append(str.substring(begin, i))
                        }
                        this.buffer.append('\\\\')
                        begin = i + 1
                        break
                    case c'\b':
                        if (begin < i) {
                            this.buffer.append(str.substring(begin, i))
                        }
                        this.buffer.append('\\b')
                        begin = i + 1
                        break
                    case c'\f':
                        if (begin < i) {
                            this.buffer.append(str.substring(begin, i))
                        }
                        this.buffer.append('\\f')
                        begin = i + 1
                        break
                    case c'\n':
                        if (begin < i) {
                            this.buffer.append(str.substring(begin, i))
                        }
                        this.buffer.append('\\n')
                        begin = i + 1
                        break
                    case c'\r':
                        if (begin < i) {
                            this.buffer.append(str.substring(begin, i))
                        }
                        this.buffer.append('\\r')
                        begin = i + 1
                        break
                    case c'\t':
                        if (begin < i) {
                            this.buffer.append(str.substring(begin, i))
                        }
                        this.buffer.append('\\t')
                        begin = i + 1
                        break
                    default:
                        if (code < 0x20 || code == 0x7F) {
                            // Control character -> \uXXXX
                            if (begin < i) {
                                this.buffer.append(str.substring(begin, i))
                            }
                            this.buffer.append('\\u')
                            this.buffer.append(code.toString(16).padStart(4, '0'))
                            begin = i + 1
                        }
                        break
                }
                i++
            } else if (code >= 0xD800 && code <= 0xDFFF) {
                // Surrogate pair handling
                if (code >= 0xD800 && code <= 0xDBFF) {
                    // High surrogate
                    if (i + 1 < len) {
                        const nextCode = str.charCodeAt(i + 1).toInt()
                        if (nextCode >= 0xDC00 && nextCode <= 0xDFFF) {
                            // Valid surrogate pair - copy both
                            i += 2
                        } else {
                            // Lone high surrogate -> \uXXXX
                            if (begin < i) {
                                this.buffer.append(str.substring(begin, i))
                            }
                            this.buffer.append('\\u')
                            this.buffer.append(code.toString(16).padStart(4, '0'))
                            begin = i
                            i++
                        }
                    } else {
                        i++
                    }
                } else {
                    // Low surrogate (0xDC00-0xDFFF) - lone
                    if (begin < i) {
                        this.buffer.append(str.substring(begin, i))
                    }
                    this.buffer.append('\\u')
                    this.buffer.append(code.toString(16).padStart(4, '0'))
                    begin = i + 1
                    i++
                }
            } else {
                i++
            }
        }

        if (begin < len) {
            this.buffer.append(str.substring(begin, len))
        }

        this.buffer.append('"')
    }

    private writeObject(obj: Any): void {
        if (obj === null) {
            this.buffer.append('null')
        } else if (obj === undefined) {
            this.buffer.append('')
        } else if (obj instanceof String) {
            this.writeStringDirect(obj)
        } else if(obj instanceof Tuple) {
            this.writeTupleValue(obj);
        } else if (obj instanceof Record) {
            this.writeStdCoreRecord(obj as Record<Numeric | string | BaseEnum<Int> | BaseEnum<Long> |
                BaseEnum<string> | BaseEnum<Float> | BaseEnum<Double> | BaseEnum<Byte> | BaseEnum<Short>, Any>)
        } else if (obj instanceof JsonReplacer) {
            let temp: Record<String, Any> = obj.jsonReplacer()
            this.writeStdCoreRecord(temp)
        } else if (obj instanceof ESValue) {
            this.writeInteropESValue(obj)
        } else if (obj instanceof JsonSerializable) {
            this.buffer.append('"'.concat(obj.toJSON(), '"'))
        } else if (obj instanceof jsonx.JsonElementSerializable) {
            this.buffer.append(JSON.stringifyJsonElement(obj))
        } else if (obj instanceof ArrayBuffer || obj instanceof DataView) {
            this.buffer.append(JSONObject.EMPTY)
        } else if (obj instanceof Function) {
            this.buffer.append('')
        } else if (this.writeValueTypeWrapper(obj)) {
            // nothing to do - write completed successfully
        } else if (obj instanceof Date) {
            this.writeObject(obj.toJSON())
        } else if (obj instanceof Array) {
            this.writeStdCoreArray(obj)
        } else if (obj instanceof Int8Array || obj instanceof Int16Array || obj instanceof Int32Array
                || obj instanceof Uint8Array || obj instanceof BigInt64Array || obj instanceof Uint16Array
                || obj instanceof Uint32Array || obj instanceof Float32Array || obj instanceof Float64Array
                || obj instanceof BigUint64Array || obj instanceof Uint8ClampedArray) {
            this.writeTypedArray(obj)
        } else if (obj instanceof Set || obj instanceof Promise || obj instanceof Map || obj instanceof RegExp) {
            this.buffer.append(JSONObject.EMPTY)
        } else if (obj instanceof JSValue) {
            this.buffer.append(JSON.stringify(obj))
        } else if (obj instanceof Object) {
            const objCls = Class.of(obj)
            if (objCls.isFixedArray()) {
                this.writeFixedArrayObject(obj as FixedArray<Any>)
            } else if (objCls.isEnum()) {
                this.writeEnumValue(obj, objCls)
            } else {
                this.writeClassValue(obj, objCls)
            }
        } else {
            const objClass = Class.ofAny(obj)
            const message = objClass ? objClass.getName() : 'value has no class metadata'
            throw new TypeError('Unsupported object type: ' + message)
        }
    }

    private writeEnumValue(obj: Object, cls: Class) {
        const valueOfMethod = reflect.getInstanceMethodsRecursiveByName(cls, "valueOf")
        if (valueOfMethod !== undefined) {
            const value: Object = valueOfMethod.invoke(obj) as Object
            if (value instanceof String) {
                this.writeStringDirect(value)
            } else {
                this.buffer.append(value)
            }
        }
    }

    private writeTupleValue(obj: Object) {
        const currentBuffer = this.buffer
        this.buffer = new StringBuilder('[')

        if (this.useIndent) {
            this.buffer.append('\n')
            this.indentLevel += 1
        }

        const tupleEntries = FieldMetadataCache.getTupleEntries(Class.of(obj))

        let fieldDumped = false
        this.path.add(obj)
        fieldDumped = this.writeTupleElements(tupleEntries, obj)
        this.path.delete(obj)

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append('\n')
        }

        this.writePadding()
        this.buffer.append(']')

        if (fieldDumped) {
            currentBuffer.append(this.buffer.toString())
        } else {
            currentBuffer.append('[]')
        }

        this.buffer = currentBuffer
    }

    private writeTupleElements(tupleEntries: Array<FieldMetadata>, obj: object): boolean {
        let fieldDumped = false
        const tupleEntriesLen = tupleEntries.length
        for (let fieldIdx = 0; fieldIdx < tupleEntriesLen; fieldIdx++) {
            const objToDump = tupleEntries[fieldIdx].getValue(obj)
            fieldDumped = this.writeEntryPrefix(fieldDumped, true, this.useIndent)

            if (this.useIndent) {
                this.buffer.append(' ')
            }

            if (objToDump != null && !(objToDump instanceof Function)) {
                this.writeObject(objToDump)
            } else {
                this.buffer.append('null')
            }
        }

        return fieldDumped
    }

    private writeStdCoreRecord<K extends Numeric | string | BaseEnum<Int> | BaseEnum<Long> | BaseEnum<string> | BaseEnum<Float> | BaseEnum<Double> | BaseEnum<Byte> | BaseEnum<Short>, V>(record: Record<K, V>): void {
        const currentBuffer = this.buffer
        this.buffer = new StringBuilder('{')

        if (this.useIndent) {
            this.buffer.append('\n')
            this.indentLevel += 1
        }

        this.path.add(record)

        let entryDumped = false
        if (this.useFieldsFilter) {
            entryDumped = this.writeFilteredRecordEntries(record, entryDumped)
        } else {
            record.forEach((entryVal: V, entryKey: K) => {
                entryDumped = this.writeRecordEntry(entryKey, entryVal, entryDumped)
            })
        }

        this.path.delete(record)

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append('\n')
        }

        this.writePadding()
        this.buffer.append('}')

        currentBuffer.append(entryDumped ? this.buffer.toString() : JSONObject.EMPTY)

        this.buffer = currentBuffer
    }

    private writeFilteredRecordEntries<K extends Numeric | string | BaseEnum<Int> | BaseEnum<Long> | BaseEnum<string> | BaseEnum<Float> | BaseEnum<Double> | BaseEnum<Byte> | BaseEnum<Short>, V>(record: Record<K, V>, entryDumped: boolean): boolean {
        const matchedEntries = new FixedArray<[K, V] | undefined>(this.fieldsFilter.length)

        record.forEach((entryVal: V, entryKey: K) => {
            const entryName = entryKey.toString()
            const entryPos = this.filterNamePos!.get(entryName)
            if (entryPos !== undefined) {
                matchedEntries[entryPos] = [entryKey, entryVal]
            }
        })

        const matchedEntriesLen = matchedEntries.length
        for (let i = 0; i < matchedEntriesLen; i++) {
            const matchedEntry = matchedEntries[i]
            if (matchedEntry === undefined) {
                continue
            }
            entryDumped = this.writeRecordEntry(matchedEntry[0], matchedEntry[1], entryDumped)
        }

        return entryDumped
    }

    private writeRecordEntry<K extends Numeric | string | BaseEnum<Int> | BaseEnum<Long> | BaseEnum<string> | BaseEnum<Float> | BaseEnum<Double> | BaseEnum<Byte> | BaseEnum<Short>, V>(entryKey: K, entryVal: V, entryDumped: boolean): boolean {
        const entryName = entryKey.toString()
        const replacedVal = this.useReplacer ? this.replacer!(entryName, entryVal) : entryVal
        return this.writeField(entryName, replacedVal, entryDumped)
    }

    private writeInteropESValue(obj: ESValue): void {
        try {
            const json = JSON.stringify(obj.unwrap())
            this.buffer.append(json.toString())
        } catch (e) {
            if (e instanceof TypeError) {
                throw new TypeError('cyclic object value')
            } else {
                throw e
            }
        }
    }

    private writeClassValue(obj: Object, objCls: Class): void {
        const currentBuffer = this.buffer
        // we need this local buffer to discard empty object prolog: '{','\n',<space> and replace it with '{}'
        this.buffer = new StringBuilder('{')

        if (this.useIndent) {
            this.buffer.append('\n')
            this.indentLevel += 1
        }

        let fieldDumped = false
        if (this.useFieldsFilter) {
            this.path.add(obj)
            fieldDumped = this.writeFilteredClassFields(objCls, obj, fieldDumped)
            this.path.delete(obj)
        } else {
            const objectLayout = FieldMetadataCache.getObjectLayout(objCls)
            const fastObjectLayout = FieldMetadataCache.hasFastObjectLayout(objCls)
            fieldDumped = this.writeClassLayoutEntries(objectLayout, fastObjectLayout, obj, fieldDumped)
        }

        if (this.useAnnotatedGetters) {
            const annotatedGetters = JSONWriter.getGetterKeyValuePairsAll(
                objCls,
                obj,
                this.useFieldsFilter,
                this.fieldsFilterSet!
            )
            this.path.add(obj)
            fieldDumped = this.writeClassFields(annotatedGetters, fieldDumped)
            this.path.delete(obj)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append('\n')
        }

        this.writePadding()
        this.buffer.append('}')

        currentBuffer.append(fieldDumped ? this.buffer.toString() : JSONObject.EMPTY)

        this.buffer = currentBuffer
    }

    private writeField(fieldName: string, fieldVal: Any, fieldDumped: boolean): boolean {
        return this.writeFieldImpl(fieldName, fieldVal, fieldDumped, true, this.useIndent, this.useSimpleFastPath)
    }

    private writeClassFields(writableFields: Array<[string, Any]>, fieldDumped: boolean): boolean {
        const FIELD_NAME = 0
        const FIELD_VALUE = 1
        const writableFieldsLen = writableFields.length

        for (let fieldIdx = 0; fieldIdx < writableFieldsLen; fieldIdx++) {
            const fieldTypeValuePair = writableFields[fieldIdx]

            const objFieldName = fieldTypeValuePair[FIELD_NAME]
            const objFieldValue = fieldTypeValuePair[FIELD_VALUE]

            const objToDump = this.useReplacer ? this.replacer!(objFieldName, objFieldValue) : objFieldValue
            fieldDumped = this.writeField(objFieldName, objToDump, fieldDumped)
        }

        return fieldDumped
    }

    private writeClassLayoutEntries(entries: Array<CachedObjectLayoutEntry>, fastObjectLayout: boolean,
        obj: Object, fieldDumped: boolean): boolean {
        if (entries.length == 0) {
            return fieldDumped
        }

        let writePadding = false
        let newlineAfterComma = false
        let useFastWriter = false
        let useReplacer = false

        if (this.useReplacer) {
            writePadding = true
            newlineAfterComma = this.useIndent
            useReplacer = true
        } else if (this.useIndent) {
            writePadding = true
            newlineAfterComma = true
        } else if (fastObjectLayout) {
            useFastWriter = true
        }

        this.path.add(obj)
        fieldDumped = this.writeClassLayoutFields(entries, obj, fieldDumped, writePadding, newlineAfterComma,
            useFastWriter, useReplacer)
        this.path.delete(obj)
        return fieldDumped
    }

    private writeClassValueFast(obj: Object, entries: Array<CachedObjectLayoutEntry>): void {
        const currentBuffer = this.buffer
        this.buffer = new StringBuilder('{')

        let fieldDumped = false
        this.path.add(obj)
        fieldDumped = this.writeClassLayoutFields(entries, obj, fieldDumped, false, false, true, false)
        this.path.delete(obj)

        this.buffer.append('}')
        currentBuffer.append(fieldDumped ? this.buffer.toString() : JSONObject.EMPTY)
        this.buffer = currentBuffer
    }

    private writeEntryPrefix(fieldDumped: boolean, writePadding: boolean, newlineAfterComma: boolean): boolean {
        if (fieldDumped) {
            this.buffer.append(',')
            if (newlineAfterComma) {
                this.buffer.append('\n')
            }
        } else {
            fieldDumped = true
        }

        if (writePadding) {
            this.writePadding()
        }

        return fieldDumped
    }

    private writeFieldImpl(fieldName: string, fieldVal: Any, fieldDumped: boolean, writePadding: boolean,
        newlineAfterComma: boolean, useFastWriter: boolean): boolean {
        if (fieldVal === undefined || fieldVal instanceof Function) {
            return fieldDumped
        }

        fieldDumped = this.writeEntryPrefix(fieldDumped, writePadding, newlineAfterComma)
        this.checkReferencesCycle(fieldVal)
        const fieldPrefix = writePadding && this.useIndent
            ? '"'.concat(fieldName, '": ')
            : '"'.concat(fieldName, '":')
        this.buffer.append(fieldPrefix)
        if (useFastWriter) {
            this.writeObjectFast(fieldVal)
        } else {
            this.writeObject(fieldVal)
        }
        return fieldDumped
    }

    private writeClassLayoutFields(entries: Array<CachedObjectLayoutEntry>, obj: object, fieldDumped: boolean,
        writePadding: boolean, newlineAfterComma: boolean, useFastWriter: boolean, useReplacer: boolean): boolean {
        const entriesLen = entries.length
        for (let fieldIdx = 0; fieldIdx < entriesLen; fieldIdx++) {
            const entry = entries[fieldIdx]
            const fieldVal = entry.metadata.field.getValue(obj)
            const objToDump = useReplacer ? this.replacer!(entry.name, fieldVal) : fieldVal
            fieldDumped = this.writeFieldImpl(entry.name, objToDump, fieldDumped, writePadding, newlineAfterComma,
                useFastWriter)
        }

        return fieldDumped
    }

    private writeFilteredClassFields(cls: Class, obj: object, fieldDumped: boolean): boolean {
        const writtenNames = this.writtenNamesBuf
        writtenNames.clear()
        const fieldsFilterLen = this.fieldsFilter.length

        for (let fieldIdx = 0; fieldIdx < fieldsFilterLen; fieldIdx++) {
            const fieldName = this.fieldsFilter[fieldIdx]
            const metadata = FieldMetadataCache.getMetadataByName(cls, fieldName)
            if (metadata === undefined || metadata.ignore) {
                continue
            }

            if (metadata.rename !== undefined && FieldMetadataCache.hasMetadataByName(cls, metadata.rename!)) {
                throw new Error(`Double usage of \`${metadata.rename}\` in ${cls.getName()}.` +
                    ' The name also is used by JSONRename in base class')
            }
            const writableFieldName = metadata.rename !== undefined
                ? validateWritableFieldName(cls, metadata, writtenNames)
                : metadata.name
            if (metadata.rename !== undefined) {
                writtenNames.add(writableFieldName)
            }

            const objFieldValue = metadata.getValue(obj)
            const objToDump = this.useReplacer ? this.replacer!(writableFieldName, objFieldValue) : objFieldValue
            fieldDumped = this.writeField(writableFieldName, objToDump, fieldDumped)
        }

        return fieldDumped
    }

    /**
    * @brief Returns values of `@JSONStringifyGetter`-annotated `get` accessor properties
    * across the class hierarchy.
    * A method is included only if it both carries the `@JSONStringifyGetter` annotation
    * AND has the `%%get-` bytecode name prefix (length 6) that identifies a `get` accessor.
    * The prefix is stripped to produce the JSON key (e.g. `%%get-area` → key `"area"`).
    */
    private static getGetterKeyValuePairsAll(cls: Class, obj: Object, useFieldsFilter: boolean,
        fieldsFilterSet: Set<string>): Array<[string, Any]> {

        const namesAndValues = new Array<[string, Any]>()
        const getterMetadata = JSONWriter.getGetterMetadata(cls)
        const getterMetadataLen = getterMetadata.length
        for (let i = 0; i < getterMetadataLen; i++) {
            const getter = getterMetadata[i]
            const name = getter[0]
            if (useFieldsFilter && !JSONWriter.searchField(fieldsFilterSet, name)) {
                continue
            }
            const value = getter[1].invoke(obj)
            namesAndValues.push([name, value])
        }
        return namesAndValues
    }

    private static getGetterMetadata(cls: Class): Array<[string, reflect.InstanceMethod]> {
        const cachedMetadata = JSONWriter.getterMetadataCache.get(cls)
        if (cachedMetadata !== undefined) {
            return cachedMetadata
        }

        const getterMetadata = new Array<[string, reflect.InstanceMethod]>()
        let currentClass: Class | undefined = cls
        while (currentClass != undefined) {
            const clsMethods = currentClass.getInstanceMethods()
            for (const method of clsMethods) {
                if (!method.isGetter()) {
                    continue
                }
                if (!JSONAPI.getJSONStringifyGetter(method)) {
                    continue
                }
                const name = method.getName().substring(JSONWriter.GETTER_PREFIX_LEN)
                getterMetadata.push([name, method])
            }
            currentClass = currentClass.getSuper()
        }

        JSONWriter.getterMetadataCache.set(cls, getterMetadata)
        return getterMetadata
    }

    private static searchField(fieldsFilterSet: Set<string>, name: string): boolean {
        return fieldsFilterSet.has(name)
    }

    private writeArrayPreamble(): void {
        this.buffer.append('[')
        if (this.useIndent) {
            this.buffer.append('\n')
            this.indentLevel += 1
        }
    }

    private writeArrayEpilogue(): void {
        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append('\n')
        }
        this.writePadding()
        this.buffer.append(']')
    }

    private writeFixedArrayLikeCore<E>(array: FixedArray<E>, useFastElementWriter: boolean): void {
        this.writeArrayPreamble()

        const arrayLength = array.length
        if (arrayLength > 0) {
            this.path.add(array)
            const lastElementIndex = arrayLength - 1
            for (let idx = 0; idx < lastElementIndex; idx++) {
                if (useFastElementWriter) {
                    this.writeObjectsArrayElementFast(array[idx], idx)
                } else {
                    this.writeObjectsArrayElement(array[idx], idx)
                }
                this.buffer.append(',')
                if (this.useIndent) {
                    this.buffer.append('\n')
                }
            }
            if (useFastElementWriter) {
                this.writeObjectsArrayElementFast(array[lastElementIndex], lastElementIndex)
            } else {
                this.writeObjectsArrayElement(array[lastElementIndex], lastElementIndex)
            }
            this.path.delete(array)
        }

        this.writeArrayEpilogue()
    }

    private writeFixedArrayObject<E>(array: FixedArray<E>): void {
        this.writeFixedArrayLikeCore(array, false)
    }

    private writeFixedArrayObjectFast<E>(array: FixedArray<E>): void {
        this.writeFixedArrayLikeCore(array, true)
    }

    private writeValueTypeWrapper(obj: Any): boolean {
        if (obj instanceof Boolean) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Byte) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Char) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Short) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Int) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Long) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Float) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof Double) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else if (obj instanceof BigInt) {
            this.buffer.append(JSON.stringify(obj))
            return true
        } else {
            return false
        }
    }

    private writeTypedArray(array: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array |
        Uint32Array | Float32Array | Float64Array | BigInt64Array | BigUint64Array | Uint8ClampedArray): void {
        this.buffer.append('{')

        if (this.useIndent) {
            this.buffer.append('\n')
            this.indentLevel += 1
        }

        if (array.length > 0) {
            this.path.add(array)

            const isBigIntArray = array instanceof BigInt64Array || array instanceof BigUint64Array
            const lastElementIndex: int = array.length - 1

            for (let idx: int = 0; idx < lastElementIndex; idx++) {
                this.buffer.append('"'.concat(idx.toString(), '":'))

                if (isBigIntArray) {
                    this.writeObjectsArrayElement((array as ArrayLike<bigint>)[idx], idx)
                } else {
                    this.writeObjectsArrayElement((array as ArrayLike<Double>)[idx], idx)
                }
                this.buffer.append(',')

                if (this.useIndent) {
                    this.buffer.append('\n')
                }
            }

            this.buffer.append('"'.concat(lastElementIndex.toString(), '":'))

            if (isBigIntArray) {
                this.writeObjectsArrayElement((array as ArrayLike<bigint>)[lastElementIndex], lastElementIndex)
            } else {
                this.writeObjectsArrayElement((array as ArrayLike<Double>)[lastElementIndex], lastElementIndex)
            }

            this.path.delete(array)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append('\n')
        }

        this.writePadding()

        this.buffer.append('}')
    }

    private writeStdCoreArrayCore<T>(array: Array<T>, arrayLength: int, useFastElementWriter: boolean): void {
        this.writeArrayPreamble()

        if (arrayLength > 0) {
            this.path.add(array)
            const lastElementIndex = arrayLength - 1
            for (let idx = 0; idx < lastElementIndex; idx++) {
                if (useFastElementWriter) {
                    this.writeObjectsArrayElementFast(array[idx], idx)
                } else {
                    this.writeObjectsArrayElement(array[idx], idx)
                }
                this.buffer.append(',')
                if (this.useIndent) {
                    this.buffer.append('\n')
                }
            }
            if (useFastElementWriter) {
                this.writeObjectsArrayElementFast(array[lastElementIndex], lastElementIndex)
            } else {
                this.writeObjectsArrayElement(array[lastElementIndex], lastElementIndex)
            }
            this.path.delete(array)
        }

        this.writeArrayEpilogue()
    }

    private writeStdCoreArray<T>(array: Array<T>): void {
        this.writeStdCoreArrayCore(array, array.length, false)
    }

    private writeStdCoreArrayFast<T>(array: Array<T>): void {
        this.writeStdCoreArrayCore(array, array.length, true)
    }

    private writeObjectsArrayElement(arrayElement: Any, elementIndex: int): void {
        this.writePadding()

        const replacedElem = this.useReplacer ? this.replacer!('' + elementIndex, arrayElement) : arrayElement

        if (replacedElem == null || replacedElem instanceof Function) {
            this.buffer.append('null')
        } else {
            this.checkReferencesCycle(replacedElem)
            if (this.useSimpleFastPath) {
                this.writeObjectFast(replacedElem)
            } else {
                this.writeObject(replacedElem)
            }
        }
    }

    private writeObjectsArrayElementFast(arrayElement: Any, elementIndex: int): void {
        if (this.useIndent) {
            this.writePadding()
        }

        if (arrayElement == null || arrayElement instanceof Function) {
            this.buffer.append('null')
        } else {
            this.checkReferencesCycle(arrayElement)
            this.writeObjectFast(arrayElement)
        }
    }

    private checkReferencesCycle(obj: Any): void {
        if (obj == null) {
            return
        }

        const cycleDetected = this.path.has(obj)
        if (cycleDetected) {
            throw new TypeError('cyclic object value')
        }
    }

    private writePadding() {
        if (this.indentLevel > 0) {
            const cache = this.getPaddingCache()
            const idx = this.indentLevel < JSONWriter.PADDING_CACHE_SIZE
                ? this.indentLevel
                : JSONWriter.PADDING_CACHE_SIZE - 1
            this.buffer.append(cache[idx])
            if (this.indentLevel >= JSONWriter.PADDING_CACHE_SIZE) {
                this.buffer.append(this.space!.repeat(this.indentLevel - JSONWriter.PADDING_CACHE_SIZE + 1))
            }
        }
    }
}

/**
* Internal method to stringify a JsonElement recursively.
* @param {JsonElement} elem - The JsonElement to stringify
* @param {(double | string)[] | undefined} replacer - Array of keys to include (currently not implemented)
* @param {int | string | undefined} space - String or number of spaces for indentation
* @param {number} depth - Current nesting depth for indentation
* @returns {string} The JSON string representation
* @private
*/
function stringifyElementRecursively(elem: jsonx.JsonElement, replacer?: (double | string)[], space?: int | string,
    depth: int = 0): string {

    const indentStr = getIndentation(space, depth)
    const nextIndentStr = getIndentation(space, depth + 1)
    const hasIndentation = space != undefined && nextIndentStr.length > 0

    switch (elem.jsonType) {
        case jsonx.JsonType.JsonNull:
            return 'null'

        case jsonx.JsonType.JsonUndefined:
            return ''

        case jsonx.JsonType.JsonTrue:
            return 'true'

        case jsonx.JsonType.JsonFalse:
            return 'false'

        case jsonx.JsonType.JsonString:
            return EscapeStringBuilder.process(elem.asString())

        case jsonx.JsonType.JsonNumber:
            const longValue = elem.tryAsLong()
            if (longValue != undefined) {
                return longValue.toString()
            }
            const doubleValue = elem.tryAsDouble()
            if (doubleValue != undefined) {
                return doubleValue.toString()
            }
            const bigintValue = elem.tryAsBigInt()
            if (bigintValue != undefined) {
                return bigintValue.toString()
            }
            return 'null'

        case jsonx.JsonType.JsonArray:
            const array = elem.asArray()
            if (array.length == 0) {
                return '[]'
            }
            let arrayResult: string = '['
            if (hasIndentation) {
                arrayResult = arrayResult.concat('\n')
            }
            for (let i = 0; i < array.length; i++) {
                if (hasIndentation) {
                    arrayResult = arrayResult.concat(nextIndentStr)
                }
                arrayResult = arrayResult.concat(stringifyElementRecursively(array[i], replacer, space, depth + 1))
                if (i < array.length - 1) {
                    arrayResult = arrayResult.concat(',')
                }
                if (hasIndentation) {
                    arrayResult = arrayResult.concat('\n')
                }
            }
            if (hasIndentation) {
                arrayResult = arrayResult.concat(indentStr)
            }
            arrayResult = arrayResult.concat(']')
            return arrayResult

        case jsonx.JsonType.JsonObject:
            let objectResult: string = '{'
            let hasEntries = false
            for (const entry of elem.$_iterator()) {
                const key = entry[0]
                const value = entry[1]
                if (value.jsonType == jsonx.JsonType.JsonUndefined) {
                    continue
                }
                if (hasEntries) {
                    objectResult = objectResult.concat(',')
                    if (hasIndentation) {
                        objectResult = objectResult.concat('\n')
                    }
                } else {
                    hasEntries = true
                    if (hasIndentation) {
                        objectResult = objectResult.concat('\n')
                    }
                }
                if (hasIndentation) {
                    objectResult = objectResult.concat(nextIndentStr)
                }
                objectResult = objectResult.concat(EscapeStringBuilder.process(key))
                objectResult = objectResult.concat(':')
                if (hasIndentation) {
                    objectResult = objectResult.concat(' ')
                }
                objectResult = objectResult.concat(stringifyElementRecursively(value, replacer, space, depth + 1))
            }
            if (!hasEntries) {
                return '{}'
            }
            if (hasIndentation) {
                objectResult = objectResult.concat('\n')
                objectResult = objectResult.concat(indentStr)
            }
            objectResult = objectResult.concat('}')
            return objectResult

        default:
            return 'null'
    }
}

class EscapeStringBuilder {
    public static escapeTable: FixedArray<string> = EscapeStringBuilder.createEscapeTable()
    private static controlCharsRange: int = 32
    private static delCode: int = 127

    private static createEscapeTable(): FixedArray<string> {
        const asciiRange: int = 128
        const table: FixedArray<string> = new FixedArray<string>(asciiRange)
        for (let code = 0; code < asciiRange; code++) {
            switch (code.toChar()) {
                case c'"':
                    table[code] = '\\"'
                    break
                case c'\\':
                    table[code] = '\\\\'
                    break
                case c'\b':
                    table[code] = '\\b'
                    break
                case c'\f':
                    table[code] = '\\f'
                    break
                case c'\n':
                    table[code] = '\\n'
                    break
                case c'\r':
                    table[code] = '\\r'
                    break
                case c'\t':
                    table[code] = '\\t'
                    break
                default:
                    if (code < EscapeStringBuilder.controlCharsRange || code == EscapeStringBuilder.delCode) {
                        table[code] = '\\u'.concat(code.toString(16).padStart(4, '0'))
                    }
                    break
            }
        }
        return table
    }

    public static process(str: string): string {
        let result: string = '"'
        let begin: int = 0
        for (let i = 0; i < str.length; i++) {
            const ch = str.charAt(i)
            const code = ch.toInt()
            if (code >= EscapeStringBuilder.controlCharsRange && ch != c'"' && ch != c'\\' &&
                code != EscapeStringBuilder.delCode) {
                continue
            }
            if (begin < i) {
                result = result.concat(str.substring(begin, i))
            }
            result = result.concat(EscapeStringBuilder.escapeTable[code])
            begin = i + 1
        }
        if (begin < str.length) {
            result = result.concat(str.substring(begin))
        }
        return result.concat('"')
    }
}

const INDENT_CACHE_SIZE: int = 20
let indentCache: FixedArray<string> | undefined = undefined
let indentCacheUnit: string | undefined = undefined

function buildIndentCache(unit: string): FixedArray<string> {
    const cache = new FixedArray<string>(INDENT_CACHE_SIZE)
    for (let i = 0; i < INDENT_CACHE_SIZE; i++) {
        cache[i] = unit.repeat(i)
    }
    indentCache = cache
    indentCacheUnit = unit
    return cache
}

function getIndentation(space?: int | string, depth: int = 0): string {
    if (space == undefined || depth == 0) {
        return ''
    }
    let unit: string
    if (space instanceof string) {
        unit = space.substring(0, 10)
    } else {
        let n = (space as int) > 0 ? (space as int) : 0
        unit = ' '.repeat(n > 10 ? 10 : n)
    }
    if (unit.length == 0) {
        return ''
    }
    const cache = indentCache !== undefined && indentCacheUnit == unit ? indentCache! : buildIndentCache(unit)
    if (depth < INDENT_CACHE_SIZE) {
        return cache[depth]
    }
    return unit.repeat(depth)
}


class TypeColor {
    public readonly static VISITED : int = 1
    public readonly static AT_STACK : int = 2

    public readonly static STRING_CLASS = Class.of('')

    public readonly static OBJECT_CLASS = Class.from<Object>()
    public readonly static NULL_CLASS = Class.from<null>()
    public readonly static ARRAY_CLASS = Class.from<Array<object>>()

    type: Class
    color: int
    public constructor(type: Class, color: int) {
        this.type = type
        this.color = color
    }

}

class UnifiedJsonParser {
    private static readonly INVALID_CACHE_INDEX: int = -2
    private static readonly CACHE_NOT_FOUND: int = -1
    private source: string
    private index: int
    private length: int
    private ch: char
    private reviver?: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement
    private bigIntOpt: int = 0
    private nextBsIdx: int = UnifiedJsonParser.INVALID_CACHE_INDEX
    private nextQuoteIdx: int = UnifiedJsonParser.INVALID_CACHE_INDEX

    public constructor(source: string, options?: jsonx.ParseOptions) {
        this.source = source
        this.index = 0
        this.length = source.length
        this.ch = this.length > 0 ? source.charAt(0) : c'\0'
        this.bigIntOpt = options != undefined ? options.bigIntMode : 0
    }

    public constructor(source: string, reviver: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement,
        options?: jsonx.ParseOptions) {

        this.source = source
        this.index = 0
        this.length = source.length
        this.ch = this.length > 0 ? source.charAt(0) : c'\0'
        this.reviver = reviver
        this.bigIntOpt = options != undefined ? options.bigIntMode : 0
    }

    private getPositionDesc(): string {
        return `${this.index}`
    }

    private updateCh(): void {
        this.ch = this.index < this.length ? this.source.charAt(this.index) : c'\0'
    }

    private advance(): void {
        this.index++
        this.updateCh()
    }

    private advanceBy(n: int): void {
        this.index += n
        this.updateCh()
    }

    public parse(): jsonx.JsonElement {
        this.skipWhitespace()
        if (this.index >= this.length) {
            throw new JsonParseError("Unexpected end of input", 0, 0)
        }
        let result = this.parseValue()
        if (this.reviver != undefined) {
            return this.reviver!("", result)
        }
        return result
    }

    public parseRecord(): Record<string, JsonRecordType> {
        this.skipWhitespace()
        if (this.index >= this.length) {
            throw new JsonParseError("Unexpected end of input", 0, 0)
        }
        let firstChar = this.ch
        if (firstChar != c'{') {
            throw new JsonParseError("JSON Record root must be an object (starting with '{')", 0, 0)
        }
        return this.parseObjectRaw()
    }

    private parseObjectRaw(): Record<string, JsonRecordType> {
        this.advance()
        this.skipWhitespace()
        let result: Record<string, JsonRecordType> = new Record<string, JsonRecordType>()

        if (this.ch == c'}') {
            this.advance()
            return result
        }

        while (true) {
            this.skipWhitespace()
            let key = this.parseStringContent()

            this.skipWhitespace()
            if (this.ch != c':') {
                throw new JsonParseError(`Expected ':' at ${this.getPositionDesc()}`,
                    this.index, this.index)
            }
            this.advance()
            this.skipWhitespace()

            let value = this.parseValueRaw()
            result.set(key, value)

            this.skipWhitespace()
            if (this.ch == c',') {
                this.advance()
            } else if (this.ch == c'}') {
                this.advance()
                return result
            } else {
                throw new JsonParseError(`Expected ',' or '}' at ${this.getPositionDesc()}`,
                    this.index, this.index)
            }
        }
    }

    private parseValueRaw(): JsonRecordType {
        let idx = this.index
        let ch = this.ch
        const len = this.length
        const src = this.source
        if (ch == c'n' && idx + 3 < len &&
            src.charAt(idx + 1) == c'u' && src.charAt(idx + 2) == c'l' && src.charAt(idx + 3) == c'l') {
            this.advanceBy(4)
            return null
        }
        if (ch == c't' && idx + 3 < len &&
            src.charAt(idx + 1) == c'r' && src.charAt(idx + 2) == c'u' && src.charAt(idx + 3) == c'e') {
            this.advanceBy(4)
            return true
        }
        if (ch == c'f' && idx + 4 < len &&
            src.charAt(idx + 1) == c'a' && src.charAt(idx + 2) == c'l' &&
            src.charAt(idx + 3) == c's' && src.charAt(idx + 4) == c'e') {
            this.advanceBy(5)
            return false
        }
        if ((ch == c'n' || ch == c't' || ch == c'f')) {
            throw new JsonParseError(`Expected '${ch == c'n' ? "null" : ch == c't' ? "true" : "false"}' at ${this.getPositionDesc()}`, idx, idx + 3)
        }

        switch (this.ch) {
            case c'"':
                return this.parseStringContent()
            case c'{':
                return this.parseObjectRaw()
            case c'[':
                return this.parseArrayRaw()
            case c'-':
            case c'0': case c'1': case c'2': case c'3': case c'4':
            case c'5': case c'6': case c'7': case c'8': case c'9':
                return this.parseNumberRaw()
            default:
                throw new JsonParseError(`Unexpected character '${this.ch}' at ${this.getPositionDesc()}`,
                    this.index, this.index)
        }
    }

    private parseNumberRaw(): bigint | Long | Double {
        let start = this.index
        let hasFraction = false
        let hasExponent = false

        if (this.ch == c'-') {
            this.advance()
        }

        if (this.ch == c'0') {
            this.advance()
        } else if (Char.isDecDigit(this.ch)) {
            while (Char.isDecDigit(this.ch)) {
                this.advance()
            }
        } else {
            throw new JsonParseError(`Invalid number format at ${this.getPositionDesc()}`, start, this.index)
        }

        if (this.ch == c'.') {
            hasFraction = true
            this.advance()
            if (!Char.isDecDigit(this.ch)) {
                throw new JsonParseError(`Invalid number format at ${this.getPositionDesc()}`, start, this.index)
            }
            while (Char.isDecDigit(this.ch)) {
                this.advance()
            }
        }

        if (this.ch == c'e' || this.ch == c'E') {
            hasExponent = true
            this.advance()
            if (this.ch == c'+' || this.ch == c'-') {
                this.advance()
            }
            if (!Char.isDecDigit(this.ch)) {
                throw new JsonParseError(`Invalid number format at ${this.getPositionDesc()}`, start, this.index)
            }
            while (Char.isDecDigit(this.ch)) {
                this.advance()
            }
        }

        let numStr = this.source.substring(start, this.index)
        let isFloat = hasFraction || hasExponent

        if (isFloat) {
            return Double.parseFloat(numStr)
        }

        if (this.bigIntOpt == 2) {
            return BigInt(numStr)
        }
        if (this.bigIntOpt == 1) {
            try {
                return Long.parseInt(numStr, 10)
            } catch (e) {
                if (e instanceof FormatError) {
                    return BigInt(numStr)
                }
                throw e
            }
        }
        return Long.parseInt(numStr, 10)
    }

    private parseArrayRaw(): Array<JsonRecordType> {
        this.advance()
        this.skipWhitespace()

        let array: Array<JsonRecordType> = new Array<JsonRecordType>()
        let idx = 0

        while (this.ch != c']') {
            this.skipWhitespace()
            let value = this.parseValueRaw()
            array.push(value)
            idx++

            this.skipWhitespace()
            if (this.ch == c',') {
                this.advance()
            } else if (this.ch != c']') {
                throw new JsonParseError(`Expected ',' or ']' at ${this.getPositionDesc()}`,
                    this.index, this.index)
            }
        }

        this.advance()
        return array
    }

    private parseValue(): jsonx.JsonElement {
        let idx = this.index
        let ch = this.ch
        const len = this.length
        const src = this.source
        if (ch == c'n' && idx + 3 < len &&
            src.charAt(idx + 1) == c'u' && src.charAt(idx + 2) == c'l' && src.charAt(idx + 3) == c'l') {
            this.advanceBy(4)
            return jsonx.JsonElement.createNull()
        }
        if (ch == c't' && idx + 3 < len &&
            src.charAt(idx + 1) == c'r' && src.charAt(idx + 2) == c'u' && src.charAt(idx + 3) == c'e') {
            this.advanceBy(4)
            return jsonx.JsonElement.createBoolean(true)
        }
        if (ch == c'f' && idx + 4 < len &&
            src.charAt(idx + 1) == c'a' && src.charAt(idx + 2) == c'l' &&
            src.charAt(idx + 3) == c's' && src.charAt(idx + 4) == c'e') {
            this.advanceBy(5)
            return jsonx.JsonElement.createBoolean(false)
        }
        if ((ch == c'n' || ch == c't' || ch == c'f')) {
            throw new JsonParseError(`Expected '${ch == c'n' ? "null" : ch == c't' ? "true" : "false"}' at ${this.getPositionDesc()}`, idx, idx + 3)
        }

        switch (this.ch) {
            case c'"':
                return jsonx.JsonElement.createString(this.parseStringContent())
            case c'{':
                return this.parseObject()
            case c'[':
                return this.parseArray()
            case c'-':
            case c'0': case c'1': case c'2': case c'3': case c'4':
            case c'5': case c'6': case c'7': case c'8': case c'9':
                return this.parseNumber()
            default:
                throw new JsonParseError(`Unexpected character '${this.ch}' at ${this.getPositionDesc()}`,
                    this.index, this.index)
        }
    }

    private parseStringContent(): string {
        this.advance()
        let start = this.index
        const src = this.source

        let qIdx = this.nextQuoteIdx
        if (qIdx < start) {
            qIdx = src.indexOf(c'"', start)
            this.nextQuoteIdx = qIdx < 0 ? -1 : qIdx
        }
        if (qIdx < 0) {
            this.index = this.length
            throw new JsonParseError(`Unterminated string at ${this.getPositionDesc()}`, start, this.index)
        }

        let bsIdx = this.nextBsIdx
        if (bsIdx != -1 && bsIdx < start) {
            bsIdx = -2
        }
        if (bsIdx == -2) {
            bsIdx = src.indexOf(c'\\', start)
            this.nextBsIdx = bsIdx < 0 ? -1 : bsIdx
        }
        if (bsIdx < 0 || bsIdx >= qIdx) {
            this.advanceBy(qIdx + 1 - this.index)
            return src.substring(start, qIdx)
        }

        this.advanceBy(bsIdx - this.index)
        return this.parseStringContentWithEscape(start)
    }

    private parseStringContentWithEscape(start: int): string {
        const src = this.source
        const len = this.length
        let scanIdx = start
        let decodedLen = 0
        let quoteIdx = -1

        while (scanIdx < len) {
            const currentCh = src.charAt(scanIdx)
            if (currentCh == c'"') {
                quoteIdx = scanIdx
                break
            }
            if (currentCh == c'\\') {
                scanIdx++
                const consumed = UnifiedJsonParser.getEscapeLength(src, scanIdx, len)
                decodedLen++
                scanIdx += consumed
                continue
            }
            decodedLen++
            scanIdx++
        }

        if (quoteIdx < 0) {
            this.index = this.length
            throw new JsonParseError(`Unterminated string at ${this.getPositionDesc()}`, start, this.index)
        }

        let decoded = new ValueArray<char>(decodedLen)
        let readIdx = start
        let writeIdx = 0
        while (readIdx < quoteIdx) {
            const currentCh = src.charAt(readIdx)
            if (currentCh == c'\\') {
                readIdx++
                const consumed = UnifiedJsonParser.decodeEscape(src, readIdx, len, decoded, writeIdx)
                writeIdx++
                readIdx += consumed
                continue
            }
            decoded[writeIdx] = currentCh
            writeIdx++
            readIdx++
        }

        this.advanceBy(quoteIdx + 1 - this.index)
        return new String(decoded)
    }

    private static getEscapeLength(src: string, index: int, length: int): int {
        if (index >= length) {
            throw new JsonParseError(`Incomplete escape sequence at ${index}`, index, index)
        }

        const escChar = src.charAt(index)
        switch (escChar) {
            case c'"':
            case c'\\':
            case c'/':
            case c'b':
            case c'f':
            case c'n':
            case c'r':
            case c't':
                return 1
            case c'u':
                return UnifiedJsonParser.parseUnicodeEscape(src, index + 1, length) + 1
            default:
                throw new JsonParseError(`Invalid escape sequence '\\${escChar}' at ${index}`,
                    index - 1, index + 1)
        }
    }

    private static decodeEscape(src: string, index: int, length: int, decoded: ValueArray<char>, decodedOffset: int): int {
        if (index >= length) {
            throw new JsonParseError(`Incomplete escape sequence at ${index}`, index, index)
        }

        const escChar = src.charAt(index)
        switch (escChar) {
            case c'"':
                decoded[decodedOffset] = c'"'
                return 1
            case c'\\':
                decoded[decodedOffset] = c'\\'
                return 1
            case c'/':
                decoded[decodedOffset] = c'/'
                return 1
            case c'b':
                decoded[decodedOffset] = c'\b'
                return 1
            case c'f':
                decoded[decodedOffset] = c'\f'
                return 1
            case c'n':
                decoded[decodedOffset] = c'\n'
                return 1
            case c'r':
                decoded[decodedOffset] = c'\r'
                return 1
            case c't':
                decoded[decodedOffset] = c'\t'
                return 1
            case c'u':
                return UnifiedJsonParser.parseUnicodeEscape(src, index + 1, length, decoded, decodedOffset) + 1
            default:
                throw new JsonParseError(`Invalid escape sequence '\\${escChar}' at ${index}`,
                    index - 1, index + 1)
        }
    }

    private static parseUnicodeEscape(src: string, index: int, length: int, decoded: ValueArray<char>,
        decodedOffset: int): int {
        const codePoint = UnifiedJsonParser.parseUnicodeEscapeCodePoint(src, index, length)
        decoded[decodedOffset] = codePoint as Char
        return 4
    }

    private static parseUnicodeEscape(src: string, index: int, length: int): int {
        UnifiedJsonParser.parseUnicodeEscapeCodePoint(src, index, length)
        return 4
    }

    private static parseUnicodeEscapeCodePoint(src: string, index: int, length: int): int {
        if (index + 4 > length) {
            throw new JsonParseError(`Incomplete Unicode escape sequence at ${index}`, index, index)
        }

        let codePoint: int = 0
        for (let i = 0; i < 4; i++) {
            const c = src.charAt(index + i)
            if (!Char.isHexDigit(c)) {
                throw new JsonParseError(`Invalid hexadecimal digit '${c}' at ${index + i}`,
                    index + i, index + i + 1)
            }
            codePoint = codePoint * 16 + unhex(c)
        }

        return codePoint
    }

    private parseNumber(): jsonx.JsonElement {
        let start = this.index
        let hasFraction = false
        let hasExponent = false

        if (this.ch == c'-') {
            this.advance()
        }

        if (this.ch == c'0') {
            this.advance()
        } else if (Char.isDecDigit(this.ch)) {
            while (Char.isDecDigit(this.ch)) {
                this.advance()
            }
        } else {
            throw new JsonParseError(`Invalid number format at ${this.getPositionDesc()}`, start, this.index)
        }

        if (this.ch == c'.') {
            hasFraction = true
            this.advance()
            if (!Char.isDecDigit(this.ch)) {
                throw new JsonParseError(`Invalid number format at ${this.getPositionDesc()}`, start, this.index)
            }
            while (Char.isDecDigit(this.ch)) {
                this.advance()
            }
        }

        if (this.ch == c'e' || this.ch == c'E') {
            hasExponent = true
            this.advance()
            if (this.ch == c'+' || this.ch == c'-') {
                this.advance()
            }
            if (!Char.isDecDigit(this.ch)) {
                throw new JsonParseError(`Invalid number format at ${this.getPositionDesc()}`, start, this.index)
            }
            while (Char.isDecDigit(this.ch)) {
                this.advance()
            }
        }

        let numStr = this.source.substring(start, this.index)
        let isFloat = hasFraction || hasExponent

        if (isFloat) {
            return jsonx.JsonElement.createDouble(Double.parseFloat(numStr))
        }

        if (this.bigIntOpt == 2) {
            return jsonx.JsonElement.createBigInt(BigInt(numStr))
        }
        if (this.bigIntOpt == 1) {
            try {
                return jsonx.JsonElement.createLong(Long.parseInt(numStr, 10))
            } catch (e) {
                if (e instanceof FormatError) {
                    return jsonx.JsonElement.createBigInt(BigInt(numStr))
                }
                throw e
            }
        }
        return jsonx.JsonElement.createLong(Long.parseInt(numStr, 10))
    }

    private parseArray(): jsonx.JsonElement {
        this.advance()
        this.skipWhitespace()

        let array = new Array<jsonx.JsonElement>()
        let result = new jsonx.JsonElement()
        result.setArray(array)
        let idx = 0
        let hasReviver = this.reviver != undefined

        while (this.ch != c']') {
            this.skipWhitespace()
            let value = this.parseValue()

            if (hasReviver) {
                value = this.reviver!(`${idx}`, value)
            }

            array.push(value)
            idx++

            this.skipWhitespace()
            if (this.ch == c',') {
                this.advance()
            } else if (this.ch != c']') {
                throw new JsonParseError(`Expected ',' or ']' at ${this.getPositionDesc()}`,
                    this.index, this.index)
            }
        }

        this.advance()
        return result
    }

    private parseObject(): jsonx.JsonElement {
        this.advance()
        this.skipWhitespace()

        if (this.ch == c'}') {
            this.advance()
            return jsonx.JsonElement.createObject(new Map<string, jsonx.JsonElement>())
        }

        let result = new jsonx.JsonElement()
        let hasReviver = this.reviver != undefined

        while (true) {
            this.skipWhitespace()
            let key = this.parseStringContent()

            this.skipWhitespace()
            if (this.ch != c':') {
                throw new JsonParseError(`Expected ':' at ${this.getPositionDesc()}`,
                    this.index, this.index)
            }
            this.advance()
            this.skipWhitespace()

            let value = this.parseValue()

            if (hasReviver) {
                value = this.reviver!(key, value)
            }

            result.setElement(key, value)

            this.skipWhitespace()
            if (this.ch == c',') {
                this.advance()
            } else if (this.ch == c'}') {
                this.advance()
                return result
            } else {
                throw new JsonParseError(`Expected ',' or '}' at ${this.getPositionDesc()}`,
                    this.index, this.index)
            }
        }
    }

    private skipWhitespace(): void {
        let idx = this.index
        const len = this.length
        const src = this.source
        while (idx < len) {
            let ch = src.charAt(idx)
            if (ch != c' ' && ch != c'\t' && ch != c'\n' && ch != c'\r') {
                break
            }
            idx++
        }
        if (idx != this.index) {
            this.advanceBy(idx - this.index)
        }
    }

    public parseToJSONValue(): JSONValue {
        this.skipWhitespace()
        if (this.index >= this.length) {
            throw new JsonParseError("Unexpected end of input", 0, 0)
        }
        let result = this.parseValueToJSON()
        this.skipWhitespace()
        if (this.index < this.length) {
            throw new JsonParseError(`Unexpected character '${this.ch}' at ${this.getPositionDesc()}`,
                this.index, this.index)
        }
        return result
    }

    private parseValueToJSON(): JSONValue {
        let idx = this.index
        let ch = this.ch
        const len = this.length
        const src = this.source
        if (ch == c'n' && idx + 3 < len &&
            src.charAt(idx + 1) == c'u' && src.charAt(idx + 2) == c'l' && src.charAt(idx + 3) == c'l') {
            this.advanceBy(4)
            return new JSONNull()
        }
        if (ch == c't' && idx + 3 < len &&
            src.charAt(idx + 1) == c'r' && src.charAt(idx + 2) == c'u' && src.charAt(idx + 3) == c'e') {
            this.advanceBy(4)
            return new JSONTrue()
        }
        if (ch == c'f' && idx + 4 < len &&
            src.charAt(idx + 1) == c'a' && src.charAt(idx + 2) == c'l' &&
            src.charAt(idx + 3) == c's' && src.charAt(idx + 4) == c'e') {
            this.advanceBy(5)
            return new JSONFalse()
        }

        switch (this.ch) {
            case c'"':
                let jsonStr = new JSONString()
                jsonStr.value = this.parseStringContent()
                return jsonStr
            case c'{':
                return this.parseObjectToJSON()
            case c'[':
                return this.parseArrayToJSON()
            case c'-':
            case c'0': case c'1': case c'2': case c'3': case c'4':
            case c'5': case c'6': case c'7': case c'8': case c'9':
                return this.parseNumberToJSON()
            default:
                throw new JsonParseError(`Unexpected character '${this.ch}' at ${this.getPositionDesc()}`,
                    this.index, this.index)
        }
    }

    private parseNumberToJSON(): JSONNumber {
        let start = this.index

        if (this.ch == c'-') {
            this.advance()
        }

        if (this.ch == c'0') {
            this.advance()
        } else if (Char.isDecDigit(this.ch)) {
            while (Char.isDecDigit(this.ch)) {
                this.advance()
            }
        } else {
            throw new JsonParseError(`Invalid number format at ${this.getPositionDesc()}`, start, this.index)
        }

        if (this.ch == c'.') {
            this.advance()
            if (!Char.isDecDigit(this.ch)) {
                throw new JsonParseError(`Invalid number format at ${this.getPositionDesc()}`, start, this.index)
            }
            while (Char.isDecDigit(this.ch)) {
                this.advance()
            }
        }

        if (this.ch == c'e' || this.ch == c'E') {
            this.advance()
            if (this.ch == c'+' || this.ch == c'-') {
                this.advance()
            }
            if (!Char.isDecDigit(this.ch)) {
                throw new JsonParseError(`Invalid number format at ${this.getPositionDesc()}`, start, this.index)
            }
            while (Char.isDecDigit(this.ch)) {
                this.advance()
            }
        }

        let numStr = this.source.substring(start, this.index)

        let jsonNum = new JSONNumber()
        jsonNum.value = Double.parseFloat(numStr)
        try {
            jsonNum.bigintValue = new BigInt(numStr)
        } catch (e) {
            jsonNum.bigintValue = new BigInt()
        }
        return jsonNum
    }

    private parseArrayToJSON(): JSONArray {
        this.advance()
        this.skipWhitespace()

        let jsonArray = new JSONArray()

        while (this.ch != c']') {
            this.skipWhitespace()
            let value = this.parseValueToJSON()
            jsonArray.values.push(value)

            this.skipWhitespace()
            if (this.ch == c',') {
                this.advance()
            } else if (this.ch != c']') {
                throw new JsonParseError(`Expected ',' or ']' at ${this.getPositionDesc()}`,
                    this.index, this.index)
            }
        }

        this.advance()
        return jsonArray
    }

    private parseObjectToJSON(): JSONObject {
        this.advance()
        this.skipWhitespace()

        let jsonObj = new JSONObject()

        while (this.ch != c'}') {
            this.skipWhitespace()

            let key = this.parseStringContent()

            this.skipWhitespace()
            if (this.ch != c':') {
                throw new JsonParseError(`Expected ':' at ${this.getPositionDesc()}`,
                    this.index, this.index)
            }
            this.advance()
            this.skipWhitespace()

            let value = this.parseValueToJSON()

            let jsonKey = new JSONString()
            jsonKey.value = key
            jsonObj.keys_.push(jsonKey)
            jsonObj.values.push(value)

            this.skipWhitespace()
            if (this.ch == c',') {
                this.advance()
            } else if (this.ch != c'}') {
                throw new JsonParseError(`Expected ',' or '}' at ${this.getPositionDesc()}`,
                    this.index, this.index)
            }
        }

        this.advance()
        return jsonObj
    }

    public static parse(source: string, options?: jsonx.ParseOptions): jsonx.JsonElement {
        let parser = new UnifiedJsonParser(source, options)
        return parser.parse()
    }

    public static parseToJSONValue(source: string): JSONValue {
        let parser = new UnifiedJsonParser(source)
        return parser.parseToJSONValue()
    }

    public static parse(source: string, reviver: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement,
        options?: jsonx.ParseOptions): jsonx.JsonElement {

        let parser = new UnifiedJsonParser(source, reviver, options)
        return parser.parse()
    }
}

class JSONValueParser {
    private reviver: ((key: string, value: Any) => Any) | undefined
    private bigIntMode: int

    private static readonly SETTER_PREFIX = '%%set-'

    public constructor(reviver: ((key: string, value: Any) => Any) | undefined, bigIntMode: int) {
        this.reviver = reviver
        this.bigIntMode = bigIntMode
    }

    private createNumber(value: JSONNumber, type: Class): Any {
        if (type === BigIntType
            && (this.bigIntMode === 2 || (!Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return value.bigintValue
        } else if ((Types.isDoubleType(type) || type.isEnum())
            && (this.bigIntMode === 0 || (Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return value.value
        } else if ((Types.isIntType(type) || type.isEnum())
            && (this.bigIntMode === 0 || (Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return value.value.toInt()
        } else if ((Types.isLongType(type) || type.isEnum()) && this.bigIntMode < 2) {
            return createLongFromJSONValue(value)
        } else if ((Types.isShortType(type) || type.isEnum()) && this.bigIntMode < 2) {
            return createShortFromJSONValue(value)
        } else if ((Types.isFloatType(type) || type.isEnum()) && this.bigIntMode < 2) {
            return createFloatFromJSONValue(value)
        } else if ((Types.isByteType(type) || type.isEnum()) && this.bigIntMode < 2) {
            return createByteFromJSONValue(value)
        } else {
            throw new Error(`${type.getName()} is expected, but get ${Class.of(value.value).getName()}
                 with bigIntMode=${this.bigIntMode}`)
        }
    }

    public parse(value: JSONValue, type: Class): Any {
        if (value instanceof JSONObject && type.isUnion()) {
            // NOTE(lyupaanastasia) parse union #27136
            throw new Error('UnionType is not supported yet')
        } else if (value instanceof JSONArray && type.isFixedArray()) {
            return this.parseArray(value, type)
        } else if (value instanceof JSONNumber) {
            const num = this.createNumber(value, type)
            if (type.isEnum()) {
                let valArray = type.getStaticMethod('values')!.invoke() as FixedArray<Any>
                for (let idx = 0; idx < valArray.length; idx++) {
                    if (valArray[idx] == num) {
                        return valArray[idx]
                    }
                }
            }
            return num

        } else if (value instanceof JSONString && Types.isCharType(type)) {
            return value.value.charAt(0)
        } else if (value instanceof JSONString && (type === TypeColor.STRING_CLASS || type.isEnum())) {
            const str = (value as JSONString).value
            if (type.isEnum()) {
                let strArray = type.getStaticMethod('values')!.invoke() as FixedArray<Any>
                for (let idx = 0; idx < strArray.length; idx++) {
                    if (strArray[idx] == str) {
                        return strArray[idx]
                    }
                }
            }
            return str
        } else if (value instanceof JSONTrue && Types.isBooleanType(type)) {
            return true
        } else if (value instanceof JSONFalse && Types.isBooleanType(type)) {
            return false
        } else if (value instanceof JSONNull && type === TypeColor.NULL_CLASS) {
            return null
        } else if (value instanceof JSONObject && this.isParseableObjectType(type)) {
            return this.parseObject(value, type)
        } else {
            const valueType = Class.of(value)
            throw new Error(`${type.getName()} is expected, but get ${valueType.getName()}`)
        }
    }

    private isParseableObjectType(type: Class): boolean {
        return !(type === TypeColor.STRING_CLASS ||
                reflect.internals.Types.isPrimitiveType(type) ||
                reflect.internals.Types.isCharType(type) ||
                type === TypeColor.NULL_CLASS ||
                type.isSubtypeOf(TypeColor.ARRAY_CLASS) ||
                type.isFixedArray() ||
                type.isInterface())
    }

    private parseObject(jsonObj: JSONObject, classType: Class): Object {
        let setClassFieldFn = (field: reflect.InstanceField, obj: object, fieldVal: Any): void => {
            field.setValue(obj, fieldVal)
        }

        if (this.reviver != undefined) {
            setClassFieldFn = (field: reflect.InstanceField, obj: object, fieldVal: Any): void => {
                const revivedVal = this.reviver!(field.getName(), fieldVal)
                field.setValue(obj, revivedVal)
            }
        }

        const obj: object = JSONValueParser.classGetDefaultCtor(classType).createInstance()
        let hasJsonParseIgnore: boolean = false
        const jsonObjFields = jsonObj.getFields()

        const classFields = reflect.getInstanceFieldsRecursive(classType)
        for (const classField of classFields) {
            hasJsonParseIgnore = JSONAPI.getJSONParseIgnore(classField);
            if (hasJsonParseIgnore) {
                continue
            }

            let jsonFieldVal = jsonObjFields.get(classField.getName())
            if (jsonFieldVal === undefined) {
                let fieldRename = JSONAPI.getJSONRename(classField);
                if (fieldRename == undefined) {
                    continue
                }
                jsonFieldVal = jsonObjFields.get(fieldRename)
                if (jsonFieldVal === undefined) {
                    continue
                }
            }

            const classFieldVal = this.parse(jsonFieldVal, classField.getType())
            setClassFieldFn(classField, obj, classFieldVal)
        }

        for (const method of classType.getInstanceMethods()) {
            const methodName = method.getName()
            if (methodName.startsWith(JSONValueParser.SETTER_PREFIX)) {
                const propName = methodName.substring(JSONValueParser.SETTER_PREFIX.length)
                const jsonFieldVal = jsonObjFields.get(propName)
                if (jsonFieldVal !== undefined) {
                    const propType = method.getParameterType(0)!
                    const classFieldVal = this.parse(jsonFieldVal, propType)

                    if (this.reviver !== undefined) {
                        const revivedVal = this.reviver!(propName, classFieldVal)
                        const setterArgs: FixedArray<Any> = [revivedVal]
                        method.invoke(obj, setterArgs)
                    } else {
                        const setterArgs: FixedArray<Any> = [classFieldVal]
                        method.invoke(obj, setterArgs)
                    }
                }
            }
        }

        return obj
    }

    private static extractRawValue(value: JSONValue): Any {
        if (value instanceof JSONString) {
            return value.value
        }
        if (value instanceof JSONNumber) {
            return value.value
        }
        if (value instanceof JSONTrue) {
            return true
        }
        if (value instanceof JSONFalse) {
            return false
        }
        if (value instanceof JSONNull) {
            return null
        }
        return value
    }

    private static classGetDefaultCtor(cls: Class): reflect.Constructor {
        for (const ctor of cls.getConstructors()) {
            if (ctor.getParametersNum() == 0) {
                return ctor
            }
        }
        throw new Error(`class ${cls.getName()} doesn't have default ctor`)
    }

    public parseInto(value: JSONValue, instance: object): Any {
        const classType = Class.of(instance)
        if (value instanceof JSONObject && this.isParseableObjectType(classType)) {
            return this.parseObjectInto(value, instance)
        }
        return this.parse(value, classType)
    }

    private parseObjectInto(jsonObj: JSONObject, existingObj: object): object {
        const classType = Class.of(existingObj)

        let setClassFieldFn = (field: reflect.InstanceField, obj: object, fieldVal: Any) => {
            field.setValue(obj, fieldVal)
        }

        if (this.reviver != undefined) {
            setClassFieldFn = (field: reflect.InstanceField, obj: object, fieldVal: Any): void => {
                const revivedVal = this.reviver!(field.getName(), fieldVal)
                if (revivedVal === undefined) {
                    return
                }
                field.setValue(obj, revivedVal)
            }
        }

        const jsonObjFields = jsonObj.getFields()

        for (const classField of reflect.getInstanceFieldsRecursive(classType)) {
            if (JSONAPI.getJSONParseIgnore(classField)) {
                continue
            }

            let jsonFieldVal = jsonObjFields.get(classField.getName())
            if (jsonFieldVal === undefined) {
                const fieldRename = JSONAPI.getJSONRename(classField)
                if (fieldRename !== undefined) {
                    jsonFieldVal = jsonObjFields.get(fieldRename)
                }
            }
            // Key absent in JSON → skip to preserve the existing field value (merge semantics)
            if (jsonFieldVal === undefined) {
                continue
            }

            const fieldType = classField.getType()
            if (jsonFieldVal instanceof JSONObject && this.isParseableObjectType(fieldType)) {
                const existingFieldVal = classField.getValue(existingObj)
                if (existingFieldVal !== null && existingFieldVal !== undefined) {
                    this.parseObjectInto(jsonFieldVal, existingFieldVal as object)
                    continue
                }
            }
            let classFieldVal: Any
            if (this.reviver !== undefined) {
                try {
                    classFieldVal = this.parse(jsonFieldVal, fieldType)
                } catch (e) {
                    if (jsonFieldVal instanceof JSONObject || jsonFieldVal instanceof JSONArray) {
                        throw new JSONTypeError(`Incorrect field: '${classType.getName()}.${classField.getName()}'. ${e.message}`)
                    }
                    classFieldVal = JSONValueParser.extractRawValue(jsonFieldVal)
                }
            } else {
                classFieldVal = this.parse(jsonFieldVal, fieldType)
            }
            setClassFieldFn(classField, existingObj, classFieldVal)
        }

        for (const method of classType.getInstanceMethods()) {
            const methodName = method.getName()
            if (!methodName.startsWith(JSONValueParser.SETTER_PREFIX)) {
                continue
            }
            const propName = methodName.substring(JSONValueParser.SETTER_PREFIX.length)
            const jsonFieldVal = jsonObjFields.get(propName)
            if (jsonFieldVal === undefined) {
                continue
            }
            const propType = method.getParameterType(0)!
            const classFieldVal = this.parse(jsonFieldVal, propType)

            if (this.reviver !== undefined) {
                const revivedVal = this.reviver!(propName, classFieldVal)
                const setterArgs: FixedArray<Any> = [revivedVal]
                method.invoke(existingObj, setterArgs)
            } else {
                const setterArgs: FixedArray<Any> = [classFieldVal]
                method.invoke(existingObj, setterArgs)
            }
        }

        return existingObj
    }

    private parseArray(jVal: JSONArray, aType: Class): Object | null {
        const len = jVal.values.length
        const componentType = aType.getFixedArrayComponentType()!

        if (componentType.isPrimitive()) {
            return this.parsePrimitiveFixedArray(jVal, componentType, len)
        }

        const arr = reflect.internals.createFixedArray(componentType, len) as FixedArray<Any>

        let setArrayElementFn = (a: FixedArray<Any>, i: int, v: Any) => { a[i] = v }

        if (this.reviver !== undefined) {
            setArrayElementFn = (array: FixedArray<Any>, index: int, elemVal: Any): void => {
                const revivedVal = this.reviver!(`${index}`, elemVal)
                array[index] = revivedVal !== undefined ? revivedVal : null
            }
        }

        for (let i = 0; i < len; i++) {
            let elem = this.parse(jVal.values.at(i), componentType)
            setArrayElementFn(arr, i, elem)
        }
        return arr
    }

    private parsePrimitiveFixedArray(jVal: JSONArray, componentType: Class, len: int): Object {
        const parsed = new Array<Any>()
        for (let i = 0; i < len; i++) {
            let elem = this.parse(jVal.values.at(i), componentType)
            if (this.reviver !== undefined) {
                const revivedVal = this.reviver!(`${i}`, elem)
                if (revivedVal !== undefined) {
                    elem = revivedVal
                }
            }
            parsed.push(elem)
        }

        const typedArr = reflect.internals.createFixedArray(componentType, len)
        for (let i = 0; i < len; i++) {
            const idx: long = i.toLong()
            const elem = parsed.at(i)
            if (elem instanceof Numeric) {
                if (Types.isIntType(componentType)) {
                    ValueAPISetElementInt(typedArr, idx, elem.toInt())
                } else if (Types.isDoubleType(componentType)) {
                    ValueAPISetElementDouble(typedArr, idx, elem.toDouble())
                } else if (Types.isLongType(componentType)) {
                    ValueAPISetElementLong(typedArr, idx, elem.toLong())
                } else if (Types.isFloatType(componentType)) {
                    ValueAPISetElementFloat(typedArr, idx, elem.toFloat())
                } else if (Types.isShortType(componentType)) {
                    ValueAPISetElementShort(typedArr, idx, elem.toShort())
                } else if (Types.isByteType(componentType)) {
                    ValueAPISetElementByte(typedArr, idx, elem.toByte())
                }
            } else if (elem instanceof Boolean) {
                ValueAPISetElementBoolean(typedArr, idx, elem.toBoolean())
            } else if (elem instanceof Char) {
                ValueAPISetElementChar(typedArr, idx, elem.toChar())
            }
        }

        return typedArr
    }
}

/**
 * Error thrown when parsing JSON fails.
 */
export class JsonParseError extends SyntaxError {
    public constructor(msg: string, start_offset?: int, end_offset?: int) {
        super(`${msg} at ${start_offset}..${end_offset}`)
    }
}


class JSONValue {}

class JSONObject extends JSONValue {
    public keys_: Array<JSONString> = new Array<JSONString>()
    public values: Array<JSONValue> = new Array<JSONValue>()
    public readonly static START_CHAR: char = c'{'
    public readonly static END_CHAR: char = c'}'
    public readonly static SEPARATOR: char = c':'
    public readonly static DELIMETER: char = c','
    public readonly static EMPTY: string = '{}'

    public getFields(): Map<String, JSONValue> {
        const propsCount = this.keys_.length

        const result = new Map<String, JSONValue>()

        for (let i = 0; i < propsCount; i++) {
            const key = this.keys_[i]
            const val = this.values[i]

            result.set(key.value, val)
        }

        return result
    }

    public override toString(): String {
        let res = new StringBuilder([JSONObject.START_CHAR] as FixedArray<char>)
        for (let i = 0; i < this.keys_.length - 1; ++i) {
            res.append('' + this.keys_.at(i) + JSONObject.SEPARATOR + this.values.at(i) + JSONObject.DELIMETER)
        }
        if (this.keys_.length > 0) {
            res.append('' + this.keys_.at(this.keys_.length - 1) + JSONObject.SEPARATOR
                + this.values.at(this.keys_.length - 1))
        }
        res.append(JSONObject.END_CHAR)
        return res.toString()
    }
}

class JSONArray extends JSONValue {
    public values: Array<JSONValue> = new Array<JSONValue>()
    public readonly static START_CHAR: char = c'['
    public readonly static END_CHAR: char = c']'
    public readonly static SEPARATOR: char = c','

    public override toString(): String {
        let res = new StringBuilder([JSONArray.START_CHAR] as FixedArray<char>)
        for (let i = 0; i < this.values.length - 1; ++i) {
            res.append('' + this.values.at(i) + JSONArray.SEPARATOR)
        }
        if (this.values.length > 0) {
            res.append('' + this.values.at(this.values.length - 1))
        }
        res.append(JSONArray.END_CHAR)
        return res.toString()
    }
}

class JSONNumber extends JSONValue {
    public value: double
    public bigintValue: bigint = new BigInt()

    public override toString(): String {
        return this.value.toString()
    }
}

class JSONString extends JSONValue {
    public value: String = ''
    public readonly static START_CHAR: char = c'"'
    public readonly static END_CHAR: char = c'"'

    public override toString(): String {
        return this.value
    }
}

class JSONTrue extends JSONValue {
    public readonly static value: string = 'true'
    public readonly static START_CHAR: char = c't'

    public override toString(): String {
        return JSONTrue.value
    }
}

class JSONFalse extends JSONValue {
    public readonly static value: string = 'false'
    public readonly static START_CHAR: char = c'f'

    public override toString(): String {
        return JSONFalse.value
    }
}

class JSONNull extends JSONValue {
    public readonly static value: string = 'null'
    public readonly static START_CHAR: char = c'n'

    public override toString(): String {
        return JSONNull.value
    }
}

/**
 * Creates a Boolean instance based on JSONValue
 *
 * @param { Boolean } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Boolean } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 */
function createBooleanFromJSONValue(json: JSONValue): Boolean {
    if (json instanceof JSONFalse) {
        return Boolean.FALSE
    } else if (json instanceof JSONTrue) {
        return Boolean.TRUE
    }
    throw new JSONTypeError('Cannot create Boolean from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Byte instance based on JSONValue
 *
 * @param { Byte } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Byte } - byte value decoded from JSON
 * @throws { JSONTypeError } if json does not encode a valid byte
 */
function createByteFromJSONValue(json: JSONValue): Byte {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        if (Double.isInteger(num) && Byte.MIN_VALUE <= num && num <= Byte.MAX_VALUE) {
            return num.toByte()
        }
    }
    throw new JSONTypeError('Cannot create Byte from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Char instance based on JSONValue
 *
 * @param { Char } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Char } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 */
function createCharFromJSONValue(json: JSONValue): Char {
    if (json instanceof JSONString) {
        let str = (json as JSONString).value
        if (str.getLength() == 1) {
            return new Char(str.charAt(0))
        }
    }
    throw new JSONTypeError('Cannot create Char from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Short instance based on JSONValue
 *
 * @param { Short } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Short } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 */
function createShortFromJSONValue(json: JSONValue): Short {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        if (Double.isInteger(num) && Short.MIN_VALUE <= num && num <= Short.MAX_VALUE) {
            return num.toShort()
        }
    }
    throw new JSONTypeError('Cannot create Short from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Int instance based on JSONValue
 *
 * @param { Int } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Int } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 */
function createIntFromJSONValue(json: JSONValue): Int {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        if (Double.isInteger(num) && Int.MIN_VALUE <= num && num <= Int.MAX_VALUE) {
            return new Int(Double.toInt(num))
        }
    }
    throw new JSONTypeError('Cannot create Int from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Long instance based on JSONValue
 *
 * @param { Long } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Long } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 */
function createLongFromJSONValue(json: JSONValue): Long {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        let value = (json as JSONNumber).bigintValue
        if (Double.isInteger(num) && Long.MIN_VALUE <= value && value <= Long.MAX_VALUE) {
            return value.getLong()
        }
    }
    throw new JSONTypeError('Cannot create Long from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Float instance based on JSONValue
 *
 * @param { Float } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Float } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 */
function createFloatFromJSONValue(json: JSONValue): Float {
    if (json instanceof JSONNumber) {
        let val = (json as JSONNumber).value
        if (-Float.MAX_VALUE <= val && val <= Float.MAX_VALUE) {
            return val.toFloat()
        }
    }
    throw new JSONTypeError('Cannot create Double from JSON', {cause: json as Object} as ErrorOptions)
}

/**
/**
 * Creates a Double instance based on JSONValue
 *
 * @param { Double } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { Double } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid boolean literal
 */
function createDoubleFromJSONValue(json: JSONValue): Double {
    if (json instanceof JSONNumber) {
        return new Double((json as JSONNumber).value)
    }
    throw new JSONTypeError('Cannot create Double from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a String instance based on JSONValue
 *
 * @param { String } this JSONValue - a JSON representation
 * @param { JSONValue } json
 * @returns { String } - true if JSON encodes true literal, false if JSON encodes false literal
 * @throws { JSONTypeError } if json does not encode a valid String
 */
function createStringFromJSONValue(json: JSONValue): String {
    if (json instanceof JSONString) {
        return (json as JSONString).value
    }
    throw new JSONTypeError('Cannot create String from JSON', {cause: json as Object} as ErrorOptions)
}

