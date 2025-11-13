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

package std.core

const BigIntType: Class;

export interface JsonReplacer {
    jsonReplacer(): Record<String, Any>
}

// NOTE (chengzhihao): $Partial is not generated for exported interfaces.issue #26111
// now use this class to throw the error.
export class JsonReplacerTemp implements JsonReplacer {
    jsonReplacer(): Record<String, Any> {
        throw new Error("frontend bug");
    }
}

export @interface JSONRename {
    newName: string
}

export @interface JSONStringifyIgnore {
}

export @interface JSONParseIgnore {
}

function searchInstanceFieldInHierarchy(cls: Class, name: string): reflect.InstanceField | undefined {
    let currClass: Class | undefined = cls
    while(currClass != undefined) {
        const candidate = currClass.getInstanceField(name)
        if (candidate != undefined) {
            return candidate
        }
        currClass = currClass.getSuper()
    }
    return undefined
}

function getClassInstanceFieldsAll(cls: Class): Array<reflect.InstanceField> {
    const baseClasses = new Array<Class>()
    while (cls.getSuper() !== undefined) {
        baseClasses.push(cls)
        cls = cls.getSuper()!
    }

    const allFields = new Map<string, reflect.InstanceField>()
    const baseClassesCount = baseClasses.length
    for (let i = baseClassesCount - 1; i >= 0; i--) {
        const currentCls = baseClasses[i]
        const clsFields = currentCls.getInstanceFields()
        for (const field of clsFields) {
            const qfName = field.getOwner().getName() + "::" + field.getName()
            allFields.set(qfName, field)
        }
    }

    return Array.from(allFields.values())
}

class JSONAPI {
    native static getJSONStringifyIgnore(field: reflect.InstanceField): boolean

    native static getJSONParseIgnore(field: reflect.InstanceField): boolean

    native static getJSONRename(field: reflect.InstanceField): string | undefined
}

export class JSON {
    /**
     * Converts byte to JSON format
     *
     * @param d: byte - byte to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of byte
     */
    public static stringifyByte(d: byte): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts short to JSON format
     *
     * @param d: short - short to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of short
     */
    public static stringifyShort(d: short): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts int to JSON format
     *
     * @param d: int - int to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of int
     */
    public static stringifyInt(d: int): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts long to JSON format
     *
     * @param d: long - long to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of long
     */
    public static stringifyLong(d: long): String {
        return StringBuilder.toString(d)
    }

    /**
     * Converts float to JSON format
     *
     * @param d: float - float to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of float
     */
    public static stringifyFloat(d: float): String {
        if (Float.isFinite(d)) {
            return StringBuilder.toString(d)
        } else {
            return "null"
        }
    }

    /**
     * Converts double to JSON format
     *
     * @param d: double - double to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of double
     */
    public static stringifyDouble(d: double): String {
        if (Double.isFinite(d)) {
            return StringBuilder.toString(d)
        } else {
            return "null"
        }
    }

    /**
     * Converts bigint to JSON format
     *
     * @param d: bigint - bigint to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of bigint
     */
    public static stringifyBigint(d: bigint): String {
        return d.toString()
    }

    /**
     * Converts char to JSON format
     *
     * @param d: char - char to be converted to a JSON as a String
     *
     * @returns String - JSON representation of char
     */
    public static stringifyChar(d: char): String {
        return "\"" + StringBuilder.toString(d) + "\""
    }

    /**
     * Converts boolean to JSON format
     *
     * @param d: boolean - boolean to be converted to a JSON as a Boolean literal
     *
     * @returns String - JSON representation of boolean
     */
    public static stringifyBoolean(d: boolean): String {
        if (d) {
            return "true"
        }
        return "false"
    }

    private static readonly ESCAPED_CHARS: FixedArray<char> = [c'\"', c'\\', c'\b', c'\f', c'\n', c'\r', c'\t']
    private static readonly NON_ESCAPED_CHARS: FixedArray<char> = [c'"', c'\\', c'b', c'f', c'n', c'r', c't']
    private static readonly JSON_SPACE_INDENT_LIMIT = 10

