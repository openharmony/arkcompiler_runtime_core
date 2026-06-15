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
    Test for binary compatibility. Adding required method to interface
    without default implementation may break the binary compatibility.
files: ['./add_required_method_to_interface_no_def_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs: |-
        TODO: Expected error message that required interface method is not implemented
    runtime-with-new-libs.ark: |-
        TODO: Expected error message that required interface method is not implemented
---*/

import { A, I1 } from './add_required_method_to_interface_no_def_n.libr.ets'

class C implements I1 {
    m(p: A): string {
        return "C.A"
    }
}

((): I1 => new C)().m(new A)
