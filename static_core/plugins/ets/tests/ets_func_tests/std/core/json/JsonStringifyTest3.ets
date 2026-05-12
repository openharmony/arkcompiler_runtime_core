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

class SimpleObj {
    name: string = '';
    year: number = 0;
    value: number = 0;
}

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.stringify primitives, objects, and wrappers')
    suite.addTest('float 3.14159 starts with 3.14', floatPi)
    suite.addTest('string with \\n and \\t', escapedNewlineTab)
    suite.addTest('string with \\" and \\\\', quoteBackslash)
    suite.addTest('byte array of 70 elements', byteArray70)
    suite.addTest('emoji character', emojiChar)
    suite.addTest('short array of 70 elements', shortArray70)
    suite.addTest('int array of 60 elements', intArray60)
    suite.addTest('long array of 29 elements', longArray29)
    suite.addTest('float NaN', floatNaN)
    suite.addTest('float scientific array', floatScientific)
    suite.addTest('float pi and e', floatPiE)
    suite.addTest('20 float elements', float20)
    suite.addTest('double scientific array', doubleScientific)
    suite.addTest('20 double elements', double20)
    suite.addTest('10 boolean elements', booleanArray10)
    suite.addTest('mixed extreme values', extremeMixed)
    suite.addTest('float variety', floatVariety)
    suite.addTest('digits 0 to 9', digits0to9)
    suite.addTest('extreme all with content check', extremeAll)
    suite.addTest('4 extreme float values', floatExtreme4)
    suite.addTest('char 0x00 and 0x7F', char0and127)
    suite.addTest('char 0x80, 0xA9, 0xFF', char128to255)
    suite.addTest('char 0x00 and 0xFFFF', charRange)
    suite.addTest('char printable ASCII', charASCII)
    suite.addTest('char control chars', charControl)
    suite.addTest('char Unicode', charUnicode)
    suite.addTest('char Greek and math', charGreekMath)
    suite.addTest('char JSON syntax', charJsonSyntax)
    suite.addTest('SimpleObj with name year value', simpleObj)
    suite.addTest('surrogate pair', surrogatePair)
    suite.addTest('null char A 0x7F', nullChars)
    suite.addTest('escape sequence chars', escapeSequence)
    suite.addTest('special chars string', specialCharsString)
    return suite.run()
}

function floatPi(): void {
    const result: string = JSON.stringify(3.14159)
    arktest.assertTrue(result.startsWith('3.14'))
}

function escapedNewlineTab(): void {
    const result: string = JSON.stringify('Hello\nWorld\tTest')
    arktest.assertTrue(result.indexOf('\\n') >= 0)
    arktest.assertTrue(result.indexOf('\\t') >= 0)
}

function quoteBackslash(): void {
    const result: string = JSON.stringify('Say "Hello" \\Backslash')
    arktest.assertTrue(result.indexOf('\\"') >= 0)
    arktest.assertTrue(result.indexOf('\\\\') >= 0)
}

function byteArray70(): void {
    const arr: FixedArray<byte> = [
        -128, -100, -50, 0, 50, 100, 127,
        -128, -100, -50, 0, 50, 100, 127,
        -128, -100, -50, 0, 50, 100, 127,
        -128, -100, -50, 0, 50, 100, 127,
        -128, -100, -50, 0, 50, 100, 127,
        -128, -100, -50, 0, 50, 100, 127,
        -128, -100, -50, 0, 50, 100, 127,
        -128, -100, -50, 0, 50, 100, 127,
        -128, -100, -50, 0, 50, 100, 127,
        -128, -100, -50, 0, 50, 100, 127
    ]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertEQ(result.split(',').length, 70)
}

function emojiChar(): void {
    const result: string = JSON.stringify('👆')
    arktest.assertTrue(result.startsWith('"'))
    arktest.assertTrue(result.endsWith('"'))
}

function shortArray70(): void {
    const arr: FixedArray<short> = [
        -32768, -10000, -1000, -100, 0, 100, 1000, 10000, 32767,
        -32768, -10000, -1000, -100, 0, 100, 1000, 10000, 32767,
        -32768, -10000, -1000, -100, 0, 100, 1000, 10000, 32767,
        -32768, -10000, -1000, -100, 0, 100, 1000, 10000, 32767,
        -32768, -10000, -1000, -100, 0, 100, 1000, 10000, 32767,
        -32768, -10000, -1000, -100, 0, 100, 1000, 10000, 32767,
        -32768, -10000, -1000, -100, 0, 100, 1000, 10000, 32767,
        1, 2, 3, 4, 5, 6, 7
    ]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertEQ(result.split(',').length, 70)
}

