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
    const suite = new arktest.ArkTestsuite('JSON.stringify primitives, objects, and wrappers')
    suite.addTest('undefined', checkUndefined)
    suite.addTest('null', checkNull)
    suite.addTest('true', checkTrue)
    suite.addTest('false', checkFalse)
    suite.addTest('0', n0)
    suite.addTest('12345', n12345)
    suite.addTest('-98765', n98765)
    suite.addTest('NaN', checkNaN)
    suite.addTest('Infinity', checkInfinity)
    suite.addTest('-Infinity', negInfinity)
    suite.addTest('9007199254740991', n90071992547409911)
    suite.addTest('-9007199254740991', n90071992547409912)
    suite.addTest('\'\'', emptyString)
    suite.addTest('{} as Record<string, Object>', emptyRecord)
    suite.addTest('[] as FixedArray<byte>', emptyFixedarrayByte)
    suite.addTest('[1, 2, 3, 4, 5]', arr12345)
    suite.addTest('[[1, 2], [3, 4], [5, 6]]', array1)
    suite.addTest('Number.MAX_VALUE', maxvalue)
    suite.addTest('Number.EPSILON', epsilon)
    suite.addTest('BigInt(9007199254740991)', bigInt9007199254740991)
    suite.addTest('byte array [0, 0, 0, -128, 127, 2]', arrByteRangeWith256)
    suite.addTest('byte array [1, NaN, 3, Infinity, -Infinity]', arrByteWithNaNInf)
    suite.addTest('\'0\'', sEsc599)
    suite.addTest('\'NaN\'', sNaN)
    suite.addTest('\'null\'', sNull)
    suite.addTest('FixedArray<byte> [1, 2, 3, 4, 5]', arrByte12345)
    suite.addTest('FixedArray<byte> [-128..127]', arrByteBoundaries)
    suite.addTest('[42] as FixedArray<byte>', arr42Byte)
    suite.addTest('FixedArray<byte> [-128, -64, -2, -1]', arrByteNegRange)
    suite.addTest('FixedArray<byte> [0, 1, 64, 127]', arrBytePosRange)
    suite.addTest('FixedArray<byte> [-128, 0, 127]', arrByteExtremes)
    suite.addTest('FixedArray<byte> [1, -1, 2, -2, 3, -3]', arrByteAltSign)
    suite.addTest('FixedArray<short> [0, 50, 100, 150, 200]', arrShortPosRange)
    suite.addTest('double array [Infinity, -Infinity, NaN]', arrDoubleInfNaN)
    suite.addTest('\'test\\nline\'', sTestNline)
    suite.addTest('\'ABC\'', sabc)
    suite.addTest('double array [1.7976931348623157e308, 5e-324, -0.0]', arrDoubleExtremeRange)
    suite.addTest('FixedArray<boolean> [true, false]', arrBooleanTF)
    suite.addTest('double array [42.0, NaN, Infinity, -Infinity, -0.0]', arrDoubleMixedEdge)
    suite.addTest('FixedArray<int> [-2147483648, 0, 2147483647]', arrIntMinZeroMax)
    suite.addTest('FixedArray<long> [-9223372036854775808, -1, 0, 1]', arrLongMinNegPos)
    suite.addTest('float MIN_VALUE from Double.toFloat', floatMinFromDouble)
    suite.addTest('FixedArray<double> [Infinity, -Infinity, NaN]', arrDoubleInfNaNEdge)
    suite.addTest('\'\\b\\f\\n\\r\\t\'', sbfnrt)
    suite.addTest('String.fromCharCode(0x41, 0x42)', stringfromCharCode0x410x42)
    suite.addTest('[1e3, 2.5e-2, 2 ** 3]', arr1e325e223)
    suite.addTest('0x0102', check0x0102)
    suite.addTest('-0', negZero)
    suite.addTest('mixed array [1, undefined, null, NaN, Infinity]', arrMixedSpecials)
    suite.addTest('[] as number[]', emptyNumber)
    suite.addTest('[42] as FixedArray<short>', arr42Short)
    suite.addTest('[] as FixedArray<short>', emptyFixedarrayShort)
    suite.addTest('FixedArray<short> [-32768, -1, 0, 1, 32767]', arrShortBoundaries)
    suite.addTest('FixedArray<short> [-32768, -16384, -256, -1]', arrShortNegPowers)
    suite.addTest('FixedArray<short> [0, 1, 256, 16384, 32767]', arrShortPosPowers)
    suite.addTest('FixedArray<short> [32767, -32768, 1, -1, 1]', arrShortMixedMaxMin)
    suite.addTest('FixedArray<short> [-128, -100, -50, 0, 50, 100]', arrShortNegPosRange)
    suite.addTest('FixedArray<short> [100, 100, 200, 200, 32767]', arrShortDuplicates)
    suite.addTest('FixedArray<short> [-32768, -32768, 32767, 32767]', arrShortRepeatedExtremes)
    suite.addTest('FixedArray<short> [-1, 0, 1, -1, 0, 1]', arrShortTripleAltSign)
    suite.addTest('[] as FixedArray<int>', emptyFixedarrayInt)
    suite.addTest('FixedArray<int> [-2147483648..2147483647]', arrIntBoundaries)
    suite.addTest('FixedArray<int> [-2147483648..1073741824]', arrIntNegPowers)
    suite.addTest('FixedArray<int> [0, 1, 256, 65536, 1073741824]', arrIntPosPowers)
    suite.addTest('FixedArray<int> [2147483647, -2147483648]', arrIntMixedMaxMin)
    suite.addTest('FixedArray<int> [2147483647..1073741824]', arrIntRepeatedMax)
    suite.addTest('FixedArray<int> [-2, -1, 0, 1, 2]', arrIntAltSign)
    suite.addTest('FixedArray<int> [Infinity, -Infinity, NaN]', arrIntEdgeToInt)
    suite.addTest('FixedArray<int> [0, 2147483647, -1 via Long.toInt]', arrIntLongConversions)
    suite.addTest('FixedArray<long> [1, 2, 3, 4, 5]', arrLong12345)
    suite.addTest('[42] as FixedArray<long>', arr42Long)
    suite.addTest('[] as FixedArray<long>', emptyFixedarrayLong)
    suite.addTest('FixedArray<long> [-9223372036854775808..9223372036854775807]', arrLongBoundaries)
    suite.addTest('FixedArray<long> [9223372036854775807, -9223372036854775808]', arrLongSingleMax)
    suite.addTest('FixedArray<long> [-9223372036854775808, -1, 0, 1]', arrLongNegRange)
    suite.addTest('FixedArray<long> min/max range values', arrLongExtremes)
    suite.addTest('FixedArray<long> [0, 1, -1, -9223372036854775808]', arrLongMixedSyntax)
    suite.addTest('FixedArray<long> [9223372036854775807, 1]', arrLongMaxAndOne)
    suite.addTest('FixedArray<long> [9223372036854775807, -9223372036854775808]', arrLongMaxAndMin)
    suite.addTest('FixedArray<long> [-2, -1, 0, 1, 2]', arrLongAltSign)
    suite.addTest('FixedArray<float> [1.0, 0.0, -0.0]', arrFloatUnityZero)
    suite.addTest('FixedArray<float> [1.5, -1.5 from Double]', arrFloatPosNegFromDouble)
    suite.addTest('FixedArray<float> [3.141592653589793 from Double]', arrFloatPiFromDouble)
    suite.addTest('[] as FixedArray<float>', emptyFixedarrayFloat)
    suite.addTest('FixedArray<float> [3.4028235e38, -3.4028235e38]', arrFloatBoundariesFromDouble)
    suite.addTest('FixedArray<float> [-3.4028235e38, 1.4e-45]', arrFloatNegBoundariesFromDouble)
    suite.addTest('FixedArray<float> [1.1754944e-38, 0.0, -0.0]', arrFloatMinNormalFromDouble)
    suite.addTest('FixedArray<float> [0.0, -0.0 from Double]', arrFloatSignedZeroFromDouble)
    suite.addTest('FixedArray<float> [NaN from Double]', arrFloatNaNFromDouble)
    suite.addTest('number from Double.toFloat(1.0)', floatFromDoubleUnity)
    suite.addTest('FixedArray<float> [-1.5, -0.0 from Double]', arrFloatNegFromDouble)
    suite.addTest('FixedArray<float> [1.0, -0.0 from Double]', arrFloatSignedZeroNegFromDouble)
    suite.addTest('FixedArray<double> [1.0, 2.0, 3.0]', arrDouble123)
    suite.addTest('FixedArray<double> [3.141592653589793]', arrDoublePi)
    suite.addTest('[] as FixedArray<double>', emptyFixedarrayDouble)
    suite.addTest('FixedArray<double> [0.0, -0.0]', arrDoubleSignedZero)
    suite.addTest('FixedArray<double> [Number.MAX_VALUE, -Number.MAX_VALUE, 0]', arrDoubleMaxMinZero)
    suite.addTest('FixedArray<double> [1.5, -1.5, 2.5, -2.5, 3.5]', arrDoubleAltSignStep1)
    suite.addTest('FixedArray<double> [1.5, NaN, 2.5, Infinity, -Infinity]', arrDoubleWithNaNInf)
    suite.addTest('FixedArray<double> [Number.MAX_VALUE, -Number.MAX_VALUE]', arrDoubleMaxMin)
    suite.addTest('FixedArray<boolean> [true, false, true, false]', arrBooleanAlt)
    suite.addTest('[] as FixedArray<boolean>', emptyFixedarrayBoolean)
    suite.addTest('FixedArray<boolean> [true, true, true]', arrBooleanAllTrue)
    suite.addTest('FixedArray<boolean> [false, false, false]', arrBooleanAllFalse)
    suite.addTest('FixedArray<boolean> [true]', arrBooleanSingleTrue)
    suite.addTest('FixedArray<boolean> [false]', arrBooleanSingleFalse)
    suite.addTest('FixedArray<boolean> [true, false, true]', arrBooleanAltShort)
    suite.addTest('FixedArray<boolean> [true, true, false, false]', arrBooleanMixed)
    suite.addTest('[1, 2.5, -3, 0, 100]', arr12530100)
    suite.addTest('number array [NaN, Infinity, -Infinity, -0, 0]', arrNaNInfinityInfinity00)
    suite.addTest('number array [MAX_VALUE, -MAX_VALUE, 0, Infinity]', arrMaxMinInfZero)
    suite.addTest('number array [MAX_SAFE_INTEGER, MAX_VALUE, -Infinity]', arrSafeIntMaxMinInf)
    suite.addTest('number array [0x0102, 0x3, 0b1010, 0o17]', arrHexOctBinFormats)
    suite.addTest('[42]', arr42)
    suite.addTest('number array [-1, -256, -65535, -999999, 0]', arrNegValues)
    suite.addTest('number array [0, -0, 1.5, NaN, Infinity, -Infinity]', arrSignedZeroNaNInf)
    suite.addTest('number array [0, 0, 0, 1.5, 1.5, NaN, NaN]', arrMixedMultiNaN)
    suite.addTest('[10, 20, 30]', arr102030)
    suite.addTest('number array [NaN, Infinity, -Infinity, 0]', arrNaNInfNegInfZero)
    suite.addTest('[-1.5, 0, 3.14, -999.99]', arr15031499999)
    suite.addTest('number array [Number.MAX_VALUE, Number.MIN_VALUE]', arrMaxValueMinValue)
    suite.addTest('[1e5, 2.5e-3, -3e10]', arr1e525e33e10)
    suite.addTest('[0, -0, 0]', arr000)
    return suite.run()
}

