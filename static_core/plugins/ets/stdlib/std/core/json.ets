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


function findFieldByName(fields: Array<reflect.InstanceField>, name: string): reflect.InstanceField | undefined {
    return fields.find((fld) => fld.getName() == name)
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

class ClassMetadata {
    readonly metadata: Array<FieldMetadata>;
    readonly nameMap: Map<string, FieldMetadata>;

    constructor(metadata: Array<FieldMetadata>) {
        this.metadata = metadata;
        this.nameMap = new Map<string, FieldMetadata>();
        for (let i = 0; i < metadata.length; i++) {
            const meta = metadata[i];
            this.nameMap.set(meta.name, meta);
        }
    }
}

class FieldMetadataCache {
    private static readonly CACHE_CAPACITY: int = 200;
    private static readonly cache: ConcurrentLRUCache<Class, ClassMetadata> = new ConcurrentLRUCache<Class, ClassMetadata>(FieldMetadataCache.CACHE_CAPACITY);

    static getMetadata(cls: Class): Array<FieldMetadata> {
        const classMeta = FieldMetadataCache.cache.computeIfAbsent(cls, (c: Class) => {
            const fields = reflect.getInstanceFieldsRecursive(c);
            const metadata = new Array<FieldMetadata>(fields.length);
            for (let i = 0; i < fields.length; i++) {
                const field = fields[i];
                const ignore = JSONAPI.getJSONStringifyIgnore(field);
                const rename = JSONAPI.getJSONRename(field);
                metadata[i] = new FieldMetadata(field, ignore, rename);
            }
            return new ClassMetadata(metadata);
        });
        return classMeta.metadata;
    }

    static getMetadataByName(cls: Class, name: string): FieldMetadata | undefined {
        const classMeta = FieldMetadataCache.cache.computeIfAbsent(cls, (c: Class) => {
            const fields = reflect.getInstanceFieldsRecursive(c);
            const metadata = new Array<FieldMetadata>(fields.length);
            for (let i = 0; i < fields.length; i++) {
                const field = fields[i];
                const ignore = JSONAPI.getJSONStringifyIgnore(field);
                const rename = JSONAPI.getJSONRename(field);
                metadata[i] = new FieldMetadata(field, ignore, rename);
            }
            return new ClassMetadata(metadata);
        });
        return classMeta.nameMap.get(name);
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

    private static readonly ESCAPED_CHARS: FixedArray<char> = [c'"', c'\\', c'\b', c'\f', c'\n', c'\r', c'\t']
    private static readonly NON_ESCAPED_CHARS: FixedArray<char> = [c'"', c'\\', c'b', c'f', c'n', c'r', c't']
    private static readonly JSON_SPACE_INDENT_LIMIT = 10

    private static scanEscapeInfo(str: String): FixedArray<int> {
        const len = str.getLength()
        let needsEscaping = false
        let hasSurrogates = false
        let extraLength = 0
        for (let i = 0; i < len; ++i) {
            const ch = str.charAt(i)
            const code = ch.toInt()
            
            // Check for ASCII characters that need escaping
            if (code < 128) {
                switch (ch) {
                    case c'"':
                    case c'\\':
                        needsEscaping = true
                        extraLength += 1  // \" or \\
                        break
                    case c'\b':
                    case c'\f':
                    case c'\n':
                    case c'\r':
                    case c'\t':
                        needsEscaping = true
                        extraLength += 1  // \b, \f, \n, \r, \t
                        break
                    default:
                        if (code < 0x20 || code == 0x7F) {
                            needsEscaping = true
                            extraLength += 5  // \uXXXX
                        }
                        break
                }
            }
            
            // Check for surrogate characters
            if (code >= 0xD800 && code <= 0xDFFF) {
                hasSurrogates = true
                if (Char.isHighSurrogate(ch)) {
                    if (i + 1 < len && Char.isLowSurrogate(str.charAt(i + 1))) {
                        // Valid surrogate pair - skip the low surrogate in next iteration
                        i++
                        // No extra length needed
                    } else {
                        // Lone high surrogate -> \uXXXX
                        needsEscaping = true
                        extraLength += 5
                    }
                } else if (Char.isLowSurrogate(ch)) {
                    // Lone low surrogate -> \uXXXX
                    needsEscaping = true
                    extraLength += 5
                }
            }
        }
        const result = new FixedArray<int>(3)
        result[0] = needsEscaping ? 1 : 0
        result[1] = hasSurrogates ? 1 : 0
        result[2] = extraLength
        return result
    }

    private static needsEscaping(str: String): boolean {
        const info = JSON.scanEscapeInfo(str)
        return info[0] != 0
    }

    /**
     * Converts String to JSON format
     *
     * @param d: String - byte to be converted to a JSON as a String
     *
     * @returns String - JSON representation of byte
     */
    public static stringify(d: String): String {
        // Scan once to get escaping requirements
        const info = JSON.scanEscapeInfo(d)
        const needsEscaping = info[0]
        const hasSurrogates = info[1]
        
        // Fast path: if string contains no characters that need escaping, simply quote it.
        if (!needsEscaping) {
            let sb = new StringBuilder('"')
            sb.append(d)
            sb.append('"')
            return sb.toString()
        }
        
        let sb = new StringBuilder('"')
        let len = d.getLength()
        const escapeTable = EscapeStringBuilder.escapeTable

        if (!hasSurrogates) {
            // No surrogate characters - use chunk copying
            let begin = 0
            for (let i = 0; i < len; ++i) {
                const ch = d.charAt(i)
                const code = ch.toInt()
                if (code < 128) {
                    const escaped = escapeTable[code]
                     if (escaped !== undefined && escaped.length > 0) {
                        if (begin < i) {
                            sb.append(d.substring(begin, i))
                        }
                        sb.append(escaped)
                        begin = i + 1
                    }
                }
            }
            if (begin < len) {
                sb.append(d.substring(begin))
            }
        } else {
            // String contains surrogate characters - use original logic with surrogate handling
            let prevChar: char = c'\0'
            for (let i = 0; i < len; ++i) {
                let currChar = d.charAt(i)
                const code = currChar.toInt()

                if (Char.isHighSurrogate(currChar)) {
                    if (i != 0 && Char.isHighSurrogate(prevChar)) {
                        JSON.appendCharHex(sb, prevChar)
                    }
                } else if (Char.isLowSurrogate(currChar)) {
                    if (i != 0 && Char.isHighSurrogate(prevChar)) {
                        sb.append(prevChar)
                        sb.append(currChar)
                    } else {
                        JSON.appendCharHex(sb, currChar)
                    }
                } else {
                    if (i != 0 && Char.isHighSurrogate(prevChar)) {
                        JSON.appendCharHex(sb, prevChar)
                    }

                    // Handle ASCII characters via escape table
                    if (code < 128) {
                        const escaped = escapeTable[code]
                        if (escaped !== undefined && escaped.length > 0) {
                            sb.append(escaped)
                        } else {
                            sb.append(currChar)
                        }
                    } else {
                        // Non-ASCII, non-surrogate character
                        sb.append(currChar)
                    }
                }

                prevChar = currChar
            }

            if (len > 0) {
                if (Char.isHighSurrogate(prevChar)) {
                    JSON.appendCharHex(sb, prevChar)
                }
            }
        }

        sb.append('"')
        return sb.toString()
    }

    private static appendCharHex(buffer: StringBuilder, chr: char): StringBuilder {
        const hex = chr.toInt().toString(16).padStart(4, '0')
        buffer.append('\\u').append(hex)
        return buffer
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
            s.append('"');
            s.append(JSON.stringify(i));
            s.append('":');
            s.append(JSON.stringify(d[i]));
            s.append(',');
        }
        if (d.length > 0) {
            s.append('"');
            s.append(JSON.stringify(last_elem));
            s.append('":');
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
            return JSON.stringifyJSValue(obj as JSValue)
        }
        return new JSONWriter().write(obj)
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
        if (obj === undefined) {
            return ''
        }
        return new JSONWriter(undefined, undefined, true).write(obj)
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

interface ArrayHandle {
    readonly length: int
    readonly array: object
    get(i: int): Any
}

class BoolArrayHandle implements ArrayHandle {
    private arr: ValueArray<boolean>

    public constructor(arr: ValueArray<boolean>) {
        this.arr = arr
    }

    public get length(): int {
        return this.arr.length
    }

    public get array(): object {
        return this.arr
    }

    public get(i: int): Any {
        return this.arr[i]
    }
}

class ByteArrayHandle implements ArrayHandle {
    private arr: ValueArray<byte>

    public constructor(arr: ValueArray<byte>) {
        this.arr = arr
    }

    public get length(): int {
        return this.arr.length
    }

    public get array(): object {
        return this.arr
    }

    public get(i: int): Any {
        return this.arr[i]
    }
}

class CharArrayHandle implements ArrayHandle {
    private arr: ValueArray<char>

    public constructor(arr: ValueArray<char>) {
        this.arr = arr
    }

    public get length(): int {
        return this.arr.length
    }

    public get array(): object {
        return this.arr
    }

    public get(i: int): Any {
        return this.arr[i]
    }
}

class ShortArrayHandle implements ArrayHandle {
    private arr: ValueArray<short>

    public constructor(arr: ValueArray<short>) {
        this.arr = arr
    }

    public get length(): int {
        return this.arr.length
    }

    public get array(): object {
        return this.arr
    }

    public get(i: int): Any {
        return this.arr[i]
    }
}

class IntArrayHandle implements ArrayHandle {
    private arr: ValueArray<int>

    public constructor(arr: ValueArray<int>) {
        this.arr = arr
    }

    public get length(): int {
        return this.arr.length
    }

    public get array(): object {
        return this.arr
    }

    public get(i: int): Any {
        return this.arr[i]
    }
}

class LongArrayHandle implements ArrayHandle {
    private arr: ValueArray<long>

    public constructor(arr: ValueArray<long>) {
        this.arr = arr
    }

    public get length(): int {
        return this.arr.length
    }

    public get array(): object {
        return this.arr
    }

    public get(i: int): Any {
        return this.arr[i]
    }
}

class FloatArrayHandle implements ArrayHandle {
    private arr: ValueArray<float>

    public constructor(arr: ValueArray<float>) {
        this.arr = arr
    }

    public get length(): int {
        return this.arr.length
    }

    public get array(): object {
        return this.arr
    }

    public get(i: int): Any {
        return this.arr[i]
    }
}

class DoubleArrayHandle implements ArrayHandle {
    private arr: ValueArray<double>

    public constructor(arr: ValueArray<double>) {
        this.arr = arr
    }

    public get length(): int {
        return this.arr.length
    }

    public get array(): object {
        return this.arr
    }

    public get(i: int): Any {
        return this.arr[i]
    }
}

class JSONWriter {
    private static readonly STD_CORE_INTEROP_JSVALUE_CLASS = Class.from<JSValue>()
    // Length of the bytecode getter prefix "%%get-" (GETTER_BEGIN in ets_type.h)
    private static readonly GETTER_PREFIX_LEN: int = 6
    private static readonly maxSpaceLength = 10

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

    private indentLevel: int = 0

    private readonly path = new Set<Any>()
    private buffer = new StringBuilder()

    public constructor(replacer?: (key: String, value: Any) => Any, space?: String, useAnnotatedGetters: boolean = false) {
        this.replacer = replacer
        this.fieldsFilter = []
        this.space = JSONWriter.truncateSpace(space)

        this.useReplacer = this.replacer != undefined
        this.useFieldsFilter = false
        this.useIndent = !(this.space == undefined || this.space == '')
        this.useAnnotatedGetters = useAnnotatedGetters
    }

    public constructor(filter: FixedArray<string>, space?: String, useAnnotatedGetters: boolean = false) {
        this.replacer = undefined
        this.fieldsFilter = filter
        this.space = JSONWriter.truncateSpace(space)

        this.useReplacer = false
        this.useFieldsFilter = true
        this.useIndent = !(this.space == undefined || this.space == '')
        this.useAnnotatedGetters = useAnnotatedGetters
    }

    public write(obj: Any): String {
        if (this.useReplacer) {
            this.writeObject(this.replacer!('', obj))
        } else {
            this.writeObject(obj)
        }

        return this.buffer.toString()
    }

    private writeObject(obj: Any): void {
        if (obj === null) {
            this.buffer.append('null')
        } else if (obj === undefined) {
            this.buffer.append('')
        } else if (obj instanceof String) {
            this.buffer.append(JSON.stringify(obj as String))
        } else if(obj instanceof Tuple) {
            this.writeTupleValue(obj);
        } else if (obj instanceof Record) {
            this.writeESCompatRecord(obj)
        } else if (obj instanceof JsonReplacer) {
            let temp: Record<String, Any> = obj.jsonReplacer()
            this.writeESCompatRecord(temp)
        } else if (obj instanceof ESValue) {
            this.writeInteropESValue(obj)
        } else if (obj instanceof JsonSerializable) {
            this.buffer.append('"' + obj.toJSON() + '"')
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
            this.writeEscompatArray(obj)
        } else if(obj instanceof Int8Array || obj instanceof Int16Array || obj instanceof Int32Array
                || obj instanceof Uint8Array || obj instanceof BigInt64Array || obj instanceof Uint16Array
                || obj instanceof Uint32Array || obj instanceof Float32Array || obj instanceof Float64Array
                || obj instanceof BigUint64Array || obj instanceof Uint8ClampedArray) {
            this.writeTypedArray(obj)
        } else if (obj instanceof Set || obj instanceof Promise || obj instanceof Map || obj instanceof RegExp) {
            this.buffer.append(JSONObject.EMPTY)
        } else if (obj instanceof JSValue) {
            this.buffer.append(JSON.stringify(obj as JSValue))
        } else if (obj instanceof Object) {
            const objCls = Class.of(obj)
            if (objCls.isFixedArray()) {
                if (obj instanceof ValueArray<boolean>) {
                    this.writeArrayHandleValue(new BoolArrayHandle(obj as ValueArray<boolean>))
                } else if (obj instanceof ValueArray<byte>) {
                    this.writeArrayHandleValue(new ByteArrayHandle(obj as ValueArray<byte>))
                } else if (obj instanceof ValueArray<char>) {
                    this.writeArrayHandleValue(new CharArrayHandle(obj as ValueArray<char>))
                } else if (obj instanceof ValueArray<short>) {
                    this.writeArrayHandleValue(new ShortArrayHandle(obj as ValueArray<short>))
                } else if (obj instanceof ValueArray<int>) {
                    this.writeArrayHandleValue(new IntArrayHandle(obj as ValueArray<int>))
                } else if (obj instanceof ValueArray<long>) {
                    this.writeArrayHandleValue(new LongArrayHandle(obj as ValueArray<long>))
                } else if (obj instanceof ValueArray<float>) {
                    this.writeArrayHandleValue(new FloatArrayHandle(obj as ValueArray<float>))
                } else if (obj instanceof ValueArray<double>) {
                    this.writeArrayHandleValue(new DoubleArrayHandle(obj as ValueArray<double>))
                } else {
                    // NOTE(aantipina): Issue #31995
                    this.writeArrayValue(obj as FixedArray<Any>)
                }
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
                this.buffer.append('\"').append(value).append("\"")
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

        const objClass = Class.of(obj)

        const writableFields = this.getWritableFields(objClass, obj)

        let fieldDumped = false
        if (writableFields.length > 0) {
            this.path.add(obj)
            fieldDumped = this.writeTupleElements(writableFields)
            this.path.delete(obj)
        }

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

    private writeTupleElements(tupleFields: Array<[string, Any]>): boolean {
        const FIELD_VALUE = 1

        let fieldDumped = false
        for (let fieldIdx = 0; fieldIdx < tupleFields.length; fieldIdx++) {
            const objToDump = tupleFields[fieldIdx][FIELD_VALUE]
            if (fieldDumped) {
                this.buffer.append(',')
                if (this.useIndent) {
                    this.buffer.append('\n')
                }
            } else {
                fieldDumped = true
            }

            this.writePadding()

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

    private writeESCompatRecord<K extends Numeric | string, V>(record: Record<K, V>): void {
        const currentBuffer = this.buffer
        this.buffer = new StringBuilder('{')

        if (this.useIndent) {
            this.buffer.append('\n')
            this.indentLevel += 1
        }

        this.path.add(record)

        let entryDumped = false
        if (this.useFieldsFilter) {
            const writableEntries = this.getRecordWritableEntries(record)

            const ENTRY_KEY = 0
            const ENTRY_VAL = 1
            for (const entry of writableEntries) {
                const entryKey = entry[ENTRY_KEY]
                const entryVal = entry[ENTRY_VAL]
                entryDumped = this.writeRecordEntry(entryKey, entryVal, entryDumped)
            }
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

    private getRecordWritableEntries<K extends Numeric | string, V>(record: Record<K, V>): Array<[K, V]> {
        // 'entry name' -> output position index
        const entryNamePos = new Map<string, int>()
        for (let idx = 0; idx < this.fieldsFilter.length; idx++) {
            const entryName = this.fieldsFilter[idx]
            entryNamePos.set(entryName, idx)
        }

        // 'entry key' -> output position index
        const entryKeyPos = new Array<[K, int]>
        for (const entryKey of record.keys()) {
            const entryName = (entryKey as object).toString()
            const entryPos = entryNamePos.get(entryName)
            if (entryPos === undefined) {
                continue
            }

            const entryKeyPosPair: [K, int] = [entryKey, entryPos]
            entryKeyPos.push(entryKeyPosPair)
        }

        // sorting 'filtered in' entries by position
        const ENTRY_POS = 1
        entryKeyPos.sort((pos1: [K, int], pos2: [K, int]) => {
            const entry1Pos = pos1[ENTRY_POS]
            const entry2Pos = pos2[ENTRY_POS]
            return (entry1Pos < entry2Pos) ? -1 : ((entry1Pos > entry2Pos) ? 1 : 0)
        })

        const writableEntries = new Array<[K, V]>()

        const ENTRY_KEY = 0
        for (const entry of entryKeyPos) {
            const entryKey = entry[ENTRY_KEY]
            const entryVal = record.get(entryKey) as V

            const writableEntry: [K, V] = [entryKey, entryVal]
            writableEntries.push(writableEntry)
        }

        return writableEntries
    }

    private writeRecordEntry<K extends Numeric | string, V>(entryKey: K, entryVal: V, entryDumped: boolean): boolean {
        const entryName = (entryKey as object).toString()
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

    private static isFieldNameAlreadySerialized(name: string, writableFields: Array<[string, Any]>): boolean {
        for (let k = 0; k < writableFields.length; k++) {
            if (writableFields[k][0] == name) {
                return true
            }
        }
        return false
    }

    private collectInterfaceFields(obj: Object, objCls: Class,
        writableFields: Array<[string, Any]>): Array<[string, Any]> {

        const interfaceFields = new Array<[string, Any]>()
        let currentClass: Class | undefined = objCls
        while (currentClass != undefined) {
            const interfaces = currentClass.getInterfaces()
            for (let i = 0; i < interfaces.length; i++) {
                const iface = interfaces[i]
                const gettersValues = JSONWriter.getGettersKeyValuePairs(
                    iface,
                    obj,
                    this.useFieldsFilter,
                    this.fieldsFilter
                )
                for (let j = 0; j < gettersValues.length; j++) {
                    const pair = gettersValues[j]
                    if (!JSONWriter.isFieldNameAlreadySerialized(pair[0], writableFields) &&
                        !JSONWriter.isFieldNameAlreadySerialized(pair[0], interfaceFields)) {
                        interfaceFields.push(pair)
                    }
                }
            }
            currentClass = currentClass.getSuper()
        }
        return interfaceFields
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
        let writableFields = new Array<[string, Any]>()
        if (reflect.isLiteralInitializedInterface(obj)) {
            const gettersValuesRaw = JSONWriter.getGettersKeyValuePairs(
                objCls,
                obj,
                this.useFieldsFilter,
                this.fieldsFilter
            )
            for (let gi = 0; gi < gettersValuesRaw.length; gi++) {
                writableFields.push(gettersValuesRaw[gi])
            }
            const fieldPairs = this.getWritableFields(objCls, obj)
            for (let fi = 0; fi < fieldPairs.length; fi++) {
                const fpair = fieldPairs[fi]
                if (!JSONWriter.isFieldNameAlreadySerialized(fpair[0], writableFields)) {
                    writableFields.push(fpair)
                }
            }
        } else {
            writableFields = this.getWritableFields(objCls, obj)
        }
        if (writableFields.length > 0) {
            this.path.add(obj)
            fieldDumped = this.writeClassFields(writableFields, fieldDumped)
            this.path.delete(obj)
        }

        if (this.useAnnotatedGetters) {
            const annotatedGetters = JSONWriter.getGetterKeyValuePairsAll(
                objCls,
                obj,
                this.useFieldsFilter,
                this.fieldsFilter
            )
            const annotatedFiltered = new Array<[string, Any]>()
            for (let ai = 0; ai < annotatedGetters.length; ai++) {
                const apair = annotatedGetters[ai]
                if (!JSONWriter.isFieldNameAlreadySerialized(apair[0], writableFields)) {
                    annotatedFiltered.push(apair)
                }
            }
            if (annotatedFiltered.length > 0) {
                this.path.add(obj)
                fieldDumped = this.writeClassFields(annotatedFiltered, fieldDumped)
                this.path.delete(obj)
                for (let aj = 0; aj < annotatedFiltered.length; aj++) {
                    writableFields.push(annotatedFiltered[aj])
                }
            }
        }

        const interfaceFields = this.collectInterfaceFields(obj, objCls, writableFields)
        if (interfaceFields.length > 0) {
            this.path.add(obj)
            fieldDumped = this.writeClassFields(interfaceFields, fieldDumped)
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
        if (fieldVal === undefined) {
            return fieldDumped
        }

        if (fieldVal instanceof Function) {
            return fieldDumped
        }

        if (fieldDumped) {
            this.buffer.append(',')
            if (this.useIndent) {
                this.buffer.append('\n')
            }
        } else {
            fieldDumped = true
        }

        this.checkReferencesCycle(fieldVal)
        this.writePadding()

        this.buffer.append('"')
        this.buffer.append(fieldName)
        this.buffer.append('":')

        if (this.useIndent) {
            this.buffer.append(' ')
        }

        this.writeObject(fieldVal)

        return fieldDumped
    }

    private writeClassFields(writableFields: Array<[string, Any]>, fieldDumped: boolean): boolean {
        const FIELD_NAME = 0
        const FIELD_VALUE = 1

        for (let fieldIdx = 0; fieldIdx < writableFields.length; fieldIdx++) {
            const fieldTypeValuePair = writableFields[fieldIdx]

            const objFieldName = fieldTypeValuePair[FIELD_NAME]
            const objFieldValue = fieldTypeValuePair[FIELD_VALUE]

            const objToDump = this.useReplacer ? this.replacer!(objFieldName, objFieldValue) : objFieldValue
            fieldDumped = this.writeField(objFieldName, objToDump, fieldDumped)
        }

        return fieldDumped
    }

    private checkSameRename(fieldName: string, fields: Array<[string, Any]>): boolean {
        return fields.some((writableField) => writableField[0] == fieldName);
    }

    private findKeyIndex(fieldName: string, fields: Array<[string, Any]>): int {
        return fields.findIndex((writableField) => writableField[0] == fieldName);
    }

    private getWritableFields(cls: Class, obj: object): Array<[string, Any]> {
        const writableFields = new Array<[string, Any]>()

        if (this.useFieldsFilter) {
            for (let fieldIdx = 0; fieldIdx < this.fieldsFilter.length; fieldIdx++) {
                const fieldName = this.fieldsFilter[fieldIdx]
                const metadata = FieldMetadataCache.getMetadataByName(cls, fieldName)
                if (metadata === undefined) {
                    continue
                }
                if (metadata.ignore) {
                    continue
                }
                let writableFieldName = metadata.getName()
                if (metadata.rename !== undefined) {
                    const renamedMetadata = FieldMetadataCache.getMetadataByName(cls, metadata.rename!)
                    if (renamedMetadata !== undefined) {
                        throw new Error(`Double usage of \`${metadata.rename}\` in ${cls.getName()}.` +
                            ' The name also is used by JSONRename in base class')
                    }
                    if (this.checkSameRename(writableFieldName, writableFields)) {
                        throw new Error(`Cannot double rename ${writableFieldName} in keys of ${cls.getName()}`)
                    }
                }
                const fieldNameValuePair: [string, Any] = [writableFieldName, metadata.getValue(obj)]
                writableFields.push(fieldNameValuePair)
            }
        } else {
            const metadataArray = FieldMetadataCache.getMetadata(cls)
            for (let i = 0; i < metadataArray.length; i++) {
                const metadata = metadataArray[i]
                if (metadata.ignore) {
                    continue
                }
                let writableFieldName = metadata.getName()
                if (metadata.rename !== undefined) {
                    const renamedMetadata = FieldMetadataCache.getMetadataByName(cls, metadata.rename!)
                    if (renamedMetadata !== undefined) {
                        throw new Error(`Double usage of \`${metadata.rename}\` in ${cls.getName()}.` +
                            ' The name also is used by JSONRename in base class')
                    }
                    if (this.checkSameRename(writableFieldName, writableFields)) {
                        throw new Error(`Cannot double rename ${writableFieldName} in keys of ${cls.getName()}`)
                    }
                }
                const fieldNameValuePair: [string, Any] = [writableFieldName, metadata.getValue(obj)]
                const index: int = this.findKeyIndex(metadata.name, writableFields)
                if (metadata.rename !== undefined || index == -1) {
                    writableFields.push(fieldNameValuePair)
                } else {
                    writableFields[index] = fieldNameValuePair
                }
            }
        }

        return writableFields
    }

    /**
    * @brief Returns values of public getters of a class.
    * Searches only within direct methods of the class. Does not apply fields filter.
    */
    private static getGettersKeyValuePairs(cls: Class, obj: Object, useFieldsFilter: boolean,
        fieldsFilter: FixedArray<string>) {

        const namesAndValues = new Array<[string, Any]>()
        const clsMethods = cls.getInstanceMethods()
        for (const method of clsMethods) {
            if (!method.isGetter()) {
                continue
            }
            const name = method.getName().substring(JSONWriter.GETTER_PREFIX_LEN)
            if (useFieldsFilter && !JSONWriter.searchField(fieldsFilter, name)) {
                continue
            }
            if (JSONWriter.isFieldNameAlreadySerialized(name, namesAndValues)) {
                continue
            }
            const value = method.invoke(obj)
            namesAndValues.push([name, value])
        }
        return namesAndValues
    }

    /**
    * @brief Returns values of `@JSONStringifyGetter`-annotated `get` accessor properties
    * across the class hierarchy.
    * A method is included only if it both carries the `@JSONStringifyGetter` annotation
    * AND has the `%%get-` bytecode name prefix (length 6) that identifies a `get` accessor.
    * The prefix is stripped to produce the JSON key (e.g. `%%get-area` → key `"area"`).
    */
    private static getGetterKeyValuePairsAll(cls: Class, obj: Object, useFieldsFilter: boolean,
        fieldsFilter: FixedArray<string>): Array<[string, Any]> {

        const namesAndValues = new Array<[string, Any]>()
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
                if (useFieldsFilter && !JSONWriter.searchField(fieldsFilter, name)) {
                    continue
                }
                if (JSONWriter.isFieldNameAlreadySerialized(name, namesAndValues)) {
                    continue
                }
                const value = method.invoke(obj)
                namesAndValues.push([name, value])
            }
            currentClass = currentClass.getSuper()
        }
        return namesAndValues
    }

    private static searchField(fieldsFilter: FixedArray<string>, name: string): boolean {
        for (let i = 0; i < fieldsFilter.length; i++) {
            if (fieldsFilter[i] == name) {
                return true
            }
        }
        return false
    }

    private writeArrayValue<E>(array: FixedArray<E>): void {
        this.buffer.append('[')

        if (this.useIndent) {
            this.buffer.append('\n')
            this.indentLevel += 1
        }

        const arrayLength = array.length
        if (arrayLength > 0) {
            this.path.add(array)

            const lastElementIndex = arrayLength - 1

            for (let idx = 0; idx < lastElementIndex; idx++) {
                this.writeArrayElementValue(array[idx], idx)
                this.buffer.append(',')

                if (this.useIndent) {
                    this.buffer.append('\n')
                }
            }

            this.writeArrayElementValue(array[lastElementIndex], lastElementIndex)

            this.path.delete(array)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append('\n')
        }

        this.writePadding()

        this.buffer.append(']')
    }

    private writeArrayHandleValue(handle: ArrayHandle): void {
        this.buffer.append('[')

        if (this.useIndent) {
            this.buffer.append('\n')
            this.indentLevel += 1
        }

        const arrayLength = handle.length
        if (arrayLength > 0) {
            this.path.add(handle.array)

            const lastElementIndex = arrayLength - 1

            for (let idx = 0; idx < lastElementIndex; idx++) {
                this.writeArrayElementValue(handle.get(idx), idx)
                this.buffer.append(',')

                if (this.useIndent) {
                    this.buffer.append('\n')
                }
            }

            this.writeArrayElementValue(handle.get(lastElementIndex), lastElementIndex)

            this.path.delete(handle.array)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append('\n')
        }

        this.writePadding()

        this.buffer.append(']')
    }

    private writeArrayElementValue(elemValue: Any, elemIndex: int): void {
        this.writeObjectsArrayElement(elemValue, elemIndex)
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

            const lastElementIndex: int = array.length - 1

            for (let idx: int = 0; idx < lastElementIndex; idx++) {
                this.buffer.append('"')
                this.buffer.append(idx)
                this.buffer.append('"')
                this.buffer.append(':')

                if (array instanceof BigInt64Array | BigUint64Array) {
                    this.writeObjectsArrayElement((array as ArrayLike<bigint>)[idx], idx)
                } else {
                    this.writeObjectsArrayElement((array as ArrayLike<Double>)[idx], idx)
                }
                this.buffer.append(',')

                if (this.useIndent) {
                    this.buffer.append('\n')
                }
            }

            this.buffer.append('"')
            this.buffer.append(lastElementIndex)
            this.buffer.append('"')
            this.buffer.append(':')

            if (array instanceof BigInt64Array | BigUint64Array) {
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

    private writeEscompatArray<T>(array: Array<T>): void {
        this.buffer.append('[')

        if (this.useIndent) {
            this.buffer.append('\n')
            this.indentLevel += 1
        }

        if (array.length > 0) {
            this.path.add(array)

            const lastElementIndex: int = array.length - 1

            for (let idx = 0; idx < lastElementIndex; idx++) {
                this.writeObjectsArrayElement(array[idx], idx)
                this.buffer.append(',')

                if (this.useIndent) {
                    this.buffer.append('\n')
                }
            }

            this.writeObjectsArrayElement(array[lastElementIndex], lastElementIndex)

            this.path.delete(array)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append('\n')
        }

        this.writePadding()

        this.buffer.append(']')
    }

    private writeObjectsArrayElement(arrayElement: Any, elementIndex: int): void {
        this.writePadding()

        const replacedElem = this.useReplacer ? this.replacer!('' + elementIndex, arrayElement) : arrayElement

        if (replacedElem == null || replacedElem instanceof Function) {
            this.buffer.append('null')
        } else {
            this.checkReferencesCycle(replacedElem)
            this.writeObject(replacedElem)
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
            const indent = this.space!.repeat(this.indentLevel)
            this.buffer.append(indent)
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
            const it = elem.$_iterator()
            const entries = Array.from(it)
            if (entries.length == 0) {
                return '{}'
            }
            let objectResult: string = '{'
            if (hasIndentation) {
                objectResult = objectResult.concat('\n')
            }
            for (let i = 0; i < entries.length; i++) {
                const key = entries[i][0]
                const value = entries[i][1]
                if (value.jsonType == jsonx.JsonType.JsonUndefined) {
                    continue
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
                if (i < entries.length - 1) {
                    objectResult = objectResult.concat(',')
                }
                if (hasIndentation) {
                    objectResult = objectResult.concat('\n')
                }
            }
            if (hasIndentation) {
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

function getIndentation(space?: int | string, depth: int = 0): string {
    if (space == undefined) {
        return ''
    }
    if (space! instanceof string)
    {
        let unit = (space! as string).substring(0, 10)
        return unit.repeat(depth)
    }
    let lower = (space! as int) > 0
        ? (space! as int)
        : 0
    let aligned = lower > 10
        ? 10
        : lower
    return ' '.repeat(aligned).repeat(depth)
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
    private static readonly ESCAPE_MAP: Map<char, string> = new Map<char, string>([
        [c'"', "\""], [c'\\', "\\"], [c'/', "/"],
        [c'b', "\b"], [c'f', "\f"], [c'n', "\n"],
        [c'r', "\r"], [c't', "\t"]
    ])

    private source: string
    private index: int
    private length: int
    private ch: char
    private reviver?: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement
    private bigIntOpt: int = 0

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
            let key = this.parseStringRaw()

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
        if ((ch == c'n' || ch == c't' || ch == c'f') && idx + 4 <= this.length) {
            let sub = this.source.substring(idx, idx + 4)
            if (sub === "null") {
                this.advanceBy(4)
                return null
            }
            if (sub === "true") {
                this.advanceBy(4)
                return true
            }
            if (sub === "fals" && idx + 5 <= this.length && this.source.charCodeAt(idx + 4) === c'e') {
                this.advanceBy(5)
                return false
            }
            throw new JsonParseError(`Expected '${ch == c'n' ? "null" : ch == c't' ? "true" : "false"}' at ${this.getPositionDesc()}`, idx, idx + 3)
        }

        switch (this.ch) {
            case c'"':
                return this.parseStringRaw()
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
        if ((ch == c'n' || ch == c't' || ch == c'f') && idx + 4 <= this.length) {
            let sub = this.source.substring(idx, idx + 4)
            if (sub === "null") {
                this.advanceBy(4)
                return jsonx.JsonElement.createNull()
            }
            if (sub === "true") {
                this.advanceBy(4)
                return jsonx.JsonElement.createBoolean(true)
            }
            if (sub === "fals" && idx + 5 <= this.length && this.source.charCodeAt(idx + 4) === c'e') {
                this.advanceBy(5)
                return jsonx.JsonElement.createBoolean(false)
            }
            throw new JsonParseError(`Expected '${ch == c'n' ? "null" : ch == c't' ? "true" : "false"}' at ${this.getPositionDesc()}`, idx, idx + 3)
        }

        switch (this.ch) {
            case c'"':
                return this.parseString()
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

    private parseString(): jsonx.JsonElement {
        this.advance()
        let start = this.index

        while (this.index < this.length) {
            if (this.ch == c'\\') {
                return this.parseStringWithEscape(start)
            }
            if (this.ch == c'"') {
                let result = this.source.substring(start, this.index)
                this.advance()
                return jsonx.JsonElement.createString(result)
            }
            this.advance()
        }

        throw new JsonParseError(`Unterminated string at ${this.getPositionDesc()}`, start, this.index)
    }

    private parseStringWithEscape(start: int): jsonx.JsonElement {
        let result = new StringBuilder()
        let lastEnd = start

        while (this.index < this.length) {
            if (this.ch == c'\0') {
                throw new JsonParseError(`Unterminated string at ${this.getPositionDesc()}`, start, this.index)
            }
            let currentChar = this.ch
            this.advance()

            if (currentChar == c'\\') {
                if (lastEnd < this.index - 1) {
                    result.append(this.source.substring(lastEnd, this.index - 1))
                }
                result.append(this.parseEscape())
                lastEnd = this.index
            } else if (currentChar == c'"') {
                if (lastEnd < this.index - 1) {
                    result.append(this.source.substring(lastEnd, this.index - 1))
                }
                return jsonx.JsonElement.createString(result.toString())
            }
        }

        throw new JsonParseError(`Unterminated string at ${this.getPositionDesc()}`, start, this.index)
    }

    private parseStringRaw(): string {
        this.advance()
        let start = this.index

        while (this.index < this.length) {
            if (this.ch == c'\\') {
                return this.parseStringWithEscapeRaw(start)
            }
            if (this.ch == c'"') {
                let result = this.source.substring(start, this.index)
                this.advance()
                return result
            }
            this.advance()
        }

        throw new JsonParseError(`Unterminated string at ${this.getPositionDesc()}`, start, this.index)
    }

    private parseStringWithEscapeRaw(start: int): string {
        let result = ""
        let lastEnd = start

        while (this.index < this.length) {
            if (this.ch == c'\0') {
                throw new JsonParseError(`Unterminated string at ${this.getPositionDesc()}`, start, this.index)
            }
            let currentChar = this.ch
            this.advance()

            if (currentChar == c'\\') {
                if (lastEnd < this.index - 1) {
                    result = result.concat(this.source.substring(lastEnd, this.index - 1))
                }
                result = result.concat(this.parseEscape())
                lastEnd = this.index
            } else if (currentChar == c'"') {
                if (lastEnd < this.index - 1) {
                    result = result.concat(this.source.substring(lastEnd, this.index - 1))
                }
                return result
            }
        }

        throw new JsonParseError(`Unterminated string at ${this.getPositionDesc()}`, start, this.index)
    }

    private parseEscape(): string {
        if (this.index >= this.length) {
            throw new JsonParseError(`Incomplete escape sequence at ${this.getPositionDesc()}`,
                this.index, this.index)
        }

        let escChar = this.ch
        this.advance()
        switch (escChar) {
            case c'"':
                return "\""
            case c'\\':
                return "\\"
            case c'/':
                return "/"
            case c'b':
                return "\b"
            case c'f':
                return "\f"
            case c'n':
                return "\n"
            case c'r':
                return "\r"
            case c't':
                return "\t"
            case c'u':
                return this.parseUnicodeEscape()
            default:
                throw new JsonParseError(`Invalid escape sequence '\\${escChar}' at ${this.getPositionDesc()}`,
                    this.index - 2, this.index)
        }
    }

    private parseUnicodeEscape(): string {
        if (this.index + 4 > this.length) {
            throw new JsonParseError(`Incomplete Unicode escape sequence at ${this.getPositionDesc()}`,
                this.index, this.index)
        }

        let hexStr = this.source.substring(this.index, this.index + 4)

        for (let i = 0; i < 4; i++) {
            let c = hexStr.charAt(i)
            if (!((c >= c'0' && c <= c'9') || (c >= c'a' && c <= c'f') || (c >= c'A' && c <= c'F'))) {
                throw new JsonParseError(`Invalid hexadecimal digit '${c}' at ${this.getPositionDesc()}`,
                    this.index + i, this.index + i + 1)
            }
        }

        this.advanceBy(4)

        let codePoint = Double.parseInt(hexStr, 16).toInt()
        return String.fromCharCode(codePoint)
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
            let key = this.parseStringRaw()

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
        while (this.index < this.length) {
            if (this.ch != c' ' && this.ch != c'\t' && this.ch != c'\n' && this.ch != c'\r') {
                break
            }
            this.advance()
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
        if ((ch == c'n' || ch == c't' || ch == c'f') && idx + 4 <= this.length) {
            let sub = this.source.substring(idx, idx + 4)
            if (sub === "null") {
                this.advanceBy(4)
                return new JSONNull()
            }
            if (sub === "true") {
                this.advanceBy(4)
                return new JSONTrue()
            }
            if (sub === "fals" && idx + 5 <= this.length && this.source.charCodeAt(idx + 4) === c'e') {
                this.advanceBy(5)
                return new JSONFalse()
            }
        }

        switch (this.ch) {
            case c'"':
                return this.parseStringToJSON()
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

    private parseStringToJSON(): JSONString {
        this.advance()
        let start = this.index

        while (this.index < this.length) {
            if (this.ch == c'\\') {
                return this.parseStringWithEscapeToJSON(start)
            }
            if (this.ch == c'"') {
                let result = this.source.substring(start, this.index)
                this.advance()
                let jsonStr = new JSONString()
                jsonStr.value = result
                return jsonStr
            }
            this.advance()
        }

        throw new JsonParseError(`Unterminated string at ${this.getPositionDesc()}`, start, this.index)
    }

    private parseStringWithEscapeToJSON(start: int): JSONString {
        let result = ""
        let lastEnd = start

        while (this.index < this.length) {
            if (this.ch == c'\0') {
                throw new JsonParseError(`Unterminated string at ${this.getPositionDesc()}`, start, this.index)
            }
            let currentChar = this.ch
            this.advance()

            if (currentChar == c'\\') {
                if (lastEnd < this.index - 1) {
                    result = result.concat(this.source.substring(lastEnd, this.index - 1))
                }
                result = result.concat(this.parseEscape())
                lastEnd = this.index
            } else if (currentChar == c'"') {
                if (lastEnd < this.index - 1) {
                    result = result.concat(this.source.substring(lastEnd, this.index - 1))
                }
                let jsonStr = new JSONString()
                jsonStr.value = result
                return jsonStr
            }
        }

        throw new JsonParseError(`Unterminated string at ${this.getPositionDesc()}`, start, this.index)
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

            let key = this.parseStringRaw()

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
            return this.parseArray(value as JSONArray, type)
        } else if (value instanceof JSONNumber) {
            const num = this.createNumber(value as JSONNumber, type)
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
            return (value as JSONString).value.charAt(0)
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
            return this.parseObject(value as JSONObject, type)
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
        let setClassFieldFn = (obj: object, fields: Array<reflect.InstanceField>, fieldName: string, fieldVal: Any) => {
            const field = findFieldByName(fields, fieldName)!
            field.setValue(obj, fieldVal)
        }

        if (this.reviver != undefined) {
            setClassFieldFn = (obj: object, fields: Array<reflect.InstanceField>, fieldName: string, fieldVal: Any) =>
            {
                const revivedVal = this.reviver!(fieldName, fieldVal)

                const field = findFieldByName(fields, fieldName)!
                field.setValue(obj, revivedVal)
            }
        }

        const obj = JSONValueParser.classGetDefaultCtor(classType).createInstance() as object
        let hasJsonParseIgnore: boolean = false
        let hasJsonStringifyIgnore: boolean = false
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
            setClassFieldFn(obj, classFields, classField.getName(), classFieldVal)
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
            return this.parseObjectInto(value as JSONObject, instance)
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
                    this.parseObjectInto(jsonFieldVal as JSONObject, existingFieldVal as object)
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
            let elem = this.parse(jVal.values.at(i) as JSONValue, componentType)
            setArrayElementFn(arr, i, elem)
        }
        return arr
    }

    private parsePrimitiveFixedArray(jVal: JSONArray, componentType: Class, len: int): Object {
        const parsed = new Array<Any>()
        for (let i = 0; i < len; i++) {
            let elem = this.parse(jVal.values.at(i) as JSONValue, componentType)
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
            return Char.valueOf(str.charAt(0))
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
            return Int.valueOf(Double.toInt(num))
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
        if (Double.isInteger(num) && Long.MIN_VALUE <= num && num <= Long.MAX_VALUE) {
            return num.toLong()
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
        return Double.valueOf((json as JSONNumber).value)
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

