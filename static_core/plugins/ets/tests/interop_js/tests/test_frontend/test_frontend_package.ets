/*
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

package Test;

import {
    Car, foo, A
} from "js_code";


function test_newcall(): boolean {
    let color = "Cyan";
    let car = new Car(color);
    return color == car.color as String;
}

function test_dyncall(): int {
    let a = new A();
    // this call is needed to fill dynamic call cache with wrong signature
    let fake_res = foo(314);
    return a.getB().propVar.foo(5) as int;
}

function test_dyncall_by_value(): int {
    let a = new A();
    // this call is needed to fill dynamic call cache with wrong signature
    let fake_res = a.getB().propVar.foo(5) as int;
    return foo(1) as int;
}
