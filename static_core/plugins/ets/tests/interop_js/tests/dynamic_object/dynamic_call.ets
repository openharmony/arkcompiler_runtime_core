/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

import { RegularFunc, Foo, MultipleCall, Lambda, ArrayOfFoo, DoubleArrayOfFoo, ArrayOfLambda } from "js_code"


function regularFunc(): int {
    let x = RegularFunc() as Int;
    let callee = RegularFunc;
    assertTrue(x == callee() as Int)
    return x
}

class DynamicField {
    public foo: JSValue = RegularFunc;
}

// Right now we need this wrapper because without it field `foo` is uninitialized
class DynFieldHelper {
    public x: Int;
    constructor() {
        let x = new DynamicField();
        x.foo();
        let callee = x.foo;
        assertTrue(x.foo() == callee() as Int)
        // return x.foo()
        this.x = x.foo();
    }
}

function dynamicField(): Int {
    return (new DynFieldHelper()).x;
}

function methodCall(): int {
    let x = Foo.foo().foo() as Int;
    let callee = Foo;
    assertTrue(x == callee.foo().foo() as Int)
    return x
}

function multipleCall(): int {
    let x = MultipleCall()() as Int;
    let callee = MultipleCall;
    assertTrue(x == callee()() as Int)
    return x
}

function lambda(): int {
    let callee = Lambda;
    let x = Lambda() as Int;
    assertTrue(x == callee() as Int) // through variable is different, because we don't need to load module
    return x;
}

function simpleArr(): int {
    let x = ArrayOfFoo[0].foo() as Int;
    let callee = ArrayOfFoo
    assertTrue(x == callee[0].foo() as Int)
    return x;
}

function multidimArray(): int {
    let x = DoubleArrayOfFoo[0][0].foo() as Int;
    let callee = DoubleArrayOfFoo
    assertTrue(x == callee[0][0].foo() as Int)
    return x;
}

function lambdaArrCall(): int {
    let x = ArrayOfLambda[0]() as Int;
    let callee = ArrayOfLambda
    assertTrue(x == callee[0]() as Int)
    return x;
}
