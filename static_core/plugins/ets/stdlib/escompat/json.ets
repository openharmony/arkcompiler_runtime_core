/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
        return StringBuilder.toString(d)
    }

    /**
     * Converts double to JSON format
     *
     * @param d: double - double to be converted to a JSON as a Number
     *
     * @returns String - JSON representation of double
     */
    public static stringify(d: double): String {
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

    private static readonly ESCAPED_CHARS: char[] = [c'\"', c'\\', c'\/', c'\b', c'\f', c'\n', c'\r', c'\t']
    private static readonly NON_ESCAPED_CHARS: char[] = [c'"', c'\\', c'/', c'b', c'f', c'n', c'r', c't']

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
        for (let i = 0; i < len; ++i) {
            let c = d.charAt(i)
            let escapedCharIndex = lastIndexOf(JSON.ESCAPED_CHARS, c, JSON.ESCAPED_CHARS.length)
            if (escapedCharIndex != -1) {
                sb.append(c'\\')
                sb.append(JSON.NON_ESCAPED_CHARS[escapedCharIndex])
            } else {
                sb.append(c)
            }
        }
        sb.append(c'\"')
        return sb.toString()
    }

    /**
     * Converts Object to JSON format
     *
     * @param d: Object - byte to be converted to a JSON as an Object
     *
     * @returns String - JSON representation of Object
     */
    public static stringify(d: Object): String {
        return JSON.stringifyObj(d)
    }

    private static native stringifyObj(d: Object): String

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
    public static stringify(d: byte[]): String {
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
     * @param d: short[] - shorts array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of shorts array
     */
    public static stringify(d: short[]): String {
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
     * @param d: int[] - ints array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of ints array
     */
    public static stringify(d: int[]): String {
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
     * @param d: long[] - longs array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of longs array
     */
    public static stringify(d: long[]): String {
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
     * @param d: byte[] - array of byte to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of array of bytes
     */
    public static stringify(d: float[]): String {
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
     * Converts doubles array to JSON format
     *
     * @param d: double[] - doubles array to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of doubles array
     */
    public static stringify(d: double[]): String {
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
     * Converts chars array to JSON format
     *
     * @param d: char[] - chars array  to be converted to a JSON as an Array of Numbers
     *
     * @returns String - JSON representation of chars array
     */
    public static stringify(d: char[]): String {
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
     * @param d: boolean[] - booleans array to be converted to a JSON as an Array of Boolean literals
     *
     * @returns String - JSON representation of booleans array
     */
    public static stringify(d: boolean[]): String {
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
        let id = typ.getId()
        let ind = used.indexOf(new TypeColor(id, 0))
        if (ind != -1) {
            return (used.at(ind) as TypeColor).color == TypeColor.VISITED
        }
        used.push(new TypeColor(id, TypeColor.AT_STACK))
        let ok = true
        if (typ instanceof ClassType) {
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
                    || typ instanceof NullType) {
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

    public static parse<T>(str: String, typ: Type): NullishType {
        if (!JSON.checkType(typ, new Array<TypeColor>())) {
            throw new Error("Incorrect type: " + typ.toString())
        }
        let value = JSONParser.parse(str)
        return JSON.parse(value, typ)
    }

    private static parseObject(jVal: JSONObject, cType: ClassType): Object {
        let object = cType.make()
        let cVal = Value.of(object) as ClassValue
        let unboxString : (v: JSONString) => String =
            (v: JSONString): String => { return v.value }
        let fieldArray = jVal.keys_.map<String>(unboxString)
        for (let field_num = 0; field_num < cType.getFieldsNum(); field_num++) {
            let field : Field = cType.getField(field_num)
            let indexInJSON = fieldArray.indexOf(field.getName())
            if (indexInJSON == -1) {
                throw new Error("Cannot find " + field.getName() + " in keys of " + cType.getName())
            } else {
                let fObject = JSON.parse(jVal.values.at(indexInJSON) as JSONValue, field.getType())
                cVal.setFieldByName(field.getName(), Value.of(fObject))
            }
        }
        return object
    }

    private static parseArray(jVal: JSONArray, aType: ArrayType): Object | null {
        let len = jVal.values.length
        let arr = aType.make(len as int)
        let arrVal = Value.of(arr) as ArrayValue
        for (let i = 0; i < len; i++) {
            let elem = JSON.parse(jVal.values.at(i) as JSONValue, aType.getElementType())
            arrVal.setElement(i, Value.of(elem))
        }
        return arr
    }

    private static parse(value: JSONValue, typ: Type): Object | null {
        if (value instanceof JSONObject && typ instanceof ClassType) {
            return JSON.parseObject(value as JSONObject, typ as ClassType)
        } else if (value instanceof JSONArray && typ instanceof ArrayType) {
            return JSON.parseArray(value as JSONArray, typ as ArrayType)
        } else if (value instanceof JSONNumber && typ instanceof DoubleType) {
            return new Double((value as JSONNumber).value)
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

}


class TypeColor {
    readonly static VISITED : int = 1
    readonly static AT_STACK : int = 2
    typeID : long
    color : int
    constructor(typeID: long, color: int) {
        this.typeID = typeID
        this.color = color
    }
    override equals(oth: NullishType): boolean {
        return (oth instanceof Object) && (oth as TypeColor).typeID == this.typeID
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
            return res
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
        this.curPos.restoreFromCopy(posCopy)
        return res
    }

    private static readonly CAN_BE_ESCAPED_CHARS: char[] = [c'"', c'\\', c'/', c'b', c'f', c'n', c'r', c't']
    private static readonly ESCAPED_CHARS: char[] = [c'\"', c'\\', c'\/', c'\b', c'\f', c'\n', c'\r', c'\t']

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
                    return res
                }
            }
            hasMetEscape = this.curChar == c'\\'
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

export class JSONValue extends Object {}

export class JSONObject extends JSONValue {
    keys_: Array<JSONString> = new Array<JSONString>()
    values: Array<JSONValue> = new Array<JSONValue>()
    readonly static START_CHAR = c'{'
    readonly static END_CHAR = c'}'
    readonly static SEPARATOR = c':'
    readonly static DELIMETER = c','

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

    public override toString(): String {
        return (new Double(this.value)).toString()
    }
}

export class JSONString extends JSONValue {
    value: String
    readonly static START_CHAR = c'"'
    readonly static END_CHAR = c'"'

    public override toString(): String {
        return this.value
    }

    public override equals(to: NullishType): boolean {
        return (to instanceof JSONString) && this.value == (to as JSONString).value
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
    static createFromJSONValue(json: JSONValue): T {
        throw new JSONTypeError("createFromJSONValue was not overrided")
    }
}
