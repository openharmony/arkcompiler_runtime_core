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

class jsvars {
    static global = JSRuntime.getGlobal();
    static gtest_env = JSRuntime.getPropertyJSValue(jsvars.global, "gtest_env");

    static m = JSRuntime.getPropertyJSValue(jsvars.gtest_env, "test_intrins");
};

class jscall {
    static _init = JSRuntime.__initJSCallClass("Ljscall;");

    // Remove after JSValue cast fix: signature type suffixes

    native static void$(top: JSValue, qname: String): void;
    native static void$a(top: JSValue, qname: String, v0: JSValue): void;
    native static void$an(top: JSValue, qname: String, a0: JSValue, a1: double): void;
    native static void$n(top: JSValue, qname: String, v0: double): void;
    native static void$s(top: JSValue, qname: String, v0: String): void;

    native static bool$a(top: JSValue, qname: String, v0: JSValue): boolean;

    native static num$a(top: JSValue, qname: String, v0: JSValue): double
    native static num$an(top: JSValue, qname: String, a0: JSValue, a1: double): double;
    native static num$ann(top: JSValue, qname: String, a0: JSValue, a1: double, a2: double): double;
    native static num$nn(top: JSValue, qname: String, a0: double, a1: double): double;
    native static num$ss(top: JSValue, qname: String, a0: String, a1: String): double;

    native static str$a(top: JSValue, qname: String, v0: JSValue): String;
    native static str$aa(top: JSValue, qname: String, v0: JSValue, v1: JSValue): String;
    native static str$ans(top: JSValue, qname: String, a0: JSValue, a1: double, a2: String): String;
    native static str$n(top: JSValue, qname: String, a0: double): String;
    native static str$ns(top: JSValue, qname: String, a0: double, a1: String): String;
    native static str$nsn(top: JSValue, qname: String, a0: double, a1: String, a2: double): String;
    native static str$s(top: JSValue, qname: String, a0: String): String;

    native static any$a(top: JSValue, qname: String, v0: JSValue): JSValue;
    native static any$a$obj(top: JSValue, qname: String, v0: JSValue): NullishType;
    native static any$b(top: JSValue, qname: String, v0: boolean): JSValue;
    native static any$n(top: JSValue, qname: String, v0: double): JSValue;
    native static any$s(top: JSValue, qname: String, v0: String): JSValue;
    native static any$Aa(top: JSValue, qname: String, v0: JSValue[]): JSValue;
    native static any$Ab(top: JSValue, qname: String, v0: boolean[]): JSValue;
    native static any$Ai(top: JSValue, qname: String, v0: int[]): JSValue;
    native static any$An(top: JSValue, qname: String, v0: double[]): JSValue;
    native static any$AAn(top: JSValue, qname: String, v0: double[][]): JSValue;
    native static any$As(top: JSValue, qname: String, v0: String[]): JSValue;
    native static any$Ao(top: JSValue, qname: String, v0: Object[]): JSValue;
    native static any$aaaa(top: JSValue, qname: String, v0: JSValue, v1: JSValue, v2: JSValue, v3: JSValue): JSValue;
    // Remove after JSValue cast fix
    native static any$(top: JSValue, qname: String, v0: JSValue): JSValue;

    native static Aa$a(top: JSValue, qname: String, v0: JSValue): JSValue[];
    native static Ab$a(top: JSValue, qname: String, v0: JSValue): boolean[];
    native static Ai$a(top: JSValue, qname: String, v0: JSValue): int[];
    native static An$a(top: JSValue, qname: String, v0: JSValue): double[];
    native static AAn$a(top: JSValue, qname: String, v0: JSValue): double[][];
    native static As$a(top: JSValue, qname: String, v0: JSValue): String[];
    native static Ao$a(top: JSValue, qname: String, v0: JSValue): Object[];

    native static ALBadClass$a(top: JSValue, qname: String, v0: JSValue): BadClass[];

    native static num$vn(fn: JSValue, rec: JSValue, a0: double): double;
};

class jsnew {
    static _init = JSRuntime.__initJSNewClass("Ljsnew;");
    native static $(top: JSValue, qname: String): JSValue;
    native static $s(top: JSValue, qname: String, v0: String): JSValue;
};

