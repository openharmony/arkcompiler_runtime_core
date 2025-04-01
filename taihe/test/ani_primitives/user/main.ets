/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import * as test_primitives from "../author_generated/primitives_test";
import * as test_void_ns from "../author_generated/void_func";

loadLibrary("ani_primitives");

let primitivesTest: test_primitives.primitivesVoid =
    test_primitives.get_interface();
let primitivesBoolTest: test_primitives.primitivesBoolean =
    test_primitives.get_interface_bool();
let primitivesIntergerTest: test_primitives.primitivesInteger =
    test_primitives.get_interface_interger();

function testMultiply() {
    let res = test_primitives.multiply(2, 4)
    assertEQ(res, 8)
}


function testFunC1() {
    let res = test_primitives.baseCFunc(10)
    assertEQ(res, true)
}

function testFunC2() {
    let res = test_primitives.baseCFunc(5)
    assertEQ(res, false)
}

function testFunA1() {
    test_primitives.baseAFunc(true)
}

function testFunA2() {
    test_primitives.baseAFunc(false)
}

function testFunB1() {
    let res = test_primitives.baseBFunc(true)
    assertEQ(res, false)
}

function testFunB2() {
    let res = test_primitives.baseBFunc(false)
    assertEQ(res, true)
}

function testFunD1() {
    let res = test_primitives.baseDFunc("test123")
    assertEQ(res, true)
}

function testFunD2() {
    let res = test_primitives.baseDFunc("test1231")
    assertEQ(res, false)
}

function testFunE() {
    const option:
        test_primitives.Foo = {testNum: 2147483647, testStr: "path/to/source"};
    try {
        let res = test_primitives.baseEFunc(option)
        assertEQ(res, "success")
    } catch (error) {
        console.error("Failed to parse res:", error);
    }
}

function testFunH1() {
    let res = test_primitives.baseHFunc(2147483647, 9223372036854775807)
    assertEQ(res, "-9223372034707292162")
}

function testFunH2() {
    let res = test_primitives.baseHFunc(-2147483648, -9223372036854775808)
    assertEQ(res, "9223372034707292160")
}

function baseFunc1() {
    let res = test_primitives.baseFunc1(3.0 as float)
    assertEQ(res, 4.0)
}

function baseFunc2() {
    test_primitives.baseFunc2(3.0 as float)
}


function baseFunc3() {
    let res = test_primitives.baseFunc3(3.0 as float, 3.14159265 as double)
    console.log("tests baseFunc3" + res)
}

function baseFunc4() {
    let res = test_primitives.baseFunc4(3.0 as double)
    console.log("tests baseFunc3" + res)
}

function baseFunc5() {
    test_primitives.baseFunc5(3.14159 as double)
}

function baseFunc6() {
    test_primitives.baseFunc6("testbaseFunc6")
}

function baseFunc6_1() {
    test_primitives.baseFunc6("testbaseFunc66")
}

function baseFunc7() {
    let res = test_primitives.baseFunc7("testbaseFunc7")
    assertEQ(res, "testbaseFunc7")
}

function baseFunc7_1() {
    let res = test_primitives.baseFunc7("testbaseFunc77")
    assertEQ(res, "testbaseFunc77false")
}

function baseFunc8() {
    let res = test_primitives.baseFunc8("testbaseFunc8", 32)
    assertEQ(res, "testbaseFunc832")
}

function baseFunc8_1() {
    let res = test_primitives.baseFunc8("testbaseFunc88", 32)
    assertEQ(res, "testbaseFunc88")
}

function baseFunc9() {
    test_primitives.baseFunc9(
        "testbaseFunc9", 32, 9223372036854775807, true, 3.1 as float)
}

function testFunI1() {
    let res =
        test_primitives.baseIFunc(3.1415926535897931 as double, 4.3 as float)
    console.log("testFunI1 ", res)
    assertEQ(res, "7.441593")
}

// function testFunI2(){
//     let res = test_primitives.baseIFunc(-1234567890.1234567 as double,-123.4
//     as float) console.log("testFunI2 ",res)
//     assertEQ(res,"-1234568013.523457")
// }


function baseFunc10() {
    test_primitives.baseFunc10()
}

function baseFunc11() {
    test_primitives.baseFunc11(32, true)
}

function baseFunc11_1() {
    test_primitives.baseFunc11(32, false)
}

function baseFunc12() {
    test_primitives.baseFunc12(32, 92233720368547758)
}

function baseFunc13() {
    test_primitives.baseFunc13(32, "testbaseFunc13")
}

function baseFunc14() {
    test_primitives.baseFunc14(92233720368547758, true)
}


function baseFunc14_1() {
    test_primitives.baseFunc14(92233720368547758, false)
}

function baseFunc15() {
    test_primitives.baseFunc15(32, 3.15 as float)
}

function baseFunc16() {
    test_primitives.baseFunc16(32, 3.141592653589793)
}

function baseFunc16_1() {
    test_primitives.baseFunc16(32, -3.141592653589793)
}


function baseFunc17() {
    test_primitives.baseFunc17(123.456 as float, true)
}

function baseFunc17_1() {
    test_primitives.baseFunc17(3.14159 as float, false)
}

function baseFunc18() {
    test_primitives.baseFunc18(123.456 as float, "testbaseFunc18")
}

function baseFunc18_1() {
    test_primitives.baseFunc18(3.14159 as float, "testbaseFunc18")
}

function baseFunc19() {
    test_primitives.baseFunc19(3.141592653589793, "testbaseFunc19")
}

function baseFunc20() {
    test_primitives.baseFunc20(3.141592653589793, true)
}

function baseFunc20_1() {
    test_primitives.baseFunc20(3.141592653589793, false)
}

function baseFunc21() {
    test_primitives.baseFunc21(32 as int, 65535 as long)
}

function baseFunc21_1() {
    test_primitives.baseFunc21(undefined, 65535 as long)
}

function baseFunc21_2() {
    test_primitives.baseFunc21(undefined, undefined)
}

function baseFunc21_3() {
    test_primitives.baseFunc21(32 as int, undefined)
}

function baseFunc22() {
    test_primitives.baseFunc22(3.14159 as float, 3.141592653589793)
}

function baseFunc22_1() {
    test_primitives.baseFunc22(undefined, 3.141592653589793)
}

function baseFunc22_2() {
    test_primitives.baseFunc22(undefined, undefined)
}

function baseFunc22_3() {
    test_primitives.baseFunc22(3.14159 as float, undefined)
}

function baseFunc23() {
    test_primitives.baseFunc23("testBaseFunc23", true)
}

function baseFunc23_1() {
    test_primitives.baseFunc23(undefined, false)
}

