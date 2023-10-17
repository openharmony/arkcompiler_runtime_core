/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

class jsvars {
    static global = JSRuntime.getGlobal();
    static gtest_env = JSRuntime.getPropertyJSValue(jsvars.global, "gtest_env");

    static m = JSRuntime.getPropertyJSValue(jsvars.gtest_env, "module");
};

class jscall {
    static _init = JSRuntime.__initJSCallClass("Ljscall;");

    native static void$(top: JSValue, qname: String): void;
    native static num$(top: JSValue, qname: String): double;
    native static any$(top: JSValue, qname: String): JSValue;
    native static any$(top: JSValue, qname: String, v0: JSValue): JSValue;
    native static any$(top: JSValue, qname: String, v0: String): JSValue;
    native static any$(top: JSValue, qname: String, v0: double, v1: String): JSValue;
    native static any$(top: JSValue, qname: String, v0: int[], v1: int): JSValue;
    // Remove after JSValue cast fix
    native static any$a(top: JSValue, qname: String, v0: JSValue): JSValue;

    native static str$(top: JSValue, qname: String): String;
    native static str$(top: JSValue, qname: String, v0: String): String;
    native static str$(top: JSValue, qname: String, v0: String, v1: String): String;
}

class jsnew {
    static _init = JSRuntime.__initJSNewClass("Ljsnew;");
    native static $(top: JSValue, qname: String, a0: double, a1: double) : JSValue;
    native static $(top: JSValue, qname: String, a0: boolean, a1: boolean) : JSValue;
    native static $(top: JSValue, qname: String) : JSValue;
    native static $(top: JSValue, qname: String, a0: JSValue, a1: JSValue) : JSValue;
    native static $(top: JSValue, qname: String, a0: String, a1: String) : JSValue;
    native static $(top: JSValue, qname: String, a0: int[], a1: int[]) : JSValue;
    // Remove after JSValue cast fix
    native static $aa(top: JSValue, qname: String, a0: JSValue, a1: JSValue) : JSValue;
    // Remove after JSValue cast fix
    native static $nn(top: JSValue, qname: String, a0: double, a1: double) : JSValue;
};

function jscall_empty(): int {
    jscall.void$(jsvars.m, ".EvaluateNumber");
    return 0;
}

function jsnew_empty(): int {
    let res_obj = jsnew.$(jsvars.m, ".ClassWithEmptyConstructor");
    let res_get_v0 = JSRuntime.getPropertyDouble(res_obj, "v0");
    let res_get_v1 = JSRuntime.getPropertyDouble(res_obj, "v1");
    if (res_get_v0 == 42 && res_get_v1 == 42) {
        return 0;
    } else {
        return 1;
    }
}

function jscall_static_method_empty(): int {
    jscall.void$(jsvars.m, ".ExampleClass.EmptyMethod");
    return 0;
}

function jscall_object(): int {
    let res_obj = jsnew.$nn(jsvars.m, ".ExampleClass", 42, 42);
    let res = jscall.any$a(jsvars.m, ".EvaluateObject", res_obj);
    let res_get_v0 = JSRuntime.getPropertyDouble(res, "v0");
    let res_get_v1 = JSRuntime.getPropertyDouble(res, "v1");
    if (res_get_v0 == 84 && res_get_v1 == 84) {
        return 0;
    } else {
        return 1;
    }
}

function jsnew_object(): int {
    let res_obj = jsnew.$nn(jsvars.m, ".ExampleClass", 42, 42);
    let res = jsnew.$aa(jsvars.m, ".ExampleClass", res_obj, res_obj);
    let res_get_v0 = JSRuntime.getPropertyJSValue(res, "v0");
    let res_get_v1 = JSRuntime.getPropertyJSValue(res, "v1");
    if (JSRuntime.getPropertyDouble(res_get_v0, "v0") == 42 && JSRuntime.getPropertyDouble(res_get_v0, "v1") == 42 &&
        JSRuntime.getPropertyDouble(res_get_v1, "v0") == 42 && JSRuntime.getPropertyDouble(res_get_v1, "v1") == 42) {
        return 0;
    } else {
        return 1;
    }
}

