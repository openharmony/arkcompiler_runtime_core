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
import * as namespace2Test from "../author_generated/my_ns";
import * as namespaceTest from "../author_generated/ns_alltest";

loadLibrary("ani_namespace");

let nameInterfaceTest: namespaceTest.functiontest.testNameSpace =
    namespaceTest.functiontest.get_interface_NameSpace();
let testInterfacePerformance1:
    namespaceTest.functiontest.testInterfacePerformance1 =
    namespaceTest.functiontest.get_interface_performance1();
let testInterfacePerformance2:
    namespaceTest.functiontest.testInterfacePerformance2 =
    namespaceTest.functiontest.get_interface_performance2();
let testInterfacePerformance3:
    namespaceTest.functiontest.testInterfacePerformance3 =
    namespaceTest.functiontest.get_interface_performance3();
let testInterfacePerformance4:
    namespaceTest.functiontest.testInterfacePerformance4 =
    namespaceTest.functiontest.get_interface_performance4();
let testInterfacePerformance5:
    namespaceTest.functiontest.testInterfacePerformance5 =
    namespaceTest.functiontest.get_interface_performance5();
let testInterfacePerformance6:
    namespaceTest.functiontest.testInterfacePerformance6 =
    namespaceTest.functiontest.get_interface_performance6();
let testInterfacePerformance7:
    namespaceTest.functiontest.testInterfacePerformance7 =
    namespaceTest.functiontest.get_interface_performance7();
let testInterfacePerformance8:
    namespaceTest.functiontest.testInterfacePerformance8 =
    namespaceTest.functiontest.get_interface_performance8();
let testInterfacePerformance9:
    namespaceTest.functiontest.testInterfacePerformance9 =
    namespaceTest.functiontest.get_interface_performance9();
let testInterfacePerformance10:
    namespaceTest.functiontest.testInterfacePerformance10 =
    namespaceTest.functiontest.get_interface_performance10();
let testInterfacePerformance11:
    namespaceTest.functiontest.testInterfacePerformance11 =
    namespaceTest.functiontest.get_interface_performance11();
let testInterfacePerformance12:
    namespaceTest.functiontest.testInterfacePerformance12 =
    namespaceTest.functiontest.get_interface_performance12();
let testInterfacePerformance13:
    namespaceTest.functiontest.testInterfacePerformance13 =
    namespaceTest.functiontest.get_interface_performance13();
let testInterfacePerformance14:
    namespaceTest.functiontest.testInterfacePerformance14 =
    namespaceTest.functiontest.get_interface_performance14();
let testInterfacePerformance15:
    namespaceTest.functiontest.testInterfacePerformance15 =
    namespaceTest.functiontest.get_interface_performance15();
let testInterfacePerformance16:
    namespaceTest.functiontest.testInterfacePerformance16 =
    namespaceTest.functiontest.get_interface_performance16();
let testInterfacePerformance17:
    namespaceTest.functiontest.testInterfacePerformance17 =
    namespaceTest.functiontest.get_interface_performance17();
let testInterfacePerformance18:
    namespaceTest.functiontest.testInterfacePerformance18 =
    namespaceTest.functiontest.get_interface_performance18();
let testInterfacePerformance19:
    namespaceTest.functiontest.testInterfacePerformance19 =
    namespaceTest.functiontest.get_interface_performance19();
let testInterfacePerformance20:
    namespaceTest.functiontest.testInterfacePerformance20 =
    namespaceTest.functiontest.get_interface_performance20();
function testCasebaseFunctionTest1() {
    namespaceTest.functiontest.baseFunctionTest1()
}

function testCasebaseFunctionTest2() {
    namespaceTest.functiontest.baseFunctionTest2(127 as byte)
}

function testCasebaseFunctionTest3() {
    namespaceTest.functiontest.baseFunctionTest3(255 as short)
}

function testCasebaseFunctionTest4() {
    namespaceTest.functiontest.baseFunctionTest4(2147483647)
}

function testCasebaseFunctionTest5() {
    namespaceTest.functiontest.baseFunctionTest5(9223372036854775807)
}

function testCasebaseFunctionTest6() {
    namespaceTest.functiontest.baseFunctionTest6(3.1415 as float)
}

function testCasebaseFunctionTest7() {
    namespaceTest.functiontest.baseFunctionTest7(3.1415125)
}

function testCasebaseFunctionTest8() {
    namespaceTest.functiontest.baseFunctionTest8("baseFunctionTest8!@#$%%^&*()")
}

function testCasebaseFunctionTest9() {
    namespaceTest.functiontest.baseFunctionTest9(false)
}

function testCasebaseFunctionTest10() {
    let byteArray: byte[] = [1, 127, -120, 0, 100];
    namespaceTest.functiontest.baseFunctionTest10(byteArray)
}

function testCasebaseFunctionTest11() {
    let shortArray: short[] = [32767, -32768, 122, 50, 10086];
    namespaceTest.functiontest.baseFunctionTest11(shortArray)
}

function testCasebaseFunctionTest12() {
    namespaceTest.functiontest.baseFunctionTest12(255 as short)
}

function testCasebaseFunctionTest12_1() {
    namespaceTest.functiontest.baseFunctionTest12(undefined)
}

function testCasebaseFunctionTest13() {
    namespaceTest.functiontest.baseFunctionTest13(9223372036854775807)
}

function testCasebaseFunctionTest13_1() {
    namespaceTest.functiontest.baseFunctionTest13(undefined)
}

function testCasebaseFunctionTest14() {
    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i]);
    }
    namespaceTest.functiontest.baseFunctionTest14(arrbuf1)
}

function testCasebaseFunctionTest15() {
    let record: Record<string, int> = {
        "test1": 0,
        "test2": 1,
        "test3": 2,
        "test4": 3,
    };
    namespaceTest.functiontest.baseFunctionTest15(record)
}

function testCasebaseFunctionTest16() {
    let res = namespaceTest.functiontest.baseFunctionTest16(110 as byte);
    assertEQ(res, 120)
}

function testCasebaseFunctionTest16_1() {
    try {
        let res = namespaceTest.functiontest.baseFunctionTest16(127 as byte);
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: baseFunctionTest16: result exceeds int8_t range")
    }
}

function testCasebaseFunctionTest17() {
    let res = namespaceTest.functiontest.baseFunctionTest17(3270 as short);
    assertEQ(res, 32700)
}

