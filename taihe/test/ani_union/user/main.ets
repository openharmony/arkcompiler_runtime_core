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
import * as UnionTest1 from "test_union";
import * as UnionTest from "union_ani";
import {BusinessError} from "@ohos.base";

loadLibrary("ani_union");

function test_printMyUnion_with_undefined() {
    let res = UnionTest.printMyUnion(undefined);
    assertEQ(res, "u")
}

function test_printMyUnion_with_string() {
    let res = UnionTest.printMyUnion("name");
    assertEQ(res, "s")
}

function test_printMyUnion_with_number() {
    let res = UnionTest.printMyUnion(123.456 as float);
    assertEQ(res, "f")
}

function test_printMyUnion_with_pair() {
    let pair: UnionTest.Pair = {a: "a", b: "b"};
    let res = UnionTest.printMyUnion(pair);
    assertEQ(res, "p")
}

function test_makeMyUnion_string() {
    const result = UnionTest.makeMyUnion("s");
    assertEQ(result, "string");
}

function test_makeMyUnion_number() {
    const result = UnionTest.makeMyUnion("f");
    assertEQ(result, 123);
}

function test_makeMyUnion_pair() {
    const result = UnionTest.makeMyUnion("p");
    if (result instanceof UnionTest.Pair) {
        assertEQ(result.a, "a");
        assertEQ(result.b, "b");
    } else {
        throw new Error("Result type error");
    }
}

function test_makeMyUnion_undefined() {
    const result = UnionTest.makeMyUnion("u");
    assertEQ(result, undefined);
}

function test_primitives() {
    let res: String;

    res = UnionTest1.printUnion("name")
    console.log(res)

    res = UnionTest1.printUnion(1 as byte)
    console.log(res)

    res = UnionTest1.printUnion(123 as short)
    console.log(res)

    res = UnionTest1.printUnion(123456 as int)
    console.log(res)

    res = UnionTest1.printUnion(1.23 as float)
    console.log(res)

    res = UnionTest1.printUnion(1.2345 as double)
    console.log(res)

    res = UnionTest1.printUnion(true)
    console.log(res)
    res = UnionTest1.printUnion(false)
    console.log(res)
}

// 定义 10 个测试函数
function test_interface_Union_primitives1() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive("Hello" as String, true);
}

function test_interface_Union_primitives2() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive("World" as String, 42 as byte);
}

function test_interface_Union_primitives3() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive(1 as byte, 12 as short);
}

function test_interface_Union_primitives4() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive(12 as short, 123 as int);
}

function test_interface_Union_primitives5() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive(123 as int, 1.23 as float);
}

function test_interface_Union_primitives6() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive(1.23 as float, 1.2345 as double);
}

function test_interface_Union_primitives7() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive(true, 42 as byte);
}

function test_interface_Union_primitives8() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive(false, 123 as short);
}

function test_interface_Union_primitives9() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive("Test" as String, 3.14 as float);
}

function test_interface_Union_primitives10() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive(12345 as int, 2.71828 as double);
}

function test_interface_Union_10param_primitives1() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive5Param(
        12345 as int, 2.71828 as double, 12 as short, 1 as byte, "hello");
}

function test_interface_Union_10param_primitives2() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive5Param(
        42 as int, 3.14 as double, 100 as short, 2 as byte, "world");
}

function test_interface_Union_10param_primitives3() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive5Param(
        9876 as int, 1.414 as double, 50 as short, 3 as byte, "test");
}

function test_interface_Union_10param_primitives4() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive5Param(
        123 as int, 0.577 as double, 200 as short, 4 as byte, "example");
}

function test_interface_Union_10param_primitives5() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive5Param(
        4567 as int, 1.618 as double, 150 as short, 5 as byte, "demo");
}
function test_interface_Union_10param_primitives6() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive5Param(
        1000 as int, 2.5 as double, 10 as short, 6 as byte, "alpha");
}

function test_interface_Union_10param_primitives7() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive5Param(
        2000 as int, 3.0 as double, 20 as short, 7 as byte, "beta");
}

function test_interface_Union_10param_primitives8() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive5Param(
        3000 as int, 1.5 as double, 30 as short, 8 as byte, "gamma");
}

function test_interface_Union_10param_primitives9() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive5Param(
        4000 as int, 0.5 as double, 40 as short, 9 as byte, "delta");
}

function test_interface_Union_10param_primitives10() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitive5Param(
        5000 as int, 1.0 as double, 50 as short, 10 as byte, "epsilon");
}



