/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

const {
    num,
    string,
    bool,
    arr,
    obj,
    isObject,
    AnyTypeMethodClass,
    create_interface_class_any_type_method,
} = require('interface_method.test.js');

function check_any_type_method_class_interface_int() {
    const IClass = new AnyTypeMethodClass();

    ASSERT_TRUE(IClass.get(num) === num);
}

function check_any_type_method_class_interface_string() {
    const IClass = new AnyTypeMethodClass();

    ASSERT_TRUE(IClass.get(string) === string);
}

function check_any_type_method_class_interface_bool() {
    const IClass = new AnyTypeMethodClass();

    ASSERT_TRUE(IClass.get(bool) === bool);
}

function check_any_type_method_class_interface_arr() {
    const IClass = new AnyTypeMethodClass();

    ASSERT_TRUE(Array.isArray(IClass.get(arr)));
}

function check_any_type_method_class_interface_object() {
    const IClass = new AnyTypeMethodClass();

    ASSERT_TRUE(isObject(IClass.get(obj)));
}

function check_interface_object_from_ets_int() {
    const IClass = new create_interface_class_any_type_method();

    ASSERT_TRUE(IClass.get(num) === num);
}

function check_interface_object_from_ets_string() {
    const IClass = new create_interface_class_any_type_method();

    ASSERT_TRUE(IClass.get(string) === string);
}

function check_interface_object_from_ets_bool() {
    const IClass = new create_interface_class_any_type_method();

    ASSERT_TRUE(IClass.get(bool) === bool);
}

function check_interface_object_from_ets_arr() {
    const IClass = new create_interface_class_any_type_method();

    ASSERT_TRUE(Array.isArray(IClass.get(arr)));
}

function check_interface_object_from_ets_object() {
    const IClass = new create_interface_class_any_type_method();

    ASSERT_TRUE(isObject(IClass.get(obj)));
}

check_any_type_method_class_interface_int();
check_any_type_method_class_interface_string();
check_any_type_method_class_interface_bool();
check_any_type_method_class_interface_arr();
check_any_type_method_class_interface_object();
check_interface_object_from_ets_int();
check_interface_object_from_ets_string();
check_interface_object_from_ets_bool();
check_interface_object_from_ets_arr();
check_interface_object_from_ets_object();
