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
    Test importing annotation from another package and using it on local entities.
    Covers ETSemitter GenCustomAnnotation import branch (lines 1135-1140).
files: [lib_anno.ets]
name: test_import
arktsconfig: arktsconfig-import.json
tags: [compile-only]
---*/

import { Priority, Service } from '@panda/lib/lib_anno'

@Priority({level: 1})
class LocalHandler {
    handle(): string { return "handled" }
}

@Priority({level: 2})
class HighPriorityTask {
    run(): void {}
}

function main(): void {
    let s = new Service("svc1")
    let h = new LocalHandler()
}
