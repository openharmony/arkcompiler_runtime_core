/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

class BoolObj {
    isTrue: boolean = true;
    isFalse: boolean = false;
}

class NullObj {
    val: Object | null = null;
}

class FloatObj {
    pi: number = 0;
}

class StringObj {
    msg: string = '';
}

class ArrayObj {
    list: number[] = [];
}

class NestedInner {
    innerVal: string = '';
}

class NestedOuter {
    out: NestedInner = new NestedInner();
}

class PathObj {
    path: string = '';
}

class UndefinedTest {
    visible: number = 1;
    hidden: Object | undefined = undefined;
}

class MaxIntObj {
    max: number = Number.MAX_SAFE_INTEGER;
}

interface StringAndNumberFields {
    a: string
    b: number
}

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.stringify primitives, objects, and wrappers')
    suite.addTest('number array [EPSILON, MAX_VALUE, MIN_VALUE, 0]', arrNumberEdgeValues)
    suite.addTest('number array in hex [0x0102, 0xCAFEBABE, 0xDEAD]', arrHexValues)
    suite.addTest('[0o17, 0b1010, 0o101]', arr0o170b10100o101)
    suite.addTest('number array [2**3, 3**2, 10**6, Math.pow(2,10)]', arrExponentValues)
    suite.addTest('number array [0, 0, 0, 1, 1, NaN, NaN, Infinity]', arrRepeatedNaNInf)
    suite.addTest('[c\'A\']', arrCA)
    suite.addTest('[c\'H\', c\'e\', c\'l\', c\'l...', arrCHello)
    suite.addTest('[c\'A\', c\'A\', c\'B\', c\'B...', arrCAABBa)
    suite.addTest('BoolObj isTrue=true isFalse=false', boolObjTrueFalse)
    suite.addTest('{ val: null } as NullObj', asObjVal)
    suite.addTest('FloatObj pi=3.14159', floatObjPi)
    suite.addTest('{} as Object', emptyObject)
    suite.addTest('{ msg: \'Hello\\nWorld\' ...', asObjMsg)
    suite.addTest('ArrayObj list=[1,2,3]', arrayObj123)
    suite.addTest('{ out: { innerVal: \'va...', asObjOutInnerval)
    suite.addTest('new UndefinedTest()', undefinedTest)
    suite.addTest('new MaxIntObj()', maxIntObj)
    suite.addTest('{ path: \'C:\\\\Windows\' ...', asObjPathC)
    suite.addTest('\'test\'', sTest)
    suite.addTest('123', n123)
    suite.addTest('(42).toByte()', toByteN421)
    suite.addTest('(0).toByte()', toByteN0)
    suite.addTest('(-42).toByte()', toByteN422)
    suite.addTest('(-128).toByte()', toByteN128)
    suite.addTest('(127).toByte()', toByteN127)
    suite.addTest('byte array via .toByte() [-127..127]', arrByteConversions)
    suite.addTest('(30000).toShort()', toShortN30000)
    suite.addTest('(0).toShort()', toShortN0)
    suite.addTest('(-15000).toShort()', toShortN15000)
    suite.addTest('(-32768).toShort()', toShortN32768)
    suite.addTest('(32767).toShort()', toShortN32767)
    suite.addTest('short array via .toShort() [-32767..32767]', arrShortConversions)
    suite.addTest('(1234567890).toInt()', toIntN1234567890)
    suite.addTest('(0).toInt()', toIntN0)
    suite.addTest('(-987654321).toInt()', toIntN987654321)
    suite.addTest('(-2147483648).toInt()', toIntN2147483648)
    suite.addTest('(2147483647).toInt()', toIntN2147483647)
    suite.addTest('int array via .toInt() [-2147483647..2147483647]', arrIntConversions)
    suite.addTest('9223372036854775807n', bigInt9223372036854775807)
    suite.addTest('0n', bigInt0)
    suite.addTest('-9223372036854775808n', bigInt9223372036854775808)
    suite.addTest('-4611686018427387904n', bigInt4611686018427387904)
    suite.addTest('-2147483648n', bigInt2147483648)
    suite.addTest('2147483647n', bigInt2147483647)
    suite.addTest('4611686018427387903n', bigInt4611686018427387903)
    suite.addTest('9223372036854775806n', bigInt9223372036854775806)
    suite.addTest('(3.14159).toFloat()', toFloatN314159)
    suite.addTest('(0.0).toFloat()', toFloatN001)
    suite.addTest('(-2.71828).toFloat()', toFloatN271828)
    suite.addTest('Double.toFloat(3.4028234663852886e38)', doubletoFloatN34028234663852886e38)
    suite.addTest('(1.40129846432481707e-45).toFloat()', toFloatN140129846432481707e45)
    suite.addTest('Double.toFloat(Infinity)', doubletoFloatInfinity)
    suite.addTest('Double.toFloat(-Infinity)', doubletoFloatNegInfinity)
    suite.addTest('Double.toFloat(NaN)', doubletoFloatNaN)
    suite.addTest('Double.toFloat(1.2e-10)', doubletoFloatN12e10)
    suite.addTest('Double.toFloat(2.5e-5)', doubletoFloatN25e5)
    suite.addTest('Double.toFloat(3.4e38)', doubletoFloatN34e38)
    suite.addTest('Double.toFloat(1.5e-38)', doubletoFloatN15e38)
    suite.addTest('Double.toFloat(0.123456789)', doubletoFloatN01234567891)
    suite.addTest('Double.toFloat(-0.123456789)', doubletoFloatN01234567892)
    suite.addTest('Double.toFloat(123.456789)', doubletoFloatN1234567891)
    suite.addTest('Double.toFloat(-123.456789)', doubletoFloatN1234567892)
    suite.addTest('(-0.0).toFloat()', toFloatN002)
    suite.addTest('3.141592653589793', n3141592653589793)
    suite.addTest('0.0', n001)
    suite.addTest('-2.718281828459045', n2718281828459045)
    suite.addTest('1.7976931348623157e308', n17976931348623157e308)
    suite.addTest('5e-324', n5e324)
    suite.addTest('1.234567890123456e-10', n1234567890123456e10)
    suite.addTest('2.5e-50', n25e50)
    suite.addTest('1.234567890123456e50', n1234567890123456e50)
    suite.addTest('0.123456789012345', n01234567890123451)
    suite.addTest('-0.123456789012345', n01234567890123452)
    suite.addTest('123456789012.345', n1234567890123451)
    suite.addTest('-123456789012.345', n1234567890123452)
    suite.addTest('-0.0', n002)
    suite.addTest('9007199254740991n', bigInt90071992547409911)
    suite.addTest('-9007199254740991n', bigInt90071992547409912)
    suite.addTest('9223372036854775807123456789n', bigInt9223372036854775807123456789)
    suite.addTest('BigInt(\'160456018070528910107\')', bigInt160456018070528910107)
    suite.addTest('6148914691236517206n', bigInt6148914691236517206)
    suite.addTest('BigInt(\'146042327230769370131...)', bigInt14604232723076937013194123847511)
    suite.addTest('BigInt 123456789012345678901', bigIntLargePos)
    suite.addTest('123456789012345678901234567890n', bigInt123456789012345678901234567890)
    suite.addTest('2n ** 100n', check2n100n)
    suite.addTest('c\'A\'', cA1)
    suite.addTest('c\'a\'', cA2)
    suite.addTest('c\'0\'', c0)
    suite.addTest('c\'9\'', c9)
    suite.addTest('c\'!\'', cSpecial1)
    suite.addTest('c\'@\'', cSpecial2)
    suite.addTest('c\'#\'', cSpecial3)
    suite.addTest('c\'$\'', cSpecial4)
    suite.addTest('c\'%\'', cSpecial5)
    suite.addTest('c\'^\'', cSpecial6)
    suite.addTest('c\'&\'', cSpecial7)
    suite.addTest('c\'*\'', cSpecial8)
    suite.addTest('c\'(\'', cSpecial9)
    suite.addTest('c\')\'', cSpecial10)
    suite.addTest('c\'-\'', cSpecial11)
    suite.addTest('c\'_\'', cSpecial12)
    suite.addTest('c\'+\'', cSpecial13)
    suite.addTest('c\'=\'', cSpecial14)
    suite.addTest('c\' \'', cSpecial15)
    suite.addTest('c\'~\'', cSpecial16)
    suite.addTest('c\'\\\'\'', cSpecial17)
    suite.addTest('!!1', check1)
    suite.addTest('!!\'test\'', test)
    suite.addTest('!!0', check0)
    suite.addTest('!!\'\'', expr)
    suite.addTest('!!null', checkNull)
    suite.addTest('!!NaN', checkNaN)
    suite.addTest('\'😊\'', sEsc930)
    suite.addTest('\'0 NaN "\'', s0NaN)
    suite.addTest('mixed array [0, -0, NaN, Infinity, -Infinity, -0, 1]', arrSignedZeroNaNInfMixed)
    suite.addTest('nested spread array [[1], [2, ...[3, ...[4, 5]]], [6]]', arrNestedSpread)
    suite.addTest('\'Hello\\nWorld\\t!...\'', sHellonWorld)
    suite.addTest('0xCAFEBABE as double', as0xCAFEBABE)
    suite.addTest('0o777 as double', as0o777)
    suite.addTest('{ a: \'Alice\', b: 25 } ...', asObjAB)
    suite.addTest('new Date(2024-01-01T00:00:00.000Z)', date20240101T000000000Z)
    suite.addTest('new String(\'test\')', stringTest)
    return suite.run()
}

