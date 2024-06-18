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
    LiteralValueMethodClass,
    create_interface_literal_value_class_from_ets,
} = require('interface_method.test.js');

function check_literal_type_method_class_interface_int() {
    const IClass = new LiteralValueMethodClass();

    ASSERT_TRUE(IClass.get(num) === num);
}

function check_literal_type_method_class_interface_string() {
    const IClass = new LiteralValueMethodClass();

    ASSERT_TRUE(IClass.get(string) === string);
}

function check_literal_interface_object_from_ets_int() {
    const IClass = new create_interface_literal_value_class_from_ets();

    ASSERT_TRUE(IClass.get(num) === num);
}

function check_literal_interface_object_from_ets_string() {
    const IClass = new create_interface_literal_value_class_from_ets();

    ASSERT_TRUE(IClass.get(string) === string);
}

function check_literal_value_type_method_class_interface_int() {
    const IClass = new LiteralValueMethodClass();

    ASSERT_TRUE(IClass.get(num) === num);
}

function check_literal_value_type_method_class_interface_string() {
    const IClass = new LiteralValueMethodClass();

    ASSERT_TRUE(IClass.get(string) === string);
}

function check_literal_value_interface_object_from_ets_int() {
    const IClass = new create_interface_literal_value_class_from_ets();

    ASSERT_TRUE(IClass.get(num) === num);
}

function check_literal_value_interface_object_from_ets_string() {
    const IClass = new create_interface_literal_value_class_from_ets();

    ASSERT_TRUE(IClass.get(string) === string);
}

check_literal_type_method_class_interface_int();
check_literal_type_method_class_interface_string();
check_literal_interface_object_from_ets_int();
check_literal_interface_object_from_ets_string();
check_literal_value_type_method_class_interface_int();
check_literal_value_type_method_class_interface_string();
check_literal_value_interface_object_from_ets_int();
check_literal_value_interface_object_from_ets_string();
