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
files: ['./change_type_param_constraint_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs:
    - "E/verifier: Cannot link class: Cannot find method '<ctor> (Lchange_type_param_constraint_n/libr/Base;)V' in class 'change_type_param_constraint_n.libr.Holder'"
    - "E/verifier: Cannot link class: Cannot find method 'get ()Lchange_type_param_constraint_n/libr/Base;' in class 'change_type_param_constraint_n.libr.Holder'"
    - "E/verifier: Verifier error 53: Cache miss for class at offset"
    - "E/verifier: Verifier error 63: Cannot resolve class id"
    - "E/verifier: Verifier error 54: Cache miss for method at offset"
    - "E/verifier: Verifier error 62: Cannot resolve method id"
    - "E/verifier: Error: method void change_type_param_constraint_n.ETSGLOBAL::main() failed to verify"
    runtime-with-new-libs.ark:
    - "E/verifier: Cannot link class: Cannot find method '<ctor> (Lchange_type_param_constraint_n/libr/Base;)V' in class 'change_type_param_constraint_n.libr.Holder'"
    - "E/verifier: Cannot link class: Cannot find method 'get ()Lchange_type_param_constraint_n/libr/Base;' in class 'change_type_param_constraint_n.libr.Holder'"
    - "E/verifier: Verifier error 53: Cache miss for class at offset"
    - "E/verifier: Verifier error 63: Cannot resolve class id"
    - "E/verifier: Verifier error 54: Cache miss for method at offset"
    - "E/verifier: Verifier error 62: Cannot resolve method id"
    - "E/verifier: Method verification failed: void change_type_param_constraint_n.ETSGLOBAL::main()"
    - "E/verifier: Class verification failed: 'change_type_param_constraint_n.ETSGLOBAL' method: void change_type_param_constraint_n.ETSGLOBAL::main()"
    - "E/runtime: Cannot initialize class 'change_type_param_constraint_n.ETSGLOBAL'"
    - "Unhandled exception: std.core.LinkerVerificationError"
    - "LinkerVerificationError: Lchange_type_param_constraint_n/ETSGLOBAL;"
    - "at std.core.LinkerError.<ctor> (RuntimeLinkerErrors.ets:20:0)"
    - "at std.core.LinkerVerificationError.<ctor> (RuntimeLinkerErrors.ets:30:0)"
---*/

import * as LIB from './change_type_param_constraint_n.libr.ets'

function main(): void {
    let h = new LIB.Holder<LIB.Base>(new LIB.Base())
    arktest.assertEQ(h.get().name(), "Base")
}