function jsnew_setproperty_object(): int {
    let res_obj = jsnew.$(jsvars.m, ".ClassWithEmptyConstructor");
    let res = jsnew.$(jsvars.m, ".ClassWithEmptyConstructor");
    JSRuntime.setPropertyJSValue(res_obj,"v0", res);
    JSRuntime.setPropertyJSValue(res_obj,"v1", res);
    let res_get_v0 = JSRuntime.getPropertyJSValue(res_obj, "v0");
    let res_get_v1 = JSRuntime.getPropertyJSValue(res_obj, "v1");
    if (JSRuntime.getPropertyDouble(res_get_v0, "v0") == 42 && JSRuntime.getPropertyDouble(res_get_v0, "v1") == 42 &&
        JSRuntime.getPropertyDouble(res_get_v1, "v0") == 42 && JSRuntime.getPropertyDouble(res_get_v1, "v1") == 42) {
        return 0;
    } else {
        return 1;
    }
}

function jscall_method_object(): int {
    let res_obj = jsnew.$nn(jsvars.m, ".ExampleClass", 42, 42);
    let res = jscall.any$(jsvars.m, ".EvaluateObject", res_obj);
    let res_get_v0 = JSRuntime.getPropertyDouble(res, "v0");
    let res_get_v1 = JSRuntime.getPropertyDouble(res, "v1");
    if (res_get_v0 == 84 && res_get_v1 == 84) {
        return 0;
    } else {
        return 1;
    }
}

function jscall_method_simple(): int {
    let res_obj = jsnew.$nn(jsvars.m, ".ExampleClass", 42, 42);
    let res = jscall.num$(res_obj, ".InstanceEvaluateNumber");
    if (res == 84) {
        return 0;
    } else {
        return 1;
    }
}

function jscall_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res = jscall.str$(jsvars.m, ".EvaluateNumber", v0, v1);
    if (res == "helloworld") {
        return 0;
    } else {
        return 1;
    }
}

function jsnew_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res_obj = jsnew.$(jsvars.m, ".ExampleClass", v0, v1);
    let res_get_v0 = JSRuntime.getPropertyString(res_obj, "v0");
    let res_get_v1 = JSRuntime.getPropertyString(res_obj, "v1");
    if (res_get_v0 == v0 && res_get_v1 == v1) {
        return 0;
    } else {
        return 1;
    }
}

function jsnew_setproperty_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res_obj = jsnew.$(jsvars.m, ".ClassWithEmptyConstructor");
    JSRuntime.setPropertyString(res_obj, "v0", v0);
    JSRuntime.setPropertyString(res_obj, "v1", v1);
    let res_get_v0 = JSRuntime.getPropertyString(res_obj, "v0");
    let res_get_v1 = JSRuntime.getPropertyString(res_obj, "v1");
    if (res_get_v0 == v0 && res_get_v1 == v1) {
        return 0;
    } else {
        return 1;
    }
}

function jscall_static_method_string(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res = jscall.str$(jsvars.m, ".ExampleClass.EvaluateNumber", v0, v1);
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

    let res = jscall.any$(jsvars.m, ".EvaluateArray", v0, 10);
    for (let i = 0; i < 10; ++i) {
        if (JSRuntime.getElementInt(res, i) != 2 * i * i) {
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


    let res_obj = jsnew.$(jsvars.m, ".ExampleClass", v0, v0);
    let res_get_v0 = JSRuntime.getPropertyJSValue(res_obj, "v0");
    let res_get_v1 = JSRuntime.getPropertyJSValue(res_obj, "v1");

    for (let i = 0; i < 10; ++i) {
        if (JSRuntime.getElementInt(res_get_v0, i) != v0[i] || JSRuntime.getElementInt(res_get_v1, i) != v0[i]) {
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

    let res_obj = jsnew.$(jsvars.m, ".ClassWithEmptyConstructor");

    JSRuntime.setPropertyJSValue(res_obj,"v0", JSRuntime.newJSValueObject(v0));
    JSRuntime.setPropertyJSValue(res_obj,"v1", JSRuntime.newJSValueObject(v0));
    let res_get_v0 = JSRuntime.getPropertyJSValue(res_obj, "v0");
    let res_get_v1 = JSRuntime.getPropertyJSValue(res_obj, "v1");

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

    let res = jscall.any$(jsvars.m, ".ExampleClass.EvaluateArray", v0, 10);
    for (let i = 0; i < 10; ++i) {
        if (JSRuntime.getElementInt(res, i) != 2 * i * i) {
            return 1;
        }
    }
    return 0;
}

function test_namespace(): int {
    let v0: int = 1;
    let v1: String = "Pinky";
    let res_obj = jscall.any$(jsvars.m, ".MyNamespace.create_kitten", v0, v1);
    let res_id = JSRuntime.getPropertyDouble(res_obj, "id");
    let res_name = JSRuntime.getPropertyString(res_obj, "name");
    if (res_id == v0 && res_name == v1) {
        return 0;
    } else {
        return 1;
    }
}