function vstringify(val: JSValue): String {
    return jscall.str$a(jsvars.m, ".StringifyValue", val);
}
function vlog(val: JSValue): void {
    jscall.void$a(jsvars.m, ".Log", val);
}
function vlog(val: String): void {
    jscall.void$s(jsvars.m, ".Log", val);
}
function unreachable(): Error {
    vlog("unreachable reached!");
    return new Error("unreachable");
}
function throwmsg(msg: String): boolean {
    throw new Error(msg);
}
class BadClass {
    static _init: boolean = throwmsg("error from BadClass static ctor");
};

function js_sum_wrapper_num_num(a: double, b: double): double {
    return jscall.num$nn(jsvars.m, ".Sum", a, b);
}
function js_sum_wrapper_num_str(a: double, b: String): String {
    return jscall.str$ns(jsvars.m, ".Sum", a, b);
}

function test_undefined(): boolean {
    let v = JSRuntime.getUndefined();
    let r = "undefined:undefined";
    if (jscall.any$a$obj(jsvars.m, ".Identity", v) !== undefined) {
        return false;
    }
    return r == vstringify(v);
}

function test_null(): boolean {
    let v = JSRuntime.getNull();
    let r = "object:null";
    if (jscall.any$a(jsvars.m, ".Identity", v) as NullishType !== null) {
        return false;
    }
    return r == vstringify(v);
}
function test_boolean(): boolean {
    let v = true;
    let r = "boolean:true";
    v = jscall.bool$a(jsvars.m, ".Identity", jscall.any$b(jsvars.m, ".Identity", v));
    return r == vstringify(jscall.any$b(jsvars.m, ".Identity", v));
}
function test_number(): boolean {
    let v: double = 42.24;
    let r = "number:42.24";
    v = jscall.num$a(jsvars.m, ".Identity", jscall.any$n(jsvars.m, ".Identity", v));
    return r == vstringify(jscall.any$n(jsvars.m, ".Identity", v));
}
function test_string(): boolean {
    let v = "foo12345678901234567890";
    let r = "string:foo12345678901234567890";
    v = jscall.str$a(jsvars.m, ".Identity", jscall.any$s(jsvars.m, ".Identity", v));
    return r == vstringify(jscall.any$s(jsvars.m, ".Identity", v));
}
function test_object(): boolean {
    let v = JSRuntime.createObject();
    let r = "object:[object Object]";
    return r == vstringify(jscall.any$a(jsvars.m, ".Identity", v));
}

function test_string_ops(): boolean {
    let strv = "foo";
    let v1 = JSRuntime.newJSValueString(strv);
    if (JSRuntime.getValueString(v1) != strv) {
        return false;
    }
    if (jscall.num$ss(jsvars.m, ".GetProp", strv, "length") != strv.length) {
        return false;
    }
    if (jscall.str$a(jsvars.m, ".Identity", jscall.any$s(jsvars.m, ".Identity", strv)) != strv) {
        return false;
    }
    return true;
}

