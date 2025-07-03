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

package escompat

import { jsonx } from "std/core"

// initialized in _initializerBlock_.ets
const BigIntType: Type;

export interface JsonReplacer {
    jsonReplacer(): Record<String, NullishType>
}

// NOTE (chengzhihao): $Partial is not generated for exported interfaces.issue #26111
// now use this class to throw the error.
export class JsonReplacerTemp implements JsonReplacer {
    jsonReplacer(): Record<String, NullishType> {
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

class JSONAPI {
    /*
        JSON intrinsics
    */
    native static getJSONStringifyIgnoreByIdx(cls: Class, idx: long): boolean

    native static getJSONStringifyIgnoreByName(cls: Class, name: string): boolean

    native static getJSONParseIgnoreFromAnnotation(cls: Class, idx: long): boolean

    native static getJSONRenameByIdx(cls: Class, idx: long): string | undefined

    native static getJSONRenameByName(cls: Class, name: string): string | undefined
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
    public static stringify(d: double): String {
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
    public static stringify(d: bigint): String {
        return d.toString()
    }

    /**
     * Converts char to JSON format
     *
     * @param d: char - char to be converted to a JSON as a String
     *
     * @returns String - JSON representation of char
     */
    public static stringify(d: char): String {
        return "\"" + StringBuilder.toString(d) + "\""
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
    public static stringify(d: String): String {
        let sb = new StringBuilder([c'\"'] as FixedArray<char>)
        let len = d.getLength()

        let prevChar: char = 0
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
                } else if (currChar <= c'\x1f' && currChar >= c'\x00') {
                    if (currChar <= c'\x0f') {
                        sb.append('\\u000')
                    } else {
                        sb.append('\\u00')
                    }
                    sb.append(Number.toString(currChar, 16))
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
        buffer.append("\\u").append(new Int(chr).toString(16))
        return buffer
    }

    /**
     * Converts an object to a JavaScript Object Notation (JSON) string.
     *
     * @param obj: Object An object to be converted.
     *
     * @returns String - JSON representation of Object
     */
    public static stringify(obj: NullishType): String {
        if (obj === undefined) {
            return "undefined"
        }
        let output = JSON.stringifyFast(obj)
        if (output !== undefined) {
            return output
        }
        return new JSONWriter().write(obj)
    }

    private static native stringifyFast(obj: NullishType): String;

    public static stringify(obj: JsonReplacer): String {
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
    public static stringify(
                obj: NullishType,
                replacer: ((key: string, value: NullishType) => NullishType) | undefined | null,
                space?: string | int): string {
        const replacerOpt: ((key: string, value: NullishType) => NullishType) | undefined = (replacer === null) ? undefined : replacer

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
    public static stringify(obj: NullishType, replacer: FixedArray<double | string>, space?: int | string): string {
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

    public static stringify(obj: NullishType, replacer: Array<double | string>, space?: int | string): string {
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

    public static native stringify(d: JSValue): String;

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
    public static stringify(d: FixedArray<short>): String {
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
    public static stringify(d: FixedArray<int>): String {
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
    public static stringify(d: FixedArray<long>): String {
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
    public static stringify(d: FixedArray<float>): String {
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
    public static stringify(d: FixedArray<double>): String {
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
    public static stringify(d: FixedArray<char>): String {
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
    public static stringify(d: FixedArray<boolean>): String {
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

    public static stringify(d: Array<Number>): String {
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

    public static stringify(d: ArrayLike<Number>): String {
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
    private static checkType(typ: Type, used: Array<TypeColor>): boolean {
        let ind = used.findIndex((tc: TypeColor) => tc.typ.equals(typ))
        if (ind != -1) {
            return (used.at(ind) as TypeColor).color == TypeColor.VISITED
        }
        used.push(new TypeColor(typ, TypeColor.AT_STACK))
        let ok = true
        if (typ.equals(BigIntType)) {
            ok = true
        } else if (typ instanceof ClassType) {
            let ct = typ as ClassType
            ok &= ct.hasEmptyConstructor()
            for (let f = 0; f < ct.getFieldsNum(); f++) {
                ok &= JSON.checkType(ct.getField(f).getType(), used)
            }
        } else if (typ instanceof ArrayType) {
            let et = (typ as ArrayType).getElementType()
            ok = JSON.checkType(et, used)
        } else if (typ instanceof UnionType) {
            // NOTE(lyupaanastasia) parse union #27136
            throw new Error("UnionType is not supported yet")
        } else if (typ instanceof StringType
                    || typ instanceof DoubleType
                    || typ instanceof BooleanType
                    || typ instanceof NullType
                    || typ instanceof IntType
                    || typ instanceof LongType
                    || typ instanceof ByteType
                    || typ instanceof ShortType
                    || typ instanceof FloatType) {
            ok = true
        } else {
            ok = false
        }
        (used.at(ind) as TypeColor).color = TypeColor.VISITED
        return ok
    }

    // TODO(kirill-mitkin): For testing purpose only, shpuld be replaced by error checking
    public static isCorrectForParsing(typ: Type): boolean {
        return JSON.checkType(typ, new Array<TypeColor>())
    }

    public static parse<T>(text: String, type: Type): Nullish<T> {
        return JSON.parse<T>(text, undefined, type)
    }

    public static parse<T>(text: string, reviver: ((key: string, value: NullishType) => NullishType) | undefined, type: Type, bigIntMode?: int): Nullish<T> {
        if (!JSON.checkType(type, new Array<TypeColor>())) {
            throw new Error("Incorrect type: " + type.toString())
        }

        if (bigIntMode == undefined) {
            bigIntMode = 0
        } else if (bigIntMode > 2 || bigIntMode < 0) {
            throw new Error("bigIntMode value should be from 0 to 2, current value is" + bigIntMode)
        }

        let jsonValue = JSONParser.parse(text)
        const parsingResult = new JSONValueParser(reviver, bigIntMode as int).parse(jsonValue, type) as T

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

export interface JsonElementDeserializable {
    /**
        * Serializes an object to a JsonElement.
        * @returns {JsonElement} The JSON representation of the object
        */
    toJSON(): jsonx.JsonElement
}

class JSONWriter {
    // NOTE(cheezzario) replace with Type.for<JSValue>() when it will be implemented
    private static readonly STD_CORE_INTEROP_JSVALUE_TYPE = (Type.of([] as FixedArray<JSValue>) as ArrayType).getElementType()
    private static readonly ESCOMPAT_ARRAY_TYPE = Type.of(new Array<Object>(0))
    private static readonly ESCOMPAT_SET_TYPE = Type.of(new Set<Object>())
    private static readonly ESCOMPAT_MAP_TYPE = Type.of(new Map<Object, Object>())
    private static readonly STD_CORE_PROMISE_TYPE = Type.of(new Promise<Object>((res) => {}))

    private readonly replacer: ((key: String, value: NullishType) => NullishType) | undefined
    private readonly fieldsFilter: FixedArray<string>
    private readonly space: String | undefined

    private readonly useReplacer: boolean
    private readonly useFieldsFilter: boolean
    private readonly useIndent: boolean

    private indentLevel: int = 0

    private readonly path = new Set<Object>()
    private buffer = new StringBuilder()

    constructor(replacer?: (key: String, value: NullishType) => NullishType, space?: String) {
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

    write(obj: NullishType): String {
        if (this.useReplacer) {
            this.writeObject(this.replacer!("", obj))
        } else {
            this.writeObject(obj)
        }

        return this.buffer.toString()
    }

    private writeObject(obj: NullishType): void {
        obj = __narrowAny(obj)
        if (obj === null) {
            this.buffer.append("null")
        } else if (obj === undefined) {
            this.buffer.append("undefined")
        } else if (obj instanceof String) {
            this.buffer.append(JSON.stringify(obj as String))
        } else if (obj instanceof RegExpExecArray) {
            this.buffer.append(JSON.stringify((obj as RegExpExecArray).result))
        } else if(obj instanceof Tuple) {
            this.writeTupleValue(obj);
        } else if (obj instanceof Record) {
            this.writeESCompatRecord(obj)
        } else if (obj instanceof JsonReplacer) {
            let temp: Record<String, NullishType> = obj.jsonReplacer()
            this.writeESCompatRecord(temp)
        } else if (obj instanceof ESValue) {
            this.writeInteropESValue(obj)
        } else if (obj instanceof JsonElementDeserializable) {
            this.buffer.append(JSON.stringifyJsonElement((obj as JsonElementDeserializable).toJSON()))
        } else if (this.writeValueTypeWrapper(obj)) {
            // nothing to do - write completed successfully
        } else {
            const objType = Type.of(obj)
            if (objType.getName() == "escompat.Date") {
                this.buffer.append("\"").append((obj as Date).toISOString()).append("\"")
            // } else if (objType.getName() == "@ohos.buffer.buffer.Buffer") {
            //     this.buffer.append(JSON.stringifyJsonElement((obj as JsonElementDeserializable).toJSON()))
            } else if (objType.getName() == JSONWriter.STD_CORE_INTEROP_JSVALUE_TYPE.getName()) {
                this.buffer.append(JSON.stringify(obj as JSValue))
            } else if (objType instanceof ArrayType) {
                this.writeArrayValue(reflect.Value.of(obj) as ArrayValue, objType as ArrayType)
            } else if (objType.getName() == JSONWriter.ESCOMPAT_ARRAY_TYPE.getName()) {
                this.writeEscompatArray(obj as Array<Object>)
            } else if (objType.getName() == JSONWriter.ESCOMPAT_SET_TYPE.getName() ||
                       objType.getName() == JSONWriter.STD_CORE_PROMISE_TYPE.getName() ||
                       objType.getName() == JSONWriter.ESCOMPAT_MAP_TYPE.getName()) {
                this.buffer.append(JSONObject.EMPTY)
            } else if (objType instanceof ClassType) {
                this.writeClassValue(obj, objType as ClassType)
            } else if (objType instanceof LambdaType) {
                this.buffer.append("undefined")
            } else {
                throw new AssertionError("Unsupported object type: " + objType)
            }
        }
    }

    private writeTupleValue(obj: Object) {
        const currentBuffer = this.buffer
        this.buffer = new StringBuilder("[")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        const objType = Type.of(obj) as ClassType

        const writableFields = this.getWritableFields(objType, reflect.Value.of(obj) as ClassValue)

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

    private writeTupleElements(tupleFields: Array<[Field, reflect.Value]>): boolean {
        const FIELD_VALUE = 1

        let fieldDumped = false
        for (let fieldIdx = 0; fieldIdx < tupleFields.length; fieldIdx++) {
            const objToDump = tupleFields[fieldIdx][FIELD_VALUE].getData();
            if (objToDump !== undefined) {
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

                this.writeObject(objToDump)
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
        } catch (e: TypeError) {
            throw new TypeError("cyclic object value")
        }
    }

    private writeClassValue(obj: Object, objType: ClassType): void {
        const currentBuffer = this.buffer
        // we need this local buffer to discard empty object prolog: '{','\n',<space> and replace it with '{}'
        this.buffer = new StringBuilder("{")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        const writableFields = this.getWritableFields(objType, reflect.Value.of(obj) as ClassValue)

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

    private writeField(fieldName: string, fieldVal: NullishType, fieldDumped: boolean): boolean {
        if (fieldVal === undefined) {
            return fieldDumped
        }

        const fieldValType = Type.of(fieldVal)
        if (fieldValType instanceof LambdaType) {
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

    private writeClassFields(writableFields: Array<[Field, reflect.Value]>): boolean {
        const FIELD_TYPE = 0
        const FIELD_VALUE = 1

        let fieldDumped = false
        for (let fieldIdx = 0; fieldIdx < writableFields.length; fieldIdx++) {
            const fieldTypeValuePair = writableFields[fieldIdx]

            const objField = fieldTypeValuePair[0] as Field
            const objFieldValue = fieldTypeValuePair[1] as reflect.Value

            const objToDump = this.useReplacer ? this.replacer!(objField.getName(), objFieldValue.getData()) : objFieldValue.getData()
            fieldDumped = this.writeField(objField.getName(), objToDump, fieldDumped)
        }

        return fieldDumped
    }

    private checkSameRename(field: Field, fields: Array<[Field, reflect.Value]>): boolean {
        return fields.some((value: [Field, reflect.Value], index: number, array:Array<[Field, reflect.Value]>)
            => {return value[0].getName() == field.getName()});
    }

    private findKeyIndex(field: Field, fields: Array<[Field, reflect.Value]>): number {
        return fields.findIndex((value: [Field, reflect.Value], index: number, array:Array<[Field, reflect.Value]>)
            => {return value[0].getName() == field.getName()});
    }

    private getWritableFields(classType: ClassType, classValue: ClassValue): Array<[Field, reflect.Value]> {
        const writableFields = new Array<[Field, reflect.Value]>()
        let hasJsonIgnore: boolean = false
        let fieldRename : string | undefined = undefined

        if (this.useFieldsFilter) {
            for (let fieldIdx = 0; fieldIdx < this.fieldsFilter.length; fieldIdx++) {
                const fieldName = this.fieldsFilter[fieldIdx]
                if (!classType.hasField(fieldName)) {
                    continue
                }

                const field = classType.getFieldByName(fieldName)
                hasJsonIgnore = JSONAPI.getJSONStringifyIgnoreByName(classType.cls, fieldName);
                if (hasJsonIgnore || field.isStatic()) {
                    continue
                }
                fieldRename = JSONAPI.getJSONRenameByName(classType.cls, fieldName);
                if (fieldRename != undefined) {
                    if (classType.hasField(fieldRename)) {
                        throw new Error("Cannot rename " + fieldRename + " in keys of " + classType.getName())
                    } else {
                        field.name = fieldRename
                    }
                    if (this.checkSameRename(field, writableFields)) {
                        throw new Error("Cannot double rename " + field.getName() + " in keys of " + classType.getName())
                    }
                }

                const fieldValue = classValue.getFieldByName(fieldName)

                const fieldTypeValuePair: [Field, reflect.Value] = [field, fieldValue]
                writableFields.push(fieldTypeValuePair)
            }
        } else {
            const fieldsCount = classValue.getFieldsNum()
            for (let fieldIdx = 0; fieldIdx < fieldsCount; fieldIdx++) {
                const field = classType.getField(fieldIdx)
                hasJsonIgnore = JSONAPI.getJSONStringifyIgnoreByIdx(classType.cls, fieldIdx);
                if (hasJsonIgnore || field.isStatic()) {
                    continue
                }
                const index: number = this.findKeyIndex(field, writableFields)
                fieldRename = JSONAPI.getJSONRenameByIdx(classType.cls, fieldIdx);
                if (fieldRename != undefined) {
                    if (classType.hasField(fieldRename)) {
                        throw new Error("Cannot rename " + fieldRename + " in keys of " + classType.getName())
                    } else {
                        field.name = fieldRename
                    }
                    if (this.checkSameRename(field, writableFields)) {
                        throw new Error("Cannot double rename " + field.getName() + " in keys of " + classType.getName())
                    }
                }

                const fieldValue = classValue.getField(fieldIdx)

                const fieldTypeValuePair: [Field, reflect.Value] = [field, fieldValue]
                if (fieldRename != undefined || index == -1) {
                    writableFields.push(fieldTypeValuePair)
                } else {
                    writableFields[index] = fieldTypeValuePair
                }
            }
        }

        return writableFields
    }

    private writeArrayValue(arrayValue: ArrayValue, arrayType: ArrayType): void {
        this.buffer.append("[")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        const arrayLength = arrayValue.getLength().toInt()
        if (arrayLength > 0) {
            const array: Object = __narrowAny(arrayValue.getData())!

            this.path.add(array)

            const lastElementIndex = arrayLength - 1

            for (let idx = 0; idx < lastElementIndex; idx++) {
                this.writeArrayElementValue(arrayValue.getElement(idx), idx)
                this.buffer.append(",")

                if (this.useIndent) {
                    this.buffer.append("\n")
                }
            }

            this.writeArrayElementValue(arrayValue.getElement(lastElementIndex), lastElementIndex)

            this.path.delete(array)
        }

        if (this.useIndent) {
            this.indentLevel -= 1
            this.buffer.append("\n")
        }

        this.writePadding()

        this.buffer.append("]")
    }

    private writeArrayElementValue(elemValue: reflect.Value, elemIndex: int): void {
        this.writeObjectsArrayElement(elemValue.getData(), elemIndex)
    }

    private writeValueTypeWrapper(obj: Object): boolean {
        if (obj instanceof Boolean) {
            this.buffer.append(JSON.stringify(obj.unboxed()))
            return true
        } else if (obj instanceof Byte) {
            this.buffer.append(JSON.stringify(obj.unboxed()))
            return true
        } else if (obj instanceof Char) {
            this.buffer.append(JSON.stringify(obj.unboxed()))
            return true
        } else if (obj instanceof Short) {
            this.buffer.append(JSON.stringify(obj.unboxed()))
            return true
        } else if (obj instanceof Int) {
            this.buffer.append(JSON.stringify(obj.unboxed()))
            return true
        } else if (obj instanceof Long) {
            this.buffer.append(JSON.stringify(obj.unboxed()))
            return true
        } else if (obj instanceof Float) {
            this.buffer.append(JSON.stringify(obj.unboxed()))
            return true
        } else if (obj instanceof Double) {
            this.buffer.append(JSON.stringify(obj.unboxed()))
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

    private writeObjectsArrayElement(arrayElement: NullishType, elementIndex: int): void {
        this.writePadding()

        const replacedArrayElement = this.useReplacer ? this.replacer!("" + elementIndex, arrayElement) : arrayElement

        const arrayElementType = Type.of(replacedArrayElement)
        if (replacedArrayElement == null || arrayElementType instanceof LambdaType) {
            this.buffer.append("null")
        } else {
            this.checkReferencesCycle(replacedArrayElement)
            this.writeObject(replacedArrayElement)
        }
    }

    private checkReferencesCycle(obj: NullishType): void {
        if (obj == null) {
            return
        }

        const cycleDetected = this.path.has(__narrowAny<object>(obj))
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
                arrayResult += stringifyElementRecursively(array[i], replacer, space, depth + 1)
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
                if (ch < 32) {
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
    typ : Type
    color : int
    constructor(typ: Type, color: int) {
        this.typ = typ
        this.color = color
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
                    throw new SyntaxError("Expected \",\" or \"" + JSONObject.END_CHAR + "\" at " + this.getCurPosDescr() + ", got \"" + this.curChar + "\"")
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
                    throw new SyntaxError("Expected , or " + JSONArray.END_CHAR + " at " + this.getCurPosDescr() + ", got " + this.curChar)
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
        res.bigintValue = Double.isInteger(res.value) ? new BigInt(res.value) : new BigInt()
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
    private reviver: ((key: string, value: NullishType) => NullishType) | undefined
    private bigIntMode: int

    constructor(reviver: ((key: string, value: NullishType) => NullishType) | undefined, bigIntMode: int) {
        this.reviver = reviver
        this.bigIntMode = bigIntMode
    }

    private createNumber(value: JSONNumber, typ: Type): NullishType {
        if (typ.equals(BigIntType) && (this.bigIntMode === 2 || (!Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return new BigInt(value.bigintValue)
        } else if (typ instanceof DoubleType && (this.bigIntMode === 0 || (Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return new Double(value.value)
        } else if (typ instanceof IntType && (this.bigIntMode === 0 || (Double.isSafeInteger(value.value) && this.bigIntMode === 1))) {
            return new Double(value.value).toInt()
        } else if (typ instanceof LongType && (this.bigIntMode === 0 || this.bigIntMode === 1)) {
            return new Long().createFromJSONValue(value)
        } else if (typ instanceof ShortType && (this.bigIntMode === 0 || this.bigIntMode === 1)) {
            return new Short().createFromJSONValue(value)
        } else if (typ instanceof FloatType && (this.bigIntMode === 0 || this.bigIntMode === 1)) {
            return new Float().createFromJSONValue(value)
        } else if (typ instanceof ByteType && (this.bigIntMode === 0 || this.bigIntMode === 1)) {
            return new Byte().createFromJSONValue(value)
        } else {
            throw new Error(typ + " is expected, but get " + Type.of(value.value) + " with bigIntMode=" + this.bigIntMode)
        }
    }

    parse(value: JSONValue, typ: Type): NullishType {
        if (value instanceof JSONObject && typ instanceof ClassType) {
            return this.parseObject(value as JSONObject, typ as ClassType)
        } else if (value instanceof JSONObject && typ instanceof UnionType) {
            // NOTE(lyupaanastasia) parse union #27136
            throw new Error("UnionType is not supported yet")
        } else if (value instanceof JSONArray && typ instanceof ArrayType) {
            return this.parseArray(value as JSONArray, typ as ArrayType)
        } else if (value instanceof JSONNumber && (typ instanceof DoubleType || typ.equals(BigIntType) || typ instanceof IntType || typ instanceof LongType || typ instanceof ByteType || typ instanceof ShortType || typ instanceof FloatType)) {
            return this.createNumber(value as JSONNumber, typ)
        } else if (value instanceof JSONString && typ instanceof StringType) {
            return (value as JSONString).value
        } else if (value instanceof JSONTrue && typ instanceof BooleanType) {
            return new Boolean(true)
        } else if (value instanceof JSONFalse && typ instanceof BooleanType) {
            return new Boolean(false)
        } else if (value instanceof JSONNull && typ instanceof NullType) {
            return null
        } else {
            throw new Error(typ + " is expected, but get " + value)
        }
    }

    private classFieldSetter(classVal: ClassValue, fieldName: string, fieldVal: NullishType): void {
        classVal.setFieldByName(fieldName, reflect.Value.of(fieldVal))
    }

    private parseObject(jsonObj: JSONObject, classType: ClassType): Object {
        let setClassFieldFn = (cv: ClassValue, fn: string, fv: NullishType) => { cv.setFieldByName(fn, reflect.Value.of(fv)) }

        if (this.reviver != undefined) {
            setClassFieldFn = (classVal: ClassValue, fieldName: string, fieldVal: NullishType): void => {
                const revivedVal = this.reviver!(fieldName, fieldVal)

                // setFieldByName doesn't allow null/undefined values
                if (revivedVal != null) {
                    classVal.setFieldByName(fieldName, reflect.Value.of(revivedVal))
                }
            }
        }

        let obj = classType.make()
        let classVal = reflect.Value.of(obj) as ClassValue
        let hasJsonParseIgnore: boolean = false
        let hasJsonStringifyIgnore: boolean = false
        const jsonObjFields = jsonObj.getFields()
        for (let field_num = 0; field_num < classType.getFieldsNum(); field_num++) {
            let classField = classType.getField(field_num)
            hasJsonParseIgnore = JSONAPI.getJSONParseIgnoreFromAnnotation(classType.cls, field_num);
            if (hasJsonParseIgnore || classField.isStatic()) {
                continue
            }

            let jsonFieldVal = jsonObjFields.get(classField.getName())
            if (jsonFieldVal === undefined) {
                hasJsonStringifyIgnore = JSONAPI.getJSONStringifyIgnoreByIdx(classType.cls, field_num);
                if (hasJsonStringifyIgnore) {
                    continue
                }
                let fieldRename = JSONAPI.getJSONRenameByIdx(classType.cls, field_num);
                if (fieldRename == undefined) {
                    throw new Error("Cannot find " + classField.getName() + " in keys of " + classType.getName())
                }
                jsonFieldVal = jsonObjFields.get(fieldRename)
                if (jsonFieldVal === undefined) {
                    throw new Error("Cannot find rename " + fieldRename + " in keys of " + classType.getName())
                }
            }

            const classFieldVal = this.parse(jsonFieldVal, classField.getType())
            setClassFieldFn(classVal, classField.getName(), classFieldVal)
        }

        return obj
    }

    private parseArray(jVal: JSONArray, aType: ArrayType): Object | null {
        let len = jVal.values.length
        let arr = aType.make(len.toInt()) as FixedArray<NullishType>

        let setArrayElementFn = (a: FixedArray<NullishType>, i: int, v: NullishType) => { a[i] = v }

        if (this.reviver !== undefined) {
            setArrayElementFn = (array: FixedArray<NullishType>, index: int, elemVal: NullishType): void => {
                const revivedVal = this.reviver!(`${index}`, elemVal)
                array[index] = revivedVal !== undefined ? revivedVal : null
            }
        }

        for (let i = 0; i < len; i++) {
            let elem = this.parse(jVal.values.at(i) as JSONValue, aType.getElementType())
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
        return ch >= c'0' && ch <= c'9'
    }

    /**
     * Checks if a character is a hexadecimal digit
     * @param ch Character to check
     * @returns true if character is a hex digit
     */
    private isHexDigit(ch: char): boolean {
        return (ch >= c'0' && ch <= c'9') ||
                (ch >= c'A' && ch <= c'F') ||
                (ch >= c'a' && ch <= c'f')
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
