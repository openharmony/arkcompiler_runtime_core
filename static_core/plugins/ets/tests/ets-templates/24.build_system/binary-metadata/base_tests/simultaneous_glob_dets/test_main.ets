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
    Negative runtime test: arktsconfig has no "files" section, so GetSourceList
    uses glob which matches .d.ets (ends with .ets). In simultaneous mode,
    globbed .d.ets files must stay SOURCE_DECL instead of MODULE. SOURCE_DECL
    gives type information but no implementation in abc, so runtime fails with
    a missing implementation. If .d.ets is compiled as MODULE, runtime succeeds
    and prints 42.
tags: [negative]
negative_steps: [verifier, runtime]
expected_error:
  verifier: ["E/verifier: Cannot link class: Cannot find method 'helperCreate (I)Ltypes/Helper;' in class 'types.ETSGLOBAL'"]
  runtime: ["Unhandled exception: std.core.LinkerVerificationError"]
name: test_main
arktsconfig: arktsconfig.json
es2panda_options: ['simultaneous']
---*/

import { Helper, helperCreate } from "./types"

function main(): void {
    let h: Helper = helperCreate(42)
    console.log(h.getValue())
}