function testCasebaseFunctionTest17_1() {
    try {
        let res = namespaceTest.functiontest.baseFunctionTest17(32767 as short);
    } catch (e) {
        assertEQ(
            e.toString(),
            "Error: baseFunctionTest17: result exceeds int16_t range")
    }
}

function testCasebaseFunctionTest18() {
    let res = namespaceTest.functiontest.baseFunctionTest18(21474836);
    assertEQ(res, 2147483600)
}

function testCasebaseFunctionTest18_1() {
    try {
        let res = namespaceTest.functiontest.baseFunctionTest18(21474837);
    } catch (e) {
        console.log("testCasebaseFunctionTest18_1 error ", e.toString())
        assertEQ(
            e.toString(),
            "Error: baseFunctionTest18: result exceeds int32_t range")
    }
}

function testCasebaseFunctionTest19() {
    let res = namespaceTest.functiontest.baseFunctionTest19(92233720368547758);
    assertEQ(res, 922337203685477580)
}

function testCasebaseFunctionTest20() {
    let res = namespaceTest.functiontest.baseFunctionTest20(3.14 as float);
    assertEQ(res, 103.14 as float)
}

function testCasebaseFunctionTest21() {
    let res = namespaceTest.functiontest.baseFunctionTest21(1);
    assertEQ(res, 2.01)
}

function testCasebaseFunctionTest22() {
    let res =
        namespaceTest.functiontest.baseFunctionTest22("baseFunctionTest22");
    assertEQ(res, "baseFunctionTest22hello ani")
}

function testCasebaseFunctionTest22_1() {
    let res =
        namespaceTest.functiontest.baseFunctionTest22("baseFunctionTest22——1");
    assertEQ(res, "baseFunctionTest22——1")
}

function testCasebaseFunctionTest23() {
    let res = namespaceTest.functiontest.baseFunctionTest23(true);
    assertEQ(res, false)
}

function testCasebaseFunctionTest23_1() {
    let res = namespaceTest.functiontest.baseFunctionTest23(false);
    assertEQ(res, true)
}

function testCasebaseFunctionTest24() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let res = namespaceTest.functiontest.baseFunctionTest24(byteArray);
    let temp: byte[] = [2, 4, 6, 8, 10];
    for (let i = 0; i < res.length; i++) {
        assertEQ(res[i], temp[i])
    }
}

function testCasebaseFunctionTest25() {
    let shortArray: short[] = [1, 2, 3, 4, 5];
    let res = namespaceTest.functiontest.baseFunctionTest25(shortArray);
    let temp: byte[] = [3, 4, 5, 6, 7];
    for (let i = 0; i < res.length; i++) {
        assertEQ(res[i], temp[i])
    }
}

function testCasebaseFunctionTest26() {
    let res = namespaceTest.functiontest.baseFunctionTest26(3276 as short);
    assertEQ(res, 3286)
}

function testCasebaseFunctionTest26_1() {
    let res = namespaceTest.functiontest.baseFunctionTest26(undefined);
    assertEQ(res, undefined)
}

function testCasebaseFunctionTest27() {
    let res = namespaceTest.functiontest.baseFunctionTest27(922337203685477580);
    assertEQ(res, 922337203685477590)
}

function testCasebaseFunctionTest27_1() {
    let res = namespaceTest.functiontest.baseFunctionTest27(undefined);
    assertEQ(res, undefined)
}

function testCasebaseFunctionTest28() {
    let numbersU8: byte[] = [11, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i]);
    }
    let res = namespaceTest.functiontest.baseFunctionTest28(arrbuf1);
    console.log("testCasebaseFunctionTest28 length: " + res.getByteLength());
    console.log("testCasebaseFunctionTest28 : " + res);
    assertEQ(res.getByteLength(), 5)
    for (let i = 0; i < res.getByteLength(); i++) {
        console.log(
            `testCasebaseFunctionTest28 Byte at index ${i}: ${res.at(i)}`);
        assertEQ(res.at(i), numbersU8[i] * 10)
    }
}

function testCasebaseFunctionTest29() {
    const myRecord: Record<string, int> = {
        "key1": 10,
        "key2": 20,
        "key3": 30,
    };

    const resultRecord: Record<string, int> = {
        "test0": 10,
        "test1": 11,
        "test2": 12,
    };
    let res = namespaceTest.functiontest.baseFunctionTest29(myRecord);
    console.log("testCasebaseFunctionTest29 ", res);
    console.log("testCasebaseFunctionTest29 test0 ", res["test0"]);
    assertEQ(res["test0"], resultRecord["test0"]);
    assertEQ(res["test1"], resultRecord["test1"]);
    assertEQ(res["test2"], resultRecord["test2"]);
}

function testCasebaseFunctionTest30() {
    let color = namespaceTest.functiontest.Color.BLUE;
    let res = namespaceTest.functiontest.baseFunctionTest30(color);
    console.log("testCasebaseFunctionTest30 ", res)
    assertEQ(res, namespaceTest.functiontest.Color.RED)
}

function testCasebaseFunctionTest31() {
    let color = namespaceTest.functiontest.Color.RED;
    namespaceTest.functiontest.baseFunctionTest31(color);
}

function testCasebaseFunctionTest32() {
    let int16Array: short[] = [121, 100, 32767, -32768, 0];
    let int64Array: long[] =
        [9223372036854775807, -100, 121, -2147483648, -9223372036854775808];
    let res = namespaceTest.functiontest.baseFunctionTest32(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        true, int16Array, int64Array);
    assertEQ(res, 100)
}

function testCasebaseFunctionTest32_1() {
    let int16Array: short[] = [121, 100, 32767, -32768, 0];
    let int64Array: long[] =
        [9223372036854775807, -100, 121, -2147483648, -9223372036854775808];
    let res = namespaceTest.functiontest.baseFunctionTest32(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        false, int16Array, int64Array);
    assertEQ(res, 127)
}

function testCasebaseFunctionTest33() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let intArray: int[] = [922337, -100, 121, -2147483648, -92233720];
    let res = namespaceTest.functiontest.baseFunctionTest33(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        true, byteArray, intArray);
    assertEQ(res, 256)
}

function testCasebaseFunctionTest33_1() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let intArray: int[] = [922337, -100, 121, -2147483648, -92233720];
    let res = namespaceTest.functiontest.baseFunctionTest33(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        false, byteArray, intArray);
    assertEQ(res, 100)
}

