/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
    static gtestEnv = JSRuntime.getPropertyJSValue(jsvars.global, "gtest_env");

    static m = JSRuntime.getPropertyJSValue(jsvars.gtestEnv, "module");
};

class jscall {
    static Init = JSRuntime.__initJSCallClass();

    native static void$(top: JSValue, qname: String): void;
    native static num$(top: JSValue, qname: String): double;
    native static any$(top: JSValue, qname: String): JSValue;
    native static any$(top: JSValue, qname: String, v0: JSValue): JSValue;
    native static any$(top: JSValue, qname: String, v0: String): JSValue;
    native static any$(top: JSValue, qname: String, v0: double, v1: String): JSValue;
    native static any$(top: JSValue, qname: String, v0: FixedArray<int>, v1: int): JSValue;
    // Remove after JSValue cast fix
    native static any$a(top: JSValue, qname: String, v0: JSValue): JSValue;

    native static str$(top: JSValue, qname: String): String;
    native static str$(top: JSValue, qname: String, v0: String): String;
    native static str$(top: JSValue, qname: String, v0: String, v1: String): String;
}

class jsnew {
    static Init = JSRuntime.__initJSNewClass();

    native static $(top: JSValue, qname: String, a0: double, a1: double) : JSValue;
    native static $(top: JSValue, qname: String, a0: boolean, a1: boolean) : JSValue;
    native static $(top: JSValue, qname: String) : JSValue;
    native static $(top: JSValue, qname: String, a0: JSValue, a1: JSValue) : JSValue;
    native static $(top: JSValue, qname: String, a0: String, a1: String) : JSValue;
    native static $(top: JSValue, qname: String, a0: FixedArray<int>, a1: FixedArray<int>) : JSValue;
    // Remove after JSValue cast fix
    native static $aa(top: JSValue, qname: String, a0: JSValue, a1: JSValue) : JSValue;
    // Remove after JSValue cast fix
    native static $nn(top: JSValue, qname: String, a0: double, a1: double) : JSValue;
};

function jscallEmpty(): int {
    jscall.void$(jsvars.m, ".evaluateNumber");
    return 0;
}

function jsnewEmpty(): int {
    let resObj = jsnew.$(jsvars.m, ".ClassWithEmptyConstructor");
    let resGetV0 = JSRuntime.getPropertyDouble(resObj, "v0");
    let resGetV1 = JSRuntime.getPropertyDouble(resObj, "v1");
    if (resGetV0 == 42 && resGetV1 == 42) {
        return 0;
    } else {
        return 1;
    }
}

function jscallStaticMethodEmpty(): int {
    jscall.void$(jsvars.m, ".ExampleClass.emptyMethod");
    return 0;
}

function jscallObject(): int {
    let resObj = jsnew.$nn(jsvars.m, ".ExampleClass", 42, 42);
    let res = jscall.any$a(jsvars.m, ".evaluateObject", resObj);
    let resGetV0 = JSRuntime.getPropertyDouble(res, "v0");
    let resGetV1 = JSRuntime.getPropertyDouble(res, "v1");
    if (resGetV0 == 84 && resGetV1 == 84) {
        return 0;
    } else {
        return 1;
    }
}

function jsnewObject(): int {
    let resObj = jsnew.$nn(jsvars.m, ".ExampleClass", 42, 42);
    let res = jsnew.$aa(jsvars.m, ".ExampleClass", resObj, resObj);
    let resGetV0 = JSRuntime.getPropertyJSValue(res, "v0");
    let resGetV1 = JSRuntime.getPropertyJSValue(res, "v1");
    if (JSRuntime.getPropertyDouble(resGetV0, "v0") == 42 && JSRuntime.getPropertyDouble(resGetV0, "v1") == 42 &&
        JSRuntime.getPropertyDouble(resGetV1, "v0") == 42 && JSRuntime.getPropertyDouble(resGetV1, "v1") == 42) {
        return 0;
    } else {
        return 1;
    }
}

function jsnewSetPropertyObject(): int {
    let resObj = jsnew.$(jsvars.m, ".ClassWithEmptyConstructor");
    let res = jsnew.$(jsvars.m, ".ClassWithEmptyConstructor");
    JSRuntime.setPropertyJSValue(resObj,"v0", res);
    JSRuntime.setPropertyJSValue(resObj,"v1", res);
    let resGetV0 = JSRuntime.getPropertyJSValue(resObj, "v0");
    let resGetV1 = JSRuntime.getPropertyJSValue(resObj, "v1");
    if (JSRuntime.getPropertyDouble(resGetV0, "v0") == 42 && JSRuntime.getPropertyDouble(resGetV0, "v1") == 42 &&
        JSRuntime.getPropertyDouble(resGetV1, "v0") == 42 && JSRuntime.getPropertyDouble(resGetV1, "v1") == 42) {
        return 0;
    } else {
        return 1;
    }
}

