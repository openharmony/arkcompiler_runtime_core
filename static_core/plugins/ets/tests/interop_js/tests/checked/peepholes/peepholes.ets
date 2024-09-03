/*
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

import { cons, car, cdr, sum, TreeNode, DynObject, Empty, make_swappable, StaticClass, extract_squared_int, ObjectWithPrototype, vundefined, dyn_storage }
    from "/plugins/ets/tests/interop_js/tests/checked/js_call/js_call.js"

//! CHECKER         Fuse getProperty and cast to string in BCO
//! RUN_BCO         method: "ETSGLOBAL.js_load_and_cast_str"
//! METHOD          "js_load_and_cast_str"
//! PASS_AFTER      "IrBuilder"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 1
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyString"
//! BC_METHOD       "ETSGLOBAL.js_load_and_cast_str"
//! INST_COUNT      /call.* std.interop.js.JSRuntime.getPropertyJSValue:std.interop.js.JSValue;std.core.String;std.interop.js.JSValue;/, 1
//! INST_COUNT      /call.* std.interop.js.JSRuntime.getPropertyString:std.interop.js.JSValue;std.core.String;std.core.String;/, 1
//! RUN             entry: "js_load_and_cast_str"

function js_load_and_cast_str(): int {
    let v = dyn_storage.str;
    if (v as String != "abcd") {
        return 1;
    }
    return 0;
}

//! CHECKER         Fuse getProperty and cast to double in BCO
//! RUN_BCO         method: "ETSGLOBAL.js_load_and_cast_dbl"
//! METHOD          "js_load_and_cast_dbl"
//! PASS_AFTER      "IrBuilder"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 1
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyDouble"
//! BC_METHOD       "ETSGLOBAL.js_load_and_cast_dbl"
//! INST_COUNT      /call.*JSRuntime.getPropertyJSValue:std.interop.js.JSValue;std.core.String;std.interop.js.JSValue;/, 1
//! INST_COUNT      /call.*JSRuntime.getPropertyDouble:std.interop.js.JSValue;std.core.String;f64;/, 1
//! RUN             entry: "js_load_and_cast_dbl"
//! RUN             entry: "js_load_and_cast_int_to_dbl"

function js_load_and_cast_dbl(): int {
    let v = dyn_storage.dbl;
    if (v as double != 1.9) {
        return 1;
    }
    return 0;
}

function js_load_and_cast_int_to_dbl(): int {
    let v = dyn_storage.integer;
    if (v as double != 6) {
        return 1;
    }
    return 0;
}

//! CHECKER         Fuse getProperty and cast to int in BCO
//! RUN_BCO         method: "ETSGLOBAL.js_load_and_cast_int"
//! METHOD          "js_load_and_cast_int"
//! PASS_AFTER      "IrBuilder"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! INST            "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT       /i32.*Cast f64/
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 1
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyDouble"
//! BC_METHOD       "ETSGLOBAL.js_load_and_cast_int"
//! INST_COUNT      /call.*JSRuntime.getPropertyJSValue:std.interop.js.JSValue;std.core.String;std.interop.js.JSValue;/, 1
//! INST_COUNT      /call.*JSRuntime.getPropertyDouble:std.interop.js.JSValue;std.core.String;f64;/, 1
//! RUN             entry: "js_load_and_cast_int"

function js_load_and_cast_int(): int {
    let v = dyn_storage.integer;
    if (v as int != 6) {
        return 1;
    }
    return 0;
}

//! CHECKER         Fuse getProperty and cast to boolean in BCO
//! RUN_BCO         method: "ETSGLOBAL.js_load_and_cast_bool"
//! METHOD          "js_load_and_cast_bool"
//! PASS_AFTER      "IrBuilder"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 4
//! INST_COUNT      "Intrinsic.JSRuntimeGetValueBoolean", 2
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyBoolean"
//! BC_METHOD       "ETSGLOBAL.js_load_and_cast_bool"
//! INST_COUNT      /call.*JSRuntime.getPropertyJSValue:std.interop.js.JSValue;std.core.String;std.interop.js.JSValue;/, 2
//! INST_COUNT      /call.*JSRuntime.getPropertyBoolean:std.interop.js.JSValue;std.core.String;u1;/, 2
//! RUN             entry: "js_load_and_cast_bool"

function js_load_and_cast_bool(): int {
    let bool_false = dyn_storage.bool_false as boolean;
    let bool_true = dyn_storage.bool_true as boolean;
    if (bool_false || !bool_true) {
        return 1;
    }
    return 0;
}

//! CHECKER         Fuse cast from string to JSValue and setProperty in BCO
//! RUN_BCO         method: "ETSGLOBAL.js_store_str"
//! METHOD          "js_store_str"
//! PASS_AFTER      "IrBuilder"
//! INST            "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT       "Intrinsic.JSRuntimeNewJSValueString"
//! INST_NEXT       "Intrinsic.JSRuntimeSetPropertyJSValue"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT       "Intrinsic.JSRuntimeSetPropertyString"
//! BC_METHOD       "ETSGLOBAL.js_store_str"
//! INST_COUNT      /call.*JSRuntime.getPropertyJSValue:std.interop.js.JSValue;std.core.String;std.interop.js.JSValue;/, 1
//! INST_COUNT      /call.*JSRuntime.setPropertyString:std.interop.js.JSValue;std.core.String;std.core.String;/, 1
//! INST_NOT        /JSRuntime.setPropertyJSValue/
//! RUN             entry: "check_js_store"

function js_store_str() {
    dyn_storage.str = "dcba";
}

function js_store_dbl() {
    dyn_storage.dbl = 2.4;
}

function js_store_int() {
    dyn_storage.integer = 31;
}

function js_store_bool() {
    dyn_storage.bool_true = false;
}

function check_js_store(): int {
    js_store_str();
    js_store_dbl();
    js_store_int();
    js_store_bool();
    if (dyn_storage.verify() as int != 4) {
        return 1;
    }
    return 0;
}