function test_interface_Union_enum_map() {
    const studentScores: Record<String, byte> = {
        "math": 90 as byte,
        "english": 85 as byte,
        "science": 95 as byte,
    };
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();

    ////enum
    let color = UnionTest1.Color.RED;
    instance.funcColorMapValue1(color)

    instance.funcColorMapValue1(studentScores)
}
function test_interface_Union_enum_map1() {
    const studentScores: Record<String, short> = {
        "math": 90 as short,
        "english": 85 as short,
        "science": 95 as short,
    };
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();

    ////enum
    let color = UnionTest1.Color.RED;
    instance.funcColorMapValue2(color)

    instance.funcColorMapValue2(studentScores)
}

function test_interface_Union_enum_map2() {
    const studentScores: Record<String, int> = {
        "math": 90 as int,
        "english": 85 as int,
        "science": 95 as int
    };
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcColorMapValue3(studentScores);
}


function test_interface_Union_enum_map3() {
    const studentScores:
        Record<String, long> = {"math": 90 as long, "english": 85 as long};
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcColorMapValue4(studentScores);
}

function test_interface_Union_enum_map4() {
    let studentScores: Record<String, float> = {
        "math": 123.45 as float,
        "english": 57.89 as float
    };
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcColorMapValue5(studentScores);
}

function test_interface_Union_enum_map5() {
    const studentScores: Record<String, double> = {
        "math": 90.5 as double,
        "english": 85.3 as double
    };
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcColorMapValue6(studentScores);
}

function test_interface_Union_enum_map6() {
    const studentPassed:
        Record<String, boolean> = {"math": true, "english": false};
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcColorMapValue7(studentPassed);
}

function test_interface_Union_enum_map7() {
    const studentNames:
        Record<String, String> = {"math": "Algebra", "english": "Literature"};
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcColorMapValue8(studentNames);
}

function test_interface_Union_enum_map8() {
    const scores: Record<int, int> = {100: 90 as int, 200: 85 as int};
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcColorMapValue9(scores);
}


function test_interface_Union_enum_map9() {
    const scores: Record<int, int> = {100: 90 as int, 200: 85 as int};
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcColorMapValue10(scores, scores);
}


function test_interface_Union_array_map() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    let arri8: byte[] = [1, 2, 3, 4];
    const studentScores: Record<String, byte> = {
        "math": 90 as byte,
        "english": 85 as byte,
        "science": 95 as byte,
    };
    console.log(instance.funcUnionArrayMap1(arri8, studentScores))
}
function test_interface_Union_array_map1() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    let arri8: byte[] = [1, 2, 3, 4];
    const studentScores: Record<String, short> = {
        "math": 90 as short,
        "english": 85 as short,
        "science": 95 as short,
    };
    console.log(instance.funcUnionArrayMap2(arri8, studentScores))
}
function test_interface_Union_array_map2() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    const studentScores: Record<String, int> = {
        "math": 90 as int,
        "english": 85 as int,
        "science": 95 as int
    };
    let arri8: byte[] = [1, 2, 3, 4]
    console.log(instance.funcUnionArrayMap3(arri8, studentScores))
}
function test_interface_Union_array_map3() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    const studentScores:
        Record<String, long> = {"math": 90 as long, "english": 85 as long};
    let arri8: byte[] = [1, 2, 3, 4]
    console.log(instance.funcUnionArrayMap4(arri8, studentScores))
}
function test_interface_Union_array_map4() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    let studentScores: Record<String, float> = {
        "math": 123.45 as float,
        "english": 57.89 as float
    };
    let arri8: byte[] = [1, 2, 3, 4]
    console.log(instance.funcUnionArrayMap5(arri8, studentScores))
}
function test_interface_Union_array_map5() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    const studentScores: Record<String, double> = {
        "math": 90.5 as double,
        "english": 85.3 as double
    };
    let arri8: byte[] = [1, 2, 3, 4]
    console.log(instance.funcUnionArrayMap6(arri8, studentScores))
}
function test_interface_Union_array_map6() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    const studentPassed:
        Record<String, boolean> = {"math": true, "english": false};
    let arri8: byte[] = [1, 2, 3, 4]
    console.log(instance.funcUnionArrayMap7(arri8, studentPassed))
}
function test_interface_Union_array_map7() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    const studentNames:
        Record<String, String> = {"math": "Algebra", "english": "Literature"};
    let arri8: byte[] = [1, 2, 3, 4]
    console.log(instance.funcUnionArrayMap8(arri8, studentNames))
}
function test_interface_Union_array_map8() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    const studentNames: Record<int, int> = {
        1: 99,
        2: 100,
    };
    let arri8: byte[] = [1, 2, 3, 4]
    console.log(instance.funcUnionArrayMap9(arri8, studentNames))
}
function test_interface_Union_array_map9() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    const studentScores1: Record<String, byte> = {
        "math": 90 as byte,
        "english": 85 as byte,
        "science": 95 as byte,
    };
    const studentScores2: Record<String, short> = {
        "math": 90 as short,
        "english": 85 as short,
        "science": 95 as short,
    };
    let arri8: byte[] = [1, 2, 3, 4]
    console.log(instance.funcUnionArrayMap10(arri8, arri8))
    console.log(instance.funcUnionArrayMap10(arri8, studentScores2))
    console.log(instance.funcUnionArrayMap10(studentScores1, arri8))
    console.log(instance.funcUnionArrayMap10(studentScores1, studentScores2))
}