function checkUndefined(): void {
    const result: string = JSON.stringify(undefined)
    arktest.assertEQ(result, '')
}

function checkNull(): void {
    const result: string = JSON.stringify(null)
    arktest.assertEQ(result, 'null')
}

function checkTrue(): void {
    const result: string = JSON.stringify(true)
    arktest.assertEQ(result, 'true')
}

function checkFalse(): void {
    const result: string = JSON.stringify(false)
    arktest.assertEQ(result, 'false')
}

function n0(): void {
    const result: string = JSON.stringify(0)
    arktest.assertEQ(result, '0')
}

function n12345(): void {
    const result: string = JSON.stringify(12345)
    arktest.assertEQ(result, '12345')
}

function n98765(): void {
    const result: string = JSON.stringify(-98765)
    arktest.assertEQ(result, '-98765')
}

function checkNaN(): void {
    const result: string = JSON.stringify(NaN)
    arktest.assertEQ(result, 'null')
}

function checkInfinity(): void {
    const result: string = JSON.stringify(Infinity)
    arktest.assertEQ(result, 'null')
}

function negInfinity(): void {
    const result: string = JSON.stringify(-Infinity)
    arktest.assertEQ(result, 'null')
}

function n90071992547409911(): void {
    const result: string = JSON.stringify(9007199254740991)
    arktest.assertEQ(result, '9007199254740991')
}

