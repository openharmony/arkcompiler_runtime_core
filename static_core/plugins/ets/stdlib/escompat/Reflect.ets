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

export class Reflect {

    private static readonly GETTER_PREFIX = "<get>"
    private static readonly SETTER_PREFIX = "<set>"
    private static native isLiteralInitializedInterfaceImpl(target: Object): boolean

    private constructor () {}

    /**
    * Gets the field of target, equivalent to target.key
    *
    * @param target the target object on which to get the field
    *
    * @param key the string name of the field to get
    *
    * @returns the value of the field
    */
    public static get(target: Object, key: PropertyKey): NullishType {
        if (target instanceof Char ||
            target instanceof Boolean ||
            target instanceof Byte ||
            target instanceof Short ||
            target instanceof Int ||
            target instanceof Long ||
            target instanceof Float ||
            target instanceof Double ||
            target instanceof String) {
            throw new Error("`target` argument of Reflect.get must have fields")
        }
        if (!Reflect.has(target, key)) {
            return undefined
        }
        const t = Type.of(target)
        if (t instanceof ClassType) {
            if (t.hasField(key)) {
                const classVal = Value.of(target) as ClassValue
                return classVal.getFieldByName(key).getData()
            } else {
                const getter = Reflect.findMethod(t, Reflect.GETTER_PREFIX + key)
                if (getter !== undefined) {
                    return getter.invoke(target, [])
                } else {
                    return undefined
                }
            }
        } else if (t instanceof ArrayType) {
            if (key == ARRAY_LENGTH_MEMBER_NAME) {
                return new Number((Value.of(target) as ArrayValue).getLength())
            }
        } else if (t instanceof LambdaType) {
            if (key == FUNCTION_LENGTH_MEMBER_NAME) {
                return new Number((t as FunctionType).getParametersNum())
            } else if (key == FUNCTION_NAME_MEMBER_NAME) {
                return ""
            }
        } else if (t instanceof EnumType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof UnionType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof TupleType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        }
        return undefined
    }

    /**
     * Gets the property of the record
     *
     * @param target Record that contains the property
     *
     * @param key name of the property
     *
     * @returns property value
     */
    public static get(target: Record<PropertyKey, NullishType>, key: string): NullishType {
        return target[key]
    }

    /**
    * Gets the element of target, equivalent to target[index]
    *
    * @param target the target object on which to get the element
    *
    * @param key the number index of the element to get
    *
    * @returns the value of the element
    */
    public static get(target: Object, index: number): Object | undefined {
        if (target instanceof Char ||
            target instanceof Boolean ||
            target instanceof Byte ||
            target instanceof Short ||
            target instanceof Int ||
            target instanceof Long ||
            target instanceof Float ||
            target instanceof Double ||
            target instanceof String) {
            throw new Error("`target` argument of Reflect.get must be indexed")
        }
        if (!Reflect.has(target, index)) {
            return undefined
        }
        let t = Type.of(target)
        if (t instanceof ArrayType) {
            let av = Value.of(target) as ArrayValue
            return __narrowAny<object | undefined>(av.getElement(index as long).getData())
        } else if (t instanceof EnumType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof UnionType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        } else if (t instanceof TupleType) {
            // NOTE(shumilov-petr): Not implemented
            throw new Error("Not implemented")
        }
        return undefined
    }

    // same as Double.numberFromString
    private static native numberFromString(s: String): double;

    private static toNumberInt(value: NullishType): double {
        if (value instanceof Double) {
            return value.unboxed();
        }
        if (value instanceof Int) {
            return value.toDouble();
        }
        if (value instanceof bigint) {
            throw new TypeError("Cannot convert a BigInt value to a number");
        }
        if (value === undefined) {
            return NaN;
        }
        if (value === null || value === false) {
            return 0;
        }
        if (value === true) {
            return 1;
        }
        if (value instanceof string) {
            return Reflect.numberFromString(value);
        }
        return NaN;
    }

    private static toNumberDouble(value: NullishType): double {
        if (value instanceof Double) {
            return value.unboxed();
        }
        if (value instanceof Int) {
            return value.toDouble();
        }
        if (value instanceof bigint) {
            throw new TypeError("Cannot convert a BigInt value to a number");
        }
        if (value === undefined) {
            return NaN;
        }
        if (value === null || value === false) {
            return 0;
        }
        if (value === true) {
            return 1;
        }
        if (value instanceof string) {
            return Double.parseFloat(value);
        }
        return NaN;
    }