function test_union_mix() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    let arri8: byte[] = [1, 2, 3, 4];
    let color = UnionTest1.Color.RED;

    console.log(instance.funcUnionMix(arri8, "hello", color, 1 as byte, true))
}
function test_union_mix_10param1() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    let arri8: byte[] = [1, 2, 3, 4];
    let color = UnionTest1.Color.RED;

    console.log(instance.funcUnionMix10Param(
        arri8, "hello", color, 1 as byte, true, 5000 as int, 1.0 as double,
        50 as short, 10 as byte, "epsilon"));
}

function test_union_mix_10param2() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    let arri8: byte[] = [1, 2, 3, 4];
    let color = UnionTest1.Color.RED;

    console.log(instance.funcUnionMix10Param(
        "hello", arri8, 1 as byte, color, true, 1.0 as double, 5000 as int,
        50 as short, 10 as byte, "epsilon"));
}

function test_union_mix_10param3() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    let arri8: byte[] = [1, 2, 3, 4];
    let color = UnionTest1.Color.RED;

    console.log(instance.funcUnionMix10Param(
        color, true, arri8, 1 as byte, "hello", 50 as short, 5000 as int,
        1.0 as double, 10 as byte, "epsilon"));
}

function test_union_mix_10param4() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    let arri8: byte[] = [1, 2, 3, 4];
    let color = UnionTest1.Color.RED;

    console.log(instance.funcUnionMix10Param(
        true, 1.0 as double, 5000 as int, 50 as short, 10 as byte, "epsilon",
        arri8, "hello", color, 1 as byte));
}

function test_union_mix_10param5() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    let arri8: byte[] = [1, 2, 3, 4];
    let color = UnionTest1.Color.RED;

    console.log(instance.funcUnionMix10Param(
        5000 as int, 1.0 as double, 50 as short, 10 as byte, "epsilon", arri8,
        "hello", color, 1 as byte, true));
}

// 2个组合primitive
function test_2_primitive1() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitiveReturn("s");
    instance.funcUnionPrimitiveReturn("i8");
}

function test_2_primitive2() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitiveReturn1("i8");
    instance.funcUnionPrimitiveReturn1("i16");
}

function test_2_primitive3() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitiveReturn2("i8_1");
    instance.funcUnionPrimitiveReturn2("i8_2");
}

function test_2_primitive4() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitiveReturn3("i16");
    instance.funcUnionPrimitiveReturn3("i32");
}

function test_2_primitive5() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitiveReturn4("i32");
    instance.funcUnionPrimitiveReturn4("f32");
}

function test_2_primitive6() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitiveReturn5("f32");
    instance.funcUnionPrimitiveReturn5("f64");
}

function test_2_primitive7() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitiveReturn6("s");
    instance.funcUnionPrimitiveReturn6("b");
}

function test_2_primitive8() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitiveReturn7("b");
    instance.funcUnionPrimitiveReturn7("i8");
}

function test_2_primitive9() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();

    instance.funcUnionPrimitiveReturn8("b");
    instance.funcUnionPrimitiveReturn8("b1");
}

function test_2_primitive10() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionPrimitiveReturn9("s");
    instance.funcUnionPrimitiveReturn9("s1");
}

// 5组合所有类型return

