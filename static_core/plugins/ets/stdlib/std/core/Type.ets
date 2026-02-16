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
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * @file Defines the Type for ArkTS
 * @kit ArkTS
 */

package std.core;

export type TypeDescriptor = string

// Internal platform-dependent descriptor, part of implementation
type RuntimeTypeDescriptor = string

class TypeAPI {
    /**
     * type TypeDescriptor = string Internal platform-dependent descriptor,
     *     part of implementation type RuntimeTypeDescriptor = string class TypeAPI { Type intrinsics
     *
     * @param { RuntimeTypeDescriptor } td Type descriptor string.
     * @param { RuntimeLinker } contextLinker Runtime linker context.
     * @returns { byte }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    native static getTypeKind(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker): byte

    /**
     * isValueType
     *
     * @param { RuntimeTypeDescriptor } td Type descriptor.
     * @param { RuntimeLinker } contextLinker Runtime linker context.
     * @returns { boolean }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    native static isValueType(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker): boolean

    /**
     * getNullTypeDescriptor
     *
     * @returns { RuntimeTypeDescriptor }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    native static getNullTypeDescriptor(): RuntimeTypeDescriptor

    /**
     * getUndefinedTypeDescriptor
     *
     * @returns { RuntimeTypeDescriptor }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    native static getUndefinedTypeDescriptor(): RuntimeTypeDescriptor

    /**
     * @param { RuntimeTypeDescriptor } td The runtime type descriptor to resolve.
     * @param { RuntimeLinker } contextLinker The runtime linker context for type resolution.
     * @returns { Class | undefined }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
    */
    native static getClass(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker): Class | undefined
    /**
     * getClassAttributes
     *
     * @param { Class } cls The class to inspect.
     * @returns { int }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    native static getClassAttributes(cls: Class): int

    native static getFieldsNum(cls: Class): long

    native static getOwnFieldsNum(cls: Class): long

    native static getFieldPtr(cls: Class, i: long): Long | undefined

    native static getOwnFieldPtr(cls: Class, i: long): Long | undefined

    native static getFieldPtrByName(cls: Class, name: string): Long | undefined

    native static getFieldDescriptor(fieldPtr: long): RuntimeTypeDescriptor

    native static getFieldOwner(fieldPtr: long): Class

    native static getField(fieldPtr: long, fieldType: Type, ownerType: Type): Field

    native static getMethodsNum(cls: Class): long

    native static getMethodDescriptor(cls: Class, i: long): RuntimeTypeDescriptor | undefined

    native static getMethod(methodType: MethodType, ownerType: Type): Method

    native static getConstructorsNum(cls: Class): long

    native static getConstructorDescriptor(cls: Class, i: long): RuntimeTypeDescriptor | undefined

    native static getInterfacesNum(cls: Class): long

    native static getInterface(cls: Class, i: long): Class | undefined

    native static isInheritedFrom(lhs: Class, rhs: Class): boolean

    /*
        Function (Lambda or Method) type intrinsics
    */
    native static getFunctionAttributes(td: RuntimeTypeDescriptor): int

    /*
        Array type intrinsics
    */
    native static getArrayElementType(cls: Class): RuntimeTypeDescriptor

    native static makeArrayInstance(elemTD: RuntimeTypeDescriptor, contextLinker: RuntimeLinker, len: long): Object

    /*
        General helpers
    */
    static getClassDescriptor(cls: Class): RuntimeTypeDescriptor {
        return cls.getDescriptor()
    }

    private static NullTypeDescriptor: RuntimeTypeDescriptor = TypeAPI.getNullTypeDescriptor()
    private static UndefinedTypeDescriptor: RuntimeTypeDescriptor = TypeAPI.getUndefinedTypeDescriptor()

    static getTypeDescriptor(o: Any): RuntimeTypeDescriptor {
        if (o === null) {
            return TypeAPI.NullTypeDescriptor
        }
        return (o === undefined) ? TypeAPI.UndefinedTypeDescriptor : TypeAPI.getClassDescriptor(Class.of(o as object))
    }

    static getTypeName(cls: Class): string {
        return cls.getName()
    }

    static convertTypeDescriptor(td: TypeDescriptor): RuntimeTypeDescriptor {
        let sb = new StringBuilder("L")
        sb.append(td.replaceChar(c'.', c'/'))
        sb.append(";")
        return sb.toString()
    }

    native static getDeclaringClassImpl(type: LambdaType | MethodType): Class | undefined
}

// NOTE(shumilov-petr): replace to enum, enum not available now
export class TypeKind {
    public static readonly NONE: byte       = 0x0
    public static readonly VOID: byte       = 0x1

    public static readonly CHAR: byte       = 0x2
    public static readonly BOOLEAN: byte    = 0x3
    public static readonly BYTE: byte       = 0x4
    public static readonly SHORT: byte      = 0x5
    public static readonly INT: byte        = 0x6
    public static readonly LONG: byte       = 0x7
    public static readonly FLOAT: byte      = 0x8
    public static readonly DOUBLE: byte     = 0x9

    public static readonly CLASS: byte      = 0xA
    public static readonly STRING: byte     = 0xB
    public static readonly INTERFACE: byte  = 0xC
    public static readonly ARRAY: byte      = 0xD
    public static readonly TUPLE: byte      = 0xE
    public static readonly LAMBDA: byte     = 0xF
    public static readonly METHOD: byte     = 0x10

    public static readonly UNION: byte      = 0x11
    public static readonly UNDEFINED: byte  = 0x12
    public static readonly NULL: byte       = 0x13

    public static readonly ENUM: byte       = 0x14

    private constructor() {}
}

// 5 lower bits stores kind's id
const TypeKindMask: byte;

// NOTE(shumilov-petr): replace to enum, enum not available now
class ValueTypeDesc {
    public static readonly BOOLEAN: RuntimeTypeDescriptor   = "Z"
    public static readonly BYTE: RuntimeTypeDescriptor      = "B"
    public static readonly SHORT: RuntimeTypeDescriptor     = "S"
    public static readonly CHAR: RuntimeTypeDescriptor      = "C"
    public static readonly INT: RuntimeTypeDescriptor       = "I"
    public static readonly LONG: RuntimeTypeDescriptor      = "J"
    public static readonly FLOAT: RuntimeTypeDescriptor     = "F"
    public static readonly DOUBLE: RuntimeTypeDescriptor    = "D"
    public static readonly VOID: RuntimeTypeDescriptor      = "V"

    private constructor() {}
}

// initialized in _initializerBlock_.ets
export const ObjectType: ClassType;

export class Attributes {
    public static readonly STATIC: int      = 1 << 0  // Field, Method
    public static readonly READONLY: int    = 1 << 1  // Field
    public static readonly FINAL: int       = 1 << 2  // Method, Class
    public static readonly ABSTRACT: int    = 1 << 3  // Method
    public static readonly CONSTRUCTOR: int = 1 << 4  // Method
    public static readonly REST: int        = 1 << 5  // TypeAPIParameter
    public static readonly OPTIONAL: int    = 1 << 6  // TypeAPIParameter
    public static readonly NATIVE: int      = 1 << 7  // Method, Lambda
    public static readonly ASYNC: int       = 1 << 8  // Method, Lambda
    public static readonly GETTER: int      = 1 << 9  // Method
    public static readonly SETTER: int      = 1 << 10 // Method

    private constructor() {}
}

export class AccessModifier {
    public static readonly PUBLIC: byte       = 0
    public static readonly PRIVATE: byte      = 1
    public static readonly PROTECTED: byte    = 2

    private constructor() {}
}

/**
 * Runtime ArkTS type representation
 */
