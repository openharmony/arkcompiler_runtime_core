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

native function ValueAPIGetFieldBoolean(obj: Object, i: long): boolean

native function ValueAPIGetFieldByte(obj: Object, i: long): byte

native function ValueAPIGetFieldShort(obj: Object, i: long): short

native function ValueAPIGetFieldChar(obj: Object, i: long): char

native function ValueAPIGetFieldInt(obj: Object, i: long): int

native function ValueAPIGetFieldFloat(obj: Object, i: long): float

native function ValueAPIGetFieldDouble(obj: Object, i: long): double

native function ValueAPIGetFieldLong(obj: Object, i: long): long

native function ValueAPIGetFieldObject(obj: Object, i: long): Object


native function ValueAPISetFieldBoolean(obj: Object, i: long, val: boolean)

native function ValueAPISetFieldByte(obj: Object, i: long, val: byte)

native function ValueAPISetFieldShort(obj: Object, i: long, val: short)

native function ValueAPISetFieldChar(obj: Object, i: long, val: char)

native function ValueAPISetFieldInt(obj: Object, i: long, val: int)

native function ValueAPISetFieldFloat(obj: Object, i: long, val: float)

native function ValueAPISetFieldDouble(obj: Object, i: long, val: double)

native function ValueAPISetFieldLong(obj: Object, i: long, val: long)

native function ValueAPISetFieldObject(obj: Object, i: long, val: Object)


native function ValueAPISetFieldByNameBoolean(obj: Object, name: String, val: boolean)

native function ValueAPISetFieldByNameByte(obj: Object, name: String, val: byte)

native function ValueAPISetFieldByNameShort(obj: Object, name: String, val: short)

native function ValueAPISetFieldByNameChar(obj: Object, name: String, val: char)

native function ValueAPISetFieldByNameInt(obj: Object, name: String, val: int)

native function ValueAPISetFieldByNameFloat(obj: Object, name: String, val: float)

native function ValueAPISetFieldByNameDouble(obj: Object, name: String, val: double)

native function ValueAPISetFieldByNameLong(obj: Object, name: String, val: long)

native function ValueAPISetFieldByNameObject(obj: Object, name: String, val: Object)


native function ValueAPISetElementBoolean(obj: Object, i: long, val: boolean)

native function ValueAPISetElementByte(obj: Object, i: long, val: byte)

native function ValueAPISetElementShort(obj: Object, i: long, val: short)

native function ValueAPISetElementChar(obj: Object, i: long, val: char)

native function ValueAPISetElementInt(obj: Object, i: long, val: int)

native function ValueAPISetElementFloat(obj: Object, i: long, val: float)

native function ValueAPISetElementDouble(obj: Object, i: long, val: double)

native function ValueAPISetElementLong(obj: Object, i: long, val: long)

native function ValueAPISetElementObject(obj: Object, i: long, val: Object)

export abstract class Value extends Object {
    public static of(o: Object | null): Value {
        let t = Type.of(o)
        if (t instanceof NullType) {
            return NullValue.getInstance()
        }
        let obj = o as Object
        if (t instanceof ClassType) {
            return new ClassValue(t as ClassType, obj)
        } else if (t instanceof ArrayType) {
            return new ArrayValue(t as ArrayType, obj)
        } else if (t instanceof StringType) {
            return new StringValue(t as StringType, obj as String)
        } else if (t instanceof BooleanType) {
            return new BooleanValue(t as BooleanType, obj as Boolean)
        } else if (t instanceof ByteType) {
            return new ByteValue(t as ByteType, obj as Byte)
        } else if (t instanceof ShortType) {
            return new ShortValue(t as ShortType, obj as Short)
        } else if (t instanceof CharType) {
            return new CharValue(t as CharType, obj as Char)
        } else if (t instanceof IntType) {
            return new IntValue(t as IntType, obj as Int)
        } else if (t instanceof FloatType) {
            return new FloatValue(t as FloatType, obj as Float)
        } else if (t instanceof DoubleType) {
            return new DoubleValue(t as DoubleType, obj as Double)
        } else if (t instanceof LongType) {
            return new LongValue(t as LongType, obj as Long)
        } else {
            throw new Error("Not implemented")
        }
    }

