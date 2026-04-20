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

{% for c in cases %}
/*---
desc: |-
    {{c.desc}}
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs:
    - "E/verifier: Cannot link class: Cannot find class Lnamespace_renaming_n/libr_{{loop.index0}}/N; in boot panda files: "
    - "E/verifier: Verifier error 55: Cache miss for field at offset"
    - "E/verifier: Verifier error 61: Cannot resolve field id"
    - "E/verifier: Error: method void namespace_renaming_n_{{loop.index0}}.ETSGLOBAL::main() failed to verify"
    runtime-with-new-libs.ark:
    - "E/verifier: Cannot link class: Cannot find class Lnamespace_renaming_n/libr_{{loop.index0}}/N;"
    - "E/verifier: Verifier error 55: Cache miss for field at offset"
    - "E/verifier: Verifier error 61: Cannot resolve field id"
    - "E/verifier: Method verification failed: void namespace_renaming_n_{{loop.index0}}.ETSGLOBAL::main()"
    - "E/verifier: Class verification failed: 'namespace_renaming_n_{{loop.index0}}.ETSGLOBAL' method: void namespace_renaming_n_{{loop.index0}}.ETSGLOBAL::main()"
    - "E/runtime: Cannot initialize class 'namespace_renaming_n_{{loop.index0}}.ETSGLOBAL'"
    - "Unhandled exception: std.core.LinkerVerificationError"
    - "LinkerVerificationError: Lnamespace_renaming_n_{{loop.index0}}/ETSGLOBAL;"
    - "at std.core.LinkerError.<ctor>"
    - "at std.core.LinkerVerificationError.<ctor>"
files: ['./namespace_renaming_n.libr_{{loop.index0}}.ets']
---*/

import { N } from './namespace_renaming_n.libr_{{loop.index0}}.ets'

function main(): void {
    arktest.assertEQ(N.a, 0)
}

{% endfor %}