function testCasebaseFunctionTest34() {
    let strArray: string[] = ["test1", "test2", "test3", "test4", "test5"];
    let int64Array: long[] =
        [9223372036854775807, -100, 121, -2147483648, -9223372036854775808];
    let res = namespaceTest.functiontest.baseFunctionTest34(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        true, strArray, int64Array);
    assertEQ(res, 10000)
}

function testCasebaseFunctionTest34_1() {
    let strArray: string[] = ["test1", "test2", "test3", "test4", "test5"];
    let int64Array: long[] =
        [9223372036854775807, -100, 121, -2147483648, -9223372036854775808];
    let res = namespaceTest.functiontest.baseFunctionTest34(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        false, strArray, int64Array);
    assertEQ(res, 100)
}

function testCasebaseFunctionTest35() {
    let boolArray: boolean[] = [true, false, true, false, true];
    let int64Array: long[] =
        [9223372036854775807, -100, 121, -2147483648, -9223372036854775808];
    let res = namespaceTest.functiontest.baseFunctionTest35(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        true, boolArray, int64Array);
    assertEQ(res, 127)
}

function testCasebaseFunctionTest35_1() {
    let boolArray: boolean[] = [true, false, true, false, true];
    let int64Array: long[] =
        [9223372036854775807, -100, 121, -2147483648, -9223372036854775808];
    let res = namespaceTest.functiontest.baseFunctionTest35(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        false, boolArray, int64Array);
    assertEQ(res, 100)
}

function testCasebaseFunctionTest36() {
    let int8Array: byte[] = [1, 2, 3, 4, 5];
    let int64Array: long[] =
        [9223372036854775807, -100, 121, -2147483648, -9223372036854775808];
    let res = namespaceTest.functiontest.baseFunctionTest36(
        10 as byte, 11 as short, 12, 10000, 3.14 as float, -2.0002, "test1",
        true, int8Array, int64Array);
    assertEQ(res.length, int8Array.length)
    let tempResult: int[] = [34, 35, 36, 37, 38];
    for (let i: int = 0; i < res.length; i++) {
        assertEQ(res[i], tempResult[i])
    }
}

function testCasebaseFunctionTest36_1() {
    let int8Array: byte[] = [1, 2, 3, 4, 5];
    let int64Array: long[] = [100, -100, 200, -300, -400];
    let res = namespaceTest.functiontest.baseFunctionTest36(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        false, int8Array, int64Array);
    assertEQ(res.length, int64Array.length)
    for (let i: int = 0; i < res.length; i++) {
        assertEQ(res[i], int64Array[i])
    }
}

function testCasebaseFunctionTest37() {
    let intArray: int[] = [922337, -100, 121, -2147483648, -92233720];
    let int64Array: long[] = [100, -100, 121, -200, 0];
    let res = namespaceTest.functiontest.baseFunctionTest37(
        10 as byte, 11 as short, 12, 10000, 3.14 as float, -2.0002, "test1",
        true, intArray, int64Array);
    assertEQ(res.length, int64Array.length)
    let tempResult: int[] = [133, -67, 154, -167, 33];
    for (let i: int = 0; i < res.length; i++) {
        assertEQ(res[i], tempResult[i])
    }
}

function testCasebaseFunctionTest37_1() {
    let intArray: int[] = [922337, -100, 121, -2147483648, -92233720];
    let int64Array: long[] = [100, -100, 200, -300, -400];
    let res = namespaceTest.functiontest.baseFunctionTest37(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        false, intArray, int64Array);
    assertEQ(res.length, int64Array.length)
    for (let i: int = 0; i < res.length; i++) {
        assertEQ(res[i], int64Array[i] * 10)
    }
}

function testCasebaseFunctionTest38() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let int16Array: short[] = [100, -100, 121, -200, -500];
    let res = namespaceTest.functiontest.baseFunctionTest38(
        10 as byte, 11 as short, 12, 10000, 3.14 as float, -2.0002, "test1",
        true, byteArray, int16Array);
    assertEQ(res, "test110111210000")
}

function testCasebaseFunctionTest38_1() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let int16Array: short[] = [100, -100, 121, -200, -500];
    let res = namespaceTest.functiontest.baseFunctionTest38(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        false, byteArray, int16Array);
    assertEQ(res, "test11100")
}

function testCasebaseFunctionTest39() {
    let boolArray: boolean[] = [true, false, true, false, true];
    let intArray: int[] = [922337, -100, 121, -2147483648, -92233720];
    let res = namespaceTest.functiontest.baseFunctionTest39(
        10 as byte, 11 as short, 12, 10000, 3.14 as float, -2.0002, "test1",
        true, boolArray, intArray);
    assertEQ(res, false)
}

function testCasebaseFunctionTest39_1() {
    let boolArray: boolean[] = [true, false, true, false, true];
    let intArray: int[] = [922337, -100, 121, -2147483648, -92233720];
    let res = namespaceTest.functiontest.baseFunctionTest39(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        false, boolArray, intArray);
    assertEQ(res, false)
}

function testCasebaseFunctionTest40() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let numbersU8: byte[] = [10, 20, 30, 40, 50];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i])
    }
    let res = namespaceTest.functiontest.baseFunctionTest40(
        10 as byte, 11 as short, 12, 10000, 3.14 as float, -2.0002, "test1",
        true, byteArray, arrbuf1);
    assertEQ(res.length, byteArray.length)
    for (let i: int = 0; i < res.length; i++) {
        assertEQ(res[i], byteArray[i] * 2)
    }
}

function testCasebaseFunctionTest40_1() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let numbersU8: byte[] = [10, 20, 30, 40, 50];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i])
    }
    let res = namespaceTest.functiontest.baseFunctionTest40(
        127 as byte, 100 as short, 256, 10000, 3.14 as float, -2.0002, "test1",
        false, byteArray, arrbuf1);
    assertEQ(res.length, byteArray.length)
    for (let i: int = 0; i < res.length; i++) {
        assertEQ(res[i], byteArray[i] + 10)
    }
}

function testCasebaseFunctionTest41() {
    let int16Array: short[] = [121, 100, 32767, -32768, 0];
    let int64Array: long[] =
        [100, -100, 9223372036854775807, -300, -9223372036854775808];
    namespaceTest.functiontest.baseFunctionTest41(
        10 as byte, 11 as short, 12, 10000, 3.14 as float, -2.0002, "test41",
        true, int16Array, int64Array);
}
function testCasebaseFunctionTest42() {
    let res =
        namespaceTest.functiontest.baseFunctionTest42(10000, 256 as short);
    assertEQ(res, 10256)
}

