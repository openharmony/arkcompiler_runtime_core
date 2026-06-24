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
    Bottom of a 3-package NAMED re-export chain (consumer -> a -> b -> c).
    Unlike `reexport_star_chain`, intermediates use selective
    `export { X } from '...'` rather than `export *` — this exercises the
    SelectiveExportAliasMultimap insertion path of InsertReExported
    repeatedly through two indirections.
tags: [not-a-test, compile-only]
name: lib_c
arktsconfig: arktsconfig-c.json
es2panda_options: ['emit-metadata']
---*/

export class Container {
    value: int
    constructor(value: int) {
        this.value = value
    }
}

export function unbox(b: Container): int {
    return b.value
}

export let MAGIC: int = 13