function arrNumberEdgeValues(): void {
    const result: string = JSON.stringify([Number.EPSILON, Number.MAX_SAFE_INTEGER, Number.MIN_SAFE_INTEGER])
    arktest.assertEQ(result, '[2.220446049250313e-16,9007199254740991,-9007199254740991]')
}

function arrHexValues(): void {
    const result: string = JSON.stringify([0x0102, 0xCAFEBABE, 0x01020304, 0x3])
    arktest.assertEQ(result, '[258,3405691582,16909060,3]')
}

function arr0o170b10100o101(): void {
    const result: string = JSON.stringify([0o17, 0b1010, 0o101])
    arktest.assertEQ(result, '[15,10,65]')
}

function arrExponentValues(): void {
    const result: string = JSON.stringify([2 ** 3, 3 ** 2, 10 ** 6, Math.pow(2, 10)])
    arktest.assertEQ(result, '[8,9,1000000,1024]')
}

function arrRepeatedNaNInf(): void {
    const result: string = JSON.stringify([0, 0, 0, 1, 1, NaN, NaN, Infinity])
    arktest.assertEQ(result, '[0,0,0,1,1,null,null,null]')
}

function arrCA(): void {
    const result: string = JSON.stringify([c'A'])
    arktest.assertEQ(result, '["A"]')
}