function baseFunc23_2() {
    test_primitives.baseFunc23(undefined, undefined)
}

function baseFunc23_3() {
    test_primitives.baseFunc23("testBaseFunc23", undefined)
}

function baseFunc24() {
    test_primitives.baseFunc24(32767 as short, 92233720368547758)
}

function baseFunc24_1() {
    test_primitives.baseFunc24(undefined, 92233720368547758)
}

function baseFunc24_2() {
    test_primitives.baseFunc24(undefined, undefined)
}

function baseFunc24_3() {
    test_primitives.baseFunc24(32767 as short, undefined)
}

function baseFunc25() {
    test_primitives.baseFunc25(32767 as short, test_primitives.Bar.GREEN)
}

function baseFunc25_1() {
    test_primitives.baseFunc25(32767 as short, test_primitives.Bar.BLUE)
}

function baseFunc25_2() {
    test_primitives.baseFunc25(32767 as short, test_primitives.Bar.RED)
}

function testCaseBaseFunc1() {
    primitivesTest.testBaseFunc1()
}

function testCaseBaseFunc2() {
    primitivesTest.testBaseFunc2(32, true)
}

function testCaseBaseFunc2_1() {
    primitivesTest.testBaseFunc2(32, false)
}

function testCaseBaseFunc3() {
    primitivesTest.testBaseFunc3(-2147483648, 92233720368547758)
}

function testCaseBaseFunc4() {
    primitivesTest.testBaseFunc4(2147483647, "testBaseFunc4")
}

function testCaseBaseFunc5() {
    primitivesTest.testBaseFunc5(-92233720368547758, true)
}


function testCaseBaseFunc5_1() {
    primitivesTest.testBaseFunc5(-92233720368547758, false)
}

function testCaseBaseFunc6() {
    primitivesTest.testBaseFunc6(32, 3.15 as float)
}

function testCaseBaseFunc7() {
    primitivesTest.testBaseFunc7(9223372036854775807, 3.141592653589793)
}

function testCaseBaseFunc7_1() {
    primitivesTest.testBaseFunc7(-9223372036854775808, -3.141592653589793)
}


function testCaseBaseFunc8() {
    primitivesTest.testBaseFunc8(123.456789 as float, true)
}

function testCaseBaseFunc8_1() {
    primitivesTest.testBaseFunc8(3.1415978 as float, false)
}

function testCaseBaseFunc9() {
    primitivesTest.testBaseFunc9(123.456789 as float, "testBaseFunc9")
}

function testCaseBaseFunc9_1() {
    primitivesTest.testBaseFunc9(3.1415978 as float, "testBaseFunc9")
}

function testCaseBaseFunc10() {
    primitivesTest.testBaseFunc10(-3.141592653589793, "testBaseFunc10")
}

function testCaseBaseFunc11() {
    primitivesTest.testBaseFunc11(-3.141592653589793, true)
}

function testCaseBaseFunc11_1() {
    primitivesTest.testBaseFunc11(-3.141592653589793, false)
}

function testCaseBaseFunc12() {
    primitivesTest.testBaseFunc12(256, 6553565536 as long)
}

function testCaseBaseFunc12_1() {
    primitivesTest.testBaseFunc12(undefined, 65535 as long)
}

function testCaseBaseFunc12_2() {
    primitivesTest.testBaseFunc12(undefined, undefined)
}

function testCaseBaseFunc12_3() {
    primitivesTest.testBaseFunc12(-65535, undefined)
}

function testCaseBaseFunc13() {
    primitivesTest.testBaseFunc13(-3.1415952 as float, 3.141592653589793)
}

function testCaseBaseFunc13_1() {
    primitivesTest.testBaseFunc13(undefined, 3.141592653589793)
}

function testCaseBaseFunc13_2() {
    primitivesTest.testBaseFunc13(undefined, undefined)
}

function testCaseBaseFunc13_3() {
    primitivesTest.testBaseFunc13(-3.1415952 as float, undefined)
}

function testCaseBaseFunc14() {
    primitivesTest.testBaseFunc14("testBaseFunc23", true)
}

function testCaseBaseFunc14_1() {
    primitivesTest.testBaseFunc14(undefined, false)
}

function testCaseBaseFunc14_2() {
    primitivesTest.testBaseFunc14(undefined, undefined)
}

function testCaseBaseFunc14_3() {
    primitivesTest.testBaseFunc14("testBaseFunc14", undefined)
}

function testCaseBaseFunc15() {
    primitivesTest.testBaseFunc15(32767 as short, 92233720368547758)
}

function testCaseBaseFunc15_1() {
    primitivesTest.testBaseFunc15(undefined, 92233720368547758)
}

function testCaseBaseFunc15_2() {
    primitivesTest.testBaseFunc15(undefined, undefined)
}

function testCaseBaseFunc15_3() {
    primitivesTest.testBaseFunc15(-32768 as short, undefined)
}

function testCaseBaseFunc16() {
    primitivesTest.testBaseFunc16(127 as byte, 32765 as short)
}

function testCaseBaseFunc16_1() {
    primitivesTest.testBaseFunc16(-128 as byte, -32765 as short)
}


function testCaseBaseFunc17() {
    let intArray: int[] = [1, 2, 3, 4, 5];
    let byteArray: byte[] = [100, 101, 102];
    primitivesTest.testBaseFunc17(intArray, byteArray)
}

function testCaseBaseFunc18() {
    let shortArray: short[] = [32767, 32767, -32768, 4, 5];
    let longArray: long[] =
        [10086, 10010, 10000, 9223372036854775807, -9223372036854775808];
    primitivesTest.testBaseFunc18(shortArray, longArray)
}

function testCaseBaseFunc19() {
    let floatArray: float[] = [3.14159, 123.0, -45678.5454, 0.052, 0.01];
    let doubleArray: double[] = [
        10086.123456, -156.45678945, 3.141592345454545, 9223372036.854775807,
        -922337203.6854775808
    ];
    console.log("testCaseBaseFunc19 floatArray ", floatArray)
    console.log("testCaseBaseFunc19 doubleArray ", doubleArray)
    primitivesTest.testBaseFunc19(floatArray, doubleArray)
}

function testCaseBaseFunc20() {
    let booleanArray: boolean[] = [true, false, false, true, true];
    let strArray: string[] =
        ["test1", "test111", "12345~!<>?|{}[]", "test333", "@#$$%^&&*123"];
    primitivesTest.testBaseFunc20(booleanArray, strArray)
}

function testCaseBaseIntegerFunc1() {
    try {
        let temp = 255 as byte;
        console.log("test error 1 ", temp)
        let res = primitivesIntergerTest.testBaseIntegerFunc1(temp)
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: out of range The i8 maximum value is 127 and minnum values is -128")
    }
}

