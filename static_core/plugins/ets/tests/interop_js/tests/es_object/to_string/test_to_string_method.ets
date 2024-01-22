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
    IntValue,
    Optional,
    Curry,
    CurryToString,
    StringifyValue,
    Throwing,
    NullVal,
    NanVal,
    UndefinedVal,
} from "/plugins/ets/tests/interop_js/tests/es_object/to_string/test_to_string_method.js";

function vstringify(val: JSValue): String {
    return StringifyValue(val) as String;
}

function cast_to_object(x: Object) : Object {
    return x;
}

class A {
    toString() : String {
        return CurryToString(123) + "A";
    };
}

function ets_jsvalue_to_str() {
    let x: JSValue = Identity(new A());
    return x + "A";
}

function ets_object_to_str() {
    let x: JSValue = Identity(new A());
    let y = cast_to_object(x);
    return y + "A";
}

function jsval_object_to_str(): String {
    let x: JSValue = Curry(123);
    try {
        let z: String = x.toString();
        assert(false);
    } catch (z: JSError) {
        let y = cast_to_object(x);
        return y + "A";
    }
    return "Unreachable"
}

function implicit_to_string(): String {
    let x = Optional(false);
    assert(vstringify(x) == "string:optional_val")
    return x + "A";
}

function test_functional1() : String {
    return Curry("Str1") + "A";
}

function test_functional2() : String {
    return CurryToString(123) + "A";
}

function test_nan_call() : String {
    return Curry(NaN) + "A";
}

function test_throwing() : String {
    try {
        return Throwing(123) + "A";
    } catch (e: JSError) {
        return e.getValue() + "A";
    }
}

function concat_null_to_string(): String {
    let x = NullVal() as JSValue | null;
    return x + "A";
}

function concat_undefined_to_string(): String {
   let x = UndefinedVal();
   let y = x + "A";
   return y;
}

function concat_nan_to_string(): String {
    let x = NanVal();
    let y = x + "A";
    return y;
}

function explicit_optional_to_string(): String {
    try {
        return Optional(true) + "A";
    } catch (e: NullPointerException) {
        return "NPE";
    }
}
