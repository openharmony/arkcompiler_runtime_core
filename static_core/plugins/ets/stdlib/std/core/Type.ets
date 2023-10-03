/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

package std.core;

type TypeDesc = string

/*
    Type intrinsics
*/
native function TypeAPIGetTypeDescriptor(o: Object): string

native function TypeAPIGetTypeKind(td: TypeDesc): byte

native function TypeAPIIsValueType(td: TypeDesc): boolean

native function TypeAPIGetTypeName(td: TypeDesc): string

native function TypeAPIGetTypeId(td: TypeDesc): long

/*
    Class type intrinsics
*/
native function TypeAPIGetFieldsNum(td: TypeDesc): long

native function TypeAPIGetField(td: TypeDesc, i: long): Field

native function TypeAPIGetFieldByName(td: TypeDesc, name: string): Field

native function TypeAPIGetMethodsNum(td: TypeDesc): long

native function TypeAPIGetMethod(td: TypeDesc, i: long): Method

native function TypeAPIGetConstructorsNum(td: TypeDesc): long

native function TypeAPIGetConstructor(td: TypeDesc, i: long): Method

native function TypeAPIMakeClassInstance(td: TypeDesc): Object

native function TypeAPIGetBaseType(td: TypeDesc): TypeDesc

native function TypeAPIGetInterfacesNum(td: TypeDesc): long

native function TypeAPIGetInterface(td: TypeDesc, i: long): TypeDesc

/*
    Function type intrinsics
*/
native function TypeAPIIsMethod(td: TypeDesc): boolean

native function TypeAPIGetReceiverType(td: TypeDesc): TypeDesc

native function TypeAPIGetParametersNum(td: TypeDesc): long

native function TypeAPIGetParameter(td: TypeDesc, i: long): Parameter

native function TypeAPIGetResultType(td: TypeDesc): TypeDesc

/*
    Array type intrinsics
*/
native function TypeAPIGetArrayElementType(td: TypeDesc): TypeDesc

native function TypeAPIMakeArrayInstance(td: TypeDesc, len: long): Object

// TODO(shumilov-petr): replace to enum, enum not available now
export type TypeKind = byte

const NoneKind: TypeKind        = 0x0
const VoidKind: TypeKind        = 0x1

const CharKind: TypeKind        = 0x2
const BooleanKind: TypeKind     = 0x3
const ByteKind: TypeKind        = 0x4
const ShortKind: TypeKind       = 0x5
const IntKind: TypeKind         = 0x6
const LongKind: TypeKind        = 0x7
const FloatKind: TypeKind       = 0x8
const DoubleKind: TypeKind      = 0x9

const ClassKind: TypeKind       = 0xA
const StringKind: TypeKind      = 0xB
const InterfaceKind: TypeKind   = 0xC
const ArrayKind: TypeKind       = 0xD
const FunctionKind: TypeKind    = 0xE
const UnionKind: TypeKind       = 0xF
const UndefinedKind: TypeKind   = 0x10
const NullKind: TypeKind        = 0x11

const EnumKind: TypeKind        = 0x12


const TypeKindMask: TypeKind = (1 << 6) - 1

// TODO(shumilov-petr): replace to enum, enum not available now
const BooleanValueTD: TypeDesc   = "Z"
const ByteValueTD: TypeDesc      = "B"
const ShortValueTD: TypeDesc     = "S"
const CharValueTD: TypeDesc      = "C"
const IntValueTD: TypeDesc       = "I"
const LongValueTD: TypeDesc      = "J"
const FloatValueTD: TypeDesc     = "F"
const DoubleValueTD: TypeDesc    = "D"

const VoidTD: TypeDesc      = "V"
const UndefinedTD: TypeDesc = "UNDEF" // TODO(shumilov-petr): get td from runtime
const NullTD: TypeDesc      = "Null"
//const ObjectType: Type      = Type.of(new Object())

export abstract class Type extends Object {
    protected td: TypeDesc