function n90071992547409912(): void {
    const result: string = JSON.stringify(-9007199254740991)
    arktest.assertEQ(result, '-9007199254740991')
}

function emptyString(): void {
    const result: string = JSON.stringify('')
    arktest.assertEQ(result, '""')
}

function emptyRecord(): void {
    const result: string = JSON.stringify({} as Record<string, Object>)
    arktest.assertEQ(result, '{}')
}

function emptyFixedarrayByte(): void {
    const result: string = JSON.stringify([] as FixedArray<byte>)
    arktest.assertEQ(result, '[]')
}

function arr12345(): void {
    const result: string = JSON.stringify([1, 2, 3, 4, 5])
    arktest.assertEQ(result, '[1,2,3,4,5]')
}

function array1(): void {
    const result: string = JSON.stringify([[1, 2], [3, 4], [5, 6]])
    arktest.assertEQ(result, '[[1,2],[3,4],[5,6]]')
}

function maxvalue(): void {
    const result: string = JSON.stringify(Number.MAX_VALUE)
    arktest.assertEQ(result, '1.7976931348623157e+308')
}

function epsilon(): void {
    const result: string = JSON.stringify(Number.EPSILON)
    arktest.assertEQ(result, '2.220446049250313e-16')
}

function bigInt9007199254740991(): void {
    const result: string = JSON.stringify(BigInt(9007199254740991))
    arktest.assertEQ(result, '9007199254740991')
}

