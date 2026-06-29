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

function main(): int {
    let suite = new arktest.ArkTestsuite('toString(radix) tests')

    suite.addTest('Char c\\n toString radix 2', testCharNewlineToStringRadix2)
    suite.addTest("Char 'd' toString radix 8", testCharDToStringRadix8)
    suite.addTest("Char 'A' toString radix 10", testCharAToStringRadix10)
    suite.addTest("Char 'ÿ' toString radix 16", testCharUmlautYToStringRadix16)
    suite.addTest("Char '#' toString radix 36", testCharHashToStringRadix36)

    suite.addTest('Byte 64 toString radix 16', testByte64ToStringRadix16)
    suite.addTest('Byte 127 toString radix 10', testByte127ToStringRadix10)
    suite.addTest('Byte 42 toString radix 2', testByte42ToStringRadix2)
    suite.addTest('Byte 100 toString radix 8', testByte100ToStringRadix8)
    suite.addTest('Byte 35 toString radix 36', testByte35ToStringRadix36)
    suite.addTest('Byte 0 toString radix 10', testByte0ToStringRadix10)
    suite.addTest('Byte 1 toString radix 36', testByte1ToStringRadix36)
    suite.addTest('Byte -128 toString radix 10', testByteMinus128ToStringRadix10)
    suite.addTest('Byte -35 toString radix 36', testByteMinus35ToStringRadix36)
    suite.addTest('Byte -1 toString radix 10', testByteMinus1ToStringRadix10)

    suite.addTest('Short 32767 toString radix 16', testShort32767ToStringRadix16)
    suite.addTest('Short 1024 toString radix 2', testShort1024ToStringRadix2)
    suite.addTest('Short 4096 toString radix 8', testShort4096ToStringRadix8)
    suite.addTest('Short 1295 toString radix 36', testShort1295ToStringRadix36)
    suite.addTest('Short 0 toString radix 10', testShort0ToStringRadix10)
    suite.addTest('Short 35 toString radix 36', testShort35ToStringRadix36)
    suite.addTest('Short -32768 toString radix 10', testShortMinus32768ToStringRadix10)
    suite.addTest('Short -1295 toString radix 36', testShortMinus1295ToStringRadix36)
    suite.addTest('Short -1 toString radix 10', testShortMinus1ToStringRadix10)

    suite.addTest('Int 255 toString radix 2', testInt255ToStringRadix2)
    suite.addTest('Int 511 toString radix 8', testInt511ToStringRadix8)
    suite.addTest('Int 65535 toString radix 16', testInt65535ToStringRadix16)
    suite.addTest('Int 46655 toString radix 36', testInt46655ToStringRadix36)
    suite.addTest('Int 0 toString radix 10', testInt0ToStringRadix10)
    suite.addTest('Int 35 toString radix 36', testInt35ToStringRadix36)
    suite.addTest('Int 1295 toString radix 36', testInt1295ToStringRadix36)
    suite.addTest('Int -46655 toString radix 36', testIntMinus46655ToStringRadix36)
    suite.addTest('Int -1295 toString radix 36', testIntMinus1295ToStringRadix36)
    suite.addTest('Int -1 toString radix 10', testIntMinus1ToStringRadix10)
    suite.addTest('Int 1000 toString radix 10', testInt1000ToStringRadix10)

    suite.addTest('Long 1023 toString radix 2', testLong1023ToStringRadix2)
    suite.addTest('Long 2097151 toString radix 8', testLong2097151ToStringRadix8)
    suite.addTest('Long 4294967295 toString radix 16', testLong4294967295ToStringRadix16)
    suite.addTest('Long 1679615 toString radix 36', testLong1679615ToStringRadix36)
    suite.addTest('Long 0 toString radix 10', testLong0ToStringRadix10)
    suite.addTest('Long 35 toString radix 36', testLong35ToStringRadix36)
    suite.addTest('Long 1295 toString radix 36', testLong1295ToStringRadix36)
    suite.addTest('Long 46655 toString radix 36', testLong46655ToStringRadix36)
    suite.addTest('Long -1679615 toString radix 36', testLongMinus1679615ToStringRadix36)
    suite.addTest('Long -46655 toString radix 36', testLongMinus46655ToStringRadix36)
    suite.addTest('Long -1295 toString radix 36', testLongMinus1295ToStringRadix36)
    suite.addTest('Long -1 toString radix 10', testLongMinus1ToStringRadix10)
    suite.addTest('Long MIN_VALUE toString radix 2', testLongMinValueToStringRadix2)
    suite.addTest('Long MIN_VALUE toString radix 10', testLongMinValueToStringRadix10)
    suite.addTest('Long MIN_VALUE toString radix 16', testLongMinValueToStringRadix16)
    suite.addTest('Long MIN_VALUE toString radix 36', testLongMinValueToStringRadix36)
    suite.addTest('Long MAX_VALUE toString radix 2', testLongMaxValueToStringRadix2)
    suite.addTest('Long MAX_VALUE toString radix 10', testLongMaxValueToStringRadix10)
    suite.addTest('Long MAX_VALUE toString radix 16', testLongMaxValueToStringRadix16)
    suite.addTest('Long MAX_VALUE toString radix 36', testLongMaxValueToStringRadix36)

    suite.addTest('Byte 64 toString radix double 16.0', testByte64ToStringRadixDouble16)
    suite.addTest('Byte 127 toString radix double 10.0', testByte127ToStringRadixDouble10)
    suite.addTest('Byte 42 toString radix double 2.0', testByte42ToStringRadixDouble2)
    suite.addTest('Byte 100 toString radix double 8.0', testByte100ToStringRadixDouble8)
    suite.addTest('Byte 35 toString radix double 36.0', testByte35ToStringRadixDouble36)
    suite.addTest('Byte 0 toString radix double 10.0', testByte0ToStringRadixDouble10)
    suite.addTest('Byte 1 toString radix double 36.0', testByte1ToStringRadixDouble36)
    suite.addTest('Byte -128 toString radix double 10.0', testByteMinus128ToStringRadixDouble10)
    suite.addTest('Byte -35 toString radix double 36.0', testByteMinus35ToStringRadixDouble36)
    suite.addTest('Byte -1 toString radix double 10.0', testByteMinus1ToStringRadixDouble10)
    suite.addTest('Byte 64 toString radix double 16.9', testByte64ToStringRadixDouble16_9)
    suite.addTest('Byte 42 toString radix double 2.1', testByte42ToStringRadixDouble2_1)
    suite.addTest('Byte 100 toString radix double 7.999', testByte100ToStringRadixDouble7_999)

    suite.addTest('Short 32767 toString radix double 16.0', testShort32767ToStringRadixDouble16)
    suite.addTest('Short 1024 toString radix double 2.0', testShort1024ToStringRadixDouble2)
    suite.addTest('Short 4096 toString radix double 8.0', testShort4096ToStringRadixDouble8)
    suite.addTest('Short 1295 toString radix double 36.0', testShort1295ToStringRadixDouble36)
    suite.addTest('Short 0 toString radix double 10.0', testShort0ToStringRadixDouble10)
    suite.addTest('Short 35 toString radix double 36.0', testShort35ToStringRadixDouble36)
    suite.addTest('Short -32768 toString radix double 10.0', testShortMinus32768ToStringRadixDouble10)
    suite.addTest('Short -1295 toString radix double 36.0', testShortMinus1295ToStringRadixDouble36)
    suite.addTest('Short -1 toString radix double 10.0', testShortMinus1ToStringRadixDouble10)
    suite.addTest('Short 1024 toString radix double 2.999', testShort1024ToStringRadixDouble2_999)
    suite.addTest('Short 4096 toString radix double 8.1', testShort4096ToStringRadixDouble8_1)
    suite.addTest('Short 1295 toString radix double 35.6', testShort1295ToStringRadixDouble35_6)

    suite.addTest('Int 255 toString radix double 2.0', testInt255ToStringRadixDouble2)
    suite.addTest('Int 511 toString radix double 8.0', testInt511ToStringRadixDouble8)
    suite.addTest('Int 65535 toString radix double 16.0', testInt65535ToStringRadixDouble16)
    suite.addTest('Int 46655 toString radix double 36.0', testInt46655ToStringRadixDouble36)
    suite.addTest('Int 0 toString radix double 10.0', testInt0ToStringRadixDouble10)
    suite.addTest('Int 35 toString radix double 36.0', testInt35ToStringRadixDouble36)
    suite.addTest('Int 1295 toString radix double 36.0', testInt1295ToStringRadixDouble36)
    suite.addTest('Int -46655 toString radix double 36.0', testIntMinus46655ToStringRadixDouble36)
    suite.addTest('Int -1295 toString radix double 36.0', testIntMinus1295ToStringRadixDouble36)
    suite.addTest('Int -1 toString radix double 10.0', testIntMinus1ToStringRadixDouble10)
    suite.addTest('Int 1000 toString radix double 10.0', testInt1000ToStringRadixDouble10)
    suite.addTest('Int 255 toString radix double 2.5', testInt255ToStringRadixDouble2_5)
    suite.addTest('Int 511 toString radix double 7.999', testInt511ToStringRadixDouble7_999)
    suite.addTest('Int 65535 toString radix double 15.6', testInt65535ToStringRadixDouble15_6)

    suite.addTest('Long 1023 toString radix double 2.0', testLong1023ToStringRadixDouble2)
    suite.addTest('Long 2097151 toString radix double 8.0', testLong2097151ToStringRadixDouble8)
    suite.addTest('Long 4294967295 toString radix double 16.0', testLong4294967295ToStringRadixDouble16)
    suite.addTest('Long 1679615 toString radix double 36.0', testLong1679615ToStringRadixDouble36)
    suite.addTest('Long 0 toString radix double 10.0', testLong0ToStringRadixDouble10)
    suite.addTest('Long 35 toString radix double 36.0', testLong35ToStringRadixDouble36)
    suite.addTest('Long 1295 toString radix double 36.0', testLong1295ToStringRadixDouble36)
    suite.addTest('Long 46655 toString radix double 36.0', testLong46655ToStringRadixDouble36)
    suite.addTest('Long -1679615 toString radix double 36.0', testLongMinus1679615ToStringRadixDouble36)
    suite.addTest('Long -46655 toString radix double 36.0', testLongMinus46655ToStringRadixDouble36)
    suite.addTest('Long -1295 toString radix double 36.0', testLongMinus1295ToStringRadixDouble36)
    suite.addTest('Long -1 toString radix double 10.0', testLongMinus1ToStringRadixDouble10)
    suite.addTest('Long MIN_VALUE toString radix double 2.0', testLongMinValueToStringRadixDouble2)
    suite.addTest('Long MIN_VALUE toString radix double 10.0', testLongMinValueToStringRadixDouble10)
    suite.addTest('Long MIN_VALUE toString radix double 16.0', testLongMinValueToStringRadixDouble16)
    suite.addTest('Long MIN_VALUE toString radix double 36.0', testLongMinValueToStringRadixDouble36)
    suite.addTest('Long MAX_VALUE toString radix double 2.0', testLongMaxValueToStringRadixDouble2)
    suite.addTest('Long MAX_VALUE toString radix double 10.0', testLongMaxValueToStringRadixDouble10)
    suite.addTest('Long MAX_VALUE toString radix double 16.0', testLongMaxValueToStringRadixDouble16)
    suite.addTest('Long MAX_VALUE toString radix double 36.0', testLongMaxValueToStringRadixDouble36)
    suite.addTest('Long 1023 toString radix double 2.1', testLong1023ToStringRadixDouble2_1)
    suite.addTest('Long 2097151 toString radix double 7.999', testLong2097151ToStringRadixDouble7_999)
    suite.addTest('Long 4294967295 toString radix double 15.6', testLong4294967295ToStringRadixDouble15_6)
    suite.addTest('Long 1679615 toString radix double 35.7', testLong1679615ToStringRadixDouble35_7)

    suite.addTest('toString radix double NaN/Infinity throws RangeError', testToStringInvalidRadixDouble)

    return suite.run()
}

