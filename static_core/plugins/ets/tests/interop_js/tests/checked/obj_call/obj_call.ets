/**
 * Copyright (c) 2019-2023 Huawei Device Co., Ltd.
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

import { get_ret_obj }
    from "/plugins/ets/tests/interop_js/tests/checked/obj_call/obj_call.js"

class EtsClass {
    int_value: int;
    constructor(value: int) {
        this.int_value = value;
    }
}
class $jscall {
    static _init = JSRuntime.__initJSCallClass("L$jscall;");

    native static get_ret_obj(top: JSValue, qname: JSValue, obj:EtsClass): EtsClass;
};


//! CHECKER      test
//! RUN          force_jit: true, entry: "test", options: "--compiler-regex=ETSGLOBAL::test"

function test() : int {
    let obj: EtsClass = new EtsClass(0);
    obj = $jscall.get_ret_obj(get_ret_obj, JSRuntime.getUndefined(), obj);
    if (obj.int_value != 5) {
       return 1;
    }
    return 0;
}
