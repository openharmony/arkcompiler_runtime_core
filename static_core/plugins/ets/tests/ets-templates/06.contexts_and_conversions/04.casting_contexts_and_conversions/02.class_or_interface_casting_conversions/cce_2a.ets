/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    A runtime error (ClassCastError) occurs during these conversion
    if the type of a converted expression cannot be converted to the
    target type
tags: [compile-only, negative]
---*/

function foo(p: Object[]|null): Object[] {
    return p!
}

function main(): int {
    let b: Numeric[] = new Long[1]
    try {
        let d = foo(b) as Boolean[]
    } catch (e) {
        return e instanceof ClassCastError ? 0 : 1
    }
    return 1
}