function testCharNewlineToStringRadix2() {
    arktest.assertEQ(c'\n'.toString(2), '1010')
}

function testCharDToStringRadix8() {
    arktest.assertEQ(c'd'.toString(8), '144')
}

function testCharAToStringRadix10() {
    arktest.assertEQ(c'A'.toString(10), '65')
}

function testCharUmlautYToStringRadix16() {
    arktest.assertEQ(c'ÿ'.toString(16), 'ff')
}

function testCharHashToStringRadix36() {
    arktest.assertEQ(c'#'.toString(36), 'z')
}

function testByte64ToStringRadix16() {
    const v: byte = 64
    arktest.assertEQ(v.toString(16), '40')
}

function testByte127ToStringRadix10() {
    const v: byte = 127
    arktest.assertEQ(v.toString(10), '127')
}

function testByte42ToStringRadix2() {
    const v: byte = 42
    arktest.assertEQ(v.toString(2), '101010')
}

function testByte100ToStringRadix8() {
    const v: byte = 100
    arktest.assertEQ(v.toString(8), '144')
}

function testByte35ToStringRadix36() {
    const v: byte = 35
    arktest.assertEQ(v.toString(36), 'z')
}

function testByte0ToStringRadix10() {
    const v: byte = 0
    arktest.assertEQ(v.toString(10), '0')
}