    private static isValidBigIntDigits(digits: string, radix: int): boolean {
        let temp: string = digits.toLowerCase();
        for (let i = 0; i < digits.length; i++) {
            let ch = temp.charAt(i);
            switch (radix) {
                case 2:
                    if (ch !== c'0' && ch !== c'1') {
                        return false;
                    }
                    break;
                case 8:
                    if (ch < c'0' || ch > c'7') {
                        return false;
                    }
                    break;
                case 10:
                    if (ch < c'0' || ch > c'9') {
                        return false;
                    }
                    break;
                case 16:
                    if (!(
                        (ch >= c'0' && ch <= c'9') ||
                        (ch >= c'a' && ch <= c'f')
                    )) {
                        return false;
                    }
                    break;
                default:
                    return false;
            }
        }
        return true;
    }

    private static parseBigIntFromString(value: string): bigint | long | double {
        const POSITIVE_SIGN = 1;
        const NEGATIVE_SIGN = 2;
        const NO_SIGN = 0;

        let trimmed: string = value.trim();
        let len: int = trimmed.length.toInt();
        let index: int = 0;

        if (len === 0) {
            return 0n;
        }

        let sign: byte = NO_SIGN;
        let firstChar = trimmed.charAt(index);
        if (firstChar === c'+') {
            if (len === 1) {
                return NaN;
            }
            sign = POSITIVE_SIGN;
            index++;
        } else if (firstChar === c'-') {
            if (len === 1) {
                return NaN;
            }
            sign = NEGATIVE_SIGN;
            index++;
        }

        if (index >= len) {
            return NaN;
        }

        let radix: int = 10;

        if (trimmed.charAt(index) === c'0') {
            index++;
            if (index >= len) {
                return 0n;
            }
            let prefix: char = trimmed.charAt(index);
            switch (prefix) {
                case c'x':
                case c'X':
                    radix = 16;
                    index++;
                    if (index >= len) {
                        return NaN;
                    }
                    break;
                case c'o':
                case c'O':
                    radix = 8;
                    index++;
                    if (index >= len) {
                        return NaN;
                    }
                    break;
                case c'b':
                case c'B':
                    radix = 2;
                    index++;
                    if (index >= len) {
                        return NaN;
                    }
                    break;
                default:
                    break;
            }
        }

        // Skip leading zeroes
        while (trimmed.charAt(index) === c'0') {
            index++;
            if (index === len) {
                return 0n;
            }
        }

        let digits = trimmed.substring(index);
        if (!Reflect.isValidBigIntDigits(digits, radix)) {
            return NaN;
        }

        let result: long|double = Reflect.stringToLongWithRadix(digits, radix);
        if (result == NaN) {
            return NaN;
        }
        let temp: long = result as long;
        return sign === NEGATIVE_SIGN ? -temp : temp;
    }

    private static stringToLongWithRadix(digits: string, radix: int): long|double {
        if (digits.length === 0) {
            let temp: long = 0;
            return temp;
        }

        let result: long = 0;
        const ZERO_CODE: int = 48; // c'0'
        const A_CODE: int = 97; // c'a'

        let temp: string = digits.toLocaleLowerCase();
        let len: int = digits.length.toInt();
        for (let i: int = 0; i < len; i++) {
            let ch: char = temp.charAt(i);
            let code: int = ch as int;

            let digit: int;
            if (code >= ZERO_CODE && code <= ZERO_CODE + 9) {
                digit = code - ZERO_CODE;
            } else if (code >= A_CODE && code <= A_CODE + 25) {
                digit = code - A_CODE + 10;
            } else {
                return NaN;
            }

            if (digit >= radix) {
                return NaN;
            }

            // Multiply previous result by radix and add current digit
            result = result * radix + digit;
        }

        return result;
    }

    private static toBigInt(value: NullishType): long|bigint|double {
        if (value instanceof Long) {
            return value.unboxed();
        }
        if (value instanceof bigint) {
            return value;
        }
        if (value === undefined || value === null) {
            throw new TypeError("Cannot convert a undefine or null value to a BigInt");
        }
        if (value instanceof Boolean) {
            return value.unboxed() === true ? 1n : 0n;
        }
        if (value instanceof Double || value instanceof Int) {
            throw new TypeError("Cannot convert a Number value to a BigInt");
        }
        if (value instanceof string) {
            let result = Reflect.parseBigIntFromString(value);
            if (result === NaN) {
                throw new SyntaxError("Cannot convert string to a BigInt, because not allow Infinity, decimal points, or exponents");
            }
            if (result instanceof Long) {
                return result.unboxed();
            } else if (result instanceof bigint) {
                return result.getLong();
            }
        }
        return NaN;
    }

