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

//! CHECKER       ArrayInt indexOf
//! RUN           force_jit: true , options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_int_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_int_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::indexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::indexOf/, 5
function test_array_int_index_of() {
    let array: Array<int> = [2, 3, 2, 3, 3, 4, 5];
    arktest.assertEQ(array.indexOf(3 ,  0),  1.);
    arktest.assertEQ(array.indexOf(3 ,  2),  3.);
    arktest.assertEQ(array.indexOf(3 ,  9), -1.);
    arktest.assertEQ(array.indexOf(3,  -3),  4.);
    arktest.assertEQ(array.indexOf(3,  -8),  1.);
}

//! CHECKER       ArrayNumber indexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_number_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_number_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::indexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::indexOf/, 5
function test_array_number_index_of() {
    let array: Array<number> = ([1.1, 2.2, 3.3, 3.3, 5.5, 2.2, 4.4]);
    arktest.assertEQ(array.indexOf(3 ,   0), -1.);
    arktest.assertEQ(array.indexOf(2.2 , 0),  1.);
    arktest.assertEQ(array.indexOf(2.2,  2),  5.);
    arktest.assertEQ(array.indexOf(2.2, -2),  5.);
    arktest.assertEQ(array.indexOf(2.2, -8),  1.);
}

//! CHECKER       ArrayShort indexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_short_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_short_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::indexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::indexOf/, 5
function test_array_short_index_of() {
    let array: Array<short> = [2, 3, 2, 3, 3, 4, 5];
    let searchElement: short = 3;
    arktest.assertEQ(array.indexOf(searchElement ,  0),  1.);
    arktest.assertEQ(array.indexOf(searchElement ,  2),  3.);
    arktest.assertEQ(array.indexOf(searchElement ,  9), -1.);
    arktest.assertEQ(array.indexOf(searchElement,  -3),  4.);
    arktest.assertEQ(array.indexOf(searchElement,  -8),  1.);
}

//! CHECKER       ArrayLong indexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_long_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_long_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::indexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::indexOf/, 5
function test_array_long_index_of() {
    let array: Array<long> = [2, 3, 2, 3, 3, 4, 5];
    let searchElement: long = 3;
    arktest.assertEQ(array.indexOf(searchElement ,  0),  1.);
    arktest.assertEQ(array.indexOf(searchElement ,  2),  3.);
    arktest.assertEQ(array.indexOf(searchElement ,  9), -1.);
    arktest.assertEQ(array.indexOf(searchElement , -3),  4.);
    arktest.assertEQ(array.indexOf(searchElement , -8),  1.);
}

//! CHECKER       ArrayFloat indexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_float_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_float_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::indexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::indexOf/, 4
function test_array_float_index_of() {
    let array: Array<float> = [1.1f, 2.2f, 3.3f, 3.3f, 5.5f, 2.2f, 4.4f];
    let searchElement : float = 2.2f;
    arktest.assertEQ(array.indexOf(searchElement,  0),  1.);
    arktest.assertEQ(array.indexOf(searchElement,  2),  5.);
    arktest.assertEQ(array.indexOf(searchElement, -2),  5.);
    arktest.assertEQ(array.indexOf(searchElement, -8),  1.);
}

//! CHECKER       ArrayChar indexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_char_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_char_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::indexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::indexOf/, 5
function test_array_char_index_of() {
    let array: Array<char> = [c'a', c'b', c'c', c'a', c'd', c'a', c'b'];
    let searchElement : char = c'b';
    arktest.assertEQ(array.indexOf(searchElement,   0),  1.);
    searchElement = c'a';
    arktest.assertEQ(array.indexOf(searchElement,   2),  3.);
    searchElement = c'd';
    arktest.assertEQ(array.indexOf(searchElement,   6), -1.);
    searchElement = c'a';
    arktest.assertEQ(array.indexOf(searchElement,  -1), -1.);
    searchElement = c'f';
    arktest.assertEQ(array.indexOf(searchElement,  3),  -1.);
}

//! CHECKER       ArrayBoolean indexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_boolean_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_boolean_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::indexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::indexOf/, 5
function test_array_boolean_index_of() {
    let array: Array<boolean> = [true, false, false, true, false, true, false];
    arktest.assertEQ(array.indexOf(false ,  0),  1.);
    arktest.assertEQ(array.indexOf(false ,  6),  6.);
    arktest.assertEQ(array.indexOf(true ,   8), -1.);
    arktest.assertEQ(array.indexOf(true,   -1), -1.);
    arktest.assertEQ(array.indexOf(false,  -5),  2.);
}

//! CHECKER       ArrayString indexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_string_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_string_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::indexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::indexOf/, 5
function test_array_string_index_of() {
    let array: Array<string> = ["this", "is", "hello", "world", "hello"];
    arktest.assertEQ(array.indexOf("hello",   0),  2.);
    arktest.assertEQ(array.indexOf("world",   2),  3.);
    arktest.assertEQ(array.indexOf("hello",   3),  4.);
    arktest.assertEQ(array.indexOf("this",   -1), -1.);
    arktest.assertEQ(array.indexOf("this",   -7),  0.);
}

//! CHECKER       ArrayInt lastIndexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_int_last_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_int_last_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::lastIndexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::lastIndexOf/, 5
function test_array_int_last_index_of() {
    let array: Array<int> = [2, 3, 2, 3, 3, 4, 5];
    arktest.assertEQ(array.lastIndexOf(3 ,  0),  -1.);
    arktest.assertEQ(array.lastIndexOf(3 ,  2),   1.);
    arktest.assertEQ(array.lastIndexOf(3 ,  9),   4.);
    arktest.assertEQ(array.lastIndexOf(3 , -3),   4.);
    arktest.assertEQ(array.lastIndexOf(3 , -8),  -1.);
}