function testByte1ToStringRadix36() {
    const v: byte = 1
    arktest.assertEQ(v.toString(36), '1')
}

function testByteMinus128ToStringRadix10() {
    const v: byte = -128
    arktest.assertEQ(v.toString(10), '-128')
}

function testByteMinus35ToStringRadix36() {
    const v: byte = -35
    arktest.assertEQ(v.toString(36), '-z')
}

function testByteMinus1ToStringRadix10() {
    const v: byte = -1
    arktest.assertEQ(v.toString(10), '-1')
}

function testShort32767ToStringRadix16() {
    const v: short = 32767
    arktest.assertEQ(v.toString(16), '7fff')
}

function testShort1024ToStringRadix2() {
    const v: short = 1024
    arktest.assertEQ(v.toString(2), '10000000000')
}

function testShort4096ToStringRadix8() {
    const v: short = 4096
    arktest.assertEQ(v.toString(8), '10000')
}

function testShort1295ToStringRadix36() {
    const v: short = 1295
    arktest.assertEQ(v.toString(36), 'zz')
}

function testShort0ToStringRadix10() {
    const v: short = 0
    arktest.assertEQ(v.toString(10), '0')
}

function testShort35ToStringRadix36() {
    const v: short = 35
    arktest.assertEQ(v.toString(36), 'z')
}