function testCaseBaseIntegerFunc1_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc1(100 as byte)
    assertEQ(res, 101)
}

function testCaseBaseIntegerFunc2() {
    try {
        let temp = 255 as byte;
        console.log("test error 2 ", temp)
        let res =
            primitivesIntergerTest.testBaseIntegerFunc2(temp, 100 as short)
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: out of range The i8 maximum value is 127 and minnum values is -128")
    }
}

function testCaseBaseIntegerFunc2_1() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc2(100 as byte, 27 as short)
    assertEQ(res, 127)
}

function testCaseBaseIntegerFunc2_2() {
    try {
        let temp = -1 as short;
        console.log("test error 2_2 ", temp)
        let res = primitivesIntergerTest.testBaseIntegerFunc2(100 as byte, temp)
    } catch (e) {
        console.log("testCaseBaseIntegerFunc2_2 error ", e.toString())
        assertEQ(
            e.toString(),
            "Error: out of range The i16 maximum value is 32767 and minnum values is -32768")
    }
}


function testCaseBaseIntegerFunc2_3() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc2(100 as byte, 32667 as short)
    assertEQ(res, -1)
}

function testCaseBaseIntegerFunc3() {
    primitivesIntergerTest.testBaseIntegerFunc3(100 as byte, 27 as short)
}

function testCaseBaseIntegerFunc4() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc4(100 as byte, 32667 as short)
    assertEQ(res, 32767)
}

function testCaseBaseIntegerFunc4_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc4(
        127 as byte, -32768 as short)
    assertEQ(res, -32641)
}

function testCaseBaseIntegerFunc4_2() {
    let res = primitivesIntergerTest.testBaseIntegerFunc4(0 as byte, 0 as short)
    assertEQ(res, 0)
}

function testCaseBaseIntegerFunc5() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc5(100 as byte, 2147483547)
    assertEQ(res, -1)
}

function testCaseBaseIntegerFunc5_1() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc5(-127 as byte, -2147483648)
    assertEQ(res, -127)
}

function testCaseBaseIntegerFunc5_2() {
    let res = primitivesIntergerTest.testBaseIntegerFunc5(0 as byte, 100)
    assertEQ(res, 100)
}

function testCaseBaseIntegerFunc6() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc6(100 as byte, 2147483547)
    assertEQ(res, 2147483647)
}

function testCaseBaseIntegerFunc6_1() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc6(-127 as byte, -2147483648)
    assertEQ(res, 2147483521)
}

function testCaseBaseIntegerFunc6_2() {
    let res = primitivesIntergerTest.testBaseIntegerFunc6(0 as byte, 0)
    assertEQ(res, 0)
}

function testCaseBaseIntegerFunc7() {
    primitivesIntergerTest.testBaseIntegerFunc7(100 as byte, 2147483647)
}

function testCaseBaseIntegerFunc7_1() {
    try {
        primitivesIntergerTest.testBaseIntegerFunc7(100 as byte, -1)
    } catch (e) {
        console.log("testCaseBaseIntegerFunc7_1 error ", e.toString())
        assertEQ(
            e.toString(),
            "Error: out of range The i32 maximum value is 2147483647 and minnum values is -2147483648")
    }
}

function testCaseBaseIntegerFunc8() {
    let res = primitivesIntergerTest.testBaseIntegerFunc8(
        100 as byte, 9223372036854775807)
    assertEQ(res, 9223372036854775707)
}

function testCaseBaseIntegerFunc8_1() {
    try {
        primitivesIntergerTest.testBaseIntegerFunc8(100 as byte, -1)
    } catch (e) {
        console.log("testCaseBaseIntegerFunc8_1 error ", e.toString())
        assertEQ(
            e.toString(),
            "Error: out of range The i64 maximum value is 9223372036854775807 and minnum values is -9223372036854775808")
    }
}

function testCaseBaseIntegerFunc8_2() {
    let res = primitivesIntergerTest.testBaseIntegerFunc8(
        100 as byte, -9223372036854775808)
    assertEQ(res, 9223372036854775708)
}

function testCaseBaseIntegerFunc9() {
    let res = primitivesIntergerTest.testBaseIntegerFunc9(100 as byte, 227)
    assertEQ(res, 127)
}

function testCaseBaseIntegerFunc9_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc9(-7 as byte, -120)
    assertEQ(res, -113)
}

function testCaseBaseIntegerFunc10() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc10(100 as byte, 3.14 as float)
    assertEQ(res, 103.14 as float)
}

function testCaseBaseIntegerFunc10_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc10(
        -7 as byte, -0.0005 as float)
    assertEQ(res, -7.0005 as float)
}

function testCaseBaseIntegerFunc11() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc11(100 as byte, 3.14 as float)
    assertEQ(res, 103)
}

function testCaseBaseIntegerFunc11_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc11(
        -7 as byte, -0.0005 as float)
    assertEQ(res, -7)
}

function testCaseBaseIntegerFunc12() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc12(100 as byte, 3.1415926)
    assertEQ(res, 103.1415926)
}

function testCaseBaseIntegerFunc12_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc12(-7 as byte, -0.0005)
    assertEQ(res, -7.0005)
}

function testCaseBaseIntegerFunc13() {
    let res = primitivesIntergerTest.testBaseIntegerFunc13(100 as byte, 27)
    assertEQ(res, 127)
}

function testCaseBaseIntegerFunc13_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc13(-7 as byte, -121)
    assertEQ(res, -128)
}

function testCaseBaseIntegerFunc14() {
    let res = primitivesIntergerTest.testBaseIntegerFunc14(
        100 as byte, "testBaseIntegerFunc14")
    assertEQ(res, "testBaseIntegerFunc14100")
}

function testCaseBaseIntegerFunc14_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc14(
        -7 as byte, "testBaseIntegerFunc14_1")
    assertEQ(res, "testBaseIntegerFunc14_1")
}

function testCaseBaseIntegerFunc15() {
    let res = primitivesIntergerTest.testBaseIntegerFunc15(
        100 as byte, "testBaseIntegerFunc15")
    assertEQ(res, 110)
}

function testCaseBaseIntegerFunc15_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc15(
        -7 as byte, "testBaseIntegerFunc15_1")
    assertEQ(res, -7)
}

function testCaseBaseIntegerFunc16() {
    let res = primitivesIntergerTest.testBaseIntegerFunc16(100 as byte, true)
    assertEQ(res, true)
}

function testCaseBaseIntegerFunc16_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc16(-7 as byte, false)
    assertEQ(res, false)
}

function testCaseBaseIntegerFunc17() {
    let res = primitivesIntergerTest.testBaseIntegerFunc17(126 as byte, true)
    assertEQ(res, 127)
}

