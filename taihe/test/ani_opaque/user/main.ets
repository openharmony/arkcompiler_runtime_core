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
import {BusinessError} from "@ohos.base";
import * as test from "opaque_test";

loadLibrary("ani_opaque");

class A {
    constructor() {}
}

function test_is_string_with_object() {
    const a = new A();
    const result = test.is_string(a);
    assertEQ(result, false)
}

function test_is_string_with_string() {
    const result = test.is_string("hello");
    assertEQ(result, true)
}

function test_get_object() {
    const obj = test.get_object();
    assertEQ(obj, "BBB")
}

function test_is_object_with_union() {
    const a: test.Person = {name: "aaa"};
    assertEQ(test.is_opaque(a), true);
    assertEQ(test.is_opaque("bbb"), false);
}

function test_get_objects() {
    const objs = test.get_objects();
    assertEQ(objs[0], "AAA");
    assertEQ(objs[1], undefined);
}

function main() {
    console.log("##############start#############");
    const suite = new ArkTestsuite("Test Suite");
    const objs = test.get_objects();
    console.log(objs);

    suite.addTest("test is_string with object", test_is_string_with_object);
    suite.addTest("test is_string with string", test_is_string_with_string);
    suite.addTest("test is_string with union", test_is_object_with_union);
    suite.addTest("test get_object", test_get_object);
    suite.addTest("test get_objects", test_get_objects);

    exit(suite.run());
}
