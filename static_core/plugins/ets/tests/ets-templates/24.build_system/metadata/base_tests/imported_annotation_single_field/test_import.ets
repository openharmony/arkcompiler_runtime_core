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
    Apply two cross-package single-field annotations in the IMPLICIT form.
    Two payload types (string and int) and four target kinds (class,
    function, method, field) maximize code coverage.
files: [lib_anno.ets]
name: test_import
arktsconfig: arktsconfig-import.json
tags: [compile-only]
---*/

import { Tag, Level } from '@panda/lib/lib_anno'

@Tag("alpha")
class Alpha {}

@Tag("beta")
class Beta {
    @Level(7)
    rank: int = 0

    @Tag("method-scoped")
    describe(): string {
        return "beta"
    }
}

@Level(5)
function foo(): void {}

@Level(10)
class Numbered {}

function main(): void {
    let a = new Alpha()
    let b = new Beta()
    let n = new Numbered()
    foo()
}