function testCaseBaseIntegerFunc17_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc17(-126 as byte, false)
    assertEQ(res, -126)
}

function testCaseBaseIntegerFunc18() {
    try {
        let res = primitivesIntergerTest.testBaseIntegerFunc18(32767 as short)
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: testBaseIntegerFunc18: result exceeds int16_t range")
    }
}

function testCaseBaseIntegerFunc18_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc18(327 as short)
    assertEQ(res, 3270)
}

function testCaseBaseIntegerFunc19() {
    primitivesIntergerTest.testBaseIntegerFunc19(32767 as short)
}

function testCaseBaseIntegerFunc19_1() {
    primitivesIntergerTest.testBaseIntegerFunc19(-32768 as short)
}

function testCaseBaseIntegerFunc20() {
    try {
        let res =
            primitivesIntergerTest.testBaseIntegerFunc20(32767 as short, 100)
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: testBaseIntegerFunc20: result exceeds int16_t range")
    }
}

function testCaseBaseIntegerFunc20_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc20(30000 as short, 2767)
    assertEQ(res, 32767)
}

function testCaseBaseIntegerFunc21() {
    try {
        let res = primitivesIntergerTest.testBaseIntegerFunc21(
            100 as short, 9223372036854775807)
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: testBaseIntegerFunc21: result exceeds int16_t range")
    }
}

function testCaseBaseIntegerFunc21_1() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc21(-30000 as short, -2768)
    assertEQ(res, -32768)
}

function testCaseBaseIntegerFunc22() {
    try {
        let res = primitivesIntergerTest.testBaseIntegerFunc22(2147483647)
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: testBaseIntegerFunc22: result exceeds int32_t range")
    }
}

function testCaseBaseIntegerFunc22_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc22(-21474836)
    assertEQ(res, -2147483600)
}

function testCaseBaseIntegerFunc23() {
    primitivesIntergerTest.testBaseIntegerFunc23(2147483647)
}

function testCaseBaseIntegerFunc23_1() {
    try {
        primitivesIntergerTest.testBaseIntegerFunc23(922337203)
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: testBaseIntegerFunc22: result exceeds int32_t range")
    }
}

function testCaseBaseIntegerFunc24() {
    let res = primitivesIntergerTest.testBaseIntegerFunc24(2147483646, 1)
    assertEQ(res, 2147483647)
}

function testCaseBaseIntegerFunc24_1() {
    try {
        primitivesIntergerTest.testBaseIntegerFunc24(32767, 9223372036854775807)
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: testBaseIntegerFunc24: result exceeds int32_t range")
    }
}

function testCaseBaseIntegerFunc25() {
    let res =
        primitivesIntergerTest.testBaseIntegerFunc25(-2147483646, -2 as byte)
    assertEQ(res, -2147483648)
}

function testCaseBaseIntegerFunc25_1() {
    try {
        primitivesIntergerTest.testBaseIntegerFunc25(2147483647, 127 as byte)
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: testBaseIntegerFunc24: result exceeds int32_t range")
    }
}

function testCaseBaseIntegerFunc26() {
    let res = primitivesIntergerTest.testBaseIntegerFunc26(9223372036854775)
    assertEQ(res, 922337203685477500)
}

function testCaseBaseIntegerFunc26_1() {
    try {
        let res =
            primitivesIntergerTest.testBaseIntegerFunc26(9223372036854775807)
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: testBaseIntegerFunc24: result exceeds int64_t range")
    }
}

function testCaseBaseIntegerFunc27() {
    primitivesIntergerTest.testBaseIntegerFunc27(-9223372036854775808)
}

function testCaseBaseIntegerFunc28() {
    let res = primitivesIntergerTest.testBaseIntegerFunc28(
        9223372036854775, "testBaseIntegerFunc28")
    assertEQ(res, "testBaseIntegerFunc289223372036854775")
}

function testCaseBaseIntegerFunc28_1() {
    let res = primitivesIntergerTest.testBaseIntegerFunc28(
        9223372036854775, "testCaseBaseIntegerFunc28_1")
    assertEQ(res, "testCaseBaseIntegerFunc28_1")
}


function testCaseBaseIntegerFunc29() {
    let res = primitivesIntergerTest.testBaseIntegerFunc29(
        9223372036854, "testBaseIntegerFunc29")
    assertEQ(res, 92233720368540)
}

function testCaseBaseIntegerFunc29_1() {
    try {
        let res = primitivesIntergerTest.testBaseIntegerFunc29(
            9223372036854775807, "testBaseIntegerFunc29_1")
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: testBaseIntegerFunc24: result exceeds int64_t range")
    }
}

function testCaseBaseIntegerFunc30() {
    let res: float =
        primitivesIntergerTest.testBaseIntegerFunc30(3.1415 as float)
    assertEQ(res, 4.1415 as float)
}

function testCaseBaseIntegerFunc30_1() {
    let res: float =
        primitivesIntergerTest.testBaseIntegerFunc30(-0.015 as float)
    assertEQ(res, 0.985 as float)
}

function testCaseBaseIntegerFunc31() {
    primitivesIntergerTest.testBaseIntegerFunc31(-0.0151456 as float)
}

function testCaseBaseIntegerFunc31_1() {
    primitivesIntergerTest.testBaseIntegerFunc31(1.000123456789 as float)
}

function testCaseBaseIntegerFunc32() {
    let res: float =
        primitivesIntergerTest.testBaseIntegerFunc32(-0.015 as float, 1.0)
    assertEQ(res, 0.985 as float)
}

function testCaseBaseIntegerFunc32_1() {
    let res: float =
        primitivesIntergerTest.testBaseIntegerFunc32(0.0001 as float, 100.12)
    assertEQ(res, 100.1201 as float)
}

function testCaseBaseIntegerFunc33() {
    let res: double = primitivesIntergerTest.testBaseIntegerFunc33(
        123.456 as float, 1000.11101)
    console.log("testCaseBaseIntegerFunc33 res ", res)
    let temp: double = Math.abs(res - 1123.56701);
    console.log("testCaseBaseIntegerFunc33 temp ", temp)
    const epsilon = Math.pow(10, -5);
    console.log("testCaseBaseIntegerFunc33 epsilon ", epsilon)
    let dif: boolean = temp < epsilon
    assertEQ(dif, true)
}

function testCaseBaseIntegerFunc33_1() {
    let res: double =
        primitivesIntergerTest.testBaseIntegerFunc33(-100.1111111 as float, 100)
    console.log("testCaseBaseIntegerFunc33_1 res ", res)
    let temp: double = Math.abs(res + 0.1111111);
    console.log("testCaseBaseIntegerFunc33_1 temp ", temp)
    const epsilon = Math.pow(10, -5);
    console.log("testCaseBaseIntegerFunc33_1 epsilon ", epsilon)
    let dif: boolean = temp < epsilon
    assertEQ(dif, true)
}