export abstract class Type extends Object {
    private td: RuntimeTypeDescriptor
    private contextLinker: RuntimeLinker

    /**
     * Resolves type by descriptor
     *
     * @param {TypeDescriptor} td type descriptor
     * @returns {Type | undefined} instance of appropriate type or undefined if resolving is failed
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static resolve(td: TypeDescriptor): Type | undefined {
        const callerClass = Class.ofCaller()
        const contextLinker = callerClass == undefined ? getBootRuntimeLinker() : callerClass.getLinker()
        const internalTd: RuntimeTypeDescriptor = TypeAPI.convertTypeDescriptor(td)
        return Type.resolve(internalTd, contextLinker)
    }

    /**
     * Resolves type by internal descriptor, part of implementation
     *
     * @param {RuntimeTypeDescriptor} td internal runtime type descriptor
     * @param {RuntimeLinker} contextLinker in which classes must be searched
     * @returns {Type | undefined} instance of appropriate type or undefined if resolving is failed
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static resolve(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker): Type | undefined {
        let kind = (TypeAPI.getTypeKind(td, contextLinker) & TypeKindMask).toByte()
        switch (kind) {
            case TypeKind.NONE:
                return undefined
            case TypeKind.VOID:
                return td == ValueTypeDesc.VOID ? VoidType.VAL : VoidType.REF
            case TypeKind.CHAR:
                return TypeAPI.isValueType(td, contextLinker) ? CharType.VAL : CharType.REF
            case TypeKind.BOOLEAN:
                return TypeAPI.isValueType(td, contextLinker) ? BooleanType.VAL : BooleanType.REF
            case TypeKind.BYTE:
                return TypeAPI.isValueType(td, contextLinker) ? ByteType.VAL : ByteType.REF
            case TypeKind.SHORT:
                return TypeAPI.isValueType(td, contextLinker) ? ShortType.VAL : ShortType.REF
            case TypeKind.INT:
                return TypeAPI.isValueType(td, contextLinker) ? IntType.VAL : IntType.REF
            case TypeKind.LONG:
                return TypeAPI.isValueType(td, contextLinker) ? LongType.VAL : LongType.REF
            case TypeKind.FLOAT:
                return TypeAPI.isValueType(td, contextLinker) ? FloatType.VAL : FloatType.REF
            case TypeKind.DOUBLE:
                return TypeAPI.isValueType(td, contextLinker) ? DoubleType.VAL : DoubleType.REF

            case TypeKind.CLASS:
                return new ClassType(td, contextLinker)
            case TypeKind.STRING:
                return StringType.REF
            case TypeKind.INTERFACE:
                return new InterfaceType(td, contextLinker)
            case TypeKind.ARRAY:
                return ArrayType.getInstance(td, contextLinker)
            case TypeKind.TUPLE:
                return new TupleType(td, contextLinker)
            case TypeKind.LAMBDA:
                return new LambdaType(td, contextLinker)
            case TypeKind.METHOD:
                return new MethodType(td, contextLinker)
            case TypeKind.UNION:
                return new UnionType(td, contextLinker)
            case TypeKind.UNDEFINED:
                return UndefinedType.REF
            case TypeKind.NULL:
                return NullType.REF

            case TypeKind.ENUM:
                return new EnumType(td, contextLinker)
            default:
                throw new TypeError("Unexpected TypeKind " + kind)
        }
    }

    /**
     * from
     *
     * @returns {Type}
     * @throws { TypeError } - Input parameter error.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static from<T>(): Type {
        //should never be executed since FE lowering this
        throw new TypeError("Type.from call will be casted by frontend")
    }

    /**
     * Returns Type of value
     *
     * @param {boolean} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: boolean): Type {
        return BooleanType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param {byte} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: byte): Type {
        return ByteType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param {char} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: char): Type {
        return CharType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param {short} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: short): Type {
        return ShortType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param {int} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: int): Type {
        return IntType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param {long} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: long): Type {
        return LongType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param {float} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: float): Type {
        return FloatType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param {double} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: double): Type {
        return DoubleType.VAL
    }

    // -----

    /**
     * Returns Type of value
     *
     * @param {string} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: string): Type {
        return StringType.REF
    }

    /**
     * Returns Type of value
     *
     * @param {FixedArray<boolean>} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: FixedArray<boolean>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.BOOLEAN)
    }

    /**
     * Returns Type of value
     *
     * @param {FixedArray<byte>} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: FixedArray<byte>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.BYTE)
    }

    /**
     * Returns Type of value
     *
     * @param {FixedArray<char>} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: FixedArray<char>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.CHAR)
    }

    /**
     * Returns Type of value
     *
     * @param {FixedArray<short>} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: FixedArray<short>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.SHORT)
    }

    /**
     * Returns Type of value
     *
     * @param {FixedArray<int>} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: FixedArray<int>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.INT)
    }

    /**
     * Returns Type of value
     *
     * @param {FixedArray<long>} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: FixedArray<long>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.LONG)
    }

    /**
     * Returns Type of value
     *
     * @param {FixedArray<float>} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: FixedArray<float>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.FLOAT)
    }

    /**
     * Returns Type of value
     *
     * @param {FixedArray<double>} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: FixedArray<double>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.DOUBLE)
    }

    // -----

    /**
     * Returns Type of value
     *
     * @param {FixedArray<Object>} v value
     * @returns {Type} Type instance of this value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(v: FixedArray<Object>): Type {
        return ArrayType.getInstance(Class.of(v))
    }

    // -----

    /**
     * of
     *
     * @param {Any} o value
     * @returns {Type}
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static of(o: Any): Type {
        const td = TypeAPI.getTypeDescriptor(o)
        const contextClass = o == undefined ? Class.ofCaller() : Class.of(o as object)
        const contextLinker = contextClass == undefined ? getBootRuntimeLinker() : contextClass.getLinker()
        return Type.resolve(td, contextLinker)!
    }

    // -----

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean}
     * @abstract
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract isPrimitive(): boolean // Or Composite

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean}
     * @abstract
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract isReference(): boolean // Or Value

    /**
     * Checks whether type has name
     *
     * @returns {boolean}
     * @abstract
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract hasName(): boolean

    /**
     * constructor
     *
     * @param {RuntimeTypeDescriptor} td type descriptor
     * @param {RuntimeLinker} contextLinker runtime linker context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    protected constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        this.td = td
        this.contextLinker = contextLinker
    }

    /**
     * getTypeDesc
     *
     * @returns {RuntimeTypeDescriptor}
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getTypeDesc(): RuntimeTypeDescriptor {
        return this.td
    }

    /**
     * getContextLinker
     *
     * @returns {RuntimeLinker}
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    protected getContextLinker(): RuntimeLinker {
        return this.contextLinker
    }

    /**
     * getClass
     *
     * @returns {Class | undefined}
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getClass(): Class | undefined {
        return TypeAPI.getClass(this.td, this.contextLinker)
    }

    // T.subTypeOf(U) means that `T extends U`
    /**
     * subTypeOf
     *
     * @param {Type} other type to check against
     * @returns {boolean}
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    protected subTypeOf(other: Type): boolean {
        if (this.equals(other)) {
            return true
        } else if (other.equals(ObjectType)) {
            let isNullish = (this) instanceof UndefinedType || (this) instanceof NullType
            return this.isReference() && !isNullish
        }
        return false
    }

    // T.assignableFrom(U) means that `T <- U`
    /**
     * assignableFrom
     *
     * @param {Type} other Type to check.
     * @returns {boolean}
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public assignableFrom(other: Type): boolean {
        if (other.subTypeOf(this)) {
            return true
        }
        if (this.isNumericType() && other.isNumericType()) {
            return true
        }
        if (this instanceof BooleanType && other instanceof BooleanType) {
            return true
        }
        if (this instanceof CharType && other instanceof CharType) {
            return true
        }
        return false
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @abstract
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract convertObject(obj: Any): Any;

    private isNumericType(): boolean {
        return (this) instanceof ByteType
            || (this) instanceof ShortType
            || (this) instanceof IntType
            || (this) instanceof LongType
            || (this) instanceof FloatType
            || (this) instanceof DoubleType
    }

    /**
     * getName
     *
     * @returns {string}
     * @abstract
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract getName(): string

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @abstract
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract getLiteral(): string

    /**
     * toString
     *
     * @returns {string}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toString(): string {
        if (this.hasName()) {
            return this.getName()
        }
        return this.getLiteral()
    }

    /**
     * equals
     *
     * @param {Type} other type to be checked against
     * @returns {boolean}
     * @abstract
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    abstract equals(other: Type): boolean

    /**
     * toRefType
     *
     * @param {Type} primitiveType primitive type
     * @returns {Type}
     * @throws {Error} - Input parameter error.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static toRefType(primitiveType: Type): Type {
        if (!primitiveType.isPrimitive()) {
            throw new Error(`type ${primitiveType.getName()} is not primitive type`)
        }

        if (primitiveType.isReference()) {
            // StringType is primitive and reference at the same time
            return primitiveType
        }

        if (primitiveType instanceof BooleanType) {
            return BooleanType.REF
        } else if (primitiveType instanceof ByteType) {
            return ByteType.REF
        } else if (primitiveType instanceof CharType) {
            return CharType.REF
        } else if (primitiveType instanceof ShortType) {
            return ShortType.REF
        } else if (primitiveType instanceof IntType) {
            return IntType.REF
        } else if (primitiveType instanceof LongType) {
            return LongType.REF
        } else if (primitiveType instanceof FloatType) {
            return FloatType.REF
        } else if (primitiveType instanceof DoubleType) {
            return DoubleType.REF
        } else if (primitiveType instanceof VoidType) {
            return VoidType.REF
        } else {
            throw new Error(`failed to convert primitive type ${primitiveType.getName()} to relevant reference type`)
        }
    }
}

/**
 * Represents null type
 * @extends Type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class NullType extends Type {
    /**
     * Null type reference
     *
     * @type {NullType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF: NullType = new NullType()

    /**
     * constructor
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor() {
        super(TypeAPI.getTypeDescriptor(null), getBootRuntimeLinker())
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type
     *
     * @throws {Error} error in case of absence of name
     * @returns {string} type name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getName(): string {
        return "null"
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        return "null"
    }

    /**
     * Checks for equality this instance with provided object, treated as a DoubleType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has NullType
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof NullType
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        if (obj != null) {
            throw new Error("invalid conversion")
        }
        return null
    }
}

/**
 * Represents undefined type
 * @extends Type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class UndefinedType extends Type {
    /**
     * Undefined type reference
     *
     * @type {UndefinedType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF: UndefinedType = new UndefinedType()

    /**
     * constructor
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor() {
        super(TypeAPI.getTypeDescriptor(undefined), getBootRuntimeLinker())
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type if exists and empty string otherwise
     *
     * @returns {string} type name
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getName(): string {
        return "undefined"
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        return "undefined"
    }

    /**
     * Checks for equality this instance with provided object, treated as a UndefinedType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has UndefinedType
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof UndefinedType
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        throw new Error("todo(kprokopenko): add when undefined becomes available")
    }
}

/**
 * Represents void type
 * @extends Type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class VoidType extends Type {
    /**
     * Void type value
     *
     * @type {VoidType} VAL
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly VAL: VoidType = new VoidType(ValueTypeDesc.VOID, true)
    /**
     * Void type reference
     *
     * @type {VoidType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF: VoidType = new VoidType(TypeAPI.getTypeDescriptor(Void.void_instance), false)

    private readonly value: boolean

    /**
     * Private constructor for VoidType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {boolean} value Whether this is a value type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor(td: RuntimeTypeDescriptor, value: boolean) {
        super(td, getBootRuntimeLinker())
        this.value = value
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return !this.value
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type if exists and empty string otherwise
     *
     * @returns {string} type name
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getName(): string {
        return "void"
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        return "void"
    }

    /**
     * Checks for equality this instance with provided object, treated as a VoidType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has VoidType
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof VoidType
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        if (!this.assignableFrom(Type.of(obj))) {
            throw new Error("invalid conversion")
        }
        return Void.void_instance
    }
}

/**
 * Represents char type
 * @extends Type
 * @note Boxed Char and primitive char both have CharType
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */

export final class CharType extends Type {
    /**
     * Char type value
     *
     * @type {CharType} VAL
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly VAL: CharType = new CharType(ValueTypeDesc.CHAR, true)
    /**
     * Char type reference
     *
     * @type {CharType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF: CharType = new CharType(TypeAPI.getTypeDescriptor(new Char()), false)
    private static readonly CLASS = Class.of(new Char())

    private isValue: boolean

    /**
     * Private constructor for CharType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {boolean} isValue Whether this is a value type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws {Error} error in case of absence of name
     * @returns {string} name of type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(CharType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "char"
        }
        return "Char"
    }

    /**
     * equals
     *
     * @param {Type} other type to be checked against
     * @returns {boolean}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof CharType
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (!this.equals(objType)) {
            throw new Error("invalid type for conversion")
        }
        return obj
    }
}

/**
 * Represents boolean type
 * @extends Type
 * @note Boxed Boolean and primitive boolean both have BooleanType
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class BooleanType extends Type {
    /**
     * Boolean type value
     *
     * @type {BooleanType} VAL
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly VAL: BooleanType = new BooleanType(ValueTypeDesc.BOOLEAN, true)
    /**
     * Boolean type reference
     *
     * @type {BooleanType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF: BooleanType = new BooleanType(TypeAPI.getTypeDescriptor(new Boolean()), false)
    private static readonly CLASS = Class.of(new Boolean())

    private isValue: boolean

    /**
     * Private constructor for BooleanType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {boolean} isValue Whether this is a value type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws {Error} error in case of absence of name
     * @returns {string} name of type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(BooleanType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "boolean"
        }
        return "Boolean"
    }

    /**
     * equals
     *
     * @param {Type} other type to be checked against
     * @returns {boolean}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof BooleanType
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (!this.equals(objType)) {
            throw new Error("invalid type for conversion")
        }
        return obj
    }
}

/**
 * Represents byte type
 * @extends Type
 * @note Boxed Byte and primitive byte both have ByteType
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class ByteType extends Type {
    /**
     * Byte type value
     *
     * @type {ByteType} VAL
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly VAL: ByteType = new ByteType(ValueTypeDesc.BYTE, true)
    /**
     * Byte type reference
     *
     * @type {ByteType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF: ByteType = new ByteType(TypeAPI.getTypeDescriptor(new Byte()), false)
    private static readonly CLASS = Class.of(new Byte())

    private isValue: boolean

    /**
     * Private constructor for ByteType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {boolean} isValue Whether this is a value type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws {Error} error in case of absence of name
     * @returns {string} name of type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(ByteType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "byte"
        }
        return "Byte"
    }

    /**
     * equals
     *
     * @param {Type} other type to be checked against
     * @returns {boolean}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof ByteType
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toByte()
    }
}

/**
 * Represents short type
 * @extends Type
 * @note Boxed Short and primitive short both have ShortType
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class ShortType extends Type {
    /**
     * Short type value
     *
     * @type {ShortType} VAL
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly VAL: ShortType = new ShortType(ValueTypeDesc.SHORT, true)
    /**
     * Short type reference
     *
     * @type {ShortType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF: ShortType = new ShortType(TypeAPI.getTypeDescriptor(new Short()), false)
    private static readonly CLASS = Class.of(new Short())

    private isValue: boolean

    /**
     * Private constructor for ShortType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {boolean} isValue Whether this is a value type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws {Error} error in case of absence of name
     * @returns {string} name of type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(ShortType.CLASS)
    }

    /**
     * Returns literal of type
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "short"
        }
        return "Short"
    }

    /**
     * Checks for equality this instance with provided object, treated as a ShortType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has ShortType and
     *     this type and other type both are reference or value
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof ShortType
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toShort()
    }
}

/**
 * Represents int type
 * @extends Type
 * @note Boxed Int and primitive int both have IntType
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class IntType extends Type {
    /**
     * Int type value
     *
     * @type {IntType} VAL
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly VAL: IntType = new IntType(ValueTypeDesc.INT, true)
    /**
     * Int type reference
     *
     * @type {IntType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF: IntType = new IntType(TypeAPI.getTypeDescriptor(new Int()), false)
    private static readonly CLASS = Class.of(new Int())

    private isValue: boolean

    /**
     * Private constructor for IntType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {boolean} isValue Whether this is a value type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type if exists and empty string otherwise
     *
     * @throws {Error} error in case of absence of name
     * @returns {string} type name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(IntType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "int"
        }
        return "Int"
    }

    /**
     * Checks for equality this instance with provided object, treated as a IntType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has IntType and
     *     this type and other type both are reference or value
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof IntType
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toInt()
    }
}

/**
 * Represents long type
 * @extends Type
 * @note Boxed Long and primitive long both have LongType
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class LongType extends Type {
    /**
     * Long type value
     *
     * @type {LongType} VAL
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly VAL = new LongType(ValueTypeDesc.LONG, true)
    /**
     * Long type reference
     *
     * @type {LongType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF = new LongType(TypeAPI.getTypeDescriptor(new Long()), false)
    private static readonly CLASS = Class.of(new Long())

    private isValue: boolean

    /**
     * Private constructor for LongType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {boolean} isValue Whether this is a value type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type if exists and empty string otherwise
     *
     * @throws {Error} error in case of absence of name
     * @returns {string} type name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(LongType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "long"
        }
        return "Long"
    }

    /**
     * Checks for equality this instance with provided object, treated as a LongType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has LongType and
     *     this type and other type both are reference or value
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof LongType
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toLong()
    }
}

/**
 * Represents float type
 * @extends Type
 * @note Boxed Float and primitive float both have FloatType
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class FloatType extends Type {
    /**
     * Float type value
     *
     * @type {FloatType} VAL
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly VAL: FloatType = new FloatType(ValueTypeDesc.FLOAT, true)
    /**
     * Float type reference
     *
     * @type {FloatType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF: FloatType = new FloatType(TypeAPI.getTypeDescriptor(new Float()), false)
    private static readonly CLASS = Class.of(new Float())

    private isValue: boolean

    /**
     * Private constructor for FloatType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {boolean} isValue Whether this is a value type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws {Error} error in case of absence of name
     * @returns {string} name of type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(FloatType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "float"
        }
        return "Float"
    }

    /**
     * Checks for equality this instance with provided object, treated as a FloatType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has FloatType and
     *     this type and other type both are reference or value
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof FloatType
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toFloat()
    }
}

/**
 * Represents double type
 * @extends Type
 * @note Boxed Double and primitive double both have DoubleType
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class DoubleType extends Type {
    /**
     * Double type value
     *
     * @type {DoubleType} VAL
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly VAL: DoubleType = new DoubleType(ValueTypeDesc.DOUBLE, true)
    /**
     * Double type reference
     *
     * @type {DoubleType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF: DoubleType = new DoubleType(TypeAPI.getTypeDescriptor(new Double()), false)
    private static readonly CLASS = Class.of(new Double())

    private isValue: boolean

    /**
     * Private constructor for DoubleType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {boolean} isValue Whether this is a value type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws {Error} error in case of absence of name
     * @returns {string} name of type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(DoubleType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "double"
        }
        return "Double"
    }

    /**
     * Checks for equality this instance with provided object, treated as a DoubleType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has DoubleType and
     *     this type and other type both are reference or value
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof DoubleType
    }

    /**
     * convertObject
     *
     * @param {Any} obj Object to convert.
     * @returns {Any}
     * @throws {Error} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toDouble()
    }
}

/**
 * Represents type of classes
 * @extends Type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class ClassType extends Type {
    private readonly cls: Class
    private readonly attrs: int

    /**
     * constructor
     *
     * @param {RuntimeTypeDescriptor} td type descriptor
     * @param {RuntimeLinker} contextLinker runtime linker context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
        this.cls = TypeAPI.getClass(td, contextLinker)!
        this.attrs = TypeAPI.getClassAttributes(this.cls)
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type
     *
     * @returns {string} type name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getName(): string {
        return TypeAPI.getTypeName(this.cls)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        return "class{...}"
    }

    /**
     * Checks for equality this instance with provided object, treated as a ClassType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has ClassType and their names are the same
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        // Must compare classes, as descriptors might be equal between classes from different contexts
        return (other instanceof ClassType) && (other as ClassType).cls == this.cls
    }

    /**
     * Returns base type of this class
     * If this type is the type of Object class then returns this
     *
     * @returns {ClassType} base type of class
     * @throws {TypeError} - Input parameter error.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getBaseType(): ClassType {
        const base = this.cls.getSuper()
        return (base == null) ? this : Type.resolve(base.getDescriptor(), base.getLinker())! as ClassType
    }

    /**
     * Returns number of direct superinterfaces of this class
     *
     * @returns {long} number of interfaces that was implemented by this class directly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getInterfacesNum(): long {
        return TypeAPI.getInterfacesNum(this.cls)
    }

    /**
     * Returns ith direct superinterface of this class
     *
     * @param {long} i index
     * @throws {TypeError} when i greater then num of interfaces
     * @returns {InterfaceType} type of ith superinterface
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getInterface(i: long): InterfaceType {
        const iface = TypeAPI.getInterface(this.cls, i)
        if (iface) {
            return Type.resolve(iface.getDescriptor(), iface.getLinker()) as InterfaceType
        }
        throw new TypeError("no interface at " + i)
    }

    /**
     * Returns number of all fields
     * including static, instance and also fields of all its superclasses
     *
     * @returns {long} number of fields
     * @example
     *
     * ```
     * class A {
     *     a : int
     * }
     *
     * class B extends A {
     *     b : int
     * }
     * ```
     * let bType class type of B, then `bType.getFieldsNum()` returns 2
     * Note that Object class also is super class of B
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getFieldsNum(): long {
        return TypeAPI.getFieldsNum(this.cls)
    }

    /**
     * Returns count of fields declared directly in this class.
     *
     * @returns {long} Number of own fields.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getOwnFieldsNum(): long {
        return TypeAPI.getOwnFieldsNum(this.cls)
    }

    /**
     * Returns ith Field of this class
     *
     * @param {long} i index (using flat semantic)
     * @throws {TypeError} when i greater then number of fields
     * @returns {Field} ith Field
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getField(i: long): Field {
        const fieldPtr = TypeAPI.getFieldPtr(this.cls, i)
        if (fieldPtr) {
            return this.getFieldImpl(fieldPtr)
        }
        throw new TypeError("no field at " + i)
    }

    /**
     * Returns ith Field owned by this class
     *
     * @param {long} i index (using flat semantic)
     * @throws {TypeError} when i greater then number of fields
     * @returns {Field} ith Field
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getOwnField(i: long): Field {
        const fieldPtr = TypeAPI.getOwnFieldPtr(this.cls, i)
        if (fieldPtr) {
            return this.getFieldImpl(fieldPtr)
        }
        throw new TypeError("no own field at " + i)
    }

    /**
     * Find Field by name
     *
     * @param {string} name name of field
     * @throws {TypeError} when class doesn't have field with this name
     * @returns {Field} Field instance with this name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getFieldByName(name: string): Field {
        const fieldPtr = TypeAPI.getFieldPtrByName(this.cls, name)
        if (fieldPtr) {
            return this.getFieldImpl(fieldPtr)
        }
        throw new TypeError("no field \"" + name + "\"")
    }

    /**
     * Finds instance method by name.
     *
     * @param {string} name Method name.
     * @returns {Method} Matching instance method.
     * @throws {TypeError} - If method not found.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getMethodByName(name: string): Method{
        for (let methodIdx = 0; methodIdx < this.getMethodsNum(); ++methodIdx) {
            const method = this.getMethod(methodIdx)
            if (method.getName() == name && !method.isStatic()) {
                return method
            }
        }
        throw new TypeError("no instance method \"" + name + "\"")
    }

    private getFieldImpl(fieldPtr: long): Field {
        const fieldDesc = TypeAPI.getFieldDescriptor(fieldPtr)
        const fieldType = Type.resolve(fieldDesc, this.getContextLinker())!
        const ownerClass = TypeAPI.getFieldOwner(fieldPtr)
        const ownerType = Type.resolve(ownerClass.getDescriptor(), ownerClass.getLinker())!
        return TypeAPI.getField(fieldPtr, fieldType, ownerType)
    }

    /**
     * Returns number of methods of this class
     * including static methods and methods of super classes
     * @example
     *
     * ```
     * class A {
     *     a(): void {}
     * }
     *
     * class B extends A {
     *     b(): void {}
     * }
     * ```
     * let bType class type of B, then `bType.getMethodsNum()` returns at least 2
     * Note that Object class also super class of B
     * @returns {long} number of methods
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getMethodsNum(): long {
        return TypeAPI.getMethodsNum(this.cls)
    }

    /**
     * Returns ith Method of this class
     *
     * @param {long} i index (using flat semantic)
     * @throws {TypeError} when i greater then number of methods
     * @returns {Method} ith method
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getMethod(i: long): Method {
        const td = TypeAPI.getMethodDescriptor(this.cls, i)
        if (td) {
            const methodType = Type.resolve(td, this.getContextLinker())! as MethodType
            const ownerClass = TypeAPI.getDeclaringClassImpl(methodType)!
            const ownerType = Type.resolve(ownerClass.getDescriptor(), ownerClass.getLinker())!
            return TypeAPI.getMethod(methodType, ownerType)
        }
        throw new TypeError("no method at " + i)
    }

    /**
     * Returns number of constructors of this class
     * Note that constructors of super class isn't considered as constructors of this class
     *
     * @returns {long} number of constructors
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getConstructorsNum(): long {
        return TypeAPI.getConstructorsNum(this.cls)
    }

    /**
     * Returns ith constructor of this class
     *
     * @param {long} i index
     * @throws {TypeError} then i greater then number of constructors
     * @returns {Method} instance representing ith constructor
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getConstructor(i: long): Method {
        const td = TypeAPI.getConstructorDescriptor(this.cls, i)
        if (td) {
            const methodType = Type.resolve(td, this.getContextLinker())! as MethodType
            const ownerClass = TypeAPI.getDeclaringClassImpl(methodType)!
            const ownerType = Type.resolve(ownerClass.getDescriptor(), ownerClass.getLinker())!
            return TypeAPI.getMethod(methodType, ownerType)
        }
        throw new TypeError("no constructor at " + i)
    }

    /**
     * Checks if this class has a parameterless (empty) constructor.
     * This method searches through all constructors of the class
     * and returns true if any constructor takes zero parameters.
     *
     * @returns {boolean} True if the class has a parameterless constructor, false otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public hasEmptyConstructor(): boolean {
        let num = this.getConstructorsNum()
        for (let i = 0; i < num; i++) {
            if (this.getConstructor(i).getType().getParametersNum() == 0) {
                return true
            }
        }
        return false
    }

    /**
     * Checks if class is final (cannot be subclassed).
     *
     * @returns {boolean} True if class is final.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public isFinal(): boolean {
        return (this.attrs & Attributes.FINAL) != 0
    }

    /**
     * Checks if this class has a field with the specified name.
     * This method searches for a field (including inherited fields)
     * with the given name and returns true if found.
     *
     * @param {string} name The name of the field to check.
     * @returns {boolean} True if the field exists in this class, false otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public hasField(name: string): boolean {
        const fieldPtr = TypeAPI.getFieldPtrByName(this.cls, name)
        return (fieldPtr !== undefined)
    }

    /**
     * Creates a new instance of this class using the default constructor.
     *
     * @returns {Object} New instance of this class.
     * @throws {Error} If class doesn't have a default constructor.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public make(): Object {
        return this.cls.createInstance()
    }

    // todo(kprokopenko): make varargs
    /**
     * Creates a new instance of this class using constructor matching provided arguments.
     *
     * @param {FixedArray<Any>} args Constructor arguments.
     * @returns {Object} New instance of this class.
     * @throws {Error} If no matching constructor found or ambiguous.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public make(args: FixedArray<Any>): Object {
        const argTypes = new Array<Type>(args.length)
        for (let i = 0; i < args.length; i++) {
            argTypes[i] = Type.of(args[i])
        }
        // collect all applicable constructors
        const ctors = new Array<Method>()
        const ctorNum = this.getConstructorsNum()
        for (let i = 0; i < ctorNum; i++) {
            const c = this.getConstructor(i)
            const ct = c.getType()
            if (ct.getParametersNum() == args.length) {
                let ok = true
                for (let arg = 0; arg < args.length; arg++) {
                    if (!ct.getParameter(arg).getType().assignableFrom(argTypes[arg])) {
                        ok = false
                        break
                    }
                }
                if (ok) {
                    ctors.push(c)
                }
            }
        }
        // inspect if constructor has a more specific one (O(n^2))
        for (let inspect = 0; inspect < ctors.length; inspect++) {
            const toRem = ctors.at(inspect)!.getType()
            let rem = false
            for (let c = 0; c < ctors.length; c++) {
                if (c == inspect) {
                    continue
                }
                rem = true
                const cur = ctors.at(c)!.getType()
                for (let a = 0; a < args.length; a++) {
                    if (!toRem.getParameter(a).getType().assignableFrom(cur.getParameter(a).getType())) {
                        rem = false;
                        break;
                    }
                }
                if (rem) {
                    break
                }
            }
            if (rem) {
                ctors.splice(inspect, 1)
                inspect--
            }
        }
        if (ctors.length != 1) {
            throw new Error("can't select consturctor: " + ctors.length + " left")
        }
        return ctors.at(0)!.invoke(undefined, args) as Object
    }

    // class.subTypeOf(U) means that `class extends U` or `class implements U`
    /**
     * Checks if this type is a subtype of another type.
     *
     * @param {Type} other Type to check against.
     * @returns {boolean} True if this type is a subtype of other.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    protected override subTypeOf(other: Type): boolean {
        if (super.subTypeOf(other)) {
            return true
        }
        // `this` is derived of `other` class?
        if (other instanceof ClassType) {
            let bt = this;
            while (!bt.equals(bt.getBaseType())) {
                if (bt.equals(other)) {
                    return true
                }
                bt = bt.getBaseType()
            }
            return false
        }
        // `this` implements `other` interface?
        if (other instanceof InterfaceType) {
            let iface = other as InterfaceType
            let bt = this
            while (!bt.equals(bt.getBaseType())) {
                let ifaceNum = bt.getInterfacesNum()
                for (let i = 0; i < ifaceNum; i++) {
                    if (bt.getInterface(i).hasSuperInterface(iface)) {
                        return true
                    }
                }
                bt = bt.getBaseType()
            }
        }
        return false
    }

    /**
     * Converts object to this type.
     *
     * @param {Any} obj Object to convert.
     * @returns {Any} Converted object.
     * @throws {Error} If conversion is invalid.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
    }
}

/**
 * Represents interface type
 * @extends Type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class InterfaceType extends Type {
    private cls: Class

    /**
     * Constructor for InterfaceType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {RuntimeLinker} contextLinker Runtime linker context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
        this.cls = TypeAPI.getClass(td, contextLinker)!
    }

    /**
     * @returns {Class} Class of interface
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getClass(): Class {
        return this.cls
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type
     *
     * @returns {string} name of type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getName(): string {
        return TypeAPI.getTypeName(this.cls)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        return "interface{...}"
    }

    /**
     * Checks for equality this instance with provided object, treated as a InterfaceType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has InterfaceType and
     * their names are the same
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        // Must compare classes, as descriptors might be equal between classes from different contexts
        return other instanceof InterfaceType && this.cls == (other as InterfaceType).cls
    }

    // `this` extends of `other` interface?
    /**
     * Checks if this interface type is a subtype of another type.
     *
     * @param {Type} other Type to check against.
     * @returns {boolean} True if this type is a subtype of other.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    protected override subTypeOf(other: Type): boolean {
        if (super.subTypeOf(other)) {
            return true
        }
        if (other instanceof InterfaceType) {
            return this.hasSuperInterface(other as InterfaceType)
        }
        return false
    }

    /**
     * Returns number of interfaces of this interface
     *
     * @returns {long} Number of interfaces.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getInterfacesNum(): long {
        return TypeAPI.getInterfacesNum(this.cls)
    }

    /**
     * Returns ith direct superinterface of this interface
     *
     * @param {long} i index
     * @throws {TypeError} when i greater then num of interfaces
     * @returns {InterfaceType} type of ith superinterface
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getInterface(i: long): InterfaceType {
        const iface = TypeAPI.getInterface(this.cls, i)
        if (iface) {
            return Type.resolve(iface.getDescriptor(), iface.getLinker()) as InterfaceType
        }
        throw new TypeError("no interface at " + i)
    }

    /**
     * Returns number of methods of this interface
     * including static methods and methods of super interfaces
     * @example
     *
     * ```
     * class A {
     *     a(): void {}
     * }
     *
     * class B extends A {
     *     b(): void {}
     * }
     * ```
     * let bType interface type of B, then `bType.getMethodsNum()` returns at least 2
     * Note that Object class also super class of B
     * @returns {long} number of methods
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getMethodsNum(): long {
        return TypeAPI.getMethodsNum(this.cls)
    }

    /**
     * Returns ith Method of this interface
     *
     * @param {long} i index (using flat semantic)
     * @throws {TypeError} when i greater then number of methods
     * @returns {Method} ith method
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getMethod(i: long): Method {
        const td = TypeAPI.getMethodDescriptor(this.cls, i)
        if (td) {
            const methodType = Type.resolve(td, this.getContextLinker())! as MethodType
            const ownerClass = TypeAPI.getDeclaringClassImpl(methodType)!
            const ownerType = Type.resolve(ownerClass.getDescriptor(), ownerClass.getLinker())!
            return TypeAPI.getMethod(methodType, ownerType)
        }
        throw new TypeError("no method at " + i)
    }

    /**
     * Checks if this interface has the specified super interface
     *
     * @param {InterfaceType} expected Expected super interface
     * @returns {boolean} True if this interface has the specified super interface
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public hasSuperInterface(expected: InterfaceType): boolean {
        if (this.equals(expected)) {
            return true
        }
        return TypeAPI.isInheritedFrom(this.cls, expected.cls)
    }

    /**
     * Converts object to this interface type
     *
     * @param {Any} obj Object to convert
     * @returns {Any} Converted object
     * @throws {Error} If conversion is invalid
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
    }
}

/**
 * Represents array type
 * @extends Type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class ArrayType extends Type {
    private elemTD: RuntimeTypeDescriptor

    public static readonly BOOLEAN_VAL: ArrayType   = new ArrayType(Class.of(new boolean[0]), ValueTypeDesc.BOOLEAN)
    public static readonly BOOLEAN_REF: ArrayType   = new ArrayType(Class.of(new Boolean[0]), TypeAPI.getTypeDescriptor(new Boolean()))
    public static readonly CHAR_VAL: ArrayType      = new ArrayType(Class.of(new char[0]), ValueTypeDesc.CHAR)
    public static readonly CHAR_REF: ArrayType      = new ArrayType(Class.of(new Char[0]), TypeAPI.getTypeDescriptor(new Char()))
    public static readonly BYTE_VAL: ArrayType      = new ArrayType(Class.of(new byte[0]), ValueTypeDesc.BYTE)
    public static readonly BYTE_REF: ArrayType      = new ArrayType(Class.of(new Byte[0]), TypeAPI.getTypeDescriptor(new Byte()))
    public static readonly SHORT_VAL: ArrayType     = new ArrayType(Class.of(new short[0]), ValueTypeDesc.SHORT)
    public static readonly SHORT_REF: ArrayType     = new ArrayType(Class.of(new Short[0]), TypeAPI.getTypeDescriptor(new Short()))
    public static readonly INT_VAL: ArrayType       = new ArrayType(Class.of(new int[0]), ValueTypeDesc.INT)
    public static readonly INT_REF: ArrayType       = new ArrayType(Class.of(new Int[0]), TypeAPI.getTypeDescriptor(new Int()))
    public static readonly LONG_VAL: ArrayType      = new ArrayType(Class.of(new long[0]), ValueTypeDesc.LONG)
    public static readonly LONG_REF: ArrayType      = new ArrayType(Class.of(new Long[0]), TypeAPI.getTypeDescriptor(new Long()))
    public static readonly FLOAT_VAL: ArrayType     = new ArrayType(Class.of(new float[0]), ValueTypeDesc.FLOAT)
    public static readonly FLOAT_REF: ArrayType     = new ArrayType(Class.of(new Float[0]), TypeAPI.getTypeDescriptor(new Float()))
    public static readonly DOUBLE_VAL: ArrayType    = new ArrayType(Class.of(new double[0]), ValueTypeDesc.DOUBLE)
    public static readonly DOUBLE_REF: ArrayType    = new ArrayType(Class.of(new Double[0]), TypeAPI.getTypeDescriptor(new Double()))

    /**
     * Private constructor for ArrayType
     *
     * @param {Class} arrayClass Array class
     * @param {RuntimeTypeDescriptor} elemTD Element type descriptor
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    private constructor(arrayClass: Class, elemTD: RuntimeTypeDescriptor) {
        super(TypeAPI.getClassDescriptor(arrayClass), arrayClass.getLinker())
        this.elemTD = elemTD
    }

    /**
     * Checks if a type is assignable to this array type
     *
     * @param {Type} other Type to check
     * @returns {boolean} True if type is assignable
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override assignableFrom(other: Type): boolean {
        if (super.assignableFrom(other)) {
            return true
        }
        return other instanceof ArrayType && (other as ArrayType).getElementType().subTypeOf(this.getElementType())
    }

    /**
     * Gets ArrayType instance from type descriptor
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {RuntimeLinker} contextLinker Runtime linker context
     * @returns {ArrayType} ArrayType instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static getInstance(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker): ArrayType {
        const arrayClass = TypeAPI.getClass(td, contextLinker)!
        return ArrayType.getInstance(arrayClass)
    }

    /**
     * Gets ArrayType instance from Class and optional element type descriptor
     *
     * @param {Class} arrayClass Array class
     * @param {RuntimeTypeDescriptor} [elemTD] Optional element type descriptor
     * @returns {ArrayType} ArrayType instance
     * @throws { TypeError } Unexpected TypeKind
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static getInstance(arrayClass: Class, elemTD?: RuntimeTypeDescriptor): ArrayType {
        elemTD = elemTD ?? TypeAPI.getArrayElementType(arrayClass)
        const contextLinker = arrayClass.getLinker()
        let ek = (TypeAPI.getTypeKind(elemTD, contextLinker) & TypeKindMask).toByte()
        switch (ek) {
            case TypeKind.BOOLEAN:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.BOOLEAN_VAL : ArrayType.BOOLEAN_REF
            case TypeKind.CHAR:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.CHAR_VAL : ArrayType.CHAR_REF
            case TypeKind.BYTE:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.BYTE_VAL : ArrayType.BYTE_REF
            case TypeKind.SHORT:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.SHORT_VAL : ArrayType.SHORT_REF
            case TypeKind.INT:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.INT_VAL : ArrayType.INT_REF
            case TypeKind.LONG:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.LONG_VAL : ArrayType.LONG_REF
            case TypeKind.FLOAT:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.FLOAT_VAL : ArrayType.FLOAT_REF
            case TypeKind.DOUBLE:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.DOUBLE_VAL : ArrayType.DOUBLE_REF
            case TypeKind.CLASS:
            case TypeKind.STRING:
            case TypeKind.INTERFACE:
            case TypeKind.ARRAY:
            case TypeKind.TUPLE:
            case TypeKind.LAMBDA:
            case TypeKind.UNION:
                return new ArrayType(arrayClass, elemTD)
            default:
                throw new TypeError("Unexpected TypeKind " + ek)
        }
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return false
    }

    /**
     * Returns name of type
     *
     * @throws {Error} error in case of absence of name
     * @returns {string} name of type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getName(): string {
        return ""
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        return this.getElementType().toString() + "[]"
    }

    /**
     * Checks for equality this instance with provided object, treated as a ArrayType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has ArrayType and
     * their element types are the same
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof ArrayType && (other as ArrayType).getElementType().equals(this.getElementType())
    }

    /**
     * Returns element type of this array
     *
     * @returns {Type} element type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getElementType(): Type {
        return Type.resolve(this.elemTD, this.getContextLinker())!
    }

    /**
     * Makes instance of this array with provided length
     * Each element are instantiated using default value
     * If element type is class value then empty constructor are called
     *
     * @param {long} length of array
     * @throws {Error} error if element type doesn't have default value
     * @returns {Object} new instance of array
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public make(length: long): Object {
        return TypeAPI.makeArrayInstance(this.elemTD, this.getContextLinker(), length)
    }

    /**
     * Converts object to this array type
     *
     * @param {Any} obj Object to convert
     * @returns {Any} Converted object
     * @throws {Error} If conversion is invalid
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
    }
}

/**
 * Represents tuple type
 * @extends Type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class TupleType extends Type {
    /**
     * Constructor for TupleType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {RuntimeLinker} contextLinker Runtime linker context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return false
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return false
    }

    /**
     * Returns name of type
     *
     * @returns {string} name of type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getName(): string {
        return ""
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        return "[ ..., ...]"
    }

    /**
     * Checks for equality this instance with provided object, treated as a TupleType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has TupleType
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return false
    }

    /**
     * Converts object to this tuple type
     *
     * @param {Any} obj Object to convert
     * @returns {Any} Converted object
     * @throws {Error} Not implemented yet
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        throw new Error("todo(kprokopenko): add when tuple becomes available")
    }
}

/**
 * Represents function type
 * @extends Type
 * @note lambdas, functions and methods have function type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export abstract class FunctionType extends Type {
    private readonly attrs: int

    protected constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
        this.attrs = TypeAPI.getFunctionAttributes(td)
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return false
    }

    /**
     * Returns name of type
     *
     * @returns {string} name of type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getName(): string {
        return ""
    }

    /**
     * Returns return type of function type
     *
     * @returns {Type} result type
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getResultType(): Type {
        const resultTypeTd = this.getResultTypeDescriptor()
        return Type.resolve(resultTypeTd, this.getContextLinker())!
    }

    /**
     * Checks for equality this instance with provided object, treated as a FunctionType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has FunctionType and
     * they both has same signatures
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return (other instanceof FunctionType) && (other as FunctionType).getTypeDesc() == this.getTypeDesc()
    }

    /**
     * Returns number of parameters
     * For static methods receiver type counted as parameter
     * For instance methods receiver type isn't counted as parameter
     *
     * @returns {long} number of parameters
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native getParametersNum(): long

    /**
     * Returns ith parameter of function type
     *
     * @param {long} i index
     * @throws {TypeError} when i greater then number of parameters
     * @return {TypeAPIParameter} TypeAPIParameter, corresponding ith parameter in signature
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getParameter(i: long): TypeAPIParameter {
        const paramDescriptor = this.getParameterDescriptor(i)
        if (paramDescriptor) {
            const paramType = Type.resolve(paramDescriptor, this.getContextLinker())!
            return this.getParameter(i, paramType)
        }
        throw new TypeError("no parameter at " + i)
    }

    /**
     * Returns number of type parameters of this class
     *
     * @returns {long} number of type parameters
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getTypeParametersNum(): long {
        return 0
    }

    private native getResultTypeDescriptor(): RuntimeTypeDescriptor

    private native getParameterDescriptor(i: long): RuntimeTypeDescriptor | undefined

    private native getParameter(i: long, paramType: Type): TypeAPIParameter
}


/**
 * Represents lambda function type
 * @extends FunctionType
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class LambdaType extends FunctionType {
    /**
     * Constructor for LambdaType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {RuntimeLinker} contextLinker Runtime linker context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
    }

    /**
     * Returns literal representation of this lambda type
     *
     * @returns {string} Literal representation
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        let sb = new StringBuilder("(")
        const paramsNum = this.getParametersNum()
        for (let i = 0; i < paramsNum; ++i) {
            sb.append(this.getParameter(i).toString())
            if (i != paramsNum - 1) {
                sb.append(", ")
            }
        }
        sb.append("): ")
        sb.append(this.getResultType().toString())
        return sb.toString()
    }

    /**
     * Make an instance of LambdaType
     *
     * @returns {Object} LambdaType instance
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public make(): Object {
        return TypeAPI.getDeclaringClassImpl(this)!.createInstance()
    }

    /**
     * Checks if a type is assignable to this lambda type
     *
     * @param {Type} other Type to check
     * @returns {boolean} True if type is assignable
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override assignableFrom(other: Type): boolean {
        if (super.assignableFrom(other)) {
            return true
        }
        if (!(other instanceof LambdaType)) {
            return false
        }
        let l = (this)
        let r = other as LambdaType
        if (l.getParametersNum() < r.getParametersNum()) {
            return false
        }

        // Parameter types are using contravariance
        for (let i = 0; i < r.getParametersNum(); i++) {
            let lt = l.getParameter(i).getType()
            let rt = r.getParameter(i).getType()

            if (!lt.subTypeOf(rt)) {
                if (rt.isPrimitive()) {
                    const rrt = Type.toRefType(rt)
                    if (!lt.subTypeOf(rrt)) {
                        return false
                    }
                } else {
                    return false
                }
            }
        }

        // Return types are using covariance
        if (r.getResultType().subTypeOf(l.getResultType())) {
            return true
        }

        const rvalResType = r.getResultType()
        if (rvalResType.isPrimitive()) {
            return Type.toRefType(rvalResType).subTypeOf(l.getResultType())
        } else {
            return false
        }
    }

    /**
     * Converts object to this lambda type
     *
     * @param {Any} obj Object to convert
     * @returns {Any} Converted object
     * @throws {Error} If conversion is invalid
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (this.assignableFrom(objType)) {
            return obj
        }
        throw new Error("invalid conversion")
    }
}

/**
 * Represents method type
 * @extends FunctionType
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class MethodType extends FunctionType {
    /**
     * Constructor for MethodType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {RuntimeLinker} contextLinker Runtime linker context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
    }

    /**
     * Returns literal representation of this method type
     *
     * @returns {string} Literal representation
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        let sb = new StringBuilder("(")
        const paramsNum = this.getParametersNum()
        if (this.hasReceiver()) {
            sb.append("this: ")
            sb.append(this.getReceiverType().toString())
            if (paramsNum > 0) {
                sb.append(", ")
            }
        }
        for (let i = 0; i < paramsNum; ++i) {
            sb.append(this.getParameter(i).toString())
            if (i != paramsNum - 1) {
                sb.append(", ")
            }
        }
        sb.append("): ")
        sb.append(this.getResultType().toString())
        return sb.toString()
    }

    /**
     * Returns receiver type of this method
     *
     * @throws {TypeError} if method has no receiver (i.e. static)
     * @returns {Type} Type corresponding to method's receiver
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getReceiverType(): Type {
        const receiverTd = this.getReceiverTypeDescriptor()
        if (receiverTd) {
            return Type.resolve(receiverTd, this.getContextLinker())!
        }
        throw new TypeError("Method does not have receiver")
    }

    /**
     * Checks if a type is assignable to this method type
     *
     * @param {Type} other Type to check
     * @returns {boolean} Always false for MethodType
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override assignableFrom(other: Type): boolean {
        return false
    }

    /**
     * Checks if this method has a receiver
     *
     * @returns {boolean} True if method has a receiver
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public hasReceiver(): boolean {
        return this.getReceiverTypeDescriptor() !== undefined
    }

    /**
     * Converts object to this method type
     *
     * @param {Any} obj Object to convert
     * @returns {Any} Converted object
     * @throws {Error} Only LambdaType can be converted
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        throw new Error("Only LambdaType can be converted")
    }

    private native getReceiverTypeDescriptor(): RuntimeTypeDescriptor | undefined
}

/**
 * Represents string type
 * @extends Type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class StringType extends Type {
    /**
     * String type reference
     *
     * @type {StringType} REF
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static readonly REF: StringType = new StringType()

    /**
     * Constructor for StringType
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        super(TypeAPI.convertTypeDescriptor("std.core.String"), getBootRuntimeLinker())
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return false
    }

    /**
     * Returns name of type
     *
     * @returns {string} type name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getName(): string {
        return ""
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        return "string"
    }

    /**
     * Checks for equality this instance with provided object, treated as a StringType
     *
     * @param {Type} other object to be checked against
     * @returns {boolean} true if object also has StringType
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return other instanceof StringType
    }

    /**
     * Converts object to this string type
     *
     * @param {Any} obj Object to convert
     * @returns {Any} Converted object
     * @throws {Error} If conversion is invalid
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (!this.equals(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
    }
}

/**
 * Represents enum type
 * @extends Type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class EnumType extends Type {
    /**
     * Constructor for EnumType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {RuntimeLinker} contextLinker Runtime linker context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
    }

    /**
     * Checks if a type is assignable to this enum type
     *
     * @param {Type} other Type to check
     * @returns {boolean} True if type is assignable
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override assignableFrom(other: Type): boolean {
        if (super.assignableFrom(other)) {
            return true
        }
        if (!(other instanceof EnumType)) {
            return false
        }
        let rt = other as EnumType
        return this.getName() == rt.getName()
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return false
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type
     *
     * @returns {string} type name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getName(): string {
        return ""
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        return "enum {...}"
    }

    /**
     * Checks for equality this instance with provided object, treated as a EnumType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has EnumType and their names are the same
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        // NOTE(shumilov-petr): not implemented
        return false
    }

    /**
     * Returns number of constants in this enum
     *
     * @returns {long} Number of constants
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getConstantsNum(): long {
        // NOTE(shumilov-petr): not implemented
        return 0
    }

    /**
     * Returns ith constant of this enum
     *
     * @param {long} i Index
     * @returns {EnumConstant} Enum constant
     * @throws {Error} Not implemented yet
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getConstant(i: long): EnumConstant {
        // NOTE(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    /**
     * Returns constant by name
     *
     * @param {string} name Constant name
     * @returns {EnumConstant} Enum constant
     * @throws {Error} Not implemented yet
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getConstantByName(name: string): EnumConstant {
        // NOTE(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    /**
     * Creates enum type with constants
     *
     * @param {EnumConstant[]} consts Array of enum constants
     * @returns {EnumType} Created enum type
     * @throws {Error} Not implemented yet
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public create(consts: EnumConstant[]): EnumType {
        // NOTE(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    /**
     * Makes enum instance with constant name
     *
     * @param {string} constantName Constant name
     * @returns {Object} Enum instance
     * @throws {Error} Not implemented yet
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public make(constantName: string): Object {
        // NOTE(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    /**
     * Converts object to this enum type
     *
     * @param {Any} obj Object to convert
     * @returns {Any} Converted object
     * @throws {Error} Not implemented yet
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        throw new Error("todo(kprokopenko): enum conversion")
    }
}

/**
 * Represents union type
 * @extends Type
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class UnionType extends Type {
    /**
     * Constructor for UnionType
     *
     * @param {RuntimeTypeDescriptor} td Type descriptor
     * @param {RuntimeLinker} contextLinker Runtime linker context
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
    }

    /**
     * Checks if a type is assignable to this union type
     *
     * @param {Type} other Type to check
     * @returns {boolean} True if type is assignable
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override assignableFrom(other: Type): boolean {
        if (other instanceof UnionType) {
            const otherUnion = other as UnionType
            // all cases of other are assignable into this
            const otherCasesN = this.getCasesNum()
            return true
        }
        return false
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns {boolean} true if type is primitive and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns {boolean} true if type is reference and false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns {boolean} true if type has name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override hasName(): boolean {
        return false
    }

    /**
     * Returns name of type
     *
     * @returns {string} type name
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getName(): string {
        return ""
    }

    /**
     * Returns literal of type if exists
     *
     * @returns {string} type literal
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override getLiteral(): string {
        return "(... | ...)"
    }

    /**
     * Returns number of union cases
     *
     * @returns {long} Number of cases
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public getCasesNum(): long {
        return 0
    }

    /**
     * Checks for equality this instance with provided object, treated as a UnionType
     *
     * @param {Type} other type to be checked against
     * @returns {boolean} true if object also has UnionType and their cases are the same
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override equals(other: Type): boolean {
        return false
    }

    /**
     * Converts object to this union type
     * @param {Any} obj Object to convert
     * @returns {Any} Converted object
     * @throws {Error} If conversion is invalid
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
    }
}
