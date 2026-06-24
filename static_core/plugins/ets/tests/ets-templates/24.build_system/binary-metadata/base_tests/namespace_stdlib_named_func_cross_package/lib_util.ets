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
    Cross-package source exporting a function whose simple name ('swap')
    collides with a std/core function, but wrapped in a NAMESPACE. Per
    spec 4.3 a namespace member is reached only by its qualified name
    ('util.swap'), so it never competes for the stdlib simple name at
    module scope. This is the spec-legal way to ship a stdlib-named helper
    (the bare-module form is rejected/buggy, see stdlib_function_name_
    collision_n). The consumer must resolve util.swap to THIS function,
    not the std/core swap overloads.
tags: [not-a-test, compile-only]
name: lib_util
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-metadata']
---*/

export namespace util {
    export function swap<T, U>(p: [T, U]): [U, T] {
        return [p[1], p[0]]
    }
}