function test_builtin_array_any(): boolean {
    let v: JSValue[] = [JSRuntime.getUndefined(), JSRuntime.newJSValueDouble(1.23), JSRuntime.newJSValueString("foo"), JSRuntime.createObject()];
    let r = "object:,1.23,foo,[object Object]";
    v = jscall.Aa$a(jsvars.m, ".Identity", jscall.any$Aa(jsvars.m, ".Identity", v));
    return r == vstringify(jscall.any$Aa(jsvars.m, ".Identity", v));
}
function test_builtin_array_boolean(): boolean {
    let v: boolean[] = [true, false, true];
    let r = "object:true,false,true";
    v = jscall.Ab$a(jsvars.m, ".Identity", jscall.any$Ab(jsvars.m, ".Identity", v));
    return r == vstringify(jscall.any$Ab(jsvars.m, ".Identity", v));
}
function test_builtin_array_int(): boolean {
    let v: int[] = [111, 222];
    let r = "object:111,222";
    v = jscall.Ai$a(jsvars.m, ".Identity", jscall.any$Ai(jsvars.m, ".Identity", v));
    return r == vstringify(jscall.any$Ai(jsvars.m, ".Identity", v));
}
function test_builtin_array_number(): boolean {
    let v: double[] = [11.1, 22.2];
    let r = "object:11.1,22.2";
    v = jscall.An$a(jsvars.m, ".Identity", jscall.any$An(jsvars.m, ".Identity", v));
    return r == vstringify(jscall.any$An(jsvars.m, ".Identity", v));
}
function test_builtin_array_string(): boolean {
    let v: String[] = ["foo0", "foo1"];
    let r = "object:foo0,foo1";
    v = jscall.As$a(jsvars.m, ".Identity", jscall.any$As(jsvars.m, ".Identity", v));
    return r == vstringify(jscall.any$As(jsvars.m, ".Identity", v));
}
function test_builtin_array_object(): boolean {
    let v: Object[] = [JSRuntime.getUndefined(), JSRuntime.createObject(), "foo", 1, jsnew.$(jsvars.global, ".Number")];
    let r = "object:,[object Object],foo,1,0";
    v = jscall.Ao$a(jsvars.m, ".Identity", jscall.any$Ao(jsvars.m, ".Identity", v));
    return r == vstringify(jscall.any$Ao(jsvars.m, ".Identity", v));
}
function test_builtin_array_multidim(): boolean {
    let v: double[][] = [[11.1, 22.2], [33.3, 44.4]];
    let r = "object:11.1,22.2,33.3,44.4";
    v = jscall.AAn$a(jsvars.m, ".Identity", jscall.any$AAn(jsvars.m, ".Identity", v));
    return r == vstringify(jscall.any$AAn(jsvars.m, ".Identity", v));
}
function test_builtin_array_instanceof(): boolean {
    let v: int[] = [111, 222];
    let ctor = JSRuntime.getPropertyJSValue(jsvars.global, "Array");
    return JSRuntime.instanceOf(jscall.any$Ai(jsvars.m, ".Identity", v), ctor);
}
function test_init_array_component(): boolean {
    let v: JSValue[] = [JSRuntime.getNull()];
    let x = jscall.ALBadClass$a(jsvars.m, ".Identity", jscall.any$Aa(jsvars.m, ".Identity", v));
    return true;
}

function test_named_access(): boolean {
    let v: double = 12.34
    let obj = JSRuntime.createObject();
    JSRuntime.setPropertyDouble(obj, "prop", v);
    return v == JSRuntime.getPropertyDouble(obj, "prop");
}

function test_newcall(): boolean {
    let color = "Cyan";
    let car = jsnew.$s(jsvars.m, ".Car", color);
    return color == JSRuntime.getPropertyString(car, "color");
}

function test_value_call(): boolean {
    let fn = JSRuntime.getPropertyJSValue(jsvars.m, "Identity");
    return 123.4 == jscall.num$vn(fn, JSRuntime.getUndefined(), 123.4);
}

function test_call_stress(): boolean {
    let workload = 128;
    let u = JSRuntime.getUndefined();
    for (let i = 0; i < workload; ++i) {
        jscall.any$aaaa(jsvars.m, ".Nop", u, u, u, u);
    }
    return true;
}

// Remove after JSValue cast fix
function test_undefined_cast_bug(): boolean {
    let v = JSRuntime.getUndefined();
    let r = "undefined:undefined";
    return r == vstringify(jscall.any$(jsvars.m, ".Identity", v));
}

function test_lambda_proxy(): boolean {
    let etsfn: (x: double) => double = (x: double): double => {
        return x;
    }

    let res = jscall.num$an(jsvars.m, ".ApplyArgs", etsfn, 123);
    return res == 123;
}

function test_lambda_proxy_recursive(): boolean {
    let etsfn: (x: double, y: String) => String = (x: double, y: String): String => {
        return jscall.str$ns(jsvars.m, ".StringifyArgs", x, y);
    }

    let res = jscall.str$ans(jsvars.m, ".ApplyArgs", etsfn, 123, "foo");
    return res == "123,foo";
}

