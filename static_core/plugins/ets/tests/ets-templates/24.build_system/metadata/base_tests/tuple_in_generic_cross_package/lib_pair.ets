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
desc: >-
    Generic library API whose return type is a tuple parameterized by
    type parameters T and U. Each consumer call with concrete types
    forces to instantiate a fresh tuple type.
tags: [not-a-test, compile-only]
name: lib_pair
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-declaration']
---*/

export function pair<T, U>(t: T, u: U): [T, U] {
    return [t, u]
}

export function flip<T, U>(p: [T, U]): [U, T] {
    return [p[1], p[0]]
}

export function first<T, U>(p: [T, U]): T {
    return p[0]
}