function testShortMinus32768ToStringRadix10() {
    const v: short = -32768
    arktest.assertEQ(v.toString(10), '-32768')
}

function testShortMinus1295ToStringRadix36() {
    const v: short = -1295
    arktest.assertEQ(v.toString(36), '-zz')
}

function testShortMinus1ToStringRadix10() {
    const v: short = -1
    arktest.assertEQ(v.toString(10), '-1')
}

function testInt255ToStringRadix2() {
    arktest.assertEQ(new Int(255).toString(2), '11111111')
}

function testInt511ToStringRadix8() {
    arktest.assertEQ(new Int(511).toString(8), '777')
}

function testInt65535ToStringRadix16() {
    arktest.assertEQ(new Int(65535).toString(16), 'ffff')
}

function testInt46655ToStringRadix36() {
    arktest.assertEQ(new Int(46655).toString(36), 'zzz')
}

function testInt0ToStringRadix10() {
    arktest.assertEQ(new Int(0).toString(10), '0')
}

function testInt35ToStringRadix36() {
    arktest.assertEQ(new Int(35).toString(36), 'z')
}

function testInt1295ToStringRadix36() {
    arktest.assertEQ(new Int(1295).toString(36), 'zz')
}

function testIntMinus46655ToStringRadix36() {
    arktest.assertEQ(new Int(-46655).toString(36), '-zzz')
}

function testIntMinus1295ToStringRadix36() {
    arktest.assertEQ(new Int(-1295).toString(36), '-zz')
}

function testIntMinus1ToStringRadix10() {
    arktest.assertEQ(new Int(-1).toString(10), '-1')
}

function testInt1000ToStringRadix10() {
    arktest.assertEQ(new Int(1000).toString(10), '1000')
}

function testLong1023ToStringRadix2() {
    arktest.assertEQ(new Long(1023).toString(2), '1111111111')
}

function testLong2097151ToStringRadix8() {
    arktest.assertEQ(new Long(2097151).toString(8), '7777777')
}

function testLong4294967295ToStringRadix16() {
    arktest.assertEQ(new Long(4294967295).toString(16), 'ffffffff')
}

