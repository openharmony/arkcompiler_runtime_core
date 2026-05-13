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
files: ['./change_generic_method_signature_type_params_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs:
    - "E/verifier: Cannot link class: Cannot find method 'm (LY;)LY;' in class 'change_generic_method_signature_type_params_n.libr.C'"
    - "E/verifier: Verifier error 54: Cache miss for method at offset"
    - "E/verifier: Verifier error 62: Cannot resolve method id"
    - "E/verifier: Error: method void change_generic_method_signature_type_params_n.ETSGLOBAL::main() failed to verify"
    runtime-with-new-libs.ark:
    - "E/verifier: Cannot link class: Cannot find method 'm (LY;)LY;' in class 'change_generic_method_signature_type_params_n.libr.C'"
    - "E/verifier: Verifier error 54: Cache miss for method at offset"
    - "E/verifier: Verifier error 62: Cannot resolve method id"
    - "E/verifier: Method verification failed: void change_generic_method_signature_type_params_n.ETSGLOBAL::main()"
    - "E/verifier: Class verification failed: 'change_generic_method_signature_type_params_n.ETSGLOBAL' method: void change_generic_method_signature_type_params_n.ETSGLOBAL::main()"
    - "E/runtime: Cannot initialize class 'change_generic_method_signature_type_params_n.ETSGLOBAL'"
    - "Unhandled exception: std.core.LinkerVerificationError"
    - "LinkerVerificationError: Lchange_generic_method_signature_type_params_n/ETSGLOBAL;"
    - "at std.core.LinkerError.<ctor> (RuntimeLinkerErrors.ets:20:0)"
    - "at std.core.LinkerVerificationError.<ctor> (RuntimeLinkerErrors.ets:30:0)"
---*/

import * as LIB from './change_generic_method_signature_type_params_n.libr.ets'

function main(): void {
    let c = new LIB.C()
    arktest.assertEQ(c.m<Number>(1), 1)
}
