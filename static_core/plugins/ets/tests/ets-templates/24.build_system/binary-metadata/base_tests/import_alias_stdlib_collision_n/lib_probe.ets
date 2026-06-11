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
    Cross-package source with three NON-colliding exports (a class, a
    function, a const). Consumers import each one under an ALIAS that DOES
    collide with a std/core entity, exercising the import-alias side of the
    stdlib-name rule.
tags: [not-a-test, compile-only]
name: lib_probe
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-metadata']
---*/

export class Widget {
    id: int = 7
}

export function pair<T, U>(t: T, u: U): [T, U] {
    return [t, u]
}

export const SETTING: int = 42