function testCaseBaseIntegerFunc34() {
    let res: double =
        primitivesIntergerTest.testBaseIntegerFunc34(3.141592653589793)
    assertEQ(res, 4.141592653589793)
}

function testCaseBaseIntegerFunc34_1() {
    let res: double =
        primitivesIntergerTest.testBaseIntegerFunc34(-0.000123456789)
    assertEQ(res, 0.999876543211)
}

function testCaseBaseIntegerFunc35() {
    primitivesIntergerTest.testBaseIntegerFunc35(123456789.123456)
}

function testCaseBaseIntegerFunc35_1() {
    primitivesIntergerTest.testBaseIntegerFunc35(123456789.123456789123456789)
}

function testCasetestBaseBoolFunc1() {
    primitivesBoolTest.testBaseBoolFunc1(true)
}

function testCasetestBaseBoolFunc1_1() {
    primitivesBoolTest.testBaseBoolFunc1(false)
}

function testCasetestBaseBoolFunc2() {
    let res = primitivesBoolTest.testBaseBoolFunc2(true)
    assertEQ(res, 65535)
}

function testCasetestBaseBoolFunc2_1() {
    let res = primitivesBoolTest.testBaseBoolFunc2(false)
    assertEQ(res, 0)
}

function testCasetestBaseBoolFunc3() {
    let res = primitivesBoolTest.testBaseBoolFunc3(true)
    assertEQ(res, false)
}

function testCasetestBaseBoolFunc3_1() {
    let res = primitivesBoolTest.testBaseBoolFunc3(false)
    assertEQ(res, true)
}

function testCasetestBaseBoolFunc4() {
    let res = primitivesBoolTest.testBaseBoolFunc4(true)
    assertEQ(res, false)
}

function testCasetestBaseBoolFunc4_1() {
    let res = primitivesBoolTest.testBaseBoolFunc4(false)
    assertEQ(res, false)
}

function testCasetestBaseBoolFunc4_2() {
    let res = primitivesBoolTest.testBaseBoolFunc4(undefined)
    assertEQ(res, true)
}

function testCasetestBaseBoolFunc5() {
    primitivesBoolTest.testBaseBoolFunc5Async(
        true, (error: Error, data?: boolean) => {
            if (!error.message) {
                console.log(
                    "main finsih test testCasetestBaseBoolFunc5 success ",
                    data);
                assertEQ(data, true);
            } else {
                console.log(
                    "main finsih test testCasetestBaseBoolFunc5 ERROR ", error);
            }
        })
}

function testCasetestBaseBoolFunc5_1() {
    primitivesBoolTest.testBaseBoolFunc5Async(
        false, (error: Error, data?: boolean) => {
            if (!error.message) {
                console.log(
                    "main finsih test testCasetestBaseBoolFunc5_1 success ",
                    data);
                assertEQ(data, false);
            } else {
                console.log(
                    "main finsih test testCasetestBaseBoolFunc5_1 ERROR ",
                    error);
            }
        })
}

function testCasetestBaseBoolFunc5_2() {
    try {
        let result: boolean =
            await primitivesBoolTest.testBaseBoolFunc5Promise(true);
        assertEQ(result, true);
        console.log("testCasetestBaseBoolFunc5_2 result: " + result);
    } catch (error) {
        console.error("testCasetestBaseBoolFunc5_2 failed: " + error);
    }
}

function testCasetestBaseBoolFunc5_3() {
    let result: boolean = primitivesBoolTest.testBaseBoolFunc5(true);
    assertEQ(result, true);
}


function testNsMyFunc1() {
    test_void_ns.mytest.myfunc1();
}

function testNsMyFunc2() {
    test_void_ns.mytest.myfunc2(120 as byte, 255 as short);
}

function testNsMyFunc3() {
    test_void_ns.mytest.myfunc3(10000, true);
}

function testNsMyFunc3_1() {
    test_void_ns.mytest.myfunc3(10000, false);
}

function testNsMyFunc4() {
    test_void_ns.mytest.myfunc4(25555, 9223372036854775807);
}

function testNsMyFunc5() {
    test_void_ns.mytest.myfunc5(2147483647, "testNsMyFunc5");
}

function testNsMyFunc6() {
    test_void_ns.mytest.myfunc6(-9223372036854775808, true);
}

function testNsMyFunc6_1() {
    test_void_ns.mytest.myfunc6(2147483647, false);
}

function testNsMyFunc7() {
    test_void_ns.mytest.myfunc7(-2147483647, 3.12 as float);
}

function testNsMyFunc8() {
    test_void_ns.mytest.myfunc8(9223372036854775807, 1.123456789);
}


function testNsMyFunc9() {
    test_void_ns.mytest.myfunc9(0.000123 as float, true);
}

function testNsMyFunc10() {
    test_void_ns.mytest.myfunc10(0.000123 as float, "testNsMyFunc10");
}

function testNsMyFunc11() {
    test_void_ns.mytest.myfunc11(-0.000123, "testNsMyFunc11");
}

function testNsMyFunc12() {
    test_void_ns.mytest.myfunc12(2147483647, 9223372036854775807);
}

function testNsMyFunc12_1() {
    test_void_ns.mytest.myfunc12(undefined, -9223372036854775808);
}

function testNsMyFunc12_2() {
    test_void_ns.mytest.myfunc12(undefined, undefined);
}

function testNsMyFunc13() {
    test_void_ns.mytest.myfunc13(3.11111 as float, 1.1245645);
}

function testNsMyFunc13_1() {
    test_void_ns.mytest.myfunc13(-3.11111 as float, undefined);
}

function testNsMyFunc13_2() {
    test_void_ns.mytest.myfunc13(undefined, undefined);
}

function testNsMyFunc14() {
    test_void_ns.mytest.myfunc14("testNsMyFunc14", true);
}

function testNsMyFunc14_1() {
    test_void_ns.mytest.myfunc14(undefined, undefined);
}

function testNsMyFunc14_2() {
    test_void_ns.mytest.myfunc14(undefined, false);
}

function testNsMyFunc15() {
    test_void_ns.mytest.myfunc15(32767 as short, 92233720368547758);
}

function testNsMyFunc15_1() {
    test_void_ns.mytest.myfunc15(-214748364 as short, undefined);
}

function testNsMyFunc15_2() {
    test_void_ns.mytest.myfunc15(undefined, undefined);
}

function testNsMyFunc16() {
    test_void_ns.mytest.myfunc16(255 as short, test_void_ns.mytest.BarTest.B)
}

