/*
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

const correct_res : int = 42;
let res : int = 0;

class A {
    public foo() : void {
        res = 41;
    }
}

class B extends A {
    public override foo() : void {
        res = 42;
    }
}

function call(a: A) : void {
    a.foo();
}

function main(): void {
    let b: B = new B();
    call(b);
    assert res == correct_res: "ERROR: Wrong res after virtual call";
}
