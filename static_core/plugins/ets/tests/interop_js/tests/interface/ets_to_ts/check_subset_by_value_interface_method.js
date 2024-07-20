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
    SubsetByValueClass,
    create_subset_by_value_class_from_ets,
} = require('interface_method.test');

function check_subset_by_value_interface_method() {
    const IClass = new SubsetByValueClass();

    ASSERT_TRUE(IClass.get().value === num);
}

function check_create_subset_by_value_class_from_ets() {
    const IClass = create_subset_by_value_class_from_ets();

    ASSERT_TRUE(IClass.get().value === num);
}



check_subset_by_value_interface_method();
check_create_subset_by_value_class_from_ets();
