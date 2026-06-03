/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Library that exports a function returning `never` across a package boundary (non-simultaneous)
tags: [not-a-test, compile-only]
name: test_lib
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-declaration']
---*/

export function failWith(msg: string): never {
    throw new Error(msg)
}

export function pickPositive(x: int): int {
    if (x <= 0) {
        failWith("non-positive")
    }
    return x
}
