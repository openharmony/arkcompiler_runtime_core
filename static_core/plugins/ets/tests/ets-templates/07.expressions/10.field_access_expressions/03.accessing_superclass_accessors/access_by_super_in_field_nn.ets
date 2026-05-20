/*---
Copyright (c) 2024-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    The result of the field access expression is computed at runtime.
    For a non-static field: The object reference expression is evaluated.
    Only the object reference type (not the class type of an actual object referred at runtime)
    is used to determine the field to be accessed
---*/

interface IName {
    name: String
}

class Abyte {
    private _a: byte = (2).toByte()
    get a(): byte { return this._a }
}
class Bbyte extends Abyte {}
final class Cbyte extends Bbyte {
    b: byte = super.a
}

class Ashort {
    private _a: short = (2).toShort()
    get a(): short { return this._a }
}
class Bshort extends Ashort {}
final class Cshort extends Bshort {
    b: short = super.a
}

class Along {
    private _a: long = 2
    get a(): long { return this._a }
}
class Blong extends Along {}
final class Clong extends Blong {
    b: long = super.a
}

class Afloat {
    private _a: float = 1.2f
    get a(): float { return this._a }
}
class Bfloat extends Afloat {}
final class Cfloat extends Bfloat {
    b: float = super.a
}

class Adouble {
    private _a: double = 2
    get a(): double { return this._a }
}
class Bdouble extends Adouble {}
final class Cdouble extends Bdouble {
    b: double = super.a
}

class Anumber {
    private _a: number = 2
    get a(): number { return this._a }
}
class Bnumber extends Anumber {}
final class Cnumber extends Bnumber {
    b: number = super.a
}

class Abigint {
    private _a: bigint = new BigInt(Long.MAX_VALUE * 2)
    get a(): bigint { return this._a }
}
class Bbigint extends Abigint {}
final class Cbigint extends Bbigint {
    b: bigint = super.a
}

class Achar {
    private _a: char = c'a'
    get a(): char { return this._a }
}
class Bchar extends Achar {}
final class Cchar extends Bchar {
    b: char = super.a
}

class Abool {
    private _a: boolean = true
    get a(): boolean { return this._a }
}
class Bbool extends Abool {}
final class Cbool extends Bbool {
    b: boolean = super.a
}

class Astr {
    private _a: string = "some string"
    get a(): string { return this._a }
}
class Bstr extends Astr {}
final class Cstr extends Bstr {
    b: string = super.a
}

class ABoxByte {
    private _a: Byte = new Byte((2).toByte())
    get a(): Byte { return this._a }
}
class BBoxByte extends ABoxByte {}
final class CBoxByte extends BBoxByte {
    b: Byte = super.a
}

class ABoxShort {
    private _a: Short = new Short((2).toShort())
    get a(): Short { return this._a }
}
class BBoxShort extends ABoxShort {}
final class CBoxShort extends BBoxShort {
    b: Short = super.a
}

class ABoxInt {
    private _a: Int = new Int(2)
    get a(): Int { return this._a }
}
class BBoxInt extends ABoxInt {}
final class CBoxInt extends BBoxInt {
    b: Int = super.a
}

class ABoxLong {
    private _a: Long = new Long(2)
    get a(): Long { return this._a }
}
class BBoxLong extends ABoxLong {}
final class CBoxLong extends BBoxLong {
    b: Long = super.a
}

class ABoxFloat {
    private _a: Float = new Float(1.2)
    get a(): Float { return this._a }
}
class BBoxFloat extends ABoxFloat {}
final class CBoxFloat extends BBoxFloat {
    b: Float = super.a
}

class ABoxDouble {
    private _a: Double = new Double(1.2)
    get a(): Double { return this._a }
}
class BBoxDouble extends ABoxDouble {}
final class CBoxDouble extends BBoxDouble {
    b: Double = super.a
}

