/*
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

/*---
desc: >-
        Selection of Best Candidate.
        If argument type is of a numeric type (see Numeric Types), char, or its boxed counterpart,
        then the candidate with a shorter conversion is better.
assert: No transformation is better than any transformation
tags: []
---*/
let candidate = 0;

function foo(value: int) {
    candidate = 1
}

function foo(value: Int) {
    candidate = 2
}

function main(): void {
    candidate = 0;
    let x: int = 42;
    foo(x);
    assertEQ(candidate, 1)
}

