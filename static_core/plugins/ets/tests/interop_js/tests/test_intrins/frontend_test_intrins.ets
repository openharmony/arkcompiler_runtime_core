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
    Identity,
    Sum,
    StringifyValue,
    Car,
    ApplyArgs,
    ThrowValue,
    Log,
    TestProxy,
    StringifyArgs,
    GetProp,
    Nop,
    Nonexisting
} from "/plugins/ets/tests/interop_js/tests/test_intrins/test_intrins.js";

function vstringify(val: JSValue): String {
    return StringifyValue(val) as String;
}
function unreachable(): Error {
    Log("unreachable reached!");
    return new Error("unreachable");
}
function throwmsg(msg: String): boolean {
    throw new Error(msg);
}
class BadClass {
    static _init: boolean = throwmsg("error from BadClass static ctor");
};

function js_sum_wrapper_num_num(a: double, b: double): double {
    return Sum(a, b);
}
function js_sum_wrapper_num_str(a: double, b: String): String {
    return Sum(a, b) as String;
}

function test_undefined(): boolean {
    let v = JSRuntime.getUndefined();
    let r = "undefined:undefined";
    if (Identity(v) as NullishType !== undefined) {
        return false;
    }
    return r == vstringify(v);
}
function test_null(): boolean {
    let v = JSRuntime.getNull();
    let r = "object:null";
    return r == vstringify(Identity(v));
}
function test_boolean(): boolean {
    let v = true;
    let r = "boolean:true";
    v = Identity(Identity(v)) as boolean;
    return r == vstringify(Identity(v));
}
function test_number(): boolean {
    let v: double = 42.24;
    let r = "number:42.24";
    v = Identity(Identity(v)) as double;
    return r == vstringify(Identity(v));
}
function test_string(): boolean {
    let v = "foo12345678901234567890"
    let r = "string:foo12345678901234567890";
    v = Identity(Identity(v)) as String;
    return r == vstringify(Identity(v));
}
function test_object(): boolean {
    let v = JSRuntime.createObject();
    let r = "object:[object Object]"
    return r == vstringify(Identity(v));
}

function test_string_ops(): boolean {
    let strv = "foo";
    let v1: JSValue = strv;
    if (v1 as String != strv) {
        return false;
    }
    if (GetProp(strv, "length") as double != strv.length) {
        return false;
    }
    if (Identity(Identity(strv)) as String != strv) {
        return false;
    }
    return true;
}

function test_builtin_array_any(): boolean {
    let v: JSValue[] = [JSRuntime.getUndefined(), JSRuntime.newJSValueDouble(1.23), JSRuntime.newJSValueString("foo"), JSRuntime.createObject()];
    let r = "object:,1.23,foo,[object Object]";
    // v = Identity(Identity(v)) as JSValue[]; // TODO(vpukhov): jscast
    let v1 = Identity(Identity(v));
    return r == vstringify(Identity(v1));
}
function test_builtin_array_boolean(): boolean {
    let v: boolean[] = [true, false, true];
    let r = "object:true,false,true";
    let v1 = Identity(Identity(v));
    return r == vstringify(Identity(v1));
}
function test_builtin_array_int(): boolean {
    let v: int[] = [111, 222];
    let r = "object:111,222";
    let v1 = Identity(Identity(v));
    return r == vstringify(Identity(v1));
}
function test_builtin_array_number(): boolean {
    let v: double[] = [11.1, 22.2];
    let r = "object:11.1,22.2";
    let v1 = Identity(Identity(v));
    return r == vstringify(Identity(v1));
}
function test_builtin_array_string(): boolean {
    let v: String[] = ["foo0", "foo1"];
    let r = "object:foo0,foo1";
    let v1 = Identity(Identity(v));
    return r == vstringify(Identity(v1));
}
function test_builtin_array_object(): boolean {
    let v: Object[] = [JSRuntime.getUndefined(), JSRuntime.createObject(), "foo", 1];
    let r = "object:,[object Object],foo,1";
    let v1 = Identity(Identity(v));
    return r == vstringify(Identity(v1));
}
function test_builtin_array_multidim(): boolean {
    let v: double[][] = [[11.1, 22.2], [33.3, 44.4]];
    let r = "object:11.1,22.2,33.3,44.4";
    let v1 = Identity(Identity(v));
    return r == vstringify(Identity(v1));
}
function test_builtin_array_instanceof(): boolean {
    let v: int[] = [111, 222];
    let ctor = JSRuntime.getGlobal().Array;
    return JSRuntime.instanceOf(Identity(v), ctor);
}
function test_init_array_component(): boolean {
    // TODO(vpukhov): relies on jscast BadClass[]
    return true;
}

