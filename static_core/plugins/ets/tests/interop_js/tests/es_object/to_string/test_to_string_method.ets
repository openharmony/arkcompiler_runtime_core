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
} from "to_string";

function vstringify(val: JSValue): String {
    return StringifyValue(val) as String;
}

function castToObject(x: Object) : Object {
    return x;
}

class A {
    toString() : String {
        return CurryToString(123) + "A";
    };
}

function etsJsvalueToStr() {
    let x: JSValue = Identity(new A());
    return x + "A";
}

function etsObjectToStr() {
    let x: JSValue = Identity(new A());
    let y = castToObject(x);
    return y + "A";
}

function jsvalObjectToStr(): String {
    let x: JSValue = Curry(123);
    try {
        let z: String = x.toString();
        assertTrue(false);
    } catch (z: TypeError) {
        let y = castToObject(x);
        return y + "A";
    }
    return "Unreachable"
}

function implicitToString(): String {
    let x = Optional(false);
    console.log(123, x)
    assertTrue(vstringify(x) == "string:optionalVal")
    return x + "A";
}

function testFunctional1() : String {
    return Curry("Str1") + "A";
}

function testFunctional2() : String {
    return CurryToString(123) + "A";
}

function testNanCall() : String {
    return Curry(NaN) + "A";
}

function testThrowing() : String {
    try {
        return Throwing() + "A";
    } catch (e: ESError) {
        return e.getJsError() + "A";
    }
}

function concatNullToString(): String {
    let x = NullVal() as JSValue | null;
    return x + "A";
}

function concatUndefinedToString(): String {
   let x = UndefinedVal();
   let y = x + "A";
   return y;
}

function concatNanToString(): String {
    let x = NanVal();
    let y = x + "A";
    return y;
}

function explicitOptionalToString(): String {
    try {
        return Optional(true) + "A";
    } catch (e: NullPointerError) {
        return "NPE";
    }
}