    public abstract getType(): Type
}

export final class ClassValue extends Value {
    private typ: ClassType
    private data: Object

    public override getType(): Type {
        return this.typ as Type
    }

    public getData(): Object {
        return this.data
    }

    internal constructor(typ: ClassType, data: Object) {
        this.typ = typ
        this.data = data
    }

    public getFieldsNum(): long {
        return this.typ.getFieldsNum()
    }

    public getField(i: long): Value {
        let ft = this.typ.getField(i).getType()
        if (!ft.isReference()) {
            if (ft instanceof BooleanType) {
                return new BooleanValue(ft as BooleanType, ValueAPIGetFieldBoolean(this.data, i))
            } else if (ft instanceof ByteType) {
                return new ByteValue(ft as ByteType, ValueAPIGetFieldByte(this.data, i))
            } else if (ft instanceof ShortType) {
                return new ShortValue(ft as ShortType, ValueAPIGetFieldShort(this.data, i))
            } else if (ft instanceof CharType) {
                return new CharValue(ft as CharType, ValueAPIGetFieldChar(this.data, i))
            } else if (ft instanceof IntType) {
                return new IntValue(ft as IntType, ValueAPIGetFieldInt(this.data, i))
            } else if (ft instanceof FloatType) {
                return new FloatValue(ft as FloatType, ValueAPIGetFieldFloat(this.data, i))
            } else if (ft instanceof DoubleType) {
                return new DoubleValue(ft as DoubleType, ValueAPIGetFieldDouble(this.data, i))
            } else if (ft instanceof LongType) {
                return new LongValue(ft as LongType, ValueAPIGetFieldLong(this.data, i))
            }
        }
        return Value.of(ValueAPIGetFieldObject(this.data, i))
    }

    public setFieldByName(name: string, val: Value) {
        let ft = this.typ.getFieldByName(name).getType()
        let vt = val.getType()
        if (!ft.assignableFrom(vt)) {
            throw new Error("Cannot assign field of type " + ft + " with value of type " + vt)
        }
        if (ft instanceof BooleanType && val instanceof BooleanValue) {
            ValueAPISetFieldByNameBoolean(this.data, name, (val as BooleanValue).getData())
        } else if (ft instanceof ByteType && val instanceof ByteValue) {
            ValueAPISetFieldByNameByte(this.data, name, (val as ByteValue).getData())
        } else if (ft instanceof ShortType && val instanceof ShortValue) {
            ValueAPISetFieldByNameShort(this.data, name, (val as ShortValue).getData())
        } else if (ft instanceof CharType && val instanceof CharValue) {
            ValueAPISetFieldByNameChar(this.data, name, (val as CharValue).getData())
        } else if (ft instanceof IntType && val instanceof IntValue) {
            ValueAPISetFieldByNameInt(this.data, name, (val as IntValue).getData())
        } else if (ft instanceof FloatType && val instanceof FloatValue) {
            ValueAPISetFieldByNameFloat(this.data, name, (val as FloatValue).getData())
        } else if (ft instanceof DoubleType && val instanceof DoubleValue) {
            ValueAPISetFieldByNameDouble(this.data, name, (val as DoubleValue).getData())
        } else if (ft instanceof LongType && val instanceof LongValue) {
            ValueAPISetFieldByNameLong(this.data, name, (val as LongValue).getData())
        } else if (ft instanceof StringType && val instanceof StringValue) {
            ValueAPISetFieldByNameObject(this.data, name, (val as StringValue).getData())
        } else if (ft instanceof ArrayType && val instanceof ArrayValue) {
            ValueAPISetFieldByNameObject(this.data, name, (val as ArrayValue).getData())
        } else if (ft instanceof ClassType && val instanceof ClassValue) {
            ValueAPISetFieldByNameObject(this.data, name, (val as ClassValue).getData())
        } else {
            throw new Error("Cannot assign field of type " + ft + " with value of type " + vt)
        }
    }

