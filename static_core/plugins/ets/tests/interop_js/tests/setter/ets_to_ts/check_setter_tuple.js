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
    TupleSet,
    num,
    string,
} = require("setter.test.js");

function check_setter_tuple() {
    const tupleSet = new TupleSet();
    const arr = [num, string];
    tupleSet.value = arr;

    ASSERT_EQ(arr, tupleSet.value);
}

function check_setter_tuple_class_from_ets() {
    const tupleSet = create_tuple_set_class();
    const arr = [num, string];
    tupleSet.value = arr;

    ASSERT_EQ(arr, tupleSet.value);
}

check_setter_tuple();
check_setter_tuple_class_from_ets();