    public static resolve(td: TypeDesc): Type {
        let kind = TypeAPIGetTypeKind(td) & TypeKindMask
        switch (kind) {
            case VoidKind:
                return VoidType.getInstance()

            case CharKind:
                return CharType.getInstance(td)
            case BooleanKind:
                return BooleanType.getInstance(td)
            case ByteKind:
                return ByteType.getInstance(td)
            case ShortKind:
                return ShortType.getInstance(td)
            case IntKind:
                return IntType.getInstance(td)
            case LongKind:
                return LongType.getInstance(td)
            case FloatKind:
                return FloatType.getInstance(td)
            case DoubleKind:
                return DoubleType.getInstance(td)

            case ClassKind:
                return new ClassType(td)
            case StringKind:
                return StringType.getInstance()
            case InterfaceKind:
                return new InterfaceType(td)
            case ArrayKind:
                return ArrayType.getInstance(td)
            case FunctionKind:
                return new FunctionType(td)
            case UnionKind:
                return new UnionType(td)
            case UndefinedKind:
                return UndefinedType.getInstance()
            case NullKind:
                return NullType.getInstance()

            case EnumKind:
                return new EnumType(td)
            default:
                // TODO(shumilov-petr): unknown type, need exception
                assert(false)
        }
    }

    public static of(v: boolean): Type {
        return BooleanType.getInstance(BooleanValueTD)
    }

    public static of(v: char): Type {
        return CharType.getInstance(CharValueTD)
    }

    public static of(v: byte): Type {
        return ByteType.getInstance(ByteValueTD)
    }

    public static of(v: short): Type {
        return ShortType.getInstance(ShortValueTD)
    }

    public static of(v: int): Type {
        return IntType.getInstance(IntValueTD)
    }

    public static of(v: long): Type {
        return LongType.getInstance(LongValueTD)
    }

    public static of(v: float): Type {
        return FloatType.getInstance(FloatValueTD)
    }

    public static of(v: double): Type {
        return DoubleType.getInstance(DoubleValueTD)
    }

    // -----

    public static of(o: Object | null): Type {
        if (o == null) {
            return NullType.getInstance()
        }
        let td = TypeAPIGetTypeDescriptor(o!)
        return Type.resolve(td)
    }

    // -----

    public static of(v: Boolean): Type {
        return BooleanType.getInstance(TypeAPIGetTypeDescriptor(v))
    }

    public static of(v: Char): Type {
        return CharType.getInstance(TypeAPIGetTypeDescriptor(v))
    }

    public static of(v: Byte): Type {
        return ByteType.getInstance(TypeAPIGetTypeDescriptor(v))
    }

    public static of(v: Short): Type {
        return ShortType.getInstance(TypeAPIGetTypeDescriptor(v))
    }

    public static of(v: Int): Type {
        return IntType.getInstance(TypeAPIGetTypeDescriptor(v))
    }

    public static of(v: Long): Type {
        return LongType.getInstance(TypeAPIGetTypeDescriptor(v))
    }

    public static of(v: Float): Type {
        return FloatType.getInstance(TypeAPIGetTypeDescriptor(v))
    }

    public static of(v: Double): Type {
        return DoubleType.getInstance(TypeAPIGetTypeDescriptor(v))
    }

    // -----

    public static of(v: string): Type {
        return StringType.getInstance()
    }

    // -----

    public static of(v: boolean[]): Type {
        return ArrayType.getInstance(TypeAPIGetTypeDescriptor(v), BooleanValueTD)
    }

    public static of(v: char[]): Type {
        return ArrayType.getInstance(TypeAPIGetTypeDescriptor(v), CharValueTD)
    }

    public static of(v: byte[]): Type {
        return ArrayType.getInstance(TypeAPIGetTypeDescriptor(v), ByteValueTD)
    }

    public static of(v: short[]): Type {
        return ArrayType.getInstance(TypeAPIGetTypeDescriptor(v), ShortValueTD)
    }

    public static of(v: int[]): Type {
        return ArrayType.getInstance(TypeAPIGetTypeDescriptor(v), IntValueTD)
    }

    public static of(v: long[]): Type {
        return ArrayType.getInstance(TypeAPIGetTypeDescriptor(v), LongValueTD)
    }