function arrCHello(): void {
    const result: string = JSON.stringify([c'H', c'e', c'l', c'l', c'o'])
    arktest.assertEQ(result, '["H","e","l","l","o"]')
}

function arrCAABBa(): void {
    const result: string = JSON.stringify([c'A', c'A', c'B', c'B', c'a', c'a'])
    arktest.assertEQ(result, '["A","A","B","B","a","a"]')
}

function boolObjTrueFalse(): void {
    const result: string = JSON.stringify({ isTrue: true, isFalse: false } as BoolObj)
    arktest.assertEQ(result, '{"isTrue":true,"isFalse":false}')
}

function asObjVal(): void {
    const result: string = JSON.stringify({ val: null } as NullObj)
    arktest.assertEQ(result, '{"val":null}')
}

function floatObjPi(): void {
    const result: string = JSON.stringify({ pi: 3.14159 } as FloatObj)
    arktest.assertEQ(result, '{"pi":3.14159}')
}

function emptyObject(): void {
    const result: string = JSON.stringify({} as Object)
    arktest.assertEQ(result, '{}')
}

function asObjMsg(): void {
    const result: string = JSON.stringify({ msg: 'Hello\nWorld' } as StringObj)
    arktest.assertEQ(result, '{"msg":"Hello\\nWorld"}')
}

function arrayObj123(): void {
    const result: string = JSON.stringify({ list: [1, 2, 3] } as ArrayObj)
    arktest.assertEQ(result, '{"list":[1,2,3]}')
}

function asObjOutInnerval(): void {
    const result: string = JSON.stringify({ out: { innerVal: 'val' } } as NestedOuter)
    arktest.assertEQ(result, '{"out":{"innerVal":"val"}}')
}

