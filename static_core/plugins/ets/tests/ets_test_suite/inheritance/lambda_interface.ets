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

const correct_res : int = 4;

let cb: (a: int, b: int): int;

class A {
    foo(a: int, b: int): int {
        return a + b + this.c;
    }

    c: int = 1;
}

function main(): void {
    let a = new A();
    cb = a.foo;
    assert cb(1, 2) == correct_res: "ERROR: Wrong res after lambda call";
}