    public setField(i: long, val: Value) {
        let ft = this.typ.getField(i).getType()
        let vt = val.getType()
        if (!ft.assignableFrom(vt)) {
            throw new Error("Cannot assign field of type " + ft + " with value of type " + vt)
        }
        if (ft instanceof BooleanType && val instanceof BooleanValue) {
            ValueAPISetFieldBoolean(this.data, i, (val as BooleanValue).getData())
        } else if (ft instanceof ByteType && val instanceof ByteValue) {
            ValueAPISetFieldByte(this.data, i, (val as ByteValue).getData())
        } else if (ft instanceof ShortType && val instanceof ShortValue) {
            ValueAPISetFieldShort(this.data, i, (val as ShortValue).getData())
        } else if (ft instanceof CharType && val instanceof CharValue) {
            ValueAPISetFieldChar(this.data, i, (val as CharValue).getData())
        } else if (ft instanceof IntType && val instanceof IntValue) {
            ValueAPISetFieldInt(this.data, i, (val as IntValue).getData())
        } else if (ft instanceof FloatType && val instanceof FloatValue) {
            ValueAPISetFieldFloat(this.data, i, (val as FloatValue).getData())
        } else if (ft instanceof DoubleType && val instanceof DoubleValue) {
            ValueAPISetFieldDouble(this.data, i, (val as DoubleValue).getData())
        } else if (ft instanceof LongType && val instanceof LongValue) {
            ValueAPISetFieldLong(this.data, i, (val as LongValue).getData())
        } else if (ft instanceof StringType && val instanceof StringValue) {
            ValueAPISetFieldObject(this.data, i, (val as StringValue).getData())
        } else if (ft instanceof ArrayType && val instanceof ArrayValue) {
            ValueAPISetFieldObject(this.data, i, (val as ArrayValue).getData())
        } else if (ft instanceof ClassType && val instanceof ClassValue) {
            ValueAPISetFieldObject(this.data, i, (val as ClassValue).getData())
        } else {
            throw new Error("Cannot assign field of type " + ft + " with value of type " + vt)
        }
    }

}

export final class ArrayValue extends Value {
    private typ: ArrayType
    private data: Object

    public override getType(): Type {
        return this.typ as Type
    }

    public getData(): Object {
        return this.data
    }

    internal constructor(typ: ArrayType, data: Object) {
        this.typ = typ
        this.data = data
    }

    public setElementByIndex(i: long, val: Value) {
        let et = this.typ.getElementType()
        let vt = val.getType()
        if (!et.assignableFrom(vt)) {
            throw new Error("Cannot assign array of type " + et + " with value of type " + vt)
        }
        if (et instanceof BooleanType && val instanceof BooleanValue) {
            ValueAPISetElementBoolean(this.data, i, (val as BooleanValue).getData())
        } else if (et instanceof ByteType && val instanceof ByteValue) {
            ValueAPISetElementByte(this.data, i, (val as ByteValue).getData())
        } else if (et instanceof ShortType && val instanceof ShortValue) {
            ValueAPISetElementShort(this.data, i, (val as ShortValue).getData())
        } else if (et instanceof CharType && val instanceof CharValue) {
            ValueAPISetElementChar(this.data, i, (val as CharValue).getData())
        } else if (et instanceof IntType && val instanceof IntValue) {
            ValueAPISetElementInt(this.data, i, (val as IntValue).getData())
        } else if (et instanceof FloatType && val instanceof FloatValue) {
            ValueAPISetElementFloat(this.data, i, (val as FloatValue).getData())
        } else if (et instanceof DoubleType && val instanceof DoubleValue) {
            ValueAPISetElementDouble(this.data, i, (val as DoubleValue).getData())
        } else if (et instanceof LongType && val instanceof LongValue) {
            ValueAPISetElementLong(this.data, i, (val as LongValue).getData())
        } else if (et instanceof StringType && val instanceof StringValue) {
            ValueAPISetElementObject(this.data, i, (val as StringValue).getData())
        } else if (et instanceof ArrayType && val instanceof ArrayValue) {
            ValueAPISetElementObject(this.data, i, (val as ArrayValue).getData())
        } else if (et instanceof ClassType && val instanceof ClassValue) {
            ValueAPISetElementObject(this.data, i, (val as ClassValue).getData())
        } else {
            throw new Error("Cannot assign array of type " + et + " with value of type " + vt)
        }
    }
}