    private static getIndexForTypedArray(target: ArrayLike<number> | ArrayLike<BigInt>, key: PropertyKey): int {
        for (let i: int = 0; i < key.length; i++) {
            let c: char = key.charAt(i);
            if (c > c'9' || c < c'0') {
                return -1;
            }
        }
        let temp: double = Double.parseInt(key, 10);
        if (temp === NaN || temp === -NaN || temp === Double.POSITIVE_INFINITY || temp === Double.NEGATIVE_INFINITY) {
            return -1;
        }
        let index: int = Double.toInt(temp);
        if (index < 0 || index >= Double.toInt(target.length)) {
            return -1;
        }
        return index;
    }

    private static setIntTypedArray(target: ArrayLike<number>, key: PropertyKey, value: NullishType): boolean {
        let index: int = Reflect.getIndexForTypedArray(target, key);
        if (index < 0) {
            return true;
        }
        let temp: double = Reflect.toNumberInt(value);
        if (temp === NaN) {
            return true;
        }
        let tempInt: int = temp.toInt();
        if (target instanceof Int8Array) {
            (target as Int8Array)[index] = tempInt;
        } else if (target instanceof Int16Array) {
            (target as Int16Array)[index] = tempInt;
        } else if (target instanceof Int32Array) {
            (target as Int32Array)[index] = tempInt;
        } else if (target instanceof Uint8Array) {
            (target as Uint8Array)[index] = tempInt;
        } else if (target instanceof Uint16Array) {
            (target as Uint16Array)[index] = tempInt;
        } else if (target instanceof Uint32Array) {
            (target as Uint32Array)[index] = tempInt;
        }
        return true;
    }

    private static setBigIntTypedArray(target: ArrayLike<BigInt>, key: PropertyKey, value: NullishType): boolean {
        let index: int = Reflect.getIndexForTypedArray(target, key);
        if (index < 0) {
            return true;
        }
        let temp = Reflect.toBigInt(value);
        if (temp === NaN) {
            return true;
        }
        let result: long = 0;
        if (temp instanceof Long) {
            result = temp.unboxed();
        } else if (temp instanceof bigint) {
            result = temp.getLong();
        }
        if (target instanceof BigInt64Array) {
            (target as BigInt64Array)[index] = result;
        } else if (target instanceof BigUint64Array) {
            (target as BigUint64Array)[index] = result;
        }
        return true;
    }

    private static setFloatTypedArray(target: ArrayLike<number>, key: PropertyKey, value: NullishType): boolean {
        let index: int = Reflect.getIndexForTypedArray(target, key);
        if (index < 0) {
            return true;
        }
        let temp: double = Reflect.toNumberDouble(value);
        if (temp === NaN) {
            return true;
        }

        if (target instanceof Float32Array) {
            (target as Float32Array)[index] = temp;
        } else if (target instanceof Float64Array) {
            (target as Float64Array)[index] = temp;
        }
        return true;
    }

    private static checkIntTypedArray(target: Object): boolean {
        return (target instanceof Int8Array
            || target instanceof Int16Array
            || target instanceof Int32Array
            || target instanceof Uint8Array
            || target instanceof Uint16Array
            || target instanceof Uint32Array);
    }

    private static checkBigIntTypedArray(target: Object): boolean {
        return (target instanceof BigInt64Array
            || target instanceof BigUint64Array);
    }

    private static checkFloatTypedArray(target: Object): boolean {
        return (target instanceof Float32Array
            || target instanceof Float64Array);
    }

    /**
    * Sets the field of target, equivalent to target.key = value
    *
    * @param target the target object on which to set the field
    *
    * @param key the name of the field to set
    *
    * @param value the value to set.
    *
    * @returns a Boolean indicating whether or not setting the field was successful
    */
    public static set(target: Object, key: PropertyKey, value: NullishType): boolean {
        let typedArrayFlag: boolean = Reflect.checkIntTypedArray(target);
        if (typedArrayFlag) {
            return Reflect.setIntTypedArray(target as ArrayLike<number>, key, value);
        }
        typedArrayFlag = Reflect.checkBigIntTypedArray(target);
        if (typedArrayFlag) {
            return Reflect.setBigIntTypedArray(target as ArrayLike<BigInt>, key, value);
        }
        typedArrayFlag = Reflect.checkFloatTypedArray(target);
        if (typedArrayFlag) {
            return Reflect.setFloatTypedArray(target as ArrayLike<number>, key, value);
        }

        if (!Reflect.has(target, key)) {
            return false
        }
        let v = Value.of(target)
        let vt = v.getType()
        if (vt instanceof ClassType) {
            let cv = v as ClassValue
            if (vt.hasField(key)) {
                try {
                    cv.setFieldByName(key, Value.of(value))
                    return true
                } catch (e: Error) {
                    return false
                }
            } else {
                const setter = Reflect.findMethod(vt, Reflect.SETTER_PREFIX + key)
                if (setter !== undefined) {
                    setter.invoke(target, [value])
                    return true
                } else {
                    return false
                }
            }
        }
        return false
    }

