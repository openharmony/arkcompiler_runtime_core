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
    Library using export specifiers with aliases (export { X as Y }).
    Covers exportSpecifier.cpp Dump branches with aliased local exports.
tags: [not-a-test, compile-only]
name: lib_spec
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-declaration']
---*/

class InternalCounter {
    private _count: int = 0
    increment(): void {
        this._count++
    }
    getCount(): int {
        return this._count
    }
}

function internalAdd(a: int, b: int): int {
    return a + b
}

let internalVersion: string = "2.0"

export { InternalCounter as Counter, internalAdd as add, internalVersion as VERSION }