function testLong1679615ToStringRadix36() {
    arktest.assertEQ(new Long(1679615).toString(36), 'zzzz')
}

function testLong0ToStringRadix10() {
    arktest.assertEQ(new Long(0).toString(10), '0')
}

function testLong35ToStringRadix36() {
    arktest.assertEQ(new Long(35).toString(36), 'z')
}

function testLong1295ToStringRadix36() {
    arktest.assertEQ(new Long(1295).toString(36), 'zz')
}

function testLong46655ToStringRadix36() {
    arktest.assertEQ(new Long(46655).toString(36), 'zzz')
}

function testLongMinus1679615ToStringRadix36() {
    arktest.assertEQ(new Long(-1679615).toString(36), '-zzzz')
}

function testLongMinus46655ToStringRadix36() {
    arktest.assertEQ(new Long(-46655).toString(36), '-zzz')
}

function testLongMinus1295ToStringRadix36() {
    arktest.assertEQ(new Long(-1295).toString(36), '-zz')
}

function testLongMinus1ToStringRadix10() {
    arktest.assertEQ(new Long(-1).toString(10), '-1')
}

function testLongMinValueToStringRadix2() {
    arktest.assertEQ(new Long(Long.MIN_VALUE).toString(2),
        '-1000000000000000000000000000000000000000000000000000000000000000')
}

function testLongMinValueToStringRadix10() {
    arktest.assertEQ(new Long(Long.MIN_VALUE).toString(10), '-9223372036854775808')
}

function testLongMinValueToStringRadix16() {
    arktest.assertEQ(new Long(Long.MIN_VALUE).toString(16), '-8000000000000000')
}

function testLongMinValueToStringRadix36() {
    arktest.assertEQ(new Long(Long.MIN_VALUE).toString(36), '-1y2p0ij32e8e8')
}

function testLongMaxValueToStringRadix2() {
    arktest.assertEQ(new Long(Long.MAX_VALUE).toString(2),
        '111111111111111111111111111111111111111111111111111111111111111')
}

function testLongMaxValueToStringRadix10() {
    arktest.assertEQ(new Long(Long.MAX_VALUE).toString(10), '9223372036854775807')
}

function testLongMaxValueToStringRadix16() {
    arktest.assertEQ(new Long(Long.MAX_VALUE).toString(16), '7fffffffffffffff')
}

function testLongMaxValueToStringRadix36() {
    arktest.assertEQ(new Long(Long.MAX_VALUE).toString(36), '1y2p0ij32e8e7')
}

function testByte64ToStringRadixDouble16() {
    const v: byte = 64
    arktest.assertEQ(v.toString(16.0), '40')
}

function testByte127ToStringRadixDouble10() {
    const v: byte = 127
    arktest.assertEQ(v.toString(10.0), '127')
}

function testByte42ToStringRadixDouble2() {
    const v: byte = 42
    arktest.assertEQ(v.toString(2.0), '101010')
}

function testByte100ToStringRadixDouble8() {
    const v: byte = 100
    arktest.assertEQ(v.toString(8.0), '144')
}

function testByte35ToStringRadixDouble36() {
    const v: byte = 35
    arktest.assertEQ(v.toString(36.0), 'z')
}

function testByte0ToStringRadixDouble10() {
    const v: byte = 0
    arktest.assertEQ(v.toString(10.0), '0')
}

function testByte1ToStringRadixDouble36() {
    const v: byte = 1
    arktest.assertEQ(v.toString(36.0), '1')
}

function testByteMinus128ToStringRadixDouble10() {
    const v: byte = -128
    arktest.assertEQ(v.toString(10.0), '-128')
}

function testByteMinus35ToStringRadixDouble36() {
    const v: byte = -35
    arktest.assertEQ(v.toString(36.0), '-z')
}

function testByteMinus1ToStringRadixDouble10() {
    const v: byte = -1
    arktest.assertEQ(v.toString(10.0), '-1')
}

function testByte64ToStringRadixDouble16_9() {
    const v: byte = 64
    arktest.assertEQ(v.toString(16.9), '40')
}

function testByte42ToStringRadixDouble2_1() {
    const v: byte = 42
    arktest.assertEQ(v.toString(2.1), '101010')
}

function testByte100ToStringRadixDouble7_999() {
    const v: byte = 100
    arktest.assertEQ(v.toString(7.999), '202')
}