function jscallMethodObject(): int {
    let resObj = jsnew.$nn(jsvars.m, ".ExampleClass", 42, 42);
    let res = jscall.any$(jsvars.m, ".evaluateObject", resObj);
    let resGetV0 = JSRuntime.getPropertyDouble(res, "v0");
    let resGetV1 = JSRuntime.getPropertyDouble(res, "v1");
    if (resGetV0 == 84 && resGetV1 == 84) {
        return 0;
    } else {
        return 1;
    }
}

function jscallMethodSimple(): int {
    let resObj = jsnew.$nn(jsvars.m, ".ExampleClass", 42, 42);
    let res = jscall.num$(resObj, ".instanceEvaluateNumber");
    if (res == 84) {
        return 0;
    } else {
        return 1;
    }
}

function jscallString(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res = jscall.str$(jsvars.m, ".evaluateNumber", v0, v1);
    if (res == "helloworld") {
        return 0;
    } else {
        return 1;
    }
}

function jsnewString(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let resObj = jsnew.$(jsvars.m, ".ExampleClass", v0, v1);
    let resGetV0 = JSRuntime.getPropertyString(resObj, "v0");
    let resGetV1 = JSRuntime.getPropertyString(resObj, "v1");
    if (resGetV0 == v0 && resGetV1 == v1) {
        return 0;
    } else {
        return 1;
    }
}

function jsnewSetPropertyString(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let resObj = jsnew.$(jsvars.m, ".ClassWithEmptyConstructor");
    JSRuntime.setPropertyString(resObj, "v0", v0);
    JSRuntime.setPropertyString(resObj, "v1", v1);
    let resGetV0 = JSRuntime.getPropertyString(resObj, "v0");
    let resGetV1 = JSRuntime.getPropertyString(resObj, "v1");
    if (resGetV0 == v0 && resGetV1 == v1) {
        return 0;
    } else {
        return 1;
    }
}

function jscallStaticMethodString(): int {
    let v0: String = "hello";
    let v1: String = "world";
    let res = jscall.str$(jsvars.m, ".ExampleClass.evaluateNumber", v0, v1);
    if (res == "helloworld") {
        return 0;
    } else {
        return 1;
    }
}

function jscallArray(): int {
    let v0: FixedArray<int> = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }

    let res = jscall.any$(jsvars.m, ".evaluateArray", v0, 10);
    for (let i = 0; i < 10; ++i) {
        if (JSRuntime.getElementInt(res, i) != 2 * i * i) {
            return 1;
        }
    }

    return 0;
}

function jsnewArray(): int {
    let v0: FixedArray<int> = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }


    let resObj = jsnew.$(jsvars.m, ".ExampleClass", v0, v0);
    let resGetV0 = JSRuntime.getPropertyJSValue(resObj, "v0");
    let resGetV1 = JSRuntime.getPropertyJSValue(resObj, "v1");

    for (let i = 0; i < 10; ++i) {
        if (JSRuntime.getElementInt(resGetV0, i) != v0[i] || JSRuntime.getElementInt(resGetV1, i) != v0[i]) {
            return 1;
        }
    }
    return 0;
}

function jsnewSetPropertyArray(): int {
    let v0: FixedArray<int> = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }

    let resObj = jsnew.$(jsvars.m, ".ClassWithEmptyConstructor");

    JSRuntime.setPropertyJSValue(resObj,"v0", JSRuntime.newJSValueObject(v0));
    JSRuntime.setPropertyJSValue(resObj,"v1", JSRuntime.newJSValueObject(v0));
    let resGetV0 = JSRuntime.getPropertyJSValue(resObj, "v0");
    let resGetV1 = JSRuntime.getPropertyJSValue(resObj, "v1");

    for (let i = 0; i < 10; ++i) {
        if (resGetV0[i] as int != v0[i] || resGetV1[i] as int != v0[i]) {
            return 1;
        }
    }
    return 0;
}

function jscallStaticMethodArray(): int {
    let v0: FixedArray<int> = new int[10];
    for (let i = 0; i < 10; ++i) {
        v0[i] = i * i;
    }

    let res = jscall.any$(jsvars.m, ".ExampleClass.evaluateArray", v0, 10);
    for (let i = 0; i < 10; ++i) {
        if (JSRuntime.getElementInt(res, i) != 2 * i * i) {
            return 1;
        }
    }
    return 0;
}

function testNamespace(): int {
    let v0: int = 1;    
    let v1: String = "Pinky";
    let resObj = jscall.any$(jsvars.m, ".MyNamespace.createKitten", v0, v1);
    let resId = JSRuntime.getPropertyDouble(resObj, "id");
    let resName = JSRuntime.getPropertyString(resObj, "name");
    if (resId == v0 && resName == v1) {
        return 0;
    } else {
        return 1;
    }
}
