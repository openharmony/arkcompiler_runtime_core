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
    SubsetRefSet,
    create_subset_ref_set_class,
    string,
} = require("setter.test.js");

function check_setter_subset_ref() {
    const extraSet = new SubsetRefSet();
    extraSet.value = string

    ASSERT_EQ(typeof string, typeof extraSet.value);
}

function check_setter_subset_ref_class_from_ets() {
    const extraSet = create_subset_ref_set_class();
    extraSet.value = string

    ASSERT_EQ(typeof string, typeof extraSet.value);
}


check_setter_subset_ref();
check_setter_subset_ref_class_from_ets();
