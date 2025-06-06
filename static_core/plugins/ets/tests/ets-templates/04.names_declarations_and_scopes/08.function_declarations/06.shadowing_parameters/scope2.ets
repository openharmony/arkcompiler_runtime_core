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
desc: >-
    Variables and other entities defined in a function are local to the function and cannot be accessed outside.
    If the name of a variable defined in the function is equal to the name of an entity in the outer scope,
    then the local definition shadows outer entity.
---*/

let q: int = 1;

function getQ(): int {
    return q;  // returns top-level q
}

function setQ(p: int): void {
    q = p;  // top-level q
}

function bar(q: int): int {  // q is shadowing top-level q
    q = q + getQ();
    return q;
}

function foo(q: int): int {
    setQ(q);
    return ++q;
}

function main(): int {
    q = 1;  // reset due to jit mode warmup

    if (bar(10) == 11 && q == 1 && foo(5) == 6 && q == 5) return 0;
    return 1;
}
