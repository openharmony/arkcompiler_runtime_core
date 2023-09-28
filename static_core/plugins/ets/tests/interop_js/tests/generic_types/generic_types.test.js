/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

const etsVm = require("lib/module/ets_interop_js_napi");

let EtsGenericValueHandle = etsVm.getClass("Lgeneric_types/test/EtsGenericValueHandle;");
let EtsGenericErrorHandle = etsVm.getClass("Lgeneric_types/test/EtsGenericErrorHandle;");
let ets_get_generic_value_identity = etsVm.getFunction("Lgeneric_types/test/ETSGLOBAL;", "ets_get_generic_value_identity");
let ets_get_generic_error_identity = etsVm.getFunction("Lgeneric_types/test/ETSGLOBAL;", "ets_get_generic_error_identity");


function check_generic_value_for_function(v) {
    // Check the parameter and the return value of the function
    let gen_value = ets_get_generic_value_identity(v);
    ASSERT_EQ(v, gen_value);
}

function check_generic_value_for_instance(v) {
    let gen_handle = new EtsGenericValueHandle(v);

    // Check the parameter and the return value of the class method
    let value = gen_handle.get_value();
    ASSERT_EQ(v, value);

    gen_handle.set_value(new ets.Object());

    // Check access to generic field
    gen_handle.value = v;
    let field_value = gen_handle.value;
    ASSERT_EQ(v, field_value);
}

function check_generic_value(v) {
    check_generic_value_for_function(v);
    check_generic_value_for_instance(v);
}


module.exports = {
    EtsGenericValueHandle,
    EtsGenericErrorHandle,
    check_generic_value,
    ets_get_generic_error_identity,
}
