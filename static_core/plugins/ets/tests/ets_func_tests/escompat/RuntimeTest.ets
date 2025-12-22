/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

function testZero(): void {
    arktest.assertTrue(Runtime.sameValue(0.0, 0.0))
    arktest.assertFalse(Runtime.sameValue(0.0, -0.0))
    arktest.assertFalse(Runtime.sameValue(-0.0, 0.0))
    arktest.assertTrue(Runtime.sameValue(-0.0, -0.0))

    arktest.assertTrue(Runtime.sameValueZero(0.0, -0.0))
    arktest.assertTrue(Runtime.sameValueZero(-0.0, 0.0))
}

function testChar(): void {
    let x1 : char = c'\a'
    let x2 : char = c'\a'
    let y : char = c'\b'
    arktest.assertTrue(Runtime.sameValueZero(x1, x2))
    arktest.assertFalse(Runtime.sameValueZero(x2, y))
}

function testBoolean(): void {
    let x1 : boolean = true
    let x2 : boolean = true
    let y : boolean = false
    arktest.assertTrue(Runtime.sameValueZero(x1, x2))
    arktest.assertFalse(Runtime.sameValueZero(x2, y))
}

function testByte(): void {
    let x1 : byte = 127
    let x2 : byte = 127
    let y : byte = 125
    let z : byte = -127
    arktest.assertTrue(Runtime.sameValueZero(x1, x2))
    arktest.assertFalse(Runtime.sameValueZero(x1, y))
    arktest.assertFalse(Runtime.sameValueZero(x2, z))
}

function testShort(): void {
    let x1 : short = 12357
    let x2 : short = 12357
    let y : short = 12355
    let z : short = -12357
    arktest.assertTrue(Runtime.sameValueZero(x1, x2))
    arktest.assertFalse(Runtime.sameValueZero(x1, y))
    arktest.assertFalse(Runtime.sameValueZero(x2, z))
}

function testLong(): void {
    let x1 : long = 1235789
    let x2 : long = 1235789
    let y : long = 1235785
    let z : long = -1235789
    arktest.assertTrue(Runtime.sameValueZero(x1, x2))
    arktest.assertFalse(Runtime.sameValueZero(x1, y))
    arktest.assertFalse(Runtime.sameValueZero(x2, z))
}

function testInt(): void {
    let x1 : int = 12357890
    let x2 : int = 12357890
    let y : int = 12357895
    let z : int = -12357890
    arktest.assertTrue(Runtime.sameValueZero(x1, x2))
    arktest.assertFalse(Runtime.sameValueZero(x1, y))
    arktest.assertFalse(Runtime.sameValueZero(x2, z))
}

function testFloat(): void {
    let x1 : float = 3.4e+38
    let x2 : float = 3.4e+38
    let y : float = 2.8e+24
    let z : float = -3.4e+38
    arktest.assertTrue(Runtime.sameValueZero(x1, x2))
    arktest.assertFalse(Runtime.sameValueZero(x1, y))
    arktest.assertFalse(Runtime.sameValueZero(x2, z))
}

function testDouble(): void {
    let x1 : double = 1.7e+308
    let x2 : double = 1.7e+308
    let y : double = 0.4e+105
    let z : double = -1.7e+308
    arktest.assertTrue(Runtime.sameValueZero(x1, x2))
    arktest.assertFalse(Runtime.sameValueZero(x1, y))
    arktest.assertFalse(Runtime.sameValueZero(x2, z))
}

function testBigInt(): void {
    let x1 : bigint = BigInt('123456789012345678901234567890')
    let x2 : bigint = BigInt('123456789012345678901234567890')
    let y : bigint = BigInt('123456789012345678901234567895')
    let z : bigint = BigInt('-123456789012345678901234567890')
    arktest.assertTrue(Runtime.sameValueZero(x1, x2))
    arktest.assertFalse(Runtime.sameValueZero(x1, y))
    arktest.assertFalse(Runtime.sameValueZero(x2, z))
}

function testString(): void {
    let x1 : string = "string X"
    let x2 : string = "string X"
    let y : string = "string Y"
    arktest.assertTrue(Runtime.sameValueZero(x1, x2))
    arktest.assertFalse(Runtime.sameValueZero(x1, y))
}

interface ObjectI {
    x: number;
    y: number;
}

class ObjectC {
    id: int
    name: string

    constructor(id: int, name: string) {
        this.id = id
        this.name = name
    }
}