    public static of(v: float[]): Type {
        return ArrayType.getInstance(TypeAPIGetTypeDescriptor(v), FloatValueTD)
    }

    public static of(v: double[]): Type {
        return ArrayType.getInstance(TypeAPIGetTypeDescriptor(v), DoubleValueTD)
    }

    // -----

    public static of(v: Object[]): Type {
        return ArrayType.getInstance(TypeAPIGetTypeDescriptor(v))
    }

    public abstract isPrimitive(): boolean // Or Composite

    public abstract isReference(): boolean // Or Value

    public abstract hasName(): boolean

    protected subTypeOf(other: Type): boolean {
        if (this.equals(other)) {
            return true
        } else if (other.equals(Type.of(new Object()))) {
            return this.isReference() &&
                            !((this) instanceof NullType ||
                                (this) instanceof UndefinedType )
        }
        return false
    }

    private isNumericType(): boolean {
        return (this) instanceof ByteType
                || (this) instanceof CharType
                || (this) instanceof ShortType
                || (this) instanceof IntType
                || (this) instanceof LongType
                || (this) instanceof FloatType
                || (this) instanceof DoubleType
    }

    public assignableFrom(other: Type): boolean {
        if (other.subTypeOf(this)) {
            return true
        }
        if (this.isNumericType() && other.isNumericType()) {
            return true
        }
        return false
    }

    public getId(): long {
        return TypeAPIGetTypeId(this.td);
    }

    public abstract getName(): string

    public abstract getLiteral(): string

    public override toString(): string {
        if (this.hasName()) {
            return this.getName()
        }
        return this.getLiteral()
    }
}

export final class UndefinedType extends Type {
    private static instance: UndefinedType | null = null

    private constructor() {
        this.td = UndefinedTD
    }

    internal static getInstance(): UndefinedType {
        if (UndefinedType.instance == null) {
            UndefinedType.instance = new UndefinedType()
        }
        return UndefinedType.instance!
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return true
    }

    public override hasName(): boolean {
        return true
    }

    public override getName(): string {
        return "undefined"
    }

    public override getLiteral(): string {
        return "undefined"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof UndefinedType
    }
}

export final class VoidType extends Type {
    private static instance: VoidType | null = null

    private constructor() {
        this.td = VoidTD
    }

    public static getInstance(): VoidType {
        if (VoidType.instance == null) {
            VoidType.instance = new VoidType()
        }
        return VoidType.instance as VoidType
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return true
    }

    public override hasName(): boolean {
        return true
    }

    public override getName(): string {
        return "void"
    }

    public override getLiteral(): string {
        return "void"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof VoidType
    }
}


export final class CharType extends Type {
    private static refInst: CharType | null = null
    private static valInst: CharType | null = null

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
    }

    internal static getInstance(td: TypeDesc): CharType {
        if (TypeAPIIsValueType(td)) {
            CharType.valInst = (CharType.valInst == null) ? new CharType(td, true) : CharType.valInst
            return CharType.valInst!
        }
        CharType.refInst = (CharType.refInst == null) ? new CharType(td, false) : CharType.refInst
        return CharType.refInst!
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return !this.isValue
    }

    public override hasName(): boolean {
        return !this.isValue
    }

    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPIGetTypeName(this.td)
    }

    public override getLiteral(): string {
        if (this.isValue) {
            return "char"
        }
        return "Char"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof CharType && this.isValue != (to as CharType).isReference()
    }
}


export final class BooleanType extends Type {
    private static refInst: BooleanType | null = null
    private static valInst: BooleanType | null = null

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
    }

    internal static getInstance(td: TypeDesc): BooleanType {
        if (TypeAPIIsValueType(td)) {
            BooleanType.valInst = (BooleanType.valInst == null) ? new BooleanType(td, true) : BooleanType.valInst
            return BooleanType.valInst!
        }
        BooleanType.refInst =(BooleanType.refInst == null) ? new BooleanType(td, false) : BooleanType.refInst
        return BooleanType.refInst!
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return !this.isValue
    }

    public override hasName(): boolean {
        return !this.isValue
    }

    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPIGetTypeName(this.td)
    }

    public override getLiteral(): string {
        if (this.isValue) {
            return "boolean"
        }
        return "Boolean"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof BooleanType && this.isValue != (to as BooleanType).isReference()
    }
}


