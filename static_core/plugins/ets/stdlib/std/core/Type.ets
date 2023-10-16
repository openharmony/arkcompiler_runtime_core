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

const OBJECT_DESC = 'Lstd/core/Object;'

export const ObjectType: ClassType = new ClassType(OBJECT_DESC)

export class Attributes {
    public static readonly STATIC: int = 1 << 0;
    public static readonly INHERITED: int = 1 << 1;
    public static readonly READONLY: int = 1 << 2;
    public static readonly FINAL: int = 1 << 3;
    public static readonly ABSTRACT: int = 1 << 4;
    public static readonly CONSTRUCTOR: int = 1 << 5;
    public static readonly REST: int = 1 << 6;
    public static readonly OPTIONAL: int = 1 << 7;

    private constructor() {}
}

export class AccessModifier {
    public static readonly PUBLIC = 0;
    public static readonly PRIVATE = 1;
    public static readonly PROTECTED = 2;

    private constructor() {}
}

export abstract class Type extends Object {
    internal td: TypeDesc

    public static resolve(td: TypeDesc): Type {
        let kind = TypeAPIGetTypeKind(td) & TypeKindMask
        switch (kind) {
            case VoidKind:
                return VoidType.VAL

            case CharKind:
                return TypeAPIIsValueType(td) ? CharType.VAL : CharType.REF
            case BooleanKind:
                return TypeAPIIsValueType(td) ? BooleanType.VAL : BooleanType.REF
            case ByteKind:
                return TypeAPIIsValueType(td) ? ByteType.VAL : ByteType.REF
            case ShortKind:
                return TypeAPIIsValueType(td) ? ShortType.VAL : ShortType.REF
            case IntKind:
                return TypeAPIIsValueType(td) ? IntType.VAL : IntType.REF
            case LongKind:
                return TypeAPIIsValueType(td) ? LongType.VAL : LongType.REF
            case FloatKind:
                return TypeAPIIsValueType(td) ? FloatType.VAL : FloatType.REF
            case DoubleKind:
                return TypeAPIIsValueType(td) ? DoubleType.VAL : DoubleType.REF

            case ClassKind:
                return new ClassType(td)
            case StringKind:
                return StringType.VAL
            case InterfaceKind:
                return new InterfaceType(td)
            case ArrayKind:
                return ArrayType.getInstance(td)
            case FunctionKind:
                return new FunctionType(td)
            case UnionKind:
                return new UnionType(td)
            case UndefinedKind:
                return UndefinedType.REF
            case NullKind:
                return NullType.REF

            case EnumKind:
                return new EnumType(td)
            default:
                // TODO(shumilov-petr): unknown type, need exception
                assert(false)
        }
    }

    public static of(v: boolean): Type {
        return BooleanType.VAL
    }

    public static of(v: char): Type {
        return CharType.VAL
    }

    public static of(v: byte): Type {
        return ByteType.VAL
    }

    public static of(v: short): Type {
        return ShortType.VAL
    }

    public static of(v: int): Type {
        return IntType.VAL
    }

    public static of(v: long): Type {
        return LongType.VAL
    }

    public static of(v: float): Type {
        return FloatType.VAL
    }

    public static of(v: double): Type {
        return DoubleType.VAL
    }

    // -----

    public static of(o: NullishType): Type {
        if (o == null) {
            return NullType.REF
        }
        let td = TypeAPIGetTypeDescriptor(o!)
        return Type.resolve(td)
    }

    // -----

    public static of(v: Boolean): Type {
        return BooleanType.REF
    }

    public static of(v: Char): Type {
        return CharType.REF
    }

    public static of(v: Byte): Type {
        return ByteType.REF
    }

    public static of(v: Short): Type {
        return ShortType.REF
    }

    public static of(v: Int): Type {
        return IntType.REF
    }

    public static of(v: Long): Type {
        return LongType.REF
    }

    public static of(v: Float): Type {
        return FloatType.REF
    }

