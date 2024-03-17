/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
    EtsGenericErrorHandle,
    check_generic_value,
    ets_get_generic_error_identity,
} = require("generic_types.test.js");


function check_error_message(err, err_msg) {
    check_generic_value(err);

    let ets_error_handle = new EtsGenericErrorHandle(err);
    let msg = ets_error_handle.get_error_message();

    ASSERT_EQ(msg, err_msg);

    let gen_error = ets_get_generic_error_identity(err);
    ASSERT_EQ(err, gen_error);
}

function check_incorrect_js_object() {
    let err_msg = "incorrect js object"
    let obj = {message: err_msg};

    ASSERT_THROWS(TypeError, () => (check_error_message(obj, err_msg)));
    ASSERT_THROWS(TypeError, () => (ets_get_generic_error_identity(obj)));
}

function check_incorrect_ets_object() {
    let err_msg = "incorrect ets object"
    let obj = new ets.Object();

    ASSERT_THROWS(TypeError, () => (check_error_message(obj, err_msg)));
    ASSERT_THROWS(TypeError, () => (ets_get_generic_error_identity(obj)));
}


// TODO(v.cherkashin): Enable when implemented
// Check js/ets errors
check_error_message(new ets.Error("ets Error message", undefined), "ets Error message");
// check_error_message(new Error("js Error message"), "js Error message");
// check_error_message(new ets.TypeError("ets TypeError message"), "ets TypeError message");
// check_error_message(new TypeError("js TypeError message"), "js TypeError message");

check_incorrect_ets_object()
check_incorrect_js_object()
