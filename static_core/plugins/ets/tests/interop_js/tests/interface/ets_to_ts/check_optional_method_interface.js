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
    WithOptionalMethodClass,
    create_class_with_optional_method,
    WithoutOptionalMethodClass,
    create_class_without_optional_method,
    optional_arg,
    optional_arg_array,
} = require('interface_method.test.js');

function check_with_optional_method_class() {
    const optionalClass = new WithOptionalMethodClass();

    ASSERT_TRUE(optionalClass.getNum() == num
        && optionalClass.getStr() == string);
}
function check_without_optional_method_class() {
    const optionalClass = new WithoutOptionalMethodClass();

    ASSERT_TRUE(optionalClass.getStr() == string);
}

function check_create_class_with_optional_method() {
    const optionalClass = create_class_with_optional_method();

    ASSERT_TRUE(optionalClass.getNum() == num
        && optionalClass.getStr() == string);
}
function check_create_class_without_optional_method() {
    const optionalClass = create_class_without_optional_method();

    ASSERT_TRUE(optionalClass.getStr() == string);
}

function check_with_optional_method_instance_class() {
    ASSERT_TRUE(withOptionalMethodInstanceClass.getNum() == ts_number
        && withOptionalMethodInstanceClass.getStr() == ts_string);
}

function check_without_optional_method_instance_class() {
    ASSERT_TRUE(withoutOptionalMethodInstanceClass.getStr() == ts_string);
}

function check_optional_arg_with_all_args() {
    const result = optional_arg(withOptionalMethodInstanceClass, withoutOptionalMethodInstanceClass);
    ASSERT_TRUE(!!result.with && !!result.without);
}

function check_optional_arg_with_one_args() {
    const result = optional_arg(withOptionalMethodInstanceClass);

    ASSERT_TRUE(!!result.with);
}

function check_spread_operator_arg_with_all_args() {
    const arr = [withOptionalMethodInstanceClass, withoutOptionalMethodInstanceClass];
    const result = optional_arg_array(...arr);
    ASSERT_TRUE(!!result.with && !!result.without);
}

function check_spread_operator_arg_with_one_args() {
    const arr = [withOptionalMethodInstanceClass];
    const result = optional_arg_array(...arr);
    ASSERT_TRUE(!!result.with);
}
