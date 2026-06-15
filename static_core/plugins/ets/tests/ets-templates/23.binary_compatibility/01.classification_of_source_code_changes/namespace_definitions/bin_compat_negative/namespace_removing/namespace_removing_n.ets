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
    Test for binary compatibility. Removing an exported inner namespace from namespace
    breaks the binary compatibility.
files: ['./namespace_removing_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs:
        - "E/verifier: Cannot link class: Cannot find class Lnamespace_removing_n/libr/N/NN;"
        - "E/verifier: Verifier error 55: Cache miss for field at offset"
        - "E/verifier: Verifier error 61: Cannot resolve field id"
        - "E/verifier: Error: method void namespace_removing_n.ETSGLOBAL::main() failed to verify"
    runtime-with-new-libs.ark:
        - "E/verifier: Cannot link class: Cannot find class Lnamespace_removing_n/libr/N/NN;"
        - "E/verifier: Verifier error 55: Cache miss for field at offset"
        - "E/verifier: Verifier error 61: Cannot resolve field id"
        - "E/verifier: Method verification failed: void namespace_removing_n.ETSGLOBAL::main()"
        - "E/verifier: Class verification failed: 'namespace_removing_n.ETSGLOBAL' method: void namespace_removing_n.ETSGLOBAL::main()"
        - "E/runtime: Cannot initialize class 'namespace_removing_n.ETSGLOBAL'"
        - "Unhandled exception: std.core.LinkerVerificationError"
        - "LinkerVerificationError: Lnamespace_removing_n/ETSGLOBAL;"
        - "  at std.core.LinkerError.<ctor> (RuntimeLinkerErrors.ets:20:0)"
        - "  at std.core.LinkerVerificationError.<ctor> (RuntimeLinkerErrors.ets:30:0)"
---*/

import { N } from './namespace_removing_n.libr.ets'

function main() {
    arktest.assertEQ(N.NN.v, 'abc')
}