export final class ByteType extends Type {
    private static refInst: ByteType | null = null
    private static valInst: ByteType | null = null

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
    }

    internal static getInstance(td: TypeDesc): ByteType {
        if (TypeAPIIsValueType(td)) {
            ByteType.valInst = (ByteType.valInst == null) ? new ByteType(td, true) : ByteType.valInst
            return ByteType.valInst!
        }
        ByteType.refInst = (ByteType.refInst == null) ? new ByteType(td, false) : ByteType.refInst
        return ByteType.refInst!
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return !this.isValue
    }

    public override hasName(): boolean {
        return !this.isValue
    }

    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPIGetTypeName(this.td)
    }

    public override getLiteral(): string {
        if (this.isValue) {
            return "byte"
        }
        return "Byte"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof ByteType && this.isValue != (to as ByteType).isReference()
    }
}


export final class ShortType extends Type {
    private static refInst: ShortType | null = null
    private static valInst: ShortType | null = null

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
    }

    internal static getInstance(td: TypeDesc): ShortType {
        if (TypeAPIIsValueType(td)) {
            ShortType.valInst = (ShortType.valInst == null) ? new ShortType(td, true) : ShortType.valInst
            return ShortType.valInst!
        }
        ShortType.refInst = (ShortType.refInst == null) ? new ShortType(td, false) : ShortType.refInst
        return ShortType.refInst!
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return !this.isValue
    }

    public override hasName(): boolean {
        return !this.isValue
    }

    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPIGetTypeName(this.td)
    }

    public override getLiteral(): string {
        if (this.isValue) {
            return "short"
        }
        return "Short"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof ShortType && this.isValue != (to as ShortType).isReference()
    }
}


export final class IntType extends Type {
    private static refInst: IntType | null = null
    private static valInst: IntType | null = null

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
    }

    internal static getInstance(td: TypeDesc): IntType {
        if (TypeAPIIsValueType(td)) {
            IntType.valInst = (IntType.valInst == null) ? new IntType(td, true) : IntType.valInst
            return IntType.valInst!
        }
        IntType.refInst = (IntType.refInst == null) ? new IntType(td, false) : IntType.refInst
        return IntType.refInst!
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return !this.isValue
    }

    public override hasName(): boolean {
        return !this.isValue
    }

    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPIGetTypeName(this.td)
    }

    public override getLiteral(): string {
        if (this.isValue) {
            return "int"
        }
        return "Int"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof IntType && this.isValue != (to as IntType).isReference()
    }
}


export final class LongType extends Type {
    private static refInst: LongType | null = null
    private static valInst: LongType | null = null

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
    }

    internal static getInstance(td: TypeDesc): LongType {
        if (TypeAPIIsValueType(td)) {
            LongType.valInst = (LongType.valInst == null) ? new LongType(td, true) : LongType.valInst
            return LongType.valInst!
        }
        LongType.refInst = (LongType.refInst == null) ? new LongType(td, false) : LongType.refInst
        return LongType.refInst!
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return !this.isValue
    }

    public override hasName(): boolean {
        return !this.isValue
    }

    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPIGetTypeName(this.td)
    }

    public override getLiteral(): string {
        if (this.isValue) {
            return "long"
        }
        return "Long"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof LongType && this.isValue != (to as LongType).isReference()
    }
}


export final class FloatType extends Type {
    private static refInst: FloatType | null = null
    private static valInst: FloatType | null = null

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
    }

    internal static getInstance(td: TypeDesc): FloatType {
        if (TypeAPIIsValueType(td)) {
            FloatType.valInst = (FloatType.valInst == null) ? new FloatType(td, true) : FloatType.valInst
            return FloatType.valInst!
        }
        FloatType.refInst = (FloatType.refInst == null) ? new FloatType(td, false) : FloatType.refInst
        return FloatType.refInst!
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return !this.isValue
    }

    public override hasName(): boolean {
        return !this.isValue
    }

    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPIGetTypeName(this.td)
    }

    public override getLiteral(): string {
        if (this.isValue) {
            return "float"
        }
        return "Float"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof FloatType && this.isValue != (to as FloatType).isReference()
    }
}


