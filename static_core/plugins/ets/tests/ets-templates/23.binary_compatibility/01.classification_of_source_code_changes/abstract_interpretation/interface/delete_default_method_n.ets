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
desc: Test for binary compatibility. Removing an interface method with default implementation breaks compatibility.
files: ['./delete_default_method_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
  verify-new-libs:
    - "E/verifier: Cannot link class: Cannot find method 'foo ()Lstd/core/String;' in class 'delete_default_method_n.libr.I1'"
  runtime-with-new-libs.ark:
    - "E/verifier: Cannot link class: Cannot find method 'foo ()Lstd/core/String;' in class 'delete_default_method_n.libr.I1'"
---*/

import * as LIB from './delete_default_method_n.libr.ets'

class C1 implements LIB.I1 {}

function main(): void {
    let c1 = new C1();
    arktest.assertEQ(c1.foo(), 'Delete interface method test')
}