function undefinedTest(): void {
    const result: string = JSON.stringify(new UndefinedTest())
    arktest.assertEQ(result, '{"visible":1}')
}

function maxIntObj(): void {
    const result: string = JSON.stringify(new MaxIntObj())
    arktest.assertEQ(result, `{"max":${Number.MAX_SAFE_INTEGER}}`)
}

function asObjPathC(): void {
    const result: string = JSON.stringify({ path: 'C:\\Windows' } as PathObj)
    arktest.assertEQ(result, '{"path":"C:\\\\Windows"}')
}

function sTest(): void {
    const result: string = JSON.stringify('test')
    arktest.assertEQ(result, '"test"')
}

function n123(): void {
    const result: string = JSON.stringify(123)
    arktest.assertEQ(result, '123')
}

function toByteN421(): void {
    const result: string = JSON.stringify((42).toByte())
    arktest.assertEQ(result, '42')
}

function toByteN0(): void {
    const result: string = JSON.stringify((0).toByte())
    arktest.assertEQ(result, '0')
}

function toByteN422(): void {
    const result: string = JSON.stringify((-42).toByte())
    arktest.assertEQ(result, '-42')
}

function toByteN128(): void {
    const result: string = JSON.stringify((-128).toByte())
    arktest.assertEQ(result, '-128')
}

function toByteN127(): void {
    const result: string = JSON.stringify((127).toByte())
    arktest.assertEQ(result, '127')
}

function arrByteConversions(): void {
    const result: string = JSON.stringify([(-127).toByte(), (-64).toByte(), (-2).toByte(), (2).toByte(), (64).toByte(),
        (126).toByte()])
    arktest.assertEQ(result, '[-127,-64,-2,2,64,126]')
}

function toShortN30000(): void {
    const result: string = JSON.stringify((30000).toShort())
    arktest.assertEQ(result, '30000')
}

function toShortN0(): void {
    const result: string = JSON.stringify((0).toShort())
    arktest.assertEQ(result, '0')
}

function toShortN15000(): void {
    const result: string = JSON.stringify((-15000).toShort())
    arktest.assertEQ(result, '-15000')
}

function toShortN32768(): void {
    const result: string = JSON.stringify((-32768).toShort())
    arktest.assertEQ(result, '-32768')
}

function toShortN32767(): void {
    const result: string = JSON.stringify((32767).toShort())
    arktest.assertEQ(result, '32767')
}

function arrShortConversions(): void {
    const result: string = JSON.stringify([(-32767).toShort(), (-16384).toShort(), (-256).toShort(), (256).toShort(),
        (16384).toShort(), (32766).toShort()])
    arktest.assertEQ(result, '[-32767,-16384,-256,256,16384,32766]')
}

function toIntN1234567890(): void {
    const result: string = JSON.stringify((1234567890).toInt())
    arktest.assertEQ(result, '1234567890')
}

function toIntN0(): void {
    const result: string = JSON.stringify((0).toInt())
    arktest.assertEQ(result, '0')
}

function toIntN987654321(): void {
    const result: string = JSON.stringify((-987654321).toInt())
    arktest.assertEQ(result, '-987654321')
}

function toIntN2147483648(): void {
    const result: string = JSON.stringify((-2147483648).toInt())
    arktest.assertEQ(result, '-2147483648')
}

function toIntN2147483647(): void {
    const result: string = JSON.stringify((2147483647).toInt())
    arktest.assertEQ(result, '2147483647')
}

function arrIntConversions(): void {
    const result: string = JSON.stringify([(-2147483647).toInt(), (-1073741824).toInt(), (-65536).toInt(),
        (65536).toInt(), (1073741823).toInt(), (2147483646).toInt()])
    arktest.assertEQ(result, '[-2147483647,-1073741824,-65536,65536,1073741823,2147483646]')
}

function bigInt9223372036854775807(): void {
    const result: string = JSON.stringify(9223372036854775807n)
    arktest.assertEQ(result, '9223372036854775807')
}

function bigInt0(): void {
    const result: string = JSON.stringify(0n)
    arktest.assertEQ(result, '0')
}