function testCasebaseFunctionTest42_1() {
    try {
        let res = namespaceTest.functiontest.baseFunctionTest42(
            2147483647, 256 as short);
    } catch (e) {
        console.log("testCasebaseFunctionTest42_1 e ", e.toString())
        assertEQ(
            e.toString(),
            "Error: baseFunctionTest42_int: result exceeds int32_t range")
    }
}

function testCasebaseFunctionTest43() {
    let record: Record<string, int> = {
        "test1": 0,
        "test2": 1,
        "test3": 2,
    };
    let res = namespaceTest.functiontest.baseFunctionTest42(10000, record);
    assertEQ(res, 10010)
}

function testCasebaseFunctionTest43_1() {
    let record: Record<string, int> = {
        "test1": 0,
        "test2": 1,
        "test3": 2,
    };
    let res = namespaceTest.functiontest.baseFunctionTest42(undefined, record);
    assertEQ(res, -2147483648)
}

function testCasebaseFunctionTest44() {
    let res = namespaceTest.functiontest.baseFunctionTest42();
    assertEQ(res, 2147483647)
}

function testCasebaseFunctionTest45() {
    const testData: namespaceTest.functiontest
        .Data = {data1: "baseFunctionTest42", data2: true, data3: 2147483647};
    namespaceTest.functiontest.baseFunctionTest42(testData);
}


function testCasebaseFunctionTest46() {
    nameInterfaceTest.baseFunctionTest43();
}

function testCasebaseFunctionTest47() {
    let res = nameInterfaceTest.baseFunctionTest44(-10 as byte);
    assertEQ(res, 90)
}

function testCasebaseFunctionTest48() {
    let res = nameInterfaceTest.baseFunctionTest45(320 as short);
    assertEQ(res, 32000)
}

function testCasebaseFunctionTest49() {
    let res = nameInterfaceTest.baseFunctionTest46(2147483);
    assertEQ(res, 214748300)
}

function testCasebaseFunctionTest50() {
    let res = nameInterfaceTest.baseFunctionTest47(9223372036854775);
    assertEQ(res, 922337203685477500)
}

function testCasebaseFunctionTest51() {
    let res = nameInterfaceTest.baseFunctionTest48(100.123 as float);
    assertEQ(res, 90.123 as float)
}

function testCasebaseFunctionTest52() {
    let res = nameInterfaceTest.baseFunctionTest49(200.66);
    assertEQ(res, 100.66)
}

function testCasebaseFunctionTest53() {
    let res =
        nameInterfaceTest.baseFunctionTest50("testCasebaseFunctionTest53");
    assertEQ(res, "testCasebaseFunctionTest53baseFunctionTest50")
}

function testCasebaseFunctionTest54() {
    let res = nameInterfaceTest.baseFunctionTest51(true);
    assertEQ(res, false)
}

function testCasebaseFunctionTest54_1() {
    let res = nameInterfaceTest.baseFunctionTest51(false);
    assertEQ(res, true)
}

function testCasebaseFunctionTest55() {
    let numbersU8: byte[] = [100, 101, 102, 103, 104];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i]);
    }
    let res = nameInterfaceTest.baseFunctionTest52(arrbuf1);
    assertEQ(res.getByteLength(), 5)
    for (let i = 0; i < res.getByteLength(); i++) {
        console.log(
            `testCasebaseFunctionTest55 Byte at index ${i}: ${res.at(i)}`);
        assertEQ(res.at(i), numbersU8[i] + 10)
    }
}

function testCasebaseFunctionTest56() {
    let res = nameInterfaceTest.baseFunctionTest53(101 as byte);
    assertEQ(res, 1)
}

function testCasebaseFunctionTest56_1() {
    let res = nameInterfaceTest.baseFunctionTest53(undefined);
    assertEQ(res, undefined)
}

function testCasebaseFunctionTest57() {
    let res = nameInterfaceTest.baseFunctionTest54(2147483647);
    assertEQ(res, 2147483547)
}

function testCasebaseFunctionTest57_1() {
    let res = nameInterfaceTest.baseFunctionTest54(undefined);
    assertEQ(res, undefined)
}

// function testCasebaseFunctionTest58(){
//     const testData1 = new namespaceTest.functiontest.Data(
//         "testCasebaseFunctionTest58",
//         true,
//         100
//     );
//     const testData2= new namespaceTest.functiontest.Data(
//         "testCasebaseFunctionTest58_1",
//         false,
//         200
//     );
//     const testData3= new namespaceTest.functiontest.Data(
//         "testCasebaseFunctionTest58_2",
//         false,
//         300
//     );
//     const myRecord: Record<short, namespaceTest.functiontest.Data> = {
//     100: testData1,
//     101: testData2,
//     102: testData3,
//     };
//     let res = nameInterfaceTest.baseFunctionTest55(myRecord);
//     console.log("testCasebaseFunctionTest58 ",res)
//     assertEQ(res,2147483547)
// }

function testCasebaseFunctionTest59() {
    let res = nameInterfaceTest.baseFunctionTest56(100);
    assertEQ(res, 10000)
}

function testCasebaseFunctionTest60() {
    let res = nameInterfaceTest.baseFunctionTest57(127 as byte);
    assertEQ(res, 12700)
}

function testCasebaseFunctionTest61() {
    let res = nameInterfaceTest.testStr;
    assertEQ(res, "testNameSpace")
}

function testCasebaseFunctionTest62() {
    let res = nameInterfaceTest.baseFunctionTest58(true);
    assertEQ(res.success, true)
    assertEQ(res.errorCode, 10000)
    assertEQ(res.errorMessage, "test58")
}

function testCasebaseFunctionTest62_1() {
    let res = nameInterfaceTest.baseFunctionTest58(false);
    assertEQ(res.success, false)
    assertEQ(res.errorCode, 100)
    assertEQ(res.errorMessage, "test581")
}


function testInter1basePerformanceFunctionTest1() {
    testInterfacePerformance1.basePerformanceFunctionTest1();
}

function testInter1basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance1.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}

function testInter2basePerformanceFunctionTest1() {
    testInterfacePerformance2.basePerformanceFunctionTest1();
}

function testInter2basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance2.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}

function testInter3basePerformanceFunctionTest1() {
    testInterfacePerformance3.basePerformanceFunctionTest1();
}

function testInter3basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance3.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}

