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
    Negative: usage references a field name that does not exist in the
    cross-package annotation declaration. Compilation
    is expected to fail.
files: [lib_anno.ets]
tags: [compile-only, negative]
name: test_import_n
arktsconfig: arktsconfig-import.json
---*/

import { Named } from '@panda/lib/lib_anno'

@Named({wrongName: "x"})
class Mislabeled {}

function main(): void {}
