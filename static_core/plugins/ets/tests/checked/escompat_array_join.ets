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

const arrayIntStrRes = "1,3,5,7,9";
const arrayIntStrRes1 = "1|3|5|7|9";
const arrayIntStrRes2 = "1test3test5test7test9";
const arrayFloatStrRes= "1.5,3.7,5.8,7.4,9.2";
const arrayFloatStrRes1 = "1.5|3.7|5.8|7.4|9.2"
const arrayFloatStrRes2 = "1.5test3.7test5.8test7.4test9.2";
const arrayIntMaxStrRes = "2147483648,0,1,2,3,4,5,6,7,8,9,2147483647";
const arrayIntMaxStrRes1 = "2147483648|0|1|2|3|4|5|6|7|8|9|2147483647";
const arrayIntMaxStrRes2 = "2147483648test0test1test2test3test4test5test6test7test8test9test2147483647";
const arrayLongMaxStrRes = "-9223372036854775808,0,1,2,3,4,5,6,7,8,9,9223372036854775807";
const arrayLongMaxStrRes1 = "-9223372036854775808|0|1|2|3|4|5|6|7|8|9|9223372036854775807";
const arrayLongMaxStrRes2 = "-9223372036854775808test0test1test2test3test4test5test6test7test8test9test9223372036854775807";
const arrayShortMaxStrRes = "32768,0,1,2,3,4,5,6,7,8,9,32767";
const arrayShortMaxStrRes1 = "32768|0|1|2|3|4|5|6|7|8|9|32767";
const arrayShortMaxStrRes2 = "32768test0test1test2test3test4test5test6test7test8test9test32767";
const arrayFloatMaxStrRes = "1.401298464324817e-45,NaN,2,4,6,8,3.4028234663852886e+38";
const arrayFloatMaxStrRes1 = "1.401298464324817e-45|NaN|2|4|6|8|3.4028234663852886e+38";
const arrayFloatMaxStrRes2 = "1.401298464324817e-45testNaNtest2test4test6test8test3.4028234663852886e+38";
const arrayDoubleMaxStrRes = "5e-324,NaN,2,4,6,8,1.7976931348623157e+308";
const arrayDoubleMaxStrRes1 = "5e-324|NaN|2|4|6|8|1.7976931348623157e+308";
const arrayDoubleMaxStrRes2 = "5e-324testNaNtest2test4test6test8test1.7976931348623157e+308";
const arrayStrRes = "This,is,hello,world";
const arrayStrRes1 = "Thisishelloworld";
const arrayStrRes2 = "This|is|hello|world";
const arrayStrRes3 = "Thistestistesthellotestworld";
const arrayStrRes4 = "ThisΩisΩhelloΩworld";


//! CHECKER       ArrayInt join
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true , options: "--compiler-dump:final", entry: "escompat_array_join.ETSGLOBAL::test_array_int_join"
//! METHOD        "escompat_array_join.ETSGLOBAL::test_array_int_join"
//! PASS_AFTER    "Codegen"
//! INST          /Call.*escompat.Array::join*/
function test_array_int_join() {
    let arrayEmpty: Array<int> = new Array<int>(0);
    let arrayInt: Array<int> = [1,3,5,7,9];
    let arrayMax: Array<int> = [Int.MIN_VALUE,0,1,2,3,4,5,6,7,8,9,Int.MAX_VALUE];
    let arrayEmptyStr = arrayEmpty.join();
    let arrayEmptyStr1 = arrayEmpty.join("|");
    let arrayEmptyStr2 = arrayEmpty.join("test");
    let arrayIntStr = arrayInt.join();
    let arrayIntStr1 = arrayInt.join("|");
    let arrayIntStr2 = arrayInt.join("test");
    let arrayMaxStr = arrayMax.join();
    let arrayMaxStr1 = arrayMax.join("|");
    let arrayMaxStr2 =arrayMax.join("test");
    assertTrue(arrayEmptyStr.isEmpty());
    assertTrue(arrayEmptyStr1.isEmpty());
    assertTrue(arrayEmptyStr2.isEmpty());
    assertEQ(arrayIntStr,  arrayIntStrRes);
    assertEQ(arrayIntStr1,  arrayIntStrRes1);
    assertEQ(arrayIntStr2,  arrayIntStrRes2);
    assertEQ(arrayMaxStr,  arrayIntMaxStrRes);
    assertEQ(arrayMaxStr1,  arrayIntMaxStrRes1);
    assertEQ(arrayMaxStr2,  arrayIntMaxStrRes2);
}

