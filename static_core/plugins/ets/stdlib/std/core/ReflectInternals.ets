/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
/**
 * @file Defines the ReflectInternals for ArkTS
 * @kit ArkTS
 */


package std.core;

/**
 * Namespace for ArkTS reflection-related internal unstable APIs, including type operations, conversions, and checks.
 *
 * @namespace reflect.internals
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export namespace reflect.internals {

/**
 * Unstable APIs, for internal use by the standard library only, Not recommended for developers to use.
 * Contains various methods for type operations, conversions, and checks.
 *
 * @extends Types
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class Types {
    private readonly static BOXED_DOUBLE = Class.from<Double>()
    private readonly static BOXED_BOOLEAN = Class.from<Boolean>()
    private readonly static BOXED_INT = Class.from<Int>()
    private readonly static BOXED_LONG = Class.from<Long>()
    private readonly static BOXED_BYTE = Class.from<Byte>()
    private readonly static BOXED_SHORT = Class.from<Short>()
    private readonly static BOXED_FLOAT = Class.from<Float>()
    private readonly static BOXED_CHAR = Class.from<Char>()

    private constructor() {}

    /**
     * Performs identity conversion, directly returns the input array-like object.
     *
     * @static
     * @param { ArrayLike<T> } array The array-like object to be converted.
     * @returns { ArrayLike<T> } The same array-like object as the input.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static identity_cast<T>(array: ArrayLike<T>): ArrayLike<T> {
        return array
    }

    /**
     * Casts an object to FixedArray type.
     *
     * @static
     * @param { object } obj The object to be converted.
     * @returns { FixedArray<E> } The converted FixedArray instance with element type `E`.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static asFixedArray<E>(obj: object): FixedArray<E> {
        return obj as FixedArray<E>
    }

    /**
     * Checks if a type can be assigned from another type. This includes subtype relationships,
     * assignment compatibility between numeric types, between boolean types, and between character types.
     *
     * @static
     * @param { Class } lhsType The target type.
     * @param { Class } rhsType The source type.
     * @returns { boolean } Returns true if rhsType can be assigned to lhsType; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isAssignableFrom(lhsType: Class, rhsType: Class): boolean {
        return (rhsType.isSubtypeOf(lhsType))
            || (Types.isNumericType(lhsType) && Types.isNumericType(rhsType))
            || (Types.isBooleanType(lhsType) && Types.isBooleanType(rhsType))
            || (Types.isCharType(lhsType) && Types.isCharType(rhsType))
    }

    /**
     * Checks if the given Class object represents a double type (including boxed and primitive types).
     *
     * @static
     * @param { Class } type The Class object to be checked.
     * @returns { boolean } Returns true if it's a double type; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isDoubleType(type: Class): boolean {
        return type === Types.BOXED_DOUBLE || type === Class.PRIMITIVE_DOUBLE
    }

    /**
     * Checks if the given Class object represents a boolean type (including boxed and primitive types).
     *
     * @static
     * @param { Class } type The Class object to be checked.
     * @returns { boolean } Returns true if it's a boolean type; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isBooleanType(type: Class): boolean {
        return type === Types.BOXED_BOOLEAN || type === Class.PRIMITIVE_BOOLEAN
    }

    /**
     * Checks if the given Class object represents an int type (including boxed and primitive types).
     *
     * @static
     * @param { Class } type The Class object to be checked.
     * @returns { boolean } Returns true if it's an int type; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isIntType(type: Class): boolean {
        return type === Types.BOXED_INT || type === Class.PRIMITIVE_INT
    }

    /**
     * Checks if the given Class object represents a long type (including boxed and primitive types).
     *
     * @static
     * @param { Class } type The Class object to be checked.
     * @returns { boolean } Returns true if it's a long type; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isLongType(type: Class): boolean {
        return type === Types.BOXED_LONG || type === Class.PRIMITIVE_LONG
    }

    /**
     * Checks if the given Class object represents a byte type (including boxed and primitive types).
     *
     * @static
     * @param { Class } type The Class object to be checked.
     * @returns { boolean } Returns true if it's a byte type; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isByteType(type: Class): boolean {
        return type === Types.BOXED_BYTE || type === Class.PRIMITIVE_BYTE
    }

    /**
     * Checks if the given Class object represents a short type (including boxed and primitive types).
     *
     * @static
     * @param { Class } type The Class object to be checked.
     * @returns { boolean } Returns true if it's a short type; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isShortType(type: Class): boolean {
        return type === Types.BOXED_SHORT || type === Class.PRIMITIVE_SHORT
    }

    /**
     * Checks if the given Class object represents a float type (including boxed and primitive types).
     *
     * @static
     * @param { Class } type The Class object to be checked.
     * @returns { boolean } Returns true if it's a float type; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isFloatType(type: Class): boolean {
        return type === Types.BOXED_FLOAT || type === Class.PRIMITIVE_FLOAT
    }

    /**
     * Checks if the given Class object represents a char type (including boxed and primitive types).
     *
     * @static
     * @param { Class } type The Class object to be checked.
     * @returns { boolean } Returns true if it's a char type; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isCharType(type: Class): boolean {
        return type === Types.BOXED_CHAR || type === Class.PRIMITIVE_CHAR
    }

    /**
     * Checks if the given Class object represents a primitive type (numeric or boolean types).
     *
     * @static
     * @param { Class } type The Class object to be checked.
     * @returns { boolean } Returns true if it's a primitive type; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isPrimitiveType(type: Class): boolean {
        return Types.isNumericType(type) || Types.isBooleanType(type)
    }

    /**
     * Checks if the given Class object represents a numeric type (including double, int, long, byte, short, float).
     *
     * @static
     * @param { Class } type The Class object to be checked.
     * @returns { boolean } Returns true if it's a numeric type; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static isNumericType(type: Class): boolean {
        return Types.isDoubleType(type)
            || Types.isIntType(type)
            || Types.isLongType(type)
            || Types.isByteType(type)
            || Types.isShortType(type)
            || Types.isFloatType(type)
    }
}

/**
 * Native function for creating a fixed array with a specified element type and length.
 *
 * @param { Class } component The element type of the fixed array.
 * @param { int } length The length of the fixed array.
 * @returns { Object } The created fixed array object.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function createFixedArray(component: Class, length: int): Object

} // namespace reflect.internals