function bigInt9223372036854775808(): void {
    const result: string = JSON.stringify(-9223372036854775808n)
    arktest.assertEQ(result, '-9223372036854775808')
}

function bigInt4611686018427387904(): void {
    const result: string = JSON.stringify(-4611686018427387904n)
    arktest.assertEQ(result, '-4611686018427387904')
}

function bigInt2147483648(): void {
    const result: string = JSON.stringify(-2147483648n)
    arktest.assertEQ(result, '-2147483648')
}

function bigInt2147483647(): void {
    const result: string = JSON.stringify(2147483647n)
    arktest.assertEQ(result, '2147483647')
}

function bigInt4611686018427387903(): void {
    const result: string = JSON.stringify(4611686018427387903n)
    arktest.assertEQ(result, '4611686018427387903')
}

function bigInt9223372036854775806(): void {
    const result: string = JSON.stringify(9223372036854775806n)
    arktest.assertEQ(result, '9223372036854775806')
}

function toFloatN314159(): void {
    const result: string = JSON.stringify((3.14159).toFloat())
    arktest.assertEQ(result, '3.14159')
}

function toFloatN001(): void {
    const result: string = JSON.stringify((0.0).toFloat())
    arktest.assertEQ(result, '0')
}

function toFloatN271828(): void {
    const result: string = JSON.stringify((-2.71828).toFloat())
    arktest.assertEQ(result, '-2.71828')
}

function doubletoFloatN34028234663852886e38(): void {
    const result: string = JSON.stringify(Double.toFloat(3.4028234663852886e38))
    arktest.assertEQ(result, '3.4028235e+38')
}

function toFloatN140129846432481707e45(): void {
    const result: string = JSON.stringify((1.40129846432481707e-45).toFloat())
    arktest.assertEQ(result, '1e-45')
}

function doubletoFloatInfinity(): void {
    const result: string = JSON.stringify(Double.toFloat(Infinity))
    arktest.assertEQ(result, 'null')
}

function doubletoFloatNegInfinity(): void {
    const result: string = JSON.stringify(Double.toFloat(-Infinity))
    arktest.assertEQ(result, 'null')
}

function doubletoFloatNaN(): void {
    const result: string = JSON.stringify(Double.toFloat(NaN))
    arktest.assertEQ(result, 'null')
}

function doubletoFloatN12e10(): void {
    const result: string = JSON.stringify(Double.toFloat(1.2e-10))
    arktest.assertEQ(result, '1.2e-10')
}

function doubletoFloatN25e5(): void {
    const result: string = JSON.stringify(Double.toFloat(2.5e-5))
    arktest.assertEQ(result, '0.000025')
}

function doubletoFloatN34e38(): void {
    const result: string = JSON.stringify(Double.toFloat(3.4e38))
    arktest.assertEQ(result, '3.4e+38')
}

function doubletoFloatN15e38(): void {
    const result: string = JSON.stringify(Double.toFloat(1.5e-38))
    arktest.assertEQ(result, '1.5e-38')
}

function doubletoFloatN01234567891(): void {
    const result: string = JSON.stringify(Double.toFloat(0.123456789))
    arktest.assertEQ(result, '0.12345679')
}

function doubletoFloatN01234567892(): void {
    const result: string = JSON.stringify(Double.toFloat(-0.123456789))
    arktest.assertEQ(result, '-0.12345679')
}

function doubletoFloatN1234567891(): void {
    const result: string = JSON.stringify(Double.toFloat(123.456789))
    arktest.assertEQ(result, '123.45679')
}

function doubletoFloatN1234567892(): void {
    const result: string = JSON.stringify(Double.toFloat(-123.456789))
    arktest.assertEQ(result, '-123.45679')
}

function toFloatN002(): void {
    const result: string = JSON.stringify((-0.0).toFloat())
    arktest.assertEQ(result, '0')
}

function n3141592653589793(): void {
    const result: string = JSON.stringify(3.141592653589793)
    arktest.assertEQ(result, '3.141592653589793')
}

function n001(): void {
    const result: string = JSON.stringify(0.0)
    arktest.assertEQ(result, '0')
}

function n2718281828459045(): void {
    const result: string = JSON.stringify(-2.718281828459045)
    arktest.assertEQ(result, '-2.718281828459045')
}