function testNsMyFunc16_1() {
    test_void_ns.mytest.myfunc16(-32768 as short, test_void_ns.mytest.BarTest.A)
}

function testNsMyFunc16_2() {
    test_void_ns.mytest.myfunc16(0 as short, test_void_ns.mytest.BarTest.C)
}



function main() {
    console.log("run main ... test_primitives ...")
    const suite = new ArkTestsuite("test function primitives")

    suite.addTest("check funcMultiply", testMultiply)
    suite.addTest("check testFunC1", testFunC1)
    suite.addTest("check testFunC2", testFunC2)
    suite.addTest("check testFunA1", testFunA1)
    suite.addTest("check testFunA2", testFunA2)
    suite.addTest("check testFunB1", testFunB1)
    suite.addTest("check testFunB2", testFunB2)
    suite.addTest("check testFunD1", testFunD1)
    suite.addTest("check testFunD2", testFunD2)
    suite.addTest("check testFunE", testFunE)
    suite.addTest("check testFunH1", testFunH1)
    suite.addTest("check testFunH2", testFunH2)
    suite.addTest("check testFunI1", testFunI1)
    // suite.addTest("check testFunI2", testFunI2)
    suite.addTest("check testbaseFunc1", baseFunc1)
    suite.addTest("check testbaseFunc2", baseFunc2)
    suite.addTest("check testbaseFunc3", baseFunc3)
    suite.addTest("check testbaseFunc4", baseFunc4)
    suite.addTest("check testbaseFunc5", baseFunc5)
    suite.addTest("check testbaseFunc6", baseFunc6)
    suite.addTest("check testbaseFunc7", baseFunc7)
    suite.addTest("check testbaseFunc8", baseFunc8)
    suite.addTest("check testbaseFunc6_1", baseFunc6_1)
    suite.addTest("check testbaseFunc7_1", baseFunc7_1)
    suite.addTest("check testbaseFunc8_1", baseFunc8_1)
    suite.addTest("check testbaseFunc9", baseFunc9)
    suite.addTest("check testbaseFunc10", baseFunc10)
    suite.addTest("check testbaseFunc11", baseFunc11)
    suite.addTest("check testbaseFunc11_1", baseFunc11_1)
    suite.addTest("check testbaseFunc12", baseFunc12)
    suite.addTest("check testbaseFunc13", baseFunc13)
    suite.addTest("check testbaseFunc14", baseFunc14)
    suite.addTest("check testbaseFunc14_1", baseFunc14_1)
    suite.addTest("check testbaseFunc15", baseFunc15)
    suite.addTest("check testbaseFunc16", baseFunc16)
    suite.addTest("check testbaseFunc16_1", baseFunc16_1)
    suite.addTest("check testbaseFunc17", baseFunc17)
    suite.addTest("check testbaseFunc17_1", baseFunc17_1)
    suite.addTest("check testbaseFunc18", baseFunc18)
    suite.addTest("check testbaseFunc18_1", baseFunc18_1)
    suite.addTest("check testbaseFunc19", baseFunc19)
    suite.addTest("check testbaseFunc20", baseFunc20)
    suite.addTest("check testbaseFunc20_1", baseFunc20_1)
    suite.addTest("check testbaseFunc21", baseFunc21)
    suite.addTest("check testbaseFunc21_1", baseFunc21_1)
    suite.addTest("check testbaseFunc21_2", baseFunc21_2)
    suite.addTest("check testbaseFunc21_3", baseFunc21_3)
    suite.addTest("check testbaseFunc22", baseFunc22)
    suite.addTest("check testbaseFunc22_1", baseFunc22_1)
    suite.addTest("check testbaseFunc22_2", baseFunc22_2)
    suite.addTest("check testbaseFunc22_3", baseFunc22_3)
    suite.addTest("check testbaseFunc23", baseFunc23)
    suite.addTest("check testbaseFunc23_1", baseFunc23_1)
    suite.addTest("check testbaseFunc23_2", baseFunc23_2)
    suite.addTest("check testbaseFunc23_3", baseFunc23_3)
    suite.addTest("check testbaseFunc24", baseFunc24)
    suite.addTest("check testbaseFunc24_1", baseFunc24_1)
    suite.addTest("check testbaseFunc24_2", baseFunc24_2)
    suite.addTest("check testbaseFunc24_3", baseFunc24_3)
    suite.addTest("check testCaseBaseFunc1", testCaseBaseFunc1)
    suite.addTest("check testCaseBaseFunc2", testCaseBaseFunc2)
    suite.addTest("check testCaseBaseFunc2_1", testCaseBaseFunc2_1)
    suite.addTest("check testCaseBaseFunc3", testCaseBaseFunc3)
    suite.addTest("check testCaseBaseFunc4", testCaseBaseFunc4)
    suite.addTest("check testCaseBaseFunc5", testCaseBaseFunc5)
    suite.addTest("check testCaseBaseFunc5_1", testCaseBaseFunc5_1)
    suite.addTest("check testCaseBaseFunc6", testCaseBaseFunc6)
    suite.addTest("check testCaseBaseFunc7", testCaseBaseFunc7)
    suite.addTest("check testCaseBaseFunc7_1", testCaseBaseFunc7_1)
    suite.addTest("check testCaseBaseFunc8", testCaseBaseFunc8)
    suite.addTest("check testCaseBaseFunc8_1", testCaseBaseFunc8_1)
    suite.addTest("check testCaseBaseFunc9", testCaseBaseFunc9)
    suite.addTest("check testCaseBaseFunc9_1", testCaseBaseFunc9_1)
    suite.addTest("check testCaseBaseFunc10", testCaseBaseFunc10)
    suite.addTest("check testCaseBaseFunc11", testCaseBaseFunc11)
    suite.addTest("check testCaseBaseFunc11_1", testCaseBaseFunc11_1)
    suite.addTest("check testCaseBaseFunc12", testCaseBaseFunc12)
    suite.addTest("check testCaseBaseFunc12_1", testCaseBaseFunc12_1)
    suite.addTest("check testCaseBaseFunc12_2", testCaseBaseFunc12_2)
    suite.addTest("check testCaseBaseFunc12_3", testCaseBaseFunc12_3)
    suite.addTest("check testCaseBaseFunc13", testCaseBaseFunc13)
    suite.addTest("check testCaseBaseFunc13_1", testCaseBaseFunc13_1)
    suite.addTest("check testCaseBaseFunc13_2", testCaseBaseFunc13_2)
    suite.addTest("check testCaseBaseFunc13_3", testCaseBaseFunc13_3)
    suite.addTest("check testCaseBaseFunc14", testCaseBaseFunc14)
    suite.addTest("check testCaseBaseFunc14_1", testCaseBaseFunc14_1)
    suite.addTest("check testCaseBaseFunc14_2", testCaseBaseFunc14_2)
    suite.addTest("check testCaseBaseFunc14_3", testCaseBaseFunc14_3)
    suite.addTest("check testCaseBaseFunc15", testCaseBaseFunc15)
    suite.addTest("check testCaseBaseFunc15_1", testCaseBaseFunc15_1)
    suite.addTest("check testCaseBaseFunc15_2", testCaseBaseFunc15_2)
    suite.addTest("check testCaseBaseFunc15_3", testCaseBaseFunc15_3)
    suite.addTest("check testCaseBaseFunc16", testCaseBaseFunc16)
    suite.addTest("check testCaseBaseFunc16_1", testCaseBaseFunc16_1)
    suite.addTest("check testCaseBaseFunc17", testCaseBaseFunc17)
    suite.addTest("check testCaseBaseFunc18", testCaseBaseFunc18)
    suite.addTest("check testCaseBaseFunc19", testCaseBaseFunc19)
    suite.addTest("check testCaseBaseFunc20", testCaseBaseFunc20)
    suite.addTest("check testCaseBaseIntegerFunc1", testCaseBaseIntegerFunc1)
    suite.addTest(
        "check testCaseBaseIntegerFunc1_1", testCaseBaseIntegerFunc1_1)
    suite.addTest("check testCaseBaseIntegerFunc2", testCaseBaseIntegerFunc2)
    suite.addTest(
        "check testCaseBaseIntegerFunc2_1", testCaseBaseIntegerFunc2_1)
    suite.addTest(
        "check testCaseBaseIntegerFunc2_2", testCaseBaseIntegerFunc2_2)
    suite.addTest(
        "check testCaseBaseIntegerFunc2_3", testCaseBaseIntegerFunc2_3)
    suite.addTest("check testCaseBaseIntegerFunc3", testCaseBaseIntegerFunc3)
    suite.addTest("check testCaseBaseIntegerFunc4", testCaseBaseIntegerFunc4)
    suite.addTest(
        "check testCaseBaseIntegerFunc4_1", testCaseBaseIntegerFunc4_1)
    suite.addTest(
        "check testCaseBaseIntegerFunc4_2", testCaseBaseIntegerFunc4_2)
    suite.addTest("check testCaseBaseIntegerFunc5", testCaseBaseIntegerFunc5)
    suite.addTest(
        "check testCaseBaseIntegerFunc5_1", testCaseBaseIntegerFunc5_1)
    suite.addTest(
        "check testCaseBaseIntegerFunc5_2", testCaseBaseIntegerFunc5_2)
    suite.addTest("check testCaseBaseIntegerFunc6", testCaseBaseIntegerFunc6)
    suite.addTest(
        "check testCaseBaseIntegerFunc6_1", testCaseBaseIntegerFunc6_1)
    suite.addTest(
        "check testCaseBaseIntegerFunc6_2", testCaseBaseIntegerFunc6_2)
    suite.addTest("check testCaseBaseIntegerFunc7", testCaseBaseIntegerFunc7)
    suite.addTest(
        "check testCaseBaseIntegerFunc7_1", testCaseBaseIntegerFunc7_1)
    suite.addTest("check testCaseBaseIntegerFunc8", testCaseBaseIntegerFunc8)
    suite.addTest(
        "check testCaseBaseIntegerFunc8_1", testCaseBaseIntegerFunc8_1)
    suite.addTest(
        "check testCaseBaseIntegerFunc8_2", testCaseBaseIntegerFunc8_2)
    suite.addTest("check testCaseBaseIntegerFunc9", testCaseBaseIntegerFunc9)
    suite.addTest(
        "check testCaseBaseIntegerFunc9_1", testCaseBaseIntegerFunc9_1)
    suite.addTest("check testCaseBaseIntegerFunc10", testCaseBaseIntegerFunc10)
    suite.addTest(
        "check testCaseBaseIntegerFunc10_1", testCaseBaseIntegerFunc10_1)
    suite.addTest("check testCaseBaseIntegerFunc11", testCaseBaseIntegerFunc11)
    suite.addTest(
        "check testCaseBaseIntegerFunc11_1", testCaseBaseIntegerFunc11_1)
    suite.addTest("check testCaseBaseIntegerFunc12", testCaseBaseIntegerFunc12)
    suite.addTest(
        "check testCaseBaseIntegerFunc12_1", testCaseBaseIntegerFunc12_1)
    suite.addTest("check testCaseBaseIntegerFunc13", testCaseBaseIntegerFunc13)
    suite.addTest(
        "check testCaseBaseIntegerFunc13_1", testCaseBaseIntegerFunc13_1)
    suite.addTest("check testCaseBaseIntegerFunc14", testCaseBaseIntegerFunc14)
    suite.addTest(
        "check testCaseBaseIntegerFunc14_1", testCaseBaseIntegerFunc14_1)
    suite.addTest("check testCaseBaseIntegerFunc15", testCaseBaseIntegerFunc15)
    suite.addTest(
        "check testCaseBaseIntegerFunc15_1", testCaseBaseIntegerFunc15_1)
    suite.addTest("check testCaseBaseIntegerFunc16", testCaseBaseIntegerFunc16)
    suite.addTest(
        "check testCaseBaseIntegerFunc16_1", testCaseBaseIntegerFunc16_1)
    suite.addTest("check testCaseBaseIntegerFunc17", testCaseBaseIntegerFunc17)
    suite.addTest(
        "check testCaseBaseIntegerFunc17_1", testCaseBaseIntegerFunc17_1)
    suite.addTest("check testCaseBaseIntegerFunc18", testCaseBaseIntegerFunc18)
    suite.addTest(
        "check testCaseBaseIntegerFunc18_1", testCaseBaseIntegerFunc18_1)
    suite.addTest("check testCaseBaseIntegerFunc19", testCaseBaseIntegerFunc19)
    suite.addTest(
        "check testCaseBaseIntegerFunc19_1", testCaseBaseIntegerFunc19_1)
    suite.addTest("check testCaseBaseIntegerFunc20", testCaseBaseIntegerFunc20)
    suite.addTest(
        "check testCaseBaseIntegerFunc20_1", testCaseBaseIntegerFunc20_1)
    suite.addTest("check testCaseBaseIntegerFunc21", testCaseBaseIntegerFunc21)
    suite.addTest(
        "check testCaseBaseIntegerFunc21_1", testCaseBaseIntegerFunc21_1)
    suite.addTest("check testCaseBaseIntegerFunc22", testCaseBaseIntegerFunc22)
    suite.addTest(
        "check testCaseBaseIntegerFunc22_1", testCaseBaseIntegerFunc22_1)
    suite.addTest("check testCaseBaseIntegerFunc23", testCaseBaseIntegerFunc23)
    suite.addTest(
        "check testCaseBaseIntegerFunc23_1", testCaseBaseIntegerFunc23_1)
    suite.addTest("check testCaseBaseIntegerFunc24", testCaseBaseIntegerFunc24)
    suite.addTest(
        "check testCaseBaseIntegerFunc24_1", testCaseBaseIntegerFunc24_1)
    suite.addTest("check testCaseBaseIntegerFunc25", testCaseBaseIntegerFunc25)
    suite.addTest(
        "check testCaseBaseIntegerFunc25_1", testCaseBaseIntegerFunc25_1)
    suite.addTest("check testCaseBaseIntegerFunc26", testCaseBaseIntegerFunc26)
    suite.addTest(
        "check testCaseBaseIntegerFunc26_1", testCaseBaseIntegerFunc26_1)
    suite.addTest("check testCaseBaseIntegerFunc27", testCaseBaseIntegerFunc27)
    suite.addTest("check testCaseBaseIntegerFunc28", testCaseBaseIntegerFunc28)
    suite.addTest(
        "check testCaseBaseIntegerFunc28_1", testCaseBaseIntegerFunc28_1)
    suite.addTest("check testCaseBaseIntegerFunc29", testCaseBaseIntegerFunc29)
    suite.addTest(
        "check testCaseBaseIntegerFunc29_1", testCaseBaseIntegerFunc29_1)
    suite.addTest("check testCaseBaseIntegerFunc30", testCaseBaseIntegerFunc30)
    suite.addTest(
        "check testCaseBaseIntegerFunc30_1", testCaseBaseIntegerFunc30_1)
    suite.addTest("check testCaseBaseIntegerFunc31", testCaseBaseIntegerFunc31)
    suite.addTest(
        "check testCaseBaseIntegerFunc31_1", testCaseBaseIntegerFunc31_1)
    suite.addTest("check testCaseBaseIntegerFunc32", testCaseBaseIntegerFunc32)
    suite.addTest(
        "check testCaseBaseIntegerFunc32_1", testCaseBaseIntegerFunc32_1)
    suite.addTest("check testCaseBaseIntegerFunc33", testCaseBaseIntegerFunc33)
    suite.addTest(
        "check testCaseBaseIntegerFunc33_1", testCaseBaseIntegerFunc33_1)
    suite.addTest("check testCaseBaseIntegerFunc34", testCaseBaseIntegerFunc34)
    suite.addTest(
        "check testCaseBaseIntegerFunc34_1", testCaseBaseIntegerFunc34_1)
    suite.addTest("check testCaseBaseIntegerFunc35", testCaseBaseIntegerFunc35)
    suite.addTest(
        "check testCaseBaseIntegerFunc35_1", testCaseBaseIntegerFunc35_1)
    suite.addTest("check testCasetestBaseBoolFunc1", testCasetestBaseBoolFunc1)
    suite.addTest(
        "check testCasetestBaseBoolFunc1_1", testCasetestBaseBoolFunc1_1)
    suite.addTest("check testCasetestBaseBoolFunc2", testCasetestBaseBoolFunc2)
    suite.addTest(
        "check testCasetestBaseBoolFunc2_1", testCasetestBaseBoolFunc2_1)
    suite.addTest("check testCasetestBaseBoolFunc3", testCasetestBaseBoolFunc3)
    suite.addTest(
        "check testCasetestBaseBoolFunc3_1", testCasetestBaseBoolFunc3_1)
    suite.addTest("check testCasetestBaseBoolFunc4", testCasetestBaseBoolFunc4)
    suite.addTest(
        "check testCasetestBaseBoolFunc4_1", testCasetestBaseBoolFunc4_1)
    suite.addTest(
        "check testCasetestBaseBoolFunc4_2", testCasetestBaseBoolFunc4_2)
    suite.addTest("check testCasetestBaseBoolFunc5", testCasetestBaseBoolFunc5)
    suite.addTest(
        "check testCasetestBaseBoolFunc5_1", testCasetestBaseBoolFunc5_1)
    suite.addTest(
        "check testCasetestBaseBoolFunc5_2", testCasetestBaseBoolFunc5_2)
    suite.addTest(
        "check testCasetestBaseBoolFunc5_3", testCasetestBaseBoolFunc5_3)

    // namspacevoid test
    suite.addTest("check testNsMyFunc1", testNsMyFunc1)
    suite.addTest("check testNsMyFunc2", testNsMyFunc2)
    suite.addTest("check testNsMyFunc3", testNsMyFunc3)
    suite.addTest("check testNsMyFunc3_1", testNsMyFunc3_1)
    suite.addTest("check testNsMyFunc4", testNsMyFunc4)
    suite.addTest("check testNsMyFunc5", testNsMyFunc5)
    suite.addTest("check testNsMyFunc6", testNsMyFunc6)
    suite.addTest("check testNsMyFunc6_1", testNsMyFunc6_1)
    suite.addTest("check testNsMyFunc7", testNsMyFunc7)
    suite.addTest("check testNsMyFunc8", testNsMyFunc8)
    suite.addTest("check testNsMyFunc9", testNsMyFunc9)
    suite.addTest("check testNsMyFunc10", testNsMyFunc10)
    suite.addTest("check testNsMyFunc11", testNsMyFunc11)
    suite.addTest("check testNsMyFunc12", testNsMyFunc12)
    suite.addTest("check testNsMyFunc12_1", testNsMyFunc12_1)
    suite.addTest("check testNsMyFunc12_2", testNsMyFunc12_2)
    suite.addTest("check testNsMyFunc13", testNsMyFunc13)
    suite.addTest("check testNsMyFunc13_1", testNsMyFunc13_1)
    suite.addTest("check testNsMyFunc13_2", testNsMyFunc13_2)
    suite.addTest("check testNsMyFunc14", testNsMyFunc14)
    suite.addTest("check testNsMyFunc14_1", testNsMyFunc14_1)
    suite.addTest("check testNsMyFunc14_2", testNsMyFunc14_2)
    suite.addTest("check testNsMyFunc15", testNsMyFunc15)
    suite.addTest("check testNsMyFunc15_1", testNsMyFunc15_1)
    suite.addTest("check testNsMyFunc15_2", testNsMyFunc15_2)
    suite.addTest("check testNsMyFunc16", testNsMyFunc16)
    suite.addTest("check testNsMyFunc16_1", testNsMyFunc16_1)
    suite.addTest("check testNsMyFunc16_2", testNsMyFunc16_2)
    suite.run()
}