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

import { GetObject, GetNumber, GetString, Nop } from "js_code";

function voidCall(iters: int): boolean {
    for (let i = 0; i < iters; ++i) {
        Nop();
    }
    return true;
}

function voidCall10str(iters: int): boolean {
    let u0: String = "foo0";
    let u1: String = "foo1";
    let u2: String = "foo2";
    let u3: String = "foo3";
    let u4: String = "foo4";
    let u5: String = "foo5";
    let u6: String = "foo6";
    let u7: String = "foo7";
    let u8: String = "foo8";
    let u9: String = "foo9";
    for (let i = 0; i < iters; ++i) {
        Nop(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9);
    }
    return true;
}

function voidCall10dbl(iters: int): boolean {
    let u0: double = 0;
    let u1: double = 1;
    let u2: double = 2;
    let u3: double = 3;
    let u4: double = 4;
    let u5: double = 5;
    let u6: double = 6;
    let u7: double = 7;
    let u8: double = 8;
    let u9: double = 9;
    for (let i = 0; i < iters; ++i) {
        Nop(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9);
    }
    return true;
}

class Empty {
};

function strCall(iters: int): boolean {
    for (let i = 0; i < iters; ++i) {
        let r = GetString() as String;
    }
    return true;
}

function anystrCall(iters: int): boolean {
    for (let i = 0; i < iters; ++i) {
        let r = GetString();
    }
    return true;
}

function dblCall(iters: int): boolean {
    for (let i = 0; i < iters; ++i) {
        let r = GetNumber() as number;
    }
    return true;
}

function anydblCall(iters: int): boolean {
    for (let i = 0; i < iters; ++i) {
        let r = GetNumber();
    }
    return true;
}

function anyobjCall(iters: int): boolean {
    for (let i = 0; i < iters; ++i) {
        let r = GetObject();
    }
    return true;
}
