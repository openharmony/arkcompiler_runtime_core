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
desc: Test for binary compatibility. Adding an annotation on a class is compatible.
files: ['./add_type_param_constraint_1_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs:
    - "E/verifier: Cannot link class: Cannot find method '<ctor> (LY;)V' in class 'add_type_param_constraint_1_n.libr.C'"
    - "E/verifier: Cannot link class: Cannot find method 'get ()LY;' in class 'add_type_param_constraint_1_n.libr.C'"
    - "E/verifier: Verifier error 53: Cache miss for class at offset"
    - "E/verifier: Verifier error 63: Cannot resolve class id"
    - "E/verifier: Verifier error 54: Cache miss for method at offset"
    - "E/verifier: Verifier error 62: Cannot resolve method id"
    - "E/verifier: Error: method void add_type_param_constraint_1_n.ETSGLOBAL::main() failed to verify"
    runtime-with-new-libs.ark:
    - "E/verifier: Cannot link class: Cannot find method '<ctor> (LY;)V' in class 'add_type_param_constraint_1_n.libr.C'"
    - "E/verifier: Cannot link class: Cannot find method 'get ()LY;' in class 'add_type_param_constraint_1_n.libr.C'"
    - "E/verifier: Verifier error 53: Cache miss for class at offset"
    - "E/verifier: Verifier error 63: Cannot resolve class id"
    - "E/verifier: Verifier error 54: Cache miss for method at offset"
    - "E/verifier: Verifier error 62: Cannot resolve method id"
    - "E/verifier: Method verification failed: void add_type_param_constraint_1_n.ETSGLOBAL::main()"
    - "E/verifier: Class verification failed: 'add_type_param_constraint_1_n.ETSGLOBAL' method: void add_type_param_constraint_1_n.ETSGLOBAL::main()"
    - "E/runtime: Cannot initialize class 'add_type_param_constraint_1_n.ETSGLOBAL'"
    - "Unhandled exception: std.core.LinkerVerificationError"
    - "LinkerVerificationError: Ladd_type_param_constraint_1_n/ETSGLOBAL;"
    - "at std.core.LinkerError.<ctor> (RuntimeLinkerErrors.ets:20:0)"
    - "at std.core.LinkerVerificationError.<ctor> (RuntimeLinkerErrors.ets:30:0)"
---*/

import * as LIB from './add_type_param_constraint_1_n.libr.ets'

function main(): void {
    let c = new LIB.C<string>("test")
    arktest.assertEQ(c.get(), "test")
}
