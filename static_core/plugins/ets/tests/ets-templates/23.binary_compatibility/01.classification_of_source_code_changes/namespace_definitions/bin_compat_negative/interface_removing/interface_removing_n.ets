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
    Test for binary compatibility. Removing an exported interface from namespace
    breaks the binary compatibility.
files: ['./interface_removing_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs:
        - "E/verifier: Cannot link class: Cannot find class Linterface_removing_n/libr/N/I;"
        - "E/verifier: Verifier error 49: Undefined register"
        - "E/verifier: Error: method Y interface_removing_n.%%lambda-lambda_invoke-0::invoke0(interface_removing_n.%%lambda-lambda_invoke-0) failed to verify"
        - "E/verifier: Verifier error 53: Cache miss for class at offset"
        - "E/verifier: Verifier error 63: Cannot resolve class id"
        - "E/verifier: Cannot link class: Cannot find class Linterface_removing_n/libr/N/I;"
        - "E/verifier: Verifier error 2010: Cannot resolve method return type"
        - "E/verifier: Error: method interface_removing_n.libr.N.I interface_removing_n.ETSGLOBAL::lambda_invoke-0() failed to verify"
    runtime-with-new-libs.ark:
        - "E/verifier: Cannot link class: Cannot find class Linterface_removing_n/libr/N/I;"
        - "E/verifier: Verifier error 53: Cache miss for class at offset"
        - "E/verifier: Verifier error 63: Cannot resolve class id"
        - "E/verifier: Verifier error 2010: Cannot resolve method return type"
        - "E/verifier: Method verification failed: interface_removing_n.libr.N.I interface_removing_n.ETSGLOBAL::lambda_invoke-0()"
        - "E/verifier: Class verification failed: 'interface_removing_n.ETSGLOBAL' method: interface_removing_n.libr.N.I interface_removing_n.ETSGLOBAL::lambda_invoke-0()"
        - "E/runtime: Cannot initialize class 'interface_removing_n.ETSGLOBAL'"
        - "Unhandled exception: std.core.LinkerVerificationError"
        - "LinkerVerificationError: Linterface_removing_n/ETSGLOBAL;"
        - "  at std.core.LinkerError.<ctor> (RuntimeLinkerErrors.ets:20:0)"
        - "  at std.core.LinkerVerificationError.<ctor> (RuntimeLinkerErrors.ets:30:0)"
---*/

import { N } from './interface_removing_n.libr.ets'

function main() {
    arktest.assertEQ(((): N.I => new N.A())().m(5), 10)
}
