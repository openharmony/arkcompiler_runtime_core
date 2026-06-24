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
    Test for binary compatibility. Removing an exported variable from namespace
    breaks the binary compatibility.
files: ['./variable_removing_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs:
        - "E/verifier: Cannot link class: Cannot find field 'v' in class 'variable_removing_n.libr.N'"
        - "E/verifier: Verifier error 55: Cache miss for field at offset"
        - "E/verifier: Verifier error 61: Cannot resolve field id"
        - "E/verifier: Error: method void variable_removing_n.ETSGLOBAL::main() failed to verify"
    runtime-with-new-libs.ark:
        - "[TID 13e971] E/verifier: Cannot link class: Cannot find field 'v' in class 'variable_removing_n.libr.N'"
        - "[TID 13e971] E/verifier: Verifier error 55: Cache miss for field at offset"
        - "[TID 13e971] E/verifier: Verifier error 61: Cannot resolve field id"
        - "[TID 13e971] E/verifier: Method verification failed: void variable_removing_n.ETSGLOBAL::main()"
        - "[TID 13e971] E/verifier: Class verification failed: 'variable_removing_n.ETSGLOBAL' method: void variable_removing_n.ETSGLOBAL::main()"
        - "[TID 13e971] E/runtime: Cannot initialize class 'variable_removing_n.ETSGLOBAL'"
        - "Unhandled exception: std.core.LinkerVerificationError"
        - "LinkerVerificationError: Lvariable_removing_n/ETSGLOBAL;"
        - "at std.core.LinkerError.<ctor> (RuntimeLinkerErrors.ets:20:0)"
        - "at std.core.LinkerVerificationError.<ctor> (RuntimeLinkerErrors.ets:30:0)"
---*/

import { N } from './variable_removing_n.libr.ets'

function main() {
    arktest.assertEQ(N.v, 'abc')
}