    private static findMethod(type: ClassType, name: string): Method | undefined {
        for (let methodIdx = 0; methodIdx < type.getMethodsNum(); methodIdx++) {
            const method = type.getMethod(methodIdx)
            if (method.getName() == name) {
                return method
            }
        }

        return undefined
    }

    /**
     * Sets the property of the record
     *
     * @param target Record that contains the property
     *
     * @param key name of the property
     *
     * @param value of the property
     *
     * @returns a Boolean indicating whether or not setting the property was successful
     */
    public static set(target: Record<PropertyKey, NullishType>, key: string, value: Object): Boolean {
        target[key] = value
        return true
    }

    /**
    * Sets the element of target, equivalent to target[index] = value
    *
    * @param target the target object on which to set the element
    *
    * @param index the index of the element to set
    *
    * @param value the value to set.
    *
    * @returns a boolean indicating whether or not setting the element was successful
    */
    public static set(target: Object, index: number, value: Object): Boolean {
        let v = Value.of(target)
        let vt = v.getType()
        if (vt instanceof ArrayType) {
            if (!Reflect.has(target, index)) {
                return false
            }
            let av = v as ArrayValue
            try {
                av.setElement(index.toLong(), Value.of(value))
            } catch (e: Error) {
                return false
            }
            return true
        }
        return false
    }

    /**
    * Returns the names of the own fields of an object
    *
    * @param o object that contains the own fields
    *
    * @returns array representation of names
    */
    public static ownKeys(target: Object): string[] {
        return Object.getOwnPropertyNames(target)
    }

    /**
     * Returns the string keys of the properties of a record
     *
     * @param target Record that contains the properties
     *
     * @returns as array of the target Record property names
     */
    public static ownKeys(target: Record<PropertyKey, NullishType>): string[] {
        return Object.getOwnPropertyNames(target)
    }

    /**
    * Determines whether an object has a member with the specified name
    *
    * @param target an object
    *
    * @param key a member name
    *
    * @returns a boolean indicating whether or not the target has the field
    */
    public static has(target: Object, key: string): boolean {
        if (!Object.hasOwn(target, key)) {
            const t = Type.of(target)
            if (t instanceof ClassType) {
                const ct = t as ClassType
                const mnum = ct.getMethodsNum()
                for (let i = 0; i < mnum; ++i) {
                    const m = ct.getMethod(i)
                    if (m.isStatic()) {
                        continue
                    }

                    const methodName = m.getName()
                    if (methodName == key) {
                        return true
                    } else if (methodName == Reflect.GETTER_PREFIX + key) {
                        return true
                    } else if (methodName == Reflect.SETTER_PREFIX + key) {
                        return true
                    }
                }
            } else if (t instanceof StringType || t instanceof ArrayType || t instanceof LambdaType) {
                return (key == OBJECT_TO_STRING_MEMBER_NAME ||
                    key == OBJECT_TO_LOCALE_STRING_MEMBER_NAME ||
                    key == OBJECT_HAS_OWN_PROPERTY_MEMBER_NAME)
            } else if (t instanceof EnumType) {
                // NOTE(shumilov-petr): Not implemented
                throw new Error("Not implemented")
            } else if (t instanceof UnionType) {
                // NOTE(shumilov-petr): Not implemented
                throw new Error("Not implemented")
            } else if (t instanceof TupleType) {
                // NOTE(shumilov-petr): Not implemented
                throw new Error("Not implemented")
            }
            return false
        }
        return true
    }

    /**
    * Determines whether a record has a property with the specified name
    *
    * @param target Record that contains a property
    *
    * @param key name of the property
    *
    * @returns a boolean indicating whether or not the record has the specified property
    */
    public static has(target: Record<PropertyKey, NullishType>, key: string): boolean {
        return target.has(key)
    }

    /**
    * Determines whether an object has a field with the specified number
    *
    * @param target an object
    *
    * @param index an element index
    *
    * @returns a boolean indicating whether or not the target has the element
    */
    public static has(target: Object, index: number): boolean {
        return Object.hasOwn(target, index)
    }

    /**
    * Determines whether an object is an interface initialized with a literal
    *
    * @param target an object
    *
    * @returns a boolean indicating whether or not the target is an interface
    *          initialized with a literal
    */
    public static isLiteralInitializedInterface(target: Object): boolean {
        return Reflect.isLiteralInitializedInterfaceImpl(target)
    }
}
