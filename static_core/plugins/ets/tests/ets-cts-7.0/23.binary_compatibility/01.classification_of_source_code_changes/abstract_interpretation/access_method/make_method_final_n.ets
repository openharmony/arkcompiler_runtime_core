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
desc: Test for binary compatibility. Making a method final breaks overriding subclass.
files: ['./make_method_final_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
  verify-new-libs:
    - "E/verifier: Cannot link class: Method overrides final method Lmake_method_final_n/C2;foo Lmake_method_final_n/libr/C1;foo"
  runtime-with-new-libs.ark:
    - "E/verifier: Cannot link class: Method overrides final method Lmake_method_final_n/C2;foo Lmake_method_final_n/libr/C1;foo"
---*/

import * as LIB from './make_method_final_n.libr.ets'

class C2 extends LIB.C1 {
    foo(): string { return 'final method overridden' }
}

function main(): void {
    let c2 = new C2();
    arktest.assertEQ(c2.foo(), 'final method overridden');
}