function n17976931348623157e308(): void {
    const result: string = JSON.stringify(1.7976931348623157e308)
    arktest.assertEQ(result, '1.7976931348623157e+308')
}

function n5e324(): void {
    const result: string = JSON.stringify(5e-324)
    arktest.assertEQ(result, '5e-324')
}

function n1234567890123456e10(): void {
    const result: string = JSON.stringify(1.234567890123456e-10)
    arktest.assertEQ(result, '1.234567890123456e-10')
}

function n25e50(): void {
    const result: string = JSON.stringify(2.5e-50)
    arktest.assertEQ(result, '2.5e-50')
}

function n1234567890123456e50(): void {
    const result: string = JSON.stringify(1.234567890123456e50)
    arktest.assertEQ(result, '1.234567890123456e+50')
}

function n01234567890123451(): void {
    const result: string = JSON.stringify(0.123456789012345)
    arktest.assertEQ(result, '0.123456789012345')
}

function n01234567890123452(): void {
    const result: string = JSON.stringify(-0.123456789012345)
    arktest.assertEQ(result, '-0.123456789012345')
}

function n1234567890123451(): void {
    const result: string = JSON.stringify(123456789012.345)
    arktest.assertEQ(result, '123456789012.345')
}

function n1234567890123452(): void {
    const result: string = JSON.stringify(-123456789012.345)
    arktest.assertEQ(result, '-123456789012.345')
}

function n002(): void {
    const result: string = JSON.stringify(-0.0)
    arktest.assertEQ(result, '0')
}

function bigInt90071992547409911(): void {
    const result: string = JSON.stringify(9007199254740991n)
    arktest.assertEQ(result, '9007199254740991')
}

function bigInt90071992547409912(): void {
    const result: string = JSON.stringify(-9007199254740991n)
    arktest.assertEQ(result, '-9007199254740991')
}

function bigInt9223372036854775807123456789(): void {
    const result: string = JSON.stringify(9223372036854775807123456789n)
    arktest.assertEQ(result, '9223372036854775807123456789')
}

function bigInt160456018070528910107(): void {
    const result: string = JSON.stringify(BigInt('160456018070528910107'))
    arktest.assertEQ(result, '160456018070528910107')
}

function bigInt6148914691236517206(): void {
    const result: string = JSON.stringify(6148914691236517206n)
    arktest.assertEQ(result, '6148914691236517206')
}

function bigInt14604232723076937013194123847511(): void {
    const result: string = JSON.stringify(BigInt('14604232723076937013194123847511'))
    arktest.assertEQ(result, '14604232723076937013194123847511')
}

function bigIntLargePos(): void {
    const result: string = JSON.stringify(123456789012345678901n + 987654321098765432109n)
    arktest.assertEQ(result, '1111111110111111111010')
}

function bigInt123456789012345678901234567890(): void {
    const result: string = JSON.stringify(123456789012345678901234567890n)
    arktest.assertEQ(result, '123456789012345678901234567890')
}

function check2n100n(): void {
    const result: string = JSON.stringify(2n ** 100n)
    arktest.assertEQ(result, '1267650600228229401496703205376')
}

function cA1(): void {
    const result: string = JSON.stringify(c'A')
    arktest.assertEQ(result, '"A"')
}

function cA2(): void {
    const result: string = JSON.stringify(c'a')
    arktest.assertEQ(result, '"a"')
}

function c0(): void {
    const result: string = JSON.stringify(c'0')
    arktest.assertEQ(result, '"0"')
}

function c9(): void {
    const result: string = JSON.stringify(c'9')
    arktest.assertEQ(result, '"9"')
}

function cSpecial1(): void {
    const result: string = JSON.stringify(c'!')
    arktest.assertEQ(result, '"!"')
}

function cSpecial2(): void {
    const result: string = JSON.stringify(c'@')
    arktest.assertEQ(result, '"@"')
}

function cSpecial3(): void {
    const result: string = JSON.stringify(c'#')
    arktest.assertEQ(result, '"#"')
}

function cSpecial4(): void {
    const result: string = JSON.stringify(c'$')
    arktest.assertEQ(result, '"$"')
}

function cSpecial5(): void {
    const result: string = JSON.stringify(c'%')
    arktest.assertEQ(result, '"%"')
}