function test_exception_forwarding_fromjs(): boolean {
    try {
        jscall.void$n(jsvars.m, ".ThrowValue", 123.321);
    } catch (e: JSError) {
        return "number:123.321" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_exception_forwarding_fromets(): boolean {
    let etsfn: (v: JSValue) => void = (v: JSValue): void => {
        throw new JSError(v);
    }

    try {
        jscall.void$an(jsvars.m, ".ApplyArgs", etsfn, 12345.6);
    } catch (e: JSError) {
        return "number:12345.6" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_exception_forwarding_recursive(): boolean {
    let etsfn: (v: JSValue) => void = (v: JSValue): void => {
        jscall.void$a(jsvars.m, ".ThrowValue", v);
        throw unreachable();
    }

    try {
        jscall.void$an(jsvars.m, ".ApplyArgs", etsfn, 123.456);
    } catch (e: JSError) {
        return "number:123.456" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_core_error_forwarding(): boolean {
    let etsfn: () => void = (): void => {
        throw new Error("funny message");
    }

    try {
        jscall.void$a(jsvars.m, ".ApplyArgs", etsfn);
    } catch (e: Error) {
        return e.message == "funny message";
    }
    throw unreachable();
}

function test_typecheck_getprop(): boolean {
    let obj = JSRuntime.createObject();
    JSRuntime.setPropertyDouble(obj, "prop", 1.23);
    try {
        JSRuntime.getPropertyString(obj, "prop");
    } catch (e: JSError) {
        return "object:TypeError: String expected" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_typecheck_jscall(): boolean {
    try {
        jscall.str$n(jsvars.m, ".Identity", 123);
    } catch (e: JSError) {
        return "object:TypeError: String expected" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_typecheck_callets(): boolean {
    try {
        let etsfn: (x: String) => String = (x: String): String => {
            throw unreachable();
        }

        jscall.num$an(jsvars.m, ".ApplyArgs", etsfn, 123);
    } catch (e: JSError) {
        return "object:TypeError: String expected" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_get_throws(): boolean {
    let obj = jsnew.$(jsvars.m, ".TestProxy");
    try {
        JSRuntime.getPropertyDouble(obj, "foo");
    } catch (e: JSError) {
        return "object:Error: get exception" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_set_throws(): boolean {
    let obj = jsnew.$(jsvars.m, ".TestProxy");
    try {
        JSRuntime.setPropertyDouble(obj, "foo", 123);
    } catch (e: JSError) {
        return "object:Error: set exception" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_jscall_resolution_throws1(): boolean {
    try {
        jscall.void$(jsvars.m, ".Nonexisting");
    } catch (e: JSError) {
        return "object:TypeError: call target is not a function" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_jscall_resolution_throws2(): boolean {
    try {
        jscall.void$(jsvars.m, ".Nonexisting.Nonexsiting");
    } catch (e: JSError) {
        return "object:TypeError: Cannot convert undefined or null to object" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_jscall_resolution_throws3(): boolean {
    let obj = jsnew.$(jsvars.m, ".TestProxy");
    try {
        jscall.void$(obj, ".foo");
    } catch (e: JSError) {
        return "object:Error: get exception" == vstringify(e.getValue());
    }
    throw unreachable();
}

function gc_jsruntime_cleanup(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let full_gc_id = GC.startGC(GC.FULL_CAUSE);
        GC.waitForFinishGC(full_gc_id);
    } catch (e) {
        assert false : "Unexpected exception during GC";
    }
}

function test_finalizers(): boolean {
    let workload = 128;
    let repeats = 4;

    for (let k = 0; k < repeats; ++k) {
        gc_jsruntime_cleanup();
        let num_refs_pre = JSRuntime.getNumSharedJSValues();

        for (let i = 0; i < workload; ++i) {
            JSRuntime.createObject();
        }
        for (let i = 0; i < workload; ++i) {
            JSRuntime.newJSValueString("foo");
        }
        for (let i = 0; i < workload; ++i) {
            JSRuntime.newJSValueDouble(1234);
        }

        assert JSRuntime.getNumSharedJSValues() == num_refs_pre + workload;
        gc_jsruntime_cleanup();
        assert JSRuntime.getNumSharedJSValues() == num_refs_pre;
    }
    return true;
}
