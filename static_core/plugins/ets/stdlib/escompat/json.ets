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


const BigIntType: Type = Type.of(new BigInt())

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

    private static readonly ESCAPED_CHARS: char[] = [c'\"', c'\\', c'\b', c'\f', c'\n', c'\r', c'\t']
    private static readonly NON_ESCAPED_CHARS: char[] = [c'"', c'\\', c'b', c'f', c'n', c'r', c't']
    private static readonly JSON_SPACE_INDENT_LIMIT = 10

    /**
     * Converts String to JSON format
     *
     * @param d: String - byte to be converted to a JSON as a String
     *
     * @returns String - JSON representation of byte
     */
    public static stringify(d: String): String {
        let sb = new StringBuilder([c'\"'])
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
        return new JSONWriter().write(obj)
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
                space?: string | number): string {
        const replacerOpt: ((key: string, value: NullishType) => NullishType) | undefined = (replacer === null) ? undefined : replacer

        if (space instanceof String || space === undefined) {
            return new JSONWriter(replacerOpt, space).write(obj)
        } else {
            return new JSONWriter(replacerOpt, JSON.spaceFromNumber(space as number)).write(obj)
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
    public static stringify(obj: NullishType, replacer: FixedArray<string | number>, space?: string | number): string {
        const filter : FixedArray<string> = new string[replacer.length]
        for (let i = 0; i < replacer.length; i++) {
            filter[i] = `${replacer[i]}`
        }

        if (space instanceof String || space === undefined) {
            return new JSONWriter(filter, space).write(obj)
        } else {
            return new JSONWriter(filter, JSON.spaceFromNumber(space as number)).write(obj)
        }
    }

    public static stringify(obj: NullishType, replacer: Array<string | number>, space?: string | number): string {
        const filter : FixedArray<string> = new string[replacer.length]
        for (let i = 0; i < replacer.length; i++) {
            filter[i] = `${replacer[i]}`
        }

        if (space instanceof String || space === undefined) {
            return new JSONWriter(filter, space).write(obj)
        } else {
            return new JSONWriter(filter, JSON.spaceFromNumber(space as number)).write(obj)
        }
    }

    private static spaceFromNumber(space: number): String {
        let spacesCount = 0
        if (space > 0) {
            if (space > JSON.JSON_SPACE_INDENT_LIMIT) {
                spacesCount = JSON.JSON_SPACE_INDENT_LIMIT
            } else {
                spacesCount = space as int
            }
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
        const parsingResult = new JSONValueParser(reviver, bigIntMode).parse(jsonValue, type) as T

        if (reviver !== undefined) {
            return reviver("", parsingResult) as T
        } else {
            return parsingResult
        }
    }
}

class JSONWriter {
    // NOTE(cheezzario) replace with Type.for<JSValue>() when it will be implemented
    private static readonly STD_CORE_INTEROP_JSVALUE_TYPE = (Type.of([] as FixedArray<JSValue>) as ArrayType).getElementType()
    private static readonly ESCOMPAT_ARRAY_TYPE = Type.of(new Array<Object>(0))
    private static readonly ESCOMPAT_SET_TYPE = Type.of(new Set<Object>())
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
            this.writeRecord(obj as Object as Record<string, NullishType>);
        } else if (obj instanceof Map) {
            this.buffer.append("{}")
        } else if (this.writeValueTypeWrapper(obj)) {
            // nothing to do - write completed successfully
        } else {
            const objType = Type.of(obj)

            if (objType.getName() == JSONWriter.STD_CORE_INTEROP_JSVALUE_TYPE.getName()) {
                this.buffer.append(JSON.stringify(obj as JSValue))
            } else if (objType instanceof ArrayType) {
                this.writeArrayValue(Value.of(obj) as ArrayValue, objType as ArrayType)
            } else if (objType.getName() == JSONWriter.ESCOMPAT_ARRAY_TYPE.getName()) {
                this.writeEscompatArray(obj as Array<Object>)
            } else if (objType.getName() == JSONWriter.ESCOMPAT_SET_TYPE.getName() ||
                       objType.getName() == JSONWriter.STD_CORE_PROMISE_TYPE.getName()) {
                this.buffer.append("{}")
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

        const writableFields = this.getWritableFields(objType, Value.of(obj) as ClassValue)

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

    private writeTupleElements(tupleFields: Array<[Field, Value]>): boolean {
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

    private writeRecord(rec: Record<string, NullishType>): void {
        this.buffer.append("{");
        this.checkReferencesCycle(rec);
        this.path.add(rec);
        let isFirst = true;
        for (let key of rec.keys()) {
            if (rec[key] !== undefined) {
                if (!isFirst) {
                    this.buffer.append(",");
                } else {
                    isFirst = false;
                }
                this.buffer.append(JSON.stringify(key as String));
                this.buffer.append(":");
                this.writeObject(rec[key]);
            }
        }
        this.path.delete(rec);
        this.buffer.append("}");
    }

    private writeClassValue(obj: Object, objType: ClassType): void {
        const currentBuffer = this.buffer
        this.buffer = new StringBuilder("{")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        const writableFields = this.getWritableFields(objType, Value.of(obj) as ClassValue)

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

        if (fieldDumped) {
            currentBuffer.append(this.buffer.toString())
        } else {
            currentBuffer.append("{}")
        }

        this.buffer = currentBuffer
    }

    private writeClassFields(writableFields: Array<[Field, Value]>): boolean {
        const FIELD_TYPE = 0
        const FIELD_VALUE = 1

        let fieldDumped = false
        for (let fieldIdx = 0; fieldIdx < writableFields.length; fieldIdx++) {
            const fieldTypeValuePair = writableFields[fieldIdx]

            const objField = fieldTypeValuePair[FIELD_TYPE]
            const objFieldValue = fieldTypeValuePair[FIELD_VALUE]

            const objToDump = this.useReplacer ? this.replacer!(objField.getName(), objFieldValue.getData()) : objFieldValue.getData()
            if (objToDump !== undefined) {
                if (fieldDumped) {
                    this.buffer.append(",")
                    if (this.useIndent) {
                        this.buffer.append("\n")
                    }
                } else {
                    fieldDumped = true
                }

                this.checkReferencesCycle(objToDump)
                this.writePadding()

                this.buffer.append('"')
                this.buffer.append(objField.getName())
                this.buffer.append('":')

                if (this.useIndent) {
                    this.buffer.append(" ")
                }

                this.writeObject(objToDump)
            }
        }

        return fieldDumped
    }

    private getWritableFields(classType: ClassType, classValue: ClassValue): Array<[Field, Value]> {
        const writableFields = new Array<[Field, Value]>()

        if (this.useFieldsFilter) {
            for (let fieldIdx = 0; fieldIdx < this.fieldsFilter.length; fieldIdx++) {
                const fieldName = this.fieldsFilter[fieldIdx]
                if (!classType.hasField(fieldName)) {
                    continue
                }

                const fieldType = classType.getFieldByName(fieldName)
                if (!this.isWritableFieldType(fieldType)) {
                    continue
                }

                const fieldValue = classValue.getFieldByName(fieldName)

                const fieldTypeValuePair: [Field, Value] = [fieldType, fieldValue]
                writableFields.push(fieldTypeValuePair)
            }
        } else {
            const fieldsCount = classValue.getFieldsNum()
            for (let fieldIdx = 0; fieldIdx < fieldsCount; fieldIdx++) {
                const fieldType = classType.getField(fieldIdx)
                if (!this.isWritableFieldType(fieldType)) {
                    continue
                }

                const fieldValue = classValue.getField(fieldIdx)

                const fieldTypeValuePair: [Field, Value] = [fieldType, fieldValue]
                writableFields.push(fieldTypeValuePair)
            }
        }

        return writableFields
    }

    private isWritableFieldType(field: Field): boolean {
        const fieldType = field.getType()

        if (field.isStatic() || fieldType instanceof LambdaType) {
            return false
        }

        return true
    }

    private writeArrayValue(arrayValue: ArrayValue, arrayType: ArrayType): void {
        this.buffer.append("[")

        if (this.useIndent) {
            this.buffer.append("\n")
            this.indentLevel += 1
        }

        const arrayLength = arrayValue.getLength() as int
        if (arrayLength > 0) {
            const array: Object = arrayValue.getData()!

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

    private writeArrayElementValue(elemValue: Value, elemIndex: int): void {
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

            const lastElementIndex = (array.length as int) - 1

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

        const cycleDetected = this.path.has(obj)
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
            return new Double(value.value).intValue()
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
        classVal.setFieldByName(fieldName, Value.of(fieldVal))
    }

    private parseObject(jsonObj: JSONObject, classType: ClassType): Object {
        let setClassFieldFn = (cv: ClassValue, fn: string, fv: NullishType) => { cv.setFieldByName(fn, Value.of(fv)) }

        if (this.reviver != undefined) {
            setClassFieldFn = (classVal: ClassValue, fieldName: string, fieldVal: NullishType): void => {
                const revivedVal = this.reviver!(fieldName, fieldVal)

                // setFieldByName doesn't allow null/undefined values
                if (revivedVal != null) {
                    classVal.setFieldByName(fieldName, Value.of(revivedVal))
                }
            }
        }

        let obj = classType.make()
        let classVal = Value.of(obj) as ClassValue
        const jsonObjFields = jsonObj.getFields()
        for (let field_num = 0; field_num < classType.getFieldsNum(); field_num++) {
            let classField = classType.getField(field_num)
            if (classField.isStatic()) {
                continue
            }

            const jsonFieldVal = jsonObjFields.get(classField.getName())
            if (jsonFieldVal === undefined) {
                throw new Error("Cannot find " + classField.getName() + " in keys of " + classType.getName())
            }

            const classFieldVal = this.parse(jsonFieldVal, classField.getType())
            setClassFieldFn(classVal, classField.getName(), classFieldVal)
        }

        return obj
    }

    private parseArray(jVal: JSONArray, aType: ArrayType): Object | null {
        let len = jVal.values.length
        let arr = aType.make(len as int) as FixedArray<NullishType>

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

export class JSONValue extends Object {}

export class JSONObject extends JSONValue {
    keys_: Array<JSONString> = new Array<JSONString>()
    values: Array<JSONValue> = new Array<JSONValue>()
    readonly static START_CHAR = c'{'
    readonly static END_CHAR = c'}'
    readonly static SEPARATOR = c':'
    readonly static DELIMETER = c','

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
        let res = new StringBuilder([JSONObject.START_CHAR])
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
        let res = new StringBuilder([JSONArray.START_CHAR])
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

export interface JSONable<T> {
    createFromJSONValue(json: JSONValue): T {
        throw new JSONTypeError("createFromJSONValue was not overrided")
    }
}
