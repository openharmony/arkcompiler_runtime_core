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
    Negative runtime test: .d.ets file with method implementations is present in the
    directory. arktsconfig has no "files" section, so GetSourceList uses glob which
    matches .d.ets (ends with .ets). In simultaneous mode, InitParseQueueForSimult
    incorrectly sets MODULE kind on the .d.ets file instead of SOURCE_DECL.
    When .d.ets is SOURCE_DECL (correct): types are external, no implementation in abc,
    runtime fails with "Cannot find class" — expected behavior for this test.
    When .d.ets is MODULE (bug): implementations are compiled into abc, runtime succeeds
    — this test will incorrectly PASS, exposing the bug.
tags: [negative]
name: test_main
arktsconfig: arktsconfig.json
es2panda_options: ['simultaneous']
---*/

import { Helper, helperCreate } from "./types"

function main(): void {
    let h: Helper = helperCreate(42)
    console.log(h.getValue())
}
