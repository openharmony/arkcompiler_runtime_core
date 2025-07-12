/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

package rest_spread.test;

function checkRestOfNumber(f: JSValue) {
    return f([11, 22, 33], 11, 22, 33);
}

function checkRestOfString(f: JSValue) {
    return f(['aa', 'bb', 'cc'], 'aa', 'bb', 'cc');
}

interface B{name: string}

class A{}

function checkRestOfObject(f: JSValue) {
    let a = new A();
    let b: B = {name: "bbb"};
    let c = () => {};
    let objArr: object[] = [a, b, c];
    return f(objArr, a, b, c);
}

function checkRestOfTuple(f: JSValue) {
    let tuple1: [int, string] = [1, 'a'];
    let tuple2: [int, string] = [2, 'b'];
    let tuple3: [int, string] = [3, 'c'];
    let tuple4: [int, string][] = [tuple1, tuple2, tuple3];
    return f(tuple4, tuple1, tuple2, tuple3);
}

function checkRestofUnion(f: JSValue) {
    let unionArr1: (int | string | boolean)[] = [1, false];
    let unionArr2: (int | string | boolean)[] = [2, 3, true, 'a', 'b'];
    return f([unionArr1, unionArr2], unionArr1, unionArr2);
}

function checkRestOfJSValue(f: JSValue) {
    let a: JSValue = 1;
    let b: JSValue = "b";
    let c: JSValue = true;
    return f([a, b, c], a, b, c);
}