function testShort32767ToStringRadixDouble16() {
    const v: short = 32767
    arktest.assertEQ(v.toString(16.0), '7fff')
}

function testShort1024ToStringRadixDouble2() {
    const v: short = 1024
    arktest.assertEQ(v.toString(2.0), '10000000000')
}

function testShort4096ToStringRadixDouble8() {
    const v: short = 4096
    arktest.assertEQ(v.toString(8.0), '10000')
}

function testShort1295ToStringRadixDouble36() {
    const v: short = 1295
    arktest.assertEQ(v.toString(36.0), 'zz')
}

function testShort0ToStringRadixDouble10() {
    const v: short = 0
    arktest.assertEQ(v.toString(10.0), '0')
}

function testShort35ToStringRadixDouble36() {
    const v: short = 35
    arktest.assertEQ(v.toString(36.0), 'z')
}

function testShortMinus32768ToStringRadixDouble10() {
    const v: short = -32768
    arktest.assertEQ(v.toString(10.0), '-32768')
}

function testShortMinus1295ToStringRadixDouble36() {
    const v: short = -1295
    arktest.assertEQ(v.toString(36.0), '-zz')
}

function testShortMinus1ToStringRadixDouble10() {
    const v: short = -1
    arktest.assertEQ(v.toString(10.0), '-1')
}

function testShort1024ToStringRadixDouble2_999() {
    const v: short = 1024
    arktest.assertEQ(v.toString(2.999), '10000000000')
}

function testShort4096ToStringRadixDouble8_1() {
    const v: short = 4096
    arktest.assertEQ(v.toString(8.1), '10000')
}

function testShort1295ToStringRadixDouble35_6() {
    const v: short = 1295
    arktest.assertEQ(v.toString(35.6), '120')
}

function testInt255ToStringRadixDouble2() {
    arktest.assertEQ(new Int(255).toString(2.0), '11111111')
}

function testInt511ToStringRadixDouble8() {
    arktest.assertEQ(new Int(511).toString(8.0), '777')
}

function testInt65535ToStringRadixDouble16() {
    arktest.assertEQ(new Int(65535).toString(16.0), 'ffff')
}

function testInt46655ToStringRadixDouble36() {
    arktest.assertEQ(new Int(46655).toString(36.0), 'zzz')
}

function testInt0ToStringRadixDouble10() {
    arktest.assertEQ(new Int(0).toString(10.0), '0')
}

function testInt35ToStringRadixDouble36() {
    arktest.assertEQ(new Int(35).toString(36.0), 'z')
}

function testInt1295ToStringRadixDouble36() {
    arktest.assertEQ(new Int(1295).toString(36.0), 'zz')
}

function testIntMinus46655ToStringRadixDouble36() {
    arktest.assertEQ(new Int(-46655).toString(36.0), '-zzz')
}

function testIntMinus1295ToStringRadixDouble36() {
    arktest.assertEQ(new Int(-1295).toString(36.0), '-zz')
}

function testIntMinus1ToStringRadixDouble10() {
    arktest.assertEQ(new Int(-1).toString(10.0), '-1')
}

function testInt1000ToStringRadixDouble10() {
    arktest.assertEQ(new Int(1000).toString(10.0), '1000')
}

function testInt255ToStringRadixDouble2_5() {
    arktest.assertEQ(new Int(255).toString(2.5), '11111111')
}

function testInt511ToStringRadixDouble7_999() {
    arktest.assertEQ(new Int(511).toString(7.999), '1330')
}

function testInt65535ToStringRadixDouble15_6() {
    arktest.assertEQ(new Int(65535).toString(15.6), '14640')
}

function testLong1023ToStringRadixDouble2() {
    arktest.assertEQ(new Long(1023).toString(2.0), '1111111111')
}

function testLong2097151ToStringRadixDouble8() {
    arktest.assertEQ(new Long(2097151).toString(8.0), '7777777')
}

function testLong4294967295ToStringRadixDouble16() {
    arktest.assertEQ(new Long(4294967295).toString(16.0), 'ffffffff')
}

function testLong1679615ToStringRadixDouble36() {
    arktest.assertEQ(new Long(1679615).toString(36.0), 'zzzz')
}