function test_mix5_return1() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionMix5Return("s");
}
function test_mix5_return2() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionMix5Return("i8");
}
function test_mix5_return3() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionMix5Return("b");
}
function test_mix5_return4() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionMix5Return("c");
}
function test_mix5_return5() {
    let instance: UnionTest1.MyInterface = UnionTest1.getInterface();
    instance.funcUnionMix5Return("arr");
}


function test_union_with_makeAndShowBasicUnion(){
    // i8
    let i8Result = UnionTest.makeAndShowBasicUnion(1 as byte);
    console.log("BasicUnion i8Result: ", i8Result);
    // i16
    let i16Result = UnionTest.makeAndShowBasicUnion(123 as short);
    console.log("BasicUnion i16Result: ", i16Result);
    // i32
    let i32Result = UnionTest.makeAndShowBasicUnion(123456 as int);
    console.log("BasicUnion i32Result: ", i32Result);
    // i64
    let i64Result = UnionTest.makeAndShowBasicUnion(123456789 as long);
    console.log("BasicUnion i64Result: ", i64Result);
    //float32
    let float32Result = UnionTest.makeAndShowBasicUnion(1.23 as float);
    console.log("BasicUnion float32Result: ", float32Result);
    //float64 
    let float64Result = UnionTest.makeAndShowBasicUnion(1.2345 as double);
    console.log("BasicUnion float64Result: ", float64Result);
    //string
    let stringResult = UnionTest.makeAndShowBasicUnion("this is string data");
    console.log("BasicUnion stringResult: ", stringResult);
    //bool true
    let boolResult = UnionTest.makeAndShowBasicUnion(true);
    console.log("BasicUnion boolResult: ", boolResult);
    //bool true
    let boolResult1 = UnionTest.makeAndShowBasicUnion(false);
    console.log("BasicUnion boolResult1: ", boolResult1);
    // null
    let nullResult = UnionTest.makeAndShowBasicUnion(null);
    console.log("BasicUnion nullResult: ", nullResult);
}

function test_union_with_makeAndShowTypedArrayUnion(){
    //u8
    let u8Arr: Uint8Array = new Uint8Array([1, 2, 3]);
    let u8ArrResult = UnionTest.makeAndShowTypedArrayUnion(u8Arr) as Uint8Array;
    assertEQ(u8ArrResult.toString(), u8Arr.toString());
    //i8
    let i8Arr: Int8Array = new Int8Array([-1, -2, -3]);
    let i8ArrResult = UnionTest.makeAndShowTypedArrayUnion(i8Arr) as Int8Array;
    assertEQ(i8ArrResult.toString(), i8Arr.toString());
    //u16
    let u16Arr: Uint16Array = new Uint16Array([10, 20, 30]);
    let u16ArrResult = UnionTest.makeAndShowTypedArrayUnion(u16Arr) as Uint16Array;
    assertEQ(u16ArrResult.toString(), u16Arr.toString());
    //i16
    let i16Arr: Int16Array = new Int16Array([-10, -20, -30]);
    let i16ArrResult = UnionTest.makeAndShowTypedArrayUnion(i16Arr) as Int16Array;
    assertEQ(i16ArrResult.toString(), i16Arr.toString());
    //u32
    let u32Arr: Uint32Array = new Uint32Array([100, 200, 300]);
    let u32ArrResult = UnionTest.makeAndShowTypedArrayUnion(u32Arr) as Uint32Array;
    assertEQ(u32ArrResult.toString(), u32Arr.toString());
    //i32
    let i32Arr: Int32Array = new Int32Array([-100, -200, -300]);
    let i32ArrResult = UnionTest.makeAndShowTypedArrayUnion(i32Arr) as Int32Array;
    assertEQ(i32ArrResult.toString(), i32Arr.toString());
    //u64
    let u64Arr: BigUint64Array = new BigUint64Array([1000n, 2000n, 3000n]);
    let u64ArrResult = UnionTest.makeAndShowTypedArrayUnion(u64Arr) as BigUint64Array;
    assertEQ(u64ArrResult.toString(), u64Arr.toString());
    //i64
    let i64Arr: BigInt64Array = new BigInt64Array([-1000n, -2000n, -3000n]);
    let i64ArrResult = UnionTest.makeAndShowTypedArrayUnion(i64Arr) as BigInt64Array;
    assertEQ(i64ArrResult.toString(), i64Arr.toString());
    //f32
    let f32Arr: Float32Array = new Float32Array([1.23, -2.14, 3.16]);
    let f32ArrResult = UnionTest.makeAndShowTypedArrayUnion(f32Arr) as Float32Array;
    assertEQ(f32ArrResult.toString(), f32Arr.toString());
    //f64
    let f64Arr: Float64Array = new Float64Array([1.234567, -2.145789, 3.161862]);
    let f64ArrResult = UnionTest.makeAndShowTypedArrayUnion(f64Arr) as Float64Array;
    assertEQ(f64ArrResult.toString(), f64Arr.toString());
    //null
    let nullResult = UnionTest.makeAndShowTypedArrayUnion(null);
    assertEQ(nullResult, null);
}

