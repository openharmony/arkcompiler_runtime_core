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
  An expression may occur:
  - inside a function body;
---*/

type Inc = (i: int) => long;

function inc(p: int): long {
    return (p + 1) as long;
}

function foo(i: int, func: Inc): long {
    return func(i);
}

function main(): int {
    let i: long = foo(12, inc);

    if (i != 13) return 1;
    return 0;
}