function testInter4basePerformanceFunctionTest1() {
    testInterfacePerformance4.basePerformanceFunctionTest1();
}

function testInter4basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance4.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}
function testInter5basePerformanceFunctionTest1() {
    testInterfacePerformance5.basePerformanceFunctionTest1();
}

function testInter5basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance5.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}

function testInter6basePerformanceFunctionTest1() {
    testInterfacePerformance6.basePerformanceFunctionTest1();
}

function testInter6basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance6.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}
function testInter7basePerformanceFunctionTest1() {
    testInterfacePerformance7.basePerformanceFunctionTest1();
}

function testInter7basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance7.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}

function testInter8basePerformanceFunctionTest1() {
    testInterfacePerformance8.basePerformanceFunctionTest1();
}

function testInter8basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance8.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}
function testInter9basePerformanceFunctionTest1() {
    testInterfacePerformance9.basePerformanceFunctionTest1();
}

function testInter9basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance9.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}
function testInter10basePerformanceFunctionTest1() {
    testInterfacePerformance10.basePerformanceFunctionTest1();
}

function testInter10basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance10.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}
function testInter11basePerformanceFunctionTest1() {
    testInterfacePerformance11.basePerformanceFunctionTest1();
}

function testInter11basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance11.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}
function testInter12basePerformanceFunctionTest1() {
    testInterfacePerformance12.basePerformanceFunctionTest1();
}

function testInter12basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance12.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}
function testInter13basePerformanceFunctionTest1() {
    testInterfacePerformance13.basePerformanceFunctionTest1();
}

function testInter13basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance13.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}

function testInter14basePerformanceFunctionTest1() {
    testInterfacePerformance14.basePerformanceFunctionTest1();
}

function testInter14basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance14.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}

function testInter15basePerformanceFunctionTest1() {
    testInterfacePerformance15.basePerformanceFunctionTest1();
}

function testInter15basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance15.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}

function testInter16basePerformanceFunctionTest1() {
    testInterfacePerformance16.basePerformanceFunctionTest1();
}

function testInter16basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance16.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}
function testInter17basePerformanceFunctionTest1() {
    testInterfacePerformance17.basePerformanceFunctionTest1();
}

function testInter17basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance17.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}
function testInter18basePerformanceFunctionTest1() {
    testInterfacePerformance18.basePerformanceFunctionTest1();
}

function testInter18basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance18.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}

function testInter19basePerformanceFunctionTest1() {
    testInterfacePerformance19.basePerformanceFunctionTest1();
}

function testInter19basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance19.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}

function testInter20basePerformanceFunctionTest1() {
    testInterfacePerformance20.basePerformanceFunctionTest1();
}

function testInter20basePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance20.basePerformanceFunctionTest2(95 as byte);
    assertEQ(res, 10)
}

// namespace2
function primitives_noreturn() {
    namespace2Test.outer.inner.primitives_noreturn(
        1 as int, 2 as double, true, "hello", 3 as byte)
}
function primitives_return() {
    let res = namespace2Test.outer.inner.primitives_return(
        1 as int, 2 as double, true, "hello", 3 as byte)
    console.log(res);
}

function containers_noreturn() {
    let arri8: byte[] = [1, 2, 3];
    let arri16: short[] = [1, 2, 3, 4];
    let arri32: int[] = [1, 2, 3, 4, 5];
    let arrf32: float[] = [1.1, 1.2, 1.3];
    let arrf64: double[] = [1.123, 2.123, 3.123];

    namespace2Test.outer.inner.containers_noreturn(
        arri8, arri16, arrf32, arrf64, "union_string")
    namespace2Test.outer.inner.containers_noreturn(
        arri8, arri16, arrf32, arrf64, 1 as byte)
}
function containers_return() {
    let arri8: byte[] = [1, 2, 3];
    let arri16: short[] = [1, 2, 3, 4];
    let arri32: int[] = [1, 2, 3, 4, 5];
    let arrf32: float[] = [1.1, 1.2, 1.3];
    let arrf64: double[] = [1.123, 2.123, 3.123];

    let res1 = namespace2Test.outer.inner.containers_return(
        arri8, arri16, arrf32, arrf64, "union_string")
    let res2 = namespace2Test.outer.inner.containers_return(
        arri8, arri16, arrf32, arrf64, 1 as byte)
    console.log(res1 + " " + res2)
}
function enum_noreturn() {
    let color1 = namespace2Test.outer.inner.Color.red;
    let color2 = namespace2Test.outer.inner.Color.yellow;
    let color3 = namespace2Test.outer.inner.Color.blue;
    let color4 = namespace2Test.outer.inner.Color.green;
    let color5 = namespace2Test.outer.inner.Color.black;

    namespace2Test.outer.inner.enum_noreturn(
        color1, color2, color3, color4, color5)
}
function enum_return() {
    let color1 = namespace2Test.outer.inner.Color.red;
    let color2 = namespace2Test.outer.inner.Color.yellow;
    let color3 = namespace2Test.outer.inner.Color.blue;
    let color4 = namespace2Test.outer.inner.Color.green;
    let color5 = namespace2Test.outer.inner.Color.black;

    let res = namespace2Test.outer.inner.enum_return(
        color1, color2, color3, color4, color5)
    console.log(res)
}

let instance: namespace2Test.outer.inner.TestInterface =
    namespace2Test.outer.inner.get_interface();

function interface_noparam_noreturn() {
    instance.noparam_noreturn()
}

function interface_primitives_noreturn() {
    instance.primitives_noreturn(1 as byte)
}
function interface_primitives_noreturn1() {
    instance.primitives_noreturn1(12 as short)
}
function interface_primitives_noreturn2() {
    instance.primitives_noreturn2(123 as int)
}
function interface_primitives_noreturn3() {
    instance.primitives_noreturn3(1.1 as float)
}
function interface_primitives_noreturn4() {
    instance.primitives_noreturn4(1.1234 as double)
}
function interface_primitives_noreturn5() {
    instance.primitives_noreturn5(true)
}
function interface_primitives_noreturn6() {
    instance.primitives_noreturn6("hello")
}
function interface_primitives_noreturn7() {
    instance.primitives_noreturn7(1234)
}