    public static of(v: Double): Type {
        return DoubleType.REF
    }

    // -----

    public static of(v: string): Type {
        return StringType.VAL
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

    internal abstract convertObject(obj: NullishType): NullishType;

    private isNumericType(): boolean {
        return (this) instanceof ByteType
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

    internal tryConvertTo(val: Object): Object {
        return val
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
    public static readonly REF: UndefinedType = new UndefinedType()

    private constructor() {
        this.td = UndefinedTD
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

    public override equals(to: NullishType): boolean {
        return to instanceof UndefinedType
    }

    internal override convertObject(obj: NullishType): NullishType {
        throw new Error("todo(kprokopenko): add when undefined becomes available")
    }
}

export final class VoidType extends Type {
    public static readonly VAL: VoidType = new VoidType()
    public static readonly REF: VoidType = VoidType.VAL

    private constructor() {
        this.td = VoidTD
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

    public override equals(to: NullishType): boolean {
        return to instanceof VoidType
    }

    internal override convertObject(obj: NullishType): NullishType {
        return Void
    }
}


export final class CharType extends Type {
    public static readonly VAL: CharType = new CharType(CharValueTD, true)
    public static readonly REF: CharType = new CharType(TypeAPIGetTypeDescriptor(new Char()), false)

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
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

    public override equals(to: NullishType): boolean {
        return to instanceof CharType && this.isValue != (to as CharType).isReference()
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (!this.equals(objType)) {
            throw new Error("invalid type for conversion")
        }
        return obj
    }
}


export final class BooleanType extends Type {
    public static readonly VAL: BooleanType = new BooleanType(BooleanValueTD, true)
    public static readonly REF: BooleanType = new BooleanType(TypeAPIGetTypeDescriptor(new Boolean()), false)

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
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

    public override equals(to: NullishType): boolean {
        return to instanceof BooleanType && this.isValue != (to as BooleanType).isReference()
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (!this.equals(objType)) {
            throw new Error("invalid type for conversion")
        }
        return obj
    }
}

export final class ByteType extends Type {
    public static readonly VAL: ByteType = new ByteType(ByteValueTD, true)
    public static readonly REF: ByteType = new ByteType(TypeAPIGetTypeDescriptor(new Byte()), false)

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
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

    public override equals(to: NullishType): boolean {
        return to instanceof ByteType && this.isValue != (to as ByteType).isReference()
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).byteValue()
    }
}


export final class ShortType extends Type {
    public static readonly VAL: ShortType = new ShortType(ShortValueTD, true)
    public static readonly REF: ShortType = new ShortType(TypeAPIGetTypeDescriptor(new Short()), false)

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
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

    public override equals(to: NullishType): boolean {
        return to instanceof ShortType && this.isValue != (to as ShortType).isReference()
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).shortValue()
    }
}


export final class IntType extends Type {
    public static readonly VAL: IntType = new IntType(IntValueTD, true)
    public static readonly REF: IntType = new IntType(TypeAPIGetTypeDescriptor(new Int()), false)

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
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

    public override equals(to: NullishType): boolean {
        return to instanceof IntType && this.isValue != (to as IntType).isReference()
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).intValue()
    }
}


export final class LongType extends Type {
    public static readonly VAL = new LongType(LongValueTD, true)
    public static readonly REF = new LongType(TypeAPIGetTypeDescriptor(new Long()), false)

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
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

    public override equals(to: NullishType): boolean {
        return to instanceof LongType && this.isValue != (to as LongType).isReference()
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).longValue()
    }
}


export final class FloatType extends Type {
    public static readonly VAL: FloatType = new FloatType(FloatValueTD, true)
    public static readonly REF: FloatType = new FloatType(TypeAPIGetTypeDescriptor(new Float()), false)

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
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

    public override equals(to: NullishType): boolean {
        return to instanceof FloatType && this.isValue != (to as FloatType).isReference()
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).floatValue()
    }
}