export final class DoubleType extends Type {
    private static refInst: DoubleType | null = null
    private static valInst: DoubleType | null = null

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
    }

    internal static getInstance(td: TypeDesc): DoubleType {
        if (TypeAPIIsValueType(td)) {
            DoubleType.valInst = (DoubleType.valInst == null) ? new DoubleType(td, true) : DoubleType.valInst
            return DoubleType.valInst!
        }
        DoubleType.refInst = (DoubleType.refInst == null) ? new DoubleType(td, false) : DoubleType.refInst
        return DoubleType.refInst!
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return !this.isValue
    }

    public override hasName(): boolean {
        return !this.isValue
    }

    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPIGetTypeName(this.td)
    }

    public override getLiteral(): string {
        if (this.isValue) {
            return "double"
        }
        return "Double"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof DoubleType && this.isValue != (to as DoubleType).isReference()
    }
}


export final class ClassType extends Type {
    public constructor(td: TypeDesc) {
        this.td = td
    }

    protected override subTypeOf(other: Type): boolean {
        if (super.subTypeOf(other)) {
            return true
        }
        if (other instanceof ClassType) {
            for (let i = this; i != i.getBaseType(); i = i.getBaseType()) {
                if (other == i) {
                    return true
                }
            }
            return false
        }
        if (other instanceof InterfaceType) {
            let used = new Array<Object>()
            let lt = other as InterfaceType
            for (let i = 0; i < this.getInterfacesNum(); i++) {
                if (this.getInterface(i).hasSuper(used, lt)) {
                    return true
                }
            }
        }
        return false
    }

    public override isPrimitive(): boolean {
        return false
    }

    public override isReference(): boolean {
        return true
    }

    public override hasName(): boolean {
        return true
    }

    public override getName(): string {
        return TypeAPIGetTypeName(this.td)
    }

    public override getLiteral(): string {
        // TODO(shumilov-petr): not implemented
        return "class{...}"
    }

    public override equals(to: Object|null): boolean {
        return (to instanceof ClassType) && (to as ClassType).td == this.td
    }

    public hasEmptyConstructor(): boolean {
        let num = this.getConstructorsNum()
        for (let i = 0; i < num; i++) {
            let cnst = this.getConstructor(i)
            let ft = cnst.getType()
            let ct = ft.getReceiverType()
            if (ft.getParametersNum() == 0 && this.equals(ct)) {
                return true
            }
        }
        return false
    }

    // TODO(kirill-mitkin): if there is no base type, returns ClassType of Object
    public getBaseType(): ClassType {
        return Type.resolve(TypeAPIGetBaseType(this.td)) as ClassType
    }

    public getInterfacesNum(): long {
        return TypeAPIGetInterfacesNum(this.td)
    }

    public getInterface(i: long): InterfaceType {
        return Type.resolve(TypeAPIGetInterface(this.td, i)) as InterfaceType
    }

    public getFieldsNum(): long {
        return TypeAPIGetFieldsNum(this.td)
    }

    public getField(i: long): Field {
        return TypeAPIGetField(this.td, i)
    }

    public getFieldByName(name: string): Field {
        return TypeAPIGetFieldByName(this.td, name)
    }

    public getMethodsNum(): long {
        return TypeAPIGetMethodsNum(this.td)
    }

    public getMethod(i: long): Method {
        return TypeAPIGetMethod(this.td, i)
    }

    public getConstructorsNum(): long {
        return TypeAPIGetConstructorsNum(this.td)
    }

    public getConstructor(i: long): Method {
        return TypeAPIGetConstructor(this.td, i)
    }

    public getTypeParametersNum(): long {
        // TODO(shumilov-petr): not implemented
        return 0
    }