function interface_primitives_return() {
    instance.primitives_return(123 as int)
}
function interface_containers_noreturn1() {
    let arri8: byte[] = [1, 2, 3, 4, 5];
    instance.containers_noreturn1(arri8)
}
function interface_containers_noreturn2() {
    instance.containers_noreturn2("union string")
    instance.containers_noreturn2(1 as byte)
}
function interface_containers_noreturn3() {
    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    instance.containers_noreturn3(arrbuf1)
}
function interface_containers_noreturn4() {
    let color1 = namespace2Test.outer.inner.Color.red;
    instance.containers_noreturn4(color1)
}
function interface_containers_noreturn5() {
    let record: Record<string, int> = {
        "test1": 0,
        "test2": 1,
        "test3": 2,
        "test4": 3,
    };
    instance.containers_noreturn5(record)
}



function interface_containers_return() {
    let res = instance.containers_return(123 as byte)
    console.log(res)
}

function interface_ErrorResponse() {
    let _struct: namespace2Test.outer.inner.ErrorResponse =
        instance.func_ErrorResponse()
}

function overload1() {
    instance.overloadFunc(1 as byte, 2 as byte)
}
function overload2() {
    let arri8: byte[] = [1, 2, 3, 4, 5];
    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    let s = instance.overloadFunc(arri8, arrbuf1)
    console.log(s)
}
function overload3() {
    instance.overloadFunc()
}
function overload4() {
    let mystruct: namespace2Test.outer.inner.Mystruct = {
        _string: "hello",
        _i8: 1 as byte
    };
    instance.overloadFunc(mystruct)
}

function ns_5() {
    namespace2Test.test1.test2.test3.test4.test5.funtest1(
        123 as int, 1.234 as double)
}

function test20interface() {
    let instance_1: namespace2Test.outer.inner.test1 =
        namespace2Test.outer.inner.get_interface_1();
    let instance_20: namespace2Test.outer.inner.test20 =
        namespace2Test.outer.inner.get_interface_20();
    instance_1.fun1()
    instance_20.fun2()
}

// attribute
function testattributeString() {
    let name = namespace2Test.outer.inner.printTestInterfaceName(instance)
    console.log("instance.name is " + name)
    assertEQ(name, "String")
}
function testattributei8() {
    let i8 = namespace2Test.outer.inner.printTestInterfaceNumberi8(instance)
    console.log("instance.i8 is " + i8)
    assertEQ(i8, -128)
}
function testattributei16() {
    let i16 = namespace2Test.outer.inner.printTestInterfaceNumberi16(instance)
    console.log("instance.i16 is " + i16)
    assertEQ(i16, -32768)
}
function testattributei32() {
    let i32 = namespace2Test.outer.inner.printTestInterfaceNumberi32(instance)
    console.log("instance.i32 is " + i32)
    assertEQ(i32, -2147483648)
}
function testattributei64() {
    let i64 = namespace2Test.outer.inner.printTestInterfaceNumberi64(instance)
    console.log("instance.i64 is " + i64)
    assertEQ(i64, 1000 as long)
}
function testattributef32() {
    let f32 = namespace2Test.outer.inner.printTestInterfaceNumberf32(instance)
    console.log("instance.f32 is " + f32)
    assertEQ(f32, 3.1415927 as float)
}
function testattributef64() {
    let f64 = namespace2Test.outer.inner.printTestInterfaceNumberf64(instance)
    console.log("instance.f64 is " + f64)
    assertEQ(f64, 3.141592653589793)
}
function testattributebool() {
    let b = namespace2Test.outer.inner.printTestInterfacebool(instance)
    assertEQ(b, true)
}
function testattributeArrayBuffer() {
    namespace2Test.outer.inner.printTestInterfaceArraybuffer(instance)
}
function testattributeArray() {
    namespace2Test.outer.inner.printTestInterfaceArray(instance)
}
function testattributeUnion() {
    namespace2Test.outer.inner.printTestInterfaceUnion(instance)
}
function testattributeRecord() {
    namespace2Test.outer.inner.printTestInterfaceRecord(instance)
}
function testattributeEnum() {
    namespace2Test.outer.inner.printTestInterfaceEnum(instance)
}

function testinheritance() {
    let instancetestA: namespace2Test.outer.inner.testA =
        namespace2Test.outer.inner.get_interface_A()
    instancetestA.fun1()
    let instancetestB: namespace2Test.outer.inner.testB =
        namespace2Test.outer.inner.get_interface_B()
    instancetestB.fun2()
    let instancetestC: namespace2Test.outer.inner.testC =
        namespace2Test.outer.inner.get_interface_C()
    instancetestC.fun3()
}