    /**
     * Converts String to JSON format
     *
     * @param d: String - byte to be converted to a JSON as a String
     *
     * @returns String - JSON representation of byte
     */
    public static stringifyString(d: String): String {
        let sb = new StringBuilder([c'\"'] as FixedArray<char>)
        let len = d.getLength()

        let prevChar: char = c'\0'
        for (let i = 0; i < len; ++i) {
            let currChar = d.charAt(i)

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

                let escapedCharIndex = lastIndexOf(JSON.ESCAPED_CHARS, currChar, JSON.ESCAPED_CHARS.length)
                if (escapedCharIndex != -1) {
                    sb.append(c'\\')
                    sb.append(JSON.NON_ESCAPED_CHARS[escapedCharIndex])
                } else if ((currChar.compareTo(c'\x1f') <= 0) && (currChar.compareTo(c'\x00') >= 0)) {
                    if (currChar.compareTo(c'\x0f') <= 0) {
                        sb.append('\\u000')
                    } else {
                        sb.append('\\u00')
                    }
                    sb.append(currChar.toInt().toString(16))
                } else {
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

        sb.append(c'\"')
        return sb.toString()
    }

    private static appendCharHex(buffer: StringBuilder, chr: char): StringBuilder {
        buffer.append("\\u").append(chr.toInt().toString(16))
        return buffer
    }

    /**
     * Converts an object to a JavaScript Object Notation (JSON) string.
     *
     * @param obj: Object An object to be converted.
     *
     * @returns String - JSON representation of Object
     */
    public static stringifyAnyObj(obj: Any): String {
        if (obj === undefined) {
            return "undefined"
        }
        const json = JSON.stringifyFast(obj)
        if (json !== undefined) {
            return json
        }
        return new JSONWriter().write(obj)
    }

    private static native stringifyFast(obj: Any): string

    public static stringifyJsonReplacer(obj: JsonReplacer): String {
        const record = obj.jsonReplacer();
        return JSON.stringify(record);
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
    public static stringifyObjValue(
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
    public static stringifyDoubleStringFixedArray(obj: Any, replacer: FixedArray<double | string>, space?: int | string): string {
        const filter : FixedArray<string> = new string[replacer.length]
        for (let i = 0; i < replacer.length; i++) {
            filter[i] = `${replacer[i]}`
        }

        if (space instanceof String || space === undefined) {
            return new JSONWriter(filter, space).write(obj)
        } else {
            return new JSONWriter(filter, JSON.spaceFromNumber(space! as int)).write(obj)
        }
    }

    public static stringifyDoubleStringArray(obj: Any, replacer: Array<double | string>, space?: int | string): string {
        const filter : FixedArray<string> = new string[replacer.length]
        for (let i = 0; i < replacer.length; i++) {
            filter[i] = `${replacer[i]}`
        }

        if (space instanceof String || space === undefined) {
            return new JSONWriter(filter, space).write(obj)
        } else {
            return new JSONWriter(filter, JSON.spaceFromNumber(space! as int)).write(obj)
        }
    }

    public static stringifyStringArray(obj: Any, replacer: Array<string>, space?: int | string): string {
        const filter : FixedArray<string> = new string[replacer.length]
        for (let i = 0; i < replacer.length; i++) {
            filter[i] = `${replacer[i]}`
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

        return " ".repeat(spacesCount)
    }

    public static native stringifyJSValue(d: JSValue): String;

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
    public static stringifyByteFixedArray(d: FixedArray<byte>): String {
        let s = new StringBuilder("[")
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

    /**
     * Converts shorts array to JSON format
     *
     * @param d: FixedArray<short> - shorts array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of shorts array
     */
    public static stringifyShortFixedArray(d: FixedArray<short>): String {
        let s = new StringBuilder("[")
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

    /**
     * Converts ints array to JSON format
     *
     * @param d: FixedArray<int> - ints array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of ints array
     */
    public static stringifyIntFixedArray(d: FixedArray<int>): String {
        let s = new StringBuilder("[")
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

    /**
     * Converts longs array to JSON format
     *
     * @param d: FixedArray<long> - longs array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of longs array
     */
    public static stringifyLongFixedArray(d: FixedArray<long>): String {
        let s = new StringBuilder("[")
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

    /**
     * Converts array of bytes to JSON format
     *
     * @param d: FixedArray<byte> - array of byte to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of array of bytes
     */
    public static stringifyFloatFixedArray(d: FixedArray<float>): String {
        let s = new StringBuilder("[")
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

    /**
     * Converts doubles array to JSON format
     *
     * @param d: FixedArray<double> - doubles array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of doubles array
     */
    public static stringifyDoubleFixedArray(d: FixedArray<double>): String {
        let s = new StringBuilder("[")
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

    /**
     * Converts chars array to JSON format
     *
     * @param d: FixedArray<char> - chars array  to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of chars array
     */
    public static stringifyCharFixedArray(d: FixedArray<char>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append("\"" + d[i] + "\"")
            s.append(',')
        }
        if (d.length > 0) {
            s.append("\"" + d[last_elem] + "\"")
        }
        s.append(']')
        return s.toString()
    }

    /**
     * Converts booleans array to JSON format
     *
     * @param d: FixedArray<boolean> - booleans array to be converted to a JSON as an Array of Boolean literals
     *
     * @returns String - JSON representation of booleans array
     */
    public static stringifyBooleanFixedArray(d: FixedArray<boolean>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            if (d[i]) {
                s.append("true,")
            } else {
                s.append("false,")
            }
        }
        if (d.length > 0) {
            s.append(d[last_elem])
        }
        s.append(']')
        return s.toString()
    }

    public static stringifyNumberArray(d: Array<Number>): String {
        let s = new StringBuilder("[")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append(JSON.stringify(d[i]));
            s.append(",");
        }
        if (d.length > 0) {
            s.append(JSON.stringify(d[last_elem]));
        }
        s.append(']');
        return s.toString()
    }

    public static stringifyNumberArrayLike(d: ArrayLike<Number>): String {
        let s = new StringBuilder("{")
        let last_elem = d.length - 1
        for (let i = 0; i < last_elem; ++i) {
            s.append("\""+JSON.stringify(i)+"\"");
            s.append(":");
            s.append(JSON.stringify(d[i]));
            s.append(",");
        }
        if (d.length > 0) {
            s.append("\""+JSON.stringify(last_elem)+"\"");
            s.append(":");
            s.append(JSON.stringify(d[last_elem]));
        }
        s.append('}');
        return s.toString()
    }

    public static overload stringify {
        stringifyByte, stringifyChar, stringifyInt, stringifyShort, stringifyLong,
        stringifyBigint, stringifyFloat, stringifyDouble, stringifyBoolean, stringifyString,
        stringifyByteFixedArray, stringifyCharFixedArray, stringifyShortFixedArray, stringifyIntFixedArray,
        stringifyLongFixedArray,  stringifyFloatFixedArray, stringifyDoubleFixedArray,
        stringifyBooleanFixedArray, stringifyDoubleStringFixedArray, stringifyNumberArray,
        stringifyNumberArrayLike, stringifyStringArray, stringifyDoubleStringArray,
        stringifyJSValue, stringifyAnyObj, stringifyJsonReplacer, stringifyObjValue,
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
    public static stringifyJsonElement(elem: jsonx.JsonElementSerializable, replacer?: (double | string)[], space?: int | string): string {
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
    public static stringifyJsonElement(elem: jsonx.JsonElement, replacer?: (double | string)[], space?: int | string): string {
        return stringifyElementRecursively(elem, replacer, space, 0)
    }

    private static IsBoxedType(typ : Type) : boolean {
        return typ instanceof BooleanType
                || typ instanceof ByteType
                || typ instanceof CharType
                || typ instanceof ShortType
                || typ instanceof IntType
                || typ instanceof LongType
                || typ instanceof FloatType
                || typ instanceof DoubleType
    }

    // TODO(kirill-mitkin): Map<Long, Long> blocked by internal issue
    private static checkType(type: Class, used: Array<TypeColor>): boolean {
        let ind = used.findIndex((tc: TypeColor) => type === tc.type)
        if (ind != -1) {
            return (used.at(ind) as TypeColor).color == TypeColor.VISITED
        }
        used.push(new TypeColor(type, TypeColor.AT_STACK))
        let ok = true
        if (type === BigIntType) {
            ok = true
        } else if (type.isFixedArray()) {
            ok = JSON.checkType(type.getFixedArrayComponentType()!, used)
        } else if (type.isUnion()) {
            // NOTE(lyupaanastasia) parse union #27136
            throw new Error("UnionType is not supported yet")
        } else if (type === TypeColor.STRING_CLASS || TypeColor.isPrimitiveType(type) || type === TypeColor.NULL_CLASS) {
            ok = true
        } else if (type.isSubtypeOf(TypeColor.OBJECT_CLASS)) {
            ok &= JSON.classHasDefaultCtor(type)
            for (const field of getClassInstanceFieldsAll(type)) {
                if (JSONAPI.getJSONParseIgnore(field)) {
                    continue
                }
                ok &= JSON.checkType(field.getType(), used)
            }
        } else {
            ok = false
        }
        (used.at(ind) as TypeColor).color = TypeColor.VISITED
        return ok
    }

    private static classHasDefaultCtor(cls: Class): boolean {
        for (const ctor of cls.getConstructors()) {
            if (ctor.getParametersNum() == 0) {
                return true
            }
        }
        return false
    }

    // TODO(kirill-mitkin): For testing purpose only, shpuld be replaced by error checking
    public static isCorrectForParsing(type: Class): boolean {
        return JSON.checkType(type, new Array<TypeColor>())
    }

    public static parse<T>(text: String, type: Type): Nullish<T> {
        const cls = type.getClass()
        if (cls !== undefined) {
            return JSON.parse<T>(text, undefined, cls)
        }
        throw new Error(`Unable to resolve type ${type} runtime class`)
    }

    public static parse<T>(text: string, reviver: ((key: string, value: Any) => Any) | undefined, type: Type, options?: jsonx.ParseOptions): Nullish<T> {
        const cls = type.getClass()
        if (cls !== undefined) {
            return JSON.parse<T>(text, reviver, cls, options)
        }
        throw new Error(`Unable to resolve type ${type} runtime class`)
    }

    public static parse<T>(json: string, type: Class): Nullish<T> {
        return JSON.parse<T>(json, undefined, type)
    }

    public static parse<T>(json: string, reviver: ((key: string, value: Any) => Any) | undefined, type: Class, options?: jsonx.ParseOptions): Nullish<T> {
        if (!JSON.checkType(type, new Array<TypeColor>())) {
            throw new Error("Incorrect type: " + type.getName())
        }

        let bigIntOpt: int = options ? options.bigIntMode as int : 0
        if (bigIntOpt > 2 || bigIntOpt < 0) {
            throw new Error("bigIntMode value should be from 0 to 2, current value is" + bigIntOpt)
        }

        let jsonValue = JSONParser.parse(json)
        const parsingResult = new JSONValueParser(reviver, bigIntOpt).parse(jsonValue, type) as T

        if (reviver !== undefined) {
            return reviver("", parsingResult) as T
        } else {
            return parsingResult
        }
    }

    /**
      * Parses a JSON string and returns a JsonElement.
      * @param {string} text - The JSON string to parse
      * @returns {JsonElement} The parsed JSON element
      * @throws {JsonParseError} If the JSON string is invalid
      */
    static parseJsonElement(text: string): jsonx.JsonElement {
        return new JsonElementParser(text).parse()
    }

    /**
      * Parses a JSON string with a reviver function and returns a JsonElement.
      * @param {string} text - The JSON string to parse
      * @param {(key: string, value: JsonElement) => JsonElement} reviver - Function to transform values
      * @returns {JsonElement} The parsed JSON element
      * @throws {JsonParseError} If the JSON string is invalid
      */
    static parseJsonElement(text: string, reviver: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement): jsonx.JsonElement {
        return new JsonElementParser(text, reviver).parse()
    }
}

class JSONWriter {
    private static readonly STD_CORE_INTEROP_JSVALUE_CLASS = Class.from<JSValue>()

    private readonly replacer: ((key: String, value: Any) => Any) | undefined
    private readonly fieldsFilter: FixedArray<string>
    private readonly space: String | undefined

    private readonly useReplacer: boolean
    private readonly useFieldsFilter: boolean
    private readonly useIndent: boolean

    private indentLevel: int = 0

    private readonly path = new Set<Object>()
    private buffer = new StringBuilder()

    private static asFixedArray<E>(obj: object): FixedArray<E> {
        return obj as FixedArray<E>
    }

    constructor(replacer?: (key: String, value: Any) => Any, space?: String) {
        this.replacer = replacer
        this.fieldsFilter = []
        this.space = space

        this.useReplacer = this.replacer != undefined
        this.useFieldsFilter = false
        this.useIndent = !(this.space == undefined || this.space == "")
    }

    constructor(filter: FixedArray<string>, space?: String) {
        this.replacer = undefined
        this.fieldsFilter = filter
        this.space = space

        this.useReplacer = false
        this.useFieldsFilter = true
        this.useIndent = !(this.space == undefined || this.space == "")
    }

    write(obj: Any): String {
        if (this.useReplacer) {
            this.writeObject(this.replacer!("", obj))
        } else {
            this.writeObject(obj)
        }

        return this.buffer.toString()
    }

    private writeObject(obj: Any): void {
        if (obj === null) {
            this.buffer.append("null")
        } else if (obj === undefined) {
            this.buffer.append("undefined")
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
        } else if (obj instanceof jsonx.JsonElementSerializable) {
            this.buffer.append(JSON.stringifyJsonElement(obj))
        } else if (obj instanceof ArrayBuffer) {
            this.buffer.append(JSONObject.EMPTY)
        } else if (obj instanceof Function) {
            this.buffer.append("undefined")
        } else if (this.writeValueTypeWrapper(obj)) {
            // nothing to do - write completed successfully
        } else if (obj instanceof Date) {
            this.writeObject(obj.toJSON())
        } else if (obj instanceof Array) {
            this.writeEscompatArray(obj)
        } else if (obj instanceof Set || obj instanceof Promise || obj instanceof Map) {
            this.buffer.append(JSONObject.EMPTY)
        } else if (obj instanceof Object) {
            const objCls = Class.of(obj)
            if (objCls === JSONWriter.STD_CORE_INTEROP_JSVALUE_CLASS) {
                this.buffer.append(JSON.stringify(obj as JSValue))
            } else if (objCls.isFixedArray()) {
                this.writeArrayValue(JSONWriter.asFixedArray(obj))
            } else {
                this.writeClassValue(obj, objCls)
            }
        } else {
            throw new AssertionError("Unsupported object type: " + typeof obj)
        }
    }

    private writeTupleValue(obj: Object) {
        const currentBuffer = this.buffer
        this.buffer = new StringBuilder("[")

        if (this.useIndent) {
            this.buffer.append("\n")
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
            this.buffer.append("\n")
        }

        this.writePadding()
        this.buffer.append("]")

        if (fieldDumped) {
            currentBuffer.append(this.buffer.toString())
        } else {
            currentBuffer.append("[]")
        }

        this.buffer = currentBuffer
    }

    private writeTupleElements(tupleFields: Array<[string, Any]>): boolean {
        const FIELD_VALUE = 1

        let fieldDumped = false
        for (let fieldIdx = 0; fieldIdx < tupleFields.length; fieldIdx++) {
            const objToDump = tupleFields[fieldIdx][FIELD_VALUE]
            if (fieldDumped) {
                this.buffer.append(",")
                if (this.useIndent) {
                    this.buffer.append("\n")
                }
            } else {
                fieldDumped = true
            }

            this.writePadding()

            if (this.useIndent) {
                this.buffer.append(" ")
            }

            if (objToDump !== undefined) {
                this.writeObject(objToDump)
            } else {
                this.buffer.append("null")
            }
        }

        return fieldDumped
    }

    private writeESCompatRecord<K extends Numeric | string, V>(record: Record<K, V>): void {
        const currentBuffer = this.buffer
        this.buffer = new StringBuilder("{")

        if (this.useIndent) {
            this.buffer.append("\n")
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
            this.buffer.append("\n")
        }

        this.writePadding()
        this.buffer.append("}")

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

    private writeRecordEntry<K extends Number | string, V>(entryKey: K, entryVal: V, entryDumped: boolean): boolean {
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
                throw new TypeError("cyclic object value")
            } else {
                throw e
            }
        }
    }

    private writeClassValue(obj: Object, objCls: Class): void {
        const currentBuffer = this.buffer
        // we need this local buffer to discard empty object prolog: '{','\n',<space> and replace it with '{}'
        this.buffer = new StringBuilder("{")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        const writableFields = this.getWritableFields(objCls, obj)

        let fieldDumped = false
        if (writableFields.length > 0) {
            this.path.add(obj)
            fieldDumped = this.writeClassFields(writableFields)
            this.path.delete(obj)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append("\n")
        }

        this.writePadding()
        this.buffer.append("}")

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
            this.buffer.append(",")
            if (this.useIndent) {
                this.buffer.append("\n")
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
            this.buffer.append(" ")
        }

        this.writeObject(fieldVal)

        return fieldDumped
    }

    private writeClassFields(writableFields: Array<[string, Any]>): boolean {
        const FIELD_NAME = 0
        const FIELD_VALUE = 1

        let fieldDumped = false
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
        let hasJsonIgnore: boolean = false
        let fieldRename : string | undefined = undefined

        if (this.useFieldsFilter) {
            for (let fieldIdx = 0; fieldIdx < this.fieldsFilter.length; fieldIdx++) {
                const fieldName = this.fieldsFilter[fieldIdx]
                let writableFieldName = fieldName
                const field = searchInstanceFieldInHierarchy(cls, fieldName)
                if (field === undefined) {
                    continue
                }

                hasJsonIgnore = JSONAPI.getJSONStringifyIgnore(field);
                if (hasJsonIgnore) {
                    continue
                }
                fieldRename = JSONAPI.getJSONRename(field);
                if (fieldRename != undefined) {
                    if (searchInstanceFieldInHierarchy(cls, fieldRename) !== undefined) {
                        throw new Error("Cannot rename " + fieldRename + " in keys of " + cls.getName())
                    } else {
                        writableFieldName = fieldRename
                    }
                    if (this.checkSameRename(writableFieldName, writableFields)) {
                        throw new Error("Cannot double rename " + writableFieldName + " in keys of " + cls.getName())
                    }
                }

                const fieldNameValuePair: [string, Any] = [writableFieldName, field.getValue(obj)]
                writableFields.push(fieldNameValuePair)
            }
        } else {
            for (const field of getClassInstanceFieldsAll(cls)) {
                let writableFieldName = field.getName()
                hasJsonIgnore = JSONAPI.getJSONStringifyIgnore(field);
                if (hasJsonIgnore) {
                    continue
                }
                const index: int = this.findKeyIndex(field.getName(), writableFields)
                fieldRename = JSONAPI.getJSONRename(field);
                if (fieldRename !== undefined) {
                    if (searchInstanceFieldInHierarchy(field.getOwner(), fieldRename) !== undefined) {
                        throw new Error("Cannot rename " + fieldRename + " in keys of " + cls.getName())
                    } else {
                        writableFieldName = fieldRename
                    }
                    if (this.checkSameRename(writableFieldName, writableFields)) {
                        throw new Error("Cannot double rename " + writableFieldName + " in keys of " + cls.getName())
                    }
                }

                const fieldNameValuePair: [string, Any] = [writableFieldName, field.getValue(obj)]
                if (fieldRename != undefined || index == -1) {
                    writableFields.push(fieldNameValuePair)
                } else {
                    writableFields[index] = fieldNameValuePair
                }
            }
        }

        return writableFields
    }

    private writeArrayValue<E>(array: FixedArray<E>): void {
        this.buffer.append("[")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        const arrayLength = array.length
        if (arrayLength > 0) {
            this.path.add(array)

            const lastElementIndex = arrayLength - 1

            for (let idx = 0; idx < lastElementIndex; idx++) {
                this.writeArrayElementValue(array[idx], idx)
                this.buffer.append(",")

                if (this.useIndent) {
                    this.buffer.append("\n")
                }
            }

            this.writeArrayElementValue(array[lastElementIndex], lastElementIndex)

            this.path.delete(array)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append("\n")
        }

        this.writePadding()

        this.buffer.append("]")
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

    private writeEscompatArray<T>(array: Array<T>): void {
        this.buffer.append("[")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        if (array.length > 0) {
            this.path.add(array)

            const lastElementIndex = array.length.toInt() - 1

            for (let idx = 0; idx < lastElementIndex; idx++) {
                this.writeObjectsArrayElement(array[idx], idx)
                this.buffer.append(",")

                if (this.useIndent) {
                    this.buffer.append("\n")
                }
            }

            this.writeObjectsArrayElement(array[lastElementIndex], lastElementIndex)

            this.path.delete(array)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append("\n")
        }

        this.writePadding()

        this.buffer.append("]")
    }

    private writeObjectsArrayElement(arrayElement: Any, elementIndex: int): void {
        this.writePadding()

        const replacedElem = this.useReplacer ? this.replacer!("" + elementIndex, arrayElement) : arrayElement

        if (replacedElem == null || replacedElem instanceof Function) {
            this.buffer.append("null")
        } else {
            this.checkReferencesCycle(replacedElem)
            this.writeObject(replacedElem)
        }
    }

    private checkReferencesCycle(obj: Any): void {
        if (obj == null) {
            return
        }

        const cycleDetected = this.path.has(obj as Object)
        if (cycleDetected) {
            throw new TypeError("cyclic object value")
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
function stringifyElementRecursively(elem: jsonx.JsonElement, replacer?: (double | string)[], space?: int | string, depth: int = 0): string {
    const indentStr = getIndentation(space, depth)
    const nextIndentStr = getIndentation(space, depth + 1)
    const hasIndentation = space != undefined && nextIndentStr.length > 0

    switch (elem.jsonType) {
        case jsonx.JsonType.JsonNull:
            return "null"

        case jsonx.JsonType.JsonUndefined:
            return "undefined"

        case jsonx.JsonType.JsonTrue:
            return "true"

        case jsonx.JsonType.JsonFalse:
            return "false"

        case jsonx.JsonType.JsonString:
            return escapeString(elem.asString())

        case jsonx.JsonType.JsonNumber:
            const intValue = elem.tryAsInteger()
            if (intValue != undefined) {
                return intValue.toString()
            }
            const doubleValue = elem.tryAsDouble()
            if (doubleValue != undefined) {
                return doubleValue.toString()
            }
            return "null"

        case jsonx.JsonType.JsonArray:
            const array = elem.asArray()
            if (array.length == 0) {
                return "[]"
            }
            let arrayResult = "["
            if (hasIndentation) {
                arrayResult += "\n"
            }
            for (let i = 0; i < array.length; i++) {
                if (hasIndentation) {
                    arrayResult += nextIndentStr
                }
                if (array[i].jsonType == jsonx.JsonType.JsonUndefined) {
                    arrayResult += "null"
                } else {
                    arrayResult += stringifyElementRecursively(array[i], replacer, space, depth + 1)
                }
                if (i < array.length - 1) {
                    arrayResult += ","
                }
                if (hasIndentation) {
                    arrayResult += "\n"
                }
            }
            if (hasIndentation) {
                arrayResult += indentStr
            }
            arrayResult += "]"
            return arrayResult

        case jsonx.JsonType.JsonObject:
            const it = elem.$_iterator()
            const entries = Array.from(it)
            if (entries.length == 0) {
                return "{}"
            }
            let objectResult = "{"
            if (hasIndentation) {
                objectResult += "\n"
            }
            for (let i = 0; i < entries.length; i++) {
                const key = entries[i][0]
                const value = entries[i][1]
                if (value.jsonType == jsonx.JsonType.JsonUndefined) {
                    continue
                }
                if (hasIndentation) {
                    objectResult += nextIndentStr
                }
                objectResult += escapeString(key)
                objectResult += ":"
                if (hasIndentation) {
                    objectResult += " "
                }
                objectResult += stringifyElementRecursively(value, replacer, space, depth + 1)
                if (i < entries.length - 1) {
                    objectResult += ","
                }
                if (hasIndentation) {
                    objectResult += "\n"
                }
            }
            if (hasIndentation) {
                objectResult += indentStr
            }
            objectResult += "}"
            return objectResult

        default:
            return "null"
    }
}

function escapeString(str: string): string {
    let result = "\""
    for (let i = 0; i < str.length; i++) {
        const ch = str.charAt(i)
        switch (ch) {
            case c'\"':
                result += "\\\""
                break
            case c'\\':
                result += "\\\\"
                break
            case c'\b':
                result += "\\b"
                break
            case c'\f':
                result += "\\f"
                break
            case c'\n':
                result += "\\n"
                break
            case c'\r':
                result += "\\r"
                break
            case c'\t':
                result += "\\t"
                break
            default:
                if (ch.toInt() < 32) {
                    let int_value = new Int(ch.toInt());
                    result += "\\u" + int_value.toString(16).padStart(4, "0")
                } else {
                    result += ch
                }
                break
        }
    }
    result += "\""
    return result
}


function getIndentation(space?: int | string, depth: int = 0): string {
    if (space == undefined) {
        return ""
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
    return " ".repeat(aligned).repeat(depth)
}


class TypeColor {
    readonly static VISITED : int = 1
    readonly static AT_STACK : int = 2

    readonly static STRING_CLASS = Class.of("")

    private readonly static BOXED_DOUBLE = Class.from<Double>()
    private readonly static BOXED_BOOLEAN = Class.from<Boolean>()
    private readonly static BOXED_INT = Class.from<Int>()
    private readonly static BOXED_LONG = Class.from<Long>()
    private readonly static BOXED_BYTE = Class.from<Byte>()
    private readonly static BOXED_SHORT = Class.from<Short>()
    private readonly static BOXED_FLOAT = Class.from<Float>()

    readonly static OBJECT_CLASS = Class.from<Object>()
    readonly static NULL_CLASS = Class.of(null)

    type: Class
    color: int
    constructor(type: Class, color: int) {
        this.type = type
        this.color = color
    }

    static isDoubleType(type: Class): boolean {
        return type === TypeColor.BOXED_DOUBLE || type === Class.PRIMITIVE_DOUBLE
    }

    static isBooleanType(type: Class): boolean {
        return type === TypeColor.BOXED_BOOLEAN || type === Class.PRIMITIVE_BOOLEAN
    }

    static isIntType(type: Class): boolean {
        return type === TypeColor.BOXED_INT || type === Class.PRIMITIVE_INT
    }

    static isLongType(type: Class): boolean {
        return type === TypeColor.BOXED_LONG || type === Class.PRIMITIVE_LONG
    }

    static isByteType(type: Class): boolean {
        return type === TypeColor.BOXED_BYTE || type === Class.PRIMITIVE_BYTE
    }

    static isShortType(type: Class): boolean {
        return type === TypeColor.BOXED_SHORT || type === Class.PRIMITIVE_SHORT
    }

    static isFloatType(type: Class): boolean {
        return type === TypeColor.BOXED_FLOAT || type === Class.PRIMITIVE_FLOAT
    }

    static isPrimitiveType(type: Class): boolean {
        return TypeColor.isDoubleType(type)
            || TypeColor.isBooleanType(type)
            || TypeColor.isIntType(type)
            || TypeColor.isLongType(type)
            || TypeColor.isByteType(type)
            || TypeColor.isShortType(type)
            || TypeColor.isFloatType(type)
    }
}

class Position {
    row: int = 1
    col: int = 1
    index: int = -1

    makeCopy(): Position {
        let c = new Position()
        c.row = this.row
        c.col = this.col
        c.index = this.index
        return c
    }

    restoreFromCopy(c: Position) {
        this.row = c.row
        this.col = c.col
        this.index = c.index
    }
}

export class JSONParser {
    private json: String
    private curPos: Position
    private curChar: char
    private escapeWhitespaces: boolean
    // TODO(ivan-tyulyandin): replace the hardcode with proper use of Array<T>/Map<T> in JSONValue inheritors

    constructor(json: String) {
        this.json = json
        this.escapeWhitespaces = true
        this.curPos = new Position()
    }

    private getCurPosDescr(): String {
        return "" + this.curPos.row + ":" + this.curPos.col
    }

    private getNextChar(): boolean {
        do {
            ++this.curPos.index
            if (this.curPos.index >= this.json.getLength()) {
                return false
            }
            this.curChar = this.json.charAt(this.curPos.index)
            ++this.curPos.col
            if (this.curChar == c'\n') {
                ++this.curPos.row
                this.curPos.col = 1
            }
        } while (this.escapeWhitespaces && (this.curChar == c' ' || this.curChar == c'\t' || this.curChar == c'\n'))
        return true
    }

    /**
     * Parses JSON into JSONValue
     *
     * @param json: String - a string with JSON
     *
     * @return JSONValue - JSON representation
     *
     * @throws SyntaxError if JSON is invalid
     *
     */
    static parse(json: String): JSONValue {
        let parser = new JSONParser(json)
        let res: JSONValue = new JSONValue()
        return parser.parse(res)
    }

    private parse(res: JSONValue): JSONValue {
        // Fetch next symbol and call related parse method
        // TODO(ivan-tyulyandin): replace with switch, does not work due to frontend bug with switch over static fields with equal names
        this.getNextChar()
        if (this.curChar == JSONObject.START_CHAR) {
            let obj = new JSONObject()
            this.parse(obj)
            res = obj
        } else if (this.curChar  == JSONArray.START_CHAR) {
            let arr = new JSONArray()
            this.parse(arr)
            res = arr
        } else if (this.curChar == JSONString.START_CHAR) {
            let str = new JSONString()
            this.parse(str)
            res = str
        } else if (this.curChar == JSONTrue.START_CHAR) {
            let tr = new JSONTrue()
            this.parse(tr)
            res = tr
        } else if (this.curChar == JSONFalse.START_CHAR) {
            let fls = new JSONFalse()
            this.parse(fls)
            res = fls
        } else if (this.curChar == JSONNull.START_CHAR) {
            let nl = new JSONNull()
            this.parse(nl)
            res = nl
        } else if (Char.isDecDigit(this.curChar) || this.curChar == c'-') {
            let n = new JSONNumber()
            this.parse(n)
            res = n
        } else {
            throw new SyntaxError("Unexpected char \"" + this.curChar + "\" at " + this.getCurPosDescr())
        }
        return res
    }

    private parseKeyValue(res: JSONObject): JSONObject {
        res.keys_.push(this.parse(new JSONValue()) as JSONString)
        this.getNextChar()
        if (this.curChar  != c':') {
            throw new SyntaxError("Expected : \",\" at " + this.getCurPosDescr() + " got \"" + this.curChar + "\"")
        }
        res.values.push(this.parse(new JSONValue()))
        return res
    }

    private parse(res: JSONObject): JSONObject {
        let posCopy = this.curPos.makeCopy()
        this.getNextChar()
        if (this.curChar == JSONObject.END_CHAR) {
            return res
        }
        this.curPos.restoreFromCopy(posCopy)

        res = this.parseKeyValue(res)
        // Parse JSONObject until }
        while (this.getNextChar()) {
            switch (this.curChar) {
                case c',':
                    res = this.parseKeyValue(res)
                    break
                // TODO(ivan-tyulyandin): replace by JSONObject.END_CHAR, frontend bug
                case c'}':
                    return res
                default:
                    throw new SyntaxError("Expected \",\" or \"" + JSONObject.END_CHAR.toString() + "\" at " + this.getCurPosDescr() + ", got \"" + this.curChar.toString() + "\"")
            }
        }
        return res
    }

    private parse(res: JSONArray): JSONArray {
        let posCopy = this.curPos.makeCopy()
        this.getNextChar()
        if (this.curChar == JSONArray.END_CHAR) {
            return res
        }
        this.curPos.restoreFromCopy(posCopy)

        res.values.push(this.parse(new JSONValue()))
        // Parse JSONArray until ]
        while (this.getNextChar()) {
            switch (this.curChar) {
                case c',':
                    res.values.push(this.parse(new JSONValue()))
                    break
                // TODO(ivan-tyulyandin): replace by JSONArray.END_CHAR, frontend bug
                case c']':
                    return res
                default:
                    throw new SyntaxError("Expected , or " + JSONArray.END_CHAR.toString() + " at " + this.getCurPosDescr() + ", got " + this.curChar.toString())
            }
        }
        return res
    }

    private parse(res: JSONNumber): JSONNumber {
        let number = new StringBuilder()
        let posCopy = this.curPos.makeCopy()
        do {
            number.append(this.curChar)
            posCopy = this.curPos.makeCopy()
        } while (this.getNextChar() && (Char.isDecDigit(this.curChar) ||
                                               this.curChar == c'.' ||
                                               this.curChar == c'e' ||
                                               this.curChar == c'-' ||
                                               this.curChar == c'+' ))
        res.value = Double.parseFloat(number.toString())
        try {
            res.bigintValue = new BigInt(number.toString())
        } catch (e) {
            res.bigintValue = new BigInt()
        }
        this.curPos.restoreFromCopy(posCopy)
        return res
    }

    private static readonly CAN_BE_ESCAPED_CHARS: FixedArray<char> = [c'"', c'\\', c'/', c'b', c'f', c'n', c'r', c't']
    private static readonly ESCAPED_CHARS: FixedArray<char> = [c'\"', c'\\', c'\/', c'\b', c'\f', c'\n', c'\r', c'\t']

    private parse(res: JSONString): JSONString {
        let resBuilder: StringBuilder = new StringBuilder()
        let hasMetEscape: boolean = false
        this.escapeWhitespaces = false
        while (this.getNextChar() && ((this.curChar != JSONString.END_CHAR) || (this.curChar == JSONString.END_CHAR && hasMetEscape))) {
            if (!hasMetEscape) {
                if (this.curChar != c'\\') {
                    resBuilder.append(this.curChar)
                }
            } else {
                let escapedCharIndex = indexOf(JSONParser.CAN_BE_ESCAPED_CHARS, this.curChar, 0)
                if (escapedCharIndex != -1) {
                    resBuilder.append(JSONParser.ESCAPED_CHARS[escapedCharIndex])
                } else {
                    throw new SyntaxError("Bad escape sequence \\" + this.curChar + " at " + this.getCurPosDescr())
                }
            }
            hasMetEscape = hasMetEscape ? false : this.curChar == c'\\'
        }
        if (this.curChar == JSONString.END_CHAR) {
            res.value = resBuilder.toString()
        } else {
            throw new SyntaxError("Unexpected end of String at " + this.getCurPosDescr())
        }
        this.escapeWhitespaces = true
        return res
    }

    private parse(res: JSONTrue): JSONTrue {
        let metTrue = this.getNextChar() && this.curChar == c'r'
                    && this.getNextChar() && this.curChar == c'u'
                    && this.getNextChar() && this.curChar == c'e'
        if (!metTrue) {
            throw new SyntaxError("Expected true at " + this.getCurPosDescr())
        }
        return res
    }

    private parse(res: JSONFalse): JSONFalse {
        let metFalse = this.getNextChar() && this.curChar == c'a'
                    && this.getNextChar() && this.curChar == c'l'
                    && this.getNextChar() && this.curChar == c's'
                    && this.getNextChar() && this.curChar == c'e'
        if (!metFalse) {
            throw new SyntaxError("Expected false at " + this.getCurPosDescr())
        }
        return res
    }

    private parse(res: JSONNull): JSONNull {
        let metNull = this.getNextChar() && this.curChar == c'u'
                    && this.getNextChar() && this.curChar == c'l'
                    && this.getNextChar() && this.curChar == c'l'
        if (!metNull) {
            throw new SyntaxError("Expected null at " + this.getCurPosDescr())
        }
        return res
    }
}

class JSONValueParser {
    private reviver: ((key: string, value: Any) => Any) | undefined
    private bigIntMode: int

    private static readonly SETTER_PREFIX = "<set>"

    constructor(reviver: ((key: string, value: Any) => Any) | undefined, bigIntMode: int) {
        this.reviver = reviver
        this.bigIntMode = bigIntMode
    }

    private createNumber(value: JSONNumber, type: Class): Any {
        if (type === BigIntType && (this.bigIntMode === 2 || (!Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return new BigInt(value.bigintValue)
        } else if (TypeColor.isDoubleType(type) && (this.bigIntMode === 0 || (Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return new Double(value.value)
        } else if (TypeColor.isIntType(type) && (this.bigIntMode === 0 || (Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return new Double(value.value).toInt()
        } else if (TypeColor.isLongType(type) && (this.bigIntMode === 0 || this.bigIntMode === 1)) {
            return new Long().createFromJSONValue(value)
        } else if (TypeColor.isShortType(type) && (this.bigIntMode === 0 || this.bigIntMode === 1)) {
            return new Short().createFromJSONValue(value)
        } else if (TypeColor.isFloatType(type) && (this.bigIntMode === 0 || this.bigIntMode === 1)) {
            return new Float().createFromJSONValue(value)
        } else if (TypeColor.isByteType(type) && (this.bigIntMode === 0 || this.bigIntMode === 1)) {
            return new Byte().createFromJSONValue(value)
        } else {
            throw new Error(type.getName() + " is expected, but get " + Class.of(value.value).getName() + " with bigIntMode=" + this.bigIntMode)
        }
    }

    parse(value: JSONValue, type: Class): Any {
        if (value instanceof JSONObject && type.isUnion()) {
            // NOTE(lyupaanastasia) parse union #27136
            throw new Error("UnionType is not supported yet")
        } else if (value instanceof JSONArray && type.isFixedArray()) {
            return this.parseArray(value as JSONArray, type)
        } else if (
                value instanceof JSONNumber && (
                    TypeColor.isDoubleType(type) ||
                    type === BigIntType ||
                    TypeColor.isIntType(type) ||
                    TypeColor.isLongType(type) ||
                    TypeColor.isByteType(type) ||
                    TypeColor.isShortType(type) ||
                    TypeColor.isFloatType(type))) {
            return this.createNumber(value as JSONNumber, type)
        } else if (value instanceof JSONString && type === TypeColor.STRING_CLASS) {
            return (value as JSONString).value
        } else if (value instanceof JSONTrue && TypeColor.isBooleanType(type)) {
            return new Boolean(true)
        } else if (value instanceof JSONFalse && TypeColor.isBooleanType(type)) {
            return new Boolean(false)
        } else if (value instanceof JSONNull && type === TypeColor.NULL_CLASS) {
            return null
        } else if (value instanceof JSONObject && type.isSubtypeOf(TypeColor.OBJECT_CLASS)) {
            return this.parseObject(value as JSONObject, type)
        } else {
            throw new Error(type.getName() + " is expected, but get " + value)
        }
    }

    private parseObject(jsonObj: JSONObject, classType: Class): Object {
        let setClassFieldFn = (obj: object, fieldName: string, fieldVal: Any) => {
            const field = searchInstanceFieldInHierarchy(Class.of(obj), fieldName)!
            field.setValue(obj, fieldVal)
        }

        if (this.reviver != undefined) {
            setClassFieldFn = (obj: object, fieldName: string, fieldVal: Any): void => {
                const revivedVal = this.reviver!(fieldName, fieldVal)

                const field = searchInstanceFieldInHierarchy(Class.of(obj), fieldName)!
                field.setValue(obj, revivedVal)
            }
        }

        const obj = JSONValueParser.classGetDefaultCtor(classType).createInstance() as object
        let hasJsonParseIgnore: boolean = false
        let hasJsonStringifyIgnore: boolean = false
        const jsonObjFields = jsonObj.getFields()

        for (const classField of getClassInstanceFieldsAll(classType)) {
            hasJsonParseIgnore = JSONAPI.getJSONParseIgnore(classField);
            if (hasJsonParseIgnore) {
                continue
            }

            let jsonFieldVal = jsonObjFields.get(classField.getName())
            if (jsonFieldVal === undefined) {
                hasJsonStringifyIgnore = JSONAPI.getJSONStringifyIgnore(classField);
                if (hasJsonStringifyIgnore) {
                    continue
                }
                let fieldRename = JSONAPI.getJSONRename(classField);
                if (fieldRename == undefined) {
                    throw new Error("Cannot find " + classField.getName() + " in keys of " + classType.getName())
                }
                jsonFieldVal = jsonObjFields.get(fieldRename)
                if (jsonFieldVal === undefined) {
                    throw new Error("Cannot find rename " + fieldRename + " in keys of " + classType.getName())
                }
            }

            const classFieldVal = this.parse(jsonFieldVal, classField.getType())
            setClassFieldFn(obj, classField.getName(), classFieldVal)
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

    private static classGetDefaultCtor(cls: Class): reflect.Constructor {
        for (const ctor of cls.getConstructors()) {
            if (ctor.getParametersNum() == 0) {
                return ctor
            }
        }
        throw new Error(`class ${cls.getName()} doesn't have default ctor`)
    }

    private parseArray(jVal: JSONArray, aType: Class): Object | null {
        let len = jVal.values.length
        // NOTE(cheezzario) Class API lacks array creation facility
        let arr = ArrayType.getInstance(aType).make(len.toInt()) as FixedArray<Any>

        let setArrayElementFn = (a: FixedArray<Any>, i: int, v: Any) => { a[i] = v }

        if (this.reviver !== undefined) {
            setArrayElementFn = (array: FixedArray<Any>, index: int, elemVal: Any): void => {
                const revivedVal = this.reviver!(`${index}`, elemVal)
                array[index] = revivedVal !== undefined ? revivedVal : null
            }
        }

        for (let i = 0; i < len; i++) {
            let elem = this.parse(jVal.values.at(i) as JSONValue, aType.getFixedArrayComponentType()!)
            setArrayElementFn(arr, i, elem)
        }
        return arr
    }
}

export enum JsonTokenKind
{
    CurlyOpen    = 0,
    CurlyClose   = 1,
    SquareOpen   = 2,
    SquareClose  = 3,
    Colon        = 4,
    Comma        = 5,
    String       = 6,
    Number       = 7,
    True         = 8,
    False        = 9,
    Null         = 10
}

export class JsonTokenSpan
{
    start: int
    end: int
}

export class JsonToken
{
    kind: JsonTokenKind
    span: JsonTokenSpan
    value: string
    constructor(kind: JsonTokenKind, span: JsonTokenSpan, value: string) {
        this.kind = kind
        this.span = span
        this.value = value
    }
}

// Grammar:
// =============================================================
// start  ::= value
// value  ::= object | array | double | string | true | false | null
// object ::= "{" [pair ("," pair)*] "}"
// array  ::= "[" [value ("," value)*] "]"
// pair   ::= string ":" value
// string ::= ESCAPED_STRING
// number ::= SIGNED_NUMBER
// true   ::= "true"
// false  ::= "false"
// null   ::= "null"

/**
 * Error thrown when parsing JSON fails.
 */
export class JsonParseError extends Error {
    public constructor(msg: string, start_offset?: int, end_offset?: int) {
        super(`${msg} at ${start_offset}..${end_offset}`)
    }
}

class JsonElementParser
{
    source: string
    tokens: Array<JsonToken>
    position: int
    reviver?: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement

    constructor(source: string) {
        this.source = source
        this.tokens = JsonLexer.fromString(source).tokenize()
        this.position = 0
    }

    constructor(source: string, reviver: (key: string, value: jsonx.JsonElement) => jsonx.JsonElement) {
        this.source = source
        this.tokens = JsonLexer.fromString(source).tokenize()
        this.position = 0
        this.reviver = reviver
    }

    public lookahead(offset: int): JsonToken | undefined
    {
        if (this.position + offset >= this.tokens.length) {
            return undefined
        }
        return this.tokens[this.position + offset]
    }

    public expect(kind: JsonTokenKind): JsonToken
    {
        let token = this.lookahead(0)
        if (token == undefined || token.kind != kind) {
            let start = token?.span.start ?? 0;
            let end = token?.span.end ?? 0;
            throw new JsonParseError(`Expected ${kind} but got ${token?.kind}`, start, end)
        }
        this.position++
        return token
    }

    public parse(): jsonx.JsonElement
    {
        let result = this.parseValue()
        if (this.reviver != undefined) {
            return this.reviver!("", result)
        }
        return result
    }

    private parseValue(): jsonx.JsonElement
    {
        let maybe_token = this.lookahead(0)
        if (maybe_token == undefined) {
            throw new JsonParseError("Unexpected end of input", 0, 0)
        }
        if (maybe_token.kind == JsonTokenKind.String) {
            return this.parseString()
        }
        if (maybe_token.kind == JsonTokenKind.Number) {
            return this.parseNumber()
        }
        if (maybe_token.kind == JsonTokenKind.True) {
            return this.parseBool()
        }
        if (maybe_token.kind == JsonTokenKind.False) {
            return this.parseBool()
        }
        if (maybe_token.kind == JsonTokenKind.Null) {
            return this.parseNull()
        }
        if (maybe_token.kind == JsonTokenKind.CurlyOpen) {
            return this.parseObject()
        }
        if (maybe_token.kind == JsonTokenKind.SquareOpen) {
            return this.parseArray()
        }
        throw new JsonParseError("Expected string, number, true, false, null, object, or array", maybe_token.span.start, maybe_token.span.end)
    }

    private parseArray(): jsonx.JsonElement
    {
        this.expect(JsonTokenKind.SquareOpen)
        let array = new Array<jsonx.JsonElement>()
        let index = 0
        while (this.lookahead(0)?.kind != JsonTokenKind.SquareClose) {
            let value = this.parseValue()
            if (this.reviver != undefined) {
                value = this.reviver!(`${index}`, value)
            }
            array.push(value)
            index++
            if (this.lookahead(0)?.kind == JsonTokenKind.Comma) {
                this.expect(JsonTokenKind.Comma)
            }
        }
        this.expect(JsonTokenKind.SquareClose)
        return jsonx.JsonElement.createArray(array)
    }

    private parseObject(): jsonx.JsonElement
    {
        this.expect(JsonTokenKind.CurlyOpen)
        let object = new Map<string, jsonx.JsonElement>()
        while (this.lookahead(0)?.kind != JsonTokenKind.CurlyClose) {
            let pair = this.parsePair()
            let key = pair[0]
            let value = pair[1]
            if (this.reviver != undefined) {
                value = this.reviver!(key, value)
            }
            object.set(key, value)
            if (this.lookahead(0)?.kind == JsonTokenKind.Comma) {
                this.expect(JsonTokenKind.Comma)
            }
        }
        this.expect(JsonTokenKind.CurlyClose)
        return jsonx.JsonElement.createObject(object)
    }

    private parsePair(): [string, jsonx.JsonElement]
    {
        let key = this.parseString().asString()
        this.expect(JsonTokenKind.Colon)
        let value = this.parseValue()
        return [key, value] as [string, jsonx.JsonElement]
    }

    private parseString(): jsonx.JsonElement
    {
        let token = this.expect(JsonTokenKind.String)
        return jsonx.JsonElement.createString(token.value)
    }

    private parseNumber(): jsonx.JsonElement
    {
        // can be extended with bigints later
        let token = this.expect(JsonTokenKind.Number)
        if (token.value.includes(".")) {
            return jsonx.JsonElement.createDouble(Double.parseFloat(token.value))
        }
        return jsonx.JsonElement.createInteger(Double.parseInt(token.value).toInt())
    }

    private parseBool(): jsonx.JsonElement
    {
        let maybe_token = this.lookahead(0)
        if (maybe_token == undefined) {
            throw new JsonParseError("Unexpected end of input", 0, 0)
        }
        if (maybe_token.kind == JsonTokenKind.True) {
            this.expect(JsonTokenKind.True)
            return jsonx.JsonElement.createBoolean(true)
        }
        if (maybe_token.kind == JsonTokenKind.False) {
            this.expect(JsonTokenKind.False)
            return jsonx.JsonElement.createBoolean(false)
        }
        throw new JsonParseError("Expected true or false", maybe_token.span.start, maybe_token.span.end)
    }

    private parseNull(): jsonx.JsonElement
    {
        this.expect(JsonTokenKind.Null)
        return jsonx.JsonElement.createNull()
    }
}

export class JsonLexer
{
    chars: char[]
    index: int
    constructor(chars: char[]) {
        this.chars = chars
        this.index = 0
    }
    public static fromString(json: string): JsonLexer {
        return new JsonLexer(json.getChars())
    }

    /**
     * Tokenizes the JSON input and returns an array of tokens.
     * @returns Array of JsonToken objects representing the tokenized input
     * @throws JsonError if invalid JSON syntax is encountered
     */
    public tokenize(): JsonToken[] {
        let tokens = new Array<JsonToken>()

        while (!this.isAtEnd()) {
            this.skipWhitespace()
            if (this.isAtEnd()) break

            let token = this.nextToken()
            if (token != null) {
                tokens.push(token)
            }
        }

        return tokens
    }

    /**
     * Reads the next token from the input
     * @returns JsonToken or null if no valid token found
     * @throws JsonError if invalid syntax is encountered
     */
    private nextToken(): JsonToken | null {
        let start = this.index
        let ch = this.advance()

        switch (ch) {
            case c'{':
                return this.makeToken(JsonTokenKind.CurlyOpen, start, "{")
            case c'}':
                return this.makeToken(JsonTokenKind.CurlyClose, start, "}")
            case c'[':
                return this.makeToken(JsonTokenKind.SquareOpen, start, "[")
            case c']':
                return this.makeToken(JsonTokenKind.SquareClose, start, "]")
            case c':':
                return this.makeToken(JsonTokenKind.Colon, start, ":")
            case c',':
                return this.makeToken(JsonTokenKind.Comma, start, ",")
            case c'"':
                return this.readString(start)
            case c't':
                return this.readKeyword(start, "true", JsonTokenKind.True)
            case c'f':
                return this.readKeyword(start, "false", JsonTokenKind.False)
            case c'n':
                return this.readKeyword(start, "null", JsonTokenKind.Null)
            default:
                if (this.isDigit(ch) || ch == c'-') {
                    this.index-- // Back up to re-read the character
                    return this.readNumber(start)
                }
                throw new JsonParseError(`Unexpected character '${ch}' at position ${start}`, start, start)
        }
    }

    /**
     * Reads a JSON string token, handling escape sequences
     * @param start Starting position of the string
     * @returns JsonToken representing the string
     * @throws JsonError if string is malformed
     */
    private readString(start: int): JsonToken {
        let value = ""

        while (!this.isAtEnd() && this.peek() != c'"') {
            let ch = this.advance()
            if (ch == c'\\') {
                if (this.isAtEnd()) {
                    throw new JsonParseError("Unterminated string escape at end of input", start, this.index)
                }
                let escaped = this.advance()
                switch (escaped) {
                    case c'"':
                        value += "\""
                        break
                    case c'\\':
                        value += "\\"
                        break
                    case c'/':
                        value += "/"
                        break
                    case c'b':
                        value += "\b"
                        break
                    case c'f':
                        value += "\f"
                        break
                    case c'n':
                        value += "\n"
                        break
                    case c'r':
                        value += "\r"
                        break
                    case c't':
                        value += "\t"
                        break
                    case c'u':
                        value += this.readUnicodeEscape()
                        break
                    default:
                        throw new JsonParseError(`Invalid escape sequence '\\${escaped}' in string`, start, this.index)
                }
            } else {
                value += ch
            }
        }

        if (this.isAtEnd()) {
            throw new JsonParseError("Unterminated string", start, this.index)
        }

        this.advance() // Consume closing quote
        return this.makeToken(JsonTokenKind.String, start, value)
    }

    /**
     * Reads a Unicode escape sequence (\uXXXX)
     * @returns The Unicode character as a string
     * @throws JsonError if escape sequence is malformed
     */
    private readUnicodeEscape(): string {
        let codeStr = ""
        for (let i = 0; i < 4; i++) {
            if (this.isAtEnd()) {
                throw new JsonParseError("Incomplete Unicode escape sequence")
            }
            let ch = this.advance()
            if (!this.isHexDigit(ch)) {
                throw new JsonParseError(`Invalid hexadecimal digit '${ch}' in Unicode escape`, this.index)
            }
            codeStr += ch
        }

        let codePoint = Double.parseInt(codeStr, 16).toInt()
        return String.fromCharCode(codePoint.toDouble())
    }

    /**
     * Reads a JSON number token
     * @param start Starting position of the number
     * @returns JsonToken representing the number
     * @throws JsonError if number format is invalid
     */
    private readNumber(start: int): JsonToken {
        let value = ""

        // Handle optional minus sign
        if (this.peek() == c'-') {
            value += this.advance()
        }

        // Read integer part
        if (this.peek() == c'0') {
            value += this.advance()
        } else if (this.isDigit(this.peek())) {
            while (!this.isAtEnd() && this.isDigit(this.peek())) {
                value += this.advance()
            }
        } else {
            throw new JsonParseError("Invalid number format", start, this.index)
        }

        // Read decimal part if present
        if (!this.isAtEnd() && this.peek() == c'.') {
            value += this.advance()
            if (this.isAtEnd() || !this.isDigit(this.peek())) {
                throw new JsonParseError("Invalid number format: decimal point must be followed by digits", start, this.index)
            }
            while (!this.isAtEnd() && this.isDigit(this.peek())) {
                value += this.advance()
            }
        }

        // Read exponent part if present
        if (!this.isAtEnd() && (this.peek() == c'e' || this.peek() == c'E')) {
            value += this.advance()
            if (!this.isAtEnd() && (this.peek() == c'+' || this.peek() == c'-')) {
                value += this.advance()
            }
            if (this.isAtEnd() || !this.isDigit(this.peek())) {
                throw new JsonParseError("Invalid number format: exponent must contain digits", start, this.index)
            }
            while (!this.isAtEnd() && this.isDigit(this.peek())) {
                value += this.advance()
            }
        }

        return this.makeToken(JsonTokenKind.Number, start, value)
    }

    /**
     * Reads a JSON keyword (true, false, null)
     * @param start Starting position of the keyword
     * @param expected The expected keyword string
     * @param kind The token kind for this keyword
     * @returns JsonToken representing the keyword
     * @throws JsonError if keyword doesn't match
     */
    private readKeyword(start: int, expected: string, kind: JsonTokenKind): JsonToken {
        this.index-- // Back up to re-read first character

        for (let i = 0; i < expected.length; i++) {
            if (this.isAtEnd() || this.advance() != expected.charAt(i)) {
                throw new JsonParseError(`Invalid keyword at position ${start}`, start, this.index)
            }
        }

        return this.makeToken(kind, start, expected)
    }

    /**
     * Creates a JsonToken with the given parameters
     * @param kind The token kind
     * @param start Starting position
     * @param value String value of the token
     * @returns New JsonToken instance
     */
    private makeToken(kind: JsonTokenKind, start: int, value: string): JsonToken {
        let span = new JsonTokenSpan()
        span.start = start
        span.end = this.index
        return new JsonToken(kind, span, value)
    }

    /**
     * Skips whitespace characters
     */
    private skipWhitespace(): void {
        while (!this.isAtEnd()) {
            let ch = this.peek()
            if (ch == c' ' || ch == c'\t' || ch == c'\n' || ch == c'\r') {
                this.advance()
            } else {
                break
            }
        }
    }

    /**
     * Advances to the next character and returns it
     * @returns The current character before advancing
     */
    private advance(): char {
        if (this.isAtEnd()) return c'\0'
        return this.chars[this.index++]
    }

    /**
     * Returns the current character without advancing
     * @returns The current character
     */
    private peek(): char {
        if (this.isAtEnd()) return c'\0'
        return this.chars[this.index]
    }

    /**
     * Checks if we've reached the end of input
     * @returns true if at end of input
     */
    private isAtEnd(): boolean {
        return this.index >= this.chars.length
    }

    /**
     * Checks if a character is a decimal digit
     * @param ch Character to check
     * @returns true if character is a digit
     */
    private isDigit(ch: char): boolean {
        return (ch.compareTo(c'0') >= 0) && (ch.compareTo(c'9') <= 0)
    }

    /**
     * Checks if a character is a hexadecimal digit
     * @param ch Character to check
     * @returns true if character is a hex digit
     */
    private isHexDigit(ch: char): boolean {
        return ((ch.compareTo(c'0') >= 0) && (ch.compareTo(c'9') <= 0)) ||
                ((ch.compareTo(c'A') >= 0) && (ch.compareTo(c'F') <= 0)) ||
                ((ch.compareTo(c'a') >= 0) && (ch.compareTo(c'f') <= 0))
    }
}

export class JSONValue extends Object {}

export class JSONObject extends JSONValue {
    keys_: Array<JSONString> = new Array<JSONString>()
    values: Array<JSONValue> = new Array<JSONValue>()
    readonly static START_CHAR = c'{'
    readonly static END_CHAR = c'}'
    readonly static SEPARATOR = c':'
    readonly static DELIMETER = c','
    readonly static EMPTY = "{}"

    getFields(): Map<String, JSONValue> {
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
            res.append("" + this.keys_.at(i) + JSONObject.SEPARATOR + this.values.at(i) + JSONObject.DELIMETER)
        }
        if (this.keys_.length > 0) {
            res.append("" + this.keys_.at(this.keys_.length - 1) + JSONObject.SEPARATOR + this.values.at(this.keys_.length - 1))
        }
        res.append(JSONObject.END_CHAR)
        return res.toString()
    }
}

export class JSONArray extends JSONValue {
    values: Array<JSONValue> = new Array<JSONValue>()
    readonly static START_CHAR = c'['
    readonly static END_CHAR = c']'
    readonly static SEPARATOR = c','

    public override toString(): String {
        let res = new StringBuilder([JSONArray.START_CHAR] as FixedArray<char>)
        for (let i = 0; i < this.values.length - 1; ++i) {
            res.append("" + this.values.at(i) + JSONArray.SEPARATOR)
        }
        if (this.values.length > 0) {
            res.append("" + this.values.at(this.values.length - 1))
        }
        res.append(JSONArray.END_CHAR)
        return res.toString()
    }
}

export class JSONNumber extends JSONValue {
    value: double
    bigintValue: bigint = new BigInt()

    public override toString(): String {
        return (new Double(this.value)).toString()
    }
}

export class JSONString extends JSONValue {
    value: String = ""
    readonly static START_CHAR = c'"'
    readonly static END_CHAR = c'"'

    public override toString(): String {
        return this.value
    }
}

export class JSONTrue extends JSONValue {
    readonly static value = "true"
    readonly static START_CHAR = c't'

    public override toString(): String {
        return JSONTrue.value
    }
}

export class JSONFalse extends JSONValue {
    readonly static value = "false"
    readonly static START_CHAR = c'f'

    public override toString(): String {
        return JSONFalse.value
    }
}

export class JSONNull extends JSONValue {
    readonly static value = "null"
    readonly static START_CHAR = c'n'

    public override toString(): String {
        return JSONNull.value
    }
}