    public getTypeParameter(i: long): Type {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public static create(name: string): ClassType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addBaseType(bt: Type): ClassType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addField(f: Field): ClassType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addMethod(f: Method): ClassType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addConstructor(ctor: Method): ClassType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addTypeParameter(tp: Type): ClassType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public make(): Object {
        if (!this.hasEmptyConstructor()) {
            throw new Error("Class " + this.getName() " doesn't have empty constructor")
        }
        return TypeAPIMakeClassInstance(this.td)
    }
}


export final class InterfaceType extends Type {
    public constructor(td: TypeDesc) {
        this.td = td
    }

    public override isPrimitive(): boolean {
        return false
    }

    public override isReference(): boolean {
        return true
    }

    public override hasName(): boolean {
        return true
    }

    public override getName(): string {
        return TypeAPIGetTypeName(this.td)
    }

    public override getLiteral(): string {
        // TODO(shumilov-petr): not implemented
        return "interface{...}"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof InterfaceType && this.td == (to as InterfaceType).td
    }

    // TODO(kirill-mitkin): change to Set<Long> when it will be possible
    internal hasSuper(used: Array<Object>, expected: InterfaceType): boolean {
        if (this.equals(expected)) {
            return true;
        }
        let found = false
        let id = Long.valueOf(this.getId())
        used.push(id)
        for (let i = 0; i < this.getInterfacesNum(); i++) {
            let iface = this.getInterface(i)
            let ind = used.indexOf(Long.valueOf(iface.getId()))
            if (ind == -1) {
                found |= iface.hasSuper(used, expected)
            }
            if (found) {
                return true
            }
        }
        return found
    }

    protected override subTypeOf(other: Type): boolean {
        if (super.subTypeOf(other)) {
            return true
        }
        if (other instanceof InterfaceType) {
            let used = new Array<Object>()
            let lt = other as InterfaceType
            for (let i = 0; i < this.getInterfacesNum(); i++) {
                if (this.getInterface(i).hasSuper(used, lt)) {
                    return true
                }
            }
            return false
        }
        return false
    }

    public getInterfacesNum(): long {
        return TypeAPIGetInterfacesNum(this.td)
    }

    public getInterface(i: long): InterfaceType {
        return Type.resolve(TypeAPIGetInterface(this.td, i)) as InterfaceType
    }

    public getFieldsNum(): long {
        // TODO(shumilov-petr): not implemented
        return 0
    }

