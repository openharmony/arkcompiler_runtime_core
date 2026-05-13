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

class AByte    { static a: byte    = (2).toByte() }
class AShort   { static a: short   = (2).toShort() }
class ALong    { static a: long    = 2 }
class AFloat   { static a: float   = 1.2f }
class ADouble  { static a: double  = 2 }
class AChar    { static a: char    = c'a' }
class ABool    { static a: boolean = true }
class AStr     { static a: string  = "some string" }
class ABoxByte    { static a: Byte    = new Byte((2).toByte()) }
class ABoxShort   { static a: Short   = new Short((2).toShort()) }
class ABoxInt     { static a: Int     = new Int(2) }
class ABoxLong    { static a: Long    = new Long(2) }
class ABoxFloat   { static a: Float   = new Float(1.2) }
class ABoxDouble  { static a: Double  = new Double(1.2) }
class ABoxChar    { static a: Char    = new Char(c'a') }
class ABoxBool    { static a: Boolean = new Boolean(true) }
class ABoxStr     { static a: String  = new String("some string") }

function main(): int {
    let suite = new arktest.ArkTestsuite("Static field access initialized by literal and boxed values")

    suite.addTest("byte primitive", () => {
        let instance: AByte = new AByte()
        arktest.assertTrue(AByte.a == 2)
    })
    suite.addTest("short primitive", () => {
        let instance: AShort = new AShort()
        arktest.assertTrue(AShort.a == 2)
    })
    suite.addTest("long primitive", () => {
        let instance: ALong = new ALong()
        arktest.assertTrue(ALong.a == 2)
    })
    suite.addTest("float primitive", () => {
        let instance: AFloat = new AFloat()
        arktest.assertTrue(AFloat.a == 1.2f)
    })
    suite.addTest("double primitive", () => {
        let instance: ADouble = new ADouble()
        arktest.assertTrue(ADouble.a == 2)
    })
    suite.addTest("char primitive", () => {
        let instance: AChar = new AChar()
        arktest.assertTrue(AChar.a == c'a')
    })
    suite.addTest("boolean primitive", () => {
        let instance: ABool = new ABool()
        arktest.assertTrue(ABool.a == true)
    })
    suite.addTest("string primitive", () => {
        let instance: AStr = new AStr()
        arktest.assertTrue(AStr.a == "some string")
    })
    suite.addTest("Byte boxed", () => {
        let instance: ABoxByte = new ABoxByte()
        arktest.assertTrue(ABoxByte.a == 2)
    })
    suite.addTest("Short boxed", () => {
        let instance: ABoxShort = new ABoxShort()
        arktest.assertTrue(ABoxShort.a == 2)
    })
    suite.addTest("Int boxed", () => {
        let instance: ABoxInt = new ABoxInt()
        arktest.assertTrue(ABoxInt.a == 2)
    })
    suite.addTest("Long boxed", () => {
        let instance: ABoxLong = new ABoxLong()
        arktest.assertTrue(ABoxLong.a == 2)
    })
    suite.addTest("Float boxed", () => {
        let instance: ABoxFloat = new ABoxFloat()
        arktest.assertTrue(ABoxFloat.a == 1.2f)
    })
    suite.addTest("Double boxed", () => {
        let instance: ABoxDouble = new ABoxDouble()
        arktest.assertTrue(ABoxDouble.a == 1.2)
    })
    suite.addTest("Char boxed", () => {
        let instance: ABoxChar = new ABoxChar()
        arktest.assertTrue(ABoxChar.a == c'a')
    })
    suite.addTest("Boolean boxed", () => {
        let instance: ABoxBool = new ABoxBool()
        arktest.assertTrue(ABoxBool.a == true)
    })
    suite.addTest("String boxed", () => {
        let instance: ABoxStr = new ABoxStr()
        arktest.assertTrue(ABoxStr.a == "some string")
    })

    return suite.run()
}
