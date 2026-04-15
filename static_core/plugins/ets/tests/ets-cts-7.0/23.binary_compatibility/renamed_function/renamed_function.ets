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
negative_steps: [runtime-with-new-libs.ark]
expected_error:
    runtime-with-new-libs.ark:
    - "E/verifier: Cannot link class: Cannot find method 'a ()D' in class 'renamed_function.libr.ETSGLOBAL'"
    - "E/verifier: Verifier error 54: Cache miss for method at offset"
    - "E/verifier: Verifier error 62: Cannot resolve method id"
    - "E/verifier: Method verification failed: void renamed_function.ETSGLOBAL::main()"
    - "E/verifier: Class verification failed: 'renamed_function.ETSGLOBAL' method: void renamed_function.ETSGLOBAL::main()"
    - "E/runtime: Cannot initialize class 'renamed_function.ETSGLOBAL'"
    - "Unhandled exception: std.core.LinkerVerificationError"
    - "LinkerVerificationError: Lrenamed_function/ETSGLOBAL;"
    - "at std.core.LinkerError.<ctor>"
    - "at std.core.LinkerVerificationError.<ctor>"
files: ['./renamed_function.libr.ets']
---*/

import { a } from './renamed_function.libr.ets';

function main(): void {
    a()
}