function main() {
    console.log("run main ... ani_namespace_test begin ...")
    const suite = new ArkTestsuite("test function namespace_test")
    // namespace2
    suite.addTest("primitives_noreturn", primitives_noreturn)
    suite.addTest("primitives_return", primitives_return)
    suite.addTest("containers_noreturn", containers_noreturn)
    suite.addTest("containers_return", containers_return)
    suite.addTest("enum_noreturn", enum_noreturn)
    suite.addTest("enum_return", enum_return)

    suite.addTest("interface_primitives_return", interface_primitives_return)
    suite.addTest(
        "interface_containers_noreturn1", interface_containers_noreturn1)
    suite.addTest(
        "interface_containers_noreturn2", interface_containers_noreturn2)
    suite.addTest(
        "interface_containers_noreturn3", interface_containers_noreturn3)
    suite.addTest(
        "interface_containers_noreturn3", interface_containers_noreturn4)
    suite.addTest(
        "interface_containers_noreturn3", interface_containers_noreturn5)
    suite.addTest("interface_containers_return", interface_containers_return)

    suite.addTest("interface_ErrorResponse", interface_ErrorResponse)

    suite.addTest("overload1", overload1)
    suite.addTest("overload2", overload2)
    suite.addTest("overload3", overload3)
    suite.addTest("overload4", overload4)

    suite.addTest("interface_noparam_noreturn", interface_noparam_noreturn)
    suite.addTest(
        "interface_primitives_noreturn", interface_primitives_noreturn)
    suite.addTest(
        "interface_primitives_noreturn1", interface_primitives_noreturn1)
    suite.addTest(
        "interface_primitives_noreturn2", interface_primitives_noreturn2)
    suite.addTest(
        "interface_primitives_noreturn3", interface_primitives_noreturn3)
    suite.addTest(
        "interface_primitives_noreturn4", interface_primitives_noreturn4)
    suite.addTest(
        "interface_primitives_noreturn5", interface_primitives_noreturn5)
    suite.addTest(
        "interface_primitives_noreturn6", interface_primitives_noreturn6)
    suite.addTest(
        "interface_primitives_noreturn7", interface_primitives_noreturn7)

    suite.addTest("test20interface", test20interface)
    suite.addTest("ns_5", ns_5)

    suite.addTest("testattributeString", testattributeString)
    suite.addTest("testattributei8", testattributei8)
    suite.addTest("testattributei16", testattributei16);
    suite.addTest("testattributei32", testattributei32);
    suite.addTest("testattributei64", testattributei64);
    suite.addTest("testattributef32", testattributef32);
    suite.addTest("testattributef64", testattributef64);
    suite.addTest("testattributebool", testattributebool);
    suite.addTest("testattributeArrayBuffer", testattributeArrayBuffer);
    suite.addTest("testattributeArray", testattributeArray);
    suite.addTest("testattributeUnion", testattributeUnion);
    suite.addTest("testattributeRecord", testattributeRecord);
    suite.addTest("testattributeEnum", testattributeEnum);
    suite.addTest("testinheritance", testinheritance);

    // namespace1
    suite.addTest("check testCasebaseFunctionTest1", testCasebaseFunctionTest1)
    suite.addTest("check testCasebaseFunctionTest2", testCasebaseFunctionTest2)
    suite.addTest("check testCasebaseFunctionTest3", testCasebaseFunctionTest3)
    suite.addTest("check testCasebaseFunctionTest4", testCasebaseFunctionTest4)
    suite.addTest("check testCasebaseFunctionTest5", testCasebaseFunctionTest5)
    suite.addTest("check testCasebaseFunctionTest6", testCasebaseFunctionTest6)
    suite.addTest("check testCasebaseFunctionTest7", testCasebaseFunctionTest7)
    suite.addTest("check testCasebaseFunctionTest8", testCasebaseFunctionTest8)
    suite.addTest("check testCasebaseFunctionTest9", testCasebaseFunctionTest9)
    suite.addTest(
        "check testCasebaseFunctionTest10", testCasebaseFunctionTest10)
    suite.addTest(
        "check testCasebaseFunctionTest11", testCasebaseFunctionTest11)
    suite.addTest(
        "check testCasebaseFunctionTest12", testCasebaseFunctionTest12)
    suite.addTest(
        "check testCasebaseFunctionTest12_1", testCasebaseFunctionTest12_1)
    suite.addTest(
        "check testCasebaseFunctionTest13", testCasebaseFunctionTest13)
    suite.addTest(
        "check testCasebaseFunctionTest13_1", testCasebaseFunctionTest13_1)
    suite.addTest(
        "check testCasebaseFunctionTest14", testCasebaseFunctionTest14)
    suite.addTest(
        "check testCasebaseFunctionTest15", testCasebaseFunctionTest15)
    suite.addTest(
        "check testCasebaseFunctionTest16", testCasebaseFunctionTest16)
    suite.addTest(
        "check testCasebaseFunctionTest16_1", testCasebaseFunctionTest16_1)
    suite.addTest(
        "check testCasebaseFunctionTest17", testCasebaseFunctionTest17)
    suite.addTest(
        "check testCasebaseFunctionTest17_1", testCasebaseFunctionTest17_1)
    suite.addTest(
        "check testCasebaseFunctionTest18", testCasebaseFunctionTest18)
    suite.addTest(
        "check testCasebaseFunctionTest18_1", testCasebaseFunctionTest18_1)
    suite.addTest(
        "check testCasebaseFunctionTest19", testCasebaseFunctionTest19)
    suite.addTest(
        "check testCasebaseFunctionTest20", testCasebaseFunctionTest20)
    suite.addTest(
        "check testCasebaseFunctionTest21", testCasebaseFunctionTest21)
    suite.addTest(
        "check testCasebaseFunctionTest22", testCasebaseFunctionTest22)
    suite.addTest(
        "check testCasebaseFunctionTest22_1", testCasebaseFunctionTest22_1)
    suite.addTest(
        "check testCasebaseFunctionTest23", testCasebaseFunctionTest23)
    suite.addTest(
        "check testCasebaseFunctionTest23_1", testCasebaseFunctionTest23_1)
    suite.addTest(
        "check testCasebaseFunctionTest24", testCasebaseFunctionTest24)
    suite.addTest(
        "check testCasebaseFunctionTest25", testCasebaseFunctionTest25)
    suite.addTest(
        "check testCasebaseFunctionTest26", testCasebaseFunctionTest26)
    suite.addTest(
        "check testCasebaseFunctionTest26_1", testCasebaseFunctionTest26_1)
    suite.addTest(
        "check testCasebaseFunctionTest27", testCasebaseFunctionTest27)
    suite.addTest(
        "check testCasebaseFunctionTest27_1", testCasebaseFunctionTest27_1)
    suite.addTest(
        "check testCasebaseFunctionTest28", testCasebaseFunctionTest28)
    suite.addTest(
        "check testCasebaseFunctionTest29", testCasebaseFunctionTest29)
    suite.addTest(
        "check testCasebaseFunctionTest30", testCasebaseFunctionTest30)
    suite.addTest(
        "check testCasebaseFunctionTest31", testCasebaseFunctionTest31)
    suite.addTest(
        "check testCasebaseFunctionTest32", testCasebaseFunctionTest32)
    suite.addTest(
        "check testCasebaseFunctionTest32_1", testCasebaseFunctionTest32_1)
    suite.addTest(
        "check testCasebaseFunctionTest33", testCasebaseFunctionTest33)
    suite.addTest(
        "check testCasebaseFunctionTest33_1", testCasebaseFunctionTest33_1)
    suite.addTest(
        "check testCasebaseFunctionTest34", testCasebaseFunctionTest34)
    suite.addTest(
        "check testCasebaseFunctionTest34_1", testCasebaseFunctionTest34_1)
    suite.addTest(
        "check testCasebaseFunctionTest35", testCasebaseFunctionTest35)
    suite.addTest(
        "check testCasebaseFunctionTest35_1", testCasebaseFunctionTest35_1)
    suite.addTest(
        "check testCasebaseFunctionTest36", testCasebaseFunctionTest36)
    suite.addTest(
        "check testCasebaseFunctionTest36_1", testCasebaseFunctionTest36_1)
    suite.addTest(
        "check testCasebaseFunctionTest37", testCasebaseFunctionTest37)
    suite.addTest(
        "check testCasebaseFunctionTest37_1", testCasebaseFunctionTest37_1)
    suite.addTest(
        "check testCasebaseFunctionTest38", testCasebaseFunctionTest38)
    suite.addTest(
        "check testCasebaseFunctionTest38_1", testCasebaseFunctionTest38_1)
    suite.addTest(
        "check testCasebaseFunctionTest39", testCasebaseFunctionTest39)
    suite.addTest(
        "check testCasebaseFunctionTest39_1", testCasebaseFunctionTest39_1)
    suite.addTest(
        "check testCasebaseFunctionTest40", testCasebaseFunctionTest40)
    suite.addTest(
        "check testCasebaseFunctionTest40_1", testCasebaseFunctionTest40_1)
    suite.addTest(
        "check testCasebaseFunctionTest41", testCasebaseFunctionTest41)
    suite.addTest(
        "check testCasebaseFunctionTest42", testCasebaseFunctionTest42)
    suite.addTest(
        "check testCasebaseFunctionTest42_1", testCasebaseFunctionTest42_1)
    suite.addTest(
        "check testCasebaseFunctionTest43", testCasebaseFunctionTest43)
    suite.addTest(
        "check testCasebaseFunctionTest43_1", testCasebaseFunctionTest43_1)
    suite.addTest(
        "check testCasebaseFunctionTest44", testCasebaseFunctionTest44)
    suite.addTest(
        "check testCasebaseFunctionTest45", testCasebaseFunctionTest45)
    suite.addTest(
        "check testCasebaseFunctionTest46", testCasebaseFunctionTest46)
    suite.addTest(
        "check testCasebaseFunctionTest47", testCasebaseFunctionTest47)
    suite.addTest(
        "check testCasebaseFunctionTest48", testCasebaseFunctionTest48)
    suite.addTest(
        "check testCasebaseFunctionTest49", testCasebaseFunctionTest49)
    suite.addTest(
        "check testCasebaseFunctionTest50", testCasebaseFunctionTest50)
    suite.addTest(
        "check testCasebaseFunctionTest51", testCasebaseFunctionTest51)
    suite.addTest(
        "check testCasebaseFunctionTest52", testCasebaseFunctionTest52)
    suite.addTest(
        "check testCasebaseFunctionTest53", testCasebaseFunctionTest53)
    suite.addTest(
        "check testCasebaseFunctionTest54", testCasebaseFunctionTest54)
    suite.addTest(
        "check testCasebaseFunctionTest54_1", testCasebaseFunctionTest54_1)
    suite.addTest(
        "check testCasebaseFunctionTest55", testCasebaseFunctionTest55)
    suite.addTest(
        "check testCasebaseFunctionTest56", testCasebaseFunctionTest56)
    suite.addTest(
        "check testCasebaseFunctionTest56_1", testCasebaseFunctionTest56_1)
    suite.addTest(
        "check testCasebaseFunctionTest57", testCasebaseFunctionTest57)
    suite.addTest(
        "check testCasebaseFunctionTest57_1", testCasebaseFunctionTest57_1)
    // suite.addTest("check testCasebaseFunctionTest58",
    // testCasebaseFunctionTest58)
    suite.addTest(
        "check testCasebaseFunctionTest59", testCasebaseFunctionTest59)
    suite.addTest(
        "check testCasebaseFunctionTest60", testCasebaseFunctionTest60)
    suite.addTest(
        "check testCasebaseFunctionTest61", testCasebaseFunctionTest61)
    suite.addTest(
        "check testCasebaseFunctionTest62", testCasebaseFunctionTest62)
    suite.addTest(
        "check testCasebaseFunctionTest62_1", testCasebaseFunctionTest62_1)
    // performance
    suite.addTest(
        "check testInter1basePerformanceFunctionTest1",
        testInter1basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter1basePerformanceFunctionTest2",
        testInter1basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter2basePerformanceFunctionTest1",
        testInter2basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter2basePerformanceFunctionTest2",
        testInter2basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter3basePerformanceFunctionTest1",
        testInter3basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter3basePerformanceFunctionTest2",
        testInter3basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter4basePerformanceFunctionTest1",
        testInter4basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter4basePerformanceFunctionTest2",
        testInter4basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter5basePerformanceFunctionTest1",
        testInter5basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter5basePerformanceFunctionTest2",
        testInter5basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter6basePerformanceFunctionTest1",
        testInter6basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter6basePerformanceFunctionTest2",
        testInter6basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter7basePerformanceFunctionTest1",
        testInter7basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter7basePerformanceFunctionTest2",
        testInter7basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter8basePerformanceFunctionTest1",
        testInter8basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter8basePerformanceFunctionTest2",
        testInter8basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter9basePerformanceFunctionTest1",
        testInter9basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter9basePerformanceFunctionTest2",
        testInter9basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter10basePerformanceFunctionTest1",
        testInter10basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter10basePerformanceFunctionTest2",
        testInter10basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter11basePerformanceFunctionTest1",
        testInter11basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter11basePerformanceFunctionTest2",
        testInter11basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter12basePerformanceFunctionTest1",
        testInter12basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter12basePerformanceFunctionTest2",
        testInter12basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter13basePerformanceFunctionTest1",
        testInter13basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter13basePerformanceFunctionTest2",
        testInter13basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter14basePerformanceFunctionTest1",
        testInter14basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter14basePerformanceFunctionTest2",
        testInter14basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter15basePerformanceFunctionTest1",
        testInter15basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter15basePerformanceFunctionTest2",
        testInter15basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter16basePerformanceFunctionTest1",
        testInter16basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter16basePerformanceFunctionTest2",
        testInter16basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter17basePerformanceFunctionTest1",
        testInter17basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter17basePerformanceFunctionTest2",
        testInter17basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter18basePerformanceFunctionTest1",
        testInter18basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter18basePerformanceFunctionTest2",
        testInter18basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter19basePerformanceFunctionTest1",
        testInter19basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter19basePerformanceFunctionTest2",
        testInter19basePerformanceFunctionTest2)
    suite.addTest(
        "check testInter20basePerformanceFunctionTest1",
        testInter20basePerformanceFunctionTest1)
    suite.addTest(
        "check testInter20basePerformanceFunctionTest2",
        testInter20basePerformanceFunctionTest2)
    suite.run()
}