export final class BooleanValue extends Value {
    private typ: BooleanType
    private data: boolean

    public override getType(): Type {
        return this.typ as Type
    }

    public getData(): Boolean {
        return this.data
    }

    internal constructor(typ: BooleanType, data: boolean) {
        this.typ = typ
        this.data = data
    }

    internal constructor(typ: BooleanType, data: Boolean) {
        this(typ, data.unboxed())
    }
}

export final class ByteValue extends Value {
    private typ: ByteType
    private data: byte

    public override getType(): Type {
        return this.typ as Type
    }

    public getData(): Byte {
        return this.data
    }

    internal constructor(typ: ByteType, data: byte) {
        this.typ = typ
        this.data = data
    }

    internal constructor(typ: ByteType, data: Byte) {
        this(typ, data.unboxed())
    }
}

export final class ShortValue extends Value {
    private typ: ShortType
    private data: short

    public override getType(): Type {
        return this.typ as Type
    }

    public getData(): Short {
        return this.data
    }

    internal constructor(typ: ShortType, data: short) {
        this.typ = typ
        this.data = data
    }

    internal constructor(typ: ShortType, data: Short) {
        this(typ, data.unboxed())
    }
}

export final class CharValue extends Value {
    private typ: CharType
    private data: char

    public override getType(): Type {
        return this.typ as Type
    }

    public getData(): Char {
        return this.data
    }

    internal constructor(typ: CharType, data: char) {
        this.typ = typ
        this.data = data
    }

    internal constructor(typ: CharType, data: Char) {
        this(typ, data.unboxed())
    }
}

export final class IntValue extends Value {
    private typ: IntType
    private data: int

    public override getType(): Type {
        return this.typ as Type
    }

    public getData(): Int {
        return this.data
    }

    internal constructor(typ: IntType, data: int) {
        this.typ = typ
        this.data = data
    }

    internal constructor(typ: IntType, data: Int) {
        this(typ, data.unboxed())
    }
}

export final class FloatValue extends Value {
    private typ: FloatType
    private data: float

    public override getType(): Type {
        return this.typ as Type
    }

    public getData(): Float {
        return this.data
    }

    internal constructor(typ: FloatType, data: float) {
        this.typ = typ
        this.data = data
    }

    internal constructor(typ: FloatType, data: Float) {
        this(typ, data.unboxed())
    }
}

export final class DoubleValue extends Value {
    private typ: DoubleType
    private data: double

    public override getType(): Type {
        return this.typ as Type
    }

    public getData(): Double {
        return this.data
    }

    internal constructor(typ: DoubleType, data: double) {
        this.typ = typ
        this.data = data
    }

    internal constructor(typ: DoubleType, data: Double) {
        this(typ, data.unboxed())
    }
}

export final class LongValue extends Value {
    private typ: LongType
    private data: long

    public override getType(): Type {
        return this.typ as Type
    }

    public getData(): Long {
        return this.data
    }

    internal constructor(typ: LongType, data: long) {
        this.typ = typ
        this.data = data
    }

    internal constructor(typ: LongType, data: Long) {
        this(typ, data.unboxed())
    }
}

export final class StringValue extends Value {
    private typ: StringType
    private data: String

    public override getType(): Type {
        return this.typ as Type
    }

    public getData(): String {
        return this.data
    }

    internal constructor(typ: StringType, data: String) {
        this.typ = typ
        this.data = data
    }
}

export final class NullValue extends Value {
    private typ: NullType
    private static instance: NullValue | null = null

    public override getType(): Type {
        return this.typ as Type
    }

    internal constructor() {}

    public static getInstance(): NullValue {
        if (NullValue.instance == null) {
            NullValue.instance = new NullValue()
        }
        return NullValue.instance!
    }
}
