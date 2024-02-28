/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

import {
    EvaluateNumber,
    ExampleClass,
    ClassWithEmptyConstructor,
    EvaluateObject,
    MyNamespace,
    EvaluateArray
} from "/plugins/ets/tests/interop_js/tests/number_subtypes/module.js"

//! CHECKER      jscall_empty JIT
//! RUN          force_jit: true, entry: "jscall_empty", options: "--compiler-regex=ETSGLOBAL::jscall_empty"
//! METHOD       "jscall_empty"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jscall::invoke/

function jscall_empty(): int {
    EvaluateNumber();
    return 0;
}

//! CHECKER      jsnew_empty JIT
//! RUN          force_jit: true, entry: "jsnew_empty", options: "--compiler-regex=ETSGLOBAL::jsnew_empty"
//! METHOD       "jsnew_empty"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jsnew::invoke/
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyInt"
//! INST_NOT     /JSRuntime.*Value/ # no casts from JSValue to primitive
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 2
//! INST_COUNT   /i32.*Cast f64/, 2

function jsnew_empty(): int {
    let res_obj = new ClassWithEmptyConstructor();
    let res_get_v0 = res_obj.v0 as int;
    let res_get_v1 = res_obj.v1 as int;
    if (res_get_v0 == 42 && res_get_v1 == 42) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER      jscall_static_method_empty JIT
//! RUN          force_jit: true, entry: "jscall_static_method_empty", options: "--compiler-regex=ETSGLOBAL::jscall_static_method_empty"
//! METHOD       "jscall_static_method_empty"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jscall::invoke/

function jscall_static_method_empty(): int {
    ExampleClass.EmptyMethod();
    return 0;
}

//! CHECKER      jscall_object JIT
//! RUN          force_jit: true, entry: "jscall_object", options: "--compiler-regex=ETSGLOBAL::jscall_object"
//! METHOD       "jscall_object"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jsnew::invoke/
//! INST_NOT     /CallStatic.*jscall::invoke/
//! INST         "Intrinsic.JSRuntimeGetPropertyDouble"
//! INST_NEXT    /i32.*Cast f64/
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyDouble"
//! INST_NEXT    /i32.*Cast f64/

function jscall_object(): int {
    let res_obj = new ExampleClass(42, 42);
    let res = EvaluateObject(res_obj);
    let res_get_v0 = res.v0 as int;
    let res_get_v1 = res.v1 as int;
    if (res_get_v0 == 84 && res_get_v1 == 84) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER      jsnew_object JIT
//! RUN          force_jit: true, entry: "jsnew_object", options: "--compiler-regex=ETSGLOBAL::jsnew_object"
//! METHOD       "jsnew_object"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jsnew::invoke/
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 4

function jsnew_object(): int {
    let res_obj = new ExampleClass(42, 42);
    let res = new ExampleClass(res_obj, res_obj);
    let res_get_v0 = res.v0;
    let res_get_v1 = res.v1;
    if (res_get_v0.v0 as int == 42 && res_get_v0.v1 as int == 42 &&
        res_get_v1.v0 as int == 42 && res_get_v1.v1 as int == 42) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER      jsnew_setproperty_object JIT
//! RUN          force_jit: true, entry: "jsnew_setproperty_object", options: "--compiler-regex=ETSGLOBAL::jsnew_setproperty_object"
//! METHOD       "jsnew_setproperty_object"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jsnew::invoke/
//! INST_COUNT   "Intrinsic.JSRuntimeSetPropertyJSValue", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 4

function jsnew_setproperty_object(): int {
    let res_obj = new ClassWithEmptyConstructor();
    let res = new ClassWithEmptyConstructor();
    res_obj.v0 = res;
    res_obj.v1 = res;
    let res_get_v0 = res_obj.v0;
    let res_get_v1 = res_obj.v1;
    if (res_get_v0.v0 as int == 42 && res_get_v0.v1 as int == 42 &&
        res_get_v1.v0 as int == 42 && res_get_v1.v1 as int == 42) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER      jscall_method_object JIT
//! RUN          force_jit: true, entry: "jscall_method_object", options: "--compiler-regex=ETSGLOBAL::jscall_method_object"
//! METHOD       "jscall_method_object"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jsnew::invoke/
//! INST_NOT     /CallStatic.*jscall::invoke/
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 2

function jscall_method_object(): int {
    let res_obj = new ExampleClass(42, 42);
    let res = EvaluateObject(res_obj);
    let res_get_v0 = res.v0 as int;
    let res_get_v1 = res.v1 as int;
    if (res_get_v0 == 84 && res_get_v1 == 84) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER      jscall_method_simple JIT
//! RUN          force_jit: true, entry: "jscall_method_simple", options: "--compiler-regex=ETSGLOBAL::jscall_method_simple"
//! METHOD       "jscall_method_simple"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jsnew::invoke/
//! INST_NOT     /CallStatic.*jscall::invoke/

function jscall_method_simple(): int {
    let res_obj = new ExampleClass(42, 42);
    let res = res_obj.InstanceEvaluateNumber() as int;
    if (res == 84) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER      jscall_string JIT
//! RUN          force_jit: true, entry: "jscall_string", options: "--compiler-regex=ETSGLOBAL::jscall_string"
//! METHOD       "jscall_string"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jsnew::invoke/
//! INST_NOT     /CallStatic.*jscall::invoke/
//! INST         "Intrinsic.JSRuntimeGetValueString"
//! INST_NEXT    "Intrinsic.StdCoreStringEquals"

function jscall_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res = EvaluateNumber(v0, v1) as String;
    if (res == "helloworld") {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER      jsnew_string JIT
//! RUN          force_jit: true, entry: "jsnew_string", options: "--compiler-regex=ETSGLOBAL::jsnew_string"
//! METHOD       "jsnew_string"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jscall::invoke/
//! INST_NOT     /CallStatic.*jsnew::invoke/
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyString", 2
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NOT     "Intrinsic.JSRuntimeGetValueString"

function jsnew_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res_obj = new ExampleClass(v0, v1);
    let res_get_v0 = res_obj.v0 as String;
    let res_get_v1 = res_obj.v1 as String;
    if (res_get_v0 == v0 && res_get_v1 == v1) {
        return 0;
    } else {
        return 1;
    }
}

//! DISABLED_CHECKER      jsnew_setproperty_string JIT
//! RUN          force_jit: true, entry: "jsnew_setproperty_string", options: "--compiler-regex=ETSGLOBAL::jsnew_setproperty_string"
//! METHOD       "jsnew_setproperty_string"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jscall::invoke/
//! INST_NOT     /CallStatic.*jsnew::invoke/
//! INST_NOT     "Intrinsic.JSRuntimeSetPropertyJSValue"
//! INST_COUNT   "Intrinsic.JSRuntimeNewJSValueString", 2
//! INST_COUNT   "Intrinsic.JSRuntimeSetPropertyString", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyString", 2
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NOT     "Intrinsic.JSRuntimeGetValueString"
//! PASS_AFTER   "Cleanup"
//! INST_NOT     "Intrinsic.JSRuntimeNewJSValueString"

function jsnew_setproperty_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res_obj = new ClassWithEmptyConstructor();
    res_obj.v0 = v0;
    res_obj.v1 = v1;
    let res_get_v0 = res_obj.v0 as String;
    let res_get_v1 = res_obj.v1 as String;
    if (res_get_v0 == v0 && res_get_v1 == v1) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER      jscall_static_method_string JIT
//! RUN          force_jit: true, entry: "jscall_static_method_string", options: "--compiler-regex=ETSGLOBAL::jscall_static_method_string"
//! METHOD       "jscall_static_method_string"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /CallStatic.*jsnew::invoke/
//! INST_NOT     "Intrinsic.JSRuntimeNewJSValueString"
//! INST_NOT     /CallStatic.*jscall::invoke/
//! INST    "Intrinsic.JSRuntimeGetValueString"

function jscall_static_method_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res = ExampleClass.EvaluateNumber(v0, v1) as String;
    if (res == "helloworld") {
        return 0;
    } else {
        return 1;
    }
}

function jscall_array(): int {
    let v0: int[] = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }

    let res = EvaluateArray(v0, 10);
    for (let i = 0; i < 10; ++i) {
        if (res[i] as int != 2 * i * i) {
            return 1;
        }
    }

    return 0;
}

function jsnew_array(): int {
    let v0: int[] = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }

    let res_obj = new ExampleClass(v0, v0);
    let res_get_v0 = res_obj.v0;
    let res_get_v1 = res_obj.v1;

    for (let i = 0; i < 10; ++i) {
        if (res_get_v0[i] as int != v0[i] || res_get_v1[i] as int != v0[i]) {
            return 1;
        }
    }
    return 0;
}

function jsnew_setproperty_array(): int {
    let v0: int[] = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }

    let res_obj = new ClassWithEmptyConstructor();

    res_obj.v0 = v0;
    res_obj.v1 = v0;
    let res_get_v0 = res_obj.v0;
    let res_get_v1 = res_obj.v1;

    for (let i = 0; i < 10; ++i) {
        if (res_get_v0[i] as int != v0[i] || res_get_v1[i] as int != v0[i]) {
            return 1;
        }
    }
    return 0;
}

function jscall_static_method_array(): int {
    let v0: int[] = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }

    let res = ExampleClass.EvaluateArray(v0, 10);
    for (let i = 0; i < 10; ++i) {
        if (res[i] as int != 2 * i * i) {
            return 1;
        }
    }
    return 0;
}

function test_namespace(): int {
    let v0: int = 1;
    let v1: String = "Pinky";
    let res_obj = MyNamespace.create_kitten(v0, v1);
    let res_id = res_obj.id as int;
    let res_name = res_obj.name as String;
    if (res_id == v0 && res_name == v1) {
        return 0;
    } else {
        return 1;
    }
}