function arrByteRangeWith256(): void {
    const result: string = JSON.stringify([0, 0, 0, -128, 127, 256, 65536])
    arktest.assertEQ(result, '[0,0,0,-128,127,256,65536]')
}

function arrByteWithNaNInf(): void {
    const result: string = JSON.stringify([1, NaN, 3, Infinity, -Infinity, 6])
    arktest.assertEQ(result, '[1,null,3,null,null,6]')
}

function sEsc599(): void {
    const result: string = JSON.stringify('0')
    arktest.assertEQ(result, '"0"')
}

function sNaN(): void {
    const result: string = JSON.stringify('NaN')
    arktest.assertEQ(result, '"NaN"')
}

function sNull(): void {
    const result: string = JSON.stringify('null')
    arktest.assertEQ(result, '"null"')
}

function arrByte12345(): void {
    const result: string = JSON.stringify([1, 2, 3, 4, 5] as FixedArray<byte>)
    arktest.assertEQ(result, '[1,2,3,4,5]')
}

function arrByteBoundaries(): void {
    const result: string = JSON.stringify([-128, -1, 0, 1, 127] as FixedArray<byte>)
    arktest.assertEQ(result, '[-128,-1,0,1,127]')
}

function arr42Byte(): void {
    const result: string = JSON.stringify([42] as FixedArray<byte>)
    arktest.assertEQ(result, '[42]')
}

function arrByteNegRange(): void {
    const result: string = JSON.stringify([-128, -64, -2, -1] as FixedArray<byte>)
    arktest.assertEQ(result, '[-128,-64,-2,-1]')
}

function arrBytePosRange(): void {
    const result: string = JSON.stringify([0, 1, 64, 127] as FixedArray<byte>)
    arktest.assertEQ(result, '[0,1,64,127]')
}

function arrByteExtremes(): void {
    const result: string = JSON.stringify([-128, 0, 127] as FixedArray<byte>)
    arktest.assertEQ(result, '[-128,0,127]')
}

function arrByteAltSign(): void {
    const result: string = JSON.stringify([1, -1, 2, -2, 3, -3] as FixedArray<byte>)
    arktest.assertEQ(result, '[1,-1,2,-2,3,-3]')
}

function arrShortPosRange(): void {
    const result: string = JSON.stringify([0, 50, 100, 150, 200, 255] as FixedArray<short>)
    arktest.assertEQ(result, '[0,50,100,150,200,255]')
}

function arrDoubleInfNaN(): void {
    const result: string = JSON.stringify([Infinity as double, -Infinity as double, NaN as double, 0.0 as double,
        3.14159 as double] as FixedArray<double>)
    arktest.assertEQ(result, '[null,null,null,0,3.14159]')
}

function sTestNline(): void {
    const result: string = JSON.stringify('test\nline')
    arktest.assertEQ(result, '"test\\nline"')
}

function sabc(): void {
    const result: string = JSON.stringify('ABC')
    arktest.assertEQ(result, '"ABC"')
}

function arrDoubleExtremeRange(): void {
    const result: string = JSON.stringify([1.7976931348623157e+308 as double, 5e-324 as double,
        2.220446049250313e-16 as double, 9007199254740991.0 as double] as FixedArray<double>)
    arktest.assertEQ(result, '[1.7976931348623157e+308,5e-324,2.220446049250313e-16,9007199254740991]')
}

function arrBooleanTF(): void {
    const result: string = JSON.stringify([true, false] as FixedArray<boolean>)
    arktest.assertEQ(result, '[true,false]')
}

function arrDoubleMixedEdge(): void {
    const result: string = JSON.stringify([42.0 as double, NaN as double, Infinity as double, -Infinity as double,
        0.0 as double] as FixedArray<double>)
    arktest.assertEQ(result, '[42,null,null,null,0]')
}

function arrIntMinZeroMax(): void {
    const result: string = JSON.stringify([-2147483648, 0, 2147483647] as FixedArray<int>)
    arktest.assertEQ(result, '[-2147483648,0,2147483647]')
}

