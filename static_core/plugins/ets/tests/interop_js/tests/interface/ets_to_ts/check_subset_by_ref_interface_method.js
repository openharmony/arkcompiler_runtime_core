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
    SubsetByRefClass,
    subset_by_ref_interface,
    call_subset_by_ref_interface_from_ets,
} = require('interface_method.test.js');

function check_subset_by_ref_interface_method() {
    const obj = new SubsetByRefClass();
    ASSERT_TRUE(subset_by_ref_interface(obj) === num);
}

function check_call_subset_by_ref_interface_from_ets() {
    ASSERT_TRUE(call_subset_by_ref_interface_from_ets() === num);
}

check_subset_by_ref_interface_method();
check_call_subset_by_ref_interface_from_ets();
