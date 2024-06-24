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
    string,
    number,
    UnionTypeClass,
    create_union_type_getter_class_from_ets,
} = require("getter.test.js");

function check_union_type_getter_class_value_string() {
    const GClass = new UnionTypeClass(string);

    ASSERT_TRUE(GClass._value === string);
}

function check_union_type_getter_class_value_number() {
    const GClass = new UnionTypeClass(number);

    ASSERT_TRUE(GClass._value === number);
}

function check_create_union_type_getter_class_from_ets_value_string() {
    const GClass = create_union_type_getter_class_from_ets(string);

    ASSERT_TRUE(GClass._value === string);
}

function check_create_union_type_getter_class_from_ets_value_number() {
    const GClass = create_union_type_getter_class_from_ets(number);

    ASSERT_TRUE(GClass._value === number);
}

function check_union_type_getter_class_string() {
    const GClass = new UnionTypeClass(string);

    ASSERT_TRUE(GClass.value === string);
}


function check_union_type_getter_class_number() {
    const GClass = new UnionTypeClass(number);

    ASSERT_TRUE(GClass.value === number);
}

function check_create_union_type_getter_class_from_ets_string() {
    const GClass = create_union_type_getter_class_from_ets(string);

    ASSERT_TRUE(GClass.value === string);
}

function check_create_union_type_getter_class_from_ets_number() {
    const GClass = create_union_type_getter_class_from_ets(number);

    ASSERT_TRUE(GClass.value === number);
}

check_union_type_getter_class_value_string();
check_union_type_getter_class_value_number();
check_create_union_type_getter_class_from_ets_value_string();
check_create_union_type_getter_class_from_ets_value_number();
check_union_type_getter_class_string();
check_union_type_getter_class_number();
check_create_union_type_getter_class_from_ets_string();
check_create_union_type_getter_class_from_ets_number();