function arrLongMinNegPos(): void {
    const result: string = JSON.stringify([-9223372036854775808 as long, 9223372036854775807 as long]
        as FixedArray<long>)
    arktest.assertEQ(result, '[-9223372036854775808,9223372036854775807]')
}

function floatMinFromDouble(): void {
    const result: string = JSON.stringify([Double.toFloat(-3.4028234663852886e38),
        Double.toFloat(1.1754943508222875e-38), Double.toFloat(3.4028234663852886e38)] as FixedArray<float>)
    arktest.assertEQ(result, '[-3.4028235e+38,1.1754944e-38,3.4028235e+38]')
}

function arrDoubleInfNaNEdge(): void {
    const result: string = JSON.stringify([Infinity, -Infinity, NaN] as FixedArray<double>)
    arktest.assertEQ(result, '[null,null,null]')
}

function sbfnrt(): void {
    const result: string = JSON.stringify('\b\f\n\r\t')
    arktest.assertEQ(result, '"\\b\\f\\n\\r\\t"')
}

function stringfromCharCode0x410x42(): void {
    const result: string = JSON.stringify(String.fromCharCode(0x41, 0x42))
    arktest.assertEQ(result, '"AB"')
}

function arr1e325e223(): void {
    const result: string = JSON.stringify([1e3, 2.5e-2, 2 ** 3])
    arktest.assertEQ(result, '[1000,0.025,8]')
}

function check0x0102(): void {
    const result: string = JSON.stringify(0x0102)
    arktest.assertEQ(result, '258')
}

function negZero(): void {
    const result: string = JSON.stringify(-0)
    arktest.assertEQ(result, '0')
}

function arrMixedSpecials(): void {
    const result: string = JSON.stringify([1, undefined, null, NaN, Infinity])
    arktest.assertEQ(result, '[1,null,null,null,null]')
}

function emptyNumber(): void {
    const result: string = JSON.stringify([] as number[])
    arktest.assertEQ(result, '[]')
}

function arr42Short(): void {
    const result: string = JSON.stringify([42] as FixedArray<short>)
    arktest.assertEQ(result, '[42]')
}

function emptyFixedarrayShort(): void {
    const result: string = JSON.stringify([] as FixedArray<short>)
    arktest.assertEQ(result, '[]')
}

function arrShortBoundaries(): void {
    const result: string = JSON.stringify([-32768, -1, 0, 1, 32767] as FixedArray<short>)
    arktest.assertEQ(result, '[-32768,-1,0,1,32767]')
}

function arrShortNegPowers(): void {
    const result: string = JSON.stringify([-32768, -16384, -256, -1] as FixedArray<short>)
    arktest.assertEQ(result, '[-32768,-16384,-256,-1]')
}

function arrShortPosPowers(): void {
    const result: string = JSON.stringify([0, 1, 256, 16384, 32767] as FixedArray<short>)
    arktest.assertEQ(result, '[0,1,256,16384,32767]')
}

function arrShortMixedMaxMin(): void {
    const result: string = JSON.stringify([32767, -32768, 1, -1, 100, -100] as FixedArray<short>)
    arktest.assertEQ(result, '[32767,-32768,1,-1,100,-100]')
}

function arrShortNegPosRange(): void {
    const result: string = JSON.stringify([-128, -100, -50, 0, 50, 100, 127] as FixedArray<short>)
    arktest.assertEQ(result, '[-128,-100,-50,0,50,100,127]')
}

function arrShortDuplicates(): void {
    const result: string = JSON.stringify([100, 100, 200, 200, 300] as FixedArray<short>)
    arktest.assertEQ(result, '[100,100,200,200,300]')
}

function arrShortRepeatedExtremes(): void {
    const result: string = JSON.stringify([-32768, -32768, 32767, 32767, -32768] as FixedArray<short>)
    arktest.assertEQ(result, '[-32768,-32768,32767,32767,-32768]')
}

function arrShortTripleAltSign(): void {
    const result: string = JSON.stringify([-1, 0, 1, -1, 0, 1] as FixedArray<short>)
    arktest.assertEQ(result, '[-1,0,1,-1,0,1]')
}

function emptyFixedarrayInt(): void {
    const result: string = JSON.stringify([] as FixedArray<int>)
    arktest.assertEQ(result, '[]')
}

function arrIntBoundaries(): void {
    const result: string = JSON.stringify([-2147483648, -1, 0, 1, 2147483647] as FixedArray<int>)
    arktest.assertEQ(result, '[-2147483648,-1,0,1,2147483647]')
}

function arrIntNegPowers(): void {
    const result: string = JSON.stringify([-2147483648, -1073741824, -65536, -256, -1] as FixedArray<int>)
    arktest.assertEQ(result, '[-2147483648,-1073741824,-65536,-256,-1]')
}