function test_named_access(): boolean {
    let v: double = 12.34
    let obj = JSRuntime.createObject();
    obj.prop = v;
    return v == obj.prop as double;
}

function test_newcall(): boolean {
    let color = "Cyan";
    let car = new Car(color);
    return color == car.color as String;
}

function test_value_call(): boolean {
    return 123.4 == Identity(123.4) as double;
}

function test_call_stress(): boolean {
    let workload = 128;
    let u = JSRuntime.getUndefined();
    for (let i = 0; i < workload; ++i) {
        Nop(u, u, u, u)
    }
    return true;
}

// Remove after JSValue cast fix
function test_undefined_cast_bug(): boolean {
    let v = JSRuntime.getUndefined();
    let r = "undefined:undefined";
    return r == vstringify(Identity(v));
}

function test_lambda_proxy(): boolean {
    let etsfn: (x: double) => double = (x: double): double => {
        return x;
    }

    let res = ApplyArgs(etsfn, 123) as int;
    return res == 123;
}

function test_lambda_proxy_recursive(): boolean {
    let etsfn: (x: double, y: String) => String = (x: double, y: String): String => {
        return StringifyArgs(x, y) as String;
    }

    let res = ApplyArgs(etsfn, 123, "foo") as String;
    return res == "123,foo";
}

function test_exception_forwarding_fromjs(): boolean {
    try {
        ThrowValue(123.321);
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
        ApplyArgs(etsfn, 12345.6);
    } catch (e: JSError) {
        return "number:12345.6" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_exception_forwarding_recursive(): boolean {
    let etsfn: (v: JSValue) => void = (v: JSValue): void => {
        ThrowValue(v);
        throw unreachable();
    }

    try {
        ApplyArgs(etsfn, 123.456);
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
        ApplyArgs(etsfn);
    } catch (e: Error) {
        return e.message == "funny message";
    }
    throw unreachable();
}

function test_typecheck_getprop(): boolean {
    let obj = JSRuntime.createObject();
    obj.prop = 1.23;
    try {
        obj.prop as String;
    } catch (e: JSError) {
        return "object:TypeError: String expected" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_typecheck_jscall(): boolean {
    try {
        Identity(123) as String;
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

        ApplyArgs(etsfn, 123);
    } catch (e: JSError) {
        return "object:TypeError: String expected" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_get_throws(): boolean {
    let obj = new TestProxy();
    try {
        obj.foo as double;
    } catch (e: JSError) {
        return "object:Error: get exception" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_set_throws(): boolean {
    let obj = new TestProxy();
    try {
        obj.foo = 123;
    } catch (e: JSError) {
        return "object:Error: set exception" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_jscall_resolution_throws1(): boolean {
    try {
        Nonexisting();
    } catch (e: JSError) {
        return "object:TypeError: call target is not a function" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_jscall_resolution_throws2(): boolean {
    try {
        Nonexisting.Nonexisting();
    } catch (e: JSError) {
        return "object:TypeError: Cannot convert undefined or null to object" == vstringify(e.getValue());
    }
    throw unreachable();
}

function test_jscall_resolution_throws3(): boolean {
    let obj = new TestProxy();
    try {
        obj.foo();
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