export final class DoubleType extends Type {
    public static readonly VAL: DoubleType = new DoubleType(DoubleValueTD, true)
    public static readonly REF: DoubleType = new DoubleType(TypeAPIGetTypeDescriptor(new Double()), false)

    private isValue: boolean

    private constructor(td: TypeDesc, isValue: boolean) {
        this.td = td
        this.isValue = isValue
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

    public override equals(to: NullishType): boolean {
        return to instanceof DoubleType && this.isValue != (to as DoubleType).isReference()
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).doubleValue()
    }
}


export final class ClassType extends Type {
    internal constructor(td: TypeDesc) {
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

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
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

    public override equals(to: NullishType): boolean {
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

    public make(): Object {
        const emptyArgs = new NullishType[0]
        return this.make(emptyArgs)
    }

    // todo(kprokopenko): make varargs
    public make(args: NullishType[]): Object {
        const argTypes = new Type[args.length]
        for (let i = 0; i < args.length; i++) {
            argTypes[i] = Type.of(args[i])
        }
        const ctors = new Array<Method>()
        // collect all applicable constructors
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
        for (let inspect = 0; inspect < ctors.length(); inspect++) {
            const toRem = ctors.at(inspect)!.getType()
            let rem = false
            for (let c = 0; c < ctors.length(); c++) {
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
        if (ctors.length() != 1) {
            throw new Error("can't select consturctor: " + ctors.length() + " left")
        }
        return ctors.at(0)!.invoke(null, args)!
    }
}


export final class InterfaceType extends Type {
    public constructor(td: TypeDesc) {
        this.td = td
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
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

    public override equals(to: NullishType): boolean {
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

    public override equals(to: NullishType): boolean {
        // TODO(shumilov-petr): not implemented
        return false
    }

    public getElementType(): Type {
        return Type.resolve(this.elemTD)
    }

    public static create(et: Type): ArrayType {
        // TODO(kprokopenko): not implemented
        throw new Error("Not implemented")
    }

    public make(length : long): Object {
        let td = this.getElementType()
        return TypeAPIMakeArrayInstance(this.elemTD, length)
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        throw new Error("todo(kprokopenko): unequal array type conversion")
    }
}


export final class FunctionType extends Type {
    private isThrowable_: boolean
    private isNative_: boolean
    private isAsync_: boolean

    internal constructor(td: TypeDesc) {
        this.td = td
        // TODO(shumilov-petr): not implemented
        this.isThrowable_ = false
        this.isNative_ = false
        this.isAsync_ = false
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
        return this.isThrowable_
    }

    public isNative(): boolean {
        return this.isNative_
    }

    public isAsync(): boolean {
        return this.isAsync_
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

    public override equals(to: NullishType): boolean {
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

    public make(): Object {
        // TODO(kprokopenko): not implemented
        throw new Error("Not implemented")
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        throw new Error("todo(kprokopenko): unequal func type conversion")
    }
}


export final class StringType extends Type {
    public static readonly VAL: StringType = new StringType()
    public static readonly REF: StringType = StringType.VAL

    internal constructor() {
        this.td = TypeAPIGetTypeDescriptor("")
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

    public override equals(to: NullishType): boolean {
        return to instanceof StringType
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (!this.equals(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
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

    public override equals(to: NullishType): boolean {
        // TODO(shumilov-petr): not implemented
        return false
    }

    internal override convertObject(obj: NullishType): NullishType {
        throw new Error("todo(kprokopenko): enum conversion")
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

    public override equals(to: NullishType): boolean {
        // TODO(shumilov-petr): not implemented
        return false
    }

    internal override convertObject(obj: NullishType): NullishType {
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

export final class NullType extends Type {
    public static readonly REF: NullType = new NullType()

    private constructor() {
        this.td = NullTD
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

    public override equals(to: NullishType): boolean {
        return (to instanceof NullType);
    }

    internal override convertObject(obj: NullishType): NullishType {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return null
    }
}