function arrIntPosPowers(): void {
    const result: string = JSON.stringify([0, 1, 256, 65536, 1073741824, 2147483647] as FixedArray<int>)
    arktest.assertEQ(result, '[0,1,256,65536,1073741824,2147483647]')
}

function arrIntMixedMaxMin(): void {
    const result: string = JSON.stringify([2147483647, -2147483648, 2147483647, -2147483648, 1, -1, 0, 1000000,
        -1000000] as FixedArray<int>)
    arktest.assertEQ(result, '[2147483647,-2147483648,2147483647,-2147483648,1,-1,0,1000000,-1000000]')
}

function arrIntRepeatedMax(): void {
    const result: string = JSON.stringify([2147483647, 2147483647, 2147483647] as FixedArray<int>)
    arktest.assertEQ(result, '[2147483647,2147483647,2147483647]')
}

function arrIntAltSign(): void {
    const result: string = JSON.stringify([-2, -1, 0, 1, 2] as FixedArray<int>)
    arktest.assertEQ(result, '[-2,-1,0,1,2]')
}

function arrIntEdgeToInt(): void {
    const result: string = JSON.stringify([Double.toInt(Infinity), Double.toInt(-Infinity), Double.toInt(NaN)]
        as FixedArray<int>)
    arktest.assertEQ(result, '[2147483647,-2147483648,0]')
}

function arrIntLongConversions(): void {
    const result: string = JSON.stringify([0, 2147483647, Long.toInt(2147483648), -2147483648, Long.toInt(-2147483649)]
        as FixedArray<int>)
    arktest.assertEQ(result, '[0,2147483647,-2147483648,-2147483648,2147483647]')
}

function arrLong12345(): void {
    const result: string = JSON.stringify([1, 2, 3, 4, 5] as FixedArray<long>)
    arktest.assertEQ(result, '[1,2,3,4,5]')
}

function arr42Long(): void {
    const result: string = JSON.stringify([42] as FixedArray<long>)
    arktest.assertEQ(result, '[42]')
}

function emptyFixedarrayLong(): void {
    const result: string = JSON.stringify([] as FixedArray<long>)
    arktest.assertEQ(result, '[]')
}

function arrLongBoundaries(): void {
    const result: string = JSON.stringify([-9223372036854775808 as long] as FixedArray<long>)
    arktest.assertEQ(result, '[-9223372036854775808]')
}

function arrLongSingleMax(): void {
    const result: string = JSON.stringify([9223372036854775807 as long] as FixedArray<long>)
    arktest.assertEQ(result, '[9223372036854775807]')
}

function arrLongNegRange(): void {
    const result: string = JSON.stringify([-9223372036854775808 as long, 0 as long, 9223372036854775807 as long]
        as FixedArray<long>)
    arktest.assertEQ(result, '[-9223372036854775808,0,9223372036854775807]')
}

function arrLongExtremes(): void {
    const result: string = JSON.stringify([-9223372036854775808 as long, -1000000 as long, -1 as long]
        as FixedArray<long>)
    arktest.assertEQ(result, '[-9223372036854775808,-1000000,-1]')
}

function arrLongMixedSyntax(): void {
    const result: string = JSON.stringify([0 as long, 1 as long, 1000000 as long, 9223372036854775807 as long]
        as FixedArray<long>)
    arktest.assertEQ(result, '[0,1,1000000,9223372036854775807]')
}

function arrLongMaxAndOne(): void {
    const result: string = JSON.stringify([9223372036854775807 as long, -9223372036854775808 as long, 1 as long,
        -1 as long] as FixedArray<long>)
    arktest.assertEQ(result, '[9223372036854775807,-9223372036854775808,1,-1]')
}

function arrLongMaxAndMin(): void {
    const result: string = JSON.stringify([9223372036854775807 as long, 9223372036854775807 as long]
        as FixedArray<long>)
    arktest.assertEQ(result, '[9223372036854775807,9223372036854775807]')
}

function arrLongAltSign(): void {
    const result: string = JSON.stringify([-2, -1, 0, 1, 2] as FixedArray<long>)
    arktest.assertEQ(result, '[-2,-1,0,1,2]')
}

function arrFloatUnityZero(): void {
    const result: string = JSON.stringify([Double.toFloat(1.0), Double.toFloat(2.0), Double.toFloat(3.0)]
        as FixedArray<float>)
    arktest.assertEQ(result, '[1,2,3]')
}

function arrFloatPosNegFromDouble(): void {
    const result: string = JSON.stringify([Double.toFloat(1.5), Double.toFloat(2.5), Double.toFloat(3.14)]
        as FixedArray<float>)
    arktest.assertEQ(result, '[1.5,2.5,3.14]')
}

