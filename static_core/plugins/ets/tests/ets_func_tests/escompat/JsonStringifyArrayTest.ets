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

function jsonStringifyArrayBoolean() {
    let arr = Array.of<boolean>(false, true, false);
    let expect = '[false,true,false]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayByte() {
    let arr = Array.of<Byte>(Byte.MIN_VALUE, -1 as byte, 0 as byte, 1 as byte, Byte.MAX_VALUE);
    let expect = '[-128,-1,0,1,127]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayChar() {
    let arr = Array.of<char>(c'0', c'1', c'a', c'z', c'A', c'Z');
    let expect = '["0","1","a","z","A","Z"]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayInt() {
    let arr = Array.of<int>(Int.MIN_VALUE, -1, 0, 1, Int.MAX_VALUE);
    let expect = '[-2147483648,-1,0,1,2147483647]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayLong() {
    let arr = Array.of<long>(Long.MIN_VALUE, -1 as long, 0 as long, 1 as long, Long.MAX_VALUE);
    let expect = '[-9223372036854775808,-1,0,1,9223372036854775807]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayFloat() {
    let arr = Array.of<float>(Float.MIN_VALUE, -1 as float, 0 as float, 1 as float, Float.MAX_VALUE,
                              Float.MAX_SAFE_INTEGER, Float.NaN, Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY);
    let expect = '[1e-45,-1,0,1,3.4028235e+38,16777215,null,null,null]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayDouble() {
    let arr = Array.of<double>(Double.MIN_VALUE, -1 as double, 0 as double, 1 as double, Double.MAX_VALUE,
                               Double.MAX_SAFE_INTEGER, Double.NaN, Double.POSITIVE_INFINITY, Double.NEGATIVE_INFINITY);
    let expect = '[5e-324,-1,0,1,1.7976931348623157e+308,9007199254740991,null,null,null]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

class TestClass {
    param1: int = 0;
    param2: string = 'test';
}

function jsonStringifyArrayClass() {
    let arr = Array.of<TestClass>(new TestClass(), new TestClass(), new TestClass());
    let expect = '[{"param1":0,"param2":"test"},{"param1":0,"param2":"test"},{"param1":0,"param2":"test"}]';
    let ret = JSON.stringify(arr);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArraySpace() {
    let arr = Array.of<double>(1, 2, 3, 4);
    let space: string | int = 1;
    let expect = '[\n 1,\n 2,\n 3,\n 4\n]';
    let ret = JSON.stringify(arr, undefined, space);
    arktest.assertEQ(ret, expect);

    space = ' ';
    ret = JSON.stringify(arr, undefined, space);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayReplacer1() {
    let arr = Array.of<double>(1, 2, 3, 4);
    let replacer: FixedArray<string | double> = [2, 4];
    let expect = '[1,2,3,4]';
    let ret = JSON.stringify(arr, replacer);
    arktest.assertEQ(ret, expect);

    replacer = ['1', '2'];
    ret = JSON.stringify(arr, replacer);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayReplacer2() {
    let arr = Array.of<int>(1, 2, 3, 4);
    let replacer: Array<string | number> = [2, 4];
    let expect = '[1,2,3,4]';
    let ret = JSON.stringify(arr, replacer);
    arktest.assertEQ(ret, expect);

    replacer = ['1', '2'];
    ret = JSON.stringify(arr, replacer);
    arktest.assertEQ(ret, expect);
}

function jsonStringifyArrayReplacer3() {
    let arr = Array.of<TestClass>(new TestClass(), new TestClass());
    let replacer: Array<string | number> = ['param2', 0];
    let expect = '[{"param2":"test"},{"param2":"test"}]';
    let ret = JSON.stringify(arr, replacer);
    arktest.assertEQ(ret, expect);
}

function replaceNumber(key: string, value: NullishType): NullishType {
    if (key == '1' || key == '4') {
        return key + '_' + value;
    } else {
        return value;
    }
}

function jsonStringifyArrayReplacer4() {
    let arr = Array.of<int>(1, 2, 3, 4);
    let expect = '[1,"1_2",3,4]';
    let ret = JSON.stringify(arr, replaceNumber);
    arktest.assertEQ(ret, expect);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Json stringify for Array tests');
    suite.addTest('for boolean', jsonStringifyArrayBoolean);
    suite.addTest('for byte', jsonStringifyArrayByte);
    suite.addTest('for char', jsonStringifyArrayChar);
    suite.addTest('for int', jsonStringifyArrayInt);
    suite.addTest('for long', jsonStringifyArrayLong);
    suite.addTest('for float', jsonStringifyArrayFloat);
    suite.addTest('for double', jsonStringifyArrayDouble);
    suite.addTest('for class', jsonStringifyArrayClass);
    suite.addTest('for space', jsonStringifyArraySpace);
    suite.addTest('for replacer 1', jsonStringifyArrayReplacer1);
    suite.addTest('for replacer 2', jsonStringifyArrayReplacer2);
    suite.addTest('for replacer 3', jsonStringifyArrayReplacer3);
    suite.addTest('for replacer 4', jsonStringifyArrayReplacer4);
    return suite.run();
}
