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
desc: Test File.
tags: [negative]
files: ['./renamed_namespace.libr.ets']
negative_steps: [runtime-with-new-libs.ark]
expected_error:
    runtime-with-new-libs.ark:
    - "E/verifier: Cannot link class: Cannot find class Lrenamed_namespace/libr/a;"
    - "E/verifier: Verifier error 55: Cache miss for field at offset"
    - "E/verifier: Verifier error 61: Cannot resolve field id"
    - "E/verifier: Method verification failed: void renamed_namespace.ETSGLOBAL::main()"
    - "E/verifier: Class verification failed: 'renamed_namespace.ETSGLOBAL' method: void renamed_namespace.ETSGLOBAL::main()"
    - "E/runtime: Cannot initialize class 'renamed_namespace.ETSGLOBAL'"
    - "Unhandled exception: std.core.LinkerVerificationError"
    - "LinkerVerificationError: Lrenamed_namespace/ETSGLOBAL;"
    - "at std.core.LinkerError.<ctor>"
    - "at std.core.LinkerVerificationError.<ctor>"
---*/

import { a as a } from './renamed_namespace.libr.ets';

function main(): void {
    arktest.assertEQ(a.b, 1)
}
