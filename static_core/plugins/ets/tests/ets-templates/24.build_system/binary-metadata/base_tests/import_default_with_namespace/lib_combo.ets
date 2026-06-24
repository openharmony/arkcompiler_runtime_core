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
    Library combining default + multiple named exports. The consumer
    will import the default AND a namespace alias in one statement
    (`import D, * as NS from ...`).
tags: [not-a-test, compile-only]
name: lib_combo
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-metadata']
---*/

export default class Engine {
    spec(): string {
        return "default-engine"
    }
}

export class Wheel {
    diameter: int = 18
}

export function build(): string {
    return "built"
}

export let MODEL: string = "X1"