function testLong0ToStringRadixDouble10() {
    arktest.assertEQ(new Long(0).toString(10.0), '0')
}

function testLong35ToStringRadixDouble36() {
    arktest.assertEQ(new Long(35).toString(36.0), 'z')
}

function testLong1295ToStringRadixDouble36() {
    arktest.assertEQ(new Long(1295).toString(36.0), 'zz')
}

function testLong46655ToStringRadixDouble36() {
    arktest.assertEQ(new Long(46655).toString(36.0), 'zzz')
}

function testLongMinus1679615ToStringRadixDouble36() {
    arktest.assertEQ(new Long(-1679615).toString(36.0), '-zzzz')
}

function testLongMinus46655ToStringRadixDouble36() {
    arktest.assertEQ(new Long(-46655).toString(36.0), '-zzz')
}

function testLongMinus1295ToStringRadixDouble36() {
    arktest.assertEQ(new Long(-1295).toString(36.0), '-zz')
}

function testLongMinus1ToStringRadixDouble10() {
    arktest.assertEQ(new Long(-1).toString(10.0), '-1')
}

function testLongMinValueToStringRadixDouble2() {
    arktest.assertEQ(new Long(Long.MIN_VALUE).toString(2.0),
        '-1000000000000000000000000000000000000000000000000000000000000000')
}

function testLongMinValueToStringRadixDouble10() {
    arktest.assertEQ(new Long(Long.MIN_VALUE).toString(10.0), '-9223372036854775808')
}

function testLongMinValueToStringRadixDouble16() {
    arktest.assertEQ(new Long(Long.MIN_VALUE).toString(16.0), '-8000000000000000')
}

function testLongMinValueToStringRadixDouble36() {
    arktest.assertEQ(new Long(Long.MIN_VALUE).toString(36.0), '-1y2p0ij32e8e8')
}

function testLongMaxValueToStringRadixDouble2() {
    arktest.assertEQ(new Long(Long.MAX_VALUE).toString(2.0),
        '111111111111111111111111111111111111111111111111111111111111111')
}

function testLongMaxValueToStringRadixDouble10() {
    arktest.assertEQ(new Long(Long.MAX_VALUE).toString(10.0), '9223372036854775807')
}

function testLongMaxValueToStringRadixDouble16() {
    arktest.assertEQ(new Long(Long.MAX_VALUE).toString(16.0), '7fffffffffffffff')
}

function testLongMaxValueToStringRadixDouble36() {
    arktest.assertEQ(new Long(Long.MAX_VALUE).toString(36.0), '1y2p0ij32e8e7')
}

function testLong1023ToStringRadixDouble2_1() {
    arktest.assertEQ(new Long(1023).toString(2.1), '1111111111')
}

function testLong2097151ToStringRadixDouble7_999() {
    arktest.assertEQ(new Long(2097151).toString(7.999), '23553100')
}

function testLong4294967295ToStringRadixDouble15_6() {
    arktest.assertEQ(new Long(4294967295).toString(15.6), '1a20dcd80')
}

function testLong1679615ToStringRadixDouble35_7() {
    arktest.assertEQ(new Long(1679615).toString(35.7), '14640')
}

function testToStringInvalidRadixDouble() {
    const rangeError = (e: Error): boolean => e instanceof RangeError

    arktest.expectThrow(() => { new Byte().toString(NaN) }, rangeError)
    arktest.expectThrow(() => { new Byte().toString(Infinity) }, rangeError)
    arktest.expectThrow(() => { new Byte().toString(-Infinity) }, rangeError)

    arktest.expectThrow(() => { new Short().toString(NaN) }, rangeError)
    arktest.expectThrow(() => { new Short().toString(Infinity) }, rangeError)
    arktest.expectThrow(() => { new Short().toString(-Infinity) }, rangeError)

    arktest.expectThrow(() => { new Int().toString(NaN) }, rangeError)
    arktest.expectThrow(() => { new Int().toString(Infinity) }, rangeError)
    arktest.expectThrow(() => { new Int().toString(-Infinity) }, rangeError)

    arktest.expectThrow(() => { new Long().toString(NaN) }, rangeError)
    arktest.expectThrow(() => { new Long().toString(Infinity) }, rangeError)
    arktest.expectThrow(() => { new Long().toString(-Infinity) }, rangeError)
}