function arrFloatPiFromDouble(): void {
    const result: string = JSON.stringify([Double.toFloat(3.14159)] as FixedArray<float>)
    arktest.assertEQ(result, '[3.14159]')
}

function emptyFixedarrayFloat(): void {
    const result: string = JSON.stringify([] as FixedArray<float>)
    arktest.assertEQ(result, '[]')
}

function arrFloatBoundariesFromDouble(): void {
    const result: string = JSON.stringify([Double.toFloat(3.4028234663852886e38)] as FixedArray<float>)
    arktest.assertEQ(result, '[3.4028235e+38]')
}

function arrFloatNegBoundariesFromDouble(): void {
    const result: string = JSON.stringify([Double.toFloat(-3.4028234663852886e38)] as FixedArray<float>)
    arktest.assertEQ(result, '[-3.4028235e+38]')
}

function arrFloatMinNormalFromDouble(): void {
    const result: string = JSON.stringify([Double.toFloat(1.1754943508222875e-38)] as FixedArray<float>)
    arktest.assertEQ(result, '[1.1754944e-38]')
}

function arrFloatSignedZeroFromDouble(): void {
    const result: string = JSON.stringify([Double.toFloat(0.0), Double.toFloat(-0.0)] as FixedArray<float>)
    arktest.assertEQ(result, '[0,0]')
}

function arrFloatNaNFromDouble(): void {
    const result: string = JSON.stringify([Double.toFloat(NaN)] as FixedArray<float>)
    arktest.assertEQ(result, '[null]')
}

function floatFromDoubleUnity(): void {
    const result: string = JSON.stringify([Double.toFloat(1.0), Double.toFloat(NaN), Double.toFloat(2.0),
        Double.toFloat(Infinity), Double.toFloat(-Infinity), Double.toFloat(0.0)] as FixedArray<float>)
    arktest.assertEQ(result, '[1,null,2,null,null,0]')
}

function arrFloatNegFromDouble(): void {
    const result: string = JSON.stringify([Double.toFloat(-1.5), Double.toFloat(-2.5), Double.toFloat(-0.1)]
        as FixedArray<float>)
    arktest.assertEQ(result, '[-1.5,-2.5,-0.1]')
}

function arrFloatSignedZeroNegFromDouble(): void {
    const result: string = JSON.stringify([Double.toFloat(1.0), Double.toFloat(-1.0), Double.toFloat(2.0),
        Double.toFloat(-2.0)] as FixedArray<float>)
    arktest.assertEQ(result, '[1,-1,2,-2]')
}

function arrDouble123(): void {
    const result: string = JSON.stringify([1.0, 2.0, 3.0] as FixedArray<double>)
    arktest.assertEQ(result, '[1,2,3]')
}

function arrDoublePi(): void {
    const result: string = JSON.stringify([3.141592653589793] as FixedArray<double>)
    arktest.assertEQ(result, '[3.141592653589793]')
}

function emptyFixedarrayDouble(): void {
    const result: string = JSON.stringify([] as FixedArray<double>)
    arktest.assertEQ(result, '[]')
}

function arrDoubleSignedZero(): void {
    const result: string = JSON.stringify([0.0, -0.0] as FixedArray<double>)
    arktest.assertEQ(result, '[0,0]')
}

function arrDoubleMaxMinZero(): void {
    const result: string = JSON.stringify([Number.MAX_VALUE, Number.MIN_VALUE, Number.EPSILON] as FixedArray<double>)
    arktest.assertEQ(result, '[1.7976931348623157e+308,5e-324,2.220446049250313e-16]')
}

function arrDoubleAltSignStep1(): void {
    const result: string = JSON.stringify([1.5, -1.5, 2.5, -2.5, 0.0] as FixedArray<double>)
    arktest.assertEQ(result, '[1.5,-1.5,2.5,-2.5,0]')
}

function arrDoubleWithNaNInf(): void {
    const result: string = JSON.stringify([1.5, NaN, 2.5, Infinity, -0.0, -Infinity] as FixedArray<double>)
    arktest.assertEQ(result, '[1.5,null,2.5,null,0,null]')
}

function arrDoubleMaxMin(): void {
    const result: string = JSON.stringify([Number.MAX_VALUE, Number.MAX_VALUE, Number.MIN_VALUE,
        Number.MIN_VALUE, 0.0] as FixedArray<double>)
    arktest.assertEQ(result, '[1.7976931348623157e+308,1.7976931348623157e+308,5e-324,5e-324,0]')
}

function arrBooleanAlt(): void {
    const result: string = JSON.stringify([true, false, true, false] as FixedArray<boolean>)
    arktest.assertEQ(result, '[true,false,true,false]')
}