function intArray60(): void {
    const arr: FixedArray<int> = [
        -2147483648, -1000000, -100000, -10000, -1000, -100, -10, 0,
        10, 100, 1000, 10000, 100000, 1000000, 2147483647,
        -2147483648, -1000000, -100000, -10000, -1000, -100, -10, 0,
        10, 100, 1000, 10000, 100000, 1000000, 2147483647,
        -2147483648, -1000000, -100000, -10000, -1000, -100, -10, 0,
        10, 100, 1000, 10000, 100000, 1000000, 2147483647,
        -2147483648, -1000000, -100000, -10000, -1000, -100, -10, 0,
        10, 100, 1000, 10000, 100000, 1000000, 2147483647
    ]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertEQ(result.split(',').length, 60)
}

function longArray29(): void {
    const arr: FixedArray<long> = [
        -9223372036854775808 as long, -1000000000 as long, -1000000 as long, 0 as long,
        1000000 as long, 1000000000 as long, 9223372036854775807 as long,
        -9223372036854775808 as long, -1000000000 as long, -1000000 as long, 0 as long,
        1000000 as long, 1000000000 as long, 9223372036854775807 as long,
        -9223372036854775808 as long, -1000000000 as long, -1000000 as long, 0 as long,
        1000000 as long, 1000000000 as long, 9223372036854775807 as long,
        1 as long, 2 as long, 3 as long, 4 as long, 5 as long, 6 as long, 7 as long, 8 as long
    ]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertEQ(result.split(',').length, 29)
}

function floatNaN(): void {
    const result: string = JSON.stringify([Double.toFloat(NaN)] as FixedArray<float>)
    arktest.assertTrue(result === '[null]' || result === '[0]')
}

function floatScientific(): void {
    const arr: FixedArray<float> = [Double.toFloat(1e10), Double.toFloat(1e-10), Double.toFloat(5.5e20)]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
}

function floatPiE(): void {
    const arr: FixedArray<float> = [Double.toFloat(3.1415926), Double.toFloat(2.7182818)]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result === '[3.141593,2.718282]' || result === '[3.1415925,2.7182817]')
}

function float20(): void {
    const arr: FixedArray<float> = [
        Double.toFloat(1.0), Double.toFloat(2.0), Double.toFloat(3.0), Double.toFloat(4.0), Double.toFloat(5.0),
        Double.toFloat(1.1), Double.toFloat(2.2), Double.toFloat(3.3), Double.toFloat(4.4), Double.toFloat(5.5),
        Double.toFloat(1.2), Double.toFloat(2.3), Double.toFloat(3.4), Double.toFloat(4.5), Double.toFloat(5.6),
        Double.toFloat(1.3), Double.toFloat(2.4), Double.toFloat(3.5), Double.toFloat(4.6), Double.toFloat(5.7)
    ]
    const result: string = JSON.stringify(arr)
    arktest.assertEQ(result.split(',').length, 20)
}

function doubleScientific(): void {
    const arr: FixedArray<double> = [1e10, 1e-10, 1.234567890123456, 1.7976931348623157e+100]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
}

function double20(): void {
    const arr: FixedArray<double> = [
        1.1, 2.2, 3.3, 4.4, 5.5,
        1.01, 2.02, 3.03, 4.04, 5.05,
        1.001, 2.002, 3.003, 4.004, 5.005,
        1.0001, 2.0002, 3.0003, 4.0004, 5.0005
    ]
    const result: string = JSON.stringify(arr)
    arktest.assertEQ(result.split(',').length, 20)
}

function booleanArray10(): void {
    const arr: FixedArray<boolean> = [true, false, true, false, true, false, true, false, true, false]
    const result: string = JSON.stringify(arr)
    arktest.assertEQ(result.split(',').length, 10)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
}

function extremeMixed(): void {
    const arr: Any[] = [-128, 127, -32768, 32767, -2147483648, 2147483647, -9223372036854775808, 9223372036854775807]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
}

function floatVariety(): void {
    const arr: Any[] = [1.40129846432481707e-45, 3.40282346638528860e+38, 1e10, 1e-10, 2.5e-2, 1e3]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.indexOf('1e+10') >= 0 || result.indexOf('10000000000') >= 0)
    arktest.assertTrue(result.indexOf('0.025') >= 0)
}

function digits0to9(): void {
    const arr: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    const result: string = JSON.stringify(arr)
    arktest.assertEQ(result.split(',').length, 10)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
}

