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

/*---
desc: 16.6 Callable Types
name: type-call-expression-short-with-param_1.ets
tags: []
---*/

let name: String = "0";
let counter: int = 0;
let pi: number = 0;

class CallMeTrinity {
    static $_invoke(val: int) { counter = val; }
    static $_invoke(val: String) { name = val; }
    static $_invoke(val: number) { pi = val; }
}

function main(): int {
    name = "0";
    counter = 0;
    pi = 0;
    CallMeTrinity("Trinity");
    CallMeTrinity(42);
    CallMeTrinity(3.14159265358)
    return name == "Trinity" && counter == 42 && pi == 3.14159265358 ? 0 : 1;
}