function cSpecial6(): void {
    const result: string = JSON.stringify(c'^')
    arktest.assertEQ(result, '"^"')
}

function cSpecial7(): void {
    const result: string = JSON.stringify(c'&')
    arktest.assertEQ(result, '"&"')
}

function cSpecial8(): void {
    const result: string = JSON.stringify(c'*')
    arktest.assertEQ(result, '"*"')
}

function cSpecial9(): void {
    const result: string = JSON.stringify(c'(')
    arktest.assertEQ(result, '"("')
}

function cSpecial10(): void {
    const result: string = JSON.stringify(c')')
    arktest.assertEQ(result, '")"')
}

function cSpecial11(): void {
    const result: string = JSON.stringify(c'-')
    arktest.assertEQ(result, '"-"')
}

function cSpecial12(): void {
    const result: string = JSON.stringify(c'_')
    arktest.assertEQ(result, '"_"')
}

function cSpecial13(): void {
    const result: string = JSON.stringify(c'+')
    arktest.assertEQ(result, '"+"')
}

function cSpecial14(): void {
    const result: string = JSON.stringify(c'=')
    arktest.assertEQ(result, '"="')
}

function cSpecial15(): void {
    const result: string = JSON.stringify(c' ')
    arktest.assertEQ(result, '" "')
}

function cSpecial16(): void {
    const result: string = JSON.stringify(c'~')
    arktest.assertEQ(result, '"~"')
}

function cSpecial17(): void {
    const result: string = JSON.stringify(c'\'')
    arktest.assertEQ(result, '"\'"')
}

function check1(): void {
    const result: string = JSON.stringify(!!1)
    arktest.assertEQ(result, 'true')
}

function test(): void {
    const result: string = JSON.stringify(!!'test')
    arktest.assertEQ(result, 'true')
}

function check0(): void {
    const result: string = JSON.stringify(!!0)
    arktest.assertEQ(result, 'false')
}

function expr(): void {
    const result: string = JSON.stringify(!!'')
    arktest.assertEQ(result, 'false')
}

function checkNull(): void {
    const result: string = JSON.stringify(!!null)
    arktest.assertEQ(result, 'false')
}

function checkNaN(): void {
    const result: string = JSON.stringify(!!NaN)
    arktest.assertEQ(result, 'false')
}

function sEsc930(): void {
    const result: string = JSON.stringify('😊')
    arktest.assertEQ(result, '"😊"')
}

function s0NaN(): void {
    const result: string = JSON.stringify('0 NaN "')
    arktest.assertEQ(result, '"0 NaN \\""')
}

function arrSignedZeroNaNInfMixed(): void {
    const result: string = JSON.stringify([0, -0, NaN, Infinity, -Infinity, 42, 3.14, 1e3])
    arktest.assertEQ(result, '[0,0,null,null,null,42,3.14,1000]')
}

function arrNestedSpread(): void {
    const result: string = JSON.stringify([[1], [2, ...[3, ...[4]]]] as number[][])
    arktest.assertEQ(result, '[[1],[2,3,4]]')
}

function sHellonWorld(): void {
    const result: string = JSON.stringify('Hello\nWorld\t!"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~')
    arktest.assertEQ(result, '"Hello\\nWorld\\t!\\"#$%&\'()*+,-./:;<=>?@[\\\\]^_`{|}~"')
}

function as0xCAFEBABE(): void {
    const result: string = JSON.stringify(0xCAFEBABE as double)
    arktest.assertEQ(result, '3405691582')
}

function as0o777(): void {
    const result: string = JSON.stringify(0o777 as double)
    arktest.assertEQ(result, '511')
}

function asObjAB(): void {
    const result: string = JSON.stringify({ a: 'Alice', b: 25 } as StringAndNumberFields)
    arktest.assertEQ(result, '{"a":"Alice","b":25}')
}

function date20240101T000000000Z(): void {
    const result: string = JSON.stringify(new Date('2024-01-01T00:00:00.000Z'))
    arktest.assertEQ(result, '"2024-01-01T00:00:00.000Z"')
}

function stringTest(): void {
    const result: string = JSON.stringify(new String('test'))
    arktest.assertEQ(result, '"test"')
}