function test_union_with_makeAndShowArrayUnion(){
    //Array<f32> Array<f64>
    let arrStruct: UnionTest.ArrayStruct = {float32Array: [1.23f, -2.14f, 3.16f], float64Array: [1.234567, -2.145789, 3.161862]};
    let arrResult: UnionTest.ArrayUnion = UnionTest.makeAndShowArrayUnion(arrStruct);
    console.log("ArrayUnion arrResult a: ", (arrResult as UnionTest.ArrayStruct)!.float32Array);
    console.log("ArrayUnion arrResult b: ", (arrResult as UnionTest.ArrayStruct)!.float64Array);

    //null
    let nullResult = UnionTest.makeAndShowArrayUnion(null);
    console.log("ArrayUnion nullResult: ", nullResult); 
}

function test_union_with_makeAndShowStructUnion(){
    //StrStruct
    let strStruct: UnionTest.StrStruct= {a: "StrStruct a", b: "StrStruct b"};
    let strResult: UnionTest.StructUnion = UnionTest.makeAndShowStructUnion(strStruct);
    console.log("StructUnion strResult a: ", (strResult as UnionTest.StrStruct)!.a);
    console.log("StructUnion strResult b: ", (strResult as UnionTest.StrStruct)!.b);

    //IntStruct
    let intStruct: UnionTest.IntStruct= {a: 1, b: 2 };
    let intResult: UnionTest.StructUnion = UnionTest.makeAndShowStructUnion(intStruct);
    console.log("StructUnion intResult a: ", (intResult as UnionTest.IntStruct)!.a);
    console.log("StructUnion intResult b: ", (intResult as UnionTest.IntStruct)!.b);

    //null
    let nullResult = UnionTest.makeAndShowStructUnion(null);
    console.log("StructUnion nullResult: ", nullResult);
}

