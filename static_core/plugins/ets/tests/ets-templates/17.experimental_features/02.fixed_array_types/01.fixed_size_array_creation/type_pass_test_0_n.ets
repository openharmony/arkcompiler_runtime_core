/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: New Expressions cannot use generic parameters to create a Fixed-size array. Attempting to do so causes a compile-time error
tags: [compile-only, negative]
expected_out: "Semantic error ESE0007: Cannot use array creation expression with type parameter."
---*/

function init<T>(size: int): FixedArray<T> {
    let rr = new FixedArray<T>(size);
    return rr;
}

function main() {
    let r = init<string>(3)
}