//! CHECKER       ArrayNumber lastIndexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_number_last_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_number_last_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::lastIndexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::lastIndexOf/, 5
function test_array_number_last_index_of() {
    let array: Array<number> = ([1.1, 2.2, 3.3, 3.3, 5.5, 2.2, 4.4]);
    arktest.assertEQ(array.lastIndexOf(3.,   0), -1.);
    arktest.assertEQ(array.lastIndexOf(2.2 , 0), -1.);
    arktest.assertEQ(array.lastIndexOf(2.2,  2),  1.);
    arktest.assertEQ(array.lastIndexOf(2.2, -2),  5.);
    arktest.assertEQ(array.lastIndexOf(2.2, -8), -1.);
}

//! CHECKER       ArrayShort lastIndexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_short_last_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_short_last_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::lastIndexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::lastIndexOf/, 5
function test_array_short_last_index_of() {
    let array: Array<short> = [2, 3, 2, 3, 3, 4, 5];
    let searchElement : short = 3;
    arktest.assertEQ(array.lastIndexOf(searchElement ,  0),  -1.);
    arktest.assertEQ(array.lastIndexOf(searchElement ,  2),   1.);
    arktest.assertEQ(array.lastIndexOf(searchElement ,  9),   4.);
    arktest.assertEQ(array.lastIndexOf(searchElement , -3),   4.);
    arktest.assertEQ(array.lastIndexOf(searchElement , -8),  -1.);
}

//! CHECKER       ArrayLong lastIndexOf
//! RUN           force_jit: true , options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_long_last_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_long_last_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::lastIndexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::lastIndexOf/, 5
function test_array_long_last_index_of() {
    let array: Array<long> = [2, 3, 2, 3, 3, 4, 5];
    let searchElement : long = 3;
    arktest.assertEQ(array.lastIndexOf(searchElement ,  0),  -1.);
    arktest.assertEQ(array.lastIndexOf(searchElement ,  2),   1.);
    arktest.assertEQ(array.lastIndexOf(searchElement ,  9),   4.);
    arktest.assertEQ(array.lastIndexOf(searchElement , -3),   4.);
    arktest.assertEQ(array.lastIndexOf(searchElement , -8),  -1.);
}

//! CHECKER       ArrayFloat lastIndexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_float_last_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_float_last_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::lastIndexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::lastIndexOf/, 4
function test_array_float_last_index_of() {
    let array: Array<float> = [1.1f, 2.2f, 3.3f, 3.3f, 5.5f, 2.2f, 4.4f];
    let searchElement : float = 2.2f;
    arktest.assertEQ(array.lastIndexOf(searchElement , 0),  -1.);
    arktest.assertEQ(array.lastIndexOf(searchElement,  2),   1.);
    arktest.assertEQ(array.lastIndexOf(searchElement, -2),   5.);
    arktest.assertEQ(array.lastIndexOf(searchElement, -8),  -1.);
}

//! CHECKER       ArrayChar lastIndexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_char_last_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_char_last_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::lastIndexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::lastIndexOf/, 5
function test_array_char_last_index_of() {
    let array: Array<char> = [c'a', c'b', c'c', c'a', c'd', c'a', c'b'];
    let searchElement : char = c'b';
    arktest.assertEQ(array.lastIndexOf(searchElement,   0),  -1.);
    searchElement = c'a';
    arktest.assertEQ(array.lastIndexOf(searchElement,   2),   0.);
    searchElement = c'd';
    arktest.assertEQ(array.lastIndexOf(searchElement,   6),   4.);
    searchElement = c'a';
    arktest.assertEQ(array.lastIndexOf(searchElement,  -1),   5.);
    searchElement = c'f';
    arktest.assertEQ(array.lastIndexOf(searchElement,  3),   -1.);
}

//! CHECKER       ArrayBoolean lastIndexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_boolean_last_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_boolean_last_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::lastIndexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::lastIndexOf/, 5
function test_array_boolean_last_index_of() {
    let array: Array<boolean> = [true, false, false, true, false, true, false];
    arktest.assertEQ(array.lastIndexOf(false ,  0),  -1.);
    arktest.assertEQ(array.lastIndexOf(false ,  6),   6.);
    arktest.assertEQ(array.lastIndexOf(true ,   8),   5.);
    arktest.assertEQ(array.lastIndexOf(true,   -1),   5.);
    arktest.assertEQ(array.lastIndexOf(false,  -5),   2.);
}

//! CHECKER       ArrayString lastIndexOf
//! RUN           force_jit: true, options: "--compiler-dump:final", entry: "escompat_array_indexof.ETSGLOBAL::test_array_string_last_index_of"
//! METHOD        "escompat_array_indexof.ETSGLOBAL::test_array_string_last_index_of"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::lastIndexOf/
//! INST_COUNT    /CallVirtual.*escompat.Array::lastIndexOf/, 5
function test_array_string_last_index_of() {
    let array: Array<string> = ["this", "is", "hello", "world", "hello"];
    arktest.assertEQ(array.lastIndexOf("hello",   0),  -1.);
    arktest.assertEQ(array.lastIndexOf("world",   2),  -1.);
    arktest.assertEQ(array.lastIndexOf("hello",   3),   2.);
    arktest.assertEQ(array.lastIndexOf("this",   -1),   0.);
    arktest.assertEQ(array.lastIndexOf("this",   -7),  -1.);
}