function emptyFixedarrayBoolean(): void {
    const result: string = JSON.stringify([] as FixedArray<boolean>)
    arktest.assertEQ(result, '[]')
}

function arrBooleanAllTrue(): void {
    const result: string = JSON.stringify([true, true, true] as FixedArray<boolean>)
    arktest.assertEQ(result, '[true,true,true]')
}

function arrBooleanAllFalse(): void {
    const result: string = JSON.stringify([false, false, false] as FixedArray<boolean>)
    arktest.assertEQ(result, '[false,false,false]')
}

function arrBooleanSingleTrue(): void {
    const result: string = JSON.stringify([true] as FixedArray<boolean>)
    arktest.assertEQ(result, '[true]')
}

function arrBooleanSingleFalse(): void {
    const result: string = JSON.stringify([false] as FixedArray<boolean>)
    arktest.assertEQ(result, '[false]')
}

function arrBooleanAltShort(): void {
    const result: string = JSON.stringify([true, false, true, false, true] as FixedArray<boolean>)
    arktest.assertEQ(result, '[true,false,true,false,true]')
}

function arrBooleanMixed(): void {
    const result: string = JSON.stringify([true, true, false, false, true, true] as FixedArray<boolean>)
    arktest.assertEQ(result, '[true,true,false,false,true,true]')
}

function arr12530100(): void {
    const result: string = JSON.stringify([1, 2.5, -3, 0, 100])
    arktest.assertEQ(result, '[1,2.5,-3,0,100]')
}

function arrNaNInfinityInfinity00(): void {
    const result: string = JSON.stringify([NaN, Infinity, -Infinity, 0, -0])
    arktest.assertEQ(result, '[null,null,null,0,0]')
}

function arrMaxMinInfZero(): void {
    const result: string = JSON.stringify([Number.MAX_VALUE, Number.MIN_VALUE, Number.EPSILON])
    arktest.assertEQ(result, '[1.7976931348623157e+308,5e-324,2.220446049250313e-16]')
}

function arrSafeIntMaxMinInf(): void {
    const result: string = JSON.stringify([Number.MAX_SAFE_INTEGER, Number.MIN_SAFE_INTEGER, 9007199254740991,
        -9007199254740991])
    arktest.assertEQ(result, '[9007199254740991,-9007199254740991,9007199254740991,-9007199254740991]')
}

function arrHexOctBinFormats(): void {
    const result: string = JSON.stringify([0x0102, 0x3, 0b1010, 0o17, 0xCAFEBABE, 0x01020304])
    arktest.assertEQ(result, '[258,3,10,15,3405691582,16909060]')
}

function arr42(): void {
    const result: string = JSON.stringify([42])
    arktest.assertEQ(result, '[42]')
}

function arrNegValues(): void {
    const result: string = JSON.stringify([-1, -256, -65535, -999999])
    arktest.assertEQ(result, '[-1,-256,-65535,-999999]')
}

function arrSignedZeroNaNInf(): void {
    const result: string = JSON.stringify([0, -0, Math.floor(1.5), Math.ceil(1.2), 2 ** 3,
        1.7976931348623157e+308 / 1e308])
    arktest.assertEQ(result, '[0,0,1,2,8,1.7976931348623157]')
}

function arrMixedMultiNaN(): void {
    const result: string = JSON.stringify([0, 0, 0, 1.5, 1.5, NaN, NaN, Infinity, Infinity])
    arktest.assertEQ(result, '[0,0,0,1.5,1.5,null,null,null,null]')
}

function arr102030(): void {
    const result: string = JSON.stringify([10, 20, 30])
    arktest.assertEQ(result, '[10,20,30]')
}

function arrNaNInfNegInfZero(): void {
    const result: string = JSON.stringify([NaN, Infinity, -Infinity])
    arktest.assertEQ(result, '[null,null,null]')
}

function arr15031499999(): void {
    const result: string = JSON.stringify([-1.5, 0, 3.14, -999.99])
    arktest.assertEQ(result, '[-1.5,0,3.14,-999.99]')
}

function arrMaxValueMinValue(): void {
    const result: string = JSON.stringify([Number.MAX_VALUE, Number.MIN_VALUE])
    arktest.assertEQ(result, '[1.7976931348623157e+308,5e-324]')
}

function arr1e525e33e10(): void {
    const result: string = JSON.stringify([1e5, 2.5e-3, -3e10])
    arktest.assertEQ(result, '[100000,0.0025,-30000000000]')
}

function arr000(): void {
    const result: string = JSON.stringify([0, -0, 0])
    arktest.assertEQ(result, '[0,0,0]')
}