function extremeAll(): void {
    const arr: Any[] = [-128, 127, -32768, 32767, -2147483648, 2147483647, -9223372036854775808, 9223372036854775807]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertTrue(result.indexOf('-128') >= 0)
    arktest.assertTrue(result.indexOf('127') >= 0)
    arktest.assertTrue(result.indexOf('32767') >= 0)
    arktest.assertTrue(result.indexOf('2147483647') >= 0)
}

function floatExtreme4(): void {
    const arr: Any[] = [1.40129846432481707e-45, 3.40282346638528860e+38, -3.40282346638528860e+38, 1.17549435e-38]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertEQ(result.split(',').length, 4)
}

function char0and127(): void {
    const arr: string[] = [String.fromCharCode(0x0000), String.fromCharCode(0x007F)]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertEQ(result.split(',').length, 2)
}

function char128to255(): void {
    const arr: string[] = [String.fromCharCode(0x0080), String.fromCharCode(0x00A9), String.fromCharCode(0x00FF)]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertEQ(result.split(',').length, 3)
}

function charRange(): void {
    const arr: string[] = [String.fromCharCode(0x0000), String.fromCharCode(0xFFFF)]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
}

function charASCII(): void {
    const arr: string[] = [String.fromCharCode(0x20), String.fromCharCode(0x30),
        String.fromCharCode(0x41), String.fromCharCode(0x61),
        String.fromCharCode(0x7A), String.fromCharCode(0x7E)]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.indexOf('" "') >= 0)
    arktest.assertTrue(result.indexOf('"0"') >= 0)
    arktest.assertTrue(result.indexOf('"A"') >= 0)
    arktest.assertTrue(result.indexOf('"a"') >= 0)
    arktest.assertEQ(result.split(',').length, 6)
}

function charControl(): void {
    const arr: string[] = [String.fromCharCode(0x09), String.fromCharCode(0x0A), String.fromCharCode(0x0D)]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertEQ(result.split(',').length, 3)
}

function charUnicode(): void {
    const arr: string[] = [String.fromCharCode(0x20AC), String.fromCharCode(0x4E2D), String.fromCharCode(0x65E5),
        String.fromCharCode(0xD800), String.fromCharCode(0xDBFF), String.fromCharCode(0xDC00),
        String.fromCharCode(0xDFFF)]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertEQ(result.split(',').length, 7)
}

function charGreekMath(): void {
    const arr: string[] = [String.fromCharCode(0x0100), String.fromCharCode(0x03B1), String.fromCharCode(0x03A9),
        String.fromCharCode(0x222B), String.fromCharCode(0x2248), String.fromCharCode(0x2260)]
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertEQ(result.split(',').length, 6)
}

function charJsonSyntax(): void {
    const arr: string[] = ['"', ',', ':', '[', ']', '{', '}', '\'']
    const result: string = JSON.stringify(arr)
    arktest.assertTrue(result.startsWith('['))
    arktest.assertTrue(result.endsWith(']'))
    arktest.assertTrue(result.length > 14)
}

function simpleObj(): void {
    const result: string = JSON.stringify({ name: 'ArkTS', year: 2025, value: 0 } as SimpleObj)
    arktest.assertTrue(result.includes('"name":"ArkTS"'))
    arktest.assertTrue(result.includes('"year":2025'))
}

function surrogatePair(): void {
    const result: string = JSON.stringify(String.fromCharCode(0xD83D, 0xDFFF))
    arktest.assertTrue(result.startsWith('"'))
    arktest.assertTrue(result.endsWith('"'))
    arktest.assertTrue(result.length > 2)
}

function nullChars(): void {
    const result: string = JSON.stringify(String.fromCharCode(0x0000) + 'A' + String.fromCharCode(0x007F))
    arktest.assertTrue(result.startsWith('"'))
    arktest.assertTrue(result.endsWith('"'))
    arktest.assertTrue(result.length > 5)
}

function escapeSequence(): void {
    const result: string = JSON.stringify(String.fromCharCode(92, 34, 92, 92, 92, 98, 92, 102, 92, 110, 92, 114, 92,
        116, 92, 34))
    arktest.assertTrue(result.split('\\\\').length > 5)
}

function specialCharsString(): void {
    const result: string = JSON.stringify('!!""##$$%%&&\'\'(())**++,,--..//::;;<<==>>??@@[[^^__``{{||}}~~')
    arktest.assertTrue(result.contains('\\"', 0))
    arktest.assertTrue(result.length > 61)
}