    public getField(i: long): Field {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public getMethodsNum(): long {
        // TODO(shumilov-petr): not implemented
        return 0
    }

    public getMethod(i: long): Method {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public getTypeParametersNum(): long {
        // TODO(shumilov-petr): not implemented
        return 0
    }

    public getTypeParameter(i: long): Type {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public static create(name: string): InterfaceType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addBaseType(bt: Type): InterfaceType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addField(f: Field): InterfaceType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addMethod(f: Method): InterfaceType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addTypeParameter(tp: Type): ClassType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }
}


export final class ArrayType extends Type {
    private elemTD: TypeDesc

    private static booleanInst: ArrayType | null = null
    private static charInst: ArrayType | null = null
    private static byteInst: ArrayType | null = null
    private static shortInst: ArrayType | null = null
    private static intInst: ArrayType | null = null
    private static longInst: ArrayType | null = null
    private static floatInst: ArrayType | null = null
    private static doubleInst: ArrayType | null = null

    private constructor(td: TypeDesc, elemTD: TypeDesc) {
        this.td = td
        this.elemTD = elemTD
    }

    public override assignableFrom(other: Type): boolean {
        if (super.assignableFrom(other)) {
            return true
        }
        return other instanceof ArrayType && (other as ArrayType).getElementType().subTypeOf(this.getElementType())
    }

    internal static getInstance(td: TypeDesc, elemTD: TypeDesc): ArrayType {
        let ek = TypeAPIGetTypeKind(elemTD) & TypeKindMask
        switch (ek) {
            case BooleanKind:
                ArrayType.booleanInst = (ArrayType.booleanInst == null) ? new ArrayType(td, elemTD) : ArrayType.booleanInst
                return ArrayType.booleanInst!
            case CharKind:
                ArrayType.charInst = (ArrayType.charInst == null) ? new ArrayType(td, elemTD) : ArrayType.charInst
                return ArrayType.charInst!
            case ByteKind:
                ArrayType.byteInst = (ArrayType.byteInst == null) ? new ArrayType(td, elemTD) : ArrayType.byteInst
                return ArrayType.byteInst!
            case ShortKind:
                ArrayType.shortInst = (ArrayType.shortInst == null) ? new ArrayType(td, elemTD) : ArrayType.shortInst
                return ArrayType.shortInst!
            case IntKind:
                ArrayType.intInst = (ArrayType.intInst == null) ? new ArrayType(td, elemTD) : ArrayType.intInst
                return ArrayType.intInst!
            case LongKind:
                ArrayType.longInst = (ArrayType.longInst == null) ? new ArrayType(td, elemTD) : ArrayType.longInst
                return ArrayType.longInst!
            case FloatKind:
                ArrayType.floatInst = (ArrayType.floatInst == null) ? new ArrayType(td, elemTD) : ArrayType.floatInst
                return ArrayType.floatInst!
            case DoubleKind:
                ArrayType.doubleInst = (ArrayType.doubleInst == null) ? new ArrayType(td, elemTD) : ArrayType.doubleInst
                return ArrayType.doubleInst!
            case ClassKind:
            case StringKind:
            case InterfaceKind:
            case ArrayKind:
            case FunctionKind:
            case UnionKind:
                return new ArrayType(td, elemTD)
            default:
                // TODO(shumilov-petr): need throw exception
                assert(false)
        }
    }

    internal static getInstance(td: TypeDesc): ArrayType {
        return ArrayType.getInstance(td, TypeAPIGetArrayElementType(td))
    }

    public override isPrimitive(): boolean {
        return false
    }

    public override isReference(): boolean {
        return true
    }

    public override hasName(): boolean {
        return false
    }

    public override getName(): string {
        return ""
    }

    public override getLiteral(): string {
        return this.getElementType().toString() + "[]"
    }

    public override equals(to: Object|null): boolean {
        // TODO(shumilov-petr): not implemented
        return false
    }

    public getElementType(): Type {
        return Type.resolve(this.elemTD)
    }

    public static create(et: Type): ArrayType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public make(length : long): Object {
        let td = this.getElementType()
        let hasDefaultValue = true
        if (td instanceof ClassType) {
            hasDefaultValue &= (td as ClassType).hasEmptyConstructor()
        }
        return TypeAPIMakeArrayInstance(this.elemTD, length)
    }
}


export final class FunctionType extends Type {
    private isThrowabl: boolean
    private isNativ: boolean
    private isAsyn: boolean

    internal constructor(td: TypeDesc) {
        this.td = td
        // TODO(shumilov-petr): not implemented
        this.isThrowabl = false
        this.isNativ = false
        this.isAsyn = false
    }

    public override assignableFrom(other: Type): boolean {
        if (super.assignableFrom(other)) {
            return true
        }
        if (!(other instanceof FunctionType)) {
            return false
        }
        let lft = (this)
        let rft = other as FunctionType
        if (this.getParametersNum() != rft.getParametersNum() || this.isMethod() || rft.isMethod()) {
            return false
        }
        for (let i = 0; i < lft.getParametersNum(); i++) {
            let lpt = lft.getParameter(i).getType()
            let rpt = rft.getParameter(i).getType()
            if (!lpt.subTypeOf(rpt)) {
                return false
            }
        }
        return rft.getResultType().subTypeOf(lft.getResultType())
    }

    public override isPrimitive(): boolean {
        return false
    }

    public override isReference(): boolean {
        return true
    }

    public override hasName(): boolean {
        return false
    }

    public isThrowable(): boolean {
        return this.isThrowabl
    }

    public isNative(): boolean {
        return this.isNativ
    }

    public isAsync(): boolean {
        return this.isAsyn
    }

    public isMethod(): boolean {
        return TypeAPIIsMethod(this.td)
    }

    public override getName(): string {
        // TODO(shumilov-petr): not implemented
        return ""
    }

    public override getLiteral(): string {
        // TODO(shumilov-petr): not implemented
        return "()"
    }

    public override equals(to: Object|null): boolean {
        // TODO(shumilov-petr): not implemented
        return false
    }

    public getReceiverType(): Type {
        return Type.resolve(TypeAPIGetReceiverType(this.td))
    }

    public getParametersNum(): long {
        return TypeAPIGetParametersNum(this.td)
    }

    public getParameter(i: long): Parameter {
        return TypeAPIGetParameter(this.td, i)
    }

    public getResultType(): Type {
        return Type.resolve(TypeAPIGetResultType(this.td))
    }

    public getTypeParametersNum(): long {
        // TODO(shumilov-petr): not implemented
        return 0
    }

    public getTypeParameter(i: long): Type {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public static create(): FunctionType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addParameter(p: Parameter): FunctionType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addReturnType(rt: Type): FunctionType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addThrows(): FunctionType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public addAsync(): FunctionType {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }

    public make(): Object {
        // TODO(shumilov-petr): not implemented
        throw new Error("Not implemented")
    }
}


export final class StringType extends Type {
    private static inst: StringType | null = null

    internal constructor() {
        this.td = TypeAPIGetTypeDescriptor("")
    }

    internal static getInstance(): StringType {
        StringType.inst = (StringType.inst == null) ? new StringType() : StringType.inst
        return StringType.inst as StringType
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return true
    }

    public override hasName(): boolean {
        return false
    }

    public override getName(): string {
        // TODO(shumilov-petr): not implemented
        return ""
    }

    public override getLiteral(): string {
        // TODO(shumilov-petr): not implemented
        return "string"
    }

    public override equals(to: Object|null): boolean {
        return to instanceof StringType
    }
}


export final class EnumType extends Type {
    internal constructor(td: TypeDesc) {
        this.td = td
    }

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

    public override isPrimitive(): boolean {
        return false
    }

    public override isReference(): boolean {
        return false
    }

    public override hasName(): boolean {
        return true
    }

    public override getName(): string {
        // TODO(shumilov-petr): not implemented
        return ""
    }

    public override getLiteral(): string {
        // TODO(shumilov-petr): not implemented
        return "enum {...}"
    }

    public override equals(to: Object|null): boolean {
        // TODO(shumilov-petr): not implemented
        return false
    }
}


export final class UnionType extends Type {
    internal constructor(td: TypeDesc) {
        this.td = td
    }

    public override assignableFrom(other: Type): boolean {
        if (super.assignableFrom(other)) {
            return true
        }
        if (other instanceof UnionType) {
            return this.equals(other)
        }
        for (let i = 0; i < this.getCasesNum(); i++) {
            let cas = this.getCase(i)
            if (other.subTypeOf(cas.getType())) {
                return true
            }
        }
        return false
    }

    public override isPrimitive(): boolean {
        return false
    }

    public override isReference(): boolean {
        return true
    }

    public override hasName(): boolean {
        return false
    }

    public override getName(): string {
        // TODO(shumilov-petr): not implemented
        return ""
    }

    public override getLiteral(): string {
        // TODO(shumilov-petr): not implemented
        return "(... | ...)"
    }

    public getCasesNum(): long {
        // TODO(kirill-mitkin): not implemented
        return 0
    }

    public getCase(i: long): UnionCase {
        // TODO(kirill-mitkin): not implemented
        throw new Error("Not implemented")
    }

    public override equals(to: Object|null): boolean {
        // TODO(shumilov-petr): not implemented
        return false
    }
}

export final class NullType extends Type {
    private static instance: NullType | null = null

    private constructor() {
        this.td = NullTD
    }

    internal static getInstance(): NullType {
        if (NullType.instance == null) {
            NullType.instance = new NullType()
        }
        return NullType.instance!
    }

    public override isPrimitive(): boolean {
        return true
    }

    public override isReference(): boolean {
        return true
    }

    public override hasName(): boolean {
        return false
    }

    public override getName(): string {
        return "null"
    }

    public override getLiteral(): string {
        return "null"
    }

    public override equals(to: Object|null): boolean {
        return (to instanceof NullType);
    }
}
