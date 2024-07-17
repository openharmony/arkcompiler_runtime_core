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
    ProtectedGetterInheritanceClass,
    create_protected_getter_inheritance_class_from_ets,
} = require("getter.test.js");

function check_protected_getter_inheritance_class_value() {
    const GClass = new ProtectedGetterInheritanceClass();

    ASSERT_TRUE(GClass._value === undefined);
}

function check_protected_getter_inheritance_class() {
    const GClass = new ProtectedGetterInheritanceClass();

    ASSERT_TRUE(GClass.value === string);
}

function check_create_protected_getter_inheritance_class_from_ets() {
    const GClass = create_protected_getter_inheritance_class_from_ets();

    ASSERT_TRUE(GClass.value === string);
}

check_protected_getter_inheritance_class_value();
check_protected_getter_inheritance_class();
check_create_protected_getter_inheritance_class_from_ets();