class ABoxChar {
    private _a: Char = new Char(c'a')
    get a(): Char { return this._a }
}
class BBoxChar extends ABoxChar {}
final class CBoxChar extends BBoxChar {
    b: Char = super.a
}

class ABoxBool {
    private _a: Boolean = new Boolean(true)
    get a(): Boolean { return this._a }
}
class BBoxBool extends ABoxBool {}
final class CBoxBool extends BBoxBool {
    b: Boolean = super.a
}

class ABoxStr {
    private _a: String = new String("some string")
    get a(): String { return this._a }
}
class BBoxStr extends ABoxStr {}
final class CBoxStr extends BBoxStr {
    b: String = super.a
}

class AIface {
    private _a: IName = { name: "Bob" }
    get a(): IName { return this._a }
}
class BIface extends AIface {}
final class CIface extends BIface {
    b: IName = super.a
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Field initialized by super accessor across types")

    suite.addTest("byte", () => {
        let instance: Cbyte = new Cbyte
        arktest.assertTrue(instance.b == 2)
    })
    suite.addTest("short", () => {
        let instance: Cshort = new Cshort
        arktest.assertTrue(instance.b == 2)
    })
    suite.addTest("long", () => {
        let instance: Clong = new Clong
        arktest.assertTrue(instance.b == 2)
    })
    suite.addTest("float", () => {
        let instance: Cfloat = new Cfloat
        arktest.assertTrue(instance.b == 1.2f.toFloat())
    })
    suite.addTest("double", () => {
        let instance: Cdouble = new Cdouble
        arktest.assertTrue(instance.b == 2)
    })
    suite.addTest("number", () => {
        let instance: Cnumber = new Cnumber
        arktest.assertTrue(instance.b == 2)
    })
    suite.addTest("bigint", () => {
        let instance: Cbigint = new Cbigint
        arktest.assertTrue(instance.b == new BigInt(Long.MAX_VALUE * 2))
    })
    suite.addTest("char", () => {
        let instance: Cchar = new Cchar
        arktest.assertTrue(instance.b == c'a')
    })
    suite.addTest("boolean", () => {
        let instance: Cbool = new Cbool
        arktest.assertTrue(instance.b == true)
    })
    suite.addTest("string", () => {
        let instance: Cstr = new Cstr
        arktest.assertTrue(instance.b == "some string")
    })
    suite.addTest("Byte boxed", () => {
        let instance: CBoxByte = new CBoxByte
        arktest.assertTrue(instance.b == 2)
    })
    suite.addTest("Short boxed", () => {
        let instance: CBoxShort = new CBoxShort
        arktest.assertTrue(instance.b == 2)
    })
    suite.addTest("Int boxed", () => {
        let instance: CBoxInt = new CBoxInt
        arktest.assertTrue(instance.b == 2)
    })
    suite.addTest("Long boxed", () => {
        let instance: CBoxLong = new CBoxLong
        arktest.assertTrue(instance.b == 2)
    })
    suite.addTest("Float boxed", () => {
        let instance: CBoxFloat = new CBoxFloat
        arktest.assertTrue(instance.b == new Float(1.2))
    })
    suite.addTest("Double boxed", () => {
        let instance: CBoxDouble = new CBoxDouble
        arktest.assertTrue(instance.b == 1.2)
    })
    suite.addTest("Char boxed", () => {
        let instance: CBoxChar = new CBoxChar
        arktest.assertTrue(instance.b == c'a')
    })
    suite.addTest("Boolean boxed", () => {
        let instance: CBoxBool = new CBoxBool
        arktest.assertTrue(instance.b == true)
    })
    suite.addTest("String boxed", () => {
        let instance: CBoxStr = new CBoxStr
        arktest.assertTrue(instance.b == "some string")
    })
    suite.addTest("interface IName", () => {
        let instance: CIface = new CIface
        arktest.assertTrue(instance.b.name == "Bob")
    })

    return suite.run()
}
