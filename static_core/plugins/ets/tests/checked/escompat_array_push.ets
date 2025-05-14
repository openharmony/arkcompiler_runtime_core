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

//! CHECKER       escompat Array push
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, entry: "escompat_array_push.ETSGLOBAL::test_array_push"
//! METHOD        "escompat_array_push.ETSGLOBAL::test_array_push"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::push/
//! INST_COUNT    /CallVirtual.*escompat.Array::push/, 4

function test_array_push() {
    let array_size: int = 3;
    let array1: Array<number> = new Array<number>(array_size);

    array1.push(-1);
    arktest.assertEQ(array1[array1.length - 1], -1);

    array1.push(0);
    arktest.assertEQ(array1[array1.length - 1], 0);

    array1.push(1);
    arktest.assertEQ(array1[array1.length - 1], 1);

    array1.push(2);
    arktest.assertEQ(array1[array1.length - 1], 2);
    arktest.assertEQ(array1.length, 7);
}

//! CHECKER       escompat Array push2
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, entry: "escompat_array_push.ETSGLOBAL::test_array_push_managed_objects"
//! METHOD        "escompat_array_push.ETSGLOBAL::test_array_push_managed_objects"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::push/
//! INST_COUNT    /CallVirtual.*escompat.Array::push/, 5
function test_array_push_managed_objects() {
    let array: Array<Object> = new Array<Object>();

    // test number
    let obj1 = 123;
    array.push(obj1);
    arktest.assertEQ(array[array.length - 1], obj1);

    // test array
    let arr = [1, 2, 3];
    array.push(arr);
    arktest.assertEQ(array[array.length - 1], arr);

    // test string
    let str = "hello";
    array.push(str);
    arktest.assertEQ(array[array.length - 1], str);
    arktest.assertEQ(array.length, 3);

    let array1: Array<null> = new Array<null>();
    // test null and undefined
    array1.push(null);
    arktest.assertEQ(array1[array1.length - 1], null);

    let array2: Array<undefined> = new Array<undefined>();
    array2.push(undefined);
    arktest.assertEQ(array2[array2.length - 1], undefined);
}

//! CHECKER       escompat Array push3
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, entry: "escompat_array_push.ETSGLOBAL::test_array_push_unions"
//! METHOD        "escompat_array_push.ETSGLOBAL::test_array_push_unions"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*escompat.Array::push/
//! INST_COUNT    /CallVirtual.*escompat.Array::push/, 5
function test_array_push_unions() {
    let array: Array<Object | null | undefined | number | string> =
        new Array<Object | null | undefined | number | string>();
    // test number
    array.push(123);
    arktest.assertEQ(array[array.length - 1], 123);

    // test string
    array.push("abc");
    arktest.assertEQ(array[array.length - 1], "abc");

    // test object
    let obj: Object = new Object()
    array.push(obj);
    arktest.assertEQ(array[array.length - 1], obj);

    // test null
    array.push(null);
    arktest.assertEQ(array[array.length - 1], null);

    // test undefined
    array.push(undefined);
    arktest.assertEQ(array[array.length - 1], undefined);

    arktest.assertEQ(array.length, 5);
}
