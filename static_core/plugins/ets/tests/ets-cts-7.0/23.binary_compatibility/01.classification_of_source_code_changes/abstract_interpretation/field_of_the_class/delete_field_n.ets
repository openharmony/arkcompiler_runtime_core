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
desc: Test for binary compatibility. Deleting a non-private field breaks compatibility.
files: ['./delete_field_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
  verify-new-libs:
    - "E/verifier: Cannot link class: Cannot find field 'f' in class 'delete_field_n.libr.C1'"
  runtime-with-new-libs.ark:
    - "E/verifier: Cannot link class: Cannot find field 'f' in class 'delete_field_n.libr.C1'"
---*/

import * as LIB from './delete_field_n.libr.ets'

function main(): void {
    let c1 = new LIB.C1();
    c1.f;
}
