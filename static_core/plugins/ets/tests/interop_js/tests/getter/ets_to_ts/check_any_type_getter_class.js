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
    check_array,
    check_obj,
    string,
    number,
    bool,
    arr,
    obj,
    tuple,
    AnyTypeClass,
    create_any_type_getter_class_from_ets_string,
    create_any_type_getter_class_from_ets_int,
    create_any_type_getter_class_from_ets_bool,
    create_any_type_getter_class_from_ets_arr,
    create_any_type_getter_class_from_ets_obj,
    create_any_type_getter_class_from_ets_tuple,
    create_any_type_getter_class_from_ets_union,
} = require("getter.test.js");

function check_class_getter_class_value_int() {
    const GClass = new AnyTypeClass(number);

    ASSERT_TRUE(GClass._value === number);
}

function check_class_getter_class_value_string() {
    const GClass = new AnyTypeClass(string);

    ASSERT_TRUE(GClass._value === string);
}

function check_class_getter_class_value_bool() {
    const GClass = new AnyTypeClass(bool);

    ASSERT_TRUE(GClass._value === bool);
}

function check_class_getter_class_value_arr() {
    const GClass = new AnyTypeClass(arr);

    ASSERT_TRUE(check_array(GClass._value) && GClass._value[0] === arr[0]);
}

function check_class_getter_class_value_obj() {
    const GClass = new AnyTypeClass(obj);

    ASSERT_TRUE(check_obj(GClass._value));
}

function check_class_getter_class_value_tuple() {
    const GClass = new AnyTypeClass(tuple);

    const res = GClass._value;

    ASSERT_TRUE(check_array(res) && res[0] === tuple[0] && res[1] === tuple[1]);
}

function check_create_any_type_getter_class_value_from_ets_string() {
    const GClass = create_any_type_getter_class_from_ets_string(string);

    ASSERT_TRUE(GClass._value === string);
}

function check_create_any_type_getter_class_value_from_ets_int() {
    const GClass = create_any_type_getter_class_from_ets_int(number);

    ASSERT_TRUE(GClass._value === number);
}

function check_create_any_type_getter_class_value_from_ets_bool() {
    const GClass = create_any_type_getter_class_from_ets_bool(bool);

    ASSERT_TRUE(GClass._value === bool);
}

function check_create_any_type_getter_class_value_from_ets_arr() {
    const GClass = create_any_type_getter_class_from_ets_arr(arr);

    ASSERT_TRUE(check_array(GClass._value) && GClass._value[0] === arr[0]);
}

function check_create_any_type_getter_class_value_from_ets_obj() {
    const GClass = create_any_type_getter_class_from_ets_obj(obj);

    ASSERT_TRUE(check_obj(GClass._value));
}

function check_create_any_type_getter_class_value_from_ets_tuple() {
    const GClass = create_any_type_getter_class_from_ets_tuple(tuple);

    const res = GClass._value;

    ASSERT_TRUE(check_array(res) && res[0] === tuple[0] && res[1] === tuple[1]);
}

function check_create_any_type_getter_class_value_from_ets_union() {
    const GClass = create_any_type_getter_class_from_ets_union(number);

    ASSERT_TRUE(GClass._value === number);
}

function check_class_getter_class_int() {
    const GClass = new AnyTypeClass(number);

    ASSERT_TRUE(GClass.value === number);
}

function check_class_getter_class_string() {
    const GClass = new AnyTypeClass(string);

    ASSERT_TRUE(GClass.value === string);
}

function check_class_getter_class_bool() {
    const GClass = new AnyTypeClass(bool);

    ASSERT_TRUE(GClass.value === bool);
}

function check_class_getter_class_arr() {
    const GClass = new AnyTypeClass(arr);

    ASSERT_TRUE(check_array(GClass.value) && GClass.value[0] === arr[0]);
}

function check_class_getter_class_obj() {
    const GClass = new AnyTypeClass(obj);

    ASSERT_TRUE(check_obj(GClass.value));
}

function check_class_getter_class_tuple() {
    const GClass = new AnyTypeClass(tuple);

    const res = GClass.value;

    ASSERT_TRUE(check_array(res) && res[0] === tuple[0] && res[1] === tuple[1]);
}

function check_create_any_type_getter_class_from_ets_string() {
    const GClass = create_any_type_getter_class_from_ets_string(string);

    ASSERT_TRUE(GClass.value === string);
}

function check_create_any_type_getter_class_from_ets_int() {
    const GClass = create_any_type_getter_class_from_ets_int(number);

    ASSERT_TRUE(GClass.value === number);
}

function check_create_any_type_getter_class_from_ets_bool() {
    const GClass = create_any_type_getter_class_from_ets_bool(bool);

    ASSERT_TRUE(GClass.value === bool);
}

function check_create_any_type_getter_class_from_ets_arr() {
    const GClass = create_any_type_getter_class_from_ets_arr(arr);

    ASSERT_TRUE(check_array(GClass.value) && GClass.value[0] === arr[0]);
}

function check_create_any_type_getter_class_from_ets_obj() {
    const GClass = create_any_type_getter_class_from_ets_obj(obj);

    ASSERT_TRUE(check_obj(GClass.value));
}

function check_create_any_type_getter_class_from_ets_tuple() {
    const GClass = create_any_type_getter_class_from_ets_tuple(tuple);

    const res = GClass.value;

    ASSERT_TRUE(check_array(res) && res[0] === tuple[0] && res[1] === tuple[1]);
}

function check_create_any_type_getter_class_from_ets_union() {
    const GClass = create_any_type_getter_class_from_ets_union(number);

    ASSERT_TRUE(GClass.value === number);
}

check_class_getter_class_value_int();
check_class_getter_class_value_string();
check_class_getter_class_value_bool();
check_class_getter_class_value_arr();
check_class_getter_class_value_obj();
check_class_getter_class_value_tuple();
check_create_any_type_getter_class_value_from_ets_string();
check_create_any_type_getter_class_value_from_ets_int();
check_create_any_type_getter_class_value_from_ets_bool();
check_create_any_type_getter_class_value_from_ets_arr();
check_create_any_type_getter_class_value_from_ets_obj();
check_create_any_type_getter_class_value_from_ets_tuple();
check_create_any_type_getter_class_value_from_ets_union();
check_class_getter_class_int();
check_class_getter_class_string();
check_class_getter_class_bool();
check_class_getter_class_arr();
check_class_getter_class_obj();
check_class_getter_class_tuple();
check_create_any_type_getter_class_from_ets_string();
check_create_any_type_getter_class_from_ets_int();
check_create_any_type_getter_class_from_ets_bool();
check_create_any_type_getter_class_from_ets_arr();
check_create_any_type_getter_class_from_ets_obj();
check_create_any_type_getter_class_from_ets_tuple();
check_create_any_type_getter_class_from_ets_union();