function main() {
    console.log("##############start#############");
    const suite = new ArkTestsuite("UnionTest Tests");

    suite.addTest("test printMyUnion with undefined", test_printMyUnion_with_undefined);
    suite.addTest("test printMyUnion with string", test_printMyUnion_with_string);
    suite.addTest("test printMyUnion with number", test_printMyUnion_with_number);
    suite.addTest("test printMyUnion with pair", test_printMyUnion_with_pair);
    suite.addTest("test makeMyUnion string", test_makeMyUnion_string);
    suite.addTest("test makeMyUnion number", test_makeMyUnion_number);
    suite.addTest("test makeMyUnion pair", test_makeMyUnion_pair);
    suite.addTest("test makeMyUnion undefined", test_makeMyUnion_undefined);

    suite.addTest("test_interface_Union_primitives1", test_interface_Union_primitives1);
    suite.addTest("test_interface_Union_primitives2", test_interface_Union_primitives2);
    suite.addTest("test_interface_Union_primitives3", test_interface_Union_primitives3);
    suite.addTest("test_interface_Union_primitives4", test_interface_Union_primitives4);
    suite.addTest("test_interface_Union_primitives5", test_interface_Union_primitives5);
    suite.addTest("test_interface_Union_primitives6", test_interface_Union_primitives6);
    suite.addTest("test_interface_Union_primitives7", test_interface_Union_primitives7);
    suite.addTest("test_interface_Union_primitives8", test_interface_Union_primitives8);
    suite.addTest("test_interface_Union_primitives9", test_interface_Union_primitives9);
    suite.addTest("test_interface_Union_primitives10", test_interface_Union_primitives10);
    // primitive_5param
    suite.addTest("test_interface_Union_10param_primitives1",test_interface_Union_10param_primitives1);
    suite.addTest("test_interface_Union_10param_primitives2",test_interface_Union_10param_primitives2);
    suite.addTest("test_interface_Union_10param_primitives3",test_interface_Union_10param_primitives3);
    suite.addTest("test_interface_Union_10param_primitives4",test_interface_Union_10param_primitives4);
    suite.addTest("test_interface_Union_10param_primitives5",test_interface_Union_10param_primitives5);
    suite.addTest("test_interface_Union_10param_primitives6",test_interface_Union_10param_primitives6);
    suite.addTest("test_interface_Union_10param_primitives7",test_interface_Union_10param_primitives7);
    suite.addTest("test_interface_Union_10param_primitives8",test_interface_Union_10param_primitives8);
    suite.addTest("test_interface_Union_10param_primitives9",test_interface_Union_10param_primitives9);
    suite.addTest("test_interface_Union_10param_primitives10",test_interface_Union_10param_primitives10);

    // mix_5param
    suite.addTest("test_union_mix", test_union_mix);
    suite.addTest("test_union_mix_10param1", test_union_mix_10param1);
    suite.addTest("test_union_mix_10param2", test_union_mix_10param2);
    suite.addTest("test_union_mix_10param3", test_union_mix_10param3);
    suite.addTest("test_union_mix_10param4", test_union_mix_10param4);
    suite.addTest("test_union_mix_10param5", test_union_mix_10param5);

    // 2组合primitive return
    suite.addTest("test_2_primitive1", test_2_primitive1);
    suite.addTest("test_2_primitive2", test_2_primitive2);
    suite.addTest("test_2_primitive3", test_2_primitive3);
    suite.addTest("test_2_primitive4", test_2_primitive4);
    suite.addTest("test_2_primitive5", test_2_primitive5);
    suite.addTest("test_2_primitive6", test_2_primitive6);
    suite.addTest("test_2_primitive7", test_2_primitive7);
    suite.addTest("test_2_primitive8", test_2_primitive8);
    suite.addTest("test_2_primitive9", test_2_primitive9);
    suite.addTest("test_2_primitive10", test_2_primitive10);

    // 5组合所有类型return
    suite.addTest("test_mix5_return1", test_mix5_return1);
    suite.addTest("test_mix5_return2", test_mix5_return2);
    suite.addTest("test_mix5_return3", test_mix5_return3);
    suite.addTest("test_mix5_return4", test_mix5_return4);
    suite.addTest("test_mix5_return5", test_mix5_return5);

    // 10组合所有类型return

    // enum map
    suite.addTest("test_interface_Union_enum_map", test_interface_Union_enum_map);
    suite.addTest("test_interface_Union_enum_map1", test_interface_Union_enum_map1);
    suite.addTest("test_interface_Union_enum_map2", test_interface_Union_enum_map2);
    suite.addTest("test_interface_Union_enum_map3", test_interface_Union_enum_map3);
    suite.addTest("test_interface_Union_enum_map4", test_interface_Union_enum_map4);
    suite.addTest("test_interface_Union_enum_map5", test_interface_Union_enum_map5);
    suite.addTest("test_interface_Union_enum_map6", test_interface_Union_enum_map6);
    suite.addTest("test_interface_Union_enum_map7", test_interface_Union_enum_map7);
    suite.addTest("test_interface_Union_enum_map8", test_interface_Union_enum_map8);
    suite.addTest("test_interface_Union_enum_map9", test_interface_Union_enum_map9);

    // arr
    suite.addTest("test_interface_Union_array_map", test_interface_Union_array_map);
    suite.addTest("test_interface_Union_array_map1", test_interface_Union_array_map1);
    suite.addTest("test_interface_Union_array_map2", test_interface_Union_array_map2);
    suite.addTest("test_interface_Union_array_map3", test_interface_Union_array_map3);
    suite.addTest("test_interface_Union_array_map4", test_interface_Union_array_map4);
    suite.addTest("test_interface_Union_array_map5", test_interface_Union_array_map5);
    suite.addTest("test_interface_Union_array_map6", test_interface_Union_array_map6);
    suite.addTest("test_interface_Union_array_map7", test_interface_Union_array_map7);
    suite.addTest("test_interface_Union_array_map8", test_interface_Union_array_map8);
    suite.addTest("test_interface_Union_array_map9", test_interface_Union_array_map9);

    // union basic
    suite.addTest("test_union_with_makeAndShowBasicUnion", test_union_with_makeAndShowBasicUnion);
    // union typedarray
    suite.addTest("test_union_with_makeAndShowTypedArrayUnion", test_union_with_makeAndShowTypedArrayUnion);    
    // union array
    suite.addTest("test_union_with_makeAndShowArrayUnion", test_union_with_makeAndShowArrayUnion);    
    //union struct 
    suite.addTest("test_union_with_makeAndShowStructUnion", test_union_with_makeAndShowStructUnion);

    exit(suite.run());
}
