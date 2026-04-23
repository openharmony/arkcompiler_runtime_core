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
desc: Test for binary compatibility. Making a method abstract breaks compatibility.
files: ['./make_method_abstract_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
  verify-new-libs:
    - "E/verifier: Error: Call to an abstract method is prohibited."
  runtime-with-new-libs.ark:
    - "E/verifier: Error: Call to an abstract method is prohibited."
---*/

import * as LIB from './make_method_abstract_n.libr.ets'

class C2 extends LIB.C1 {}

function main(): void {
    let c2 = new C2();
    arktest.assertEQ(c2.foo(), 'abstract method access');
}