function testObject(): void {
    let x1 : ObjectI = {x: 12357, y: -12357}
    let x2 : ObjectI = {x: 12357, y: -12357}
    let y = x1
    arktest.assertTrue(Runtime.sameValueZero(x1, x1))
    arktest.assertFalse(Runtime.sameValueZero(x1, x2))
    arktest.assertTrue(Runtime.sameValueZero(x1, y))

    let w1 = new ObjectC(1, "ObjectC")
    let w2 = new ObjectC(1, "ObjectC")
    let v = w1
    arktest.assertTrue(Runtime.sameValueZero(w1, w1))
    arktest.assertFalse(Runtime.sameValueZero(w1, w2))
    arktest.assertTrue(Runtime.sameValueZero(w1, v))
}

enum Color1 {Red, Green, Blue}
enum Color2 {Red = "Red", Green = "Green", Blue = "Blue"}
enum Animal {Cat, Dog, Cow}

function testEnum(): void {
    let x : Color1 = Color1.Green
    let y : Color2 = Color2.Green
    let z : Animal = Animal.Dog
    arktest.assertTrue(Runtime.sameValueZero(x, Color1.Green))
    arktest.assertTrue(Runtime.sameValueZero(x, 1))
    arktest.assertFalse(Runtime.sameValueZero(x, "Green"))
    arktest.assertFalse(Runtime.sameValueZero(y, 1))
    arktest.assertTrue(Runtime.sameValueZero(y, "Green"))
    arktest.assertTrue(Runtime.sameValueZero(z, 1))
    arktest.assertTrue(Runtime.sameValueZero(x, z))
    arktest.assertFalse(Runtime.sameValueZero(y, z))
    arktest.assertTrue(Runtime.sameValueZero(Color1.Green, Animal.Dog))
}

function makeFloat64NaN(payload: number): number {
    const buf = new ArrayBuffer(8);
    const view = new DataView(buf);
    const hi = (0x7FF << 20) | ((payload >> 32) & 0x800FFFFF);
    const lo = payload & 0xFFFFFFFF;
    view.setUint32(0, lo, true);
    view.setUint32(4, hi, true);
    return view.getFloat64(0, true);
}

function makeFloat32NaN(payload: number): number {
    const buf = new ArrayBuffer(4);
    const view = new DataView(buf);
    const hi = (0xFF << 7) | ((payload >> 16) & 0x803F);
    const lo = payload & 0xFFFF;
    view.setUint16(0, lo, true);
    view.setUint16(2, hi, true);
    return view.getFloat32(0, true);
}

function testNaN(): void {
    let f1 = makeFloat32NaN(1)
    let f2 = makeFloat32NaN(2)
    let d1 = makeFloat64NaN(3)
    let d2 = makeFloat64NaN(4)
    arktest.assertTrue(isNaN(f1))
    arktest.assertTrue(isNaN(f2))
    arktest.assertTrue(isNaN(d1))
    arktest.assertTrue(isNaN(d2))
    arktest.assertTrue(Runtime.sameValueZero(f1, f2))
    arktest.assertTrue(Runtime.sameValueZero(d1, d2))
    arktest.assertTrue(Runtime.sameValueZero(f1, d1))
}

function testUndefinedNull(): void {
    let u1 : undefined | number
    let u2 : undefined | ObjectC
    arktest.assertTrue(u1 == undefined)
    arktest.assertTrue(u2 == undefined)
    arktest.assertTrue(Runtime.sameValueZero(u1, undefined))
    arktest.assertTrue(Runtime.sameValueZero(u1, u2))

    let n1 : null | number
    let n2 : null | ObjectC
    n1 = null
    n2 = null
    arktest.assertTrue(Runtime.sameValueZero(n1, null))
    arktest.assertTrue(Runtime.sameValueZero(n1, n2))
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("Runtime.sameValueZero tests")
    testSuite.addTest("Runtime.sameValueZero zero", testZero)
    testSuite.addTest("Runtime.sameValueZero char", testChar)
    testSuite.addTest("Runtime.sameValueZero boolean", testBoolean)
    testSuite.addTest("Runtime.sameValueZero byte", testByte)
    testSuite.addTest("Runtime.sameValueZero short", testShort)
    testSuite.addTest("Runtime.sameValueZero long", testLong)
    testSuite.addTest("Runtime.sameValueZero int", testInt)
    testSuite.addTest("Runtime.sameValueZero float", testFloat)
    testSuite.addTest("Runtime.sameValueZero double", testDouble)
    testSuite.addTest("Runtime.sameValueZero bigint", testBigInt)
    testSuite.addTest("Runtime.sameValueZero string", testString)
    testSuite.addTest("Runtime.sameValueZero object", testObject)
    testSuite.addTest("Runtime.sameValueZero enum", testEnum)
    testSuite.addTest("Runtime.sameValueZero NaN", testNaN)
    testSuite.addTest("Runtime.sameValueZero undefined, null", testUndefinedNull)
    return testSuite.run()
}