//! CHECKER       ArrayLong join
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true , options: "--compiler-dump:final", entry: "escompat_array_join.ETSGLOBAL::test_array_long_join"
//! METHOD        "escompat_array_join.ETSGLOBAL::test_array_long_join"
//! PASS_AFTER    "Codegen"
//! INST          /Call.*escompat.Array::join*/
function test_array_long_join() {
    let arrayEmpty: Array<long> = new Array<long>(0);
    let arrayLong: Array<long> = [1,3,5,7,9];
    let arrayMax: Array<long> = [Long.MIN_VALUE,0,1,2,3,4,5,6,7,8,9,Long.MAX_VALUE];
    let arrayEmptyStr = arrayEmpty.join();
    let arrayEmptyStr1 = arrayEmpty.join("|");
    let arrayEmptyStr2 = arrayEmpty.join("test");
    let arrayIntStr = arrayLong.join();
    let arrayIntStr1 = arrayLong.join("|");
    let arrayIntStr2 = arrayLong.join("test");
    let arrayMaxStr = arrayMax.join();
    let arrayMaxStr1 = arrayMax.join("|");
    let arrayMaxStr2 =arrayMax.join("test");
    assertTrue(arrayEmptyStr.isEmpty());
    assertTrue(arrayEmptyStr1.isEmpty());
    assertTrue(arrayEmptyStr2.isEmpty());
    assertEQ(arrayIntStr,  arrayIntStrRes);
    assertEQ(arrayIntStr1,  arrayIntStrRes1);
    assertEQ(arrayIntStr2,  arrayIntStrRes2);
    assertEQ(arrayMaxStr,  arrayLongMaxStrRes);
    assertEQ(arrayMaxStr1,  arrayLongMaxStrRes1);
    assertEQ(arrayMaxStr2,  arrayLongMaxStrRes2);
}

//! CHECKER       ArrayShort join
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true , options: "--compiler-dump:final", entry: "escompat_array_join.ETSGLOBAL::test_array_short_join"
//! METHOD        "escompat_array_join.ETSGLOBAL::test_array_short_join"
//! PASS_AFTER    "Codegen"
//! INST          /Call.*escompat.Array::join*/
function test_array_short_join() {
    let arrayEmpty: Array<short> = new Array<short>(0);
    let arrayShort: Array<short> = [1,3,5,7,9];
    let arrayMax: Array<short> = [Short.MIN_VALUE,0,1,2,3,4,5,6,7,8,9,Short.MAX_VALUE];
    let arrayEmptyStr = arrayEmpty.join();
    let arrayEmptyStr1 = arrayEmpty.join("|");
    let arrayEmptyStr2 = arrayEmpty.join("test");
    let arrayIntStr = arrayShort.join();
    let arrayIntStr1 = arrayShort.join("|");
    let arrayIntStr2 = arrayShort.join("test");
    let arrayMaxStr = arrayMax.join();
    let arrayMaxStr1 = arrayMax.join("|");
    let arrayMaxStr2 =arrayMax.join("test");
    assertTrue(arrayEmptyStr.isEmpty());
    assertTrue(arrayEmptyStr1.isEmpty());
    assertTrue(arrayEmptyStr2.isEmpty());
    assertEQ(arrayIntStr,  arrayIntStrRes);
    assertEQ(arrayIntStr1,  arrayIntStrRes1);
    assertEQ(arrayIntStr2,  arrayIntStrRes2);
    assertEQ(arrayMaxStr,  arrayShortMaxStrRes);
    assertEQ(arrayMaxStr1,  arrayShortMaxStrRes1);
    assertEQ(arrayMaxStr2,  arrayShortMaxStrRes2);
}

//! CHECKER       ArrayDouble join
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true , options: "--compiler-dump:final", entry: "escompat_array_join.ETSGLOBAL::test_array_double_join"
//! METHOD        "escompat_array_join.ETSGLOBAL::test_array_double_join"
//! PASS_AFTER    "Codegen"
//! INST          /Call.*escompat.Array::join*/
function test_array_double_join() {
    let arrayEmpty: Array<double> = new Array<double>(0);
    let arrayMax: Array<double> = [Number.MIN_VALUE, Number.NaN, 2, 4, 6, 8, Number.MAX_VALUE];
    let arrayEmptyStr = arrayEmpty.join();
    let arrayEmptyStr1 = arrayEmpty.join("|");
    let arrayEmptyStr2 = arrayEmpty.join("test");
    let arrayMaxStr = arrayMax.join();
    let arrayMaxStr1 = arrayMax.join("|");
    let arrayMaxStr2 =arrayMax.join("test");
    assertTrue(arrayEmptyStr.isEmpty());
    assertTrue(arrayEmptyStr1.isEmpty());
    assertTrue(arrayEmptyStr2.isEmpty());
    assertEQ(arrayMaxStr, arrayDoubleMaxStrRes);
    assertEQ(arrayMaxStr1, arrayDoubleMaxStrRes1);
    assertEQ(arrayMaxStr2, arrayDoubleMaxStrRes2);
}

//! CHECKER       ArrayString join
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true , options: "--compiler-dump:final", entry: "escompat_array_join.ETSGLOBAL::test_array_string_join"
//! METHOD        "escompat_array_join.ETSGLOBAL::test_array_string_join"
//! PASS_AFTER    "Codegen"
//! INST          /Call.*escompat.Array::join*/
function test_array_string_join() {
    let arrayEmpty: Array<string> = new Array<string>(0);
    let arrayString: Array<string> = ["This", "is", "hello", "world"];
    let arrayEmptyStr = arrayEmpty.join();
    let arrayEmptyStr1 = arrayEmpty.join("|");
    let arrayEmptyStr2 = arrayEmpty.join("test");
    let arrayStr = arrayString.join();
    let arrayStr1 = arrayString.join("");
    let arrayStr2 = arrayString.join("|");
    let arrayStr3 =arrayString.join("test");
    let arrayStr4 =arrayString.join("Ω");
    assertEQ(arrayStr, arrayStrRes);
    assertEQ(arrayStr1, arrayStrRes1);
    assertEQ(arrayStr2, arrayStrRes2);
    assertEQ(arrayStr3, arrayStrRes3);
    assertEQ(arrayStr4, arrayStrRes4);
}
