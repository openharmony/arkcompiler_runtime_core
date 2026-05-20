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
    03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_arg_unresolved
tags: [negative]
negative_steps: [compile.verifier, verifier, runtime.ark, ark-with-jit.ark, ark-after-aot.ark,
hybrid-ark.ark, ark-with-jit-repeats.ark]
expected_error:
    verifier: 'E/verifier: Verifier error 2011: Cannot resolve method argument type'
    verifier: 'E/verifier: Error: method void union_arg_unresolved.ETSGLOBAL::foo() failed to verify'
    compile.verifier: 'E/verifier: Verifier error 2011: Cannot resolve method argument type'
    compile.verifier: 'E/verifier: Error: method void union_arg_unresolved.ETSGLOBAL::foo() failed to verify'
    runtime.ark: 'E/verifier: Method verification failed: void union_arg_unresolved.ETSGLOBAL::foo()'
    ark-with-jit.ark: 'E/verifier: Method verification failed: void union_arg_unresolved.ETSGLOBAL::foo()'
    ark-after-aot.ark: 'E/verifier: Method verification failed: void union_arg_unresolved.ETSGLOBAL::foo()'
    hybrid-ark.ark: 'E/verifier: Method verification failed: void union_arg_unresolved.ETSGLOBAL::foo()'
    ark-with-jit-repeats.ark: 'E/verifier: Method verification failed: void union_arg_unresolved.ETSGLOBAL::foo()'
---*/

declare class NoImpl {}

class classWithNoImplField {
    public static bar(a: NoImpl | int) {
        return 1;
    }
}

function foo() {
    const klass = new classWithNoImplField();
    classWithNoImplField.bar(1)
}
function main() {
    arktest.expectThrow(() => { foo() }, (e: Error) => e instanceof LinkerVerificationError)
}
